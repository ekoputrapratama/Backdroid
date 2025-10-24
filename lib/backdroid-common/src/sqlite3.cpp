#include <QtSql/QSqlDatabase>
#include "sqlite3.h"

bool isNumber(const QVariant &variant) {
  switch (variant.userType()) {
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
      return true;
  }

  return false;
}

bool isString(const QVariant &variant) {
  return variant.userType() == QMetaType::QString;
}

SQLite3::SQLite3(QQmlApplicationEngine *engine) {
  db = QSqlDatabase::addDatabase("QSQLITE", "Connection");
  this->engine = engine;
}

SQLite3::~SQLite3() {
  delete &db;
}

void SQLite3::connect(const QString &dbName) {
  db.setDatabaseName(dbName);
}

void SQLite3::open() {
  db.open();
}

void SQLite3::exec(const QString &queryStr) {
  QSqlQuery query(db);
  query.prepare(queryStr);

  if (!query.exec()) {
    qCritical() << "error when executing query" << queryStr;
    qCritical() << db.lastError().text();
  }
}

QList<QVariant> SQLite3::select(const QString &queryStr) {
  QList<QVariant> list = {};
  QSqlQuery query(db);
  query.prepare(queryStr);
  query.exec();
  while (query.next()) {
    QJSValue val = engine->newObject();
    QSqlRecord record = query.record();
    int colCount = record.count();

    for (int i = 0; i < colCount; i++) {
      QString name = record.fieldName(i);
      QVariant value = query.value(i);

      if (isString(value)) {
        val.setProperty(name, QJSValue(value.toString()));
      } else if (isNumber(value)) {
        val.setProperty(name, QJSValue(value.toInt()));
      }
    }
    list.append(QVariant::fromValue(val));
  }

  return list;
}

bool SQLite3::isColumnExists(const QString &table, const QString &column) {
  QSqlQuery query(db);
  query.prepare("PRAGMA table_info(" + table + ")");
  bool columnExists = false;
  query.exec();
  while (query.next()) {
    QVariant name = query.value(1);
    qDebug() << "column name " << name.toString();
    if (column == name.toString()) {
      columnExists = true;
      break;
    }
  }
  return columnExists;
}

void SQLite3::close() {
  db.close();
}
// qQmlRegisterType()

// QML_REGISTER_TYPES_AND_REVISIONS()
