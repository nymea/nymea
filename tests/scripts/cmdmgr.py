#!/usr/bin/python

import telnetlib
import json

HOST='localhost'
PORT=1234
commandId=0

methods = {'-> List supported vendors': 'list_vendors',
           '-> List supported devices': 'list_deviceClasses',
           '-> List configured devices': 'list_configured_devices',
           '-> List configured device params': 'list_configured_device_params',
           '-> Add device': 'add_device',
           '-> Remove a device': 'remove_device',
           '-> List supported devices by vendor': 'list_deviceClasses_by_vendor',
           '-> Execute an action': 'execute_action',
           '-> List device states': 'list_device_states',
           '-> Add a rule': 'add_rule',
           '-> List rules': 'list_rules',
           '-> List rule details': 'list_rule_detail',
           '-> List rules containing a certain device' : 'list_rules_containig_deviceId',
           '-> Remove a rule': 'remove_rule'}


def get_selection(title, options):
    print "\n\n", title
    for i in range(0,len(options)):
        print "%5i: %s" % (i, options[i])
    selection = raw_input("Enter selection: ")
    if not selection:
	print "\n   -> error in selection"
	return None
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
    if not vendors:
        print "\n    No vendors found. Please install guh-plugins and restart guhd."
        return ""
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

def list_configured_device_params():
    deviceId = select_configured_device()
    device = get_device(deviceId)
    deviceParams = device['params']
    print "\nParams of the device with the id ", deviceId, "\n"
    print "=== Params ==="
    for i in range(len(deviceParams)):
        print "%20s: %s" % (deviceParams[i]['name'], deviceParams[i]['value'])
    print "=== Params ==="


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
        print "    No supported devices for this vendor"
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
    return None


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
        if any("allowedValues" in item for item in paramType):
	    selection = get_selection("Please select one of following allowed values:", paramType['allowedValues'])
	    paramValue = paramType['allowedValues'][selection]
	    param = {}
	    param['name'] = paramType['name']
	    param['value'] = paramValue
	else:  
	    paramValue = raw_input("Please enter value for parameter %s (type: %s): " % (paramType['name'], paramType['type']))
	    param = {}
	    param['name'] = paramType['name']
	    param['value'] = paramValue
        params.append(param)
    #print "got params:", params
    return params


def select_valueOperator():
    valueOperators = ["ValueOperatorEquals", "ValueOperatorNotEquals", "ValueOperatorLess", "ValueOperatorGreater", "ValueOperatorLessOrEqual", "ValueOperatorGreaterOrEqual"]
    selection = get_selection("Please select an operator to compare this parameter: ", valueOperators)
    if selection != None:
        return valueOperators[selection]
    return None


def select_stateOperator():
    stateOperators = ["StateOperatorAnd", "StateOperatorOr"]
    selection = get_selection("Please select an operator to compare this state: ", stateOperators)
    if selection != None:
        return stateOperators[selection]
    return None
  

def read_paramDescriptors(paramTypes):
    params = []
    for paramType in paramTypes:
        paramValue = raw_input("Please enter value for parameter <%s> (type: %s): " % (paramType['name'], paramType['type']))
        operator = select_valueOperator()
        param = {}
        param['name'] = paramType['name']
        param['value'] = paramValue
        param['operator'] = operator
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
    if not deviceDescriptorIdList:
        print "\n    No device found"
        return -1
    selection = get_selection("Please select a device descriptor", deviceDescriptorList)
    if selection != None:
        return deviceDescriptorIdList[selection]


def get_deviceClass(deviceClassId):
    deviceClasses = get_deviceClasses()
    for deviceClass in deviceClasses:
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
    print_device_error_code(response['params']['deviceError'])


