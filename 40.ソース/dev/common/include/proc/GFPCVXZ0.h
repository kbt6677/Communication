/**
 * @brief GFPCVXZ0.h PATHWAYサーバ共通ヘッダーファイル
 *                   Define/Template
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/
/* STANDARD HEADER */
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <stdio.h>    nolist
#include <stdarg.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "errcd.h"
#include "ipc.h"
#include "limit.h"
#include "GFPCGXG0.h"
#include "GFPCGX80.h"
#include "GFPCGXB0.h"
#include "GFPCGXC0.h"
#include "GFPCGXD0.h"
#include "GFPCGX50.h"
#include "NWM_CTU.h"
#include "GFPOGGZ3_encode.h"
#include "GFPOGGZ4_traceout.h"

#ifndef _GFPCVXZ0_H_
#define _GFPCVXZ0_H_

/* ------------------------------------------------------------------------------------------ */
/* Define 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */
                                                        /* 数値                               */
#define DEF_FNO_UNKOWN                         -1       /*   File No初期値                    */
#define DEF_BUF_SPACE                        0x20       /*   Bffuer 初期値 SPACE              */
#define DEF_BUF_NULL                         0x00       /*   Bffuer 初期値 0                  */
#define DEF_BUF_LENGTH                      12000       /*   受信Buffer Length                */
#define DEF_FLAG_ON                             1       /*   Flag ON                          */
#define DEF_FLAG_OFF                            0       /*   Flag OFF                         */
#define DEF_RET_OK                              0       /*   Ret code OK                      */
#define DEF_RET_NG                             -1       /*   Ret code NG                      */
#define DEF_RET_NG_UNLOCK                      -2       /*   Ret code NG UNLOCK 要            */
#define DEF_RET_NORMAL_END                      1       /*   Ret code Process normal end      */
#define DEF_RET_OPNER_TBL_OVER                 -1       /*   Ret code opner table over        */
#define DEF_RTN_NORMAL                          0       /*   返却値: 正常終了                 */
#define DEF_RTN_ERROR                           1       /*         : 異常終了                 */
#define DEF_TIME_INI                            0L      /*   Timer 初期値                     */
#define DEF_LCNT_INI                            0L      /*   long counter 初期値              */
#define DEF_ADDR_INI                            0L      /*   Address 初期値                   */
#define DEF_TAG_INI                            -1L      /*   TAG 初期値                       */
#define DEF_TIME_NOLIMIT_IO                    -1L      /*   timerなしI/O                     */
#define DEF_RCV_WAITDEPTH                       0       /*   enable wait operations           */
#define DEF_NORMAL_END                          0       /*   正常終了                         */
#define DEF_ABNORMAL_END                        1       /*   異常終了                         */
#define DEF_RECVE_NO                            0       /*   $RECEIVE Open No                 */
#define DEF_PRMNM_MAX_LEN                      32       /*   PARAMETER NAME  MAX LENGTH       */
#define DEF_PRMVL_MAX_LEN                      32       /*   PARAMETER VALUE MAX LENGTH       */
#define DEF_SVRCLS_CHK_LEN                     23       /*   SERVERCLASS CHECK LENGTH         */
#define DEF_NUMCHK_MAX_LEN                    100       /*   numeric chk MAX len loop stopper */
#define DEF_LOCK_OFF                            0       /*   Record Lock OFF                  */
#define DEF_LOCK_ON                             1       /*   Record Lock ON                   */
#define DEF_DAY_USEC                  86400000000       /* マイクロ秒（1日）                  */
                                                        /* char                               */
#define DEF_RECEIVE_FNAME               "$RECEIVE"      /*   $RECEIVE file name               */
#define DEF_IO_NORMAL_END                     "00"      /*   normal end                       */
#define DEF_BUF_CZERO                          '0'      /*   Bffuer 初期値 0 (char)           */
#define DEF_BUF_NO_SET                         '}'      /*   Keyエリア設定なし                */
                                                        /* File IO Type                       */
