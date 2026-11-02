#!/usr/bin/env bash
set -u
BUILD="${1:-$(dirname "$0")/../build}"
BUILD="$(cd "$BUILD" && pwd)"
SHARE="${2:-}"
DESKTOP="$(cd "$(dirname "$0")/../desktop" && pwd)"
CLI="$BUILD/v31np"
MOUNT="$BUILD/v31np-mount"
WORK="$(mktemp -d)"
fails=0
checks=0
declare -A PIDS

check() {
    checks=$((checks + 1))
    if eval "$2"; then echo "ok   $1"; else echo "FAIL $1"; fails=$((fails + 1)); fi
}

up() {
    "$MOUNT" "$1" "$2" -f 2>>"$WORK/mount.log" &
    MPID=$!
    PIDS["$2"]=$MPID
    for _ in $(seq 1 50); do mountpoint -q "$2" && return 0; sleep 0.1; done
    return 1
}

down() {
    for _ in $(seq 1 50); do fusermount3 -u "$1" 2>/dev/null && break; sleep 0.1; done
    if mountpoint -q "$1"; then
        checks=$((checks + 1)); fails=$((fails + 1)); echo "FAIL unmount of $1 within 5 s"
        fusermount3 -uz "$1" 2>/dev/null; kill "${PIDS[$1]}" 2>/dev/null
    fi
    wait "${PIDS[$1]}" 2>/dev/null
}

cleanup() {
    for m in "$WORK"/mnt*; do mountpoint -q "$m" 2>/dev/null && fusermount3 -u "$m"; done
    [ -n "$SHARE" ] && [ -n "${ROAD:-}" ] && rm -rf "$ROAD"
    rm -rf "$WORK"
}
trap cleanup EXIT

cd "$WORK"
export XDG_CACHE_HOME="$WORK/cache"
J="FUTALUZTU UERSIIA UUTI flat.v31np.txt"

settled() {
    for _ in $(seq 1 60); do [ ! -e "$J" ] && return 0; sleep 0.1; done
    return 1
}
mkdir -p src/docs/deep "src/ünï cødé" src/ALICEV31NP mnt
printf Hello > src/docs/note.txt
printf x > 'src/ALICEV31NP/2026\1-1.txt'
head -c 2000000 /dev/urandom > "src/ünï cødé/big.bin"
: > src/empty.txt
printf a > src/A.txt
printf b > src/a.txt
for n in photo.zip photo.avi "№12" "№7" v u c s 9; do printf '%s' "$n" > "src/docs/deep/$n"; done
"$CLI" pack flat.v31np src >/dev/null

