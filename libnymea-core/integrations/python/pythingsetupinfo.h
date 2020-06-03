#ifndef PYTHINGSETUPINFO_H
#define PYTHINGSETUPINFO_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/thingsetupinfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct {
    PyObject_HEAD
    ThingSetupInfo* ptrObj;
} PyThingSetupInfo;


static int PyThingSetupInfo_init(PyThingSetupInfo */*self*/, PyObject */*args*/, PyObject */*kwds*/)
// initialize PyVoice Object
{
    return 0;
}


static void PyThingSetupInfo_dealloc(PyThingSetupInfo * self)
// destruct the object
{
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingSetupInfo_finish(PyThingSetupInfo* self, PyObject* args)
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


static PyMethodDef PyThingDiscoveryInfo_methods[] = {
    { "finish", (PyCFunction)PyThingDiscoveryInfo_finish,    METH_VARARGS,       "finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingSetupInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingSetupInfo",   /* tp_name */
    sizeof(PyThingSetupInfo), /* tp_basicsize */
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

#endif // PYTHINGSETUPINFO_H