def add_discovered_device(deviceClassId, deviceDescriptorId):
    params = {}
    params['deviceClassId'] = deviceClassId
    params['deviceDescriptorId'] = deviceDescriptorId

    deviceClass = get_deviceClass(deviceClassId)
    if deviceClass['setupMethod'] == "SetupMethodJustAdd":
        response = send_command("Devices.AddConfiguredDevice", params)
        if not response['status'] == "success":
            print "Adding device failed: %s" % response['params']['deviceError']
        else:
            print "Device added successfully. Device ID: %s" % response['params']['deviceId']
    else:
        params = {}
        params['deviceClassId'] = deviceClassId
        params['deviceDescriptorId'] = deviceDescriptorId
        response = send_command("Devices.PairDevice", params)
        print "pairdevice response:", response
        if not response['status'] == "success":
            print "Pairing failed: %s", response['params']['deviceError']
            return
        else:
            print "\nPairing device %s\n\n%s" % (deviceClass['name'], response['params']['displayMessage'])
            if response['params']['setupMethod'] == "SetupMethodPushButton":
                raw_input("Press enter to confirm")

            params = {}
            params['pairingTransactionId'] = response['params']['pairingTransactionId']
            response = send_command("Devices.ConfirmPairing", params)
	if response['status'] == "success":
            print "Device paired successfully"
        else:
            print "Error pairing device: %s" % response['params']['deviceError']


def add_device():
    deviceClassId = select_deviceClass()
    if deviceClassId == "":
        print "    Empty deviceClass. Can't continue"
        return
    deviceClass = get_deviceClass(deviceClassId)
    print "createmethods are", deviceClass['createMethods']
    if "CreateMethodUser" in deviceClass['createMethods']:
        add_configured_device(deviceClassId)
    elif "CreateMethodDiscovery" in deviceClass['createMethods']:
        deviceDescriptorId = discover_device(deviceClassId)
        if deviceDescriptorId == -1:
            return
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
    print_device_error_code(response['params']['deviceError'])


def select_actionType(deviceClassId):
    actions = get_action_types(deviceClassId)
    if not actions:
	return ""
    actionList = []
    print "got actions", actions
    for i in range(len(actions)):
        print "got actiontype", actions[i]
        actionList.append(actions[i]['name'])
    selection = get_selection("Please select an action type:", actionList)
    return actions[selection]


def select_eventType(deviceClassId):
    eventTypes = get_eventTypes(deviceClassId)
    if not eventTypes:
        return ""
    eventTypeList = []
    for i in range(len(eventTypes)):
        eventTypeList.append(eventTypes[i]['name'])
    selection = get_selection("Please select an event type:", eventTypeList)
    return eventTypes[selection]


def execute_action():
    deviceId = select_device()
    device = get_device(deviceId)
    actionType = select_actionType(device['deviceClassId'])
    if actionType == "":
        print "\n    This device has no actions"
        return
    actionTypeId = actionType['id']
    params = {}
    params['actionTypeId'] = actionTypeId
    params['deviceId'] = deviceId
    actionType = get_actionType(actionTypeId)
    actionParams = read_params(actionType['paramTypes'])
    params['params'] = actionParams
    response = send_command("Actions.ExecuteAction", params)
    print_device_error_code(response['params']['deviceError'])
    

def print_device_error_code(deviceError):
    if deviceError == "DeviceErrorNoError":
        print "\nSuccess! (", deviceError, ")"
    elif deviceError == "DeviceErrorPluginNotFound":
        print "\nERROR: the plugin could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorDeviceNotFound":
        print "\nERROR: the device could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorDeviceClassNotFound":
        print "\nERROR: the deviceClass could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorActionTypeNotFound":
        print "\nERROR: the actionType could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorStateTypeNotFound":
        print "\nERROR: the stateType could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorEventTypeNotFound":
        print "\nERROR: the eventType could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorDeviceDescriptorNotFound":
        print "\nERROR: the deviceDescriptor could not be found. (", deviceError, ")"
    elif deviceError == "DeviceErrorMissingParameter":
        print "\nERROR: some parameters are missing. (", deviceError, ")"
    elif deviceError == "DeviceErrorInvalidParameter":
        print "\nERROR: invalid parameter. (", deviceError, ")"
    elif deviceError == "DeviceErrorSetupFailed":
        print "\nERROR: setup failed. (", deviceError, ")"
    elif deviceError == "DeviceErrorDuplicateUuid":
        print "\nERROR: uuid allready exists. (", deviceError, ")"
    elif deviceError == "DeviceErrorCreationMethodNotSupported":
        print "\nERROR: the selected CreationMethod is not supported for this device. (", deviceError, ")"
    elif deviceError == "DeviceErrorSetupMethodNotSupported":
        print "\nERROR: the selected SetupMethod is not supported for this device. (", deviceError, ")"
    elif deviceError == "DeviceErrorHardwareNotAvailable":
        print "\nERROR: the hardware is not available. (", deviceError, ")"
    elif deviceError == "DeviceErrorHardwareFailure":
        print "\nERROR: hardware failure. Something went wrong with the hardware. (", deviceError, ")"
    elif deviceError == "DeviceErrorAsync":
        print "\nINFO: the response will need some time. (", deviceError, ")"
    elif deviceError == "DeviceErrorDeviceInUse":
        print "\nERROR: the device is currently in use. Try again later. (", deviceError, ")"
    elif deviceError == "DeviceErrorPairingTransactionIdNotFound":
        print "\nERROR: the pairingTransactionId could not be found. (", deviceError, ")"
    else:
        print "\nERROR: Unknown error code: ", deviceError,  "Please take a look at the newest API version."


