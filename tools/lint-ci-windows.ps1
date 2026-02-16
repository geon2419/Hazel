[CmdletBinding()]
param(
    [ValidateSet("format", "warnings", "all")]
    [string]$Mode = "all",

    [ValidateSet("advisory", "blocking")]
    [string]$Enforcement = "advisory",

    [switch]$Fix
)

$target = Join-Path $PSScriptRoot "lint.ps1"
& $target -Mode $Mode -Enforcement $Enforcement -Fix:$Fix
exit $LASTEXITCODE
