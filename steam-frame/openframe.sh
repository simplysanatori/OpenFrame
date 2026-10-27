#!/bin/bash
# OpenFrame launch wrapper. The Steam shortcut runs it as `"<app>/openframe.sh" %command%` (the
# shortcut's program is <app>/halo), or you can run it directly. There is no screen for messages in
# VR: everything goes to the session log.
#
# Every launch: re-applies SteamVR's per-app settings (it drops them now and then), starts the
# compositor timing logger, runs the game, then leaves a session folder
# $OPENFRAME_HOME/sessions/<date-time>/ with launch.log, stdout.log (the game's output, a crash's
# backtrace included), debug.txt (the game's own log, when this run wrote one), vrtiming.log and
# perfcriteria.txt (Steam's performance log). $OPENFRAME_HOME/sessions/latest points at the newest.
#
# What it hands the game (OpenH1's settings, port/linux/README.md "Settings"):
#   HALO_CONFIG_FILE   $OPENFRAME_HOME/config.toml (the installer writes it; paths.data names the
#                      folder with maps/, which is $OPENFRAME_HOME)
#   HALO_UPDATE_AUTO   false, always: OpenH1's self-updater would replace this fork with OpenH1's own
#                      release. Update OpenFrame by installing a newer OpenFrame release.
#   HALO_VR_REFRESH_RATE  from OPENFRAME_REFRESH_HZ
#   XR_RUNTIME_JSON    passed through when you set it (an OpenXR runtime's manifest to use instead of
#                      the active one); unset, the game uses the active runtime (SteamVR's)
# Any other HALO_* variable (HALO_VR=0, HALO_VR_TURN=smooth, ...) set in the environment, in Steam's
# launch options or in session.env reaches the game and wins over config.toml.
#
# Environment (all optional; session.env below is the way to set them from the Steam library):
#   OPENFRAME_HOME         data folder, default ~/.local/share/OpenFrame (config.toml, maps/, saves/, sessions/)
#   OPENFRAME_REFRESH_HZ   refresh rate asked of SteamVR and of the game, 72, 90 or 120 (default 90)
#   OPENFRAME_PACING       reserved for the game's pacing modes; passed through, the game ignores it today
#   OPENFRAME_SYSMEM=0     leaves TU_DEBUG unset (Turnip's tiled rendering). By default the wrapper
#                          sets TU_DEBUG=sysmem (renders straight to memory, as the sibling Frame ports do)
#   OPENFRAME_MOTION_SMOOTHING  SteamVR motionSmoothingOverride for the app: 0 global, 1 on, 2 off (default), 3 always on
#   OPENFRAME_VRTIMING=0   no compositor timing log
#   OPENFRAME_KEEP_SESSIONS  number of session folders kept (default 30)
# session.env: if $OPENFRAME_HOME/session.env exists it is sourced for ONE launch (VAR=value lines,
# any of the above or HALO_* settings), then renamed session.env.used. session.env.next, if present,
# then becomes session.env for the launch after (two-launch tests).
# Test hooks: OPENFRAME_VRCMD, OPENFRAME_STEAMVR_DIR, OPENFRAME_VRSETTINGS, OPENFRAME_VRTIMING_PY,
# OPENFRAME_EXE, OPENFRAME_PERFCRITERIA.
HERE=$(dirname "$(readlink -f "$0")")
OFHOME=${OPENFRAME_HOME:-$HOME/.local/share/OpenFrame}
STAMP=$(date +%Y%m%d-%H%M%S)
SESSION=$OFHOME/sessions/$STAMP
mkdir -p "$SESSION" || exit 1
ln -sfn "$STAMP" "$OFHOME/sessions/latest"
exec > "$SESSION/launch.log" 2>&1
log() { echo "$(date +%T) $*"; }
log "OpenFrame wrapper, session $STAMP, steam appid '${SteamAppId:-unset}'"

