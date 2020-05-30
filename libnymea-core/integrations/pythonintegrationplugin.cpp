#include <Python.h>
#include "python/PyThingDiscoveryInfo.h"

#include "pythonintegrationplugin.h"

#include "loggingcategories.h"

#include <QFileInfo>

// Write to stdout/stderr
PyObject* nymea_write(PyObject* /*self*/, PyObject* args)
{
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


    PyThingDiscoveryInfoType.tp_new = PyType_GenericNew;
    PyThingDiscoveryInfoType.tp_basicsize=sizeof(PyThingDiscoveryInfo);
    PyThingDiscoveryInfoType.tp_dealloc=(destructor) PyThingDiscoveryInfo_dealloc;
    PyThingDiscoveryInfoType.tp_flags=Py_TPFLAGS_DEFAULT;
    PyThingDiscoveryInfoType.tp_doc="ThingDiscoveryInfo class";
    PyThingDiscoveryInfoType.tp_methods=PyThingDiscoveryInfo_methods;
    //~ PyVoiceType.tp_members=Noddy_members;
    PyThingDiscoveryInfoType.tp_init=(initproc)PyThingDiscoveryInfo_init;

    if (PyType_Ready(&PyThingDiscoveryInfoType) < 0)
        return NULL;

    PyModule_AddObject(m, "ThingDiscoveryInfo", (PyObject *)&PyThingDiscoveryInfoType); // Add Voice object to the module

    return m;
}

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

bool PythonIntegrationPlugin::loadScript(const QString &scriptFile)
{
    // TODO: Call this just once and call Py_Finalize()
    PyImport_AppendInittab("nymea", PyInit_nymea);
    Py_Initialize();
    PyImport_ImportModule("nymea");

    QFileInfo fi(scriptFile);
    qCDebug(dcThingManager()) << "Importing" << fi.absolutePath() << fi.fileName() << fi.baseName();

    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString(fi.absolutePath().toUtf8()));


    PyObject *pName = PyUnicode_FromString(fi.baseName().toUtf8());
    qCDebug(dcThingManager()) << "Importing python plugin from" << scriptFile;
    m_module = PyImport_Import(pName);

    if (!m_module) {
        qCWarning(dcThingManager()) << "Error importing plugin";
        return false;
    }
    return true;
}

QJsonObject PythonIntegrationPlugin::metaData() const
{
    QVariantMap pluginMetaData;
    pluginMetaData.insert("id", "ccc6dbc8-e352-48a1-8e87-3c89a4669fc2");
    pluginMetaData.insert("name", "CloudNotifications");
    pluginMetaData.insert("displayName", tr("Cloud Notifications"));

    QVariantList interfaces;
//    interfaces.append("notifications");
    interfaces.append("connectable");

    QVariantList createMethods;
    createMethods.append("discovery");

    QVariantMap testActionParamTitle;
    testActionParamTitle.insert("id", "c9545e1c-55cd-42ca-a00f-43f21dfdf05a");
    testActionParamTitle.insert("name", "title");
    testActionParamTitle.insert("displayName", tr("Title"));
    testActionParamTitle.insert("type", "QString");

    QVariantList notifyActionParamTypes;
    notifyActionParamTypes.append(testActionParamTitle);

    QVariantMap notifyAction;
    notifyAction.insert("id", "cc6ad463-0a63-4570-ae13-956f50faa3a6");
    notifyAction.insert("name", "notify");
    notifyAction.insert("displayName", tr("Send notification"));
    notifyAction.insert("paramTypes", notifyActionParamTypes);

    QVariantList actionTypes;
    actionTypes.append(notifyAction);

    QVariantMap connectedState;
    connectedState.insert("id", "292b5c5d-a6fc-43b6-a59e-f3e1a3ab42b4");
    connectedState.insert("name", "connected");
    connectedState.insert("displayName", tr("connected"));
    connectedState.insert("type", "bool");
    connectedState.insert("displayNameEvent", tr("Connected changed"));
    connectedState.insert("defaultValue", false);

    QVariantList stateTypes;
    stateTypes.append(connectedState);


    QVariantMap cloudNotificationsThingClass;
    cloudNotificationsThingClass.insert("id", "0f1a441a-3793-4a1c-91fc-35d752443aff");
    cloudNotificationsThingClass.insert("name", "PyTest");
    cloudNotificationsThingClass.insert("displayName", tr("Python test"));
    cloudNotificationsThingClass.insert("createMethods", createMethods);
    cloudNotificationsThingClass.insert("interfaces", interfaces);
    cloudNotificationsThingClass.insert("actionTypes", actionTypes);
    cloudNotificationsThingClass.insert("stateTypes", stateTypes);

    QVariantList thingClasses;
    thingClasses.append(cloudNotificationsThingClass);

    QVariantMap guhVendor;
    guhVendor.insert("id", "2062d64d-3232-433c-88bc-0d33c0ba2ba6"); // nymea's id
    guhVendor.insert("name", "nymea");
    guhVendor.insert("displayName", "nymea");
    guhVendor.insert("thingClasses", thingClasses);

    QVariantList vendors;
    vendors.append(guhVendor);
    pluginMetaData.insert("vendors", vendors);

    return QJsonObject::fromVariantMap(pluginMetaData);

}

void PythonIntegrationPlugin::init()
{
    qCDebug(dcThingManager()) << "Python wrapper: init()";
    PyObject *pFunc = PyObject_GetAttrString(m_module, "init");
    if(!pFunc || !PyCallable_Check(pFunc)) {
        qCDebug(dcThingManager()) << "Python plugin does not implement \"init()\" method.";
        return;
    }
    PyObject_CallObject(pFunc, nullptr);
}

void PythonIntegrationPlugin::discoverThings(ThingDiscoveryInfo *info)
{
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

    PyObject_CallFunction(pFunc, "O", pyInfo);

    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        if(pvalue) {
            PyObject *pstr = PyObject_Str(pvalue);
            if(pstr) {
                const char* err_msg = PyUnicode_AsUTF8(pstr);
                if(pstr) {
                    qWarning() << QString(err_msg);
                }

            }
            PyErr_Restore(ptype, pvalue, ptraceback);
        }
    }

}

