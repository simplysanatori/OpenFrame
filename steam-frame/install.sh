#!/bin/bash
# Installs OpenFrame on the Steam Frame: the game into ~/Games/OpenFrame/app, the launch wrapper and
# tools beside it, the settings and the maps in OPENFRAME_HOME, and a non-Steam shortcut in Steam's
# library (through Steam's own client API, so Steam must be running). No sudo.
#
#   install.sh [options] [openframe-*-linux-arm64.tar.gz | UNPACKED_RELEASE_DIR]
#   install.sh --dev [BUILD_DIR]     install a build tree (default <repo>/build/linux_arm64) as
#                                    "OpenFrame (dev)" in app-dev; run again after a rebuild
#
#   --maps DIR         the maps of your own Xbox Halo: Combat Evolved disc: a folder with the 24 .map
#                      files, or one that contains maps/. They are copied (never moved) into
#                      OPENFRAME_HOME/maps, and config.toml's paths.data points at OPENFRAME_HOME.
#                      Without it, existing maps are kept.
#   --name NAME        shortcut name (default "OpenFrame", with --dev "OpenFrame (dev)")
#   -h, --help
#
# Environment:
#   OPENFRAME_APP=DIR         where the program goes (default ~/Games/OpenFrame/app; dev: app-dev)
#   OPENFRAME_HOME=DIR        data folder (default ~/.local/share/OpenFrame): config.toml, maps/, saves/, sessions/
#   OPENFRAME_SKIP_STEAM=1    trial mode: no Steam shortcut at all (Steam need not be running)
#   OPENFRAME_STEAM_DIR=DIR   edit the shortcuts.vdf files of this (copied) Steam folder offline instead
#                             of the live one: DIR/userdata/*/config/shortcuts.vdf, grid art beside it.
#                             Used for dry runs; nothing of the real Steam is touched.
# The settings file is written once with the settings this install needs (the data folder, the saves
# folder, the refresh rate, and update.auto = false: OpenH1's self-updater would replace this fork with
# OpenH1's own release). Running install.sh again sets those keys and leaves the rest of the file alone.
# The shortcut is made with OpenVR=1, AllowOverlay=1 and AllowDesktopConfig=1, and is updated, not
# duplicated, when it exists. The library art (art/) is written when it holds images.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
export PYTHONDONTWRITEBYTECODE=1
DEV=0; NAME=; MAPS_ARG=; SRC=
while [ $# -gt 0 ]; do
    case $1 in
        --dev) DEV=1 ;;
        --name) NAME=$2; shift ;;
        --maps) MAPS_ARG=$2; shift ;;
        -h | --help) sed -n '2,/^set -e/p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        -*) echo "unknown option $1" >&2; exit 2 ;;
        *) SRC=$1 ;;
    esac
    shift
done
OFHOME=${OPENFRAME_HOME:-$HOME/.local/share/OpenFrame}
STEAM=${OPENFRAME_STEAM_DIR:-$HOME/.local/share/Steam}
SCRATCH=
cleanup() { [ -n "$SCRATCH" ] && rm -rf "$SCRATCH"; return 0; }
trap cleanup EXIT

# ---- where the files come from: ROOT holds openframe.sh, tools/, art/, brokers.txt; BIN_DIR holds halo and libSDL3.so.0
if [ "$DEV" = 1 ]; then
    NAME=${NAME:-OpenFrame (dev)}
    APP=${OPENFRAME_APP:-$HOME/Games/OpenFrame/app-dev}
    BIN_DIR=$(cd "${SRC:-$HERE/../build/linux_arm64}" 2> /dev/null && pwd) || { echo "no build tree at ${SRC:-$HERE/../build/linux_arm64}" >&2; exit 1; }
    [ -x "$BIN_DIR/halo" ] || { echo "no halo in $BIN_DIR (build with steam-frame/build.sh first)" >&2; exit 1; }
    ROOT=$HERE
    BROKERS=$HERE/../port/assets/network/brokers.txt
