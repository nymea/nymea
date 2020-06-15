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





    // Spawn a event loop for python
    s_asyncio = PyImport_ImportModule("asyncio");
    PyObject *get_event_loop = PyObject_GetAttrString(s_asyncio, "get_event_loop");
    PyObject *loop = PyObject_CallFunctionObjArgs(get_event_loop, nullptr);
    PyObject *run_forever = PyObject_GetAttrString(loop, "run_forever");

    // Need to release ths lock from the main thread before spawning the new thread
    s_mainThread = PyEval_SaveThread();

    QtConcurrent::run([=](){
        PyGILState_STATE s = PyGILState_Ensure();
        PyObject_CallFunctionObjArgs(run_forever, nullptr);
        PyGILState_Release(s);
    });

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
    m_metaData = jsonDoc.toVariant().toMap();


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
    QString category = m_metaData.value("name").toString();
    category = category.left(1).toUpper() + category.right(category.length() - 1);
    logger->category = static_cast<char*>(malloc(category.length() + 1));
    memset(logger->category, '0', category.length() +1);
    strcpy(logger->category, category.toUtf8().data());
    PyModule_AddObject(m_module, "logger", reinterpret_cast<PyObject*>(logger));


    // Export metadata ids into module
    exportIds();

    PyGILState_Release(s);
    return true;
}

QJsonObject PythonIntegrationPlugin::metaData() const
{
    return QJsonObject::fromVariantMap(m_metaData);
}

void PythonIntegrationPlugin::init()
{
    callPluginFunction("init", nullptr);
}

void PythonIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
    PyThingDiscoveryInfo *pyInfo = reinterpret_cast<PyThingDiscoveryInfo*>(_PyObject_New(&PyThingDiscoveryInfoType));
    pyInfo->ptrObj = info;

    connect(info, &ThingDiscoveryInfo::finished, this, [=](){
        PyObject_Free(pyInfo);
    });

    callPluginFunction("discoverThings", reinterpret_cast<PyObject*>(pyInfo));
}

void PythonIntegrationPlugin::setupThing(ThingSetupInfo *info)
{
    PyThing *pyThing = reinterpret_cast<PyThing*>(_PyObject_New(&PyThingType));
    pyThing->ptrObj = info->thing();
    m_things.insert(info->thing(), pyThing);
    Py_INCREF(pyThing);

    PyThingSetupInfo *pyInfo = reinterpret_cast<PyThingSetupInfo*>(_PyObject_New(&PyThingSetupInfoType));
    pyInfo->ptrObj = info;
    pyInfo->thing = pyThing;

    connect(info, &ThingSetupInfo::finished, this, [=](){
        PyObject_Free(pyInfo);
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

    pyThing->ptrObj = nullptr;
    Py_DECREF(pyThing);

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
    foreach (const QVariant &vendorVariant, m_metaData.value("vendors").toList()) {
        QVariantMap vendor = vendorVariant.toMap();
        QString vendorIdName = vendor.value("name").toString() + "VendorId";
        QString vendorId = vendor.value("id").toString();
        PyModule_AddStringConstant(m_module, vendorIdName.toUtf8(), vendorId.toUtf8());

        foreach (const QVariant &thingClassVariant, vendor.value("thingClasses").toList()) {
            QVariantMap thingClass = thingClassVariant.toMap();
            QString thingClassIdName = thingClass.value("name").toString() + "ThingClassId";
            QString thingClassId = thingClass.value("id").toString();
            PyModule_AddStringConstant(m_module, thingClassIdName.toUtf8(), thingClassId.toUtf8());
        }
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

    PyObject *get_event_loop = PyObject_GetAttrString(s_asyncio, "get_event_loop");
    PyObject *loop = PyObject_CallFunctionObjArgs(get_event_loop, nullptr);

    PyObject *run_coroutine_threadsafe = PyObject_GetAttrString(s_asyncio, "run_coroutine_threadsafe");
    PyObject *task = PyObject_CallFunctionObjArgs(run_coroutine_threadsafe, future, loop, nullptr);
    dumpError();

    PyObject *add_done_callback = PyObject_GetAttrString(task, "add_done_callback");
    dumpError();

    PyObject *task_done = PyObject_GetAttrString(s_nymeaModule, "task_done");
    PyObject *result = PyObject_CallFunctionObjArgs(add_done_callback, task_done, nullptr);
    dumpError();

    Py_DECREF(get_event_loop);
    Py_DECREF(loop);
    Py_DECREF(run_coroutine_threadsafe);
    Py_DECREF(task);
    Py_DECREF(add_done_callback);
    Py_DECREF(get_event_loop);
    Py_DECREF(result);

    PyGILState_Release(s);
}

