#ifndef PYTHING_H
#define PYTHING_H

#include <Python.h>
#include "structmember.h"

#include "pyparam.h"

#include "integrations/thing.h"
#include "loggingcategories.h"

#include <QPointer>
#include <QThread>
#include <QMutexLocker>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

/* Note: Thing is not threadsafe so we must never access thing directly in here.
 * For writing access, invoking with QueuedConnections will decouple stuff
 * For reading access, we keep a cache of the thing properties here and sync them
 * over to the cache when they change.
 * When using this, make sure to call PyThing_setThing() after constructing it.
 */


typedef struct _thing {
    PyObject_HEAD
    Thing *thing = nullptr;
    PyObject *name = nullptr;
    PyObject *params = nullptr;
    PyObject *settings = nullptr;
    PyObject *nameChangedHandler = nullptr;
    QMutex *mutex = nullptr;
} PyThing;


static PyObject* PyThing_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyThing *self = (PyThing*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    self->mutex = new QMutex();

    return (PyObject*)self;
}

static void PyThing_setThing(PyThing *self, Thing *thing)
{
    self->thing = thing;

    self->name = PyUnicode_FromString(self->thing->name().toUtf8().data());
    Py_INCREF(self->name);

    QObject::connect(thing, &Thing::nameChanged, [=](){
        self->mutex->lock();
        Py_XDECREF(self->name);
        self->name = PyUnicode_FromString(self->thing->name().toUtf8().data());

        if (!self->nameChangedHandler) {
            self->mutex->unlock();
            return;
        }
        self->mutex->unlock();

        PyGILState_STATE s = PyGILState_Ensure();
        PyObject_CallFunctionObjArgs(self->nameChangedHandler, self, nullptr);

        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            if (pvalue) {
                PyObject *pstr = PyObject_Str(pvalue);
                if (pstr) {
                    const char* err_msg = PyUnicode_AsUTF8(pstr);
                    if (pstr) {
                        qCWarning(dcThingManager()) << QString(err_msg);
                    }
                }
                PyErr_Restore(ptype, pvalue, ptraceback);
            }
        }

        PyGILState_Release(s);
    });

    self->params = PyParams_FromParamList(self->thing->params());
    Py_INCREF(self->params);

    self->settings = PyParams_FromParamList(self->thing->settings());
    Py_INCREF(self->settings);
    QObject::connect(thing, &Thing::settingChanged, [=](){
        QMutexLocker(self->mutex);
        Py_XDECREF(self->settings);
        self->settings = PyParams_FromParamList(self->thing->settings());
    });
}


static void PyThing_dealloc(PyThing * self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->params);
    Py_XDECREF(self->settings);
    Py_XDECREF(self->nameChangedHandler);
    delete self->mutex;
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyThing_getName(PyThing *self, void */*closure*/)
{
    QMutexLocker(self->mutex);
    if (!self->thing) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }

    Py_INCREF(self->name);
    return self->name;
}

static int PyThing_setName(PyThing *self, PyObject *value, void */*closure*/){
    QString name = QString(PyUnicode_AsUTF8(value));
    QMutexLocker(self->mutex);
    QMetaObject::invokeMethod(self->thing, "setName", Qt::QueuedConnection, Q_ARG(QString, name));
    return 0;
}

static PyObject *PyThing_getSettings(PyThing *self, void */*closure*/)
{
    QMutexLocker(self->mutex);
    if (!self->thing) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }
    Py_INCREF(self->settings);
    return self->settings;
}

static int PyThing_setSettings(PyThing */*self*/, PyObject */*value*/, void */*closure*/){
    //    self->thing->setName(QString(PyUnicode_AsUTF8(value)));
    return 0;
}

static PyObject * PyThing_setStateValue(PyThing* self, PyObject* args)
{
    char *stateTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "sO", &stateTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    StateTypeId stateTypeId = StateTypeId(stateTypeIdStr);
    QVariant value = PyObjectToQVariant(valueObj);

    QMutexLocker(self->mutex);
    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "setStateValue", Qt::QueuedConnection, Q_ARG(StateTypeId, stateTypeId), Q_ARG(QVariant, value));
    }

    Py_RETURN_NONE;
}

static PyObject * PyThing_emitEvent(PyThing* self, PyObject* args)
{
    char *eventTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "s|O", &eventTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    EventTypeId eventTypeId = EventTypeId(eventTypeIdStr);
    ParamList params = PyParams_ToParamList(valueObj);

    QMutexLocker(self->mutex);
    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "emitEvent", Qt::QueuedConnection, Q_ARG(EventTypeId, eventTypeId), Q_ARG(ParamList, params));
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyThing_getset[] = {
    {"name", (getter)PyThing_getName, (setter)PyThing_setName, "Thing name", nullptr},
    {"settings", (getter)PyThing_getSettings, (setter)PyThing_setSettings, "Thing settings", nullptr},
    {nullptr , nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMethodDef PyThing_methods[] = {
    { "setStateValue", (PyCFunction)PyThing_setStateValue,    METH_VARARGS,       "Set a things state value" },
    { "emitEvent", (PyCFunction)PyThing_emitEvent,    METH_VARARGS,       "Emits an event" },
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyThing_members[] = {
    {"nameChangedHandler", T_OBJECT_EX, offsetof(PyThing, nameChangedHandler), 0, "Set a callback for when the thing name changes"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyTypeObject PyThingType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.Thing",              /* tp_name */
    sizeof(PyThing),            /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)PyThing_dealloc, /* tp_dealloc */
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
    "Thing",                    /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyThing_methods,            /* tp_methods */
    PyThing_members,            /* tp_members */
    PyThing_getset,             /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    (newfunc)PyThing_new,       /* tp_new */
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

static void registerThingType(PyObject *module)
{
    if (PyType_Ready(&PyThingType) < 0) {
        return;
    }
    PyModule_AddObject(module, "Thing", reinterpret_cast<PyObject*>(&PyThingType));
}


#pragma GCC diagnostic pop

#endif // PYTHING_H
