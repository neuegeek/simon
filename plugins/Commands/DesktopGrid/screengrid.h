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

#ifndef SIMON_SCREENGRID_H_8518680DB8A44F5B9A7BE97EADECD6F3
#define SIMON_SCREENGRID_H_8518680DB8A44F5B9A7BE97EADECD6F3

#include <QWidget>

class ScreenGrid : public QWidget
{
  Q_OBJECT
    signals:
  void cancel();

  public:
    ScreenGrid(QWidget* parent);
    void keyPressEvent(QKeyEvent *event);

};
#endif
