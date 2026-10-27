#!/bin/bash
# Builds OpenFrame's native Arm64 VR game on the Steam Frame itself, the way OpenH1 builds it:
#   python3 configure.py --vr --release   then   ninja linux_arm64
#
#   steam-frame/build.sh [ninja targets...]       (default target: linux_arm64)
#
# Result: build/linux_arm64/halo with libSDL3.so.0 beside it. The first configure downloads the pinned
# guest LLVM (about 1.5 GB, about 700 MB kept in ~/.cache/openh1), musl, SDL3 and the Vulkan and
# OpenXR headers, so it needs the network. Builds are slow the first time (30 minutes or more on the
# Frame's five CPUs) and fast afterwards, because ccache keeps the host objects.
#
# Environment:
#   OPENFRAME_BUILD_CONFIG   release (default; LTO and profile guided) or debug (assertions stop the game)
#   OPENFRAME_CONFIGURE_FLAGS  extra configure.py flags (for example --profile)
#   OPENFRAME_RECONFIGURE=1  run configure.py again even if build.ninja exists (it also runs again when the flags change)
#   OPENFRAME_CONFIGURE_ONLY=1  configure, then stop
#   JOBS                     ninja jobs (default 4; the Frame has 5 CPUs and 15 GB of RAM)
#   CCACHE_DIR               default ~/.cache/ccache-openframe
#   OPENFRAME_BUILD_LOCK     lock file, default ~/.cache/openframe-build.lock: one heavy job at a time
# The build waits while ~/.cache/headset-in-use names a live process (a game wrapper of this family
# writes it while you play) and pauses a running build, so a compile never competes with the headset.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/.." && pwd)
cd "$REPO"
CONFIG=${OPENFRAME_BUILD_CONFIG:-release}
FLAGS="--vr"
[ "$CONFIG" = release ] && FLAGS="$FLAGS --release"
FLAGS="$FLAGS $OPENFRAME_CONFIGURE_FLAGS"
export CCACHE_DIR=${CCACHE_DIR:-$HOME/.cache/ccache-openframe}
LAUNCH=""
command -v ccache > /dev/null && LAUNCH="--compiler-launcher ccache"

# env -u LD_LIBRARY_PATH: Steam's runtime libraries must not reach the toolchain.
configure() {
    echo "build: configure.py $FLAGS $LAUNCH"
    env -u LD_LIBRARY_PATH python3 configure.py $FLAGS $LAUNCH
    mkdir -p build && echo "$FLAGS" > build/.openframe-flags
}
need_configure() {
    [ -n "$OPENFRAME_RECONFIGURE" ] || [ ! -f build.ninja ] || [ "$(cat build/.openframe-flags 2> /dev/null)" != "$FLAGS" ]
}
if [ -n "$OPENFRAME_CONFIGURE_ONLY" ]; then configure; exit; fi
[ $# -gt 0 ] || set -- linux_arm64

MARK=${XDG_CACHE_HOME:-$HOME/.cache}/headset-in-use
in_use() { [ -f "$MARK" ] && kill -0 "$(awk '{print $2}' "$MARK" 2> /dev/null)" 2> /dev/null; }
while in_use; do echo "build: headset in use, waiting"; sleep 60; done
mkdir -p "$(dirname "${OPENFRAME_BUILD_LOCK:-$HOME/.cache/openframe-build.lock}")"
exec 9> "${OPENFRAME_BUILD_LOCK:-$HOME/.cache/openframe-build.lock}"
echo "build: waiting for the lock"; flock 9; echo "build: building $REPO"
if need_configure; then configure; fi
env -u LD_LIBRARY_PATH setsid nice -n 5 ninja -j"${JOBS:-4}" "$@" &
NINJA=$!
trap 'kill -CONT -$NINJA 2>/dev/null; kill -TERM -$NINJA 2>/dev/null' TERM INT HUP
PAUSED=0
while kill -0 $NINJA 2> /dev/null; do
    if in_use; then
        [ $PAUSED = 0 ] && { echo "build: headset in use, build paused"; kill -STOP -$NINJA; PAUSED=1; }
    elif [ $PAUSED = 1 ]; then
        echo "build: build resumed"; kill -CONT -$NINJA; PAUSED=0
    fi
    sleep 5
done
wait $NINJA
