#include "devicewatcher.h"
#ifdef Q_OS_LINUX
#include <inttypes.h>
#include <libudev.h>
#include <libusb-1.0/libusb.h>

static inline QStringList listFromEntries(udev_list_entry *l) {
  QStringList list;
  udev_list_entry *entry;

  udev_list_entry_foreach(entry, l) {
    list.append(QString::fromUtf8(udev_list_entry_get_name(entry)));
  }

  return list;
}
#endif

DeviceWatcher::DeviceWatcher(QObject *parent) : QThread(parent) {
  if (!QDBusConnection::sessionBus().isConnected()) {
    fprintf(stderr,
            "Cannot connect to the D-Bus session bus.\n"
            "To start it, run:\n"
            "\teval `dbus-launch --auto-syntax`\n");
    // return 1;
  }
  QDBusServiceWatcher serviceWatcher("io.github.mixaline", QDBusConnection::sessionBus(),
                                     QDBusServiceWatcher::WatchForRegistration
                                         | QDBusServiceWatcher::WatchForUnregistration,
                                     this);

  serviceWatcher.addWatchedService(SERVICE_NAME);
  serviceWatcher.setWatchMode(QDBusServiceWatcher::WatchModeFlag::WatchForRegistration);
  connect(&serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this,
          &DeviceWatcher::serviceRegistered);

  iface = new QDBusInterface(SERVICE_NAME, DEVICE_MANAGER_PATH, DEVICE_MANAGER_INTERFACE,
                             QDBusConnection::sessionBus(), this);

  if (!iface->isValid()) {
    qDebug() << "Cannot initialize dbus interface";
    qWarning() << QDBusConnection::sessionBus().lastError().message();
  }

  // QObject::connect(&serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this,
  //                  &DeviceWatcher::serviceRegistered);

  if (iface->isValid()) {
    QDBusConnection::sessionBus().connect(SERVICE_NAME, DEVICE_MANAGER_PATH,
                                          DEVICE_MANAGER_INTERFACE, "onDeviceAdded", this,
                                          SLOT(deviceAdded(QDBusMessage)));
    QDBusConnection::sessionBus().connect(SERVICE_NAME, DEVICE_MANAGER_PATH,
                                          DEVICE_MANAGER_INTERFACE, "onDeviceChanged", this,
                                          SLOT(deviceChanged(QDBusMessage)));
    QDBusConnection::sessionBus().connect(SERVICE_NAME, DEVICE_MANAGER_PATH,
                                          DEVICE_MANAGER_INTERFACE, "onDeviceRemoved", this,
                                          SLOT(deviceChanged(QDBusMessage)));
  }
#if defined(Q_OS_WIN32)

#elif defined(Q_OS_LINUX)
  int result = libusb_init(&libusbContext);
  if (result != LIBUSB_SUCCESS) {
    qCritical("Failed to initialise libusb. libusb error: %d\n", result);
  }

  udev_ctx = udev_new();
  monitor = udev_monitor_new_from_netlink(udev_ctx, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", "usb_device");
  udev_monitor_filter_add_match_subsystem_devtype(monitor, "tty", NULL);
  udev_monitor_enable_receiving(monitor);
  start();
#endif
}

DeviceWatcher::~DeviceWatcher() {
}

AndroidDeviceList *DeviceWatcher::deviceList() {
  AndroidDeviceList *list = new AndroidDeviceList();
#if defined(Q_OS_LINUX)
  struct udev_list_entry *devices, *entry;
  if (enumerate == nullptr) {
    enumerate = udev_enumerate_new(udev_ctx);
  }

  if (!enumerate) {
    qCritical() << "Cannot create udev enumerate context";
    return nullptr;
  }

  udev_enumerate_add_match_subsystem(enumerate, "usb");
  udev_enumerate_scan_devices(enumerate);

  devices = udev_enumerate_get_list_entry(enumerate);
  if (!devices) {
    qCritical() << "Failed to get device list";
    return nullptr;
  }

  udev_list_entry_foreach(entry, devices) {
    const char *path;

    path = udev_list_entry_get_name(entry);
    struct udev_device *device = udev_device_new_from_syspath(udev_ctx, path);

    // some android device have model named Android
    QString model = QString::fromUtf8(udev_device_get_property_value(device, "ID_MODEL"));
    QString devlinks = listFromEntries(udev_device_get_devlinks_list_entry(device)).join(" ");

    bool isAndroid = (model.contains("Android") || devlinks.contains("android_adb"));

    if (isAndroid) {
      QString vid = QString::fromUtf8(udev_device_get_sysattr_value(device, "idVendor")).toLower();
      QString pid = QString::fromUtf8(udev_device_get_sysattr_value(device, "idProduct")).toLower();
      QString manufacturer
          = QString::fromUtf8(udev_device_get_sysattr_value(device, "manufacturer"));
      QString serial = QString::fromUtf8(udev_device_get_sysattr_value(device, "serial"));
      QString product = QString::fromUtf8(udev_device_get_sysattr_value(device, "product"));

      AndroidDevice *d = new AndroidDevice();

      d->setVendorId(vid);
      d->setProductId(pid);
      d->setSerial(serial);
      d->setManufacturer(manufacturer);
      d->setModel(model);
      d->setProduct(product);
      d->setState("unauthorized");

      // check if we should set odinMode to true
      int count = kSupportedSamsungDeviceCount;
      for (int i = 0; i < count; i++) {
        SamsungDeviceIdentifier idf = supportedSamsungDevices[i];
        if (idf.productId == pid && idf.vendorId == vid) {
          d->setOdinMode(true);
          d->setState("odin");
          d->setModel("Samsung Galaxy Series");
          break;
        }
      }

      list->push(d);
    }

    udev_device_unref(device);
  }
#else
#endif
  return list;
}

#ifdef Q_OS_LINUX
void DeviceWatcher::run() {
  struct udev_device *dev;
  int fd;

  fd = udev_monitor_get_fd(monitor);

  forever {
    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(fd, &fds)) {
      dev = udev_monitor_receive_device(monitor);
      if (dev) {

        QString action = QString::fromUtf8(udev_device_get_action(dev));
        // some android device have model named Android
        QString model = QString::fromUtf8(udev_device_get_property_value(dev, "ID_MODEL"));
        QString devlinks = listFromEntries(udev_device_get_devlinks_list_entry(dev)).join(" ");
        QString manufacturer
            = QString::fromUtf8(udev_device_get_sysattr_value(dev, "manufacturer"));
        QString serial = QString::fromUtf8(udev_device_get_sysattr_value(dev, "serial"));
        QString product = QString::fromUtf8(udev_device_get_sysattr_value(dev, "product"));
        QString vid = QString::fromUtf8(udev_device_get_sysattr_value(dev, "idVendor")).toLower();
        QString pid = QString::fromUtf8(udev_device_get_sysattr_value(dev, "idProduct")).toLower();
        QString subsystem = QString::fromUtf8(udev_device_get_subsystem(dev));
        QString devpath = QString::fromUtf8(udev_device_get_devpath(dev));
        QString devtype = QString::fromUtf8(udev_device_get_devtype(dev));

        bool isAndroid = (model.contains("Android") || devlinks.contains("android_adb"));
        if (action == "add" && isAndroid) {

          AndroidDevice *d = new AndroidDevice();

          d->setId(serial);
          d->setSerial(serial);
          d->setVendorId(vid);
          d->setProductId(pid);
          d->setManufacturer(manufacturer);
          d->setModel(model);
          d->setProduct(product);
          d->setIsUsbDevice(true);

          QVariant device = QVariant::fromValue(d);
          onDeviceAdded(device);
        } else if (action == "remove") {
          QString serial = QString::fromUtf8(udev_device_get_property_value(dev, "ID_SERIAL"));

          AndroidDevice *d = new AndroidDevice();
          d->setSerial(serial);

          QVariant device = QVariant::fromValue(d);
          onDeviceRemoved(device);
        } else if (action == "bind" && isAndroid) {

          AndroidDevice *d = new AndroidDevice();

          d->setId(serial);
          d->setVendorId(vid);
          d->setProductId(pid);
          d->setSerial(serial);
          d->setManufacturer(manufacturer);
          d->setModel(model);
          d->setProduct(product);
          d->setIsUsbDevice(true);

          QVariant device = QVariant::fromValue(d);
          onDeviceChanged(device);
        }

        /* free dev */
        udev_device_unref(dev);
      }
    }
    /* 500 milliseconds */
    usleep(500 * 1000);

    if (shouldTerminate) {
      break;
    }
  }
  udev_monitor_unref(monitor);
  udev_unref(udev_ctx);
}
#endif

