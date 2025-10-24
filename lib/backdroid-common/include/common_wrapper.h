#ifndef COMMON_WRAPPER_H
#define COMMON_WRAPPER_H

#include "android_device.h"
#include "async_process.h"
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QThread>
#include <QtQml/QJSValue>
#include <QtQml/QJSValueList>

// class Stella;

class ShellResult : public QObject {
  Q_OBJECT
public:
  Q_PROPERTY(QString output READ output WRITE setOutput);
  Q_PROPERTY(QString error READ error WRITE setError);
  Q_PROPERTY(int code READ code WRITE setCode);

  void setOutput(QString output);
  QString output() const;
  void setError(QString error);
  QString error() const;
  int code() const;
  void setCode(int code);

private:
  int m_code;
  QString m_output;
  QString m_error;
};

class CommonWrapper : public QObject {
  Q_OBJECT
private:
  QList<AsyncProcess *> asyncProcess;
  void executeCommandFinished(const int code, const QString &cmd, const QStringList &args, const QString &tag,
                              QJSValue cb = QJSValue::UndefinedValue);
  void errorFound(QProcess::ProcessError e, const QString &cmd, const QStringList &args);
  AsyncProcess *getAsyncProcessByTag(const QString &tag);

public:
  AndroidDeviceList *devices;
  explicit CommonWrapper(QObject *parent = nullptr);
  ~CommonWrapper();
  void killAllProcess();
  ShellResult *run(const QString &cmd, const QStringList &args);
  void runAsync(const QString &cmd, const QStringList &args, const QString &tag, QJSValue cb = QJSValue::UndefinedValue, const QString &cwd = NULL);
};

#endif
