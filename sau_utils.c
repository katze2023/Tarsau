#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

/* 
 * MİMARİ MANTIK: Dosya içeriğini byte-seviyesinde tarayarak, sistem programlama 
 * kuralları gereği 0x00-0x1F arası kontrol karakterlerinden yalnızca \n, \r, \t 
 * izin verilir. Diğer kontrol karakterleri dosyanın arşiv formatını bozacağı 
 * için tespit edilir. Read çağrısı ile tamponlu okuma yapılır.
 */
int sau_validate_ascii_stream(int fd, const char *fname) {
    unsigned char buf[SAU_IO_BUFFER];
    ssize_t bytes_read;

    if (lseek(fd, 0, SEEK_SET) == -1) return -1;

    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            unsigned char b = buf[i];
            if (b > 0x7F || (b < 0x20 && b != '\n' && b != '\r' && b != '\t')) {
                fprintf(stderr, "%s giriş dosyasının formatı uyumsuzdur!\n", fname);
                exit(0);
            }
        }
    }
    return (bytes_read == 0) ? 0 : -1;
}

/* 
 * MİMARİ MANTIK: Veri bloklarını src_fd'den dst_fd'ye aktarır. 
 * Arşivleme sürecinde metadata'dan sonra gelen ham veri bölümünün 
 * yazılması için kullanılır. POSIX 'read' ve 'write' çağrılarının 
 * dönüş değerleri kontrol edilerek kısmi yazmalar (partial writes) yönetilir.
 */
ssize_t sau_push_bytes(int src_fd, int dst_fd, size_t n) {
    unsigned char buf[SAU_IO_BUFFER];
    size_t total_written = 0;
    while (total_written < n) {
        size_t to_read = (n - total_written > SAU_IO_BUFFER) ? SAU_IO_BUFFER : (n - total_written);
        ssize_t r = read(src_fd, buf, to_read);
        if (r <= 0) return -1;
        if (write(dst_fd, buf, r) != r) return -1;
        total_written += r;
    }
    return (ssize_t)total_written;
}

/* 
 * MİMARİ MANTIK: Arşivden çıkarılan dosyaların ham verisini hedef dosyaya 
 * yazmak için kullanılır. 'lseek' ile işaretçi yönetimi yapılmadan, 
 * ardışık akış modeli ile veriyi parçalar halinde hedefe aktarır.
 */
ssize_t sau_pull_bytes(int src_fd, int dst_fd, size_t n) {
    return sau_push_bytes(src_fd, dst_fd, n);
}

/* 
 * MİMARİ MANTIK: 'stat' sistem çağrısı ile dosyanın metadata bilgilerine 
 * erişir. st_mode alanından maskeleme yaparak sadece dosya izin bitlerini 
 * ayıklar. Bu, arşiv dosyasının metadata bölümünde dosya izinlerini 
 * saklamamızı sağlar.
 */
int sau_fetch_permissions(const char *path, mode_t *out) {
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    *out = st.st_mode & 07777;
    return 0;
}