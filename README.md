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

## GitHub Repository

https://github.com/katze2023/Tarsau

## Proje Dosya Yapısı

```text
.
├── main.c
├── archive.c
├── extract.c
├── utils.c
├── tarsau.h
├── Makefile
├── README.md
└── rapor.pdf

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

## Derleme
Projeyi derlemek için: make

## Arşiv Oluşturma
./tarsau -b file1.txt file2.txt file3.txt -o archive.sau
Eğer -o parametresi verilmezse varsayılan çıktı dosyası: a.sau

## Arşiv Açma
./tarsau -a archive.sau output_directory




Bu proje yalnızca eğitim amaçlı geliştirilmiştir.
