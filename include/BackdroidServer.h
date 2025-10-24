#ifndef BNR_SERVER_H
#define BNR_SERVER_H

#include <QObject>
#include <QTcpServer>

class BackdroidServer : public QObject {
  Q_OBJECT
private:
  /* data */
  QTcpServer *server;
  void initServer();

public:
  BackdroidServer(QObject *parent = nullptr);
  ~BackdroidServer();

private Q_SLOTS:
  void onNewConnection();

protected:
  void incomingConnection(qintptr handle);
};

#endif
