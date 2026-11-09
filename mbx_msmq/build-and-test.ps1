param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [switch]$RunIntegration
)
$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path $vswhere)) { throw 'Install Visual Studio 2026 Desktop development with C++ and Windows SDK.' }
$msbuild = & $vswhere -latest -version '[18.0,19.0)' -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (!$msbuild) { throw 'Visual Studio 2026 C++ MSBuild was not found.' }
& $msbuild (Join-Path $PSScriptRoot 'mbx_msmq.sln') /m /t:Rebuild "/p:Configuration=$Configuration" /p:Platform=x64
if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
$out = Join-Path $PSScriptRoot "bin\x64\$Configuration"
$dll = Join-Path $out 'mbx.dll'
$pe = [System.IO.File]::ReadAllBytes($dll)
$peOffset = [BitConverter]::ToInt32($pe, 0x3c)
if ([BitConverter]::ToUInt16($pe, $peOffset + 4) -ne 0x8664) { throw 'DLL is not an x64 PE file.' }
Write-Host "Built x64 DLL: $dll"
if ($RunIntegration) {
    $service = Get-Service -Name MSMQ -ErrorAction SilentlyContinue
    if (!$service -or $service.Status -ne 'Running') { throw 'Enable Message Queuing (MSMQ) Server and start the MSMQ service first.' }
    & (Join-Path $out 'msmq_integration.exe')
    if ($LASTEXITCODE -ne 0) { throw "MSMQ integration tests failed: $LASTEXITCODE" }
    Write-Host 'PASS: local MSMQ integration. Remote queues and site HSMS integration require separate tests.'
} else {
    Write-Host 'Build only. Add -RunIntegration to create/test/delete unique local private test queues.'
}
