/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX40                                    */
/*        FUNCTION          ････ 電文振分(outbond)                           */
/*                                                                           */
/*        AUTHER            ････ HAS kimura                                  */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-11-04                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 木村   2024/11/04 (電文振分(outbound))新規作成                  */
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
//#pragma  inspect,symbols,nostdfiles,saveabend,highpin,highrequesters,extensions

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <errno.h>    nolist
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <stdarg.h>   nolist
#include <tal.h>      nolist
#include <ctype.h>    nolist
#include <cextdecs.h> nolist
#include <zsysc>      nolist
#include <stdio.h>    nolist
#include <string.h>   nolist
#include <errno.h>    nolist
#include <tal.h>      nolist
#include <zspic>      nolist
#include <zfilc>      nolist

/* USER HEADER     */
#include "vproc.h"
#include "common.h"
#include "limit.h"
#include "file.h"
#include "ipc.h"
#include "ems.h"
#include "errcd.h"
#include "GFPOGGZ3_encode.h"
#include "GFPOGGZ4_traceout.h"
#include "GFPCGX40.h"
#include "GFPCGX80.h"
#include "GFPCGX90.h"
#include "GFPCGXB0.h"
#include "GFPCGXD0.h"
#include "GFPCGXC0.h"
#include "GFPCGXG0.h"
#include "NWM_ENI.h"
#include "NWM_ENC.h"
#include "NWM_HDE.h"
#include "NWM_NTE.h"
#include "GFPCVX40.h"

/********** グローバルデータ ***********/
t_myinfo_def        g_myinfo_def;        /* 自プロセス情報               */
t_denbun_lct_info   g_denbun_lct_info[MDSO_IF_MAX]; /* 電文項目位置情報      */
short               g_denbun_lct_info_cnt;          /* 電文項目位置情報数    */
short               g_denbun_lct_info_no;           /* 電文項目位置情報配列番号 */
char                g_recv_buf[MDSO_MAX_DATA_SIZE]; /* $RECEIVE I/O用バッファ */
char                g_resp_buf[MDSO_MAX_DATA_SIZE]; /* 応答メッセージ用バッファ*/
char                g_turn_tbl_cnt;      /* 送信電文折返し振分先設定テーブル配列数 */
t_turn_tbl          g_turn_tbl[30];      /* 送信電文折返し振分先設定テーブル */
t_con_list          g_con_list[MDSO_CONNECTION_MAX];     /* コネクションリスト */
short               g_con_list_cnt;      /* コネクションリスト数         */
t_line_list_st      g_line_list_st[MDSO_CONNECTION_MAX]; /* 回線ラウンドロビンリスト(ステーション) */
short               g_line_list_st_cnt;  /* 回線ラウンドロビンリスト(ステーション)数 */
t_line_list_if      g_line_list_if[MDSO_IF_MAX];         /* 回線ラウンドロビンリスト(インタフェース) */
short               g_line_list_if_cnt;  /* 回線ラウンドロビンリスト(インタフェース)数 */
short               g_line_list_if_no;   /* 回線ラウンドロビンリスト(インタフェース)使用中配列番号 */
t_encdec_con        g_encdec_con;        /* 暗号化/復号実行時のATALLA接続情報  */
t_logcon_data       g_logcon_data;       /* ログ出力接続情報             */
char                g_mti_data[4];       /* MTI保存領域                  */
char                g_log_file_name[47]; /* 保存したログファイル名       */
t_file_data         g_file_data;         /* ファイル情報                 */
char                g_internal_error_code[7]; /* 内部エラーコード        */
char                g_rcv_if_id[5];       /* 受信インタフェース識別      */
char                g_rcv_station_id[6];  /* 受信ステーション識別        */
char                g_rcv_con_id[6];      /* 受信コネクション識別        */
short               g_rcv_send_spec;      /* 送信先指定タイプ            */
short               g_rcv_re_send_range;  /* 送信先再選択範囲            */
short               g_rcv_final_send_range;  /* 送信先選択範囲(確定)     */
t_nw_info           g_nw_info[MDSO_IF_MAX]; /* NWファイル情報            */
short               g_nw_info_cnt;        /* NWファイル情報数            */
char                g_data_kind;          /* 電文種別                    */
char                g_data_log_data_kind; /* 電文ログキーの電文種別      */
char                g_connect_unit;       /* コネクション管理単位        */
char                g_station_st_unit;    /* 開局/閉局管理単位           */
t_err_log           g_err_log;            /* エラーログ情報              */
COM_STP_arg_1_def   g_openersinfo;        /* オープナー用テーブル */
zsys_ddl_receiveinformation_def  g_recv_info; /* REPLY情報               */
short               guardian_errcode_data[64];
short               *guardian_errcode = guardian_errcode_data;
oggz1in_def         g_cg010in;            /* メッセージ出力              */
char                g_lcn[15+1];          /* GFP内部LCN                  */
oggz1in_def         g_cg010in_modle;      /* メッセージ出力(共通モジュール用)  */
COM_ERL_arg_2_def   g_COM_ERL_arg_2_def;  /* エラー出力ログファイル情報  */
t_make_data_tbl     g_make_data_tbl;      /* 編集電文                    */
gflin_pkey_def      g_rcv_gflin;          /* センターID                   */

/* TRACE用変数 */
char  EXMYSRVCLSNAME[15];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;

