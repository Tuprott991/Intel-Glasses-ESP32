# Build and Flash Intel Glasses Project

# Check if configuration file exists
if (!(Test-Path "src\intel_glasses_config.h")) {
    Write-Host "ERROR: Configuration file not found!" -ForegroundColor Red
    Write-Host "Please copy src\config_template.h to src\intel_glasses_config.h and configure your settings" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Steps to configure:" -ForegroundColor Green
    Write-Host "1. Copy config_template.h to intel_glasses_config.h"
    Write-Host "2. Edit intel_glasses_config.h with your cloud API settings"
    Write-Host "3. Configure your 4G module APN settings"
    Write-Host "4. Verify hardware pin assignments"
    exit 1
}

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "    INTEL GLASSES BUILD SCRIPT      " -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check PlatformIO installation
Write-Host "Checking PlatformIO installation..." -ForegroundColor Yellow
$pio = Get-Command pio -ErrorAction SilentlyContinue
if (-not $pio) {
    Write-Host "ERROR: PlatformIO not found!" -ForegroundColor Red
    Write-Host "Please install PlatformIO CLI or run from PlatformIO IDE terminal" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ PlatformIO found: $($pio.Source)" -ForegroundColor Green
Write-Host ""

# Clean build environment
Write-Host "Cleaning build environment..." -ForegroundColor Yellow
pio run --target clean

# Update libraries
Write-Host "Installing/updating dependencies..." -ForegroundColor Yellow
pio lib install

# Build project
Write-Host ""
Write-Host "Building Intel Glasses firmware..." -ForegroundColor Yellow
Write-Host "Target: ESP32-S3-DevKitC-1-N16R8V" -ForegroundColor Cyan
Write-Host ""

$buildResult = pio run
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "BUILD FAILED!" -ForegroundColor Red
    Write-Host "Please check the error messages above and fix any issues." -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "✓ Build completed successfully!" -ForegroundColor Green
Write-Host ""

# Prompt for flashing
$flash = Read-Host "Do you want to flash the firmware to the device now? (y/N)"

if ($flash -eq "y" -or $flash -eq "Y" -or $flash -eq "yes") {
    Write-Host ""
    Write-Host "Flashing firmware to ESP32-S3..." -ForegroundColor Yellow
    Write-Host "Make sure your device is connected via USB and in download mode" -ForegroundColor Cyan
    Write-Host ""
    
    # Flash firmware
    $flashResult = pio run --target upload
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "FLASH FAILED!" -ForegroundColor Red
        Write-Host "Please check:" -ForegroundColor Yellow
        Write-Host "- USB cable is connected properly"
        Write-Host "- Device is in download mode (hold BOOT button while pressing RESET)"
        Write-Host "- Correct COM port is selected"
        Write-Host "- No other applications are using the COM port"
        exit 1
    }
    
    Write-Host ""
    Write-Host "✓ Firmware flashed successfully!" -ForegroundColor Green
    
    # Prompt for monitoring
    $monitor = Read-Host "Do you want to start serial monitoring? (y/N)"
    
    if ($monitor -eq "y" -or $monitor -eq "Y" -or $monitor -eq "yes") {
        Write-Host ""
        Write-Host "Starting serial monitor..." -ForegroundColor Yellow
        Write-Host "Press Ctrl+C to exit monitoring" -ForegroundColor Cyan
        Write-Host ""
        pio device monitor
    }
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "        BUILD SCRIPT COMPLETE       " -ForegroundColor Cyan  
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Green
Write-Host "1. Insert SIM card with active data plan"
Write-Host "2. Configure your cloud API endpoints"
Write-Host "3. Test basic functionality with serial monitor"
Write-Host "4. Verify 4G connectivity and API responses"
Write-Host ""
Write-Host "For troubleshooting, see README.md" -ForegroundColor Yellow
Write-Host ""
