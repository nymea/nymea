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

#ifndef DEVICEPLUGINMOCK_H
#define DEVICEPLUGINMOCK_H

#include "devices/deviceplugin.h"

#include <QProcess>

class HttpDaemon;

class DevicePluginMock : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "io.nymea.DevicePlugin" FILE "devicepluginmock.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMock();
    ~DevicePluginMock() override;

    void discoverDevices(DeviceDiscoveryInfo *info) override;

    void setupDevice(DeviceSetupInfo *info) override;
    void postSetupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void startMonitoringAutoDevices() override;

    void startPairing(DevicePairingInfo *info) override;
    void confirmPairing(DevicePairingInfo *info, const QString &username, const QString &secret) override;

    void browseDevice(BrowseResult *result) override;
    void browserItem(BrowserItemResult *result) override;

public slots:
    void executeAction(DeviceActionInfo *info) override;
    void executeBrowserItem(BrowserActionInfo *info) override;
    void executeBrowserItemAction(BrowserItemActionInfo *info) override;

private slots:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &id);
    void onDisappear();
    void onReconfigureAutoDevice();
    void generateDiscoveredDevices(DeviceDiscoveryInfo *info);
    void generateDiscoveredPushButtonDevices(DeviceDiscoveryInfo *info);
    void generateDiscoveredDisplayPinDevices(DeviceDiscoveryInfo *info);

    void onPushButtonPressed();
    void onPluginConfigChanged();

private:
    void generateBrowseItems();

private:
    class VirtualFsNode {
    public:
        VirtualFsNode(const BrowserItem &item):item(item) {}
        ~VirtualFsNode() { while (!childs.isEmpty()) delete childs.takeFirst(); }
        BrowserItem item;
        QList<VirtualFsNode*> childs;
        void addChild(VirtualFsNode* child) {childs.append(child); }
        VirtualFsNode *findNode(const QString &id) {
            if (item.id() == id) return this;
            foreach (VirtualFsNode *child, childs) {
                VirtualFsNode *node = child->findNode(id);
                if (node) return node;
            }
            return nullptr;
        }
    };

    QHash<Device*, HttpDaemon*> m_daemons;
    QList<QPair<Action, Device*> > m_asyncActions;

    int m_discoveredDeviceCount;
    bool m_pushbuttonPressed;

    VirtualFsNode* m_virtualFs = nullptr;
};

#endif // DEVICEPLUGINMOCK_H
