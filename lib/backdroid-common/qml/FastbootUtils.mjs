import {
  backdroid, adb, fastboot, timer, listPartitionCmd,
  VERSION, separator
} from 'vars.mjs';

/**
 * Boot an image
 *
 * @export
 * @param {String} serial
 * @param {String} partition
 * @param {String} file
 * @returns {Promise<any>}
 */
export function boot(serial, partition, file) {
  return new Promise((resolve, reject) => {
    fastboot.boot(serial, partition, file, (c, o, e) => {
      if (c == 0) {
        resolve([c, o]);
      } else {
        reject(e);
      }
    });
  });
}
