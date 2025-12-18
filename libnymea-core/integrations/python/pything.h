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

#ifndef PYTHING_H
#define PYTHING_H

#include "structmember.h"
#include <Python.h>

#include "pyparam.h"

#include "integrations/thing.h"
#include "loggingcategories.h"

#include <QMetaEnum>
#include <QPointer>
#include <QThread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/* Note:
 * When using this, make sure to call PyThing_setThing() while holding the GIL to initialize
 * stuff after constructing it.
 *
 * The Thing class is not threadsafe and self->thing is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to hold the GIL whenver accessing the pointer value for invoking stuff.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */

typedef struct _thing
{
    PyObject_HEAD Thing *thing = nullptr; // the actual thing in nymea (not thread-safe!)
    ThingClass *thingClass = nullptr;     // A copy of the thing class. This is owned by the python thread
    PyObject *pyId = nullptr;
    PyObject *pyThingClassId = nullptr;
    PyObject *pyParentId = nullptr;
    PyObject *pyName = nullptr;
    PyObject *pyParams = nullptr;
    PyObject *pySettings = nullptr;
    PyObject *pyNameChangedHandler = nullptr;
    PyObject *pySettingChangedHandler = nullptr;
    PyObject *pyStates = nullptr;         // A copy of the things states
    PyThreadState *threadState = nullptr; // The python threadstate this thing belongs to
} PyThing;

