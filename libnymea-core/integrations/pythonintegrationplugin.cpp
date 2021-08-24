#include <Python.h>

#include "pythonintegrationplugin.h"
#include "python/pynymeamodule.h"
#include "python/pystdouthandler.h"
#include "python/pypluginstorage.h"
#include "python/pyapikeystorage.h"

#include "loggingcategories.h"

#include <QFileInfo>
#include <QMetaEnum>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>
#include <QCoreApplication>
#include <QMutex>
#include <QFuture>
#include <QFutureWatcher>

NYMEA_LOGGING_CATEGORY(dcPythonIntegrations, "PythonIntegrations")

PyThreadState* PythonIntegrationPlugin::s_mainThreadState = nullptr;
QHash<PythonIntegrationPlugin*, PyObject*> PythonIntegrationPlugin::s_plugins;

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
            Py_DECREF(next);
            continue;
        }
        PyThingDescriptor *pyDescriptor = (PyThingDescriptor*)next;

        ThingClassId thingClassId;
        if (pyDescriptor->pyThingClassId) {
            thingClassId = ThingClassId(PyUnicode_AsUTF8(pyDescriptor->pyThingClassId));
        }
        ThingId parentId;
        if (pyDescriptor->pyParentId) {
            parentId = ThingId(PyUnicode_AsUTF8(pyDescriptor->pyParentId));
        }
        QString name;
        if (pyDescriptor->pyName) {
            name = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyName));
        }
        QString description;
        if (pyDescriptor->pyDescription) {
            description = QString::fromUtf8(PyUnicode_AsUTF8(pyDescriptor->pyDescription));
        }

        ThingDescriptor descriptor(thingClassId, name, description, parentId);

        if (pyDescriptor->pyParams) {
            descriptor.setParams(PyParams_ToParamList(pyDescriptor->pyParams));
        }

        descriptors.append(descriptor);
        Py_DECREF(next);
    }

    PythonIntegrationPlugin *plugin = s_plugins.key(self);
    QMetaObject::invokeMethod(plugin, "autoThingsAppeared", Qt::QueuedConnection, Q_ARG(ThingDescriptors, descriptors));

    Py_DECREF(iter);
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

PyObject *PythonIntegrationPlugin::pyPluginStorage(PyObject *self, PyObject */*args*/)
{
    // Note: Passing the pluginsStorage() pointer directly into python. Implies that it must not be
    // accessed in the main thread without obtaining the GIL
    PyObject *pluginStorage = PyObject_CallObject((PyObject*)&PyPluginStorageType, nullptr);
    PyPluginStorage_setPluginStorage((PyPluginStorage*)pluginStorage, s_plugins.key(self)->pluginStorage());
    return pluginStorage;
}

PyObject *PythonIntegrationPlugin::pyApiKeyStorage(PyObject *self, PyObject *args)
{
    // Note: Passing the apiKeyStorage() pointer directly into python. Implies that it must not be
    // accessed in the main thread without obtaining the GIL
    PyObject *pyApiKeyStorage = PyObject_CallObject((PyObject*)&PyApiKeyStorageType, args);
    PyApiKeyStorage_setApiKeyStorage((PyApiKeyStorage*)pyApiKeyStorage, s_plugins.key(self)->apiKeyStorage());
    return pyApiKeyStorage;
}

