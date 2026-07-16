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
- Expected life at these capacities: central ~2.5–7 months, peripheral ~8–12+ months per charge.

## Repo Architecture

Five files do everything:

- `build.yaml` — build matrix, all `nice_nano_v2`: `lily58_left nice_view_adapter nice_view` (+ snippet `studio-rpc-usb-uart`), `lily58_right nice_view_adapter nice_view`, and `settings_reset` (pairing-recovery firmware). Shield order matters: `nice_view_adapter` must precede `nice_view`.
- `config/west.yml` — pins ZMK to the **`v0.3`** tag.
- `.github/workflows/build.yml` — delegates to the reusable workflow `zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3`. Has `paths-ignore` for `keymap-drawer/**` and `**.md` so diagram/doc commits don't burn firmware builds. A second `release` job (main-branch pushes + manual runs only) downloads the `firmware` artifact, renames the uf2s to friendly names (`lily58_left.uf2` etc.), and publishes a GitHub Release via `softprops/action-gh-release@v2` — tag `vYYYY.MM.DD-<shortsha>`, structured body with flash table + commit link.
  - ⚠️ **The ZMK version is pinned in TWO places** (west.yml `revision:` and the workflow `@ref`). Always bump both together.
- `.github/workflows/draw-keymaps.yml` — keymap-drawer CI via `caksoylar/keymap-drawer/.github/workflows/draw-zmk.yml@main`; on keymap pushes it renders `keymap-drawer/lily58.svg` (+ `.yaml`) and commits them back (README embeds the SVG). Styling lives in `keymap_drawer.config.yaml`.
- `config/lily58.conf` — Kconfig: deep sleep ON (30 min idle timeout), BT TX +8 dBm, eager debounce (press 1 ms / release 10 ms), ZMK Studio on with locking off, USB logging off.
- `config/lily58.keymap` — 3 active layers (Base / Lower / Raise) + 3 `status = "reserved"` layers (ZMK Studio runtime-layer slots — keep them). Encoder bound to volume on all layers. No hold-taps/combos/macros yet.

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
pip install keymap-drawer
keymap parse -z config/lily58.keymap > keymap-drawer/lily58.yaml
keymap draw keymap-drawer/lily58.yaml > keymap-drawer/lily58.svg
```

## Key Facts & Gotchas

- **ZMK v0.3.0 is the latest stable** (verified 2026-07-16; no v0.4 tag yet — claims of "v0.4.1" refer to the separate zmk-cli repo). v0.4 will move to Zephyr 4.1 + HWMv2; nothing in it we need yet.
- The v0.3 pin avoids ZMK issue #2990 (nice!nano v2 power regression on `main` from `SOC_DCDC_NRF52X_HV`). Don't switch back to `revision: main` casually.
- `CONFIG_ZMK_STUDIO_LOCKING=n` means any USB host can edit the keymap via ZMK Studio. If ever re-enabling locking, add a `&studio_unlock` binding FIRST or Studio becomes unreachable.
- Deep sleep wipes unsaved ZMK Studio changes — save Studio edits before walking away.
- Halves lost pairing? Flash `settings_reset.uf2` (built in every CI artifact) to BOTH halves, then re-flash normal left/right firmware.
- keymap-drawer's bot commit lands on `main` after each keymap push — `git pull` before local work to avoid diverging.
- The keymap encoder binding (`&inc_dec_kp C_VOL_UP C_VOL_DN`) is inert unless an encoder is physically installed.
- nice!view custom widgets: for ZMK v0.3 use **nice-view-gem release v0.3.0** specifically (its `main` needs Zephyr 4.1 / ZMK main).
- Blank Choc caps fit fine; on choc-spaced PCBs the caps sit nearly gapless, on MX-spaced they'd show gaps — cosmetic only.

## Roadmap (planned — prepare, do NOT implement without Krish's go-ahead)

1. **Homerow mods / combos** *(prep only — wait for Krish)* — plan: `&mt`-style hold-taps on A/S/D/F + J/K/L/;, `tapping-term-ms` ~200–280, `flavor = "balanced"`, positional hold-tap (`hold-trigger-key-positions`) to kill misfires; consider `require-prior-idle-ms`. Combos live in a `combos { ... }` devicetree node. Start conservative; Sunsets' light 40 gf makes accidental holds likelier.
2. **nice!view custom widget** *(prep only — wait for Krish)* — nice-view-gem v0.3.0: add module to `west.yml`, swap shield `nice_view` → `nice_view_gem` in build.yaml, set `CONFIG_ZMK_DISPLAY=y` + `CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y`. Alternatives: nice-view-mod, zmk-nice-view-hid, zmk-nice-oled.

Done: ~~keymap-drawer CI~~, ~~settings_reset target~~ (2026-07-16, see log).

## Self-Learning Protocol (mandatory)

This repo's CLAUDE.md is **self-improving**. Whenever you (Claude) fix a problem, hit a gotcha, verify a fact, make a config decision, or learn anything a future session would benefit from:

1. Append a dated one-liner to the **Learnings Log** below (newest first). Keep entries terse: date — what — why it matters.
2. Mirror anything durable into persistent memory (`~/.claude/projects/-Users-krish-GitHub-lily58-zmk-config/memory/`) so it survives even without this file.
3. If a log entry supersedes something above (a version bump, a changed fact), update the body text too — the log records history, the body stays current.
4. **README.md must always mirror reality too** (Krish's standing rule): any keyboard change — new feature, config change, switch/keycap/battery/case swap, workflow change — updates BOTH this file and README in the same commit. Neither doc is ever allowed to drift from the physical keyboard or the repo.

## Learnings Log

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
