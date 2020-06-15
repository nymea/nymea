#ifndef PYNYMEALOGGINGHANDLER_H
#define PYNYMEALOGGINGHANDLER_H

#include <Python.h>
#include "structmember.h"

#include <QStringList>
#include <QLoggingCategory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct {
    PyObject_HEAD
    char *category;
} PyNymeaLoggingHandler;

static int PyNymeaLoggingHandler_init(PyNymeaLoggingHandler */*self*/, PyObject */*args*/, PyObject */*kwds*/)
{
    return 0;
}

static void PyNymeaLoggingHandler_dealloc(PyNymeaLoggingHandler * self)
// destruct the object
{
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}


static PyObject * PyNymeaLoggingHandler_log(PyNymeaLoggingHandler* self, PyObject* args)
{
    QStringList strings;
    for (int i = 0; i < PyTuple_GET_SIZE(args); i++) {
        PyObject *obj = PyTuple_GET_ITEM(args, i);
        PyObject* repr = PyObject_Repr(obj);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);
        Py_XDECREF(repr);
        Py_XDECREF(str);
        strings.append(bytes);
    }
    qCDebug(QLoggingCategory(self->category)).noquote() << strings.join(' ');
    Py_RETURN_NONE;
}

static PyObject * PyNymeaLoggingHandler_warn(PyNymeaLoggingHandler* self, PyObject* args)
{
    QStringList strings;
    for (int i = 0; i < PyTuple_GET_SIZE(args); i++) {
        PyObject *obj = PyTuple_GET_ITEM(args, i);
        PyObject* repr = PyObject_Repr(obj);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);
        Py_XDECREF(repr);
        Py_XDECREF(str);
        strings.append(bytes);
    }
    qCWarning(QLoggingCategory(self->category)).noquote() << strings.join(' ');
    Py_RETURN_NONE;
}

static PyMethodDef PyNymeaLoggingHandler_methods[] = {
    { "log", (PyCFunction)PyNymeaLoggingHandler_log,    METH_VARARGS,       "Add a new descriptor to the discovery" },
    { "warn", (PyCFunction)PyNymeaLoggingHandler_warn,    METH_VARARGS,       "Add a new descriptor to the discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyNymeaLoggingHandlerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.NymeaLoggingHandler",   /* tp_name */
    sizeof(PyNymeaLoggingHandler), /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Logging handler for nymea", /* tp_doc */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


static void registerNymeaLoggingHandler(PyObject *module)
{

    PyNymeaLoggingHandlerType.tp_new = PyType_GenericNew;
    PyNymeaLoggingHandlerType.tp_dealloc = reinterpret_cast<destructor>(PyNymeaLoggingHandler_dealloc);
    PyNymeaLoggingHandlerType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyNymeaLoggingHandlerType.tp_doc = "NymeaLoggingHandler class";
    PyNymeaLoggingHandlerType.tp_methods = PyNymeaLoggingHandler_methods;
    PyNymeaLoggingHandlerType.tp_init = reinterpret_cast<initproc>(PyNymeaLoggingHandler_init);
    if (PyType_Ready(&PyNymeaLoggingHandlerType) == 0) {
        PyModule_AddObject(module, "NymeaLoggingHandler", (PyObject *)&PyNymeaLoggingHandlerType);
    }
}


#endif // PYNYMEALOGGINGHANDLER_H
