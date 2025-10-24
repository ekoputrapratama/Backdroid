#ifndef ANDROID_DEVICE_H
#define ANDROID_DEVICE_H

#include <QtCore/QObject>

static const QString kVidSamsung = "04e8";
static const QString kPidGalaxyS = "6601";
static const QString kPidGalaxyS2 = "685d";
static const QString kPidDroidCharge = "68c3";
static const int kSupportedSamsungDeviceCount = 3;

class SamsungDeviceIdentifier {
public:
  const QString vendorId;
  const QString productId;

  SamsungDeviceIdentifier(QString vid, QString pid) : vendorId(vid), productId(pid) {
  }
};

const SamsungDeviceIdentifier supportedSamsungDevices[3]
    = { SamsungDeviceIdentifier(kVidSamsung, kPidGalaxyS),
        SamsungDeviceIdentifier(kVidSamsung, kPidGalaxyS2),
        SamsungDeviceIdentifier(kVidSamsung, kPidDroidCharge) };

class AndroidDevice : public QObject {
  Q_OBJECT
public:
  enum { kPidGalaxyS = 0x6601, kPidGalaxyS2 = 0x685D, kPidDroidCharge = 0x68C3 };

  AndroidDevice(QObject *parent = nullptr);
  Q_PROPERTY(QString id READ id WRITE setId);
  Q_PROPERTY(QString serial READ serial WRITE setSerial);
  Q_PROPERTY(QString state READ state WRITE setState);
  Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer);
  Q_PROPERTY(QString model READ model WRITE setModel);
  Q_PROPERTY(QString product READ product WRITE setProduct);
  Q_PROPERTY(QString vendorId READ vendorId WRITE setVendorId);
  Q_PROPERTY(QString productId READ productId WRITE setProductId);
  Q_PROPERTY(bool odinMode READ odinMode WRITE setOdinMode);
  Q_PROPERTY(bool edlMode READ edlMode WRITE setEdlMode);
  Q_PROPERTY(bool isUsbDevice READ isUsbDevice WRITE setIsUsbDevice);

  void setId(QString id);
  QString id() const;
  void setSerial(QString serial);
  QString serial() const;
  void setState(QString state);
  QString state() const;
  void setManufacturer(QString m);
  QString manufacturer() const;
  void setModel(QString m);
  QString model() const;
  void setProduct(QString p);
  QString product() const;
  void setVendorId(QString p);
  QString vendorId() const;
  void setProductId(QString p);
  QString productId() const;

  void setOdinMode(bool v);
  bool odinMode() const;
  void setEdlMode(bool v);
  bool edlMode() const;
  void setIsUsbDevice(bool v);
  bool isUsbDevice() const;
  QString path;

private:
  QString m_id;
  QString m_serial;
  QString m_state;
  QString m_manufacturer;
  QString m_model;
  QString m_product;
  QString m_vendorId;
  QString m_productId;
  bool m_odinMode = false;
  bool m_edlMode = false;
  bool m_isUsbDevice = false;
};

class AndroidDeviceList : public QObject {
  Q_OBJECT
private:
  /* data */
  QList<AndroidDevice *> devices = {};

public:
  AndroidDeviceList();
  ~AndroidDeviceList();

public slots:
  void push(AndroidDevice *device);
  void append(AndroidDevice *device);
  void add(AndroidDevice *device);
  void addAll(QList<AndroidDevice *> devices = {});
  void clear();
  int length();
  bool hasDeviceWithSerial(const QString &serial);
  AndroidDevice *get(const int &index);
  AndroidDevice *getBySerial(const QString &serial);
  void removeBySerial(const QString &serial);
  bool isEmpty();
};

#endif // ANDROID_DEVICE_H
