# Steam Frame defaults

What OpenFrame sets for the Steam Frame, against OpenH1's defaults, and why.
Every setting stays the player's: `config.toml` (`~/.local/share/OpenFrame`),
VR Setup, or one launch's `session.env` (`HALO_*` lines, or the wrapper's
`OPENFRAME_*`).

The Frame, as these defaults assume it: Adreno 750 on Turnip (Vulkan),
SteamVR 2.18's OpenXR runtime, eyes of 1728x1728 recommended, 72, 90 or
120 Hz chosen by the app (`XR_FB_display_refresh_rate`), eye tracking
(`XR_EXT_eye_gaze_interaction`), no space warp or frame synthesis, and
the wrapper's `TU_DEBUG=sysmem`.

OpenH1's VR defaults were already chosen and measured on a Frame (its
README, "Steam Frame" and "Frame rate"), so OpenFrame changes none of them
today: the table says where a change is worth measuring, and what to
measure, before it becomes a default.

| Setting | OpenH1 default | OpenFrame default | Why | Risk of changing it |
| --- | --- | --- | --- | --- |
| `vr.refresh_rate` | `90` | `90` (the installer writes it when `config.toml` lacks it; the wrapper's `OPENFRAME_REFRESH_HZ`, when given, wins at run time) | 11.1 ms a frame leaves the GPU room at 1728x1728 (OpenH1 measured about 8 ms GPU there with its older renderer); 72 Hz for heavy levels | 120 Hz needs the GPU under about 7 ms: unmeasured with the Vulkan renderer, SteamVR halves the rate when it is missed |
| `vr.resolution_scale` | `1.0` (1728x1728) | `1.0` | The runtime's recommended size; 1.25 was at the edge of 72 Hz, 1.5 dropped to 36 Hz (OpenH1's Frame numbers) | Above 1.0 at 90 Hz drops frames |
| `vr.foveation` | `"eye"` | `"eye"` | SteamVR on the Frame offers `XR_EXT_eye_gaze_interaction` (its client logs list it); half the eyes' pixels drawn; without tracking it falls back to `"fixed"` | `"off"` doubles the eye pass's pixels |
| `vr.foveation_inset` | `0.5` | `0.5` | 864x864 inset of 1728; the blend hides its edge | 0.4 is faster but its edge may show where the eye tracker lags (unmeasured on the Frame) |
| `vr.depth` | `false` | `false` | SteamVR on the Frame cannot take the eyes' depth, and it is never sent while foveated | none (true does nothing useful there) |
| `vr.late_latch` | `true` | `true` | Pacing: not touched by this review | n/a |
| Eye anti-aliasing | none (the VR eyes have no FXAA/SMAA pass; MSAA cost 15 ms at 2x on the old renderer) | none | No budget at 90 Hz | Alpha-tested foliage and grates crawl; MSAA on the Vulkan eyes is unmeasured on the Frame |
| `display.graphics` | `"enhanced"` (per-pixel lighting, sharper shadows, anisotropic filtering, enhanced dynamic lights) | `"enhanced"` | OpenH1's look; its cost on the Frame's GPU is unmeasured ("to be measured" in OpenH1's README) | If `HALO_GPU_STATS=1 HALO_VR_GPU_TIME=1` shows the GPU over budget, `"classic"` is the first thing to try |
| `vr.hud_inset`, `vr.hud_offset_*`, `vr.hud_scale`, `vr.hud_<group>_*` | none (OpenFrame's keys) | `0`, sizes `1`: the corners where OpenH1 puts them | They fit the stock Frame's view; with a glasses spacer the view is narrower and the corners are cut off (see below) | Inset pulls the shields in too; above about 0.3 the corners crowd the reticle |
| `vr.hud_lag` | `0.03` s | `0.03` | A slight trail reads as a helmet's HUD; the layer is composited by SteamVR either way | `0` locks the HUD to the head (view space) |
| `vr.turn`, `vr.snap_turn_angle` | snap, 30 degrees | snap, 30 | Comfort first; VR Setup changes it | none |
| `vr.world_scale` | `1.0` | `1.0` | 1:1 | none |
| `vr.height` | `"seated"` | `"seated"` | The eyes are wherever you recentre (both grips held), standing or seated, with no floor or height to get right | `"standing"` needs `vr.player_height` and SteamVR's floor right |
| `vr.physical_crouch` | `true` | `true` | Standalone, crouching in the room is natural; measured from the recentred height | none |
| `vr.melee_gesture` | `true` | `true` | The weapon swing melees; B still does | Hard swings near walls |
| `vr.weapon_haptics` | `1.0` | `1.0` | The Frame's controllers take OpenXR haptics | Their feel on the Frame is untested |
| `vr.hrtf` | `true` | `true` | Directional sound in the headset's speakers | Its CPU cost on the Frame's 5 usable cores is unmeasured |
| `vr.dominant_hand`, `vr.swap_sticks` | right, as the hand | right, as the hand | VR Setup > PLAYER... > DOMINANT HAND; `docs/openframe-features.md` "Left-handed" | none |
| `update.auto` | `true` | `false` (installer) | OpenH1's updater would replace this fork | none |
| `TU_DEBUG` | unset | `sysmem` (wrapper; `OPENFRAME_SYSMEM=0` leaves it unset) | As the sibling Frame ports render | Not touched by this review |

If the HUD's corners are cut off (second headset trial: with a glasses
spacer the Frame's comfortable view is narrower, and the ammunition and
grenades at the top left and the motion tracker at the bottom left sat
partly past its edge; the shields at the top right were fine): VR Setup >
HUD... > HUD LAYOUT..., or
in `config.toml` under `[vr]` (moves are fractions of the HUD's width or
height; y is up, so less than 0 is down):

```toml
hud_inset = 0.2          # 0.15 to 0.25: every corner brought a fifth of the way in
hud_offset_y = 0.0       # -0.02 (a little down) if the top corners are cut off more than the bottom
hud_weapon_x = 0.0       # 0.02 to 0.06: the ammo (top left) further right, inward
hud_weapon_y = 0.0       # -0.02 to -0.06: the ammo further down, inward
hud_tracker_x = 0.0      # 0.02 to 0.06: the tracker (bottom left) further right, inward
hud_tracker_y = 0.0      # 0.02 to 0.06: the tracker further up, inward
```

Either way works: the inset alone, or each corner's own move alone (each
corner's move comes first, then the inset, then `hud_offset_x`, `_y` for
all). At `vr.hud_distance` 2 and `vr.hud_size` 60 the ammo sits about 22
degrees left of the middle and 17 up (with `vr.hud_wrap` 1): an inset of
0.2 brings it about 4.4 degrees right and 3.5 down; `hud_weapon_x = 0.05`
is 3 degrees (5% of the HUD's 60). These are suggestions, not defaults: the
shipped values stay at 0. **HUD SIZE** (`vr.hud_size`, VR Setup > HUD...) narrows
the whole HUD, the reticle with it; **ALL SIZE** (`vr.hud_scale`) shrinks
the corners alone.

The Frame's controller profile: SteamVR 2.18.2 on the Frame accepts both
`/interaction_profiles/valve/frame_controller_valve` and
`/interaction_profiles/valve/frame_controller` (both are in its
`vrclient.so`, and its client logs translate bindings from apps using
either; its server names the controller type `frame_controller`). OpenH1
suggests its bindings for both (`port/linux/host/host_vr.c`,
`actions_create`), the same bindings, so their order does not matter.

The refresh rate: the wrapper (`steam-frame/openframe.sh`) exports
`HALO_VR_REFRESH_RATE` only when `OPENFRAME_REFRESH_HZ` (72, 90 or 120) is
given, in the environment or `session.env`, because an environment value
wins over `config.toml`. Otherwise the game reads `vr.refresh_rate` (VR
Setup's REFRESH RATE) and the wrapper asks SteamVR's per-app
`preferredRefreshRate` for that same rate (90 when `config.toml` has none, or
holds a rate the Frame lacks). VR Setup > DISPLAY...'s REFRESH RATE offers 72, 90 and 120
Hz only: the Frame has no 80 or 144, and the headset's own (0) is not offered.

To measure before changing a default: `HALO_GPU_STATS=1` and
`HALO_VR_GPU_TIME=1` in `session.env` (the log's `vr: elapsed render GPU
timeline per frame`), or `tools/gpu_busy.py --hz <rate>` over ssh while
playing (OpenH1's `port/linux/frame/README.md`, "Measuring performance").
