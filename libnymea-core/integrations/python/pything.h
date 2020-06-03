#ifndef PYTHING_H
#define PYTHING_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thing.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct {
    PyObject_HEAD
    Thing* ptrObj;
} PyThing;


static int PyThing_init(PyThing */*self*/, PyObject */*args*/, PyObject */*kwds*/)
// initialize PyVoice Object
{
    return 0;
}


static void PyThing_dealloc(PyThing * self)
// destruct the object
{
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}


static PyMethodDef PyThing_methods[] = {
//    { "addDescriptor", (PyCFunction)PyThing_addDescriptor,    METH_VARARGS,       "Add a new descriptor to the discovery" },
//    { "finish", (PyCFunction)PyThing_finish,    METH_VARARGS,       "finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingDiscoveryInfoType = {
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


#pragma GCC diagnostic pop

#endif // PYTHING_H
