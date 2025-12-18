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

#ifndef PYTHINGPAIRINGINFO_H
#define PYTHINGPAIRINGINFO_H

#include "structmember.h"
#include <Python.h>

#include "pyparam.h"

#include "integrations/thingpairinginfo.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMutex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 * When using this, make sure to call PyThingPairingInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingPairingInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the pairing step and destroyed it but the PyThingPairingInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */

typedef struct
{
    PyObject_HEAD ThingPairingInfo *info;
    PyObject *pyTransactionId = nullptr;
    PyObject *pyThingClassId = nullptr;
    PyObject *pyThingId = nullptr;
    PyObject *pyThingName = nullptr;
    PyObject *pyParentId = nullptr;
    PyObject *pyParams = nullptr;
    PyObject *pyOAuthUrl = nullptr;
} PyThingPairingInfo;

static PyMemberDef PyThingPairingInfo_members[] = {
    {"transactionId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyTransactionId), READONLY, "The transaction id for this pairing procedure."},
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingClassId), READONLY, "The ThingClassId for the thing to be set up."},
    {"thingId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingId), READONLY, "The ThingId for the thing to be set up."},
    {"thingName", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingName), READONLY, "The ThingId for the thing to be set up."},
    {"parentId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyParentId), READONLY, "The ThingId for the parent of the thing to be set up."},
    {"params", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyParams), READONLY, "The params for the thing to be set up."},
    {"oAuthUrl", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyOAuthUrl), 0, "An OAuth url if required for the pairing."},
    {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static int PyThingPairingInfo_init(PyThingPairingInfo * /*self*/, PyObject * /*args*/, PyObject * /*kwds*/)
{
    qCDebug(dcPythonIntegrations()) << "+++ ThingPairingInfo";
    return 0;
}

void PyThingPairingInfo_setInfo(PyThingPairingInfo *self, ThingPairingInfo *info)
{
    self->info = info;
    self->pyTransactionId = PyUnicode_FromString(info->transactionId().toString().toUtf8());
    self->pyThingClassId = PyUnicode_FromString(info->thingClassId().toString().toUtf8());
    self->pyThingId = PyUnicode_FromString(info->thingId().toString().toUtf8());
    self->pyThingName = PyUnicode_FromString(info->thingName().toUtf8());
    self->pyParentId = PyUnicode_FromString(info->parentId().toString().toUtf8());
    self->pyParams = PyParams_FromParamList(info->params());
}

static void PyThingPairingInfo_dealloc(PyThingPairingInfo *self)
{
    qCDebug(dcPythonIntegrations()) << "--- ThingPairingInfo";
    Py_XDECREF(self->pyTransactionId);
    Py_XDECREF(self->pyThingClassId);
    Py_XDECREF(self->pyThingId);
    Py_XDECREF(self->pyThingName);
    Py_XDECREF(self->pyParentId);
    Py_XDECREF(self->pyParams);
    Py_XDECREF(self->pyOAuthUrl);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThingPairingInfo_paramValue(PyThingPairingInfo *self, PyObject *args)
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
    qCWarning(dcPythonIntegrations()) << "No such ParamTypeId in thing params" << paramTypeId;
    Py_RETURN_NONE;
}

static PyObject *PyThingPairingInfo_finish(PyThingPairingInfo *self, PyObject *args)
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
        if (self->pyOAuthUrl) {
            QString oAuthUrl = QString::fromUtf8(PyUnicode_AsUTF8AndSize(self->pyOAuthUrl, nullptr));
            QMetaObject::invokeMethod(self->info, "setOAuthUrl", Qt::QueuedConnection, Q_ARG(QUrl, oAuthUrl));
        }
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }
    Py_RETURN_NONE;
}

static PyMethodDef PyThingPairingInfo_methods[] = {
    {"paramValue", (PyCFunction) PyThingPairingInfo_paramValue, METH_VARARGS, "Get a param value for the thing to be paired"},
    {"finish", (PyCFunction) PyThingPairingInfo_finish, METH_VARARGS, "Finish a discovery"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingPairingInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.ThingPairingInfo", /* tp_name */
    sizeof(PyThingPairingInfo),                              /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor) PyThingPairingInfo_dealloc,                 /* tp_dealloc */
};

static void registerThingPairingInfoType(PyObject *module)
{
    PyThingPairingInfoType.tp_new = PyType_GenericNew;
    PyThingPairingInfoType.tp_init = (initproc) PyThingPairingInfo_init;
    PyThingPairingInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingPairingInfoType.tp_methods = PyThingPairingInfo_methods;
    PyThingPairingInfoType.tp_members = PyThingPairingInfo_members;
    PyThingPairingInfoType.tp_doc = "The ThingPairingInfo is used to aithenticate with a thing.";

    if (PyType_Ready(&PyThingPairingInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingPairingInfo", (PyObject *) &PyThingPairingInfoType);
}

#pragma GCC diagnostic pop

#endif // PYTHINGPAIRINGINFO_H
