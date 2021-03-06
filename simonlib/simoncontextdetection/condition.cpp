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

#include "condition.h"
#include "compoundcondition.h"
#include <QWidget>

Condition::Condition(QObject *parent, const QVariantList &args)
{
    Q_UNUSED(args)

    m_pluginName = "";
    m_satisfied = true;
    m_inverted = false;

    this->setParent(parent);
}

bool Condition::isSatisfied()
{
    if (m_inverted)
    {
        return !m_satisfied;
    }
    else
    {
        return m_satisfied;
    }
}

QDomElement Condition::serialize(QDomDocument *doc)
{
    QDomElement conditionElement = doc->createElement("condition");

    conditionElement.setAttribute("name", m_pluginName);

    QDomElement invertElem = doc->createElement("inverted");
    invertElem.appendChild(doc->createTextNode(m_inverted ? "1" : "0"));

    conditionElement.appendChild(invertElem);

    return privateSerialize(doc, conditionElement);
}

bool Condition::deSerialize(QDomElement elem)
{
    m_pluginName = elem.attribute("name");

    QDomElement inverted = elem.firstChildElement();
    m_inverted = (inverted.text().toInt() == 1);

    return this->privateDeSerialize(elem);
}
