/*
 *   Copyright (C) 2011 Adam Nash <adam.t.nash@gmail.com>
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

#include "contextview.h"
#include "contextviewprivate.h"
#include <KDE/KLocalizedString>

ContextView::ContextView(QWidget* parent)
: InlineWidget(i18n("Context"),
KIcon("preferences-activities"),
i18n("Modify the context dependence of the scenario"), parent),
d(new ContextViewPrivate(this))
{
  QVBoxLayout *lay = new QVBoxLayout(this);
  lay->addWidget(d);

  hide();
}

void ContextView::displayScenarioPrivate(Scenario *scenario)
{
  d->displayScenario(scenario);
}

ContextView::~ContextView()
{
  d->deleteLater();
}
