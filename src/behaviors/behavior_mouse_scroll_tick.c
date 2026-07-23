/*
 * Copyright (c) 2026 Krish Patel
 *
 * SPDX-License-Identifier: MIT
 *
 * Discrete mouse-wheel scroll behavior for rotary encoders.
 *
 * The built-in &msc (zmk,behavior-input-two-axis) is velocity-over-time: it ramps a
 * target scroll speed via a timer, so an *instantaneous* encoder detent (press+release
 * back-to-back) produces ~zero movement. This behavior instead emits ONE discrete HID
 * wheel tick per press. Bind it under a zmk,behavior-sensor-rotate(-var) so the sensor
 * queues a press+release per detent:
 *
 *   msct: mouse_scroll_tick { compatible = "zmk,behavior-mouse-scroll-tick"; #binding-cells = <1>; };
 *   inc_dec_scroll: inc_dec_scroll {
 *       compatible = "zmk,behavior-sensor-rotate-var";
 *       #sensor-binding-cells = <2>;
 *       bindings = <&msct>, <&msct>;
 *   };
 *   // in a layer:  sensor-bindings = <&inc_dec_scroll 1 (-1)>;   // CW=+1 tick, CCW=-1 tick
 *
 * param1 is the signed vertical wheel delta (+1 scroll up / -1 scroll down on macOS;
 * swap the two sensor-binding params to flip). Requires CONFIG_ZMK_POINTING=y for the
 * mouse HID report + zmk_hid_mouse_scroll_set().
 */

#define DT_DRV_COMPAT zmk_behavior_mouse_scroll_tick

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/hid.h>
#include <zmk/endpoints.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    int16_t delta = (int16_t)binding->param1;
    LOG_DBG("mouse-scroll-tick position %d delta %d", event.position, delta);

    zmk_hid_mouse_scroll_set(0, delta);
    zmk_endpoints_send_mouse_report();
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    zmk_hid_mouse_scroll_set(0, 0);
    zmk_endpoints_send_mouse_report();
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mouse_scroll_tick_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define MST_INST(n)                                                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                 \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                     \
                            &behavior_mouse_scroll_tick_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MST_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
