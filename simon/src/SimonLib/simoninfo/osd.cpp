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


#include "osd.h"
#include <QString>
#include <KIcon>
#include <QPixmap>
#include <QTimer>
#include <QLabel>
#include <QPalette>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <KLocalizedString>
#include <KStandardDirs>

/**
 * @brief Constructor
 *
 * Creates a new OSD object. It 
 * 	initializes the timer (member)
 *		sets its Timeout to the argument
 *		links the Timeout-event to close() function of QWidget
 *		starts the timer
 * 	initializes the Label (member)
 * 		sets its Text according to the argument
 * 		stets its size and position
 *	sets the Widget geometry
 *		uses a QDesktopWidget to get the desktops geometry
 *	sets the Palette of the Widget to simon-Colors
 *
 * @author Peter Grasch
 */
OSD::OSD(QString message, int timeout, KIcon *icon)
	: QLabel(0, Qt::WindowStaysOnTopHint|Qt::ToolTip|Qt::FramelessWindowHint)
{
	setPixmap(QPixmap(KStandardDirs::locate("appdata", "themes/default/osd.png")));
	
	this->timer = new QTimer();
	
	QDesktopWidget* tmp = QApplication::desktop();

	int x,y;
	
	x=(tmp->width()/2) - 175;
	y=(tmp->height())-120;
	
	this->setGeometry(x, y,350,80);
	connect(this->timer, SIGNAL(timeout()), this, SLOT(deleteLater()));
	
	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, QColor(255, 255, 255, 255));
	setPalette(pal);
	
	this->message = new QLabel(this);
	this->message->setGeometry(86, 10, 255, 60);
	this->message->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
	this->message->setText(message);
	this->message->setWordWrap(true);
	this->message->show();
	
	this->timer->start(timeout);

	this->icon = new QLabel(this);
	this->icon->setGeometry(16, 16, 48, 48);
	if (!icon)
	{
		this->icon->setPixmap(KIcon("simon").pixmap(QSize(48,48)));
	}
	else {
		this->icon->setPixmap(icon->pixmap(48));
	}
	
	show();
}

/**
 * \brief Closes the info on mouse-press
 * @param event The mousevent to comply with qts definition
 */
void OSD::mouseReleaseEvent(QMouseEvent *event)
{
	QWidget::mouseReleaseEvent(event);
	deleteLater();
}

/**
 * @brief Overwritten paintEvent
 *
 * Overloads the Widgets paint-Function to draw the background-Pixmap
 *
 * @author Peter Grasch
 */
// void OSD::paintEvent( QPaintEvent *e )
// {
// 	QPainter painter(this);
// 	QPixmap bg = QPixmap(":/images/osd.png");
// 	painter.drawPixmap(0,0,bg);
// 	QWidget::paintEvent(e);
// }

/**
 * \brief Destructor
 * \author Peter Grasch
 */
OSD::~OSD()
{
    timer->deleteLater();
    message->deleteLater();
    icon->deleteLater();
}

