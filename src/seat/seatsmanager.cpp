// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "seatsmanager.h"

#include <qwinputdevice.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(treelandSeatManager, "treeland.seatmanager", QtInfoMsg)

SeatsManager::SeatsManager(WServer *server, QObject *parent)
    : QObject(parent)
    , m_server(server)
{
}

SeatsManager::~SeatsManager()
{
    QMap<QString, WSeat*> seatsToDelete;
    seatsToDelete.swap(m_seats);
    m_deviceRules.clear();
    m_defaultSeat = nullptr;

    for (auto seat : std::as_const(seatsToDelete)) {
        if (m_server) {
            m_server->detach(seat);
        }
        delete seat;
    }
}

WSeat *SeatsManager::createSeat(const QString &name, bool isFallback)
{
    if (m_seats.contains(name)) {
        qCDebug(treelandSeatManager) << "Seat" << name << "already exists";
        return m_seats[name];
    }

    WSeat *seat = new WSeat(name);
    m_seats[name] = seat;

    if (isFallback) {
        m_defaultSeat = seat;
    }

    if (m_server && m_server->isRunning()) {
        m_server->attach(seat);
    }

    Q_EMIT seatAdded(seat);
    qCDebug(treelandSeatManager) << "Created seat:" << name << "fallback:" << isFallback;
    
    return seat;
}

void SeatsManager::removeSeat(const QString &name)
{
    if (!m_seats.contains(name)) {
        qCWarning(treelandSeatManager) << "Cannot remove non-existent seat:" << name;
        return;
    }

    WSeat *seat = m_seats.take(name);
    
    if (m_defaultSeat == seat && !m_seats.isEmpty()) {
        m_defaultSeat = m_seats.begin().value();
        qCDebug(treelandSeatManager) << "Fallback seat changed to:" << m_defaultSeat->name();
    } else if (m_defaultSeat == seat) {
        m_defaultSeat = nullptr;
        qCDebug(treelandSeatManager) << "No fallback seat available";
    }
    
    QList<WInputDevice*> devices = seat->deviceList();
    for (auto device : std::as_const(devices)) {
        seat->detachInputDevice(device);
        
        // Ensure device is reassigned to appropriate seat
        if (!m_seats.isEmpty()) {
            bool assigned = autoAssignDevice(device);
            if (!assigned) {
                qCWarning(treelandSeatManager) << "Failed to reassign device after seat removal:" << device->name();
                // Assign to fallback seat if available
                if (auto fallback = fallbackSeat()) {
                    fallback->attachInputDevice(device);
                    qCDebug(treelandSeatManager) << "Device assigned to fallback seat:" << device->name();
                }
            }
        }
    }
    
    if (m_server) {
        m_server->detach(seat);
    }

    Q_EMIT seatRemoved(seat);
    qCDebug(treelandSeatManager) << "Removed seat:" << name;
    
    delete seat;
    
    // Clean up device rules for removed seat
    m_deviceRules.remove(name);
}

void SeatsManager::removeSeat(WSeat *seat)
{
    Q_ASSERT(seat);
        
    QString seatName;
    for (auto it = m_seats.begin(); it != m_seats.end(); ++it) {
        if (it.value() == seat) {
            seatName = it.key();
            break;
        }
    }
    
    if (!seatName.isEmpty()) {
        removeSeat(seatName);
    } else {
        qCWarning(treelandSeatManager) << "Attempted to remove a seat that is not managed by SeatsManager";
    }
}

WSeat *SeatsManager::getSeat(const QString &name) const
{
    return m_seats.value(name);
}

QList<WSeat*> SeatsManager::seats() const
{
    return m_seats.values();
}

WSeat *SeatsManager::fallbackSeat() const
{
    return m_defaultSeat;
}

void SeatsManager::assignDeviceToSeat(WInputDevice *device, const QString &seatName)
{
    if (!device) {
        qCWarning(treelandSeatManager) << "Cannot assign null device to seat";
        return;
    }
    
    for (auto seat : std::as_const(m_seats)) {
        if (seat->deviceList().contains(device)) {
            if (seat->name() == seatName) {
                qCDebug(treelandSeatManager) << "Device" << device->name() << "already assigned to seat" << seatName;
                return;
            }
            
            seat->detachInputDevice(device);
            qCDebug(treelandSeatManager) << "Device" << device->name() << "detached from seat" << seat->name();
            break;
        }
    }
    
    if (m_seats.contains(seatName)) {
        m_seats[seatName]->attachInputDevice(device);
        qCDebug(treelandSeatManager) << "Device" << device->name() << "assigned to seat" << seatName;
    } else if (fallbackSeat()) {
        fallbackSeat()->attachInputDevice(device);
        qCDebug(treelandSeatManager) << "Device" << device->name() << "assigned to fallback seat";
    } else {
        qCWarning(treelandSeatManager) << "Cannot assign device" << device->name() << "- no seats available";
    }
}

