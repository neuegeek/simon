//
// C++ Implementation: runlinuxbackend
//
// Description: 
//
//
// Author: Peter Grasch <bedahr@gmx.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "runlinuxbackend.h"

#include <QProcess>

/**
 *	@brief Constructor
 *
 *	@author Peter Grasch
 */
RunLinuxBackend::RunLinuxBackend()
{
}

/**
 *	@brief Passes the given URL to the systems URL Handler
 *	
 *	Uses QDesktopServices
 *
 *	@param QString place
 *	The URL
 *	@author Peter Grasch
 */
void RunLinuxBackend::goTo(QString place)
{
	QDesktopServices::openUrl(QUrl(place));
}

/**
 *	@brief Runs the command
 *	
 *	This doesn't need the full path as long as the command lies within $PATH
 *
 *	@param QString command
 *	The command - it will be passed to the system as-is
 *	@author Peter Grasch
 */
void RunLinuxBackend::run(QString command)
{
	QProcess::startDetached  (command);
}


/**
 *	@brief Destructor
 *
 *	@author Peter Grasch
 */
RunLinuxBackend::~RunLinuxBackend()
{
}


