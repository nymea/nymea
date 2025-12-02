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

#ifndef PYBROWSERITEM_H
#define PYBROWSERITEM_H

#include <Python.h>
#include "structmember.h"

#include <QMetaEnum>

#include "types/browseritem.h"
#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


typedef struct {
    PyObject_HEAD
    PyObject* pyId;
    PyObject* pyDisplayName;
    PyObject* pyDescription;
    PyObject* pyThumbnail;
    bool browsable = false;
    bool executable = false;
    bool disabled = false;
    int icon = (int)BrowserItem::BrowserIconNone;
} PyBrowserItem;


static PyMemberDef PyBrowserItem_members[] = {
    {"id", T_OBJECT_EX, offsetof(PyBrowserItem, pyId), 0, "BrowserItem id"},
    {"displayName", T_OBJECT_EX, offsetof(PyBrowserItem, pyDisplayName), 0, "The name of this item"},
    {"description", T_OBJECT_EX, offsetof(PyBrowserItem, pyDescription), 0, "The description of this item"},
    {"tumbnail", T_OBJECT_EX, offsetof(PyBrowserItem, pyThumbnail), 0, "An URL pointing to the thumbnail"},
    {"browsable", T_OBJECT_EX, offsetof(PyBrowserItem, browsable), 0, "A boolean if this item can be browsed (e.g. a folder)"},
    {"executable", T_OBJECT_EX, offsetof(PyBrowserItem, executable), 0, "A boolean if this item can be launched"},
    {"disabled", T_OBJECT_EX, offsetof(PyBrowserItem, disabled), 0, "A boolean if this item is disabled"},
    {"icon", T_OBJECT_EX, offsetof(PyBrowserItem, icon), 0, "The icon to be used"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static int PyBrowserItem_init(PyBrowserItem *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"id", "displayName", "description", "thumbnail", "browsable", "executable", "disabled", "icon", nullptr};
    PyObject *id = nullptr, *displayName = nullptr, *description = nullptr, *thumbnail = nullptr;
    bool browsable = false, executable = false, disabled = false;
    int icon = (int)BrowserItem::BrowserIconNone;

    qCDebug(dcPythonIntegrations()) << "+++ PyBrowserItem";
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOOObbbi", kwlist, &id, &displayName, &description, &thumbnail, &browsable, &executable, &disabled, &icon))
        return -1;

    if (id) {
        Py_INCREF(id);
        self->pyId = id;
    }
    if (displayName) {
        Py_INCREF(displayName);
        self->pyDisplayName = displayName;
    }
    if (description) {
        Py_INCREF(description);
        self->pyDescription = description;
    }
    if (thumbnail) {
        Py_INCREF(thumbnail);
        self->pyThumbnail = thumbnail;
    }
    self->browsable = browsable;
    self->executable = executable;
    self->disabled = disabled;
    self->icon = icon;
    return 0;
}

static void PyBrowserItem_dealloc(PyBrowserItem* self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyBrowserItem";
    Py_XDECREF(self->pyId);
    Py_XDECREF(self->pyDisplayName);
    Py_XDECREF(self->pyDescription);
    Py_XDECREF(self->pyThumbnail);
    Py_TYPE(self)->tp_free(self);
}

static PyTypeObject PyBrowserItemType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.BrowserItem",       /* tp_name */
    sizeof(PyBrowserItem),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyBrowserItem_dealloc, /* tp_dealloc */
};



static void registerBrowserItemType(PyObject *module)
{
    PyBrowserItemType.tp_new = PyType_GenericNew;
    PyBrowserItemType.tp_members = PyBrowserItem_members;
    PyBrowserItemType.tp_init = reinterpret_cast<initproc>(PyBrowserItem_init);
    PyBrowserItemType.tp_doc = "BrowserItems are used to return entries in a things browser to nymea.";
    PyBrowserItemType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyBrowserItemType) < 0) {
        return;
    }
    PyModule_AddObject(module, "BrowserItem", reinterpret_cast<PyObject*>(&PyBrowserItemType));

    QMetaEnum browserIconEnum = QMetaEnum::fromType<BrowserItem::BrowserIcon>();
    for (int i = 0; i < browserIconEnum.keyCount(); i++) {
        PyModule_AddObject(module, browserIconEnum.key(i), PyLong_FromLong(browserIconEnum.value(i)));
    }
}

#pragma GCC diagnostic pop

#endif // PYBROWSERITEM_H
