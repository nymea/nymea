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
import subprocess

__version__='1.0.0'


#--------------------------------------------------------------------------
def printInfo(info):
    print(info)


#--------------------------------------------------------------------------
def printWarning(warning):
    print('Warning: ' + warning)


#--------------------------------------------------------------------------
def printError(error):
    print('Error: ' + error)


#--------------------------------------------------------------------------
def writeToFile(line):
    outputFile.write('%s\n' % line)


#--------------------------------------------------------------------------
def writeCodeSection(jsonData):
    writeToFile('\code')
    writeToFile(json.dumps(jsonData, sort_keys=True, indent=4))
    writeToFile('\endcode')


#--------------------------------------------------------------------------
def getJsonString(object, key):
    for objectKey, value in object.items():
        if objectKey is key:
            return value

    return None


#--------------------------------------------------------------------------
def extractReferences(object):
    referenceList = []
    for key, value in object.iteritems():
        keyString = ('%s' % key)
        if keyString.startswith('$ref:'):
            referenceList.append(keyString)

        valueString = ('%s' % value)
        if valueString.startswith('$ref:'):
            referenceList.append(valueString)

        elif isinstance(value, dict):
            referenceList.extend(extractReferences(value))

        elif isinstance(value, list):
            for item in value:
                itemString = ('%s' % item)
                if itemString.startswith('$ref:'):
                    referenceList.append(itemString)

                elif isinstance(item, dict):
                    referenceList.extend(extractReferences(item))

    return referenceList


#--------------------------------------------------------------------------
def createReferenceLine(object):
    # Get list of all references
    referenceList = []
    fullReferenceList = extractReferences(object)
    for reference in fullReferenceList:
       if reference not in referenceList:
           referenceList.append(reference.replace('$ref:', ''))

    if not referenceList:
        return ""

    # Write references from content
    referencesString = "See also: "
    referenceCount = len(referenceList)
    for i in range(len(referenceList)):
        if i is referenceCount - 1:
            referencesString += '\l{%s}' % referenceList[i]
        else:
            referencesString += '\l{%s}, ' % referenceList[i]

    return referencesString


#--------------------------------------------------------------------------
def extractTypes(types):
    typesList = []
    for type in types:
        typesList.append(type)

    typesSorted = sorted(typesList)
    for type in typesSorted:
        writeToFile('\section2 %s' % type)
        writeCodeSection(types[type])
        if isinstance(types[type], dict):
            writeToFile(createReferenceLine(types[type]))

#--------------------------------------------------------------------------
def extractMethods(methods):
    methodsList = []
    for method in methods:
        methodsList.append(method)

    methodsSorted = sorted(methodsList)
    for method in methodsSorted:
        writeToFile('\section2 %s' % method)
        writeToFile('%s' % methods[method]['description'])
        writeToFile('Params')
        writeCodeSection(methods[method]['params'])
        writeToFile('Returns')
        writeCodeSection(methods[method]['returns'])
        writeToFile(createReferenceLine(methods[method]))

#--------------------------------------------------------------------------
def extractNotifications(notifications):
    notificationsList = []
    for notification in notifications:
        notificationsList.append(notification)

    notificationsSorted = sorted(notificationsList)
    for notification in notificationsSorted:
        writeToFile('\section2 %s' % notification)
        writeToFile('%s' % notifications[notification]['description'])
        writeToFile('Params')
        writeCodeSection(notifications[notification]['params'])
        writeToFile(createReferenceLine(notifications[notification]))


#--------------------------------------------------------------------------
def writeDocumentationContent(apiVersion, apiJson):
    printInfo('--> Write API documentation content')
    printInfo('--> API version: \"%s\"' % (version))

    writeToFile('/*!')
    writeToFile('In the following section you can find a detaild description of the current API version %s.' % apiVersion)

    writeToFile('\list')
    writeToFile('\li \l{Types}')
    writeToFile('\li \l{Methods}')
    writeToFile('\li \l{Notifications}')
    writeToFile('\endlist')


    if 'types' in apiJson:
        writeToFile('\section1 Types')
        extractTypes(apiJson['types'])

    if 'methods' in apiJson:
        writeToFile('\section1 Methods')
        extractMethods(apiJson['methods'])

    if 'notifications' in apiJson:
        writeToFile("\section1 Notifications")
        extractNotifications(apiJson['notifications'])

    writeToFile("\section1 Full introspect")
    writeCodeSection(apiJson)
    writeToFile('*/')


###########################################################################
# Main
###########################################################################

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='This tool generates a qdoc file out of the api.json file for the online documentation.')
    parser.add_argument('-v', '--version', action='version', version=__version__)
    parser.add_argument('-j', '--jsonfile', help='The API JSON input file name with the JSON RPC api definition', metavar='jsonfile', default='../tests/auto/api.json')
    parser.add_argument('-o', '--output', help='The qdoc outputFile with the generated documentation script', metavar='output', default='./jsonrpc-api.qdoc')
    args = parser.parse_args()

    # Print build information for debugging
    printInfo('Json file: %s' % (args.jsonfile))
    printInfo('Output: %s' % (args.output))

    # Open API file for reading
    try:
        inputFile = open(args.jsonfile, 'r')
    except:
        printError('Could not open input file \"%s\"' % (args.jsonfile))
        exit -1

    # Open qdoc file for writing
    try:
        outputFile = open(args.output, 'w')
    except:
        printError('Could not open output file \"%s\"' % (args.output))
        exit -1

    # Read version line and create json content of the rest
    inputFileContent = inputFile.read().splitlines(True)
    inputFile.close()

    # Parse verion and json data
    version = inputFileContent[0].strip()
    jsonRawContent = ''
    for line in inputFileContent[1:]:
        jsonRawContent += line

    # Parse json content
    try:
        apiJson = json.loads(jsonRawContent)
    except ValueError as error:
        printError(' --> Could not load json content')
        printError('     %s' % (error))
        exit -1

    # Sort alphabetically
    apiJsonSortedRaw = json.dumps(apiJson, sort_keys=True, indent=4)
    apiJsonSorted = json.loads(apiJsonSortedRaw)
    writeDocumentationContent(version, apiJsonSorted)
