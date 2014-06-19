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
           'List supported Devices by vendor': 'list_deviceClasses_by_vendor' }


def get_selection(title, options):
    print "\n\n", title
    for i in range(0,len(options)):
        print "%i: %s" % (i, options[i])
    selection = raw_input("Enter selection: ")
    return int(selection)

def send_command(method, params = None):
    global commandId
    if params == None or len(params) == 0:
        command = '{"id": %i, "method":"%s"}\n' % (commandId, method)
    else:
        paramString = ""
        for i in range(0, len(params)):
            if paramString != "":
                paramString = paramString + ", "
            paramKey = params.keys()[i]
            paramString = paramString + ('"%s":"%s"' % (paramKey, params[paramKey].encode('utf-8')))
        command = '{"id": %i, "method":"%s", "params":{%s}}\n' % (commandId, method, paramString)

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
    return vendorIdList[selection]

def get_deviceClasses(vendorId = None):
    params = {};
    if vendorId != None:
        params['vendorId'] = vendorId
    return send_command("Devices.GetSupportedDevices", params)['params']['deviceClasses']

def list_deviceClasses(vendorId = None):
    response = get_deviceClasses(vendorId)
    print "=== Devices ==="
    for deviceClass in response:
        print "%40s  %s" % (deviceClass['name'], deviceClass['id'])
    print "=== Devices ==="

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
    return deviceClassIdList[selection]

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


def discover_device(deviceClassId = None):
    if deviceClassId == None:
        deviceClassId = select_deviceClass()
    params = {}
    params['deviceClassId'] = deviceClassId
    print "\ndiscovering..."
    response = send_command("Devices.GetDiscoveredDevices", params)
    deviceDescriptorList = [];
    deviceDescriptorIdList = [];
    for deviceDescriptor in response['params']['deviceDescriptors']:
        deviceDescriptorList.append("%s (%s)" % (deviceDescriptor['title'], deviceDescriptor['description']))
        deviceDescriptorIdList.append(deviceDescriptor['id'])
    selection = get_selection("Please select a device descriptor", deviceDescriptorList)
    return deviceDescriptorIdList[selection]

def get_deviceClass(deviceClassId):
    deviceClasses = get_deviceClasses()
    for deviceClass in deviceClasses:
#        print "got deviceclass", deviceClass
        if deviceClass['id'] == deviceClassId:
            return deviceClass
    return None

def add_configured_device(deviceClassId):
    params = {}
    params['deviceClassId'] = deviceClassId
    response = send_command("Devices.AddConfiguredDevice", params)
    if response['params']['success'] != "true":
        print "Error executing method: %s" % response['params']['errorMessage']
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
    print "createmethod is", deviceClass['createMethod']
    if deviceClass['createMethod'] == "CreateMethodUser":
        add_configured_device(deviceClassId)
    elif deviceClass['createMethod'] == "CreateMethodDiscovery":
        deviceDescriptorId = discover_device(deviceClassId)
        add_discovered_device(deviceClassId, deviceDescriptorId)
    elif deviceClass['createMethod'] == "CreateMethodAuto":
        print "Can't create this device manually. It'll be created automatically when hardware is discovered."

def select_device():
    devices = get_configured_devices()
    deviceList = []
    deviceIdList = []
    for i in range(len(devices)):
        deviceList.append(devices[i]['name'])
        deviceIdList.append(devices[i]['id'])
    selection = get_selection("Please select a device", deviceList)
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
        print "Error deleting device %s" % deviceId


tn = telnetlib.Telnet(HOST, PORT)
packet = tn.read_until("\n}\n")

packet = json.loads(packet)
print "connected to", packet["server"], "\nserver version:", packet["version"], "\nprotocol version:", packet["protocol version"], "\n"

while True:
    selection = get_selection("What do you want to do?", methods.keys())
    selectionKey = methods.keys()
    methodName = methods[methods.keys()[selection]]
    methodToCall = globals()[methods[methods.keys()[selection]]]
    methodToCall()


