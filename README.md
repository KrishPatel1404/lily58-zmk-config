<div align="center">

# Lily58 Wireless — ZMK Config

**Typeractive Lily58 · nice!nano v2 · nice!view · Kailh Choc Sunset · custom 3D-printed case**

[![Build ZMK firmware](https://github.com/KrishPatel1404/lily58-zmk-config/actions/workflows/build.yml/badge.svg)](https://github.com/KrishPatel1404/lily58-zmk-config/actions/workflows/build.yml)
![ZMK](https://img.shields.io/badge/ZMK-v0.3-blue)
![Board](https://img.shields.io/badge/board-nice!nano_v2-9cf)
![Wireless](https://img.shields.io/badge/split-BLE-success)

</div>

---

## ⌨️ The Build

| | |
|---|---|
| **Keyboard** | [Typeractive Lily58 wireless](https://typeractive.xyz/products/lily58-partially-assembled-pcb) — 58-key column-staggered split (6×4+4 per half), designed by [kata0510](https://github.com/kata0510/Lily58) |
| **Controllers** | 2× [nice!nano v2](https://nicekeyboards.com/docs/nice-nano/) (nRF52840, BLE 5, UF2 bootloader) |
| **Displays** | 2× [nice!view](https://nicekeyboards.com/docs/nice-view/) — Sharp memory-in-pixel LCD, ~1000× less power than OLED. Custom widgets via [zmk-nice-oled](https://github.com/mctechnology17/zmk-nice-oled): live modifier indicators, battery, layer, peripheral animation |
| **Switches** | 60× Kailh **Choc Sunset** tactile — 40 gf actuation, 55 gf bump, 3.0 mm travel, factory-lubed |
| **Keycaps** | Blank Choc v1, all white — 8× convex 1u (thumbs), 2× homing 1u, 2× 1.5u, rest 1u |
| **Batteries** | 2× 1800 mAh LIP1359 (PS3-controller replacement cells) — months per charge |
| **Case** | Custom CAD-modeled faceplate + bottom shell, 3.25° typing angle, printed in white |
| **Encoder** | 1× **EC12 rotary encoder, no push button** on the **left** half — hand-soldered to the left nice!nano v2's `P0.31` (A) + `P0.29` (B) pads, common to `GND` |

## 🗺️ Keymap

<div align="center">

![Keymap diagram](keymap-drawer/lily58.svg "Lily58 keymap")

</div>

Homerow mods on A/S/D/F + J/K/L/; (Ctrl · Shift · Opt · Cmd, mirrored).

Left encoder: **Base** = mouse-wheel scroll · **Lower** = brightness · **Raise** = volume. Scroll uses a small custom in-repo behavior (`&msct`, `src/behaviors/`) that emits one discrete HID wheel tick per detent — the built-in `&msc` can't scroll on an encoder (velocity-based; an instant detent = zero movement), and the `gpio-qdec` input-device route gives a smoother wheel but can't also do brightness/volume on v0.3.0. This keeps all three on the layer-aware keymap sensor.

Edit with [ZMK Studio](https://zmk.studio), [Keymap Editor](https://nickcoutsos.github.io/keymap-editor/), or [`config/lily58.keymap`](config/lily58.keymap).

## 🖱️ Custom behavior: `mouse-scroll-tick`

ZMK v0.3 has no way to make a rotary encoder scroll a mouse wheel: the built-in `&msc` is velocity-over-time, so an instantaneous detent produces zero movement. So this repo ships a tiny custom behavior — which also makes the repo a Zephyr module (CI compiles it automatically):

```
src/behaviors/behavior_mouse_scroll_tick.c    # emits one HID wheel tick per press
dts/bindings/behaviors/…mouse-scroll-tick.yaml # its devicetree binding
zephyr/module.yml + CMakeLists.txt             # makes the repo a ZMK module
```

`&msct` sends a single scroll report per encoder detent; it's wrapped by a `sensor-rotate-var` (`&inc_dec_scroll`) and bound per layer alongside the stock brightness/volume behaviors, so one knob does all three. **Scroll speed** = the two params in the Base layer's `sensor-bindings = <&inc_dec_scroll 1 (-1)>` (bigger = faster); **direction** = swap the signs. Only builds on the central (left) half, where HID output lives.

## 🔨 Building & Flashing

Firmware-relevant pushes build in CI ([ZMK v0.3](https://github.com/zmkfirmware/zmk/releases)); successful main builds auto-publish to [**Releases**](../../releases).

1. Grab `lily58_left.uf2` / `lily58_right.uf2` from the [latest release](../../releases/latest)
2. Plug a half in via USB-C, **double-tap the reset button** → it mounts as a `NICENANO` drive
3. Drag the matching `.uf2` on (left file → left half, right → right)

> Keymap-only changes usually need just the **left** (central) half reflashed. Anything touching split behavior: flash both.

**Halves not pairing?** Flash `settings_reset.uf2` (Actions `firmware` artifact) to both halves, then re-flash normal firmware.

## 🔋 Battery Notes

- [ZMK power profiler](https://zmk.dev/power-profiler) estimate (1800 mAh, nice!view, 2 BLE profiles, 30% asleep): **central ~4 months (±4 wks)**, **peripheral ~1 year (±3 mo)** per charge.
- Deep sleep after 30 min idle (~20 µA); first keypress reconnects in ~2 s.
- Charges via USB-C at 100 mA — full charge takes overnight.

**⚠️ Battery safety:** replacement-pack polarity isn't standardized — multimeter-verify red/+ lands on **B+** before connecting (reversed = dead board, fire risk). nice!nano has no low-voltage cutoff; use packs with a protection circuit.

## 📁 Repo Layout

```
build.yaml                    # build matrix: left/right + nice!view (+ Studio snippet), settings_reset
config/
  lily58.keymap               # layers & bindings
  lily58.conf                 # Kconfig: deep sleep (30 min), BT power, debounce, Studio
  west.yml                    # ZMK pinned to v0.3 + zmk-nice-oled widget module
keymap_drawer.config.yaml     # keymap diagram styling
keymap-drawer/                # auto-generated keymap SVG/YAML (CI output)
.github/workflows/
  build.yml                   # CI → zmkfirmware reusable build workflow
  draw-keymaps.yml            # CI → keymap-drawer diagram render
```

## 🔗 Links

[Typeractive build guide](https://docs.typeractive.xyz/build-guides/lily58-wireless) · [firmware guide](https://docs.typeractive.xyz/build-guides/lily58-wireless/firmware) · [ZMK docs](https://zmk.dev/) · [ZMK GitHub](https://github.com/zmkfirmware) · [Lily58 upstream](https://github.com/kata0510/Lily58) · [keymap-drawer](https://github.com/caksoylar/keymap-drawer)
