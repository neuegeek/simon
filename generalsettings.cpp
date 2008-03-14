//
// C++ Implementation: generalsettings
//
// Description: 
//
//
// Author: Peter Grasch <bedahr@gmx.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "generalsettings.h"
#include <QInputDialog>
#include <QVariant>
#include <QSettings>
#include "settings.h"

/**
 * \brief Constructs a new GeneralSettings object
 * \author Peter Grasch
 * @param parent The parent of the widget
 */
GeneralSettings::GeneralSettings(QWidget* parent): SystemWidget(tr("Allgemeine Einstellungen"), QIcon(":/images/icons/computer.svg"), tr("Grundlegende Einstellungen rund um Simon"), parent)
{
	ui.setupUi(this);
	guessChildTriggers(this);
	hide();


	connect(ui.cbStartSimonOnBoot, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui.cbAskBeforeExit, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));

	connect(ui.leCommands, SIGNAL(editingFinished()), this, SIGNAL(changed()));
	connect(ui.leShortcuts, SIGNAL(editingFinished()), this, SIGNAL(changed()));
	connect(ui.leProgramCategories, SIGNAL(editingFinished()), this, SIGNAL(changed()));
	connect(ui.leTempDir, SIGNAL(editingFinished()), this, SIGNAL(changed()));
	connect(ui.lePathToTexts, SIGNAL(editingFinished()), this, SIGNAL(changed()));

	//set help
	help = tr("Hier k�nnen Sie grundlegende Einstellungen rund um Simon ver�ndern.\n\nDazu z�hlen: Pfade und Sicherheitsabfragen.\n\nEinstellungen in diesem Modul k�nnen kritisch f�r die Verwendung von simon sein.\n\nWenn das H�ckchen \"Starte juliusd wenn n�tig\" aktiviert ist, wird versucht juliusd lokal zu starten, sollte zu keinem laufenden D�mon verbunden werden k�nnen.");
}

/**
 * \brief Determines if the page is completely configured
 * 
 * \author Peter Grasch
 * @return if we are done here
 */
bool GeneralSettings::isComplete()
{
	return ((!ui.leCommands->text().isEmpty()) && (!ui.leShortcuts->text().isEmpty()) && 
			(!ui.leProgramCategories->text().isEmpty()) &&  (!ui.leTempDir->text().isEmpty()) && 
			(!ui.lePathToTexts->text().isEmpty()));
}

/**
 * \brief Applys the changed settings
 * \author Peter Grasch
 * @return Success
 */
bool GeneralSettings::apply()
{
	//performance:
	Settings::set("Performance/MaxDisplayedWords", ui.sbMaxSimultaniouslyShownWords->value());
	//general
	Settings::set("SimonAutostart", ui.cbStartSimonOnBoot->isChecked());
	Settings::set("AskBeforeExit", ui.cbAskBeforeExit->isChecked());

	//paths
	Settings::set("PathToCommands", ui.leCommands->text());
	Settings::set("PathToShortcuts", ui.leShortcuts->text());
	Settings::set("PathToProgramCategories", ui.leProgramCategories->text());
	Settings::set("TempDir", ui.leTempDir->text());
	Settings::set("PathToTexts", ui.lePathToTexts->text());

	Settings::set("ConfigDone", !ui.pbShowFirstRunWizard->isChecked());


// #ifdef __WIN32
// 	QSettings settings;
// 	settings.setPath("Microsoft", "Windows", QSettings::UserScope);
// 	if (ui.cbStartSimonOnBoot->isChecked()) {
// 		// Want to start on boot up
// 		QString appPath = qApp->applicationFilePath();
// 		settings.writeEntry("/CurrentVersion/Run/simon.exe", appPath);
// 	} else {
// 		// Do not want to start on boot up
// 		settings.removeEntry("/CurrentVersion/Run/simon.exe");
// 	}
// #endif

	return true;
}


/**
 * \brief Resets the settings
 * \author Peter Grasch
 * @return Success
 */
bool GeneralSettings::reset()
{
	return init();
}


/**
 * \brief Initialy loads the settings
 * \author Peter Grasch
 * @return Success
 */
bool GeneralSettings::init()
{
	ui.pbShowFirstRunWizard->setChecked(!Settings::get("ConfigDone").toBool());

	//performance
	ui.sbMaxSimultaniouslyShownWords->setValue(Settings::get("Performance/MaxDisplayedWords").toInt());

	//general
	ui.cbStartSimonOnBoot->setChecked(Settings::get("SimonAutostart").toBool());
	ui.cbAskBeforeExit->setChecked(Settings::get("AskBeforeExit").toBool());


	//paths
	ui.leCommands->setText(Settings::get("PathToCommands").toString());
	ui.leShortcuts->setText(Settings::get("PathToShortcuts").toString());
	ui.leProgramCategories->setText(Settings::get("PathToProgramCategories").toString());
	ui.leTempDir->setText(Settings::get("TempDir").toString());
	ui.lePathToTexts->setText(Settings::get("PathToTexts").toString());

	return true;
}





/**
 * \brief Destructor
 * \author Peter Grasch
 */
GeneralSettings::~GeneralSettings()
{
}


