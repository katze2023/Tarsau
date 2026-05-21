#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * MİMARİ MANTIK:
 * Arşivleme sürecinde metadata bölümü ve veri bölümünü sıralı yazar.
 * Metadata önce bir buffer'da derlenir, ardından HEADER_SIZE_FIELD (10 byte)
 * yazılır. Metadata parse hatalarını engellemek için '|' ve ',' karakter 
 * kontrolü sıkı bir şekilde yapılır.
 */
int sau_build_archive(int argc, char **argv) {
    char *out_path = argv[0];
    int dst_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) { perror("Arşiv açılamadı"); exit(1); }

    char metadata_body[8192] = ""; // Metadata body için yeterli alan
    SauEntry entries[SAU_MAX_FILES];
    int file_count = argc - 1;

    for (int i = 0; i < file_count; i++) {
        char *path = argv[i + 1];
        
        // Karakter kontrolü
        if (strchr(path, '|') || strchr(path, ',')) {
            write(STDERR_FILENO, "Geçersiz dosya adı karakteri.\n", 29);
            exit(1);
        }

        int fd = open(path, O_RDONLY);
        if (fd == -1) { perror(path); exit(1); }
        
        // ASCII doğrulama (Adım 2 fonksiyonu)
        sau_validate_ascii_stream(fd, path);
        
        // İzinleri ve boyutu al
        struct stat st;
        fstat(fd, &st);
        entries[i].size = st.st_size;
        sau_fetch_permissions(path, &entries[i].permissions);
        strncpy(entries[i].filename, path, 255);
        
        // Metadata Body inşası
        char entry_str[512];
        snprintf(entry_str, sizeof(entry_str), "|%s,%04o,%zu", 
                 entries[i].filename, (unsigned int)entries[i].permissions, entries[i].size);
        strcat(metadata_body, entry_str);
        
        close(fd);
    }
    strcat(metadata_body, "|");

    // HEADER_SIZE_FIELD yazımı (10 byte sabit)
    char header[11];
    snprintf(header, sizeof(header), "%010zu", strlen(metadata_body));
    write(dst_fd, header, 10);
    write(dst_fd, metadata_body, strlen(metadata_body));

    // Ham veri bölümü
    for (int i = 0; i < file_count; i++) {
        int fd = open(argv[i + 1], O_RDONLY);
        sau_push_bytes(fd, dst_fd, entries[i].size);
        close(fd);
    }

    close(dst_fd);
    write(STDOUT_FILENO, "Dosyalar birleştirildi.\n", 24);
    return 0;
}