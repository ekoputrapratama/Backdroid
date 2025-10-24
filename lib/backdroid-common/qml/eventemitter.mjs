export class EventEmitter {
  constructor() {
    this.events = {};
  }

  emit(tag, ...args) {
    if (!this.events[tag]) {
      this.events[tag] = [];
    }

    let removed = [];
    let index = 0;
    for (let i = 0; i < this.events[tag].length; i++) {
      let obj = this.events[tag][i];
      let fn = obj.callback;

      try {
        fn.call(fn, ...args);
      } catch (e) {
        // this error can happen when the view that give the callback is already destroyed
        // for example a view that use Loader, when the Loader source changed the previous
        // view will be destroyed and all reference to it will not be valid.
        console.warn("EventEmitter :", "unable to call callback");
        removed.push(i);
      }

      if (obj.once && !removed.includes(i)) {
        removed.push(i);
      }

      index++;
    }
    for (let index of removed) {
      this.events[tag].splice(index, 1);
    }

  }

  on(tag, fn) {
    if (!this.events[tag]) {
      this.events[tag] = [];
    }
    let obj = {
      callback: fn,
      once: false
    }
    this.events[tag].push(obj);
  }

  once(tag, fn) {
    if (!this.events[tag]) {
      this.events[tag] = [];
    }
    console.log("adding callback for event", tag)
    let obj = {
      callback: fn,
      once: true
    }
    this.events[tag].push(obj);
  }
}
