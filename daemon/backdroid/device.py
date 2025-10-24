
from os import path
import struct
import time
import json
# from PyQt5.QtGui import SIGNAL
import stella.adb as adb
from PyQt5.QtCore import (QObject, QVariant, QThread, QCoreApplication, QTimer, 
                          QSettings, pyqtSlot, pyqtSignal, pyqtProperty, Q_CLASSINFO, 
                          QMutex, QWaitCondition, QByteArray)
from PyQt5.QtDBus import QDBusConnection, QDBusMessage, QDBusAbstractAdaptor, QDBusObjectPath, QDBusVariant
from PyQt5.QtNetwork import QTcpServer, QTcpSocket, QHostAddress,QNetworkInterface, QHostAddress

DEVICE_MANAGER_PATH = "/io/github/stella/DeviceManager"
DEVICE_MANAGER_INTERFACE = "io.github.stella.DeviceManager"
DEVICE_INTERFACE = "io.github.stella.Device"

class DeviceHandler(QThread):
  isAuthorized = False
  _shouldTerminate = False
  _shouldRestart = False
  _condition = QWaitCondition()
  _mutex = QMutex()
  _socket = None
  _threads = []
  _messages = []
  _dataLen = 0
  _buffer = bytearray(b"")
  _isRequestingRoot = False
  _info = None

  onNewMessage = pyqtSignal(QDBusVariant)
  onDeviceDisconnected = pyqtSignal()
  onInfoReceived = pyqtSignal(dict)
  def __init__(self,descriptor, parent=None):
    QObject.__init__(self, parent=parent)

    self._descriptor = descriptor
    
  def _onDeviceDisconnected(self):
    print(f"device diconnected {self.serial}")
    self.onDeviceDisconnected.emit()


  def start(self):
    self._socket = QTcpSocket(self)
    self._socket.setSocketDescriptor(self._descriptor)
    self._socket.readyRead.connect(self.readyRead)
    self._socket.disconnected.connect(self._onDeviceDisconnected)
    if not self.isRunning():
      super().start(QThread.Priority.NormalPriority)
    else:
      self._shouldRestart = True
      self._condition.wakeAll()

    
  def stop(self):
    print("stopping client handler...")
    if self._info['isRooted']:
      adb.disconnect(self._info['ip'], 5555)
      buffer = struct.pack(f"!bi", 5, 0)
      self.send(buffer)
      # wait for a while until the message to disable adb wireless is sent
      time.sleep(0.4)

    self._shouldTerminate = True
    self._condition.wakeAll()
    self._socket.disconnected.disconnect(self._onDeviceDisconnected)
    self._socket.readyRead.disconnect(self.readyRead)
    self._socket.close()
    self._socket.deleteLater()
    while(super().isRunning()):
      time.sleep(0.1)

  def isRooted(self):
    if self._info is None:
      return False
    
    return self._info['isRooted']

  def name(self):
    if self._info is None:
      return None

    return self._info['name']

  def serial(self):
    if self._info is None:
      return None
      
    return self._info['serial']
  
  def send(self, msg):
    print(f"sending message to device {msg}")
    self._messages.append(msg)
    if not self.isRunning():
      super().start(QThread.Priority.NormalPriority)
    else:
      self._shouldRestart = True
      self._condition.wakeAll()

  def readyRead(self):
    
    chunk = self._socket.readAll()
    length = chunk.size()

    # this is always the first data the client send to identify the device
    # it will probably a chunk if the data size is getting bigger, so for now we dont need 
    # to save the chunk to instance variable
    if self._info is None:
      info = str(chunk.data().strip(b'\x00').decode('ascii'))
      print(info)
      self._info = json.loads(info)
      
      print("New device connected")
      print(f"Name      : {self._info['name']}")
      print(f"Serial    : {self._info['serial']}")
      print(f"isRooted  : {self._info['isRooted']}")
      print(f"IP        : {self._info['ip']}")
      if self._info['isRooted']:
        buffer = struct.pack(f"!bi", 5, 1)
        self.send(buffer)
        # wait for a while until the message to enable adb wireless is sent
        time.sleep(0.4)
        adb.connect(self._info['ip'], 5555)
        
      self.onInfoReceived.emit(self._info)
      return

    self.onNewMessage.emit(QDBusVariant(QVariant(chunk)))
    

  def run(self):
    self._mutex.lock()
    descriptor = self._descriptor
    # do not use socket from instance variable or pass self 
    # as a parent of QObject here cuz this method is running
    # in a different thread
    socket = QTcpSocket()
    socket.setSocketDescriptor(descriptor)
    self._mutex.unlock()

    while(True):
      
      while(len(self._messages) > 0):
        self._mutex.lock()
        msg = self._messages.pop(0)
        self._mutex.unlock()
        socket.write(msg)
        socket.flush()
      

      self._mutex.lock()
      if not self._shouldRestart:
        self._condition.wait(self._mutex)
      self._shouldRestart = False
      self._mutex.unlock()

      if self._shouldTerminate:
        socket.close()
        socket.deleteLater()
        break

