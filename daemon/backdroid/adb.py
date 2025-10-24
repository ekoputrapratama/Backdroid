import sys
import subprocess

from os import path
from stella.config import Configuration

isLinux = sys.platform.startswith('linux')
isWindows = sys.platform.startswith('win')
adb = None


def findAdbPath():
  global adb
  settings = Configuration("Stella")
  dirs = []
  if settings.has("applicationDir"):
    appDir = settings.get("applicationDir")
    if isLinux:
      if path.exists(path.join(appDir, "adb")):
        adb = path.join(appDir, "adb")
        return
  
  if isLinux:
    dirs.append("/usr/bin")
    dirs.append("/usr/local/bin")
    dirs.append(path.expanduser("~/.local/bin"))

  for p in dirs:
    if path.exists(path.join(p, "adb")):
      adb = path.join(p, "adb")
      break

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

  thread = threading.Thread(target=run_in_thread, args=(popen_args, env))
  threads.append(thread)
  thread.start()

  # returns immediately after the thread starts
  return thread

def connect(ip, port):
  global adb

  if adb is None:
    findAdbPath()

  command = [adb, "connect", f"{ip}:{port}"]
  subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def disconnect(ip, port):
  global adb

  command = [adb, "disconnect", f"{ip}:{port}"]
  subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