def print_rule_error_code(ruleError):
    if ruleError == "RuleErrorNoError":
        print "\nSuccess! (", ruleError, ")"
    elif ruleError == "RuleErrorInvalidRuleId":
        print "\nERROR: the ruleId is not valid. (", ruleError, ")"
    elif ruleError == "RuleErrorRuleNotFound":
        print "\nERROR: the rule could not be found. (", ruleError, ")"
    elif ruleError == "RuleErrorDeviceNotFound":
        print "\nERROR: the device could not be found for this rule. (", ruleError, ")"
    elif ruleError == "RuleErrorEventTypeNotFound":
        print "\nERROR: the eventType could not be found for this rule. (", ruleError, ")"
    elif ruleError == "RuleErrorActionTypeNotFound":
        print "\nERROR: the actionType could not be found for this rule. (", ruleError, ")"
    elif ruleError == "RuleErrorInvalidParameter":
        print "\nERROR: invalid parameter in this rule. (", ruleError, ")"
    elif ruleError == "RuleErrorMissingParameter":
        print "\nERROR: missing parameter in this rule. (", ruleError, ")"
    else:
        print "\nERROR: Unknown error code: ", ruleError,  "Please take a look at the newest API version."



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
        #print_device_error_code(response['params']['deviceError'])
        print "%s: %s" % (deviceClass['stateTypes'][i]['name'], response['params']['value'])
    print "=== States ==="


def create_eventDescriptors():
    enough = False
    eventDescriptors = []
    while not enough:
        print "\nCreating EventDescriptor:"
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


def create_eventDescriptor():
    print "\nCreating EventDescriptor:"
    deviceId = select_configured_device()
    device = get_device(deviceId)
    eventType = select_eventType(device['deviceClassId']);
    params = read_paramDescriptors(eventType['paramTypes'])
    eventDescriptor = {}
    eventDescriptor['deviceId'] = deviceId
    eventDescriptor['eventTypeId'] = eventType['id']
    if len(params) > 0:
        eventDescriptor['paramDescriptors'] = params

    print "got eventDescriptors:", eventDescriptor
    return eventDescriptor


def create_actions():
    enough = False
    actions = []
    while not enough:
        print "\nCreating Action:"
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
    #print "got actions:", actions
    return actions


def add_rule():
    ruleType = select_rule_type()
    if ruleType == "EventBasedRule":
	params = {}
	#params['name'] = raw_input("Please enter the name of the rule: ")
	params['eventDescriptor'] = create_eventDescriptor()
	params['actions'] = create_actions()
	print "adding rule with params:", params
        response = send_command("Rules.AddRule", params)
        print_rule_error_code(response['params']['ruleError'])
    elif ruleType == "StateBasedRule":
        params = {}
        params['eventDescriptorList'] = create_eventDescriptors()
        if len(params['eventDescriptorList']) > 1:
            params['stateEvaluator'] = select_stateOperator()
        params['actions'] = create_actions()
        print "adding rule with params:", params
        response = send_command("Rules.AddRule", params)
        print_rule_error_code(response['params']['ruleError'])
    elif ruleType == "MixedRule":
	print "not implemented yet in this script...comming soon ;)"
    else: 
	print "    No rule added";


