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
