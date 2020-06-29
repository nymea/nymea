#ifndef PYUTILS_H
#define PYUTILS_H

#include <Python.h>

#include "loggingcategories.h"

#include <QVariant>

/* Returns a PyObject. RefCount will be 1 */
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
        pyValue = PyLong_FromLong(value.toLongLong());
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
        qCWarning(dcThingManager) << "Unhandled data type in conversion from Param to PyParam!";
        pyValue = Py_None;
        Py_INCREF(pyValue);
        break;
    }

    return pyValue;
}

QVariant PyObjectToQVariant(PyObject *pyObject)
{
    // FIXME: is there any better way to do this?
    PyObject* repr = PyObject_Repr(pyObject);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AS_STRING(str);

    QVariant value(bytes);

    Py_XDECREF(repr);
    Py_XDECREF(str);

    return value;
}

#endif // PYUTILS_H