static PyObject *PyThing_new(PyTypeObject *type, PyObject * /*args*/, PyObject * /*kwds*/)
{
    PyThing *self = (PyThing *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyThing" << self;

    return (PyObject *) self;
}

static void PyThing_setThing(PyThing *self, Thing *thing, PyThreadState *threadState)
{
    self->thing = thing;
    self->threadState = threadState;

    // Creating a copy because we cannot access the actual thing from the python thread
    self->thingClass = new ThingClass(thing->thingClass());

    self->pyId = PyUnicode_FromString(self->thing->id().toString().toUtf8().data());
    self->pyThingClassId = PyUnicode_FromString(self->thing->thingClassId().toString().toUtf8().data());
    self->pyParentId = PyUnicode_FromString(self->thing->parentId().toString().toUtf8().data());
    self->pyName = PyUnicode_FromString(self->thing->name().toUtf8().data());
    self->pyParams = PyParams_FromParamList(self->thing->params());
    self->pySettings = PyParams_FromParamList(self->thing->settings());

    self->pyStates = PyList_New(thing->states().count());
    for (int i = 0; i < thing->states().count(); i++) {
        State state = thing->states().at(i);
        PyObject *pyState = Py_BuildValue("{s:s, s:O}", "stateTypeId", state.stateTypeId().toString().toUtf8().data(), "value", QVariantToPyObject(state.value()));
        PyList_SetItem(self->pyStates, i, pyState);
    }

    // Connects signal handlers from the Thing to sync stuff over to the pyThing in a
    // thread-safe manner.

    // Those lambdas Will be executed in the main thread context. This means we
    // can access self->thing, but need to hold the GIL for interacting with python
    QObject::connect(thing, &Thing::nameChanged, [=]() {
        PyEval_RestoreThread(self->threadState);
        Py_XDECREF(self->pyName);
        self->pyName = PyUnicode_FromString(self->thing->name().toUtf8().data());
        if (self->pyNameChangedHandler) {
            PyObject *ret = PyObject_CallFunctionObjArgs(self->pyNameChangedHandler, self, nullptr);
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            Py_XDECREF(ret);
        }
        PyEval_ReleaseThread(self->threadState);
    });

    QObject::connect(thing, &Thing::settingChanged, [=](const ParamTypeId &paramTypeId, const QVariant &value) {
        PyEval_RestoreThread(self->threadState);
        Py_XDECREF(self->pySettings);
        self->pySettings = PyParams_FromParamList(self->thing->settings());
        if (self->pySettingChangedHandler) {
            PyObject *ret = PyObject_CallFunctionObjArgs(self->pySettingChangedHandler,
                                                         self,
                                                         PyUnicode_FromString(paramTypeId.toString().toUtf8().data()),
                                                         QVariantToPyObject(value),
                                                         nullptr);
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            Py_XDECREF(ret);
        }
        PyEval_ReleaseThread(self->threadState);
    });

    QObject::connect(thing, &Thing::stateValueChanged, [=](const StateTypeId &stateTypeId, const QVariant &value) {
        PyEval_RestoreThread(self->threadState);
        for (int i = 0; i < PyList_Size(self->pyStates); i++) {
            PyObject *pyState = PyList_GetItem(self->pyStates, i);
            PyObject *pyStateTypeId = PyDict_GetItemString(pyState, "stateTypeId");
            StateTypeId stid = StateTypeId(PyUnicode_AsUTF8AndSize(pyStateTypeId, nullptr));
            if (stid == stateTypeId) {
                pyState = Py_BuildValue("{s:s, s:O}", "stateTypeId", stateTypeId.toString().toUtf8().data(), "value", QVariantToPyObject(value));
                PyList_SetItem(self->pyStates, i, pyState);
                break;
            }
        }
        PyEval_ReleaseThread(self->threadState);
    });
}

static void PyThing_dealloc(PyThing *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThing" << self;
    Py_XDECREF(self->pyId);
    Py_XDECREF(self->pyThingClassId);
    Py_XDECREF(self->pyParentId);
    Py_XDECREF(self->pyName);
    Py_XDECREF(self->pyParams);
    Py_XDECREF(self->pySettings);
    Py_XDECREF(self->pyStates);
    Py_XDECREF(self->pyNameChangedHandler);
    Py_XDECREF(self->pySettingChangedHandler);
    delete self->thingClass;
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThing_getName(PyThing *self, void * /*closure*/)
{
    Py_INCREF(self->pyName);
    return self->pyName;
}

static PyObject *PyThing_getId(PyThing *self, void * /*closure*/)
{
    Py_INCREF(self->pyId);
    return self->pyId;
}

static PyObject *PyThing_getThingClassId(PyThing *self, void * /*closure*/)
{
    Py_INCREF(self->pyThingClassId);
    return self->pyThingClassId;
}

static PyObject *PyThing_getParentId(PyThing *self, void * /*closure*/)
{
    Py_INCREF(self->pyParentId);
    return self->pyParentId;
}

static int PyThing_setName(PyThing *self, PyObject *value, void * /*closure*/)
{
    QString name = QString(PyUnicode_AsUTF8(value));
    if (!self->thing) {
        return -1;
    }
    QMetaObject::invokeMethod(self->thing, "setName", Qt::QueuedConnection, Q_ARG(QString, name));
    return 0;
}

static PyObject *PyThing_paramValue(PyThing *self, PyObject *args)
{
    char *paramTypeIdStr = nullptr;

    if (!PyArg_ParseTuple(args, "s", &paramTypeIdStr)) {
        qCWarning(dcThingManager()) << "Error parsing parameters";
        return nullptr;
    }

    ParamTypeId paramTypeId = ParamTypeId(paramTypeIdStr);
    PyObject *iterator = PyObject_GetIter(self->pyParams);
    while (iterator) {
        PyObject *pyParam = PyIter_Next(iterator);
        if (!pyParam) {
            break;
        }

        Param param = PyParam_ToParam((PyParam *) pyParam);
        Py_DECREF(pyParam);

        if (param.paramTypeId() != paramTypeId) {
            continue;
        }

        Py_DECREF(iterator);

        return QVariantToPyObject(param.value());
    }

    Py_DECREF(iterator);
    qCWarning(dcPythonIntegrations()) << "No param for paramTypeId:" << paramTypeId;
    Py_RETURN_NONE;
}

static PyObject *PyThing_setting(PyThing *self, PyObject *args)
{
    char *paramTypeIdStr = nullptr;

    if (!PyArg_ParseTuple(args, "s", &paramTypeIdStr)) {
        qCWarning(dcThingManager()) << "Error parsing parameters";
        return nullptr;
    }

    ParamTypeId paramTypeId = ParamTypeId(paramTypeIdStr);
    PyObject *iterator = PyObject_GetIter(self->pySettings);
    while (iterator) {
        PyObject *pyParam = PyIter_Next(iterator);
        if (!pyParam) {
            break;
        }

        Param param = PyParam_ToParam((PyParam *) pyParam);
        Py_DECREF(pyParam);

        if (param.paramTypeId() != paramTypeId) {
            continue;
        }

        Py_DECREF(iterator);

        return QVariantToPyObject(param.value());
    }

    Py_DECREF(iterator);
    qCWarning(dcPythonIntegrations()) << "No setting for paramTypeId:" << paramTypeId;
    Py_RETURN_NONE;
}

static PyObject *PyThing_getSettings(PyThing *self, void * /*closure*/)
{
    Py_INCREF(self->pySettings);
    return self->pySettings;
}

static int PyThing_setSettings(PyThing * /*self*/, PyObject * /*value*/, void * /*closure*/)
{
    //    self->thing->setName(QString(PyUnicode_AsUTF8(value)));
    return 0;
}

static PyObject *PyThing_stateValue(PyThing *self, PyObject *args)
{
    char *stateTypeIdStr = nullptr;

    if (!PyArg_ParseTuple(args, "s", &stateTypeIdStr)) {
        PyErr_SetString(PyExc_ValueError, "Error parsing arguments. Signature is 's'");
        return nullptr;
    }

    StateTypeId stateTypeId = StateTypeId(stateTypeIdStr);

    for (int i = 0; i < PyList_Size(self->pyStates); i++) {
        PyObject *pyState = PyList_GetItem(self->pyStates, i);
        PyObject *pyStateTypeId = PyDict_GetItemString(pyState, "stateTypeId");
        StateTypeId stid = StateTypeId(PyUnicode_AsUTF8AndSize(pyStateTypeId, nullptr));
        if (stid == stateTypeId) {
            PyObject *value = PyDict_GetItemString(pyState, "value");
            Py_INCREF(value);
            return value;
        }
    }

    PyErr_SetString(PyExc_ValueError, QString("No state type %1 in thing class %2").arg(stateTypeId.toString()).arg(self->thingClass->name()).toUtf8());
    return nullptr;
}

static PyObject *PyThing_setStateValue(PyThing *self, PyObject *args)
{
    char *stateTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "sO", &stateTypeIdStr, &valueObj)) {
        PyErr_SetString(PyExc_ValueError, "Error parsing arguments. Signature is 'sO'");
        return nullptr;
    }

    StateTypeId stateTypeId = StateTypeId(stateTypeIdStr);
    QVariant value = PyObjectToQVariant(valueObj);

    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "setStateValue", Qt::QueuedConnection, Q_ARG(StateTypeId, stateTypeId), Q_ARG(QVariant, value));
    }

    Py_RETURN_NONE;
}

