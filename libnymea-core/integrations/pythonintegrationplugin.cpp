#include <Python.h>

#include "python/pynymealogginghandler.h"
#include "python/pything.h"
#include "python/pythingdiscoveryinfo.h"
#include "python/pythingsetupinfo.h"

#include "pythonintegrationplugin.h"

#include "loggingcategories.h"

#include <QFileInfo>
#include <QMetaEnum>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>

QHash<PyObject*, PyThreadState*> s_modules;
PyThreadState* PythonIntegrationPlugin::s_mainThread = nullptr;
PyObject* PythonIntegrationPlugin::s_nymeaModule = nullptr;
PyObject* PythonIntegrationPlugin::s_asyncio = nullptr;





// Write to stdout/stderr
PyObject* nymea_write(PyObject* /*self*/, PyObject* args)
{
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return nullptr;
    if (!QByteArray(what).trimmed().isEmpty()) {
        qCDebug(dcThingManager()) << what;
    }
    Py_RETURN_NONE;
}

// Flush stdout/stderr
PyObject* nymea_flush(PyObject* /*self*/, PyObject* /*args*/)
{
    // Not really needed... qDebug() flushes already on its own
    Py_RETURN_NONE;
}

PyObject* task_done(PyObject* /*self*/, PyObject* args)
{

    PyObject *result = nullptr;

    if (!PyArg_ParseTuple(args, "O", &result)) {
        qCWarning(dcThingManager()) << "Cannot fetch result from coroutine callback.";
        return nullptr;
    }

    PyObject *exceptionMethod = PyObject_GetAttrString(result, "exception");

    PyObject *exception = PyObject_CallFunctionObjArgs(exceptionMethod, nullptr);
    if (exception != Py_None) {
        PyObject* repr = PyObject_Repr(exception);
        PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
        const char *bytes = PyBytes_AS_STRING(str);
        Py_XDECREF(repr);
        Py_XDECREF(str);

        qCWarning(dcThingManager()) << "Exception:" << bytes;

    }

    Py_RETURN_NONE;
}


static PyMethodDef nymea_methods[] =
{
    {"write", nymea_write, METH_VARARGS, "write to stdout through qDebug()"},
    {"flush", nymea_flush, METH_VARARGS, "flush stdout (no-op)"},
    {"task_done", task_done, METH_VARARGS, "callback to clean up after asyc coroutines"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyModuleDef nymea_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "nymea",               // const char* m_name;
    "nymea module for python based integration plugins",       // const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    nymea_methods,        // PyMethodDef *m_methods
    //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
    nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_nymea(void)
{
    PyObject* m = PyModule_Create(&nymea_module);
    // Overrride stdout/stderr to use qDebug instead
    PySys_SetObject("stdout", m);
    PySys_SetObject("stderr", m);


    registerNymeaLoggingHandler(m);
    registerThingType(m);
    registerThingDescriptorType(m);
    registerThingDiscoveryInfoType(m);
    registerThingSetupInfoType(m);

    return m;
}

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

PythonIntegrationPlugin::~PythonIntegrationPlugin()
{
    m_eventLoop.cancel();
    m_eventLoop.waitForFinished();
}

void PythonIntegrationPlugin::initPython()
{
    PyImport_AppendInittab("nymea", PyInit_nymea);
    Py_InitializeEx(0);
    PyEval_InitThreads();

    // Import nymea module into this interpreter
    s_nymeaModule = PyImport_ImportModule("nymea");

    // We'll be using asyncio everywhere, so let's import it right away
    s_asyncio = PyImport_ImportModule("asyncio");

    // Need to release ths lock from the main thread before spawning new threads
    s_mainThread = PyEval_SaveThread();
}

bool PythonIntegrationPlugin::loadScript(const QString &scriptFile)
{
    QFileInfo fi(scriptFile);

    QFile metaData(fi.absolutePath() + "/" + fi.baseName() + ".json");
    if (!metaData.open(QFile::ReadOnly)) {
        qCWarning(dcThingManager()) << "Error opening metadata file:" << metaData.fileName();
        return false;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(metaData.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcThingManager()) << "Error parsing metadata file:" << error.errorString();
        return false;
    }
    m_metaData = PluginMetadata(jsonDoc.object());


    PyGILState_STATE s = PyGILState_Ensure();

    // Finally, import the plugin
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString(fi.absolutePath().toUtf8()));
    m_module = PyImport_ImportModule(fi.baseName().toUtf8());

    if (!m_module) {
        dumpError();
        qCWarning(dcThingManager()) << "Error importing python plugin from:" << fi.absoluteFilePath();
        PyGILState_Release(s);
        return false;
    }
    qCDebug(dcThingManager()) << "Imported python plugin from" << fi.absoluteFilePath();


    // Set up logger with appropriate logging category
    PyNymeaLoggingHandler *logger = reinterpret_cast<PyNymeaLoggingHandler*>(_PyObject_New(&PyNymeaLoggingHandlerType));
    QString category = m_metaData.pluginName();
    category.replace(0, 1, category[0].toUpper());
    logger->category = static_cast<char*>(malloc(category.length() + 1));
    memset(logger->category, '0', category.length() +1);
    strcpy(logger->category, category.toUtf8().data());
    PyModule_AddObject(m_module, "logger", reinterpret_cast<PyObject*>(logger));


    // Export metadata ids into module
    exportIds();

    PyGILState_Release(s);
    return true;
}

void PythonIntegrationPlugin::init()
{
    callPluginFunction("init", nullptr);
}

void PythonIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
    PyThingDiscoveryInfo *pyInfo = PyObject_New(PyThingDiscoveryInfo, &PyThingDiscoveryInfoType);
    pyInfo->ptrObj = info;

    connect(info, &ThingDiscoveryInfo::destroyed, this, [=](){
        PyGILState_STATE s = PyGILState_Ensure();
        pyInfo->ptrObj = nullptr;
        PyObject_Del(pyInfo);
        PyGILState_Release(s);
    });

    callPluginFunction("discoverThings", reinterpret_cast<PyObject*>(pyInfo));
}

