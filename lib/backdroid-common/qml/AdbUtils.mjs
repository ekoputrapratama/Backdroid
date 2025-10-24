import {
  backdroid, adb, fastboot, timer, listPartitionCmd,
  VERSION, separator
} from 'vars.mjs';
import { isDirectory, isWritable, isReadeable } from 'utils.mjs';

/**
 *
 *
 * @export
 * @param {String} serial
 * @param {"recovery"|"bootloader"|"sideload"|"download"|"edl"} toState
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
 *
 *
 * @export
 * @param {Object} device
 * @param {Object|String} file remote file descriptor or path
 * @param {String} name local destination directory
 * @param {Boolean} [forceAsRoot=false]
 * @returns {Promise<any>}
 */
export function pull(device, file, name, forceAsRoot = false) {
  return new Promise((resolve, reject) => {
    if (typeof file === "object") {
      if (file.exists && !file.denied && !forceAsRoot) {
        adb.pull(device.serial, file.path, name, (c, o, e) => {
          resolve([c, o]);
        });
      } else if (file.denied && device.isRooted) {
        let filename = backdroid.filename(file.path);
        let isDir = isDirectory(device.serial, file.path);
        adb.shell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${file} /sdcard"`], _ => {
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
        adb.shell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${file} /sdcard"`], () => {
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
export function push(device, local, remote, forceAsRoot = false) {
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
          adb.shell(device.serial, ["su", "-c", `"cp ${isDir ? "-R" : ""} ${from} ${to}"`], () => {
            console.debug(`Removing ${isDir ? "directory" : "file"} /sdcard/${filename}`)
            adb.shell(device.serial, ["rm", (isDir) ? "-R" : "", `/sdcard/${filename}`]);
            resolve([c, o]);
          });
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
 * @param {String} apkPath path to apk file
 * @returns {Promise<any>}
 */
export function install(serial, apkPath) {
  return new Promise((resolve, reject) => {
    let filename = backdroid.filename(apkPath);
    console.debug(`Installing apk ${filename}`)
    adb.install(serial, apkPath, () => {
      resolve();
    });
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
export function shell(serial, args) {
  return new Promise((resolve, reject) => {
    adb.shell(serial, args, (c, o, e) => {
      resolve([c, o, e])
    })
  });
}
