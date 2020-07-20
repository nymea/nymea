#ifndef PYTHINGDESCRIPTOR_H
#define PYTHINGDESCRIPTOR_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thingdescriptor.h"
#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"


typedef struct {
    PyObject_HEAD
    PyObject* pyThingClassId;
    PyObject* pyName;
    PyObject* pyDescription;
    PyObject* pyThingId;
    PyObject* pyParams;
} PyThingDescriptor;


static PyMemberDef PyThingDescriptor_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyThingClassId), 0, "Descriptor thingClassId"},
    {"name", T_OBJECT_EX, offsetof(PyThingDescriptor, pyName), 0, "Descriptor name"},
    {"description", T_OBJECT_EX, offsetof(PyThingDescriptor, pyDescription), 0, "Descriptor description"},
    {"thingId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyDescription), 0, "The thingId, if there exists a thing for this descriptor already."},
    {"params", T_OBJECT_EX, offsetof(PyThingDescriptor, pyParams), 0, "Params for the thing described by this descriptor."},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static int PyThingDescriptor_init(PyThingDescriptor *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"thingClassId", "name", "description", "thingId", "params", nullptr};
    PyObject *thingClassId = nullptr, *name = nullptr, *description = nullptr, *thingId = nullptr, *params = nullptr;

    qCDebug(dcPythonIntegrations()) << "+++ PyThingDescriptor";
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOOOO", kwlist, &thingClassId, &name, &description, &thingId, &params))
        return -1;

    if (thingClassId) {
        Py_INCREF(thingClassId);
        self->pyThingClassId = thingClassId;
    }
    if (name) {
        Py_INCREF(name);
        self->pyName = name;
    }
    if (description) {
        Py_INCREF(description);
        self->pyDescription = description;
    }
    if (thingId) {
        Py_INCREF(thingId);
        self->pyThingId = thingId;
    }
    if (params) {
        Py_INCREF(params);
        self->pyParams = params;
    }
    return 0;
}

static void PyThingDescriptor_dealloc(PyThingDescriptor * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThingDescriptor";
    Py_XDECREF(self->pyThingClassId);
    Py_XDECREF(self->pyName);
    Py_XDECREF(self->pyDescription);
    Py_XDECREF(self->pyThingId);
    Py_XDECREF(self->pyParams);
    Py_TYPE(self)->tp_free(self);
}

static PyTypeObject PyThingDescriptorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingDescriptor",   /* tp_name */
    sizeof(PyThingDescriptor), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyThingDescriptor_dealloc, /* tp_dealloc */
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
    "ThingDescriptor",         /* tp_doc */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};



static void registerThingDescriptorType(PyObject *module)
{
    PyThingDescriptorType.tp_new = PyType_GenericNew;
    PyThingDescriptorType.tp_members = PyThingDescriptor_members;
    PyThingDescriptorType.tp_init = reinterpret_cast<initproc>(PyThingDescriptor_init);

    if (PyType_Ready(&PyThingDescriptorType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDescriptor", reinterpret_cast<PyObject*>(&PyThingDescriptorType));
}

#pragma GCC diagnostic pop

#endif // PYTHINGDESCRIPTOR_H