void PythonIntegrationPlugin::setupThing(ThingSetupInfo *info)
{
    PyThing *pyThing = PyObject_New(PyThing, &PyThingType);
    pyThing->ptrObj = info->thing();

    PyThingSetupInfo *pyInfo = PyObject_New(PyThingSetupInfo, &PyThingSetupInfoType);
    pyInfo->ptrObj = info;
    pyInfo->thing = pyThing;


    connect(info, &ThingSetupInfo::finished, this, [=](){
        if (info->status() == Thing::ThingErrorNoError) {
            m_things.insert(info->thing(), pyThing);
        } else {
            PyGILState_STATE s = PyGILState_Ensure();
            Py_DECREF(pyThing);
            PyGILState_Release(s);
        }
    });
    connect(info, &ThingSetupInfo::aborted, this, [=](){
        PyGILState_STATE s = PyGILState_Ensure();
        Py_DECREF(pyThing);
        PyGILState_Release(s);
    });
    connect(info, &ThingSetupInfo::destroyed, this, [=](){
        PyGILState_STATE s = PyGILState_Ensure();
        PyObject_Del(pyInfo);
        PyGILState_Release(s);
    });


    callPluginFunction("setupThing", reinterpret_cast<PyObject*>(pyInfo));
}

void PythonIntegrationPlugin::postSetupThing(Thing *thing)
{
    PyThing* pyThing = m_things.value(thing);
    callPluginFunction("postSetupThing", reinterpret_cast<PyObject*>(pyThing));
}

void PythonIntegrationPlugin::thingRemoved(Thing *thing)
{
    PyThing *pyThing = m_things.value(thing);

    callPluginFunction("thingRemoved", reinterpret_cast<PyObject*>(pyThing));

    PyGILState_STATE s = PyGILState_Ensure();

    pyThing->ptrObj = nullptr;
    Py_DECREF(pyThing);


    PyGILState_Release(s);

    m_things.remove(thing);
}

