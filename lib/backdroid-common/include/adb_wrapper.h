#ifndef ADB_WRAPPER_H
#define ADB_WRAPPER_H

#include <QtCore/QRegularExpression>
#include <QtCore/QObject>
#include <QtCore/QtCore>

#include "android_device.h"
#include "common_wrapper.h"

class AdbWrapper : public CommonWrapper {
  Q_OBJECT
public:
  QString adb;

  AdbWrapper(QObject *parent = nullptr);
  virtual ~AdbWrapper();

public Q_SLOTS:
  AndroidDeviceList *getDevices();
  void startServer(QJSValue cb = QJSValue::UndefinedValue);
  void killServer();
  void connect(const QString &ipAndPort, QJSValue cb = QJSValue::UndefinedValue);
  void disconnect(const QString &ipAndPort, QJSValue cb = QJSValue::UndefinedValue);
  void tcpip(const QString &port, QJSValue cb = QJSValue::UndefinedValue);
  void install(const QString &serial, const QString &apkPath,
               QJSValue cb = QJSValue::UndefinedValue);
  void uninstall(const QString &serial, const QString &packageName,
                 QJSValue cb = QJSValue::UndefinedValue);
  void pull(const QString &serial, const QString &path, const QString &filename,
            QJSValue cb = QJSValue::UndefinedValue);
  void push(const QString &serial, const QString &filename, const QString &path,
            QJSValue cb = QJSValue::UndefinedValue);
  void sideload(const QString &serial, const QString &filepath,
                QJSValue cb = QJSValue::UndefinedValue);
  void reboot(const QString &serial, const QString &toState,
              QJSValue cb = QJSValue::UndefinedValue);
  void waitFor(const QString &serial, const QString &state, QJSValue cb = QJSValue::UndefinedValue);
  void backup(const QString &serial, const QString &cwd, QJSValue cb = QJSValue::UndefinedValue);
  void forward(const QString &serial, QVariantList arguments);
  // void reverse(const QString &serial, QVariantList arguments);
  QVariant shell(const QString &serial, const QStringList &args,
                 QJSValue cb = QJSValue::UndefinedValue);
  bool isUsbDevice(QString serial);
};

#endif // ADBWRAPPER_H
