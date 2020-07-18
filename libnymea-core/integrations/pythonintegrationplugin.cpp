#include <Python.h>

#include "python/pynymealogginghandler.h"
#include "python/pything.h"
#include "python/pythingdiscoveryinfo.h"
#include "python/pythingsetupinfo.h"
#include "python/pyparam.h"
#include "python/pythingactioninfo.h"
#include "python/pythingpairinginfo.h"

#include "pythonintegrationplugin.h"

#include "loggingcategories.h"

#include <QFileInfo>
#include <QMetaEnum>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>
#include <QCoreApplication>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>

PyThreadState* PythonIntegrationPlugin::s_mainThreadState = nullptr;
QThreadPool* PythonIntegrationPlugin::s_threadPool = nullptr;

QHash<PythonIntegrationPlugin*, PyObject*> PythonIntegrationPlugin::s_plugins;


PyObject* PythonIntegrationPlugin::task_done(PyObject* self, PyObject* args)
{
    Q_UNUSED(self)

    PyObject *result = nullptr;

    if (!PyArg_ParseTuple(args, "O", &result)) {
        qCWarning(dcPythonIntegrations()) << "Cannot fetch result from coroutine callback.";
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

//        PyObject *traceback = PyObject_CallMethodObjArgs(exception, "__traceback__", nullptr);

        qCWarning(dcPythonIntegrations()) << "Exception in plugin:" << bytes;

        PyErr_Clear();
    }

    Py_RETURN_NONE;
}


static PyMethodDef nymea_methods[] =
{
    {"task_done", PythonIntegrationPlugin::task_done, METH_VARARGS, "callback to clean up after asyc coroutines"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyModuleDef nymea_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "nymea",               // const char* m_name;
    "nymea module for python based integration plugins",       // const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    nymea_methods,        // PyMethodDef *m_methods
    nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_nymea(void)
{
    // Overrride stdout/stderr to use qDebug instead
    PyObject* pyLog = PyModule_Create(&pyLog_module);
    PySys_SetObject("stdout", pyLog);
    PyObject* pyWarn = PyModule_Create(&pyWarn_module);
    PySys_SetObject("stderr", pyWarn);


    // Register nymea types
    PyObject* m = PyModule_Create(&nymea_module);
    registerNymeaLoggingHandler(m);
    registerParamType(m);
    registerThingType(m);
    registerThingDescriptorType(m);
    registerThingDiscoveryInfoType(m);
    registerThingPairingInfoType(m);
    registerThingSetupInfoType(m);
    registerThingActionInfoType(m);

    return m;
}

PyObject* PythonIntegrationPlugin::pyConfiguration(PyObject* self, PyObject* /*args*/)
{
    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot find plugin instance for this python module.";
        return nullptr;
    }
    plugin->m_mutex.lock();
    PyObject *params = PyParams_FromParamList(plugin->configuration());
    plugin->m_mutex.unlock();
    return params;
}

PyObject *PythonIntegrationPlugin::pyConfigValue(PyObject *self, PyObject *args)
{
    char *paramTypeIdStr = nullptr;

    if (!PyArg_ParseTuple(args, "s", &paramTypeIdStr)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    ParamTypeId paramTypeId = ParamTypeId(paramTypeIdStr);

    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot find plugin instance for this python module.";
        return nullptr;
    }

    plugin->m_mutex.lock();
    QVariant value = plugin->m_pluginConfigCopy.paramValue(paramTypeId);
    plugin->m_mutex.unlock();
    return QVariantToPyObject(value);
}

PyObject *PythonIntegrationPlugin::pySetConfigValue(PyObject *self, PyObject *args)
{
    char *paramTypeIdStr = nullptr;
    PyObject *valueObj = nullptr;

    if (!PyArg_ParseTuple(args, "sO", &paramTypeIdStr, &valueObj)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }

    ParamTypeId paramTypeId = EventTypeId(paramTypeIdStr);
    QVariant value = PyObjectToQVariant(valueObj);

    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot find plugin instance for this python module.";
        return nullptr;
    }

    QMetaObject::invokeMethod(plugin, "setConfigValue", Qt::QueuedConnection, Q_ARG(ParamTypeId, paramTypeId), Q_ARG(QVariant, value));

    Py_RETURN_NONE;
}

