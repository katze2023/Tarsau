#ifndef TARSAU_H
#define TARSAU_H

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>  /* mode_t, size_t */
#include <unistd.h>     /* ssize_t, write, STDERR_FILENO, STDOUT_FILENO */
#include <stddef.h>
#include <string.h>     /* strlen — yalnızca write() uzunluğu hesabında kullanılır */

/* ===== Sistem Limitleri ===== */
#define SAU_MAX_FILES         32
#define SAU_MAX_TOTAL_BYTES   (200UL * 1024UL * 1024UL)   /* 200 MB */
#define SAU_HEADER_SIZE_LEN   10
#define SAU_IO_BUFFER         65536
#define SAU_EXT               ".sau"

/*
 * Metadata buffer boyutu:
 *   Her kayıt max: |<255 char dosya adı>,<4 char izin>,<20 char boyut>|
 *   = 1 + 255 + 1 + 4 + 1 + 20 = 282 byte
 *   32 dosya * 282 + 1 (kapanış |) = 9025 byte
 *   Güvenlik payıyla 10240 seçildi.
 */
#define SAU_META_BUF_SIZE     10240

/* ===== Dosya Girdi Yapısı ===== */
typedef struct {
    char   filename[256];
    mode_t permissions;
    size_t size;
} SauEntry;

/* ===================================================================
 * sau_write_fd — POSIX write() için güvenli yardımcı.
 *
 * Neden gerekli?
 *   write(fd, "Türkçe string\n", 14) gibi hardcoded byte sayıları
 *   UTF-8 kodlamasında ('ş', 'ı', 'ç' vb. her biri 2 byte) yanlış
 *   uzunluk verir ve mesajın son kısmı kesilir. strlen() kullanımı
 *   bu hatayı ortadan kaldırır.
 * =================================================================== */
static inline void sau_write_fd(int fd, const char *s) {
    size_t len = strlen(s);
    /* Kısmi yazma (partial write) sonsuz döngüye girmemek için tek deneme yeterli */
    (void)write(fd, s, len);
}

/* Kolaylık makroları */
#define SAU_ERR(msg)  sau_write_fd(STDERR_FILENO, (msg))
#define SAU_OUT(msg)  sau_write_fd(STDOUT_FILENO, (msg))

/* ===== Modüller Arası Fonksiyon Prototipleri ===== */
int     sau_validate_ascii_stream(int fd, const char *fname);
ssize_t sau_push_bytes(int src_fd, int dst_fd, size_t n);
ssize_t sau_pull_bytes(int src_fd, int dst_fd, size_t n);
int     sau_fetch_permissions(const char *path, mode_t *out);

/* ===== Ana İş İşlemcileri ===== */
int sau_build_archive(int argc, char **argv);
int sau_extract_archive(int argc, char **argv);

#endif /* TARSAU_H */