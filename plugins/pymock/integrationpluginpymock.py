import nymea
import asyncio

watchingAutoThings = False

async def init():
    logger.log("Python mock plugin init")

    while True:
        await asyncio.sleep(2);
        logger.log("Updating stuff")
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


async def discoverThings(info):
    logger.log("Discovery started for", info.thingClassId, "with result count:", info.params[0].value)
    await asyncio.sleep(1) # Some delay for giving a feeling of a discovery
    # Add 2 new discovery results
    for i in range(0, info.params[0].value):
        info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, "Python mock thing %i" % i))
    # Also add existing ones again so reconfiguration is possible
    for thing in myThings():
        if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
            info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, thing.name, thingId=thing.id))

    info.finish(nymea.ThingErrorNoError)


async def startPairing(info):
    logger.log("startPairing for", info.thingName, info.thingId, info.params)
    info.finish(nymea.ThingErrorNoError, "Log in as user \"john\" with password \"smith\".")


async def confirmPairing(info, username, secret):
    logger.log("confirming pairing for", info.thingName, username, secret)
    await asyncio.sleep(1)
    if username == "john" and secret == "smith":
        info.finish(nymea.ThingErrorNoError)
    else:
        info.finish(nymea.ThingErrorAuthenticationFailure, "Error logging in here!")


async def setupThing(info):
    logger.log("setupThing for", info.thing.name)
    info.finish(nymea.ThingErrorNoError)


async def postSetupThing(thing):
    logger.log("postSetupThing for", thing.name)
    thing.nameChangedHandler = lambda thing : logger.log("Thing name changed", thing.name)

    if thing.thingClassId == pyMockAutoThingClassId:
        logger.log("State 1 value:", thing.stateValue(pyMockAutoState1StateTypeId))

    if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
        logger.log("Param 1 value:", thing.paramValue(pyMockDiscoveryPairingThingParam1ParamTypeId))
        logger.log("Setting 1 value:", thing.setting(pyMockDiscoveryPairingSettingsSetting1ParamTypeId))


async def executeAction(info):
    logger.log("executeAction for", info.thing.name, info.actionTypeId, "with params", info.params[0].value)
    info.finish(nymea.ThingErrorNoError)


def autoThings():
    autoThings = []
    for thing in myThings():
        if thing.thingClassId == pyMockAutoThingClassId:
            autoThings.append(thing)
    return autoThings


# Intentionally commented out to also have a test case for unimplmented functions
# def thingRemoved(thing):
#    logger.log("thingRemoved for", thing.name)
