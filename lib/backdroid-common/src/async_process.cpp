#include "async_process.h"

AsyncProcess::AsyncProcess(QString tag, QProcess *process) {
  this->tag = tag;
  this->process = process;
}

AsyncProcess::~AsyncProcess() {
  if (process) {
    process->close();
    process = nullptr;
  }
}

void AsyncProcess::start(const QString &cmd, const QStringList &args, AsyncProcessCallback cb) {
  process->connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this, cb](int c) {
    //            executeCmdFinished(c, tag);
    QString output(process->readAllStandardOutput());
    QString error(process->readAllStandardError());
    cb(c, output, error, this->tag);
  });
  process->start(cmd, args);
}

void AsyncProcess::kill() {
  process->kill();
}

void AsyncProcess::terminate() {
  process->terminate();
}

void AsyncProcess::close() {
  process->close();
}

QProcess::ProcessState AsyncProcess::running() {
  return process->Running;
}
QProcess *AsyncProcess::getProcess() {
  return process;
}
