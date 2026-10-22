#!/bin/bash
# Yokoi standalone / RGDS+ Stock OS. No changes to firmware files.
set -u
base=$(CDPATH= cd -P "$(dirname "$0")" && pwd) || exit 1
app="$base/Yokoi_RGDSPlus"
[ -d "$app" ] || exit 1
cd "$app" || exit 1
umask 077
exec > "$app/Yokoi_Arranque.txt" 2>&1
printf 'Yokoi RGDS+ 1.4\n'; date
[ "$(uname -m)" = aarch64 ] || { echo 'Requer Linux ARM64'; exit 1; }
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/var/run}"
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"
export SDL_VIDEODRIVER=wayland SDL_AUDIODRIVER=alsa
export SDL_MOUSE_TOUCH_EVENTS=0 SDL_TOUCH_MOUSE_EVENTS=0
export LD_LIBRARY_PATH="$app/lib:/usr/lib:/lib:/mnt/vendor/lib"
unset LD_PRELOAD
[ -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ] || { echo 'Wayland indisponivel'; exit 1; }
chmod +x ./yokoi || exit 1
if [ "${YOKOI_DIAGNOSTICS:-0}" = 1 ]; then
    /lib/ld-linux-aarch64.so.1 --list ./yokoi || exit 1
fi
# Physical input and volume are handled by the native application.
volume_node=/sys/class/anbernic_misc/openbor_volume
volume_saved=''
if [ -r "$volume_node" ] && [ -w "$volume_node" ]; then
    volume_saved=$(cat "$volume_node")
    case "$volume_saved" in 0|1|2|3|4|5|6|7|8|9|10) printf 'RGDS entry volume: %s/10\n' "$volume_saved";; *) volume_saved='';; esac
fi
touch_node=/sys/class/anbernic_misc/tpctrl
touch_saved=''
restore_system() {
    if [ -n "$touch_saved" ]; then printf '%s\n' "$touch_saved" > "$touch_node"; fi
    if [ -n "$volume_saved" ]; then
        if printf '%s\n' "$volume_saved" > "$volume_node"; then
            printf 'RGDS restored entry volume: %s/10\n' "$volume_saved"
        else
            printf 'RGDS could not restore entry volume\n'
        fi
    fi
}
trap restore_system EXIT
if [ -r "$touch_node" ] && [ -w "$touch_node" ]; then
    touch_saved=$(cat "$touch_node")
    case "$touch_saved" in 0|1) printf '0\n' > "$touch_node"; printf 'RGDS touch mode: previous=%s active=0\n' "$touch_saved";; *) touch_saved='';; esac
fi
./yokoi &
child=$!
trap 'kill -TERM "$child" 2>/dev/null; wait "$child" 2>/dev/null; exit 130' INT
trap 'kill -TERM "$child" 2>/dev/null; wait "$child" 2>/dev/null; exit 143' TERM HUP
wait "$child"
result=$?
printf 'Codigo de saida: %s\n' "$result"
exit "$result"