void DeviceWatcher::stop() {
  shouldTerminate = true;
  // clang-format off
  while (isRunning());
  // clang-format on
}

void DeviceWatcher::serviceRegistered(const QString &name) {
  qDebug() << "service registered" << name;
  if (name != SERVICE_NAME)
    return;
  if (!iface->isValid()) {
    iface = new QDBusInterface(SERVICE_NAME, DEVICE_MANAGER_PATH, DEVICE_MANAGER_INTERFACE,
                               QDBusConnection::sessionBus(), this);

    QDBusConnection::sessionBus().connect(SERVICE_NAME, DEVICE_MANAGER_PATH,
                                          DEVICE_MANAGER_INTERFACE, "onDeviceAdded", this,
                                          SLOT(deviceAdded(QDBusMessage)));
  }
}

void DeviceWatcher::serviceUnregistered(QString name) {
}

void DeviceWatcher::deviceAdded(QDBusMessage msg) {
  QString devicePath = msg.arguments()[0].toString();
  onDeviceAdded(QVariant());
  qDebug() << "DeviceWatcher new device added";
}

void DeviceWatcher::deviceChanged(QDBusMessage msg) {
  QString devicePath = msg.arguments()[0].toString();
  QString serial = msg.arguments()[1].toString();
  qDebug() << "DeviceWatcher device changed" << serial;
  onDeviceChanged(QVariant());
}

void DeviceWatcher::deviceRemoved(QDBusMessage msg) {
  qDebug() << "DeviceWatcher device removed";
  QString devicePath = msg.arguments()[0].toString();

  onDeviceRemoved(QVariant());
}

void DeviceWatcher::slotDeviceAdded(const QString &dev) {
  // onDeviceAdded(dev);
}

void DeviceWatcher::slotDeviceRemoved(const QString &dev) {
  qDebug("tid=%#llx %s: remove %s", (quintptr)QThread::currentThreadId(), __PRETTY_FUNCTION__,
         qPrintable(dev));
  // onDeviceRemoved(dev);
}
void DeviceWatcher::slotDeviceChanged(const QString &dev) {
  qDebug("tid=%#llx %s: change %s", (quintptr)QThread::currentThreadId(), __PRETTY_FUNCTION__,
         qPrintable(dev));
}
