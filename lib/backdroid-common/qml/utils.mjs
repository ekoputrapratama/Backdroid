import {
  backdroid, adb, fastboot, timer, listPartitionCmd,
  VERSION, separator
} from 'vars.mjs';

export function hasBusyBox(device) {
  const paths = ["/sbin", "/system/bin", "/system/xbin"];
  let exists = false;
  for (let path of paths) {
    const file = path + '/busybox'
    if (fileExists(device, file)) {
      exists = true;
      break;
    }
  }
  return exists;
}

export function getFileExtension(path) {
  return path.split(".").pop();
}

export function getRecoveryName(device) {
  const paths = ["/cache/recovery/log", "/tmp/recovery.log"];

  for (let log in paths) {
    let cmd = ["grep", "-m", 2, "-E",
      " Recovery v|Starting TWRP|Welcome to|PhilZ|Starting recovery \(",
      log
    ];

    if (hasBusyBox(device)) {
      cmd.unshift("busybox");
    }
    let result = adb.shell(device.serial, cmd);
    if (clearText(result.output).contains("Welcome")) {

    } else if (result.output.contains("Recovery")) {
    } else if (result.output.contains("PhilZ")) {
      // let cmdStr = `
      //   recovery=$(grep -m 2 -E "PhilZ|ClockworkMod" /tmp/recovery.log); 
      //   printf "$recovery" | tr "\n" " " | sed -E "s@(^PhilZ.*)(ClockworkMod.*)@\1 \(\2\)\n@g"'`;
      // result = adb.shell(device.serial, cmdStr);
    } else if (result.output.contains("Starting recovery (")) {

    } else if (result.output.startsWith("Starting")) {
    }
  }
}

export function cloneObject(obj) {
  let result = {};
  for (let key in obj) {
    result[key] = obj[key];
  }
  return result;
}

export function clearText(text) {
  if (!text) return text;
  return text.replace(/\r|\n/g, '');
}

export function splitlines(txt) {
  return txt.split(/\n|\r|\r\n/);
}


export function isPartitionMounted(device, partition) {
  let result = adb.shell(device.serial, ["cat", "/proc/mounts"]);

  let partitions = splitlines(result.output)
    .filter(l => l && l.length > 0)
    .map(line => {
      let sp = line.split(" ").filter(l => l && l.length > 0);

      return {
        devPath: sp[0],
        mountPoint: sp[1]
      }
    }).filter(part => part.mountPoint.includes(partition));

  return partitions.length > 0;
}

export function mountPartition(device, partition) {
  let partitions = listPartitions(device);
  let parts = partitions.filter(line => partition.includes(line[0]));
  // console.log("part", partitions);

  for (let p of parts) {
    if (!isPartitionMounted(device, p[0])) {
      console.log("mounting partition ", p[0], p[1])
      let result = adb.shell(device.serial, ["mount", "-o", "rw", p[1], `/${p[0]}`]);
      console.log("mounted partition ", p[0], p[1], result.code, result.output, result.error)
    } else {
      console.log("mounting partition ", p[0], p[1])
      let result = adb.shell(device.serial, ["mount", "-o", "remount,rw", `/${p[0]}`]);
      console.log("mounted partition ", p[0], p[1], result.output, result.error)
    }
  }
}
/**
 * Check if a folder or file have executable permission
 *
 * @export
 * @param {String} serial
 * @param {String} path
 * @returns {Boolean}
 */
export function isExecutable(serial, path) {
  console.debug("Checking if path is executable", path)
  let args = [`[ -x '${path}' ] && echo 'true' || echo 'false';`];
  let result = adb.shell(serial, args);
  return clearText(result.output) === "true";
}
/**
 * Check if a folder or file have read permission
 *
 * @export
 * @param {String} serial
 * @param {String} path
 * @returns {Boolean}
 */
export function isReadeable(serial, path) {
  console.debug("Checking if path is readable", path)
  let args = [`[ -r '${path}' ] && echo 'true' || echo 'false';`];
  let result = adb.shell(serial, args);
  return clearText(result.output) === "true";
}
/**
 * Check if a folder or file have write permission
 *
 * @export
 * @param {String} serial
 * @param {String} path
 * @returns {Boolean}
 */
export function isWritable(serial, path) {
  console.debug("Checking if path is writable", path)
  let args = [`[ -w '${path}' ] && echo 'true' || echo 'false';`];
  let result = adb.shell(serial, args);
  return clearText(result.output) === "true";
}

/**
 * Check if file on remote device is a directory
 *
 * @export
 * @param {String} serial device serial
 * @param {String} path path to a file or directory on remote device
 * @returns {Boolean}
 */
