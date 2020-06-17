#ifndef PYTHING_H
#define PYTHING_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thing.h"
#include "loggingcategories.h"

#include <QPointer>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct _thing {
    PyObject_HEAD
    Thing *ptrObj;
} PyThing;


static int PyThing_init(PyThing */*self*/, PyObject */*args*/, PyObject */*kwds*/) {
    return 0;
}


static void PyThing_dealloc(PyThing * self) {
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThing_getName(PyThing *self, void */*closure*/)
{
    if (!self->ptrObj) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }
    // FIXME: Needs blocking queued connection
    PyObject *ret = PyUnicode_FromString(self->ptrObj->name().toUtf8().data());
    Py_INCREF(ret);
    return ret;
}

static int PyThing_setName(PyThing *self, PyObject *value, void */*closure*/){
    // FIXME: Needs queued connection
    self->ptrObj->setName(QString(PyUnicode_AsUTF8(value)));
    return 0;
}

static PyObject * PyThing_setStateValue(PyThing* self, PyObject* args)
{
    char *stateTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    // FIXME: is there any better way to do this? Value is a variant
    if (!PyArg_ParseTuple(args, "sO", &stateTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    StateTypeId stateTypeId = StateTypeId(stateTypeIdStr);

    PyObject* repr = PyObject_Repr(valueObj);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AS_STRING(str);

    QVariant value(bytes);

    if (self->ptrObj != nullptr) {
        QMetaObject::invokeMethod(self->ptrObj, "setStateValue", Qt::QueuedConnection, Q_ARG(StateTypeId, stateTypeId), Q_ARG(QVariant, value));
    }

    Py_XDECREF(repr);
    Py_XDECREF(str);

    Py_RETURN_NONE;
}

static PyObject * PyThing_emitEvent(PyThing* self, PyObject* args)
{
    char *eventTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "s|O", &eventTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    EventTypeId eventTypeId = EventTypeId(eventTypeIdStr);

//    if (valueObj != nullptr) {
//        PyObject* repr = PyObject_Repr(valueObj);
//        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
//        const char *bytes = PyBytes_AS_STRING(str);

//        QVariant value(bytes);
//    }


    if (self->ptrObj != nullptr) {
        QMetaObject::invokeMethod(self->ptrObj, "emitEvent", Qt::QueuedConnection, Q_ARG(EventTypeId, eventTypeId));
    }

//    Py_XDECREF(repr);
//    Py_XDECREF(str);

    Py_RETURN_NONE;
}

static PyGetSetDef PyThing_getseters[] = {
    {"name", (getter)PyThing_getName, (setter)PyThing_setName, "Thingname", nullptr},
    {nullptr , nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMethodDef PyThing_methods[] = {
    { "setStateValue", (PyCFunction)PyThing_setStateValue,    METH_VARARGS,       "Set a things state value" },
    { "emitEvent", (PyCFunction)PyThing_emitEvent,    METH_VARARGS,       "Emits an event" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.Thing",             /* tp_name */
    sizeof(PyThing),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Noddy objects",           /* tp_doc */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static void registerThingType(PyObject *module)
{
    PyThingType.tp_new = PyType_GenericNew;
    PyThingType.tp_dealloc= reinterpret_cast<destructor>(PyThing_dealloc);
    PyThingType.tp_basicsize = sizeof(PyThing);
    PyThingType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingType.tp_doc = "Thing class";
    PyThingType.tp_methods = PyThing_methods;
    PyThingType.tp_getset = PyThing_getseters;
//    PyThingType.tp_members = PyThingSetupInfo_members;
    PyThingType.tp_init = reinterpret_cast<initproc>(PyThing_init);

    if (PyType_Ready(&PyThingType) < 0) {
        return;
    }
    PyModule_AddObject(module, "Thing", reinterpret_cast<PyObject*>(&PyThingType));
}


#pragma GCC diagnostic pop

#endif // PYTHING_H