# One launch's test environment.
if [ -f "$OFHOME/session.env" ]; then
    log "session.env:"; cat "$OFHOME/session.env"; cp "$OFHOME/session.env" "$SESSION/session.env"
    set -a; . "$OFHOME/session.env"; set +a
    mv -f "$OFHOME/session.env" "$OFHOME/session.env.used"
    log "session.env renamed to session.env.used"
    if [ -f "$OFHOME/session.env.next" ]; then
        mv -f "$OFHOME/session.env.next" "$OFHOME/session.env"; log "session.env.next is the next launch's session.env"
    fi
fi

# The game: Steam's %command% is a chain of Steam's own helpers (steam-launch-wrapper, reaper
# SteamLaunch AppId=...) ending with the binary the shortcut names. It runs as given. Without
# arguments, the installed binary runs.
if [ -n "$OPENFRAME_EXE" ]; then CMD=("$OPENFRAME_EXE")
elif [ $# -gt 0 ]; then CMD=("$@")
else CMD=("$HERE/halo"); fi
EXE=${CMD[${#CMD[@]}-1]}
[ -x "$EXE" ] || { log "REFUSING TO START: no game binary at $EXE (run install.sh)"; exit 1; }

# The game's settings.
export HALO_CONFIG_FILE=$OFHOME/config.toml
[ -f "$HALO_CONFIG_FILE" ] || log "WARNING: no $HALO_CONFIG_FILE (run install.sh); the game starts with its defaults and looks for maps/ by itself"
[ -d "$OFHOME/maps" ] || log "WARNING: no $OFHOME/maps: the game's first-start screen asks for the Xbox disc image (install.sh --maps copies a maps folder)"
export HALO_UPDATE_AUTO=false          # never OpenH1's updater (it would replace this fork)
unset HALO_FRAME_LAUNCHER HALO_FRAME_UPDATED
cd "$OFHOME" || exit 1                 # the game's current folder: debug.txt and the data lookup
HZ=${OPENFRAME_REFRESH_HZ:-90}
case "$HZ" in 72 | 90 | 120) ;; *) log "OPENFRAME_REFRESH_HZ=$HZ is not 72, 90 or 120, using 90"; HZ=90 ;; esac
export HALO_VR_REFRESH_RATE=$HZ.0
log "refresh rate $HZ Hz (HALO_VR_REFRESH_RATE=$HALO_VR_REFRESH_RATE)"
[ -n "$XR_RUNTIME_JSON" ] && log "XR_RUNTIME_JSON=$XR_RUNTIME_JSON"
[ -n "$OPENFRAME_PACING" ] && { export OPENFRAME_PACING; log "OPENFRAME_PACING=$OPENFRAME_PACING (reserved: the game ignores it today)"; }
# Direct rendering by default (the GPU renders straight to memory instead of in tiles).
[ "${OPENFRAME_SYSMEM:-1}" = 1 ] && { export TU_DEBUG=${TU_DEBUG:+$TU_DEBUG,}sysmem; log "TU_DEBUG=$TU_DEBUG"; }

# SteamVR's per-app settings, every launch. SteamAppId is set by Steam for a library launch.
MS=${OPENFRAME_MOTION_SMOOTHING:-2}
case "$MS" in 0 | 1 | 2 | 3) ;; *) log "OPENFRAME_MOTION_SMOOTHING=$MS is not 0 to 3, using 2"; MS=2 ;; esac
VRCMD=${OPENFRAME_VRCMD:-$(command -v vrcmd || echo /opt/steamvr/bin/linuxarm64/vrcmd)}
VRSETTINGS=${OPENFRAME_VRSETTINGS:-$HOME/.config/openvr/config/steamvr.vrsettings}
STEAMVR_DIR=${OPENFRAME_STEAMVR_DIR:-/opt/steamvr/bin/linuxarm64}
SETTINGS="preferredRefreshRate=$HZ framesToThrottle=0 additionalFramesToPredict=0 motionSmoothingOverride=$MS"
if [ -z "$SteamAppId" ]; then
    log "no SteamAppId: SteamVR per-app settings NOT applied (not a Steam library launch)"
elif [ ! -x "$VRCMD" ]; then
    log "no vrcmd at $VRCMD: SteamVR per-app settings NOT applied"
