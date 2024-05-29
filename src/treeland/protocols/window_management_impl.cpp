// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "window_management_impl.h"
#include "utils.h"

#include <wayland-server-core.h>
#include <cassert>

#define TREELAND_WINDOW_MANAGEMENT_V1_VERSION 1


void treeland_window_management_handle_show_desktop([[maybe_unused]]struct wl_client *client,
                                              struct wl_resource *resource,
                                              uint32_t state) {
    auto *manager = treeland_window_management_from_resource(resource);
    wl_signal_emit(&manager->events.show_desktop_changed, &state);
}

static const struct treeland_window_management_v1_interface window_management_impl {
    .show_desktop = treeland_window_management_handle_show_desktop,
    .destroy = resource_handle_destroy,
};

struct treeland_window_management_v1 *treeland_window_management_v1_create(struct wl_display *display)
{
    auto *manager =
        static_cast<treeland_window_management_v1 *>(calloc(1, sizeof(treeland_window_management_v1)));
    if (manager == nullptr) {
        return nullptr;
    }
    manager->global = wl_global_create(display,
                                       &treeland_window_management_v1_interface,
                                       TREELAND_WINDOW_MANAGEMENT_V1_VERSION,
                                       manager,
                                       treeland_window_management_bind);

    wl_list_init(&manager->resources);
    wl_signal_init(&manager->events.show_desktop_changed);

    manager->display_destroy.notify = treeland_window_management_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);
    return manager;
}

void treeland_window_management_v1_show_desktop(treeland_window_management_v1 *manager,
                                                   uint32_t state)
{
    manager->state = state;
    wl_resource *resource;
    wl_list_for_each(resource, &manager->resources, link)
    {
        treeland_window_management_v1_send_show_desktop_changed(resource, state);
    }
}

struct treeland_window_management_v1 *treeland_window_management_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &treeland_window_management_v1_interface,
                                   &window_management_impl));
    struct treeland_window_management_v1 *manager =
        static_cast<treeland_window_management_v1 *>(wl_resource_get_user_data(resource));
    assert(manager != NULL);
    return manager;
}

void treeland_window_management_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct treeland_window_management_v1 *manager = static_cast<treeland_window_management_v1 *>(data);

    struct wl_resource *resource =
        wl_resource_create(client, &treeland_window_management_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &window_management_impl, manager, NULL);
    wl_list_insert(&manager->resources, wl_resource_get_link(resource));

    treeland_window_management_v1_send_show_desktop_changed(resource, manager->state);
}

void treeland_window_management_display_destroy(struct wl_listener *listener,
                                           [[maybe_unused]] void *data)
{
    struct treeland_window_management_v1 *manager =
        wl_container_of(listener, manager, display_destroy);

    wl_global_destroy(manager->global);
    wl_list_remove(&manager->display_destroy.link);
    free(manager);
}
