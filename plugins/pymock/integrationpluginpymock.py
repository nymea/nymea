import nymea
import asyncio

watchingAutoThings = False

def init():
    logger.log("Python mock plugin init")


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
    logger.log("Discovery started for") #, info.thingClassId, "with result count:") #, info.params[0].value)
    logger.log("a")
    await asyncio.sleep(1) # Some delay for giving a feeling of a discovery
    # Add 2 new discovery results
    logger.log("b")
    info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, "Python mock thing 1"))
    info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, "Python mock thing 2"))
    logger.log("c")
    # Also add existing ones again so reconfiguration is possible
    for thing in myThings():
        logger.log("d")
        if thing.thingClassId == pyMockDiscoveryPairingThingClassId:
            logger.log("e")
            info.addDescriptor(nymea.ThingDescriptor(pyMockDiscoveryPairingThingClassId, thing.name, thingId=thing.id))
            logger.log("f")

    logger.log("g")
    info.finish(nymea.ThingErrorNoError)
    logger.log("h")


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
#    logger.log("setupThing for", info.thing.name, info.thing.params)
    logger.log("setupThing for", info.thing.name)
    info.finish(nymea.ThingErrorNoError)


def postSetupThing(thing):
    logger.log("postSetupThing for", thing.name, thing.params[0].value)
    thing.nameChangedHandler = lambda thing : logger.log("Thing name changed", thing.name)

    if thing.thingClassId == pyMockAutoThingClassId:
        logger.log("State 1 value:", thing.stateValue(pyMockAutoState1StateTypeId))


def autoThings():
    autoThings = []
    for thing in myThings():
        if thing.thingClassId == pyMockAutoThingClassId:
            autoThings.append(thing)
    return autoThings
