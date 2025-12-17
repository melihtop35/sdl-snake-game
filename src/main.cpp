#include "../include/Engine/Game.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    // Kullanılmayan parametre uyarısını engelle
    (void)argc;
    (void)argv;

#ifdef _WIN32
    // Windows'ta DPI awareness'ı aktif et - gerçek ekran çözünürlüğünü almak için
    SetProcessDPIAware();
#endif

    // Oyun örneği oluştur
    Game game;

    // Oyunu başlat (tam ekran olarak açılacak)
    if (!game.Initialize("Yilan Oyunu - SDL2"))
    {
        std::cerr << "Oyun baslatilamadi!" << std::endl;
        return -1;
    }

    std::cout << "Oyun baslatildi!" << std::endl;
    std::cout << "Kontroller:" << std::endl;
    std::cout << "  - Yon tuslari veya WASD: Hareket" << std::endl;
    std::cout << "  - ESC: Duraklat / Geri" << std::endl;
    std::cout << "  - Fare: Menu navigasyonu" << std::endl;

    // Ana oyun döngüsünü çalıştır
    game.Run();

    // Temizleme (destructor da yapar ama açık olsun)
    game.Cleanup();

    std::cout << "Oyun sonlandi. Gule gule!" << std::endl;

    return 0;
}
