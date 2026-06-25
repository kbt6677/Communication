#
# Makefile
# for common utility module
#

PROGRAM = libgfpcgx20.a

# ディレクトリ
COMMON_DIR  = ../../..

MODULE_DIR  = comlib/GFPCGX20

SRC_DIR     = .
OBJ_DIR     = $(COMMON_DIR)/obj/$(MODULE_DIR)
LIST_DIR    = $(COMMON_DIR)/list/$(MODULE_DIR)

INCLUDE_DIRS = /G/system/zsysdefs \
               /G/system/zspidef \
               ./include \
               $(COMMON_DIR)/include \
               $(COMMON_DIR)/include/comlib \
               $(COMMON_DIR)/include/gfplib

# ファイルリスト
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# コンパイラとフラグ
CC = c11
CFLAGS = -c -g \
        $(addprefix -I, $(INCLUDE_DIRS)) \
        -Wsystype=guardian -Wextensions -Wnomap -Wlist -Wnosuppress -WBstatic \
        -Wnowarn=262,247 -Wmultibyte_char \
        -O1


.PHONY: all clean

# デフォルトターゲット
all: $(OBJ_DIR) $(LIST_DIR) $(OBJS) $(PROGRAM)

# output directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIST_DIR):
	mkdir -p $(LIST_DIR)

# compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	mv -f  $(SRC_DIR)/*.L $(LIST_DIR)/

$(PROGRAM): $(OBJS)
	$(AR) -r $@ $(OBJS)
	mv -f $(PROGRAM) $(OBJ_DIR)

depend:
	makedepend -D__MAKE_DEPEND__ $(CFLAGS) *.c

# cleanup
clean:
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/*.a
