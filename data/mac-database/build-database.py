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

import os
import sys
import json
import sqlite3
import requests
import argparse

# Note: this db is no longer free
databaseFileName = 'mac-addresses.db'

parser = argparse.ArgumentParser(description='Build mac address database from source.')
parser.add_argument('--macaddressio', action='store_true', help='Download and generate using macaddress.io as source (note: not free any more)')

args = parser.parse_args()

vendorInfoHash = {}

if args.macaddressio:
    # Deprecated: not free any more, but would still work if you buy the db
    downloadUrl='https://macaddress.io/database/macaddress.io-db.json'
    jsonDataFileName = 'macaddress.io-db.json'
    print('Downloading', downloadUrl, '...')
    downloadRequest = requests.get(downloadUrl)
    open(jsonDataFileName, 'wb').write(downloadRequest.content)

    print('Reading JSON data..')
    jsonDataFile = open(jsonDataFileName, 'r')
    lines = jsonDataFile.readlines()
    for line in lines:
        vendorMap = json.loads(line)
        vendorInfoHash[vendorMap['oui']] = vendorMap['companyName']

    jsonDataFile.close()

else:
    downloadUrl="https://maclookup.app/downloads/json-database/get-db"
    jsonDataFileName = 'mac-vendors-export.json'
    print('Downloading', downloadUrl, '...')
    downloadRequest = requests.get(downloadUrl)
    open(jsonDataFileName, 'wb').write(downloadRequest.content)

    if not os.path.exists(jsonDataFileName):
        print('Could not find %s. Please download the json data from https://maclookup.app/downloads/json-database and place it into this folder.')

    print('Reading JSON data..')
    # Example: {"macPrefix":"00:00:0C","vendorName":"Cisco Systems, Inc","private":false,"blockType":"MA-L","lastUpdate":"2015/11/17"}

    jsonDataFile = open(jsonDataFileName, 'r')
    jsonData = json.load(jsonDataFile)
    jsonDataFile.close()
    for vendorMap in jsonData:
        vendorInfoHash[vendorMap['macPrefix']] = vendorMap['vendorName']
        print(vendorMap['macPrefix'], vendorMap['vendorName'])



if os.path.exists(databaseFileName):
    print('Delete old db file', databaseFileName)
    os.remove(databaseFileName)

print('Build up database', databaseFileName)
connection = sqlite3.connect(databaseFileName)
cursor = connection.cursor()
cursor.execute('CREATE TABLE companyNames (companyName TEXT PRIMARY KEY, UNIQUE(companyName));')
cursor.execute('CREATE TABLE oui (oui TEXT PRIMARY KEY, companyNameIndex INTEGER, UNIQUE(oui)) WITHOUT ROWID;')
#cursor.execute('CREATE UNIQUE INDEX ouiIndex ON `oui` (`oui`);')

# Insert all vendor names alphabetically
print('Writing company names into database...')
sortedVendorHash = sorted(vendorInfoHash.items(), key=lambda x: x[1], reverse=False)
vendorCount = 0
for vendorInfo in sortedVendorHash:
    insertQuery = 'INSERT OR IGNORE INTO companyNames (companyName) VALUES(?);'
    cursor.execute(insertQuery, [vendorInfo[1]])
    cursor.execute('SELECT COUNT(companyName) FROM companyNames;')
    countResult = cursor.fetchall()
    vendorCount = countResult[0][0]

connection.commit()

# Insert all oui with reference to company name
print('Writing OUI into database with company name reference...')
# Sort by oui for good binary search in the db
sortedOuiHash = sorted(vendorInfoHash.items(), key=lambda x: x[0], reverse=False)
ouiCount = 0
for vendorInfo in sortedOuiHash:
    insertQuery = 'INSERT OR IGNORE INTO oui (oui, companyNameIndex) VALUES(?, (SELECT rowid FROM companyNames WHERE companyName = ?));'
    cursor.execute(insertQuery, [vendorInfo[0].replace(':', ''), vendorInfo[1]])
    cursor.execute('SELECT COUNT(oui) FROM oui;')
    countResult = cursor.fetchall()
    ouiCount = countResult[0][0]

connection.commit()
connection.close()
print('Finished successfully. Loaded', ouiCount, 'OUI values from', vendorCount, 'manufacturers into', databaseFileName)
