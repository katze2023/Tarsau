# ==============================================================================
# Sakarya Üniversitesi - Bilgisayar Mühendisliği Bölümü
# Sistem Programlama Dersi (Dr. Öğr. Üyesi Abdullah Sevin)
# Tarsau Projesi Kural (Makefile) Dosyası
# ==============================================================================

# 1. MAKROLARIN TANIMLANMASI 
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
OBJS = sau_main.o sau_archive.o sau_extract.o sau_utils.o
EXE = tarsau

# .PHONY hedefi, dizinde 'clean' adında bir dosya olması durumunda çakışmayı önler.
.PHONY: all clean debug release

all: ${EXE}

# 2. ANA HEDEF / BAĞLAMA 
${EXE} : ${OBJS}
	${CC} ${CFLAGS} -o ${EXE} ${OBJS}
	@echo == Derleme islemi basari ile tamamlandi!.. ==

# 3. AÇIK BAĞIMLILIK KONTROLLERİ 
sau_main.o: sau_main.c sau_common.h
	${CC} ${CFLAGS} -c sau_main.c

sau_archive.o: sau_archive.c sau_common.h
	${CC} ${CFLAGS} -c sau_archive.c

sau_extract.o: sau_extract.c sau_common.h
	${CC} ${CFLAGS} -c sau_extract.c

sau_utils.o: sau_utils.c sau_common.h
	${CC} ${CFLAGS} -c sau_utils.c

# 4. DERLEME MODLARI
debug: CFLAGS += -g -O0
debug: ${EXE}

release: CFLAGS += -O3
release: ${EXE}

# 5. TEMİZLİK KURALI 
clean:
	@echo == Temizlik islemi baslatildi... ==
	rm -f ${OBJS} ${EXE} *.sau