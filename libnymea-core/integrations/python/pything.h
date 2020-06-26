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

typedef struct _thing {
    PyObject_HEAD
    Thing *thing;
    QMutex *mutex;
} PyThing;


static PyObject* PyThing_new(PyTypeObject *type, PyObject */*args*/, PyObject */*kwds*/) {
    PyThing *self = (PyThing*)type->tp_alloc(type, 0);
    if (self == NULL) {
        return nullptr;
    }
    self->mutex = new QMutex();
    return (PyObject*)self;
}


static void PyThing_dealloc(PyThing * self) {
    Py_TYPE(self)->tp_free(self);
    delete self->mutex;
}

static PyObject *PyThing_getName(PyThing *self, void */*closure*/)
{
    QMutexLocker(self->mutex);
    if (!self->thing) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }
    QString name;
    QMetaObject::invokeMethod(self->thing, "name", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, name));
    PyObject *ret = PyUnicode_FromString(name.toUtf8().data());
    Py_INCREF(ret);
    return ret;
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
    qWarning() << "setting thread" << QThread::currentThread();
    ParamList settings;
    QMetaObject::invokeMethod(self->thing, "settings", Qt::BlockingQueuedConnection, Q_RETURN_ARG(ParamList, settings));
    PyObject *ret = PyParam_FromParamList(settings);
    Py_INCREF(ret);
    return ret;
}

static int PyThing_setSettings(PyThing */*self*/, PyObject */*value*/, void */*closure*/){
    //    self->thing->setName(QString(PyUnicode_AsUTF8(value)));
    return 0;
}

static PyObject * PyThing_setStateValue(PyThing* self, PyObject* args)
{
    char *stateTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    // FIXME: is there any better way to do this? Value is a variant
    if (!PyArg_ParseTuple(args, "sO", &stateTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    StateTypeId stateTypeId = StateTypeId(stateTypeIdStr);

    PyObject* repr = PyObject_Repr(valueObj);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AS_STRING(str);

    QVariant value(bytes);

    QMutexLocker(self->mutex);
    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "setStateValue", Qt::QueuedConnection, Q_ARG(StateTypeId, stateTypeId), Q_ARG(QVariant, value));
    }

    Py_XDECREF(repr);
    Py_XDECREF(str);

    Py_RETURN_NONE;
}

static PyObject *PyThing_settingChanged(PyThing *self, void */*closure*/)
{
    QMutexLocker(self->mutex);
    if (!self->thing) {
        PyErr_SetString(PyExc_ValueError, "Thing has been removed from the system.");
        return nullptr;
    }
    ParamList settings;
    QMetaObject::invokeMethod(self->thing, "settings", Qt::BlockingQueuedConnection, Q_RETURN_ARG(ParamList, settings));
    PyObject *ret = PyParam_FromParamList(settings);
    Py_INCREF(ret);
    return ret;
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
    ParamList params;

    if (valueObj != nullptr) {
        PyObject *iter = PyObject_GetIter(valueObj);

        while (iter) {
            PyObject *next = PyIter_Next(iter);
            if (!next) {
                break;
            }
            if (next->ob_type != &PyParamType) {
                qCWarning(dcThingManager()) << "Invalid parameter passed in param list";
                continue;
            }

            PyParam *pyParam = reinterpret_cast<PyParam*>(next);
            ParamTypeId paramTypeId = ParamTypeId(PyUnicode_AsUTF8(pyParam->pyParamTypeId));

            // Is there a better way to convert a PyObject to a QVariant?
            PyObject* repr = PyObject_Repr(pyParam->pyValue);
            PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
            const char *bytes = PyBytes_AS_STRING(str);
            Py_XDECREF(repr);
            Py_XDECREF(str);

            QVariant value(bytes);
            params.append(Param(paramTypeId, value));
        }
    }

    QMutexLocker(self->mutex);
    if (self->thing != nullptr) {
        QMetaObject::invokeMethod(self->thing, "emitEvent", Qt::QueuedConnection, Q_ARG(EventTypeId, eventTypeId), Q_ARG(ParamList, params));
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyThing_getset[] = {
    {"name", (getter)PyThing_getName, (setter)PyThing_setName, "Thing name", nullptr},
    {"settings", (getter)PyThing_getSettings, (setter)PyThing_setSettings, "Thing settings", nullptr},
    {"settingChanged", (getter)PyThing_settingChanged, nullptr, "Signal for changed settings", nullptr},
    {nullptr , nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMethodDef PyThing_methods[] = {
    { "setStateValue", (PyCFunction)PyThing_setStateValue,    METH_VARARGS,       "Set a things state value" },
    { "emitEvent", (PyCFunction)PyThing_emitEvent,    METH_VARARGS,       "Emits an event" },
    {nullptr, nullptr, 0, nullptr} // sentinel
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
    0,                          /* tp_members */
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