#define DEF_FILEIO_OPEN                 "OPEN    "      /*   open                             */
#define DEF_FILEIO_CLOSE                "CLOSE   "      /*   close                            */
#define DEF_FILEIO_START                "START   "      /*   start                            */
#define DEF_FILEIO_WRITE                "WRITE   "      /*   write                            */
#define DEF_FILEIO_READ                 "READ    "      /*   read                             */
#define DEF_FILEIO_RUPDX                "READUP  "      /*   readdupdatex                     */
#define DEF_FILEIO_REPLY                "REPLY   "      /*   reply                            */
#define DEF_FILEIO_UPDATE               "REWRITE "      /*   writeupdate                      */
#define DEF_FILEIO_UNLOCKREC            "UNLOCK  "      /*   unlockrec                        */
#define DEF_FILEIO_GETRINFO "FILE_GETRECEIVEINFO_"      /*   file_getreceiveinfo_             */
                                                        /* エラーログ出力 IO種別              */
#define DEF_ERL_IOTYPE_OPEN                     1       /*   OPEN                             */
#define DEF_ERL_IOTYPE_WRITE                    2       /*   WRITE                            */
#define DEF_ERL_IOTYPE_CLOSE                    3       /*   CLOSE                            */
                                                        /* Message edit type                  */
#define DEF_MSG_TYPE_SKIP                      '@'      /*   Skip                             */
#define DEF_MSG_TYPE_CHAR                      'C'      /*   char data                        */
#define DEF_MSG_TYPE_HEX                       'H'      /*   char -> Hex Convert              */
#define DEF_MSG_TYPE_LCN                       'L'      /*   char LCN 15桁固定                */
#define DEF_MSG_TYPE_SVRCLS                    'S'      /*   char ServerClass論理名 8桁固定   */
#define DEF_MSG_TYPE_CNECT                     'T'      /*   char 接続先 24桁固定             */
#define DEF_MSG_TYPE_BIN_1                     '1'      /*   Binary  1桁                      */
#define DEF_MSG_TYPE_BIN_2                     '2'      /*   Binary  2桁                      */
#define DEF_MSG_TYPE_BIN_3                     '3'      /*   Binary  3桁                      */
#define DEF_MSG_TYPE_BIN_4                     '4'      /*   Binary  4桁                      */
#define DEF_MSG_TYPE_BIN_5                     '5'      /*   Binary  5桁                      */
#define DEF_MSG_TYPE_BIN_U5                    'U'      /*   Binary  5桁 Unsigned             */
#define DEF_MSG_TYPE_BIN_8                     '8'      /*   Binary  8桁                      */
                                                        /* メッセージ通知区分                 */
#define DEF_MSGTTKB_NORMAL                     '*'      /*   正常                             */
#define DEF_MSGTTKB_SYSTEM_ERR                 'S'      /*   Systemエラー                     */
#define DEF_MSGTTKB_WARNING                    'W'      /*   警告                             */
#define DEF_MSGTTKB_GYOM_ERR                   'E'      /*   業務エラー                       */
                                                        /* TRACE Function                     */
#define DEF_TRC_FUNC_INIT                      '0'      /*   INIT(Start)                      */
#define DEF_TRC_FUNC_MAIN                      '1'      /*   メイン処理                       */
#define DEF_TRC_FUNC_OUTPUT                    '1'      /*   出力処理                         */
#define DEF_TRC_FUNC_END                       '2'      /*   終了処理                         */
                                                        /* NW情報ファイルテーブル             */
#define DEF_FNWI_IDX_SITE                       0       /*   サイト識別                       */
#define DEF_FNWI_IDX_INTERFACE                  1       /*   インタフェース識別               */
#define DEF_FNWI_IDX_STATION                    2       /*   ステーション識別                 */

#define DEF_MSG_TYPE_LCN_LEN                   15       /* LCN 固定桁数                       */
#define DEF_MSG_TYPE_SVRCLS_LEN                 8       /* ServerClass 固定桁数               */
#define DEF_MSG_TYPE_CNECT_LEN                 24       /* 接続先 固定桁数                    */

#define DEF_GLMLG_KEY_LEN                      19       /* キー長                             */
#define DEF_GCSST_KEY_LEN                      24       /* キー長                             */
#define DEF_GFNWS_KEY_LEN                      18       /* 接続先固有情報ファイルキー長       */

