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

#ifndef PYBROWSERACTIONINFO_H
#define PYBROWSERACTIONINFO_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/browseractioninfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 *
 * When using this, make sure to call PyBrowserActionInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The BrowserActionInfo class is not threadsafe and self->info is owned by nymeas main thread.
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
    BrowserActionInfo* info;
    PyThing *pyThing;
    PyObject *pyItemId;
} PyBrowserActionInfo;


static PyObject* PyBrowserActionInfo_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyBrowserActionInfo *self = (PyBrowserActionInfo*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyBrowserActionInfo";
    return (PyObject*)self;
}

void PyBrowserActionInfo_setInfo(PyBrowserActionInfo *self, BrowserActionInfo *info, PyThing *pyThing)
{
    self->info = info;
    self->pyThing = pyThing;
    Py_INCREF(pyThing);
    self->pyItemId = PyUnicode_FromString(info->browserAction().itemId().toUtf8());
}

static void PyBrowserActionInfo_dealloc(PyBrowserActionInfo * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyBrowserActionInfo";
    Py_DECREF(self->pyThing);
    Py_DECREF(self->pyItemId);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyBrowserActionInfo_finish(PyBrowserActionInfo* self, PyObject* args) {
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

static PyMemberDef PyBrowserActionInfo_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyBrowserActionInfo, pyThing), 0, "Thing this action is for"},
    {"itemId", T_OBJECT_EX, offsetof(PyBrowserActionInfo, pyItemId), 0, "The browser item id to be executed"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyBrowserActionInfo_methods[] = {
    { "finish", (PyCFunction)PyBrowserActionInfo_finish, METH_VARARGS, "finish an action" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyBrowserActionInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.BrowserActionInfo",    /* tp_name */
    sizeof(PyBrowserActionInfo),  /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor)PyBrowserActionInfo_dealloc, /* tp_dealloc */
};

static void registerBrowserActionInfoType(PyObject *module)
{
    PyBrowserActionInfoType.tp_new = (newfunc)PyBrowserActionInfo_new;
    PyBrowserActionInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyBrowserActionInfoType.tp_methods = PyBrowserActionInfo_methods;
    PyBrowserActionInfoType.tp_members = PyBrowserActionInfo_members;
    PyBrowserActionInfoType.tp_doc = "The BrowserActionInfo is used to execute browser items";

    if (PyType_Ready(&PyBrowserActionInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "BrowserActionInfo", (PyObject *)&PyBrowserActionInfoType);
}


#pragma GCC diagnostic pop

#endif // PYBROWSERACTIONINFO_H