void PythonIntegrationPlugin::dumpError()
{
    if (!PyErr_Occurred()) {
        return;
    }

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

void PythonIntegrationPlugin::exportIds()
{
    qCDebug(dcThingManager()) << "Exporting plugin IDs:";
    QString pluginName = "pluginId";
    QString pluginId = m_metaData.pluginId().toString();
    qCDebug(dcThingManager()) << "- Plugin:" << pluginName << pluginId;
    PyModule_AddStringConstant(m_module, pluginName.toUtf8(), pluginId.toUtf8());

    foreach (const ThingClass &thingClass, supportedThings()) {
        exportThingClass(thingClass);
    }
}

void PythonIntegrationPlugin::exportThingClass(const ThingClass &thingClass)
{
    QString variableName = QString("%1ThingClassId").arg(thingClass.name());
    if (m_variableNames.contains(variableName)) {
        qWarning().nospace() << "Error: Duplicate name " << variableName << " for ThingClass " << thingClass.id() << ". Skipping entry.";
        return;
    }
    m_variableNames.append(variableName);

    qCDebug(dcThingManager()) << "|- ThingClass:" << variableName << thingClass.id();
    PyModule_AddStringConstant(m_module, variableName.toUtf8(), thingClass.id().toString().toUtf8());

    exportParamTypes(thingClass.paramTypes(), thingClass.name(), "", "thing");
    exportParamTypes(thingClass.settingsTypes(), thingClass.name(), "", "settings");
    exportParamTypes(thingClass.discoveryParamTypes(), thingClass.name(), "", "discovery");

    exportStateTypes(thingClass.stateTypes(), thingClass.name());
    exportEventTypes(thingClass.eventTypes(), thingClass.name());
    exportActionTypes(thingClass.actionTypes(), thingClass.name());
    exportBrowserItemActionTypes(thingClass.browserItemActionTypes(), thingClass.name());
}

void PythonIntegrationPlugin::exportParamTypes(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName)
{
    foreach (const ParamType &paramType, paramTypes) {
        QString variableName = QString("%1ParamTypeId").arg(thingClassName + typeName[0].toUpper() + typeName.right(typeName.length()-1) + typeClass + paramType.name()[0].toUpper() + paramType.name().right(paramType.name().length() -1 ));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for ParamTypeId " << paramType.id() << ". Skipping entry.";
            continue;
        }
        m_variableNames.append(variableName);

        PyModule_AddStringConstant(m_module, variableName.toUtf8(), paramType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName)
{
    foreach (const StateType &stateType, stateTypes) {
        QString variableName = QString("%1%2StateTypeId").arg(thingClassName, stateType.name()[0].toUpper() + stateType.name().right(stateType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for StateType " << stateType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        qCDebug(dcThingManager()) << "|- StateType:" << variableName << stateType.id();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), stateType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName)
{
    foreach (const EventType &eventType, eventTypes) {
        QString variableName = QString("%1%2EventTypeId").arg(thingClassName, eventType.name()[0].toUpper() + eventType.name().right(eventType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for EventType " << eventType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), eventType.id().toString().toUtf8());

        exportParamTypes(eventType.paramTypes(), thingClassName, "Event", eventType.name());
    }

}

void PythonIntegrationPlugin::exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2ActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for ActionType " << actionType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), actionType.id().toString().toUtf8());

        exportParamTypes(actionType.paramTypes(), thingClassName, "Action", actionType.name());
    }
}

void PythonIntegrationPlugin::exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2BrowserItemActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        if (m_variableNames.contains(variableName)) {
            qWarning().nospace() << "Error: Duplicate name " << variableName << " for Browser Item ActionType " << actionType.name() << " in ThingClass " << thingClassName << ". Skipping entry.";
            return;
        }
        m_variableNames.append(variableName);
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), actionType.id().toString().toUtf8());

        exportParamTypes(actionType.paramTypes(), thingClassName, "BrowserItemAction", actionType.name());
    }

}


void PythonIntegrationPlugin::callPluginFunction(const QString &function, PyObject *param)
{
    PyGILState_STATE s = PyGILState_Ensure();

    qCDebug(dcThingManager()) << "Calling python plugin function" << function;
    PyObject *pFunc = PyObject_GetAttrString(m_module, function.toUtf8());
    if(!pFunc || !PyCallable_Check(pFunc)) {
        Py_XDECREF(pFunc);
        qCWarning(dcThingManager()) << "Python plugin does not implement" << function;
        PyGILState_Release(s);
        return;
    }


    dumpError();

    PyObject *future = PyObject_CallFunctionObjArgs(pFunc, param, nullptr);

    Py_XDECREF(pFunc);

    if (PyErr_Occurred()) {
        qCWarning(dcThingManager()) << "Error calling python method:";
        dumpError();
        PyGILState_Release(s);
        return;
    }

    if (QByteArray(future->ob_type->tp_name) != "coroutine") {
        PyGILState_Release(s);
        return;
    }

    // Spawn a event loop for python
    PyObject *new_event_loop = PyObject_GetAttrString(s_asyncio, "new_event_loop");
    PyObject *loop = PyObject_CallFunctionObjArgs(new_event_loop, nullptr);

    PyObject *run_coroutine_threadsafe = PyObject_GetAttrString(loop, "create_task");
    PyObject *task = PyObject_CallFunctionObjArgs(run_coroutine_threadsafe, future, nullptr);
    dumpError();

    PyObject *add_done_callback = PyObject_GetAttrString(task, "add_done_callback");
    dumpError();

    PyObject *task_done = PyObject_GetAttrString(s_nymeaModule, "task_done");
    PyObject *result = PyObject_CallFunctionObjArgs(add_done_callback, task_done, nullptr);
    dumpError();

    PyObject *run_until_complete = PyObject_GetAttrString(loop, "run_until_complete");
    QtConcurrent::run([=](){
        PyGILState_STATE s = PyGILState_Ensure();
        PyObject_CallFunctionObjArgs(run_until_complete, task, nullptr);
        PyGILState_Release(s);
    });

    Py_DECREF(new_event_loop);
    Py_DECREF(loop);
    Py_DECREF(run_coroutine_threadsafe);
    Py_DECREF(task);
    Py_DECREF(add_done_callback);
    Py_DECREF(result);

    PyGILState_Release(s);
}

