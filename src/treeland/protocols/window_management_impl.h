// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "window-management-protocol.h"

struct treeland_window_management_v1 {
    struct wl_global *global;
    struct wl_list resources;

    struct {
        struct wl_signal show_desktop_changed;
    } events;

    uint32_t state; //show_desktop_state 0: disable, 1: enable, 2: preview;

    struct wl_listener display_destroy;
};

struct treeland_window_management_v1 *treeland_window_management_v1_create(struct wl_display *display);

void treeland_window_management_v1_show_desktop(struct treeland_window_management_v1 *manager,
                                                   uint32_t state);

struct treeland_window_management_v1 *treeland_window_management_from_resource(struct wl_resource *resource);

void treeland_window_management_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);

void treeland_window_management_display_destroy(struct wl_listener *listener, void *data);
