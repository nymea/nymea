import nymea
import asyncio

def init():
    logger.log("Python mock plugin init")


def configValueChanged(paramTypeId, value):
    logger.log("Plugin config value changed:", paramTypeId, value)
    if paramTypeId == pyMockPluginAutoThingCountParamTypeId:
        logger.log("Auto Thing Count plugin config changed:", value, "Currently there are:", len(autoThings()), "auto things")
        things = autoThings();
        for i in range(len(things), value):
            logger.log("Creating new auto thing")
            descriptor = nymea.ThingDescriptor(pyMockAutoThingClassId, "Python Mock auto thing")
            autoThingsAppeared([descriptor])

        for i in range(len(value), things):
            logger.log("Removing auto thing")
            autoThingDisappeared(things[i].id)


def startMonitoringAutoThings():
    logger.log("Start monitoring auto things. Have %i auto devices. Need %i." % (len(autoThings()), configValue(pyMockPluginAutoThingCountParamTypeId)))
    for i in range(len(autoThings()), configValue(pyMockPluginAutoThingCountParamTypeId)):
        logger.log("Creating new auto thing")
        descriptor = nymea.ThingDescriptor(pyMockAutoThingClassId, "Python Mock auto thing")
        autoThingsAppeared([descriptor])
    logger.log("Done start monitoring auto things")


async def setupThing(info):
    logger.log("setupThing for", info.thing.name)
    info.finish(nymea.ThingErrorNoError)


def autoThings():
    autoThings = []
    for thing in myThings():
        if thing.thingClassId == pyMockAutoThingClassId:
            autoThings.append(thing)
    return autoThings