PyObject *PythonIntegrationPlugin::pyMyThings(PyObject *self, PyObject */*args*/)
{
    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    if (!plugin) {
        qCWarning(dcThingManager()) << "Cannot find plugin instance for this python module.";
        return nullptr;
    }
    
    plugin->m_mutex.lock();
    PyObject* result = PyTuple_New(plugin->m_things.count());
    for (int i = 0; i < plugin->m_things.count(); i++) {
        Thing *thing = plugin->m_things.keys().at(i);
        PyThing *pyThing = plugin->m_things.value(thing);
        Py_INCREF(pyThing);
        PyTuple_SET_ITEM(result, i, (PyObject*)pyThing);
    }
    plugin->m_mutex.unlock();
    return result;
}

PyObject *PythonIntegrationPlugin::pyAutoThingsAppeared(PyObject *self, PyObject *args)
{
    PyObject *pyDescriptors;

    if (!PyArg_ParseTuple(args, "O", &pyDescriptors)) {
        qCWarning(dcThingManager()) << "Error parsing args. Not a param list";
        return nullptr;
    }

    PyObject *iter = PyObject_GetIter(pyDescriptors);
    if (!iter) {
        Py_DECREF(pyDescriptors);
        qCWarning(dcThingManager()) << "Error parsing args. Not a param list";
        return nullptr;
    }

    ThingDescriptors descriptors;

    while (true) {
        PyObject *next = PyIter_Next(iter);
        if (!next) {
            // nothing left in the iterator
            break;
        }

        if (next->ob_type != &PyThingDescriptorType) {
            PyErr_SetString(PyExc_ValueError, "Invalid argument. Not a ThingDescriptor.");
            continue;
        }
        PyThingDescriptor *pyDescriptor = (PyThingDescriptor*)next;

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

        if (pyDescriptor->pyParams) {
            descriptor.setParams(PyParams_ToParamList(pyDescriptor->pyParams));
        }

        descriptors.append(descriptor);
        Py_DECREF(next);
    }

    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    QMetaObject::invokeMethod(plugin, "autoThingsAppeared", Qt::QueuedConnection, Q_ARG(ThingDescriptors, descriptors));

    Py_DECREF(iter);
    Py_DECREF(pyDescriptors);

    Py_RETURN_NONE;
}

PyObject *PythonIntegrationPlugin::pyAutoThingDisappeared(PyObject *self, PyObject *args)
{
    char *thingIdStr = nullptr;

    if (!PyArg_ParseTuple(args, "s", &thingIdStr)) {
        qCWarning(dcThingManager) << "Error parsing parameters";
        return nullptr;
    }
    ThingId thingId(thingIdStr);
    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    QMetaObject::invokeMethod(plugin, "autoThingDisappeared", Qt::QueuedConnection, Q_ARG(ThingId, thingId));

    Py_RETURN_NONE;
}

