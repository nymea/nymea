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

#ifndef PYPARAM_H
#define PYPARAM_H

#include <Python.h>
#include <structmember.h>

#include "pyutils.h"

#include "types/param.h"
#include "types/paramtype.h"

#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


typedef struct _pyparam {
    PyObject_HEAD
    PyObject* pyParamTypeId = nullptr;
    PyObject* pyValue = nullptr;
} PyParam;

static PyMethodDef PyParam_methods[] = {
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyParam_members[] = {
    {"paramTypeId", T_OBJECT_EX, offsetof(PyParam, pyParamTypeId), 0, "Param type ID"},
    {"value", T_OBJECT_EX, offsetof(PyParam, pyValue), 0, "Param value"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};


static int PyParam_init(PyParam *self, PyObject *args, PyObject *kwds)
{
    qCDebug(dcPythonIntegrations()) << "+++ PyParam";
    static char *kwlist[] = {"paramTypeId", "value", nullptr};
    PyObject *paramTypeId = nullptr, *value = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist, &paramTypeId, &value))
        return -1;

    if (paramTypeId) {
        Py_INCREF(paramTypeId);
        self->pyParamTypeId = paramTypeId;
    }
    if (value) {
        Py_INCREF(value);
        self->pyValue = value;
    }
    return 0;
}

static void PyParam_dealloc(PyParam * self) {
    qCDebug(dcPythonIntegrations()) << "--- PyParam";
    Py_XDECREF(self->pyParamTypeId);
    Py_XDECREF(self->pyValue);
    Py_TYPE(self)->tp_free(self);
}

static PyTypeObject PyParamType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.Param",   /* tp_name */
    sizeof(PyParam), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyParam_dealloc,/* tp_dealloc */
};

static PyParam* PyParam_fromParam(const Param &param)
{
    PyObject *pyParamValue = QVariantToPyObject(param.value());
    PyObject *args = Py_BuildValue("(sO)", param.paramTypeId().toString().toUtf8().data(), pyParamValue);

    PyParam *pyParam = (PyParam*)PyObject_CallObject((PyObject*)&PyParamType, args);

    Py_DECREF(pyParamValue);
    Py_DECREF(args);

    return pyParam;
}

static Param PyParam_ToParam(PyParam *pyParam)
{
    ParamTypeId paramTypeId = ParamTypeId(PyUnicode_AsUTF8AndSize(pyParam->pyParamTypeId, nullptr));
    QVariant value = PyObjectToQVariant(pyParam->pyValue);
    return Param(paramTypeId, value);
}

static PyObject* PyParams_FromParamList(const ParamList &params)
{
    PyObject* result = PyTuple_New(params.length());
    for (int i = 0; i < params.count(); i++) {
        PyParam *pyParam = PyParam_fromParam(params.at(i));
        PyTuple_SetItem(result, i, (PyObject*)pyParam);
    }
    return result;
}

static ParamList PyParams_ToParamList(PyObject *pyParams)
{
    ParamList params;

    if (pyParams == nullptr) {
        return params;
    }

    PyObject *iter = PyObject_GetIter(pyParams);

    while (iter) {
        PyObject *next = PyIter_Next(iter);
        if (!next) {
            break;
        }
        if (next->ob_type != &PyParamType) {
            qCWarning(dcThingManager()) << "Invalid parameter passed in param list";
            continue;
        }

        PyParam *pyParam = reinterpret_cast<PyParam*>(next);
        params.append(PyParam_ToParam(pyParam));
        Py_DECREF(next);
    }

    Py_DECREF(iter);
    return params;
}

static void registerParamType(PyObject *module)
{
    PyParamType.tp_new = PyType_GenericNew;
    PyParamType.tp_init = reinterpret_cast<initproc>(PyParam_init);
    PyParamType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyParamType.tp_methods = PyParam_methods;
    PyParamType.tp_members = PyParam_members;
    PyParamType.tp_doc = "Param class";

    if (PyType_Ready(&PyParamType) < 0) {
        return;
    }
    PyModule_AddObject(module, "Param", reinterpret_cast<PyObject*>(&PyParamType));
}

#pragma GCC diagnostic pop

#endif // PYPARAM_H
