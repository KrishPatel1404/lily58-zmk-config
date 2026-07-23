# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ZMK firmware config for **Krish's Typeractive Lily58 wireless split keyboard**. Forked from Nick Winans' Lily58 config (which descends from `unified-zmk-config-template`). There is no local toolchain requirement — firmware is built by GitHub Actions on every push.

## The Physical Keyboard (source of truth — do not re-ask)

| Part | Detail |
|---|---|
| PCB | Typeractive Lily58 wireless (partially assembled: hotswap sockets, diodes, battery jack, power switch, reset pre-soldered) |
| Controllers | 2× **nice!nano v2** (nRF52840, 1MB flash/256KB RAM, UF2 bootloader, charges at 100 mA default) |
| Displays | 2× **nice!view** (Sharp LS011B7DH03 memory-in-pixel LCD, 160×68, ~10 µA panel draw) — native support on Typeractive PCB, no CS bodge wire needed |
| Switches | 60× **Kailh Choc Sunset** tactile (Choc v1/PG1350, 40 gf actuation, 55 gf bump, 1.5 mm pre-travel / 3.0 mm total, factory-lubed) — bought from KEEBD |
| Keycaps | Blank Choc v1 caps, all white (two-prong stem; MX caps never fit): 8× convex 1u (thumbs), 2× homing 1u (F/J), 2× 1.5u, rest standard 1u |
| Batteries | 2× **1800 mAh LIP1359** PS3-controller replacement cells (3.7 V LiPo). Label likely overstated — real capacity ~700–1300 mAh typical for these. See battery safety below |
| Case | **Custom CAD-modeled case**, white print, faceplate + bottom shell, fixed **3.25° typing angle** |
| Central half | **Left** (carries the `studio-rpc-usb-uart` snippet; plug USB into left) |

### 🔴 Battery safety (LIP1359 packs)

- **Polarity is NOT standardized**: game-controller packs and the maker/JST convention are opposite ~half the time. Reversed polarity can destroy the nice!nano and is a LiPo fire risk. Any battery work: verify red/+ lands on B+ with a multimeter first, per pack — never assume both packs match.
- nice!nano v2 has **no low-voltage cutoff** — the pack's own protection circuit (PCM) is the only over-discharge guard. Cheap replacements sometimes omit it.
- Keep charging at the **default 100 mA** (LIP1359 spec max is 0.4 A; do not bridge the 500 mA jumper). Full charge ≈ 13–18 h.
- Expected life — Krish's ZMK power profiler run (1800 mAh, split, nice!view, 2 BLE profiles, 30% asleep): **central 3 mo 4 wks (±4 wks), peripheral 1 yr 3 d (±3 mo)**. Real capacity likely lower than label, scale down accordingly.

## Repo Architecture

Five files do everything:

