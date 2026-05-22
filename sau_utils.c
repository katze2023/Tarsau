/*
* Sakarya Üniversitesi - Bilgisayar Mühendisliği Bölümü
* Sistem Programlama Dersi - 2025-2026 Bahar Dönemi
* Tarsau Projesi Kural (Makefile) Dosyası
* Fatih Kaya - G231210072
*/

#include "sau_common.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

/*
 * sau_validate_ascii_stream — Dosyanın ASCII metin dosyası olup olmadığını denetler
 *
 * İşleyiş:
 * - Dosya içeriği baştan sona byte seviyesinde okunur.
 * - Geçerli karakterler: Yazdırılabilir standart ASCII (0x20-0x7E) ve 
 * gerekli kontrol karakterleridir (Tab: 0x09, Satır sonu: 0x0A, Satır başı: 0x0D).
 * - Eğer ikili (binary) veri veya geçersiz bir kontrol karakteri bulunursa
 * istenilen formatta hata mesajı basılır ve program 0 koduyla (istenildiği gibi) sonlandırılır.
 */

int sau_validate_ascii_stream(int fd, const char *fname) {
    unsigned char buf[SAU_IO_BUFFER];
    ssize_t       bytes_read;

    /* Pointer dosyanın en başına konumlandır */
    if (lseek(fd, 0, SEEK_SET) == -1) return -1;

    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            unsigned char b = buf[i];
            /* İzin verilen karakter kontrolü. İzin verilen karakterler: yazdırılabilir ASCII + \t \n \r */
            if (b > 0x7F || (b < 0x20 && b != '\n' && b != '\r' && b != '\t')) {
                /*
                 * Hatalı bir karaktere rastlanırsa programın istenilen uyarıyı vererek anında sorunsuz (exit 0) olarak sonlanması sağlanır.
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
 * sau_push_bytes — Kaynak dosyadan hedef dosyaya veri kopyalar (Arşivleme için)
 *
 * İşleyiş:
 * - src_fd (kaynak dosya) üzerinden n byte okur ve dst_fd (hedef dosya) üzerine yazar.
 * - read ve write sistem çağrılarının tam olarak istenen miktarda byte 
 * işlememesi (partial read/write) riskine karşı döngülerle korunmuştur.
 */
ssize_t sau_push_bytes(int src_fd, int dst_fd, size_t n) {
    unsigned char buf[SAU_IO_BUFFER];
    size_t        total_written = 0;

    /* Kopyalanması gereken toplam byte sayısına ulaşana kadar döngüye girer */
    while (total_written < n) {
        /* Okunacak byte miktarı tampon boyutundan büyükse parça parça okunur */
        size_t  to_read = (n - total_written > SAU_IO_BUFFER)
                          ? SAU_IO_BUFFER
                          : (n - total_written);
        ssize_t r = read(src_fd, buf, to_read);
        if (r <= 0) return -1;         /* Okuma hatası veya beklenmeyen dosya sonu (EOF) */       

        /* Okunan veriyi hedef dosyaya yazar */
        ssize_t written = 0;
        while (written < r) {
            ssize_t w = write(dst_fd, buf + written, (size_t)(r - written));
            if (w <= 0) return -1;     /* Yazma hatası */
            written += w;
        }
        total_written += (size_t)r;
    }
    return (ssize_t)total_written;
}

/*
 * sau_pull_bytes — Arşiv dosyasından dışarıya veri çıkartır (Arşiv açma için)
 *
 * İşleyiş:
 * - Veri transfer mekanizması tamamen sau_push_bytes ile aynıdır.
 * - Kodun okunabilirliğini artırmak ve arşivleme (push) ile arşivden
 * çıkartma (pull) eylemlerini kavramsal olarak ayırmak için kullanılır.
 */
ssize_t sau_pull_bytes(int src_fd, int dst_fd, size_t n) {
    return sau_push_bytes(src_fd, dst_fd, n);
}

/*
 * sau_fetch_permissions — Belirtilen dosyanın izin bitlerini okur
 *
 * İşleyiş:
 * - stat() fonksiyonu kullanılarak dosyanın üst verileri (metadata) alınır.
 * - Mode bilgisi içinden 07777 bit maskesi yardımıyla yalnızca erişim izinleri 
 * (read/write/execute ve özel bitler) elde edilir.
 */
int sau_fetch_permissions(const char *path, mode_t *out) {
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    *out = st.st_mode & 07777;
    return 0;
}