static PyMethodDef plugin_methods[] =
{
    {"configuration", PythonIntegrationPlugin::pyConfiguration, METH_VARARGS, "Get the plugin configuration."},
    {"configValue", PythonIntegrationPlugin::pyConfigValue, METH_VARARGS, "Get the plugin configuration value for a given config paramTypeId."},
    {"setConfigValue", PythonIntegrationPlugin::pySetConfigValue, METH_VARARGS, "Set the plugin configuration value for a given config paramTypeId."},
    {"myThings", PythonIntegrationPlugin::pyMyThings, METH_VARARGS, "Obtain a list of things owned by this plugin."},
    {"autoThingsAppeared", PythonIntegrationPlugin::pyAutoThingsAppeared, METH_VARARGS, "Inform the system about auto setup things having appeared."},
    {"autoThingDisappeared", PythonIntegrationPlugin::pyAutoThingDisappeared, METH_VARARGS, "Inform the system about auto setup things having disappeared."},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

PythonIntegrationPlugin::~PythonIntegrationPlugin()
{
    // Acquire GIL for this plugin's interpreter
    PyEval_RestoreThread(m_threadState);

    // Cancel all the thread in here
    while (!m_runningTasks.isEmpty()) {
        QFutureWatcher<void> *watcher = m_runningTasks.values().first();
        watcher->cancel();
        watcher->waitForFinished();
    }

    Py_XDECREF(s_plugins.take(this));

    Py_EndInterpreter(m_threadState);

    PyThreadState_Swap(s_mainThreadState);
    PyEval_ReleaseThread(s_mainThreadState);
}

void PythonIntegrationPlugin::initPython()
{
    Q_ASSERT_X(s_mainThreadState == nullptr, "PythonIntegrationPlugin::initPython()", "initPython() must be called exactly once.");

    PyImport_AppendInittab("nymea", PyInit_nymea);
    Py_InitializeEx(0);
    PyEval_InitThreads();
    // Store the main thread state and release the GIL
    s_mainThreadState = PyEval_SaveThread();

    // Allocate a shared thread pool for the plugins
    s_threadPool = new QThreadPool();
    qCDebug(dcPythonIntegrations()) << "Created a thread pool with a maximum of" << s_threadPool->maxThreadCount() << "threads for python plugins.";
}

void PythonIntegrationPlugin::deinitPython()
{
    PyEval_RestoreThread(s_mainThreadState);

    Py_FinalizeEx();

    s_threadPool->deleteLater();
    s_threadPool = nullptr;
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
    setMetaData(PluginMetadata(jsonDoc.object()));
    if (!metadata().isValid()) {
        qCWarning(dcThingManager()) << "Plugin metadata not valid for plugin:" << scriptFile;
        return false;
    }

    // Grab the main thread context and GIL
    PyEval_RestoreThread(s_mainThreadState);

    // Create a new interpreter
    m_threadState = Py_NewInterpreter();

    // Switch to the new interpreter thread state
    PyThreadState_Swap(m_threadState);

    // Import nymea module into this interpreter
    m_nymeaModule = PyImport_ImportModule("nymea");

    // Set up import paths for the plugin
    PyObject* sysPath = PySys_GetObject("path");
    PyObject* pluginImportPath = PyUnicode_FromString(fi.absolutePath().toUtf8());
    PyList_Append(sysPath, pluginImportPath);
    Py_DECREF(pluginImportPath);

    PyObject* pluginModulesImportPath = PyUnicode_FromString(QString("%1/modules/").arg(fi.absolutePath()).toUtf8());
    PyList_Append(sysPath, pluginModulesImportPath);
    Py_DECREF(pluginModulesImportPath);

    m_module = PyImport_ImportModule(fi.baseName().toUtf8());

    if (!m_module) {
        qCWarning(dcThingManager()) << "Error importing python plugin from:" << fi.absoluteFilePath();
        PyErr_Print();
        PyErr_Clear();
        PyEval_ReleaseThread(m_threadState);
        return false;
    }
    qCDebug(dcThingManager()) << "Imported python plugin from" << fi.absoluteFilePath();

    s_plugins.insert(this, m_module);

    // We'll be using asyncio everywhere, so let's import it right away
    // IMPORTANT: The asyncio module is a bit special in a sense that it actually shares
    // stuff between interpreters. See https://docs.python.org/3/c-api/init.html#c.Py_NewInterpreter
    // for the listed bugs and caveats.
    // If we destroy the first interpreter that imports asyncio, things will become crashy as the
    // interpreter will tear down the module, leaving shallow copies of the modules dict in other
    // interpreters. So let's import the module down here after we're sure this module compiles and
    // won't be unloaded any more.
    // Note: This becomes a problem when we'll be supporting to unload plugins at runtime.
    m_asyncio = PyImport_ImportModule("asyncio");

    // Set up logger with appropriate logging category
    PyNymeaLoggingHandler *logger = reinterpret_cast<PyNymeaLoggingHandler*>(_PyObject_New(&PyNymeaLoggingHandlerType));
    QString category = metadata().pluginName();
    category.replace(0, 1, category[0].toUpper());
    logger->category = static_cast<char*>(malloc(category.length() + 1));
    memset(logger->category, '0', category.length() +1);
    strcpy(logger->category, category.toUtf8().data());
    PyModule_AddObject(m_module, "logger", reinterpret_cast<PyObject*>(logger));


    // Export metadata ids into module
    exportIds();

    // Register config access methods
    PyModule_AddFunctions(m_module, plugin_methods);

    PyEval_ReleaseThread(m_threadState);

    // Set up connections to be forwareded into the plugin
    connect(this, &PythonIntegrationPlugin::configValueChanged, this, [this](const ParamTypeId &paramTypeId, const QVariant &value){
        // Sync changed value to the thread-safe copy
        m_mutex.lock();
        m_pluginConfigCopy.setParamValue(paramTypeId, value);
        m_mutex.unlock();

        // And call the handler - if any
        PyObject *pyParamTypeId = PyUnicode_FromString(paramTypeId.toString().toUtf8());
        PyObject *pyValue = QVariantToPyObject(value);
        callPluginFunction("configValueChanged", pyParamTypeId, pyValue);
        Py_DECREF(pyParamTypeId);
        Py_DECREF(pyValue);
    });

    return true;
}

void PythonIntegrationPlugin::init()
{
    m_mutex.lock();
    m_pluginConfigCopy = configuration();
    m_mutex.unlock();

    callPluginFunction("init");
}

void PythonIntegrationPlugin::startMonitoringAutoThings()
{
    callPluginFunction("startMonitoringAutoThings");
}

void PythonIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
    PyEval_RestoreThread(m_threadState);

    PyThingDiscoveryInfo *pyInfo = (PyThingDiscoveryInfo*)PyObject_CallObject((PyObject*)&PyThingDiscoveryInfoType, NULL);
    PyThingDiscoveryInfo_setInfo(pyInfo, info);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &ThingDiscoveryInfo::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyInfo->info = nullptr;
        Py_DECREF(pyInfo);
        PyEval_ReleaseThread(m_threadState);
    });

    callPluginFunction("discoverThings", reinterpret_cast<PyObject*>(pyInfo));
}

