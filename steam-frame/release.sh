#!/usr/bin/env bash
# Builds the OpenFrame release tarball from a finished build. It does not build anything.
#
#   steam-frame/release.sh [--version VER] [--build DIR] [--out DIR]
#
#   --version VER the release version, e.g. 0.1.0-alpha.1 (default: the VERSION line below)
#   --build DIR   the build folder (default <repo>/build/linux_arm64): halo and libSDL3.so.0
#   --out DIR     where the tarball and SHA256SUMS go (default <repo>/steam-frame/release)
#   --stage DIR   only stage the file set into DIR (created if missing) and stop: no tarball, no checks.
#                 install.sh --from-build uses this to install straight from a build tree.
#
# The tarball is openframe-<version>-linux-arm64.tar.gz, with no top level folder: halo,
# libSDL3.so.0, openframe.sh, install.sh, uninstall.sh, collect-logs.sh, steam-cef.py,
# tools/vrtiming.py, brokers.txt, art/ (the Steam library images, when there are any), README.md,
# README.upstream.md, CREDITS.md, LICENSE, THIRD-PARTY-NOTICES.md and the libraries' licence files.
# Never the maps, a config.toml, a save or a log. Then steam-frame/tools/privacy-check.sh runs on the
# tarball, and SHA256SUMS is written beside it. Any failure deletes the tarball again.
set -euo pipefail

VERSION=0.1.0-alpha.1
here=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
repo=$(cd "$here/.." && pwd)
build=$repo/build/linux_arm64 out=$here/release stage_only=

fail() { echo "release.sh: error: $*" >&2; exit 1; }
while [[ $# -gt 0 ]]; do
    case $1 in
        --version) VERSION=${2:?}; shift ;;
        --build) build=${2:?}; shift ;;
        --out) out=${2:?}; shift ;;
        --stage) stage_only=${2:?}; shift ;;
        -h | --help) sed -n '2,/^set -e/p' "$0" | sed '$d; s/^# \{0,1\}//'; exit 0 ;;
        *) fail "unknown argument $1" ;;
    esac
    shift
done
NAME=openframe-$VERSION-linux-arm64
[[ -x $build/halo ]] || fail "no $build/halo (build with steam-frame/build.sh first)"
[[ -f $build/libSDL3.so.0 ]] || fail "no $build/libSDL3.so.0"
# a VR build: the strings host_vr.c carries (as OpenH1's tools/frame_package.py checks)
grep -q -a xrNegotiateLoaderRuntimeInterface "$build/halo" && grep -q -a XR_KHR_vulkan_enable2 "$build/halo" ||
    fail "$build/halo is not a VR build (python3 configure.py --vr, then ninja linux_arm64)"
for f in README.md README.upstream.md CREDITS.md LICENSE THIRD-PARTY-NOTICES.md port/assets/network/brokers.txt \
         steam-frame/openframe.sh steam-frame/install.sh steam-frame/uninstall.sh steam-frame/collect-logs.sh \
         steam-frame/steam-cef.py steam-frame/tools/vrtiming.py steam-frame/tools/privacy-check.sh \
         port/third_party/mbedtls/LICENSE port/third_party/miniupnpc/LICENSE port/assets/fonts/Overpass-OFL.txt \
         port/third_party/expat/COPYING port/third_party/opus/COPYING port/assets/icons/lucide/LICENSE; do
    [[ -f $repo/$f ]] || fail "missing $f"
done

tarball=$out/$NAME.tar.gz
if [[ -n $stage_only ]]; then
    mkdir -p "$stage_only"
    stage=$(cd "$stage_only" && pwd)
else
    mkdir -p "$out" "$HOME/.cache"
    out=$(cd "$out" && pwd)
    stage=$(mktemp -d "${TMPDIR:-$HOME/.cache}/of-release.XXXXXX")
    tarball=$out/$NAME.tar.gz
    rm -f "$tarball"
    trap 'rm -f "$tarball"; rm -rf "$stage"' ERR
    trap 'rm -rf "$stage"' EXIT
fi

echo "release: staging"
install -m 0755 "$build/halo" "$stage/halo"
install -m 0755 "$build/libSDL3.so.0" "$stage/libSDL3.so.0"
[[ -f $build/third_party/SDL3/LICENSE.txt ]] && install -m 0644 "$build/third_party/SDL3/LICENSE.txt" "$stage/SDL3-LICENSE.txt"
for f in openframe.sh install.sh uninstall.sh collect-logs.sh steam-cef.py; do install -m 0755 "$here/$f" "$stage/$f"; done
mkdir -p "$stage/tools" "$stage/art"
install -m 0644 "$here/tools/vrtiming.py" "$stage/tools/vrtiming.py"
install -m 0644 "$repo/port/assets/network/brokers.txt" "$stage/brokers.txt"
# the library images, when the owner has added them (art/README.md stays in the repository)
for f in "$here"/art/*.png "$here"/art/*.ico; do [[ -f $f ]] && install -m 0644 "$f" "$stage/art/"; done || true
install -m 0644 "$repo/README.md" "$repo/README.upstream.md" "$repo/CREDITS.md" "$repo/LICENSE" "$repo/THIRD-PARTY-NOTICES.md" "$stage/"
mkdir -p "$stage/licenses"
install -m 0644 "$repo/port/third_party/mbedtls/LICENSE" "$stage/licenses/mbedtls-LICENSE.txt"
install -m 0644 "$repo/port/third_party/miniupnpc/LICENSE" "$stage/licenses/miniupnpc-LICENSE.txt"
install -m 0644 "$repo/port/assets/fonts/Overpass-OFL.txt" "$stage/licenses/Overpass-OFL.txt"
install -m 0644 "$repo/port/third_party/expat/COPYING" "$stage/licenses/expat-COPYING.txt"
install -m 0644 "$repo/port/third_party/opus/COPYING" "$stage/licenses/opus-COPYING.txt"
install -m 0644 "$repo/port/assets/icons/lucide/LICENSE" "$stage/licenses/lucide-LICENSE.txt"
[[ -f $stage/SDL3-LICENSE.txt ]] && mv "$stage/SDL3-LICENSE.txt" "$stage/licenses/SDL3-LICENSE.txt"
# VERSION: what the tarball was built from
{
    echo "openframe $VERSION"
    echo "commit $(git -C "$repo" rev-parse HEAD 2> /dev/null || echo unknown)"
    echo "packaged $(date -u '+%Y-%m-%d %H:%M UTC')"
} > "$stage/VERSION"
# The maps, a config, saves and logs must never get in.
if find "$stage" \( -iname '*.map' -o -iname '*.iso' -o -iname '*.xiso' -o -name 'config.toml' -o -name 'debug.txt' -o -name '*.log' \) | grep -q .; then
    fail "the staging folder holds a map, disc image, config or log"
fi

[[ -n $stage_only ]] && { echo "release: staged in $stage"; exit 0; }

echo "release: packing"
# Fixed order, owner and times, so the same inputs make the same tarball.
(cd "$stage" && find . -mindepth 1 -printf '%P\n' | LC_ALL=C sort |
    tar --no-recursion --owner=0 --group=0 --numeric-owner --mtime='2026-01-01 00:00Z' -czf "$tarball" -T -)

echo "release: privacy check"
"$here/tools/privacy-check.sh" "$tarball"

(cd "$out" && sha256sum "$NAME.tar.gz" > SHA256SUMS)
trap 'rm -rf "$stage"' EXIT; trap - ERR
echo "release: $tarball ($(du -h "$tarball" | cut -f1))"
cat "$out/SHA256SUMS"
