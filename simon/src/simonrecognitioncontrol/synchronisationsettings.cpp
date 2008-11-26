/*
 *   Copyright (C) 2008 Peter Grasch <grasch@simon-listens.org>
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

#include "synchronisationsettings.h"
#include "recognitionconfiguration.h"
#include "recognitioncontrol.h"

#include <QListWidgetItem>

#include <KIcon>
#include <KMessageBox>
#include <KProgressDialog>

/**
 * \brief Constructor - inits the help text and the gui
 * \author Peter Grasch
 * @param parent the parent of the widget
 */
SynchronisationSettings::SynchronisationSettings(QWidget* parent, const QVariantList& args): KCModule(KGlobal::mainComponent(), parent)
{
	Q_UNUSED(args);

	dlg=0;
	ui.setupUi(this);

	ui.pbLoadList->setIcon(KIcon("view-refresh"));
	ui.pbSelectModel->setIcon(KIcon("dialog-ok-apply"));

	addConfig(RecognitionConfiguration::self(), this);
	
	connect(ui.pbLoadList, SIGNAL(clicked()), this, SLOT(loadList()));
	connect(ui.pbSelectModel, SIGNAL(clicked()), this, SLOT(selectModel()));
	connect(ui.lwModels, SIGNAL(currentRowChanged(int)), this, SLOT(modelSelectionChanged()));
	connect(RecognitionControl::getInstance(), SIGNAL(modelsAvailable(QList<QDateTime>)), this, SLOT(displayList(QList<QDateTime>)));
}

void SynchronisationSettings::loadList()
{
	if (!dlg)
	{
		dlg = new KProgressDialog(this, i18n("Lade verfügbare Modelle"), i18n("Lade Liste verfügbarer Modelle..."));
		dlg->progressBar()->setValue(0);
		dlg->progressBar()->setMaximum(0);
		dlg->showCancelButton(false);
	}
		else dlg->show();
	
	ui.lwModels->clear();

	if (!RecognitionControl::getInstance()->getAvailableModels())
	{
		KMessageBox::sorry(this, i18n("Die Anfrage konnte nicht an den Server gesendet werden."));
		dlg->reject();
		dlg->deleteLater();
		dlg=0;
	}
}

void SynchronisationSettings::displayList(const QList<QDateTime>& models)
{
	ui.lwModels->clear();
	if (dlg)
	{
		dlg->progressBar()->setValue(1);
		dlg->progressBar()->setMaximum(1);
		dlg->accept();
		dlg->deleteLater();
		dlg=0;
	}
	if (models.isEmpty())
	{
		KMessageBox::sorry(this, i18n("Keine Modelle gefunden"));
		return;
	}

	foreach (const QDateTime& date, models)
	{
		QListWidgetItem *item = new QListWidgetItem(ui.lwModels);
		item->setText(date.toString());
		item->setData(Qt::UserRole, date);
		ui.lwModels->addItem(item);
	}
	
	ui.lwModels->setCurrentRow(models.count() -1);
	ui.pbSelectModel->setEnabled(false); //current model is selected

	ui.lwModels->setEnabled(models.count() > 1);
}


void SynchronisationSettings::modelSelectionChanged()
{
	if (ui.lwModels->currentRow() == ui.lwModels->count()-1)
		//this is either -1 or the already active model
		ui.pbSelectModel->setEnabled(false);
	else ui.pbSelectModel->setEnabled(true);
}

void SynchronisationSettings::showEvent(QShowEvent*)
{
	loadList();
}

void SynchronisationSettings::selectModel()
{
	if (ui.lwModels->currentRow() == -1) {
		KMessageBox::information(this, i18n("Bitte selektieren Sie ein Sprachmodell aus der Liste"));
		return;
	}

	QDateTime modelDate = ui.lwModels->currentItem()->data(Qt::UserRole).toDateTime();
	if (modelDate.isNull()) {
		KMessageBox::sorry(this, i18n("Dieses Modell besitzt kein gültiges Datum."));
		return;
	}

	if (RecognitionControl::getInstance()->switchToModel(modelDate))
		KMessageBox::information(this, i18n("Die Anfrage wurde an den Server gesendet.\n\nBitte überwachen Sie den Fortschritt der Synchronisation im simon Hauptfenster."));
	else 
		KMessageBox::sorry(this, i18n("Die Anfrage konnte nicht an den Server gesendet werden."));
}




SynchronisationSettings::~SynchronisationSettings()
{}