void PythonIntegrationPlugin::startPairing(ThingPairingInfo *info)
{
    PyEval_RestoreThread(m_threadState);

    PyThingPairingInfo *pyInfo = (PyThingPairingInfo*)PyObject_CallObject((PyObject*)&PyThingPairingInfoType, nullptr);
    PyThingPairingInfo_setInfo(pyInfo, info);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &ThingPairingInfo::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyInfo->info = nullptr;
        Py_DECREF(pyInfo);
        PyEval_ReleaseThread(m_threadState);
    });

    bool result = callPluginFunction("startPairing", reinterpret_cast<PyObject*>(pyInfo));
    if (!result) {
        info->finish(Thing::ThingErrorHardwareFailure, "Plugin error: " + pluginName());
    }
}

void PythonIntegrationPlugin::confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret)
{
    PyEval_RestoreThread(m_threadState);

    PyThingPairingInfo *pyInfo = (PyThingPairingInfo*)PyObject_CallObject((PyObject*)&PyThingPairingInfoType, nullptr);
    PyThingPairingInfo_setInfo(pyInfo, info);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &ThingPairingInfo::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyInfo->info = nullptr;
        Py_DECREF(pyInfo);
        PyEval_ReleaseThread(m_threadState);
    });

    PyObject *pyUsername = PyUnicode_FromString(username.toUtf8().data());
    PyObject *pySecret = PyUnicode_FromString(secret.toUtf8().data());
    bool result = callPluginFunction("confirmPairing", reinterpret_cast<PyObject*>(pyInfo), pyUsername, pySecret);
    if (!result) {
        info->finish(Thing::ThingErrorHardwareFailure, "Plugin error: " + pluginName());
    }

    Py_DECREF(pyUsername);
    Py_DECREF(pySecret);
}

void PythonIntegrationPlugin::setupThing(ThingSetupInfo *info)
{
    PyEval_RestoreThread(m_threadState);

    PyThing *pyThing = nullptr;
    if (m_things.contains(info->thing())) {
        pyThing = m_things.value(info->thing());
    } else {
        pyThing = (PyThing*)PyObject_CallObject((PyObject*)&PyThingType, NULL);
        PyThing_setThing(pyThing, info->thing());
        m_things.insert(info->thing(), pyThing);
    }

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, (PyObject*)pyThing);

    PyThingSetupInfo *pyInfo = (PyThingSetupInfo*)PyObject_CallObject((PyObject*)&PyThingSetupInfoType, args);
    Py_DECREF(args);

    pyInfo->info = info;

    PyEval_ReleaseThread(m_threadState);

    connect(info->thing(), &Thing::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyThing->thing = nullptr;
        Py_DECREF(pyThing);
        PyEval_ReleaseThread(m_threadState);
    });
    connect(info, &ThingSetupInfo::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyInfo->info = nullptr;
        Py_DECREF(pyInfo);
        PyEval_ReleaseThread(m_threadState);
    });


    bool result = callPluginFunction("setupThing", reinterpret_cast<PyObject*>(pyInfo));
    if (!result) {
        // The python code did not even start, so let's finish (fail) the setup right away
        info->finish(Thing::ThingErrorSetupFailed);
    }
}

