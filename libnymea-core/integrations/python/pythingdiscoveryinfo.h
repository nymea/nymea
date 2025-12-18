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

#ifndef PYTHINGDISCOVERYINFO_H
#define PYTHINGDISCOVERYINFO_H

#include "structmember.h"
#include <Python.h>

#include "pyparam.h"
#include "pythingdescriptor.h"

#include "integrations/thingdiscoveryinfo.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMutex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 * When using this, make sure to call PyThingDiscoveryInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingDiscoveryInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the discovery and destroyed it but the PyThingDiscoveryInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */

typedef struct
{
    PyObject_HEAD ThingDiscoveryInfo *info;
    PyObject *pyThingClassId = nullptr;
    PyObject *pyParams = nullptr;
} PyThingDiscoveryInfo;

static PyObject *PyThingDiscoveryInfo_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
    PyThingDiscoveryInfo *self = (PyThingDiscoveryInfo *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyThingDiscoveryInfo";
    return (PyObject *) self;
}

void PyThingDiscoveryInfo_setInfo(PyThingDiscoveryInfo *self, ThingDiscoveryInfo *info)
{
    self->info = info;
    self->pyThingClassId = PyUnicode_FromString(info->thingClassId().toString().toUtf8().data());
    self->pyParams = PyParams_FromParamList(info->params());
}

static void PyThingDiscoveryInfo_dealloc(PyThingDiscoveryInfo *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThingDiscoveryInfo";
    Py_DECREF(self->pyThingClassId);
    Py_DECREF(self->pyParams);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThingDiscoveryInfo_paramValue(PyThingDiscoveryInfo *self, PyObject *args)
{
    char *paramTypeIdStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &paramTypeIdStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in paramValue call. Expected: paramValue(paramTypeId)");
        return nullptr;
    }

    ParamTypeId paramTypeId = ParamTypeId(paramTypeIdStr);
    for (int i = 0; i < PyTuple_Size(self->pyParams); i++) {
        PyParam *pyParam = reinterpret_cast<PyParam *>(PyTuple_GetItem(self->pyParams, i));
        // We're intentionally converting both ids to QUuid here in order to be more flexible with different UUID notations
        ParamTypeId ptid = ParamTypeId(PyUnicode_AsUTF8AndSize(pyParam->pyParamTypeId, nullptr));
        if (ptid == paramTypeId) {
            Py_INCREF(pyParam->pyValue);
            return pyParam->pyValue;
        }
    }
    qCWarning(dcPythonIntegrations()) << "No such ParamTypeId in discovery params" << paramTypeId;
    Py_RETURN_NONE;
}

static PyObject *PyThingDiscoveryInfo_finish(PyThingDiscoveryInfo *self, PyObject *args)
{
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "i|s", &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(ThingError, message = \"\"");
        return nullptr;
    }

    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }
    Py_RETURN_NONE;
}

static PyObject *PyThingDiscoveryInfo_addDescriptor(PyThingDiscoveryInfo *self, PyObject *args)
{
    PyObject *pyObj = nullptr;

    if (!PyArg_ParseTuple(args, "O", &pyObj)) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument to ThingDiscoveryInfo.addDescriptor(). Not a ThingDescriptor.");
        return nullptr;
    }
    if (pyObj->ob_type != &PyThingDescriptorType) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument to ThingDiscoveryInfo.addDescriptor(). Not a ThingDescriptor.");
        return nullptr;
    }
    PyThingDescriptor *pyDescriptor = (PyThingDescriptor *) pyObj;

    ThingClassId thingClassId;
    if (pyDescriptor->pyThingClassId) {
        thingClassId = ThingClassId(PyUnicode_AsUTF8(pyDescriptor->pyThingClassId));
    }
    ThingId parentId;
    if (pyDescriptor->pyParentId) {
        parentId = ThingId(PyUnicode_AsUTF8(pyDescriptor->pyParentId));
    }
    QString name;
    if (pyDescriptor->pyName) {
        name = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyName));
    }
    QString description;
    if (pyDescriptor->pyDescription) {
        description = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyDescription));
    }

    ThingDescriptor descriptor(thingClassId, name, description, parentId);
    if (pyDescriptor->pyThingId) {
        descriptor.setThingId(ThingId(QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyThingId))));
    }

    if (pyDescriptor->pyParams) {
        descriptor.setParams(PyParams_ToParamList(pyDescriptor->pyParams));
    }

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "addThingDescriptor", Qt::QueuedConnection, Q_ARG(ThingDescriptor, descriptor));
    }

    Py_RETURN_NONE;
}

static PyMemberDef PyThingDiscoveryInfo_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDiscoveryInfo, pyThingClassId), READONLY, "The ThingClassId this discovery is for."},
    {"params", T_OBJECT_EX, offsetof(PyThingDiscoveryInfo, pyParams), READONLY, "The params for this discovery"},
    {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static PyMethodDef PyThingDiscoveryInfo_methods[] = {
    {"paramValue", (PyCFunction) PyThingDiscoveryInfo_paramValue, METH_VARARGS, "Get a discovery param value"},
    {"addDescriptor", (PyCFunction) PyThingDiscoveryInfo_addDescriptor, METH_VARARGS, "Add a new descriptor to the discovery"},
    {"finish", (PyCFunction) PyThingDiscoveryInfo_finish, METH_VARARGS, "Finish a discovery"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingDiscoveryInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.ThingDiscoveryInfo", /* tp_name */
    sizeof(PyThingDiscoveryInfo),                              /* tp_basicsize */
    0,                                                         /* tp_itemsize */
    (destructor) PyThingDiscoveryInfo_dealloc,                 /* tp_dealloc */
};

static void registerThingDiscoveryInfoType(PyObject *module)
{
    PyThingDiscoveryInfoType.tp_new = (newfunc) PyThingDiscoveryInfo_new;
    PyThingDiscoveryInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingDiscoveryInfoType.tp_methods = PyThingDiscoveryInfo_methods;
    PyThingDiscoveryInfoType.tp_members = PyThingDiscoveryInfo_members;
    PyThingDiscoveryInfoType.tp_doc = "The ThingDiscoveryInfo is used to perform discoveries of things.";

    if (PyType_Ready(&PyThingDiscoveryInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDiscoveryInfo", (PyObject *) &PyThingDiscoveryInfoType);
}

#pragma GCC diagnostic pop

#endif // PYTHINGDISCOVERYINFO_H
