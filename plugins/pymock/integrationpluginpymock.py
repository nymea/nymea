import nymea
import time
#from fastdotcom import fast_com

watchingAutoThings = False
loopRunning = False

def init():
    global loopRunning
    loopRunning = True

    logger.log("Python mock plugin init")
    logger.warn("Python mock warning")
    print("python stdout")

    while loopRunning:
        time.sleep(5);
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
    logger.log("Bye bye")


def deinit():
    logger.log("shutting down")
    global loopRunning
    loopRunning = False


def configValueChanged(paramTypeId, value):
    logger.log("Plugin config value changed:", paramTypeId, value, watchingAutoThings)
    if watchingAutoThings and paramTypeId == pyMockPluginAutoThingCountParamTypeId:
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


def startMonitoringAutoThings():
    global watchingAutoThings
    watchingAutoThings = True
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


def discoverThings(info):
    logger.log("Discovery started for", info.thingClassId, "with result count:", info.params[0].value)
    time.sleep(10) # Some delay for giving a feeling of a discovery
    # Add 2 new discovery results
    for i in range(0, info.params[0].value):
        info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, "Python mock thing %i" % i))
    # Also add existing ones again so reconfiguration is possible
    for thing in myThings():
        if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
            info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, thing.name, thingId=thing.id))

    info.finish(nymea.ThingErrorNoError)


def startPairing(info):
    logger.log("startPairing for", info.thingName, info.thingId, info.params)
    info.finish(nymea.ThingErrorNoError, "Log in as user \"john\" with password \"smith\".")


def confirmPairing(info, username, secret):
    logger.log("confirming pairing for", info.thingName, username, secret)
    time.sleep(1)
    if username == "john" and secret == "smith":
        info.finish(nymea.ThingErrorNoError)
    else:
        info.finish(nymea.ThingErrorAuthenticationFailure, "Error logging in here!")


def setupThing(info):
    logger.log("setupThing for", info.thing.name)
    info.finish(nymea.ThingErrorNoError)
    info.thing.nameChangedHandler = thingNameChanged
    info.thing.settingChangedHandler = thingSettingChanged


def postSetupThing(thing):
    logger.log("postSetupThing for", thing.name)
    thing.nameChangedHandler = lambda thing : logger.log("Thing name changed", thing.name)

    if thing.thingClassId == pyMockAutoThingClassId:
        logger.log("State 1 value:", thing.stateValue(pyMockAutoState1StateTypeId))

    if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
        logger.log("Param 1 value:", thing.paramValue(pyMockDiscoveryPairingThingParam1ParamTypeId))
        logger.log("Setting 1 value:", thing.setting(pyMockDiscoveryPairingSettingsSetting1ParamTypeId))


def executeAction(info):
    logger.log("executeAction for", info.thing.name, info.actionTypeId, "with params", info.params)
    paramValueByIndex = info.params[0].value
    paramValueById = info.paramValue(pyMockAction1ActionParam1ParamTypeId)
    logger.log("Param by index:", paramValueByIndex, "by ID:", paramValueById)
    info.finish(nymea.ThingErrorNoError)


def autoThings():
    autoThings = []
    for thing in myThings():
        if thing.thingClassId == pyMockAutoThingClassId:
            autoThings.append(thing)
    return autoThings


def thingNameChanged(thing, name):
    logger.log("Thing name changed:", thing.name)


def thingSettingChanged(thing, paramTypeId, value):
    logger.log("Thing setting changed:", thing.name, paramTypeId, value)


# Intentionally commented out to also have a test case for unimplmented functions
# def thingRemoved(thing):
#    logger.log("thingRemoved for", thing.name)
