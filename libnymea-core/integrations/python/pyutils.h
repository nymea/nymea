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

void PyDumpError()
{
    if (!PyErr_Occurred()) {
        return;
    }

}

// Write to stdout
PyObject* pyLog_write(PyObject* /*self*/, PyObject* args)
{
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return nullptr;
    if (!QByteArray(what).trimmed().isEmpty()) {
        qCDebug(dcThingManager()) << what;
    }
    Py_RETURN_NONE;
}
PyObject* pyLog_flush(PyObject* /*self*/, PyObject* /*args*/)
{
    // Not really needed... qDebug() flushes already on its own
    Py_RETURN_NONE;
}

static PyMethodDef pyLog_methods[] =
{
    {"write", pyLog_write, METH_VARARGS, "Writes to stdout through qDebug()"},
    {"flush", pyLog_flush, METH_VARARGS, "noop"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyModuleDef pyLog_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "pyLog",               // const char* m_name;
    "pyLog stdout override",// const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    pyLog_methods,        // PyMethodDef *m_methods
    //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
    nullptr, nullptr, nullptr, nullptr
};

// Write to stderr
PyObject* pyWarn_write(PyObject* /*self*/, PyObject* args)
{
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return nullptr;
    if (!QByteArray(what).trimmed().isEmpty()) {
        qCWarning(dcThingManager()) << what;
    }
    Py_RETURN_NONE;
}
PyObject* pyWarn_flush(PyObject* /*self*/, PyObject* /*args*/)
{
    // Not really needed... qDebug() flushes already on its own
    Py_RETURN_NONE;
}

static PyMethodDef pyWarn_methods[] =
{
    {"write", pyWarn_write, METH_VARARGS, "Writes to stderr through qWarnging()"},
    {"flush", pyWarn_flush, METH_VARARGS, "noop"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyModuleDef pyWarn_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "pyWarn",               // const char* m_name;
    "pyWarn stdout override",// const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    pyWarn_methods,        // PyMethodDef *m_methods
    //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
    nullptr, nullptr, nullptr, nullptr
};

#endif // PYUTILS_H