- `build.yaml` — build matrix, all `nice_nano_v2`: `lily58_left nice_view_adapter nice_epaper` (+ snippet `studio-rpc-usb-uart`), `lily58_right nice_view_adapter nice_epaper`, and `settings_reset` (pairing-recovery firmware). `nice_epaper` = zmk-nice-oled's shield for nice!view panels; shield order matters: `nice_view_adapter` must precede it.
- `config/west.yml` — pins ZMK to the **`v0.3`** tag + the `zmk-nice-oled` widget module (mctechnology17, pinned to a commit sha — bump deliberately).
- `.github/workflows/build.yml` — delegates to the reusable workflow `zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3`. Push trigger uses a `paths` ALLOWLIST (`config/**`, `build.yaml`, the workflow itself) — docs, README, and keymap-drawer bot commits never build or release.
  - ⚠️ **PATHS ALLOWLIST RULE (from Krish):** whenever a new firmware-affecting file or folder is added to this repo — a custom shield/widget module, `boards/**`, a new snippet, extra `.conf`/`.dtsi`/`.keymap` outside `config/`, anything the compiled uf2 depends on — you MUST add its path to the `paths:` list in build.yml in the same commit, or firmware silently stops rebuilding for it. Files that only affect docs/diagrams stay OFF the list. A second `release` job (main-branch pushes + manual runs only) downloads the `firmware` artifact, renames the uf2s to friendly names (`lily58_left.uf2` etc.), and publishes a GitHub Release via `softprops/action-gh-release@v2` — tag `vYYYY.MM.DD-<shortsha>`, title = timestamp only in NZ time (`TZ='Pacific/Auckland'`, auto NZST/NZDT), minimal body (commit link + flash one-liner + recovery note — no Built/stack lines, no table, per Krish). Releases attach left/right only; `settings_reset.uf2` stays artifact-only (Krish's call).
  - ⚠️ **The ZMK version is pinned in TWO places** (west.yml `revision:` and the workflow `@ref`). Always bump both together.
- `.github/workflows/draw-keymaps.yml` — keymap-drawer CI via `caksoylar/keymap-drawer/.github/workflows/draw-zmk.yml@main`; on keymap pushes it renders `keymap-drawer/lily58.svg` (+ `.yaml`) and commits them back (README embeds the SVG). Styling lives in `keymap_drawer.config.yaml`.
- `config/lily58.conf` — Kconfig: deep sleep ON (30 min idle timeout), BT TX +8 dBm, eager debounce (press 1 ms / release 10 ms), ZMK Studio on with locking off, USB logging off, zmk-nice-oled widgets (`CONFIG_NICE_EPAPER_ON=y` + modifier indicators; animation alternatives commented).
- `config/lily58.keymap` — 3 layers (Base / Lower / Raise). No reserved ZMK Studio runtime-layer slots — removed 2026-07-23 per Krish (adding layers live in Studio now needs a reflash; recoverable from git if wanted). **Homerow mods** on A/S/D/F + J/K/L/; via custom `hml`/`hmr` hold-taps (Mac order: Ctrl-Shift-Opt-Cmd mirrored; balanced flavor, tapping-term 280 ms, require-prior-idle 150 ms; opposite-hand triggers PLUS same-hand homerow-mod positions so same-hand mods stack, e.g. Cmd+Shift — added 2026-07-23). No combos/macros yet. **Rotary encoder** (EC12, no push button) on the LEFT half — per-layer `sensor-bindings`: Base = mouse-wheel scroll ± (`&scroll_enc SCRL_UP SCRL_DOWN`, a `zmk,behavior-sensor-rotate-var` wrapping `&msc`; needs `CONFIG_ZMK_POINTING=y` + `#include <dt-bindings/zmk/pointing.h>`), Lower = brightness ±, Raise = volume ±. Hardware-verified 2026-07-23.

## Git Workflow Rule (from Krish, 2026-07-16)

**Never `git push` without asking.** Commit locally as work progresses; when a chunk is done, ask Krish whether to push. Every push burns a CI firmware build and (on main) publishes a GitHub Release — he controls the trigger.

## Common Commands

No local build needed — push and let CI build:

```bash
git push                                  # triggers firmware build
gh run watch                              # watch the build
gh run download -n firmware -D firmware/  # grab latest .uf2 artifacts
```

Flash: plug half in over USB-C → double-tap reset → mounts as UF2 mass-storage drive → drag the matching `.uf2` (left firmware to left half, right to right). Flash **both** halves whenever split-communication-relevant things change; keymap-only changes generally need only the central (left).

Optional local build (rarely needed):

```bash
west init -l config && west update && west zephyr-export
west build -s zmk/app -b nice_nano_v2 -- -DZMK_CONFIG=$PWD/config -DSHIELD="lily58_left nice_view_adapter nice_view" -DSNIPPET=studio-rpc-usb-uart
```

Keymap diagram (runs automatically in CI on keymap pushes; local render if wanted):

```bash
# needs Python >=3.10 (system python3 is 3.9): uv venv --python 3.12 (uv at ~/.local/bin/uv)
pip install keymap-drawer
# -c goes BEFORE the subcommand (inside `parse`, -c means --columns!); CI auto-detects the config
keymap -c keymap_drawer.config.yaml parse -z config/lily58.keymap > keymap-drawer/lily58.yaml
keymap -c keymap_drawer.config.yaml draw keymap-drawer/lily58.yaml > keymap-drawer/lily58.svg
```

## Key Facts & Gotchas

- **ZMK v0.3.0 is the latest stable** (verified 2026-07-16; no v0.4 tag yet — claims of "v0.4.1" refer to the separate zmk-cli repo). v0.4 will move to Zephyr 4.1 + HWMv2; nothing in it we need yet.
- The v0.3 pin avoids ZMK issue #2990 (nice!nano v2 power regression on `main` from `SOC_DCDC_NRF52X_HV`). Don't switch back to `revision: main` casually.
- `CONFIG_ZMK_STUDIO_LOCKING=n` means any USB host can edit the keymap via ZMK Studio. If ever re-enabling locking, add a `&studio_unlock` binding FIRST or Studio becomes unreachable.
- Deep sleep wipes unsaved ZMK Studio changes — save Studio edits before walking away.
- Halves lost pairing? Flash `settings_reset.uf2` (built in every CI artifact) to BOTH halves, then re-flash normal left/right firmware.
- keymap-drawer's bot commit lands on `main` after each keymap push — `git pull` before local work to avoid diverging.
- **Rotary encoder — LEFT half only** (EC12, no push button; hardware-verified 2026-07-23). Stock ZMK v0.3 lily58 shield already defines `left_encoder` (`compatible = "alps,ec11"` — that's ZMK's driver name, drives EC11/EC12 alike; `pro_micro 21`=A / `20`=B, steps 80) + `sensors` node (`triggers-per-rotation=20`, lists only left), and `lily58_left.overlay` sets it `okay` — so enabling it needs NO overlay/node redefinition: just `CONFIG_EC11=y` + `CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y` in `.conf` and one `sensor-bindings` line per keymap layer. `pro_micro` is the ZMK connector nodelabel (a shim — it stays in devicetree even though the board is a nice!nano v2, NOT a Pro Micro); on the nice!nano v2 those logical pins are the pads carrying **P0.31** (`pro_micro 21` = A) and **P0.29** (`pro_micro 20` = B), common leg to `GND`. Encoder works ONLY on the left; right half never has one. Wrong direction? swap the two keycodes. Wiring in memory (`rotary-encoder-future-add`) and the wiring artifact.
- nice!view widgets come from **zmk-nice-oled** (pinned by sha in west.yml; its nice!view shield is `nice_epaper`, NOT `nice_oled`). If ever switching to nice-view-gem: v0.3.0 release only (its `main` needs Zephyr 4.1).
- Blank Choc caps fit fine; on choc-spaced PCBs the caps sit nearly gapless, on MX-spaced they'd show gaps — cosmetic only.

## Roadmap (planned — prepare, do NOT implement without Krish's go-ahead)

Nothing queued. Current display setup is FINAL for now (Krish, 2026-07-16): central = battery/output/layer/modifier-indicators, peripheral = battery + Gem animation. Easy future toggles if he asks (all one-line conf changes in zmk-nice-oled): Bongo Cat/Luna WPM, Cat/Pokemon/Spaceman animations, static image (battery saver), sleep art, inverted mode, CapsLock indicator, Raw HID clock/volume/weather/Spotify (needs `zmk-hid-host` on the Mac), custom own art (LVGL Image Converter → C array). Widget history: zmk-nice-oled picked over nice-view-gem (fixed TX-6 look), nice-view-mod (DIY art), hammerbeam-slideshow.

Dropped by Krish (2026-07-16, don't re-suggest): combos, `&soft_off`, caps-word. Keymap refinement is Krish's own job via the web Keymap Editor — don't restructure his keymap uninvited.

Done: ~~keymap-drawer CI~~, ~~settings_reset target~~, ~~homerow mods~~, ~~keymap-drawer styling~~, ~~nice!view custom widgets (zmk-nice-oled)~~ (2026-07-16, see log), ~~left rotary encoder (EC12)~~ (2026-07-23, hardware-verified).

## Self-Learning Protocol (mandatory)

This repo's CLAUDE.md is **self-improving**. Whenever you (Claude) fix a problem, hit a gotcha, verify a fact, make a config decision, or learn anything a future session would benefit from:

1. Append a dated one-liner to the **Learnings Log** below (newest first). Keep entries terse: date — what — why it matters.
2. Mirror anything durable into persistent memory (`~/.claude/projects/-Users-krish-GitHub-lily58-zmk-config/memory/`) so it survives even without this file.
3. If a log entry supersedes something above (a version bump, a changed fact), update the body text too — the log records history, the body stays current.
4. **README.md must always mirror reality too** (Krish's standing rule): any keyboard change — new feature, config change, switch/keycap/battery/case swap, workflow change — updates BOTH this file and README in the same commit. Neither doc is ever allowed to drift from the physical keyboard or the repo.

## Learnings Log

- **2026-07-23** — Encoder layer roles finalized (Krish): Base = scroll ±, Lower = brightness ±, Raise = **volume ±** (was PgDn/PgUp). Merged with his web-editor push (origin/main 42108c1) via stash+ff+pop; only conflict was Base sensor-bindings (kept scroll). Upstream had also moved volume to Raise + brightness to Base — reconciled to the final roles above.
- **2026-07-23** — Base-layer encoder changed from volume ± to **mouse-wheel scroll ±** (Krish wants real page scroll, not PgUp/PgDn). Uses HID scroll so it works on macOS/Linux/Windows. Recipe: `#include <dt-bindings/zmk/pointing.h>`, define `scroll_enc` = `zmk,behavior-sensor-rotate-var` (`#sensor-binding-cells=2`, `bindings = <&msc>, <&msc>`, `tap-ms=30`), then `sensor-bindings = <&scroll_enc SCRL_UP SCRL_DOWN>`; enable `CONFIG_ZMK_POINTING=y`. Verified vs zmkfirmware/zmk@v0.3.0: symbol is `ZMK_POINTING` (not the old `ZMK_MOUSE`), scroll macros `SCRL_UP`/`SCRL_DOWN` live in `pointing.h` (mouse.h just includes it). CW=up, CCW=down; swap the two macros if reversed on hardware. Lower/Raise encoder bindings unchanged. Not flash-tested.
- **2026-07-23** — Encoder **hardware-verified** (Krish flashed left half, works). Part is an **EC12, no push button** (not EC11 — devicetree `compatible = "alps,ec11"` is just ZMK's driver name, drives both). Removed all WIP scaffolding: reverted build.yml to identical-with-main (no `rotary`-branch prerelease logic — now a normal release on merge), stripped WIP flags from README/CLAUDE.md. PR `rotary` → main opened.
- **2026-07-23** — keymap-drawer does NOT parse ZMK `sensor-bindings` (rotary encoders) — verified vs its README (parses combos/hold-taps/mod-morphs/sticky/layer-names only). So the left encoder never shows in `lily58.svg`; its per-layer actions are documented in the README text instead. The draw CI ran green on the encoder push but produced no diff for exactly this reason. Don't add encoder keycode legends to `keymap_drawer.config.yaml` — dead config.
- **2026-07-23** — Board is a **nice!nano v2, NOT a Pro Micro** (Krish flagged it). ZMK's `pro_micro` connector nodelabel is just a compatibility shim and STAYS in devicetree; verified against `zmkfirmware/zmk@v0.3.0` `arduino_pro_micro_pins.dtsi` gpio-map: `pro_micro 21` → `&gpio0 31` (**P0.31**), `pro_micro 20` → `&gpio0 29` (**P0.29**). So on the physical nice!nano v2 the encoder A/B solder to the pads carrying P0.31 / P0.29, common to GND. Docs/artifact now use the nice!nano P0.xx labels, not "pro micro pin N".
- **2026-07-23** — Enabled ONE rotary encoder on the LEFT half (branch `rotary`, per Krish). Verified vs zmkfirmware/zmk v0.3.0 lily58 shield + community wireless configs (fabriziopicco99, sl8a): shield ALREADY defines `left_encoder` + `sensors` (only left) and `lily58_left.overlay` enables it, so no overlay/node work needed — just uncomment `CONFIG_EC11=y` + `CONFIG_EC11_TRIGGER_GLOBAL_THREAD=y` and add per-layer `sensor-bindings` (Base vol±, Lower brightness±, Raise PgDn/PgUp). Community configs' `nice_nano.overlay` files touch ONLY nice!view/underglow, never encoders. build.yml: `rotary`-branch pushes now publish a WIP **prerelease** (tag `rotary-vDATE-sha`, name `🚧 Rotary Encoder WIP`), main unchanged. NOT flash-tested. Don't merge to main until knob verified on hardware.

- **2026-07-23** — Enabled same-hand homerow-mod STACKING (Krish wants e.g. right-hand Cmd+Shift held → left T = Cmd+Shift+T). Not a combo/macro — just added each hand's own homerow-mod positions to its hold-tap `hold-trigger-key-positions` (hml += 25 26 27 28 = A/S/D/F; hmr += 31 32 33 34 = J/K/L/;). Tradeoff: pausing then typing two adjacent homerow letters ("sad") can misfire a mod; require-prior-idle-ms=150 keeps fast typing safe — bump to 200 if it annoys. Not flash-tested.
- **2026-07-23** — Cleanup: removed the 3 `status = "reserved"` extra layers (Krish doesn't need Studio live-add slots) and aligned the Base-layer homerow-mods row to the ASCII column grid (whitespace-only, no behavior change). Confirmed NO encoder/sensor-bindings remain anywhere in repo (grep clean — already gone since 2026-07-16). Not flash-tested.
- **2026-07-16** — zmk-nice-oled merged to main; widget firmware CI-built green first try (release `v2026.07.16-f930103`). Worktree + branch cleaned up. NOT yet flash-tested on hardware — if displays act up, first suspects: `CONFIG_NICE_EPAPER_ON` flags and module sha pin in west.yml.
- **2026-07-16** — nice!view widget branch (`nice-view-widget`): zmk-nice-oled module installed. Key facts: module's nice!view shield is `nice_epaper` (NOT `nice_oled`); requires `CONFIG_NICE_EPAPER_ON=y` + `CONFIG_NICE_OLED_ON=n`; pinned to commit sha in west.yml (module has no ZMK-v0.3 tag, README says "TESTED USING ZMK v0.3.0"); RAM warning — don't stack WPM graph + Bongo Cat + Raw HID on nRF52840.
- **2026-07-16** — keymap-drawer styling done (Opus subagent, verified with local render + xmllint): Mac mod glyphs (⌘⌥⇧⌃) via `zmk_keycode_map`, theme-aware CSS in `svg_extra_style`. Gotchas: (1) `zmk_keycode_map` REPLACES the default map — must re-declare all punctuation or `EXCL` renders literally; (2) `keymap -c <config> parse` — the `-c` goes BEFORE the subcommand (inside `parse` it means --columns); (3) custom dark-mode colors need their own `@media (prefers-color-scheme: dark)` block inside `svg_extra_style`; (4) plain unicode symbols > `$$mdi:$$` glyphs (no network fetch in CI).
- **2026-07-16** — nice!view widget research: Krish picked **zmk-nice-oled** (option B) over nice-view-gem (fixed look), nice-view-mod (DIY art), hammerbeam-slideshow. Implemented on branch `nice-view-widget` in a worktree.
- **2026-07-16** — Homerow mods added (Krish's go-ahead, his order: A=Ctrl S=Shift D=Opt F=Cmd, mirrored right). Custom `hml`/`hmr` hold-taps: balanced, tapping-term 280, quick-tap 175, require-prior-idle 150, hold-trigger-on-release, opposite-hand + thumb trigger positions (Lily58: left hand = 0-5/12-17/24-29/36-42, right = 6-11/18-23/30-35/43-49, thumbs = 50-57). Old plain Ctrl kept at pos 24. If misfires: raise require-prior-idle; if missed holds: lower tapping-term.
- **2026-07-16** — Removed all encoder `sensor-bindings` from keymap (no encoder hardware). Release timestamps switched UTC → NZ time per Krish. Future encoder re-add researched + saved to memory.
- **2026-07-16** — Switched build.yml from `paths-ignore` to a `paths` allowlist (config/**, build.yaml, workflow itself) so only firmware-relevant commits build/release. Companion rule added above: new firmware-affecting paths MUST be added to the allowlist in the same commit.
- **2026-07-16** — Release body slimmed to commit line + flash one-liner + recovery note (Krish: no Built/stack lines, no file table).
- **2026-07-16** — Release style per Krish: title = date only (no "Lily58 firmware —" prefix), body has no Built line (title covers it), stack line unlabeled. NOTE: the "Source code (zip/tar.gz)" entries on releases are GitHub auto-generated links, not assets — no API/setting removes them; don't waste time trying.
- **2026-07-16** — Krish ran the ZMK power profiler (1800 mAh/nice!view/2 profiles/30% asleep): central ≈4 months, peripheral ≈1 year. Recorded in both docs. Also tightened README wording per his style preference: concise, no long explainer lines.
- **2026-07-16** — First auto-release published: `v2026.07.16-42ec651`, left/right assets only, body renders correctly. Release pipeline verified end-to-end.
- **2026-07-16** — 🐛 keymap-drawer `footer_text` is injected as raw XML: HTML entities (`&bull;`) are undefined in XML → malformed SVG → GitHub shows "Invalid image source" and the README image 404s. Use literal unicode chars (•) in footer_text. Drawer logs were clean — validate SVGs with `xmllint --noout` when the image won't render.
- **2026-07-16** — Krish: releases attach `lily58_left.uf2` + `lily58_right.uf2` only; settings_reset stays in the Actions artifact.
- **2026-07-16** — Added auto-release job to build.yml: every successful main push publishes a GitHub Release with renamed uf2s attached. Gotcha avoided: `github.event.head_commit.message` is multiline — inject only the first line (via env + `head -1`) or the markdown body breaks; it's also empty on `workflow_dispatch`, needs fallback.
- **2026-07-16** — Krish's rule: NO `git push` without asking him first. Commit locally, ask when chunk done. (Also in memory.)
- **2026-07-16** — First full CI run green: all 3 firmware targets (left/right/settings_reset) + drawer (17 s). Drawer bot commit race is REAL — it pushed `keymap-drawer render` within ~a minute of the trigger and rejected my next push; always `git pull --rebase` after any keymap push. Build logs show harmless Node 20 deprecation warnings from upstream ZMK workflow actions — not ours to fix.
- **2026-07-16** — Added keymap-drawer CI (`draw-keymaps.yml`). Verified against upstream: reusable workflow inputs `commit_message`/`amend_commit`/`destination` all exist; `parse_config.zmk_remove_keycode_prefix` and `draw_config.dark_mode: auto` + `footer_text` confirmed valid in CONFIGURATION.md. `dark_mode: auto` makes the SVG follow GitHub's theme.
- **2026-07-16** — Added `settings_reset` build target; every firmware artifact now includes recovery uf2.
- **2026-07-16** — Added `paths-ignore` (`keymap-drawer/**`, `**.md`) to build.yml so the drawer bot's SVG commit doesn't trigger a pointless ~10 min firmware build.
- **2026-07-16** — Bumped sleep timeout to 30 min (`CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=1800000`) per Krish — default 15 min felt too aggressive.
- **2026-07-16** — Enabled `CONFIG_ZMK_SLEEP=y` (Krish approved). ~2 s reconnect on wake.
- **2026-07-16** — Verified ZMK v0.3.0 = latest stable tag; v0.4 (Zephyr 4.1/HWMv2) announced but unreleased. Pin is current, in two places (west.yml + workflow ref).
- **2026-07-16** — Confirmed keymap-drawer CI recipe (reusable workflow `draw-zmk.yml@main`, config at repo-root `keymap_drawer.config.yaml`, SVGs to `keymap-drawer/`).
- **2026-07-16** — Documented LIP1359 battery hazards: polarity unstandardized (multimeter-check every pack), no board-level LVC on nice!nano v2, charge at 100 mA only, label capacity overstated (~700–1300 mAh real).
- **2026-07-16** — Repo mapped: 5 files, fork of Nick Winans' config, only PR ever merged = #15 (v0.3 pin). No settings_reset target. Central = left.

## Reference Links

- ZMK docs: https://zmk.dev/ · power profiler: https://zmk.dev/power-profiler · Studio: https://zmk.dev/docs/features/studio
- ZMK GitHub: https://github.com/zmkfirmware
- Typeractive docs (this board's build guide): https://docs.typeractive.xyz/build-guides/lily58-wireless · firmware page: https://docs.typeractive.xyz/build-guides/lily58-wireless/firmware
- nice!nano/nice!view docs: https://nicekeyboards.com/docs/nice-nano/ · https://nicekeyboards.com/docs/nice-view/
- Lily58 upstream (kata0510): https://github.com/kata0510/Lily58
- keymap-drawer: https://github.com/caksoylar/keymap-drawer
- GUI keymap editor (non-Studio alternative): https://nickcoutsos.github.io/keymap-editor/
- ZMK Studio web app: https://zmk.studio (Chrome/Edge, USB)