else
    NAME=${NAME:-OpenFrame}
    APP=${OPENFRAME_APP:-$HOME/Games/OpenFrame/app}
    if [ -f "$SRC" ]; then
        mkdir -p "${TMPDIR:-$HOME/.cache}"
        SCRATCH=$(mktemp -d "${TMPDIR:-$HOME/.cache}/of-install.XXXXXX")
        tar -xzf "$SRC" -C "$SCRATCH"
        ROOT=$SCRATCH
        [ -f "$ROOT/openframe.sh" ] || ROOT=$(dirname "$(find "$SCRATCH" -maxdepth 2 -name openframe.sh | head -1)")
    elif [ -d "$SRC" ]; then
        ROOT=$(cd "$SRC" && pwd)
    elif [ -z "$SRC" ] && [ -x "$HERE/halo" ]; then
        ROOT=$HERE      # run from inside an unpacked release
    else
        echo "usage: $0 [--maps DIR] [--dev [BUILD_DIR]] | openframe-*-linux-arm64.tar.gz | UNPACKED_RELEASE_DIR" >&2; exit 1
    fi
    BIN_DIR=$ROOT
    [ -x "$BIN_DIR/halo" ] && [ -f "$ROOT/openframe.sh" ] || { echo "$ROOT is not an OpenFrame release (halo and openframe.sh expected)" >&2; exit 1; }
    BROKERS=$ROOT/brokers.txt
fi
[ -f "$BIN_DIR/libSDL3.so.0" ] || { echo "no libSDL3.so.0 beside halo in $BIN_DIR" >&2; exit 1; }
[ -f "$ROOT/openframe.sh" ] && [ -f "$ROOT/tools/vrtiming.py" ] || { echo "openframe.sh or tools/vrtiming.py missing in $ROOT" >&2; exit 1; }
case $NAME in *\'* | *\\*) echo "the shortcut name cannot hold a quote or backslash" >&2; exit 2 ;; esac
if [ -z "$OPENFRAME_SKIP_STEAM" ] && [ -z "$OPENFRAME_STEAM_DIR" ]; then
    curl -s --max-time 3 127.0.0.1:8080/json > /dev/null || {
        echo "Steam's debugging port is not reachable. Make sure Steam is running (on SteamOS it is on by default)," >&2
        echo "or set OPENFRAME_SKIP_STEAM=1 to install without the library entry." >&2
        exit 1
    }
fi
if pgrep -f "^$APP/halo( |\$)" > /dev/null 2>&1; then echo "the game is running from $APP; quit it first" >&2; exit 1; fi

# ---- the maps: checked before anything is written
MAPS_SRC=
if [ -n "$MAPS_ARG" ]; then
    if [ -d "$MAPS_ARG/maps" ]; then MAPS_SRC=$MAPS_ARG/maps; else MAPS_SRC=$MAPS_ARG; fi
    [ -d "$MAPS_SRC" ] || { echo "no such maps folder: $MAPS_ARG" >&2; exit 1; }
    [ -n "$(find "$MAPS_SRC" -maxdepth 1 -iname ui.map -print -quit)" ] ||
        { echo "$MAPS_SRC has no ui.map: it is not the maps folder of an Xbox Halo: Combat Evolved disc" >&2; exit 1; }
    NEED_KB=$(du -sk "$MAPS_SRC" | cut -f1)
    mkdir -p "$OFHOME"
    AVAIL_KB=$(df -Pk "$OFHOME" | awk 'NR==2 {print $4}')
    [ "$AVAIL_KB" -gt $((NEED_KB + 500000)) ] || { echo "not enough free space in $OFHOME for the maps ($((NEED_KB / 1024)) MB needed)" >&2; exit 1; }
fi

# ---- the program: halo and libSDL3.so.0 (the game finds it beside itself), the wrapper, the tools
echo "installing into $APP"
mkdir -p "$APP/tools" "$OFHOME"
cp "$BIN_DIR/halo" "$APP/halo.new" && mv "$APP/halo.new" "$APP/halo"
cp "$BIN_DIR/libSDL3.so.0" "$APP/libSDL3.so.0.new" && mv "$APP/libSDL3.so.0.new" "$APP/libSDL3.so.0"
chmod +x "$APP/halo"
cp "$ROOT/openframe.sh" "$APP/openframe.sh"
cp "$ROOT/tools/vrtiming.py" "$APP/tools/vrtiming.py"
cp "$ROOT/steam-cef.py" "$APP/tools/steam-cef.py"
for f in install.sh uninstall.sh collect-logs.sh; do [ -f "$ROOT/$f" ] && cp "$ROOT/$f" "$APP/tools/$f"; done
chmod +x "$APP/openframe.sh" "$APP/tools/"*.sh "$APP/tools/steam-cef.py" 2> /dev/null || true
for f in README.md README.upstream.md CREDITS.md LICENSE THIRD-PARTY-NOTICES.md NOTICE.md; do [ -f "$ROOT/$f" ] && cp "$ROOT/$f" "$APP/$f"; done
if [ -d "$ROOT/licenses" ]; then rm -rf "$APP/licenses"; cp -r "$ROOT/licenses" "$APP/licenses"; fi

