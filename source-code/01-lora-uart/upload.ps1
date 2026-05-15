# Upload LoRa Sender & Receiver ke Arduino Uno
# Sender  -> COM8
# Receiver -> COM9

$ROOT = $PSScriptRoot
$FQBN = "arduino:avr:uno"

function Compile-And-Upload {
    param($sketch, $port, $label)
    Write-Host "`n=== $label ===" -ForegroundColor Cyan
    Write-Host "Compile $sketch ..." -ForegroundColor Yellow
    arduino-cli compile --fqbn $FQBN "$ROOT\$sketch"
    if ($LASTEXITCODE -ne 0) { Write-Host "COMPILE GAGAL" -ForegroundColor Red; return }

    Write-Host "Upload ke $port ..." -ForegroundColor Yellow
    arduino-cli upload -p $port --fqbn $FQBN "$ROOT\$sketch"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "UPLOAD OK: $label -> $port" -ForegroundColor Green
    } else {
        Write-Host "UPLOAD GAGAL" -ForegroundColor Red
    }
}

Compile-And-Upload "sender"   "COM8" "SENDER  (COM8)"
Compile-And-Upload "receiver" "COM9" "RECEIVER (COM9)"

Write-Host "`nSelesai. Buka Serial Monitor di COM8 dan COM9, baud 9600." -ForegroundColor Green
