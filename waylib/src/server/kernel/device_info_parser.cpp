// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "device_info_parser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

DeviceInfoParser& DeviceInfoParser::instance()
{
    static DeviceInfoParser parser;
    return parser;
}

void DeviceInfoParser::refreshDeviceInfo()
{
    QMutexLocker locker(&m_mutex);
    refreshDeviceInfoUnlocked();
}

void DeviceInfoParser::refreshDeviceInfoUnlocked()
{
    m_deviceMap.clear();

    QFile procFile("/proc/bus/input/devices");
    if (!procFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QString content = procFile.readAll();
    QStringList blocks = content.split("\n\n", Qt::SkipEmptyParts);

    for (const QString& block : blocks) {
        parseDeviceBlock(block.trimmed());
    }
}

void DeviceInfoParser::parseDeviceBlock(const QString& block)
{
    static const QRegularExpression nameRegex("Name=\"([^\"]+)\"");
    ProcDeviceInfo info;
    QStringList lines = block.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("N: Name=")) {
            auto match = nameRegex.match(line);
            if (match.hasMatch()) {
                info.name = match.captured(1);
            }
        }
        else if (line.startsWith("P: Phys=")) {
            info.physPath = line.mid(8);
        }
    }

    if (info.isValid()) {
        m_deviceMap[info.name] = info;
    }
}

ProcDeviceInfo DeviceInfoParser::getDeviceInfo(const QString& deviceName)
{
    QMutexLocker locker(&m_mutex);
    if (!m_deviceMap.contains(deviceName)) {
        refreshDeviceInfoUnlocked();
        if (!m_deviceMap.contains(deviceName)) {
            // Device not found even after a refresh; return empty without caching
            // so that future calls will re-try the refresh (e.g. if device appears later).
            return ProcDeviceInfo{deviceName, QString()};
        }
    }

    return m_deviceMap.value(deviceName);
}

QString DeviceInfoParser::getPhysicalPath(const QString& deviceName)
{
    ProcDeviceInfo info = getDeviceInfo(deviceName);
    return info.physPath;
}
