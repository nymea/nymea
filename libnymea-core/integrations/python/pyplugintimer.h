// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PYPLUGINTIMER_H
#define PYPLUGINTIMER_H

#include "structmember.h"
#include <Python.h>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#include "loggingcategories.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

typedef struct
{
    PyObject_HEAD PyObject *pyTimeoutHandler = nullptr;
    QTimer *timer;
    int interval;
    PyInterpreterState *interpreter;
} PyPluginTimer;

static int PyPluginTimer_init(PyPluginTimer *self, PyObject *args, PyObject * /*kwds*/)
{
    PyObject *handler;

    qCDebug(dcPythonIntegrations()) << "+++ PyPluginTimer";
    if (!PyArg_ParseTuple(args, "i|O", &self->interval, &handler)) {
        return -1;
    }

    // QTimer needs to be run in a thread that has a QEventLoop but we don't necessarily have one in
    // python threads. So we're moving the timer to the main app thread.
    self->timer = new QTimer();
    self->timer->start(self->interval * 1000);
    self->timer->moveToThread(QCoreApplication::instance()->thread());

    self->pyTimeoutHandler = handler;
    Py_XINCREF(handler);

    // Remember the interpreter from the current thread so we can run the callback in the correct interpreter
    self->interpreter = PyThreadState_GET()->interp;

    QObject::connect(self->timer, &QTimer::timeout, [=]() {
        qCDebug(dcPythonIntegrations) << "Plugin timer timeout" << self->pyTimeoutHandler;

        // Spawn a new thread for the callback of the timer (like we do for every python call).
        // FIXME: Ideally we'd use the plugin's thread pool but we can't easily access that here.
        // If the timer callback blocks for longer than the timer interval, we might end up with
        // tons of threads...
        QFuture<void> future = QtConcurrent::run([=]() {
            // Acquire GIL and make the new thread state the current one
            PyThreadState *threadState = PyThreadState_New(self->interpreter);
            PyEval_RestoreThread(threadState);

            if (self->pyTimeoutHandler) {
                PyObject *ret = PyObject_CallFunction(self->pyTimeoutHandler, nullptr);
                if (PyErr_Occurred()) {
                    PyErr_Print();
                }
                Py_XDECREF(ret);
            }
            PyThreadState_Clear(threadState);
            PyEval_ReleaseThread(threadState);
            PyThreadState_Delete(threadState);
        });
    });

    return 0;
}

static void PyPluginTimer_dealloc(PyPluginTimer *self)
{
    qCDebug(dcPythonIntegrations()) << "--- PyPluginTimer";
    Py_XDECREF(self->pyTimeoutHandler);

    QMetaObject::invokeMethod(self->timer, "stop", Qt::QueuedConnection);
    self->timer->deleteLater();

    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyPluginTimer_getInterval(PyPluginTimer *self, void * /*closure*/)
{
    return PyLong_FromLong(self->interval);
}

static int PyPluginTimer_setInterval(PyPluginTimer *self, PyObject *value, void * /*closure*/)
{
    self->interval = PyLong_AsLong(value);
    QMetaObject::invokeMethod(self->timer, "start", Qt::QueuedConnection, Q_ARG(int, self->interval * 1000));
    return 0;
}

static PyGetSetDef PyPluginTimer_getset[] = {
    {"interval", (getter) PyPluginTimer_getInterval, (setter) PyPluginTimer_setInterval, "Timer interval", nullptr}, {nullptr, nullptr, nullptr, nullptr, nullptr} /* Sentinel */
};

static PyMemberDef PyPluginTimer_members[] = {
    {"timeoutHandler", T_OBJECT_EX, offsetof(PyPluginTimer, pyTimeoutHandler), 0, "Set a callback for when the timer timeout triggers."}, {nullptr, 0, 0, 0, nullptr} /* Sentinel */
};

static PyTypeObject PyPluginTimerType = {
    PyVarObject_HEAD_INIT(NULL, 0) "nymea.PluginTimer", /* tp_name */
    sizeof(PyPluginTimer),                              /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor) PyPluginTimer_dealloc,                 /* tp_dealloc */
};

static void registerPluginTimerType(PyObject *module)
{
    PyPluginTimerType.tp_new = PyType_GenericNew;
    PyPluginTimerType.tp_members = PyPluginTimer_members;
    PyPluginTimerType.tp_getset = PyPluginTimer_getset;
    PyPluginTimerType.tp_init = reinterpret_cast<initproc>(PyPluginTimer_init);
    PyPluginTimerType.tp_doc = "PluginTimers can be used to perform repeating tasks, such as polling a device or online service.";
    PyPluginTimerType.tp_flags = Py_TPFLAGS_DEFAULT;

    if (PyType_Ready(&PyPluginTimerType) < 0) {
        return;
    }
    PyModule_AddObject(module, "PluginTimer", reinterpret_cast<PyObject *>(&PyPluginTimerType));
}

#pragma GCC diagnostic pop

#endif // PYPLUGINTIMER_H
