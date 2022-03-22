#ifndef PYUTILS_H
#define PYUTILS_H

#include <Python.h>
#include <datetime.h>

#include <QLoggingCategory>
#include <QVariant>
#include <QDateTime>

Q_DECLARE_LOGGING_CATEGORY(dcPythonIntegrations)

/* Returns a new reference to PyObject*. */
PyObject *QVariantToPyObject(const QVariant &value)
{
    PyObject *pyValue = nullptr;

    switch (value.type()) {
    case QVariant::Bool:
        pyValue = PyBool_FromLong(value.toBool());
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        pyValue = PyLong_FromLongLong(value.toLongLong());
        break;
    case QVariant::String:
    case QVariant::ByteArray:
        pyValue = PyUnicode_FromString(value.toString().toUtf8());
        break;
    case QVariant::Double:
        pyValue = PyFloat_FromDouble(value.toDouble());
        break;
    case QVariant::Invalid:
        pyValue = Py_None;
        Py_INCREF(pyValue);
        break;
    case QVariant::DateTime: {
        PyObject *longObj = PyLong_FromLongLong(value.toDateTime().toMSecsSinceEpoch() / 1000);
        PyObject *timeTuple = Py_BuildValue("(O)", longObj);
        pyValue = PyDateTime_FromTimestamp(timeTuple);
        Py_DECREF(longObj);
        Py_DECREF(timeTuple);
        break;
    }
    case QVariant::StringList: {
        QStringList stringList = value.toStringList();
        pyValue = PyTuple_New(stringList.length());
        for (int i = 0; i < stringList.count(); i++) {
            PyObject *entry = PyUnicode_FromString(stringList.at(i).toUtf8());
            PyTuple_SetItem(pyValue, i, entry);
        }
        break;
    }
    default:
        Q_ASSERT_X(false, "pyutils.h", QString("Unhandled data type in converting QVariant to PyObject: %1").arg(value.type()).toUtf8());
        qCWarning(dcPythonIntegrations()) << "Unhandled data type" << value.type() << "in conversion from Param to PyParam!";
        pyValue = Py_None;
        Py_INCREF(pyValue);
        break;
    }

    return pyValue;
}

QVariant PyObjectToQVariant(PyObject *pyObject)
{
    if (qstrcmp(pyObject->ob_type->tp_name, "int") == 0) {
        return QVariant(PyLong_AsLongLong(pyObject));
    }

    if (qstrcmp(pyObject->ob_type->tp_name, "str") == 0) {
        return QVariant(PyUnicode_AsUTF8AndSize(pyObject, nullptr));
    }

    if (qstrcmp(pyObject->ob_type->tp_name, "double") == 0) {
        return QVariant(PyFloat_AsDouble(pyObject));
    }

    if (qstrcmp(pyObject->ob_type->tp_name, "float") == 0) {
        return QVariant(PyFloat_AsDouble(pyObject));
    }

    if (qstrcmp(pyObject->ob_type->tp_name, "bool") == 0) {
        return QVariant(PyObject_IsTrue(pyObject));
    }

    if (qstrcmp(pyObject->ob_type->tp_name, "datetime.datetime") == 0) {
        PyObject *timestampFunction = PyObject_GetAttrString(pyObject, "timestamp");
        PyObject *timestampFunctionResult = PyObject_CallFunction(timestampFunction, nullptr);
        QDateTime ret = QDateTime::fromMSecsSinceEpoch(PyLong_AsLongLong(timestampFunctionResult) * 1000);
        Py_DECREF(timestampFunction);
        Py_DECREF(timestampFunctionResult);
        return ret;
    }

    Q_ASSERT_X(false, "pyutils.h", QString("Unhandled data type in converting PyObject to QVariant: %1").arg(pyObject->ob_type->tp_name).toUtf8());
    qCWarning(dcPythonIntegrations()) << QString("Unhandled data type in converting PyObject to QVariant: %1").arg(pyObject->ob_type->tp_name).toUtf8();
    return QVariant();
}


#endif // PYUTILS_H
