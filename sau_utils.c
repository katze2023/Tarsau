#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

/*
 * sau_validate_ascii_stream — POSIX ASCII doğrulama
 *
 * MİMARİ MANTIK:
 *   Dosya içeriğini byte seviyesinde tarayarak metin dosyası olup olmadığını
 *   doğrular. Sistem programlama kuralları gereği:
 *     - 0x20–0x7E → yazdırılabilir ASCII karakterler (geçerli)
 *     - 0x09 (\t), 0x0A (\n), 0x0D (\r) → izin verilen kontrol karakterleri
 *     - 0x80 ve üstü → 8-bit / binary veri → geçersiz
 *     - Diğer 0x00–0x1F kontrol karakterleri → geçersiz
 *
 *   DÜZELTİLEN HATA — fprintf KULLANIMI:
 *     Orijinal kodda `fprintf(stderr, ...)` kullanılıyordu. Bu, stdio.h
 *     tamponlu (buffered) I/O'ya aittir ve proje isterine göre YASAKTIR.
 *     Yerine doğrudan POSIX write() çağrısına dayanan SAU_ERR() makrosu
 *     kullanıldı.
 *
 *   Hata durumunda exit(0): Belge "sorunsuz çıkılmalı" diyor, bu nedenle
 *   exit kodu 0'dır (program başarısız değil, kullanım hatası bildirdi).
 */
int sau_validate_ascii_stream(int fd, const char *fname) {
    unsigned char buf[SAU_IO_BUFFER];
    ssize_t       bytes_read;

    /* Dosya başına sar — validate çağrısından önce fd konumu belirsiz olabilir */
    if (lseek(fd, 0, SEEK_SET) == -1) return -1;

    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            unsigned char b = buf[i];
            /* İzin verilen karakterler: yazdırılabilir ASCII + \t \n \r */
            if (b > 0x7F || (b < 0x20 && b != '\n' && b != '\r' && b != '\t')) {
                /*
                 * HATA MESAJI FORMAT:
                 *   "<dosya_adı> giriş dosyasının formatı uyumsuzdur!"
                 *   Belgedeki format kelimesi kelimesine uygulanıyor.
                 *   write() ile doğrudan kernel'e yazılır — stdio tamponu yok.
                 */
                SAU_ERR(fname);
                SAU_ERR(" giriş dosyasının formatı uyumsuzdur!\n");
                exit(0);    /* exit(0): belge "sorunsuz çıkılmalı" diyor */
            }
        }
    }

    /* bytes_read == 0 → EOF, normal bitiş; -1 → read() sistem hatası */
    return (bytes_read == 0) ? 0 : -1;
}

/*
 * sau_push_bytes — Veri bloğu kopyalayıcı (kaynak → hedef)
 *
 * MİMARİ MANTIK:
 *   src_fd'den n byte okuyup dst_fd'ye yazar. POSIX read/write çağrılarının
 *   dönüş değerleri kontrol edilerek kısmi yazmalar (partial write) ve
 *   kısmi okumalar (partial read) yönetilir. Dahili SAU_IO_BUFFER boyutlu
 *   yığın tamponu ile çalışır, heap tahsisi yapmaz.
 */
ssize_t sau_push_bytes(int src_fd, int dst_fd, size_t n) {
    unsigned char buf[SAU_IO_BUFFER];
    size_t        total_written = 0;

    while (total_written < n) {
        size_t  to_read = (n - total_written > SAU_IO_BUFFER)
                          ? SAU_IO_BUFFER
                          : (n - total_written);
        ssize_t r = read(src_fd, buf, to_read);
        if (r <= 0) return -1;                          /* EOF veya hata */

        /* Kısmi yazma koruması: write() her zaman r byte yazmayabilir */
        ssize_t written = 0;
        while (written < r) {
            ssize_t w = write(dst_fd, buf + written, (size_t)(r - written));
            if (w <= 0) return -1;
            written += w;
        }
        total_written += (size_t)r;
    }
    return (ssize_t)total_written;
}

/*
 * sau_pull_bytes — Arşivden dosya verisi çıkartıcı
 *
 * MİMARİ MANTIK:
 *   İşlevsel olarak sau_push_bytes ile aynıdır. İsimlendirme, bağlam
 *   netliği için ayrı tutulmuştur: "push" arşivleme yönünü, "pull"
 *   çıkarma yönünü ifade eder.
 */
ssize_t sau_pull_bytes(int src_fd, int dst_fd, size_t n) {
    return sau_push_bytes(src_fd, dst_fd, n);
}

/*
 * sau_fetch_permissions — Dosya izin bitlerini oku
 *
 * MİMARİ MANTIK:
 *   stat() sistem çağrısı ile dosyanın metadata bilgilerine erişir.
 *   st_mode alanından 07777 maskesi ile sadece izin bitlerini ayıklar
 *   (setuid/setgid/sticky dahil). Bu değer arşiv metadata'sında 4
 *   basamaklı oktal olarak saklanır.
 */
int sau_fetch_permissions(const char *path, mode_t *out) {
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    *out = st.st_mode & 07777;
    return 0;
}