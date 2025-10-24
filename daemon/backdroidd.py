#!/usr/bin/env python
import os
import re
import sys
import fnmatch
import signal
import threading
import subprocess
import configparser
from os import path

from PyQt5.QtCore import (QObject, QThread, QCoreApplication, QTimer, QSettings, pyqtSlot, Q_CLASSINFO)
from PyQt5.QtDBus import QDBusConnection, QDBusMessage, QDBusAbstractAdaptor
from PyQt5.QtNetwork import QTcpServer, QTcpSocket, QHostAddress,QNetworkInterface, QHostAddress

from backdroid.device import DeviceManagerDaemon
from backdroid.network import NetworkManager
from backdroid.config import Configuration

isMac = sys.platform.startswith('darwin')
isLinux = sys.platform.startswith('linux')
isWindows = sys.platform.startswith('win')
isPosix = os.name == 'posix'

SERVICE_NAME = "io.github.backdroid"
SERVICE_ROOT_PATH = "/io/github/backdroid"
DAEMON_PATH = "/io/github/backdroid/Backdroidd"
DEVICE_MANAGER_PATH = "/io/github/backdroid/DeviceManager"

threads = []
processes = []

_SYNC_LOCK = threading.Lock()


# Decorator that adds thread synchronization to a function
def synchronized(lock):
  def _decorator(func):
    def _wrapper(*args, **kwargs):
      lock.acquire()
      ret_value = func(*args, **kwargs)
      lock.release()
      return ret_value
    return _wrapper
  return _decorator


@synchronized(_SYNC_LOCK)
def popen_and_call(popen_args, env):
  """
  Runs the given args in a subprocess.Popen, and then calls the function
  on_exit when the subprocess completes.
  on_exit is a callable object, and popen_args is a list/tuple of args that 
  would give to subprocess.Popen.
  """
  global threads

  def run_in_thread(popen_args, env):
    proc = subprocess.Popen(popen_args,
      env=env,
      shell=False
    )
    processes.append(proc)
    code = proc.wait()

  thread = threading.Thread(target=run_in_thread, args=(popen_args,env))
  threads.append(thread)
  thread.start()

  # returns immediately after the thread starts
  return thread


def findFiles(pattern, path, regex=False):
  matches = []
  for root, dirs, files in os.walk(path):
    for basename in files:
      if not regex:
        if fnmatch.fnmatch(basename, pattern):
          filename = os.path.join(root, basename)
          matches.append(filename)
      else:
        if len(re.findall(pattern, basename)) > 0:
          filename = os.path.join(root, basename)
          matches.append(filename)

  return matches

class PluginInfo(configparser.ConfigParser):
  def __init__(self, filepath):
    super().__init__()
    self._filepath = filepath
    self.read(self._filepath, encoding='utf-8')

  def isValid(self) -> bool:
    """"""
    return self.has_section("Plugin") and self.has_option("Plugin", "Daemon")


