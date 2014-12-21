#!/usr/bin/python

import telnetlib
import json
import datetime

HOST='localhost'
PORT=1234
commandId=0

methods = {'1': 'add_device',
           '2': 'remove_device',
           '3': 'list_configured_device_params',
           '4': 'list_device_states',
           '5': 'execute_action',
           '6': 'add_rule',
           '7': 'remove_rule',
           '8': 'list_rule_detail',
           '9': 'enable_disable_rule',
           '10': 'list_vendors',
           '11': 'list_configured_devices',
           '12': 'list_deviceClasses',
           '13': 'list_deviceClasses_by_vendor',
           '14': 'list_rules',
           '15': 'list_rules_containig_deviceId',
           '16': 'list_logEntries'
          }  


def get_menu_selection():
    print ""
    print "----------------------------------------"
    print "What do you want to do?"
    print "----------------------------------------"
    print ""
    print "Devices --------------------------------"
    print "     1  -> Add a new device"
    print "     2  -> Remove a device"
    print "     3  -> List device parameters"
    print "     4  -> List device states"
    print "     5  -> Execute an action"
    print ""
    print "Rules ----------------------------------"
    print "     6  -> Add a new rule"
    print "     7  -> Remove a rule"
    print "     8  -> Rule details"
    print "     9  -> Enable/Disable a rule"
    print ""
    print "Other ----------------------------------"
    print "     10 -> List supported vendors"
    print "     11 -> List configured devices"
    print "     12 -> List supported devices"
    print "     13 -> List supported devices by vendor"
    print "     14 -> List configured rules"
    print "     15 -> List rules containing a certain device"
    print "     16 -> Print log"
    print "----------------------------------------"
    print ""
    selection = raw_input("Enter selection: ")
    n = 0
    for i in methods.keys():
        if i == selection:
            return methods.values()[int(n)]
        
        n= n+1
        
    print "\nError in selection!"
    return None


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
        return None
    
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
        return None
    
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
        return None
    
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
        print "\n    Timeout: no device found"
        return None
    
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
    if "actionType" in response['params']:
        return response['params']['actionType']
    return None


def get_eventType(eventTypeId):
    params = {}
    params['eventTypeId'] = eventTypeId
    response = send_command("Events.GetEventType", params)
    return response['params']['eventType']

def add_configured_device(deviceClassId):
    deviceClass = get_deviceClass(deviceClassId)

    params = {}
    params['deviceClassId'] = deviceClassId

    deviceParams = read_params(deviceClass['paramTypes'])
    if len(deviceParams) > 0:
        params['deviceParams'] = deviceParams

    print "add device command params:", params
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
        if deviceDescriptorId == None:
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
	return None
    
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
        return None
    
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
            
    return actions


def add_rule():
    params = {}
    params['eventDescriptorList'] = create_eventDescriptors()
    if len(params['eventDescriptorList']) > 1:
        params['stateEvaluator'] = select_stateOperator()
        
    params['actions'] = create_actions()
    print "adding rule with params:", params
    response = send_command("Rules.AddRule", params)
    print_rule_error_code(response['params']['ruleError'])


def enable_disable_rule():
    ruleId = select_rule()
    if ruleId == "":
        print "\n    No rules found"
        return
    
    actionTypes = ["enable", "disable"]
    selection = get_selection("What do you want to do with this rule: ", actionTypes)     
    if selection == 0:
	params = {}
	params['ruleId'] = ruleId
	response = send_command("Rules.EnableRule", params)
        print_rule_error_code(response['params']['ruleError'])
    else:
	params = {}
	params['ruleId'] = ruleId
	response = send_command("Rules.DisableRule", params)
        print_rule_error_code(response['params']['ruleError'])


def get_rule_status(ruleId):
    params = {}
    params['ruleId'] = ruleId
    response = send_command("Rules.GetRuleDetails", params)
    if response['params']['rule']['enabled'] == True:
	return "enabled"
    else:
	return "disabled"

