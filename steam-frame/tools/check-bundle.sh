#!/bin/bash
# Fails unless an unpacked release (the app folder) loads nothing from the system but the libraries on
# host-libs.allow, on every SteamOS build listed in host-sonames/.  The same idea as Azahar for Frame's and
# Moonlight XR for Frame's check-bundle.sh.
#   steam-frame/tools/check-bundle.sh <app dir>
# For every ELF file in the app dir (halo, libSDL3.so.0):
#   - each library it needs (NEEDED) is beside it in the app dir, or is on the allowlist and named in every
#     host-sonames/*.txt (so it exists, under that soname, on every SteamOS build we know);
#   - it needs no glibc symbol newer than GLIBC_2.39 (the oldest SteamOS) and no libstdc++ or libgcc symbol
#     newer than the oldest SteamOS's (GLIBCXX_3.4.34, CXXABI_1.3.15, GCC_15.0.0);
#   - it has no RPATH; every RUNPATH entry is relative ($ORIGIN); halo's RUNPATH is exactly $ORIGIN;
#   - resolved here (ldd, LD_LIBRARY_PATH unset), each library it needs comes from the app dir or is allowlisted.
# Libraries opened at run time (dlopen) are not NEEDED, so the names inside halo and libSDL3.so.0 are read as well:
# each that looks like a library name must be bundled, or on the allowlist and on every SteamOS build; the few that
# are not (optional ones SDL 3 tries and does without) are listed in OPTIONAL_DLOPEN below and only reported.
# The OpenXR runtime is the one SteamVR names in the active runtime file, never ours.
# ALLOW_MISSING_HOST=1: an allowlisted library this machine lacks is a warning, not a failure.
# GLIBC_MAX, GLIBCXX_MAX, CXXABI_MAX, GCC_MAX override the limits (not for a release).
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
APP="$(cd "${1:?usage: check-bundle.sh <app dir>}" && pwd)"
ALLOW="${CHECK_BUNDLE_ALLOW:-$HERE/host-libs.allow}"
LISTS=("$HERE"/host-sonames/*.txt)
GLIBC_MAX="${GLIBC_MAX:-2.39}" GLIBCXX_MAX="${GLIBCXX_MAX:-3.4.34}" CXXABI_MAX="${CXXABI_MAX:-1.3.15}" GCC_MAX="${GCC_MAX:-15.0.0}"
# names SDL 3 or the game try with dlopen and do without: the tray's appindicator libraries, an old udev, the
# OpenGL ES 1 libraries, SDL's own name, Steam's API library (only when run from Steam), the Vulkan name without a version
OPTIONAL_DLOPEN=" libappindicator3.so.1 libayatana-appindicator3.so.1 libudev.so.0 libGLES_CM.so.1 libGLESv1_CM.so.1 libsteam_api.so libvulkan.so libSDL3.so.0 "
for t in objdump readelf ldd strings; do command -v "$t" >/dev/null || { echo "check-bundle: missing tool $t"; exit 1; }; done
[ -f "${LISTS[0]}" ] || { echo "check-bundle: no host-sonames/*.txt"; exit 1; }
[ "${#LISTS[@]}" -ge 4 ] || { echo "check-bundle: expected four host-sonames files, found ${#LISTS[@]}"; exit 1; }
mapfile -t PATS < <(sed -e 's/#.*//' -e 's/[[:space:]]//g' "$ALLOW" | grep .)
allowed() { local p; for p in "${PATS[@]}"; do [[ "$1" == $p ]] && return 0; done; return 1; }
newer() { [ "$1" != "$2" ] && [ "$(printf '%s\n%s\n' "$1" "$2" | sort -V | tail -n 1)" = "$1" ]; }   # newer A B: A > B
bad=0; warn=0; n=0
fail() { echo "  FAIL $*"; bad=$((bad+1)); }
declare -A EXT=() DL=()
# nothing on the allowlist may be shipped
for f in "$APP"/*.so*; do [ -e "$f" ] && allowed "${f##*/}" && fail "${f##*/} is on the allowlist: it must come from the system"; done
[ -d "$APP/lib" ] && fail "app/lib exists: this layout keeps its libraries beside halo"
while IFS= read -r -d '' f; do
    [ "$(head -c 4 "$f" | od -An -tx1 | tr -d ' \n')" = 7f454c46 ] || continue
    n=$((n+1)); rel="${f#"$APP"/}"
    dyn="$(readelf -d "$f" 2>/dev/null)"
    grep -q '(RPATH)' <<<"$dyn" && fail "$rel has an RPATH"
    rp="$(sed -n 's/.*(RUNPATH).*\[\(.*\)\]/\1/p' <<<"$dyn")"
    if [ "$rel" = halo ] && [ "$rp" != '$ORIGIN' ]; then fail "$rel: RUNPATH '$rp', not \$ORIGIN"; fi
    IFS=: read -ra rps <<<"$rp"
    for e in "${rps[@]}"; do [ -z "$e" ] || [[ "$e" == '$ORIGIN'* ]] || fail "$rel: RUNPATH entry '$e' is not relative to \$ORIGIN"; done
    vers="$(objdump -p "$f" 2>/dev/null | sed -n '/^Version References:/,/^$/p')"
    for k in GLIBC:$GLIBC_MAX GLIBCXX:$GLIBCXX_MAX CXXABI:$CXXABI_MAX GCC:$GCC_MAX; do
        top="$(grep -oE "[[:space:]]${k%%:*}_[0-9][0-9.]*" <<<"$vers" | sed "s/^[[:space:]]*${k%%:*}_//" | sort -uV | tail -n 1)"
        [ -n "$top" ] && newer "$top" "${k#*:}" && fail "$rel needs ${k%%:*}_$top (the oldest SteamOS has up to ${k%%:*}_${k#*:})"
    done
    mapfile -t need < <(objdump -p "$f" 2>/dev/null | awk '$1 == "NEEDED" { print $2 }')
    for s in "${need[@]}"; do
        [ -f "$APP/$s" ] && continue
        if ! allowed "$s"; then fail "$rel needs $s: not in the app folder and not on host-libs.allow"; continue; fi
        EXT[$s]=1
        for l in "${LISTS[@]}"; do grep -qxF "$s" "$l" || fail "$rel needs $s, which SteamOS ${l##*/} does not have"; done
    done
    # resolved here, only the libraries it needs itself (the system's libraries' own needs are the system's)
    out="$(env -u LD_LIBRARY_PATH ldd "$f" 2>&1)"
    for s in "${need[@]}"; do
        line="$(grep -E "^[[:space:]]*$(sed 's/[.+]/\\&/g' <<<"$s") => " <<<"$out" | head -n 1)"
        [ -n "$line" ] || continue
        p="$(awk '{ print $3 }' <<<"$line")"
        if [ "$p" = not ]; then
            if allowed "$s" && [ "${ALLOW_MISSING_HOST:-0}" = 1 ]; then warn=$((warn+1)); else fail "$rel: $s not found"; fi
            continue
        fi
        real="$(readlink -f "$p")"
        [[ "$real" == "$APP/"* ]] && continue
        if [ -f "$APP/$s" ]; then fail "$rel loads $s from $real, not its own copy in the app folder"
        elif ! allowed "$s"; then fail "$rel loads $s from $real, outside the app"; fi
    done
    # libraries opened at run time
    while read -r s; do
        [ -n "$s" ] || continue
        if [ -f "$APP/$s" ]; then continue
        elif [[ "$OPTIONAL_DLOPEN" == *" $s "* ]]; then DL[$s]=optional; continue
        elif ! allowed "$s"; then fail "$rel opens $s at run time: not bundled, not on host-libs.allow, not an optional one"; continue; fi
        DL[$s]=host
        for l in "${LISTS[@]}"; do grep -qxF "$s" "$l" || fail "$rel opens $s at run time, which SteamOS ${l##*/} does not have"; done
    done < <(strings -a "$f" | grep -E '^lib[A-Za-z0-9_+-]+\.so(\.[0-9]+)*$' | sort -u)
done < <(find "$APP" -type f -print0 | sort -z)
echo "check-bundle: $n ELF files; from the system: $(printf '%s\n' "${!EXT[@]}" | sort | tr '\n' ' ')"
echo "check-bundle: opened at run time from the system: $(for k in "${!DL[@]}"; do [ "${DL[$k]}" = host ] && echo "$k"; done | sort | tr '\n' ' ')"
echo "check-bundle: optional, tried and done without: $(for k in "${!DL[@]}"; do [ "${DL[$k]}" = optional ] && echo "$k"; done | sort | tr '\n' ' ')"
echo "check-bundle: limits: glibc $GLIBC_MAX, GLIBCXX $GLIBCXX_MAX"
[ "$warn" = 0 ] || echo "check-bundle: $warn allowlisted libraries are not installed on this machine (ALLOW_MISSING_HOST=1)"
[ "$bad" = 0 ] && { echo "check-bundle ok: $APP"; exit 0; }
echo "check-bundle FAILED: $bad problems in $APP"; exit 1