class Backdroidd(QDBusAbstractAdaptor):
  _pluginsDaemon = []
  _server = None
  _networkManager = None
  _connectionType = None
  _settings = None
  _pluginsDir = []
  Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)
  # Q_CLASSINFO("D-Bus Object Path", f"{SERVICE_ROOT_PATH}/Backdroid")
  Q_CLASSINFO("D-Bus Introspection",
    f'<interface name="{SERVICE_NAME}">\n'
    ' </interface>\n')

  def __init__(self, bus, parent=None):
    QDBusAbstractAdaptor.__init__(self, parent)
    
    bus.registerObject(SERVICE_ROOT_PATH, self, QDBusConnection.ExportAllSlots | QDBusConnection.ExportAllSignals)
    self._bus = bus
    self._loadedPlugins = {}
    self._networkManager = NetworkManager(self)
    self._managerObject = QObject()
    self._manager = DeviceManagerDaemon(bus, self._managerObject)
    self._settings = Configuration("Backdroid")
    self._loadPlugins()

    # self._bus.registerObject("/io/github/backdroid/DeviceManager", self._manager, QDBusConnection.ExportAdaptors)
    # self._bus.unregisterObject("/io/github")
  
  def stop(self):
    self._manager.stop()
    del self._manager
    del self._networkManager
    del self._settings
    QCoreApplication.quit()
    

  def _loadPlugins(self):
    print("_loadPlugins()")
    settings = self._settings
    if settings.has("pluginsDir"):
      self._pluginsDir = settings.get("pluginsDir")
      print(f"pluginsDir {self.pluginsDir}")
    else:
      if settings.has("applicationDir"):
        appDir = settings.get("applicationDir")
        self._pluginsDir.append(path.join(appDir, "plugins"))

      if isLinux:
        self._pluginsDir.append(path.join("usr", "share", "backdroid", "plugins"))
        self._pluginsDir.append(path.join("usr", "local","share", "backdroid", "plugins"))
        self._pluginsDir.append(path.join(path.expanduser("~/"), ".local", "share","backdroid", "plugins"))

    identities_paths = []
    for directory in self._pluginsDir:
      identities_paths += findFiles("*.plugin", directory)

    print(f"plugins path {identities_paths}")
    plugins = []
    for f in identities_paths:
      info = PluginInfo(f)
      name = f
      if info.has_section("Plugin") and info.has_option("Plugin", "Name"):
        name = info.get("Plugin", "Name")
      else:
        continue

      # if it's already exists it means that user just add a new plugins directory
      if name in self._loadedPlugins.keys():
        continue

      if not info.isValid():
        continue
        # log.plugins.debug(f"Plugin identity {name} is not valid, please read documentation "
        #                     "about how to write plugin.")
      else:
        parentdir = os.path.dirname(f)
        name = info.get("Plugin", "Name")
        module_path = os.path.join(parentdir, info.get("Plugin", "Daemon"))
        daemon = info.get("Plugin", "Daemon")
        if not settings.has(f"plugins.{name}.daemon"):
          settings.set(f"plugins.{name}.daemon.path", module_path)
          settings.set(f"plugins.{name}.daemon.enabled", True)
          settings.save()

        if os.path.exists(module_path):
          plugins.append(module_path)
        else:
          log.plugins.warning(f"module specified in {f} doesn't exists, it will be ignored.")

    print(f"{len(plugins)} daemon found.")
    for plugin in plugins:
      try:
        env = os.environ 
        cmd = [plugin]
        print(f"running daemon {cmd}")
        popen_and_call(cmd, env)
      except Exception as e:
        print(str(e))
        pass

  def getDevices(self):
    ""


def sigint_handler(*args):
    """Handler for the SIGINT signal."""
    # if QMessageBox.question(None, '', "Are you sure you want to quit?",
    #                         QMessageBox.Yes | QMessageBox.No,
    #                         QMessageBox.No) == QMessageBox.Yes:
    QCoreApplication.quit()


 
if __name__ == "__main__":
  # sys.path.append(path.dirname(__file__))
  
  app = QCoreApplication(sys.argv)
  app.setOrganizationName("Backdroid");
  app.setOrganizationDomain("backdroid.github.io");
  app.setApplicationName("Backdroid");

  bus = QDBusConnection.sessionBus()
  if not bus.isConnected():
    print(f"Cannot connect to the D-Bus session bus.")
  
  print(f"registering dbus service {SERVICE_NAME}")
  if not bus.registerService(SERVICE_NAME):
    print(QDBusConnection.sessionBus().lastError().message())

  backdroidd = Backdroidd(bus, app)
  
  signal.signal(signal.SIGINT, sigint_handler)
  signal.signal(signal.SIGTERM, sigint_handler)
  
  # Python cannot handle signals while the Qt event loop is running.
  # so we need to use QTimer to let the interpreter run from time to time.
  # https://stackoverflow.com/questions/4938723/what-is-the-correct-way-to-make-my-pyqt-application-quit-when-killed-from-the-co
  timer = QTimer()
  timer.start(500)  # You may change this if you wish.
  timer.timeout.connect(lambda: None)  # Let the interpreter run each 500 ms.
  
  def appQuitted():
    backdroidd.stop()
    for p in processes:
      p.kill()

    for t in threads:
      t.join()
      
    
  app.aboutToQuit.connect(appQuitted)
  app.exec()
