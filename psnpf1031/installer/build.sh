#!/bin/sh
set -eu
umask 022
here=$(dirname -- "$(realpath -- "$0")")
root=$(realpath -- "$here/..")
version=1.0.0
release=1
summary="V31np format for GNU/Linux"
about="Opens .v31np archives as folders in Files (FUSE mount), with right-click extract and Baíbíz pack, and the v31np command line."
maintainer="v31n <kokain@v31null.de>"
build="$here/build/linux"
stage="$here/build/linux-stage"
out="$here/out"

cmake -S "$root/linux" -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "$build" --target v31np v31np-mount v31np-nautilus

place() {
    tree=$1
    extensions=$2
    install -Dm755 "$build/v31np" "$tree/usr/bin/v31np"
    install -Dm755 "$build/v31np-mount" "$tree/usr/bin/v31np-mount"
    install -Dm755 "$root/linux/desktop/v31np-open" "$tree/usr/bin/v31np-open"
    install -Dm755 "$root/linux/desktop/v31np-menu" "$tree/usr/bin/v31np-menu"
    strip --strip-unneeded "$tree/usr/bin/v31np" "$tree/usr/bin/v31np-mount"
    install -Dm644 "$root/linux/desktop/v31np.xml" "$tree/usr/share/mime/packages/v31np.xml"
    install -Dm644 "$root/linux/desktop/v31np-open.desktop" "$tree/usr/share/applications/v31np-open.desktop"
    install -Dm644 "$stage/icon.png" "$tree/usr/share/icons/hicolor/256x256/mimetypes/application-x-v31np.png"
    install -Dm644 "$build/libv31np-nautilus.so" "$tree$extensions/libv31np-nautilus.so"
    strip --strip-unneeded "$tree$extensions/libv31np-nautilus.so"
}

rm -rf "$stage"
mkdir -p "$stage" "$out"
python3 -I - "$here/nzuu.ico" "$stage/icon.png" <<'PY'
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

arch=$(dpkg --print-architecture)
multiarch=$(dpkg-architecture -qDEB_HOST_MULTIARCH)
debname="v31np_${version}_${arch}"
deb="$stage/deb"
place "$deb" "/usr/lib/$multiarch/nautilus/extensions-4"
work="$stage/shlibs"
mkdir -p "$work/debian"
printf 'Source: v31np\n' > "$work/debian/control"
libs=$(cd "$work" && dpkg-shlibdeps -O "$deb/usr/bin/v31np" "$deb/usr/bin/v31np-mount" 2>/dev/null | sed -n 's/^shlibs:Depends=//p')
mkdir -p "$deb/DEBIAN"
cat > "$deb/DEBIAN/control" <<CONTROL
Package: v31np
Version: $version
Architecture: $arch
Maintainer: $maintainer
Installed-Size: $(du -sk "$deb/usr" | cut -f1)
Depends: $libs, fuse3, zenity
Recommends: nautilus, libnautilus-extension4
Section: utils
Priority: optional
Description: $summary
 Opens .v31np archives as folders in Files (FUSE mount), with right-click
 extract and Baíbíz pack, and the v31np command line.
CONTROL
rm -f "$out/$debname.deb"
fakeroot dpkg-deb --build --root-owner-group "$deb" "$out/$debname.deb" >/dev/null

machine=$(uname -m)
rpmtop="$stage/rpm"
mkdir -p "$rpmtop/SPECS"
place "$stage/rpmroot" "/usr/lib64/nautilus/extensions-4"
cat > "$rpmtop/SPECS/v31np.spec" <<SPEC
%global __requires_exclude ^libnautilus-extension
%global debug_package %{nil}
%global __os_install_post %{nil}
Name: v31np
Version: $version
Release: $release
Summary: $summary
License: Proprietary
Packager: $maintainer
Requires: fuse3, zenity
Recommends: nautilus
%description
$about
%install
cp -a "$stage/rpmroot/." "%{buildroot}/"
%files
/usr/bin/v31np
/usr/bin/v31np-mount
/usr/bin/v31np-open
/usr/bin/v31np-menu
/usr/share/mime/packages/v31np.xml
/usr/share/applications/v31np-open.desktop
/usr/share/icons/hicolor/256x256/mimetypes/application-x-v31np.png
/usr/lib64/nautilus/extensions-4/libv31np-nautilus.so
SPEC
rpmbuild --quiet -bb --define "_topdir $rpmtop" --target "$machine" "$rpmtop/SPECS/v31np.spec"
rpmfile=$(find "$rpmtop/RPMS" -name '*.rpm' | head -1)
cp "$rpmfile" "$out/"

if command -v makepkg >/dev/null && command -v bsdtar >/dev/null; then
    pkgdir="$stage/arch"
    mkdir -p "$pkgdir"
    place "$stage/archroot" "/usr/lib/nautilus/extensions-4"
    cat > "$pkgdir/PKGBUILD" <<PKG
pkgname=v31np
pkgver=$version
pkgrel=$release
pkgdesc="$summary"
arch=('$machine')
license=('LicenseRef-v31n')
depends=('fuse3' 'glib2' 'zenity' 'gcc-libs' 'glibc')
optdepends=('nautilus: Files icon, double-click and right-click menu')
options=('!strip' '!debug')
package() {
    cp -a "$stage/archroot/." "\$pkgdir/"
}
PKG
    (cd "$pkgdir" && PKGDEST="$out" PKGEXT=.pkg.tar.zst makepkg --nodeps --force >/dev/null 2>"$stage/makepkg.log") || { cat "$stage/makepkg.log" >&2; rm -f "$out"/v31np-*.pkg.tar.zst; exit 1; }
else
    echo "Arch package skipped: makepkg and bsdtar are needed" >&2
fi

tarname="v31np_${version}_linux_${arch}"
place "$stage/$tarname" "/usr/lib/nautilus/extensions-4"
install -Dm755 "$here/linux-install.sh" "$stage/$tarname/install.sh"
rm -f "$out/$tarname.tar.gz"
tar -C "$stage" --owner=0 --group=0 -czf "$out/$tarname.tar.gz" "$tarname"
ls -1 "$out" | grep -E '^v31np' | sed "s#^#$out/#"
