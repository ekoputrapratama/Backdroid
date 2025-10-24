#ifndef STELLA_SETTINGS_H
#define STELLA_SETTINGS_H

// using namespace std;
#include <QDebug>
#include <QList>
#include <QObject>
#include <QVariantList>

class SettingValue : public QObject {
  Q_OBJECT
public:
  // inline SettingsValue() : SettingsValue(nullptr) {
  // }
  SettingValue(const SettingValue &val);
  SettingValue(QObject *parent = nullptr);
  ~SettingValue();
};
Q_DECLARE_METATYPE(SettingValue)

enum ValueType { INTEGER, STRING, LONG };

class Settings {
private:
  /* data */
  void *instance;

  void initModule(QString org, QString name);
  QVariant parseValue(void *value);

public:
  inline Settings(QString org) : Settings(org, "settings") {
  }
  Settings(QString org, QString name);
  ~Settings();

  void set(const char *key, const char *value);
  void set(const QString &key, const QString &value);
  void set(const char *key, const QString &value);
  void set(const char *key, const int &value);
  void set(const char *key, const bool &value);
  void set(const QString &key, const bool &value);
  void set(const QString &key, const QObject *value);
  void set(const QString &key, const QObject &value);
  void set(const char *key, const QObject *value);
  void set(const char *key, const QObject &value);
  void set(const char *key, const QList<QVariant> &value);
  void set(const char *key, const QList<QString> &value);
  void set(const char *key, const QList<char *> &value);
  void set(const char *key, const QList<int> &value);
  void set(const char *key, const QList<QObject> &value);
  void set(const char *key, const QList<QObject *> &value);

  QVariant get(const char *key);
  bool has(const char *key);
  bool has(const QString &key);
  void save();
};

#endif