void PythonIntegrationPlugin::postSetupThing(Thing *thing)
{
    PyThing* pyThing = m_things.value(thing);
    Py_INCREF(pyThing);

    bool success = callPluginFunction("postSetupThing", reinterpret_cast<PyObject*>(pyThing));
    if (!success) {
        Py_DECREF(pyThing);
    }
}

void PythonIntegrationPlugin::executeAction(ThingActionInfo *info)
{
    PyThing *pyThing = m_things.value(info->thing());

    PyEval_RestoreThread(m_threadState);

    PyThingActionInfo *pyInfo = (PyThingActionInfo*)PyObject_CallObject((PyObject*)&PyThingActionInfoType, NULL);
    PyThingActionInfo_setInfo(pyInfo, info, pyThing);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &ThingActionInfo::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyInfo->info = nullptr;
        Py_DECREF(pyInfo);
        PyEval_ReleaseThread(m_threadState);
    });

    bool success = callPluginFunction("executeAction", reinterpret_cast<PyObject*>(pyInfo));
    if (!success) {
        info->finish(Thing::ThingErrorUnsupportedFeature);
    }
}

void PythonIntegrationPlugin::thingRemoved(Thing *thing)
{
    PyThing *pyThing = m_things.value(thing);
    Py_INCREF(pyThing);

    bool success = callPluginFunction("thingRemoved", reinterpret_cast<PyObject*>(pyThing));
    if (!success) {
        Py_DECREF(pyThing);
    }

    m_mutex.lock();
    m_things.remove(thing);
    m_mutex.unlock();
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
    QString pluginName = metadata().pluginName();
    QString pluginId = metadata().pluginId().toString();
    qCDebug(dcThingManager()) << "- Plugin:" << pluginName << pluginId;
    PyModule_AddStringConstant(m_module, "pluginId", pluginId.toUtf8());

    exportParamTypes(configurationDescription(), pluginName, "", "plugin");

    foreach (const Vendor &vendor, supportedVendors()) {
        qCDebug(dcThingManager()) << "|- Vendor:" << vendor.name() << vendor.id().toString();
        PyModule_AddStringConstant(m_module, QString("%1VendorId").arg(vendor.name()).toUtf8(), vendor.id().toString().toUtf8());
    }

    foreach (const ThingClass &thingClass, supportedThings()) {
        exportThingClass(thingClass);
    }
}

