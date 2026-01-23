// Copyright (C) 2025 UnionTech Software Technology Co., Ltd.
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
    m_parsed = true;
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
    if (!m_parsed) {
        refreshDeviceInfo();
    }

    if (!m_deviceMap.contains(deviceName)) {
        refreshDeviceInfo();
    }

    return m_deviceMap.value(deviceName);
}

QString DeviceInfoParser::getPhysicalPath(const QString& deviceName)
{
    ProcDeviceInfo info = getDeviceInfo(deviceName);
    return info.physPath;
}
