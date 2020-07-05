#ifndef PYTHINGDISCOVERYINFO_H
#define PYTHINGDISCOVERYINFO_H


#include <Python.h>
#include "structmember.h"

#include "pythingdescriptor.h"
#include "pyparam.h"

#include "integrations/thingdiscoveryinfo.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMutex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"


/* Note:
 * When using this, make sure to call PyThingDiscoveryInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingDiscoveryInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the discovery and destroyed it but the PyThingDiscoveryInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */


typedef struct {
    PyObject_HEAD
    ThingDiscoveryInfo* info;
    PyObject* pyThingClassId = nullptr;
    PyObject *pyParams = nullptr;
} PyThingDiscoveryInfo;

static PyObject* PyThingDiscoveryInfo_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/)
{
    PyThingDiscoveryInfo *self = (PyThingDiscoveryInfo*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qWarning() << "++++ PyThingDiscoveryInfo";
    return (PyObject*)self;
}

void PyThingDiscoveryInfo_setInfo(PyThingDiscoveryInfo *self, ThingDiscoveryInfo *info)
{
    self->info = info;
    self->pyThingClassId = PyUnicode_FromString(info->thingClassId().toString().toUtf8().data());
    self->pyParams = PyParams_FromParamList(info->params());
}

static void PyThingDiscoveryInfo_dealloc(PyThingDiscoveryInfo * self)
{
    qWarning() << "---- PyThingDiscoveryInfo";
    Py_DECREF(self->pyThingClassId);
    Py_DECREF(self->pyParams);
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

static PyObject * PyThingDiscoveryInfo_addDescriptor(PyThingDiscoveryInfo* self, PyObject* args)
{
    PyObject *pyObj = nullptr;

    if (!PyArg_ParseTuple(args, "O", &pyObj)) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument to ThingDiscoveryInfo.addDescriptor(). Not a ThingDescriptor.");
        return nullptr;
    }
    if (pyObj->ob_type != &PyThingDescriptorType) {
        PyErr_SetString(PyExc_ValueError, "Invalid argument to ThingDiscoveryInfo.addDescriptor(). Not a ThingDescriptor.");
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
    if (pyDescriptor->pyThingId) {
        descriptor.setThingId(ThingId(QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyThingId))));
    }

    if (pyDescriptor->pyParams) {
        descriptor.setParams(PyParams_ToParamList(pyDescriptor->pyParams));
    }

    if (self->info) {
        QMetaObject::invokeMethod(self->info, "addThingDescriptor", Qt::QueuedConnection, Q_ARG(ThingDescriptor, descriptor));
    }

    Py_RETURN_NONE;
}

static PyMemberDef PyThingDiscoveryInfo_members[] = {
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingDiscoveryInfo, pyThingClassId), READONLY, "The ThingClassId this discovery is for."},
    {"params", T_OBJECT_EX, offsetof(PyThingDiscoveryInfo, pyParams), READONLY, "The params for this discovery"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyThingDiscoveryInfo_methods[] = {
    { "addDescriptor", (PyCFunction)PyThingDiscoveryInfo_addDescriptor, METH_VARARGS, "Add a new descriptor to the discovery" },
    { "finish", (PyCFunction)PyThingDiscoveryInfo_finish, METH_VARARGS, "Finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingDiscoveryInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingDiscoveryInfo", /* tp_name */
    sizeof(PyThingDiscoveryInfo), /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThingDiscoveryInfo_dealloc, /* tp_dealloc */
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
    "ThingDiscoveryInfo",       /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyThingDiscoveryInfo_methods, /* tp_methods */
    PyThingDiscoveryInfo_members, /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    (newfunc)PyThingDiscoveryInfo_new, /* tp_new */
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



static void registerThingDiscoveryInfoType(PyObject *module)
{

    if (PyType_Ready(&PyThingDiscoveryInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingDiscoveryInfo", (PyObject *)&PyThingDiscoveryInfoType);
}




#pragma GCC diagnostic pop

#endif // PYTHINGDISCOVERYINFO_H
