#
# Makefile
# for network module
#

# ディレクトリ
COMMON_DIR  = ../../../../common
NW_DIR      = ../../../
MODULE_DIR  = nwlib/GFPCSJC0

SRC_DIR     = .
OBJ_DIR     = $(NW_DIR)/obj/$(MODULE_DIR)
LIST_DIR    = $(NW_DIR)/list/$(MODULE_DIR)

INCLUDE_DIRS = /G/system/zsysdefs \
               /G/system/zspidef \
               $(NW_DIR)/include \
               $(COMMON_DIR)/include \
               $(COMMON_DIR)/include/comlib \
               $(COMMON_DIR)/include/nwlib \
               $(COMMON_DIR)/include/gfplib \
               $(COMMON_DIR)/include/proc \

# ファイルリスト
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# コンパイラとフラグ
CC = c11
CFLAGS = -c -g \
        $(addprefix -I, $(INCLUDE_DIRS)) \
        -Wsystype=guardian -Wextensions -Wnomap -Wlist -Wnosuppress -WBstatic \
        -Wnowarn=262 -Wmultibyte_char \
        -O1


.PHONY: all clean

# デフォルトターゲット
all: $(OBJ_DIR) $(LIST_DIR) $(OBJS)

# output directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIST_DIR):
	mkdir -p $(LIST_DIR)

# compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	mv -f  $(SRC_DIR)/*.L $(LIST_DIR)/

# cleanup
clean:
	rm -rf $(OBJ_DIR)/*.o
