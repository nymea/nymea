#ifndef PYTHING_H
#define PYTHING_H

#include <Python.h>
#include "structmember.h"

#include "integrations/thing.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct _thing {
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

static PyObject *PyThing_getName(PyThing *self, void */*closure*/)
{
    if (!self->ptrObj) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }
    PyObject *ret = PyUnicode_FromString(self->ptrObj->name().toUtf8().data());
    Py_INCREF(ret);
    return ret;
}

static int PyThing_setName(PyThing *self, PyObject *value, void */*closure*/){
    self->ptrObj->setName(QString(PyUnicode_AsUTF8(value)));
    return 0;
}

static PyObject * PyThing_setStateValue(PyThing* self, PyObject* args)
{
    char *stateTypeId;
    int status;

    if (PyArg_ParseTuple(args, "ss", &stateTypeId, &message)) {
        (self->ptrObj)->finish(static_cast<Thing::ThingError>(status), QString(message));
        Py_RETURN_NONE;
    }
    PyErr_Clear();

    if (PyArg_ParseTuple(args, "i", &status)) {
        (self->ptrObj)->finish(static_cast<Thing::ThingError>(status));
        Py_RETURN_NONE;
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyThing_getseters[] = {
    {"name", (getter)PyThing_getName, (setter)PyThing_setName, "Thingname", nullptr},
    {nullptr , nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMethodDef PyThing_methods[] = {
    { "setStateValue", (PyCFunction)PyThing_setStateValue,    METH_VARARGS,       "Set a things state value" },
//    { "finish", (PyCFunction)PyThing_finish,    METH_VARARGS,       "finish a discovery" },
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
