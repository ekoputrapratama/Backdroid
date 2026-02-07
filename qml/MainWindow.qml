import QtQuick 2.15
import QtQml 2.15
import QtQuick.Window
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtQml.Models 2.15
import QtQuick.Layouts
import Qt.labs.qmlmodels

// import "../" 1.0
import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars
import "/common/queue.mjs" as Queue

// import "backdroid.mjs" as BackDroid

ApplicationWindow {
  id: mainWindow
  visible: true
  title: qsTr("BackDroid")
  minimumWidth: 850
  minimumHeight: 600
  width: 850
  height: 600

  property int apkCount: 0
  property int currentProgress: 0
  property int maxProgress: 0
  property string progressMessage: ""
  property bool backupIsRunning: false
  property bool restoreIsRunning: false
  property bool allChecked: false

  WindowStateSaver {
    window: mainWindow
    windowName: "BackDroid"
    defaultX: Screen.width / 2 - width / 2
    defaultY: Screen.height / 2 - height / 2
  }

  MessageDialog {
    id: dialog
    text: ""
  }
  Dialog {
    id: devicesDialog
    visible: false
    title: "Select Device"
    anchors.centerIn: parent
    contentItem: Rectangle {
      color: "transparent"
      implicitWidth: 400
      implicitHeight: 100
      SimpleComboBox {
        anchors.centerIn: parent
        label: "Device"
      }
    }
  }
  function onConsoleLog(text) {
    if (typeof text !== 'string')
      text = JSON.stringify(text);

    // logs.append(text);
  }
  function refreshDevices() {
    let devices = backdroid.getDevices().map(d => {
      let device = Object.assign({
        status: 'pending'
      }, Utils.cloneObject(d));
      return device;
    }).reduce((prev, current) => {
      if (current)
        prev[current['serial']] = current;
      return prev;
    }, {});

    for (const serial in devices) {
      let device = devices[serial];
      if (device.state === "offline") {
        continue;
      }
      getDeviceInfo(device);
    }

    let newSerials = Object.keys(devices);
    let serials = Object.keys(Vars.devices);

    for (let serial of serials) {
      let oldDevice = Vars.devices[serial];
      if (!newSerials.includes(serial) && !oldDevice.nextState) {
        delete Vars.devices[serial];
      } else if (newSerials.includes(serial) && oldDevice.nextState) {
        let newDevice = devices[serial];
        if (oldDevice.nextState === newDevice.state) {
          delete newDevice.nextState;
          Vars.devices[serial] = Object.assign(oldDevice, newDevice);
        }
      }
    }

    for (let serial of newSerials) {
      let newDevice = devices[serial];
      if (!serials.includes(serial)) {
        Vars.devices[serial] = newDevice;
      }
    }

    let queue = new Queue.DeviceQueue(devices);
    Vars.setQueue(queue);
  }

  function checkAdbPermission() {
    let devices = Vars.devices;

    for (let serial in devices) {
      let device = devices[serial];

      if (device.state === 'unauthorized') {
        console.info(`Device ${device.model} cannot be unlocked through adb, removing it from the list.`);
      }
    }
  }

  function checkRootStatus(device) {
    let result = backdroid.adb.shell(device.serial, ['su', '-v']);
    let output = result.output;

    let suName = output.replace(/\r|\n/g, '');
    let names = output.split(/\n/);
    let isRooted = false;
    let isRootUser = false;

    if (names && names.length > 1 && !suName.endsWith('not found')) {
      suName = Utils.clearText(names[0]);
      isRooted = true;
    }

    // on some recovery like Philz Touch recovery, the device gave
    // device state instead of recovery state
    // but the shell was running as root
    if (!isRooted) {
      result = backdroid.adb.shell(device.serial, ['whoami']);
      let user = Utils.clearText(result.output);
      isRooted = user === "root";
      suName = "Unknown";
      isRootUser = user === "root";
    } else {
      result = backdroid.adb.shell(device.serial, ['whoami']);
      let user = Utils.clearText(result.output);
      isRootUser = user === "root";
    }

    console.info(`Root status     : ${isRooted}`);
    device.isRooted = isRooted;
    device.isRootUser = isRootUser;
    if (device.isRooted)
      console.info(`Superuser       : ${suName.replace(/\r\n/, '')}`);
  }

  function parseDevices(list) {
    let devices = Vars.devices;
    for (let d of list) {
      let sp = d.split(/\t/);
      let device = {
        serial: sp[0],
        state: sp[1],
        status: 'pending'
      };
      devices[sp[0]] = Object.assign(devices[sp[0]] || {}, device);
    }

    logs.append(`${Object.keys(devices).length} devices found`);
    return devices;
  }

  function getDeviceInfo(device) {
    if (device.state === 'device' || device.state === 'recovery') {
      let manufacturer = backdroid.getProp(device['serial'], 'ro.product.manufacturer');
      let name = backdroid.getProp(device['serial'], 'ro.product.name');
      let brand = backdroid.getProp(device['serial'], 'ro.product.brand');
      let cpuAbi = backdroid.getProp(device['serial'], 'ro.product.cpu.abi');
      let version = backdroid.getProp(device['serial'], 'ro.build.version.release');
      let model = backdroid.getProp(device['serial'], 'ro.product.model');
      let sdk = backdroid.getProp(device['serial'], 'ro.build.version.sdk');

      let preview = backdroid.getProp(device['serial'], 'ro.build.version.preview_sdk');

      if (model === name) {
        model = backdroid.getProp(device['serial'], 'ro.product.device');
      }

      sdk = parseInt(sdk);

      try {
        if (parseInt(preview) > 0) {
          sdk += 1;
        }
      } catch (e) {
        console.error("cannot get preview sdk version");
      }

      if (!device.isUsbDevice) {
        let id = backdroid.getProp(device['serial'], 'ro.serialno');
        device['id'] = Utils.clearText(id);
      }

      manufacturer = Utils.clearText(manufacturer);
      brand = Utils.clearText(brand);
      name = Utils.clearText(name);
      model = Utils.clearText(model);
      cpuAbi = Utils.clearText(cpuAbi);
      version = Utils.clearText(version);

      device['manufacturer'] = manufacturer;
      device['name'] = name;
      device['brand'] = brand;
      device['model'] = model;
      device['sdk'] = sdk;
      device['cpuAbi'] = cpuAbi;
      device['version'] = parseFloat(version);

      console.info(`==================================`);
      if (device['state'] !== 'unauthorized') {
        console.info(`ID              : ${device.id}`);
        console.info(`Serial          : ${device.serial}`);
        console.info(`Mode            : ${device['state']}`);
        console.info(`Vendor ID       : ${device['vendorId']}`);
        console.info(`Product ID      : ${device['productId']}`);
        console.info(`Manufacturer    : ${manufacturer}`);
        console.info(`Brand           : ${brand}`);
        console.info(`Name            : ${name}`);
        console.info(`Model           : ${model}`);
        console.info(`CPU ABI         : ${cpuAbi}`);
        console.info(`Android Version : ${device.version}`);
        console.info(`Sdk Version     : ${device.sdk}`);
        checkRootStatus(device);
      } else {
        console.info("Cannot get device information");
      }
      console.info(`==================================`);
    } else {
      console.info(`==================================`);
      console.info(`Mode : ${device.state}`);
      if (device.manufacturer && device.manufacturer.length > 0) {
        console.info(`Manufacturer : ${device.manufacturer}`);
      }
      if (device.model && device.model.length > 0) {
        console.info(`Model : ${device.model}`);
      }
      if (device.vendorId && device.vendorId.length > 0) {
        console.info(`Vendor ID : ${device['vendorId']}`);
      }
      if (device.productId && device.productId.length > 0) {
        console.info(`Product ID : ${device['productId']}`);
      }
      console.info(`==================================`);
    }
  }

  function onDeviceAdded(device) {
    console.info('new device detected');
    // refreshDevices('add');
    Vars.events.emit('device-added', device);
    // console.log('device', JSON.stringify(device))
  }
  function onDeviceRemoved(device) {
    console.info('device removed');
    refreshDevices();
    Vars.events.emit('device-removed', device);
    // console.log('device', JSON.stringify(device))
  }
  function onDeviceChanged(device) {
    refreshDevices();
    console.info('device changed', JSON.stringify(device));
    // if(!Vars.devices[device.serial]) {
    //   Vars.devices[device.serial] = device;
    //   getDeviceInfo(Vars.devices[device.serial]);
    // }
    Vars.events.emit('device-changed', device);
  }

  function length(o) {
    return Utils.length(o);
  }

  function updateButtonState() {
    if (length(Vars.devices) > 0 && (!backupIsRunning || !restoreIsRunning)) {
      backupBtn.enabled = true;
      restoreBtn.enabled = true;
    } else {
      backupBtn.enabled = false;
      restoreBtn.enabled = false;
    }
  }

  function refreshPackage() {
    let device = Vars.currentDevice;
    listModel.clear();
    if (device && device.state === 'device') {
      apkCount = 0;
      if (backupRb.checked) {
        allChecked = true;
        Utils.getPackages(device, true, true).forEach(pkg => {
          apkCount++;
          listModel.append({
            checked: true,
            pkg: pkg
          });
        });
      } else {
        allChecked = false;
        const sep = Vars.separator;
        const dir = backdroid.backupDir + sep + device.serial;

        const apkDir = dir + sep + "apk";
        const apkList = backdroid.listDirectory(apkDir);
        apkList.forEach(pkg => {
          apkCount++;
          listModel.append({
            checked: false,
            pkg: pkg
          });
        });
      }
    }
  }

  function getCheckedPackages() {
    let device = Vars.currentDevice;
    let model = apkTable.model;
    let result = [];
    apkCount = 0;

    if (backupRb.checked) {
      let packages = Utils.getPackages(device);

      for (let p of packages) {
        for (let i = 0; i < model.count; i++) {
          let pkg = model.get(i);
          if (pkg.checked && pkg.pkg === p.name) {
            result.push(p);
            apkCount++;
          }
        }
      }
    } else {
      const sep = Vars.separator;
      const dir = backdroid.backupDir + sep + device.serial;

      const apkDir = dir + sep + "apk";
      const apkList = backdroid.listDirectory(apkDir);
      apkList.forEach(name => {
        for (let i = 0; i < model.count; i++) {
          let pkg = model.get(i);
          if (pkg.checked && pkg.pkg === name) {
            result.push(name);
            apkCount++;
          }
        }
      });
    }
    return result;
  }

  function backupPartitions(partitions) {
    let device = Vars.currentDevice;
    const parts = Utils.listPartitions(device);

    if (parts.length > 0 && partitions.length > 0) {
      promise = promise.then(_ => {
        console.info('backing up partitions...');
        backdroid.mkdir(dir + sep + "partitions");
        return Promise.resolve();
      });

      for (let [name, partition] of parts) {
        promise = promise.then(_ => {
          if (!partitions.includes(name)) {
            return Promise.resolve();
          }

          console.info(`backing up ${name} partition`);
          return Utils.runShell(device.serial, ['su', '-c', 'dd', `if=${partition}`, `of=/sdcard/${name}.img`]).then(_ => {
            let dest = dir + sep + "partitions" + sep + name + ".img";
            return Utils.pullFile(device, `/sdcard/${name}.img`, dest).then(_ => {
              console.info("finished pulling partition, removing it from sdcard");
              return Utils.runShell(device.serial, ['rm', `/sdcard/${name}.img`]);
            });
          });
        });
      }
      promise.then(_ => {
        console.info("backup partitions finished!");
        return Promise.resolve();
      });
    }
  }

  function startRestore() {
    const device = Vars.currentDevice;
    const sep = Vars.separator;
    const dir = backdroid.backupDir + sep + device.id;

    const apkDir = dir + sep + "apk";
    const apkList = getCheckedPackages();

    let promise = Promise.resolve();
    if (appChk.checked && apkList.length > 0) {
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = apkCount;
        return Promise.resolve();
      });

      console.info("restoring applications...");
      let installedApk = Utils.getPackages(device, true, true);

      for (let apk of apkList) {
        let path = apkDir + sep + apk;
        let name = backdroid.basename(path);

        promise = promise.then(_ => {
          let apkPath = apkDir + sep + apk;

          return Promise.resolve().then(_ => {
            progressMessage = `restoring apk ${name}`;
            if (!installedApk.includes(name)) {
              console.info("restoring app", name);
              return Utils.installApk(device.serial, apkPath);
            }
            console.info(`${name} already installed`);
            return Utils.forceStopApp(device.serial, name);
          }).then(_ => {
            if (dataChk.checked) {
              return Promise.resolve().then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = backdroid.basename(apkPath);
                if (device.isRooted) {
                  console.info(`restoring system data for package ${packageName}`);
                  let dataDir = dir + sep + "system-data" + sep + packageName;
                  let dest = "/data/data/" + packageName;

                  return Utils.pushFile(device, dataDir, dest, true).then(_ => {
                    let ownerGroup = Utils.getOwnerGroup(device, dest);
                    console.info("updating ownership for folder", dest, "to", ownerGroup);
                    return Utils.chown(device, dest, ownerGroup, true);
                  });
                }
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = backdroid.basename(apkPath);
                let dataDir = dir + sep + "data" + sep + packageName;
                if (backdroid.exists(dataDir)) {
                  console.info(`restoring data for package ${packageName}`);
                  let dest = "/sdcard/Android/data/" + packageName;
                  return Utils.pushFile(device, dataDir, dest);
                }

                return Promise.resolve();
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = backdroid.basename(apkPath);
                let obbDir = dir + sep + "obb" + sep + packageName;
                if (backdroid.exists(obbDir)) {
                  console.info(`restoring opaque binary blob files for package ${packageName}`);
                  let dest = "/sdcard/Android/obb/" + packageName;
                  return Utils.pushFile(device, obbDir, dest);
                }

                return Promise.resolve();
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = backdroid.basename(apkPath);
                let mediaDir = dir + sep + "media" + sep + packageName;
                if (backdroid.exists(mediaDir)) {
                  console.info(`restoring media files for package ${packageName}`);
                  let dest = "/sdcard/Android/media/" + packageName;
                  return Utils.pushFile(device, mediaDir, dest);
                }
                currentProgress++;
                return Promise.resolve();
              });
            }
            currentProgress++;
            return Promise.resolve();
          });
        });
      }

      promise = promise.then(_ => {
        console.info('restore applications finished');
        return Promise.resolve();
      });
    }

    if (contactsChk.checked && device.isRooted) {
      let dest = "/data/data/com.android.providers.contacts/databases";
      let contactsDir = dir + sep + "contacts" + sep + "databases";

      promise = promise.then(_ => {
        console.info("restoring contacts...");
        currentProgress = 0;
        maxProgress = 2;
        return Utils.forceStopApp(device.serial, "com.android.providers.contacts");
      }).then(_ => {
        currentProgress++;
        return Utils.pushFile(device, contactsDir, dest, true).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for folder", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        }).then(_ => {
          currentProgress++;
          console.info("restore contacts finished!");
          return Promise.resolve();
        });
      });
    } else if (contactsChk.checked && !device.isRooted) {
      console.info("cannot restore contacts without root permission");
    }

    if (wifiChk.checked && device.isRooted) {
      let wifiDir = dir + sep + "misc" + sep + "wifi";
      promise = promise.then(_ => {
        console.info("restoring wifi configuration and saved network...");
        currentProgress = 0;
        maxProgress = 2;
        return Promise.resolve();
      }).then(_ => {
        let wifiConfig = wifiDir + sep + "softap.conf";
        let dest = "/data/misc/wifi/softap.conf";

        console.info("restoring wifi configuration...");
        return Utils.pushFile(device, wifiConfig, dest, true).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for file", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        }).then(_ => {
          currentProgress++;
          if (device.version >= Vars.VERSION.LOLLIPOP && device.version < Vars.VERSION.PIE) {
            wifiConfig = wifiDir + sep + "wpa_supplicant.conf";
            dest = "/data/misc/wifi/wpa_supplicant.conf";
          } else if (device.version === Vars.VERSION.PIE) {
            wifiConfig = dir + sep + "misc" + sep + "wifi" + sep + "WifiConfigStore.xml";
            dest = "/data/misc/wifi/WifiConfigStore.xml";
          }
          console.info("restoring wifi saved network...");
          return Utils.pushFile(device, wifiConfig, dest, true).then(_ => {
            let ownerGroup = Utils.getOwnerGroup(device, dest);
            console.info("updating ownership for file", dest);
            return Utils.chown(device, dest, ownerGroup, true);
          }).then(_ => {
            currentProgress++;
            return Promise.resolve();
          });
        });
      });
    } else if (contactsChk.checked && !device.isRooted) {
      console.info("cannot restore contacts without root permission");
    }

    if (messagesChk.checked && device.isRooted) {
      promise = promise.then(_ => {
        console.info("restoring messages...");
        currentProgress = 0;
        maxProgress = 1;
        return Promise.resolve();
      }).then(_ => {
        let dest = "/data/data/com.android.providers.telephony/databases";
        let messagesDir = dir + sep + "messages" + sep + "databases";

        return Utils.pushFile(device, messagesDir, dest).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for folder", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        });
      }).then(_ => {
        console.info("restore messages finished!");
        currentProgress++;
        return Promise.resolve();
      });
    } else if (messagesChk.checked && !device.isRooted) {
      console.info("cannot restore messages without root permission");
    }

    promise = promise.then(_ => {
      progressMessage = `restore finished!`;
      restoreIsRunning = false;
      updateButtonState();
    });
  }

  function startBackup() {
    let device = Vars.currentDevice;

    let promise = Promise.resolve();
    let sep = Vars.separator;
    let dir = backdroid.backupDir + sep + device.id;

    let packages = getCheckedPackages();

    if (appChk.checked && packages.length > 0) {
      let apkDir = dir + sep + "apk";
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = apkCount;
        progressMessage = `backing up applications...`;
        console.info(progressMessage);
        backdroid.mkdir(apkDir);
        return Promise.resolve();
      });

      for (let p of packages) {
        promise = promise.then(_ => {
          let dest = `${apkDir + sep + p.name}.apk`;
          console.info("backing up apk ", p.name);
          progressMessage = `backing up apk ${p.name}`;
          return Utils.pullFile(device, p.path, dest).then(_ => {
            if (dataChk.checked) {
              return Promise.resolve().then(_ => {
                let dataDir = "/sdcard/Android/data/" + p.name;
                let dest = dir + sep + "data" + sep + p.name;
                backdroid.mkdir(dir + sep + "data");
                backdroid.rmdir(dest);
                if (Utils.fileExists(device, dataDir)) {
                  console.info(`backing up user data for package ${p.name}`);
                  return Utils.pullFile(device, dataDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                if (device.isRooted) {
                  let dataDir = "/data/data/" + p.name;
                  let dest = dir + sep + "system-data" + sep + p.name;
                  backdroid.mkdir(dir + sep + "system-data");
                  backdroid.rmdir(dest);
                  console.info(`backing up system data for package ${p.name}`);
                  return Utils.pullFile(device, dataDir, dest, true);
                }

                return Promsie.resolve();
              }).then(_ => {
                let mediaDir = "/sdcard/Android/media/" + p.name;
                let dest = dir + sep + "media" + sep + p.name;

                backdroid.mkdir(dir + sep + "media");
                backdroid.rmdir(dest);

                if (Utils.fileExists(device, mediaDir)) {
                  console.info(`backing up media files for package ${p.name}`);
                  return Utils.pullFile(device, mediaDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                let obbDir = "/sdcard/Android/obb/" + p.name;
                let dest = dir + sep + "obb" + sep + p.name;

                backdroid.mkdir(dir + sep + "obb");
                backdroid.rmdir(dest);

                if (Utils.fileExists(device, obbDir)) {
                  console.info(`backing up opaque binary blob files for package ${p.name}`);
                  return Utils.pullFile(device, obbDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                currentProgress++;
                return Promise.resolve();
              });
            }
            currentProgress++;
            return Promise.resolve();
          });
        });
      }

      promise = promise.then(_ => {
        progressMessage = `backup finished!`;
        console.info('backup applications finished');
        return Promise.resolve();
      });
    }

    if (contactsChk.checked && device.isRooted) {
      let contactsDir = "/data/data/com.android.providers.contacts/databases";
      let dest = `${dir + sep + "contacts"}`;
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = 1;
        progressMessage = "backing up contacts...";
        console.info(progressMessage);
        // backdroid.rmdir(dest);
        backdroid.mkdir(dir + sep + "contacts");

        return Promise.resolve();
      }).then(_ => {
        return Utils.pullFile(device, contactsDir, dest, true).then(_ => {
          console.info("backup contacts finished");
          currentProgress = 1;
          return Promise.resolve();
        });
      });
    } else if (contactsChk.checked && !device.isRooted) {
      console.info("cannot backup contacts without root permission");
    }

    if (wifiChk.checked && device.isRooted) {
      promise = promise.then(_ => {
        backdroid.mkdir(dir + sep + "misc");
        backdroid.mkdir(dir + sep + "misc" + sep + "wifi");
        currentProgress = 0;
        maxProgress = 2;
        console.info("backing up wifi configuration and saved network...");
        return Promise.resolve();
      }).then(_ => {
        let wifiConfig = '/data/misc/wifi/softap.conf';
        let dest = dir + sep + "misc" + sep + "wifi" + sep + "softap.conf";
        console.info("backing up wifi configuration");
        return Utils.pullFile(device, wifiConfig, dest, true);
      }).then(_ => {
        currentProgress++;
        let wifiStoreFile, dest;

        if (device.version >= Vars.VERSION.LOLLIPOP && device.version < Vars.VERSION.PIE) {
          wifiStoreFile = "/data/misc/wifi/wpa_supplicant.conf";
          dest = dir + sep + "misc" + sep + "wifi" + sep + "wpa_supplicant.conf";
        } else if (device.version === Vars.VERSION.PIE) {
          wifiStoreFile = "/data/misc/wifi/WifiConfigStore.xml";
          dest = dir + sep + "misc" + sep + "wifi" + sep + "WifiConfigStore.xml";
        }

        console.info("backing up saved wifi network");
        return Utils.pullFile(device, wifiStoreFile, dest, true);
      }).then(_ => {
        currentProgress++;
        console.info("backup wifi configuration and saved network finished!");
        return Promise.resolve();
      });
    } else if (wifiChk.checked && !device.isRooted) {
      console.info("cannot backup wifi configuration without root permission");
    }

    if (messagesChk.checked && device.isRooted) {
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = 1;
        backdroid.mkdir(dir + sep + "messages");
        progressMessage = "backing up messages...";
        console.info(progressMessage);
        return Promise.resolve();
      }).then(_ => {
        let messagesDir = "/data/data/com.android.providers.telephony/databases";
        let dest = dest = dir + sep + "messages";

        return Utils.pullFile(device, messagesDir, dest);
      }).then(_ => {
        currentProgress++;
        console.info("backup messages finished!");
        return Promise.resolve();
      });
    } else if (messagesChk.checked && !device.isRooted) {
      console.info("cannot backup messages without root permission");
    }

    promise.then(_ => {
      backupIsRunning = false;
      updateButtonState();
    });
  }
  GroupBox {
    id: mode
    contentHeight: 45
    anchors.right: parent.right
    anchors.rightMargin: 10
    title: ""
    anchors.top: parent.top
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    RadioButton {
      id: backupRb
      text: qsTr("Backup")
      anchors.left: parent.left
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      checked: true
      onCheckedChanged: {
        console.log("Backup mode selected");
        refreshPackage();

        if (backupRb.checked) {
          backupBtn.visible = true;
          restoreBtn.visible = false;
        } else {
          backupBtn.visible = false;
          restoreBtn.visible = true;
        }
      }
    }

    RadioButton {
      id: restoreRb
      text: qsTr("Restore")
      anchors.left: backupRb.right
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      onCheckedChanged:
      // if (fastbootRb.checked) {
      //   keepDataChk.visible = true
      //   lockCrcChk.visible = true
      // } else {
      //   keepDataChk.visible = false
      //   lockCrcChk.visible = false
      // }
      {}
    }
  }
  CheckBox {
    id: dataChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    text: qsTr("Data")
  }

  CheckBox {
    id: appChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: dataChk.right
    anchors.leftMargin: 10
    text: qsTr("Application")
    onCheckedChanged: {
      if (appChk.checked) {
        apkTable.visible = true;
      } else {
        apkTable.visible = false;
      }
    }
  }

  CheckBox {
    id: messagesChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: appChk.right
    anchors.leftMargin: 10
    text: qsTr("Messages")
  }

  CheckBox {
    id: contactsChk
    anchors.top: dataChk.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    text: qsTr("Contacts")
  }

  CheckBox {
    id: wifiChk
    anchors.top: appChk.bottom
    anchors.topMargin: 10
    anchors.left: contactsChk.right
    anchors.leftMargin: 10
    text: qsTr("Wifi")
  }

  TableView {
    id: apkTable
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: wifiChk.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: progress.top
    anchors.bottomMargin: 10
    visible: false
    clip: true
    delegate: Item {
      required property var model
      Rectangle {
        id: cell
        implicitWidth: apkTable.width
        implicitHeight: 50
        y: 50

        color: '#252c35'
        FlexboxLayout {
          id: flexLayout
          anchors.fill: parent
          wrap: FlexboxLayout.Wrap
          direction: FlexboxLayout.Row
          justifyContent: FlexboxLayout.JustifyStart
          alignItems: FlexboxLayout.AlignCenter
          Rectangle {
            implicitWidth: 50
            implicitHeight: 50
            color: "transparent"
            CheckBox {
              x: implicitWidth / 2 - implicitWidth / 2
              checked: model.checked // read from the model when created or recycled
              visible: true
              anchors.centerIn: parent
            }
          }

          Text {
            // anchors.leftMargin: 10
            height: 50
            text: pkg
            width: apkTable.width - 50
            visible: true
            color: "white"
            font.bold: true
          }
        }

        // MouseArea {
        //   anchors.fill: parent
        //   onClicked: parent.model.checked = !parent.checked
        // }
      }
    }
    HorizontalHeaderView {
      id: horizontalHeader
      syncView: apkTable
      model: ["", "Package Name"]
      delegate: Rectangle {
        implicitWidth: (index === 0) ? 50 : apkTable.width - 50
        implicitHeight: 50
        color: "#1f1f1f"
        CheckBox {
          id: allChk
          anchors.centerIn: parent
          // width: styleData.width
          checked: allChecked
          visible: (index === 0) // Show only in the 1st column
          text: ""
          onCheckedChanged: {
            for (let i = 0; i < listModel.count; i++) {
              listModel.setProperty(i, "checked", checked);
              // listModel.get(i).checked = checked;
            }
          }
        }
        Text {
          text: modelData
          color: "white"
          font.bold: true
          anchors.fill: parent
          // horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          visible: (index === 1)
        }
      }
    }
    model: ListModel {
      id: listModel
    }
  }

  ProgressBar {
    id: progress
    height: 20
    value: currentProgress
    to: maxProgress
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.bottom: restoreBtn.top
    anchors.bottomMargin: 5
    anchors.topMargin: 5
    background: Rectangle {
      radius: 4
      color: "#3C3F41"
    }
    Text {
      text: progressMessage
      anchors.centerIn: parent
      color: "#ffffff"
    }
  }

  Button {
    id: restoreBtn
    text: qsTr("Start Restore")
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    visible: false
    background: Rectangle {
      color: "#0078d4"
      opacity: enabled ? 1 : 0.3
      anchors.fill: parent
      radius: 10
    }
    onClicked: {
      startRestore();
    }
  }

  Button {
    id: backupBtn
    text: qsTr("Start Backup")
    enabled: false
    anchors.topMargin: 20
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    contentItem: Text {
      text: backupBtn.text
      opacity: backupBtn.enabled ? 1.0 : 0.3
      color: '#ffffff'
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
      elide: Text.ElideRight
    }
    background: Rectangle {
      color: "#0078d4"
      opacity: enabled ? 1 : 0.3
      anchors.fill: parent
      radius: 10
    }
    onClicked: {
      backupIsRunning = true;
      updateButtonState();
      let device = Vars.currentDevice;
      devicesDialog.open();

      if (!device.isRooted && !device.isRootUser && dataChk.checked && length(Vars.devices) > 1) {
        dialog.icon = StandardIcon.Warning;
        dialog.text = "Your device is not rooted, some data may not be able to be restored.";
        function onAccepted() {
          console.log("accepted");
          startBackup();
          dialog.onAccepted.disconnect(onAccepted);
        }
        dialog.onAccepted.connect(onAccepted);
        dialog.open();
      } else {
        startBackup();
      }

      // if(device && device.isRooted){
      //   startBackup(partitions);
      // } else if(device && !device.isRooted) {
      //   if(device.state === 'device') {
      //     Utils.reboot(device.serial, 'bootloader').then(_ => {

      //     });
      //   }
      // }
    }
  }

  Timer {
    id: timer
  }
  Component.onCompleted: {
    Vars.setBackdroid(backdroid);
    Vars.setTimer(timer);
    Vars.setAdb(backdroid.adb);
    Vars.setFastboot(backdroid.fastboot);

    backdroid.adb.startServer(c => {
      refreshDevices();

      let devices = Vars.devices;

      let queue = new Queue.DeviceQueue(devices);
      Vars.setQueue(queue);
      for (let serial in devices) {
        let device = devices[serial];
        queue[serial] = device;
      }

      if (!queue.isEmpty() && queue.hasAuthorizedDevice()) {
        let device = queue.firstAuthorizedDevice;

        Vars.setCurrentDevice(Utils.cloneObject(device));
        // queue.removeBySerial(device.serial);
        // device = Vars.currentDevice;
        // removeKeyFiles(device.serial);
      } else if (!queue.isEmpty() && !queue.hasAuthorizedDevice()) {
        let device = queue.first;
        Vars.setCurrentDevice(Utils.cloneObject(device));
      }
      Vars.events.emit('app-ready');
      Vars.setInitialized(true);
      try {
        backdroid.watcher.onDeviceAdded.connect(onDeviceAdded);
        backdroid.watcher.onDeviceRemoved.connect(onDeviceRemoved);
        backdroid.watcher.onDeviceChanged.connect(onDeviceChanged);
      } catch (e) {
        console.error(e);
      }
    });

    if (!Vars.initialized) {
      Vars.events.once('app-ready', () => {
        updateButtonState();
        refreshPackage();
      });
    } else {
      updateButtonState();
      refreshPackage();
    }

    Vars.events.on('device-added', () => {
      updateButtonState();
      refreshPackage();
    });
    Vars.events.on('device-removed', () => {
      updateButtonState();
    });
  }
}
