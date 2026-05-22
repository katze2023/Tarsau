/*
* Sakarya Üniversitesi - Bilgisayar Mühendisliği Bölümü
* Sistem Programlama Dersi - 2025-2026 Bahar Dönemi
* Tarsau Projesi Kural (Makefile) Dosyası
* Fatih Kaya - G231210072
*/

#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>     /* snprintf — sadece string biçimlendirme */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/*
 * sau_extract_archive — .sau uzantılı arşivleri dışarı çıkartan fonksiyon
 *
 * İşleyiş:
 *   1. HEADER_SIZE_FIELD (10 byte) okunur → metadata boyutu belirlenir.
 *   2. Metadata body heap'e okunur ve '|' ile token'lara ayrılır.
 *   3. Her token "dosya_adı,izin,boyut" biçiminde POSIX-uyumlu manuel
 *      parse ile ayrıştırılır (sscanf/fscanf gibi stdio I/O fonksiyon YOK).
 *   4. Hedef dizin yoksa mkdir() ile oluşturulur.
 *   5. Her dosya open()+sau_pull_bytes() ile çıkarılır.
 *   6. chmod() ile orijinal izinler geri atanır.
 *   7. Çıktı mesajı write() ile yazılır.
 *
 * Parametreler:
 *   argv[0] → arşiv dosyası yolu
 *   argv[1] → hedef dizin yolu
 *   argc    → 2 (main.c tarafından her zaman 2 gönderilir)
 */
int sau_extract_archive(int argc, char **argv) {
    (void)argc;   /* argc şu an kullanılmıyor, derleyici uyarısını önle */

    char *arch_path  = argv[0];
    char *target_dir = argv[1];

    /* Hedef dizin yoksa oluştur */
    mkdir(target_dir, 0755);

    /* ---- Arşiv dosyasını aç ---- */
    int fd = open(arch_path, O_RDONLY);
    if (fd == -1) {
        SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
        exit(1);
    }

    /* Arşivin başındaki 10 byte'lık (metadata uzunluğunu tutan) kısmı okunur */
    char header[SAU_HEADER_SIZE_LEN + 1];
    header[SAU_HEADER_SIZE_LEN] = '\0';

    if (read(fd, header, SAU_HEADER_SIZE_LEN) != SAU_HEADER_SIZE_LEN) {
        SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
        close(fd);
        exit(1);
    }

    /* Header'ın tamamının rakam olup olmadığını doğrulama */
    for (int i = 0; i < SAU_HEADER_SIZE_LEN; i++) {
        if (header[i] < '0' || header[i] > '9') {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            close(fd);
            exit(1);
        }
    }

    char  *end_ptr;
    size_t meta_size = (size_t)strtoul(header, &end_ptr, 10);

    if (meta_size == 0) {
        SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
        close(fd);
        exit(1);
    }

    /* Metadata'nın bellekte tutulması için yeterli boyutta alan (heap üzerinde) ayırıyoruz */
    char *meta_body = malloc(meta_size + 1);
    if (meta_body == NULL) {
        SAU_ERR("Bellek tahsis hatası.\n");
        close(fd);
        exit(1);
    }

    /* Sistemin büyük okumalarda yapabileceği kısmı okumalara (partial read) karşı döngüyle veriyi çekiyoruz */
    size_t total_read = 0;
    while (total_read < meta_size) {
        ssize_t r = read(fd, meta_body + total_read, meta_size - total_read);
        if (r <= 0) {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(meta_body);
            close(fd);
            exit(1);
        }
        total_read += (size_t)r;
    }
    meta_body[meta_size] = '\0';

    /* ---- Token ayrıştırma: |dosya1,izin,boyut|dosya2,...| ---- */
    char  extracted_names[SAU_MAX_FILES][256];
    int   files_extracted = 0;

    char *token = strtok(meta_body, "|");

    while (token != NULL && files_extracted < SAU_MAX_FILES) {
        /*
         * Elde edilen her parçanın formatı: dosya_adı,izin,boyut
         * "," kullanılarak manuel olarak ayrıştırılır (sscanf/fscanf YOK)
         */
        char *comma1 = strchr(token, ',');
        if (comma1 == NULL) 
        {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(meta_body);
            close(fd);
            exit(1);
        }

        char *comma2 = strchr(comma1 + 1, ',');
        if (comma2 == NULL) 
        {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(meta_body);
            close(fd);
            exit(1);
        }

        /* Dosya adı */
        char name[256] = {0};
        size_t name_len = (size_t)(comma1 - token);
        if (name_len == 0 || name_len >= 256) 
        {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(meta_body);
            close(fd);
            exit(1);
        }
        strncpy(name, token, name_len);
        name[name_len] = '\0';

        /* İzin (oktal string, örn. "0755") */
        char p_str[8] = {0};
        size_t p_len = (size_t)(comma2 - (comma1 + 1));
        if (p_len == 0 || p_len >= 8) 
        {
            SAU_ERR("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(meta_body);
            close(fd);
            exit(1);
        }
        strncpy(p_str, comma1 + 1, p_len);
        p_str[p_len] = '\0';

        /* Boyut */
        char *size_str = comma2 + 1;

        mode_t perm = (mode_t)strtoul(p_str,  NULL, 8);
        size_t size = (size_t)strtoull(size_str, NULL, 10);

        /* ---- Hedef dosya yolunu oluştur ---- */
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", target_dir, name);

        /* ---- Dosyayı yaz ---- */
        int out_fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1) {
            perror("Dosya yazılamadı");
            free(meta_body);
            close(fd);
            exit(1);
        }

        if (sau_pull_bytes(fd, out_fd, size) == -1) {
            perror("Veri kopyalama hatası");
            close(out_fd);
            free(meta_body);
            close(fd);
            exit(1);
        }
        close(out_fd);

        /* İzinleri geri yükle — hata kritik değil, belge "sorunsuz çıkılmalı" diyor */
        (void)chmod(full_path, perm);

        /* Çıktı mesajı için dosya adını sakla */
        strncpy(extracted_names[files_extracted], name, 255);
        extracted_names[files_extracted][255] = '\0';
        files_extracted++;

        token = strtok(NULL, "|");
    }

    free(meta_body);
    close(fd);

    /* 
    * Çıktı mesajını proje belgesinde istenen formata (d1 dizininde t1, t2 ve t3.txt dosyaları açıldı)
    * uygun olarak, stdio.h tamponları (printf vb) kullanmadan, ardışık write() çağrılarıyla hazırlır ve yazdırır
    */
    SAU_OUT(target_dir);
    SAU_OUT(" dizininde ");
    for (int i = 0; i < files_extracted; i++) 
    {
        if (i > 0) 
        {
            /* Son dosyadan önce ' ve ' ekle, diğerleri arasına virgül ekle */
            if (i == files_extracted - 1) 
            {
                SAU_OUT(" ve ");
            } else {
                SAU_OUT(", ");
            }
        }
        SAU_OUT(extracted_names[i]);
    }
    SAU_OUT(" dosyaları açıldı.\n");

    return 0;
}