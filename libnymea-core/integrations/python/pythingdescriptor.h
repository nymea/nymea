#ifndef PYTHINGDESCRIPTOR_H
#define PYTHINGDESCRIPTOR_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thingdescriptor.h"
#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


typedef struct {
    PyObject_HEAD
    PyObject* pyThingClassId;
    PyObject* pyName;
    PyObject* pyDescription;
    PyObject* pyThingId;
    PyObject* pyParentId;
    PyObject* pyParams;
} PyThingDescriptor;


static PyMemberDef PyThingDescriptor_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyThingClassId), 0, "Descriptor thingClassId"},
    {"name", T_OBJECT_EX, offsetof(PyThingDescriptor, pyName), 0, "Descriptor name"},
    {"description", T_OBJECT_EX, offsetof(PyThingDescriptor, pyDescription), 0, "Descriptor description"},
    {"thingId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyDescription), 0, "The thingId, if there exists a thing for this descriptor already."},
    {"parentId", T_OBJECT_EX, offsetof(PyThingDescriptor, pyParentId), 0, "The thingId of the parent, if this thing is a child."},
    {"params", T_OBJECT_EX, offsetof(PyThingDescriptor, pyParams), 0, "Params for the thing described by this descriptor."},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static int PyThingDescriptor_init(PyThingDescriptor *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"thingClassId", "name", "description", "thingId", "parentId", "params", nullptr};
    PyObject *thingClassId = nullptr, *name = nullptr, *description = nullptr, *thingId = nullptr, *parentId = nullptr, *params = nullptr;

    qCDebug(dcPythonIntegrations()) << "+++ PyThingDescriptor";
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOOOOO", kwlist, &thingClassId, &name, &description, &thingId, &parentId, &params))
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
    if (parentId) {
        Py_INCREF(parentId);
        self->pyParentId = parentId;
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
    Py_XDECREF(self->pyParentId);
    Py_XDECREF(self->pyParams);
    Py_TYPE(self)->tp_free(self);
}

static PyTypeObject PyThingDescriptorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingDescriptor",   /* tp_name */
    sizeof(PyThingDescriptor), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyThingDescriptor_dealloc, /* tp_dealloc */
};



static void registerThingDescriptorType(PyObject *module)
{
    PyThingDescriptorType.tp_new = PyType_GenericNew;
    PyThingDescriptorType.tp_members = PyThingDescriptor_members;
    PyThingDescriptorType.tp_init = reinterpret_cast<initproc>(PyThingDescriptor_init);
    PyThingDescriptorType.tp_doc = "ThingDescriptors are used to inform the system about things that may be added.";
    PyThingDescriptorType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyThingDescriptorType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDescriptor", reinterpret_cast<PyObject*>(&PyThingDescriptorType));
}

#pragma GCC diagnostic pop

#endif // PYTHINGDESCRIPTOR_H
