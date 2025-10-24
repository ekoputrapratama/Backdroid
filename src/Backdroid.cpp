#include "Backdroid.h"
#undef slots
#include <Python.h>
#define slots Q_SLOTS

static BackDroid *instance;
/**
 * BackDroid
 */


BackDroid::BackDroid(QQmlApplicationEngine *e) : engine(e) {
  Q_INIT_RESOURCE(common);
  Q_INIT_RESOURCE(backdroid);

  instance = this;
  QString path = QCoreApplication::applicationDirPath();
  appDir = path;

#if defined(BACKDROID_DEBUG)
  QString backdroidModuleDir = QDir(appDir).absolutePath().replace("build", "daemon");

  PyRun_SimpleString("import sys");
  PyRun_SimpleString(QString("sys.path.append(\"%1\")").arg(backdroidModuleDir).toStdString().c_str());
#endif

  settings = new Settings("Mixaline");
  settings->set("applicationDir", appDir);
  settings->save();

#if defined(BACKDROID_DEBUG)
  runDaemon();
#endif

  m_backupDir = QDir(appDir).filePath("backups");

  _watcher = new DeviceWatcher();
  _adb = new AdbWrapper();
  _fastboot = new FastbootWrapper(_watcher);

  QQmlContext *rootContext = engine->rootContext();
  rootContext->setContextProperty("watcher", _watcher);
}

BackDroid::~BackDroid() {
  delete _adb;
  delete _fastboot;
  Q_CLEANUP_RESOURCE(common);
  Q_CLEANUP_RESOURCE(backdroid);

  if (_watcher->isRunning()) {
    _watcher->stop();
    delete _watcher;
  }
}

#ifdef BACKDROID_DEBUG
void BackDroid::runDaemon() {
  QString backdroidModuleDir = QDir(appDir).absolutePath().replace("build", "daemon");

  daemonProcess = new QProcess();
  QProcess::connect(daemonProcess, &QProcess::errorOccurred, daemonProcess,
                    [this](QProcess::ProcessError e) {
                      qCritical() << e;
                    });

  QString program
      = QDir(appDir).absolutePath().replace("build", "daemon/").append("backdroidd.py");

  daemonProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
  env.append("PYTHONPATH=" + backdroidModuleDir);
  daemonProcess->setEnvironment(env);
  daemonProcess->start(program, QStringList());
  daemonProcess->waitForFinished(200);
}
#endif

QList<QVariant> BackDroid::getDevices() {

  AndroidDeviceList *adbDevices = _adb->getDevices();

  QList<QVariant> devices = {};
  this->devices.clear();
  for (int i = 0; i < adbDevices->length(); i++) {
    AndroidDevice *d = adbDevices->get(i);
    this->devices.push(d);

    QVariant device = QVariant::fromValue(d);
    devices.append(device);
  }

  return devices;
}

void BackDroid::setBackupDir(QString backupDir) {
  m_backupDir = backupDir;
}

QString BackDroid::backupDir() const {
  return m_backupDir;
}

QVariant BackDroid::adb() const {
  return QVariant::fromValue(_adb);
}

QVariant BackDroid::fastboot() const {
  return QVariant::fromValue(_fastboot);
}

QVariant BackDroid::watcher() const {
  return QVariant::fromValue(_watcher);
}

QJSValue BackDroid::getProp(const QString &serial, const QString &key, QJSValue cb) {
  // ShellResult *result = m_adb->shell(serial, { "getprop", key }, cb);
  QVariant out = _adb->shell(serial, { "getprop", key }, cb);
  QObject *obj = qvariant_cast<QObject *>(out);
  ShellResult *result = qobject_cast<ShellResult *>(obj);
  return result->output();
}

// QVariant BackDroid::pluginsModel() const {
//   return QVariant::fromValue(pluginManager->pluginsModel());
// }

void BackDroid::mkdir(const QString &dir, QJSValue cb) {

  if (!QDir(dir).exists()) {
    QDir().mkpath(dir);
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

void BackDroid::rm(const QString &path, QJSValue cb) {
  QFile file(path);
  if (file.exists()) {
    file.remove();
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

void BackDroid::rmdir(const QString &path, QJSValue cb) {
  if (QDir(path).exists()) {
    QDir(path).removeRecursively();
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

QString BackDroid::filename(const QString &path) {
  QFileInfo info(path);
  return info.fileName();
}

QString BackDroid::basename(const QString &path) {
  QFileInfo info(path);

  return info.completeBaseName();
}

QJSValue BackDroid::exists(const QString &path) {
  QFileInfo file(path);
  return QJSValue(file.exists());
}

QJSValue BackDroid::isDirectory(const QString &path) {
  QFileInfo file(path);
  return QJSValue(file.isDir());
}

QStringList BackDroid::listDirectory(const QString &path) {
  QStringList result = {};
  QStringList list = QDir(path).entryList();

  foreach (QString name, list) {
    if (name != "." && name != "..") {
      result.append(name);
    }
  }
  return result;
}
