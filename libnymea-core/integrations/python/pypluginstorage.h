#ifndef PYPLUGINSTORAGE_H
#define PYPLUGINSTORAGE_H

#include <Python.h>
#include "structmember.h"
#include "pyutils.h"

//#include "integrations/pythonintegrationplugin.h"
#include "loggingcategories.h"
#include "nymeasettings.h"
#include "typeutils.h"

#include <QSettings>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


typedef struct {
    PyObject_HEAD
    QSettings *settings;
} PyPluginStorage;


static PyMemberDef PyPluginStorage_members[] = {
    {nullptr, 0, 0, 0, nullptr}  /* Sentinel */
};

static PyObject * PyPluginStorage_value(PyPluginStorage* self, PyObject* args) {
    char *keyStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &keyStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: value(key)");
        return nullptr;
    }

    QVariant value = self->settings->value(keyStr);
    return QVariantToPyObject(value);
};

static PyObject * PyPluginStorage_setValue(PyPluginStorage* self, PyObject* args) {
    char *keyStr = nullptr;
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "sO", &keyStr, &value)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: value(key, value)");
        return nullptr;
    }

    self->settings->setValue(keyStr, PyObjectToQVariant(value));
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_beginGroup(PyPluginStorage* self, PyObject* args) {
    char *groupStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &groupStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in value call. Expected: beginGroup(group)");
        return nullptr;
    }

    self->settings->beginGroup(groupStr);
    Py_RETURN_NONE;
};

static PyObject * PyPluginStorage_endGroup(PyPluginStorage* self, PyObject* args) {
    Q_UNUSED(args)
    self->settings->endGroup();
    Py_RETURN_NONE;
};

static PyMethodDef PyPluginStorage_methods[] = {
    { "value", (PyCFunction)PyPluginStorage_value,       METH_VARARGS, "Get a value from the plugin storage" },
    { "setValue", (PyCFunction)PyPluginStorage_setValue, METH_VARARGS, "Set a value to the plugin storage"},
    { "beginGroup", (PyCFunction)PyPluginStorage_beginGroup, METH_VARARGS, "Begin a group in the plugin storage."},
    { "endGroup", (PyCFunction)PyPluginStorage_endGroup, METH_VARARGS, "End a group in the plugin storage."},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static int PyPluginStorage_init(PyPluginStorage *self, PyObject *args, PyObject *kwds)
{
    Q_UNUSED(kwds)
    char *pluginIdStr = nullptr;
    if (!PyArg_ParseTuple(args, "s", &pluginIdStr)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments in constructor. Expected: PluginStorage(pluginId)");
        return -1;
    }

    if (!pluginIdStr) {
        return -1;
    }
    PluginId pluginId(pluginIdStr);
    if (pluginId.isNull()) {
        return -1;
    }

    self->settings = new QSettings(NymeaSettings::settingsPath() + "/pluginconfig-" + pluginId.toString().remove(QRegExp("[{}]")) + ".conf", QSettings::IniFormat);

    qCDebug(dcPythonIntegrations()) << "+++ PyPluginStorage";
    return 0;
}

static void PyPluginStorage_dealloc(PyPluginStorage * self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyPluginStorage";
    Py_TYPE(self)->tp_free(self);
}

static PyTypeObject PyPluginStorageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "nymea.PluginStorage",   /* tp_name */
    sizeof(PyPluginStorage), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyPluginStorage_dealloc, /* tp_dealloc */
};



static void registerPluginStorageType(PyObject *module)
{
    PyPluginStorageType.tp_new = PyType_GenericNew;
    PyPluginStorageType.tp_members = PyPluginStorage_members;
    PyPluginStorageType.tp_methods = PyPluginStorage_methods;
    PyPluginStorageType.tp_init = reinterpret_cast<initproc>(PyPluginStorage_init);
    PyPluginStorageType.tp_doc = "PluginStorage can be used by plugins to store key-value pairs to a persistant place.";
    PyPluginStorageType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyPluginStorageType) < 0) {
        return;
    }
    PyModule_AddObject(module, "PluginStorage", reinterpret_cast<PyObject*>(&PyPluginStorageType));
}

#pragma GCC diagnostic pop

#endif // PYPLUGINSTORAGE_H