void PythonIntegrationPlugin::exportThingClass(const ThingClass &thingClass)
{
    QString variableName = QString("%1ThingClassId").arg(thingClass.name());

    qCDebug(dcThingManager()) << "|- ThingClass:" << variableName << thingClass.id().toString();
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
        qCDebug(dcThingManager()) << "  |- ParamType:" << variableName << paramType.id().toString();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), paramType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName)
{
    foreach (const StateType &stateType, stateTypes) {
        QString variableName = QString("%1%2StateTypeId").arg(thingClassName, stateType.name()[0].toUpper() + stateType.name().right(stateType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- StateType:" << variableName << stateType.id().toString();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), stateType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName)
{
    foreach (const EventType &eventType, eventTypes) {
        QString variableName = QString("%1%2EventTypeId").arg(thingClassName, eventType.name()[0].toUpper() + eventType.name().right(eventType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- EventType:" << variableName << eventType.id().toString();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), eventType.id().toString().toUtf8());
        exportParamTypes(eventType.paramTypes(), thingClassName, "Event", eventType.name());
    }

}

void PythonIntegrationPlugin::exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2ActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- ActionType:" << variableName << actionType.id().toString();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), actionType.id().toString().toUtf8());
        exportParamTypes(actionType.paramTypes(), thingClassName, "Action", actionType.name());
    }
}

void PythonIntegrationPlugin::exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2BrowserItemActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- BrowserActionType:" << variableName << actionType.id().toString();
        PyModule_AddStringConstant(m_module, variableName.toUtf8(), actionType.id().toString().toUtf8());
        exportParamTypes(actionType.paramTypes(), thingClassName, "BrowserItemAction", actionType.name());
    }

}


bool PythonIntegrationPlugin::callPluginFunction(const QString &function, PyObject *param1, PyObject *param2, PyObject *param3)
{
    PyEval_RestoreThread(m_threadState);

    qCDebug(dcThingManager()) << "Calling python plugin function" << function << "on plugin" << pluginName();
    PyObject *pluginFunction = PyObject_GetAttrString(m_module, function.toUtf8());
    if(!pluginFunction || !PyCallable_Check(pluginFunction)) {
        PyErr_Clear();
        Py_XDECREF(pluginFunction);
        qCDebug(dcThingManager()) << "Python plugin" << pluginName() << "does not implement" << function;
        PyEval_ReleaseThread(m_threadState);
        return false;
    }


    PyObject *pluginFunctionResult = PyObject_CallFunctionObjArgs(pluginFunction, param1, param2, param3, nullptr);

    Py_XDECREF(pluginFunction);

    if (PyErr_Occurred()) {
        qCWarning(dcThingManager()) << "Error calling python method:" << function << "on plugin" << pluginName();
        PyErr_Print();
        PyErr_Clear();
        PyEval_ReleaseThread(m_threadState);
        return false;
    }

    if (QByteArray(pluginFunctionResult->ob_type->tp_name) != "coroutine") {
        Py_DECREF(pluginFunctionResult);
        PyEval_ReleaseThread(m_threadState);
        return true;
    }

    // Spawn a new event loop for the thread
    QFuture<void> future = QtConcurrent::run([this, pluginFunctionResult, function](){
        qCDebug(dcPythonIntegrations()) << "Spawning thread for" << function << "in plugin" << metadata().pluginName();

        // Register this new thread in the interpreter
        PyThreadState *thread = PyThreadState_New(m_threadState->interp);

        // Acquire GIL and make the new thread state the current one
        PyEval_RestoreThread(thread);

        PyObject *new_event_loop = PyObject_GetAttrString(m_asyncio, "new_event_loop");
        PyObject *loop = PyObject_CallFunctionObjArgs(new_event_loop, nullptr);

        Py_DECREF(new_event_loop);

        PyObject *create_task = PyObject_GetAttrString(loop, "create_task");
        PyObject *task = PyObject_CallFunctionObjArgs(create_task, pluginFunctionResult, nullptr);
        dumpError();

        Py_DECREF(create_task);
        Py_DECREF(pluginFunctionResult);

        PyObject *add_done_callback = PyObject_GetAttrString(task, "add_done_callback");
        dumpError();

        PyObject *task_done = PyObject_GetAttrString(m_nymeaModule, "task_done");
        PyObject *coroutineResult = PyObject_CallFunctionObjArgs(add_done_callback, task_done, nullptr);
        dumpError();

        Py_DECREF(coroutineResult);
        Py_DECREF(add_done_callback);
        Py_DECREF(task_done);

        PyObject *run_until_complete = PyObject_GetAttrString(loop, "run_until_complete");

        Py_DECREF(loop);

        PyObject *taskResult = PyObject_CallFunctionObjArgs(run_until_complete, task, nullptr);
        dumpError();

        Py_XDECREF(taskResult);
        Py_DECREF(run_until_complete);
        Py_DECREF(task);

        // Destroy the thread and release the GIL
        PyThreadState_Clear(thread);
        PyThreadState_DeleteCurrent();
        qCDebug(dcPythonIntegrations()) << "Thread for" << function << "in plugin" << metadata().pluginName() << "ended";
    });

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    watcher->setFuture(future);
    m_runningTasks.insert(watcher);

    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher](){
        m_runningTasks.remove(watcher);
        delete watcher;
    });


    PyEval_ReleaseThread(m_threadState);
    return true;
}

