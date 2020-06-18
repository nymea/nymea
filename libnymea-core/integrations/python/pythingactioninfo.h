#ifndef PYTHINGACTIONINFO_H
#define PYTHINGACTIONINFO_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/thingactioninfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

typedef struct {
    PyObject_HEAD
    ThingActionInfo* info;
    PyThing *pyThing;
    PyObject *pyActionTypeId;
    PyObject *pyParams;
} PyThingActionInfo;


static int PyThingActionInfo_init(PyThingActionInfo */*self*/, PyObject */*args*/, PyObject */*kwds*/) {
    return 0;
}


static void PyThingActionInfo_dealloc(PyThingActionInfo * self) {
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingActionInfo_finish(PyThingActionInfo* self, PyObject* args) {
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "i|s", &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(ThingError, message = \"\"");
        return nullptr;
    }

    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }

    Py_RETURN_NONE;
}

static PyMemberDef PyThingActionInfo_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyThingActionInfo, pyThing), 0, "Thing this action is for"},
    {"actionTypeId", T_OBJECT_EX, offsetof(PyThingActionInfo, pyActionTypeId), 0, "The action type id for this action"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyThingActionInfo_methods[] = {
    { "finish", (PyCFunction)PyThingActionInfo_finish,    METH_VARARGS,       "finish an action" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingActionInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingActionInfo",   /* tp_name */
    sizeof(PyThingActionInfo), /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_vectorcall_offset */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_as_async */
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

static void registerThingActionInfoType(PyObject *module) {
    PyThingActionInfoType.tp_new = PyType_GenericNew;
    PyThingActionInfoType.tp_dealloc=(destructor) PyThingActionInfo_dealloc;
    PyThingActionInfoType.tp_basicsize = sizeof(PyThingActionInfo);
    PyThingActionInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingActionInfoType.tp_doc = "ThingActionInfo class";
    PyThingActionInfoType.tp_methods = PyThingActionInfo_methods;
    PyThingActionInfoType.tp_members = PyThingActionInfo_members;
    PyThingActionInfoType.tp_init = (initproc)PyThingActionInfo_init;

    if (PyType_Ready(&PyThingActionInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingActionInfo", (PyObject *)&PyThingActionInfoType);
}




#pragma GCC diagnostic pop

#endif // PYTHINGACTIONINFO_H
