
from PyQt5.QtCore import (QObject, QCoreApplication, QTimer, QSettings, pyqtSlot, pyqtSignal, QByteArray, QVariant)
from PyQt5.QtDBus import (QDBusConnection, QDBusMessage, QDBusAbstractAdaptor, 
                          QDBusObjectPath, QDBusInterface, QDBusServiceWatcher, QDBusReply,
                          QDBusVariant)

SERVICE_NAME = "io.github.stella"
DEVICE_MANAGER_PATH = "/io/github/stella/DeviceManager"
DEVICE_MANAGER_INTERFACE = "io.github.stella.DeviceManager"
DEVICE_INTERFACE = "io.github.stella.Device"


class Device(QObject):
  _iface = None
  _watcher = None
  _path = None
  onNewMessage = pyqtSignal(QByteArray)
  def __init__(self, path, parent=None):
    QObject.__init__(self, parent=parent)
    self._path = path
    bus = QDBusConnection.sessionBus()
    self._iface = QDBusInterface(SERVICE_NAME, path, DEVICE_INTERFACE, bus)
    
    if not self._iface.isValid():
      print(f"Cannot initialize device interface."
            f"Service Name    : {SERVICE_NAME}"
            f"Object Path     : {path}"
            f"Interface Name  : {DEVICE_INTERFACE}")
      print("%s\n" % bus.lastError().message())
      return

    self._watcher = QDBusServiceWatcher(SERVICE_NAME, bus)
    self._watcher.serviceUnregistered.connect(self._serviceUnregistered)

    self.start()
    
  def serial(self):
    ""
    msg = self._iface.call("serial")
    reply = QDBusReply(msg)
    if not reply.isValid():
      return None

    return reply.value()

  def name(self):
    msg = self._iface.call("name")
    reply = QDBusReply(msg)
    if not reply.isValid():
      return None

    return reply.value()

  def isRooted(self):
    msg = self._iface.call("isRooted")
    reply = QDBusReply(msg)
    if not reply.isValid():
      return None
      
    rooted = reply.value()
    return rooted

  def start(self):
    bus = QDBusConnection.sessionBus()
    bus.connect(SERVICE_NAME, 
      self._path, 
      DEVICE_INTERFACE, 
      "onNewMessage", 
      self._onNewMessage
    )
    

  def stop(self):
    del self._watcher
    del self._iface
    bus = QDBusConnection.sessionBus()
    bus.disconnect(SERVICE_NAME, 
      self._path,
      DEVICE_INTERFACE, 
      "onNewMessage", 
      self._onNewMessage
    )

  @pyqtSlot(str)
  def _serviceUnregistered(self, name):
    self.stop()

  @pyqtSlot(QDBusMessage)
  def _onNewMessage(self, msg):
    self.onNewMessage.emit(msg.arguments()[0])

  def send(self, buffer):
    self._iface.call("sendMessage", QDBusVariant(QVariant(QByteArray(buffer))))

  

class DeviceManager(QObject):
  _manager = None
  _iface = None
  _devices = []
  _watcher = None
  onDeviceAdded = pyqtSignal(Device)
  onDeviceRemoved = pyqtSignal(str)
  def __init__(self, parent=None):
    QObject.__init__(self, parent=parent)

    bus = QDBusConnection.sessionBus()
    self._watcher = QDBusServiceWatcher(SERVICE_NAME, bus)
    self._watcher.serviceUnregistered.connect(self.stop)
    self._watcher.serviceRegistered.connect(self._serviceRegistered)

    self._iface = QDBusInterface(SERVICE_NAME, DEVICE_MANAGER_PATH, DEVICE_MANAGER_INTERFACE, bus)
    if not self._iface.isValid():
      print(f"Cannot initialize dbus interface.\n"
            f"Service Name    : {SERVICE_NAME}\n"
            f"Object Path     : {DEVICE_MANAGER_PATH}\n"
            f"Interface Name  : {DEVICE_MANAGER_INTERFACE}\n")
      print("%s\n" % bus.lastError().message())
      # del self._iface
      return

    self.start()

  @pyqtSlot(str)
  def _serviceRegistered(self, name):
    self.start()

  @pyqtSlot(str)
  def stop(self, name):
    del self._iface
    # del self._devices[0:]
    bus = QDBusConnection.sessionBus()
    bus.disconnect(SERVICE_NAME, 
      DEVICE_MANAGER_PATH, 
      DEVICE_MANAGER_INTERFACE, 
      "onDeviceAdded", 
      self._deviceAdded
    )
    bus.disconnect(SERVICE_NAME, 
      DEVICE_MANAGER_PATH, 
      DEVICE_MANAGER_INTERFACE, 
      "onDeviceRemoved", 
      self._deviceRemoved
    )
  
  
  def start(self):
    print("starting DeviceManager client")
    bus = QDBusConnection.sessionBus()
    if self._iface is None or not self._iface.isValid():
      self._iface = QDBusInterface(SERVICE_NAME, DEVICE_MANAGER_PATH, DEVICE_MANAGER_INTERFACE, bus)

      if not self._iface.isValid():
        print(f"Cannot initialize dbus interface.\n"
              f"Service Name    : {SERVICE_NAME}\n"
              f"Object Path     : {DEVICE_MANAGER_PATH}\n"
              f"Interface Name  : {DEVICE_MANAGER_INTERFACE}\n")
        print("%s\n" % bus.lastError().message())
        return

    bus.connect(SERVICE_NAME, 
      DEVICE_MANAGER_PATH, 
      DEVICE_MANAGER_INTERFACE, 
      "onDeviceAdded", 
      self._deviceAdded
    )
    bus.connect(SERVICE_NAME, 
      DEVICE_MANAGER_PATH, 
      DEVICE_MANAGER_INTERFACE, 
      "onDeviceRemoved", 
      self._deviceRemoved
    )

  @pyqtSlot(QDBusMessage)
  def _deviceAdded(self, msg):
    print(f"new device added {msg}")
    devicePath = msg.arguments()[0]
    device = Device(devicePath, self)
    self._devices.append(device)
    self.onDeviceAdded.emit(device)

  @pyqtSlot(QDBusMessage)
  def _deviceRemoved(self, msg):
    devicePath = msg.arguments()[0]
    self.onDeviceRemoved.emit(devicePath)
    for device in self._devices:
      if device._path == devicePath:
        device.stop()
        self._devices.remove(device)

  def getDevices(self):
      return self._devices
