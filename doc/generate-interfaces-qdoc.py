#!/usr/bin/env python

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

import argparse
import traceback
import json
import os
import sys
import subprocess

__version__='1.0.0'


#--------------------------------------------------------------------------
def printInfo(info):
    print('[+] ' + info)


#--------------------------------------------------------------------------
def printWarning(warning):
    print('[-] Warning: ' + warning)


#--------------------------------------------------------------------------
def printError(error):
    print('[!] Error: ' + error)


#--------------------------------------------------------------------------
def writeToFile(line):
    outputFile.write('%s\n' % line)


#--------------------------------------------------------------------------
def writeCodeSection(jsonData):
    writeToFile('\code')
    writeToFile(json.dumps(jsonData, sort_keys=True, indent=4))
    writeToFile('\endcode')
    writeToFile('')


#--------------------------------------------------------------------------
def loadInterfaces():
    printInfo('Loading interfaces files from %s' % interfacesDirectory)
    
    interfaces = {}
    interfaceFiles = []
    
    # Read the file list
    for fileName in os.listdir(interfacesDirectory):
        if ".json" in fileName:
            interfaceFiles.append(interfacesDirectory + "/" + fileName)

    # Sort file lists for being able to get the last n days logs
    interfaceFiles.sort()
    for fileName in interfaceFiles:
        name = os.path.basename(fileName)
        interfaceName = os.path.splitext(name)[0]
        #printInfo('    %s --> %s | %s' % (interfaceName, name, fileName))
        interfaces[interfaceName] = loadJsonData(fileName)
        
    return interfaces
        
        
#--------------------------------------------------------------------------
def loadJsonData(fileName):
    # Open the file
    try:
        jsonFile = open(fileName, 'r')
    except:
        printError('Could not open JSON file \"%s\"' % (fileName))
        exit(-1)

    jsonFileContent = jsonFile.read()
    jsonFile.close()
    
    # Parse json content
    try:
        data = json.loads(jsonFileContent)
    except ValueError as error:
        printError('Could not load json content from %s' % (fileName))
        printError('     %s' % (error))
        exit(-1)

    return data


#--------------------------------------------------------------------------
def writeDocumentationContent():
    printInfo('Write interfaces documentation content to %s' % outputFileName)

    writeToFile('\section1 Available interfaces')
    writeToFile('This following list shows you the current available interfaces.')
    writeToFile('')
    
    # Create the interfaces list
    writeToFile('\list')
    for interfaceName in interfaceNames:
        writeToFile('    \li \l{%s}' % interfaceName)
    
    writeToFile('\endlist')
    writeToFile('')
    writeToFile('')

    # Extract interface information
    writeInterfaces()
    
    

#--------------------------------------------------------------------------
def writeInterfaces():
    for interfaceName in interfaceNames:
        writeToFile('\section2 %s' % interfaceName)
        interfaceJson = interfaces[interfaceName]
        # If a desciption is provided, use it and remove it from the map
        if 'description' in interfaceJson:
            writeToFile('%s' % interfaceJson['description'] )
            interfaceJson.pop('description')
            
        writeCodeSection(interfaceJson)
        writeToFile('')
        
        writeExtends(interfaceJson)
        writeToFile('')

        


#--------------------------------------------------------------------------
def writeExtends(interfaceJson):
    if 'extends' in interfaceJson:
        if type(interfaceJson['extends']) is list:
            #printInfo('extends is list: %s' % interfaceJson['extends'])
            extendsList = list(interfaceJson['extends'])
            extendsString = "See also: "
            
            extendsCount = len(extendsList)
            for i in range(len(extendsList)):
                if i is extendsCount - 1:
                    extendsString += '\l{%s}' % extendsList[i]
                else:
                    extendsString += '\l{%s}, ' % extendsList[i]

            writeToFile(extendsString)
        else:
            #printInfo('extends is string: %s' % interfaceJson['extends'])
            writeToFile('See also: \l{%s}' % interfaceJson['extends'])
        


###########################################################################
# Main
###########################################################################

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='This tool generates a qdoc file out of the interfaces JSON files for the online documentation.')
    parser.add_argument('-v', '--version', action='version', version=__version__)
    parser.add_argument('-i', '--interfaces', help='The path to the interfaces JSON files. Default is ../libnymea/interfaces/', metavar='path', default='../libnymea/interfaces/')
    parser.add_argument('-o', '--output', help='The qdoc output file with the generated documentation script. Default is interfacelist.qdoc', metavar='output', default='interfacelist.qdoc')
    args = parser.parse_args()

    interfacesDirectory = os.path.abspath(args.interfaces)
    outputFileName = os.path.dirname(os.path.realpath(sys.argv[0])) + "/" + args.output
    
    # Print build information for debugging
    printInfo('--> Interfaces directory: %s' % interfacesDirectory)
    printInfo('--> Output: %s' % outputFileName)

    # Verify interfaces path
    if not os.path.isdir(interfacesDirectory):
        printError('The given interfaces directory path does not exist \"%s\"' % (interfacesDirectory))
        exit(-1)


    # Open qdoc file for writing
    try:
        outputFile = open(outputFileName, 'w')
    except:
        printError('Could not open output file \"%s\"' % (outputFileName))
        exit(-1)


    # Load all interface files
    interfaces = loadInterfaces()
    
    # Get a alphabetic sorted list of interfaces
    interfaceNames = interfaces.keys()
    interfaceNames.sort()
    
    # Write the documentation
    writeDocumentationContent()
    
    outputFile.close()
    printInfo('Done.')

