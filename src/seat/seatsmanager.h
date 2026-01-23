// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wseat.h>
#include <winputdevice.h>
#include <wserver.h>

#include <QObject>
#include <QMap>
#include <QRegularExpression>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE

// SeatsManager manages multi-seat configuration and device assignment for treeland.
// This class implements compositor-specific policies for seat creation, device
// assignment rules, and configuration persistence.
class SeatsManager : public QObject
{
    Q_OBJECT

public:
    explicit SeatsManager(WServer *server, QObject *parent = nullptr);
    ~SeatsManager();

    // Seat management
    WSeat *createSeat(const QString &name, bool isFallback = false);
    void removeSeat(const QString &name);
    void removeSeat(WSeat *seat);
    WSeat *getSeat(const QString &name) const;
    QList<WSeat*> seats() const;
    WSeat *fallbackSeat() const;
    
    // Device assignment
    void assignDeviceToSeat(WInputDevice *device, const QString &seatName);
    bool autoAssignDevice(WInputDevice *device);
    
    // Configuration management
    void loadConfig(const QJsonObject &config);
    QJsonObject saveConfig() const;
    
    // Device matching rules
    void addDeviceRule(const QString &seatName, const QString &rule);
    void removeDeviceRule(const QString &seatName, const QString &rule);
    QStringList deviceRules(const QString &seatName) const;

    // Device matching (moved from WSeat)
    static bool matchesDevice(WInputDevice *device, const QList<QRegularExpression> &rules);
    bool deviceMatchesSeat(WInputDevice *device, WSeat *seat) const;
    WSeat *findSeatForDevice(WInputDevice *device) const;

Q_SIGNALS:
    void seatAdded(WSeat *seat);
    void seatRemoved(WSeat *seat);

private:
    WServer *m_server = nullptr;
    QMap<QString, WSeat*> m_seats;
    QMap<QString, QList<QRegularExpression>> m_deviceRules;
    WSeat *m_defaultSeat = nullptr;
};
