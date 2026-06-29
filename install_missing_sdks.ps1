# ==============================================================================
# VideoStudio - Automated SDK & Dependency Installer
# Automates the setup of missing external SDKs, AI runtimes, and GPU libraries.
# ==============================================================================

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ThirdPartyDir = Join-Path $ScriptDir "third_party"
$TempDir = Join-Path $ScriptDir "sdk_temp"

if (-not (Test-Path $ThirdPartyDir)) { New-Item -ItemType Directory -Path $ThirdPartyDir -Force | Out-Null }
if (-not (Test-Path $TempDir)) { New-Item -ItemType Directory -Path $TempDir -Force | Out-Null }

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  Starting VideoStudio Missing SDK & Tool Setup" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Cyan

# 1. OpenCV Prebuilt Windows Binaries
$OpenCvDir = Join-Path $ThirdPartyDir "opencv"
if (-not (Test-Path $OpenCvDir)) {
    Write-Host "`n[1/5] Downloading OpenCV 4.8.0 Windows SDK..." -ForegroundColor Yellow
    $OpenCvUrl = "https://github.com/opencv/opencv/releases/download/4.8.0/opencv-4.8.0-windows.exe"
    $OpenCvInstaller = Join-Path $TempDir "opencv_installer.exe"
    
    if (-not (Test-Path $OpenCvInstaller)) {
        Invoke-WebRequest -Uri $OpenCvUrl -OutFile $OpenCvInstaller
    }
    Write-Host "Extracting OpenCV SDK to third_party/opencv..." -ForegroundColor Yellow
    # Extracting self-extracting archive silently
    Start-Process -FilePath $OpenCvInstaller -ArgumentList "-o`"$ThirdPartyDir`" -y" -Wait -NoNewWindow
    Write-Host "OpenCV successfully installed!" -ForegroundColor Green
} else {
    Write-Host "`n[1/5] OpenCV SDK is already present in third_party/opencv." -ForegroundColor Green
}

# 2. 3D LUT Color Grading Packs
$LutDir = Join-Path $ScriptDir "assets\luts"
if (-not (Test-Path $LutDir)) {
    Write-Host "`n[2/5] Creating cinematic 3D LUT directory structure..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $LutDir -Force | Out-Null
    Write-Host "Generating sample cinematic .cube LUT files..." -ForegroundColor Yellow
    
    $SampleLut = Join-Path $LutDir "CapCut_TealAndOrange.cube"
    @"
TITLE "CapCut Teal and Orange 3D LUT"
LUT_3D_SIZE 16
0.012 0.015 0.022
0.045 0.051 0.068
0.112 0.125 0.145
0.245 0.265 0.312
0.512 0.542 0.621
0.821 0.854 0.912
1.000 1.000 1.000
"@ | Set-Content -Path $SampleLut
    Write-Host "Sample cinematic LUT pack installed at assets/luts!" -ForegroundColor Green
} else {
    Write-Host "`n[2/5] Cinematic 3D LUT directory is already present." -ForegroundColor Green
}

# 3. NVIDIA CUDA Toolkit & cuDNN Check
Write-Host "`n[3/5] Checking for NVIDIA CUDA Toolkit runtime..." -ForegroundColor Yellow
if (Get-Command "nvcc.exe" -ErrorAction SilentlyContinue) {
    Write-Host "NVIDIA CUDA Toolkit detected in system PATH!" -ForegroundColor Green
} else {
    Write-Host "NVIDIA CUDA Toolkit not found in PATH." -ForegroundColor DarkYellow
    Write-Host "-> To unlock 10x-20x ONNX AI acceleration, download CUDA 12.x from: https://developer.nvidia.com/cuda-downloads" -ForegroundColor Cyan
}

# 4. Qt Shader Tools Check
Write-Host "`n[4/5] Checking Qt Shader Tools (qsb.exe)..." -ForegroundColor Yellow
if (Get-Command "qsb.exe" -ErrorAction SilentlyContinue) {
    Write-Host "Qt Shader Tools bundle compiler (qsb) detected!" -ForegroundColor Green
} else {
    Write-Host "Qt Shader Tools (qsb.exe) not found in system PATH." -ForegroundColor DarkYellow
    Write-Host "-> To compile custom CapCut XShader equivalents, ensure 'Qt Shader Tools' is checked in your Qt Maintenance Tool." -ForegroundColor Cyan
}

# 5. Clean up temp directory
Write-Host "`n[5/5] Cleaning up temporary download directory..." -ForegroundColor Yellow
if (Test-Path $TempDir) { Remove-Item -Path $TempDir -Recurse -Force | Out-Null }

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "  Setup Script Completed Successfully!" -ForegroundColor Green
Write-Host "  CMake FetchContent will download rlottie, lua, sol2, and"
Write-Host "  vst3sdk automatically on your next CMake build."
Write-Host "============================================================" -ForegroundColor Cyan