static PyObject *PyThing_emitEvent(PyThing *self, PyObject *args)
{
    char *eventTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "s|O", &eventTypeIdStr, &valueObj)) {
        PyErr_SetString(PyExc_TypeError, "Supplied arguments for emitEvent must be a ParamList");
        return nullptr;
    }
    if (qstrcmp(valueObj->ob_type->tp_name, "list") != 0) {
        PyErr_SetString(PyExc_TypeError, "Supplied arguments for emitEvent must be a ParamList");
        return nullptr;
    }

    EventTypeId eventTypeId = EventTypeId(eventTypeIdStr);
    EventType eventType = self->thingClass->eventTypes().findById(eventTypeId);
    if (!eventType.isValid()) {
        PyErr_SetString(PyExc_ValueError, QString("No event type %1 in thing class %2").arg(eventTypeId.toString()).arg(self->thingClass->name()).toUtf8());
        return nullptr;
    }

    ParamList params = PyParams_ToParamList(valueObj);

    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "emitEvent", Qt::QueuedConnection, Q_ARG(EventTypeId, eventTypeId), Q_ARG(ParamList, params));
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyThing_getset[] = {
    {"name", (getter) PyThing_getName, (setter) PyThing_setName, "Thing name", nullptr},
    {"id", (getter) PyThing_getId, 0, "ThingId", nullptr},
    {"thingClassId", (getter) PyThing_getThingClassId, 0, "ThingClassId", nullptr},
    {"parentId", (getter) PyThing_getParentId, 0, "Parent thing id", nullptr},
    {"settings", (getter) PyThing_getSettings, (setter) PyThing_setSettings, "Thing settings", nullptr},
    {nullptr, nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMethodDef PyThing_methods[] = {
    {"paramValue", (PyCFunction) PyThing_paramValue, METH_VARARGS, "Get a things param value by paramTypeId"},
    {"setting", (PyCFunction) PyThing_setting, METH_VARARGS, "Get a things setting value by paramTypeId"},
    {"stateValue", (PyCFunction) PyThing_stateValue, METH_VARARGS, "Get a things state value by stateTypeId"},
    {"setStateValue", (PyCFunction) PyThing_setStateValue, METH_VARARGS, "Set a certain things state value by stateTypeIp"},
    {"emitEvent", (PyCFunction) PyThing_emitEvent, METH_VARARGS, "Emits an event"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyThing_members[] = {
    {"params", T_OBJECT_EX, offsetof(PyThing, pyParams), READONLY, "Thing params"},
    {"nameChangedHandler", T_OBJECT_EX, offsetof(PyThing, pyNameChangedHandler), 0, "Set a callback for when the thing name changes"},
    {"settingChangedHandler", T_OBJECT_EX, offsetof(PyThing, pySettingChangedHandler), 0, "Set a callback for when a thing setting changes"},
    {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static PyTypeObject PyThingType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.Thing", /* tp_name */
    sizeof(PyThing),                              /* tp_basicsize */
    0,                                            /* tp_itemsize */
    (destructor) PyThing_dealloc,                 /* tp_dealloc */
};

static void registerThingType(PyObject *module)
{
    PyThingType.tp_new = (newfunc) PyThing_new;
    PyThingType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingType.tp_methods = PyThing_methods;
    PyThingType.tp_members = PyThing_members;
    PyThingType.tp_getset = PyThing_getset;
    PyThingType.tp_doc = "The Thing class represents a thing in nymea.";

    if (PyType_Ready(&PyThingType) < 0) {
        return;
    }
    PyModule_AddObject(module, "Thing", reinterpret_cast<PyObject *>(&PyThingType));

    QMetaEnum thingErrorEnum = QMetaEnum::fromType<Thing::ThingError>();
    for (int i = 0; i < thingErrorEnum.keyCount(); i++) {
        PyModule_AddObject(module, thingErrorEnum.key(i), PyLong_FromLong(thingErrorEnum.value(i)));
    }
}

#pragma GCC diagnostic pop

#endif // PYTHING_H
