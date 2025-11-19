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

#ifndef PYBROWSERITEMRESULT_H
#define PYBROWSERITEMRESULT_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/browseritemresult.h"
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

typedef struct {
    PyObject_HEAD
    BrowserItemResult* browserItemResult;
    PyThing *pyThing;
    PyObject *pyItemId;
    PyObject *pyLocale;
} PyBrowserItemResult;


static PyObject* PyBrowserItemResult_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyBrowserItemResult *self = (PyBrowserItemResult*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyBrowserItemResult";
    return (PyObject*)self;
}

void PyBrowserItemResult_setBrowserItemResult(PyBrowserItemResult *self, BrowserItemResult *browserItemResult, PyThing *pyThing)
{
    self->browserItemResult = browserItemResult;
    self->pyThing = pyThing;
    Py_INCREF(pyThing);
    self->pyItemId = PyUnicode_FromString(browserItemResult->itemId().toUtf8());
    self->pyLocale = PyUnicode_FromString(browserItemResult->locale().name().toUtf8());
}


static void PyBrowserItemResult_dealloc(PyBrowserItemResult* self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyBrowserItemResult";
    Py_DECREF(self->pyThing);
    Py_DECREF(self->pyItemId);
    Py_DECREF(self->pyLocale);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* PyBrowserItemResult_finish(PyBrowserItemResult* self, PyObject* args) {
    PyObject *pyObj;
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "|Ois", &pyObj, &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(BrowserItem) or finish(ThingError, message = \"\")");
        return nullptr;
    }

    if (pyObj != nullptr) {
        if (pyObj->ob_type != &PyBrowserItemType) {
            PyErr_SetString(PyExc_ValueError, "Invalid argument to BrowserItemResult.finish(BrowserItem). Not a BrowserItem.");
            return nullptr;
        }
        PyBrowserItem *pyBrowserItem = (PyBrowserItem*)pyObj;
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

        if (self->browserItemResult) {
            QMetaObject::invokeMethod(self->browserItemResult, "finish", Qt::QueuedConnection, Q_ARG(BrowserItem, browserItem));
        }
        Py_RETURN_NONE;
    }


    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    if (self->browserItemResult) {
        QMetaObject::invokeMethod(self->browserItemResult, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }

    Py_RETURN_NONE;
}


static PyMemberDef PyBrowserItemResult_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyBrowserItemResult, pyThing), 0, "Thing this browse request is for"},
    {"itemId", T_OBJECT_EX, offsetof(PyBrowserItemResult, pyItemId), 0, "The itemId of the item to be returned."},
    {"locale", T_OBJECT_EX, offsetof(PyBrowserItemResult, pyLocale), 0, "The locale strings should be translated to."},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyBrowserItemResult_methods[] = {
    { "finish", (PyCFunction)PyBrowserItemResult_finish, METH_VARARGS, "Finish a browser item request" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyBrowserItemResultType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.BrowserItemResult",       /* tp_name */
    sizeof(PyBrowserItemResult),     /* tp_basicsize */
    0,                               /* tp_itemsize */
    (destructor)PyBrowserItemResult_dealloc, /* tp_dealloc */
};

static void registerBrowserItemResultType(PyObject *module)
{
    PyBrowserItemResultType.tp_new = (newfunc)PyBrowserItemResult_new;
    PyBrowserItemResultType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyBrowserItemResultType.tp_methods = PyBrowserItemResult_methods;
    PyBrowserItemResultType.tp_members = PyBrowserItemResult_members;
    PyBrowserItemResultType.tp_doc = "The BrowserItemResult is used fetch an individual entry from the thing browser";

    if (PyType_Ready(&PyBrowserItemResultType) < 0) {
        return;
    }
    PyModule_AddObject(module, "BrowserItemResult", (PyObject *)&PyBrowserItemResultType);
}


#pragma GCC diagnostic pop

#endif // PYBROWSERITEMRESULT_H
