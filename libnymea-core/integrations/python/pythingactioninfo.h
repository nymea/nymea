#ifndef PYTHINGACTIONINFO_H
#define PYTHINGACTIONINFO_H

#include <Python.h>
#include "structmember.h"

#include "pything.h"

#include "integrations/thingactioninfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

/* Note:
 *
 * When using this, make sure to call PyThingActionInfo_setInfo() while holding the GIL to initialize
 * stuff after constructing it. Also set info to nullptr while holding the GIL when the info object vanishes.
 *
 * The ThingActionInfo class is not threadsafe and self->info is owned by nymeas main thread.
 * So we must never directly access anything of it in here.
 *
 * For writing to it, invoking methods with QueuedConnections will thread-decouple stuff.
 * Make sure to check if the info object is still valid (it might not be if nymea finished
 * the setup and destroyed it but the PyThingSetupInfo is not garbage collected yet.
 *
 * For reading access, we keep copies of the thing properties here and sync them
 * over to the according py* members when they change.
 *
 */

typedef struct {
    PyObject_HEAD
    ThingActionInfo* info;
    PyThing *pyThing;
    PyObject *pyActionTypeId;
    PyObject *pyParams;
} PyThingActionInfo;


static PyObject* PyThingActionInfo_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyThingActionInfo *self = (PyThingActionInfo*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    qCDebug(dcPythonIntegrations()) << "+++ PyThingActionInfo";
    return (PyObject*)self;
}

void PyThingActionInfo_setInfo(PyThingActionInfo *self, ThingActionInfo *info, PyThing *pyThing)
{
    self->info = info;
    self->pyThing = pyThing;
    Py_INCREF(pyThing);
    self->pyActionTypeId = PyUnicode_FromString(info->action().actionTypeId().toString().toUtf8());
    self->pyParams = PyParams_FromParamList(info->action().params());
}


static void PyThingActionInfo_dealloc(PyThingActionInfo * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyThingActionInfo";
    Py_DECREF(self->pyThing);
    Py_DECREF(self->pyActionTypeId);
    Py_DECREF(self->pyParams);
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
    {"params", T_OBJECT_EX, offsetof(PyThingActionInfo, pyParams), 0, "The params for this action"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyMethodDef PyThingActionInfo_methods[] = {
    { "finish", (PyCFunction)PyThingActionInfo_finish,    METH_VARARGS,       "finish an action" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyTypeObject PyThingActionInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.ThingActionInfo",    /* tp_name */
    sizeof(PyThingActionInfo),  /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThingActionInfo_dealloc, /* tp_dealloc */
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
    "ThingActionInfo",          /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyThingActionInfo_methods,  /* tp_methods */
    PyThingActionInfo_members,  /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    (newfunc)PyThingActionInfo_new, /* tp_new */
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

static void registerThingActionInfoType(PyObject *module)
{
    if (PyType_Ready(&PyThingActionInfoType) < 0) {
        return;
    }
    PyModule_AddObject(module, "ThingActionInfo", (PyObject *)&PyThingActionInfoType);
}




#pragma GCC diagnostic pop

#endif // PYTHINGACTIONINFO_H
