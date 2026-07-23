# Custom in-repo ZMK behavior. Compile the source into ZMK's `app` target (not a
# standalone zephyr_library) so it inherits ZMK's include dirs — otherwise
# <drivers/behavior.h> / <zmk/*.h> aren't found. This is the standard ZMK
# behavior-module pattern (cf. urob/zmk-* modules).
# Gated on CONFIG_ZMK_POINTING: the behavior calls zmk_hid_mouse_scroll_set() /
# zmk_endpoints_send_mouse_report(), which only exist with the mouse HID report.
target_sources_ifdef(CONFIG_ZMK_POINTING app PRIVATE src/behaviors/behavior_mouse_scroll_tick.c)
