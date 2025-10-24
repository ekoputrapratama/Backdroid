#ifndef ASYNC_PROCESS_H
#define ASYNC_PROCESS_H

#include <QtCore/QObject>
#include <QtCore/QProcess>

typedef void (*AsyncProcessCallback)(int, QString, QString, QString);

class AsyncProcess : public QObject {
  Q_OBJECT
public:
  QString tag;

  AsyncProcess(QString tag, QProcess *process);
  ~AsyncProcess();

  void start(const QString &cmd, const QStringList &args, AsyncProcessCallback cb);
  void kill();
  void terminate();
  void close();

  QProcess::ProcessState running();
  QProcess *getProcess();

private:
  QProcess *process;
};

#endif // ASYNC_PROCESS_H
