
export class DeviceQueue {
  constructor(devices) {
    this.devices = [];
    for (let serial in devices) {
      this.devices.push(devices[serial]);
    }
  }

  findBySerial(serial) {
    return this.devices.filter(function (item) {
      return item.serial === serial;
    });
  }

  findByModel(model) {
    return this.devices.filter(function (item) {
      return item.model === model;
    });
  }

  findByStatus(status) {
    return this.devices.filter(function (item) {
      return item.status === status;
    });
  }

  removeBySerial(serial) {
    this.devices = this.devices.filter(function (item) {
      return item.serial !== serial;
    });
  }
  removeByModel(model) {
    this.devices = this.devices.filter(function (item) {
      return item.model !== model;
    });
  }

  get first() {
    return this.devices[0];
  }
  
  get firstAuthorizedDevice() {
    let res = null;

    for (let device of this.devices) {
      if (device.state !== 'unauthorized') {
        res = device;
        break;
      }
    }
    return res;
  }

  get authorizedDevices() {
    return this.devices.filter(function (item) {
      return item.state !== 'unauthorized';
    });
  }

  setAllNextState(state) {
    this.devices = this.devices.map(d => {
      d.nextState = state;
    });
  }

  hasAuthorizedDevice() {
    let satisfied = false;
    for (let d of this.devices) {
      if (d.state !== 'unauthorized') {
        satisfied = true;
        break;
      }
    }
    return satisfied;
  }

  isEmpty() {
    return this.devices.length === 0;
  }
}
