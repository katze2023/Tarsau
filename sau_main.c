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
    SAU_ERR(msg);
    exit(1);
}

/*
 * MİMARİ MANTIK VE POSIX SEMANTİĞİ:
 * Programın giriş noktasıdır (Entry point). Yüksek seviye akış kontrolünü ve
 * komut satırı argümanlarının ayrıştırılmasını sağlar.
 *
 *  -b opsiyonunda:
 *    - Giriş dosyası sayısı SAU_MAX_FILES (32) ile sınırlanır.
 *    - Argüman dizisi [çıktı_dosyası, girdi1, girdi2...] formatına dönüştürülür.
 *
 *  -a opsiyonunda:
 *    - Uzantı doğrulaması (SAU_EXT = ".sau") yapılır.
 *    - Dizi [arşiv_dosyası, hedef_dizin] formatına dönüştürülür.
 *    - Yalnızca 2 ek parametre kabul edilir; fazlası hata verir.
 *
 * Bu sayede arşivleme ve çıkarma modülleri sadece iş mantığına odaklanır.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        sau_die("Kullanım: tarsau -b dosya1... -o cikti.sau VEYA tarsau -a arsiv.sau [hedef_dizin]\n");
    }

    /* ================================================================
     * -b : Arşiv oluşturma modu
     * ================================================================ */
    if (strcmp(argv[1], "-b") == 0) {
        char *out_file = "a.sau";
        char *in_files[SAU_MAX_FILES + 1];  /* [0] → çıktı adı, [1..N] → girdiler */
        int   in_count = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    out_file = argv[++i];
                } else {
                    sau_die("Çıktı dosyası adı belirtilmedi.\n");
                }
            } else {
                if (in_count >= SAU_MAX_FILES) {
                    /* Limit kontrolü: main'de yapılıyor; archive.c'de de aynı limit vardır. */
                    sau_die("Çok fazla giriş dosyası.\n");
                }
                in_files[in_count + 1] = argv[i];   /* [0] boş bırakılıyor, sonra çıktı adı konacak */
                in_count++;
            }
        }

        if (in_count == 0) {
            sau_die("Arşivlenecek giriş dosyası bulunamadı.\n");
        }

        in_files[0] = out_file;
        return sau_build_archive(in_count + 1, in_files);
    }

    /* ================================================================
     * -a : Arşiv çıkarma modu
     * ================================================================ */
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }

        /* Uzantı doğrulaması: son 4 karakter ".sau" olmalı */
        size_t len = strlen(argv[2]);
        if (len < 4 || strcmp(argv[2] + len - 4, SAU_EXT) != 0) {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }

        /* -a'dan sonra en fazla 2 parametre: arşiv + hedef dizin */
        if (argc > 4) {
            sau_die("Fazla parametre girildi.\n");
        }

        char *ext_args[2];
        ext_args[0] = argv[2];                          /* arşiv dosyası */
        ext_args[1] = (argc >= 4) ? argv[3] : ".";     /* hedef dizin (yoksa geçerli dizin) */

        return sau_extract_archive(2, ext_args);
    }

    sau_die("Geçersiz parametre. Lütfen -b veya -a kullanın.\n");
    return 1;
}