export function isDirectory(serial, path) {
  console.debug("Checking if path is a directory", path);
  let args = [`[ -d '${path}' ] && echo 'true' || echo 'false';`];
  let result = adb.shell(serial, args);

  return Boolean(result.output);
}

/**
 * Check if a file is exists on remote device
 *
 * @export
 * @param {Object} device device descriptor
 * @param {Object|String} file file descriptor or path
 * @returns {Boolean}
 */
export function fileExists(device, file) {
  let result, exists, cmd, denied;
  console.debug(`Checking if file exists ${file}`)
  cmd = `[ -e ${file.path || file} ] && echo "true" || echo "false";`;

  if (device.isRooted && !device.isRootUser) {
    result = adb.shell(device.serial, [`su -c '${cmd}'`]);
  } else {
    result = adb.shell(device.serial, [cmd]);
  }

  exists = clearText(result.output) === "true";

  cmd = `[ -r ${file.path || file} ] && echo "true" || echo "false";`;
  if (device.isRooted && !device.isRootUser) {
    result = adb.shell(device.serial, [`su -c '${cmd}'`]);
  } else {
    result = adb.shell(device.serial, [cmd]);
  }

  denied = clearText(result.output) === "false";

  file.exists = exists;
  file.denied = denied;

  return exists;
}

/**
 *
 *
 * @export
 * @param {Object} device
 * @param {Object|String} file remote file descriptor or path
 * @param {String} name local destination directory
 * @param {Boolean} [forceAsRoot=false]
 * @returns {Promise<any>}
 */
export function pullFile(device, file, name, forceAsRoot = false) {
  return new Promise((resolve, reject) => {
    if (typeof file === "object") {
      if (file.exists && !file.denied && !forceAsRoot) {
        adb.pull(device.serial, file.path, name, (c, o, e) => {
          resolve([c, o]);
        });
      } else if (file.denied && device.isRooted) {
        let filename = backdroid.filename(file.path);
        let isDir = isDirectory(device.serial, file.path);
        runShell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${file} /sdcard"`]).then(_ => {
          adb.pull(device.serial, `/sdcard/${filename}`, name, (c, o, e) => {
            resolve([c, o]);
            adb.shell(device.serial, ["rm", (isDir) ? "-R" : "", `/sdcard/${filename}`]);
          });
        });
      }
    } else {
      if (isReadeable(device.serial, file) && !forceAsRoot) {
        adb.pull(device.serial, file, name, (c, o, e) => {
          resolve([c, o]);
        });
      } else if (device.isRooted) {
        let filename = backdroid.filename(file);
        let isDir = isDirectory(device.serial, file);
        runShell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${file} /sdcard"`]).then(_ => {
          adb.pull(device.serial, `/sdcard/${filename}`, name, (c, o, e) => {
            resolve([c, o]);
            adb.shell(device.serial, ["rm", (isDir) ? "-R" : "", `/sdcard/${filename}`]);
          });
        })
      }
    }
  });
}
/**
 *
 *
 * @export
 * @param {Object} device device object
 * @param {String} local local file path
 * @param {Object|String} remote remote file descriptor or path
 * @param {boolean} [forceAsRoot=false]
 * @returns {Promise<any>}
 */
export function pushFile(device, local, remote, forceAsRoot = false) {
  return new Promise((resolve, reject) => {
    if (typeof remote === "object") {
      if (remote.exists && !remote.denied && !forceAsRoot) {
        let isDir = backdroid.isDirectory(local);
        console.debug(`Pushing ${isDir ? "directory" : "file"} ${local} to ${remote.path}`)
        adb.push(serial, local, remote.path, (c, o, e) => {
          resolve([c, o]);
        });
      } else if (remote.denied && device.isRooted) {
        let filename = backdroid.filename(local);
        let isDir = backdroid.isDirectory(local);
        let exists = fileExists(device, remote.path);
        console.debug(`Pushing ${isDir ? "directory" : "file"} ${local} to /sdcard/${filename}`)
        adb.push(device.serial, local, `/sdcard/${filename}`, (c, o, e) => {
          let from = `/sdcard/${filename}${isDir && exists ? "/*" : ""}`;
          let to = `${remote.path}${isDir && exists ? "/" : ""}`;
          console.debug(`Copying ${isDir ? "directory" : "file"} /sdcard/${filename} to ${to}`)
          runShell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${from} ${to}"`]).then(_ => {
            console.debug(`Removing ${isDir ? "directory" : "file"} /sdcard/${filename}`)
            adb.shell(device.serial, ["rm", (isDir) ? "-R" : "", `/sdcard/${filename}`]);
            resolve([c, o]);
          })
        });
      }
    } else if (typeof remote === "string") {
      if (isWritable(device.serial, remote) && !forceAsRoot) {
        let isDir = backdroid.isDirectory(local);
        console.debug(`Pushing ${isDir ? "directory" : "file"} ${local} to ${remote}`)
        adb.push(device.serial, local, remote, (c, o, e) => {
          resolve([c, o]);
        });
      } else if (device.isRooted) {
        let filename = backdroid.filename(local);
        let isDir = backdroid.isDirectory(local);
        let exists = fileExists(device, remote);
        console.debug(`Pushing ${isDir ? "directory" : "file"} ${local} to /sdcard/${filename}`)
        adb.push(device.serial, local, `/sdcard/${filename}`, (c, o, e) => {
          let from = `/sdcard/${filename}${isDir && exists ? "/*" : ""}`;
          let to = `${remote}`;
          console.debug(`Copying ${isDir ? "directory" : "file"} /sdcard/${filename} to ${to}`)
          adb.shell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${from} ${to}"`], () => {
            console.debug(`Removing ${isDir ? "directory" : "file"} /sdcard/${filename}`)
            adb.shell(device.serial, ["rm", (isDir) ? "-R" : "", `/sdcard/${filename}`], () => {
              resolve([c, o, e])
            });
          });
        });
      }
    }

  });
}
/**
 *
 *
 * @export
 * @param {String} serial device serial
 * @param {String} file local file path
 * @returns {Promise<any>}
 */
