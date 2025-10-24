#include "adb_wrapper.h"


AdbWrapper::AdbWrapper(QObject *parent) : CommonWrapper(parent) {
  QString path = QCoreApplication::applicationDirPath();
  devices = new AndroidDeviceList();
#if defined(Q_OS_LINUX)
  this->adb = QDir(path).filePath("adb");
#elif defined(Q_OS_WIN32)
  this->adb = QDir(path).filePath("adb.exe");
#endif
}

AdbWrapper::~AdbWrapper() {
  delete devices;
}

void AdbWrapper::startServer(QJSValue cb) {
  runAsync(adb, { "start-server" }, "start-adb", cb);
}

void AdbWrapper::killServer() {
  run(adb, { "kill-server" });
}

void AdbWrapper::backup(const QString &serial, const QString &cwd, QJSValue cb) {
  runAsync(adb, { "backup" }, "adb-backup", cb, cwd);
}

AndroidDeviceList *AdbWrapper::getDevices() {

  AndroidDeviceList *devices = new AndroidDeviceList();
  ShellResult *result = this->run(adb, { "devices", "-l" });
  QStringList lines
      = result->output().split(QRegularExpression("\n|\r|\r\n"), Qt::SplitBehaviorFlags::SkipEmptyParts);
  lines.pop_front();
  this->devices->clear();

  foreach (QString s, lines) {
    AndroidDevice *device = new AndroidDevice();
    QStringList sp = s.split(QRegularExpression(" "), Qt::SplitBehaviorFlags::SkipEmptyParts);

    device->setSerial(sp[0]);
    device->setState(sp[1]);

    devices->append(device);
    this->devices->push(device);
  }
  return devices;
}

void AdbWrapper::reboot(const QString &serial, const QString &toState, QJSValue cb) {
  QString tag;
  run(adb, { "-s", serial, "reboot", toState });
  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

void AdbWrapper::waitFor(const QString &serial, const QString &state, QJSValue cb) {
  QString tag = "wait-for-" + state;
  runAsync(adb, { "-s", serial, tag }, tag, cb);
}

void AdbWrapper::connect(const QString &ipAndPort, QJSValue cb) {
}
void AdbWrapper::disconnect(const QString &ipAndPort, QJSValue cb) {
}
void AdbWrapper::tcpip(const QString &port, QJSValue cb) {
}

void AdbWrapper::install(const QString &serial, const QString &apkPath, QJSValue cb) {
  runAsync(adb, { "-s", serial, "install", apkPath }, "adb-install", cb);
}

void AdbWrapper::uninstall(const QString &serial, const QString &packageName, QJSValue cb) {
  runAsync(adb, { "-s", serial, "uninstall", packageName }, "adb-uninstall", cb);
}

void AdbWrapper::pull(const QString &serial, const QString &path, const QString &filename,
                      QJSValue cb) {
  runAsync(adb, { "-s", serial, "pull", path, filename }, "adb-pull", cb);
}

void AdbWrapper::push(const QString &serial, const QString &filename, const QString &path,
                      QJSValue cb) {
  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    runAsync(adb, { "-s", serial, "push", filename, path }, "adb-push", cb);
  } else {
    run(adb, { "-s", serial, "push", filename, path });
  }
}

void AdbWrapper::sideload(const QString &serial, const QString &filepath, QJSValue cb) {
  runAsync(adb, { "-s", serial, "sideload", filepath }, "adb-sideload", cb);
}

QVariant AdbWrapper::shell(const QString &serial, const QStringList &args, QJSValue cb) {
  QStringList adbArgs = { "-s", serial, "shell" };
  foreach (QString arg, args) { adbArgs.append(arg); }

  if (cb.isUndefined() || cb.isNull()) {
    return QVariant::fromValue(run(adb, adbArgs));
  } else {
    runAsync(adb, adbArgs, "adb-shell", cb);
    return QVariant::fromValue(new ShellResult());
  }
}

bool AdbWrapper::isUsbDevice(QString serial) {
  QProcess *process = new QProcess();

  QStringList args = { "devices", "-l" };

  process->start(adb, args);
  process->waitForFinished();

  QString output(process->readAllStandardOutput());

  QStringList lines = output.split(QRegularExpression("\n|\r|\r\n"), Qt::SplitBehaviorFlags::SkipEmptyParts);

  QString line = "";
  foreach (QString l, lines) {
    if (l.startsWith(serial)) {
      line = l;
      break;
    }
  }

  return !line.isEmpty() && line.contains("usb");
}

void AdbWrapper::forward(const QString &serial, QVariantList arguments) {
  QStringList args = {
    "-s",
    serial,
    "forward",
  };

  foreach (QVariant arg, arguments) { args.append(arg.toString()); }

  run(adb, args);
}
