# Custom in-repo ZMK behaviors. Gated on CONFIG_ZMK_POINTING because the scroll
# behavior calls zmk_hid_mouse_scroll_set()/zmk_endpoints_send_mouse_report(),
# which only exist when the mouse HID report is compiled in.
if(CONFIG_ZMK_POINTING)
  zephyr_library()
  zephyr_library_sources(src/behaviors/behavior_mouse_scroll_tick.c)
endif()