/****************************************************************************/
/*  FUNCTION        : 1.1.0  main                                           */
/*  CALLING SEQ.    : int  main (void)                                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0：プロセス終了                                       */
/*  DESCRIPTION     : メイン                                                */
/****************************************************************************/
int main (void)
{
    /* 初期化処理 */
    MDSO_initialize();

    /* EMS */
    MDSO_message_output(DEF_EVT_PROC_START,
                        DEF_NERR_NOMAL,
                        "@X", DEF_GFPCVX40);

    while ( g_myinfo_def.end_flag == MDSO_OFF ) {
        /* 主処理 */
        MDSO_main();
    }

    /* 終了処理 */
    MDSO_end();

    return MDSO_RET_OK;

} /* end of main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_initialize                                */
/*  CALLING SEQ.    : void MDSO_initialize (void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期化処理                                            */
/****************************************************************************/
void MDSO_initialize (void)
{
    short s_ret;
    lk_zac2001t_arg_1_def Trace_on;

    /* 局状態管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def station_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def station_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def station_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def station_COM_IOM_arg_6_def;
    char ch_station_sub_prog_sts[2];

    /* 回線管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def line_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def line_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def line_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def line_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* グローバルデータ初期化 */
    memset( &g_myinfo_def,      0, sizeof(g_myinfo_def) );
    memset( &g_recv_buf,        0, sizeof(g_recv_buf) );
    memset( &g_resp_buf,        0, sizeof(g_resp_buf) );
    g_turn_tbl_cnt = 0;
    memset( &g_turn_tbl[0],     0, sizeof(g_turn_tbl) );
    memset( &g_encdec_con,      0, sizeof(g_encdec_con) );
    memset( &g_mti_data,        0, sizeof(g_mti_data) );
    memset( g_log_file_name,    0, sizeof(g_log_file_name) );
    memset( &g_file_data,       0, sizeof(g_file_data) );
    memset( &g_nw_info,         0, sizeof(g_nw_info) );
    memset( &g_denbun_lct_info, 0, sizeof(g_denbun_lct_info) );
    memset( &g_make_data_tbl,   0, sizeof(g_make_data_tbl) );

    /* グローバル情報の初期化(オープナー情報テーブル) */
    memset((char *)&g_openersinfo, ' ',sizeof(g_openersinfo));

    /*----------------------------------------------*/
    /* メッセージ出力初期設定                       */
    /*----------------------------------------------*/
    memset((char *)&g_cg010in, 0x20, sizeof(g_cg010in));
    g_cg010in.subrcd     = '0';
    g_cg010in.emsinf.rcd = '0';
    memcpy(g_cg010in.uytrminf.proctimer, "0002", 4); // temp
    memcpy(g_cg010in.uytrminf.uytrmmonlen, "00", 2); // temp
    memcpy(g_cg010in.uytrminf.uytrmsrvlen, "00", 2); // temp
    memcpy(g_cg010in.emsinf.emsgkinf.prgid, DEF_GFPCVX40, strlen(DEF_GFPCVX40));
    memset((char *)&g_cg010in_modle, 0x20, sizeof(g_cg010in_modle));

    s_ret = MDSO_init_param();            /*パラメータ取得処理*/
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* TRACE初期化 */
    Trace_on.func_flg = '0';    /*トレース初期処理*/
    TRACEOUT((char *)&Trace_on);

    /* 物理名情報ファイル読み込み処理 */
    s_ret =  MDSO_init_getphyfile();
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* NW情報ファイル取得 */
    s_ret = MDSO_init_getnwfile();
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* 共通モジュール用EMSデータ作成 */
    MDSO_module_ems_make(&g_cg010in_modle);

    /* 暗号・復号処理用初期処理 */
    s_ret = MDSO_encdec_init();
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* 局状態管理ファイルオープン */
    /* IOモジュールパラメータ初期化 */
    memset( &station_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(station_COM_IOM_arg_3_def) );
    memset( &station_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(station_COM_IOM_arg_4_def) );
    memset( &station_COM_IOM_arg_5_def, 0, sizeof(station_COM_IOM_arg_5_def) );
    memset( &station_COM_IOM_arg_6_def, 0, sizeof(station_COM_IOM_arg_6_def) );
    memset( ch_station_sub_prog_sts, 0, sizeof(ch_station_sub_prog_sts) );

    /* 局状態管理ファイルオープン */
    memcpy(station_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data.my_pname,
                               sizeof(station_COM_IOM_arg_3_def.prog_id));
    memcpy(station_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(station_COM_IOM_arg_3_def.file_name,
        g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_3_def.file_name));
    memcpy(station_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(station_COM_IOM_arg_3_def.file_io_type));
    memcpy(station_COM_IOM_arg_4_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(station_COM_IOM_arg_4_def.file_name,
        g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_4_def.file_name));
    station_COM_IOM_arg_4_def.file_no = g_file_data.station_sts_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_station_sub_prog_sts,
        &station_COM_IOM_arg_3_def,
        &station_COM_IOM_arg_4_def,
        &station_COM_IOM_arg_5_def,
        &station_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        *guardian_errcode = station_COM_IOM_arg_6_def.guardian_errcode;
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_CEN_STS,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            station_COM_IOM_arg_6_def.guardian_errcode);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* ファイル番号設定 */
    g_file_data.station_sts_file_no = station_COM_IOM_arg_4_def.file_no;

    /* 回線管理ファイルのオープン */
    /* IOモジュールパラメータ初期化 */
    memset( &line_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_3_def) );
    memset( &line_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_4_def) );
    memset( &line_COM_IOM_arg_5_def, 0, sizeof(line_COM_IOM_arg_5_def) );
    memset( &line_COM_IOM_arg_6_def, 0, sizeof(line_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 回線管理ファイルのオープン */
    memcpy(line_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data.my_pname,
                               sizeof(line_COM_IOM_arg_3_def.prog_id));
    memcpy(line_COM_IOM_arg_3_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
    memcpy(line_COM_IOM_arg_3_def.file_name,
        g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_3_def.file_name));
    memcpy(line_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(line_COM_IOM_arg_3_def.file_io_type));
    memcpy(line_COM_IOM_arg_4_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
    memcpy(line_COM_IOM_arg_4_def.file_name,
        g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_4_def.file_name));
    line_COM_IOM_arg_4_def.file_no = g_file_data.line_ctl_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_sub_prog_sts,
        &line_COM_IOM_arg_3_def,
        &line_COM_IOM_arg_4_def,
        &line_COM_IOM_arg_5_def,
        &line_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_LIN_MG,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            line_COM_IOM_arg_6_def.guardian_errcode);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* ファイル番号設定 */
    g_file_data.line_ctl_file_no = line_COM_IOM_arg_4_def.file_no;

    /* 回線ステータスファイルのオープン */
    /* IOモジュールパラメータ初期化 */
    memset( &line_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_3_def) );
    memset( &line_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_4_def) );
    memset( &line_COM_IOM_arg_5_def, 0, sizeof(line_COM_IOM_arg_5_def) );
    memset( &line_COM_IOM_arg_6_def, 0, sizeof(line_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 回線ステータスファイルのオープン */
    memcpy(line_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data.my_pname,
                               sizeof(line_COM_IOM_arg_3_def.prog_id));
    memcpy(line_COM_IOM_arg_3_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
    memcpy(line_COM_IOM_arg_3_def.file_name,
        g_file_data.line_st_file_name, sizeof(line_COM_IOM_arg_3_def.file_name));
    memcpy(line_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(line_COM_IOM_arg_3_def.file_io_type));
    memcpy(line_COM_IOM_arg_4_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
    memcpy(line_COM_IOM_arg_4_def.file_name,
        g_file_data.line_st_file_name, sizeof(line_COM_IOM_arg_4_def.file_name));
    line_COM_IOM_arg_4_def.file_no = g_file_data.line_st_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_sub_prog_sts,
        &line_COM_IOM_arg_3_def,
        &line_COM_IOM_arg_4_def,
        &line_COM_IOM_arg_5_def,
        &line_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_LIN_STS,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            line_COM_IOM_arg_6_def.guardian_errcode);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* ファイル番号設定 */
    g_file_data.line_st_file_no = line_COM_IOM_arg_4_def.file_no;

    /* 送信電文折返し振分先設定ファイル読み込み処理 */
    s_ret = MDSO_turn_tbl_make();
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* コネクション制御ラウンドロビンリスト作成処理 */
    s_ret = MDSO_conlist_make();
    if ( MDSO_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* エラーログオープン */
    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &g_COM_ERL_arg_2_def, MDSO_SPACE, sizeof(g_COM_ERL_arg_2_def) );
    memset( &t_COM_ERL_arg_3_def, MDSO_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = MDSO_ERR_LOG_OPEN;
    t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_COM_ERL_arg_1_def.data_len     = 0;
    g_err_log.file_id = -1;

    /* ファイル番号 -1設定 */
    g_COM_ERL_arg_2_def.file_no = -1;

    /* エラーログ共通処理実行 */
    s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                     &g_COM_ERL_arg_2_def,
                     &g_cg010in_modle,
                     &t_COM_ERL_arg_3_def,
                     g_myinfo_def.proc_data_sub.module_id);

    /* エラーログ結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,DEF_NERR_FILE_IO_ERR, "@X@E", "COM_ERL", s_ret);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

    /* $RECEIVEファイルオープン */
    s_ret = FILE_OPEN_(MDSO_RECEIVE_FILENAME,
                       (short)strlen(MDSO_RECEIVE_FILENAME),
                       &g_myinfo_def.recv_fno,
                       ZSYS_VAL_OPENACC_READWRITE,
                       ZSYS_VAL_OPENEXCL_SHARED,
                       MDSO_RCV_NOWAITDEPTH,
                       MDSO_RECVDEPTH);
    if ( MDSO_RET_OK != s_ret ) {
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            MDSO_RECEIVE_FILENAME,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            0);

        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSO_ON;
        return;
    }

} /* end of MDSO_initialize */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_init_param                                */
/*  CALLING SEQ.    : short MDSO_init_param (void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
short MDSO_init_param (void)
{
    char    ch_file_name[48];
    char    ch_file_id[32];
    short   s_file_name_len    = 0;
    char    ch_paramname[32];
    char    ch_param[64];
    short   ch_param_cnt =0;
    char    ch_prc_name_log[9] = DEF_PRC_FURI_O;
    short   s_result = 0;
    unsigned long l_set_time   = 0;
    char       ch_set_time[5];
    char       ch_leng_local[3];
    unsigned int  i_leng_local = 0;
    unsigned long l_time_data=0;

    memset(ch_file_name, 0, sizeof(ch_file_name));

    /* config格納領域初期化 */
    memset( &g_myinfo_def.config_data, 0, sizeof(g_myinfo_def.config_data) );

    /* オープナープロセス管理モジュール */
    COM_STP_INIT(&g_openersinfo);

    /* プロセス情報取得処理設定データ初期化 */
    memset( &g_myinfo_def.proc_data, MDSO_ZERO, sizeof(g_myinfo_def.proc_data));

    /* プロセス情報取得処理 */
    s_result = COM_PRC(&g_myinfo_def.proc_data);

    if ( s_result != MDSO_RET_OK ){
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_PRM_RD_ERR_INV,
                            "@X@E", "COM_PRC", s_result);

        return MDSO_RET_NG;
    }

    /* モジュールID */
    memcpy(g_myinfo_def.proc_data_sub.module_id, "GFPCVX40    ",
                 sizeof(g_myinfo_def.proc_data_sub.module_id));

    /* サーバクラス論理ID取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_SRV_LOGICAL_ID, strlen(DEF_SRV_LOGICAL_ID));
    s_result = get_param_by_name(ch_paramname, ch_param, 23+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        return MDSO_RET_NG;
    }
    if (strlen(ch_param) < 23) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        return MDSO_RET_NG;
    }

    /* サイト識別設定 */
    memcpy((char *)&g_myinfo_def.config_data.site_id, &ch_param[ch_param_cnt], MDSO_SITE_ID_LENG);
    ch_param_cnt += (MDSO_SITE_ID_LENG+1);
    /* N/W識別設定 */
    memcpy((char *)&g_myinfo_def.config_data.network_id, &ch_param[ch_param_cnt], MDSO_NW_ID_LENG);
    ch_param_cnt += (MDSO_NW_ID_LENG+1);
    /* グループ識別設定 */
    memcpy((char *)g_myinfo_def.config_data.group_id, &ch_param[ch_param_cnt], MDSO_GROUP_ID_LENG);
    ch_param_cnt += (MDSO_GROUP_ID_LENG+1);
    /* サーバクラス論理名設定 */
    memcpy((char *)g_myinfo_def.config_data.serverclass_name, &ch_param[ch_param_cnt], MDSO_SERVERCLASS_NAME_LENG);
    ch_param_cnt += (MDSO_SERVERCLASS_NAME_LENG+1);
    /* サーバクラス論理番号設定 */
    memcpy((char *)g_myinfo_def.config_data.serverclass_no, &ch_param[ch_param_cnt], MDSO_SERVERCLASS_NO_LENG);

    /* 物理名情報ファイル取得(ASSIGN情報) */
    memset(ch_file_id, ' ', sizeof(ch_file_id));
    memcpy( ch_file_id, DEF_ASN_GFPHI, strlen(DEF_ASN_GFPHI));
    COM_ASN(ch_file_id ,ch_file_name, &s_file_name_len );
    if ( (0 == s_file_name_len) || (47 < s_file_name_len) ){
        /* 物理名長異常 */
        MDSO_message_output(DEF_EVT_ASN_FILE_GET_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", "GFPHI", s_file_name_len);
        return MDSO_RET_NG;
    }
    /* 物理名設定 */
    memset( g_file_data.phy_file_name, ' ', sizeof(g_file_data.phy_file_name));
    memcpy( g_file_data.phy_file_name, ch_file_name, s_file_name_len);

    /* プロセス論理名取得 */
    memcpy( &g_myinfo_def.proc_data_sub.my_prcname, ch_prc_name_log,
            sizeof(g_myinfo_def.proc_data_sub.my_prcname));

    /* 運用監視端末出力サーバ・サーバクラス名取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_MSG_SRV_NAME, strlen(DEF_MSG_SRV_NAME));
    s_result = get_param_by_name(ch_paramname, ch_param, 16+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_MSG_SRV_NAME, s_result);
        return MDSO_RET_NG;
    }
    memcpy( g_myinfo_def.config_data.ems_serverclass_name, ch_param,
                    sizeof(g_myinfo_def.config_data.ems_serverclass_name) );

    /* 運用監視端末出力情報・運用監視端末出力サーバ名    */
    memcpy(g_cg010in.uytrminf.uytrmsrv,
        g_myinfo_def.config_data.ems_serverclass_name,
        12);

    /* 運用監視端末出力情報・運用監視端末出力サーバ名長  */
    i_leng_local = strlen(ch_param);
    sprintf(ch_leng_local, "%02d", i_leng_local);
    memcpy(g_cg010in.uytrminf.uytrmsrvlen, ch_leng_local, 2);

    /* 運用監視端末出力サーバ・PATHMON名取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_MSG_MON_NAME, strlen(DEF_MSG_MON_NAME));
    s_result = get_param_by_name(ch_paramname, ch_param, 32+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_MSG_MON_NAME, s_result);
        return MDSO_RET_NG;
    }
    memcpy( g_myinfo_def.config_data.ems_pathmon, ch_param,
                sizeof(g_myinfo_def.config_data.ems_pathmon) );

    /* 運用監視端末出力情報・運用監視端末出力PATHMON名   */
    memcpy(g_cg010in.uytrminf.uytrmmon,
        g_myinfo_def.config_data.ems_pathmon,
        13);
    /* 運用監視端末出力情報・運用監視端末出力PATHMON名長 */
    memset(ch_leng_local,0,3);
    i_leng_local = strlen(ch_param);
    sprintf(ch_leng_local, "%02d", i_leng_local);
    memcpy(g_cg010in.uytrminf.uytrmmonlen, ch_leng_local, 2);

    /* ファイルI/O完了待ちタイマー値取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_FILE_IO_TIMER_10MSECOND, strlen(DEF_FILE_IO_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_FILE_IO_TIMER_10MSECOND, s_result);
        return MDSO_RET_NG;
    }
    g_myinfo_def.config_data.io_timer = atol(ch_param);

    /* PATHSENDタイマー値取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_PSEND_TIMER_10MSECOND, strlen(DEF_PSEND_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_PSEND_TIMER_10MSECOND, s_result);
        return MDSO_RET_NG;
    }
    g_myinfo_def.config_data.send_timer = atol(ch_param);

    /* PATHSENDリトライ回数取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_PSEND_RETRY_CNT, strlen(DEF_PSEND_RETRY_CNT));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        MDSO_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_PSEND_RETRY_CNT, s_result);
        return MDSO_RET_NG;
    }
    g_myinfo_def.config_data.send_retry_count = atol(ch_param);

    /* 運用監視端末出力情報・プロセスI/Oタイマ(秒単位)   */
    memset(ch_set_time,0,5);
    l_set_time = g_myinfo_def.config_data.send_timer / 100;
    l_time_data = g_myinfo_def.config_data.send_timer % 100;
    if ( l_time_data != 0){
        l_set_time += 1;
    }
    sprintf(ch_set_time, "%04d", l_set_time);
    memcpy(g_cg010in.uytrminf.proctimer,
        ch_set_time, 4);

    return MDSO_RET_OK;

} /* end of MDSO_init_param */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_init_getphyfile                           */
/*  CALLING SEQ.    : short MDSO_init_getphyfile (void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 物理名情報ファイル読み出し処理                        */
/****************************************************************************/
short MDSO_init_getphyfile (void)
{
    short s_ret = 0;
    char  ch_pathmon_set_flag  = MDSO_OFF;
    char  ch_phy_set_flag      = MDSO_OFF;
    char  ch_file_no[5];
    char  ch_log_data_set_flag = MDSO_OFF;
    char  ch_line_set_flag     = MDSO_OFF;
    char  ch_linest_set_flag   = MDSO_OFF;
    char  ch_turn_set_flag     = MDSO_OFF;
    char  ch_nw_set_flag       = MDSO_OFF;
    char  ch_my_proc_set_flag  = MDSO_OFF;
    char  ch_all_data_set_flag = MDSO_OFF;
    db_gfphi_def *phy_tbl_local;

    /* 物理名情報ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def phy_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def phy_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def phy_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def phy_COM_IOM_arg_6_def;
    char ch_phy_sub_prog_sts[2];

    /* 物理名情報ファイル プライマリーキー ポインタ*/
    t_filekey_gfphi *gfphi_p_key;

    /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
    memset( &phy_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts,    0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイルオープン */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(phy_COM_IOM_arg_3_def.file_io_type));
    memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_4_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
    phy_COM_IOM_arg_4_def.file_no = g_file_data.phy_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_phy_sub_prog_sts,
        &phy_COM_IOM_arg_3_def,
        &phy_COM_IOM_arg_4_def,
        &phy_COM_IOM_arg_5_def,
        &phy_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* ファイル番号設定 */
    g_file_data.phy_file_no = phy_COM_IOM_arg_4_def.file_no;

    /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
    memset( &phy_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts,    0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイル読込み開始処理 */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(phy_COM_IOM_arg_3_def.file_io_type));
    memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_4_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
    phy_COM_IOM_arg_4_def.file_no = g_file_data.phy_file_no;
    phy_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(phy_COM_IOM_arg_5_def.key_type));
    gfphi_p_key = (t_filekey_gfphi *)&phy_COM_IOM_arg_5_def.key_value;
    gfphi_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gfphi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfphi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gfphi_p_key->pri_key_part1.grp_id));
    phy_COM_IOM_arg_5_def.key_len          = sizeof(gfphi_p_key->pri_key_part1);
    phy_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
    phy_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    phy_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    phy_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    phy_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfphi_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_phy_sub_prog_sts,
        &phy_COM_IOM_arg_3_def,
        &phy_COM_IOM_arg_4_def,
        &phy_COM_IOM_arg_5_def,
        &phy_COM_IOM_arg_6_def);

    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            phy_COM_IOM_arg_5_def.key_value,
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* 物理名情報ファイル展開処理 */
    while (memcmp(ch_phy_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_phy_sub_prog_sts)) == 0) {
        phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;

        /* 自サーバクラス論理ID確認 */
        if ( 0 == memcmp( (char *)g_myinfo_def.config_data.serverclass_name,
                     (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                     sizeof(g_myinfo_def.config_data.serverclass_name))){

            /* サーバクラス論理ID一致 */

            /* PATHMONプロセス名取得*/
            if (( MDSO_ON != ch_pathmon_set_flag ) &&
                ( MDSO_SPACE != phy_tbl_local->srv_cls_info.pathmon_name[0] )){
                memcpy( (char *)g_myinfo_def.pathmon_name,
                        (char *)phy_tbl_local->srv_cls_info.pathmon_name,
                        sizeof(g_myinfo_def.pathmon_name) );
                ch_pathmon_set_flag = MDSO_ON;
            }
        }

        /* 自サーバクラス論理ID確認 */
        if (( 0 == memcmp( (char *)g_myinfo_def.config_data.serverclass_name,
                     (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                     sizeof(g_myinfo_def.config_data.serverclass_name))) ||
           ( 0 == memcmp( (char *)DEF_SC_NAME_DEFAULT,
                     (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                     sizeof(g_myinfo_def.config_data.serverclass_name)))) {
            /* ファイル種類検索 */
            if ( ( MDSO_ON != ch_phy_set_flag ) &&
                 ( 0 == memcmp( (char *)DEF_FL_CEN_STS,
                             (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                             sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 局状態管理ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.station_sts_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.station_sts_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.station_sts_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_phy_set_flag = MDSO_ON;

            }
            else if ( ( MDSO_ON != ch_turn_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_RTN_DST,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 送信電文折返し振分先設定ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.data_turn_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.data_turn_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.data_turn_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_turn_set_flag = MDSO_ON;

            }
            else if ( ( MDSO_ON != ch_line_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_LIN_MG,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 回線管理ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.line_ctl_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.line_ctl_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.line_ctl_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_line_set_flag = MDSO_ON;

            }
            else if ( ( MDSO_ON != ch_linest_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_LIN_STS,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 回線ステータスファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.line_st_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.line_st_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.line_st_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_linest_set_flag = MDSO_ON;

            }
            else if ( ( MDSO_ON != ch_nw_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_NW_INFO,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* NW情報ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.nw_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.nw_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.nw_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_nw_set_flag = MDSO_ON;

            }
        }
        else if ( 0 == memcmp( (char *)DEF_SC_LOG_OUT,
                          (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                          sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind))) {
            /* サーバクラス論理IDがログ出力 */
            /* ログ出力サーバー情報取得*/
            if ( MDSO_ON != ch_log_data_set_flag ){
                if ((0 == memcmp(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num,
                                 MDSO_REDUN_ZERO,
                                 sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num))) &&
                    ( MDSO_SPACE != phy_tbl_local->srv_cls_info.srv_cls_name[0] )) {
                    memcpy( (char *)g_logcon_data.domain_name,
                            (char *)phy_tbl_local->srv_cls_info.domain_name,
                            sizeof(g_logcon_data.domain_name) );
                    memcpy( (char *)g_logcon_data.pathmon_name,
                            (char *)phy_tbl_local->srv_cls_info.pathmon_name,
                            sizeof(g_logcon_data.pathmon_name) );
                    memcpy( (char *)g_logcon_data.server_class,
                            (char *)phy_tbl_local->srv_cls_info.srv_cls_name,
                            sizeof(g_logcon_data.server_class) );
                    g_logcon_data.pathsend_timer = g_myinfo_def.config_data.send_timer;
                    g_logcon_data.retry_cnt = (short)g_myinfo_def.config_data.send_retry_count;
                    ch_log_data_set_flag = MDSO_ON;
                }
            }
        }

        /* 自プロセス情報取得 */
        if ( ( MDSO_ON != ch_my_proc_set_flag ) &&
             ( 0 == memcmp( (char *)DEF_PRC_FURI_O,
                          (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                          sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind) ))) {
            /* 自プロセス情報取得 */
            memcpy( g_myinfo_def.proc_data_sub.my_prcno,
                    phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                    sizeof(g_myinfo_def.proc_data_sub.my_prcno));
            ch_my_proc_set_flag = MDSO_ON;
        }

        /* 対象全ファイル設定済み判定 */
        if ( ( MDSO_ON == ch_phy_set_flag ) &&
             ( MDSO_ON == ch_turn_set_flag ) &&
             ( MDSO_ON == ch_line_set_flag ) &&
             ( MDSO_ON == ch_linest_set_flag ) &&
             ( MDSO_ON == ch_nw_set_flag ) &&
             ( MDSO_ON == ch_log_data_set_flag ) &&
             ( MDSO_ON == ch_my_proc_set_flag ) ){
            /* 設定完了ループ終了 正常終了*/
            ch_all_data_set_flag = MDSO_ON;
            break;
        }

        /* 物理名情報ファイルNEXTREAD処理 */
        memset( &phy_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
        memset( &phy_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
        memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
        memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );

        /* 物理名情報ファイルNEXTREAD処理 */
        memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(phy_COM_IOM_arg_3_def.prog_id));
        memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_3_def.file_name,
            g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
        memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_READ,
                                    sizeof(phy_COM_IOM_arg_3_def.file_io_type));
        memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_4_def.file_name,
            g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
        phy_COM_IOM_arg_4_def.file_no          = g_file_data.phy_file_no;
        phy_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(phy_COM_IOM_arg_5_def.key_type));
        gfphi_p_key = (t_filekey_gfphi *)&phy_COM_IOM_arg_5_def.key_value;
        gfphi_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gfphi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfphi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfphi_p_key->pri_key_part1.grp_id));
        phy_COM_IOM_arg_5_def.key_len          = sizeof(gfphi_p_key->pri_key_part1);
        phy_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
        phy_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
        phy_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
        phy_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
        phy_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfphi_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_phy_sub_prog_sts,
            &phy_COM_IOM_arg_3_def,
            &phy_COM_IOM_arg_4_def,
            &phy_COM_IOM_arg_5_def,
            &phy_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ) {
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_NEXTREAD,
                phy_COM_IOM_arg_5_def.key_value,
                phy_COM_IOM_arg_6_def.guardian_errcode);

            return MDSO_RET_NG;
        }

    }  /* end of while 物理名情報ファイル展開処理 */

    if ( ch_all_data_set_flag != MDSO_ON ){
        /* 設定未完了 異常終了*/
        return MDSO_RET_NG;
    }

    /* 冗長番号取得処理 */
    /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
    memset( &phy_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts,    0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイル読込み開始処理 */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(phy_COM_IOM_arg_3_def.file_io_type));
    memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_4_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
    phy_COM_IOM_arg_4_def.file_no = g_file_data.phy_file_no;
    phy_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(phy_COM_IOM_arg_5_def.key_value,
           g_myinfo_def.proc_data.my_pname,
           sizeof(g_myinfo_def.proc_data.my_pname));
    memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_A2,
                    sizeof(phy_COM_IOM_arg_5_def.key_type));
    phy_COM_IOM_arg_5_def.key_len          = g_myinfo_def.proc_data.my_pname_len;
    phy_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_APPROXIMATE;
    phy_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    phy_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    phy_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    phy_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfphi_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_phy_sub_prog_sts,
        &phy_COM_IOM_arg_3_def,
        &phy_COM_IOM_arg_4_def,
        &phy_COM_IOM_arg_5_def,
        &phy_COM_IOM_arg_6_def);

    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            "",
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* 自冗長番号設定 */
    phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;

    memcpy( (char *)g_myinfo_def.proc_data_sub.my_prcmlt,
            (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num,
            sizeof(g_myinfo_def.proc_data_sub.my_prcmlt) );

    /* 設定完了 */
    return MDSO_RET_OK;

} /* end of MDSO_init_getphyfile */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_init_getnwfile                            */
/*  CALLING SEQ.    : short MDSO_init_getnwfile (void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : NW情報ファイル取得処理                                */
/****************************************************************************/
short MDSO_init_getnwfile (void)
{
    short        s_ret = 0;
    db_gfnwi_def *nw_tbl_local;
    char         ch_save_data[16];

    /* 電文項目位置情報数 初期化 */
    g_denbun_lct_info_cnt = 0;

    /* コネクション管理単位初期化 */
    g_connect_unit = MDSO_SPACE;

    /* 開局/閉局管理単位 */
    g_station_st_unit = MDSO_SPACE;

    /* NW情報ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def nw_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def nw_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def nw_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def nw_COM_IOM_arg_6_def;
    char ch_nw_sub_prog_sts[2];

    /* NW情報ファイル プライマリーキー ポインタ*/
    t_filekey_gfnwi *gfnwi_p_key;

    /* NW情報ファイル用IOモジュールパラメータ初期化 */
    memset( &nw_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
    memset( &nw_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
    memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
    memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );
    memset( ch_nw_sub_prog_sts,    0, sizeof(ch_nw_sub_prog_sts) );

    /* NW情報ファイルオープン */
    memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(nw_COM_IOM_arg_3_def.prog_id));
    memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_3_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
    memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(nw_COM_IOM_arg_3_def.file_io_type));
    memcpy(nw_COM_IOM_arg_4_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_4_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_4_def.file_name));
    nw_COM_IOM_arg_4_def.file_no = g_file_data.nw_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_nw_sub_prog_sts,
        &nw_COM_IOM_arg_3_def,
        &nw_COM_IOM_arg_4_def,
        &nw_COM_IOM_arg_5_def,
        &nw_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_NW_INFO,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            nw_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* ファイル番号設定 */
    g_file_data.nw_file_no = nw_COM_IOM_arg_4_def.file_no;

    /* NW情報ファイル用IOモジュールパラメータ初期化 */
    memset( &nw_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
    memset( &nw_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
    memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
    memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );
    memset( ch_nw_sub_prog_sts,    0, sizeof(ch_nw_sub_prog_sts) );

    /* NW情報ファイル読込み開始処理 */
    memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(nw_COM_IOM_arg_3_def.prog_id));
    memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_3_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
    memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(nw_COM_IOM_arg_3_def.file_io_type));
    memcpy(nw_COM_IOM_arg_4_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_4_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_4_def.file_name));
    nw_COM_IOM_arg_4_def.file_no          = g_file_data.nw_file_no;
    nw_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(nw_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(nw_COM_IOM_arg_5_def.key_type));
    gfnwi_p_key = (t_filekey_gfnwi *)&nw_COM_IOM_arg_5_def.key_value;
    gfnwi_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gfnwi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfnwi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gfnwi_p_key->pri_key_part1.grp_id));
    nw_COM_IOM_arg_5_def.key_len          = sizeof(gfnwi_p_key->pri_key_part1);
    nw_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
    nw_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    nw_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    nw_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    nw_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfnwi_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_nw_sub_prog_sts,
        &nw_COM_IOM_arg_3_def,
        &nw_COM_IOM_arg_4_def,
        &nw_COM_IOM_arg_5_def,
        &nw_COM_IOM_arg_6_def);

    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_NW_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            nw_COM_IOM_arg_5_def.key_value,
            nw_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* NW情報ファイル展開処理 */
    while (memcmp(ch_nw_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_nw_sub_prog_sts)) == 0) {

        nw_tbl_local = (db_gfnwi_def *)nw_COM_IOM_arg_6_def.rec_area;

        /* 設定テーブル数最大長チェック */
        if ( g_denbun_lct_info_cnt >= MDSO_IF_MAX ) {
            /* 設定テーブル数最大長オーバー */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TBL MAX OVER",
                0);

            return MDSO_RET_NG;
        }

        /* コネクション管理単位設定 */
        if (g_connect_unit == MDSO_SPACE){
            g_connect_unit = nw_tbl_local->mng_lyr_info.connect_num_mng_lyr;
        }

        /* 開局/閉局管理単位設定 */
        if (g_station_st_unit == MDSO_SPACE){
            g_station_st_unit = nw_tbl_local->mng_lyr_info.open_close_mng_lyr;
        }

        /* インタフェース識別取得 */
        memcpy( g_nw_info[g_denbun_lct_info_cnt].interface_id,
                nw_tbl_local->pri_key.if_id,
                sizeof( g_nw_info[g_denbun_lct_info_cnt].interface_id ) );

        /* ステーション識別取得 */
        memcpy( g_nw_info[g_denbun_lct_info_cnt].station_id,
                nw_tbl_local->pri_key.station_id,
                sizeof( g_nw_info[g_denbun_lct_info_cnt].station_id ) );

        /* NW区分取得 */
        memcpy( g_nw_info[g_denbun_lct_info_cnt].nw_segment,
                nw_tbl_local->nw_id_info.nw_kubun,
                sizeof( g_nw_info[g_denbun_lct_info_cnt].nw_segment ) );

        if ( g_nw_info[0].nw_segment[0] == MDSO_SPACE ){
            if ( nw_tbl_local->nw_id_info.nw_kubun[0] != MDSO_SPACE ){
                memcpy( g_nw_info[0].nw_segment,
                        nw_tbl_local->nw_id_info.nw_kubun,
                        sizeof( g_nw_info[0].nw_segment ) );
            }
        }

        /* インタフェース識別名取得 */
        memcpy( g_nw_info[g_denbun_lct_info_cnt].interface_name,
                nw_tbl_local->nw_id_info.nw_if,
                sizeof( g_nw_info[g_denbun_lct_info_cnt].interface_name ) );

        /* ステーション識別名取得 */
        memcpy( g_nw_info[g_denbun_lct_info_cnt].station_name,
                nw_tbl_local->nw_id_info.nw_station,
                sizeof( g_nw_info[g_denbun_lct_info_cnt].station_name ) );

        /* インタフェース識別取得 */
        memcpy( g_denbun_lct_info[g_denbun_lct_info_cnt].interface_id,
                nw_tbl_local->pri_key.if_id,
                sizeof( g_denbun_lct_info[g_denbun_lct_info_cnt].interface_id ) );

        /* ステーション識別取得 */
        memcpy( g_denbun_lct_info[g_denbun_lct_info_cnt].station_id,
                nw_tbl_local->pri_key.station_id,
                sizeof( g_denbun_lct_info[g_denbun_lct_info_cnt].station_id ) );

        /* データレングス開始位置取得 */
        memset( ch_save_data, 0, sizeof(ch_save_data) );
        memcpy( ch_save_data, 
                nw_tbl_local->denbun_item_lct_info.data_len_start_lct,
                sizeof(nw_tbl_local->denbun_item_lct_info.data_len_start_lct) );
        g_denbun_lct_info[g_denbun_lct_info_cnt].data_len_start_lct = (short)atoi(ch_save_data);

        /* データレングス項目長取得 */
        memset( ch_save_data, 0, sizeof(ch_save_data) );
        memcpy( ch_save_data, 
                nw_tbl_local->denbun_item_lct_info.data_len_size,
                sizeof(nw_tbl_local->denbun_item_lct_info.data_len_size) );
        g_denbun_lct_info[g_denbun_lct_info_cnt].data_len_size = (short)atoi(ch_save_data);

        /* データ項目属性取得 */
        memcpy( g_denbun_lct_info[g_denbun_lct_info_cnt].data_len_attribute, 
                nw_tbl_local->denbun_item_lct_info.data_len_attribute,
                sizeof(g_denbun_lct_info[g_denbun_lct_info_cnt].data_len_attribute) );

        /* データレングスINCLUDE識別取得 */
        g_denbun_lct_info[g_denbun_lct_info_cnt].data_len_include_id =
            nw_tbl_local->denbun_item_lct_info.data_len_include_id;

        /* 電文データ開始位置取得 */
        memset( ch_save_data, 0, sizeof(ch_save_data) );
        memcpy( ch_save_data, 
                nw_tbl_local->denbun_item_lct_info.denbun_start_lct,
                sizeof(nw_tbl_local->denbun_item_lct_info.denbun_start_lct) );
        g_denbun_lct_info[g_denbun_lct_info_cnt].denbun_start_lct = (short)atoi(ch_save_data);

        /* MTI開始位置取得 */
        memset( ch_save_data, 0, sizeof(ch_save_data) );
        memcpy( ch_save_data, 
                nw_tbl_local->denbun_item_lct_info.mti_start_lct,
                sizeof(nw_tbl_local->denbun_item_lct_info.mti_start_lct) );
        g_denbun_lct_info[g_denbun_lct_info_cnt].mti_start_lct = (short)atoi(ch_save_data);

        /* MTI項目長取得 */
        memset( ch_save_data, 0, sizeof(ch_save_data) );
        memcpy( ch_save_data, 
                nw_tbl_local->denbun_item_lct_info.mti_item_len,
                sizeof(nw_tbl_local->denbun_item_lct_info.mti_item_len) );
        g_denbun_lct_info[g_denbun_lct_info_cnt].mti_item_len = (short)atoi(ch_save_data);

        /* MTI項目属性取得 */
        memcpy( g_denbun_lct_info[g_denbun_lct_info_cnt].mti_item_attribute, 
                nw_tbl_local->denbun_item_lct_info.mti_item_attribute,
                sizeof(g_denbun_lct_info[g_denbun_lct_info_cnt].mti_item_attribute) );

        /* 送信先再選択要否取得 */
        g_nw_info[g_denbun_lct_info_cnt].send_re_select_need =
                nw_tbl_local->shori_kbn_info.send_re_select_need;

        g_denbun_lct_info_cnt += 1;

        g_nw_info_cnt = g_denbun_lct_info_cnt;

        /* NW情報ファイルNEXTREAD処理 */
        memset( &nw_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
        memset( &nw_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
        memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
        memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );

        /* NW情報ファイルNEXTREAD処理 */
        memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(nw_COM_IOM_arg_3_def.prog_id));
        memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(nw_COM_IOM_arg_3_def.file_name,
            g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
        memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_READ,
                                    sizeof(nw_COM_IOM_arg_3_def.file_io_type));
        memcpy(nw_COM_IOM_arg_4_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(nw_COM_IOM_arg_4_def.file_name,
            g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_4_def.file_name));
        nw_COM_IOM_arg_4_def.file_no = g_file_data.nw_file_no;
        nw_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(nw_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(nw_COM_IOM_arg_5_def.key_type));
        gfnwi_p_key = (t_filekey_gfnwi *)&nw_COM_IOM_arg_5_def.key_value;
        gfnwi_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gfnwi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfnwi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfnwi_p_key->pri_key_part1.grp_id));
        nw_COM_IOM_arg_5_def.key_len          = sizeof(gfnwi_p_key->pri_key_part1);
        nw_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
        nw_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
        nw_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
        nw_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
        nw_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfnwi_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_nw_sub_prog_sts,
            &nw_COM_IOM_arg_3_def,
            &nw_COM_IOM_arg_4_def,
            &nw_COM_IOM_arg_5_def,
            &nw_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ) {
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_NEXTREAD,
                nw_COM_IOM_arg_5_def.key_value,
                nw_COM_IOM_arg_6_def.guardian_errcode);
            return MDSO_RET_NG;
        }

    }  /* end of while NW情報ファイル展開処理 */

    /* 設定完了 */
    return MDSO_RET_OK;

} /* end of MDSO_init_getnwfile */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_encdec_init                               */
/*  CALLING SEQ.    : short MDSO_encdec_init (void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 暗号・復号処理用初期処理                              */
/****************************************************************************/
short MDSO_encdec_init (void)
{
    short s_ret = MDSO_RET_NG;

    /* 個別モジュールパラメータ*/
    NWM_ENI_arg_1_def t_NWM_ENI_arg_1_def;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def_own;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def_other;
    NWM_ENI_arg_3_def t_NWM_ENI_arg_3_def;
    NWM_ENI_arg_4_def t_NWM_ENI_arg_4_def;
    char ch_module_id[8+1];
    ems_info_add    ems_info_add_local;

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_ENI_arg_1_def, MDSO_SPACE, sizeof(t_NWM_ENI_arg_1_def) );
    memset( &t_NWM_ENI_arg_2_def_own,   0, sizeof(t_NWM_ENI_arg_2_def_own) );
    memset( &t_NWM_ENI_arg_2_def_other, 0, sizeof(t_NWM_ENI_arg_2_def_other) );
    memset( &t_NWM_ENI_arg_3_def, 0, sizeof(t_NWM_ENI_arg_3_def) );
    memset( &t_NWM_ENI_arg_4_def, 0, sizeof(t_NWM_ENI_arg_4_def) );
    memset( ch_module_id, 0, sizeof(ch_module_id) );
    memcpy( ch_module_id, g_myinfo_def.proc_data_sub.module_id, sizeof(ch_module_id) );
    memset( &ems_info_add_local,  0, sizeof(ems_info_add));

    /* 暗号・復号処理用初期処理 */
    memcpy(t_NWM_ENI_arg_1_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(t_NWM_ENI_arg_1_def.file_name,
        g_file_data.phy_file_name, sizeof(t_NWM_ENI_arg_1_def.file_name));
    t_NWM_ENI_arg_1_def.file_no  = g_file_data.phy_file_no;
    t_NWM_ENI_arg_1_def.io_timer = g_myinfo_def.config_data.io_timer;
    t_NWM_ENI_arg_4_def.site_id  = g_myinfo_def.config_data.site_id;
    t_NWM_ENI_arg_4_def.nw_id    = g_myinfo_def.config_data.network_id;
    memcpy( t_NWM_ENI_arg_4_def.grp_id, g_myinfo_def.config_data.group_id,
               sizeof(t_NWM_ENI_arg_4_def.grp_id) );

    /* 個別モジュール */
    s_ret = NWM_ENI(DEF_NWM_ENI_OWN_NODE_ONLY,
                    &t_NWM_ENI_arg_1_def,
                    &t_NWM_ENI_arg_2_def_own,
                    &t_NWM_ENI_arg_2_def_other,
                    &t_NWM_ENI_arg_3_def,
                    &t_NWM_ENI_arg_4_def,
                    ch_module_id,
                    &g_cg010in_modle,
                    &ems_info_add_local
                    );

    /* 個別モジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "NWM_ENI", s_ret);

        return MDSO_RET_NG;
    }

    /* グローバル領域初期化 */
    memset( &g_encdec_con, 0, sizeof(g_encdec_con) );

    /* 暗号・復号情報格納処理 */
    /* 鍵管理ファイルID */
    memcpy( g_encdec_con.enc_start_data.key_file_id, t_NWM_ENI_arg_2_def_own.file_id,
                          sizeof(g_encdec_con.enc_start_data.key_file_id) );
    /* 鍵管理ファイル名 */
    memcpy( g_encdec_con.enc_start_data.key_file_name, t_NWM_ENI_arg_2_def_own.file_name,
                          sizeof(g_encdec_con.enc_start_data.key_file_name) );
    /* 鍵管理ファイル番号 */
    g_encdec_con.enc_start_data.key_file_no  = t_NWM_ENI_arg_2_def_own.file_no;
    /* 鍵管理I/Oタイマー */
    g_encdec_con.enc_start_data.key_io_timer = t_NWM_ENI_arg_2_def_own.io_timer;
    /* PATHMONプロセス名 */
    memcpy( g_encdec_con.enc_start_data.pathmon_name, t_NWM_ENI_arg_3_def.domain_name,
                          sizeof(g_encdec_con.enc_start_data.pathmon_name) );
    /* サーバクラス名 */
    memcpy( g_encdec_con.enc_start_data.server_class, t_NWM_ENI_arg_3_def.server_name,
                          sizeof(g_encdec_con.enc_start_data.server_class) );
    /* PATHSENDタイマー */
    g_encdec_con.enc_start_data.pathsend_timer = t_NWM_ENI_arg_3_def.pathsend_timer;
    /* PATHSENDリトライ回数 */
    g_encdec_con.enc_start_data.retry_cnt = t_NWM_ENI_arg_3_def.pathsend_retry_cnt;

    return MDSO_RET_OK;

} /* end of MDSO_encdec_init */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_turn_tbl_make                             */
/*  CALLING SEQ.    : short MDSO_turn_tbl_make ( void )                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 送信電文折返し振分先ファイル読み込み処理              */
/****************************************************************************/
short MDSO_turn_tbl_make(void)
{
    short        s_ret                = MDSO_RET_NG;
    short        s_turn_tbl_cnt       = 0;
    db_gfqbk_def *turn_tbl_local;
                                /* 受信電分振分先一時設定テーブル */
    t_con_list con_list;

    /* 送信電文折返し振分先ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def turn_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def turn_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def turn_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def turn_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* 送信電文折返し振分先設定ファイル プライマリーキー ポインタ*/
    t_filekey_gfqbk *gfqbk_p_key;

    /* 送信電文折返し振分先テーブル作成 */
    memset(&g_turn_tbl[0], 0x00, sizeof(g_turn_tbl));

    /* IOモジュールパラメータ初期化 */
    memset( &turn_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_3_def) );
    memset( &turn_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_4_def) );
    memset( &turn_COM_IOM_arg_5_def, 0, sizeof(turn_COM_IOM_arg_5_def) );
    memset( &turn_COM_IOM_arg_6_def, 0, sizeof(turn_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts,         0, sizeof(ch_sub_prog_sts) );

    /* 送信電文折返し振分先設定ファイル読み込み処理 */
    /* 送信電文折返し振分先設定ファイルのオープン */
    memcpy(turn_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(turn_COM_IOM_arg_3_def.prog_id));
    memcpy(turn_COM_IOM_arg_3_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
    memcpy(turn_COM_IOM_arg_3_def.file_name,
        g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_3_def.file_name));
    memcpy(turn_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_OPEN,
                                sizeof(turn_COM_IOM_arg_3_def.file_io_type));
    memcpy(turn_COM_IOM_arg_4_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
    memcpy(turn_COM_IOM_arg_4_def.file_name,
        g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_4_def.file_name));
    turn_COM_IOM_arg_4_def.file_no = g_file_data.data_turn_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_sub_prog_sts,
        &turn_COM_IOM_arg_3_def,
        &turn_COM_IOM_arg_4_def,
        &turn_COM_IOM_arg_5_def,
        &turn_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSO_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_RTN_DST,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            turn_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* ファイル番号設定 */
    g_file_data.data_turn_file_no = turn_COM_IOM_arg_4_def.file_no;

    /* IOモジュールパラメータ初期化 */
    memset( &turn_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_3_def) );
    memset( &turn_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_4_def) );
    memset( &turn_COM_IOM_arg_5_def, 0, sizeof(turn_COM_IOM_arg_5_def) );
    memset( &turn_COM_IOM_arg_6_def, 0, sizeof(turn_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 送信電文折返し振分先設定ファイル読込み開始処理 */
    memcpy(turn_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(turn_COM_IOM_arg_3_def.prog_id));
    memcpy(turn_COM_IOM_arg_3_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
    memcpy(turn_COM_IOM_arg_3_def.file_name,
        g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_3_def.file_name));
    memcpy(turn_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(turn_COM_IOM_arg_3_def.file_io_type));
    memcpy(turn_COM_IOM_arg_4_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
    memcpy(turn_COM_IOM_arg_4_def.file_name,
        g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_4_def.file_name));
    turn_COM_IOM_arg_4_def.file_no = g_file_data.data_turn_file_no;
    turn_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(turn_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(turn_COM_IOM_arg_5_def.key_type));
    gfqbk_p_key = (t_filekey_gfqbk *)&turn_COM_IOM_arg_5_def.key_value;
    gfqbk_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gfqbk_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfqbk_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gfqbk_p_key->pri_key_part1.grp_id));
    turn_COM_IOM_arg_5_def.key_len = sizeof(gfqbk_p_key->pri_key_part1);
    turn_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
    turn_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    turn_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    turn_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    turn_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfqbk_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_sub_prog_sts,
        &turn_COM_IOM_arg_3_def,
        &turn_COM_IOM_arg_4_def,
        &turn_COM_IOM_arg_5_def,
        &turn_COM_IOM_arg_6_def);

    if (( s_ret != MDSO_RET_OK ) ||
        ( memcmp(ch_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) != 0)) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_RTN_DST,
            DEF_COM_IOM_FUNC_STARTREAD,
            turn_COM_IOM_arg_5_def.key_value,
            turn_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* 送信電文折返し振分先設定ファイル展開処理 */
    while (memcmp(ch_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) == 0) {

        turn_tbl_local = (db_gfqbk_def *)turn_COM_IOM_arg_6_def.rec_area;

        /* 設定テーブル数最大長チェック */
        if ( s_turn_tbl_cnt >= MDSO_TUERN_DETOUR_TBL_MAX ) {
            /* 設定テーブル数最大長オーバー */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TABLE OVER",
                0);
            return MDSO_RET_NG;
        }
        /* レコード項目チェック(MTI) */
        if (isdigit(turn_tbl_local->pri_key.mti_id[0]) &&
            isdigit(turn_tbl_local->pri_key.mti_id[1]) &&
            isdigit(turn_tbl_local->pri_key.mti_id[2]) &&
            isdigit(turn_tbl_local->pri_key.mti_id[3])) {
            /* MTI正常(数字) */
        }
        else {
            /* MTI異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_STARTREAD,
                "MTI NG",
                0);
            return MDSO_RET_NG;
        }

        /* レコード項目チェック(サーバクラス論理名) */
        if ( ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[0] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[1] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[2] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[3] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[4] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[5] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[6] ) ) &&
             ( isalpha( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_name[7] ) ) ) {
            /* サーバクラス種類正常 */
        }
        else{
            /* サーバクラス種類異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS NG",
                0);
            return MDSO_RET_NG;
        }
        /* レコード項目チェック(サーバクラス論理番号) */
        if ( ( isdigit( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_num[0] ) ) &&
             ( isdigit( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_num[1] ) ) &&
             ( isdigit( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_num[2] ) ) &&
             ( isdigit( turn_tbl_local->recv_qfile_info.srv_cls_id.srv_cls_num[3] ) ) ) {
            /* サーバクラス論理番号正常 */
        }
        else{
            /* サーバクラス論理番号異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS NUM NG",
                0);
            return MDSO_RET_NG;
        }
        /* 送信電文折返し振分先設定ファイルからグローバルテーブルに設定 */
        memcpy(&g_turn_tbl[s_turn_tbl_cnt],
               &turn_tbl_local->pri_key.mti_id,
               sizeof(t_turn_tbl));
        /* 設定テーブル数更新 */
        g_turn_tbl_cnt += 1;

        /* 送信先物理情報設定処理 */
        memset( &con_list, 0, sizeof(con_list));

        memcpy( con_list.server_class,
                &g_turn_tbl[s_turn_tbl_cnt].t_rcv_que,
                sizeof(con_list.server_class));

        /* 物理名情報ファイル参照 */
        s_ret = MDSO_phy_file_read ( &con_list );

        if ( MDSO_RET_OK != s_ret ){
            /* エラー終了 */
            return MDSO_RET_NG;
        }

        /* ドメイン名設定 */
        memcpy( (char *)g_turn_tbl[s_turn_tbl_cnt].t_phy_data.domain_name,
                (char *)&con_list.domain_name,
                sizeof(g_turn_tbl[s_turn_tbl_cnt].t_phy_data.domain_name) );

        /* PATHMON名設定 */
        memcpy( (char *)g_turn_tbl[s_turn_tbl_cnt].t_phy_data.pathmon,
                (char *)&con_list.pathmon_name,
                sizeof(g_turn_tbl[s_turn_tbl_cnt].t_phy_data.pathmon) );

        /* 物理サーバークラス情報を設定 */
        memcpy( (char *)g_turn_tbl[s_turn_tbl_cnt].t_phy_data.serverclass,
                (char *)&con_list.server_class_phy,
                sizeof(g_turn_tbl[s_turn_tbl_cnt].t_phy_data.serverclass) );

        /* 送信電文折返し振分先設定データ設定数更新 */
        s_turn_tbl_cnt += 1;

        /* IOモジュールパラメータ初期化 */
        memset( &turn_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_3_def) );
        memset( &turn_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(turn_COM_IOM_arg_4_def) );
        memset( &turn_COM_IOM_arg_5_def, 0, sizeof(turn_COM_IOM_arg_5_def) );
        memset( &turn_COM_IOM_arg_6_def, 0, sizeof(turn_COM_IOM_arg_6_def) );

        /* 送信電文折返し振分先設定ファイルNEXTREAD処理 */
        memcpy(turn_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(turn_COM_IOM_arg_3_def.prog_id));
        memcpy(turn_COM_IOM_arg_3_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
        memcpy(turn_COM_IOM_arg_3_def.file_name,
            g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_3_def.file_name));
        memcpy(turn_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_READ,
                                    sizeof(turn_COM_IOM_arg_3_def.file_io_type));
        memcpy(turn_COM_IOM_arg_4_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
        memcpy(turn_COM_IOM_arg_4_def.file_name,
            g_file_data.data_turn_file_name, sizeof(turn_COM_IOM_arg_4_def.file_name));
        turn_COM_IOM_arg_4_def.file_no = g_file_data.data_turn_file_no;
        turn_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(turn_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(turn_COM_IOM_arg_5_def.key_type));
        gfqbk_p_key = (t_filekey_gfqbk *)&turn_COM_IOM_arg_5_def.key_value;
        gfqbk_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gfqbk_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfqbk_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfqbk_p_key->pri_key_part1.grp_id));
        turn_COM_IOM_arg_5_def.key_len = sizeof(gfqbk_p_key->pri_key_part1);
        turn_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
        turn_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
        turn_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
        turn_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
        turn_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfqbk_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_sub_prog_sts,
            &turn_COM_IOM_arg_3_def,
            &turn_COM_IOM_arg_4_def,
            &turn_COM_IOM_arg_5_def,
            &turn_COM_IOM_arg_6_def);

        /* 送信電文折返し振分先設定ファイル情報異常終了 */
        if ( MDSO_RET_OK != s_ret ) {
            /* エラー終了 */
            /* ファイルI/Oエラー(EMS) */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_NEXTREAD,
                turn_COM_IOM_arg_5_def.key_value,
                turn_COM_IOM_arg_6_def.guardian_errcode);
                return MDSO_RET_NG;
        }

    } /* end of while 送信電文折返し振分先設定ファイル展開処理 */

    return MDSO_RET_OK;

} /* end of MDSO_turn_tbl_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_conlist_make                              */
/*  CALLING SEQ.    : short MDSO_conlist_make ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : コネクション制御ラウンドロビンリスト作成処理          */
/****************************************************************************/
short MDSO_conlist_make(void)
{
/* (補足説明) */
/*   ファイル再読込処理にて、途中でエラーを検知した場合に元の情報を維持するため、 */
/*   一旦ローカルに作成し、作成成功したらグローバルを置き換える。                 */

    short           s_ret;
    short           s_con_list_cnt        = 0;
    short           s_line_list_st_cnt    = 0;
    short           s_line_list_if_cnt    = 0;
    t_con_list      ch_con_list_local[MDSO_CONNECTION_MAX];     /* 一時保存用テーブル */
    t_line_list_st  ch_line_list_st_local[MDSO_CONNECTION_MAX]; /* 一時保存用テーブル */
    t_line_list_if  ch_line_list_if_local[MDSO_IF_MAX];         /* 一時保存用テーブル */
    t_con_list      ch_phy_con_list;                            /* 物理情報取得用 */
    short           s_cnt                 = 0;
    short           loop_cnt              = 0;
    db_gflin_def *line_tbl_local;
    db_gclst_def *line_st_file;

    /* 回線管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def line_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def line_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def line_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def line_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* 回線管理ファイル プライマリーキー ポインタ*/
    t_filekey_gflin *gflin_p_key;

    /* 一時保存用テーブル初期化 */
    memset(ch_con_list_local, ' ', sizeof(ch_con_list_local));
    memset(ch_line_list_st_local, 0, sizeof(ch_line_list_st_local));
    memset(ch_line_list_if_local, 0, sizeof(ch_line_list_if_local));

    /* 回線管理ファイル読み込み処理                                   */
    /*   (自グループの全ての有効なレコードをコネクションリストに設定) */
    /* IOモジュールパラメータ初期化 */
    memset( &line_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_3_def) );
    memset( &line_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_4_def) );
    memset( &line_COM_IOM_arg_5_def, 0, sizeof(line_COM_IOM_arg_5_def) );
    memset( &line_COM_IOM_arg_6_def, 0, sizeof(line_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 回線管理設定ファイル読込み開始処理 */
    memcpy(line_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(line_COM_IOM_arg_3_def.prog_id));
    memcpy(line_COM_IOM_arg_3_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
    memcpy(line_COM_IOM_arg_3_def.file_name,
        g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_3_def.file_name));
    memcpy(line_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(line_COM_IOM_arg_3_def.file_io_type));
    memcpy(line_COM_IOM_arg_4_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
    memcpy(line_COM_IOM_arg_4_def.file_name,
        g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_4_def.file_name));
    line_COM_IOM_arg_4_def.file_no = g_file_data.line_ctl_file_no;
    line_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(line_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(line_COM_IOM_arg_5_def.key_type));
    gflin_p_key = (t_filekey_gflin *)&line_COM_IOM_arg_5_def.key_value;
    gflin_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gflin_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gflin_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gflin_p_key->pri_key_part1.grp_id));
    line_COM_IOM_arg_5_def.key_len = sizeof(gflin_p_key->pri_key_part1);
    line_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
    line_COM_IOM_arg_5_def.lock_flg = DEF_COM_IOM_NOLOCK;
    line_COM_IOM_arg_5_def.asc_desc_type = DEF_COM_IOM_ASCEND;
    line_COM_IOM_arg_5_def.io_timer = g_myinfo_def.config_data.io_timer;
    line_COM_IOM_arg_5_def.rec_len = sizeof(db_gflin_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_sub_prog_sts,
        &line_COM_IOM_arg_3_def,
        &line_COM_IOM_arg_4_def,
        &line_COM_IOM_arg_5_def,
        &line_COM_IOM_arg_6_def);

    if ( s_ret != MDSO_RET_OK ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_LIN_MG,
            DEF_COM_IOM_FUNC_STARTREAD,
            line_COM_IOM_arg_5_def.key_value,
            line_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* 回線管理ファイル展開処理 */
    while (memcmp(ch_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) == 0) {
        line_tbl_local = (db_gflin_def *)line_COM_IOM_arg_6_def.rec_area;

        /* レコード数確認 */
        if ( s_con_list_cnt >= MDSO_CONNECTION_MAX ) {
            /* 最大数オーバーエラー */
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_LIN_MG,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TABLE MAX OVER",
                0);
            return MDSO_RET_NG;
        }

        /* コネクション制御(サーバ、クライアント)が対象 */
        /* 無効レコードは除外 */
        if ((( memcmp(line_tbl_local->pri_key.connect_id, MDSO_CON_CS, 2 ) == 0 ) ||
             ( memcmp(line_tbl_local->pri_key.connect_id, MDSO_CON_CC, 2 ) == 0 )) &&
            ( line_tbl_local->invalid_flg != DEF_INVALID_FLG_ON                  )) {
            /* インタフェース識別設定 */
            memcpy( ch_con_list_local[s_con_list_cnt].interface_id,
                    line_tbl_local->pri_key.if_id,
                    sizeof(ch_con_list_local[s_con_list_cnt].interface_id));

            /* ステーション識別設定 */
            memcpy( ch_con_list_local[s_con_list_cnt].station_id,
                    line_tbl_local->pri_key.station_id,
                    sizeof(ch_con_list_local[s_con_list_cnt].station_id));

            /* コネクション識別設定 */
            memcpy( ch_con_list_local[s_con_list_cnt].connection_id,
                    line_tbl_local->pri_key.connect_id,
                    sizeof(ch_con_list_local[s_con_list_cnt].connection_id));

            /* サーバークラス論理番号設定 */
            memcpy( ch_con_list_local[s_con_list_cnt].server_class,
                    &line_tbl_local->alt1_key_info.srv_cls_id,
                    sizeof(ch_con_list_local[s_con_list_cnt].server_class));

            /* 物理サーバークラス取得処理 */
            memset( &ch_phy_con_list, ' ', sizeof(ch_phy_con_list));
            memcpy( &ch_phy_con_list, &ch_con_list_local[s_con_list_cnt], sizeof(ch_phy_con_list) );
            s_ret = MDSO_phy_file_read(&ch_phy_con_list);
            if ( MDSO_RET_OK != s_ret ) {
                /* 物理名情報取得エラー */
                /* エラー終了 */
                return MDSO_RET_NG;
            }

            /* PATHMON名設定 */
            memcpy( ch_con_list_local[s_con_list_cnt].pathmon_name,
                    ch_phy_con_list.pathmon_name,
                    sizeof(ch_con_list_local[s_con_list_cnt].pathmon_name));
            /* 物理サーバークラス名 */
            memcpy( ch_con_list_local[s_con_list_cnt].server_class_phy,
                    ch_phy_con_list.server_class_phy,
                    sizeof(ch_con_list_local[s_con_list_cnt].server_class_phy));
            /* テーブル数更新 */
            s_con_list_cnt += 1;
        }

        /* IOモジュールパラメータ初期化 */
        memset( &line_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_3_def) );
        memset( &line_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_4_def) );
        memset( &line_COM_IOM_arg_5_def, 0, sizeof(line_COM_IOM_arg_5_def) );
        memset( &line_COM_IOM_arg_6_def, 0, sizeof(line_COM_IOM_arg_6_def) );

        /* 回線管理ファイルNEXTREAD処理 */
        memcpy(line_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(line_COM_IOM_arg_3_def.prog_id));
        memcpy(line_COM_IOM_arg_3_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
        memcpy(line_COM_IOM_arg_3_def.file_name,
            g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_3_def.file_name));
        memcpy(line_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_READ,
                                    sizeof(line_COM_IOM_arg_3_def.file_io_type));
        memcpy(line_COM_IOM_arg_4_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
        memcpy(line_COM_IOM_arg_4_def.file_name,
            g_file_data.line_ctl_file_name, sizeof(line_COM_IOM_arg_4_def.file_name));
        line_COM_IOM_arg_4_def.file_no = g_file_data.line_ctl_file_no;
        line_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(line_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(line_COM_IOM_arg_5_def.key_type));
        gflin_p_key = (t_filekey_gflin *)&line_COM_IOM_arg_5_def.key_value;
        gflin_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gflin_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gflin_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gflin_p_key->pri_key_part1.grp_id));
        line_COM_IOM_arg_5_def.key_len = sizeof(gflin_p_key->pri_key_part1);
        line_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
        line_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
        line_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
        line_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
        line_COM_IOM_arg_5_def.rec_len          = sizeof(db_gflin_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_sub_prog_sts,
            &line_COM_IOM_arg_3_def,
            &line_COM_IOM_arg_4_def,
            &line_COM_IOM_arg_5_def,
            &line_COM_IOM_arg_6_def);

        /* 回線管理ファイル情報異常終了 */
        if ( MDSO_RET_OK != s_ret ) {
            /* エラー終了 */
            /* ファイルI/Oエラー(EMS) */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_LIN_MG,
                DEF_COM_IOM_FUNC_NEXTREAD,
                line_COM_IOM_arg_5_def.key_value,
                line_COM_IOM_arg_6_def.guardian_errcode);
            return MDSO_RET_NG;
        }
    } /* end of while */

    /* コネクションリスト作成完了確認 */
    if ( s_con_list_cnt == 0 ) {
        /* 作成失敗 */
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
            DEF_FL_LIN_MG,
            DEF_COM_IOM_FUNC_STARTREAD,
            "CONNECTION LIST MAKE NG",
            0);
        return MDSO_RET_NG;
    }

    /* コネクションリストにコネクション状態を設定 */
    /*   (回線ステータスファイル読み込み)         */
    for ( loop_cnt = 0; loop_cnt < s_con_list_cnt; loop_cnt++ ) {
        /* IOモジュールパラメータ初期化 */
        memset( &line_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_3_def) );
        memset( &line_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(line_COM_IOM_arg_4_def) );
        memset( &line_COM_IOM_arg_5_def, 0, sizeof(line_COM_IOM_arg_5_def) );
        memset( &line_COM_IOM_arg_6_def, 0, sizeof(line_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(line_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(line_COM_IOM_arg_3_def.prog_id));
        memcpy(line_COM_IOM_arg_3_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
        memcpy(line_COM_IOM_arg_3_def.file_name,
            g_file_data.line_st_file_name, sizeof(line_COM_IOM_arg_3_def.file_name));
        memcpy(line_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                    sizeof(line_COM_IOM_arg_3_def.file_io_type));
        memcpy(line_COM_IOM_arg_4_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
        memcpy(line_COM_IOM_arg_4_def.file_name,
            g_file_data.line_st_file_name, sizeof(line_COM_IOM_arg_4_def.file_name));
        line_COM_IOM_arg_4_def.file_no = g_file_data.line_st_file_no;
        line_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(line_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(line_COM_IOM_arg_5_def.key_type));
        gflin_p_key = (t_filekey_gflin *)&line_COM_IOM_arg_5_def.key_value;
        gflin_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gflin_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gflin_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gflin_p_key->pri_key_part1.grp_id));
        memcpy(gflin_p_key->if_id, ch_con_list_local[loop_cnt].interface_id,
                                                  sizeof(gflin_p_key->if_id));
        memcpy(gflin_p_key->station_id, ch_con_list_local[loop_cnt].station_id,
                                                sizeof(gflin_p_key->station_id));
        memcpy(gflin_p_key->connect_id, ch_con_list_local[loop_cnt].connection_id,
                                                   sizeof(gflin_p_key->connect_id));
        line_COM_IOM_arg_5_def.key_len = DEF_GFLIN_PKEY_LEN;
        line_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_EXACT;
        line_COM_IOM_arg_5_def.lock_flg = DEF_COM_IOM_NOLOCK;
        line_COM_IOM_arg_5_def.asc_desc_type = DEF_COM_IOM_ASCEND;
        line_COM_IOM_arg_5_def.io_timer = g_myinfo_def.config_data.io_timer;
        line_COM_IOM_arg_5_def.rec_len  = sizeof(db_gclst_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_STARTREAD,
            ch_sub_prog_sts,
            &line_COM_IOM_arg_3_def,
            &line_COM_IOM_arg_4_def,
            &line_COM_IOM_arg_5_def,
            &line_COM_IOM_arg_6_def);

        if (( s_ret != MDSO_RET_OK ) ||
            ( memcmp(ch_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) != 0 )) {
            /* エラー終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E","","",
                DEF_FL_LIN_STS,
                DEF_COM_IOM_FUNC_STARTREAD,
                line_COM_IOM_arg_5_def.key_value,
                line_COM_IOM_arg_6_def.guardian_errcode);
            return MDSO_RET_NG;
        }

        line_st_file = (db_gclst_def *)line_COM_IOM_arg_6_def.rec_area;

        /* コネクション状態設定 */
        if ( memcmp(line_st_file->connect_sts_info.connect_sts,
                    DEF_CONNECT_STS_DISCONN,
                    sizeof(line_st_file->connect_sts_info.connect_sts)) == 0){
            ch_con_list_local[loop_cnt].connection_st = MDSO_CONNECT_STS_DISCONN_INT;
        }
        else if ( memcmp(line_st_file->connect_sts_info.connect_sts,
                  DEF_CONNECT_STS_LISTEN,
                  sizeof(line_st_file->connect_sts_info.connect_sts)) == 0){
            ch_con_list_local[loop_cnt].connection_st = MDSO_CONNECT_STS_LISTEN_INT;
        }
        else if ( memcmp(line_st_file->connect_sts_info.connect_sts,
                  DEF_CONNECT_STS_CONNECT,
                  sizeof(line_st_file->connect_sts_info.connect_sts)) == 0){
            ch_con_list_local[loop_cnt].connection_st = MDSO_CONNECT_STS_CONNECT_INT;
        }
        else if ( memcmp(line_st_file->connect_sts_info.connect_sts,
                  DEF_CONNECT_STS_RECONNECT,
                  sizeof(line_st_file->connect_sts_info.connect_sts)) == 0){
            ch_con_list_local[loop_cnt].connection_st = MDSO_CONNECT_STS_RECONNECT_INT;
        }
        else {
            /* エラー終了 */
            /* ファイルI/Oエラー(EMS) */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_LIN_STS,
                DEF_COM_IOM_FUNC_NEXTREAD,
                "CONNECTION STATUS NG",
                0);
            return MDSO_RET_NG;
        }
        /* その他項目初期化 (ALLスペースで初期化済みのため、それ以外の項目) */
        ch_con_list_local[loop_cnt].station_sts_get  = 0;
        ch_con_list_local[loop_cnt].connection_get   = 0;
        ch_con_list_local[loop_cnt].line_list_st_idx = 0;
    }

    /* 回線ラウンドロビンリスト(ステーション)作成 */
    /*   1件目を設定 */
    /*   (全体を0で初期化しているので、0以外の項目を設定) */
    memcpy( ch_line_list_st_local[0].interface_id,
            ch_con_list_local[0].interface_id,
            sizeof(ch_line_list_st_local[0].interface_id));
    memcpy( ch_line_list_st_local[0].station_id,
            ch_con_list_local[0].station_id,
            sizeof(ch_line_list_st_local[0].station_id));
    ch_line_list_st_local[0].list_top_no = 0;
    memset(ch_line_list_st_local[0].station_sts, ' ',
           sizeof(ch_line_list_st_local[0].station_sts));
    ch_con_list_local[0].line_list_st_idx = 0;
    s_cnt = 1;

    /*   2件目以降を設定 */
    for ( loop_cnt = 1; loop_cnt < s_con_list_cnt; loop_cnt++ ) {
        if (( memcmp(ch_con_list_local[loop_cnt].interface_id,
                     ch_line_list_st_local[s_line_list_st_cnt].interface_id,
                     sizeof(ch_con_list_local[loop_cnt].interface_id)) == 0 ) &&
            ( memcmp(ch_con_list_local[loop_cnt].station_id,
                     ch_line_list_st_local[s_line_list_st_cnt].station_id,
                     sizeof(ch_con_list_local[loop_cnt].station_id)) == 0 )) {
            ch_con_list_local[loop_cnt].line_list_st_idx = s_line_list_st_cnt;
            s_cnt += 1;
        }
        else {
            /* 現IDXに対する残処理 */
            ch_line_list_st_local[s_line_list_st_cnt].list_cnt = s_cnt;

            /* IDX更新 */
            s_line_list_st_cnt += 1;
            memcpy( ch_line_list_st_local[s_line_list_st_cnt].interface_id,
                    ch_con_list_local[loop_cnt].interface_id,
                    sizeof(ch_line_list_st_local[s_line_list_st_cnt].interface_id));
            memcpy( ch_line_list_st_local[s_line_list_st_cnt].station_id,
                    ch_con_list_local[loop_cnt].station_id,
                    sizeof(ch_line_list_st_local[s_line_list_st_cnt].station_id));
            ch_line_list_st_local[s_line_list_st_cnt].list_top_no = loop_cnt;
            memset(ch_line_list_st_local[s_line_list_st_cnt].station_sts, ' ',
                   sizeof(ch_line_list_st_local[s_line_list_st_cnt].station_sts));
            ch_con_list_local[loop_cnt].line_list_st_idx = s_line_list_st_cnt;
            s_cnt = 1;
        }
    }
    /*   最終残処理 */
    ch_line_list_st_local[s_line_list_st_cnt].list_cnt = s_cnt;
    s_line_list_st_cnt += 1;

    /* 回線ラウンドロビンリスト(インタフェース)作成 */
    /*   1件目を設定 */
    /*   (全体を0で初期化しているので、0以外の項目を設定) */
    memcpy( ch_line_list_if_local[0].interface_id,
            ch_line_list_st_local[0].interface_id,
            sizeof(ch_line_list_if_local[0].interface_id));
    ch_line_list_if_local[0].list_top_no = 0;
    memset(ch_line_list_if_local[0].station_sts, ' ',
           sizeof(ch_line_list_if_local[0].station_sts));
    ch_line_list_st_local[0].line_list_if_idx = 0;
    s_cnt = 1;

    /*   2件目以降を設定 */
    for ( loop_cnt = 1; loop_cnt < s_line_list_st_cnt; loop_cnt++ ) {
        if ( memcmp(ch_line_list_st_local[loop_cnt].interface_id,
                    ch_line_list_if_local[s_line_list_if_cnt].interface_id,
                    sizeof(ch_line_list_st_local[loop_cnt].interface_id)) == 0 ) {
            ch_line_list_st_local[loop_cnt].line_list_if_idx = s_line_list_if_cnt;
            s_cnt += 1;
        }
        else {
            /* 現IDXに対する残処理 */
            ch_line_list_if_local[s_line_list_if_cnt].list_cnt = s_cnt;

            /* IDX更新 */
            s_line_list_if_cnt += 1;
            memcpy( ch_line_list_if_local[s_line_list_if_cnt].interface_id,
                    ch_line_list_st_local[loop_cnt].interface_id,
                    sizeof(ch_line_list_if_local[s_line_list_if_cnt].interface_id));
            ch_line_list_if_local[s_line_list_if_cnt].list_top_no = loop_cnt;
            memset(ch_line_list_if_local[s_line_list_if_cnt].station_sts, ' ',
                   sizeof(ch_line_list_if_local[s_line_list_if_cnt].station_sts));
            ch_line_list_st_local[loop_cnt].line_list_if_idx = s_line_list_if_cnt;
            s_cnt = 1;
        }
    }
    /*   最終残処理 */
    ch_line_list_if_local[s_line_list_if_cnt].list_cnt = s_cnt;
    s_line_list_if_cnt += 1;

    /* グローバルのテーブルを置き換え */
    memcpy( g_con_list, ch_con_list_local, sizeof(g_con_list));
    memcpy( g_line_list_st, ch_line_list_st_local, sizeof(g_line_list_st));
    memcpy( g_line_list_if, ch_line_list_if_local, sizeof(g_line_list_if));
    g_con_list_cnt = s_con_list_cnt;
    g_line_list_st_cnt = s_line_list_st_cnt;
    g_line_list_if_cnt = s_line_list_if_cnt;

    return MDSO_RET_OK;

} /* end of MDSO_conlist_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_main                                      */
/*  CALLING SEQ.    : void MDSO_main (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void MDSO_main(void)
{
    short s_ret = 0;

    /* 電文受信処理 */
    s_ret = MDSO_recv_read();
    if ( MDSO_RET_OK != s_ret ){
        g_myinfo_def.end_flag = MDSO_ON;
    }

} /* end of MDSO_main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_recv_read                                 */
/*  CALLING SEQ.    : short MDSO_recv_read ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : $RECEIVE処理                                          */
/****************************************************************************/
short MDSO_recv_read(void)
{
    short          s_ret      = 0;
    short          s_err      = 0;
    unsigned short s_read_cnt = 0;
    int            i_CC       = 0;
    zsys_ddl_smsg_open_def *sys_msg_p;

    memset(g_recv_buf,0x00,sizeof(g_recv_buf));

    /* $RECEIVE処理 */
    READUPDATEX ( g_myinfo_def.recv_fno,
                  g_recv_buf, sizeof(g_recv_buf),
                  &s_read_cnt );

    /* 受信メッセージ取得(FILE番号が0の場合) */
    if (g_myinfo_def.recv_fno == 0) {
        s_err = FILE_GETRECEIVEINFO_((short _far *)&g_recv_info);
    }
    FILE_GETINFO_ ( g_myinfo_def.recv_fno, &s_err );
 
    switch ( s_err ) {
    case ZFIL_ERR_OK :               /*NORMAL MESSAGE receive*/
       /* 電文受信処理 */
        s_ret = MDSO_req_recv((common_header_def *)g_recv_buf, (short)s_read_cnt);

        break;

    case ZFIL_ERR_SYSMESS :           /*SYSTEM MESSAGE receive*/
        sys_msg_p = (zsys_ddl_smsg_open_def *)g_recv_buf;
        switch ( sys_msg_p->u_z_msgnumber.z_msgnumber ) {
            case ZSYS_VAL_SMSG_OPEN :
                /* システムメッセージ受信処理 */
                MDSO_sys_open();
                break;
            case ZSYS_VAL_SMSG_CLOSE:
                /* システムメッセージ受信処理 */
                MDSO_sys_close();
                break;
            default :
                i_CC = REPLYX();
                if (_status_ne(i_CC)) {
                    FILE_GETINFO_(g_myinfo_def.recv_fno, &s_err);
                    MDSO_end();
                }
                break;
        }
        break;
    default :
        MDSO_message_output(DEF_EVT_REQ_ERR, DEF_NERR_IPC_SEISA_ERR, "@X@i",
            "ｱﾝｻﾎﾟｰﾄIPC", g_recv_buf);
        return MDSO_RET_NG;
    }
    return s_ret;

} /* end of MDSO_recv_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_req_recv                                  */
/*  CALLING SEQ.    : short MDSO_req_recv ( common_header_def *rcv_ipc,     */
/*                                          short rcv_data_len )            */
/*  ARGUMENT        : 1. rcv_ipc        (I) 受信IPC                         */
/*                  : 2. rcv_data_len   (I) 受信電文長                      */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信処理                                          */
/****************************************************************************/
short MDSO_req_recv (common_header_def *rcv_ipc, short rcv_data_len)
{
    short s_ret      = 0;
    short reply_code = 0;
    char  ch_module_err_code[7+1];
    c502_def *c502_rcv;

    memset(ch_module_err_code, '0', sizeof(ch_module_err_code));

    /* IPC内容精査 */
    s_ret = MDSO_ipc_chk(rcv_ipc, rcv_data_len);
    if ( MDSO_IPC_ERR_DATA_LEN == s_ret ){
        /* IPCチェック電文長異常 */
        /* 受信電文データ長チェックエラー(内部エラー) */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_REQ_ERR, DEF_NERR_IPC_SEISA_ERR, "@X@i",
                            "ﾚﾝｸﾞｽ ｴﾗｰ", rcv_ipc);
    }
    else if ( MDSO_IPC_ERR_MSG == s_ret ){
        /* IPCチェックインタフェースコード異常 */
        /* 受信電文ヘッダ精査エラー(内部エラー) */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_HEADR_SEISA_ERR,DEF_NERR_IPC_SEISA_ERR ,
                            "@L@C@X@X",
                            "","","IPC KIND","interface_code");
    }
    else{
        if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_Q_GET_NT_REQ,
                        sizeof(rcv_ipc->interface_code))) {
            /* キュー取出し通知要求 */
            s_ret = MDSO_que_req_recv( (c302_def *)rcv_ipc );
            /* IPC受付後は正常終了とする */
            s_ret = MDSO_RET_OK;
        }
        else if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_CON_STS_NT_REQ,
                       sizeof(rcv_ipc->interface_code))) {
            /* コネクション状態通知要求 */
            s_ret = MDSO_status_req( (c107_def *)rcv_ipc );
            /* IPC受付後は正常終了とする */
            s_ret = MDSO_RET_OK;
        }
        else if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_CMD_PRC_REQ,
                       sizeof(rcv_ipc->interface_code))) {
            c502_rcv = (c502_def*)rcv_ipc;
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_CMD_RCV,DEF_NERR_NOMAL,
                               "@L@C@X@X",
                                g_lcn,
                                &c502_rcv->command_info.connection_logical_name,
                                c502_rcv->common_header.interface_code,
                                c502_rcv->command_info.command_name);

            if (memcmp(c502_rcv->command_info.command_name,
                       DEF_IPC_CMD_FL_RE_READ,
                       sizeof(c502_rcv->command_info.command_name)) == 0){

                /* 接続構成変更 */
                s_ret = MDSO_file_up();
                if ( MDSO_RET_OK == s_ret ){
                    /* 正常応答 */
                    reply_code = MDSO_RET_OK;
                }
                else {
                    /* 異常応答 */
                    reply_code = MDSO_IPC_ERR;
                    memcpy(ch_module_err_code, g_internal_error_code, sizeof(g_internal_error_code));
                }
            }
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_CMD,ch_module_err_code ,
                               "@L@C@X@2",
                                g_lcn,
                                &c502_rcv->command_info.connection_logical_name,
                                c502_rcv->common_header.interface_code,
                                reply_code);
        }
    }

    /* リプライ処理 */
    if ( MDSO_RET_OK == s_ret ){
        /* 正常応答 */
        reply_code = MDSO_RET_OK;
        /* リプライ処理 */
        s_ret =  MDSO_recv_reply ((char *)rcv_ipc,
                                  reply_code,
                                  ch_module_err_code );
    }
    else {
        /* 異常応答 */
        reply_code = MDSO_IPC_ERR;
        /* リプライ処理 */
        s_ret =  MDSO_recv_reply ((char *)rcv_ipc,
                                  reply_code,
                                  g_internal_error_code );
    }

    /* リプライ処理異常時、次電文受信処理を行う */
    return MDSO_RET_OK;

} /* end of MDSO_req_recv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_ipc_chk                                   */
/*  CALLING SEQ.    : short MDSO_ipc_chk ( common_header_def *rcv_ipc,      */
/*                                         short rcv_data_len )             */
/*  ARGUMENT        : 1. rcv_ipc        (I) 受信IPC                         */
/*                  : 2. rcv_data_len   (I) 受信電文長（RECEIVE）           */
/*  RETURN CODE     : 0:正常終了 1：インタフェースコード異常 2：データ長異常*/
/*  DESCRIPTION     : IPC内容精査処理                                       */
/****************************************************************************/
short MDSO_ipc_chk (common_header_def *rcv_ipc, short rcv_data_len)
{
    c502_def *c502_def_adr;

    /* データ長確認 */
    if ( rcv_data_len != (rcv_ipc->control_data_length + sizeof(common_header_def)) ){
        /* 電文長異常 */
        return MDSO_IPC_ERR_DATA_LEN;
    }

    if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_Q_GET_NT_REQ,
                    sizeof(rcv_ipc->interface_code))) {
        /* キュー取出し通知要求 */
    }
    else if ( 0 == memcmp( rcv_ipc->interface_code, DEF_IPC_IFCD_CON_STS_NT_REQ,
                   sizeof(rcv_ipc->interface_code))) {
        /* コネクション状態通知要求 */
    }
    else if ( 0 == memcmp( rcv_ipc->interface_code, DEF_IPC_IFCD_CMD_PRC_REQ,
                   sizeof(rcv_ipc->interface_code))) {
        /* コマンド要求 */
        c502_def_adr = (c502_def *)rcv_ipc;
        if ( 0 == memcmp(DEF_IPC_CMD_FL_RE_READ,
             c502_def_adr->command_info.command_name,
             sizeof(c502_def_adr->command_info.command_name))) {
            /* 接続構成変更 */
        }
        else {
            /* 接続構成変更以外は異常 */
            return MDSO_IPC_ERR_MSG;
        }
    }
    else {
        /* インタフェースコード異常 */
        return MDSO_IPC_ERR_MSG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_ipc_chk */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_recv_reply                                */
/*  CALLING SEQ.    : short MDSO_recv_reply ( char *rcv_buff,               */
/*                                            short reply_err_code,         */
/*                                            char *module_err_code )       */
/*  ARGUMENT        : 1. rcv_buff         (I) 受信バッファ                  */
/*                  : 2. reply_err_code   (I) エラーコード                  */
/*                  : 3. module_err_code  (I) 内部エラーコード              */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信応答処理                                      */
/****************************************************************************/
short MDSO_recv_reply (char *rcv_buff,
                       short reply_err_code,
                       char *module_err_code )
{
    _cc_status     i_ret           = 0;
    c302_def       *reply_ipc      = (c302_def *)rcv_buff;
    unsigned short s_count_written = 0;
    short          s_msg_tag       = 0;
    short          s_error_return  = 0;
    short          s_msg_size      = 0;

    /* リプライIPC作成 */
    if ( 0 == memcmp( DEF_IPC_IFCD_Q_GET_NT_REQ, 
                      reply_ipc->common_header.interface_code,
                      sizeof(reply_ipc->common_header.interface_code))) {
        /* キュー取出し通知要求 */
        memcpy( reply_ipc->common_header.interface_code,
                DEF_IPC_IFCD_Q_GET_NT_RSP,
                sizeof(reply_ipc->common_header.interface_code) );
    }
    else if ( 0 == memcmp( DEF_IPC_IFCD_CON_STS_NT_REQ, 
                      reply_ipc->common_header.interface_code,
                      sizeof(reply_ipc->common_header.interface_code))) {
        /* コネクション状態通知要求 */
        memcpy( reply_ipc->common_header.interface_code,
                DEF_IPC_IFCD_CON_STS_NT_RSP,
                sizeof(reply_ipc->common_header.interface_code) );
    }
    else if ( 0 == memcmp( DEF_IPC_IFCD_CMD_PRC_REQ, 
                      reply_ipc->common_header.interface_code,
                      sizeof(reply_ipc->common_header.interface_code))) {
        /* コマンド要求 */
        memcpy( reply_ipc->common_header.interface_code,
                DEF_IPC_IFCD_CMD_PRC_RSP,
                sizeof(reply_ipc->common_header.interface_code) );
        /* データ長に0を設定 */
        reply_ipc->common_header.control_data_length = 0;
    }
    else {
        /* コード異常のためコード変更せずリプライ */
    }

    /* 電文長設定 */
    s_msg_size = reply_ipc->common_header.control_data_length
                 + sizeof(common_header_def);

    /* エラーコード設定 */
    reply_ipc->common_header.error_code = reply_err_code;

    /* 内部エラーコード設定 */
    memcpy( reply_ipc->common_header.internal_error_code,
            module_err_code,
            sizeof(reply_ipc->common_header.internal_error_code) );

    /* リプライ処理 */
    i_ret = REPLYX((char *)rcv_buff,
                           s_msg_size,
                           &s_count_written,
                           s_msg_tag,
                           s_error_return );

    if (MDSO_RET_OK != i_ret) {
        /* 異常終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "REPLY ERR",
                            rcv_buff);
    }

   return MDSO_RET_OK;

} /* end of MDSO_recv_reply */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_que_req_recv                              */
/*  CALLING SEQ.    : short MDSO_que_req_recv ( c302_def *rcv_ipc )         */
/*  ARGUMENT        : 1. rcv_ipc      (I) 受信バッファ                      */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : キュー取出し通知要求処理                              */
/****************************************************************************/
short MDSO_que_req_recv (c302_def *rcv_ipc)
{
    short        s_ret          = 0;
    short        s_ret_2        = 0;
    char         ch_log_file_name[MDSO_LOG_FILE_NAME_SIZE];
    c202_def     set_send_data;
    c302_def     rcv_ipc_save;
    short        loop_cnt       = 0;
    short        s_con_list_no  = 0;
    char         ch_edit_data[9999];
    db_gqnwq_def *ch_rcv_que_data = (db_gqnwq_def*)rcv_ipc->msg_data;

    memset( &set_send_data,        0,          sizeof(set_send_data) );
    memset( &rcv_ipc_save,         0,          sizeof(rcv_ipc_save) );
    memcpy( &rcv_ipc_save,         rcv_ipc,    sizeof(rcv_ipc_save) );
    memset( ch_edit_data,          0,          sizeof(ch_edit_data) );
    memset( g_lcn,                 0,          sizeof(g_lcn));
    memset( g_rcv_if_id,           MDSO_SPACE, sizeof(g_rcv_if_id) );
    memset( g_rcv_station_id,      MDSO_SPACE, sizeof(g_rcv_station_id) );
    memset( g_rcv_con_id,          MDSO_SPACE, sizeof(g_rcv_con_id) );
    memset( &g_rcv_gflin,          MDSO_SPACE, sizeof(g_rcv_gflin) );
    memset( g_internal_error_code, MDSO_SPACE, sizeof(g_internal_error_code));

    g_rcv_send_spec = MDSO_SEND_RANGE_INIT;
    g_rcv_re_send_range = MDSO_SEND_RANGE_INIT;
    g_rcv_final_send_range = MDSO_SEND_RANGE_INIT;

    /* GFP内部LCN保存 */
    memcpy( g_lcn, &ch_rcv_que_data->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id, 15);

    memcpy( &g_rcv_gflin,
            &ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id,
            sizeof(g_rcv_gflin) );

    if ( g_rcv_gflin.site_name == MDSO_SPACE ) {
        g_rcv_gflin.site_name = g_myinfo_def.config_data.site_id;
    }

    if ( g_rcv_gflin.nw_name == MDSO_SPACE ) {
        g_rcv_gflin.nw_name = g_myinfo_def.config_data.network_id;
    }

    if ( g_rcv_gflin.group_name[0] == MDSO_SPACE ) {
        memcpy(g_rcv_gflin.group_name, g_myinfo_def.config_data.group_id,
                                           sizeof(g_rcv_gflin.group_name));
    }

    /* 送信先指定情報取得 */
    if (( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id[0] != MDSO_SPACE ) &&
        ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.station_id[0] != MDSO_SPACE ) &&
        ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.connect_id[0] != MDSO_SPACE )) {
        g_rcv_send_spec = MDSO_SEND_RANGE_CONNECTION;
        memcpy( g_rcv_if_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id,
                sizeof(g_rcv_if_id) );
        memcpy( g_rcv_station_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.station_id,
                sizeof(g_rcv_station_id) );
        memcpy( g_rcv_con_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.connect_id,
                sizeof(g_rcv_con_id) );
    }
    else if (( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id[0] != MDSO_SPACE ) &&
             ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.station_id[0] != MDSO_SPACE ) &&
             ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.connect_id[0] == MDSO_SPACE )) {
        g_rcv_send_spec = MDSO_SEND_RANGE_STATION;
        memcpy( g_rcv_if_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id,
                sizeof(g_rcv_if_id) );
        memcpy( g_rcv_station_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.station_id,
                sizeof(g_rcv_station_id) );
    }
    else if (( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id[0] != MDSO_SPACE ) &&
             ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.station_id[0] == MDSO_SPACE ) &&
             ( ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.connect_id[0] == MDSO_SPACE )) {
        g_rcv_send_spec = MDSO_SEND_RANGE_INTERFACE;
        memcpy( g_rcv_if_id,
                ch_rcv_que_data->tushin_cntrl_info.line_info.recv_connect_id.if_id,
                sizeof(g_rcv_if_id) );
    }
    else if ( ch_rcv_que_data->tushin_cntrl_info.station_id[0] != MDSO_SPACE ) {
        for ( loop_cnt=0; loop_cnt<g_nw_info_cnt; loop_cnt++ ) {
            if ( memcmp( g_nw_info[loop_cnt].station_name,
                         ch_rcv_que_data->tushin_cntrl_info.station_id,
                         sizeof(g_nw_info[loop_cnt].station_name)) == 0 ) {
                g_rcv_send_spec = MDSO_SEND_RANGE_STATION;
                memcpy( g_rcv_if_id,
                        g_nw_info[loop_cnt].interface_id,
                        sizeof(g_rcv_if_id) );
                memcpy( g_rcv_station_id,
                        g_nw_info[loop_cnt].station_id,
                        sizeof(g_rcv_station_id) );
                break;
            }
        }
    }
    else if ( ch_rcv_que_data->tushin_cntrl_info.if_id[0] != MDSO_SPACE ) {
        for ( loop_cnt=0; loop_cnt<g_nw_info_cnt; loop_cnt++ ) {
            if ( memcmp( g_nw_info[loop_cnt].interface_name,
                         ch_rcv_que_data->tushin_cntrl_info.if_id,
                         sizeof(g_nw_info[loop_cnt].interface_name)) == 0 ) {
                g_rcv_send_spec = MDSO_SEND_RANGE_INTERFACE;
                memcpy( g_rcv_if_id,
                        g_nw_info[loop_cnt].interface_id,
                        sizeof(g_rcv_if_id) );
                break;
            }
        }
    }

    /* 送信先再選択範囲 */
    if ( ch_rcv_que_data->tushin_cntrl_info.station_id[0] != MDSO_SPACE ) {
        g_rcv_re_send_range = MDSO_SEND_RANGE_STATION;
    }
    else if ( ch_rcv_que_data->tushin_cntrl_info.if_id[0] != MDSO_SPACE ) {
        g_rcv_re_send_range = MDSO_SEND_RANGE_INTERFACE;
    }
    else {
        if ( g_connect_unit == DEF_CONNECT_NUM_MNG_LYR_ST ) {
            g_rcv_re_send_range = MDSO_SEND_RANGE_STATION;
        }
        else {
            /* 複数インタフェースに跨って送信先を選択することはあり得ないため、 */
            /* 上記のどの条件にも合致しない場合はインタフェースとする。         */
            g_rcv_re_send_range = MDSO_SEND_RANGE_INTERFACE;
        }
    }

    memcpy(g_rcv_gflin.interface_name, g_rcv_if_id,      sizeof(g_rcv_gflin.interface_name));
    memcpy(g_rcv_gflin.station_name,   g_rcv_station_id, sizeof(g_rcv_gflin.interface_name));

    /* 電文項目位置情報検索 */
    g_denbun_lct_info_no = MDSO_RET_NG;
    for ( loop_cnt=0; loop_cnt<g_denbun_lct_info_cnt; loop_cnt++ ){
        /* インタフェース識別検索 */
        /* 電文項目位置情報および送信先再選択要否はインタフェース単位管理 */
        if ((memcmp( g_rcv_if_id,
                      g_denbun_lct_info[loop_cnt].interface_id,
                      sizeof(g_denbun_lct_info[loop_cnt].interface_id)) == 0) &&
            ( memcmp( MDSO_STATION_NONE,
                      g_denbun_lct_info[loop_cnt].station_id,
                      sizeof(g_denbun_lct_info[loop_cnt].station_id))   == 0)) {
            /* 電文項目位置情報配列番号設定 */
            g_denbun_lct_info_no = loop_cnt;
            break;
        }
    }

    if (( g_denbun_lct_info_no == MDSO_RET_NG ) ||
        ( g_rcv_send_spec == MDSO_SEND_RANGE_INIT )) {
        /* 電文項目位置情報配列番号なし または 送信先指定不正 */
        /*   ※送信先指定のインタフェースが不正ということはアプリ不具合か環境不具合  */
        /*     しかあり得ないため、送信不可応答は実施せずに破棄する。                */
        /* 送信不可エラー終了 */
        /* 送信先選択不可(内部エラー) */
        memcpy( g_internal_error_code,
                DEF_NERR_DST_SELECT_ERR,
                sizeof(g_internal_error_code));

        /* エラーログ出力 */
        s_ret_2 = MDSO_err_log_set ( rcv_ipc,
                                     MDSO_ERR_LOG_NAIBU_ERR,
                                     g_internal_error_code);

        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                            DEF_NERR_DST_SELECT_ERR,
                            "@L@C@X@X",
                            g_lcn,
                            &g_rcv_gflin,
                            g_mti_data,
                            "SEND CONN NONE");
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 送信先判定処理 */
    s_ret = MDSO_send_judge(rcv_ipc, &s_con_list_no);
    if ( s_ret != MDSO_RET_OK ){
        switch (g_data_kind) {
            case MDSO_SVC_REQ:     /* 業務要求 */
            case MDSO_CTL_REQ:     /* 制御要求 */
            case MDSO_CTL_RSP:     /* 制御応答 */
                /* 送信不可応答 */
                s_ret = MDSO_RET_BAD_SEND;
                break;
            default:
                /* エラー終了 */
                s_ret = MDSO_RET_NG;
        }

        if ( s_ret == MDSO_RET_BAD_SEND ) {
            /* 送信不可応答作成送信処理 */
            s_ret_2 = MDSO_err_resp(rcv_ipc);
            if ( MDSO_RET_OK != s_ret_2 ){
                /* 処理失敗時も以降の処理を実行 */
            }
            /* 送信先コネクションなし */
            if ( g_data_kind != MDSO_SVC_REQ ) {
                /* 業務要求の場合、EMS出力無し */
                MDSO_message_output(DEF_EVT_RSP_CONN_NON,
                                    g_internal_error_code,
                                    "@L@X@X", g_lcn,
                                    "",
                                    "");
            }
        }
        else {
            /* 送信不可エラー終了 */
            /* エラーログ出力 */
            s_ret_2 = MDSO_err_log_set ( rcv_ipc,
                                         MDSO_ERR_LOG_NAIBU_ERR,
                                         g_internal_error_code);
            if ( s_ret_2 != MDSO_RET_OK ){
                /* 処理失敗時も以降の処理を実行 */
            }

            /* 送信先選択不可(内部エラー) */
            /* EMS出力 */
            /* 送信先コネクションなし */
            MDSO_message_output(DEF_EVT_RSP_CONN_NON,
                        g_internal_error_code,
                        "@L@X@X", g_lcn,
                        "",
                        "");
        }
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 電文暗号化処理 */
    s_ret = MDSO_encode( &rcv_ipc_save );
    if ( MDSO_RET_OK != s_ret ){
        /* エラー終了 */
        switch (g_data_kind) {
        case MDSO_CTL_RSP:     /* 制御応答 */
        case MDSO_SVC_REQ:     /* 業務要求 */
        case MDSO_CTL_REQ:     /* 制御要求 */
            /* 送信不可応答 */
            /* 送信不可応答送信作成処理 */
            s_ret = MDSO_err_resp(rcv_ipc);
            break;
        case MDSO_SVC_RSP:     /* 業務応答 */
        default:
            /* エラーログ出力 */
            s_ret = MDSO_err_log_set ( rcv_ipc,
                                       MDSO_ERR_LOG_NAIBU_ERR,
                                       g_internal_error_code);
           break;
        }

        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 電文ヘッダ編集処理 */
    s_ret = MDSO_header_edit();
    if ( MDSO_RET_NG == s_ret ){
        /* 電文ヘッダ精査エラー(内部エラー) */
        /* 現状NGリターンは無し */
        memcpy( g_internal_error_code,
                DEF_NERR_RCV_DENBUN_HEADR_ERR,
                sizeof(g_internal_error_code));
        /* エラーログ出力 */
        s_ret_2 = MDSO_err_log_set ( rcv_ipc,
                                     MDSO_ERR_LOG_NAIBU_ERR,
                                     g_internal_error_code);
        if ( s_ret_2 != MDSO_RET_OK ){
            /* 処理失敗時も以降の処理を実行 */
        }

        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 電文送信要求作成処理 */
    s_ret = MDSO_data_make( rcv_ipc,
                            &g_con_list[s_con_list_no],
                            &set_send_data);
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 電文ログ出力 */
    memset( ch_log_file_name, MDSO_SPACE, sizeof(ch_log_file_name));
    s_ret = MDSO_log_output( rcv_ipc,
                          &set_send_data,
                          ch_log_file_name );

    /* ログファイル名をグローバルデータに保存する */
    memcpy( g_log_file_name, ch_log_file_name, sizeof(g_log_file_name) );
    /* 二重受信判定 */
    if ( MDSO_RET_DOUBLE_SET == s_ret ) {
        /* 二重受信時の処理 */
        /* 電文送信要求送信処理を行わずリプライ処理を行う */
        return MDSO_RET_OK;
    }
    else if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        memcpy( g_internal_error_code,
                DEF_NERR_FILE_IO_ERR,
                sizeof(g_internal_error_code));

        /* エラーログ出力 */
        s_ret = MDSO_err_log_set ( rcv_ipc,
                                   MDSO_ERR_LOG_NAIBU_ERR,
                                   g_internal_error_code);
    }

    /* 電文送信要求送信処理 */
    s_ret = MDSO_data_con_send(&g_con_list[s_con_list_no], &set_send_data);

    if ( MDSO_RET_BAD_SEND == s_ret ) {
        /* 送信先選択不可(内部エラー) */
        memcpy( g_internal_error_code,
                DEF_NERR_CON_SEND_ERR,
                sizeof(g_internal_error_code));
        /* 送信不可応答送信作成処理 */
        s_ret_2 = MDSO_err_resp(rcv_ipc);
        /* 送信先選択不可エラー(内部エラー)戻り先で設定 */
        return MDSO_RET_NG;
    }
    else if ( MDSO_RET_NG == s_ret ) {
        memcpy( g_internal_error_code,
                DEF_NERR_CON_SEND_ERR,
                sizeof(g_internal_error_code));
        /* エラーログ出力 */
        s_ret = MDSO_err_log_set ( rcv_ipc,
                                     MDSO_ERR_LOG_NAIBU_ERR,
                                     g_internal_error_code);
        /* エラー終了 */
        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_que_req_recv */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_send_judge                                      */
/*  CALLING SEQ.    : short MDSO_send_judge ( c302_def *ch_rcv_ipc,               */
/*                                            short *s_con_data_no )              */
/*  ARGUMENT        : 1. ch_rcv_ipc      (I) 受信電文                             */
/*                  : 2. s_con_data_no   (O) コネクションリストの配列番号         */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 振分先判定処理                                              */
/**********************************************************************************/
short MDSO_send_judge ( c302_def *ch_rcv_ipc,
                        short *s_con_data_no )
{
    short   s_ret;
    short   s_tbl_st_idx     = 0;  /* 回線ラウンドロビンリスト(ステーション)処理中IDX    */
    short   s_tbl_st_top     = 0;  /* 回線ラウンドロビンリスト(ステーション)開始IDX      */
    short   s_tbl_st_cnt     = 0;  /* 回線ラウンドロビンリスト(ステーション)エレメント数 */
    short   s_tbl_con_top    = 0;  /* コネクションリスト開始IDX                          */
    short   s_tbl_con_cnt    = 0;  /* コネクションリストエレメント数                     */
    short   loop_cnt         = 0;
    char    ch_station_sts[2];
    short   s_set_con_no     = -1;

    db_gqnwq_def *ch_rcv_que_data = (db_gqnwq_def*)ch_rcv_ipc->msg_data;

    /* 送信電文種別取得 */
    g_data_kind = ch_rcv_que_data->denbun_send_recv_info.send_denbun_shubetu;

    /* 電文ログキーの電文種別取得 */
    g_data_log_data_kind = ch_rcv_que_data->denbun_send_recv_info.denbun_log_key.denbun_shubetu;

    /* 使用中IDXを初期化 */
    g_line_list_if_no = -1;

    /* 局状態取得済みフラグ、コネクション選択済みフラグをクリア */
    /*   ※送信先指定のインタフェース以外を使用することはないため、  */
    /*     該当インタフェースのみクリア                              */
    /*   回線ラウンドロビンリスト(インタフェース) */
    for ( loop_cnt = 0; loop_cnt < g_line_list_if_cnt; loop_cnt++ ) {
        if ( memcmp(g_rcv_if_id,
                    g_line_list_if[loop_cnt].interface_id,
                    sizeof(g_rcv_if_id)) == 0 ) {
            g_line_list_if_no = loop_cnt;
            g_line_list_if[loop_cnt].station_sts_get = MDSO_CST_GET_NON;
            s_tbl_st_top = g_line_list_if[loop_cnt].list_top_no;
            s_tbl_st_cnt = g_line_list_if[loop_cnt].list_cnt;
            break;
        }
    }

    if ( g_line_list_if_no == -1 ) {
        return MDSO_RET_NG;
    }

    /*   回線ラウンドロビンリスト(ステーション) */
    for ( loop_cnt = s_tbl_st_top; loop_cnt < (s_tbl_st_top + s_tbl_st_cnt); loop_cnt++ ) {
        g_line_list_st[loop_cnt].station_sts_get = MDSO_CST_GET_NON;
        if ( loop_cnt == s_tbl_st_top ) {
            s_tbl_con_top = g_line_list_st[loop_cnt].list_top_no;
            s_tbl_con_cnt = g_line_list_st[loop_cnt].list_cnt;
        }
        else {
            s_tbl_con_cnt += g_line_list_st[loop_cnt].list_cnt;
        }
    }

    /*   コネクションリスト */
    for ( loop_cnt = s_tbl_con_top; loop_cnt < (s_tbl_con_top + s_tbl_con_cnt); loop_cnt++ ) {
        g_con_list[loop_cnt].station_sts_get = MDSO_CST_GET_NON;
        g_con_list[loop_cnt].connection_get = MDSO_CON_GET_NON;
    }

    /* 送信先選択範囲の確定 */
    if ( g_rcv_send_spec == MDSO_SEND_RANGE_CONNECTION ) {
        /* 送信先再選択要否による */
        if ( g_nw_info[g_denbun_lct_info_no].send_re_select_need 
               == DEF_DST_RE_SELECT_NEED_OFF ) {
            g_rcv_final_send_range = MDSO_SEND_RANGE_CONNECTION;
        }
        else {
            g_rcv_final_send_range = g_rcv_re_send_range;
        }
    }
    else {
        g_rcv_final_send_range = g_rcv_send_spec;
    }

    /* コネクション指定時 */
    if ( g_rcv_send_spec == MDSO_SEND_RANGE_CONNECTION ) {
        for ( loop_cnt = s_tbl_con_top; loop_cnt < (s_tbl_con_top + s_tbl_con_cnt); loop_cnt++ ) {
            if (( memcmp(g_rcv_if_id,
                         g_con_list[loop_cnt].interface_id,
                         sizeof(g_rcv_if_id))      == 0 ) &&
                ( memcmp(g_rcv_station_id,
                         g_con_list[loop_cnt].station_id,
                         sizeof(g_rcv_station_id)) == 0 ) &&
                ( memcmp(g_rcv_con_id,
                         g_con_list[loop_cnt].connection_id,
                         sizeof(g_rcv_con_id))    == 0  )) {
                if ( g_con_list[loop_cnt].connection_st == MDSO_CONNECT_STS_CONNECT_INT ) {
                    /* 局状態取得 */
                    s_ret = MDSO_station_sts_read( loop_cnt,
                                                   ch_station_sts );
                    if ( s_ret != MDSO_RET_OK ) {
                        return s_ret;
                    }
                    /* 電文送信時局状態判定 */
                    s_ret = NWM_NTE( g_data_kind,
                                     g_data_log_data_kind,
                                     ch_station_sts );
                    if( s_ret == DEF_HDE_SEND_OK ) {
                        s_set_con_no = loop_cnt;    /* 送信先コネクション確定 */
                        g_con_list[loop_cnt].connection_get = MDSO_CON_GET_DONE;
                    }
                }
                break;
            }
        }
    }

    /* ラウンドロビン判定 */
    if ( s_set_con_no == -1 ) {
        if ( g_rcv_final_send_range != MDSO_SEND_RANGE_CONNECTION ) {
            s_ret = MDSO_send_judge_rr ( &s_set_con_no );
            if ( s_ret != MDSO_RET_OK ) {
                return s_ret;
            }
        }
    }

    /* 最終判定 */
    if ( s_set_con_no == -1 ) {
        memcpy( g_internal_error_code,
                DEF_NERR_DST_SELECT_ERR,
                sizeof(g_internal_error_code));
        return MDSO_RET_NG;
    }

    /* 出力パラメータ設定 */
    *s_con_data_no  = s_set_con_no;

    return MDSO_RET_OK;

} /* end of MDSO_send_judge */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_send_judge_rr                                   */
/*  CALLING SEQ.    : short MDSO_send_judge_rr ( short *con_list_idx )            */
/*  ARGUMENT        : 1. con_list_idx  (O) コネクションリストのIDX                */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 送信先ラウンドロビン判定                                    */
/**********************************************************************************/
short MDSO_send_judge_rr ( short *con_list_idx )
{
    short   s_ret;
    short   s_tbl_st_idx     = 0;  /* 回線ラウンドロビンリスト(ステーション)処理中IDX    */
    short   s_tbl_st_top     = 0;  /* 回線ラウンドロビンリスト(ステーション)開始IDX      */
    short   s_tbl_st_cnt     = 0;  /* 回線ラウンドロビンリスト(ステーション)エレメント数 */
    short   s_tbl_con_top    = 0;  /* コネクションリスト開始IDX                          */
    short   s_tbl_con_cnt    = 0;  /* コネクションリストエレメント数                     */
    short   s_wk_st_top      = 0;
    short   s_wk_st_cnt      = 0;
    short   s_wk_st_rrb      = 0;
    short   s_wk_st_idx      = 0;
    short   s_wk_con_top     = 0;
    short   s_wk_con_cnt     = 0;
    short   s_wk_con_idx     = 0;
    short   s_wk_con_rrb     = 0;
    short   s_idx_st         = 0;
    short   s_idx_con        = 0;
    short   loop_cnt         = 0;
    short   loop_cnt2        = 0;
    char    ch_station_sts[2];
    short   s_set_con_no     = -1;

    s_tbl_st_top = g_line_list_if[g_line_list_if_no].list_top_no;
    s_tbl_st_cnt = g_line_list_if[g_line_list_if_no].list_cnt;

    if ( g_rcv_final_send_range == MDSO_SEND_RANGE_STATION ) {
        /* 送信先選択範囲＝ステーション */
        s_tbl_st_idx = -1;
        for ( loop_cnt = s_tbl_st_top;
              loop_cnt < (s_tbl_st_top + s_tbl_st_cnt);
              loop_cnt++ ) {
            if (( memcmp(g_rcv_if_id,
                         g_line_list_st[loop_cnt].interface_id,
                         sizeof(g_rcv_if_id))      == 0 ) &&
                ( memcmp(g_rcv_station_id,
                         g_line_list_st[loop_cnt].station_id,
                         sizeof(g_rcv_station_id)) == 0 )) {
                s_tbl_st_idx = loop_cnt;
                s_wk_con_top = g_line_list_st[loop_cnt].list_top_no;
                s_wk_con_cnt = g_line_list_st[loop_cnt].list_cnt;
                s_wk_con_rrb = g_line_list_st[loop_cnt].list_no;
                break;
            }
        }
        if ( s_tbl_st_idx != -1 ) {
            /* コネクションリスト検索 */
            s_idx_con = s_wk_con_rrb;
            for ( loop_cnt2 = 0; loop_cnt2 < s_wk_con_cnt; loop_cnt2++ ) {
                s_wk_con_idx = s_wk_con_top + s_idx_con;
                if (( g_con_list[s_wk_con_idx].connection_st 
                        == MDSO_CONNECT_STS_CONNECT_INT      ) &&
                    ( g_con_list[s_wk_con_idx].connection_get 
                        == MDSO_CON_GET_NON                  )) {
                    /* 局状態取得 */
                    s_ret = MDSO_station_sts_read( s_wk_con_idx,
                                                   ch_station_sts );
                    if ( s_ret != MDSO_RET_OK ) {
                        return s_ret;
                    }
                    /* 電文送信時局状態判定 */
                    s_ret = NWM_NTE( g_data_kind,
                                     g_data_log_data_kind,
                                     ch_station_sts );
                    if( s_ret == DEF_HDE_SEND_OK ) {
                        s_set_con_no = s_wk_con_idx;    /* 送信先コネクション確定 */
                        g_con_list[s_set_con_no].connection_get = MDSO_CON_GET_DONE;
                        break;
                    }
                }
                if ( s_idx_con < s_wk_con_cnt - 1 ) {
                    s_idx_con += 1;
                }
                else {
                    s_idx_con = 0;
                }
            }  /* End of for */
            /* ラウンドロビン番号(コネクション)更新 */
            /*   ※送信可能があった場合、確定コネクションの次 */
            /*     送信可能が無かった場合、現番号の次         */
            if ( s_set_con_no == -1 ) {
                s_idx_con = s_wk_con_rrb;
            }
            if ( s_idx_con < s_wk_con_cnt - 1 ) {
                g_line_list_st[s_tbl_st_idx].list_no = s_idx_con + 1;
            }
            else {
                g_line_list_st[s_tbl_st_idx].list_no = 0;
            }
        }
    }
    else if ( g_rcv_final_send_range == MDSO_SEND_RANGE_INTERFACE ) {
        /* 送信先選択範囲＝インタフェース */
        s_wk_st_rrb = g_line_list_if[g_line_list_if_no].list_no;
        s_idx_st = s_wk_st_rrb;
        for ( loop_cnt = 0; loop_cnt < s_tbl_st_cnt; loop_cnt++ ) {
            s_tbl_st_idx = s_tbl_st_top + s_idx_st;
            s_wk_con_top = g_line_list_st[s_tbl_st_idx].list_top_no;
            s_wk_con_cnt = g_line_list_st[s_tbl_st_idx].list_cnt;
            s_wk_con_rrb = g_line_list_st[s_tbl_st_idx].list_no;
            s_idx_con = s_wk_con_rrb;
            for ( loop_cnt2 = 0; loop_cnt2 < s_wk_con_cnt; loop_cnt2++ ) {
                s_wk_con_idx = s_wk_con_top + s_idx_con;
                if (( g_con_list[s_wk_con_idx].connection_st 
                        == MDSO_CONNECT_STS_CONNECT_INT      ) &&
                    ( g_con_list[s_wk_con_idx].connection_get 
                        == MDSO_CON_GET_NON                  )) {
                    /* 局状態取得 */
                    s_ret = MDSO_station_sts_read( s_wk_con_idx,
                                                   ch_station_sts );
                    if ( s_ret != MDSO_RET_OK ) {
                        return s_ret;
                    }
                    /* 電文送信時局状態判定 */
                    s_ret = NWM_NTE( g_data_kind,
                                     g_data_log_data_kind,
                                     ch_station_sts );
                    if( s_ret == DEF_HDE_SEND_OK ) {
                        s_set_con_no = s_wk_con_idx;    /* 送信先コネクション確定 */
                        g_con_list[s_set_con_no].connection_get = MDSO_CON_GET_DONE;
                        break;
                    }
                }
                if ( s_idx_con < s_wk_con_cnt - 1 ) {
                    s_idx_con += 1;
                }
                else {
                    s_idx_con = 0;
                }
            }  /* End of for(loo_cnt2) */
            /* ラウンドロビン番号(コネクション)更新 */
            /*   ※送信可能があった場合、確定コネクションの次 */
            /*     送信可能が無かった場合、現番号の次         */
            if ( s_set_con_no == -1 ) {
                s_idx_con = s_wk_con_rrb;
            }
            if ( s_idx_con < s_wk_con_cnt - 1 ) {
                g_line_list_st[s_tbl_st_idx].list_no = s_idx_con + 1;
            }
            else {
                g_line_list_st[s_tbl_st_idx].list_no = 0;
            }
            /* ループ継続判定 */
            if ( s_set_con_no != -1 ) {
                break;
            }
            if ( s_idx_st < s_tbl_st_cnt - 1 ) {
                s_idx_st += 1;
            }
            else {
                s_idx_st = 0;
            }
        }   /* End of for(loo_cnt) */
        /* ラウンドロビン番号(ステーション)更新 */
        if ( s_wk_st_rrb < s_tbl_st_cnt - 1 ) {
            g_line_list_if[g_line_list_if_no].list_no = s_wk_st_rrb + 1;
        }
        else {
            g_line_list_if[g_line_list_if_no].list_no = 0;
        }
    }

    if ( s_set_con_no == -1 ) {
        memcpy( g_internal_error_code,
                DEF_NERR_DST_SELECT_ERR,
                sizeof(g_internal_error_code));
        return MDSO_RET_NG;
    }

    *con_list_idx = s_set_con_no;

    return MDSO_RET_OK;

} /* end of MDSO_send_judge_rr */

/******************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_encode                                      */
/*  CALLING SEQ.    : short MDSO_encode ( c302_def *ch_rcv_data_adr)          */
/*  ARGUMENT        : 1. ch_rcv_data_adr   (I) 受信電文                       */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                  */
/*  DESCRIPTION     : 暗号化処理                                              */
/******************************************************************************/
short MDSO_encode ( c302_def *ch_rcv_data_adr)
{
    short s_ret;
    char  ch_data_lengset[5];
    char  rcv_data[MAX_TEXT_BUF_LEN];
    db_gqnwq_def *ch_rcv_que_data = (db_gqnwq_def*)ch_rcv_data_adr->msg_data;

    /* 個別モジュールパラメータ*/
    NWM_ENC_arg_2_def t_NWM_ENC_arg_2_def;
    NWM_ENC_arg_3_def t_NWM_ENC_arg_3_def;
    NWM_ENC_arg_4_def t_NWM_ENC_arg_4_def;
    NWM_ENC_arg_5_def t_NWM_ENC_arg_5_def;
    char ch_module_id[8+1];

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_ENC_arg_2_def, 0, sizeof(t_NWM_ENC_arg_2_def) );
    memset( &t_NWM_ENC_arg_3_def, 0, sizeof(t_NWM_ENC_arg_3_def) );
    memset( &t_NWM_ENC_arg_4_def, 0, sizeof(t_NWM_ENC_arg_4_def) );
    memset( &t_NWM_ENC_arg_5_def, 0, sizeof(t_NWM_ENC_arg_5_def) );
    memset( ch_module_id, 0, sizeof(ch_module_id) );
    memcpy( ch_module_id, g_myinfo_def.proc_data_sub.module_id, sizeof(ch_module_id) );

    /* グローバル領域初期化 */
    memset( &g_encdec_con.enc_conf_data, 0, sizeof(g_encdec_con.enc_conf_data) );

    /* 受信電文初期化 */
    memset( rcv_data, 0, sizeof(rcv_data));

    /* 暗号化処理 */
    memcpy(t_NWM_ENC_arg_2_def.file_id,
           g_encdec_con.enc_start_data.key_file_id,
           sizeof(t_NWM_ENC_arg_2_def.file_id));
    memcpy(t_NWM_ENC_arg_2_def.file_name,
           g_encdec_con.enc_start_data.key_file_name,
           sizeof(t_NWM_ENC_arg_2_def.file_name));
    t_NWM_ENC_arg_2_def.file_no            = g_encdec_con.enc_start_data.key_file_no;
    t_NWM_ENC_arg_2_def.io_timer           = g_myinfo_def.config_data.io_timer;

    memcpy( t_NWM_ENC_arg_3_def.domain_name, g_encdec_con.enc_start_data.pathmon_name,
               sizeof(t_NWM_ENC_arg_3_def.domain_name) );
    memcpy( t_NWM_ENC_arg_3_def.server_name, g_encdec_con.enc_start_data.server_class,
               sizeof(t_NWM_ENC_arg_3_def.server_name) );
    t_NWM_ENC_arg_3_def.pathsend_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_NWM_ENC_arg_3_def.pathsend_retry_cnt = (short)g_myinfo_def.config_data.send_retry_count;

    t_NWM_ENC_arg_4_def.site_id            = g_myinfo_def.config_data.site_id;
    t_NWM_ENC_arg_4_def.nw_id              = g_myinfo_def.config_data.network_id;
    memcpy( t_NWM_ENC_arg_4_def.grp_id, g_myinfo_def.config_data.group_id,
               sizeof(t_NWM_ENC_arg_4_def.grp_id) );
    memcpy( t_NWM_ENC_arg_4_def.if_id,
            g_rcv_if_id,
            sizeof(t_NWM_ENC_arg_4_def.if_id) );
    memcpy( t_NWM_ENC_arg_4_def.station_id,
            g_rcv_station_id,
            sizeof(t_NWM_ENC_arg_4_def.station_id) );

    memcpy( ch_data_lengset, ch_rcv_que_data->denbun_area.denbun_len, 4);
    ch_data_lengset[4] = 0;
    g_make_data_tbl.rcv_data_enc_set.make_data_length = (short)atoi(ch_data_lengset);

    memcpy( g_make_data_tbl.rcv_data_enc_set.make_data,
            ch_rcv_que_data->denbun_area.denbun,
            g_make_data_tbl.rcv_data_enc_set.make_data_length );

    t_NWM_ENC_arg_5_def.before_msg = g_make_data_tbl.rcv_data_enc_set.make_data;
    t_NWM_ENC_arg_5_def.before_len = g_make_data_tbl.rcv_data_enc_set.make_data_length;
    t_NWM_ENC_arg_5_def.after_msg  = g_make_data_tbl.encode_data.make_data;
    t_NWM_ENC_arg_5_def.after_len  = 0;

    /* 個別モジュール */
    s_ret = NWM_ENC(MDSO_ENCODE,
                    &t_NWM_ENC_arg_2_def,
                    &t_NWM_ENC_arg_3_def,
                    &t_NWM_ENC_arg_4_def,
                    &t_NWM_ENC_arg_5_def,
                    ch_module_id);
    /* 個別モジュール結果判定 */
    if ( s_ret >= 2 ) {
        switch(s_ret){
        case DEF_NWM_ENC_RTN_NG_KC:
            memcpy( g_internal_error_code,
                    DEF_NERR_KC_ENC_ERR,
                    sizeof(g_internal_error_code));
            break;
        case DEF_NWM_ENC_RTN_NG_KMAC:
            memcpy( g_internal_error_code,
                    DEF_NERR_KMAC_CALC_ERR,
                    sizeof(g_internal_error_code));
            break;
        case DEF_NWM_ENC_RTN_NG_AUTHORI:
        case DEF_NWM_ENC_RTN_NG_DIGITS:
        case DEF_NWM_ENC_RTN_NG_ATALLA:
        case DEF_NWM_ENC_RTN_NG_IO:
        default:
            memcpy( g_internal_error_code,
                    DEF_NERR_DENBUN_ENC_ERR,
                    sizeof(g_internal_error_code));
            break;
        }

        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            g_internal_error_code,
                            "@X@E", "NWM_ENC", s_ret);

       return MDSO_RET_NG;
    }

    /* 暗号化結果格納処理 */
    /* 暗号化後データ格納 */
    g_make_data_tbl.encode_data.make_data_length = t_NWM_ENC_arg_5_def.after_len;

    /* 暗号化前電文保存(チェックデジット更新済み) */
    g_make_data_tbl.rcv_data_enc_set.make_data_length = t_NWM_ENC_arg_5_def.before_len;

    /* 正常終了 */
    return MDSO_RET_OK;

} /* end of MDSO_encode */

/******************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_header_edit                                 */
/*  CALLING SEQ.    : short MDSO_header_edit ( void )                         */
/*  ARGUMENT        : void                                                    */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                  */
/*  DESCRIPTION     : 電文ヘッダ編集処理                                      */
/******************************************************************************/
short MDSO_header_edit( void )
{
    /* 個別モジュールパラメータ*/
    NWM_HDE_arg_1_def t_NWM_HDE_arg_1_def;
    NWM_HDE_arg_2_def t_NWM_HDE_arg_2_def;

    /********************/
    /* 暗号化前電文編集 */
    /********************/

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_HDE_arg_1_def, 0, sizeof(t_NWM_HDE_arg_1_def) );
    memset( &t_NWM_HDE_arg_2_def, 0, sizeof(t_NWM_HDE_arg_2_def) );

    /* グローバル領域初期化 */
    memset( g_make_data_tbl.rcv_make_data.make_data, 0, sizeof(g_make_data_tbl.rcv_make_data.make_data) );

    /* 電文ヘッダ編集処理 */
    t_NWM_HDE_arg_1_def.msg_in_addr  = g_make_data_tbl.rcv_data_enc_set.make_data;
    t_NWM_HDE_arg_1_def.msg_in_len   = g_make_data_tbl.rcv_data_enc_set.make_data_length;

    t_NWM_HDE_arg_2_def.msg_out_addr = g_make_data_tbl.rcv_make_data.make_data;
    t_NWM_HDE_arg_2_def.msg_out_len  = 0;

    /* 個別モジュール */
    NWM_HDE(&t_NWM_HDE_arg_1_def,
            &t_NWM_HDE_arg_2_def,
            g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct,
            g_data_kind);

    g_make_data_tbl.rcv_make_data.make_data_length = t_NWM_HDE_arg_2_def.msg_out_len;

    /********************/
    /* 暗号化後電文編集 */
    /********************/

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_HDE_arg_1_def, 0, sizeof(t_NWM_HDE_arg_1_def) );
    memset( &t_NWM_HDE_arg_2_def, 0, sizeof(t_NWM_HDE_arg_2_def) );

    /* グローバル領域初期化 */
    memset( g_make_data_tbl.send_data.make_data, 0, sizeof(g_make_data_tbl.send_data.make_data) );

    /* 電文ヘッダ編集処理 */
    t_NWM_HDE_arg_1_def.msg_in_addr  = g_make_data_tbl.encode_data.make_data;
    t_NWM_HDE_arg_1_def.msg_in_len   = g_make_data_tbl.encode_data.make_data_length;

    t_NWM_HDE_arg_2_def.msg_out_addr = g_make_data_tbl.send_data.make_data;
    t_NWM_HDE_arg_2_def.msg_out_len  = 0;

    /* 個別モジュール */
    NWM_HDE(&t_NWM_HDE_arg_1_def,
            &t_NWM_HDE_arg_2_def,
            g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct,
            g_data_kind);

    g_make_data_tbl.send_data.make_data_length = t_NWM_HDE_arg_2_def.msg_out_len;

    /* 正常終了 */
    return MDSO_RET_OK;

} /* end of MDSO_header_edit */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_data_make                                       */
/*  CALLING SEQ.    : short MDSO_data_make ( c302_def *rcv_data_adr,              */
/*                                           t_con_list *con_tbl,                 */
/*                                           c202_def *send_data_adr)             */
/*  ARGUMENT        : 1. rcv_data_adr    (I) 受信データ                           */
/*                  : 2. con_tbl         (I) 送信先コネクションリスト             */
/*                  : 3. send_data_adr   (O) 送信電文作成領域                     */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 送信電文作成処理                                            */
/**********************************************************************************/
short MDSO_data_make ( c302_def *rcv_data_adr,
                       t_con_list *con_tbl,
                       c202_def *send_data_adr )
{
    c202_def     c202_send_data;
    db_gqnwq_def *c302_que_file = (db_gqnwq_def *)rcv_data_adr->msg_data;

    /* 送信電文設定領域クリア */
    memset( &c202_send_data, 0, sizeof(c202_send_data) );

    /* IPC interface_code設定 (C202) */
    memcpy( c202_send_data.common_header.interface_code,
                       DEF_IPC_IFCD_DEN_SND_REQ, 4 );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( c202_send_data.common_header.internal_error_code, MDSO_SPACE,
            sizeof(c202_send_data.common_header.internal_error_code));
    memset( c202_send_data.common_header.filler_1, MDSO_SPACE,
            sizeof(c202_send_data.common_header.filler_1));

    /* IPC サイト識別、NW識別、グループ識別設定 */
    memcpy( &c202_send_data.text_send_info.recv_con_id,
            &c302_que_file->tushin_cntrl_info.line_info.recv_connect_id,
            sizeof(c202_send_data.text_send_info.recv_con_id));

    /* IPC サイト識別設定 */
    c202_send_data.text_send_info.recv_con_id.site_name = g_myinfo_def.config_data.site_id;

    /* IPC NW識別設定 */
    c202_send_data.text_send_info.recv_con_id.nw_name = g_myinfo_def.config_data.network_id;

    /* IPC グループ識別設定 */
    memcpy( c202_send_data.text_send_info.recv_con_id.group_name,
            g_myinfo_def.config_data.group_id,
            sizeof(c202_send_data.text_send_info.recv_con_id.group_name));

    /* IPC インタフェース識別設定 */
    memcpy( c202_send_data.text_send_info.recv_con_id.interface_name,
            g_rcv_if_id,
            sizeof(c202_send_data.text_send_info.recv_con_id.interface_name));

    /* IPC ステーション識別設定 */
    memcpy( c202_send_data.text_send_info.recv_con_id.station_name,
            con_tbl->station_id,
            sizeof(c202_send_data.text_send_info.recv_con_id.station_name));

    /* IPC コネクション識別設定 */
    memcpy( c202_send_data.text_send_info.recv_con_id.connection_name,
            con_tbl->connection_id,
            sizeof(c202_send_data.text_send_info.recv_con_id.connection_name));

    /* IPC GFP内部LCN */
    memcpy( c202_send_data.text_log_key.transaction_id.gfp_lcn,
            &c302_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            sizeof(c202_send_data.text_log_key.transaction_id.gfp_lcn) );

    /* IPC 電文送受信情報 電文ログKEY 電文形態 */
    c202_send_data.text_log_key.transaction_id.text_format =
        c302_que_file->denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai;

    /* IPC 電文送受信情報 電文ログKEY 電文種別 */
    c202_send_data.text_log_key.text_type =
        c302_que_file->denbun_send_recv_info.denbun_log_key.denbun_shubetu;

    /* IPC 電文送受信情報 電文ログKEY 再送回数 */
    memcpy( c202_send_data.text_log_key.retry_count,
            c302_que_file->denbun_send_recv_info.denbun_log_key.re_send_num, 3 );

    /* IPC 電文送受信情報 電文送信要求元プロセス論理KEY プロセス論理名 */
    memcpy( c202_send_data.src_prc_lgc_key.prc_id.prc_kind,
            g_myinfo_def.proc_data.my_pname,
            sizeof(c202_send_data.src_prc_lgc_key.prc_id.prc_kind));

    /* IPC 電文送受信情報 電文送信要求元プロセス論理KEY プロセス論理番号 */
    memcpy( c202_send_data.src_prc_lgc_key.prc_id.prc_num,
            g_myinfo_def.proc_data_sub.my_prcno,
            sizeof(c202_send_data.src_prc_lgc_key.prc_id.prc_num));

    /* IPC 電文送受信情報 電文送信要求元プロセス論理KEY プロセス冗長化番号 */
    memcpy( c202_send_data.src_prc_lgc_key.prc_mlt_num,
            g_myinfo_def.proc_data_sub.my_prcmlt,
            sizeof(c202_send_data.src_prc_lgc_key.prc_mlt_num));

    /* IPC 電文送受信情報 予備 スペース設定 */
    memset( c202_send_data.filler_1, ' ', sizeof(c202_send_data.filler_1));

    /* IPC 送受信電文設定 */
    memcpy( c202_send_data.msg_info.msg_data,
            g_make_data_tbl.send_data.make_data,
            g_make_data_tbl.send_data.make_data_length );

    /* データ長保存 */
    g_myinfo_def.data_len = g_make_data_tbl.send_data.make_data_length;

    /* IPC DATA部電文長設定 */
    c202_send_data.msg_info.msg_len = g_make_data_tbl.send_data.make_data_length;

    /* IPC 送受信電文全体長設定 */
    c202_send_data.common_header.control_data_length =
          (unsigned short)((sizeof(c202_def) - 9999 - sizeof(common_header_def))
                          + c202_send_data.msg_info.msg_len);

    /* C202データ設定 */
    memcpy( send_data_adr, &c202_send_data, sizeof(c202_def) );

    return MDSO_RET_OK;

} /* end of MDSO_data_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_log_output                                */
/*  CALLING SEQ.    : short MDSO_log_output ( c302_def *rcvdata,            */
/*                                            c202_def *senddata,           */
/*                                            char *c_log_file_name )       */
/*  ARGUMENT        : 1. rcvdata          (I) ログ出力対象受信電文          */
/*                  : 2. senddata         (I) ログ出力対象送信電文          */
/*                  : 3. c_log_file_name  (O) 出力先ログファイル名          */
/*  RETURN CODE     : 0:正常終了 1:二重送信 -1:異常終了                     */
/*  DESCRIPTION     : 電文ログ出力処理（二重送信判定）                      */
/****************************************************************************/
short MDSO_log_output ( c302_def *rcvdata,
                        c202_def *senddata,
                        char *c_log_file_name )
{
    short    s_ret = 0;

    c601_def st_log_ipc;         /* ログ出力要求IPC設定テーブル */
    r601_def *st_log_rcv_ipc;    /* ログ出力応答IPC設定テーブル */

    db_gqnwq_def *c302_rcv_que = (db_gqnwq_def *)rcvdata->msg_data;
    char ch_datalen_local[6];

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* タイムスタンプパラメータ */
    COM_UNQ_arg_1_def t_COM_UNQ_arg_1_def;
    char ch_datetime_hex[16+1];

    /* ログ出力要求IPC設定テーブル初期化 */
    memset( &st_log_ipc, 0, sizeof(st_log_ipc) );

    /* IPC interface_code設定 */
    memcpy( st_log_ipc.common_header.interface_code, DEF_IPC_IFCD_LG_OUT_REQ_DEN_REQ, 4 );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( st_log_ipc.common_header.internal_error_code, MDSO_SPACE,
            sizeof(st_log_ipc.common_header.internal_error_code));
    memset( st_log_ipc.common_header.filler_1, MDSO_SPACE,
            sizeof(st_log_ipc.common_header.filler_1));

    /* IPC ログファイル名にSPACEを設定 */
    memset( st_log_ipc.logfile_id, MDSO_SPACE,
            sizeof(st_log_ipc.logfile_id));

    /* IPC 電文送受信情報 通信ログ保存ファイル名 */
    memcpy( st_log_ipc.logfile_id,
            c302_rcv_que->denbun_send_recv_info.tushin_log_save_filename,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename));

    /* IPC 登録/更新区分設定 */
    st_log_ipc.entry_update_cate = MDSO_LOG_SET;

    /* タイムスタンプ取得 */
    memset( (char *)&t_COM_UNQ_arg_1_def, 0, sizeof(t_COM_UNQ_arg_1_def));
    memset( ch_datetime_hex, 0, sizeof(ch_datetime_hex));
    COM_UNQ( &t_COM_UNQ_arg_1_def, ch_datetime_hex );

    /* IPC プライマリーキー設定 */
    memcpy( &st_log_ipc.t_glnlg.pri_key.part_id[0],
            &t_COM_UNQ_arg_1_def.cc[1],
            2 );

    /* IPC プライマリーキー 電文受信時刻設定 */
    memcpy( st_log_ipc.t_glnlg.pri_key.tushin_denbun_id.time_stamp,
            &t_COM_UNQ_arg_1_def,
            sizeof(st_log_ipc.t_glnlg.pri_key.tushin_denbun_id.time_stamp) );

    memcpy( st_log_ipc.t_glnlg.pri_key.tushin_denbun_id.time_stamp_branch,
            ch_datetime_hex,
            sizeof(st_log_ipc.t_glnlg.pri_key.tushin_denbun_id.time_stamp_branch) );

    /* IPC 送受信識別設定 (送信電文="2") */
    st_log_ipc.t_glnlg.send_recv_id = MDSO_SEND_DATA;

    /* IPC 送受信電文長設定 */
    memset(ch_datalen_local, 0, sizeof(ch_datalen_local));
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%05d",
             senddata->msg_info.msg_len );
    memcpy( st_log_ipc.t_glnlg.send_recv_denbun_len,
            ch_datalen_local,
            sizeof(st_log_ipc.t_glnlg.send_recv_denbun_len));

    /* MTI設定 */
    memcpy( st_log_ipc.t_glnlg.mti_id,
            c302_rcv_que->denbun_send_recv_info.mti_id,
            sizeof(st_log_ipc.t_glnlg.mti_id) );

    /* IPC メッセージ種別設定 スペース設定 */
    memset( st_log_ipc.t_glnlg.msg_shubetu, ' ', 
            sizeof(st_log_ipc.t_glnlg.msg_shubetu) );

    /* IPC コネクション論理ID設定 */
    memcpy( &st_log_ipc.t_glnlg.connect_id,
            &senddata->text_send_info.recv_con_id,
            sizeof(st_log_ipc.t_glnlg.connect_id) );

    /* IPC 東阪振分情報 スペース設定 */
    memset( &st_log_ipc.t_glnlg.furiwake_info, ' ',
             sizeof(st_log_ipc.t_glnlg.furiwake_info));

    /* IPC 電文送受信情報 電文受信時刻設定 */
    memcpy( &st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_recv_time[0],
            &c302_rcv_que->denbun_send_recv_info.denbun_recv_time[0],
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_recv_time) );

    /* IPC 電文送受信情報 受信時局状態 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.recv_kyoku_sts =
               c302_rcv_que->denbun_send_recv_info.recv_kyoku_sts;

    /* IPC 電文送受信情報 NW区分設定 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.nw_kubun,
            c302_rcv_que->denbun_send_recv_info.nw_kubun,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.nw_kubun) );

    /* IPC 電文送受信情報 MTI設定 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id,
            c302_rcv_que->denbun_send_recv_info.mti_id,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id) );

    /* IPC 電文送受信情報 送信電文種別 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.send_denbun_shubetu =
        c302_rcv_que->denbun_send_recv_info.send_denbun_shubetu;

    /* IPC 電文送受信情報 電文ログKEY GFP内部LCN */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            &c302_rcv_que->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id) );

    /* IPC 電文送受信情報 電文ログKEY 電文形態 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai = 
      c302_rcv_que->denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai;

    /* IPC 電文送受信情報 電文ログKEY 電文種別 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.denbun_shubetu =
      c302_rcv_que->denbun_send_recv_info.denbun_log_key.denbun_shubetu;

    /* IPC 電文送受信情報 電文ログKEY 再送回数 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.re_send_num,
            c302_rcv_que->denbun_send_recv_info.denbun_log_key.re_send_num,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.re_send_num) );

    /* IPC 電文送受信情報 電文フォーマット区分 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_fmt_kubun =
      c302_rcv_que->denbun_send_recv_info.denbun_fmt_kubun;

    /* IPC 電文送受信情報 通信ログ保存ファイル名 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename,
            c302_rcv_que->denbun_send_recv_info.tushin_log_save_filename,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename) );

    /* IPC 電文送受信情報 通信ログKEY */
    memcpy( &st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_key.part_id[0],
            &c302_rcv_que->denbun_send_recv_info.tushin_log_key.part_id[0],
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_key) );

    /* ダミー領域スペース設定 */
    memset( st_log_ipc.t_glnlg.furiwake_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.furiwake_info.future_use));
    memset( st_log_ipc.t_glnlg.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.future_use));
    memset( st_log_ipc.t_glnlg.denbun_send_recv_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.future_use));

    /* IPC 通信制御情報 */
    memcpy( &st_log_ipc.t_glnlg.tushin_cntrl_info,
            &c302_rcv_que->tushin_cntrl_info,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info) );

    memset( st_log_ipc.t_glnlg.tushin_cntrl_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.future_use));

    /* IPC 電文長設定 */
    memset(ch_datalen_local, 0, sizeof(ch_datalen_local));
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%05d",
             g_make_data_tbl.rcv_make_data.make_data_length );

    memcpy( st_log_ipc.t_glnlg.denbun_area.denbun_len,
            ch_datalen_local,
            sizeof(st_log_ipc.t_glnlg.denbun_area.denbun_len));

    /* IPC MTI開始位置設定 */
    st_log_ipc.t_glnlg.denbun_area.mti_start_lct[0] = '0';
    memcpy( &st_log_ipc.t_glnlg.denbun_area.mti_start_lct[1],
            c302_rcv_que->denbun_area.mti_start_lct,
            sizeof(c302_rcv_que->denbun_area.mti_start_lct));

    /* IPC 送受信電文設定 */
    memcpy( st_log_ipc.t_glnlg.denbun_area.denbun,
            g_make_data_tbl.rcv_make_data.make_data,
            g_make_data_tbl.rcv_make_data.make_data_length);

    /* IPC 送受信電文全体長設定 */
    st_log_ipc.common_header.control_data_length =
         sizeof(st_log_ipc) -
         sizeof(st_log_ipc.common_header) -
         sizeof(st_log_ipc.t_glnlg.denbun_area.denbun) +
         g_make_data_tbl.rcv_make_data.make_data_length;

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_3_def, MDSO_SPACE, sizeof(t_COM_PSD_arg_3_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           MDSO_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));

    if (g_logcon_data.domain_name[0] != MDSO_SPACE){
        /* ドメイン名設定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.domain_name,
                                     sizeof(g_logcon_data.domain_name));
    }
    else {
        /* PATHMON名指定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.pathmon_name,
                                     sizeof(g_logcon_data.pathmon_name));
    }

    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_logcon_data.server_class,
                       sizeof(g_logcon_data.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &st_log_ipc, sizeof(st_log_ipc) );

    t_COM_PSD_arg_1_def.receive_max_len = sizeof(r601_def);

    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_logcon_data.pathsend_timer;

    t_COM_PSD_arg_1_def.retry_cnt = g_logcon_data.retry_cnt;

    memcpy(t_COM_PSD_arg_2_def.prog_id,
           g_myinfo_def.proc_data_sub.module_id,
           sizeof(t_COM_PSD_arg_2_def.prog_id) );

    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_logcon_data.server_class,
                       sizeof(g_logcon_data.server_class));

    t_COM_PSD_arg_1_def.req_send_len
              = st_log_ipc.common_header.control_data_length
                      + sizeof(st_log_ipc.common_header);

    /* サーバークラス論理ID設定 */
    memcpy( t_COM_PSD_arg_4_def.srv_logical_id,
            g_myinfo_def.config_data.serverclass_name,
            sizeof(g_myinfo_def.config_data.serverclass_name));

  /* GFP内部LCN設定 */
    memcpy( t_COM_PSD_arg_4_def.lcn,
            g_lcn,
            sizeof(g_lcn));

    /* PATHSEND共通処理実行 */
    s_ret = COM_PSD( &t_COM_PSD_arg_1_def,
                     &t_COM_PSD_arg_2_def,
                     &t_COM_PSD_arg_3_def,
                     &g_cg010in_modle,
                     &t_COM_PSD_arg_4_def );

    /* PATHSEND結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_PSD", s_ret);

        memcpy( g_internal_error_code,
                DEF_NERR_PSEND_ERR_RE_OUT,
                sizeof(g_internal_error_code));
        return MDSO_RET_NG;
    }

    st_log_rcv_ipc =  (r601_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R601) */
    if ( memcmp(st_log_rcv_ipc->common_header.interface_code,
                DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
                sizeof(st_log_rcv_ipc->common_header.interface_code)) != 0 ) {
        /* EMS出力 応答エラー */
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_FILE_IO_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R601 ERROR          ",
                            st_log_rcv_ipc);
        return MDSO_RET_NG;
    }

    /* PATHSEND結果確認 */
    /* ログファイル名を受信バッファから取得 */
    memcpy( c_log_file_name, (char *)st_log_rcv_ipc->logfile_id,
            sizeof(st_log_rcv_ipc->logfile_id) );

    if ( MDSO_RET_LCN_DOUBLE == st_log_rcv_ipc->common_header.error_code ) {
        /* 二重送信 */
        return MDSO_RET_DOUBLE_SET;
    }
    else if ( MDSO_RET_OK != st_log_rcv_ipc->common_header.error_code ) {
        /* 異常終了 */
        /* EMS出力 応答エラー */
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_FILE_IO_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R601 ERROR          ",
                            &st_log_rcv_ipc);
        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_log_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_data_con_send                             */
