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

#ifndef PYTHINGACTIONINFO_H
#define PYTHINGACTIONINFO_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/thingactioninfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 *
 * When using this, make sure to call PyThingActionInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingActionInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the setup and destroyed it but the PyThingSetupInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */

typedef struct {
    PyObject_HEAD
    ThingActionInfo* info;
    PyThing *pyThing;
    PyObject *pyActionTypeId;
    PyObject *pyParams;
} PyThingActionInfo;


static PyObject* PyThingActionInfo_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyThingActionInfo *self = (PyThingActionInfo*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyThingActionInfo";
    return (PyObject*)self;
}

void PyThingActionInfo_setInfo(PyThingActionInfo *self, ThingActionInfo *info, PyThing *pyThing)
{
    self->info = info;
    self->pyThing = pyThing;
    Py_INCREF(pyThing);
    self->pyActionTypeId = PyUnicode_FromString(info->action().actionTypeId().toString().toUtf8());
    self->pyParams = PyParams_FromParamList(info->action().params());
}


static void PyThingActionInfo_dealloc(PyThingActionInfo * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThingActionInfo";
    Py_DECREF(self->pyThing);
    Py_DECREF(self->pyActionTypeId);
    Py_DECREF(self->pyParams);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingActionInfo_finish(PyThingActionInfo* self, PyObject* args) {
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "i|s", &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(ThingError, message = \"\")");
        return nullptr;
    }

    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }

    Py_RETURN_NONE;
}

static PyObject * PyThingActionInfo_paramValue(PyThingActionInfo* self, PyObject* args) {
    char *paramTypeIdStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &paramTypeIdStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in paramValue call. Expected: paramValue(paramTypeId)");
        return nullptr;
    }

    ParamTypeId paramTypeId = ParamTypeId(paramTypeIdStr);
    for (int i = 0; i < PyTuple_Size(self->pyParams); i++) {
        PyParam *pyParam = reinterpret_cast<PyParam*>(PyTuple_GetItem(self->pyParams, i));
        // We're intentionally converting both ids to QUuid here in order to be more flexible with different UUID notations
        ParamTypeId ptid = ParamTypeId(PyUnicode_AsUTF8AndSize(pyParam->pyParamTypeId, nullptr));
        if (ptid == paramTypeId) {
            Py_INCREF(pyParam->pyValue);
            return pyParam->pyValue;
        }
    }
    qCWarning(dcPythonIntegrations()) << "No such ParamTypeId in action params" << paramTypeId;
    Py_RETURN_NONE;
}

static PyMemberDef PyThingActionInfo_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyThingActionInfo, pyThing), 0, "Thing this action is for"},
    {"actionTypeId", T_OBJECT_EX, offsetof(PyThingActionInfo, pyActionTypeId), 0, "The action type id for this action"},
    {"params", T_OBJECT_EX, offsetof(PyThingActionInfo, pyParams), 0, "The params for this action"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyThingActionInfo_methods[] = {
    { "finish", (PyCFunction)PyThingActionInfo_finish,    METH_VARARGS,       "finish an action" },
    { "paramValue", (PyCFunction)PyThingActionInfo_paramValue, METH_VARARGS, "Get an actions param value"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingActionInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingActionInfo",    /* tp_name */
    sizeof(PyThingActionInfo),  /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThingActionInfo_dealloc, /* tp_dealloc */
};

static void registerThingActionInfoType(PyObject *module)
{
    PyThingActionInfoType.tp_new = (newfunc)PyThingActionInfo_new;
    PyThingActionInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingActionInfoType.tp_methods = PyThingActionInfo_methods;
    PyThingActionInfoType.tp_members = PyThingActionInfo_members;
    PyThingActionInfoType.tp_doc = "The ThingActionInfo is used to dispatch actions to things";

    if (PyType_Ready(&PyThingActionInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingActionInfo", (PyObject *)&PyThingActionInfoType);
}


#pragma GCC diagnostic pop

#endif // PYTHINGACTIONINFO_H