static PyMethodDef plugin_methods[] =
{
    {"configuration", PythonIntegrationPlugin::pyConfiguration, METH_VARARGS, "Get the plugin configuration."},
    {"configValue", PythonIntegrationPlugin::pyConfigValue, METH_VARARGS, "Get the plugin configuration value for a given config paramTypeId."},
    {"setConfigValue", PythonIntegrationPlugin::pySetConfigValue, METH_VARARGS, "Set the plugin configuration value for a given config paramTypeId."},
    {"myThings", PythonIntegrationPlugin::pyMyThings, METH_VARARGS, "Obtain a list of things owned by this plugin."},
    {"autoThingsAppeared", PythonIntegrationPlugin::pyAutoThingsAppeared, METH_VARARGS, "Inform the system about auto setup things having appeared."},
    {"autoThingDisappeared", PythonIntegrationPlugin::pyAutoThingDisappeared, METH_VARARGS, "Inform the system about auto setup things having disappeared."},
    {"pluginStorage", PythonIntegrationPlugin::pyPluginStorage, METH_VARARGS, "Obtain the plugin storage for this plugin."},
    {"apiKeyStorage", PythonIntegrationPlugin::pyApiKeyStorage, METH_VARARGS, "Obtain the API key storage for this plugin."},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

PythonIntegrationPlugin::~PythonIntegrationPlugin()
{
    if (m_pluginModule) {
        callPluginFunction("deinit");
    }

    // Acquire GIL for this plugin's interpreter
    PyEval_RestoreThread(m_threadState);

    if (m_pluginModule) {
        while (!m_runningTasks.isEmpty()) {
            QFutureWatcher<void> *watcher = m_runningTasks.keys().first();
            QString function = m_runningTasks.value(watcher);

            Py_BEGIN_ALLOW_THREADS
            qCDebug(dcPythonIntegrations()) << "Waiting for" << metadata().pluginName() << "to finish" << function;
            watcher->waitForFinished();
            Py_END_ALLOW_THREADS
        }
    }

    s_plugins.take(this);
    Py_XDECREF(m_pluginModule);
    Py_DECREF(m_nymeaModule);
    Py_XDECREF(m_logger);
    Py_XDECREF(m_stdOutHandler);
    Py_XDECREF(m_stdErrHandler);

    Py_EndInterpreter(m_threadState);

    PyThreadState_Swap(s_mainThreadState);
    PyEval_ReleaseThread(s_mainThreadState);
}

void PythonIntegrationPlugin::initPython()
{
    Q_ASSERT_X(s_mainThreadState == nullptr, "PythonIntegrationPlugin::initPython()", "initPython() must be called exactly once before calling deinitPython().");

    // Only modify the init tab once (initPython() might be called again after calling deinitPython())
    static bool initTabPrepared = false;
    if (!initTabPrepared) {
        PyImport_AppendInittab("nymea", PyInit_nymea);
        initTabPrepared = true;
    }

    // Initialize the python engine and fire up threading support
    Py_InitializeEx(0);
    PyEval_InitThreads();

    // Store the main thread state and release the GIL
    s_mainThreadState = PyEval_SaveThread();
}

void PythonIntegrationPlugin::deinitPython()
{
    // Restore our main thread state
    PyEval_RestoreThread(s_mainThreadState);

    // Tear down the python engine
    Py_Finalize();

    // Our main thread state is destroyed now
    s_mainThreadState = nullptr;
}

bool PythonIntegrationPlugin::loadScript(const QString &scriptFile)
{
    // Grab the main thread context and GIL
    PyEval_RestoreThread(s_mainThreadState);

    // Create a new interpreter
    m_threadState = Py_NewInterpreter();

    // Switch to the new interpreter thread state
    PyThreadState_Swap(m_threadState);

    // Import nymea module into this interpreter
    m_nymeaModule = PyImport_ImportModule("nymea");

    QFileInfo fi(scriptFile);

    QFile metaDataFile(fi.absolutePath() + "/" + fi.baseName() + ".json");
    if (!metaDataFile.open(QFile::ReadOnly)) {
        qCWarning(dcPythonIntegrations()) << "Error opening metadata file:" << metaDataFile.fileName();
        PyEval_ReleaseThread(m_threadState);
        return false;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(metaDataFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPythonIntegrations()) << "Error parsing metadata file:" << error.errorString();
        PyEval_ReleaseThread(m_threadState);
        return false;
    }
    PluginMetadata metadata(jsonDoc.object());
    if (!metadata.isValid()) {
        qCWarning(dcPythonIntegrations()) << "Plugin metadata not valid for plugin:" << scriptFile;
        foreach (const QString &error, metadata.validationErrors()) {
            qCWarning(dcThingManager()) << error;
        }
        PyEval_ReleaseThread(m_threadState);
        return false;
    }
    setMetaData(metadata);

    // Set up import path for the plugin directory
    // We intentionally strip site-packages and dist-packages because
    // that's too unpredictive in distribution. Instead all dependencies
    // should be installed into the plugins "modules" subdir.
    PyObject* sysPath = PySys_GetObject("path");
    QStringList importPaths;
    for (int i = 0; i < PyList_Size(sysPath); i++) {
        QString path = QString::fromUtf8(PyUnicode_AsUTF8(PyList_GetItem(sysPath, i)));
        if (!path.contains("site-packages") && !path.contains("dist-packages")) {
            importPaths.append(path);
        }
    }
    importPaths.append(fi.absolutePath());
    importPaths.append(QString("%1/modules/").arg(fi.absolutePath()));

    PyObject* pluginPaths = PyList_New(importPaths.length());
    for (int i = 0; i < importPaths.length(); i++) {
        const QString &path = importPaths.at(i);
        PyObject *pyPath = PyUnicode_FromString(path.toUtf8());
        PyList_SetItem(pluginPaths, i, pyPath);
    }
    PySys_SetObject("path", pluginPaths);

    // Import the plugin
    m_pluginModule = PyImport_ImportModule(fi.baseName().toUtf8());

    if (!m_pluginModule) {
        qCWarning(dcThingManager()) << "Error importing python plugin from:" << fi.absoluteFilePath();
        PyErr_Print();
        PyErr_Clear();
        PyEval_ReleaseThread(m_threadState);
        return false;
    }
    qCDebug(dcThingManager()) << "Imported python plugin from" << fi.absoluteFilePath();

    s_plugins.insert(this, m_pluginModule);

    // Set up logger with appropriate logging category
    QString category = metadata.pluginName();
    category.replace(0, 1, category[0].toUpper());
    PyObject *args = Py_BuildValue("(s)", category.toUtf8().data());
    m_logger = PyObject_CallObject((PyObject*)&PyNymeaLoggingHandlerType, args);
    Py_DECREF(args);

    // Override stdout and stderr
    args = Py_BuildValue("(si)", category.toUtf8().data(), QtMsgType::QtDebugMsg);
    m_stdOutHandler = PyObject_CallObject((PyObject*)&PyStdOutHandlerType, args);
    Py_DECREF(args);
    PySys_SetObject("stdout", m_stdOutHandler);
    args = Py_BuildValue("(si)", category.toUtf8().data(), QtMsgType::QtWarningMsg);
    m_stdErrHandler = PyObject_CallObject((PyObject*)&PyStdOutHandlerType, args);
    PySys_SetObject("stderr", m_stdErrHandler);
    Py_DECREF(args);

    int loggerAdded = PyModule_AddObject(m_pluginModule, "logger", m_logger);
    if (loggerAdded != 0) {
        qCWarning(dcPythonIntegrations()) << "Failed to add the logger object";
        Py_DECREF(m_logger);
        m_logger = nullptr;
    }

    // Export metadata ids into module
    exportIds();

    // Register plugin api methods (plugin params etc)
    PyModule_AddFunctions(m_pluginModule, plugin_methods);

    // As python does not have an event loop by default and uses blocking code a lot, we'll
    // call every plugin method in a threaded way to prevent blocking the core while still not
    // forcing every plugin developer to deal with threading in the plugin.
    // In oder to not create and destroy a thread for each plugin api call, we'll be using a
    // thread pool.
    // The maximum number of threads in a plugin will be amount of things it manages + 2.
    // This would allow for e.g. running an event loop using init(), performing something on a thing
    // and still allow the user to perform a discovery at the same time. On the other hand, this is
    // strict enough to not encourage the plugin developer to block forever in ever api call but use
    // proper task processing means (timers, event loops etc) instead.
    // Plugins can still spawn more threads on their own if the need to but have to manage them on their own.
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(2);
    qCDebug(dcPythonIntegrations()) << "Created a thread pool with a maximum of" << m_threadPool->maxThreadCount() << "threads for python plugin" << metadata.pluginName();

    PyEval_ReleaseThread(m_threadState);

    // Set up connections to be forwareded into the plugin
    connect(this, &PythonIntegrationPlugin::configValueChanged, this, [this](const ParamTypeId &paramTypeId, const QVariant &value){
        // Sync changed value to the thread-safe copy
        m_mutex.lock();
        m_pluginConfigCopy.setParamValue(paramTypeId, value);
        m_mutex.unlock();

        // And call the handler - if any
        PyEval_RestoreThread(m_threadState);
        PyObject *pyParamTypeId = PyUnicode_FromString(paramTypeId.toString().toUtf8());
        PyObject *pyValue = QVariantToPyObject(value);
        PyEval_ReleaseThread(m_threadState);
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

    Thing *thing = info->thing();

    PyThing *pyThing = nullptr;
    if (m_things.contains(thing)) {
        pyThing = m_things.value(thing);
    } else {
        pyThing = (PyThing*)PyObject_CallObject((PyObject*)&PyThingType, NULL);
        PyThing_setThing(pyThing, thing, m_threadState);
        m_things.insert(thing, pyThing);
    }

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, (PyObject*)pyThing);
    Py_INCREF(pyThing);

    PyThingSetupInfo *pyInfo = (PyThingSetupInfo*)PyObject_CallObject((PyObject*)&PyThingSetupInfoType, args);
    Py_DECREF(args);

    pyInfo->info = info;

    m_threadPool->setMaxThreadCount(m_threadPool->maxThreadCount() + 1);
    qCDebug(dcPythonIntegrations()) << "Expanded thread pool for plugin" << metadata().pluginName() << "to" << m_threadPool->maxThreadCount();

    PyEval_ReleaseThread(m_threadState);

    connect(info->thing(), &Thing::destroyed, this, [=](){
        PyEval_RestoreThread(m_threadState);
        pyThing->thing = nullptr;
        Py_DECREF(pyThing);
        m_threadPool->setMaxThreadCount(m_threadPool->maxThreadCount() - 1);
        qCDebug(dcPythonIntegrations()) << "Shrunk thread pool for plugin" << metadata().pluginName() << "to" << m_threadPool->maxThreadCount();
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
    callPluginFunction("postSetupThing", reinterpret_cast<PyObject*>(pyThing));
}

void PythonIntegrationPlugin::executeAction(ThingActionInfo *info)
{
    PyThing *pyThing = m_things.value(info->thing());

    PyEval_RestoreThread(m_threadState);

    PyThingActionInfo *pyInfo = (PyThingActionInfo*)PyObject_CallObject((PyObject*)&PyThingActionInfoType, NULL);
    PyThingActionInfo_setInfo(pyInfo, info, pyThing);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &ThingActionInfo::destroyed, this, [=](){
        qCDebug(dcPythonIntegrations()) << "ThingActionInfo destroyed";
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
    PyThing *pyThing = m_things.take(thing);
    callPluginFunction("thingRemoved", reinterpret_cast<PyObject*>(pyThing));
}

void PythonIntegrationPlugin::browseThing(BrowseResult *result)
{
    PyThing *pyThing = m_things.value(result->thing());

    PyEval_RestoreThread(m_threadState);

    PyBrowseResult *pyBrowseResult = (PyBrowseResult*)PyObject_CallObject((PyObject*)&PyBrowseResultType, NULL);
    PyBrowseResult_setBrowseResult(pyBrowseResult, result, pyThing);

    PyEval_ReleaseThread(m_threadState);

    connect(result, &BrowseResult::destroyed, this, [=](){
        qCDebug(dcPythonIntegrations()) << "BrowseResult destroyed";
        PyEval_RestoreThread(m_threadState);
        pyBrowseResult->browseResult = nullptr;
        Py_DECREF(pyBrowseResult);
        PyEval_ReleaseThread(m_threadState);
    });

    bool success = callPluginFunction("browseThing", reinterpret_cast<PyObject*>(pyBrowseResult));
    if (!success) {
        result->finish(Thing::ThingErrorUnsupportedFeature);
    }
}

void PythonIntegrationPlugin::executeBrowserItem(BrowserActionInfo *info)
{
    PyThing *pyThing = m_things.value(info->thing());

    PyEval_RestoreThread(m_threadState);

    PyBrowserActionInfo *pyBrowserActionInfo = (PyBrowserActionInfo*)PyObject_CallObject((PyObject*)&PyBrowserActionInfoType, NULL);
    PyBrowserActionInfo_setInfo(pyBrowserActionInfo, info, pyThing);

    PyEval_ReleaseThread(m_threadState);

    connect(info, &BrowserActionInfo::destroyed, this, [=](){
        qCDebug(dcPythonIntegrations()) << "BrowserActionInfo destroyed";
        PyEval_RestoreThread(m_threadState);
        pyBrowserActionInfo->info = nullptr;
        Py_DECREF(pyBrowserActionInfo);
        PyEval_ReleaseThread(m_threadState);
    });

    bool success = callPluginFunction("executeBrowserItem", reinterpret_cast<PyObject*>(pyBrowserActionInfo));
    if (!success) {
        info->finish(Thing::ThingErrorUnsupportedFeature);
    }
}

void PythonIntegrationPlugin::browserItem(BrowserItemResult *result)
{
    PyThing *pyThing = m_things.value(result->thing());

    PyEval_RestoreThread(m_threadState);

    PyBrowserItemResult *pyBrowserItemResult = (PyBrowserItemResult*)PyObject_CallObject((PyObject*)&PyBrowserItemResultType, NULL);
    PyBrowserItemResult_setBrowserItemResult(pyBrowserItemResult, result, pyThing);

    PyEval_ReleaseThread(m_threadState);

    connect(result, &BrowserItemResult::destroyed, this, [=](){
        qCDebug(dcPythonIntegrations()) << "BrowseItemResult destroyed";
        PyEval_RestoreThread(m_threadState);
        pyBrowserItemResult->browserItemResult = nullptr;
        Py_DECREF(pyBrowserItemResult);
        PyEval_ReleaseThread(m_threadState);
    });

    bool success = callPluginFunction("browserItem", reinterpret_cast<PyObject*>(pyBrowserItemResult));
    if (!success) {
        result->finish(Thing::ThingErrorUnsupportedFeature);
    }
}

void PythonIntegrationPlugin::exportIds()
{
    qCDebug(dcThingManager()) << "Exporting plugin IDs:";
    QString pluginName = metadata().pluginName();
    QString pluginId = metadata().pluginId().toString();
    qCDebug(dcThingManager()) << "- Plugin:" << pluginName << pluginId;
    PyModule_AddStringConstant(m_pluginModule, "pluginId", pluginId.toUtf8());

    exportParamTypes(configurationDescription(), pluginName, "", "plugin");

    foreach (const Vendor &vendor, supportedVendors()) {
        qCDebug(dcThingManager()) << "|- Vendor:" << vendor.name() << vendor.id().toString();
        PyModule_AddStringConstant(m_pluginModule, QString("%1VendorId").arg(vendor.name()).toUtf8(), vendor.id().toString().toUtf8());
    }

    foreach (const ThingClass &thingClass, supportedThings()) {
        exportThingClass(thingClass);
    }
}

void PythonIntegrationPlugin::exportThingClass(const ThingClass &thingClass)
{
    QString variableName = QString("%1ThingClassId").arg(thingClass.name());

    qCDebug(dcThingManager()) << "|- ThingClass:" << variableName << thingClass.id().toString();
    PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), thingClass.id().toString().toUtf8());

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
        PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), paramType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName)
{
    foreach (const StateType &stateType, stateTypes) {
        QString variableName = QString("%1%2StateTypeId").arg(thingClassName, stateType.name()[0].toUpper() + stateType.name().right(stateType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- StateType:" << variableName << stateType.id().toString();
        PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), stateType.id().toString().toUtf8());
    }
}

void PythonIntegrationPlugin::exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName)
{
    foreach (const EventType &eventType, eventTypes) {
        QString variableName = QString("%1%2EventTypeId").arg(thingClassName, eventType.name()[0].toUpper() + eventType.name().right(eventType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- EventType:" << variableName << eventType.id().toString();
        PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), eventType.id().toString().toUtf8());
        exportParamTypes(eventType.paramTypes(), thingClassName, "Event", eventType.name());
    }
}

void PythonIntegrationPlugin::exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2ActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- ActionType:" << variableName << actionType.id().toString();
        PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), actionType.id().toString().toUtf8());
        exportParamTypes(actionType.paramTypes(), thingClassName, "Action", actionType.name());
    }
}

