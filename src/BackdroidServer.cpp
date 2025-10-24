#include "BackdroidServer.h"
#include <QDebug>
#include <QNetworkInterface>

BackdroidServer::BackdroidServer(QObject *parent) : QObject(parent) {
  initServer();
}

BackdroidServer::~BackdroidServer() {
}

void BackdroidServer::initServer() {
  qDebug() << "initServer";
  server = new QTcpServer(this);

  if (!server->listen(QHostAddress::Any, 59560)) {
    qCritical() << tr("Unable to start the server: %1.").arg(server->errorString());
    // close();
    return;
  }

  QString ipAddress;
  QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
  // use the first non-localhost IPv4 address
  for (int i = 0; i < ipAddressesList.size(); ++i) {
    if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()) {
      ipAddress = ipAddressesList.at(i).toString();
      break;
    }
  }

  // if we did not find one, use IPv4 localhost
  if (ipAddress.isEmpty())
    ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

  qDebug() << tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                 "Run the Fortune Client example now.")
                  .arg(ipAddress)
                  .arg(server->serverPort());
}

void BackdroidServer::incomingConnection(qintptr handle) {
  qDebug() << "incoming connection" << handle;
}
