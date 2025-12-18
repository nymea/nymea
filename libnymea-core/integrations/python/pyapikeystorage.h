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

#ifndef PYAPIKEYSTORAGE_H
#define PYAPIKEYSTORAGE_H

#include "pyutils.h"
#include "structmember.h"
#include <Python.h>

#include "loggingcategories.h"
#include "network/apikeys/apikeystorage.h"
#include "nymeasettings.h"
#include "typeutils.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 * When using this, make sure to call PyApiKeyStorage_setApiKeyStorage() while holding the GIL to initialize
 * stuff after constructing it.
 *
 * The apiKeyStorage() pointer is owned by the C++ plugin class, however, without an actual C++
 * plugin it will never be accessed by anyone. So we pass it into the python context and use it in
 * there.
 *
 */

typedef struct
{
    PyObject_HEAD ApiKey *apiKey;
} PyApiKey;

static int PyApiKey_init(PyApiKey * /*self*/, PyObject * /*args*/, PyObject * /*kwds*/)
{
    qCDebug(dcPythonIntegrations()) << "+++ PyApiKey";
    return 0;
}

void PyApiKey_setApiKey(PyApiKey *self, const ApiKey &apiKey)
{
    self->apiKey = new ApiKey(apiKey);
}

static void PyApiKey_dealloc(PyApiKey *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyApiKey";
    delete self->apiKey;
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyApiKey_data(PyApiKey *self, PyObject *args)
{
    char *keyStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &keyStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: requestKey(key)");
        return nullptr;
    }

    QByteArray data = self->apiKey->data(QString(keyStr));
    return QVariantToPyObject(data);
};

static PyMethodDef PyApiKey_methods[] = {
    {"data", (PyCFunction) PyApiKey_data, METH_VARARGS, "Get data from the API key."}, {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyApiKeyType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.ApiKey", /* tp_name */
    sizeof(PyApiKey),                              /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor) PyApiKey_dealloc,                 /* tp_dealloc */
};

typedef struct
{
    PyObject_HEAD ApiKeyStorage *apiKeyStorage;
} PyApiKeyStorage;

static int PyApiKeyStorage_init(PyApiKeyStorage * /*self*/, PyObject * /*args*/, PyObject * /*kwds*/)
{
    qCDebug(dcPythonIntegrations()) << "+++ PyApiKeyStorage";
    return 0;
}

void PyApiKeyStorage_setApiKeyStorage(PyApiKeyStorage *self, ApiKeyStorage *apiKeyStorage)
{
    self->apiKeyStorage = apiKeyStorage;
}

static void PyApiKeyStorage_dealloc(PyApiKeyStorage *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyApiKeyStorage";
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyApiKeyStorage_requestKey(PyApiKeyStorage *self, PyObject *args)
{
    char *nameStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &nameStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: requestKey(name)");
        return nullptr;
    }

    PyApiKey *pyApiKey = (PyApiKey *) PyObject_CallObject((PyObject *) &PyApiKeyType, args);
    PyApiKey_setApiKey(pyApiKey, self->apiKeyStorage->requestKey(nameStr));
    return (PyObject *) pyApiKey;
};

static PyMethodDef PyApiKeyStorage_methods[] = {
    {"requestKey", (PyCFunction) PyApiKeyStorage_requestKey, METH_VARARGS, "Get an API key from the API key storage."}, {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyApiKeyStorageType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.ApiKeyStorage", /* tp_name */
    sizeof(PyApiKeyStorage),                              /* tp_basicsize */
    0,                                                    /* tp_itemsize */
    (destructor) PyApiKeyStorage_dealloc,                 /* tp_dealloc */
};

static void registerApiKeyStorageType(PyObject *module)
{
    PyApiKeyStorageType.tp_new = PyType_GenericNew;
    PyApiKeyStorageType.tp_methods = PyApiKeyStorage_methods;
    PyApiKeyStorageType.tp_init = reinterpret_cast<initproc>(PyApiKeyStorage_init);
    PyApiKeyStorageType.tp_doc = "ApiKeyStorage holds API keys. API keys need to be requested in the plugin JSON file.";
    PyApiKeyStorageType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyApiKeyStorageType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ApiKeyStorage", reinterpret_cast<PyObject *>(&PyApiKeyStorageType));

    PyApiKeyType.tp_new = PyType_GenericNew;
    PyApiKeyType.tp_methods = PyApiKey_methods;
    PyApiKeyType.tp_init = reinterpret_cast<initproc>(PyApiKey_init);
    PyApiKeyType.tp_doc = "ApiKey holds an API keys data as key-value pairs.";
    PyApiKeyType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyApiKeyType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ApiKey", reinterpret_cast<PyObject *>(&PyApiKeyType));
}

#pragma GCC diagnostic pop

#endif // PYAPIKEYSTORAGE_H