/*  CALLING SEQ.    : short MDSO_data_con_send( t_con_list *con_list_tbl,   */
/*                                              c202_def *send_deta_adr)    */
/*  ARGUMENT        : 1. con_list_tbl     (I) 送信先コネクション制御情報    */
/*                  : 2. send_deta_adr    (I) 送信電文のアドレス            */
/*  RETURN CODE     : 0:正常終了 1:迂回不可応答 -1:迂回不可エラー終了       */
/*  DESCRIPTION     : 電文送信要求処理                                      */
/****************************************************************************/
short MDSO_data_con_send (  t_con_list *con_list_tbl,
                            c202_def *send_deta_adr)
{
    short s_ret;
    short loop_cnt              = 0;
    short s_psd_retry_cnt_prm   = 0;    /* PATHSENDリトライ回数パラメータ    */
    short s_psd_retry_cnt       = 0;    /* PATHSENDリトライ回数              */
    short s_psd_retry_flg       = 0;    /* PATHSENDリトライ制御フラグ        */
    short s_con_retry_cnt_prm   = 0;    /* コネクション再選択リトライ回数パラメータ */
    short s_con_retry_cnt       = 0;    /* コネクション再選択リトライ回数    */
    short s_con_retry_flg       = 0;    /* コネクション再選択リトライフラグ  */
    short s_con_retry_judge     = 0;    /* コネクション再選択リトライ判定    */
    short s_con_data_no         = -1;
    t_con_list *con_list_tbl_save = con_list_tbl;

    c202_def send_data_save;
    r202_def *r202_data_save;

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    memset( &send_data_save, 0, sizeof(send_data_save) );
    memcpy( &send_data_save, send_deta_adr, sizeof(send_data_save) );

    s_psd_retry_cnt_prm = (short)g_myinfo_def.config_data.send_retry_count;
    s_con_retry_cnt_prm = MDSO_CON_RETRY_CNT;

    s_con_retry_cnt = 0;
    s_con_retry_flg = 0;
    while ( s_con_retry_flg == 0 ) {
        s_con_retry_judge = 0;

        /* PATHSENDパラメータ初期化 */
        memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
        memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
        memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
        memset( &t_COM_PSD_arg_4_def, MDSO_SPACE, sizeof(t_COM_PSD_arg_4_def) );

        /* PATHSEND用情報設定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               con_list_tbl_save->pathmon_name,
               sizeof(t_COM_PSD_arg_1_def.pathmon_name));

        /* サーバークラス論理ID */
        memset(t_COM_PSD_arg_1_def.serverclass_name, ' ',
               sizeof(t_COM_PSD_arg_1_def.serverclass_name));
        memcpy(t_COM_PSD_arg_1_def.serverclass_name,
               con_list_tbl_save->server_class_phy,
               sizeof(con_list_tbl_save->server_class_phy));

        t_COM_PSD_arg_1_def.receive_max_len = MDSO_MAX_DATA_SIZE;
        t_COM_PSD_arg_1_def.send_timer_msec = (long)g_myinfo_def.config_data.send_timer;
        t_COM_PSD_arg_1_def.retry_cnt       = 0;

        memcpy(t_COM_PSD_arg_2_def.prog_id,
               g_myinfo_def.proc_data_sub.module_id,
               sizeof(t_COM_PSD_arg_2_def.prog_id) );

        t_COM_PSD_arg_1_def.req_send_len = 
           send_deta_adr->common_header.control_data_length 
           + sizeof(send_deta_adr->common_header);

        /* サーバークラス論理ID設定 */
        memcpy( t_COM_PSD_arg_4_def.srv_logical_id,
                g_myinfo_def.config_data.serverclass_name,
                sizeof(g_myinfo_def.config_data.serverclass_name));

        /* GFP内部LCN設定 */
        memcpy( t_COM_PSD_arg_4_def.lcn,
                g_lcn,
                sizeof(g_lcn));

        /* PATHSEND処理                                                                 */
        /*   接続先への二重送信を避けるため、PATHSENDリトライを制御。                   */
        /*   コネクション制御は送信要求PATHSENDに対するリプライ後に回線へsendするため、 */
        /*   PATHSENDエラーを検知する状況では回線へのsendは実行されていないと思われる。 */
        /*   ただし、PATHSENDタイムアウトだけはコネクション制御の処理がどこまで実行     */
        /*   されているか分からないためリトライしない。                                 */
        s_psd_retry_cnt = 0;
        s_psd_retry_flg = 0;
        while ( s_psd_retry_flg == 0 ) {
            memcpy(t_COM_PSD_arg_1_def.msg_buf, &send_data_save, sizeof(send_data_save));
            t_COM_PSD_arg_1_def.receive_len = 0;
            t_COM_PSD_arg_3_def.guardian_errcode = 0;
            t_COM_PSD_arg_3_def.pathsend_errcode = 0;

            /* PATHSEND共通処理実行 */
            s_ret = COM_PSD( &t_COM_PSD_arg_1_def,
                             &t_COM_PSD_arg_2_def,
                             &t_COM_PSD_arg_3_def,
                             &g_cg010in_modle,
                             &t_COM_PSD_arg_4_def );
            if ( s_ret == MDSO_RET_OK ) {
                s_psd_retry_flg = 1;
            }
            else {
                if ( t_COM_PSD_arg_3_def.guardian_errcode == MDSO_GERR_TIMEOUT ) {
                    s_psd_retry_flg = 1;
                }
                else {
                    if ( s_psd_retry_cnt < s_psd_retry_cnt_prm ) {
                        s_psd_retry_cnt += 1;
                    }
                    else {
                        s_psd_retry_flg = 1;
                    }
                }
            }
        }  /* End of while (PATHSENDリトライ) */

        /* 電文送信応答判定 */
        if ( s_ret == MDSO_RET_OK ) {
            r202_data_save = (r202_def*)t_COM_PSD_arg_1_def.msg_buf;
            /* 電文送信応答のインタフェースコード判定(R202) */
            if ( memcmp(r202_data_save->common_header.interface_code,
                        DEF_IPC_IFCD_DEN_SND_RSP,
                        sizeof(send_data_save.common_header.interface_code)) == 0 ) {

                /* 電文送信応答のエラーコード判定 */
                if (r202_data_save->common_header.error_code != DEF_IPC_ERRCD_OK) {
                    /* 異常時の迂回処理実行 */
                    s_con_retry_judge = 1;    /* コネクション再選択リトライ実施 */
                    s_ret = MDSO_RET_NG;
                }
            }
            else {
                /* 異常時の迂回処理実行 */
                s_ret = MDSO_RET_NG;
            }
            if ( s_ret == MDSO_RET_NG ) {
                /* EMS出力 応答エラー */
                MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_CON_SEND_ERR , "@L@X@X@i",
                                    g_lcn, g_myinfo_def.config_data.serverclass_name,
                                    "R202 ERROR          ",
                                    r202_data_save);
            }
        }

        /* 処理結果判定 */
        if ( s_ret == MDSO_RET_OK ) {
            s_con_retry_flg = 1;
        }
        else {
            if ( s_con_retry_judge == 1 ) {
                if ( s_con_retry_cnt < s_con_retry_cnt_prm ) {
                    s_con_retry_cnt += 1;
                    s_ret = MDSO_send_judge_rr ( &s_con_data_no );
                    if ( s_ret == MDSO_RET_OK ) {
                        /* 送信先選択済み */
                        con_list_tbl_save = &g_con_list[s_con_data_no];
                    }
                }
            }
            if ( s_ret != MDSO_RET_OK ) {
                s_con_retry_flg = 1;
                /* 送信先選択不可エラー(内部エラー)戻り先で設定 */
                /* 送信不可応答またはエラー終了を戻り先で実行する */
                /* EMS出力 */
                /* 送信先コネクションなし */
                if ( g_data_kind != MDSO_SVC_REQ ) {
                    /* 業務要求の場合、EMS出力無し */
                    MDSO_message_output(DEF_EVT_RSP_CONN_NON,
                                        DEF_NERR_CON_SEND_ERR,
                                        "@L@X@X", g_lcn,
                                        con_list_tbl_save->pathmon_name,
                                        con_list_tbl_save->server_class_phy);
                }
                memcpy( g_internal_error_code,
                        DEF_NERR_CON_SEND_ERR,
                        sizeof(g_internal_error_code));

                switch (g_data_kind) {
                    case MDSO_SVC_REQ:     /* 業務要求 */
                    case MDSO_CTL_REQ:     /* 制御要求 */
                    case MDSO_CTL_RSP:     /* 制御応答 */
                        /* 送信不可応答 */
                        return MDSO_RET_BAD_SEND;
                    default:
                        /* エラー終了 */
                        return MDSO_RET_NG;
                }
            }
        }
    }  /* End of while (コネクション再選択リトライ) */

    return MDSO_RET_OK;

} /* end of MDSO_data_con_send */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_err_data_make                                   */
/*  CALLING SEQ.    : short MDSO_err_data_make ( c302_def *rcv_data_adr)          */
/*  ARGUMENT        : 1. rcv_data_adr    (I) 受信電文                             */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 送信不可応答作成送信処理                                    */
/**********************************************************************************/
short MDSO_err_resp ( c302_def *rcv_data_adr )
{
    short         s_ret          = 0;
    short         ch_turn_tbl_no = 0;
    c301_def      c301_send_data;
    db_gqnwq_def *c302_rcv_que   = (db_gqnwq_def *)rcv_data_adr->msg_data;

    memset((char *)&c301_send_data, 0, sizeof(c301_send_data));

    /* 送信先キュー検索 */
    s_ret = MDSO_turn_que_get ( c302_rcv_que->denbun_send_recv_info.mti_id,
                                &ch_turn_tbl_no );

    /* 送信先キュー検索結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    /* 送信不可応答 電文作成 */
    s_ret = MDSO_turn_ipc_make( rcv_data_adr,
                                &c301_send_data );

    /* 送信不可応答 電文作成結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        
        return MDSO_RET_NG;
    }

    /* キュー登録要求送信処理 */
    s_ret = MDSO_turn_ipc_send(&g_turn_tbl[ch_turn_tbl_no],
                               &c301_send_data);

    /* キュー登録要求送信結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_err_resp */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_turn_que_get                                    */
/*  CALLING SEQ.    : short MDSO_turn_que_get ( char  *ch_rcv_mti,                */
/*                                              short *ch_turn_tbl_no)            */
/*  ARGUMENT        : 1. ch_rcv_mti      (I) MTI取得対象メッセージ判定結果        */
/*                  : 2. ch_turn_tbl_no  (O) 電文折返し振分先設定テーブル配列番号 */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 振分先キュー取得                                            */
/**********************************************************************************/
short MDSO_turn_que_get ( char  *ch_rcv_mti,
                          short *ch_turn_tbl_no )
{
    char  set_mti[4];
    short loop_cnt          = 0;
    char  data_kind_set_flg = MDSO_OFF;

    /* MTI設定 */
    memcpy( set_mti, ch_rcv_mti, 4);

    /* 送信電文折返し振分先設定テーブル検索 */
    for ( loop_cnt=0; loop_cnt<g_turn_tbl_cnt; loop_cnt++ ){
        if ( 0 == memcmp( g_turn_tbl[loop_cnt].t_primary_key.mti,
                          set_mti,
                          sizeof(g_turn_tbl[loop_cnt].t_primary_key.mti) ) ){
            /* 対象MTIテーブルあり */
            /* MTIをグローバル変数に設定する */
            memcpy( g_mti_data, set_mti, sizeof(g_mti_data) );
            /* 設定済フラグON */
            data_kind_set_flg = MDSO_ON;
            break;
        }
    }  /* end of while (loop_cnt) */

    /* 設定済フラグ確認 */
    if ( MDSO_ON != data_kind_set_flg ) {
        /* テーブルなし */
        /* 異常終了 */
        /* 振分先判定エラー(内部エラー) */
        strncpy( g_internal_error_code,
                 DEF_NERR_FURIWAKE_DST_HANTE_ERR,
                 sizeof(g_internal_error_code));
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                            DEF_NERR_FURIWAKE_DST_HANTE_ERR,
                            "@L@C@X@E",
                            g_lcn,
                            &g_rcv_gflin,
                            g_mti_data, data_kind_set_flg);
        return MDSO_RET_NG;
    }
    else {
        /* 送信電文折返し振分先設定テーブル検索済み */
        /* 送信電文折返し振分先設定テーブルの配列番号を返却 */
        *ch_turn_tbl_no = (char)loop_cnt;
    }

    return MDSO_RET_OK;

} /* end of MDSO_detour_que_get */

