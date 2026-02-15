[CmdletBinding()]
param(
    [ValidateSet("format", "warnings", "all")]
    [string]$Mode = "all",

    [ValidateSet("advisory", "blocking")]
    [string]$Enforcement = "advisory",

    [switch]$Fix
)

$script:HasBlockingFailures = $false
$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$script:SourceRoots = @(
    (Join-Path $script:RepoRoot "Hazel/src"),
    (Join-Path $script:RepoRoot "Hazel-Test/src"),
    (Join-Path $script:RepoRoot "Sandbox/src")
)
$script:FileExtensions = @(".h", ".hpp", ".hh", ".cpp", ".cc", ".cxx", ".inl")
$script:ExcludePathRegex = '(?i)\\(vendor|bin|bin-int|\.vs)(\\|$)'
$script:FirstPartyWarningRegex = '(?i)(Hazel[\\/]+src|Hazel-Test[\\/]+src|Sandbox[\\/]+src)[\\/]'

function Write-Section {
    param([string]$Title)
    Write-Host ""
    Write-Host "== $Title =="
}

function Report-LintIssue {
    param([string]$Message)
    if ($Enforcement -eq "blocking") {
        Write-Host "[lint][error] $Message"
        $script:HasBlockingFailures = $true
    } else {
        Write-Warning $Message
    }
}

function Resolve-ClangFormatPath {
    $onPath = Get-Command clang-format -ErrorAction SilentlyContinue
    if ($onPath -and (Test-Path -Path $onPath.Source)) {
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
    if (Test-Path -Path $vsBase) {
        $dynamicCandidates = @(Get-ChildItem -Path $vsBase -Recurse -Filter clang-format.exe -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match '\\VC\\Tools\\Llvm\\' } |
            Select-Object -ExpandProperty FullName)
        foreach ($candidate in $dynamicCandidates) {
            $candidates.Add($candidate)
        }
    }

    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-Path -Path $candidate) {
            return $candidate
        }
    }

    return $null
}

function Get-FirstPartySourceFiles {
    $files = @()

    foreach ($sourceRoot in $script:SourceRoots) {
        if (-not (Test-Path -Path $sourceRoot)) {
            continue
        }

        $files += Get-ChildItem -Path $sourceRoot -Recurse -File
    }

    return @(
        $files |
        Where-Object {
            ($script:FileExtensions -contains $_.Extension.ToLowerInvariant()) -and
            ($_.FullName -notmatch $script:ExcludePathRegex)
        } |
        Sort-Object -Property FullName -Unique |
        Select-Object -ExpandProperty FullName
    )
}

function Invoke-FormatLint {
    param([bool]$ApplyFixes)

    Write-Section "Format lint"

    $clangFormat = Resolve-ClangFormatPath
    if (-not $clangFormat) {
        Report-LintIssue "clang-format not found. Install LLVM or Visual Studio C++ LLVM tools."
        return
    }

    $sourceFiles = @(Get-FirstPartySourceFiles)
    if ($sourceFiles.Count -eq 0) {
        Write-Host "[lint] No source files matched lint scope."
        return
    }

    Write-Host "[lint] clang-format: $clangFormat"
    Write-Host "[lint] files: $($sourceFiles.Count)"

    if ($ApplyFixes) {
        foreach ($file in $sourceFiles) {
            $tempOut = Join-Path ([System.IO.Path]::GetTempPath()) ("hazel-clang-format-{0}.out" -f [Guid]::NewGuid().ToString("N"))
            $tempErr = Join-Path ([System.IO.Path]::GetTempPath()) ("hazel-clang-format-{0}.err" -f [Guid]::NewGuid().ToString("N"))
            try {
                $proc = Start-Process -FilePath $clangFormat `
                    -ArgumentList @("--style=file", "--fallback-style=none", $file) `
                    -NoNewWindow -Wait -PassThru `
                    -RedirectStandardOutput $tempOut `
                    -RedirectStandardError $tempErr

                if ($proc.ExitCode -ne 0) {
                    $errText = ""
                    if (Test-Path -Path $tempErr) {
                        $errText = (Get-Content -Path $tempErr -Raw)
                    }
                    Report-LintIssue "clang-format failed while fixing '$file'. $errText"
                    return
                }

                $original = Get-Content -Path $file -Raw
                $formatted = Get-Content -Path $tempOut -Raw
                $useCrLf = $original.Contains("`r`n")
                if ($useCrLf) {
                    $formatted = $formatted -replace "(?<!`r)`n", "`r`n"
                } else {
                    $formatted = $formatted -replace "`r`n", "`n"
                }

                if ($formatted -ne $original) {
                    [System.IO.File]::WriteAllText(
                        $file,
                        $formatted,
                        (New-Object System.Text.UTF8Encoding($false))
                    )
                }
            }
            finally {
                Remove-Item -Path $tempOut -ErrorAction SilentlyContinue
                Remove-Item -Path $tempErr -ErrorAction SilentlyContinue
            }
        }

        Write-Host "[lint] Format fixes applied."
        return
    }

    $violations = @()
    foreach ($file in $sourceFiles) {
        $null = & $clangFormat --dry-run --Werror --style=file --fallback-style=none $file 2>&1
        if ($LASTEXITCODE -ne 0) {
            $violations += $file
        }
    }

    if ($violations.Count -eq 0) {
        Write-Host "[lint] No formatting violations found."
        return
    }

    Report-LintIssue "Formatting violations found in $($violations.Count) file(s)."
    $violations | ForEach-Object { Write-Host "  - $_" }
}