#define DEF_GLMLG_DENBUN_OFFSET               205       /* 制御電文ログ 電文オフセット        */

/* ------------------------------------------------------------------------------------------ */
/* 仮(共通に準備されていない) define                                                          */
/* ------------------------------------------------------------------------------------------ */
#define DEF_ASN_GLELG                      "GLELG"      /* エラー出力ログファイル             */

/* ------------------------------------------------------------------------------------------ */
/* 構造体 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */
                                                        /* ---------------------------------- */
typedef struct __myinfo_def                             /* 自プロセス情報                     */
{                                                       /* ---------------------------------- */
    char    site_id;                                    /*  サイト識別                        */
    char    network_id;                                 /*  N/W識別                           */
    char    nw_kbn[2];                                  /*  N/W区分                           */
    char    group_id[5];                                /*  グループ識別                      */
    char    prog_id[8+1];                               /*  プログラムID                      */
    char    serverclass_name[8];                        /*  サーバクラス論理名                */
    char    serverclass_no[4];                          /*  サーバクラス論理番号              */
    long    io_timer;                                   /*  ファイルI/Oタイマー               */
    char    ems_serverclass_name[16+1];                 /*  運用監視端末出力サーバSERVERCLASS */
    char    ems_pathmon[16+1];                          /*  運用監視端末出力サーバPATHMON     */
    long    proc_io_timer;                              /*  プロセスI/Oタイマー               */
    long    send_timer;                                 /*  PATHSENDタイマー                  */
    long    send_retry_count;                           /*  PATHSENDリトライ回数              */
    char    my_prcname[4];                              /*  自プロセス論理名                  */
    char    my_prcno[4];                                /*  自プロセス論理番号                */
    short   recv_fno;                                   /*  $RECEIVEファイル番号              */
    char*   recv_buf;                                   /*  $RECEIVEバッファーポインタ        */
    char    pathmon_name[ZSYS_VAL_LEN_PROCESSNAME+1];   /*  PATHMONプロセス名                 */
    short   end_flg;                                    /*  終了フラグ                        */
    short   data_len;                                   /*  データ長                          */
    short   data_len_del_flag;                          /*  データ長領域削除済みフラグ        */
} t_myinfo_def;
                                                        /* ---------------------------------- */
typedef struct __t_sendinfo_def                         /* PATHSEND情報                       */
{                                                       /* ---------------------------------- */
  char*  psend_data;                                    /*  PATHSENDデータ                    */
  short  psend_send_len;                                /*  PATHSEND送信サイズ                */
  short  psend_resp_len;                                /*  PATHSEND応答サイズ                */
  short  psend_err;                                     /*  PATHSENDエラーコード              */
  short  gerr;                                          /*  Guardianエラーコード              */
} t_sendinfo_def;
                                                        /* ---------------------------------- */
typedef struct __t_logcon_data                          /* ログ出力接続情報                   */
{                                                       /* ---------------------------------- */
    char          domain_name[8];                       /*  ドメイン名                        */
    char          pathmon_name[16];                     /*  PATHMONプロセス名                 */
    char          server_class[16];                     /*  サーバクラス名                    */
    unsigned long pathsend_timer;                       /*  タイマ                            */
    short         retry_cnt;                            /*  PATHSENDリトライ回数              */
} t_logcon_data;
                                                        /* ---------------------------------- */
typedef struct __t_com_file_data                        /* 共通ファイル情報                   */
{                                                       /* ---------------------------------- */
    char   phy_file_name[ZSYS_VAL_LEN_FILENAME+1];      /*  物理名情報ファイル名(物理)        */
    short  phy_file_no;                                 /*  物理名情報ファイル番号            */
    char   nw_file_name[ZSYS_VAL_LEN_FILENAME+1];       /*  NW情報ファイル名(物理)            */
    short  nw_file_no;                                  /*  NW情報ファイル番号                */
    char   erlg_file_name[ZSYS_VAL_LEN_FILENAME+1];     /*  エラー出力ログファイル名(物理)    */
    short  erlg_file_no;                                /*  エラー出力ログファイル番号        */

} t_com_file_data;
                                                        /* ---------------------------------- */
