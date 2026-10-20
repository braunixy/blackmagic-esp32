[CmdletBinding()]
param (
    [string]$Port
)

$ErrorActionPreference = "Stop"

# Activate ESP-IDF environment if idf.py is not yet in PATH
if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    $exportPath = "C:\Users\Brauni\esp\v5.4.1\esp-idf\export.ps1"
    if (Test-Path $exportPath) {
        Write-Host "Activating ESP-IDF 5.4.1..." -ForegroundColor Cyan
        . $exportPath
    } else {
        Write-Error "ESP-IDF export.ps1 not found at $exportPath"
    }
}

if (-not $Port) {
    # Detect COM port
    $comPorts = [System.IO.Ports.SerialPort]::GetPortNames()
    if ($comPorts.Count -eq 1) {
        $Port = $comPorts[0]
        Write-Host "Auto-detected board at $Port" -ForegroundColor Green
    } elseif ($comPorts.Count -gt 1) {
        Write-Host "Multiple COM ports detected: $($comPorts -join ', ')" -ForegroundColor Yellow
        $Port = $comPorts[0]
        Write-Host "Defaulting to $Port (pass -Port COMx to specify)" -ForegroundColor Yellow
    } else {
        Write-Host "No COM port detected. Please specify your board's port, e.g.: .\scripts\flash.ps1 -Port COM3" -ForegroundColor Red
        exit 1
    }
}

Write-Host "Flashing blackmagic firmware to $Port..." -ForegroundColor Cyan
idf.py -p $Port flash
