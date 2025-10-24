import { EventEmitter } from 'eventemitter.mjs';

export let backdroid = null;
export let adb = null;
export let fastboot = null;
export let isWindows = Qt.platform.os === 'windows';
export let isLinux = Qt.platform.os === 'linux';
export let devices = {};
export let logger = null;
export let queue = null;
export let currentDevice = null;
export let timer = null;
export let initialized = false;
export let events = new EventEmitter();
export const separator = isWindows ? "\\" : "/";
export const VERSION = {
  KITKAT: parseFloat("4.4.4"),
  LOLLIPOP: parseFloat("5.1.1"),
  MARSHMELLOW: 6,
  NOUGAT: 7,
  OREO: 8,
  PIE: 9
};

export const RecoveryType = {
  TWRP: 0,
  CM: 1,
  STOCK: 2,
  PHILZ_TOUCH: 3,
  GENERIC: 4
}

export let adbFiles = [
  {
    path: "/data/property/persist.sys.usb.config",
    name: 'persist.sys.usb.config'
  },
  {
    path: "/data/data/com.android.providers.settings/databases/settings.db",
    name: "settings.db"
  },
  {
    path: "/data/data/com.android.settings/shared_prefs/com.android.settings_preferences.xml",
    name: "com.android.settings_preferences.xml"
  },
  {
    // Android Pie LinageOS
    path: "/data/property/persistent_properties",
    name: "persistent_properties"
  }
];

export const xiaomiFiles = [
  "misc.txt",
  "flash_all.sh",
  "flash_all_except_data_storage.sh",
  "flash_all_lock.sh",
  "flash_all_lock_crc.sh"
];

export const listPartitionCmd = [
  ["ls", "-la", "/dev/block/by-name/"],
  ["ls", "-la", "/dev/block/bootdevice/by-name/"],
  ["ls", "-la", "/dev/block/platform/*/by-name/"]
]

export function setCurrentDevice(device) {
  currentDevice = device;
}

export function setBackdroid(u) {
  backdroid = u;
}

export function setAdb(a) {
  adb = a;
}

export function setFastboot(f) {
  fastboot = f;
}

export function setQueue(q) {
  queue = q;
}

export function setTimer(t) {
  timer = t;
}

export function setLogger(l) {
  logger = l;
}

export function setInitialized(i) {
  initialized = i;
}
