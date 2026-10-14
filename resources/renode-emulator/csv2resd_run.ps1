<#
.SYNOPSIS
    Convert an MRT flight-log CSV into a Renode RESD stream.

.DESCRIPTION
    Wraps the two steps needed to turn an engineering-units CSV into a RESD file:

      1. pre.py       - rescales columns into RESD's integer units, strips the
                        UTF-8 BOM, rounds decimals, clamps the int32 fields.
      2. csv2resd.py  - Renode's official converter.

    Only columns actually present are mapped, so a log with just pressure and
    temperature works fine. Detection is by header name, not position.

.PARAMETER InputCsv
    Source CSV. Must have a header row.

.PARAMETER OutputResd
    Destination .resd path. Defaults to the input path with a .resd extension.

.PARAMETER Frequency
    Sample rate in Hz. Supply this for uniform data; the output is then a single
    CONSTANT_FREQUENCY block. Omit it (the default) to use the CSV's own
    timestamp column, giving ARBITRARY_TIMESTAMP blocks that preserve irregular
    spacing exactly.

.PARAMETER TimestampColumn
    Header name or 0-based index of the timestamp column, used when -Frequency is
    omitted. The default is index 0, which also sidesteps UTF-8 BOM corruption of
    the first header name.

.PARAMETER KeepScaledCsv
    Keep the intermediate scaled CSV instead of deleting it.

.EXAMPLE
    .\csv2resd_run.ps1 -InputCsv data\data.csv

.EXAMPLE
    .\csv2resd_run.ps1 -InputCsv data\data.csv -Frequency 200

.NOTES
    Requires the 'construct' package in whichever interpreter runs the conversion:

        py -m pip install construct
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$InputCsv,
    [string]$OutputResd,
    [int]$Frequency = 0,
    [string]$TimestampColumn = '0',
    [switch]$KeepScaledCsv
)

# Native tools (python, csv2resd) legitimately write to stderr; under 'Stop' that
# becomes a terminating error before $LASTEXITCODE can be inspected. Everything
# below validates explicitly and signals failure with throw, which terminates
# regardless of this setting.
$ErrorActionPreference = 'Continue'

$csv2resd = 'C:\Program Files\Renode\tools\csv2resd\csv2resd.py'
$preScript = Join-Path $PSScriptRoot 'pre.py'

function Copy-Shared {
    # Excel and friends open CSVs with a deny-read share mode, so a plain read or
    # File.Copy fails while the sheet is open. Opening with FileShare.ReadWrite
    # gets us a handle anyway, which is what you want for a working data file.
    param([string]$Source, [string]$Destination)
    $src = [System.IO.File]::Open((Resolve-Path $Source), 'Open', 'Read', 'ReadWrite')
    try {
        $dst = [System.IO.File]::Create($Destination)
        try { $src.CopyTo($dst) } finally { $dst.Close() }
    } finally { $src.Close() }
}

function Read-HeaderShared {
    param([string]$Path)
    $fs = [System.IO.File]::Open((Resolve-Path $Path), 'Open', 'Read', 'ReadWrite')
    try {
        $sr = [System.IO.StreamReader]::new($fs, $true)   # $true = strip BOM
        try { return $sr.ReadLine() } finally { $sr.Close() }
    } finally { $fs.Close() }
}

function Resolve-Python {
    # The 'python' on PATH may be the 0-byte Microsoft Store App Execution Alias,
    # which prints "Python was not found" and exits. Probe for real version output.
    $candidates = @()
    if ($env:RENODE_PYTHON) { $candidates += $env:RENODE_PYTHON }
    $candidates += @(
        (Join-Path $env:LOCALAPPDATA 'Python\pythoncore-3.14-64\python.exe'),
        'C:\Users\aneur\AppData\Local\Python\pythoncore-3.14-64\python.exe'
    )
    foreach ($c in $candidates) {
        if ($c -and (Test-Path $c)) { return $c }
    }
    foreach ($launcher in @('py', 'python', 'python3')) {
        $cmd = Get-Command $launcher -ErrorAction SilentlyContinue
        if (-not $cmd) { continue }
        if ((& $cmd --version 2>&1) -match '^Python 3') { return $cmd.Source }
    }
    throw 'No usable Python 3 found. Set $env:RENODE_PYTHON to a python.exe path.'
}

# --- validate -------------------------------------------------------------

$py = Resolve-Python
if (-not (Test-Path $csv2resd))   { throw "csv2resd not found at $csv2resd" }
if (-not (Test-Path $preScript))  { throw "pre.py not found at $preScript" }
if (-not (Test-Path $InputCsv))   { throw "Input CSV not found: $InputCsv" }

