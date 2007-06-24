//
// C++ Implementation: importremotewizardpage
//
// Description: 
//
//
// Author: Peter Grasch <bedahr@gmx.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "importremotewizardpage.h"
#include "logger.h"


ImportRemoteWizardPage::ImportRemoteWizardPage(QWidget *parent) : QWizardPage(parent)
{}

void ImportRemoteWizardPage::registerField(const QString &name, QWidget *widget, const char* 
		property, const char* changedSignal)
{
	QWizardPage::registerField(name, widget, property, changedSignal);
}

void ImportRemoteWizardPage::fetchList()
{
	QuickDownloader *downloader = new QuickDownloader(this);

	Logger::log("Fetching list of availible remote trainingstexts");
	
	connect (downloader, SIGNAL(downloadFinished(QString)), this, SLOT(importList(QString)));
	downloader->download("http://simon.pytalhost.org/texts/list.xml");
}

void ImportRemoteWizardPage::importList(QString path)
{
	XMLTrainingTextList *tlist = new XMLTrainingTextList(path);
	tlist->load();
	QHash<QString, QString> textlist = tlist->getTrainingTextList();

	QListWidgetItem *item;
	list->clear();
	for (int i=0; i < textlist.count(); i++)
	{
		item = new QListWidgetItem(list);
		item->setText(textlist.keys().at(i));
		item->setData(Qt::UserRole, textlist.values().at(i));
		list->addItem(item);
	}
}
