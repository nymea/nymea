#!/usr/bin/python

import telnetlib
import json

HOST='localhost'
PORT=1234
commandId=0

methods = {'List supported Vendors': 'list_vendors',
           'List supported Devices': 'list_deviceClasses',
           'List configured Devices': 'list_configured_devices',
           'Add Device': 'add_device',
           'Remove a device': 'remove_device',
           'List supported Devices by vendor': 'list_deviceClasses_by_vendor',
           'Execute an action': 'execute_action',
           'See a device`s states': 'list_device_states',
           'Add a rule': 'add_rule',
           'List rules': 'list_rules',
           'Remove rule': 'remove_rule'}


def get_selection(title, options):

    print "\n\n", title
    for i in range(0,len(options)):
        print "%i: %s" % (i, options[i])
    selection = raw_input("Enter selection: ")
    if not selection:
	print "-> error in selection"
	return
      
    return int(selection)

def send_command(method, params = None):
    global commandId
    commandObj = {}
    commandObj['id'] = commandId
    commandObj['method'] = method
    if not params == None and len(params) > 0:
        commandObj['params'] = params

    command = json.dumps(commandObj) + '\n'
    commandId = commandId + 1
    tn.write(command)
    response = json.loads(tn.read_until("\n}\n"))
    if response['status'] != "success":
        print "JSON error happened: %s" % response
    return response

def get_vendors():
    return send_command("Devices.GetSupportedVendors")

def list_vendors():
    response = get_vendors();
    print "=== Vendors ==="
    for vendor in response['params']['vendors']:
        print "%40s  %s" % (vendor['name'], vendor['id'])
    print "=== Vendors ==="

def select_vendor():
    vendors = get_vendors()['params']['vendors']
    vendorList = []
    vendorIdList = []
    for i in range(0,len(vendors)):
        vendorList.append(vendors[i]['name'])
        vendorIdList.append(vendors[i]['id'])
    selection = get_selection("Please select vendor", vendorList)
    if selection != None:
	return vendorIdList[selection]

def get_deviceClasses(vendorId = None):
    params = {};
    if vendorId != None:
        params['vendorId'] = vendorId
    return send_command("Devices.GetSupportedDevices", params)['params']['deviceClasses']

def list_deviceClasses(vendorId = None):
    response = get_deviceClasses(vendorId)
    print "=== DeviceClasses ==="
    for deviceClass in response:
        print "%40s  %s" % (deviceClass['name'], deviceClass['id'])
    print "=== DeviceClasses ==="

def select_deviceClass():
    vendorId = select_vendor()
    deviceClasses = get_deviceClasses(vendorId)
    if len(deviceClasses) == 0:
        print "No supported devices for this vendor"
        return ""
    deviceClassList = []
    deviceClassIdList = []
    for i in range(0,len(deviceClasses)):
        deviceClassList.append(deviceClasses[i]['name'])
        deviceClassIdList.append(deviceClasses[i]['id'])
    selection = get_selection("Please select device class", deviceClassList)
    if selection != None:
	return deviceClassIdList[selection]

def select_configured_device():
    devices = get_configured_devices()
    deviceList = []
    deviceIdList = []
    for device in devices:
        deviceList.append(device['name'])
        deviceIdList.append(device['id'])
    selection = get_selection("Please select a device: ", deviceList)
    if selection != None:    
	return deviceIdList[selection]

def get_action_types(deviceClassId):
    params = {}
    params['deviceClassId'] = deviceClassId
    return send_command("Devices.GetActionTypes", params)['params']['actionTypes']

def get_eventTypes(deviceClassId):
    params = {}
    params['deviceClassId'] = deviceClassId
    return send_command("Devices.GetEventTypes", params)['params']['eventTypes']

def list_deviceClasses_by_vendor():
    vendorId = select_vendor()
    list_deviceClasses(vendorId)

def get_configured_devices():
    return send_command("Devices.GetConfiguredDevices")['params']['devices']

def list_configured_devices():
    deviceList = get_configured_devices()
    print "=== Configured Devices ==="
    for device in deviceList:
        print "Name: %40s, ID: %s, DeviceClassID: %s" % (device['name'], device['id'], device['deviceClassId'])
    print "=== Configured Devices ==="


def read_params(paramTypes):
    params = []
    for paramType in paramTypes:
        paramValue = raw_input("Please enter value for parameter %s (type: %s): " % (paramType['name'], paramType['type']))
        param = {}
        param['name'] = paramType['name']
        param['value'] = paramValue
#        param[paramType['name']] = paramValue
        params.append(param)
    print "got params:", params
    return params


def select_valueOperator():
    valueOperators = ["OperatorTypeEquals", "OperatorTypeNotEquals", "OperatorTypeLess", "OperatorTypeGreater"]
    selection = get_selection("Please select an operator to compare this parameter: ", valueOperators)
    if selection != None:
        return valueOperators[selection]

