// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wquickwaylandserver.h>

#include <qwsignalconnector.h>

// #include <QList>
#include <QQmlEngine>

struct treeland_window_management_v1;

WAYLIB_SERVER_BEGIN_NAMESPACE
class WOutput;
WAYLIB_SERVER_END_NAMESPACE

class TreelandWindowManagement : public Waylib::Server::WQuickWaylandServerInterface
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TreelandWindowManagement(QObject *parent = nullptr);

    uint32_t state();
    void showDesktop(uint32_t state);

public Q_SLOTS:
    void on_show_desktop(void *data);

protected:
    void create() override;

Q_SIGNALS:
    void requestShowDesktop(uint32_t);

private:
    treeland_window_management_v1 *m_handle{ nullptr };
    QW_NAMESPACE::QWSignalConnector m_sc;
};
