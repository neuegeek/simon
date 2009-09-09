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

#ifndef COMMANDSETTINGS_H
#define COMMANDSETTINGS_H

#include "ui_commandsettingsdlg.h"
#include <KCModule>
#include <QStringList>
#include <QVariantList>
#include <QHash>
#include "action.h"

class QListWidgetItem;
class KPageWidget;
class KPageWidgetItem;
class Action;
/**
 * \class CommandSettings
 * \author Peter Grasch
 * \brief KCModule that manages some general options and what command-plugins to load
 * \date 13.08.2007
 * \version 0.1
 */
class CommandSettings : public KCModule
{
Q_OBJECT

signals:
	void actionsChanged(QList<Action::Ptr> actions);
	void recognitionResultsFilterParametersChanged();

private:
	bool forceChangeFlag;
	static CommandSettings* instance;

	Ui::CommandSettingsDlg ui;
	KSharedConfig::Ptr config;
	bool isChanged;

	KPageWidget *pageWidget;
	QHash<KCModule*, KPageWidgetItem*> moduleHash;

	QList<Action::Ptr> actions;

	QList<Action::Ptr> availableCommandManagers();

	QStringList findDefaultPlugins(const QList<Action::Ptr>& actions);
	
	void updatePluginListWidgetItem(QListWidgetItem *item, const QString& trigger);

	void displayList(QListWidget *listWidget, QList<Action::Ptr> actions);

public slots:
	virtual void save();
	virtual void load();
	virtual void defaults();
 
private slots:
	void clear();
	void slotChanged();
	void pluginChanged(bool isChanged);
	void activePluginSelectionChanged(QListWidgetItem* activePluginItem);
	void availablePluginSelectionChanged(QListWidgetItem* availablePluginItem);
	void currentTriggerChanged(const QString& newTrigger);
	void applyToAllClicked();
	void initPluginListWidgetItem(QListWidgetItem* item);

public:
	static CommandSettings* getInstance(QWidget *parent=0, const QVariantList& args=QVariantList())
	{
		if (!instance) instance = new CommandSettings(parent, args);
		return instance;
	}

	CommandSettings(QWidget* parent=0, const QVariantList& args=QVariantList());
	QList<Action::Ptr> getActivePlugins();

	float getMinimumConfidence();
	bool useDYM();
	
	void registerPlugIn(KCModule *plugin);
	void unregisterPlugIn(KCModule *plugin);

	~CommandSettings();
};

#endif
