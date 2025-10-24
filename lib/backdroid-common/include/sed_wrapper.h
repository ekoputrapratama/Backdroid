#ifndef SED_WRAPPER_H
#define SED_WRAPPER_H

#include "common_wrapper.h"
#include <QtCore/QProcess>

class SedWrapper : public CommonWrapper {
  Q_OBJECT
private:
  QString sed;

public:
  SedWrapper();
  ~SedWrapper();

public slots:
  void subtitute(const QString &what, const QString &to, const QString &filepath);
};

#endif
