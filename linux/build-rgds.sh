#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."

if [ "$(uname -sm)" != "Linux x86_64" ]; then
    echo "This bootstrap requires an x86-64 Linux host." >&2
    exit 1
fi
for tool in curl sha256sum tar xz flock apt-get dpkg-deb readelf; do
    command -v "$tool" >/dev/null || { echo "Missing prerequisite: $tool" >&2; exit 1; }
done
keyring=/usr/share/keyrings/ubuntu-archive-keyring.gpg
[ -r "$keyring" ] || { echo "Install ubuntu-keyring before running this script." >&2; exit 1; }

cache="${YOKOI_RGDS_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/yokoi-rgds}"
case "$cache" in
    /*) ;;
    *) echo "YOKOI_RGDS_CACHE must be an absolute path." >&2; exit 1 ;;
esac
case "$cache" in
    *[[:space:]]*) echo "The build cache path must not contain whitespace." >&2; exit 1 ;;
esac
mkdir -p "$cache"
exec 9>"$cache/build.lock"
flock 9

name=arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu
archive="$cache/$name.tar.xz"
sdk="$cache/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu"
checksum=12fcdf13a7430655229b20438a49e8566e26551ba08759922cdaf4695b0d4e23
if [ ! -f "$sdk/.yokoi-ready" ]; then
    if [ ! -f "$archive" ]; then
        curl --fail --location --retry 3 \
            "https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu/13.2.rel1/binrel/$name.tar.xz" \
            -o "$archive.part"
        mv "$archive.part" "$archive"
    fi
    printf '%s  %s\n' "$checksum" "$archive" | sha256sum --check
    tar -xJf "$archive" -C "$cache"
    touch "$sdk/.yokoi-ready"
fi

sdl="$cache/sdl-jammy-arm64"
if [ ! -f "$sdl/.yokoi-ready" ]; then
    aptdir="$cache/apt"
    mkdir -p "$aptdir/lists/partial" "$aptdir/archives/partial" "$aptdir/packages" "$aptdir/empty" "$sdl"
    printf 'deb [arch=arm64 signed-by=%s] https://ports.ubuntu.com/ubuntu-ports jammy main universe\n' \
        "$keyring" > "$aptdir/sources.list"
    # Isolate apt configuration, indexes and downloads from the host installation.
    cat > "$aptdir/apt.conf" <<EOF
Dir::Etc::main "/dev/null";
Dir::Etc::parts "$aptdir/empty";
Dir::Etc::sourcelist "$aptdir/sources.list";
Dir::Etc::sourceparts "$aptdir/empty";
Dir::State::lists "$aptdir/lists";
Dir::Cache::archives "$aptdir/archives";
Dir::Cache::pkgcache "";
Dir::Cache::srcpkgcache "";
APT::Architecture "arm64";
APT::Architectures { "arm64"; };
APT::Sandbox::User "$(id -un)";
Acquire::Languages "none";
EOF
    APT_CONFIG="$aptdir/apt.conf" apt-get update --error-on=any
    (
        cd "$aptdir/packages"
        APT_CONFIG="$aptdir/apt.conf" apt-get download libsdl2-dev:arm64 libsdl2-2.0-0:arm64
        for package in libsdl2*.deb; do
            dpkg-deb --extract "$package" "$sdl"
        done
    )
    touch "$sdl/.yokoi-ready"
fi

export CXX="$sdk/bin/aarch64-none-linux-gnu-g++"
export CPPFLAGS="-I$sdl/usr/include/aarch64-linux-gnu"
export SDL_CFLAGS="-I$sdl/usr/include/SDL2 -D_REENTRANT"
# Only SDL is staged for linking. Its backend dependencies come from Stock OS.
export SDL_LIBS="-L$sdl/usr/lib/aarch64-linux-gnu -Wl,--allow-shlib-undefined -lSDL2"
export OUTPUT="${OUTPUT:-linux/build/yokoi-rgds}"
unset LDFLAGS
bash linux/build.sh

readelf -h "$OUTPUT" | grep -q 'Machine:.*AArch64'
readelf -l "$OUTPUT" | grep -q '/lib/ld-linux-aarch64.so.1'
versions=$(readelf --version-info "$OUTPUT")
for family in GLIBC GLIBCXX; do
    limit=2.38
    [ "$family" != GLIBCXX ] || limit=3.4.31
    required=$(printf '%s\n' "$versions" | grep -oE "${family}_[0-9.]+" | sort -Vu | tail -n 1)
    if [ -n "$required" ]; then
        highest=$(printf '%s\n%s\n' "$limit" "${required#*_}" | sort -V | tail -n 1)
        if [ "$highest" != "$limit" ]; then
            echo "Incompatible runtime requirement: $required (maximum ${family}_$limit)." >&2
            exit 1
        fi
        printf 'Runtime requirement: %s (maximum %s_%s)\n' "$required" "$family" "$limit"
    fi
done
printf 'Built and checked ARM64 Linux executable: %s\n' "$OUTPUT"
printf 'Keep the existing device SDL/runtime libraries; deploy only the executable.\n'