typedef struct __t_gfphi_pri_key                        /* 物理名情報ファイル プライマリkey   */
{                                                       /* ---------------------------------- */
    char               site_id;                         /*  サイト識別                        */
    char               nw_id;                           /*  N/W識別                           */
    char               grp_id[5];                       /*  グループ識別                      */
    struct
    {                                                   /*  サーバクラス論理KEY               */
       struct
       {                                                /*   サーバクラス論理ID               */
          char         srv_cls_kind[8];                 /*    サーバクラス種類                */
          char         srv_cls_num[4];                  /*    サーバクラス論理番号            */
       } srv_cls_id;
       char            srv_cls_mlt_num[4];              /*   サーバクラス冗長化番号           */
    } srv_cls_key;
    struct
    {                                                   /*  プロセス／ファイル論理KEY         */
       struct
       {                                                /*   プロセス／ファイル論理ID         */
          char         prc_file_kind[8];                /*    プロセス／ファイル種類          */
          char         prc_file_num[4];                 /*    プロセス／ファイル論理番号      */
       } prc_file_id;
       char            prc_file_mlt_num[4];             /*   プロセス／ファイル冗長化番号     */
    } prc_file_key;
} t_gfphi_pri_key;
                                                        /* ---------------------------------- */
typedef struct __t_gfnwi_pri_key_def                    /* NW情報ファイル  プライマリkey      */
{                                                       /* ---------------------------------- */
    char               site_id;                         /*  サイト識別                        */
    char               nw_id;                           /*  N/W識別                           */
    char               grp_id[5];                       /*  グループ識別                      */
    char               if_id[5];                        /*  インタフェース識別                */
    char               station_id[6];                   /*  ステーション識別                  */
} t_gfnwi_pri_key_def;
                                                        /* ---------------------------------- */
typedef struct __iocomp_def                             /* I/O完了情報                        */
{                                                       /* ---------------------------------- */
    char      proc_name[8+1];                           /*  Process名                         */
    short     proc_name_len;                            /*  Process名Length                   */
    short     ferror;                                   /*  ErrorCode                         */
    zsys_ddl_receiveinformation_def  recv_info;         /*  受信メッセージ                    */
} t_iocomp_def;
                                                        /* ---------------------------------- */
typedef struct __t_encdec_con                           /* ATALLA接続情報                     */
{                                                       /* ---------------------------------- */
    struct {
        char          key_file_id[8];                   /*  鍵管理ファイルID                  */
        char          key_file_name[47];                /*  鍵管理ファイル名                  */
        short         key_file_no;                      /*  鍵管理ファイル番号                */
        unsigned long key_io_timer;                     /*  鍵管理I/Oタイマー                 */
        char          domain_name[8];                   /*  domain名                          */
        char          pathmon_name[16];                 /*  PATHMONプロセス名                 */
        char          server_class[16];                 /*  サーバクラス名                    */
        unsigned long pathsend_timer;                   /*  PATHSENDタイマー                  */
        short         retry_cnt;                        /*  PATHSENDリトライ回数              */
    } atalla_info;
    struct {
        char          mac_value[32];                    /*                                    */
        short         mac_len;                          /*                                    */
        char          kc[32];                           /*                                    */
        short         kc_len;                           /*                                    */
        char          kmac[32];                         /*                                    */
        short         kmac_len;                         /*                                    */
        char          before_data[10000];               /*                                    */
        short         before_data_length;               /*                                    */
        char          encode_data[10000];               /*                                    */
        short         encode_data_length;               /*                                    */
    } enc_data;
} t_encdec_con;

/* 関数のプロトタイプ宣言 */
/* ------------------------------------------------------------------------------------------ */
/* 共通メイン関数                                                                             */
/* ------------------------------------------------------------------------------------------ */
int    main (void);                                     /* メイン処理                         */
void   CMIN_init (void);                                /* 初期処理                           */
void   CMIN_main(void);                                 /* 主処理                             */
void   CMIN_finish(void);                               /* 終了処理                           */
short  CMIN_get_params(void);                           /* パラメータ取得処理                 */
short  CMIN_com_get_physical_names (void);              /* 共通 物理ファイル名取得処理        */
short  CMIN_get_phy_name (char *p_key_value);           /* 物理ファイル名取得処理             */
short  CMIN_file_open ( char  *p_file_id                /* ファイルopen処理                   */
                      , char  *p_file_name
                      , short *p_file_no );
