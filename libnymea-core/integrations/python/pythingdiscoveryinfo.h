#ifndef PYTHINGDISCOVERYINFO_H
#define PYTHINGDISCOVERYINFO_H


#include <Python.h>
#include "structmember.h"

#include "pythingdescriptor.h"

#include "integrations/thingdiscoveryinfo.h"

#include <QDebug>
#include <QMetaEnum>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"


typedef struct {
    PyObject_HEAD
    ThingDiscoveryInfo* info;
} PyThingDiscoveryInfo;


static int PyThingDiscoveryInfo_init(PyThingDiscoveryInfo */*self*/, PyObject */*args*/, PyObject */*kwds*/) {
    return 0;
}


static void PyThingDiscoveryInfo_dealloc(PyThingDiscoveryInfo * self) {
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingDiscoveryInfo_finish(PyThingDiscoveryInfo* self, PyObject* args)
{
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
    if (pyDescriptor->pyThingClassId) {
        thingClassId = ThingClassId(PyUnicode_AsUTF8(pyDescriptor->pyThingClassId));
    }
    QString name;
    if (pyDescriptor->pyName) {
        name = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyName));
    }
    QString description;
    if (pyDescriptor->pyDescription) {
        description = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyDescription));
    }

    ThingDescriptor descriptor(thingClassId, name, description);

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "addThingDescriptor", Qt::QueuedConnection, Q_ARG(ThingDescriptor, descriptor));
    }

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



static void registerThingDiscoveryInfoType(PyObject *module)
{
    PyThingDiscoveryInfoType.tp_new = PyType_GenericNew;
    PyThingDiscoveryInfoType.tp_basicsize = sizeof(PyThingDiscoveryInfo);
    PyThingDiscoveryInfoType.tp_dealloc = reinterpret_cast<destructor>(PyThingDiscoveryInfo_dealloc);
    PyThingDiscoveryInfoType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingDiscoveryInfoType.tp_doc = "ThingDiscoveryInfo class";
    PyThingDiscoveryInfoType.tp_methods = PyThingDiscoveryInfo_methods;
    PyThingDiscoveryInfoType.tp_init = reinterpret_cast<initproc>(PyThingDiscoveryInfo_init);

    if (PyType_Ready(&PyThingDiscoveryInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDiscoveryInfo", (PyObject *)&PyThingDiscoveryInfoType);

    QMetaEnum thingErrorEnum = QMetaEnum::fromType<Thing::ThingError>();
    for (int i = 0; i < thingErrorEnum.keyCount(); i++) {
        PyModule_AddObject(module, thingErrorEnum.key(i), PyLong_FromLong(thingErrorEnum.value(i)));
    }
}




#pragma GCC diagnostic pop

#endif // PYTHINGDISCOVERYINFO_H
