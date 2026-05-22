/*
* Sakarya Üniversitesi - Bilgisayar Mühendisliği Bölümü
* Sistem Programlama Dersi - 2025-2026 Bahar Dönemi
* Tarsau Projesi Kural (Makefile) Dosyası
* Fatih Kaya - G231210072
*/

#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>     /* snprintf — sadece string biçimlendirme, dosya I/O değil */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * sau_build_archive — ".sau" formatındaki arşiv dosyasını oluşturan fonksiyon
 *
 * İşleyiş:
 *   1. Tüm giriş dosyaları doğrulanır ve meta bilgileri toplanır.
 *   2. Metadata body bir tamponda derlenir.
 *   3. HEADER_SIZE_FIELD (10 byte, soldan sıfır dolgulu ASCII sayı) yazılır.
 *   4. Metadata body yazılır.
 *   5. Ham veri bölümü: dosyalar sırayla, ayırıcı olmaksızın yazılır.
 *
 * Parametreler:
 *   argv[0]       → çıktı arşiv dosyası yolu
 *   argv[1..N-1]  → arşivlenecek giriş dosyaları
 *   argc          → toplam eleman sayısı (1 çıktı + N giriş)
 */
int sau_build_archive(int argc, char **argv) {
    char    *out_path   = argv[0];
    int      file_count = argc - 1;

    /* Dosya sayısının aşılıp aşılmadığının güvenlik kontrolü */
    if (file_count > SAU_MAX_FILES) {
        SAU_ERR("Çok fazla giriş dosyası.\n");
        exit(1);
    }

    /* Dosyaların üstveri (metadata) kayıtlarının tutulacağı string tamponu başlatılır */
    char     metadata_body[SAU_META_BUF_SIZE];
    metadata_body[0] = '\0';

    /* Dosyaların üstveri (metadata) kayıtlarının tutulacağı string tamponu başlatılır */
    SauEntry entries[SAU_MAX_FILES];
    size_t   total_size = 0;

    /* ---- Doğrulama ve metadata derleme ---- */
    for (int i = 0; i < file_count; i++) {
        char *path = argv[i + 1];

        /* ============================================================
         * Dosya adında geçersiz karakter kontrolü
         *   '|' → metadata ayırıcısı ile çakışır
         *   ',' → alan ayırıcısı ile çakışır
         * ============================================================ */
        if (strchr(path, '|') || strchr(path, ',')) {
            SAU_ERR("Geçersiz dosya adı karakteri.\n");
            exit(1);
        }

        int fd = open(path, O_RDONLY);
        if (fd == -1) { perror(path); exit(1); }

        /* ASCII doğrulama — geçersizse mesaj basıp exit(0) yapar */
        sau_validate_ascii_stream(fd, path);

        /* stat() ile boyut ve izin bilgisi */
        struct stat st;
        if (fstat(fd, &st) == -1) { perror(path); exit(1); }

        entries[i].size = (size_t)st.st_size;


        total_size += entries[i].size;
        if (total_size > SAU_MAX_TOTAL_BYTES) 
        {
            SAU_ERR("Toplam dosya boyutu sınırı aşıldı.\n");
            close(fd);
            exit(1);
        }

        /* Dosyanın yetki izinlerini kayıt edilir */
        if (sau_fetch_permissions(path, &entries[i].permissions) == -1) 
        {
            perror(path); exit(1);
        }

        strncpy(entries[i].filename, path, 255);
        entries[i].filename[255] = '\0';

        /* Metadata kaydı: |dosya_adı,izin,boyut| */
        char entry_str[320];
        snprintf(entry_str, sizeof(entry_str), "|%s,%04o,%zu",
                 entries[i].filename,
                 (unsigned int)entries[i].permissions,
                 entries[i].size);

        /* Metadata buffer taşmasına karşı boyut kontrolü  */
        if (strlen(metadata_body) + strlen(entry_str) + 2 >= SAU_META_BUF_SIZE) 
        {
            SAU_ERR("İç hata: metadata tamponu doldu.\n");
            close(fd);
            exit(1);
        }
        strcat(metadata_body, entry_str); /* Oluşturulan kayıt ana metadata string'ine eklenir */

        close(fd);
    }
    strcat(metadata_body, "|");   /* Kapanış ayırıcısı */

    /* ---- Arşiv dosyasını aç ---- */
    int dst_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) { perror("Arşiv açılamadı"); exit(1); }

    /* HEADER_SIZE_FIELD: 10 byte, soldan sıfır dolgulu ASCII sayı
     * Örnek: metadata_body 42 byte ise → "0000000042"
     * Buffer 21 byte: 10 rakam + null + güvenlik payı (derleyici uyarısına karşı) */
    char header[21];
    snprintf(header, sizeof(header), "%010zu", strlen(metadata_body));
    if (write(dst_fd, header, SAU_HEADER_SIZE_LEN) != SAU_HEADER_SIZE_LEN) 
    {
        perror("Header yazılamadı"); exit(1);
    }

    /* Metadata bloğunun kendisi dosyaya doğrudan write() döngüsü ile yazılır */
    {
        size_t  meta_len = strlen(metadata_body);
        size_t  written  = 0;
        while (written < meta_len) 
        {
            ssize_t w = write(dst_fd, metadata_body + written, meta_len - written);
            if (w <= 0) { perror("Metadata yazılamadı"); exit(1); }
            written += (size_t)w;
        }
    }

    /* ---- Dosyalar sırayla okunur ve hiçbir aracı/ayırıcı karakter olmadan arşive basılır ---- */
    for (int i = 0; i < file_count; i++) 
    {
        int fd = open(argv[i + 1], O_RDONLY);
        if (fd == -1) { perror(argv[i + 1]); exit(1); }
        if (sau_push_bytes(fd, dst_fd, entries[i].size) == -1) 
        {
            perror("Veri kopyalama hatası"); exit(1);
        }
        close(fd);
    }

    close(dst_fd);

    /* İşlemin başarılı olduğuna dair proje isterinde yer alan mesaj ekrana yazdırılır */
    SAU_OUT("Dosyalar birleştirildi.\n");
    return 0;
}