up flat.v31np mnt
check "flat archive mounts" "mountpoint -q mnt"
check "mounted tree equals source" "diff -r src mnt >/dev/null"
check "V31N order with folders first" "[ \"\$(ls -U mnt/docs/deep | tr '\n' ' ')\" = '9 v u photo.avi photo.zip s c №7 №12 ' ]"
check "case-sensitive names both present" "[ -f mnt/A.txt ] && [ -f mnt/a.txt ]"
check "files are writable on a local disk" "[ \"\$(stat -c %a mnt/docs/note.txt)\" = 644 ]"
echo hi > mnt/new.txt
check "new file shows at once" "ls mnt | grep -qx new.txt && [ \"\$(cat mnt/new.txt)\" = hi ]"
mv mnt/docs/note.txt mnt/docs/renamed.txt
check "rename shows at once" "ls mnt/docs | grep -qx renamed.txt && ! ls mnt/docs | grep -qx note.txt && [ \"\$(cat mnt/docs/renamed.txt)\" = Hello ]"
check "journal under the plain name, listed in .hidden" "[ -f \"\$J\" ] && [ ! -e \".\$J\" ] && grep -qxF \"\$J\" .hidden"
mv mnt/A.txt mnt/a.txt
check "rename onto a taken name replaces it" "[ ! -e mnt/A.txt ] && [ \"\$(cat mnt/a.txt)\" = a ]"
printf ' more' >> mnt/docs/renamed.txt
check "append copies on write" "[ \"\$(cat mnt/docs/renamed.txt)\" = 'Hello more' ]"
printf saved > mnt/docs/.renamed.swp && mv mnt/docs/.renamed.swp mnt/docs/renamed.txt
check "editor save by temp and rename" "[ \"\$(cat mnt/docs/renamed.txt)\" = saved ] && [ ! -e mnt/docs/.renamed.swp ]"
mkdir mnt/fresh
check "new folder shows at once" "[ -d mnt/fresh ]"
rm mnt/new.txt
check "removed file is gone at once" "[ ! -e mnt/new.txt ] && ! ls mnt | grep -qx new.txt"
check "non-empty folder refuses rmdir" "! rmdir mnt/docs 2>/dev/null && [ -d mnt/docs ]"
check "symlinks are refused" "! ln -s x mnt/link 2>/dev/null && [ ! -e mnt/link ] && [ ! -L mnt/link ]"
down mnt
rm -rf got
"$CLI" extract flat.v31np got >/dev/null 2>&1
check "unmount repacks the archive" "[ ! -e \"\$J\" ] && \"$CLI\" test flat.v31np >/dev/null"
check "repacked contents are right" "[ \"\$(cat got/docs/renamed.txt)\" = saved ] && [ \"\$(cat got/a.txt)\" = a ] && [ ! -e got/A.txt ] && [ ! -e got/new.txt ] && [ ! -e got/docs/note.txt ] && cmp -s 'got/ünï cødé/big.bin' 'src/ünï cødé/big.bin'"
check "no temps left in the cache" "[ -z \"\$(ls -A cache/v31np)\" ]"

up flat.v31np mnt
printf idle > mnt/idle.txt
exec 4< mnt/docs/renamed.txt
sleep 2.5
check "no repack while a file is open" "[ -e \"\$J\" ]"
exec 4<&-
check "idle repack after 1.5 s" "settled && [ \"\$(cat mnt/idle.txt)\" = idle ] && [ -z \"\$(ls -A cache/v31np)\" ]"
exec 4< mnt/docs/renamed.txt
printf mine > mnt/mine.txt
tp=$(awk -F'\t' '$1 == "T" { print $4 }' "$J")
sed -i -E 's/^(T\t[^\t]*\t[^\t]*\t[^\t]*\t)[^\t\r]*/\1elsewhere-1/' "$J"
check "a foreign row makes the mount read-only" "! (echo z > mnt/other.txt) 2>/dev/null && [ \"\$(stat -c %a mnt/docs/renamed.txt)\" = 444 ]"
check "a foreign row's file is not shown" "[ ! -e mnt/mine.txt ]"
exec 4<&-
down mnt
check "a foreign journal is never repacked and its files are kept" "[ -n \"\$tp\" ] && [ -f \"\$J\" ] && [ -f \"\$tp\" ] && grep -q elsewhere-1 \"\$J\""
rm -f "$J" "$tp"

up flat.v31np mnt
printf old > mnt/old.txt
kill -9 "${PIDS[mnt]}" 2>/dev/null
wait "${PIDS[mnt]}" 2>/dev/null
fusermount3 -u mnt 2>/dev/null
check "a killed mount leaves its journal" "[ -f \"\$J\" ]"
mv "$J" ".$J"
up flat.v31np mnt
check "a journal under the old dotted name is read" "[ \"\$(cat mnt/old.txt)\" = old ]"
printf more > mnt/more.txt
check "the next change moves it to the plain name" "[ -f \"\$J\" ] && [ ! -e \".\$J\" ] && [ \"\$(cat mnt/old.txt)\" = old ] && [ \"\$(cat mnt/more.txt)\" = more ]"
down mnt
rm -rf got
"$CLI" extract flat.v31np got >/dev/null 2>&1
check "the moved journal repacks" "[ ! -e \"\$J\" ] && [ ! -e \".\$J\" ] && [ \"\$(cat got/old.txt)\" = old ] && [ \"\$(cat got/more.txt)\" = more ] && [ -z \"\$(ls -A cache/v31np)\" ]"
printf 'mine' > .hidden
up flat.v31np mnt
printf h > mnt/h.txt
check ".hidden without a final newline keeps its last line" "[ \"\$(head -n1 .hidden)\" = mine ] && grep -qxF \"\$J\" .hidden && [ \"\$(grep -cxF \"\$J\" .hidden)\" = 1 ]"
printf h2 > mnt/h2.txt
check ".hidden lists the journal once" "[ \"\$(grep -cxF \"\$J\" .hidden)\" = 1 ]"
down mnt

