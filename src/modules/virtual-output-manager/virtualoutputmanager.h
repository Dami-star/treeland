// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once
#include "impl/virtual_output_manager_impl.h"

#include <wxdgsurface.h>
#include <QObject>
#include <wserver.h>


QW_USE_NAMESPACE
WAYLIB_SERVER_USE_NAMESPACE

class TreelandVirtualOutputManager;

class TreelandVirtualOutput : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    TreelandVirtualOutput(WSurface *target, TreelandVirtualOutputManager *manager);

Q_SIGNALS:

private:
    WSurface *m_target;
    TreelandVirtualOutputManager *m_manager;
};

class TreelandVirtualOutputManager : public QObject, public WServerInterface
{
    Q_OBJECT

public:
    explicit TreelandVirtualOutputManager(QObject *parent = nullptr);

    // static TreelandVirtualOutput *qmlAttachedProperties(QObject *target);
    void setVirtualOutput(QString name, struct wl_array *array);

Q_SIGNALS:

    void requestCreateVirtualOutput(QString name, struct wl_array *array);

private Q_SLOTS:
    void onVirtualOutputCreated(treeland_virtual_output_v1 *virtual_output);

private:
    void create(WServer *server) override;

    std::vector<treeland_virtual_output_v1 *> m_virtualOutput;
    treeland_virtual_output_manager_v1 *m_handle{ nullptr };
};
