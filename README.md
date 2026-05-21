# Tarsau - Sistem Programlama Dersi Proje Ödevi

Sakarya Üniversitesi Bilgisayar Mühendisliği  
Sistem Programlama Dersi 2025-2026 Bahar Dönemi Projesi

## Proje Amacı

Bu proje kapsamında, `tar`, `zip` ve `rar` benzeri çalışan ancak sıkıştırma yapmayan bir arşivleme programı geliştirilmiştir.

Programın adı `tarsau` olup aşağıdaki işlemleri gerçekleştirmektedir:

- Birden fazla ASCII metin dosyasını tek bir `.sau` arşiv dosyasında birleştirir.
- Arşiv içindeki dosyaları belirtilen klasöre geri açar.
- Dosya izinlerini (okuma, yazma, çalıştırma) korur.
- Hatalı durumlarda uygun hata mesajları verir.

## Öğrenci Bilgileri

- Ad Soyad: Fatih Kaya
- Öğrenci No: G231210072
- E-Posta: fatihkayacinar@gmail.com

## GitHub Repository

https://github.com/katze2023/Tarsau

## Proje Detayları 

```text
## Proje Dosya Yapısı
.
├── main.c        # CLI argüman ayrıştırma, komut yönlendirme ve üst akış kontrolü
├── archive.c     # Arşiv oluşturma motoru: Metadata inşası ve ham veri push süreçleri
├── extract.c     # Arşiv açma motoru: Metadata parsing, dizin/dosya üretimi ve chmod
├── utils.c       # Byte akış yöneticileri (push/pull), ASCII doğrulaması ve izin okuma
├── tarsau.h      # Ortak makrolar, sistem limitleri ve fonksiyon prototipleri
├── Makefile      # Gelişmiş otomasyon ve derleme scripti
├── README.md     # Proje dokümantasyonu
└── rapor.pdf     # Detaylı teknik tasarım ve test raporu

## Kullanılan Teknolojiler

- C (C11 Standardı)
- POSIX Sistem Çağrıları
- GNU Compiler Collection (gcc)
- Makefile
- Git & GitHub
- Linux / Unix

## Kullanılan Sistem Çağrıları

- `open()`
- `read()`
- `write()`
- `close()`
- `lseek()`
- `stat()`
- `mkdir()`
- `chmod()`

## .SAU Dosya Formatı 
Üretilen .sau dosyaları byte-level olarak iki bitişik bölümden oluşur ve padding içermez.
Dosya isimlerinde | veya , karakterlerinin bulunması durumunda program hata vererek işlemi durdurur. 0 byte uzunluğundaki dosyalar arşivlenebilir; veri kısmına byte yazılmaz.

+------------------------+-------------------------------------------------------+-----------------------+
|  HEADER_SIZE_FIELD     |                    METADATA_BODY                      |    RAW FILE DATA      |
|       (10 Byte)        |                 (Değişken Uzunluk)                    |  (Sıralı Ham İçerik)  |
+------------------------+-------------------------------------------------------+-----------------------+
| Soldan sıfır dolgulu   | |dosya1,izin,boyut|dosya2,izin,boyut|...              | Ayırıcı olmaksızın    |
| ASCII metadata boyutu  | Örn: |hello.txt,0644,13|run.sh,0755,42|               | ardışık ham byte'lar  |
+------------------------+-------------------------------------------------------+-----------------------+

## Derleme Komutları
Gelişmiş Makefile mimarisi hem akademik hata ayıklama (debug) hem de optimize edilmiş dağıtım (release) modlarını destekler:

Standart Derleme (C11 standardı ve tüm uyarılar aktif)
$ make

Hata Ayıklama Modu (GDB sembolleri eklenir, optimizasyon kapatılır)
$ make debug

Üretim/Dağıtım Modu (En yüksek seviye derleyici optimizasyonu: -O3)
$ make release

Temizlik
$ make clean


## Kullanım Senaryoları

### 1. Arşiv Oluşturma (-b)
Belirtilen girdileri tek bir yapıda birleştirir. `-o` ile çıktı adı verilmezse varsayılan olarak `a.sau` üretilir.

```bash
# Özel çıktı ismi belirterek:
$ ./tarsau -b file1.txt file2.txt file3.txt -o archive.sau

# Varsayılan çıktı ismiyle (a.sau):
$ ./tarsau -b file1.txt file2.txt
Başarı durumunda çıktı (stdout): Dosyalar birleştirildi.

2. Arşiv Açma (-a)
Arşiv dosyasını ayrıştırarak hedef klasöre çıkartır. Klasör mevcut değilse otomatik olarak oluşturulur.

Bash
$ ./tarsau -a archive.sau output_directory


# ====================================================================================================
#                                       HATA YÖNETİM MATRİSİ
# ====================================================================================================
# Durum / Hata Tetikleyicisi              | Çıktı Kanalı | Mesaj İçeriği / Davranış               | Exit Code
# ----------------------------------------+--------------+----------------------------------------+-----------
# Giriş dosyası ASCII formatına uyumsuz   | stderr       | <dosya> giriş dosyasının formatı...    | 0
# Giriş dosyası sayısı > 32               | stderr       | Çok fazla giriş dosyası.               | 1
# Toplam dosya boyutu > 200 MB            | stderr       | Toplam dosya boyutu sınırı aşıldı.     | 1
# Dosya adında geçersiz karakter (| veya ,)| stderr       | Geçersiz dosya adı karakteri.          | 1
# Geçersiz veya bozuk arşiv / Parse hatası| stderr       | Arşiv dosyası uygunsuz veya bozuk!     | 1
# Sistem kaynaklı I/O hatası (İzin vb.)   | stderr       | perror() çıktısı dinamik olarak basılır| 1
# ====================================================================================================

```
Bu proje yalnızca eğitim amaçlı geliştirilmiştir.