mkdir -p deepest/z mid/m out
printf bottom > deepest/z/bottom.txt
head -c 300000 /dev/urandom > deepest/big.bin
"$CLI" pack mid/deep.v31np deepest >/dev/null
printf middle > mid/m/mid.txt
"$CLI" pack out/mid.v31np mid >/dev/null
printf top > out/top.txt
printf 'not an archive' > out/fake.v31np
"$CLI" pack nested.v31np out >/dev/null
up nested.v31np mnt
check "nested archive shown as folder" "[ -d mnt/mid.v31np ] && [ -d mnt/mid.v31np/deep.v31np ]"
check "nested data byte-identical" "cmp -s mnt/mid.v31np/deep.v31np/big.bin deepest/big.bin && [ \"\$(cat mnt/mid.v31np/deep.v31np/z/bottom.txt)\" = bottom ]"
check "invalid .v31np stays a file" "[ -f mnt/fake.v31np ] && [ \"\$(cat mnt/fake.v31np)\" = 'not an archive' ]"
N="FUTALUZTU UERSIIA UUTI nested.v31np.txt"
echo new > mnt/mid.v31np/new.txt
check "new file inside a nested archive shows at once" "[ \"\$(cat mnt/mid.v31np/new.txt)\" = new ]"
echo deeper > mnt/mid.v31np/deep.v31np/z/added.txt
check "writing two levels deep" "[ \"\$(cat mnt/mid.v31np/deep.v31np/z/added.txt)\" = deeper ]"
mv mnt/mid.v31np/m/mid.txt mnt/mid.v31np/m/renamed.txt
check "rename inside a nested archive shows at once" "[ \"\$(cat mnt/mid.v31np/m/renamed.txt)\" = middle ] && [ ! -e mnt/mid.v31np/m/mid.txt ]"
printf ' more' >> mnt/mid.v31np/deep.v31np/z/bottom.txt
check "append copies on write two levels deep" "[ \"\$(cat mnt/mid.v31np/deep.v31np/z/bottom.txt)\" = 'bottom more' ]"
mkdir mnt/mid.v31np/deep.v31np/hollow
rm mnt/mid.v31np/deep.v31np/big.bin
check "mkdir and rm inside a nested archive" "[ -d mnt/mid.v31np/deep.v31np/hollow ] && [ ! -e mnt/mid.v31np/deep.v31np/big.bin ]"
mv mnt/top.txt mnt/mid.v31np/top.txt
check "move between levels" "[ ! -e mnt/top.txt ] && [ \"\$(cat mnt/mid.v31np/top.txt)\" = top ]"
check "renaming a changed nested archive to a plain name is refused" "! mv mnt/mid.v31np mnt/mid.zip 2>/dev/null && [ -d mnt/mid.v31np ]"
mv mnt/mid.v31np mnt/middle.v31np
mkdir mnt/box
mv mnt/middle.v31np mnt/box/
mv mnt/box mnt/crate
check "renaming folders above a nested archive keeps its changes" "[ \"\$(cat mnt/crate/middle.v31np/new.txt)\" = new ] && [ \"\$(cat mnt/crate/middle.v31np/deep.v31np/z/added.txt)\" = deeper ] && [ ! -e mnt/mid.v31np ]"
down mnt
rm -rf g1 g2 g3
"$CLI" extract nested.v31np g1 >/dev/null 2>&1
"$CLI" extract g1/crate/middle.v31np g2 >/dev/null 2>&1
"$CLI" extract g2/deep.v31np g3 >/dev/null 2>&1
check "unmount repacks nested archives" "[ ! -e \"\$N\" ] && \"$CLI\" test nested.v31np >/dev/null && \"$CLI\" test g1/crate/middle.v31np >/dev/null && \"$CLI\" test g2/deep.v31np >/dev/null"
check "repacked nested contents are right" "[ \"\$(cat g2/new.txt)\" = new ] && [ \"\$(cat g2/top.txt)\" = top ] && [ \"\$(cat g2/m/renamed.txt)\" = middle ] && [ \"\$(cat g3/z/added.txt)\" = deeper ] && [ \"\$(cat g3/z/bottom.txt)\" = 'bottom more' ] && [ ! -e g3/big.bin ] && [ ! -e g1/top.txt ] && [ -f g1/fake.v31np ]"
check "no temps left after the nested repack" "[ -z \"\$(ls -A cache/v31np)\" ]"

