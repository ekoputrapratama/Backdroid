#include "fastboot_wrapper.h"
#include "common_wrapper.h"
#include <qt/QtCore/qregularexpression.h>

/**
 * CallbackObject
 */
CallbackObject::CallbackObject() {
}
CallbackObject::~CallbackObject() {
}

/**
 * FastbootWrapper
 */
FastbootWrapper::FastbootWrapper(DeviceWatcher *watcher) : CommonWrapper(nullptr) {

  devices = new AndroidDeviceList();

  QString path = QCoreApplication::applicationDirPath();
  QObject::connect(watcher, &DeviceWatcher::onDeviceAdded, watcher, [this](QVariant dev) {
    deviceAdded(dev);
  });
  QObject::connect(watcher, &DeviceWatcher::onDeviceRemoved, watcher, [this](QVariant dev) {
    deviceRemoved(dev);
  });

#if defined(Q_OS_LINUX)
  this->fastboot = QDir(path).filePath("fastboot");
#elif defined(Q_OS_WIN32)
  this->fastboot = QDir(path).filePath("fastboot.exe");
#endif
}
FastbootWrapper::~FastbootWrapper() {
}

void FastbootWrapper::deviceAdded(const QVariant &dev) {
  getDevices();
}

void FastbootWrapper::deviceRemoved(const QVariant &dev) {
  getDevices();
}

AndroidDeviceList *FastbootWrapper::getDevices() {
  AndroidDeviceList *devices = new AndroidDeviceList();
  ShellResult *result = this->run(fastboot, { "devices" });
  QStringList lines = result->output().split(QRegularExpression("\n|\r|\r\n"), Qt::SkipEmptyParts);

  // this->devices.clear();
  foreach (QString s, lines) {
    AndroidDevice *device = new AndroidDevice();
    QStringList sp = s.split(QRegularExpression(" "), Qt::SkipEmptyParts);

    device->setSerial(sp[0]);
    device->setState(sp[1]);

    devices->append(device);
  }

  foreach (CallbackObject *o, waitCallback) {
    if (o->action == "added" && devices->hasDeviceWithSerial(o->serial)) {
      QJSValue cb = o->callback;
      if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
        cb.call({});
      }
    } else if (o->action == "removed" && !devices->hasDeviceWithSerial(o->serial)) {
      QJSValue cb = o->callback;
      if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
        cb.call({});
      }
    }
  }

  this->devices = devices;
  return devices;
}

void FastbootWrapper::reboot(const QString &serial) {
  run(fastboot, { "-s", serial, "reboot" });
}

void FastbootWrapper::boot(const QString &serial, const QString &filepath, QJSValue cb) {
  runAsync(fastboot, { "-s", serial, "boot", filepath }, "boot-recovery", cb);
}

void FastbootWrapper::flashing(const QString &serial, const QString &action) {
}
void FastbootWrapper::flash(const QString &serial, const QString &partition,
                            const QString &filepath, QJSValue cb) {
  runAsync(fastboot, { "-s", serial, "flash", partition, filepath }, "flash" + partition, cb);
}
void FastbootWrapper::waitDevice(const QString &serial, const QString &action, QJSValue cb) {
  CallbackObject *obj = new CallbackObject();
  obj->serial = serial;
  obj->action = action;
  obj->callback = cb;
  waitCallback.append(obj);
}
ShellResult *FastbootWrapper::getVar(const QString &serial, const QString &name) {
  return run(fastboot, { "-s", serial, "getvar", name });
}
void FastbootWrapper::oem(const QString &serial, const QString &action) {
  run(fastboot, { "-s", serial, "oem", action });
}
