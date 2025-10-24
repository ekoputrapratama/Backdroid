#include "common_wrapper.h"


/**
 * ShellResult
 */
void ShellResult::setOutput(QString output) {
  m_output = output;
}
QString ShellResult::output() const {
  return m_output;
}
void ShellResult::setError(QString error) {
  m_error = error;
}
QString ShellResult::error() const {
  return m_error;
}

int ShellResult::code() const {
  return m_code;
}
void ShellResult::setCode(int code) {
  m_code = code;
}
/*
 * CommonWrapper
 */

CommonWrapper::CommonWrapper(QObject *parent) : QObject(parent) {
}

CommonWrapper::~CommonWrapper() {
}

AsyncProcess *CommonWrapper::getAsyncProcessByTag(const QString &tag) {
  AsyncProcess *res = nullptr;

  if (asyncProcess.length() > 0) {
    foreach (AsyncProcess *p, this->asyncProcess) {
      if (p->tag == tag) {
        res = p;
        break;
      }
    }
  }

  if (res == nullptr) {
    res = new AsyncProcess(tag, new QProcess());
    this->asyncProcess.append(res);
  }

  return res;
}

void CommonWrapper::killAllProcess() {
  foreach (AsyncProcess *p, asyncProcess) {
    if (p->running() == QProcess::ProcessState::Running) {
      p->close();
      p->terminate();
    }
  }
  asyncProcess.clear();
}

ShellResult *CommonWrapper::run(const QString &cmd, const QStringList &args) {
  QProcess *process = new QProcess();
  QProcess::connect(process, &QProcess::errorOccurred, process,
                    [this, cmd, args](QProcess::ProcessError e) {
                      errorFound(e, cmd, args);
                    });

  // qDebug() << "executing command"
  //          << "adb" << args;
  process->start(cmd, args);
  process->waitForFinished();

  QString output(process->readAllStandardOutput());
  QString error(process->readAllStandardError());
  process->close();

  ShellResult *result = new ShellResult();
  result->setCode(process->exitCode());
  result->setOutput(output);
  result->setError(error);

  return result;
}

void CommonWrapper::runAsync(const QString &cmd, const QStringList &args, const QString &tag,
                             QJSValue cb, const QString &cwd) {

  AsyncProcess *p = getAsyncProcessByTag(tag);
  QProcess *process = p->getProcess();

  if (process->state() == QProcess::ProcessState::NotRunning) {
    QObject *error_context = new QObject();
    QObject::connect(process, &QProcess::errorOccurred, error_context,
                     [this, cmd, args, error_context](QProcess::ProcessError e) {
                       errorFound(e, cmd, args);
                       delete error_context;
                     });

    QObject *cmd_context = new QObject();
    QObject::connect(
        process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        cmd_context, [this, cmd, args, tag, cb, cmd_context](int c) {
          executeCommandFinished(c, cmd, args, tag, cb);
          delete cmd_context;
        });
    if (!cwd.isEmpty()) {
      process->setWorkingDirectory(cwd);
    }
    process->start(cmd, args);
  } else {
    if (!cwd.isEmpty()) {
      process->setWorkingDirectory(cwd);
    }
    // process->waitForFinished();
    process->execute(cmd, args);
  }
}

void CommonWrapper::executeCommandFinished(const int code, const QString &cmd,
                                           const QStringList &args, const QString &tag,
                                           QJSValue cb) {

  if (!cb.isUndefined() && !cb.isNull() && cb.isCallable()) {
    QJSValueList params = QJSValueList {};
    AsyncProcess *p = getAsyncProcessByTag(tag);
    QProcess *process = p->getProcess();
    QString output(process->readAllStandardOutput());
    QString error(process->readAllStandardError());
    params << code << output << error << tag;

    cb.call(params);
  }
}

void CommonWrapper::errorFound(QProcess::ProcessError e, const QString &cmd,
                               const QStringList &args) {
  qCritical() << "error during executing command: " << cmd << args << e;
}