up nested.v31np mnt
cp mid/deep.v31np mnt/copied.v31np
check "an archive copied in opens as a folder" "[ -d mnt/copied.v31np ] && cmp -s mnt/copied.v31np/big.bin deepest/big.bin && [ \"\$(cat mnt/copied.v31np/z/bottom.txt)\" = bottom ]"
echo inside > mnt/copied.v31np/inside.txt
rm -r mnt/crate/middle.v31np/deep.v31np
check "rm -r removes a nested archive" "[ ! -e mnt/crate/middle.v31np/deep.v31np ] && [ \"\$(cat mnt/copied.v31np/inside.txt)\" = inside ]"
check "a removed nested archive leaves no section behind" "grep -q \$'^@\\tcopied.v31np\\r\$' \"\$N\" && ! grep -q 'deep.v31np' \"\$N\""
down mnt
rm -rf g1 g2 g4
"$CLI" extract nested.v31np g1 >/dev/null 2>&1
"$CLI" extract g1/crate/middle.v31np g2 >/dev/null 2>&1
"$CLI" extract g1/copied.v31np g4 >/dev/null 2>&1
check "repacked copy and removal are right" "[ \"\$(cat g4/inside.txt)\" = inside ] && cmp -s g4/big.bin deepest/big.bin && [ ! -e g2/deep.v31np ] && [ \"\$(cat g2/new.txt)\" = new ] && [ -z \"\$(ls -A cache/v31np)\" ]"

mkdir -p bomb
printf seed > bomb/seed.txt
"$CLI" pack l0.v31np bomb >/dev/null
for i in $(seq 1 20); do rm -rf w; mkdir w; mv "l$((i - 1)).v31np" w/; "$CLI" pack "l$i.v31np" w >/dev/null; done
up l20.v31np mnt
p=mnt; n=0
while [ -d "$p" ] && [ $n -lt 40 ]; do p="$p/$(ls "$p" | head -1)"; n=$((n + 1)); done
check "nesting stops at 16 levels" "[ $n -eq 17 ] && [ -f \"$p\" ]"
down mnt

up flat.v31np mnt
exec 3< "mnt/ünï cødé/big.bin"
head -c 100 <&3 >/dev/null
mkdir -p second
printf 'second version' > second/only.txt
"$CLI" pack swap.v31np second >/dev/null
mv swap.v31np flat.v31np
sleep 1.2
check "listing refreshes after the archive is replaced" "[ \"\$(ls mnt)\" = only.txt ]"
rest=$(head -c 1999900 <&3 | wc -c)
exec 3<&-
check "open handle keeps its snapshot" "[ $rest -eq 1999900 ]"
down mnt

