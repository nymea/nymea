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

#ifndef PYBROWSERESULT_H
#define PYBROWSERESULT_H

#include "structmember.h"
#include <Python.h>

#include "pything.h"

#include "integrations/browseresult.h"
#include "pybrowseritem.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 *
 * When using this, make sure to call PyBrowseResult_setBrowseResult() while holding the GIL to initialize
 * stuff after constructing it. Also set broeseResult to nullptr while holding the GIL when the browseResult object vanishes.
 *
 * The BrowseResult class is not threadsafe and self->browseResult is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the setup and destroyed it but the PyThingSetupInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the BrowseResults properties here.
 *
 */

typedef struct
{
    PyObject_HEAD BrowseResult *browseResult;
    PyThing *pyThing;
    PyObject *pyItemId;
    PyObject *pyLocale;
} PyBrowseResult;

static PyObject *PyBrowseResult_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
    PyBrowseResult *self = (PyBrowseResult *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyBrowseResult";
    return (PyObject *) self;
}

void PyBrowseResult_setBrowseResult(PyBrowseResult *self, BrowseResult *browseResult, PyThing *pyThing)
{
    self->browseResult = browseResult;
    self->pyThing = pyThing;
    Py_INCREF(pyThing);
    self->pyItemId = PyUnicode_FromString(browseResult->itemId().toUtf8());
    self->pyLocale = PyUnicode_FromString(browseResult->locale().name().toUtf8());
}

static void PyBrowseResult_dealloc(PyBrowseResult *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyBrowseResult";
    Py_DECREF(self->pyThing);
    Py_DECREF(self->pyItemId);
    Py_DECREF(self->pyLocale);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyBrowseResult_addItem(PyBrowseResult *self, PyObject *args)
{
    Q_UNUSED(self)
    PyObject *pyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &pyObj)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in addItem call. Expected: addItem(BrowserItem)");
        return nullptr;
    }
    if (pyObj->ob_type != &PyBrowserItemType) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument to BrowseResult.addItem(BrowserItem). Not a BrowserItem.");
        return nullptr;
    }
    PyBrowserItem *pyBrowserItem = (PyBrowserItem *) pyObj;

    QString id;
    if (pyBrowserItem->pyId) {
        id = QString::fromUtf8(PyUnicode_AsUTF8(pyBrowserItem->pyId));
    }
    QString displayName;
    if (pyBrowserItem->pyDisplayName) {
        displayName = QString::fromUtf8(PyUnicode_AsUTF8(pyBrowserItem->pyDisplayName));
    }

    BrowserItem browserItem(id, displayName);
    if (pyBrowserItem->pyDescription) {
        browserItem.setDescription(QString::fromUtf8(PyUnicode_AsUTF8(pyBrowserItem->pyDescription)));
    }

    if (pyBrowserItem->pyThumbnail) {
        browserItem.setThumbnail(QString::fromUtf8(PyUnicode_AsUTF8(pyBrowserItem->pyThumbnail)));
    }

    browserItem.setBrowsable(pyBrowserItem->browsable);
    browserItem.setExecutable(pyBrowserItem->executable);
    browserItem.setDisabled(pyBrowserItem->disabled);
    browserItem.setIcon(static_cast<BrowserItem::BrowserIcon>(pyBrowserItem->icon));

    if (self->browseResult) {
        QMetaObject::invokeMethod(self->browseResult, "addItem", Qt::QueuedConnection, Q_ARG(BrowserItem, browserItem));
    }
    Py_RETURN_NONE;
}

static PyObject *PyBrowseResult_finish(PyBrowseResult *self, PyObject *args)
{
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "i|s", &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(ThingError, message = \"\")");
        return nullptr;
    }

    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    if (self->browseResult) {
        QMetaObject::invokeMethod(self->browseResult, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }

    Py_RETURN_NONE;
}

static PyMemberDef PyBrowseResult_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyBrowseResult, pyThing), 0, "Thing this browse request is for"},
    {"itemId", T_OBJECT_EX, offsetof(PyBrowseResult, pyItemId), 0, "The itemId of the item that should be browsed. Empty if the root item is requested"},
    {"locale", T_OBJECT_EX, offsetof(PyBrowseResult, pyLocale), 0, "The locale strings should be translated to."},
    {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static PyMethodDef PyBrowseResult_methods[] = {
    {"addItem", (PyCFunction) PyBrowseResult_addItem, METH_VARARGS, "Add a browser item to the result"},
    {"finish", (PyCFunction) PyBrowseResult_finish, METH_VARARGS, "Finish a browse request"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyBrowseResultType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.BrowseResult", /* tp_name */
    sizeof(PyBrowseResult),                              /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor) PyBrowseResult_dealloc,                 /* tp_dealloc */
};

static void registerBrowseResultType(PyObject *module)
{
    PyBrowseResultType.tp_new = (newfunc) PyBrowseResult_new;
    PyBrowseResultType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyBrowseResultType.tp_methods = PyBrowseResult_methods;
    PyBrowseResultType.tp_members = PyBrowseResult_members;
    PyBrowseResultType.tp_doc = "The BrowseResult is used fetch browser entries from things";

    if (PyType_Ready(&PyBrowseResultType) < 0) {
        return;
    }
    PyModule_AddObject(module, "BrowseResult", (PyObject *) &PyBrowseResultType);
}

#pragma GCC diagnostic pop

#endif // PYBROWSERESULT_H
