# Backdroid 🛡️📱

**Backdroid** is a Qt6 Quick application for backing up and restoring Android devices using ADB.  
It provides a clean, native interface for managing device data, with a Python daemon handling background tasks and system-level operations.

---

## 🔧 Features

- 📦 **Full Backup**: Pull apps, contacts, messages, wifi, data, and system data via ADB 
- 🔁 **Restore Support**: Push backed-up files back to device safely
- 🧠 **Device Detection**: Auto-detect connected Android devices (in progress)
- 🗂️ **Modular UI**: Built with Qt6 for cross-platform performance
- 🐍 **Python Daemon**: Handles background tasks and low-level logic

---

## 🚀 Build & Run

### 🔹 Requirements
- Qt 6.x (tested with Qt 6.10)
- Python 3.13+ (for daemon and Settings)

### 🔹 Build Instructions
```bash
git clone https://github.com/ekoputrapratama/backdroid.git
cd backdroid
mkdir build
cmake -B build -G Ninja
ninja
./backdroid
```

## NOTE
- Some backup features like contacts, messages, wifi, data and system data need root access, right now i'm working on some of this problems, read more on the last part of this note
- this thing still cant watch usb devices and save settings because some integration from the old system that needs to run 2 daemon, either combine it into one daemon or make both separate executable so it can be called from the main app. this app use some of the code from [qdevicewatcher](https://github.com/wang-bin/qdevicewatcher) to watch usb devices. only work for linux right now.
- there is some method i have tried to be able to backup contacts, messages and wifi without root, but this will need the android to install the android application and communicate with tcp server to send those data, while the script itself is written in python, so right now i tried to convert it to C++ so it can be included in this app.