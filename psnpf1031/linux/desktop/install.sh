#!/bin/sh
set -eu
here=$(dirname -- "$(realpath -- "$0")")
repo=$(realpath -- "$here/../..")
menu=no
if [ "${1:-}" = --menu ]; then
    menu=yes
    shift
fi
case "${1:-}" in
    -*) printf 'usage: install.sh [--menu] [build folder]\n' >&2; exit 2 ;;
esac
build=$(realpath -- "${1:-$repo/linux/build-release}")
data=${XDG_DATA_HOME:-$HOME/.local/share}
bin=${V31NP_BIN:-$HOME/.local/bin}
for tool in v31np v31np-mount; do
    [ -x "$build/$tool" ] || { printf 'install.sh: build %s first (%s)\n' "$tool" "$build" >&2; exit 1; }
done
install -Dm755 "$build/v31np" "$bin/v31np"
install -Dm755 "$build/v31np-mount" "$bin/v31np-mount"
install -Dm755 "$here/v31np-open" "$bin/v31np-open"
install -Dm755 "$here/v31np-menu" "$bin/v31np-menu"
install -Dm644 "$here/v31np.xml" "$data/mime/packages/v31np.xml"
update-mime-database "$data/mime"
icons="$data/icons/hicolor"
mkdir -p "$icons/256x256/mimetypes"
python3 -I - "$repo/installer/nzuu.ico" "$icons/256x256/mimetypes/application-x-v31np.png" <<'PY'
import struct, sys
data = open(sys.argv[1], "rb").read()
count = struct.unpack("<H", data[4:6])[0]
best = None
for i in range(count):
    w, h, _, _, _, _, size, off = struct.unpack("<BBBBHHII", data[6 + 16 * i:22 + 16 * i])
    side = w or 256
    if data[off:off + 8] == b"\x89PNG\r\n\x1a\n" and (best is None or side > best[0]):
        best = (side, data[off:off + size])
if best is None:
    sys.exit("no PNG image in the icon")
open(sys.argv[2], "wb").write(best[1])
PY
gtk-update-icon-cache -f -t "$icons" >/dev/null 2>&1 || true
mkdir -p "$data/applications"
sed "s#^Exec=v31np-open #Exec=$bin/v31np-open #" "$here/v31np-open.desktop" > "$data/applications/v31np-open.desktop"
update-desktop-database "$data/applications" >/dev/null 2>&1 || true
xdg-mime default v31np-open.desktop application/x-v31np
if [ "$menu" = yes ]; then
    module="$build/libv31np-nautilus.so"
    [ -f "$module" ] || { printf 'install.sh: %s is missing (libnautilus-extension-dev needed at build time)\n' "$module" >&2; exit 1; }
    target=$(pkg-config --variable=extensiondir libnautilus-extension-4)
    sudo install -Dm644 "$module" "$target/libv31np-nautilus.so"
    nautilus -q >/dev/null 2>&1 || true
    printf 'menu installed into %s (Files restarted)\n' "$target"
fi
printf 'installed into %s and %s\n' "$bin" "$data"
