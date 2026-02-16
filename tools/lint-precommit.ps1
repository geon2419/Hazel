[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

function Resolve-ClangFormatPath {
    $onPath = Get-Command clang-format -ErrorAction SilentlyContinue
    if ($onPath -and (Test-Path -LiteralPath $onPath.Source)) {
        return $onPath.Source
    }

    $candidates = New-Object System.Collections.Generic.List[string]
    $candidates.Add("C:\Program Files\LLVM\bin\clang-format.exe")

    $vsRoots = @(
        "C:\Program Files\Microsoft Visual Studio\2022",
        "C:\Program Files\Microsoft Visual Studio\18"
    )
    $editions = @("Enterprise", "Professional", "Community", "BuildTools")
    $llvmBins = @(
        "VC\Tools\Llvm\x64\bin\clang-format.exe",
        "VC\Tools\Llvm\bin\clang-format.exe",
        "VC\Tools\Llvm\ARM64\bin\clang-format.exe"
    )

    foreach ($vsRoot in $vsRoots) {
        foreach ($edition in $editions) {
            foreach ($llvmBin in $llvmBins) {
                $candidates.Add((Join-Path (Join-Path $vsRoot $edition) $llvmBin))
            }
        }
    }

    $vsBase = "C:\Program Files\Microsoft Visual Studio"
    if (Test-Path -LiteralPath $vsBase) {
        $dynamicCandidates = @(Get-ChildItem -Path $vsBase -Recurse -Filter clang-format.exe -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match '\\VC\\Tools\\Llvm\\' } |
            Select-Object -ExpandProperty FullName)
        foreach ($candidate in $dynamicCandidates) {
            $candidates.Add($candidate)
        }
    }

    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    return $null
}

$repoRoot = (git rev-parse --show-toplevel).Trim()
if (-not $repoRoot) {
    Write-Host "pre-commit: failed to resolve repository root (detected OS: Windows (PowerShell))"
    exit 1
}

$clangFormat = Resolve-ClangFormatPath
if (-not $clangFormat) {
    Write-Host "pre-commit: clang-format not found on PATH (detected OS: Windows (PowerShell))"
    Write-Host "Install LLVM clang-format and retry."
    exit 1
}

$allowedExts = @(".h", ".hpp", ".hh", ".cpp", ".cc", ".cxx", ".inl")
$targetFiles = New-Object System.Collections.Generic.List[string]

Push-Location $repoRoot
try {
    $staged = @(git diff --cached --name-only --diff-filter=ACMR)
    if ($LASTEXITCODE -ne 0) {
        Write-Host "pre-commit: failed to query staged files (detected OS: Windows (PowerShell))"
        exit 1
    }

    foreach ($path in $staged) {
        $normalizedPath = $path.Trim()
        if (-not $normalizedPath) {
            continue
        }

        if ($normalizedPath -notmatch '^(Hazel/src/|Hazel-Test/src/|Sandbox/src/)') {
            continue
        }

        $ext = [System.IO.Path]::GetExtension($normalizedPath).ToLowerInvariant()
        if ($allowedExts -notcontains $ext) {
            continue
        }

        $diskPath = Join-Path $repoRoot ($normalizedPath -replace '/', '\')
        if (Test-Path -LiteralPath $diskPath) {
            $targetFiles.Add($normalizedPath)
        }
    }

    if ($targetFiles.Count -eq 0) {
        Write-Host "pre-commit: no staged C/C++ source files to lint (detected OS: Windows (PowerShell))"
        exit 0
    }

    $violations = New-Object System.Collections.Generic.List[string]
    foreach ($file in $targetFiles) {
        $tempOut = Join-Path ([System.IO.Path]::GetTempPath()) ("hazel-precommit-{0}.out" -f [Guid]::NewGuid().ToString("N"))
        $tempErr = Join-Path ([System.IO.Path]::GetTempPath()) ("hazel-precommit-{0}.err" -f [Guid]::NewGuid().ToString("N"))
        try {
            $proc = Start-Process -FilePath $clangFormat `
                -ArgumentList @("--dry-run", "--Werror", "--style=file", "--fallback-style=none", $file) `
                -NoNewWindow -Wait -PassThru `
                -RedirectStandardOutput $tempOut `
                -RedirectStandardError $tempErr

            if ($proc.ExitCode -ne 0) {
                $violations.Add($file)
            }
        }
        finally {
            Remove-Item -Path $tempOut -ErrorAction SilentlyContinue
            Remove-Item -Path $tempErr -ErrorAction SilentlyContinue
        }
    }

    if ($violations.Count -gt 0) {
        Write-Host "pre-commit: formatting violations found (detected OS: Windows (PowerShell))"
        foreach ($file in $violations) {
            Write-Host "  - $file"
        }
        exit 1
    }

    Write-Host "pre-commit: lint passed for $($targetFiles.Count) staged file(s) (detected OS: Windows (PowerShell))"
    exit 0
}
finally {
    Pop-Location
}
