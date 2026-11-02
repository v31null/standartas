#!/bin/sh
set -eu
data=${XDG_DATA_HOME:-$HOME/.local/share}
bin=${V31NP_BIN:-$HOME/.local/bin}
rm -f "$bin/v31np" "$bin/v31np-mount" "$bin/v31np-open" "$bin/v31np-menu"
rm -f "$data/mime/packages/v31np.xml"
update-mime-database "$data/mime"
rm -f "$data/icons/hicolor/256x256/mimetypes/application-x-v31np.png"
gtk-update-icon-cache -f -t "$data/icons/hicolor" >/dev/null 2>&1 || true
rm -f "$data/applications/v31np-open.desktop"
update-desktop-database "$data/applications" >/dev/null 2>&1 || true
target=$(pkg-config --variable=extensiondir libnautilus-extension-4 2>/dev/null || true)
if [ -n "$target" ] && [ -f "$target/libv31np-nautilus.so" ]; then
    sudo rm -f "$target/libv31np-nautilus.so"
    nautilus -q >/dev/null 2>&1 || true
fi
printf 'removed from %s and %s\n' "$bin" "$data"