def select_rule_type():
    ruleTypes = ["EventBasedRule", "StateBasedRule", "MixedRule"]
    selection = get_selection("Please select a rule type: ", ruleTypes)     
    return ruleTypes[selection]


def list_rules():
    response = send_command("Rules.GetRules", {})
    if not response['params']['ruleIds']:
        print "\n    No rules found."
        return
    print "\nRules found:"
    for i in range(len(response['params']['ruleIds'])):
        print response['params']['ruleIds'][i]


def list_rule_detail():
    ruleId = select_rule()
    if ruleId == "":
        print "\n    No rules found"
        return
    params = {}
    params['ruleId'] = ruleId
    response = send_command("Rules.GetRuleDetails", params)
    print response
    print "\nThe rule", ruleId, "depends on following EventDescriptors and triggers"
    print "only if ", get_stateEvaluator_text(response['params']['rule']['stateEvaluator']['operator']), "are true.\n"
    print "Events:"
    for i in range(len(response['params']['rule']['eventDescriptors'])):
        eventDescriptor = response['params']['rule']['eventDescriptors'][i]
        #print eventDescriptor
        device = get_device(eventDescriptor['deviceId'])
        paramDescriptors = eventDescriptor['paramDescriptors']
        print  "%5s. -> %40s -> eventTypeId: %10s: " %(i, device['name'], eventDescriptor['eventTypeId'])
        #print paramDescriptors
        for i in range(len(paramDescriptors)):
            print "%58s %s %s" %(paramDescriptors[i]['name'], get_valueOperator_symbol(paramDescriptors[i]['operator']), paramDescriptors[i]['value'])
        print ""
    print "\nActions:"
    for i in range(len(response['params']['rule']['actions'])):
        action = response['params']['rule']['actions'][i]
        device = get_device(action['deviceId'])
        actionType = get_actionType(response['params']['rule']['actions'][i]['actionTypeId'])
        actionParams = response['params']['rule']['actions'][i]['params']
        print  "%5s. ->  %40s -> action: %s" %(i, device['name'], actionType['name'])
        for i in range(len(actionParams)):
            print "%61s: %s" %(actionParams[i]['name'], actionParams[i]['value'])
        print ""    

def get_valueOperator_symbol(valueOperator):
    if valueOperator == "ValueOperatorEquals":
        return "="
    elif valueOperator == "ValueOperatorNotEquals":
        return "!="
    elif valueOperator == "ValueOperatorLess":
        return "<" 
    elif valueOperator == "ValueOperatorGreater":
        return ">" 
    elif valueOperator == "ValueOperatorLessOrEqual":
        return "<=" 
    elif valueOperator == "ValueOperatorGreaterOrEqual":
        return ">=" 
    else:
        return "<unknown value operator>"
    
def get_stateEvaluator_text(stateEvaluator):
    if stateEvaluator == "StateOperatorAnd":
        return "ALL of the events/states"
    elif stateEvaluator == "StateOperatorOr":
        return "ONE of this events/states"
    else:
        return "<unknown state evaluator>"
    
    
def list_rules_containig_deviceId():
    deviceId = select_configured_device()
    device = get_device(deviceId)
    params = {}
    params['deviceId'] = deviceId
    response = send_command("Rules.FindRules", params)
    if not response['params']['ruleIds']:
        print "\nThere is no rule containig this device."
        return
    print "\nFollowing rules contain this device"
    for i in range(len(response['params']['ruleIds'])):
        print "Device ", deviceId, "found in rule", response['params']['ruleIds'][i]


def select_rule():
    ruleIds = send_command("Rules.GetRules", {})['params']['ruleIds']
    if not ruleIds:
        return ""
    selection = get_selection("Please select rule:", ruleIds)
    if selection != None:
	return ruleIds[selection]


def remove_rule():
    ruleId = select_rule()
    if ruleId == "":
        print "\nNo rules found"
        return
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


