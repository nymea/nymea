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
    ThingSetupInfo* info;
    PyThing *pyThing;
    QMutex *mutex;
} PyThingSetupInfo;


static PyObject* PyThingSetupInfo_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyThingSetupInfo *self = (PyThingSetupInfo*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    self->mutex = new QMutex();
    return (PyObject*)self;
}

static void PyThingSetupInfo_dealloc(PyThingSetupInfo * self) {
    Py_TYPE(self)->tp_free(self);
    delete self->mutex;
}

static PyObject * PyThingSetupInfo_finish(PyThingSetupInfo* self, PyObject* args) {
    int status;
    char *message = nullptr;

    if (!PyArg_ParseTuple(args, "i|s", &status, &message)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in finish call. Expected: finish(ThingError, message = \"\"");
        return nullptr;
    }

    Thing::ThingError thingError = static_cast<Thing::ThingError>(status);
    QString displayMessage = message != nullptr ? QString(message) : QString();

    QMutexLocker(self->mutex);
    if (self->info) {
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }

    Py_RETURN_NONE;
}

static PyMemberDef PyThingSetupInfo_members[] = {
    {"thing", T_OBJECT_EX, offsetof(PyThingSetupInfo, pyThing), 0, "Thing being setup in this setup transaction"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyThingSetupInfo_methods[] = {
    { "finish", (PyCFunction)PyThingSetupInfo_finish,    METH_VARARGS,       "finish a setup" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingSetupInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingSetupInfo",     /* tp_name */
    sizeof(PyThingSetupInfo),   /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThingSetupInfo_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "ThingSetupInfo",           /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyThingSetupInfo_methods,   /* tp_methods */
    PyThingSetupInfo_members,   /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    (newfunc)PyThingSetupInfo_new, /* tp_new */
    0,                          /* tp_free */
    0,                          /* tp_is_gc */
    0,                          /* tp_bases */
    0,                          /* tp_mro */
    0,                          /* tp_cache */
    0,                          /* tp_subclasses */
    0,                          /* tp_weaklist */
    0,                          /* tp_del */
    0,                          /* tp_version_tag */
    0,                          /* tp_finalize */
    0,                          /* tp_vectorcall */
    0,                          /* tp_print DEPRECATED*/
};

static void registerThingSetupInfoType(PyObject *module) {
    if (PyType_Ready(&PyThingSetupInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingSetupInfo", (PyObject *)&PyThingSetupInfoType);
}




#pragma GCC diagnostic pop

#endif // PYTHINGSETUPINFO_H
