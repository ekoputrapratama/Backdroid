#include "sed_wrapper.h"
#include <QtCore/QtCore>

SedWrapper::SedWrapper() : CommonWrapper(nullptr) {
  QString path = QCoreApplication::applicationDirPath();
#if defined(Q_OS_LINUX)
  sed = QDir(path).filePath("sed");
#elif defined(Q_OS_WIN32)
  sed = QDir(path).filePath("sed.exe");
#endif
}

SedWrapper::~SedWrapper() {
}

void SedWrapper::subtitute(const QString &what, const QString &to, const QString &filepath) {
  QString str = "s@" + what + "@" + to + "@g";
  QProcess *process = new QProcess();
  // QProcess::connect(process, &QProcess::errorOccurred, process, [this, cmd,
  // args](QProcess::ProcessError e) {
  //   errorFound(e, cmd, args);
  // });

  // qDebug() << "executing command" << cmd << args;
  process->start(sed, { "-i.bak", str, filepath });
  process->waitForFinished();

  QString output(process->readAllStandardOutput());
  QString error(process->readAllStandardError());
  process->close();
}