/******************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_turn_ipc_make                               */
/*  CALLING SEQ.    : short MDSO_turn_ipc_make ( c302_def *c302_rcv_data,     */
/*                                               c301_def *c301_send_data)    */
/*  ARGUMENT        : 1. c302_rcv_data   (I) 受信電文                         */
/*                  : 2. c301_send_data  (O) 送信電文作成領域                 */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                  */
/*  DESCRIPTION     : 送信不可応答作成処理                                    */
/******************************************************************************/
short MDSO_turn_ipc_make ( c302_def *c302_rcv_data,
                           c301_def *c301_send_data )
{
    db_gqnwq_def *c301_que_file = (db_gqnwq_def *)c301_send_data->que_rgs_info.msg_data;
    db_gqnwq_def *c302_que_file = (db_gqnwq_def *)c302_rcv_data->msg_data;
    char  ch_denbun_len[5];
    short s_denbun_len = 0;

    /* IPC interface_code設定 (C301) */
    memcpy( c301_send_data->common_header.interface_code,
                       DEF_IPC_IFCD_Q_RGST_REQ, 4 );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( c301_send_data->common_header.internal_error_code, MDSO_SPACE,
            sizeof(c301_send_data->common_header.internal_error_code));
    memset( c301_send_data->common_header.filler_1, MDSO_SPACE,
            sizeof(c301_send_data->common_header.filler_1));

    /* IPC GFP内部LCN */
    memcpy( c301_send_data->que_rgs_info.gfp_lcn,
            &c302_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            sizeof(c301_send_data->que_rgs_info.gfp_lcn) );

    /* IPC サイト識別 */
    c301_send_data->que_rgs_info.serverclass_info.site_name =
          g_myinfo_def.config_data.site_id;

    /* IPC サーバクラス論理名   */
    memcpy( c301_send_data->que_rgs_info.serverclass_info.serverclass_id.serverclass_name,
            g_myinfo_def.config_data.serverclass_name,
            sizeof(c301_send_data->que_rgs_info.serverclass_info.serverclass_id.serverclass_name) );

    /* IPC サーバクラス論理番号 */
    memcpy( c301_send_data->que_rgs_info.serverclass_info.serverclass_id.serverclass_num,
            g_myinfo_def.config_data.serverclass_no,
            sizeof(c301_send_data->que_rgs_info.serverclass_info.serverclass_id.serverclass_num) );

    /* NWキューファイルを受信電文から送信電文にコピー */
    memcpy( c301_que_file, c302_que_file, db_gqnwq_def_Size );

    /* IPC 制御情報 処理コード区分設定 */
    /* 電文送信不可応答 */
    memcpy( &c301_que_file->cntrl_info.shori_kubun, MDSO_C301_KIND_TURN_IPC, 2 );

    /* IPC 制御情報 内部エラーコード設定 */
    memcpy( c301_que_file->cntrl_info.err_code, DEF_NERR_DST_SELECT_ERR,
               sizeof( c301_que_file->cntrl_info.err_code ) );

    /* IPC 送受信電文全体長設定 */
    memcpy(ch_denbun_len, c301_que_file->denbun_area.denbun_len, 4);
    ch_denbun_len[4] = 0;
    s_denbun_len = (short)atoi(ch_denbun_len);

    c301_send_data->common_header.control_data_length =
          (unsigned short)((sizeof(c301_send_data->que_rgs_info) - 9999)
                          + (db_gqnwq_def_Size - 9999)
                          + s_denbun_len);

    return MDSO_RET_OK;

} /* end of MDSO_turn_ipc_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_turn_ipc_send                             */
/*  CALLING SEQ.    : short MDSO_turn_ipc_send( t_turn_tbl *ch_turn_tbl,    */
/*                                              c301_def *send_deta_adr)    */
/*  ARGUMENT        : 1. ch_turn_tbl    (I) 送信先振分テーブルのアドレス    */
/*                  : 2. send_deta_adr  (I) 送信電文のアドレス              */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : キュー登録要求送信処理                                */
/****************************************************************************/
short MDSO_turn_ipc_send ( t_turn_tbl *ch_turn_tbl,
                           c301_def *send_deta_adr)
{
    short s_ret;

    c301_def send_data_save;
    r301_def *r301_data_save;

    memset( &send_data_save, 0, sizeof(send_data_save) );
    memcpy( &send_data_save, send_deta_adr, sizeof(send_data_save) );

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, MDSO_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    /* 送信先振分テーブルにより送信先設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           MDSO_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));
    if (ch_turn_tbl->t_phy_data.domain_name[0] != MDSO_SPACE){
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               ch_turn_tbl->t_phy_data.domain_name,
               sizeof(ch_turn_tbl->t_phy_data.domain_name));
    }
    else {
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               ch_turn_tbl->t_phy_data.pathmon,
               sizeof(ch_turn_tbl->t_phy_data.pathmon));
    }
    memcpy(t_COM_PSD_arg_1_def.serverclass_name,
           ch_turn_tbl->t_phy_data.serverclass,
           sizeof(ch_turn_tbl->t_phy_data.serverclass));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &send_data_save, sizeof(send_data_save) );

    t_COM_PSD_arg_1_def.receive_max_len = MDSO_MAX_DATA_SIZE;
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_myinfo_def.config_data.send_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = (short)g_myinfo_def.config_data.send_retry_count;

    memcpy(t_COM_PSD_arg_2_def.prog_id,
           g_myinfo_def.proc_data_sub.module_id,
           sizeof(t_COM_PSD_arg_2_def.prog_id) );

    t_COM_PSD_arg_1_def.req_send_len = 
       send_deta_adr->common_header.control_data_length 
       + sizeof(send_deta_adr->common_header);

    /* サーバークラス論理ID設定 */
    memcpy( t_COM_PSD_arg_4_def.srv_logical_id,
            g_myinfo_def.config_data.serverclass_name,
            sizeof(g_myinfo_def.config_data.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy( t_COM_PSD_arg_4_def.lcn,
            g_lcn,
            sizeof(g_lcn));

    /* PATHSEND共通処理実行 */
    s_ret = COM_PSD( &t_COM_PSD_arg_1_def,
                     &t_COM_PSD_arg_2_def,
                     &t_COM_PSD_arg_3_def,
                     &g_cg010in_modle,
                     &t_COM_PSD_arg_4_def );

    /* PATHSEND結果確認 */
    if ( MDSO_RET_OK != s_ret ) {
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_PSEND_ERR_RE_OUT,
                            "@X@E", "COM_PSD", s_ret);

        memcpy( g_internal_error_code,
                DEF_NERR_PSEND_ERR_RE_OUT,
                sizeof(g_internal_error_code));

        /* PATHSEND異常終了 */
       return MDSO_RET_NG;
    }

    r301_data_save = (r301_def*)t_COM_PSD_arg_1_def.msg_buf;

    /* 応答電文異常終了判定 */
    /* 応答電文のインタフェースコード判定(R301) */
    if ( memcmp(r301_data_save->common_header.interface_code,
                DEF_IPC_IFCD_Q_RGST_RSP,
                sizeof(r301_data_save->common_header.interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (r301_data_save->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常時処理実行 */
            s_ret = MDSO_RET_NG;
        }
    }
    else {
        /* 異常時処理実行 */
        s_ret = MDSO_RET_NG;
    }
    if ( s_ret == MDSO_RET_NG ) {
        /* EMS出力 応答エラー */
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R301 ERROR          ",
                            r301_data_save);
       return MDSO_RET_NG;
    }

    /* 送信成功 */
    return MDSO_RET_OK;

} /* end of MDSO_turn_ipc_send */

/***********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_status_req                                       */
/*  CALLING SEQ.    : short MDSO_status_req( c107_def *rcv_ipc )                   */
/*  ARGUMENT        : 1. rcv_ipc    (I) 受信IPC                                    */
/*  RETURN CODE     : 0:正常 -1:異常                                               */
/*  DESCRIPTION     : コネクション状態通知要求                                     */
/***********************************************************************************/
short MDSO_status_req ( c107_def *rcv_ipc)
{
    short  loop_cnt             = 0;
    short  s_set_con_no         = -1;
    short  s_line_list_st_top   = -1;
    short  s_line_list_st_cnt   = 0;
    short  s_con_list_top       = -1;
    short  s_con_list_cnt       = 0;

    /* グループ確認 */
    if ( (rcv_ipc->line_info.site_name != g_myinfo_def.config_data.site_id ) ||
         (rcv_ipc->line_info.nw_name != g_myinfo_def.config_data.network_id ) ||
         ( memcmp( rcv_ipc->line_info.group_name,
                   g_myinfo_def.config_data.group_id,
                   sizeof(rcv_ipc->line_info.group_name)) != 0 )){
        /* 指定グループ異常 */
        return MDSO_RET_NG;
    }

    /* 回線ラウンドロビンリスト(インタフェース)検索 */
    for ( loop_cnt = 0; loop_cnt < g_line_list_if_cnt; loop_cnt++ ) {
        if ( memcmp( rcv_ipc->line_info.interface_name,
                    g_line_list_if[loop_cnt].interface_id,
                    sizeof(rcv_ipc->line_info.interface_name)) == 0 ) {
            s_line_list_st_top = g_line_list_if[loop_cnt].list_top_no;
            s_line_list_st_cnt = g_line_list_if[loop_cnt].list_cnt;
            break;
        }
    }

    if ( s_line_list_st_top != -1 ) {
        /* 回線ラウンドロビンリスト(ステーション)検索 */
        for ( loop_cnt = s_line_list_st_top;
              loop_cnt < (s_line_list_st_top + s_line_list_st_cnt);
              loop_cnt++ ) {
            if (( memcmp( rcv_ipc->line_info.interface_name,
                         g_line_list_st[loop_cnt].interface_id,
                         sizeof(rcv_ipc->line_info.interface_name)) == 0) &&
                ( memcmp( rcv_ipc->line_info.station_name,
                         g_line_list_st[loop_cnt].station_id,
                         sizeof(rcv_ipc->line_info.station_name)) == 0)) {
                s_con_list_top = g_line_list_st[loop_cnt].list_top_no;
                s_con_list_cnt = g_line_list_st[loop_cnt].list_cnt;
                break;
            }
        }
        if ( s_con_list_top != -1 ) {
            /* コネクションリスト検索 */
            for ( loop_cnt = s_con_list_top;
                  loop_cnt < (s_con_list_top + s_con_list_cnt);
                  loop_cnt++ ) {
                /* コネクション識別でテーブル検索 */
                if (( memcmp( rcv_ipc->line_info.interface_name,
                             g_con_list[loop_cnt].interface_id,
                             sizeof(rcv_ipc->line_info.interface_name)) == 0) &&
                    ( memcmp( rcv_ipc->line_info.station_name,
                             g_con_list[loop_cnt].station_id,
                             sizeof(rcv_ipc->line_info.station_name)) == 0) &&
                    ( memcmp( rcv_ipc->line_info.src_connection_name,
                              g_con_list[loop_cnt].connection_id,
                              sizeof(rcv_ipc->line_info.src_connection_name)) == 0)) {
                    /* コネクション識別子一致 */
                    /* コネクション状態設定 */
                    if ( memcmp(rcv_ipc->connection_status_info.connection_status,
                                DEF_CONNECT_STS_DISCONN,
                                sizeof(rcv_ipc->connection_status_info.connection_status)) == 0){
                        g_con_list[loop_cnt].connection_st = MDSO_CONNECT_STS_DISCONN_INT;
                    }
                    else if ( memcmp(rcv_ipc->connection_status_info.connection_status,
                              DEF_CONNECT_STS_LISTEN,
                              sizeof(rcv_ipc->connection_status_info.connection_status)) == 0){
                        g_con_list[loop_cnt].connection_st = MDSO_CONNECT_STS_LISTEN_INT;
                    }
                    else if ( memcmp(rcv_ipc->connection_status_info.connection_status,
                              DEF_CONNECT_STS_CONNECT,
                              sizeof(rcv_ipc->connection_status_info.connection_status)) == 0){
                        g_con_list[loop_cnt].connection_st = MDSO_CONNECT_STS_CONNECT_INT;
                    }
                    else if ( memcmp(rcv_ipc->connection_status_info.connection_status,
                              DEF_CONNECT_STS_RECONNECT,
                              sizeof(rcv_ipc->connection_status_info.connection_status)) == 0){
                        g_con_list[loop_cnt].connection_st = MDSO_CONNECT_STS_RECONNECT_INT;
                    }
                    else {
                        /* エラー終了 */
                        return MDSO_RET_NG;
                    }

                    s_set_con_no = loop_cnt;
                    break;
                }
            }
        }
    }

    /* コネクション情報なしエラー */
    if ( s_set_con_no == -1 ) {
        /* 異常終了 */
        memcpy( g_internal_error_code,
                DEF_NERR_CON_SELECT_ERR,
                sizeof(g_internal_error_code));

        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_status_req */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_err_log_set                               */
/*  CALLING SEQ.    : short MDSO_err_log_set ( c302_def *rcvdata,           */
/*                                             char err_kind,               */
/*                                             char *err_code)              */
/*  ARGUMENT        : 1. rcvdata    (I) ログ出力対象電文                    */
/*                  : 2. err_kind   (I) ログ出力用エラー種別                */
/*                  : 3. err_code   (I) ログ出力用エラー番号                */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : エラーログ作成出力処理                                */
/****************************************************************************/
short MDSO_err_log_set ( c302_def *rcvdata,
                         char err_kind,
                         char *err_code)
{
    short        s_ret           = 0;
    unsigned int s_data_all_leng = 0;
    char         ch_leng_set[5];
    short        s_leng_set      = 0;
    db_glelg_def st_log_tbl;         /* ログ出力テーブル */
    db_gqnwq_def *c302_rcv_que = (db_gqnwq_def *)rcvdata->msg_data;

    /* タイムスタンプパラメータ */
    COM_UNQ_arg_1_def t_COM_UNQ_arg_1_def;
    char ch_datetime_hex[16+1];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* ログ出力要求テーブル初期化 */
    memset( &st_log_tbl, 0, sizeof(st_log_tbl) );

    /* タイムスタンプ取得 */
    memset( (char *)&t_COM_UNQ_arg_1_def, 0, sizeof(t_COM_UNQ_arg_1_def));
    memset( ch_datetime_hex, 0, sizeof(ch_datetime_hex));
    COM_UNQ( &t_COM_UNQ_arg_1_def, ch_datetime_hex );

    /* エラーログ プライマリーキー設定 */
    memcpy( &st_log_tbl.pri_key.part_id[0],
            &t_COM_UNQ_arg_1_def.cc[1],
            sizeof(st_log_tbl.pri_key.part_id) );

    /* エラーログ プライマリーキー タイムスタンプ設定 */
    memcpy( &st_log_tbl.pri_key.time_stamp[0],
            &t_COM_UNQ_arg_1_def,
            sizeof(st_log_tbl.pri_key.time_stamp) );

    /* エラーログ プライマリーキー タイムスタンプ枝番設定 */
    memcpy( &st_log_tbl.pri_key.time_stamp_branch[0],
            ch_datetime_hex,
            sizeof(st_log_tbl.pri_key.time_stamp_branch) );

    /* エラーログ エラー電文識別 */
    st_log_tbl.err_denbun_id = err_kind;

    /* MTI設定 */
    memcpy( st_log_tbl.mti_id,
            c302_rcv_que->denbun_send_recv_info.mti_id,
            sizeof(st_log_tbl.mti_id) );

    /* エラーログ レスポンスコード スペース設定 */
    memset( st_log_tbl.res_code, ' ', 
            sizeof(st_log_tbl.res_code) );

    /* エラーログ 内部エラーコード スペース設定 */
    memcpy( st_log_tbl.naibu_err_code,
            err_code, 
            sizeof(st_log_tbl.naibu_err_code) );

    /* エラーログ サーバークラス論理ID設定 */
    memcpy( st_log_tbl.srv_cls_info.srv_cls_id,
            g_myinfo_def.config_data.serverclass_name, 
            sizeof(st_log_tbl.srv_cls_info.srv_cls_id) );

    /* エラーログ サーバクラス論理番号 */
    memcpy( st_log_tbl.srv_cls_info.srv_cls_mlt_num,
            g_myinfo_def.config_data.serverclass_no,
            sizeof(st_log_tbl.srv_cls_info.srv_cls_mlt_num) );

    /* エラーログ 電文送受信情報 電文受信時刻設定 */
    memcpy( &st_log_tbl.denbun_send_recv_info.denbun_recv_time[0],
            &c302_rcv_que->denbun_send_recv_info.denbun_recv_time[0],
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_recv_time) );

    /* エラーログ 電文送受信情報 受信時局状態 受信時局状態 */
    st_log_tbl.denbun_send_recv_info.recv_kyoku_sts =
               c302_rcv_que->denbun_send_recv_info.recv_kyoku_sts;

    /* エラーログ 電文送受信情報 NW区分設定 */
    memcpy( st_log_tbl.denbun_send_recv_info.nw_kubun,
            c302_rcv_que->denbun_send_recv_info.nw_kubun,
            sizeof(st_log_tbl.denbun_send_recv_info.nw_kubun) );

    /* エラーログ 電文送受信情報 MTI設定 */
    memcpy( st_log_tbl.denbun_send_recv_info.mti_id,
            c302_rcv_que->denbun_send_recv_info.mti_id,
            sizeof(st_log_tbl.denbun_send_recv_info.mti_id) );

    /* エラーログ 電文送受信情報 送信電文種別 */
    st_log_tbl.denbun_send_recv_info.send_denbun_shubetu =
        c302_rcv_que->denbun_send_recv_info.send_denbun_shubetu;

    /* エラーログ 電文送受信情報 電文ログKEY GFP内部LCN */
    memcpy( st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.lcn_id,
            &c302_rcv_que->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.lcn_id) );

    /* エラーログ 電文送受信情報 電文ログKEY 電文形態 */
    st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai = 
      c302_rcv_que->denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai;

    /* エラーログ 電文送受信情報 電文ログKEY 電文種別 */
    st_log_tbl.denbun_send_recv_info.denbun_log_key.denbun_shubetu =
      c302_rcv_que->denbun_send_recv_info.denbun_log_key.denbun_shubetu;

    /* エラーログ 電文送受信情報 電文ログKEY 再送回数 */
    memcpy( st_log_tbl.denbun_send_recv_info.denbun_log_key.re_send_num,
            c302_rcv_que->denbun_send_recv_info.denbun_log_key.re_send_num,
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key.re_send_num) );

    /* エラーログ 電文送受信情報 電文フォーマット区分 */
    st_log_tbl.denbun_send_recv_info.denbun_fmt_kubun =
      c302_rcv_que->denbun_send_recv_info.denbun_fmt_kubun;

    /* エラーログ 電文送受信情報 通信ログ保存ファイル名 */
    memcpy( st_log_tbl.denbun_send_recv_info.tushin_log_save_filename,
            c302_rcv_que->denbun_send_recv_info.tushin_log_save_filename,
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_save_filename) );

    /* エラーログ 電文送受信情報 通信ログKEY */
    memcpy( &st_log_tbl.denbun_send_recv_info.tushin_log_key.part_id[0],
            &c302_rcv_que->denbun_send_recv_info.tushin_log_key.part_id[0],
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_key) );

    /* エラーログ 通信制御情報 */
    memcpy( &st_log_tbl.tushin_cntrl_info,
            &c302_rcv_que->tushin_cntrl_info,
            sizeof(st_log_tbl.tushin_cntrl_info) );

    /* エラーログ サイト識別設定 */
    st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id[0] = g_myinfo_def.config_data.site_id;

    /* エラーログ NW識別設定 */
    st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id[1] = g_myinfo_def.config_data.network_id;

    /* エラーログ グループ識別設定 */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id[2],
            g_myinfo_def.config_data.group_id,
            5 );

    /* IPC インタフェース識別設定 */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id[7],
            g_rcv_if_id,
            5 );

    /* IPC ステーション識別設定 */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id[12],
            g_rcv_station_id,
            6 );

    /* ダミー領域クリア */
    memset( st_log_tbl.future_use, MDSO_SPACE, sizeof(st_log_tbl.future_use));
    memset( st_log_tbl.denbun_send_recv_info.future_use,
            MDSO_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.future_use));
    memset( st_log_tbl.tushin_cntrl_info.future_use,
            MDSO_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.future_use));

    /* エラーログ 電文長設定 */
    memset( st_log_tbl.denbun_area.denbun_len, '0', 5 );
    memcpy( &st_log_tbl.denbun_area.denbun_len[1],
            c302_rcv_que->denbun_area.denbun_len, 4 );

    /* エラーログ MTI開始位置設定 */
    memset( st_log_tbl.denbun_area.mti_start_lct, '0', 5 );
    memcpy( &st_log_tbl.denbun_area.mti_start_lct[1],
            c302_rcv_que->denbun_area.mti_start_lct, 4 );

    /* エラーログ 送受信電文設定 */
    memset( ch_leng_set, 0, 5 );
    memcpy( ch_leng_set, c302_rcv_que->denbun_area.denbun_len, 4);
    s_leng_set = (short)atoi(ch_leng_set);
    memcpy( st_log_tbl.denbun_area.denbun,
            c302_rcv_que->denbun_area.denbun,
            s_leng_set);

    /* エラーログ 送受信電文全体長設定 */
    s_data_all_leng = 
         s_leng_set +
         db_glelg_def_Size -
         sizeof(st_log_tbl.denbun_area.denbun);

    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &t_COM_ERL_arg_3_def, MDSO_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = MDSO_ERR_LOG_WRITE;
    t_COM_ERL_arg_1_def.io_timer = (long)g_myinfo_def.config_data.send_timer;
    t_COM_ERL_arg_1_def.data_len = (short)s_data_all_leng;

    t_COM_ERL_arg_1_def.data_area = (char *)&st_log_tbl;

    /* サーバークラス論理ID設定 */
    memcpy(t_COM_ERL_arg_3_def.srv_logical_id,
            g_myinfo_def.config_data.serverclass_name,
            sizeof(g_myinfo_def.config_data.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy(t_COM_ERL_arg_3_def.lcn,
            g_lcn,
            sizeof(g_lcn));

    /* 接続先(サイト識別～コネクション識別) */
    memcpy(t_COM_ERL_arg_3_def.connect,
           &c302_rcv_que->tushin_cntrl_info.line_info.recv_connect_id,
           sizeof(c302_rcv_que->tushin_cntrl_info.line_info.recv_connect_id));

    /* エラーログ共通処理実行 */
    s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                     &g_COM_ERL_arg_2_def,
                     &g_cg010in_modle,
                     &t_COM_ERL_arg_3_def,
                     g_myinfo_def.proc_data_sub.module_id);

    /* エラーログ結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_ERL", s_ret);

        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_err_log_set */

/***********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_file_up                                          */
/*  CALLING SEQ.    : short MDSO_file_up( void )                                   */
/*  ARGUMENT        : void                                                         */
/*  RETURN CODE     : 0:正常 -1:異常                                               */
/*  DESCRIPTION     : コネクション状態通知要求                                     */
/***********************************************************************************/
short MDSO_file_up (void)
{
    short s_ret = 0;

    /* コネクション制御ラウンドロビンリスト作成処理 */
    s_ret = MDSO_conlist_make();

    /* 処理結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSO_RET_NG;
    }

    return MDSO_RET_OK;

} /* end of MDSO_file_up */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_sys_open                                  */
/*  CALLING SEQ.    : void MDSO_sys_open ( void )                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : OPENメッセージ処理                                    */
/****************************************************************************/
void MDSO_sys_open (void)
{
    short   s_rcv_err = 0;
    int     i_CC      = 0;
    short   s_ret     = 0;

    char    wk_pname[ZSYS_VAL_LEN_PROCESSNAME+1];
    short   wk_pname_len;
    short   wk_openid;
    short   wk_reply_cd;
    zsys_ddl_smsg_open_reply_def *rep_msg;
    zsys_ddl_smsg_open_def *sys_msg_p;
    
    memset(wk_pname, 0x00, sizeof(wk_pname));
    wk_pname_len = 0;
    wk_openid = 0;
    wk_reply_cd = 0;

    sys_msg_p = (zsys_ddl_smsg_open_def *)g_recv_buf;

    /* オープン発行元プロセス名取得 */
    PROCESSHANDLE_DECOMPOSE_(
        (short *)&g_recv_info.z_sender,,,,,,,
        wk_pname,
        sizeof(wk_pname),
        &wk_pname_len);

    /* オープン発行元プロセス名判定 */
    if (PROCESSHANDLE_COMPARE_(
        (short *)&g_recv_info.z_sender, g_myinfo_def.proc_data.ans_phandle) != 0) {
        wk_openid = MDSO_OPENID_ANCESTOR;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else if (memcmp(sys_msg_p->u_z_data.z_qualifier,"#CONN",5) == 0) {
        wk_openid = MDSO_OPENID_ROUT;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else if (memcmp(sys_msg_p->u_z_data.z_qualifier,"#GFPCI",6) == 0) {
        wk_openid = MDSO_OPENID_ROUT;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else if (memcmp(wk_pname, "$ZL", 3) == 0) {
        wk_openid = MDSO_OPENID_ROUT;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else {
        wk_reply_cd = 48;
    }

    /* オープナープロセス管理モジュール */
    s_ret = COM_STP_JUDGE(&g_openersinfo, g_recv_buf);
    
    if(s_ret < 0){
        /* EMS */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_NOMAL,
                            "@X@E", "COM_STP_JUDGE OPEN", s_ret);
    }

    /* リプライメッセージ */
    memset(g_resp_buf, 0, sizeof(g_resp_buf));

    rep_msg = (zsys_ddl_smsg_open_reply_def *)g_resp_buf;
    rep_msg->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
    rep_msg->z_openid    = wk_openid;

    /* リプライ処理 */
    i_CC = REPLYX(g_resp_buf,
                  zsys_ddl_smsg_open_reply_def_Size,
                  /* count-written */,
                  ,
                  wk_reply_cd);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(g_myinfo_def.recv_fno, &s_rcv_err);
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "SYS OPEN REPLY ERR",
                            g_resp_buf);
    }

} /* end of MDSO_sys_open */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_sys_close                                 */
/*  CALLING SEQ.    : void MDSO_sys_close ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : CLOSEメッセージ処理                                   */
/****************************************************************************/
void MDSO_sys_close (void)
{

    short   s_rcv_err = 0;
    int     i_CC      = 0;
    short   s_ret     = 0;

    /* オープナープロセス管理モジュール */
    s_ret = COM_STP_JUDGE(&g_openersinfo, g_recv_buf);
    
    if(s_ret < 0){
        /* EMS */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_NOMAL,
                            "@X@E", "COM_STP_JUDGE CLOSE", s_ret);
        g_myinfo_def.end_flag = MDSO_ON;
    }
    /* クローズ判定 0：停止不要 1:停止 */
    if (s_ret == 1){
        g_myinfo_def.end_flag = MDSO_NOMAL_END;
    }

    /* リプライ処理 */
    i_CC = REPLYX(g_recv_buf,,, , 0 /* ZFIL_ERR_OK */);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(g_myinfo_def.recv_fno, &s_rcv_err);
        MDSO_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "SYS CLOSE REPLY ERR",
                            g_recv_buf);
    }

} /* end of MDSO_sys_close */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_end                                       */
/*  CALLING SEQ.    : void MDSO_end ( void )                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void MDSO_end (void)
{
    short s_ret = 0;
    char  ch_mdso_sub_prog_sts[2];
    lk_zac2001t_arg_1_def Trace_off;

    /* IOモジュールパラメータ*/
    COM_IOM_arg_3_def mdso_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def mdso_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def mdso_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def mdso_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* 局状態管理ファイルクローズ */
    if (g_file_data.station_sts_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        /* 局状態管理ファイルクローズ */
        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.station_sts_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.station_sts_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.station_sts_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_CEN_STS,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 回線管理ファイルクローズ */
    if (g_file_data.line_ctl_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.line_ctl_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.line_ctl_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.line_ctl_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdso_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_LIN_MG,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 回線ステータスファイルクローズ */
    if (g_file_data.line_st_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.line_st_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GCLST, strlen(DEF_GCLST));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.line_st_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.line_st_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdso_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_LIN_STS,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 送信電文折返し振分先設定ファイルクローズ */
    if (g_file_data.data_turn_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.data_turn_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GFQBK, strlen(DEF_GFQBK));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.data_turn_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.data_turn_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdso_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_RTN_DST,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* NW情報ファイルクローズ */
    if (g_file_data.nw_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.nw_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.nw_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.nw_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdso_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 物理名情報ファイルクローズ */
    if (g_file_data.phy_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdso_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_3_def) );
        memset( &mdso_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(mdso_COM_IOM_arg_4_def) );
        memset( &mdso_COM_IOM_arg_5_def, 0, sizeof(mdso_COM_IOM_arg_5_def) );
        memset( &mdso_COM_IOM_arg_6_def, 0, sizeof(mdso_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdso_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdso_COM_IOM_arg_3_def.prog_id));
        memcpy(mdso_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(mdso_COM_IOM_arg_3_def.file_name,
            g_file_data.phy_file_name, sizeof(mdso_COM_IOM_arg_3_def.file_name));
        memcpy(mdso_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_CLOSE,
                                    sizeof(mdso_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdso_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(mdso_COM_IOM_arg_4_def.file_name,
            g_file_data.phy_file_name, sizeof(mdso_COM_IOM_arg_4_def.file_name));
        mdso_COM_IOM_arg_4_def.file_no = g_file_data.phy_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdso_sub_prog_sts,
            &mdso_COM_IOM_arg_3_def,
            &mdso_COM_IOM_arg_4_def,
            &mdso_COM_IOM_arg_5_def,
            &mdso_COM_IOM_arg_6_def);

        if ( MDSO_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                g_rcv_if_id,
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdso_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* エラーログクローズ */
    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &t_COM_ERL_arg_3_def, MDSO_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = MDSO_ERR_LOG_CLOSE;
    t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_COM_ERL_arg_1_def.data_len     = 0;

    /* エラーログ共通処理実行 */
    s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                     &g_COM_ERL_arg_2_def,
                     &g_cg010in_modle,
                     &t_COM_ERL_arg_3_def,
                     g_myinfo_def.proc_data_sub.module_id);

    /* エラーログ結果判定 */
    if ( MDSO_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_ERL CLOSE", s_ret);
    }

    /* トレース終了 */
    Trace_off.func_flg = '2';    /*トレース終了処理*/
    TRACEOUT((char *)&Trace_off);

    if ( g_myinfo_def.end_flag == MDSO_NOMAL_END ){
        MDSO_message_output(DEF_EVT_PROC_NORMAL_END, DEF_NERR_NOMAL, "@R",
                            g_myinfo_def.proc_data.my_pname);
        PROCESS_STOP_(,,MDSO_NORMAL_TERMINATION);
    }
    else {
        MDSO_message_output(DEF_EVT_PROC_ABNORMAL_END, DEF_NERR_NOMAL, "@R",
                            g_myinfo_def.proc_data.my_pname);
        PROCESS_STOP_(,,MDSO_ABNORMAL_TERMINATION);
    }

} /* end of MDSO_end */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_message_output                            */
/*  CALLING SEQ.    : void MDSO_message_output ( short   s_event_code,      */
/*                                               char    *pch_inter_code,   */
/*                                               char    *pch_format, ...)  */
/*  ARGUMENT        : 1. s_event_code     (I) EMSイベントコード             */
/*                  : 2. pch_inter_code   (I) 内部エラーコード              */
/*                  : 3. pch_format       (I) EMS設定データ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ出力処理                                    */
/****************************************************************************/
void MDSO_message_output(
    short   s_event_code,
    char    *pch_inter_code,
    char    *pch_format, ...)
{
    char        ch_text[80];
    char        ch_fmt[80];
    char        *pch_ep, *pch_sp;
    short       s_var, s_vcnt = 0, s_cnt, s_idx, s_param_cnt = 0, loop_flg = 1;
    va_list     va_ap;

    /* EMS出力情報・リターンコード     */
    g_cg010in.emsinf.rcd = '0';

    memset((char *)&g_cg010in.emsinf.emsgkinf, MDSO_SPACE, sizeof(g_cg010in.emsinf.emsgkinf));

    /* イベントコード*/
    memset(ch_text, 0x00, sizeof(ch_text));
    sprintf(ch_text, "%05d", s_event_code);
    memcpy(g_cg010in.emsinf.msgid, ch_text, strlen(ch_text));

    /* 内部エラーコード */
    memcpy(g_cg010in.emsinf.emsgkinf.inter_errcd,
        pch_inter_code, sizeof(g_cg010in.emsinf.emsgkinf.inter_errcd));

    /* メッセージ通知区分 */
    if (memcmp( pch_inter_code, DEF_NERR_NOMAL, 7 ) == 0){
        /* 正常メッセージ */
        g_cg010in.emsinf.emsgkinf.msgttkb = '*';
    }
    else{
        /* 異常メッセージ */
        g_cg010in.emsinf.emsgkinf.msgttkb = 'E';
    }

    /* システム名 */
    memcpy( g_cg010in.emsinf.emsgkinf.sysnm,  DEF_EMS_SYSNM_GFP, strlen(DEF_EMS_SYSNM_GFP));

    /* server分類 */
    memcpy( g_cg010in.emsinf.emsgkinf.srv_kbn,  DEF_EMS_SRV_KBN_COM, strlen(DEF_EMS_SRV_KBN_COM));

    /* NW識別 被仕向 */
    if ( g_nw_info[0].nw_segment[0] != 0 ){
        memcpy( g_cg010in.emsinf.emsgkinf.h_nw_kbn,
                g_nw_info[0].nw_segment,
                sizeof(g_cg010in.emsinf.emsgkinf.h_nw_kbn));
    }

    /* NW識別 仕向 */
    memset( g_cg010in.emsinf.emsgkinf.s_nw_kbn,  MDSO_SPACE,
                      sizeof(g_cg010in.emsinf.emsgkinf.s_nw_kbn));

    /* プログラム名 */
    memcpy( g_cg010in.emsinf.emsgkinf.prgid,
            g_myinfo_def.proc_data_sub.module_id,
            sizeof(g_myinfo_def.proc_data_sub.module_id));

    /* プロセス名 */
    memcpy( g_cg010in.emsinf.emsgkinf.trmnm,
            g_myinfo_def.proc_data.my_pname,
            ZSYS_VAL_LEN_PROCESSNAME);

    /* 任意メッセージ */
    memset((char *)&g_cg010in.emsinf.emsnninf, 0x20, sizeof(g_cg010in.emsinf.emsnninf));

    /* ①GFP通信制御メッセージ番号 */

    /* ②サーバークラス論理ID */
    if (g_myinfo_def.config_data.serverclass_name[0] != 0x00) {
        memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
            g_myinfo_def.config_data.serverclass_name, 8);
    }
    s_param_cnt++;

    memset(ch_fmt, 0x00, sizeof(ch_fmt));
    memcpy(ch_fmt, pch_format, strlen(pch_format));
    for (s_cnt=0,s_idx=0; s_idx < strlen(ch_fmt); s_idx++) if (ch_fmt[s_idx]==0x40) s_cnt++;
    va_start(va_ap, pch_format);
    pch_sp = ch_fmt;
    while (pch_sp[0] && loop_flg) {
        switch (pch_sp[0]) {
        case '@':
            pch_sp++;
            break;
        case 'X':       /* テキスト */
        case 'I':       /* 内部エラーコード */
        case 'S':       /* サーバークラス論理ID */
        case 'L':       /* GFP内部LCN */
        case 'C':       /* センターID */
        case 'T':       /* TCP/IPプロセス名 */
        case 'A':       /* 自IPアドレス */
        case 'a':       /* 接続先IPアドレス */
        case 'F':       /* ファイル物理名 */
        case 'f':       /* ファイル論理名 */
        case 'R':       /* プロセス名 */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, strlen(pch_ep));
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'K':       /* キー */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, strlen(pch_ep));
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, _min(strlen(ch_text), 40));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'i':       /* IPCヘッダ */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            HEX2CHAR(pch_ep, ch_text, 24);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl, ch_text, 48);
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '1':       /* 1桁BINARY(フラグ) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%01d", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '2':       /* 2桁BINARY(CPU番号) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%02d", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '3':       /* 3桁BINARY(内部テーブル数) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%03d", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '4':       /* 4桁BINARY(システムエラーコード) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%04d", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '5':       /* 5桁BINARY */
        case 'e':       /* エラーコード */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%05d", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'U':       /* 5桁BINARY(UNSIGN) */
        case 'M':       /* メッセージ番号 */
        case 'E':       /* エラーコード */
        case 'P':       /* 自ポート番号 */
        case 'p':       /* 接続先ポート番号 */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%05u", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '8':       /* 8桁BINARY */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%08", s_var);
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'D':       /* 受信電文 */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, sizeof(ch_text));
            memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                   ch_text,
                   sizeof(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        default:
            pch_sp++;
            break;
        }
        if (s_vcnt==s_cnt) break;
    }
    va_end(va_ap);

    /* 運用監視端末出力 */
    GFPOGGZ1(&g_cg010in);

} /* end of MDSO_message_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_module_ems_make                           */
/*  CALLING SEQ.    : void MDSO_module_ems_make  ( oggz1in_def* t_cg010in)  */
/*  ARGUMENT        : 1. t_cg010in        (I) EMSデータ設定領域             */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ作成処理                                    */
/****************************************************************************/
void MDSO_module_ems_make( oggz1in_def *t_cg010in )
{
    memcpy( t_cg010in, &g_cg010in, sizeof(oggz1in_def));

    /* サブルーチンリプライコードにスペース設定 */
    t_cg010in->subrcd = MDSO_SPACE;

    /* リターンコードにスペース設定 */
    t_cg010in->emsinf.rcd = MDSO_SPACE;

    /* メッセージIDにスペース設定 */
    memset(t_cg010in->emsinf.msgid, MDSO_SPACE, sizeof(t_cg010in->emsinf.msgid));

    /* EMS出力情報クリア */
    memset((char *)&t_cg010in->emsinf.emsgkinf, MDSO_SPACE, sizeof(t_cg010in->emsinf.emsgkinf));

    /* システム名 */
    memcpy( t_cg010in->emsinf.emsgkinf.sysnm,  DEF_EMS_SYSNM_GFP, strlen(DEF_EMS_SYSNM_GFP));

    /* server分類 */
    memcpy( t_cg010in->emsinf.emsgkinf.srv_kbn,  DEF_EMS_SRV_KBN_COM, strlen(DEF_EMS_SRV_KBN_COM));

    /* NW識別 被仕向 */
    if ( g_nw_info[0].nw_segment[0] != 0 ){
        memcpy( t_cg010in->emsinf.emsgkinf.h_nw_kbn,
                g_nw_info[0].nw_segment,
                sizeof(t_cg010in->emsinf.emsgkinf.h_nw_kbn));
    }

    /* NW識別 仕向 */
    memset( t_cg010in->emsinf.emsgkinf.s_nw_kbn,  MDSO_SPACE,
                      sizeof(t_cg010in->emsinf.emsgkinf.s_nw_kbn));

    /* プログラム名 */
    memcpy( t_cg010in->emsinf.emsgkinf.prgid,
            g_myinfo_def.proc_data_sub.module_id,
            sizeof(g_myinfo_def.proc_data_sub.module_id));

    /* プロセス名 */
    memcpy( t_cg010in->emsinf.emsgkinf.trmnm,
            g_myinfo_def.proc_data.my_pname,
            ZSYS_VAL_LEN_PROCESSNAME);

    /* 任意メッセージ */
    memset((char *)&t_cg010in->emsinf.emsnninf, MDSO_SPACE, sizeof(t_cg010in->emsinf.emsnninf));

} /* end of MDSO_module_ems_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_station_sts_read                          */
/*  CALLING SEQ.    : short MDSO_station_sts_read ( short con_list_idx,     */
/*                                                  char *station_sts )     */
/*  ARGUMENT        : 1. con_list_idx  (I) コネクションリストIDX            */
/*                    2. station_sts   (O) 局状態                           */
/*  RETURN CODE     : short 0:正常終了 1:異常終了                           */
/*  DESCRIPTION     : 局状態取得処理                                        */
/****************************************************************************/
short MDSO_station_sts_read ( short con_list_idx,
                              char *station_sts )
{
    short   s_ret;
    short   s_tbl_st_idx     = 0;
    short   s_tbl_if_idx     = 0;
    short   s_cst_get_flg    = 0;

    db_gcsst_def *station_st_tbl_local;

    /* 局状態管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def station_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def station_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def station_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def station_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* 局状態管理ファイル プライマリーキー ポインタ*/
    t_filekey_gcsst *gcsst_p_key;

    /* IDX設定 */
    s_tbl_st_idx = g_con_list[con_list_idx].line_list_st_idx;
    s_tbl_if_idx = g_line_list_st[s_tbl_st_idx].line_list_if_idx;

    /* 局状態取得済みチェック */
    s_cst_get_flg = MDSO_CST_GET_NON;
    if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_IF ) {
        if ( g_line_list_if[s_tbl_if_idx].station_sts_get == MDSO_CST_GET_DONE ) {
            memcpy(station_sts,
                   g_line_list_if[s_tbl_if_idx].station_sts,
                   MDSO_CST_SIZE);
            s_cst_get_flg = MDSO_CST_GET_DONE;
        }
    }
    else if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_ST ) {
        if ( g_line_list_st[s_tbl_st_idx].station_sts_get == MDSO_CST_GET_DONE ) {
            memcpy(station_sts,
                   g_line_list_st[s_tbl_st_idx].station_sts,
                   MDSO_CST_SIZE);
            s_cst_get_flg = MDSO_CST_GET_DONE;
        }
    }
    else {
        if ( g_con_list[con_list_idx].station_sts_get == MDSO_CST_GET_DONE ) {
            memcpy(station_sts,
                   g_con_list[con_list_idx].station_sts,
                   MDSO_CST_SIZE);
            s_cst_get_flg = MDSO_CST_GET_DONE;
        }
    }

    /* 局状態未取得の場合、局状態管理ファイルから取得 */
    if ( s_cst_get_flg == MDSO_CST_GET_NON ) {
        /* IOモジュールパラメータ初期化 */
        memset( &station_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(station_COM_IOM_arg_3_def) );
        memset( &station_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(station_COM_IOM_arg_4_def) );
        memset( &station_COM_IOM_arg_5_def, 0, sizeof(station_COM_IOM_arg_5_def) );
        memset( &station_COM_IOM_arg_6_def, 0, sizeof(station_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        /* 局状態管理ファイル読込み処理 */
        memcpy(station_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(station_COM_IOM_arg_3_def.prog_id));
        memcpy(station_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(station_COM_IOM_arg_3_def.file_name,
            g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_3_def.file_name));
        memcpy(station_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                    sizeof(station_COM_IOM_arg_3_def.file_io_type));
        memcpy(station_COM_IOM_arg_4_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(station_COM_IOM_arg_4_def.file_name,
            g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_4_def.file_name));
        station_COM_IOM_arg_4_def.file_no = g_file_data.station_sts_file_no;
        station_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(station_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(station_COM_IOM_arg_5_def.key_type));
        memset(station_COM_IOM_arg_5_def.key_value, MDSO_SPACE,
               sizeof(station_COM_IOM_arg_5_def.key_value));
        gcsst_p_key = (t_filekey_gcsst *)&station_COM_IOM_arg_5_def.key_value;
        gcsst_p_key->site_id = g_myinfo_def.config_data.site_id;
        gcsst_p_key->nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gcsst_p_key->grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gcsst_p_key->grp_id));
        if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_IF ) {
            memcpy(gcsst_p_key->if_id,
                   g_con_list[con_list_idx].interface_id,
                   sizeof(gcsst_p_key->if_id));
        }
        else if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_ST ) {
            memcpy(gcsst_p_key->if_id,
                   g_con_list[con_list_idx].interface_id,
                   sizeof(gcsst_p_key->if_id));
            memcpy(gcsst_p_key->station_id,
                   g_con_list[con_list_idx].station_id,
                   sizeof(gcsst_p_key->station_id));
        }
        else {
            memcpy(gcsst_p_key->if_id,
                   g_con_list[con_list_idx].interface_id,
                   sizeof(gcsst_p_key->if_id));
            memcpy(gcsst_p_key->station_id,
                   g_con_list[con_list_idx].station_id,
                   sizeof(gcsst_p_key->station_id));
            memcpy(gcsst_p_key->connect_id,
                   g_con_list[con_list_idx].connection_id,
                   sizeof(gcsst_p_key->connect_id));
        }
        station_COM_IOM_arg_5_def.key_len = DEF_GCSST_PKEY_LEN;
        station_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_EXACT;
        station_COM_IOM_arg_5_def.lock_flg = DEF_COM_IOM_NOLOCK;
        station_COM_IOM_arg_5_def.asc_desc_type = DEF_COM_IOM_ASCEND;
        station_COM_IOM_arg_5_def.io_timer = g_myinfo_def.config_data.io_timer;
        station_COM_IOM_arg_5_def.rec_len = sizeof(db_gcsst_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_STARTREAD,
            ch_sub_prog_sts,
            &station_COM_IOM_arg_3_def,
            &station_COM_IOM_arg_4_def,
            &station_COM_IOM_arg_5_def,
            &station_COM_IOM_arg_6_def);

        if (( s_ret != MDSO_RET_OK ) ||
            ( memcmp(ch_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) != 0 )) {
            /* 異常終了 */
            /* EMS出力 */
            MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                g_lcn,
                &g_rcv_gflin,
                DEF_FL_CEN_STS,
                DEF_COM_IOM_FUNC_STARTREAD,
                station_COM_IOM_arg_5_def.key_value,
                station_COM_IOM_arg_6_def.guardian_errcode);

            /* 内部エラーコード設定 */
            memcpy( g_internal_error_code,
                    DEF_NERR_FILE_IO_ERR,
                    sizeof(g_internal_error_code));

            return MDSO_RET_NG;
        }

        station_st_tbl_local = (db_gcsst_def *)&station_COM_IOM_arg_6_def.rec_area;

        memcpy(station_sts,
               station_st_tbl_local->state_sts_info.state_sts,
               MDSO_CST_SIZE);

        if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_IF ) {
            memcpy(g_line_list_if[s_tbl_if_idx].station_sts,
                   station_st_tbl_local->state_sts_info.state_sts,
                   MDSO_CST_SIZE);
            g_line_list_if[s_tbl_if_idx].station_sts_get = MDSO_CST_GET_DONE;
        }
        else if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_ST ) {
            memcpy(g_line_list_st[s_tbl_st_idx].station_sts,
                   station_st_tbl_local->state_sts_info.state_sts,
                   MDSO_CST_SIZE);
            g_line_list_st[s_tbl_st_idx].station_sts_get = MDSO_CST_GET_DONE;
        }
        else {
            memcpy(g_con_list[con_list_idx].station_sts,
                   station_st_tbl_local->state_sts_info.state_sts,
                   MDSO_CST_SIZE);
            g_con_list[con_list_idx].station_sts_get = MDSO_CST_GET_DONE;
        }
    }

    return MDSO_RET_OK;

} /* end of MDSO_station_sts_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSO_phy_file_read                             */
/*  CALLING SEQ.    : void MDSO_phy_file_read ( t_con_list* )               */
/*  ARGUMENT        : 1. con_list         (IO) 検索対象コネクションリスト   */
/*                                                (物理情報設定領域を含む)  */
/*  RETURN CODE     : short 0:正常終了 1:異常終了                           */
/*  DESCRIPTION     : 物理サーバークラス取得処理                            */
/****************************************************************************/
short MDSO_phy_file_read ( t_con_list *con_list )
{
    short s_ret           = 0;
    db_gfphi_def *phy_tbl_local;

    /* 物理名情報ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def phy_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def phy_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def phy_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def phy_COM_IOM_arg_6_def;
    char ch_phy_sub_prog_sts[2];

    /* 物理名情報ファイル プライマリーキー ポインタ*/
    t_filekey_gfphi *gfphi_p_key;

    /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
    memset( &phy_COM_IOM_arg_3_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSO_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts, 0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイル読込み開始処理 */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSO_FILEIO_TYPE_START,
                                sizeof(phy_COM_IOM_arg_3_def.file_io_type));
    memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_4_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
    phy_COM_IOM_arg_4_def.file_no          = g_file_data.phy_file_no;
    phy_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(phy_COM_IOM_arg_5_def.key_type));
    gfphi_p_key = (t_filekey_gfphi *)&phy_COM_IOM_arg_5_def.key_value;
    gfphi_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gfphi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfphi_p_key->pri_key_part1.grp_id,
           g_myinfo_def.config_data.group_id,
           sizeof(gfphi_p_key->pri_key_part1.grp_id));
    memcpy((char *)&gfphi_p_key->srv_cls_key.srv_cls_id,
           (char *)&con_list->server_class,
           sizeof(gfphi_p_key->srv_cls_key.srv_cls_id));
    memcpy(gfphi_p_key->srv_cls_key.srv_cls_mlt_num,
           MDSO_REDUN_ZERO,
           sizeof(gfphi_p_key->srv_cls_key.srv_cls_mlt_num));
    memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind,
           DEF_SC_NAME_DEFAULT,
           sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind));
    memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num,
           DEF_SC_NUM_DEFAULT,
           sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num));
    memcpy(gfphi_p_key->prc_file_key.prc_file_mlt_num,
           DEF_SC_DUP_DEFAULT,
           sizeof(gfphi_p_key->prc_file_key.prc_file_mlt_num));
    phy_COM_IOM_arg_5_def.key_len = DEF_GFPHI_PKEY_LEN;
    phy_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_EXACT;
    phy_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    phy_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    phy_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    phy_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfphi_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_phy_sub_prog_sts,
        &phy_COM_IOM_arg_3_def,
        &phy_COM_IOM_arg_4_def,
        &phy_COM_IOM_arg_5_def,
        &phy_COM_IOM_arg_6_def);

    if (( s_ret != MDSO_RET_OK) ||
        ( memcmp(ch_phy_sub_prog_sts, MDSO_IO_NORMAL_END, sizeof(ch_phy_sub_prog_sts)) != 0)) {
        /* エラー終了 */
        /* EMS出力 */
        MDSO_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E",
            g_lcn,
            &g_rcv_gflin,
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            phy_COM_IOM_arg_5_def.key_value,
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSO_RET_NG;
    }

    /* 物理名情報ファイル展開処理 */
    phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;

    /* 物理サーバークラスドメイン名を設定 */
    memcpy( (char *)&con_list->domain_name,
            (char *)&phy_tbl_local->srv_cls_info.domain_name,
            sizeof(con_list->domain_name) );
    /* 物理サーバークラスPATHMON名を設定 */
    memcpy( (char *)&con_list->pathmon_name,
            (char *)&phy_tbl_local->srv_cls_info.pathmon_name,
            sizeof(con_list->pathmon_name) );
    /* 物理サーバークラスを設定 */
    memcpy( (char *)&con_list->server_class_phy,
            (char *)&phy_tbl_local->srv_cls_info.srv_cls_name,
            sizeof(con_list->server_class_phy) );

    return MDSO_RET_OK;

} /* end of MDSO_phy_file_read */

