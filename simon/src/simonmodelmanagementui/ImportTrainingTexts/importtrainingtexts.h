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


#ifndef IMPORTTRAININGTEXTS_H
#define IMPORTTRAININGTEXTS_H

#include <QWizard>

class QWidget;
class QFileDialog;
class QWizardPage;

/**
 * \brief Guides the user through the process of adding a new trainingtext
 * 
 * This class shows a Wizard that walks the user through that process
 * 
 * \author Peter Grasch
 * \date 20.05.2007
 * \version 0.1
*/
class ImportTrainingTexts : public QWizard{

	Q_OBJECT
private:
	QFileDialog *fd;
	int prevId;
public:
    ImportTrainingTexts(QWidget *parent=0);

    void start();


    //creating the pages for the wizard
    QWizardPage* createIntroPage();
    QWizardPage* createSourcePage();
    QWizardPage* createRemoteImportPage();
    QWizardPage* createLocalImportPage();
    QWizardPage* createWorkingPage();
    QWizardPage* createFinishedPage();

    ~ImportTrainingTexts();

};

#endif