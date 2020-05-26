#include <Python.h>

#include "pythonintegrationplugin.h"

#include "loggingcategories.h"

#include <QFileInfo>

PyObject* aview_write(PyObject* /*self*/, PyObject* args)
{
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return nullptr;
    qCDebug(dcThingManager()) << what;
    return Py_BuildValue("");
}


PyObject* aview_flush(PyObject* /*self*/, PyObject* /*args*/)
{
    return Py_BuildValue("");
}

static PyMethodDef aview_methods[] =
{
    {"write", aview_write, METH_VARARGS, "doc for write"},
    {"flush", aview_flush, METH_VARARGS, "doc for flush"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

static PyModuleDef aview_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "aview",               // const char* m_name;
    "doc for aview",       // const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    aview_methods,        // PyMethodDef *m_methods
    //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
    nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_aview(void)
{
    PyObject* m = PyModule_Create(&aview_module);
    PySys_SetObject("stdout", m);
    PySys_SetObject("stderr", m);
    return m;
}

PythonIntegrationPlugin::PythonIntegrationPlugin(QObject *parent) : IntegrationPlugin(parent)
{

}

bool PythonIntegrationPlugin::loadScript(const QString &scriptFile)
{
    // TODO: Call this just once and call Py_Finalize()
    PyImport_AppendInittab("aview", PyInit_aview);
    Py_Initialize();
    PyImport_ImportModule("aview");

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
    PyObject *pFunc = PyObject_GetAttrString(m_module, "init");
    if(!pFunc || !PyCallable_Check(pFunc)) {
        qCWarning(dcThingManager()) << "Python plugin does not implement \"discoverThings()\" method.";
        return;
    }
//    PyObject* args = Py_BuildValue("(s)", ln.c_str());
//    if (!args)
//    {
//        PyErr_Print();
//        Py_DECREF(filterFunc);
//        return "Error building args tuple";
//    }

//    PyObject_CallObject(pFunc, nullptr);

}

