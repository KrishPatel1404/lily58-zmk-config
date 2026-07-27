# nice!view Displays — How They Work & How to Customize

Reference for the two nice!view screens on Krish's Lily58. Covers the hardware,
where the code lives, what shows now, and every widget you can turn on.

> **Source of truth**: the `zmk-nice-oled` module, pinned by commit sha in
> `config/west.yml` (`46f824abb2bd41f1287c5c68abd14122af6042a3`). This doc was
> generated from that exact commit's `Kconfig.defconfig`. If you bump the sha,
> re-check the flag list — options change between commits.

---

## 1. Hardware

| Thing | Detail |
|---|---|
| Panel | 2× **nice!view** — Sharp LS011B7DH03 memory-in-pixel LCD |
| Resolution | **160×68 px**, but the module treats it **portrait 68×160** (canvas W=68, H=160) |
| Color | **1 bit** — pure black/white, no grayscale, no color |
| Power | ~10 µA panel draw (memory-in-pixel = only redraws on change, near-zero idle) |
| Left screen | central half — status info (battery, layer, output, modifiers) |
| Right screen | peripheral half — battery + animation |

Displays are NOT touch, NOT backlit. Sunlight-readable, always-on cheap.

---

## 2. Where the code lives

**None of the display code is in this repo.** It all comes from the external
module `zmk-nice-oled` (mctechnology17), pulled at CI build time via
`config/west.yml`.

