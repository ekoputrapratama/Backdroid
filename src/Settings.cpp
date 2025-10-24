#include "Settings.h"

#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QMetaProperty>
#undef slots
#include <Python.h>
#define slots Q_SLOTS

SettingValue::SettingValue(const SettingValue &val) {
}
SettingValue::SettingValue(QObject *parent) : QObject(parent) {
}
SettingValue::~SettingValue() {
}

Settings::Settings(QString org, QString name) {

  QString configPath = QDir::homePath()
                           .append(QDir::separator())
                           .append(".config")
                           .append(QDir::separator())
                           .append(org)
                           .append(QDir::separator())
                           .append(name + ".json");

  QFileInfo info(configPath);
  if (!info.exists()) {
    QDir().mkpath(info.absoluteDir().absolutePath());
  } else {
    QFile file(info.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
      qWarning() << "cannot load configuration from" << configPath;
      return;
    }

    QByteArray data = file.readAll();
  }
  initModule(org, name);
}

Settings::~Settings() {
}

void Settings::initModule(QString org, QString name) {
  PyObject *pName, *pModule, *pDict, *pClass, *pArgs;
  pName = PyUnicode_FromString("backdroid.config");
  pModule = PyImport_Import(pName);

  if (pModule == nullptr) {
    PyErr_Print();
    qCritical() << "Fails to import the module.";
    return;
  }
  Py_DECREF(pName);

  // dict is a borrowed reference.
  pDict = PyModule_GetDict(pModule);
  if (pDict == nullptr) {
    PyErr_Print();
    qCritical() << "Fails to get the dictionary.";
    return;
  }
  Py_DECREF(pModule);

  // Builds the name of a callable class
  pClass = PyDict_GetItemString(pDict, "Configuration");
  if (pClass == nullptr) {
    PyErr_Print();
    qCritical() << "Fails to get the Python class.\n";
    return;
  }

  Py_DECREF(pDict);

  // Creates an instance of the class
  if (PyCallable_Check(pClass)) {
    pArgs = PyTuple_New(2);
    PyObject *arg1 = PyUnicode_FromString(org.toStdString().c_str());
    PyObject *arg2 = PyUnicode_FromString(name.toStdString().c_str());
    PyTuple_SetItem(pArgs, 0, arg1);
    PyTuple_SetItem(pArgs, 1, arg2);

    instance = PyObject_CallObject(pClass, pArgs);
    if (!instance) {
      qCritical() << "Cannot instantiate the Python class";
      PyErr_Print();
      return;
    }

    Py_DECREF(pClass);
    Py_DECREF(arg1);
    Py_DECREF(arg2);

  } else {
    qCritical() << "Cannot instantiate the Python class";
    Py_DECREF(pClass);
  }
}

void Settings::set(const char *key, const QString &value) {
  // document.object().
  set(key, value.toStdString().c_str());
}

void Settings::set(const QString &key, const QString &value) {
  set(key.toStdString().c_str(), value.toStdString().c_str());
}

void Settings::set(const QString &key, const bool &value) {
  set(key.toStdString().c_str(), value);
}

void Settings::set(const char *key, const bool &value) {
  PyObject *pName = PyUnicode_FromString("set");
  PyObject *pKey = PyUnicode_FromString(key);
  PyObject *pVal = PyBool_FromLong(value);

  PyObject *ret = PyObject_CallMethodObjArgs((PyObject *)instance, pName, pKey, pVal, nullptr);
  if (!ret) {
    qCritical() << "Cannot set setting value";
    PyErr_Print();
  }

  Py_DECREF(pName);
  Py_DECREF(pKey);
  Py_DECREF(pVal);
}

void Settings::set(const char *key, const int &value) {

  PyObject *ret = PyObject_CallMethod((PyObject *)instance, "set", "sl", key, value);
  if (!ret) {
    qCritical() << "Cannot set setting value";
    PyErr_Print();
  }
}

void Settings::set(const char *key, const QObject *value) {
  PyObject *pDict, *pVal;
  QList<QByteArray> names = QList(value->dynamicPropertyNames());

  pDict = PyDict_New();
  QList<PyObject *> pyValues = {};
  foreach (QByteArray c, names) {
    QVariant property = value->property(c.data());

    if (property.typeId() == QMetaType::Type::QString) {
      pVal = PyUnicode_FromString(property.toString().toStdString().c_str());
      PyDict_SetItemString(pDict, c.data(), pVal);
    } else if (property.typeId() == QMetaType::Type::Int) {
      pVal = PyLong_FromLong(property.toInt());
      PyDict_SetItemString(pDict, c.constData(), pVal);
    }

    Py_DECREF(pVal);
  }

  PyObject *pKey = PyUnicode_FromString(key);
  PyObject *pName = PyUnicode_FromString("set");
  PyObject *ret = PyObject_CallMethodObjArgs((PyObject *)instance, pName, pKey, pDict, nullptr);
  if (!ret) {
    qCritical() << "Cannot set setting value";
    PyErr_Print();
  }

  Py_DECREF(ret);
  Py_DECREF(pName);
  Py_DECREF(pKey);
  Py_DECREF(pDict);
}