bool SeatsManager::autoAssignDevice(WInputDevice *device)
{
    if (!device) {
        qCWarning(treelandSeatManager) << "Cannot auto-assign null device";
        return false;
    }
    
    for (auto seat : std::as_const(m_seats)) {
        if (seat->deviceList().contains(device)) {
            qCDebug(treelandSeatManager) << "Device" << device->name() << "already assigned to seat" << seat->name();
            return true;
        }
    }
    
    WSeat *targetSeat = findSeatForDevice(device);
    
    if (targetSeat) {
        targetSeat->attachInputDevice(device);
        qCDebug(treelandSeatManager) << "Device" << device->name() << "auto-assigned to seat" << targetSeat->name();
        return true;
    } else if (fallbackSeat()) {
        fallbackSeat()->attachInputDevice(device);
        qCDebug(treelandSeatManager) << "Device" << device->name() << "auto-assigned to fallback seat";
        return true;
    }
    
    qCWarning(treelandSeatManager) << "Failed to auto-assign device" << device->name();
    return false;
}

void SeatsManager::addDeviceRule(const QString &seatName, const QString &rule)
{
    if (seatName.isEmpty()) {
        qCWarning(treelandSeatManager) << "Cannot add device rule for seat with empty name";
        return;
    }
    
    if (rule.isEmpty()) {
        qCWarning(treelandSeatManager) << "Cannot add empty device rule";
        return;
    }
    
    if (!m_seats.contains(seatName)) {
        qCWarning(treelandSeatManager) << "Cannot add device rule for non-existent seat:" << seatName;
        return;
    }
    
    QRegularExpression regex(rule);
    if (!regex.isValid()) {
        qCWarning(treelandSeatManager) << "Invalid regex pattern for device rule:" << rule << "Error:" << regex.errorString();
        return;
    }
    
    if (!m_deviceRules.contains(seatName)) {
        m_deviceRules[seatName] = QList<QRegularExpression>();
    }
    
    // Check for duplicate rules
    for (const auto &existingRule : std::as_const(m_deviceRules[seatName])) {
        if (existingRule.pattern() == rule) {
            qCDebug(treelandSeatManager) << "Device rule already exists for seat" << seatName << ":" << rule;
            return;
        }
    }
    
    m_deviceRules[seatName].append(regex);
    qCDebug(treelandSeatManager) << "Added device rule for seat" << seatName << ":" << rule;
}

void SeatsManager::removeDeviceRule(const QString &seatName, const QString &rule)
{
    if (!m_deviceRules.contains(seatName)) {
        qCDebug(treelandSeatManager) << "No device rules for seat:" << seatName;
        return;
    }
        
    QRegularExpression regex(rule);
    if (!regex.isValid()) {
        qCWarning(treelandSeatManager) << "Invalid regex pattern:" << rule;
        return;
    }
        
    auto &rules = m_deviceRules[seatName];
    for (int i = 0; i < rules.size(); i++) {
        if (rules[i].pattern() == regex.pattern()) {
            rules.removeAt(i);
            qCDebug(treelandSeatManager) << "Removed device rule from seat" << seatName << ":" << rule;
            break;
        }
    }
    
    if (rules.isEmpty()) {
        m_deviceRules.remove(seatName);
        qCDebug(treelandSeatManager) << "All device rules removed for seat:" << seatName;
    }
}

QStringList SeatsManager::deviceRules(const QString &seatName) const
{
    if (!m_deviceRules.contains(seatName))
        return QStringList();
        
    QStringList result;
    const auto &rules = m_deviceRules[seatName];
    for (const auto &regex : std::as_const(rules)) {
        result.append(regex.pattern());
    }
    
    return result;
}