def list_rules():
    response = send_command("Rules.GetRules", {})
    if not response['params']['ruleIds']:
        print "\n    No rules found."
        return None
    
    print "\nRules found:"
    for i in range(len(response['params']['ruleIds'])):
	ruleId = response['params']['ruleIds'][i]
	params = {}
	params['ruleId'] = ruleId
	ruleDetail = send_command("Rules.GetRuleDetails", params)
	#print ruleDetail
	print response['params']['ruleIds'][i], "(", get_rule_status(ruleId), ")"


def get_rule_detail(ruleId):
    params = {}
    params['ruleId'] = ruleId
    response = send_command("Rules.GetRuleDetails", params)
    if 'rule' in response['params']:
        return response['params']['rule']
    return None

def list_rule_detail():
    ruleId = select_rule()
    if ruleId == "":
        print "\n    No rules found"
        return None
    
    rule = get_rule_detail(ruleId)
    print "\nDetails for rule", ruleId, "which currently is", get_rule_status(ruleId) 
    print "\nEvents ->", get_stateEvaluator_text(rule['stateEvaluator']['operator']), ":"
    for i in range(len(rule['eventDescriptors'])):
        eventDescriptor = rule['eventDescriptors'][i]
        device = get_device(eventDescriptor['deviceId'])
        eventType = get_eventType(eventDescriptor['eventTypeId'])
        paramDescriptors = eventDescriptor['paramDescriptors']
        print  "%5s. -> %40s -> eventTypeId: %10s: " %(i, device['name'], eventType['name'])
        for i in range(len(paramDescriptors)):
            print "%58s %s %s" %(paramDescriptors[i]['name'], get_valueOperator_symbol(paramDescriptors[i]['operator']), paramDescriptors[i]['value'])

    print "\nActions:"
    for i in range(len(rule['actions'])):
        action = rule['actions'][i]
        device = get_device(action['deviceId'])
        actionType = get_actionType(rule['actions'][i]['actionTypeId'])
        actionParams = rule['actions'][i]['params']
        print  "%5s. ->  %40s -> action: %s" %(i, device['name'], actionType['name'])
        for i in range(len(actionParams)):
            print "%61s: %s" %(actionParams[i]['name'], actionParams[i]['value'])
      

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
        return "(AND) | ALL of the events/states has to be true/emited."
    elif stateEvaluator == "StateOperatorOr":
        return "(OR) | ONE of the events/states has to be true/emited."
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
        return None
    
    print "\nFollowing rules contain this device"
    for i in range(len(response['params']['ruleIds'])):
        print "Device ", deviceId, "found in rule", response['params']['ruleIds'][i]


def select_rule():
    ruleIds = send_command("Rules.GetRules", {})['params']['ruleIds']
    if not ruleIds:
        return None
    
    selection = get_selection("Please select rule:", ruleIds)
    if selection != None:
	return ruleIds[selection]


def remove_rule():
    ruleId = select_rule()
    if ruleId == "":
        print "\nNo rule found"
        return None
    
    params = {}
    params['ruleId'] = ruleId
    response = send_command("Rules.RemoveRule", params)
    print "removeRule response", response

def get_stateType(stateTypeId):
    params = {}
    params['stateTypeId'] = stateTypeId
    response = send_command("States.GetStateType", params);
    if "stateType" in response['params']:
        return response['params']['stateType']
    return None

