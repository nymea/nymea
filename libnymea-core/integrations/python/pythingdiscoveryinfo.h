#ifndef PYTHINGDISCOVERYINFO_H
#define PYTHINGDISCOVERYINFO_H


#include <Python.h>
#include "structmember.h"

#include "integrations/thingdiscoveryinfo.h"

#include <QDebug>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"



typedef struct {
    PyObject_HEAD
    PyObject* thingClassId;
    PyObject* name;
    PyObject* description;
    ThingDescriptor descriptor;
} PyThingDescriptor;

static PyMethodDef PyThingDescriptor_methods[] = {
//    { "finish", (PyCFunction)PyThingDiscoveryInfo_finish,    METH_VARARGS,       "finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyThingDescriptor_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDescriptor, thingClassId), 0, "Descriptor thingClassId"},
    {"name", T_OBJECT_EX, offsetof(PyThingDescriptor, name), 0, "Descriptor name"},
    {"description", T_OBJECT_EX, offsetof(PyThingDescriptor, description), 0, "Descriptor description"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};


static int PyThingDescriptor_init(PyThingDescriptor *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"thingClassId", "name", "description", nullptr};
    PyObject *thingClassId = nullptr, *name = nullptr, *description = nullptr, *tmp = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist, &thingClassId, &name, &description))
        return -1;

    if (thingClassId) {
        tmp = self->thingClassId;
        Py_INCREF(thingClassId);
        self->thingClassId = thingClassId;
        Py_XDECREF(tmp);
    }
    if (name) {
        tmp = self->name;
        Py_INCREF(name);
        self->name = name;
        Py_XDECREF(tmp);
    }
    if (description) {
        tmp = self->description;
        Py_INCREF(description);
        self->description = description;
        Py_XDECREF(tmp);
    }
    return 0;
}

//static PyGetSetDef PyThingDescriptor_getsetters[] = {
//    {"name", (getter) PyThingDescriptor_getName, (setter) PyThingDescriptir_setName,
//     "Descriptor name", NULL},
//    {"last", (getter) Custom_getlast, (setter) Custom_setlast,
//     "last name", NULL},
//    {NULL}  /* Sentinel */
//};

static PyTypeObject PyThingDescriptorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingDescriptor",   /* tp_name */
    sizeof(PyThingDescriptor), /* tp_basicsize */
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


















typedef struct {
    PyObject_HEAD
    ThingDiscoveryInfo* ptrObj;
} PyThingDiscoveryInfo;


static int PyThingDiscoveryInfo_init(PyThingDiscoveryInfo */*self*/, PyObject */*args*/, PyObject */*kwds*/)
// initialize PyVoice Object
{
    return 0;
}


static void PyThingDiscoveryInfo_dealloc(PyThingDiscoveryInfo * self)
// destruct the object
{
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingDiscoveryInfo_finish(PyThingDiscoveryInfo* self, PyObject* args)
{
    int status;
    char *message;

    if (PyArg_ParseTuple(args, "is", &status, &message)) {
        (self->ptrObj)->finish(static_cast<Thing::ThingError>(status), QString(message));
        return Py_BuildValue("");
    }

    if (PyArg_ParseTuple(args, "i", &status)) {
        (self->ptrObj)->finish(static_cast<Thing::ThingError>(status));
        return Py_BuildValue("");
    }

    return nullptr;
}

static PyObject * PyThingDiscoveryInfo_addDescriptor(PyThingDiscoveryInfo* self, PyObject* args) {

    PyObject *pyObj = nullptr;

    if (!PyArg_ParseTuple(args, "O", &pyObj)) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument. Not a ThingDescriptor.");
        return nullptr;
    }
    if (pyObj->ob_type != &PyThingDescriptorType) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument. Not a ThingDescriptor.");
        return nullptr;
    }
    PyThingDescriptor *pyDescriptor = (PyThingDescriptor*)pyObj;

    ThingClassId thingClassId;
    if (pyDescriptor->thingClassId) {
        thingClassId = ThingClassId(PyUnicode_AsUTF8(pyDescriptor->thingClassId));
    }
    QString name;
    if (pyDescriptor->name) {
        name = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->name));
    }
    QString description;
    if (pyDescriptor->description) {
        description = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->description));
    }

    ThingDescriptor descriptor(thingClassId, name, description);

    self->ptrObj->addThingDescriptor(descriptor);

    return Py_BuildValue("");
}

static PyMethodDef PyThingDiscoveryInfo_methods[] = {
    { "addDescriptor", (PyCFunction)PyThingDiscoveryInfo_addDescriptor,    METH_VARARGS,       "Add a new descriptor to the discovery" },
    { "finish", (PyCFunction)PyThingDiscoveryInfo_finish,    METH_VARARGS,       "finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingDiscoveryInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingDiscoveryInfo",   /* tp_name */
    sizeof(PyThingDiscoveryInfo), /* tp_basicsize */
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










#pragma GCC diagnostic pop

#endif // PYTHINGDISCOVERYINFO_H
