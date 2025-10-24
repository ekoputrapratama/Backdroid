
import { currentDevice, separator } from '/common/vars.mjs';
import { getPackages, fileExists } from '/common/utils.mjs';
import { pull } from '/common/AdbUtils.mjs';

// export function getPkgs() {
//   return getPackages(currentDevice, true, true);
// }

// export function startBackup(params, onProgressCb, packages = []) {
//   const device = currentDevice;
//   const sep = separator;
//   const dir = bnr.backupDir + sep + device.id;
//   let promise = Promise.resolve();

//   const [backupApk, backupData, backupContact, backupMessage, backupWifi] = params;
//   let currentProgress = 0;

//   if (backupApk && packages.length > 0) {
//     let apkDir = dir + sep + "apk";
//     promise = promise.then(_ => {
//       onProgressCb(currentProgress, packages.length, `backing up applications...`);
//       stella.mkdir(apkDir);
//       return Promise.resolve();
//     });

//     for (let p of packages) {
//       promise = promise.then(_ => {
//         let dest = `${apkDir + sep + p.name}.apk`;
//         console.info("backing up apk ", p.name);
//         onProgressCb(currentProgress, packages.length, `backing up apk ${p.name}`);
//         return pull(device, p.path, dest).then(_ => {
//           if (backupData) {
//             return Promise.resolve().then(_ => {
//               let dataDir = "/sdcard/Android/data/" + p.name;
//               let dest = dir + sep + "data" + sep + p.name;
//               stella.mkdir(dir + sep + "data");
//               stella.rmdir(dest);
//               if (fileExists(device, dataDir)) {
//                 console.info(`backing up user data for package ${p.name}`);
//                 return pull(device, dataDir, dest);
//               }
//               return Promise.resolve();
//             }).then(_ => {
//               if (device.isRooted) {
//                 let dataDir = "/data/data/" + p.name;
//                 let dest = dir + sep + "system-data" + sep + p.name;
//                 stella.mkdir(dir + sep + "system-data");
//                 stella.rmdir(dest);
//                 console.info(`backing up system data for package ${p.name}`);
//                 return pull(device, dataDir, dest, true);
//               }

//               return Promsie.resolve();
//             }).then(_ => {
//               let mediaDir = "/sdcard/Android/media/" + p.name;
//               let dest = dir + sep + "media" + sep + p.name;

//               stella.mkdir(dir + sep + "media");
//               stella.rmdir(dest);

//               if (fileExists(device, mediaDir)) {
//                 console.info(`backing up media files for package ${p.name}`);
//                 return pull(device, mediaDir, dest);
//               }
//               return Promise.resolve();
//             }).then(_ => {
//               let obbDir = "/sdcard/Android/obb/" + p.name;
//               let dest = dir + sep + "obb" + sep + p.name;

//               stella.mkdir(dir + sep + "obb");
//               stella.rmdir(dest);

//               if (fileExists(device, obbDir)) {
//                 console.info(`backing up opaque binary blob files for package ${p.name}`);
//                 return pull(device, obbDir, dest);
//               }
//               return Promise.resolve();
//             }).then(_ => {

//               currentProgress++;
//               onProgressCb(currentProgress, packages.length, `backing up apk ${p.name}`);
//               return Promise.resolve();
//             })
//           }
//           currentProgress++;
//           onProgressCb(currentProgress, packages.length, `backing up apk ${p.name}`);
//           return Promise.resolve();
//         });
//       })
//     }
//   }
// }


