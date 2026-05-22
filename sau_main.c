/*
* Sakarya Üniversitesi - Bilgisayar Mühendisliği Bölümü
* Sistem Programlama Dersi - 2025-2026 Bahar Dönemi
* Tarsau Projesi Kural (Makefile) Dosyası
* Fatih Kaya - G231210072
*/

#include "sau_common.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
 * sau_die — Kritik hatalarda programı mesaj vererek sonlandırır.
 * * Güvenli POSIX write metodunu (SAU_ERR) kullanarak hata mesajını basar
 * ve işletim sistemine başarısızlık (1) kodunu döndürerek çıkar.
 */
static void sau_die(const char *msg) 
{
    SAU_ERR(msg);
    exit(1);
}

/*
 * main — Programın giriş noktası ve komut satırı argümanı (CLI) işleyicisi
 *
 * İşleyiş:
 * 1. Kullanıcıdan gelen parametreleri kontrol eder.
 * 2. Eğer '-b' parametresi verilmişse: Arşiv oluşturma moduna geçer.
 * - Dosya sayısının sınırını (max 32) kontrol eder.
 * - '-o' ile bir çıktı dosyası verilmişse onu, verilmemişse "a.sau" adını alır.
 * 3. Eğer '-a' parametresi verilmişse: Arşiv açma moduna geçer.
 * - Dosyanın uzantısının ".sau" olup olmadığını doğrular.
 * - İsteğe bağlı hedef dizin parametresini ayrıştırır (yoksa mevcut dizin '.' alınır).
 */
int main(int argc, char **argv) 
{
    /* En az işlem bayrağı (-a veya -b) ve bir argüman girilmesi zorunludur */
    if (argc < 2) 
    {
        sau_die("Kullanım: tarsau -b dosya1... -o cikti.sau VEYA tarsau -a arsiv.sau [hedef_dizin]\n");
    }

    /* ================================================================
     * -b : Arşiv oluşturma modu
     * ================================================================ */
    if (strcmp(argv[1], "-b") == 0) 
    {
        char *out_file = "a.sau";            /* Varsayılan çıktı dosyası */
        char *in_files[SAU_MAX_FILES + 1];  /* [0] → çıktı adı, [1..N] → girdiler */
        int   in_count = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) 
            {
                if (i + 1 < argc) 
                {
                    out_file = argv[++i];    /* -o'dan sonraki argümanı çıktı dosyası olarak al */
                } else 
                {
                    sau_die("Çıktı dosyası adı belirtilmedi.\n");
                }
            } else {
                if (in_count >= SAU_MAX_FILES) 
                {
                    /* Limit kontrolü (ana kontrol main klasöründe) */
                    sau_die("Çok fazla giriş dosyası.\n");
                }
                in_files[in_count + 1] = argv[i];   /* Girdi dosyasını diziye kaydet */
                in_count++;
            }
        }

        /* Hiçbir dosya belirtilmediyse hata ver */
        if (in_count == 0) 
        {
            sau_die("Arşivlenecek giriş dosyası bulunamadı.\n");
        }

        in_files[0] = out_file;
        return sau_build_archive(in_count + 1, in_files);
    }

    /* ================================================================
     * -a : Arşiv çıkarma modu
     * ================================================================ */
    else if (strcmp(argv[1], "-a") == 0) 
    {
        if (argc < 3) {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }

        /* Uzantı doğrulaması: son 4 karakter ".sau" olmalı */
        size_t len = strlen(argv[2]);
        if (len < 4 || strcmp(argv[2] + len - 4, SAU_EXT) != 0) 
        {
            sau_die("Arşiv dosyası uygunsuz veya bozuk!\n");
        }

        /* -a'dan sonra en fazla 2 parametre: arşiv + hedef dizin */
        if (argc > 4) {
            sau_die("Fazla parametre girildi.\n");
        }

        char *ext_args[2];
        ext_args[0] = argv[2];                          /* arşiv dosyası */
        ext_args[1] = (argc >= 4) ? argv[3] : ".";     /* hedef dizin (yoksa geçerli dizin kullanılacak) */

        return sau_extract_archive(2, ext_args);
    }

    /* Parametre -a veya -b değilse geçersiz işlem hatası ver */
    sau_die("Geçersiz parametre. Lütfen -b veya -a kullanın.\n");
    return 1;
}