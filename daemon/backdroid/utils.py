import os
import sys
import subprocess
import threading


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
def spawn(popen_args, env=None):
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
    

  if env is None:
    env = os.environ

  thread = threading.Thread(target=run_in_thread, args=(popen_args,env))
  threads.append(thread)
  thread.start()

  # returns immediately after the thread starts
  return thread
