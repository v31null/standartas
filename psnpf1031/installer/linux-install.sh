#!/bin/sh
set -eu
here=$(dirname -- "$(realpath -- "$0")")
prefix=${1:-/usr/local}
mkdir -p "$prefix/bin" "$prefix/share"
cp -r "$here/usr/bin/." "$prefix/bin/"
cp -r "$here/usr/share/." "$prefix/share/"
update-mime-database "$prefix/share/mime" >/dev/null 2>&1 || true
gtk-update-icon-cache -f -t "$prefix/share/icons/hicolor" >/dev/null 2>&1 || true
update-desktop-database "$prefix/share/applications" >/dev/null 2>&1 || true
for dir in /usr/lib/*/nautilus/extensions-4 /usr/lib64/nautilus/extensions-4 /usr/lib/nautilus/extensions-4; do
    if [ -d "$dir" ]; then
        cp "$here/usr/lib/nautilus/extensions-4/libv31np-nautilus.so" "$dir/"
        nautilus -q >/dev/null 2>&1 || true
        break
    fi
done
printf 'installed into %s\n' "$prefix"
