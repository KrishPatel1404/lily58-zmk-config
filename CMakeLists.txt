# Custom in-repo ZMK behavior. Compile the source into ZMK's `app` target (not a
# standalone zephyr_library) so it inherits ZMK's include dirs — otherwise
# <drivers/behavior.h> / <zmk/*.h> aren't found. Standard ZMK behavior-module
# pattern (cf. urob/zmk-* modules).
#
# Central-only guard: ZMK compiles src/hid.c + src/endpoints.c ONLY on the central
# half (the peripheral has no HID output), so zmk_hid_mouse_scroll_set() and
# zmk_endpoints_send_mouse_report() don't exist on the peripheral. Building this
# behavior there = undefined-reference link error. The encoder is on the left
# (central) anyway, so restrict to central — mirrors how ZMK guards its own
# behavior_mouse_key_press.c.
#
# Also gated on CONFIG_ZMK_POINTING (the mouse HID report the scroll rides on).
if((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_POINTING app PRIVATE src/behaviors/behavior_mouse_scroll_tick.c)
endif()
