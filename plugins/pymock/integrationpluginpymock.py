# SPDX-License-Identifier: GPL-3.0-or-later

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
# Copyright (C) 2013 - 2024, nymea GmbH
# Copyright (C) 2024 - 2025, chargebyte austria GmbH
#
# This file is part of nymea.
#
# nymea is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# nymea is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with nymea. If not, see <https://www.gnu.org/licenses/>.
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

import nymea
import time

globalPluginTimer = None
pluginTimers = {}

# Optional, for initialisation, if needed
def init():
    logger.log("Python mock plugin init")
    logger.warn("Python mock warning")
    print("python stdout")


# Optional, clean up stuff if needed
def deinit():
    logger.log("shutting down")


# Optional, if the plugin should create auto things, this is the right place to create them,
# or, start monitoring the network (or whatever) for them to appear
def startMonitoringAutoThings():
    logger.log("Start monitoring auto things. Have %i auto devices. Need %i." % (len(autoThings()), configValue(pyMockPluginAutoThingCountParamTypeId)))
    things = autoThings();
    for i in range(len(things), configValue(pyMockPluginAutoThingCountParamTypeId)):
        logger.log("Creating new auto thing")
        descriptor = nymea.ThingDescriptor(pyMockAutoThingClassId, "Python Mock auto thing")
        descriptor.params = [nymea.Param(pyMockAutoThingParam1ParamTypeId, True)]
        autoThingsAppeared([descriptor])
    for i in range(configValue(pyMockPluginAutoThingCountParamTypeId), len(things)):
        logger.log("Removing auto thing")
        autoThingDisappeared(things[i].id)

    logger.log("Done start monitoring auto things")


# If the plugin supports things of createMethod "discovery", nymea will call this to discover things
def discoverThings(info):
    logger.log("Discovery started for", info.thingClassId, "with result count:", info.params[0].value)
    time.sleep(5) # Some delay for giving a feeling of a real discovery
    # Add discovery results (in this example the amount given by the discovery params)
    for i in range(0, info.params[0].value):
        info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, "Python mock thing %i" % i))
    # Also add existing ones again so reconfiguration is possible, setting the existing thing ID properly
    for thing in myThings():
        if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
            info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, thing.name, thingId=thing.id))

    info.finish(nymea.ThingErrorNoError)


# If the plugin supports things with a setupMethod other than "justAdd", this will be called to initiate login/pairing
def startPairing(info):
    logger.log("startPairing for", info.thingName, info.thingId, info.params)
    info.finish(nymea.ThingErrorNoError, "Log in as user \"john\" with password \"smith\".")


# If the plugin supports things with a setupMethod other than "justAdd", this will be called to complete login/pairing
def confirmPairing(info, username, secret):
    logger.log("confirming pairing for", info.thingName, username, secret)
    time.sleep(1)
    if username == "john" and secret == "smith":
        info.finish(nymea.ThingErrorNoError)
    else:
        info.finish(nymea.ThingErrorAuthenticationFailure, "Error logging in here!")


# Mandatory, a new thing is being set up. Initialize (connect etc...) it
def setupThing(info):
    logger.log("setupThing for", info.thing.name)
    info.finish(nymea.ThingErrorNoError)

    # Signal handlers
    info.thing.settingChangedHandler = thingSettingChanged
    info.thing.nameChangedHandler = lambda info : logger.log("Thing name changed", info.thing.name)


# Optional, run additional code after a successful thing setup
def postSetupThing(thing):
    logger.log("postSetupThing for", thing.name)

    global globalPluginTimer
    if globalPluginTimer is None:
        globalPluginTimer = nymea.PluginTimer(5, timerTriggered)

    if thing.thingClassId == pyMockAutoThingClassId:
        logger.log("State 1 value:", thing.stateValue(pyMockAutoState1StateTypeId))

    if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
        logger.log("Param 1 value:", thing.paramValue(pyMockDiscoveryPairingThingParam1ParamTypeId))
        logger.log("Setting 1 value:", thing.setting(pyMockDiscoveryPairingSettingsSetting1ParamTypeId))

    if thing.thingClassId == pyMockThingClassId:
        interval = thing.setting(pyMockSettingsIntervalParamTypeId)

        pluginTimer = nymea.PluginTimer(interval, lambda thing=thing : logger.log("Timer triggered for %s. (Interval: %i)" % (thing.name, thing.setting(pyMockSettingsIntervalParamTypeId))))

        logger.log("Thing timer interval for %s: %i" % (thing.name, pluginTimer.interval))
        pluginTimers[thing] = pluginTimer


# Optional, do cleanups when a thing is removed
def thingRemoved(thing):
    logger.log("thingRemoved for", thing.name)
    logger.log("Remaining things:", len(myThings()))

    if thing.thingClassId == pyMockThingClassId:
        del pluginTimers[thing]

    # Clean up the global plugin timer if there are no things left
    if len(myThings()) == 0:
        global globalPluginTimer
        globalPluginTimer = None


