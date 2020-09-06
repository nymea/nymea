#ifndef PYUTILS_H
#define PYUTILS_H

#include <Python.h>

#include <QLoggingCategory>
#include <QVariant>

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
    default:
        qCWarning(dcPythonIntegrations()) << "Unhandled data type in conversion from Param to PyParam!";
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

    Q_ASSERT_X(false, "pyutils.h", QString("Unhandled data type in converting PyObject to QVariant: %1").arg(pyObject->ob_type->tp_name).toUtf8());
    qCWarning(dcPythonIntegrations()) << QString("Unhandled data type in converting PyObject to QVariant: %1").arg(pyObject->ob_type->tp_name).toUtf8();
    return QVariant();
}


#endif // PYUTILS_H