void SeatsManager::loadConfig(const QJsonObject &config)
{
    qCDebug(treelandSeatManager) << "Loading seat configuration";
    
    QMap<QString, WSeat*> seatsToDelete;
    seatsToDelete.swap(m_seats);
    m_deviceRules.clear();
    m_defaultSeat = nullptr;

    for (auto seat : std::as_const(seatsToDelete)) {
        if (m_server) {
            m_server->detach(seat);
        }
        Q_EMIT seatRemoved(seat);
        delete seat;
    }

    QJsonArray seatsArray = config["seats"].toArray();
    for (const auto &seatValue : std::as_const(seatsArray)) {
        QJsonObject seatObj = seatValue.toObject();
        QString name = seatObj["name"].toString();
        bool isFallback = seatObj["fallback"].toBool();

        createSeat(name, isFallback);

        QJsonArray rulesArray = seatObj["deviceRules"].toArray();
        for (const auto &ruleValue : std::as_const(rulesArray)) {
            QString rule = ruleValue.toString();
            addDeviceRule(name, rule);
        }
    }

    if (m_seats.isEmpty()) {
        qCDebug(treelandSeatManager) << "No seats in config, creating default seat0";
        createSeat("seat0", true);
    }

    if (!m_defaultSeat && !m_seats.isEmpty()) {
        m_defaultSeat = m_seats.begin().value();
        qCDebug(treelandSeatManager) << "Set fallback seat to:" << m_defaultSeat->name();
    }
    
    qCDebug(treelandSeatManager) << "Loaded" << m_seats.size() << "seats";
}

QJsonObject SeatsManager::saveConfig() const
{
    QJsonObject config;
    QJsonArray seatsArray;
    
    for (auto it = m_seats.begin(); it != m_seats.end(); ++it) {
        QString name = it.key();
        WSeat *seat = it.value();
        
        QJsonObject seatObj;
        seatObj["name"] = name;
        seatObj["fallback"] = (seat == m_defaultSeat);
        
        QJsonArray rulesArray;
        const auto rules = deviceRules(name);
        for (const auto &rule : std::as_const(rules)) {
            rulesArray.append(rule);
        }
        seatObj["deviceRules"] = rulesArray;
        
        seatsArray.append(seatObj);
    }
    
    config["seats"] = seatsArray;
    qCDebug(treelandSeatManager) << "Saved configuration for" << m_seats.size() << "seats";
    return config;
}

bool SeatsManager::matchesDevice(WInputDevice *device, const QList<QRegularExpression> &rules)
{
    if (!device)
        return false;
        
    QString deviceName = device->name();
    WInputDevice::Type deviceType = device->type();
    QString devicePath = device->devicePath();

    QString deviceInfo = QString("%1:%2")
                        .arg(static_cast<int>(deviceType))
                        .arg(deviceName);
    QString deviceInfoWithPath = QString("%1:%2|%3")
                                .arg(static_cast<int>(deviceType))
                                .arg(deviceName)
                                .arg(devicePath);

    for (const auto &rule : rules) {
        if (rule.match(deviceInfo).hasMatch())
            return true;
        if (rule.match(deviceInfoWithPath).hasMatch())
            return true;
    }
    return false;
}

bool SeatsManager::deviceMatchesSeat(WInputDevice *device, WSeat *seat) const
{
    if (!device || !seat)
        return false;
        
    QString seatName = seat->name();
    
    if (!m_deviceRules.contains(seatName)) {
        return false;
    }

    bool matches = matchesDevice(device, m_deviceRules[seatName]);
    return matches;
}

WSeat *SeatsManager::findSeatForDevice(WInputDevice *device) const
{
    if (!device)
        return nullptr;
        
    for (auto seat : std::as_const(m_seats)) {
        if (seat->deviceList().contains(device)) {
            return seat;
        }
    }

    for (auto it = m_seats.begin(); it != m_seats.end(); ++it) {
        WSeat *seat = it.value();
        if (seat == m_defaultSeat) {
            continue;
        }
        if (deviceMatchesSeat(device, seat)) {
            return seat;
        }
    }

    WSeat *fallbackSeat = this->fallbackSeat();
    if (fallbackSeat) {
        bool hasRules = m_deviceRules.contains(fallbackSeat->name()) &&
                       !m_deviceRules[fallbackSeat->name()].isEmpty();

        if (hasRules && deviceMatchesSeat(device, fallbackSeat)) {
            return fallbackSeat;
        } else if (!hasRules) {
            return fallbackSeat;
        }
    }
    
    return nullptr;
}