# ---- the data folder: brokers.txt (internet play's MQTT brokers; the game reads the one beside config.toml)
if [ -f "$BROKERS" ] && [ ! -e "$OFHOME/brokers.txt" ]; then cp "$BROKERS" "$OFHOME/brokers.txt"; fi

# ---- the maps: copied into a partial folder, which takes the final name when it is complete
if [ -n "$MAPS_SRC" ]; then
    if [ -d "$OFHOME/maps" ] && [ -n "$(ls -A "$OFHOME/maps" 2> /dev/null)" ]; then
        echo "maps: $OFHOME/maps already holds files; they are kept (remove the folder to copy again)"
    else
        echo "maps: copying from $MAPS_SRC (about 2 GB)"
        rm -rf "$OFHOME/maps.partial"; mkdir -p "$OFHOME/maps.partial"
        cp -r "$MAPS_SRC/." "$OFHOME/maps.partial/"
        rm -rf "$OFHOME/maps"; mv "$OFHOME/maps.partial" "$OFHOME/maps"
    fi
fi
NMAPS=$(find "$OFHOME/maps" -maxdepth 1 -iname '*.map' 2> /dev/null | wc -l)
if [ "$NMAPS" -gt 0 ]; then echo "maps: $NMAPS .map files in $OFHOME/maps"; [ "$NMAPS" -ge 24 ] || echo "warning: the Xbox disc has 24 maps; $NMAPS here"; fi

# ---- config.toml: only the keys this install needs are set; every other line is kept
python3 -I - "$OFHOME/config.toml" "$OFHOME" << 'PY'
import os, re, sys
path, home = sys.argv[1], sys.argv[2]
want = [("paths", "data", '"%s"' % home), ("paths", "saves", '"%s/saves"' % home),
        ("update", "auto", "false"), ("vr", "refresh_rate", "90.0")]
# vr.refresh_rate is set only when the file does not have it yet (the wrapper's OPENFRAME_REFRESH_HZ wins at run time anyway)
try:
    lines = open(path, encoding="utf-8").read().split("\n")
except OSError:
    lines = ["# OpenFrame: written by install.sh. The game adds the rest of its settings itself.", ""]
def find_section(name):
    for i, l in enumerate(lines):
        if l.strip() == "[%s]" % name:
            return i
    return -1
for section, key, value in want:
    s = find_section(section)
    if s < 0:
        while lines and lines[-1] == "":
            lines.pop()
        lines += ["", "[%s]" % section, "%s = %s" % (key, value), ""]
        continue
    end = next((j for j in range(s + 1, len(lines)) if re.match(r"\s*\[", lines[j])), len(lines))
    hit = next((j for j in range(s + 1, end) if re.match(r"\s*#?\s*%s\s*=" % re.escape(key), lines[j])), -1)
    live = hit >= 0 and not lines[hit].lstrip().startswith("#")
    if key == "refresh_rate" and live:
        continue
    if hit >= 0:
        lines[hit] = "%s = %s" % (key, value)
    else:
        lines.insert(s + 1, "%s = %s" % (key, value))
open(path + ".new", "w", encoding="utf-8").write("\n".join(lines).rstrip("\n") + "\n")
os.replace(path + ".new", path)
PY
echo "settings: $OFHOME/config.toml (paths.data, paths.saves, update.auto = false)"

# ---- Steam shortcut
EXE=$APP/halo
LAUNCH="\"$APP/openframe.sh\" %command%"
APPID=none; GRID=
if [ -n "$OPENFRAME_SKIP_STEAM" ]; then
    echo "OPENFRAME_SKIP_STEAM set: no Steam shortcut"