def list_logEntries():
    params = {}
    response = send_command("Logging.GetLogEntries", params)
    stateTypeIdCache = {}
    actionTypeIdCache = {}
    eventTypeIdCache = {}
    deviceIdCache = {}
    ruleIdCache = {}
    for i in range(len(response['params']['logEntries'])):
        entry = response['params']['logEntries'][i]

        if entry['loggingLevel'] == "LoggingLevelInfo":
            levelString = "(I)"
            error = ""
        else:
            levelString = "(A)"
            error = entry['errorCode']

        if entry['source'] == "LoggingSourceSystem":
            deviceName = "Guh Server"
            sourceType = "System Event"
            symbolString = "->"
            sourceName = "Active changed"
            if entry['active'] == True:
                value = "active"
            else:
                value = "inactive"

        if entry['source'] == "LoggingSourceStates":
            typeId = entry['typeId']
            sourceType = "State Changed"
            symbolString = "->"
            if typeId in stateTypeIdCache:
                sourceName = stateTypeIdCache[typeId]
            else:
                stateType = get_stateType(typeId)
                if stateType is not None:
                    sourceName = stateType["name"]
                    stateTypeIdCache[typeId] = sourceName
                else:
                    sourceName = typeId
            value = entry['value']
            if entry['deviceId'] in deviceIdCache:
                deviceName = deviceIdCache[entry['deviceId']]
            else:
                device = get_device(entry['deviceId'])
                if device is not None:
                    deviceName = device['name']
                    deviceIdCache[entry['deviceId']] = deviceName
                else:
                    deviceName = typeId


        if entry['source'] == "LoggingSourceActions":
            typeId = entry['typeId']
            sourceType = "Action executed"
            symbolString = "()"
            if typeId in actionTypeIdCache:
                sourceName = actionTypeIdCache[typeId]
            else:
                actionType = get_actionType(typeId)
                if actionType is not None:
                    sourceName = actionType['name']
                else:
                    sourceName = typeId
                actionTypeIdCache[typeId] = sourceName
            value = entry['value']
            if entry['deviceId'] in deviceIdCache:
                deviceName = deviceIdCache[entry['deviceId']]
            else:
                device = get_device(entry['deviceId'])
                if device is not None:
                    deviceName = device['name']
                else:
                    deviceName = entry['deviceId']
                deviceIdCache[entry['deviceId']] = deviceName

        if entry['source'] == "LoggingSourceEvents":
            typeId = entry['typeId']
            sourceType = "Event triggered"
            symbolString = "()"
            if typeId in eventTypeIdCache:
                sourceName = eventTypeIdCache[typeId]
            else:
                eventType = get_eventType(typeId)
                sourceName = eventType['name']
                eventTypeIdCache[typeId] = sourceName
            value = entry['value']
            if entry['deviceId'] in deviceIdCache:
                deviceName = deviceIdCache[entry['deviceId']]
            else:
                device = get_device(entry['deviceId'])
                if device is not None:
                    deviceName = device['name']
                else:
                    devieName = entry['deviceId']
                deviceIdCache[entry['deviceId']] = deviceName


        if entry['source'] == "LoggingSourceRules":
            typeId = entry['typeId']
            if entry['eventType'] == "LoggingEventTypeTrigger":
                sourceType = "Rule triggered"
                sourceName = "triggered"
                symbolString = "()"
                value = ""
            else:
                sourceType = "Rule active changed"
                symbolString = "()"
                sourceName = "active"
                if entry['active']:
                    value = "active"
                else:
                    value = "inactive"

            if typeId in ruleIdCache:
                deviceName = ruleIdCache[typeId]
            else:
                rule = get_rule_detail(typeId)
                if rule is not None and 'name' in rule:
                    deviceName = rule['name']
                else:
                    deviceName = typeId
                ruleIdCache[typeId] = deviceName

        timestamp = datetime.datetime.fromtimestamp(entry['timestamp']/1000)
        sourceType = sourceType.ljust(20)
        deviceName = deviceName.ljust(38)
        sourceName = sourceName.ljust(38)
        value = value.ljust(30)
        print levelString, timestamp, ":", sourceType, ":", deviceName, ":", sourceName, symbolString, value, ":", error

############################################################################################
import sys

if len(sys.argv) > 1:
    HOST=sys.argv[1]

tn = telnetlib.Telnet(HOST, PORT)
packet = tn.read_until("\n}\n")

packet = json.loads(packet)
print "connected to", packet["server"], "\nserver version:", packet["version"], "\nprotocol version:", packet["protocol version"], "\n"

while True:
    method = get_menu_selection()
    if method != None:
        #print "call method: ", method
        methodCall = globals()[method]
	methodCall()


