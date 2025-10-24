/******************************************************************************
        Name: description
    Copyright (C) 2011-2015 Wang Bin <wbsecg1@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#ifdef Q_OS_LINUX
#include <libudev.h>
#include <libusb-1.0/libusb.h>
#endif
#ifdef Q_OS_WIN32
#include "qdevicewatcher.h"
#endif

#include "android_device.h"
// #include <QDBusConnection>
// #include <QDBusInterface>
#include <QMutex>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/qglobal.h>
#include <QtDBus/QtDBus>
#ifndef __GNUC__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define SERVICE_NAME             "io.github.mixaline"
#define DEVICE_MANAGER_PATH      "/io/github/mixaline/DeviceManager"
#define DEVICE_MANAGER_INTERFACE "io.github.mixaline.DeviceManager"

struct libusb_context;
struct udev;
struct udev_monitor;
struct udev_enumerate;

class DeviceWatcherPrivate : public QObject {
  Q_OBJECT
Q_SIGNALS:
  void deviceAdded(QString dev);
  void deviceRemoved(QString dev);
};

class DeviceWatcher : public QThread {
  Q_OBJECT

public:
  explicit DeviceWatcher(QObject *parent = nullptr);
  ~DeviceWatcher();
#ifdef Q_OS_LINUX
  void run();
#endif
  void stop();
Q_SIGNALS:
  void onDeviceAdded(const QVariant &dev);
  void onDeviceRemoved(const QVariant &dev);
  void onDeviceChanged(const QVariant &dev);

public Q_SLOTS:

  // #ifdef Q_OS_LINUX
  //   void hotplugCallback(struct libusb_context* ctx, struct libusb_device* dev,
  //   libusb_hotplug_event event, void* user_data) {
  //   }
  // #endif
  void slotDeviceAdded(const QString &dev);
  void slotDeviceRemoved(const QString &dev);
  void slotDeviceChanged(const QString &dev);
  void deviceAdded(QDBusMessage msg);
  void deviceRemoved(QDBusMessage msg);
  void deviceChanged(QDBusMessage msg);
  void serviceRegistered(const QString &);
  void serviceUnregistered(QString name);
  AndroidDeviceList *deviceList();

private:
  bool shouldTerminate = false;
  QDBusInterface *iface;
  QList<AndroidDevice *> devices;
#ifdef Q_OS_LINUX
  udev *udev_ctx;
  udev_monitor *monitor;
  udev_enumerate *enumerate = nullptr;
  libusb_context *libusbContext;
#elif defined(Q_OS_WIN32)

#endif
};

#endif // DEVICEWATCHER_H