elif [ -n "$OPENFRAME_STEAM_DIR" ]; then
    for CFG in "$STEAM"/userdata/*/config; do
        [ -d "$CFG" ] || continue
        APPID=$(python3 "$HERE/steam-cef.py" --vdf "$CFG/shortcuts.vdf" --name "$NAME" --exe "\"$EXE\"" \
            --startdir "\"$APP\"" --opts "$LAUNCH")
        echo "shortcut '$NAME' written to $CFG/shortcuts.vdf (appid $APPID)"
        GRID=$CFG/grid
    done
    [ "$APPID" != none ] || echo "warning: no userdata/*/config in $STEAM; no shortcut written"
else
    CEF="python3 $HERE/steam-cef.py"
    EXISTING=$($CEF "(() => { for (const a of (appStore.allApps || [])) { if (a.display_name === '$NAME' && a.app_type === 1073741824) return a.appid; } return 0; })()" 2> /dev/null || echo 0)
    if [ "$EXISTING" != "0" ] && [ -n "$EXISTING" ]; then
        APPID=$EXISTING
        echo "Steam already has a '$NAME' shortcut (appid $APPID): updating it"
        $CEF "(() => { SteamClient.Apps.SetShortcutExe($APPID, '\"$EXE\"'); SteamClient.Apps.SetShortcutStartDir($APPID, '\"$APP\"'); return 1; })()" > /dev/null
    else
        APPID=$($CEF "(async () => { const id = await SteamClient.Apps.AddShortcut('$NAME', '$EXE', '$APP', ''); SteamClient.Apps.SetShortcutName(id, '$NAME'); return id; })()")
        echo "added the '$NAME' shortcut (appid $APPID)"
    fi
    # OpenVR=1 (SetShortcutIsVR); a new shortcut already has AllowOverlay=1 and AllowDesktopConfig=1
    $CEF "(() => { SteamClient.Apps.SetShortcutLaunchOptions($APPID, '$LAUNCH'); SteamClient.Apps.SetShortcutIsVR($APPID, true); return 1; })()" > /dev/null
    GRID=$(ls -d "$STEAM"/userdata/*/config 2> /dev/null | head -1)/grid
fi

# ---- library art: written for the shortcut's appid when the art folder holds images
if [ "$APPID" != none ] && [ -n "$GRID" ]; then
    ART=$ROOT/art
    if ls "$ART"/*.png "$ART"/*.ico > /dev/null 2>&1; then
        NEWID=$(( APPID & 0xffffffff ))
        mkdir -p "$GRID"
        for pair in grid.png:.png grid_p.png:p.png hero.png:_hero.png logo.png:_logo.png icon.ico:_icon.ico; do
            [ -f "$ART/${pair%%:*}" ] || continue
            cp "$ART/${pair%%:*}" "$GRID/$NEWID${pair##*:}"
        done
        echo "art: library images written to $GRID ($NEWID*)"
        if [ -n "$OPENFRAME_STEAM_DIR" ] && [ -f "$GRID/${NEWID}_icon.ico" ]; then
            for CFG in "$STEAM"/userdata/*/config; do
                [ -d "$CFG" ] && python3 "$HERE/steam-cef.py" --vdf "$CFG/shortcuts.vdf" --name "$NAME" --exe "\"$EXE\"" \
                    --startdir "\"$APP\"" --opts "$LAUNCH" --icon "$CFG/grid/${NEWID}_icon.ico" > /dev/null
            done
        fi
    else
        echo "art: no images in $ART yet; the library entry has Steam's default look"
    fi
fi

cat << MSG

Installed in $APP.
$([ "$APPID" != none ] && echo "Steam library entry \"$NAME\" (appid $APPID) runs $EXE through $APP/openframe.sh.")
$([ "$NMAPS" -gt 0 ] || echo "Before you play: install.sh --maps <the maps folder of your Xbox Halo: Combat Evolved disc>")
Launch it from the library with SteamVR running.
Logs: $OFHOME/sessions/latest/ (launch.log, stdout.log, debug.txt, vrtiming.log)
MSG