mkdir -p sel/photos/sub sel/photos/hollow
printf a > sel/photos/sub/a.txt
printf n > sel/note.txt
"$CLI" make sel/made.v31np sel/photos sel/note.txt sel/photos/ >/dev/null
check "make packs a selection at the top level" "[ \"\$(\"$CLI\" list sel/made.v31np | awk 'NF == 3 { print \$3 }' | tr '\n' ' ')\" = 'note.txt photos/sub/a.txt ' ]"
check "make never overwrites" "! \"$CLI\" make sel/made.v31np sel/note.txt 2>/dev/null && [ \"\$(\"$CLI\" list sel/made.v31np | tail -1)\" = \"\$(\"$CLI\" list sel/made.v31np | tail -1)\" ] && \"$CLI\" test sel/made.v31np >/dev/null"
check "name gives the first free name with a suffix" "[ \"\$(\"$CLI\" name sel made.v31np)\" = madea.v31np ] && [ \"\$(\"$CLI\" name sel fresh.v31np)\" = fresh.v31np ]"
MENU="$DESKTOP/v31np-menu"
PATH="$BUILD:$PATH" "$MENU" pack sel/photos
PATH="$BUILD:$PATH" "$MENU" name sel/photos.v31np
PATH="$BUILD:$PATH" "$MENU" allhere sel/photos.v31np sel/made.v31np
check "menu: pack, extract to its folder, extract several" "\"$CLI\" test sel/photos.v31np >/dev/null && [ -f sel/photos/photos/sub/a.txt ] && [ -f 'sel/baíbíz e alùz/made/note.txt' ] && [ -f 'sel/baíbíz e alùz/photos/photos/sub/a.txt' ]"
"$MOUNT" --idle=2 sel/made.v31np mnt 2>>"$WORK/mount.log"
for _ in $(seq 1 50); do mountpoint -q mnt && break; sleep 0.1; done
exec 6< mnt/note.txt
sleep 3
check "idle mount stays while a file is open" "mountpoint -q mnt"
exec 6<&-
for _ in $(seq 1 80); do grep -q " $WORK/mnt " /proc/mounts || break; sleep 0.1; done
check "idle mount unmounts itself afterwards" "! grep -q \" $WORK/mnt \" /proc/mounts"

if [ -n "$SHARE" ]; then
    ROAD="$SHARE/mount-test-$$"
    RJ="$ROAD/FUTALUZTU UERSIIA UUTI road.v31np.txt"
    POST="$ROAD/NIEURFASMIT UUTI road.v31np.lock"
    mkdir -p bin
    cat > bin/zenity <<ZEN
