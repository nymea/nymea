#include <Python.h>
#include "python/pythingdiscoveryinfo.h"

#include "pythonintegrationplugin.h"

#include "loggingcategories.h"

#include <QFileInfo>
#include <QMetaEnum>
#include <QJsonDocument>

QHash<PyObject*, PyThreadState*> s_modules;

// Write to stdout/stderr
PyObject* nymea_write(PyObject* self, PyObject* args)
{
    qWarning() << "write called" << self;
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return nullptr;
    qCDebug(dcThingManager()) << what;
    return Py_BuildValue("");
}

// Flush stdout/stderr
PyObject* nymea_flush(PyObject* /*self*/, PyObject* /*args*/)
{
    // Not really needed... qDebug() fluses already on its own
    return Py_BuildValue("");
}

static PyMethodDef nymea_methods[] =
{
    {"write", nymea_write, METH_VARARGS, "write to stdout through qDebug()"},
    {"flush", nymea_flush, METH_VARARGS, "flush stdout (no-op)"},
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


    QMetaEnum thingErrorEnum = QMetaEnum::fromType<Thing::ThingError>();
    for (int i = 0; i < thingErrorEnum.keyCount(); i++) {
        PyModule_AddObject(m, thingErrorEnum.key(i), PyLong_FromLong(thingErrorEnum.value(i)));
    }


    PyThingDiscoveryInfoType.tp_new = PyType_GenericNew;
    PyThingDiscoveryInfoType.tp_basicsize=sizeof(PyThingDiscoveryInfo);
    PyThingDiscoveryInfoType.tp_dealloc=(destructor) PyThingDiscoveryInfo_dealloc;
    PyThingDiscoveryInfoType.tp_flags=Py_TPFLAGS_DEFAULT;
    PyThingDiscoveryInfoType.tp_doc="ThingDiscoveryInfo class";
    PyThingDiscoveryInfoType.tp_methods=PyThingDiscoveryInfo_methods;
    PyThingDiscoveryInfoType.tp_init=(initproc)PyThingDiscoveryInfo_init;

    if (PyType_Ready(&PyThingDiscoveryInfoType) < 0) {
        qCWarning(dcThingManager()) << "Error creating PyThingDiscoveryInfo";
        return nullptr;
    }
    PyModule_AddObject(m, "ThingDiscoveryInfo", (PyObject *)&PyThingDiscoveryInfoType);

    PyThingDescriptorType.tp_new = PyType_GenericNew;
    PyThingDescriptorType.tp_basicsize = sizeof(PyThingDescriptor);
    PyThingDescriptorType.tp_flags = Py_TPFLAGS_DEFAULT;
    PyThingDescriptorType.tp_doc = "ThingDescriptor class";
    PyThingDescriptorType.tp_methods = PyThingDescriptor_methods;
    PyThingDescriptorType.tp_members = PyThingDescriptor_members;
    PyThingDescriptorType.tp_init = (initproc)PyThingDescriptor_init;

    if (PyType_Ready(&PyThingDescriptorType) < 0) {
        qCWarning(dcThingManager()) << "Error creating PyThingDescriptor";
        return nullptr;
    }
    PyModule_AddObject(m, "ThingDescriptor", (PyObject *)&PyThingDescriptorType);


    return m;
}

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

void PythonIntegrationPlugin::initPython()
{
    // TODO: Call this just once and call Py_Finalize()
    PyImport_AppendInittab("nymea", PyInit_nymea);
    Py_InitializeEx(0);

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


    // Create a new sub-interpreter for this plugin
    m_interpreter = Py_NewInterpreter();
    PyThreadState_Swap(m_interpreter);


    // Import nymea module into this interpreter
    PyImport_ImportModule("nymea");

    // Add this plugin's locatioon to the import path
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString(fi.absolutePath().toUtf8()));

    // Finally, import the plugin
    m_module = PyImport_ImportModule(fi.baseName().toUtf8());

    if (!m_module) {
        dumpError();
        qCWarning(dcThingManager()) << "Error importing python plugin from:" << fi.absoluteFilePath();
        return false;
    }
    qCDebug(dcThingManager()) << "Imported python plugin from" << fi.absoluteFilePath();


    exportIds();


    s_modules.insert(m_module, m_interpreter);

    return true;
}

QJsonObject PythonIntegrationPlugin::metaData() const
{
    return QJsonObject::fromVariantMap(m_metaData);

}

void PythonIntegrationPlugin::init()
{
    PyThreadState_Swap(m_interpreter);

    qCDebug(dcThingManager()) << "Python wrapper: init()" << m_interpreter;
    PyObject *pFunc = PyObject_GetAttrString(m_module, "init");
    if(!pFunc || !PyCallable_Check(pFunc)) {
        qCDebug(dcThingManager()) << "Python plugin does not implement \"init()\" method.";
        return;
    }
    PyObject_CallObject(pFunc, nullptr);
    dumpError();
}

void PythonIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
    PyThreadState_Swap(m_interpreter);

    qCDebug(dcThingManager()) << "Python wrapper: discoverThings()" << info;
    PyObject *pFunc = PyObject_GetAttrString(m_module, "discoverThings");
    if(!pFunc || !PyCallable_Check(pFunc)) {
        qCWarning(dcThingManager()) << "Python plugin does not implement \"discoverThings()\" method.";
        return;
    }

    PyThingDiscoveryInfo *pyInfo = (PyThingDiscoveryInfo*)_PyObject_New(&PyThingDiscoveryInfoType);
    pyInfo->ptrObj = info;

    connect(info, &ThingDiscoveryInfo::finished, this, [=](){
        PyObject_Free(pyInfo);
    });


//    PyObject *loop = PyObject_GetAttrString(m_module, "_loop");
//    dumpError();
//    qWarning() << "loop:" << loop;


    PyObject *future = PyObject_CallFunctionObjArgs(pFunc, pyInfo, nullptr);
    dumpError();

    qWarning() << "future:" << future;

    PyObject *asyncio = PyObject_GetAttrString(m_module, "asyncio");
    qWarning() << "ayncio:" << asyncio;

    PyObject *loop = PyObject_GetAttrString(m_module, "loop");
    PyObject *create_task = PyObject_GetAttrString(loop, "create_task");
    PyObject *task = PyObject_CallFunctionObjArgs(create_task, future, nullptr);

    dumpError();
    qWarning() << "Create task:" << task;

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
        qCDebug(dcThingManager()) << "Exporting:" << vendorIdName;
        PyModule_AddStringConstant(m_module, vendorIdName.toUtf8(), vendorId.toUtf8());

        foreach (const QVariant &thingClassVariant, vendor.value("thingClasses").toList()) {
            QVariantMap thingClass = thingClassVariant.toMap();
            QString thingClassIdName = thingClass.value("name").toString() + "ThingClassId";
            QString thingClassId = thingClass.value("id").toString();
            PyModule_AddStringConstant(m_module, thingClassIdName.toUtf8(), thingClassId.toUtf8());
        }
    }

}

