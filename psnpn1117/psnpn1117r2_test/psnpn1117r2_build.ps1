$ErrorActionPreference = "Stop"
$here = $PSScriptRoot
$msquicVersion = "2.6.4"
$msquicSha = "DFE047F99B6571F107DBE8FF32501DE177B1B0FBE97C6FA143C4E1045AC775E5"
$msquicUrl = "https://www.nuget.org/api/v2/package/Microsoft.Native.Quic.MsQuic.OpenSSL/$msquicVersion"
$dll = Join-Path $here "msquic.dll"
$csc = Join-Path $env:WINDIR "Microsoft.NET\Framework64\v4.0.30319\csc.exe"

if (-not (Test-Path $dll)) {
    $package = (Invoke-WebRequest -UseBasicParsing $msquicUrl).Content
    Add-Type -AssemblyName System.IO.Compression
    $zip = New-Object System.IO.Compression.ZipArchive (New-Object System.IO.MemoryStream (, $package))
    $entry = $zip.GetEntry("build/native/bin/x64/msquic.dll")
    $buffer = New-Object System.IO.MemoryStream
    $stream = $entry.Open()
    $stream.CopyTo($buffer)
    $stream.Dispose()
    $zip.Dispose()
    $bytes = $buffer.ToArray()
    $sha = [BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($bytes)).Replace("-", "")
    if ($sha -ne $msquicSha) { throw "msquic.dll $sha" }
    [IO.File]::WriteAllBytes($dll, $bytes)
}

& $csc /nologo /unsafe /optimize /platform:x64 /target:exe "/out:$(Join-Path $here 'test.exe')" /r:System.Web.Extensions.dll (Join-Path $here "*.cs")
if ($LASTEXITCODE -ne 0) { throw "csc" }
