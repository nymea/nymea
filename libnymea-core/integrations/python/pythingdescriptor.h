#ifndef PYTHINGDESCRIPTOR_H
#define PYTHINGDESCRIPTOR_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thingdescriptor.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"


typedef struct {
    PyObject_HEAD
    PyObject* pyThingClassId;
    PyObject* pyName;
    PyObject* pyDescription;
} PyThingDescriptor;

static PyMethodDef PyThingDescriptor_methods[] = {
//    { "finish", (PyCFunction)PyThingDiscoveryInfo_finish,    METH_VARARGS,       "finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyThingDescriptor_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyThingClassId), 0, "Descriptor thingClassId"},
    {"name", T_OBJECT_EX, offsetof(PyThingDescriptor, pyName), 0, "Descriptor name"},
    {"description", T_OBJECT_EX, offsetof(PyThingDescriptor, pyDescription), 0, "Descriptor description"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};


static int PyThingDescriptor_init(PyThingDescriptor *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"thingClassId", "name", "description", nullptr};
    PyObject *thingClassId = nullptr, *name = nullptr, *description = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist, &thingClassId, &name, &description))
        return -1;

    if (thingClassId) {
        Py_XDECREF(self->pyThingClassId);
        Py_INCREF(thingClassId);
        self->pyThingClassId = thingClassId;
    }
    if (name) {
        Py_XDECREF(self->pyName);
        Py_INCREF(name);
        self->pyName = name;
    }
    if (description) {
        Py_XDECREF(self->pyDescription);
        Py_INCREF(description);
        self->pyDescription = description;
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



static void registerThingDescriptorType(PyObject *module)
{
    PyThingDescriptorType.tp_new = PyType_GenericNew;
    PyThingDescriptorType.tp_basicsize = sizeof(PyThingDescriptor);
    PyThingDescriptorType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingDescriptorType.tp_doc = "ThingDescriptor class";
    PyThingDescriptorType.tp_methods = PyThingDescriptor_methods;
    PyThingDescriptorType.tp_members = PyThingDescriptor_members;
    PyThingDescriptorType.tp_init = reinterpret_cast<initproc>(PyThingDescriptor_init);

    if (PyType_Ready(&PyThingDescriptorType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDescriptor", reinterpret_cast<PyObject*>(&PyThingDescriptorType));
}

#pragma GCC diagnostic pop

#endif // PYTHINGDESCRIPTOR_H
