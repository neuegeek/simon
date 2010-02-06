/*
 *   Copyright (C) 2009 Peter Grasch <grasch@simon-listens.org>
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


#ifndef SCENARIODISPLAY_H
#define SCENARIODISPLAY_H
#include <QString>
#include <QDomElement>
#include "simonmodelmanagement_export.h"

class Scenario;

class MODELMANAGEMENT_EXPORT ScenarioDisplay{

protected:
	Scenario *scenario;
	virtual void displayScenarioPrivate(Scenario *scenario)=0;
public:
	ScenarioDisplay() : scenario(NULL)
	{}

	void displayScenario(Scenario *scenario);

	Scenario* currentScenario() { return scenario; }

	virtual ~ScenarioDisplay() {}

};

#endif
