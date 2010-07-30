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

#include "dialogcommand.h"

#include <simonactions/actionmanager.h>

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QTimer>
#include <QVariant>

#include <KIcon>
#include <KLocalizedString>
#include <KDebug>

const QString DialogCommand::staticCategoryText()
{
  return i18n("Dialog");
}


const QString DialogCommand::getCategoryText() const
{
  return DialogCommand::staticCategoryText();
}


const KIcon DialogCommand::staticCategoryIcon()
{
  return KIcon("im-user");
}


const KIcon DialogCommand::getCategoryIcon() const
{
  return DialogCommand::staticCategoryIcon();
}


const QMap<QString,QVariant> DialogCommand::getValueMapPrivate() const
{
  QMap<QString,QVariant> out;
  out.insert(i18nc("Label for a bool value", "Switch state"), m_changeDialogState ? i18n("Yes") : i18n("No"));
  out.insert(i18n("Next state"), QString::number(m_nextDialogState));
  out.insert(i18nc("Label for a bool value", "Execute command(s)"), m_executeCommands ? i18n("Yes") : i18n("No"));
  out.insert(i18n("Commands"), m_commands.join("\n"));
  return out;
}


bool DialogCommand::triggerPrivate(int *state)
{
  Q_UNUSED(state);
  kDebug() << "Triggering...";
  if (m_changeDialogState)
    emit requestDialogState(m_nextDialogState);
  if (m_executeCommands)
  {
    for (int i=0; i < m_commands.count();i++)
      ActionManager::getInstance()->triggerCommand(m_commandTypes[i], m_commands[i]);
  }

  return true;
}


QDomElement DialogCommand::serializePrivate(QDomDocument *doc, QDomElement& commandElem)
{
  QDomElement presentationElem = doc->createElement("presentation");
  QDomElement textElem = doc->createElement("text");
  textElem.appendChild(doc->createTextNode(m_text));
  presentationElem.appendChild(textElem);
  QDomElement showIconElem = doc->createElement("icon");
  showIconElem.setAttribute("enabled", m_showIcon );
  presentationElem.appendChild(showIconElem);
  commandElem.appendChild(presentationElem);

  QDomElement autoElem = doc->createElement("auto");
  QDomElement autoActiveElem = doc->createElement("active");
  QDomElement autoTimeoutElem = doc->createElement("timeout");
  autoActiveElem.appendChild(doc->createTextNode(m_activateAutomatically ? "1" : "0"));
  autoTimeoutElem.appendChild(doc->createTextNode(QString::number(m_activateAfter)));
  autoElem.appendChild(autoActiveElem);
  autoElem.appendChild(autoTimeoutElem);
  commandElem.appendChild(autoElem);

  QDomElement switchStateElem = doc->createElement("switchState");
  switchStateElem.setAttribute("enabled", m_changeDialogState);
  switchStateElem.appendChild(doc->createTextNode(QString::number(m_nextDialogState)));
  commandElem.appendChild(switchStateElem);

  QDomElement childCommandsElement = doc->createElement("childCommands");
  childCommandsElement.setAttribute("enabled", m_executeCommands);

  for (int i=0; i < m_commands.count(); i++) {
    QDomElement childComElement = doc->createElement("childCommand");
    QDomElement childTriggerElem = doc->createElement("trigger");
    QDomElement childCategoryElem = doc->createElement("category");

    childTriggerElem.appendChild(doc->createTextNode(m_commands[i]));
    childCategoryElem.appendChild(doc->createTextNode(m_commandTypes[i]));

    childComElement.appendChild(childTriggerElem);
    childComElement.appendChild(childCategoryElem);
    childCommandsElement.appendChild(childComElement);
  }
  commandElem.appendChild(childCommandsElement);

  return commandElem;
}


bool DialogCommand::deSerializePrivate(const QDomElement& commandElem)
{
  QDomElement presentationElem = commandElem.firstChildElement("presentation");
  if (presentationElem.isNull()) return false;

  QDomElement textElem = presentationElem.firstChildElement("text");
  QDomElement showIconElem = presentationElem.firstChildElement("icon");
  m_text = textElem.text();
  m_showIcon = showIconElem.attribute("enabled").toInt();

  QDomElement autoElem = commandElem.firstChildElement("auto");
  QDomElement autoActiveElem = autoElem.firstChildElement("active");
  QDomElement autoTimeoutElem = autoElem.firstChildElement("timeout");

  m_activateAutomatically = (autoActiveElem.text().toInt() != 0);
  m_activateAfter = autoTimeoutElem.text().toInt();

  QDomElement switchStateElem = commandElem.firstChildElement("switchState");
  m_changeDialogState = switchStateElem.attribute("enabled").toInt();
  m_nextDialogState = switchStateElem.text().toInt();

  QDomElement childCommandsElem = commandElem.firstChildElement("childCommands");

  m_executeCommands = childCommandsElem.attribute("enabled").toInt();

  m_commands.clear();
  m_commandTypes.clear();

  QDomElement childCommandElem = childCommandsElem.firstChildElement();

  while (!childCommandElem.isNull()) {
    QDomElement childCommandTriggerElem = childCommandElem.firstChildElement();
    QDomElement childCommandCategoryElem = childCommandTriggerElem.nextSiblingElement();
    m_commands << childCommandTriggerElem.text();
    m_commandTypes << childCommandCategoryElem.text();
    childCommandElem = childCommandElem.nextSiblingElement();
  }
  kDebug() << "Triggers: " << m_commands;
  kDebug() << "Categories: " << m_commandTypes;

  kDebug() << "Deserialized " << m_text;
  return true;
}

void DialogCommand::createStateLink(int thisState)
{
  setBoundState(thisState);
  if (m_changeDialogState)
    setTargetState(SimonCommand::GreedyState|m_nextDialogState);
  else
    setTargetState(thisState);
}

void DialogCommand::autoTrigger()
{
  int fake;
  trigger(&fake);
}

void DialogCommand::presented()
{
  //this command has just been shown to the user...
  
  if (m_activateAutomatically)
    QTimer::singleShot(m_activateAfter, this, SLOT(autoTrigger()));
}

STATIC_CREATE_INSTANCE_C(DialogCommand);