void   CMIN_file_close( char  *p_file_id                /* ファイルopen処理                   */
                      , char  *p_file_name
                      , short *p_file_no );
short  CMIN_get_group_info(void);                       /* N/Wグループ情報取得処理            */
void   CMIN_read_recv(void);                            /* $RECEIVE処理                       */
void   CMIN_send_reply(char *, short, short);           /* リプライ処理                       */
void   CMIN_message_output(short, char, char*, char*, ...); /* メッセージ出力                 */
void   CMIN_abend (void);                               /* ABEND処理                          */
/* ------------------------------------------------------------------------------------------ */
/* 共通モジュール関数                                                                         */
/* ------------------------------------------------------------------------------------------ */
short  CMIN_num_check (char *check_buf);                /* 数字文字チェック処理               */
short  CMIN_check_datetime(char*);                      /* 日付形式チェック                   */
short  CMIN_get_day_of_year(char*, char*);              /* 通算日算出処理                     */
short  CMIN_read_glmlg    ( char              *pch_pname            /* 物理ファイル名         */
                          , short              sh_fie_no            /* ファイル番号           */
                          , char              *pch_rec_key          /* 読込みキー             */
                          , short              sh_lock              /* LOCK有無               */
                          , char              *pch_rec              /* 読込んだレコード       */
                          , short             *psh_error       );   /* I/Oエラーコード        */
short  CMIN_put_glmlg     ( char              *pch_pname            /* 物理ファイル名         */
                          , short              sh_fie_no            /* ファイル番号           */
                          , char              *pch_rec_key          /* 読込みキー             */
                          , short              sh_lock              /* LOCK有無               */
                          , char              *pch_rec              /* 読込んだレコード       */
                          , short             *psh_error       );   /* I/Oエラーコード        */
short  CMIN_update_glmlg  ( char              *pch_pname            /* 物理ファイル名         */
                          , short              sh_fie_no            /* ファイル番号           */
                          , char              *pch_rec_key          /* 読込みキー             */
                          , short              sh_lock              /* LOCK有無               */
                          , char              *pch_rec              /* 読込んだレコード       */
                          , short             *psh_error       );   /* I/Oエラーコード        */
short  CMIN_unlock_glmlg  ( char              *pch_pname            /* 物理ファイル名         */
                          , short              sh_fie_no            /* ファイル番号           */
                          , char              *pch_rec_key          /* 読込みキー             */
                          , short              sh_lock              /* LOCK有無               */
                          , char              *pch_rec              /* 読込んだレコード       */
                          , short             *psh_error       );   /* I/Oエラーコード        */
short  CMIN_read_gfnws    ( char              *pch_pname            /* 物理ファイル名         */
                          , short              sh_fie_no            /* ファイル番号           */
                          , char              *pch_rec_key          /* 読込みキー             */
                          , short              sh_lock              /* LOCK有無               */
                          , char              *pch_rec              /* 読込んだレコード       */
                          , short             *psh_error       );   /* I/Oエラーコード        */

/* ------------------------------------------------------------------------------------------ */
/* 個別モジュール用関数                                                                       */
/* ------------------------------------------------------------------------------------------ */
void   CMIN_kbt_set_prgid(void);                        /* 個別 プログラムIDセット            */
short  CMIN_kbt_get_physical_names (void);              /* 個別 物理ファイル名取得処理        */
short  CMIN_kbt_file_open(void);                        /* 個別 ファイルopen処理              */
short  CMIN_kbt_init(void);                             /* 個別 初期処理                      */
void   CMIN_handle_req_msg(void);                       /* 電文受信処理                       */
void   CMIN_kbt_file_close(void);                       /* 個別 ファイルclose処理             */

#endif
