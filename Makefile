NATIVECC = gcc
CROSS=m68k-atari-mint-
CC = $(CROSS)gcc

STRIPX = ./ext/stripx/stripx
LIBCMINI = ./ext/libcmini

SFLAGS = -m68000
CFLAGS = -Os -std=gnu99 -Wall -Wstrict-prototypes -Wmissing-prototypes -W -nostdlib -ffast-math -I$(LIBCMINI)/include
LFLAGS = -L$(LIBCMINI)/build -nodefaultlibs -nostdlib -nostartfiles -lcmini -lgem

OBJS = \
	$(LIBCMINI)/build/crt0.o \
	ext/lgcc.o \
	chgres.o \
	popup.o \
	utils.o \
	save.o \
	res.o \
	$(empty)

all: chgres.zip

chgres.prg: $(OBJS) Makefile $(STRIPX)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LFLAGS)
	$(STRIPX) -v -f chgres.prg

%.o : %.S
	$(CC) $(SFLAGS) -c $< -o $@

$(LIBCMINI)/build/crt0.o :
	cd $(LIBCMINI) && make

chgres.zip: chgres.prg
	zip -r ./chgres.zip . -i auto/wdialog.prg chgres.prg chgres.rsc readme.md

clean:
	$(RM) *.o *.prg *.zip
	$(RM) $(STRIPX)
	cd $(LIBCMINI) && make clean

$(STRIPX) : $(STRIPX).c
	$(NATIVECC) $(STRIPX).c -o $(STRIPX)

