import nymea

def init():
    logger.log("Python mock plugin init")
    logger.log("Number of auto mocks", configValue(pyMockPluginAutoThingCountParamTypeId))


def configValueChanged(paramTypeId, value):
    logger.log("Plugin config value changed:", paramTypeId, value)


def startMonitoringAutoThings():
    logger.log("Start monitoring auto things. Already have", len(myThings()))
    for i in range(configValue(pyMockPluginAutoThingCountParamTypeId), len(myThings())):
        logger.log("auto thing")
#        descriptor = nymea.
#        autoThingsAppeared(
