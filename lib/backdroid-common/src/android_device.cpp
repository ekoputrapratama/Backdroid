
#include "android_device.h"
#include <QtCore/QtGlobal>

/**
 * AndroidDeviceList
 */
AndroidDeviceList::AndroidDeviceList() {
}
AndroidDeviceList::~AndroidDeviceList() {
  // delete devices;
}

void AndroidDeviceList::push(AndroidDevice *device) {
  devices.append(device);
}

void AndroidDeviceList::append(AndroidDevice *device) {
  devices.append(device);
}

void AndroidDeviceList::add(AndroidDevice *device) {
  devices.append(device);
}

void AndroidDeviceList::addAll(QList<AndroidDevice *> devices) {
  foreach (AndroidDevice *d, devices) { push(d); }
}

void AndroidDeviceList::clear() {
  devices.clear();
}

int AndroidDeviceList::length() {
  return this->devices.length();
}

bool AndroidDeviceList::hasDeviceWithSerial(const QString &serial) {
  AndroidDevice *device = nullptr;

  foreach (AndroidDevice *d, this->devices) {
    if (d->serial() == serial) {
      device = d;
      break;
    }
  }
  return device != nullptr;
}

AndroidDevice *AndroidDeviceList::get(const int &index) {
  return devices.at(index);
}

AndroidDevice *AndroidDeviceList::getBySerial(const QString &serial) {
  AndroidDevice *device = nullptr;

  foreach (AndroidDevice *d, this->devices) {
    if (d->serial() == serial) {
      device = d;
      break;
    }
  }
  return device;
}

void AndroidDeviceList::removeBySerial(const QString &serial) {
  QList<AndroidDevice *> devices = {};

  foreach (AndroidDevice *d, this->devices) {
    if (d->serial() != serial) {
      devices.append(d);
    }
  }
  this->devices = devices;
}

bool AndroidDeviceList::isEmpty() {
  return devices.length() == 0;
}

AndroidDevice::AndroidDevice(QObject *parent) : QObject(parent) {
}

void AndroidDevice::setId(QString id) {
  m_id = id;
}

QString AndroidDevice::id() const {
  return m_id;
}

void AndroidDevice::setSerial(QString serial) {
  m_serial = serial;
}

QString AndroidDevice::serial() const {
  return m_serial;
}

void AndroidDevice::setState(QString state) {
  m_state = state;
}

QString AndroidDevice::state() const {
  return m_state;
}

void AndroidDevice::setManufacturer(QString m) {
  m_manufacturer = m;
}

QString AndroidDevice::manufacturer() const {
  return m_manufacturer;
}

void AndroidDevice::setModel(QString m) {
  m_model = m;
}

QString AndroidDevice::model() const {
  return m_model;
}

void AndroidDevice::setProduct(QString p) {
  m_product = p;
}

QString AndroidDevice::product() const {
  return m_product;
}

void AndroidDevice::setProductId(QString p) {
  m_productId = p;
}

QString AndroidDevice::productId() const {
  return m_productId;
}

void AndroidDevice::setVendorId(QString p) {
  m_vendorId = p;
}

QString AndroidDevice::vendorId() const {
  return m_vendorId;
}

void AndroidDevice::setEdlMode(bool v) {
  m_edlMode = v;
}

bool AndroidDevice::edlMode() const {
  return m_edlMode;
}

void AndroidDevice::setOdinMode(bool v) {
  m_edlMode = v;
}

bool AndroidDevice::odinMode() const {
  return m_odinMode;
}
void AndroidDevice::setIsUsbDevice(bool v) {
  m_isUsbDevice = v;
}
bool AndroidDevice::isUsbDevice() const {
  return m_isUsbDevice;
}
