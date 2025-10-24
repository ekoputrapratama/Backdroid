#ifndef FASTBOOT_WRAPPER_H
#define FASTBOOT_WRAPPER_H

#include <QtCore/QObject>
#include <QtCore/QtCore>

#include "android_device.h"
#include "async_process.h"
#include "common_wrapper.h"
#include "devicewatcher.h"

class CallbackObject : public QObject {
  Q_OBJECT
public:
  CallbackObject();
  ~CallbackObject();
  QString action;
  QString serial;
  QJSValue callback;
};

class FastbootWrapper : public CommonWrapper {
  Q_OBJECT
public:
  QString fastboot;

  FastbootWrapper(DeviceWatcher *watcher);
  virtual ~FastbootWrapper();

public slots:
  void deviceAdded(const QVariant &dev);
  void deviceRemoved(const QVariant &dev);
  AndroidDeviceList *getDevices();
  void reboot(const QString &serial);
  void boot(const QString &serial, const QString &filepath, QJSValue cb);
  void flashing(const QString &serial, const QString &action);
  void flash(const QString &serial, const QString &partition, const QString &filepath, QJSValue cb);
  void waitDevice(const QString &serial, const QString &action, QJSValue cb);
  ShellResult *getVar(const QString &serial, const QString &name);
  void oem(const QString &serial, const QString &action);

private:
  QList<CallbackObject *> waitCallback;
};

#endif // FASTBOOT_WRAPPER_H