def read_paramDescriptors(paramTypes):
    params = []
    for paramType in paramTypes:
        paramValue = raw_input("Please enter value for parameter %s (type: %s): " % (paramType['name'], paramType['type']))
        operator = select_valueOperator()
        param = {}
        param['name'] = paramType['name']
        param['value'] = paramValue
        param['operator'] = operator
#        param[paramType['name']] = paramValue
        params.append(param)
    print "got params:", params
    return params


def discover_device(deviceClassId = None):
    if deviceClassId == None:
        deviceClassId = select_deviceClass()
    deviceClass = get_deviceClass(deviceClassId)

    params = {}
    params['deviceClassId'] = deviceClassId

    discoveryParams = read_params(deviceClass['discoveryParamTypes'])
    if len(discoveryParams) > 0:
        params['discoveryParams'] = discoveryParams

    print "\ndiscovering..."
    response = send_command("Devices.GetDiscoveredDevices", params)
    deviceDescriptorList = [];
    deviceDescriptorIdList = [];
    for deviceDescriptor in response['params']['deviceDescriptors']:
        deviceDescriptorList.append("%s (%s)" % (deviceDescriptor['title'], deviceDescriptor['description']))
        deviceDescriptorIdList.append(deviceDescriptor['id'])
    selection = get_selection("Please select a device descriptor", deviceDescriptorList)
    if selection != None:
        return deviceDescriptorIdList[selection]

def get_deviceClass(deviceClassId):
    deviceClasses = get_deviceClasses()
    for deviceClass in deviceClasses:
#        print "got deviceclass", deviceClass
        if deviceClass['id'] == deviceClassId:
            return deviceClass
    return None

def get_device(deviceId):
    devices = get_configured_devices()
    for device in devices:
        if device['id'] == deviceId:
            return device
    return None

def get_actionType(actionTypeId):
    params = {}
    params['actionTypeId'] = actionTypeId
    response = send_command("Actions.GetActionType", params)
    print "got actionType", response
    return response['params']['actionType']


def add_configured_device(deviceClassId):
    deviceClass = get_deviceClass(deviceClassId)

    params = {}
    params['deviceClassId'] = deviceClassId

    deviceParams = read_params(deviceClass['paramTypes'])
    if len(deviceParams) > 0:
        params['deviceParams'] = deviceParams

    print "adddevice command params:", params
    response = send_command("Devices.AddConfiguredDevice", params)
    if response['params']['success'] != True:
        print "Error executing method: %s" % response
        return
    print "Added device: %s" % response['params']['deviceId']

def add_discovered_device(deviceClassId, deviceDescriptorId):
    params = {}
    params['deviceClassId'] = deviceClassId
    params['deviceDescriptorId'] = deviceDescriptorId

    deviceClass = get_deviceClass(deviceClassId)
    if deviceClass['setupMethod'] == "SetupMethodJustAdd":
        response = send_command("Devices.AddConfiguredDevice", params)
        if not response['params']['success']:
            print "Adding device failed: %s" % response['params']['errorMessage']
        else:
            print "Device added successfully. Device ID: %s" % response['params']['deviceId']
    else:
        params = {}
        params['deviceClassId'] = deviceClassId
        params['deviceDescriptorId'] = deviceDescriptorId
        response = send_command("Devices.PairDevice", params)
        print "pairdevice response:", response
        if not response['params']['success']:
            print "Pairing failed: %s", response['params']['errorMessage']
            return
        else:
            print "\nPairing device %s\n\n%s" % (deviceClass['name'], response['params']['displayMessage'])
            if response['params']['setupMethod'] == "SetupMethodPushButton":
                raw_input("Press enter to confirm")

            params = {}
            params['pairingTransactionId'] = response['params']['pairingTransactionId']
            response = send_command("Devices.ConfirmPairing", params)
            if response['params']['success']:
                success = True
                print "Device paired successfully"
            else:
                print "Error pairing device: %s" % response['params']['errorMessage']


def add_device():
    deviceClassId = select_deviceClass()
    if deviceClassId == "":
        print "Empty deviceClass. Can't continue"
        return
    deviceClass = get_deviceClass(deviceClassId)
    print "createmethods are", deviceClass['createMethods']
    if "CreateMethodUser" in deviceClass['createMethods']:
        add_configured_device(deviceClassId)
    elif "CreateMethodDiscovery" in deviceClass['createMethods']:
        deviceDescriptorId = discover_device(deviceClassId)
        add_discovered_device(deviceClassId, deviceDescriptorId)
    elif "CreateMethodAuto" in deviceClass['createMethods']:
        print "Can't create this device manually. It'll be created automatically when hardware is discovered."

def select_device():
    devices = get_configured_devices()
    deviceList = []
    deviceIdList = []
    for i in range(len(devices)):
        deviceList.append(devices[i]['name'])
        deviceIdList.append(devices[i]['id'])
    selection = get_selection("Please select a device", deviceList)
    if selection != None:
        return deviceIdList[selection]