# Callback for the plugin timer. If polling is needed, fetch values and set thing states accordingly
def timerTriggered():
    logger.log("Timer triggered")
    for thing in myThings():
        if thing.thingClassId == pyMockThingClassId:
            logger.log("Emitting event 1 for", thing.name, "eventTypeId", pyMockEvent1EventTypeId)
            thing.emitEvent(pyMockEvent1EventTypeId, [nymea.Param(pyMockEvent1EventParam1ParamTypeId, "Im an event")])
            logger.log("Setting state 1 for", thing.name, "to", thing.stateValue(pyMockState1StateTypeId) + 1)
            thing.setStateValue(pyMockState1StateTypeId, thing.stateValue(pyMockState1StateTypeId) + 1)
        if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
            logger.log("Emitting event 1 for", thing.name)
            thing.emitEvent(pyMockDiscoveryPairingEvent1EventTypeId, [nymea.Param(pyMockDiscoveryPairingEvent1EventParam1ParamTypeId, "Im an event")])
            logger.log("Setting state 1 for", thing.name, "Old value is:", thing.stateValue(pyMockDiscoveryPairingState1StateTypeId))
            thing.setStateValue(pyMockDiscoveryPairingState1StateTypeId, thing.stateValue(pyMockDiscoveryPairingState1StateTypeId) + 1)


# If the plugin supports things with actions, nymea will call this to run actions
def executeAction(info):
    logger.log("executeAction for", info.thing.name, info.actionTypeId, "with params", info.params)
    paramValueByIndex = info.params[0].value
    paramValueById = info.paramValue(pyMockAction1ActionParam1ParamTypeId)
    logger.log("Param by index:", paramValueByIndex, "by ID:", paramValueById)
    info.finish(nymea.ThingErrorNoError)


# Callback handler when the user changes settings for a particular thing
def thingSettingChanged(thing, paramTypeId, value):
    logger.log("Thing setting changed:", thing.name, paramTypeId, value)

    if thing.thingClassId == pyMockThingClassId:
        if paramTypeId == pyMockSettingsIntervalParamTypeId:
            logger.log("Adjusting plugin timer to interval:", value)
            timer = pluginTimers[thing]
            timer.interval = value


# Callback handler when the user changes the global plugin configuration
def configValueChanged(paramTypeId, value):
    logger.log("Plugin config value changed:", paramTypeId, value)
    if paramTypeId == pyMockPluginAutoThingCountParamTypeId:
        logger.log("Auto Thing Count plugin config changed:", value, "Currently there are:", len(autoThings()), "auto things")
        things = autoThings();
        for i in range(len(things), value):
            logger.log("Creating new auto thing")
            descriptor = nymea.ThingDescriptor(pyMockAutoThingClassId, "Python Mock auto thing")
            descriptor.params = [nymea.Param(pyMockAutoThingParam1ParamTypeId, True)]
            autoThingsAppeared([descriptor])

        for i in range(value, len(things)):
            logger.log("Removing auto thing")
            autoThingDisappeared(things[i].id)


# If a plugin supports browsable things, nymea will call this to browse a thing
def browseThing(result):
    logger.log("browseThing called", result.thing.name, result.itemId)
    if result.itemId == "":
        result.addItem(nymea.BrowserItem("001", "Item 0", "I'm a folder", browsable=True, icon=nymea.BrowserIconFolder))
        result.addItem(nymea.BrowserItem("002", "Item 1", "I'm executable", executable=True, icon=nymea.BrowserIconApplication))
        result.addItem(nymea.BrowserItem("003", "Item 2", "I'm a file", icon=nymea.BrowserIconFile))
        result.addItem(nymea.BrowserItem("004", "Item 3", "I have a nice thumbnail", thumbnail="https://github.com/nymea/nymea/raw/master/icons/nymea-logo-256x256.png"))
        result.addItem(nymea.BrowserItem("005", "Item 4", "I'm disabled", disabled=True, icon=nymea.BrowserIconFile))
        result.addItem(nymea.BrowserItem("favorites", "Favorites", "I'm the best!", icon=nymea.BrowserIconFavorites))

    if result.itemId == "001":
        result.addItem(nymea.BrowserItem("011", "Item in subdir", "I'm in a subfolder", icon=nymea.BrowserIconFile))

    result.finish(nymea.ThingErrorNoError)


# If a thingclass supports browser item actions, nymea will call this upon execution
# Intentionally commented out to also have a test case for unimplmented functions
#def executeBrowserItem(info):
#    logger.log("executeBrowserItem called for thing", info.thing.name, "and item", info.itemId)
#    info.finish(nymea.ThingErrorNoError)


# Helper functions can be added too
def autoThings():
    autoThings = []
    for thing in myThings():
        if thing.thingClassId == pyMockAutoThingClassId:
            autoThings.append(thing)
    return autoThings