export function sideload(serial, file) {
  return new Promise((resolve, reject) => {
    adb.sideload(serial, file, (c, o, e) => {
      resolve([c, o]);
    });
  });
}
/**
 * Boot a recovery image
 *
 * @export
 * @param {String} serial
 * @param {String} file
 * @returns {Promise<any>}
 */
export function bootRecovery(serial, file) {
  return new Promise((resolve, reject) => {
    fastboot.boot(serial, file, (c, o, e) => {
      resolve([c, o]);
    });
  });
}
/**
 *
 *
 * @export
 * @param {String} serial
 * @param {"recovery"|"bootloader"|"sideload"|"download"|"edl"} type
 * @returns {Promise<any>}
 */
export function reboot(device, toState) {
  return new Promise((resolve, reject) => {
    if (device.state === "fastboot") {
      adb.waitFor(device.serial, toState, resolve);
      fastboot.reboot(device.serial);
    } else if (toState === "bootloader" && device.state !== "fastboot") {

    } else {
      adb.reboot(device.serial, toState, resolve);
    }
  });
}
/**
 *
 *
 * @export
 * @param {String} serial
 * @param {Array<String>} args
 * @returns {Promise<any>}
 */
export function runShell(serial, args) {
  return new Promise((resolve, reject) => {
    adb.shell(serial, args, (c, o, e) => {
      resolve([c, o, e])
    })
  });
}

export function listPartitions(device) {
  let lines = [];
  const partitions = [
    "boot",
    "recovery",
    "data",
    "cache",
    "userdata",
    "system",
    "datafs"
  ];
  let useParted = false;
  for (let arg of listPartitionCmd) {
    let result = adb.shell(device.serial, arg);
    let output = clearText(result.output) || clearText(result.error);
    useParted = true;
    if (!output.endsWith("No such file or directory")) {
      lines = splitlines(result.output);
      useParted = false;
      break;
    }
  }

  if (useParted && device.isRooted) {
    let s = separator;
    let partedDir = backdroid.appDir + s + "files" + s + "parted"
    if (!fileExists(device, "/sbin/parted")) {
      backdroid.pushFile(device.serial, partedDir, "/sbin/parted")
    }

    let result = adb.shell(device.serial, ["parted", "/dev/block/mmcblk0", "print"]);
    lines = splitlines(result.output).filter(l => l.length > 0);
    lines.splice(0, 5);

    lines = lines.map(line => {

      let l = line.split(" ").filter(l => l.length > 0);
      let name = l.length > 5 ? l[5].toLowerCase() : l[4].toLowerCase();
      if (name === 'userdata' || name === 'datafs') name = 'data';

      let p = [name, `/dev/block/mmcblk0p${l[0]}`];

      return p;
    }).filter(line => partitions.includes(line[0]));
  } else {

    lines = lines.filter(l => l.length > 0 && l.includes("->")).map(line => {

      let l = line.split("->");

      let name = l[0].split(" ").filter(l => l.length > 0);
      name = name[name.length - 1];
      l[0] = name.toLowerCase();
      l[1] = l[1].replace(" ", "");
      return l;
    }).filter(line => partitions.includes(line[0]));
  }

  return lines;
}

