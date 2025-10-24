#ifndef BACKDROID_H
#define BACKDROID_H

#include "adb_wrapper.h"
#include "devicewatcher.h"
#include "fastboot_wrapper.h"
#include "Settings.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QResource>
#include <QtCore/QtPlugin>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>


class BackDroid : public QObject {
  Q_OBJECT

public:
  BackDroid(QQmlApplicationEngine *engine);
  ~BackDroid();

  Q_PROPERTY(QString backupDir READ backupDir WRITE setBackupDir);
  void setBackupDir(QString backupDir);
  QString backupDir() const;

  Q_SIGNAL void onConsoleMessage(const QString message);

  Q_PROPERTY(QVariant adb READ adb CONSTANT);
  Q_PROPERTY(QVariant watcher READ watcher CONSTANT);
  Q_PROPERTY(QVariant fastboot READ fastboot CONSTANT);

  QVariant adb() const;
  QVariant fastboot() const;
  QVariant watcher() const;
private:
  /* data */
  QQmlApplicationEngine *engine;
  QString appDir;
  QString m_backupDir;
  AdbWrapper *_adb;
  FastbootWrapper *_fastboot;
  DeviceWatcher *_watcher;
  AndroidDeviceList devices;
  Settings *settings;

#ifdef BACKDROID_DEBUG
  QProcess *daemonProcess;
  void runDaemon();
#endif
  void initPyFunctions();
public Q_SLOTS:
  void mkdir(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  void rmdir(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  void rm(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  QString filename(const QString &path);
  QString basename(const QString &path);
  QJSValue isDirectory(const QString &path);
  QStringList listDirectory(const QString &path);
  QJSValue exists(const QString &path);

  QJSValue getProp(const QString &serial, const QString &key,
                   QJSValue cb = QJSValue::UndefinedValue);

  QList<QVariant> getDevices();
};

void *PyInit_backdroid(void *);

#endif