void PythonIntegrationPlugin::exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName)
{
    foreach (const ActionType &actionType, actionTypes) {
        QString variableName = QString("%1%2BrowserItemActionTypeId").arg(thingClassName, actionType.name()[0].toUpper() + actionType.name().right(actionType.name().length() - 1));
        qCDebug(dcThingManager()) << " |- BrowserActionType:" << variableName << actionType.id().toString();
        PyModule_AddStringConstant(m_pluginModule, variableName.toUtf8(), actionType.id().toString().toUtf8());
        exportParamTypes(actionType.paramTypes(), thingClassName, "BrowserItemAction", actionType.name());
    }
}

bool PythonIntegrationPlugin::callPluginFunction(const QString &function, PyObject *param1, PyObject *param2, PyObject *param3)
{
    PyEval_RestoreThread(m_threadState);

    qCDebug(dcThingManager()) << "Calling python plugin function" << function << "on plugin" << pluginName();
    PyObject *pluginFunction = PyObject_GetAttrString(m_pluginModule, function.toUtf8());
    if(!pluginFunction || !PyCallable_Check(pluginFunction)) {
        PyErr_Clear();
        Py_XDECREF(pluginFunction);
        qCDebug(dcThingManager()) << "Python plugin" << pluginName() << "does not implement" << function;
        PyEval_ReleaseThread(m_threadState);
        return false;
    }

    Py_XINCREF(param1);
    Py_XINCREF(param2);
    Py_XINCREF(param3);

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

    // Run the plugin function in the thread pool
    QFuture<void> future = QtConcurrent::run(m_threadPool, [=](){
        qCDebug(dcPythonIntegrations()) << "+++ Thread for" << function << "in plugin" << metadata().pluginName();

        // Register this new thread in the interpreter
        PyThreadState *threadState = PyThreadState_New(m_threadState->interp);

        // Acquire GIL and make the new thread state the current one
        PyEval_RestoreThread(threadState);

        PyObject *pluginFunctionResult = PyObject_CallFunctionObjArgs(pluginFunction, param1, param2, param3, nullptr);

        if (PyErr_Occurred()) {
            qCWarning(dcThingManager()) << "Error calling python method:" << function << "on plugin" << pluginName();
            PyErr_Print();
        }

        Py_DECREF(pluginFunction);
        Py_XDECREF(pluginFunctionResult);
        Py_XDECREF(param1);
        Py_XDECREF(param2);
        Py_XDECREF(param3);

        m_runningTasks.remove(watcher);

        // Destroy the thread and release the GIL
        PyThreadState_Clear(threadState);
        PyEval_ReleaseThread(threadState);
        PyThreadState_Delete(threadState);
        qCDebug(dcPythonIntegrations()) << "--- Thread for" << function << "in plugin" << metadata().pluginName();
    });
    watcher->setFuture(future);
    m_runningTasks.insert(watcher, function);

    PyEval_ReleaseThread(m_threadState);
    return true;
}