/**
 * Get all installed third party packages from remote device
 *
 * @export
 * @param {Object} device
 * @param {boolean} [keepAsString=false]
 * @param {boolean} [onlyPackageName=false]
 * @returns {Array<any>}
 */
export function getPackages(device, keepAsString = false, onlyPackageName = false) {
  let serial = device.serial;
  let result = adb.shell(serial, ['pm', 'list', 'packages', '-3', (onlyPackageName) ? '' : '-f']);

  let packages = splitlines(result.output)
    .filter(l => l.length > 0 && l.includes("package"));

  if (!keepAsString) {
    packages = packages.map(p => {
      let pkg = {};
      if (device.version >= VERSION.KITKAT && device.version < VERSION.PIE) {
        let sp = p.split(":")[1];
        let pp = sp.split("=");
        pkg = {
          path: pp[0],
          name: pp[1]
        }
      } else if (device.version === VERSION.PIE) {
        let sp = p.split(":")[1];
        let path = sp.slice(0, sp.lastIndexOf("="));
        let name = sp.slice(sp.lastIndexOf("=") + 1, sp.length);
        pkg = {
          name,
          path
        }
      }

      return pkg;
    });
  } else {
    packages = packages.map(p => {
      return p.split(":")[1];
    })
  }
  return packages;
}

/**
 *
 *
 * @export
 * @param {Number} time
 * @param {Function} func
 */
export function delay(time, func) {
  timer.interval = time;
  timer.repeat = false;
  if (func && typeof func === 'function') {
    timer.triggered.connect(func);
    timer.triggered.connect(function () {
      timer.triggered.disconnect(func);
      timer.triggered.disconnect(this);
    });
  }
  timer.start();
}
/**
 *
 *
 * @export
 * @param {Array|Object} obj
 * @returns {Number}
 */
export function length(obj) {
  if (typeof obj == "object") {
    return Object.keys(obj).length;
  } else if (Array.isArray(obj)) {
    return obj.length;
  }
}

/**
 * Get Owner and group name from file or folder in remote device
 *
 * @export
 * @param {Object} device
 * @param {String} path
 * @returns
 */
export function getOwnerGroup(device, path) {
  let result;
  console.debug(`Getting owner and group for ${path}`)
  if (isExecutable(device.serial, path)) {
    result = adb.shell(device.serial, ["ls", "-ld", path]);
  } else if (device.isRooted) {
    result = adb.shell(device.serial, ["su", "-c", `"ls -ld ${path}"`]);
  }

  let output = result.output.split(" ").filter(l => l && l.length > 0);

  let ownerGroup;
  if (device.version >= VERSION.KITKAT && device.version < VERSION.PIE) {
    let owner = output[1];
    let group = output[2];
    ownerGroup = `${owner}:${group}`;
  } else {
    let owner = output[2];
    let group = output[3];
    ownerGroup = `${owner}:${group}`;
  }

  return ownerGroup;
}
/**
 *
 *
 * @export
 * @param {String} serial device serial
 * @param {String} apkPath path to apk file
 * @returns {Promise<any>}
 */
export function installApk(serial, apkPath) {
  return new Promise((resolve, reject) => {
    let filename = backdroid.filename(apkPath);
    console.debug(`Installing apk ${filename}`)
    adb.install(serial, apkPath, () => {
      resolve();
    });
  });
}
/**
 * Force stop application by package name
 *
 * @export
 * @param {String} serial
 * @param {String} packageName
 * @returns {Promise<any>}
 */
export function forceStopApp(serial, packageName) {
  return new Promise(resolve => {
    console.debug(`Force stopping app ${packageName}`);
    adb.shell(serial, ["am", "force-stop", packageName], () => {
      resolve();
    })
  });
}
/**
 * Change owner and group for file or folder in remote device
 *
 * @export
 * @param {Object} device
 * @param {String} path
 * @param {String} ownerGroup
 * @param {boolean} [recursive=false]
 * @returns {Promise<any>}
 */
export function chown(device, path, ownerGroup, recursive = false) {
  return new Promise((resolve) => {
    console.debug(`change owner and group for ${path} to ${ownerGroup}`);
    if (isWritable(device.serial, path)) {
      let cmd = ["chown", recursive ? "-hR" : "-h", ownerGroup, path];
      if (hasBusyBox(device)) {
        cmd.unshift("busybox");
      }
      adb.shell(device.serial, cmd, () => {
        resolve();
      });
    } else if (device.isRooted) {
      let cmd = `chown ${recursive ? "-hR" : "-h"} ${ownerGroup} ${path}`
      if (hasBusyBox(device)) {
        cmd = "busybox " + cmd;
      }
      adb.shell(device.serial, ["su", "-c", `"${cmd}"`], () => {
        resolve();
      });
    }
  });
}
