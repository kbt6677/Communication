#
# Makefile
# for process program (network)
# link only

# 最終的なプログラム
PROGRAM = GFPCVJ70

# ディレクトリ
COMMON_DIR  = ../../../../common
NW_DIR      = ../../..

MODULE_DIR  = proc/$(PROGRAM)

OBJ_DIR     = $(COMMON_DIR)/obj/$(MODULE_DIR)

EXE_DIR    = $(NW_DIR)/exe

COMLIB_DIR = $(COMMON_DIR)/obj/comlib
CNWLIB_DIR = $(COMMON_DIR)/obj/nwlib
CPRLIB_DIR = $(COMMON_DIR)/obj/proc
NNWLIB_DIR = $(NW_DIR)/obj/nwlib

GFPLIB_DIR = /G/GFPL01/XCOMU680
GFPEXE_DIR = /G/GFPL01/WCOMC680
GFPEXE_GDIR = $$GFPL01.WCOMC680

# ファイルリスト
OBJS = $(wildcard $(OBJ_DIR)/*.o)

LIBOBJS = \
		$(CPRLIB_DIR)/GFPCVXZ0/GFPCVXZ0.o \
		$(CPRLIB_DIR)/GFPCVX70/GFPCVX70.o \
		$(NNWLIB_DIR)/GFPCSJ40/GFPCSJ40.o \
		$(NNWLIB_DIR)/GFPCSJ50/GFPCSJ50.o \
		$(NNWLIB_DIR)/GFPCSJ60/GFPCSJ60.o \
		$(NNWLIB_DIR)/GFPCSJA0/GFPCSJA0.o \
		$(COMLIB_DIR)/GFPCGX40/GFPCGX40.o \
		$(COMLIB_DIR)/GFPCGX50/GFPCGX50.o \
		$(COMLIB_DIR)/GFPCGX80/GFPCGX80.o \
		$(COMLIB_DIR)/GFPCGX90/GFPCGX90.o \
		$(COMLIB_DIR)/GFPCGXA0/GFPCGXA0.o \
		$(COMLIB_DIR)/GFPCGXB0/GFPCGXB0.o \
		$(COMLIB_DIR)/GFPCGXC0/GFPCGXC0.o \
		$(COMLIB_DIR)/GFPCGXD0/GFPCGXD0.o \
		$(COMLIB_DIR)/GFPCGXG0/GFPCGXG0.o \
		$(COMLIB_DIR)/GFPCGXE0/GFPCGXE0.o \
		$(COMLIB_DIR)/GFPCGX20/libgfpcgx20.a \
		$(GFPLIB_DIR)/gfpoggz1 \
		$(GFPLIB_DIR)/gfpoggz3 \
		$(GFPLIB_DIR)/gfpoggz4 \


LIBS = -lxcobdll -lcob
#LIBS  LIBS-lcre -lcrtl -lossk -lossf -lsec -li18n -licnv -losse -linet -lossh -lossc

TARGET = $(EXE_DIR)/$(PROGRAM)

# コンパイラとフラグ
CC = c11
CFLAGS = -c -g \
        $(addprefix -I, $(INCLUDE_DIRS)) \
        -Wsystype=guardian -Wextensions -Wnomap -Wlist -Wnosuppress -WBstatic \
        -Wnowarn=262 -Wmultibyte_char \
        -O1

# リンカーとフラグ
LD = xld
LDFLAGS = \
        -set systype guardian -set inspect on -set saveabend on -set highpin on \
        -set highrequestors on -set runnamed on \
        -allow_missing_libs -Bdynamic -verbose \
        $(LIBS) \
        "${COMP_ROOT}/usr/lib/ccpmainx.o" \
        -obey "${COMP_ROOT}/usr/lib/libc.txt"


.PHONY: all clean

# デフォルトターゲット
all: $(EXE_DIR) $(TARGET)

$(EXE_DIR):
	mkdir -p $(EXE_DIR)

# linker
$(TARGET): $(OBJS) $(LIBOBJS)
	$(LD) -o $@ $(LDFLAGS) -nostdfiles $^

# cleanup
clean:
	rm -f $(TARGET)

#install
install:
	cp -p -Wclobber $(TARGET) $(GFPEXE_DIR)/$(PROGRAM)
	gtacl -c 'FUP ALTER $(GFPEXE_GDIR).$(PROGRAM),CODE 500'