else
    for kv in $SETTINGS; do
        "$VRCMD" --set-settings-int "steam.app.$SteamAppId.${kv%=*}" "${kv#*=}" > /dev/null 2>&1 ||
            log "WARNING: vrcmd could not set ${kv%=*} (is SteamVR running?)"
    done
    sleep 0.3   # SteamVR writes its settings file shortly after a change
    python3 -I - "$VRSETTINGS" "steam.app.$SteamAppId" $SETTINGS << 'PY' || log "settings read-back failed"
import json, sys, time
path, section, pairs = sys.argv[1], sys.argv[2], sys.argv[3:]
try:
    got = json.load(open(path)).get(section, {})
except (OSError, ValueError) as e:
    print(f"{time.strftime('%T')} read-back: cannot read {path} ({e.__class__.__name__}); settings unverified")
    sys.exit(0)
bad = 0
for p in pairs:
    k, v = p.split("=")
    if str(got.get(k)) != v:
        bad += 1
        print(f"{time.strftime('%T')} MISMATCH {section}.{k}: wanted {v}, SteamVR has {got.get(k)}")
print(f"{time.strftime('%T')} read-back: {len(pairs) - bad} of {len(pairs)} per-app settings as asked")
PY
fi

# Compositor timing for the whole session; it cannot stop the game from starting.
VRT_PID=
VRT=${OPENFRAME_VRTIMING_PY:-$HERE/tools/vrtiming.py}
if [ "${OPENFRAME_VRTIMING:-1}" != 0 ] && [ -f "$VRT" ] && [ -f "$STEAMVR_DIR/libopenvr_api.so" ]; then
    env -u LD_LIBRARY_PATH python3 -I "$VRT" 86400 "$SESSION/vrtiming.log" > /dev/null 2>&1 &
    VRT_PID=$!
else
    log "no vrtiming.log (disabled, or vrtiming.py / libopenvr_api.so missing)"
fi

# Not exec: the wrapper stays to stop the logger and collect the logs. A signal for the wrapper
# (Steam stopping the app) goes on to the game.
log "starting ${CMD[*]}"
touch "$SESSION/.start"
"${CMD[@]}" > "$SESSION/stdout.log" 2>&1 &
GAME_PID=$!
# The headset is in use: development builds and GPU runs on this machine wait while this file
# exists and names a live process.
HEADSET_MARK=${XDG_CACHE_HOME:-$HOME/.cache}/headset-in-use
echo "openframe $GAME_PID" > "$HEADSET_MARK" 2> /dev/null
trap 'kill -TERM "$GAME_PID" 2> /dev/null' TERM INT HUP
rc=0
while kill -0 "$GAME_PID" 2> /dev/null; do   # a signal interrupts wait before the game is gone
    wait "$GAME_PID"; rc=$?
done
trap - TERM INT HUP
[ "$(awk '{print $2}' "$HEADSET_MARK" 2> /dev/null)" = "$GAME_PID" ] && rm -f "$HEADSET_MARK"
log "game exited with status $rc"
[ -n "$VRT_PID" ] && { kill "$VRT_PID" 2> /dev/null; wait "$VRT_PID" 2> /dev/null; }

# The game's own log (debug.txt in the data folder), when this run wrote it, and Steam's performance log.
[ -f "$OFHOME/debug.txt" ] && [ "$OFHOME/debug.txt" -nt "$SESSION/.start" ] && cp "$OFHOME/debug.txt" "$SESSION/debug.txt"
PERF=${OPENFRAME_PERFCRITERIA:-$HOME/.local/share/Steam/logs/perfcriteria.txt}
[ -f "$PERF" ] && [ "$PERF" -nt "$SESSION/.start" ] && cp -f "$PERF" "$SESSION/perfcriteria.txt" 2> /dev/null
rm -f "$SESSION/.start"
# Keep the newest sessions only.
KEEP=${OPENFRAME_KEEP_SESSIONS:-30}
ls -1d "$OFHOME"/sessions/2* 2> /dev/null | head -n -"$KEEP" | xargs -r rm -rf
exit $rc
