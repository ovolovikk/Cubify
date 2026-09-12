# ./scripts/build.ps1 Debug/Release [-Clean]

param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true

$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root 'build'

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item $buildDir -Recurse -Force
}

cmake -S $root -B $buildDir -G 'Visual Studio 17 2022' -DCUBIFY_STRICT_WARNINGS=ON
cmake --build $buildDir --config $Config