def remove_device():
    deviceId = select_device()
    print "should remove device", deviceId
    params = {}
    params['deviceId'] = deviceId
    response = send_command("Devices.RemoveConfiguredDevice", params)
    if response['params']['success']:
        print "Successfully deleted device"
    else:
        print "Error deleting device: %s" % response['params']['errorMessage']

def select_actionType(deviceClassId):
    actions = get_action_types(deviceClassId)
    actionList = []
    print "got actions", actions
    for i in range(len(actions)):
        print "got actiontype", actions[i]
        actionList.append(actions[i]['name'])
    selection = get_selection("Please select an action type:", actionList)
    return actions[selection]


def select_eventType(deviceClassId):
    eventTypes = get_eventTypes(deviceClassId)
    eventTypeList = []
    for i in range(len(eventTypes)):
        eventTypeList.append(eventTypes[i]['name'])
    selection = get_selection("Please select an action type:", eventTypeList)
    if selection != None:
        return eventTypes[selection]


def execute_action():
    deviceId = select_device()
    device = get_device(deviceId)
    actionTypeId = select_actionType(device['deviceClassId'])['id']
    params = {}
    params['actionTypeId'] = actionTypeId
    params['deviceId'] = deviceId
    send_command("Actions.ExecuteAction", params)
    actionType = get_actionType(actionTypeId)
    actionParams = read_params(actionType['paramTypes'])
    params['params'] = actionParams
    response = send_command("Actions.ExecuteAction", params)
    print "execute action response", response

def list_device_states():
    deviceId = select_device()
    device = get_device(deviceId)
    deviceClass = get_deviceClass(device['deviceClassId'])
    print "\n\n=== States for device %s (%s) ===" % (device['name'], deviceId)
    for i in range(len(deviceClass['stateTypes'])):
        params = {}
        params['deviceId'] = deviceId
        params['stateTypeId'] = deviceClass['stateTypes'][i]['id']

        response = send_command("Devices.GetStateValue", params)
        print "%s: %s" % (deviceClass['stateTypes'][i]['name'], response['params']['value'])
    print "=== States ==="



def create_eventDescriptors():
    enough = False
    eventDescriptors = []
    while not enough:
        print "Creating EventDescriptor:"
        deviceId = select_configured_device()
        device = get_device(deviceId)
        eventType = select_eventType(device['deviceClassId']);
        params = read_paramDescriptors(eventType['paramTypes'])
        eventDescriptor = {}
        eventDescriptor['deviceId'] = deviceId
        eventDescriptor['eventTypeId'] = eventType['id']
        if len(params) > 0:
            eventDescriptor['paramDescriptors'] = params

        eventDescriptors.append(eventDescriptor)

        input = raw_input("Do you want to add another EventDescriptor? (y/N): ")
        if not input == "y":
            enough = True
    print "got eventDescriptors:", eventDescriptors
    return eventDescriptors


def create_actions():
    enough = False
    actions = []
    while not enough:
        print "Creating Action:"
        deviceId = select_configured_device()
        device = get_device(deviceId)
        actionType = select_actionType(device['deviceClassId'])
        params = read_params(actionType['paramTypes'])
        action = {}
        action['deviceId'] = deviceId
        action['actionTypeId'] = actionType['id']
        if len(params) > 0:
            action['params'] = params

        actions.append(action)

        input = raw_input("Do you want to add another action? (y/N): ")
        if not input == "y":
            enough = True
    print "got actions:", actions
    return actions

def add_rule():
    params = {}
    params['eventDescriptorList'] = create_eventDescriptors()
    params['actions'] = create_actions()
    print "adding rule:", params
    result = send_command("Rules.AddRule", params)
    print "AddRule result:", result

def list_rules():
    result = send_command("Rules.GetRules", {})
    print "got rules", result


def select_rule():
    ruleIds = send_command("Rules.GetRules", {})['params']['ruleIds']
    selection = get_selection("Please select rule:", ruleIds)
    if selection != None:
	return ruleIds[selection]

def remove_rule():
    ruleId = select_rule()
    params = {}
    params['ruleId'] = ruleId
    response = send_command("Rules.RemoveRule", params)
    print "removeRule response", response

import sys

if len(sys.argv) > 1:
    HOST=sys.argv[1]

tn = telnetlib.Telnet(HOST, PORT)
packet = tn.read_until("\n}\n")

packet = json.loads(packet)
print "connected to", packet["server"], "\nserver version:", packet["version"], "\nprotocol version:", packet["protocol version"], "\n"

while True:
    selection = get_selection("What do you want to do?", methods.keys())
    if selection != None:
	selectionKey = methods.keys()
	methodName = methods[methods.keys()[selection]]
	methodToCall = globals()[methods[methods.keys()[selection]]]
	methodToCall()