#!/usr/bin/env bash
printf '%s\n' "\$@" > "$WORK/zenity.args"
exit "\$(cat "$WORK/zenity.answer" 2>/dev/null || echo 1)"
ZEN
    chmod +x bin/zenity
    export PATH="$WORK/bin:$PATH"
    officer() { sed -n '1,/^@/p' "$RJ" | tr -d '\r'; }
    newest() { ls "$1/ALICEV31NP" 2>/dev/null | grep -F "$PLATE\\" | sed "s/^.*\\\\//; s/\\.txt\$//" | sort -n | tail -1; }
    record() { tr -d '\r' < "$1/ALICEV31NP/$PLATE\\$(newest "$1").txt"; }
    recordat() { tr -d '\r' < "$1/ALICEV31NP/$PLATE\\$(( $(newest "$1") - $2 )).txt"; }
    mkdir -p "$ROAD" roadsrc/docs roadin roadnew mnt2
    head -c 3000000 /dev/urandom > roadsrc/big.bin
    printf old > roadsrc/v.txt
    printf Hello > roadsrc/docs/note.txt
    printf inner > roadin/i.txt
    "$CLI" pack roadsrc/inner.v31np roadin >/dev/null
    printf new > roadnew/v.txt
    "$CLI" pack "$ROAD/road.v31np" roadsrc >/dev/null
    "$CLI" pack roadnew.v31np roadnew >/dev/null
    up "$ROAD/road.v31np" mnt
    P1=$MPID
    up "$ROAD/road.v31np" mnt2
    check "share: files are writable" "[ \"\$(stat -c %a mnt/v.txt)\" = 644 ]"
    exec 3< mnt/big.bin
    head -c 1000 <&3 >/dev/null
    check "share: the archive is not held open, even with a file open" "! ls -l /proc/$P1/fd 2>/dev/null | grep -q road.v31np"
    exec 3<&-
    echo hi > mnt/new.txt
    sleep 1.2
    check "share: a new file lands and the other mount sees it" "[ \"\$(cat mnt/new.txt)\" = hi ] && [ \"\$(cat mnt2/new.txt)\" = hi ]"
    OSNAME=$(. /etc/os-release; echo "$PRETTY_NAME")
    check "share: the post shows plate and system" "head -c 300 \"\$POST\" | tr -d '\\0' | grep -qF \"os=\$OSNAME\""
    PLATE=$(head -c 300 "$POST" | tr -d '\0' | sed -n 's/^plate=\([^ ]*\) os=.*/\1/p')
    YEAR=$(date +%Y)
    check "share: no temps left beside the archive or in the cache" "[ \"\$(ls -A \"$ROAD\" | grep -v -e '^road.v31np\$' -e '^NIEURFASMIT UUTI road.v31np.lock\$' -e '^.hidden\$' -e '^FUTALUZTU UERSIIA UUTI road.v31np.txt\$')\" = '' ] && [ -z \"\$(ls -A cache/v31np)\" ]"
    check "share: the journal holds the officer, the lane free, listed in .hidden" "officer | grep -qx \$'S\\t1' && officer | grep -q \$'^W\\t.*\\tleave\$' && ! officer | grep -q \$'^[LQ]\\t' && grep -qxF 'FUTALUZTU UERSIIA UUTI road.v31np.txt' \"$ROAD/.hidden\""
    check "share: the car leaves a record of custody" "[ -n \"\$PLATE\" ] && [ -f \"mnt2/ALICEV31NP/\$PLATE\\\\1.txt\" ] && [ -f \"mnt2/ALICEV31NP/\$YEAR\\\\1-1.txt\" ] && [ \"\$(record mnt2 | sed -n '1p;2p;6p;7p' | tr '\\n' '|')\" = \"\$PLATE|\$OSNAME|add|new.txt|\" ]"
    check "share: the year file starts with the key of the plate file" "[ \"\$(tr -d '\\r' < \"mnt2/ALICEV31NP/\$YEAR\\\\1-1.txt\" | head -1)\" = \"ALICEV31NP/\$PLATE\\\\\\\\1.txt\" ]"
    printf more >> mnt/new.txt
    mv mnt/new.txt mnt/renamed.txt
    sleep 1.2
    check "share: append and rename land" "[ \"\$(cat mnt2/renamed.txt)\" = 'hi
