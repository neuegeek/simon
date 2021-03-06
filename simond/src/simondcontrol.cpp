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

#include "simondcontrol.h"
#include "recognitioncontrolfactory.h"
#include <simonddatabaseaccess/databaseaccess.h>

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KDebug>

SimondControl::SimondControl(QObject* parent) : QTcpServer(parent),
db(new DatabaseAccess(this)),
m_recognitionControlFactory(new RecognitionControlFactory())
{
  connect (db, SIGNAL(error(QString)), this, SLOT(handleError(QString)));
}


bool SimondControl::init()
{
  if (!db->init())
    return false;

  //FIXME encryption!
  KConfig config(KStandardDirs::locate("config", "simondrc"));
  KConfigGroup cGroup2(&config, "User");
  m_keepSamples = cGroup2.readEntry("KeepRecognitionSamples", false);
  kDebug() << "FOO1" << m_keepSamples;

  bool isolatedMode = cGroup2.readEntry("IsolatedMode", false);
  kDebug() << "FOO2" << isolatedMode;
  m_recognitionControlFactory->setIsolatedMode(isolatedMode);

  KConfigGroup cGroup(&config, "Network");
  int port = cGroup.readEntry("Port", 4444);
  if (cGroup.readEntry("BindTo", true)) {
    QString hostName = cGroup.readEntry("Host", "127.0.0.1");
    startServer(QHostAddress(hostName), port);
  }
  else {
    startServer(QHostAddress::Any, port);
  }

  if (cGroup.readEntry("LimitedWriteAccess", true)) {
    m_writeAccessHost = cGroup.readEntry("WriteAccessHost", "127.0.0.1");
  }
  else {
    m_writeAccessHost = QHostAddress::Any;
  }
    
  return true;
}


void SimondControl::handleError(const QString& error)
{
  kDebug() << error;
}


void SimondControl::startServer(const QHostAddress& allowedClient, quint16 port)
{
  kWarning() << "Starting server listening on port " << port;
  if (!listen(allowedClient, port)) {
    kWarning() << "Couldn't bind to port";
    exit(0);
  }
  kWarning() << "Server listening on port  " << port;
}


void SimondControl::stopServer()
{
  kDebug() << "Stopping server";

  close();

  foreach (ClientSocket *client, clients) {
    client->close();
  }

  db->deleteLater();
  db = 0;
}


void SimondControl::incomingConnection (int descriptor)
{
  ClientSocket *clientSocket = new ClientSocket(descriptor, db, m_recognitionControlFactory, m_keepSamples, m_writeAccessHost, this);

  //TODO: Implement the "ForceEncryption" setting which only allows encrypted settings
  //(configuration item)
  // 	socket->setProtocol(QSsl::SslV2);
  // 	socket->setCiphers(Settings::getS("Cipher"));
  // 	socket->startServerEncryption();
  // 	socket->ignoreSslErrors();

  connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
    this, SLOT(connectionClosing(QAbstractSocket::SocketState)));

  clients << clientSocket;
}

void SimondControl::connectionClosing(QAbstractSocket::SocketState state)
{
  if (state != QAbstractSocket::ClosingState) return;

  for (int i=0; i<clients.count(); i++) {
    if (clients[i]->state() == state) {
      kDebug() << "Connection dropped from " << clients[i]->localAddress().toString();
      clients[i]->disconnectFromHost();
      clients[i]->waitForDisconnected();
      clients.takeAt(i)->deleteLater();
      i--;
    }
  }
  kDebug() << "client count: " << clients.count();
}


SimondControl::~SimondControl()
{
  stopServer();
  db->deleteLater();
}
