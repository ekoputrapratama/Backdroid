#ifndef SQLITE3_H
#define SQLITE3_H

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlModuleRegistration>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

class SQLite3 : public QObject {
  Q_OBJECT
  QML_ELEMENT
private:
  /* data */
  QSqlDatabase db;
  QQmlApplicationEngine *engine;

public:
  SQLite3(QQmlApplicationEngine *engine = nullptr);
  ~SQLite3();

public slots:
  void connect(const QString &dbName);
  void exec(const QString &query);
  void open();
  void close();
  QList<QVariant> select(const QString &query);
  // bool tableExists(const QString &tableName);
  bool isColumnExists(const QString &tableName, const QString &column);
};

#endif
