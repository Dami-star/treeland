// Copyright (C) 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <QString>
#include <QMap>
#include <QRegularExpression>

struct ProcDeviceInfo {
    QString name;
    QString physPath;
    
    bool isValid() const {
        return !name.isEmpty() && !physPath.isEmpty();
    }
};

class DeviceInfoParser {
public:
    static DeviceInfoParser& instance();
    
    void refreshDeviceInfo();
    ProcDeviceInfo getDeviceInfo(const QString& deviceName);
    QString getPhysicalPath(const QString& deviceName);
    
private:
    DeviceInfoParser() = default;
    void parseDeviceBlock(const QString& block);
    
    QMap<QString, ProcDeviceInfo> m_deviceMap;
    bool m_parsed = false;
};
