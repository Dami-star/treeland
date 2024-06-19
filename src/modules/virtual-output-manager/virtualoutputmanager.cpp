// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "virtualoutputmanager.h"

#include <wserver.h>

#include <qwdisplay.h>
#include <qwsignalconnector.h>

#include <woutput.h>
#include <wxdgsurface.h>
#include <wxwaylandsurface.h>

#include <QDebug>
#include <QQmlInfo>

extern "C" {
#include <wayland-server-core.h>
}

static TreelandVirtualOutputManager *VIRTUALOUTPUT_MANAGER = nullptr;

TreelandVirtualOutput::TreelandVirtualOutput(
    WSurface *target, TreelandVirtualOutputManager *manager)
    : QObject(target)
    , m_target(target)
    , m_manager(manager)
{
}

TreelandVirtualOutputManager::TreelandVirtualOutputManager(QObject *parent)
{
    if (VIRTUALOUTPUT_MANAGER) {
        qFatal("There are multiple instances of TreelandVirtualOutputManager");
    }

    VIRTUALOUTPUT_MANAGER = this;
}

void TreelandVirtualOutputManager::create(WServer *server)
{
    m_handle = treeland_virtual_output_manager_v1::create(server->handle());
    connect(m_handle,
            &treeland_virtual_output_manager_v1::virtualOutputCreated,
            this,
            &TreelandVirtualOutputManager::onVirtualOutputCreated);
    // return new WServerInterface(m_handle, m_handle->global);
}

void TreelandVirtualOutputManager::onVirtualOutputCreated(treeland_virtual_output_v1 *virtual_output)
{
    m_virtualOutput.push_back(virtual_output);
    connect(virtual_output, &treeland_virtual_output_v1::beforeDestroy, this, [this, virtual_output] {
        std::erase_if(m_virtualOutput, [virtual_output](auto *p) {
            return p == virtual_output;
        });
    });

    if(virtual_output->name.isEmpty() || !virtual_output->screen_outputs || virtual_output->screen_outputs->size > 0)
        return; // tode: send error to client

    Q_EMIT requestCreateVirtualOutput(virtual_output->name, virtual_output->screen_outputs);
}

void TreelandVirtualOutputManager::setVirtualOutput(QString name, struct wl_array *array)
{
    if(name.isEmpty() || !array || array->size > 0) {
        qDebug() << "";
        return;
    }

    Q_EMIT requestCreateVirtualOutput(name, array);
}

// TreelandVirtualOutput *TreelandVirtualOutputManager::qmlAttachedProperties(
//     QObject *target)
// {
//     if (auto *surface = qobject_cast<WXdgSurface *>(target)) {
//         return new TreelandVirtualOutput(surface->surface(),
//                                          VIRTUALOUTPUT_MANAGER);
//     }

//     if (auto *surface = qobject_cast<WXWaylandSurface *>(target)) {
//         return new TreelandVirtualOutput(surface->surface(),
//                                          VIRTUALOUTPUT_MANAGER);
//     }

//     return nullptr;
// }
