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

#ifndef PYTHINGSETUPINFO_H
#define PYTHINGSETUPINFO_H

#include "structmember.h"
#include <Python.h>

#include "pything.h"

#include "integrations/thingsetupinfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 *
 * When using this, make sure to call PyThingSetupInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingSetupInfo class is not threadsafe and self->info is owned by nymeas main thread.
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

typedef struct
{
    PyObject_HEAD ThingSetupInfo *info;
    PyThing *pyThing;
} PyThingSetupInfo;

static PyObject *PyThingSetupInfo_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyThingSetupInfo *self = (PyThingSetupInfo *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyThingSetupInfo";

    static char *kwlist[] = {"thing", nullptr};
    PyObject *pyThing = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &pyThing)) {
        PyErr_Print();
        PyErr_SetString(PyExc_ValueError, "Invalid arguments.");
        return nullptr;
    }

    self->pyThing = (PyThing *) pyThing;
    Py_INCREF(self->pyThing);

    return (PyObject *) self;
}

static void PyThingSetupInfo_dealloc(PyThingSetupInfo *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThingSetupInfo";
    Py_DECREF(self->pyThing);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThingSetupInfo_finish(PyThingSetupInfo *self, PyObject *args)
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

static PyMemberDef PyThingSetupInfo_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyThingSetupInfo, pyThing), 0, "Thing being setup in this setup transaction"}, {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static PyMethodDef PyThingSetupInfo_methods[] = {
    {"finish", (PyCFunction) PyThingSetupInfo_finish, METH_VARARGS, "finish a setup"}, {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingSetupInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.ThingSetupInfo", /* tp_name */
    sizeof(PyThingSetupInfo),                              /* tp_basicsize */
    0,                                                     /* tp_itemsize */
    (destructor) PyThingSetupInfo_dealloc,                 /* tp_dealloc */
};

static void registerThingSetupInfoType(PyObject *module)
{
    PyThingSetupInfoType.tp_new = (newfunc) PyThingSetupInfo_new;
    PyThingSetupInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingSetupInfoType.tp_methods = PyThingSetupInfo_methods;
    PyThingSetupInfoType.tp_members = PyThingSetupInfo_members;
    PyThingSetupInfoType.tp_doc = "The ThingSetupInfo is used to set up a thing.";

    if (PyType_Ready(&PyThingSetupInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingSetupInfo", (PyObject *) &PyThingSetupInfoType);
}

#pragma GCC diagnostic pop

#endif // PYTHINGSETUPINFO_H
