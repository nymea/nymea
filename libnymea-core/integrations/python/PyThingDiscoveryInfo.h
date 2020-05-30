#ifndef PYTHINGDISCOVERYINFO_H
#define PYTHINGDISCOVERYINFO_H


#include <Python.h>

#include "integrations/thingdiscoveryinfo.h"

#include <QDebug>

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

    return Py_False;
}

static PyMethodDef PyThingDiscoveryInfo_methods[] = {
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
#endif // PYTHINGDISCOVERYINFO_H