void Settings::set(const QString &key, const QObject *value) {
  set(key.toStdString().c_str(), value);
}
void Settings::set(const QString &key, const QObject &value) {
  set(key.toStdString().c_str(), &value);
}

void Settings::set(const char *key, const QObject &value) {
  set(key, &value);
}

void Settings::set(const char *key, const char *value) {
  qDebug() << "set setting value" << key << value;

  PyObject *ret = PyObject_CallMethod((PyObject *)instance, "set", "ss", key, value, nullptr);
  if (!ret) {
    qCritical() << "Cannot set setting value";
    PyErr_Print();
  }
}

void Settings::set(const char *key, const QList<char *> &value) {
  PyObject *pList, *pVal;
  qDebug() << "set setting value" << key << value;
  qDebug() << "list size" << value.size();

  pList = PyList_New(0);
  foreach (char *str, value) {
    pVal = PyUnicode_FromString(str);
    PyList_Append(pList, pVal);
    Py_DECREF(pVal);
  }

  PyObject *pKey = PyUnicode_FromString(key);
  PyObject *pName = PyUnicode_FromString("set");
  PyObject *ret = PyObject_CallMethodObjArgs((PyObject *)instance, pName, pKey, pList, nullptr);
  if (!ret) {
    qCritical() << "Cannot set setting value";
    PyErr_Print();
  }

  Py_DECREF(pName);
  Py_DECREF(pKey);
  Py_DECREF(pList);
}

void Settings::set(const char *key, const QList<QString> &value) {
}

QVariant Settings::parseValue(void *value) {
  // PyObject *pVal;

  if (PyUnicode_Check(value)) {
    const char *str = PyUnicode_AsUTF8((PyObject *)value);
    qDebug() << "returned value is string" << str;
    return QVariant(str);
  } else if (PyLong_Check(value)) {
    return QVariant(PyLong_AsInt((PyObject *)value));
  } else if (PyFloat_Check(value)) {
    return QVariant(PyFloat_AsDouble((PyObject *)value));
  } else if (PyDict_Check(value)) {
    // need to becareful when playing with dictionary
    // don't randomly decrese the reference
    // cuz dictionary not copying those data but referencing it
    // it will surely be segmentation fault if it's freed randomly
    QVariantMap map = QVariantMap();
    PyObject *keys = PyDict_Keys((PyObject *)value);
    PyObject *next;
    if (PyList_Check(keys)) {
      // okay, it's a list

      for (Py_ssize_t i = 0; i < PyList_Size(keys); ++i) {
        next = PyList_GetItem(keys, i);
        // do something with next
        const char *key = PyUnicode_AsUTF8(next);
        PyObject *pVal = PyDict_GetItem((PyObject *)value, next);

        QVariant val = parseValue(pVal);
        map.insert(key, val);
        // this item is a list member so it's safe to decrease the reference
        Py_DECREF(next);
      }
    }

    return QVariant::fromValue(map);
  } else if (PyList_Check(value)) {
    QVariantList list;
    PyObject *pVal;
    for (Py_ssize_t i = 0; i < PyList_Size((PyObject *)value); ++i) {
      pVal = PyList_GetItem((PyObject *)value, i);
      QVariant val = parseValue(pVal);
      list.append(val);
    }
    Py_DECREF(pVal);
    return QVariant(list);
  } else if (PySet_Check(value)) {
    // return QVariant(QSet<QString>());
  }
  Py_DECREF(value);
  return QVariant();
}

QVariant Settings::get(const char *key) {
  PyObject *value = PyObject_CallMethod((PyObject *)instance, "get", "s", key, nullptr);

  if (!value) {
    qDebug() << "cannot get value";
    PyErr_Print();
  }

  return parseValue(value);
}

void Settings::save() {
  PyObject *ret = PyObject_CallMethod((PyObject *)instance, "save", nullptr);
  if (!ret) {
    qCritical() << "Cannot save settings";
    PyErr_Print();
  }
}
bool Settings::has(const char *key) {
  PyObject *ret = PyObject_CallMethod((PyObject *)instance, "has", "s", key, nullptr);
  if (!ret) {
    qCritical() << "Cannot check settings key";
    PyErr_Print();
  }

  long res = PyLong_AsLong(ret);

  return (res > 0) ? true : false;
}

bool Settings::has(const QString &key) {
  return has(key.toStdString().c_str());
}
