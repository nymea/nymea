#ifndef PYNYMEAMODULE_H
#define PYNYMEAMODULE_H

#include <Python.h>

#include "pystdouthandler.h"
#include "pynymealogginghandler.h"
#include "pything.h"
#include "pythingdiscoveryinfo.h"
#include "pythingsetupinfo.h"
#include "pyparam.h"
#include "pythingactioninfo.h"
#include "pythingpairinginfo.h"
#include "pypluginstorage.h"
#include "pyapikeystorage.h"
#include "pybrowseresult.h"
#include "pybrowseritem.h"
#include "pybrowseractioninfo.h"
#include "pybrowseritemresult.h"
#include "pyplugintimer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#pragma GCC diagnostic ignored "-Wwrite-strings"

static int nymea_exec(PyObject *m) {

    registerStdOutHandler(m);
    registerNymeaLoggingHandler(m);
    registerParamType(m);
    registerThingType(m);
    registerThingDescriptorType(m);
    registerThingDiscoveryInfoType(m);
    registerThingPairingInfoType(m);
    registerThingSetupInfoType(m);
    registerThingActionInfoType(m);
    registerPluginStorageType(m);
    registerApiKeyStorageType(m);
    registerBrowseResultType(m);
    registerBrowserItemType(m);
    registerBrowserActionInfoType(m);
    registerBrowserItemResultType(m);
    registerPluginTimerType(m);

    return 0;
}

static struct PyModuleDef_Slot nymea_slots[] = {
    {Py_mod_exec, (void*)nymea_exec},
    {0, NULL},
};

static struct PyModuleDef nymea_module = {
    PyModuleDef_HEAD_INIT,
    "nymea",
    "The nymea module. Provdes types used in the nymea plugin API.",
    0,
    nullptr, // methods
    nymea_slots, // slots
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC PyInit_nymea(void)
{
    return PyModuleDef_Init(&nymea_module);
}

#pragma GCC diagnostic pop

#endif // PYNYMEAMODULE_H
