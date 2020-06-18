#ifndef PYPARAM_H
#define PYPARAM_H

#include <Python.h>
#include "structmember.h"

#include "types/param.h"

#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"


typedef struct _pyparam {
    PyObject_HEAD
    PyObject* pyParamTypeId = nullptr;
    PyObject* pyValue = nullptr;
} PyParam;

static void PyParam_dealloc(PyParam * self) {
    // FIXME: Why is this not called? Seems we're leaking...
    Q_ASSERT(false);
    Py_XDECREF(self->pyParamTypeId);
    Py_XDECREF(self->pyValue);
    Py_TYPE(self)->tp_free(self);
}

static PyMethodDef PyParam_methods[] = {
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyMemberDef PyParam_members[] = {
    {"paramTypeId", T_OBJECT_EX, offsetof(PyParam, pyParamTypeId), 0, "Param type ID"},
    {"value", T_OBJECT_EX, offsetof(PyParam, pyValue), 0, "Param value"},
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};


static int PyParam_init(PyParam *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"paramTypeId", "value", nullptr};
    PyObject *paramTypeId = nullptr, *value = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist, &paramTypeId, &value))
        return -1;

    if (paramTypeId) {
        Py_XDECREF(self->pyParamTypeId);
        Py_INCREF(paramTypeId);
        self->pyParamTypeId = paramTypeId;
    }
    if (value) {
        Py_XDECREF(self->pyValue);
        Py_INCREF(value);
        self->pyValue = value;
    }
    return 0;
}

static PyTypeObject PyParamType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.Param",   /* tp_name */
    sizeof(PyParam), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyParam_dealloc,/* tp_dealloc */
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
    0,                         /* tp_doc */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static PyParam* PyParam_fromParam(const Param &param)
{
    PyParam *pyParam = PyObject_New(PyParam, &PyParamType);
    pyParam->pyParamTypeId = PyUnicode_FromString(param.paramTypeId().toString().toUtf8());

    switch (param.value().type()) {
    case QVariant::Bool:
        pyParam->pyValue = PyBool_FromLong(*(long*)param.value().data());
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        pyParam->pyValue = PyLong_FromLong(*(long*)param.value().data());
        break;
    case QVariant::String:
    case QVariant::ByteArray:
        pyParam->pyValue = PyUnicode_FromString(param.value().toString().toUtf8());
        break;
    case QVariant::Double:
        pyParam->pyValue = PyFloat_FromDouble(param.value().toDouble());
        break;
    case QVariant::Invalid:
        pyParam->pyValue = Py_None;
        Py_INCREF(pyParam->pyValue);
        break;
    default:
        qCWarning(dcThingManager) << "Unhandled data type in conversion from Param to PyParam!";
        pyParam->pyValue = Py_None;
        Py_INCREF(pyParam->pyValue);
        break;
    }
    return pyParam;
}

static PyObject* PyParam_FromParamList(const ParamList &params)
{
    PyObject* result = PyTuple_New(params.count());
    for (int i = 0; i < params.count(); i++) {
        PyTuple_SET_ITEM(result, i, (PyObject*)PyParam_fromParam(params.at(i)));
    }
    return result;
}


static void registerParamType(PyObject *module)
{
    PyParamType.tp_new = PyType_GenericNew;
    PyParamType.tp_basicsize = sizeof(PyParam);
    PyParamType.tp_dealloc=(destructor) PyParam_dealloc;
    PyParamType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyParamType.tp_doc = "Param class";
    PyParamType.tp_methods = PyParam_methods;
    PyParamType.tp_members = PyParam_members;
    PyParamType.tp_init = reinterpret_cast<initproc>(PyParam_init);

    if (PyType_Ready(&PyParamType) < 0) {
        return;
    }
    PyModule_AddObject(module, "Param", reinterpret_cast<PyObject*>(&PyParamType));
}

#pragma GCC diagnostic pop

#endif // PYPARAM_H