more' ] && [ ! -e mnt2/new.txt ]"
    check "share: a rename records the old key then the new key" "[ \"\$(record mnt2 | sed -n '6,8p' | tr '\\n' '|')\" = 'rename|new.txt|renamed.txt|' ]"
    mkdir mnt/empty
    sleep 1.2
    check "share: an empty folder lives in an officer-only journal" "[ -d mnt2/empty ] && [ -f \"\$RJ\" ] && grep -q \$'^D\\tempty' \"\$RJ\" && ! grep -qE \$'^[TFMRH]\\t' \"\$RJ\""
    check "share: the empty folder is leased to its creator" "grep -q \$'^D\\tempty\\t'\"\$PLATE\"\$'\\t' \"\$RJ\""
    rmdir mnt/empty
    sleep 1.2
    check "share: removing the empty folder keeps the journal, without the folder" "[ ! -e mnt2/empty ] && [ -f \"\$RJ\" ] && ! grep -q \$'^D\\t' \"\$RJ\""
    printf saved > mnt/.renamed.swp && mv mnt/.renamed.swp mnt/renamed.txt
    check "share: editor save by temp and rename" "[ \"\$(cat mnt/renamed.txt)\" = saved ] && [ ! -e mnt/.renamed.swp ]"
    check "share: an editor save records the temp's add, a delete, then the rename" "[ \"\$(recordat mnt 2 | sed -n '6,7p' | tr '\\n' '|')\" = 'add|.renamed.swp|' ] && [ \"\$(recordat mnt 1 | sed -n '6,7p' | tr '\\n' '|')\" = 'delete|renamed.txt|' ] && [ \"\$(recordat mnt 0 | sed -n '6,8p' | tr '\\n' '|')\" = 'rename|.renamed.swp|renamed.txt|' ]"
    N0=$(newest mnt)
    printf over > mnt/renamed.txt
    check "share: an overwrite lands once" "[ \"\$(newest mnt)\" = \"\$((N0 + 2))\" ]"
    check "share: an overwrite records a delete, then an add" "[ \"\$(cat mnt/renamed.txt)\" = over ] && [ \"\$(recordat mnt 1 | sed -n '6,7p' | tr '\\n' '|')\" = 'delete|renamed.txt|' ] && [ \"\$(recordat mnt 0 | sed -n '6,7p' | tr '\\n' '|')\" = 'add|renamed.txt|' ]"
    exec 4> mnt/w.part
    printf part >&4
    mv mnt/w.part mnt/w.txt
    printf more >&4
    exec 4>&-
    check "share: a file renamed while still open lands whole" "[ \"\$(cat mnt/w.txt)\" = partmore ] && [ ! -e mnt/w.part ]"
    mv mnt/w.txt mnt/v.txt
    check "share: a rename onto a taken name records a delete, then the rename" "[ \"\$(cat mnt/v.txt)\" = partmore ] && [ ! -e mnt/w.txt ] && [ \"\$(recordat mnt 1 | sed -n '6,7p' | tr '\\n' '|')\" = 'delete|v.txt|' ] && [ \"\$(recordat mnt 0 | sed -n '6,8p' | tr '\\n' '|')\" = 'rename|w.txt|v.txt|' ]"
    N0=$(newest mnt)
    printf x > mnt/fresh.txt
    check "share: a new file written by the shell lands once" "[ \"\$(cat mnt/fresh.txt)\" = x ] && [ \"\$(newest mnt)\" = \"\$((N0 + 1))\" ] && [ \"\$(record mnt | sed -n '6,7p' | tr '\\n' '|')\" = 'add|fresh.txt|' ]"
    : > mnt/fresh.txt
    touch mnt/touched.txt
    sleep 1.2
    check "share: emptying and touching land at close" "[ -f mnt2/fresh.txt ] && [ ! -s mnt2/fresh.txt ] && [ -f mnt2/touched.txt ] && [ \"\$(newest mnt)\" = \"\$((N0 + 4))\" ] && [ \"\$(record mnt | sed -n '6,7p' | tr '\\n' '|')\" = 'add|touched.txt|' ]"
    echo n > mnt/inner.v31np/n.txt
    rm mnt/docs/note.txt
    sleep 1.2
    check "share: nested write and remove land" "[ \"\$(cat mnt2/inner.v31np/n.txt)\" = n ] && [ \"\$(cat mnt2/inner.v31np/i.txt)\" = inner ] && [ ! -e mnt2/docs/note.txt ] && [ -d mnt2/docs ]"
    check "share: a nested change is recorded inside the nested archive" "[ \"\$(record mnt2/inner.v31np | sed -n '6,7p' | tr '\\n' '|')\" = 'add|n.txt|' ] && [ \"\$(record mnt2 | sed -n '6,7p' | tr '\\n' '|')\" = 'delete|docs/note.txt|' ]"
    printf one > c1.txt; printf two > c2.txt
    cp c1.txt mnt/c1.txt & C1=$!
    cp c2.txt mnt2/c2.txt & C2=$!
    wait $C1 $C2
    sleep 1.2
    check "share: two mounts writing at once both land" "[ \"\$(cat mnt/c2.txt)\" = two ] && [ \"\$(cat mnt2/c1.txt)\" = one ] && \"$CLI\" test \"$ROAD/road.v31np\" >/dev/null"
    stall() {
        touch "$ROAD/clock.probe"; local now; now=$(stat -c %Y "$ROAD/clock.probe"); rm -f "$ROAD/clock.probe"
        python3 - "$RJ" "$now" "$1" <<'PY'
import sys, time
path, now, ahead = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
def stamp(t):
    g = time.gmtime(t + 9 * 3600)
    return "%04d%02d%02d%s%02d%02d%02d" % (g.tm_year, g.tm_mon, g.tm_mday, "AM" if g.tm_hour < 12 else "PM", g.tm_hour % 12, g.tm_min + 1, g.tm_sec + 1)
text = open(path, newline="").read()
head, sep, rest = text.partition("\r\n@")
lines = head.split("\r\n")
shift = 0
for i, l in enumerate(lines):
    if l.startswith("G\t"):
        shift = int(l.split("\t")[1]) + 1
        lines[i] = "G\t%d" % shift
lines = [l for l in lines if not l.startswith("L\t")]
at = max(i for i, l in enumerate(lines) if l.startswith(("G\t", "N\t", "S\t"))) + 1
lines.insert(at, "L\t999\tghost-1\tGhost OS\t%s\t%s\t%d" % (stamp(now - 200), stamp(now + ahead), shift))
open(path + ".edit", "w", newline="").write("\r\n".join(lines) + sep + rest)
PY
        mv -f "$RJ.edit" "$RJ"
    }
    echo 0 > zenity.answer
    stall 30
    printf yes > mnt/over.txt
    sleep 1.2
    check "share: a missed holder with one waiting car asks, and Yes overrides" "grep -qx -- '--title=road.v31np' zenity.args && grep -qx -- '--text=Still waiting for ghost-1 (Ghost OS), which holds this archive. Go ahead anyway?' zenity.args && [ \"\$(cat mnt2/over.txt)\" = yes ] && officer | grep -q \$'^W\\t999\\tghost-1\\tGhost OS\\t.*\\toverridden\$' && officer | grep -q \$'\\toverride\$'"
    check "share: the override is recorded in custody" "ls mnt2/ALICEV31NP | grep -F \"\$PLATE\\\\\" >/dev/null && for f in mnt2/ALICEV31NP/\"\$PLATE\"*; do tr -d '\\r' < \"\$f\"; done | grep -qx override"
    rm -f zenity.args; echo 1 > zenity.answer
    stall 15
    start=$(date +%s)
    printf no > mnt/wait.txt
    took=$(( $(date +%s) - start ))
    check "share: No waits for the lease to run out, then drives" "grep -qx -- '--title=road.v31np' zenity.args && [ \"\$(cat mnt/wait.txt)\" = no ] && [ \$took -ge 10 ] && officer | grep -q \$'\\tghost-1\\tGhost OS\\t.*\\texpired\$'"
    check "share: untouched data intact and the archive valid" "cmp -s mnt2/big.bin roadsrc/big.bin && \"$CLI\" test \"$ROAD/road.v31np\" >/dev/null"
    down mnt2
    exec 3< mnt/big.bin
    head -c 1000 <&3 >/dev/null
    cp roadnew.v31np "$ROAD/road.tmp" && mv -f "$ROAD/road.tmp" "$ROAD/road.v31np"
    cat <&3 > roadrest.bin 2>/dev/null
    exec 3<&-
    tail -c +1001 roadsrc/big.bin > roadold.bin
    check "share: an open file never reads a replaced archive" "cmp -s -n \"\$(stat -c %s roadrest.bin)\" roadrest.bin roadold.bin"
    sleep 1.2
    check "share: listing refreshes after an outside replace" "[ \"\$(ls mnt)\" = v.txt ] && [ \"\$(cat mnt/v.txt)\" = new ]"
    down mnt
    check "share: nothing left behind" "[ -z \"\$(ls -A cache/v31np)\" ] && [ \"\$(ls -A \"$ROAD\" | grep -v -e '^road.v31np\$' -e '^NIEURFASMIT UUTI road.v31np.lock\$' -e '^.hidden\$' -e '^FUTALUZTU UERSIIA UUTI road.v31np.txt\$')\" = '' ]"
fi

check "no sanitizer reports from the mount" "! grep -qE 'AddressSanitizer|LeakSanitizer|runtime error' mount.log"
echo "$checks checks, $fails failures"
[ $fails -eq 0 ]
