$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'validate-stages.ps1')
if (!$env:GDK) { throw 'Set GDK to the SGDK installation directory.' }
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & (Join-Path $env:GDK 'bin/make.exe') -f (Join-Path $env:GDK 'makefile.gen') @args
    if ($LASTEXITCODE -ne 0) { throw "SGDK build failed: $LASTEXITCODE" }
} finally { Pop-Location }
