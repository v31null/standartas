$here = $PSScriptRoot
$root = Split-Path $here
$build = Join-Path $here "build"
$out = Join-Path $here "out"
$nsisDir = Join-Path $here "nsis"
$makensis = Join-Path $nsisDir "nsis-3.10\makensis.exe"
$nsisUrl = "https://sourceforge.net/projects/nsis/files/NSIS%203/3.10/nsis-3.10.zip/download"
$nsisSha = "FCDCE3229717A2A148E7CDA0AB5BDB667F39D8FB33EDE1DA8DABC336BD5AD110"

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vcvars = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 -find "VC\Auxiliary\Build\vcvarsall.bat" | Select-Object -First 1
if (-not $vcvars) { throw "vcvarsall.bat" }

if (-not (Test-Path $makensis)) {
    New-Item -ItemType Directory -Force $nsisDir | Out-Null
    $zip = Join-Path $nsisDir "nsis-3.10.zip"
    curl.exe -sSL -A "Wget/1.21" -o $zip $nsisUrl
    if ((Get-FileHash $zip -Algorithm SHA256).Hash -ne $nsisSha) { throw "nsis-3.10.zip" }
    Expand-Archive $zip $nsisDir
}

$lib = "v31np", "tree", "order" | ForEach-Object { '"' + (Join-Path $root "lib\$_.cpp") + '"' }
$shell = "dll", "archive", "data", "folder", "menu", "drop" | ForEach-Object { '"' + (Join-Path $root "explorer\$_.cpp") + '"' }
$plugin = @('"' + (Join-Path $root "sevenzip\handler.cpp") + '"')
$flags = "/nologo /std:c++20 /utf-8 /EHsc /W4 /O2 /MT /LD /DUNICODE /D_UNICODE"
$shellLibs = "shell32.lib shlwapi.lib ole32.lib oleaut32.lib propsys.lib advapi32.lib user32.lib"
$targets = [ordered]@{ x86 = "x64_x86"; x64 = "x64"; arm64 = "x64_arm64" }

foreach ($arch in $targets.Keys) {
    $dir = Join-Path $build $arch
    $shellObj = Join-Path $dir "obj\shell"
    $pluginObj = Join-Path $dir "obj\plugin"
    New-Item -ItemType Directory -Force $shellObj, $pluginObj | Out-Null
    $res = Join-Path $dir "obj\version.res"
    $zero = "rc /nologo /fo `"$res`" `"$root\lib\version.rc`""
    $one = "cl $flags /Fo`"$shellObj\\`" $($shell + $lib -join ' ') `"$res`" /Fe:`"$dir\v31npshell.dll`" /link /DEF:`"$root\explorer\shell.def`" $shellLibs"
    $two = "cl $flags /Fo`"$pluginObj\\`" $($plugin + $lib -join ' ') `"$res`" /Fe:`"$dir\v31np.dll`" /link /DEF:`"$root\sevenzip\v31np.def`" oleaut32.lib ole32.lib"
    cmd /c "`"$vcvars`" $($targets[$arch]) >nul 2>nul && $zero && $one && $two"
    if ($LASTEXITCODE) { throw $arch }
}

New-Item -ItemType Directory -Force $out | Out-Null
& $makensis /V2 /INPUTCHARSET UTF8 "/DOUT=$out" "/DBUILD=$build" "/DHERE=$here" (Join-Path $here "installer.nsi")
exit $LASTEXITCODE
