/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXB0                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               IOモジュール                                */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCGXB0_H
#define _GFPCGXB0_H

#define DEF_COM_IOM_FUNC_OPEN                     "OPEN"
#define DEF_COM_IOM_FUNC_STARTREAD                "SRED"
#define DEF_COM_IOM_FUNC_NEXTREAD                 "NRED"
#define DEF_COM_IOM_FUNC_ADD                      "ADD "
#define DEF_COM_IOM_FUNC_UPDATE                   "UPDT"
#define DEF_COM_IOM_FUNC_DELELTE                  "DELT"
#define DEF_COM_IOM_FUNC_UNLOC                    "ULOC"
#define DEF_COM_IOM_FUNC_CLOSE                    "CLOS"
#define DEF_COM_IOM_NO_ERR                        "00"
#define DEF_COM_IOM_FILEOPEN_ERR                  "Z1"
#define DEF_COM_IOM_EOF_ERR                       "Z2"
#define DEF_COM_IOM_READ_ERR                      "Z3"
#define DEF_COM_IOM_WRITE_ERR                     "Z4"
#define DEF_COM_IOM_TIMEOUT_ERR                   "Z5"
#define DEF_COM_IOM_DISCFULL_ERR                  "Z6"
#define DEF_COM_IOM_DUPLICATE_ERR                 "Z7"
#define DEF_COM_IOM_OTHER_ERR                     "Z9"
#define DEF_COM_IOM_KEYTYPE_PRI                   "10"
#define DEF_COM_IOM_KEYTYPE_A1                    "01"
#define DEF_COM_IOM_KEYTYPE_A2                    "02"
#define DEF_COM_IOM_KEYTYPE_A3                    "03"
#define DEF_COM_IOM_KEYTYPE_A4                    "04"
#define DEF_COM_IOM_KEYTYPE_A5                    "05"
#define DEF_COM_IOM_KEYTYPE_A6                    "06"
#define DEF_COM_IOM_KEYTYPE_A7                    "07"
#define DEF_COM_IOM_KEYTYPE_A8                    "08"
#define DEF_COM_IOM_KEYTYPE_A9                    "09"
#define DEF_COM_IOM_APPROXIMATE                   0
#define DEF_COM_IOM_GENERIC                       1
#define DEF_COM_IOM_EXACT                         2
#define DEF_COM_IOM_NOLOCK                        0
#define DEF_COM_IOM_LOCK                          1
#define DEF_COM_IOM_LOCKFREE                      2
#define DEF_COM_IOM_ASCEND                        0
#define DEF_COM_IOM_DESCEND                       1
#define DEF_COM_IOM_PARTITION_KEY_NOT             0
#define DEF_COM_IOM_PARTITION_KEY_FIX             1
#define DEF_COM_IOM_PARTITION_KEY_MODULUS         2

/* typedef定義 */
#pragma fieldalign shared2 __COM_IOM_arg_3
typedef struct __COM_IOM_arg_3
{
    char            prog_id[8];
    char            file_id[8];
    char            file_name[48];
    char            file_io_type[8];
} COM_IOM_arg_3_def;

#pragma fieldalign shared2 __COM_IOM_arg_4
typedef struct __COM_IOM_arg_4
{
    char           file_id[8];
    char           file_name[48];
    short          file_no;
} COM_IOM_arg_4_def;

#pragma fieldalign shared2 __COM_IOM_arg_5
typedef struct __COM_IOM_arg_5
{
    short          part_key_type;
    short          part_key_position;
    short          part_key_len;
    char           key_value[70];
    char           key_type[2];
    short          key_len;
    short          compare_len;
    short          positioning_mode;
    short          lock_flg;
    short          asc_desc_type;
    long           io_timer;
    short          rec_len;
    char           rec_area[20000];
} COM_IOM_arg_5_def;

#pragma fieldalign shared2 __COM_IOM_arg_6
typedef struct __COM_IOM_arg_6
{
    short          guardian_errcode;
    char           err_proc[30];
    char           file_name[48];
    short          rec_len;
    char           rec_area[20000];
} COM_IOM_arg_6_def;

/* プロトタイプ宣言 */
short COM_IOM(char *, char *
            , COM_IOM_arg_3_def *
            , COM_IOM_arg_4_def *
            , COM_IOM_arg_5_def *
            , COM_IOM_arg_6_def *);

#endif

