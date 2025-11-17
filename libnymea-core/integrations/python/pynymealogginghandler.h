// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PYNYMEALOGGINGHANDLER_H
#define PYNYMEALOGGINGHANDLER_H

#include <Python.h>
#include "structmember.h"

#include <QStringList>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcPythonIntegrations)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

typedef struct {
    PyObject_HEAD
    char *category;
} PyNymeaLoggingHandler;

static int PyNymeaLoggingHandler_init(PyNymeaLoggingHandler *self, PyObject *args, PyObject */*kwds*/)
{
    qCDebug(dcPythonIntegrations()) << "+++ PyNymeaLoggingHandler";
    char *category = nullptr;
    if (!PyArg_ParseTuple(args, "s", &category)) {
        qCWarning(dcPythonIntegrations()) << "PyNymeaLoggingHandler: Error parsing parameters";
        return -1;
    }

    self->category = qstrdup(category);

    return 0;
}

static void PyNymeaLoggingHandler_dealloc(PyNymeaLoggingHandler * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyNymeaLoggingHandler";
    delete[] self->category;
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyNymeaLoggingHandler_info(PyNymeaLoggingHandler* self, PyObject* args)
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
    // FIXME: We'll want to use qCInfo() here but the system can't really deal with that yet
    // Move from qCDebug() to qCInfo() when we support controlling that
    qCDebug(QLoggingCategory(self->category)).noquote() << strings.join(' ');
    Py_RETURN_NONE;
}

static PyObject * PyNymeaLoggingHandler_debug(PyNymeaLoggingHandler* self, PyObject* args)
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
    { "log", (PyCFunction)PyNymeaLoggingHandler_info,    METH_VARARGS, "Log an info message to the nymea log. Same as info()." },
    { "info", (PyCFunction)PyNymeaLoggingHandler_info,   METH_VARARGS, "Log an info message to the nymea log." },
    { "debug", (PyCFunction)PyNymeaLoggingHandler_debug,   METH_VARARGS, "Log a debug message to the nymea log." },
    { "warn", (PyCFunction)PyNymeaLoggingHandler_warn,   METH_VARARGS, "Log a warning message to the nymea log." },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyNymeaLoggingHandlerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.NymeaLoggingHandler",   /* tp_name */
    sizeof(PyNymeaLoggingHandler), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyNymeaLoggingHandler_dealloc,/* tp_dealloc */
};


static void registerNymeaLoggingHandler(PyObject *module)
{

    PyNymeaLoggingHandlerType.tp_new = PyType_GenericNew;
    PyNymeaLoggingHandlerType.tp_init = reinterpret_cast<initproc>(PyNymeaLoggingHandler_init);
    PyNymeaLoggingHandlerType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyNymeaLoggingHandlerType.tp_methods = PyNymeaLoggingHandler_methods;
    PyNymeaLoggingHandlerType.tp_doc = "Logging handler for nymea.";

    if (PyType_Ready(&PyNymeaLoggingHandlerType) == 0) {
        PyModule_AddObject(module, "NymeaLoggingHandler", (PyObject *)&PyNymeaLoggingHandlerType);
    }
}

#pragma GCC diagnostic pop

#endif // PYNYMEALOGGINGHANDLER_H
