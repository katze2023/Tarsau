#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/*
 * MİMARİ MANTIK:
 * 1. HEADER_SIZE_FIELD okunur ve metadata boyutu belirlenir.
 * 2. Metadata body okunur ve '|' karakteri ile parse edilir.
 * 3. Her bir dosya için 'mkdir' (gerekirse) ve 'open' işlemleri yapılır.
 * 4. Dosya verisi 'sau_pull_bytes' ile aktarılır.
 * 5. 'chmod' ile orijinal izinler atanır.
 */
int sau_extract_archive(int argc, char **argv) {
    char *arch_path = argv[0];
    char *target_dir = argv[1];

    // Dizin oluşturma
    mkdir(target_dir, 0755);

    int fd = open(arch_path, O_RDONLY);
    if (fd == -1) { perror("Arşiv açılamadı"); exit(1); }

    char header[11] = {0};
    if (read(fd, header, 10) != 10) {
        write(STDERR_FILENO, "Arşiv dosyası uygunsuz veya bozuk!\n", 35);
        exit(1);
    }
    size_t meta_size = atoi(header);

    char *meta_body = malloc(meta_size + 1);
    read(fd, meta_body, meta_size);
    meta_body[meta_size] = '\0';

    char *token = strtok(meta_body, "|");
    int files_extracted = 0;

    while (token != NULL) {
        char name[256], p_str[5], s_str[20];
        // Metadata parse: dosya_adı,izinler,boyut
        sscanf(token, "%255[^,],%4s,%s", name, p_str, s_str);
        
        mode_t mode = (mode_t)strtol(p_str, NULL, 8);
        size_t size = (size_t)atoll(s_str);

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", target_dir, name);

        int out_fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1) { perror("Dosya yazılamadı"); exit(1); }

        sau_pull_bytes(fd, out_fd, size);
        close(out_fd);

        if (chmod(full_path, mode) == -1) {
            // Kritik hata değil, uyarı ver ve devam et
        }

        files_extracted++;
        token = strtok(NULL, "|");
    }

    free(meta_body);
    close(fd);

    printf("%s dizininde %d dosyalar açıldı.\n", target_dir, files_extracted);
    return 0;
}