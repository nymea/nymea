/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "zigbeenodeinitializer.h"
#include "loggingcategories.h"

#include <zigbeenetwork.h>

Q_DECLARE_LOGGING_CATEGORY(dcZigbee)

ZigbeeNodeInitializer::ZigbeeNodeInitializer(ZigbeeNetwork *network, QObject *parent) :
    QObject(parent),
    m_network(network)
{

}

void ZigbeeNodeInitializer::initializeNode(ZigbeeNode *node)
{
    qCDebug(dcZigbee()) << "Start initializing node internally" << node;

    // Bind the coordinator to group 0x0000
    //m_network->coordinatorNode()->deviceObject()->requestBindShortAddress(0x01, ZigbeeClusterLibrary::ClusterIdOnOff, 0x0000);

    // Initialize and configure server clusters
    foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {
        // Configure attribute reporting
        if (endpoint->hasInputCluster(ZigbeeClusterLibrary::ClusterIdPowerConfiguration)) {

            // Read current battery remaining
            qCDebug(dcZigbee()) << "Read power configuration cluster attributes" << node;
            ZigbeeClusterReply *readAttributeReply = endpoint->getInputCluster(ZigbeeClusterLibrary::ClusterIdPowerConfiguration)->readAttributes({ZigbeeClusterPowerConfiguration::AttributeBatteryPercentageRemaining});
            connect(readAttributeReply, &ZigbeeClusterReply::finished, node, [=](){
                if (readAttributeReply->error() != ZigbeeClusterReply::ErrorNoError) {
                    qCWarning(dcZigbee()) << "Failed to read power cluster attributes" << readAttributeReply->error();
                    //emit nodeInitialized(node);
                    return;
                }
                qCDebug(dcZigbee()) << "Read power configuration cluster attributes finished successfully";

                // Bind the cluster to the coordinator
                qCDebug(dcZigbee()) << "Bind power configuration cluster to coordinaotr";
                ZigbeeDeviceObjectReply * zdoReply = node->deviceObject()->requestBindIeeeAddress(endpoint->endpointId(), ZigbeeClusterLibrary::ClusterIdPowerConfiguration, m_network->coordinatorNode()->extendedAddress(), 0x01);
                connect(zdoReply, &ZigbeeDeviceObjectReply::finished, node, [=](){
                    if (zdoReply->error() != ZigbeeDeviceObjectReply::ErrorNoError) {
                        qCWarning(dcZigbee()) << "Failed to bind power cluster to coordinator" << readAttributeReply->error();
                        return;
                    }
                    qCDebug(dcZigbee()) << "Bind power configuration cluster to coordinaotr finished successfully";

                    // Configure attribute rporting for battery remaining
                    ZigbeeClusterLibrary::AttributeReportingConfiguration reportingConfig;
                    reportingConfig.attributeId = ZigbeeClusterPowerConfiguration::AttributeBatteryPercentageRemaining;
                    reportingConfig.dataType = Zigbee::Uint8;
                    reportingConfig.minReportingInterval = 300;
                    reportingConfig.maxReportingInterval = 2700;
                    reportingConfig.reportableChange = ZigbeeDataType(static_cast<quint8>(14)).data();

                    qCDebug(dcZigbee()) << "Configure attribute reporting for power configuration cluster to coordinator";
                    ZigbeeClusterReply *reportingReply = endpoint->getInputCluster(ZigbeeClusterLibrary::ClusterIdPowerConfiguration)->configureReporting({reportingConfig});
                    connect(reportingReply, &ZigbeeClusterReply::finished, this, [reportingReply](){
                        if (reportingReply->error() != ZigbeeClusterReply::ErrorNoError) {
                            qCWarning(dcZigbee()) << "Failed to read power cluster attributes" << reportingReply->error();
                            return;
                        }

                        qCDebug(dcZigbee()) << "Reporting config finished" << ZigbeeClusterLibrary::parseAttributeReportingStatusRecords(reportingReply->responseFrame().payload);
                    });
                });
            });
        }
    }


    // Initialize and configure client clusters
    foreach (ZigbeeNodeEndpoint *endpoint, node->endpoints()) {

        if (endpoint->hasOutputCluster(ZigbeeClusterLibrary::ClusterIdOnOff)) {
            // Bind command
            ZigbeeDeviceObjectReply *zdoReply = node->deviceObject()->requestBindShortAddress(endpoint->endpointId(), ZigbeeClusterLibrary::ClusterIdOnOff, m_network->coordinatorNode()->shortAddress());
            connect(zdoReply, &ZigbeeDeviceObjectReply::finished, this, [zdoReply](){
                if (zdoReply->error() != ZigbeeDeviceObjectReply::ErrorNoError) {
                    qCWarning(dcZigbee()) << "Failed to bind OnOff cluster attributes" << zdoReply->error();
                    return;
                }
            });
        }

        if (endpoint->hasOutputCluster(ZigbeeClusterLibrary::ClusterIdLevelControl)) {
            // Bind command
            ZigbeeDeviceObjectReply *zdoReply = node->deviceObject()->requestBindShortAddress(endpoint->endpointId(), ZigbeeClusterLibrary::ClusterIdLevelControl, m_network->coordinatorNode()->shortAddress());
            connect(zdoReply, &ZigbeeDeviceObjectReply::finished, this, [this, node, zdoReply](){
                if (zdoReply->error() != ZigbeeDeviceObjectReply::ErrorNoError) {
                    qCWarning(dcZigbee()) << "Failed to bind Level cluster attributes" << zdoReply->error();
                    //emit nodeInitialized(node);
                    return;
                }
            });
        }
    }

    //emit nodeInitialized(node);
}
