import {
  backdroid, adb, fastboot, timer, listPartitionCmd,
  VERSION, separator
} from 'vars.mjs';

export function join(...args) {
  return args.join(separator);
}

export function exists(...args) {
  let exists = false;

  if (args.length > 1) {
    let device = args[0];
    let file = args[1];
    let result, cmd;
    console.debug(`Checking if file exists ${file}`)
    cmd = `[ -e ${file.path || file} ] && echo "true" || echo "false";`;

    if (device.isRooted && !device.isRootUser) {
      result = adb.shell(device.serial, [`su -c '${cmd}'`]);
    } else {
      result = adb.shell(device.serial, [cmd]);
    }

    exists = clearText(result.output) === "true";

    if (typeof file === "object") {
      cmd = `[ -r ${file.path} ] && echo "true" || echo "false";`;
      if (device.isRooted && !device.isRootUser) {
        result = adb.shell(device.serial, [`su -c '${cmd}'`]);
      } else {
        result = adb.shell(device.serial, [cmd]);
      }

      let denied = clearText(result.output) === "false";

      file.exists = exists;
      file.denied = denied;
    }
  } else {
    exists = backdroid.exists(args[0]);
  }

  return exists;
}
