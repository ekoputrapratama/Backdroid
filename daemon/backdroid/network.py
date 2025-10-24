from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot
from PyQt5.QtDBus import QDBusConnection, QDBusInterface, QDBusMessage

SERVICE_NAME = "org.freedesktop.NetworkManager"
NETWORK_CONNECTED_NO_INTERNET = 3
NETWORK_CONNECTED = 4
NETWORK_DISCONNECTED = 1
CONNECTION_TYPE_NONE = ""
CONNECTION_TYPE_ETHERNET = "802-3-ethernet"
CONNECTION_TYPE_WIRELESS = "802-11-wireless"

class NetworkManager(QObject):
  onDeviceAdded = pyqtSignal()
  onConnectionTypeChanged = pyqtSignal(str)
  onConnectivityChanged = pyqtSignal(int)
  def __init__(self, parent=None):
    QObject.__init__(self, parent=parent)
    bus = QDBusConnection.systemBus()
    # self.manager = bus.get_object(SERVICE_NAME, "/org/freedesktop/NetworkManager")
    # self.manager_props = dbus.Interface(self.manager, 'org.freedesktop.DBus.Properties')
    # self.manager_props.connect_to_signal("PropertiesChanged", self.propsChanged)
    self.iface = QDBusInterface(SERVICE_NAME, "/org/freedesktop/NetworkManager", "", bus)
    # self.iface.connect("")
    QDBusConnection.systemBus().connect(SERVICE_NAME, 
      "/org/freedesktop/NetworkManager", 
      SERVICE_NAME, 
      "PropertiesChanged", 
      self.propsChanged
    )
    
  @pyqtSlot(QDBusMessage)
  def propsChanged(self, prop):
    msg = prop.arguments()[0]
    
    # print(f"properties changed {msg}")
    if "Connectivity" in msg.keys():
      self.onConnectivityChanged.emit(msg['Connectivity'])
    
    if "PrimaryConnectionType" in msg.keys():
      self.onConnectionTypeChanged.emit(msg['PrimaryConnectionType'])
    