Graphics stack = **LVGL** (Zephyr's embedded UI lib) rendering at 1 bpp.
The module ships a *custom status screen* that replaces ZMK's stock one, plus a
pile of optional widgets. You control all of it with **Kconfig flags** in
`config/lily58.conf` — no C to write for anything built-in.

Flags that wire it up (already set):

```conf
CONFIG_ZMK_DISPLAY=y                          # display subsystem on
CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y     # use module's screen, not stock
CONFIG_NICE_EPAPER_ON=y                        # nice!view profile (shield = nice_epaper)
CONFIG_NICE_OLED_ON=n                          # NOT the OLED profile
```

> **Naming trap**: the module is called "nice-oled" and its OLED shield is
> `nice_oled`, but the **nice!view shield is `nice_epaper`**. You use epaper.
> Every flag is still prefixed `NICE_OLED_...` regardless. Don't let the names
> fool you.

Three display "profiles" exist in the module — pick ONE:
`NICE_OLED_ON` (SSD1306 OLED), `NICE_EPAPER_ON` (nice!view — **ours**),
`NICE_CUSTOM_ON` (roll-your-own dimensions). Many defaults differ per profile.

---

## 3. What shows RIGHT NOW

**Left (central):** battery %, output icon (BLE profile / USB), current layer,
live modifier indicators (⌘⌥⇧⌃ light when a homerow mod is held).

**Right (peripheral):** battery % + **Gem** animation.

Current explicit flags in `config/lily58.conf`:

```conf
CONFIG_NICE_EPAPER_ON=y
CONFIG_NICE_OLED_ON=n
CONFIG_NICE_OLED_WIDGET_MODIFIERS_INDICATORS=y
```

Everything else is the module's **default-on** set (see §5).

---

## 4. How to customize

1. Add/flip a `CONFIG_...=y|n` line in `config/lily58.conf`.
2. Commit. Push (triggers CI build).
3. Flash the affected half(s). Central-only widget → reflash **left**.
   Peripheral animation → reflash **right**. When unsure, flash both.

**Widget positions are ALL configurable** too — every widget has
`_CUSTOM_X` / `_CUSTOM_Y` int flags (pixel coords on the 68×160 canvas). Example:
move the layer readout: `CONFIG_NICE_OLED_WIDGET_LAYER_CUSTOM_Y=140`.

> **RAM ceiling (nRF52840)**: do NOT stack heavy widgets — e.g. WPM graph +
> Bongo Cat + Raw HID together can exhaust RAM and fail the build or crash at
> runtime. Add one heavy thing at a time and test.

---

## 5. Full widget menu (from the pinned commit)

Flag = prefix every name below with `CONFIG_` and append `=y` / `=n`.
"Default" = value when you don't set it (for the **nice!view / epaper** profile).

### Global / display-wide
| Flag | Default | Does |
|---|---|---|
| `NICE_OLED_WIDGET_INVERTED` | n | Invert colors (white-on-black) |
| `NICE_OLED_SHOW_SLEEP_ART_ON_IDLE` | n | Show sleep art when idle |
| `NICE_OLED_SHOW_SLEEP_ART_ON_SLEEP` | n | Show sleep art when asleep |
| `NICE_OLED_WIDGET_OUTPUT_BACKGROUND` | y (epaper) | Output icon background box |

### Central (left) status widgets
| Flag | Default | Does |
|---|---|---|
| `NICE_OLED_WIDGET_LAYER` | y | Current layer readout |
| `NICE_OLED_WIDGET_PROFILE_BIG` | y (epaper) | Bigger BLE-profile display |
| `NICE_OLED_WIDGET_MODIFIERS_INDICATORS` | y | Held-modifier icons (**we use this**) |
| `NICE_OLED_WIDGET_HID_INDICATORS` | y | Caps/Num/Scroll-lock indicators |

**Modifier indicator styling** (only if MODIFIERS on):
- Style: `..._FIXED_LETTER` (C S A G text) or `..._FIXED_SYMBOL` (icons, default).
- OS symbols: `..._FIXED_SYMBOL_MACOS` (default, ⌘⌥) or `..._SYMBOL_WINDOWS`.
- Layout: `..._FIXED_HOR` (one line) / `..._FIXED_VER` (stacked) / `..._FIXED_BOX` (2×2 grid, default).
- Animated variants: `..._MODIFIERS_INDICATORS_LUNA` or `..._BONGO_CAT` (replace static icons with an animation reacting to mods).

### WPM widgets (central; all need typing-speed tracking)
| Flag | Default | Does |
|---|---|---|
| `NICE_OLED_WIDGET_WPM` | y | Master WPM widget |
| `NICE_OLED_WIDGET_WPM_NUMBER` | n | Plain WPM number |
| `NICE_OLED_WIDGET_WPM_SPEEDOMETER` | n | Analog gauge + needle |
| `NICE_OLED_WIDGET_WPM_GRAPH` | n | Line graph (heavy — RAM) |
| `NICE_OLED_WIDGET_WPM_LUNA` | n | **Luna the dog** — runs faster as you type |
| `NICE_OLED_WIDGET_WPM_BONGO_CAT` | y | **Bongo Cat** — taps to your WPM |

Graph range: `..._WPM_GRAPH_FIXED_RANGE` (y) + `..._FIXED_RANGE_MAX` (100).

### Peripheral (right) animations — pick ONE
| Flag | Default | Does |
|---|---|---|
| `NICE_OLED_WIDGET_ANIMATION_PERIPHERAL` | y | Master toggle (animation on right) |
| `..._ANIMATION_PERIPHERAL_GEM` | n* | Spinning gem |
| `..._ANIMATION_PERIPHERAL_CAT` | y* | Cat |
| `..._ANIMATION_PERIPHERAL_HEAD` | n | Nodding head |
| `..._ANIMATION_PERIPHERAL_POKEMON` | n | Pokémon |
| `..._ANIMATION_PERIPHERAL_SPACEMAN` | n | Spaceman |
| `..._ANIMATION_PERIPHERAL_SMART_BATTERY` | n | Battery-themed animation |
| `..._ANIMATION_PERIPHERAL_MS` | (per-anim) | Frame duration ms |

> \* The module's raw default is **Cat**, but Krish's setup shows **Gem** — so Gem
> is being enabled somewhere in the effective config / an earlier build choice.
> If you want to be explicit, set `CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_GEM=y`
> and the others `=n`.

**Static image instead of animation** (battery saver — no frames redrawing):
- `NICE_OLED_WIDGET_STATIC_IMAGE_PERIPHERAL=y` (auto-on if animation off)
- Images: `..._STATIC_IMAGE_PERIPHERAL_VIM`, `..._VIP_MARCOS`.

### Raw HID widgets (need a host daemon — see §6)
| Flag | Default | Does |
|---|---|---|
| `NICE_OLED_WIDGET_RAW_HID` | n | Master Raw-HID toggle |
| `..._RAW_HID_TIME` | y | Clock |
| `..._RAW_HID_VOLUME` | y | System volume |
| `..._RAW_HID_LAYOUT` | y | Keyboard layout (list: `..._LAYOUT_LIST`) |
| `..._RAW_HID_WEATHER` | n | Temperature |
| `..._RAW_HID_MEDIA_PLAYER_SPOTIFY_MACOS` | n | Now-playing Spotify (macOS) |

### Battery display options (central)
| Flag | Does |
|---|---|
| `..._CENTRAL_SHOW_BATTERY_PERIPHERAL_ALL` | Show central + all peripheral batteries |
| `..._CENTRAL_SHOW_BATTERY_PERIPHERAL_ONLY` | Only peripheral batteries |
| `..._CENTRAL_SHOW_BATTERY_PERIPHERAL_AND_CENTRAL` | 1 peripheral + central |

### Responsive mode (smoother animation, more resources)
`NICE_OLED_WIDGET_RESPONSIVE=y` bumps the display thread stack (2560→4096) and
priority (5→3). Optional `..._RESPONSIVE_BONGO_CAT`.

---

## 6. Raw HID — what it needs

The clock / volume / weather / Spotify widgets can't get that data from the
keyboard alone. They receive it over **Raw HID** from a **daemon running on the
Mac** (e.g. `zmk-hid-host` or QMK-XAP-style bridge). Without the host app the
widgets show nothing/placeholder. Extra setup, extra always-running process.
Usage page/usage default `0xFF60` / `0x61`, report size 32.

---

## 7. Fully custom art / new widgets

Only path that needs real code:

1. Make a 1-bpp image (68×160 or smaller region).
2. Convert with the **LVGL Image Converter** → C array (`.c`).
3. Drop it into a **fork** of `zmk-nice-oled`, wire it as a widget/animation.
4. Re-pin `west.yml` `revision:` to your fork's commit sha.

You can't add art from this repo alone — widgets must live in the module. For a
one-off static image, forking + swapping an existing animation's frames is the
least-effort route.

---

## 8. Quick recipes

```conf
# Swap right-half animation Gem -> Spaceman
CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_GEM=n
CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_SPACEMAN=y

# Bongo Cat reacting to typing speed on the LEFT
CONFIG_NICE_OLED_WIDGET_WPM=y
CONFIG_NICE_OLED_WIDGET_WPM_BONGO_CAT=y

# Battery-saver: static image on right, no animation
CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL=n
CONFIG_NICE_OLED_WIDGET_STATIC_IMAGE_PERIPHERAL=y

# Invert both screens (white on black)
CONFIG_NICE_OLED_WIDGET_INVERTED=y

# Show peripheral battery on the central screen too
CONFIG_NICE_OLED_WIDGET_CENTRAL_SHOW_BATTERY_PERIPHERAL_ALL=y

# Add a clock (REQUIRES a Raw-HID host daemon on the Mac)
CONFIG_NICE_OLED_WIDGET_RAW_HID=y
CONFIG_NICE_OLED_WIDGET_RAW_HID_TIME=y
```

After any change: commit → push → flash the affected half.

---

*Generated 2026-07-24 from zmk-nice-oled @ `46f824a`. This doc affects docs only —
NOT on the build.yml paths allowlist, so editing it won't trigger a firmware build.*
