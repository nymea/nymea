#ifndef PYTHINGPAIRINGINFO_H
#define PYTHINGPAIRINGINFO_H


#include <Python.h>
#include "structmember.h"

#include "pyparam.h"

#include "integrations/thingpairinginfo.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMutex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

/* Note:
 * When using this, make sure to call PyThingPairingInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingPairingInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the pairing step and destroyed it but the PyThingPairingInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */



typedef struct {
    PyObject_HEAD
    ThingPairingInfo* info;
    PyObject *pyTransactionId = nullptr;
    PyObject *pyThingClassId = nullptr;
    PyObject *pyThingId = nullptr;
    PyObject *pyThingName = nullptr;
    PyObject *pyParentId = nullptr;
    PyObject *pyParams = nullptr;
    PyObject *pyOAuthUrl = nullptr;
} PyThingPairingInfo;

static PyMemberDef PyThingPairingInfo_members[] = {
    {"transactionId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyTransactionId), READONLY, "The transaction id for this pairing procedure."},
    {"thingClassId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingClassId), READONLY, "The ThingClassId for the thing to be set up."},
    {"thingId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingId), READONLY, "The ThingId for the thing to be set up."},
    {"thingName", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyThingName), READONLY, "The ThingId for the thing to be set up."},
    {"parentId", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyParentId), READONLY, "The ThingId for the parent of the thing to be set up."},
    {"params", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyParams), READONLY, "The params for the thing to be set up."},
    {"oAuthUrl", T_OBJECT_EX, offsetof(PyThingPairingInfo, pyOAuthUrl), 0, "An OAuth url if required for the pairing."},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static int PyThingPairingInfo_init(PyThingPairingInfo */*self*/, PyObject */*args*/, PyObject */*kwds*/)
{
    qWarning() << "++++ ThingPairingInfo";
    return 0;
}

void PyThingPairingInfo_setInfo(PyThingPairingInfo *self, ThingPairingInfo *info)
{
    self->info = info;
    self->pyTransactionId = PyUnicode_FromString(info->transactionId().toString().toUtf8());
    self->pyThingClassId = PyUnicode_FromString(info->thingClassId().toString().toUtf8());
    self->pyThingId = PyUnicode_FromString(info->thingId().toString().toUtf8());
    self->pyThingName = PyUnicode_FromString(info->thingName().toUtf8());
    self->pyParentId = PyUnicode_FromString(info->parentId().toString().toUtf8());
    self->pyParams = PyParams_FromParamList(info->params());
}

static void PyThingPairingInfo_dealloc(PyThingPairingInfo * self)
{
    qWarning() << "---- ThingPairingInfo";
    Py_XDECREF(self->pyTransactionId);
    Py_XDECREF(self->pyThingClassId);
    Py_XDECREF(self->pyThingId);
    Py_XDECREF(self->pyThingName);
    Py_XDECREF(self->pyParentId);
    Py_XDECREF(self->pyParams);
    Py_XDECREF(self->pyOAuthUrl);
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyThingPairingInfo_finish(PyThingPairingInfo* self, PyObject* args)
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
        if (self->pyOAuthUrl) {
            QString oAuthUrl = QString::fromUtf8(PyUnicode_AsUTF8AndSize(self->pyOAuthUrl, nullptr));
            QMetaObject::invokeMethod(self->info, "setOAuthUrl", Qt::QueuedConnection, Q_ARG(QString, oAuthUrl));
        }
        QMetaObject::invokeMethod(self->info, "finish", Qt::QueuedConnection, Q_ARG(Thing::ThingError, thingError), Q_ARG(QString, displayMessage));
    }
    Py_RETURN_NONE;
}

static PyMethodDef PyThingPairingInfo_methods[] = {
    { "finish", (PyCFunction)PyThingPairingInfo_finish, METH_VARARGS, "Finish a discovery" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingPairingInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingPairingInfo", /* tp_name */
    sizeof(PyThingPairingInfo), /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThingPairingInfo_dealloc, /* tp_dealloc */
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
    "ThingPairingInfo",         /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyThingPairingInfo_methods, /* tp_methods */
    PyThingPairingInfo_members, /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)PyThingPairingInfo_init,    /* tp_init */
    0,                          /* tp_alloc */
    PyType_GenericNew,          /* tp_new */
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



static void registerThingPairingInfoType(PyObject *module)
{

    if (PyType_Ready(&PyThingPairingInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingPairingInfo", (PyObject *)&PyThingPairingInfoType);
}




#pragma GCC diagnostic pop

#endif // PYTHINGPAIRINGINFO_H
