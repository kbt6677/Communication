#
# Makefile
# for process program (common)
# compile and link

# 最終的なプログラム
PROGRAM = GFPCVX10

# ディレクトリ
COMMON_DIR  = ../../..

MODULE_DIR  = proc/GFPCVX10

SRC_DIR     = .
OBJ_DIR     = $(COMMON_DIR)/obj/$(MODULE_DIR)
LIST_DIR    = $(COMMON_DIR)/list/$(MODULE_DIR)

INCLUDE_DIRS = /G/system/zsysdefs \
               /G/system/zspidef \
               $(COMMON_DIR)/include \
               $(COMMON_DIR)/include/comlib \
               $(COMMON_DIR)/include/nwlib \
               $(COMMON_DIR)/include/gfplib \

EXE_DIR    = $(COMMON_DIR)/exe

COMLIB_DIR = $(COMMON_DIR)/obj/comlib
CNWLIB_DIR = $(COMMON_DIR)/obj/nwlib

GFPLIB_DIR = /G/GFPL01/XCOMU680
GFPEXE_DIR = /G/GFPL01/WCOMC680
GFPEXE_GDIR = $$GFPL01.WCOMC680

# ファイルリスト
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

LIBOBJS = \
	$(COMLIB_DIR)/GFPCGX50/GFPCGX50.o \
	$(COMLIB_DIR)/GFPCGX60/GFPCGX60.o \
	$(COMLIB_DIR)/GFPCGX90/GFPCGX90.o \
	$(COMLIB_DIR)/GFPCGXA0/GFPCGXA0.o \
	$(COMLIB_DIR)/GFPCGXB0/GFPCGXB0.o \
	$(COMLIB_DIR)/GFPCGXC0/GFPCGXC0.o \
	$(COMLIB_DIR)/GFPCGXD0/GFPCGXD0.o \
	$(COMLIB_DIR)/GFPCGXG0/GFPCGXG0.o \
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
all: $(OBJ_DIR) $(LIST_DIR) $(EXE_DIR) $(TARGET)

# output directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIST_DIR):
	mkdir -p $(LIST_DIR)

$(EXE_DIR):
	mkdir -p $(EXE_DIR)

# compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	mv -f  $(SRC_DIR)/*.L $(LIST_DIR)/

# linker
$(TARGET): $(OBJS) $(LIBOBJS)
	$(LD) -o $@ $(LDFLAGS) -nostdfiles $^

# cleanup
clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET)

#install
install:
	cp -p -Wclobber $(TARGET) $(GFPEXE_DIR)/$(PROGRAM)
	gtacl -c 'FUP ALTER $(GFPEXE_GDIR).$(PROGRAM),CODE 500'

