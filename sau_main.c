#include "sau_common.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
 * MİMARİ MANTIK VE POSIX SEMANTİĞİ:
 * Sistem çağrıları seviyesinde standart hata çıktısına (stderr) güvenli 
 * yazma işlemi gerçekleştiren yardımcı fonksiyondur. Standart C kütüphanesi 
 * tamponlamasından (buffering) kaçınmak adına doğrudan 'write' POSIX çağrısı 
 * kullanılmıştır. Hata fırlatıp programı sonlandırır.
 */
static void sau_die(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    exit(1);
}

/*
 * MİMARİ MANTIK VE POSIX SEMANTİĞİ:
 * Programın giriş noktasıdır (Entry point). Yüksek seviye akış kontrolünü ve 
 * komut satırı argümanlarının ayrıştırılmasını sağlar. 
 * -b opsiyonunda giriş dosyalarını sayar (SAU_MAX_FILES kontrolü) ve 
 *    argüman dizisini [çıktı_dosyası, girdi1, girdi2...] formatına dönüştürür.
 * -a opsiyonunda uzantı doğrulaması (SAU_EXT) yapar ve diziyi 
 *    [arşiv_dosyası, hedef_dizin] formatına dönüştürür.
 * Bu sayede arşivleme ve çıkarma modülleri sadece iş mantığına odaklanır.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        sau_die("Kullanım: tarsau -b dosya1... -o cikti.sau VEYA tarsau -a arsiv.sau [hedef_dizin]\n");
    }

    if (strcmp(argv[1], "-b") == 0) {
        char *out_file = "a.sau";
        char *in_files[SAU_MAX_FILES + 1];
        int in_count = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    out_file = argv[i + 1];
                    i++; 
                } else {
                    sau_die("Çıktı dosyası adı belirtilmedi.\n");
                }
            } else {
                if (in_count >= SAU_MAX_FILES) {
                    sau_die("Çok fazla giriş dosyası.\n");
                }
                in_files[in_count + 1] = argv[i];
                in_count++;
            }
        }

        if (in_count == 0) {
            sau_die("Arşivlenecek giriş dosyası bulunamadı.\n");
        }

        in_files[0] = out_file;
        return sau_build_archive(in_count + 1, in_files);
    }
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }
        
        size_t len = strlen(argv[2]);
        if (len < 4 || strcmp(argv[2] + len - 4, SAU_EXT) != 0) {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }

        char *ext_args[2];
        ext_args[0] = argv[2]; 
        ext_args[1] = (argc >= 4) ? argv[3] : "."; 

        if (argc > 4) {
            sau_die("Fazla parametre girildi.\n");
        }

        return sau_extract_archive(2, ext_args);
    }

    sau_die("Geçersiz parametre. Lütfen -b veya -a kullanın.\n");
    return 1;
}