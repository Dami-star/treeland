// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "windowmanagement.h"
#include "window_management_impl.h"

#include <wserver.h>

#include <qwdisplay.h>

#include <QDebug>
#include <QQmlInfo>

extern "C" {
#include <wayland-server-core.h>
}

TreelandWindowManagement::TreelandWindowManagement(QObject *parent)
    : Waylib::Server::WQuickWaylandServerInterface(parent)
{
}

void TreelandWindowManagement::on_show_desktop(void *data)
{
    uint32_t state = *(static_cast<const uint32_t *>(data));
    Q_EMIT requestShowDesktop(state);
}

uint32_t TreelandWindowManagement::state()
{
    return m_handle->state;
}

void TreelandWindowManagement::showDesktop(uint32_t state)
{
    uint32_t s = 0;
    switch (state) {
    case TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_DISABLED:
        s = TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_DISABLED;
        break;
    case TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_ENABLED:
        s = TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_ENABLED;
        break;
    case TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_PREVIEW:
        s = TREELAND_WINDOW_MANAGEMENT_V1_SHOW_DESKTOP_PREVIEW;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    treeland_window_management_v1_show_desktop(m_handle, s);

    qmlWarning(this) << QString("Try to show desktop state (%1)!").arg(s);
}

void TreelandWindowManagement::create()
{
    m_handle = treeland_window_management_v1_create(server()->handle()->handle());

    m_sc.connect(&m_handle->events.show_desktop_changed,
                 this,
                 &TreelandWindowManagement::on_show_desktop);
}