if (-not $OutputResd) {
    $dir = Split-Path -Parent (Resolve-Path $InputCsv)
    $OutputResd = Join-Path $dir ([IO.Path]::GetFileNameWithoutExtension($InputCsv) + '.resd')
}
$outDir = Split-Path -Parent $OutputResd
if ($outDir -and -not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

Write-Host "Python    : $py" -ForegroundColor DarkGray

# construct is a hard dependency of csv2resd's grammar module. Fail fast and
# clearly rather than letting it surface as a traceback from inside csv2resd.
& $py -c "import construct" 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "csv2resd requires 'construct'. Install it with:  $py -m pip install construct"
}

# --- header, with any BOM already stripped ---------------------------------

$headerLine = Read-HeaderShared $InputCsv
if (-not $headerLine) { throw "Input CSV is empty: $InputCsv" }
$header = @($headerLine -split ',' | ForEach-Object { $_.Trim() })

# Type -> source columns -> destination.
# Vectors require an explicit x,y,z destination; scalars must NOT have one, or
# construct raises "cannot unpack non-iterable int object".
$map = @(
    @{ Type = 'acceleration'; Cols = @('X Accel', 'Y Accel', 'Z Accel'); Dest = 'x,y,z:0' }
    @{ Type = 'angular_rate'; Cols = @('X AngRate', 'Y AngRate', 'Z AngRate'); Dest = 'x,y,z:0' }
    @{ Type = 'pressure';     Cols = @('Pressure');                      Dest = '' }
    @{ Type = 'temperature';  Cols = @('Temp');                           Dest = '' }
)

$mapArgs = @()
$mapped = @()
foreach ($m in $map) {
    $cols = @($m.Cols | Where-Object { $header -contains $_ })
    if ($cols.Count -ne $m.Cols.Count) { continue }
    $spec = $m.Type + ':' + ($cols -join ',')
    if ($m.Dest) { $spec += ':' + $m.Dest }
    $mapArgs += @('-m', $spec)
    $mapped += $m.Type
}

if (-not $mapped) {
    throw "None of the expected columns were found. Header was: $($header -join ', ')"
}
Write-Host "Mapped    : $($mapped -join ', ')" -ForegroundColor DarkGray

# --- work on a private copy so an open Excel handle cannot block us ---------

# Excel and other viewers open CSVs with a deny-read share mode, which makes both
# PowerShell and Python fail to read while the sheet is open. Copying through a
# FileShare.ReadWrite handle avoids needing the file to be closed.
$work = Join-Path ([IO.Path]::GetTempPath()) ('mrtcsv_' + [Guid]::NewGuid().ToString('N') + '.csv')
$scaled = Join-Path $outDir ([IO.Path]::GetFileNameWithoutExtension($OutputResd) + '.scaled.csv')
Copy-Shared $InputCsv $work

try {
    # --- step 1: rescale ----------------------------------------------------

    Write-Host "`n[1/2] rescaling columns" -ForegroundColor Cyan
    & $py $preScript $work $scaled
    if ($LASTEXITCODE -ne 0) { throw "pre.py failed (exit $LASTEXITCODE)" }

    # --- step 2: convert ---------------------------------------------------

    $convert = @($csv2resd, '-i', $scaled) + $mapArgs
    if ($Frequency -gt 0) {
        $convert += @('-f', $Frequency)
    } else {
        $convert += @('-t', $TimestampColumn)
    }
    $convert += $OutputResd

    Write-Host "`n[2/2] running csv2resd ($([IO.Path]::GetFileName($OutputResd)))" -ForegroundColor Cyan
    & $py @convert
    if ($LASTEXITCODE -ne 0) {
        # csv2resd writes an 8-byte stub before it can fail, so check the exit code.
        throw "csv2resd failed (exit $LASTEXITCODE)"
    }
}
finally {
    Remove-Item $work -ErrorAction SilentlyContinue
    if (-not $KeepScaledCsv) { Remove-Item $scaled -ErrorAction SilentlyContinue }
}

if (-not (Test-Path $OutputResd)) { throw "csv2resd reported success but produced no file" }
$size = (Get-Item $OutputResd).Length
if ($size -le 8) { throw "Output is only $size bytes - that is a bare RESD header, not data." }

Write-Host "`nOK  $OutputResd  ($size bytes)" -ForegroundColor Green
if ($Frequency -gt 0) {
    Write-Host "    CONSTANT_FREQUENCY blocks at $Frequency Hz"
} else {
    Write-Host "    ARBITRARY_TIMESTAMP blocks from column '$TimestampColumn'"
}
Write-Host "    feed it with, for example:"
Write-Host "      sysbus.i2c2.lsm6dsr FeedAccelerationSamplesFromRESD @$OutputResd" -ForegroundColor DarkGray