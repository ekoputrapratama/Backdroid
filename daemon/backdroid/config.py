import os
import traceback  # noqa
import typing
import functools
import yaml

from typing import Callable
from os import makedirs
from PyQt5.QtCore import pyqtSignal, pyqtSlot, QFileSystemWatcher, QObject

instance = None
change_filters = {}

def createWindow():
  print("createWindow")

class change_filter:
  def __init__(self, option, function: bool = False) -> None:
    self._option = option
    self._function = function

    if not option in change_filters.keys():
      change_filters[option] = []

    change_filters[option].append(self)

  def checkMatch(self, option: str = None) -> bool:
    """Check if the given option matches the filter."""
    if option is None:
        # Called directly, not from a config change event.
        return True
    elif option == self._option:
        return True
    elif option.startswith(self._option + '.'):
        # prefix match
        return True
    else:
        return False

  def __call__(self, func: Callable) -> Callable:
    """Filter calls to the decorated function.

    Gets called when a function should be decorated.

    Adds a filter which returns if we're not interested in the change-event
    and calls the wrapped function if we are.

    We assume the function passed doesn't take any parameters.

    Args:
        func: The function to be decorated.

    Return:
        The decorated function.
    """
    if self._function:
      @functools.wraps(func)
      def func_wrapper(option: str, value) -> typing.Any:
        """Call the underlying function."""
        if self.checkMatch(option):
          return func(option, value)
        return None
      
      self.callback = func_wrapper
      return func_wrapper
    else:
      @functools.wraps(func)
      def meth_wrapper(self_wrapper, option: str, value) -> typing.Any:
        """Call the underlying function."""
        if self.checkMatch(option):
          return func(self_wrapper, option, value)
        
        return None
      self.callback = meth_wrapper
      return meth_wrapper


class Configuration(QObject):
  _filters = []
  _watcher = None
  _ignoreChanges = False
  changed = pyqtSignal()

  def __init__(self, org, name="settings", path: str = None) -> None:
    QObject.__init__(self, parent = None)
    self.loadFrom = path
    
    defaultConfigPath = os.path.expanduser(f"~/.config/{org}/{name}.yaml")
    
    if not path is None and not os.path.exists(path):
      if path.endswith("yaml"):
        with open(path, "w+") as f:
          f.write("")
    elif path is None:
      
      if not os.path.exists(os.path.dirname(defaultConfigPath)):
        makedirs(os.path.dirname(defaultConfigPath))

      if not os.path.exists(defaultConfigPath):
        with open(defaultConfigPath, "w+") as f:
          f.write("")

      self.loadFrom = defaultConfigPath

    print(f"using config path {self.loadFrom}")
    self._watcher = QFileSystemWatcher(self)
    self._watcher.addPath(self.loadFrom)
    self._watcher.fileChanged.connect(self.onFileChanged)
    self.config = self.loadConfig()

    global instance
    instance = self

  @pyqtSlot(str)
  def onFileChanged(self, file):
    config = self.loadConfig()
    if self.config != config and len(config.keys()) > 0 and not self._ignoreChanges:
      self.config.update(config)
      


  def _filterData(self, key: str, data: str) -> str:
    if not self._filters:
      return data

    for callback in self._filters:
      data = callback(key, data)

    return data

  @classmethod
  def addFilter(cls, callback: Callable[[str, str], None]) -> None:
    cls._filters.append(callback)

  def loadConfig(self, key: str = None) -> dict:
    config = dict()
    try:
      with open(self.loadFrom, 'r') as f:
        data = f.read()
        if key is not None:
            data = self._filterData(key, data)
            config = yaml.safe_load(data)
            config = config[key]
        else:
            config = yaml.safe_load(data)
            if config is None:
              config = dict()

    except Exception:
      print(f"couldn't load config file from {self.loadFrom}")

    return {key: value for key, value in config.items()}

  def keys(self):
    return self.config.keys()
  
  def has(self, key):
    temp = None
    try:
      for k in key.split("."):
        if temp is None:
          temp = self.config[k]
        else:
          temp = temp[k]
    except Exception:
      # print("cannot get config value for {}".format(key))
      temp = None

    return (temp != None)

  def get(self, key: str, defaultValue=None):
    # print(f"get value for key {key}")
    temp = None
    try:
      for k in key.split("."):
        if temp is None:
          temp = self.config[k]
        else:
          temp = temp[k]
    except Exception:
      print("cannot get config value for {}".format(key))
      temp = None

    if temp is None and defaultValue is not None:
      return defaultValue

    # print(f"get value {temp}")
    return temp

  def set(self, key, value, shouldSave = False) -> None:
    # print(f"set value for key {key} to {value}")
    temp = None
    try:
      splitted = key.split(".")
      if len(splitted) > 1:
        max_slice = len(splitted) - 1
        for k in splitted[0:max_slice]:
          if temp is None:
            if not k in self.config.keys():
              self.config[k] = {}

            temp = self.config[k]
          else:
            if not k in temp.keys():
              temp[k] = {}

            temp = temp[k]

        temp[splitted[max_slice]] = value
      else:
        self.config[key] = value

      # self.changed.emit(key, value)
      if shouldSave:
        self.save()
    except Exception as e:
        print("cannot set config value for {}\n{}".format(key, str(e)))
        temp = None

  def save(self, path=None):
    # print("saving settings")
    self._ignoreChanges = True
    if path is None:
      try:
        writable = os.access(os.path.dirname(self.loadFrom), os.W_OK)
        data = yaml.dump(self.config)
        if writable:
          with open(self.loadFrom, "w+") as f:
            f.write(data)
        else:
          print("Cannot save config file, file is not writable.")
      except Exception as e:
        print(f"cannot save config file {str(e)}")
    else:
      try:
        data = yaml.dump(self.config)
        with open(path, "w+") as f:
          f.write(data)
      except Exception as e:
        print(f"cannot save config file {str(e)}")

    self._ignoreChanges = False

  def loadDefaultConfig(self) -> dict:
    load_from_orig = self.loadFrom  # noqa
    self.loadFrom = __file__
    defaultConfig = self.loadConfig("Example")

    return defaultConfig

  @classmethod
  def removeFilter(cls, callback: Callable[[str, str], None]) -> None:
    cls._filters.remove(callback)
