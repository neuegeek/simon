/*
 *   Copyright (C) 2010 Peter Grasch <grasch@simon-listens.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "soundserver.h"
#include "soundconfig.h"
#include "soundinputclient.h"
#include "soundoutputclient.h"
#include "simonsoundinput.h"
#include "simonsoundoutput.h"

#include <QObject>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>

#include <KDebug>
#include <KLocalizedString>

SoundServer* SoundServer::instance=NULL;

/**
 * \brief Constructor
 */
SoundServer::SoundServer(QObject* parent) : QObject(parent)
{
}


QString SoundServer::defaultInputDevice()
{
	QString systemDefault = QAudioDeviceInfo::defaultInputDevice().deviceName();

	foreach(const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
	{
		if (deviceInfo.deviceName() == "pulse")
		{
			systemDefault = "pulse";
			break;
		}
	}

	return systemDefault;
}

QString SoundServer::defaultOutputDevice()
{
	QString systemDefault = QAudioDeviceInfo::defaultOutputDevice().deviceName();

	foreach(const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
	{
		if (deviceInfo.deviceName() == "pulse")
		{
			systemDefault = "pulse";
			break;
		}
	}

	return systemDefault;
}



bool SoundServer::registerInputClient(SoundInputClient* client)
{
	kDebug() << "Register input client";
	if (client->isExclusive())
	{
		QHashIterator<SimonSound::DeviceConfiguration, SimonSoundInput*> i(inputs);
		while (i.hasNext())
		{
			i.next();
			SimonSoundInput *soundInput = i.value();

			//suspend all other inputs
			soundInput->suspendInputClients();
		}
	}


	bool succ = true;

	SimonSound::DeviceConfiguration clientRequestedSoundConfiguration = client->deviceConfiguration();
	if (!inputs.contains(client->deviceConfiguration())) //recording not currently running
	{
		kDebug() << "No input for this particular configuration... Creating one";

		SimonSoundInput *soundInput = new SimonSoundInput(this);
		connect(soundInput, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
		connect(soundInput, SIGNAL(recordingFinished()), this, SLOT(slotRecordingFinished()));
		//then start recording
		succ = soundInput->startRecording(clientRequestedSoundConfiguration);
		if (!succ)
			//we had to adjust the format slightly and _that_ is already loaded
		{
			soundInput->deleteLater();
		}
		else
		{
			if (inputs.contains(clientRequestedSoundConfiguration))
				soundInput->deleteLater();
			else
				inputs.insert(clientRequestedSoundConfiguration, soundInput);

			if (! (client->deviceConfiguration() == clientRequestedSoundConfiguration) )
				client->setDeviceConfiguration(clientRequestedSoundConfiguration); // found something supported that is very close
		}
	}

	if (succ)
	{
		SimonSoundInput *input = inputs.value(clientRequestedSoundConfiguration);
		input->registerInputClient(client);
	}

	return succ;
}

void SoundServer::slotRecordingFinished()
{
	SimonSoundInput *input = dynamic_cast<SimonSoundInput*>(sender());
	Q_ASSERT(input);

	QHashIterator<SimonSound::DeviceConfiguration, SimonSoundInput*> i(inputs);
	
	while (i.hasNext())
	{
		i.next();
		if (i.value() == input)
			inputs.remove(i.key());
	}
	input->deleteLater();
}

void SoundServer::slotPlaybackFinished()
{
	SimonSoundOutput *output = dynamic_cast<SimonSoundOutput*>(sender());
	Q_ASSERT(output);

	QHashIterator<SimonSound::DeviceConfiguration, SimonSoundOutput*> i(outputs);
	
	while (i.hasNext())
	{
		i.next();
		if (i.value() == output)
			outputs.remove(i.key());
	}
	output->deleteLater();
}


bool SoundServer::deRegisterInputClient(SoundInputClient* client)
{
	kDebug() << "Deregistering input client";

	bool success = true;

	QHashIterator<SimonSound::DeviceConfiguration, SimonSoundInput*> i(inputs);
	while (i.hasNext())
	{
		i.next();
		success = i.value()->deRegisterInputClient(client) && success;
	}

	return success;
}



void SoundServer::suspendRecording()
{
	foreach (SimonSoundInput *in, inputs.values())
		in->suspendInput();
}

void SoundServer::resumeRecording()
{
	foreach (SimonSoundInput *in, inputs.values())
		in->resumeInput();
}

void SoundServer::suspendPlayback()
{
	foreach (SimonSoundOutput *out, outputs.values())
		out->suspendOutput();
}

void SoundServer::resumePlayback()
{
	foreach (SimonSoundOutput *out, outputs.values())
		out->resumeOutput();
}




bool SoundServer::registerOutputClient(SoundOutputClient* client)
{
	kDebug() << "Register output client";
	SimonSound::DeviceConfiguration clientRequestedSoundConfiguration = client->deviceConfiguration();

	bool succ = true;
	if (!outputs.contains(clientRequestedSoundConfiguration))
	{
		//create output for this configuration
		kDebug() << "No output for this particular configuration... Creating one";

		SimonSoundOutput *soundOutput = new SimonSoundOutput(this);
		connect(soundOutput, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
		connect(soundOutput, SIGNAL(playbackFinished()), this, SLOT(slotPlaybackFinished()));
		//then start playback
		succ = soundOutput->startPlayback(clientRequestedSoundConfiguration);
		if (!succ) {
			//failed
			soundOutput->deleteLater();
		} else {
			//we had to adjust the format slightly and _that_ is already loaded
			if (outputs.contains(clientRequestedSoundConfiguration))
				soundOutput->deleteLater();
			else
				outputs.insert(clientRequestedSoundConfiguration, soundOutput);

			if (! (client->deviceConfiguration() == clientRequestedSoundConfiguration) )
				client->setDeviceConfiguration(clientRequestedSoundConfiguration); // found something supported that is very close
		}
	}

	if (succ)
	{
		SimonSoundOutput *output = outputs.value(clientRequestedSoundConfiguration);
		output->registerOutputClient(client);
	}

	return succ;
}

bool SoundServer::deRegisterOutputClient(SoundOutputClient* client)
{
	Q_UNUSED(client);

	kDebug() << "Deregistering output client";

	bool success = true;

	QHashIterator<SimonSound::DeviceConfiguration, SimonSoundOutput*> i(outputs);
	while (i.hasNext())
	{
		i.next();
		success = i.value()->deRegisterOutputClient(client) && success;
	}

	return success;
}


qint64 SoundServer::byteSizeToLength(qint64 bytes, SimonSound::DeviceConfiguration device)
{
	return bytes / (device.channels() * 2 /* 16 bit */ * ((float)device.sampleRate() / 1000.0f));
}

qint64 SoundServer::lengthToByteSize(qint64 length, SimonSound::DeviceConfiguration device)
{
	return length * (device.channels() * 2 /* 16 bit */ * ((float)device.sampleRate() / 1000.0f));
}



bool SoundServer::reinitializeDevices()
{
	kDebug() << "Reinitialize devices...";
	bool succ = true;

	//TODO
	
	//succ = stopPlayback() && succ;
	//succ = startPlayback() && succ;

	//succ = stopRecording() && succ;
	//succ = startRecording() && succ;

	return succ;
}

int SoundServer::getInputDeviceCount()
{
	return SoundConfiguration::soundInputDevices().count();
}

int SoundServer::getOutputDeviceCount()
{
	return SoundConfiguration::soundOutputDevices().count();
}

bool SoundServer::getDefaultToPowerTraining()
{
	return SoundConfiguration::defaultToPowerTraining();
}

int SoundServer::getLevelThreshold()
{
	return SoundConfiguration::level();
}

int SoundServer::getHeadMargin()
{
	return SoundConfiguration::headMargin();
}

int SoundServer::getTailMargin()
{
	return SoundConfiguration::tailMargin();
}

int SoundServer::getShortSampleCutoff()
{
	return SoundConfiguration::skipSamples();
}


/**
 * \brief Destructor
 */
SoundServer::~SoundServer()
{
}


