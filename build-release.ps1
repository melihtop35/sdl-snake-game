# SnakeGame Release Build Script
# Bu script projeyi release modunda derler, gerekli dosyalarla paketler ve ZIP çıktısı alır

param(
    [string]$IconPath = "assets\sprites\snake_green_head.ico",
    [string]$OutputName = "SnakeGame",
    [switch]$KeepDist = $false
)

$ErrorActionPreference = "Stop"

# Renklendirme fonksiyonları
function Write-Step { param($msg) Write-Host ">> $msg" -ForegroundColor Cyan }
function Write-Success { param($msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-Error { param($msg) Write-Host "[HATA] $msg" -ForegroundColor Red }

# Çalışma dizini
$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

Write-Host ""
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "   SnakeGame Release Build Script" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# MSYS2/MinGW yolları
$MSYS2_ROOT = "C:\msys64\ucrt64"
$GCC = "$MSYS2_ROOT\bin\g++.exe"
$WINDRES = "$MSYS2_ROOT\bin\windres.exe"

# Dizinler
$DistDir = Join-Path $ProjectRoot "dist"
$BuildDir = Join-Path $ProjectRoot "build"

# Temizlik
Write-Step "Önceki build dosyaları temizleniyor..."
if (Test-Path $DistDir) { Remove-Item $DistDir -Recurse -Force }
if (Test-Path $BuildDir) { Remove-Item $BuildDir -Recurse -Force }
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
Write-Success "Dizinler hazırlandı"

# Icon hazırlama
Write-Step "Icon dosyası hazırlanıyor..."
$RcPath = Join-Path $BuildDir "resource.rc"
$ResPath = Join-Path $BuildDir "resource.res"

# ICO dosyasını kontrol et (artık doğrudan ICO kullanıyoruz)
$IcoFullPath = Join-Path $ProjectRoot $IconPath

if (Test-Path $IcoFullPath) {
    # ICO dosyasını build dizinine kopyala
    $IcoPath = Join-Path $BuildDir "icon.ico"
    Copy-Item $IcoFullPath $IcoPath -Force
    Write-Success "Icon bulundu: $IcoFullPath"
}
else {
    Write-Host "[UYARI] Icon dosyası bulunamadı: $IcoFullPath" -ForegroundColor Yellow
    $IcoPath = $null
}

# Resource dosyası oluştur
$ResourceArgs = @()
if ($IcoPath -and (Test-Path $IcoPath)) {
    $RcContent = @"
// Windows Resource File
#include <windows.h>

// Icon
IDI_ICON1 ICON "$($IcoPath -replace '\\', '\\\\')"

// Version Info
VS_VERSION_INFO VERSIONINFO
FILEVERSION 1,0,0,0
PRODUCTVERSION 1,0,0,0
FILEFLAGSMASK 0x3fL
FILEFLAGS 0x0L
FILEOS VOS__WINDOWS32
FILETYPE VFT_APP
FILESUBTYPE VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "MeganSS"
            VALUE "FileDescription", "Snake Game - Classic Snake Game"
            VALUE "FileVersion", "1.0.0.0"
            VALUE "InternalName", "$OutputName"
            VALUE "LegalCopyright", "Copyright (C) 2026 MeganSS. All rights reserved."
            VALUE "OriginalFilename", "$OutputName.exe"
            VALUE "ProductName", "Snake Game"
            VALUE "ProductVersion", "1.0.0.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
"@
    
    Set-Content -Path $RcPath -Value $RcContent -Encoding UTF8
    
    # Resource dosyasını derle
    Write-Step "Resource dosyası derleniyor..."
    & $WINDRES $RcPath -O coff -o $ResPath
    if ($LASTEXITCODE -eq 0 -and (Test-Path $ResPath)) {
        Write-Success "Resource dosyası derlendi"
        $ResourceArgs = @($ResPath)
    }
    else {
        Write-Host "[UYARI] Resource dosyası derlenemedi" -ForegroundColor Yellow
        $ResourceArgs = @()
    }
}

# Kaynak dosyaları
$Sources = @(
    "src\main.cpp",
    "src\Engine\Game.cpp",
    "src\Engine\GameObject.cpp",
    "src\Engine\GameManager.cpp",
    "src\Engine\AudioManager.cpp",
    "src\Game\Entity.cpp",
    "src\Game\Player.cpp",
    "src\Game\Snake.cpp",
    "src\Game\Food.cpp",
    "src\Game\FoodPool.cpp",
    "src\Input\InputManager.cpp",
    "src\UI\UIManager.cpp",
    "src\Effects\ParticleEmitter.cpp",
    "src\Effects\ParticleSystem.cpp",
    "third_party\imgui\imgui.cpp",
    "third_party\imgui\imgui_draw.cpp",
    "third_party\imgui\imgui_tables.cpp",
    "third_party\imgui\imgui_widgets.cpp",
    "third_party\imgui\imgui_impl_sdl2.cpp",
    "third_party\imgui\imgui_impl_sdlrenderer2.cpp"
)

# Tam yolları oluştur
$SourceFiles = $Sources | ForEach-Object { Join-Path $ProjectRoot $_ }

# Çıktı dosyası
$ExePath = Join-Path $DistDir "$OutputName.exe"

# Derleme
Write-Step "Proje derleniyor (Release mode)..."
Write-Host "   Kaynak dosya sayısı: $($Sources.Count)" -ForegroundColor Gray

$CompilerArgs = @(
    "-O2",                          # Optimizasyon seviyesi
    "-DNDEBUG",                     # Debug tanımlarını kaldır
    "-mwindows",                    # Windows subsystem (konsol penceresi açılmaz)
    "-static-libgcc",               # GCC runtime'ı statik bağla
    "-static-libstdc++",            # C++ runtime'ı statik bağla
    "-I", "$MSYS2_ROOT\include",
    "-I", (Join-Path $ProjectRoot "include"),
    "-I", (Join-Path $ProjectRoot "third_party")
) + $SourceFiles + $ResourceArgs + @(
    "-o", $ExePath,
    "-L", "$MSYS2_ROOT\lib",
    "-lmingw32",
    "-lSDL2main",
    "-lSDL2",
    "-lSDL2_image",
    "-lSDL2_ttf",
    "-lSDL2_mixer"
)

& $GCC @CompilerArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Derleme başarısız!"
    exit 1
}
Write-Success "Derleme tamamlandı: $ExePath"

# DLL dosyalarını kopyala
Write-Step "Tüm gerekli DLL dosyaları kopyalanıyor..."
$DLLDir = "$MSYS2_ROOT\bin"

# MSYS2/UCRT64 bin klasöründeki tüm DLL'leri kopyala
$AllDLLs = Get-ChildItem -Path $DLLDir -Filter "*.dll" -File
$CopiedDLLs = 0
foreach ($dll in $AllDLLs) {
    Copy-Item $dll.FullName -Destination $DistDir -Force
    $CopiedDLLs++
}
Write-Success "$CopiedDLLs DLL dosyası kopyalandı"

# Assets klasörünü kopyala
Write-Step "Assets dosyaları kopyalanıyor..."
$AssetsSource = Join-Path $ProjectRoot "assets"
$AssetsDest = Join-Path $DistDir "assets"
Copy-Item $AssetsSource -Destination $AssetsDest -Recurse -Force
Write-Success "Assets kopyalandı"

# imgui.ini dosyasını kopyala (varsa)
$ImguiIni = Join-Path $ProjectRoot "imgui.ini"
if (Test-Path $ImguiIni) {
    Copy-Item $ImguiIni -Destination $DistDir -Force
    Write-Success "imgui.ini kopyalandı"
}

# ZIP oluştur
Write-Step "ZIP arşivi oluşturuluyor..."
$ZipName = "$OutputName-v1.0.0-win64.zip"
$ZipPath = Join-Path $ProjectRoot $ZipName

if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}

Compress-Archive -Path "$DistDir\*" -DestinationPath $ZipPath -CompressionLevel Optimal
Write-Success "ZIP oluşturuldu: $ZipPath"

# Dosya boyutlarını göster
$ExeSize = (Get-Item $ExePath).Length / 1MB
$ZipSize = (Get-Item $ZipPath).Length / 1MB
Write-Host ""
Write-Host "Dosya Boyutları:" -ForegroundColor Magenta
Write-Host "   EXE: $([math]::Round($ExeSize, 2)) MB"
Write-Host "   ZIP: $([math]::Round($ZipSize, 2)) MB"

# Temizlik
if (-not $KeepDist) {
    Write-Step "Geçici dosyalar temizleniyor..."
    Remove-Item $DistDir -Recurse -Force
    Remove-Item $BuildDir -Recurse -Force
    Write-Success "dist ve build klasörleri kaldırıldı"
}
else {
    Write-Host "[INFO] -KeepDist parametresi ile dist klasörü korundu" -ForegroundColor Gray
    # Sadece build klasörünü temizle
    Remove-Item $BuildDir -Recurse -Force
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   Build Tamamlandı!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Çıktı dosyası: $ZipPath" -ForegroundColor White
Write-Host ""
