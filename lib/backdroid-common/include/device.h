#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

class Device : public QObject {
  Q_OBJECT
private:
  QString m_serial;
  QString m_state;
  QString m_manufacturer;
  QString m_model;
  QString m_product;
  QString m_vendorId;
  QString m_productId;

public:
  Device(QObject *parent = nullptr);
  ~Device();

  Q_PROPERTY(QString serial READ serial WRITE setSerial);
  Q_PROPERTY(QString state READ state WRITE setState);
  Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer);
  Q_PROPERTY(QString model READ model WRITE setModel);
  Q_PROPERTY(QString product READ product WRITE setProduct);
  Q_PROPERTY(QString vendorId READ vendorId WRITE setVendorId);
  Q_PROPERTY(QString productId READ productId WRITE setProductId);

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
};

#endif
