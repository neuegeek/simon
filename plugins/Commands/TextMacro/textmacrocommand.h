/*
 *   Copyright (C) 2008 Peter Grasch <peter.grasch@bedahr.org>
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

#ifndef SIMON_TEXTMACROCOMMAND_H_4490833F6B8D4F8AB97995EC2C0745D1
#define SIMON_TEXTMACROCOMMAND_H_4490833F6B8D4F8AB97995EC2C0745D1

#include <simonscenarios/command.h>

/**
 *	@class TextMacroCommand
 *	@brief Ressembles one tectmacro
 *
 *	@version 0.1
 *	@date 20.05.2008
 *	@author Peter Grasch
 */
class TextMacroCommand : public Command
{

  private:
    QString text;

  protected:
    bool triggerPrivate(int *state);
    const QMap<QString,QVariant> getValueMapPrivate() const;
    QDomElement serializePrivate(QDomDocument *doc, QDomElement& commandElem);
    bool deSerializePrivate(const QDomElement& commandElem);
    TextMacroCommand() {}

  public:
    static const QString staticCategoryText();
    static const KIcon staticCategoryIcon();

    const KIcon getCategoryIcon() const;
    const QString getCategoryText() const;

    /**
     * @brief Constructor
     *
     *	@author Peter Grasch
     */
    TextMacroCommand(const QString& name, const QString& iconSrc, const QString& description, const QString& text_) : Command(name, iconSrc, description),
    text(text_) {
    }

    STATIC_CREATE_INSTANCE_H(TextMacroCommand);

    /**
     * @brief Returns the text
     *
     *	@author Peter Grasch
     */
    const QString getText() const { return this->text; }

    ~TextMacroCommand() {}

};
#endif
