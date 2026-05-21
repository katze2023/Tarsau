#ifndef SAU_COMMON_H
#define SAU_COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>  /* mode_t, size_t için */
#include <unistd.h>     /* ssize_t için gereklidir */
#include <stddef.h>

#define SAU_MAX_FILES         32
#define SAU_MAX_TOTAL_BYTES   (200UL * 1024UL * 1024UL)
#define SAU_HEADER_SIZE_LEN   10
#define SAU_IO_BUFFER         65536
#define SAU_EXT               ".sau"

typedef struct {
    char filename[256];
    mode_t permissions;
    size_t size;
} SauEntry;

/* Modüller Arası Fonksiyon Prototipleri */
int sau_validate_ascii_stream(int fd, const char *fname);
ssize_t sau_push_bytes(int src_fd, int dst_fd, size_t n);
ssize_t sau_pull_bytes(int src_fd, int dst_fd, size_t n);
int sau_fetch_permissions(const char *path, mode_t *out);

/* Ana İş İşlemcileri */
int sau_build_archive(int argc, char **argv);
int sau_extract_archive(int argc, char **argv);

#endif /* SAU_COMMON_H */