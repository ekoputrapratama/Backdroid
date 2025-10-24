#include "main.h"
#include "sqlite3.h"
#include "Backdroid.h"
#undef slots
#include <Python.h>
#define slots Q_SLOTS

QQmlApplicationEngine *engine;
BackDroid *backdroid = nullptr;


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
  QString mode;
  if (type == QtInfoMsg) {
    mode = "INFO";
  } else if (type == QtWarningMsg) {
    mode = "WARNING";
  } else if (type == QtCriticalMsg) {
    mode = "CRITICAL";
  } else if (type == QtFatalMsg) {
    mode = "FATAL";
  } else {
    mode = "DEBUG";
  }
  // std::cout << &mode << " : " << &message;
  // printf("", mode, message);
  qDebug("%s : %s", qPrintable(mode), qPrintable(message));
  // if (!engine->rootObjects().isEmpty() && mode == "INFO") {
  //   QObject *mainForm = engine->rootObjects().first();
  //   QMetaObject::invokeMethod(mainForm, "onConsoleLog", Q_ARG(QVariant, message));
  // } else if (mode == "INFO") {
  //   bnr->onConsoleMessage(message);
  // }
}

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setOrganizationName("Mixaline");
  app.setOrganizationDomain("io.github.mixaline");
  app.setApplicationName("BackDroid");

  qInstallMessageHandler(messageHandler);

  Py_Initialize();

  engine = new QQmlApplicationEngine();

  backdroid = new BackDroid(engine);
  SQLite3 *sqlite = new SQLite3(engine);

  QQmlContext *rootContext = engine->rootContext();
  rootContext->setContextProperty("backdroid", backdroid);
  rootContext->setContextProperty("sqlite3", sqlite);
  // rootContext->setContextProperty("watcher", watcher);
  // rootContext->setContextProperty("pluginsModel", bnr->pluginsModel());

  engine->load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));

  if (engine->rootObjects().isEmpty())
    return -1;

  app.connect(&app, &QGuiApplication::aboutToQuit, backdroid, []() {
    // if (watcher->isRunning()) {
    //   watcher->quit();
    // }
    // ::bnr->killAllProcess();
    // ::bnr->stopAdb();
    backdroid->deleteLater();
  });
  int ret = app.exec();
  // Py_FinalizeEx();
  return ret;
}