function Invoke-WarningsLint {
    Write-Section "Warning lint"

    $msbuildCommand = Get-Command msbuild -ErrorAction SilentlyContinue
    if (-not $msbuildCommand) {
        Report-LintIssue "msbuild not found on PATH."
        return
    }

    $solutionPath = Join-Path $script:RepoRoot "Hazel.sln"
    if (-not (Test-Path -Path $solutionPath)) {
        Report-LintIssue "Solution file not found: $solutionPath"
        return
    }

    $logPath = Join-Path ([System.IO.Path]::GetTempPath()) ("hazel-lint-msbuild-{0}.log" -f [Guid]::NewGuid().ToString("N"))
    Write-Host "[lint] msbuild log: $logPath"

    $buildArgs = @(
        $solutionPath,
        "/m",
        "/t:Hazel-Test",
        "/p:Configuration=Debug",
        "/p:Platform=x64",
        "/nologo",
        "/verbosity:minimal"
    )

    & $msbuildCommand.Source @buildArgs *> $logPath
    $buildExitCode = $LASTEXITCODE

    $buildOutput = @()
    if (Test-Path -Path $logPath) {
        $buildOutput = @(Get-Content -Path $logPath)
    }

    $firstPartyWarnings = @(
        $buildOutput |
        Where-Object { $_ -match '(?i)\bwarning\b' -and $_ -match $script:FirstPartyWarningRegex } |
        Select-Object -Unique
    )

    if ($firstPartyWarnings.Count -gt 0) {
        Report-LintIssue "First-party compiler warnings detected: $($firstPartyWarnings.Count)"
        $firstPartyWarnings | ForEach-Object { Write-Host "  - $_" }
    } else {
        Write-Host "[lint] No first-party compiler warnings found."
    }

    if ($buildExitCode -ne 0) {
        Report-LintIssue "msbuild exited with code $buildExitCode. Inspect log: $logPath"
    }
}

Write-Section "Hazel lint"
Write-Host "[lint] mode: $Mode"
Write-Host "[lint] enforcement: $Enforcement"
Write-Host "[lint] fix: $Fix"

if ($Fix -and $Mode -eq "warnings") {
    Write-Warning "-Fix has no effect in warnings mode."
}

Push-Location $script:RepoRoot
try {
    switch ($Mode) {
        "format" {
            Invoke-FormatLint -ApplyFixes:$Fix
        }
        "warnings" {
            Invoke-WarningsLint
        }
        "all" {
            Invoke-FormatLint -ApplyFixes:$Fix
            Invoke-WarningsLint
        }
    }
}
finally {
    Pop-Location
}

if ($script:HasBlockingFailures) {
    Write-Host ""
    Write-Host "[lint] Completed with blocking failures."
    exit 1
}

Write-Host ""
Write-Host "[lint] Completed."
exit 0
