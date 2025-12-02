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

#ifndef PYPLUGINSTORAGE_H
#define PYPLUGINSTORAGE_H

#include <Python.h>
#include "structmember.h"
#include "pyutils.h"

#include "loggingcategories.h"
#include "nymeasettings.h"
#include "typeutils.h"

#include <QSettings>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 * When using this, make sure to call PyPluginStorage_setPluginStorage() while holding the GIL to initialize
 * stuff after constructing it.
 *
 * The pluginStorage() pointer is owned by the C++ plugin class, however, without an actual C++
 * plugin it will never be accessed by anyone. So we pass it into the python context and use it in
 * there.
 *
 */

typedef struct {
    PyObject_HEAD
    QSettings *pluginStorage;
} PyPluginStorage;

static int PyPluginStorage_init(PyPluginStorage */*self*/, PyObject */*args*/, PyObject */*kwds*/)
{
    qCDebug(dcPythonIntegrations()) << "+++ PyPluginStorage";
    return 0;
}

void PyPluginStorage_setPluginStorage(PyPluginStorage *self, QSettings* pluginStorage)
{
    self->pluginStorage = pluginStorage;
}

static void PyPluginStorage_dealloc(PyPluginStorage * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyPluginStorage";
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyPluginStorage_value(PyPluginStorage* self, PyObject* args) {
    char *keyStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &keyStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: value(key)");
        return nullptr;
    }

    QVariant value = self->pluginStorage->value(keyStr);
    return QVariantToPyObject(value);
};

static PyObject * PyPluginStorage_setValue(PyPluginStorage* self, PyObject* args) {
    char *keyStr = nullptr;
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "sO", &keyStr, &value)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in setValue call. Expected: setValue(key, value)");
        return nullptr;
    }

    self->pluginStorage->setValue(keyStr, PyObjectToQVariant(value));
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_beginGroup(PyPluginStorage* self, PyObject* args) {
    char *groupStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &groupStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in beginGroup call. Expected: beginGroup(group)");
        return nullptr;
    }

    self->pluginStorage->beginGroup(groupStr);
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_endGroup(PyPluginStorage* self, PyObject* args) {
    Q_UNUSED(args)
    self->pluginStorage->endGroup();
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_remove(PyPluginStorage *self, PyObject* args) {
    char *keyStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &keyStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in remove call. Expected: remove(key)");
        return nullptr;
    }
    self->pluginStorage->remove(keyStr);
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_childKeys(PyPluginStorage* self, PyObject* args) {
    Q_UNUSED(args)
    QStringList keys = self->pluginStorage->childKeys();
    return QVariantToPyObject(keys);
};

static PyMethodDef PyPluginStorage_methods[] = {
    { "value", (PyCFunction)PyPluginStorage_value, METH_VARARGS, "Get a value from the plugin storage" },
    { "setValue", (PyCFunction)PyPluginStorage_setValue, METH_VARARGS, "Set a value to the plugin storage"},
    { "beginGroup", (PyCFunction)PyPluginStorage_beginGroup, METH_VARARGS, "Begin a group in the plugin storage."},
    { "endGroup", (PyCFunction)PyPluginStorage_endGroup, METH_VARARGS, "End a group in the plugin storage."},
    { "remove", (PyCFunction)PyPluginStorage_remove, METH_VARARGS, "Remove an entry/group from the plugin storage."},
    { "childKeys", (PyCFunction)PyPluginStorage_childKeys, METH_VARARGS, "List all keys (including subgroups) of the current group."},
    {nullptr, nullptr, 0, nullptr} // sentinel
};


static PyTypeObject PyPluginStorageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.PluginStorage",   /* tp_name */
    sizeof(PyPluginStorage), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyPluginStorage_dealloc, /* tp_dealloc */
};



static void registerPluginStorageType(PyObject *module)
{
    PyPluginStorageType.tp_new = PyType_GenericNew;
    PyPluginStorageType.tp_methods = PyPluginStorage_methods;
    PyPluginStorageType.tp_init = reinterpret_cast<initproc>(PyPluginStorage_init);
    PyPluginStorageType.tp_doc = "PluginStorage can be used by plugins to store key-value pairs to a persistant place.";
    PyPluginStorageType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyPluginStorageType) < 0) {
        return;
    }
    PyModule_AddObject(module, "PluginStorage", reinterpret_cast<PyObject*>(&PyPluginStorageType));
}

#pragma GCC diagnostic pop

#endif // PYPLUGINSTORAGE_H