class Device(QObject):
  _handler = None
  onNewMessage = pyqtSignal(QDBusVariant)
  onInfoUpdated = pyqtSignal()
  Q_CLASSINFO("D-Bus Interface", DEVICE_INTERFACE)
  Q_CLASSINFO("D-Bus Introspection",
    f'  <interface name="{DEVICE_INTERFACE}">\n'
    '    <method name="sendMessage">\n'
    '      <arg direction="in" type="v" name="message"/>\n'
    '    </method>\n'
    '    <signal name="onNewMessage">\n'
    '      <arg type="v" name="device" />\n'
    '    </signal>\n'
    '    <method name="serial">\n'
    '      <arg direction="out" name="serial" type="s" />\n'
    '    </method>\n'
    '    <method name="name">\n'
    '      <arg direction="out" name="name" type="s" />\n'
    '    </method>\n'
    '    <method name="isRooted">\n'
    '      <arg direction="out" name="rooted" type="b" />\n'
    '    </method>\n'
    '  </interface>\n')

  def __init__(self, descriptor, path=None, parent=None):
    QObject.__init__(self, parent=parent)
    self._path = path
    bus = QDBusConnection.sessionBus()
    bus.registerObject(path, self, QDBusConnection.ExportAllSlots | QDBusConnection.ExportAllSignals)
    self._handler = DeviceHandler(descriptor, self)
    self._handler.onNewMessage.connect(self._onNewMessage)
    self._handler.onDeviceDisconnected.connect(self._onDeviceDisconnected)
    self._handler.onInfoReceived.connect(self._onDeviceChanged)
    

  @pyqtSlot(result=str)
  def serial(self):
    return self._handler.serial()

  @pyqtSlot(result=str)
  def name(self):
    return self._handler.name()

  @pyqtSlot(result=bool)
  def isRooted(self):
    return self._handler.isRooted()

  @pyqtSlot(QDBusVariant)
  def _onNewMessage(self, msg):
    # count = self.receivers(self.onNewMessage)
    # print(f"receiver count {count}")
    self.onNewMessage.emit(msg)
  
  @pyqtSlot()
  def _onDeviceDisconnected(self):
    self.stop()

  @pyqtSlot()
  def _onDeviceChanged(self):
    self.parent().deviceChanged(self)

  @pyqtSlot(QDBusVariant)  
  def sendMessage(self, msg):    
    self._handler.send(msg.variant())

  def start(self):
    self._handler.start()

  def stop(self):
    bus = QDBusConnection.sessionBus()
    bus.unregisterObject(self._path)
    self._handler.stop()
    self.parent().deviceRemoved(self)


class DeviceManagerServer(QTcpServer):
  _devices = []
  
  def __init__(self, parent=None):
    QObject.__init__(self, parent=parent)

  def start(self):
    print("starting stella server...")
    if super().isListening():
      return

    if not super().listen(QHostAddress.Any, 59560):
      print("Unable to start server")
      return

    ipAddress = None
    ipAddresses = QNetworkInterface.allAddresses()

    for address in ipAddresses:
      if address != QHostAddress.LocalHost and address.toIPv4Address() > 0:
        print(f"address {address.toString()}")
        ipAddress = address.toString()

    if ipAddress is None:
      ipAddress = QHostAddress(QHostAddress.LocalHost).toString()

    print(f"The server is running on\nIP: {ipAddress}\nport: {self.serverPort()}")

  def stop(self):
    print("stopping server...")
    if super().isListening():
      self.close()
    
    for device in self._devices:
      device.stop()
      device.deleteLater()

    self._devices = []

    while(super().isListening()):
      time.sleep(0.1)
    
    self.deleteLater()
    print("server stopped")

  def restart(self):
    print("restarting server...")
    self.stop()
    self.start()

  def deviceRemoved(self, device):
    self.parent().onDeviceRemoved.emit(QDBusObjectPath(device._path))
    self._devices.remove(device)

  def deviceChanged(self, device):
    self.parent().onDeviceChanged.emit(QDBusObjectPath(device._path), device.serial())

  def incomingConnection(self, descriptor):
    devicePath = f"{DEVICE_MANAGER_PATH}/Device{len(self._devices)}"
    print(f"new device detected {devicePath}")
    device = Device(descriptor, devicePath, self)
    device.setObjectName(devicePath)
    self._devices.append(device)
    self.parent().onDeviceAdded.emit(QDBusObjectPath(devicePath))
    device.start()
    
  
  def getDevicePaths(self):
    ""
    return [QDBusObjectPath(d.objectName()) for d in self._devices]
  
class DeviceManagerDaemon(QObject):
  Q_CLASSINFO("D-Bus Interface", DEVICE_MANAGER_INTERFACE)
  Q_CLASSINFO("D-Bus Object Path", DEVICE_MANAGER_PATH)
  Q_CLASSINFO("D-Bus Introspection",
    f'  <interface name="{DEVICE_MANAGER_INTERFACE}">\n'
    '    <method name="getDevices">\n'
    '      <arg direction="out" type="av" name="devices"/>\n'
    '    </method>\n'
    '    <signal name="onDeviceAdded">\n'
    '       <arg type="o" name="path" />\n'
    '    </signal>\n'
    '    <signal name="onDeviceChanged">\n'
    '       <arg type="o" name="path" />\n'
    '       <arg type="s" name="serial" />\n'
    '    </signal>\n'
    '    <signal name="onDeviceRemoved">\n'
    '       <arg type="o" name="path" />\n'
    '    </signal>\n'
    '  </interface>\n')
  onDeviceAdded = pyqtSignal(QDBusObjectPath)
  onDeviceChanged = pyqtSignal(QDBusObjectPath, str)
  onDeviceRemoved = pyqtSignal(QDBusObjectPath)
  def __init__(self, bus, parent=None):
    QObject.__init__(self, parent)
    bus.registerObject(DEVICE_MANAGER_PATH, self, QDBusConnection.ExportAllSlots | QDBusConnection.ExportAllSignals)
    self._server = DeviceManagerServer(self)
    self._server.start()
    

  def stop(self):
    self._server.stop()
    del self._server

  @pyqtSlot(result=list)
  def getDevices(self):
    return self._server.getDevicePaths()
