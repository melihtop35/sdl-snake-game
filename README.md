# SDL Snake Game

SDL2 ile C++ kullanılarak geliştirilmiş klasik yılan oyunu. Oyun tam ekran açılır; menülerde fareyle gezinebilir, yılanı klavye ile kontrol edebilirsiniz. Proje; oyun motoru, giriş, arayüz, ses ve parçacık efektleri için ayrı modüller içerir.

## Özellikler

- Klasik yılan oynanışı
- Yön tuşları veya **WASD** ile hareket
- Menü navigasyonu için fare desteği
- **ESC** ile duraklatma / geri dönme
- SDL2 tabanlı grafik, ses ve kaynak yönetimi
- Oyuna eşlik eden sprite, yazı tipi ve ses varlıkları

## Gereksinimler

- Windows
- MSYS2 **UCRT64** ortamı
- UCRT64 GCC (`g++`) ve `windres`
- SDL2 ile SDL2_image, SDL2_ttf ve SDL2_mixer geliştirme kütüphaneleri
- PowerShell

`build-release.ps1` derleyici ve kütüphaneleri varsayılan olarak `C:\msys64\ucrt64` altında arar. MSYS2 farklı bir konuma kuruluysa betikteki `$MSYS2_ROOT` değerini güncelleyin.

## Derleme ve çalıştırma

Depoyu klonlayın:

```powershell
git clone https://github.com/melihtop35/sdl-snake-game.git
cd sdl-snake-game
```

Release sürümünü oluşturun:

```powershell
.\build-release.ps1
```

Betik, `dist` ve `build` klasörlerini hazırlar, uygulamayı derler ve gerekli dosyaları içeren `SnakeGame-v1.0.0-win64.zip` arşivini oluşturur. Varsayılan olarak geçici `dist` ve `build` klasörleri tamamlanınca temizlenir. `dist` klasörünü saklamak için:

```powershell
.\build-release.ps1 -KeepDist
```

Derleme tamamlandıktan sonra oluşturulan ZIP arşivini açın ve `SnakeGame.exe` dosyasını çalıştırın. Oyun varlıklarının çalışması için `assets` klasörünü uygulamayla birlikte tutun.

Betikteki diğer parametreler:

```powershell
.\build-release.ps1 -OutputName MySnakeGame -IconPath "assets\sprites\snake_green_head.ico"
```

## Kontroller

| Tuş / giriş | İşlev |
| --- | --- |
| Yön tuşları veya **WASD** | Yılanı hareket ettirir |
| **ESC** | Duraklatır veya geri döner |
| Fare | Menüde gezinir |

## Proje yapısı

```text
assets/       Görseller, yazı tipleri ve sesler
include/      Başlık dosyaları
src/          C++ kaynak kodu
  Effects/    Parçacık efektleri
  Engine/     Oyun motoru ve yöneticiler
  Game/       Oyuncu, yılan ve yiyecek sınıfları
  Input/      Girdi yönetimi
  UI/         Arayüz yönetimi
third_party/  Üçüncü taraf kodlar (Dear ImGui)
build-release.ps1  Windows release derleme ve paketleme betiği
```

## Üçüncü taraf yazılımlar

Arayüz için `third_party/imgui` altında Dear ImGui ve SDL2 renderer/platform entegrasyonları kullanılır. Üçüncü taraf bileşenlerin lisans ve bildirimleri için ilgili kaynak dosyaları ve lisans belgelerine bakın.

## Lisans

Bu depoda bir lisans dosyası bulunmadığından, kullanım ve dağıtım koşulları belirtilmemiştir. Yeniden dağıtımdan önce depo sahibinden izin alın.
