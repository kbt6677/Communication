/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX30                                    */
/*        FUNCTION          ････ 電文振分(inbound)                           */
/*                                                                           */
/*        AUTHER            ････ HAS kimura                                  */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-09-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 木村   2024/09/25 (電文振分(inbound))新規作成                   */
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
//#pragma  inspect,symbols,nostdfiles,saveabend,highpin,highrequesters,extensions

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <errno.h> nolist
#include <string.h> nolist
#include <stdlib.h> nolist
#include <stdarg.h> nolist
#include <tal.h> nolist
#include <ctype.h> nolist
#include <cextdecs.h> nolist
#include <zsysc> nolist

#include <stdio.h> nolist
#include <string.h> nolist
#include <errno.h> nolist
#include <tal.h> nolist
#include <zspic> nolist
#include <zfilc> nolist

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
#include "GFPCGXB0.h"
#include "GFPCGXD0.h"
#include "GFPCGXC0.h"
#include "GFPCGXG0.h"
#include "NWM_MTI.h"
#include "NWM_ENI.h"
#include "NWM_ENC.h"
#include "NWM_HDE.h"
#include "NWM_NTE.h"
#include "GFPCVX30.h"


/********** グローバルデータ ***********/
t_myinfo_def        g_myinfo_def;        /* 自プロセス情報               */
t_denbun_lct_info   g_denbun_lct_info[30*80];   /* 電文項目位置情報      */
short               g_denbun_lct_info_cnt;      /* 電文項目位置情報数    */
short               g_denbun_lct_info_no;       /* 電文項目位置情報配列番号 */
t_iocomp_def        g_iocomp;            /* I/O完了情報                  */
char                g_recv_buf[MDSI_MAX_DATA_SIZE]; /* $RECEIVE I/O用バッファ */
char                g_resp_buf[MDSI_MAX_DATA_SIZE]; /* 応答メッセージ用バッファ*/
c201_def            *g_rcv_req;          /* 受信電分設定バッファ         */
c502_def            *g_rcv_cmd_req;      /* 受信電分設定バッファ         */
short               g_msg_length;        /* 暗号復号後電文長             */
db_gqnwq_def        *g_tcv_req_dec;      /* 復号後受信電分設定バッファ   */
short               g_req_dat_len;       /* 要求電文部データ長           */
char                g_send_buf[MDSI_MAX_DATA_SIZE];/* PATHSEND用バッファ */
char                g_naibu_tranid[41];  /* 内部トランザクションID       */
char                g_snd_pname[6+1];    /* 送信元プロセス名             */
char                g_sys_datetime[8+1]; /* システム日時取得モジュール用 */
short               g_detour_tbl_cnt;    /* 受信電分振分先設定ファイルをメモリに展開した配列数 */
t_detour_tbl        g_detour_tbl[30];    /* 受信電分振分先設定ファイルをメモリに展開 */
t_tohan_list        g_tohan_list;        /* 東阪振分比率から振分順を決定し設定 */
t_tohan_rate        g_tohan_rate;        /* 東阪振分比率                 */
t_encdec_con        g_encdec_con;        /* 暗号化/復号実行時のATALLA接続情報  */
t_lcncon_data       g_lcncon_data;       /* GFP内部LCN採番接続情報       */
t_logcon_data       g_logcon_data;       /* ログ出力接続情報             */
char                g_lcn[15];           /* GFP内部LCN保存領域           */
short               g_mti_kind;          /* MTI取得対象メッセージ判定処理結果      */
char                g_mti_data[4];       /* MTI保存領域                  */
char                g_mti_save[4];       /* MTI保存領域(障害電文作成依頼設定用)    */
char                g_detour_data_no;    /* 受信電分振分先設定テーブルの送信先決定した配列番号 */
char                g_tohan_kind;        /* 送信先決定した東阪種別       */
char                g_tohan_proc_kind[2];/* 振分先選択処理区分           */
char                g_log_file_name[47]; /* 保存したログファイル名       */
t_file_data         g_file_data;         /* ファイル情報                 */
char                g_reply_info[10];    /* リプライ情報                 */
char                g_rcv_log_msg_kind[2];  /* ログ種別                  */
short               g_mti_judge;         /* MTI有無情報                  */
char                g_internal_error_code[7]; /* 内部エラーコード        */
char                g_rcv_if_id[5];       /* 受信インタフェース識別      */
char                g_rcv_station_id[6];  /* 受信ステーション識別        */
t_nw_info           g_nw_info[30*80];     /* NWファイル情報              */
short               g_nw_info_cnt;        /* NWファイル情報数            */
c601_def            g_st_log_ipc;         /* ログ格納領域                */
zsys_ddl_receiveinformation_def  g_recv_info; /* REPLY情報               */
short               g_date_start_posi;    /* 電文開始位置                */
short               g_mti_start_posi;     /* MTI開始位置                 */
t_err_log           g_err_log;            /* エラーログ情報              */
char                g_connect_unit;       /* コネクション管理単位        */
char                g_station_st_unit;    /* 開局/閉局管理単位           */
char                g_syogai_ind_unit;    /* 障害電文通知単位            */
char                g_station_sts;        /* 局状態                      */
COM_STP_arg_1_def   g_openersinfo;        /* オープナー用テーブル */
short               guardian_errcode_data[64];
short               *guardian_errcode = guardian_errcode_data;
char                g_error_add[64];
oggz1in_def         g_cg010in;            /* メッセージ出力   */
oggz1in_def         g_cg010in_modle;      /* メッセージ出力(共通モジュール用) */
COM_ERL_arg_2_def   g_COM_ERL_arg_2_def;  /* エラー出力ログファイル情報       */
gflin_pkey_def      g_rcv_gflin;          /* センターID                  */
char                g_mti_ng_flg;         /* 不正MTI受信フラグ           */


/* TRACE用変数 */
char  EXMYSRVCLSNAME[15];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;


/****************************************************************************/
/*  FUNCTION        : 1.1.0  main                                           */
/*  CALLING SEQ.    : int main (void)                                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0：プロセス終了                                       */
/*  DESCRIPTION     : メイン                                                */
/****************************************************************************/
int main (void)
{
    /* 初期化処理 */
    MDSI_initialize();

    /* プロセス起動 */
    /* EMS */
    MDSI_message_output(DEF_EVT_PROC_START,
                        DEF_NERR_NOMAL,
                        "@X", DEF_GFPCVX30);

    while ( g_myinfo_def.end_flag == MDSI_OFF ) {
        /* 主処理 */
        MDSI_main();
    }

    /* 終了処理 */
    MDSI_end();

    return MDSI_RET_OK;

} /* end of main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_initialize                                */
/*  CALLING SEQ.    : void MDSI_initialize (void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期化処理                                            */
/****************************************************************************/
void MDSI_initialize (void)
{
    short s_ret;
    lk_zac2001t_arg_1_def Trace_on;


    /* 局状態管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def station_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def station_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def station_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def station_COM_IOM_arg_6_def;
    char ch_station_sub_prog_sts[2];

    /* 東阪振分比率設定ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def tohan_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def tohan_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def tohan_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def tohan_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* グローバルデータ初期化 */
    memset( &g_myinfo_def,    0, sizeof(g_myinfo_def) );
    memset( &g_iocomp,        0, sizeof(g_iocomp) );
    memset( &g_recv_buf,      0, sizeof(g_recv_buf) );
    memset( &g_resp_buf,      0, sizeof(g_resp_buf) );
    g_msg_length = 0;
    g_req_dat_len = 0;
    memset( &g_send_buf,      0, sizeof(g_send_buf) );
    memset( g_naibu_tranid,   0, sizeof(g_naibu_tranid) );
    memset( &g_snd_pname,     0, sizeof(g_snd_pname) );
    memset( &g_sys_datetime,  0, sizeof(g_sys_datetime) );
    g_detour_tbl_cnt = 0;
    memset( &g_detour_tbl[0], 0, sizeof(g_detour_tbl) );
    memset( &g_tohan_list,    0, sizeof(g_tohan_list) );
    memset( &g_encdec_con,    0, sizeof(g_encdec_con) );
    memset( &g_lcn[0],        MDSI_SPACE, sizeof(g_lcn) );
    g_mti_kind = 0;
    memset( &g_mti_data,      0, sizeof(g_mti_data) );
    g_detour_data_no = 0;
    g_tohan_kind = ' ';
    memset( g_tohan_proc_kind, ' ', sizeof(g_tohan_proc_kind));
    memset( g_log_file_name,    0, sizeof(g_log_file_name) );
    memset( &g_file_data,       0, sizeof(g_file_data) );
    memset( &g_nw_info,         0, sizeof(g_nw_info) );
    memset( &g_denbun_lct_info, 0, sizeof(g_denbun_lct_info) );
    g_err_log.file_id = -1;

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
    memcpy(g_cg010in.emsinf.emsgkinf.prgid, DEF_GFPCVX30, strlen(DEF_GFPCVX30));
    memset((char *)&g_cg010in_modle, 0x20, sizeof(g_cg010in_modle));

    s_ret = MDSI_init_param();            /*パラメータ取得処理*/
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* TRACE初期化 */
    Trace_on.func_flg = '0';    /*トレース初期処理*/
    TRACEOUT((char *)&Trace_on);

    /* 物理名情報ファイル読み込み処理 */
    s_ret =  MDSI_init_getphyfile();
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* NW情報ファイル取得 */
    s_ret = MDSI_init_getnwfile();
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* 共通モジュール用EMSデータ作成 */
    MDSI_module_ems_make(&g_cg010in_modle);

    /* 暗号・復号処理用初期処理 */
    s_ret = MDSI_encdec_init();
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* 局状態管理ファイルオープン */
    /* IOモジュールパラメータ初期化 */
    memset( &station_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(station_COM_IOM_arg_3_def) );
    memset( &station_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(station_COM_IOM_arg_4_def) );
    memset( &station_COM_IOM_arg_5_def, 0, sizeof(station_COM_IOM_arg_5_def) );
    memset( &station_COM_IOM_arg_6_def, 0, sizeof(station_COM_IOM_arg_6_def) );
    memset( ch_station_sub_prog_sts, 0, sizeof(ch_station_sub_prog_sts) );

    /* 局状態管理ファイルオープン */
    memcpy(station_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(station_COM_IOM_arg_3_def.prog_id));
    memcpy(station_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(station_COM_IOM_arg_3_def.file_name,
        g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_3_def.file_name));
    memcpy(station_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_OPEN,
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
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_CEN_STS,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            station_COM_IOM_arg_6_def.guardian_errcode);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* ファイル番号設定 */
    g_file_data.station_sts_file_no = station_COM_IOM_arg_4_def.file_no;

    /* 東阪振分比率設定ファイルのオープン */
    /* IOモジュールパラメータ初期化 */
    memset( &tohan_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(tohan_COM_IOM_arg_3_def) );
    memset( &tohan_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(tohan_COM_IOM_arg_4_def) );
    memset( &tohan_COM_IOM_arg_5_def, 0, sizeof(tohan_COM_IOM_arg_5_def) );
    memset( &tohan_COM_IOM_arg_6_def, 0, sizeof(tohan_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 東阪振分比率設定ファイルのオープン */
    memcpy(tohan_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(tohan_COM_IOM_arg_3_def.prog_id));
    memcpy(tohan_COM_IOM_arg_3_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
    memcpy(tohan_COM_IOM_arg_3_def.file_name,
        g_file_data.tohan_rate_file_name, sizeof(tohan_COM_IOM_arg_3_def.file_name));
    memcpy(tohan_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_OPEN,
                                sizeof(tohan_COM_IOM_arg_3_def.file_io_type));
    memcpy(tohan_COM_IOM_arg_4_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
    memcpy(tohan_COM_IOM_arg_4_def.file_name,
        g_file_data.tohan_rate_file_name, sizeof(tohan_COM_IOM_arg_4_def.file_name));
    tohan_COM_IOM_arg_4_def.file_no = g_file_data.tohan_rate_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_sub_prog_sts,
        &tohan_COM_IOM_arg_3_def,
        &tohan_COM_IOM_arg_4_def,
        &tohan_COM_IOM_arg_5_def,
        &tohan_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            tohan_COM_IOM_arg_6_def.guardian_errcode);

        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* ファイル番号設定 */
    g_file_data.tohan_rate_file_no = tohan_COM_IOM_arg_4_def.file_no;

    /* 受信電文振分先ファイル読み込み処理 */
    s_ret = MDSI_detour_tbl_make();
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* 東阪振分比率設定テーブル作成処理 */
    s_ret = MDSI_detourlist_make();
    if ( MDSI_RET_OK != s_ret ) {
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* エラーログオープン */
    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &g_COM_ERL_arg_2_def, MDSI_SPACE, sizeof(g_COM_ERL_arg_2_def) );
    memset( &t_COM_ERL_arg_3_def, MDSI_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = MDSI_ERR_LOG_OPEN;
    t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_COM_ERL_arg_1_def.data_len     = 0;

    /* ファイル番号 -1設定 */
    g_COM_ERL_arg_2_def.file_no = -1;

    /* エラーログ共通処理実行 */
    s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                     &g_COM_ERL_arg_2_def,
                     &g_cg010in_modle,
                     &t_COM_ERL_arg_3_def,
                     g_myinfo_def.proc_data_sub.module_id);

    /* エラーログ結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,DEF_NERR_FILE_IO_ERR , "@X@E", "COM_ERL", s_ret);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

    /* $RECEIVEファイルオープン */
    s_ret = FILE_OPEN_(MDSI_RECEIVE_FILENAME,
                       (short)strlen(MDSI_RECEIVE_FILENAME),
                       &g_myinfo_def.recv_fno,
                       ZSYS_VAL_OPENACC_READWRITE,
                       ZSYS_VAL_OPENEXCL_SHARED,
                       MDSI_RCV_NOWAITDEPTH,
                       MDSI_RECVDEPTH);
    if ( MDSI_RET_OK != s_ret ) {
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            MDSI_RECEIVE_FILENAME,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            s_ret);
        /* 終了フラグON */
        g_myinfo_def.end_flag = MDSI_ON;
        return;
    }

} /* end of MDSI_initialize */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_init_param                                */
/*  CALLING SEQ.    : short MDSI_init_param (void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
short MDSI_init_param (void)
{
    char          ch_file_name[48];
    char          ch_file_id[32];
    short         s_file_name_len   = 0;
    char          ch_paramname[32];
    char          ch_param[64];
    short         s_result;
    short         ch_param_cnt       = 0;
    char          ch_prc_name_log[9] = DEF_PRC_FURI_I;
    unsigned long l_set_time         = 0;
    char          ch_set_time[5];
    char          ch_leng_local[3];
    unsigned int  i_leng_local       = 0;
    unsigned long l_time_data        = 0;

    memset(ch_file_name, 0, sizeof(ch_file_name));

    /* config格納領域初期化 */
    memset( &g_myinfo_def.config_data, 0, sizeof(g_myinfo_def.config_data) );

    /* オープナープロセス管理モジュール */
    COM_STP_INIT(&g_openersinfo);

    /* プロセス情報取得処理設定データ初期化 */
    memset( &g_myinfo_def.proc_data, MDSI_ZERO, sizeof(g_myinfo_def.proc_data));

    /* プロセス情報取得処理 */
    s_result = COM_PRC(&g_myinfo_def.proc_data);

    if ( s_result != MDSI_RET_OK ){
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_PRM_RD_ERR_INV,
                            "@X@E", "COM_PRC", s_result);

        return MDSI_RET_NG;
    }

    /* モジュールID */
    memcpy(g_myinfo_def.proc_data_sub.module_id, "GFPCVX30    ",
                 sizeof(g_myinfo_def.proc_data_sub.module_id));

    /* サーバクラス論理ID取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param, 0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_SRV_LOGICAL_ID, strlen(DEF_SRV_LOGICAL_ID));
    s_result = get_param_by_name(ch_paramname, ch_param, 23+1);
    if (s_result < 0) {
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        return MDSI_RET_NG;
    }
    if (strlen(ch_param) < 23) {
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        return MDSI_RET_NG;
    }

    /* サイト識別設定 */
    memcpy((char *)&g_myinfo_def.config_data.site_id, &ch_param[ch_param_cnt], MDSI_SITE_ID_LENG);
    ch_param_cnt += (MDSI_SITE_ID_LENG+1);
    /* N/W識別設定 */
    memcpy((char *)&g_myinfo_def.config_data.network_id, &ch_param[ch_param_cnt], MDSI_NW_ID_LENG);
    ch_param_cnt += (MDSI_NW_ID_LENG+1);
    /* グループ識別設定 */
    memcpy((char *)g_myinfo_def.config_data.group_id, &ch_param[ch_param_cnt], MDSI_GROUP_ID_LENG);
    ch_param_cnt += (MDSI_GROUP_ID_LENG+1);
    /* サーバクラス論理名設定 */
    memcpy((char *)g_myinfo_def.config_data.serverclass_name, &ch_param[ch_param_cnt], MDSI_SERVERCLASS_NAME_LENG);
    ch_param_cnt += (MDSI_SERVERCLASS_NAME_LENG+1);
    /* サーバクラス論理番号設定 */
    memcpy((char *)g_myinfo_def.config_data.serverclass_no, &ch_param[ch_param_cnt], MDSI_SERVERCLASS_NO_LENG);

    /* 物理名情報ファイル取得(ASSIGN情報) */
    memset(ch_file_id, ' ', sizeof(ch_file_id));
    memcpy( ch_file_id, DEF_ASN_GFPHI, strlen(DEF_ASN_GFPHI));
    COM_ASN(ch_file_id ,ch_file_name, &s_file_name_len );
    if ( (0 == s_file_name_len) || (47 < s_file_name_len) ){
        /* 物理名長異常 */
        MDSI_message_output(DEF_EVT_ASN_FILE_GET_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", "GFPHI", s_file_name_len);
        return MDSI_RET_NG;
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
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_MSG_SRV_NAME, s_result);
        return MDSI_RET_NG;
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
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_MSG_MON_NAME, s_result);
        return MDSI_RET_NG;
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
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_FILE_IO_TIMER_10MSECOND, s_result);
        return MDSI_RET_NG;
    }
    g_myinfo_def.config_data.io_timer = atol(ch_param);

    /* PATHSENDタイマー値取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param,     0, sizeof(ch_param));
    memcpy( ch_paramname, DEF_PSEND_TIMER_10MSECOND, strlen(DEF_PSEND_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_PSEND_TIMER_10MSECOND, s_result);
        return MDSI_RET_NG;
    }
    g_myinfo_def.config_data.send_timer = atol(ch_param);

    /* PATHSENDリトライ回数取得 */
    memset( ch_paramname, 0, sizeof(ch_paramname));
    memset( ch_param,     0, sizeof(ch_param));
    memcpy(ch_paramname, DEF_PSEND_RETRY_CNT, strlen(DEF_PSEND_RETRY_CNT));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        MDSI_message_output(DEF_EVT_CONFIG_ERR, DEF_NERR_PRM_RD_ERR_INV,
                            "@X@e", DEF_PSEND_RETRY_CNT, s_result);
        return MDSI_RET_NG;
    }
    g_myinfo_def.config_data.send_retry_count = atol(ch_param);

    /* 運用監視端末出力情報・プロセスI/Oタイマ(秒単位)   */
    memset(ch_set_time,0,5);
    l_set_time  = g_myinfo_def.config_data.send_timer / 100;
    l_time_data = g_myinfo_def.config_data.send_timer % 100;
    if ( l_time_data != 0){
        l_set_time += 1;
    }
    sprintf(ch_set_time, "%04d", l_set_time);
    memcpy(g_cg010in.uytrminf.proctimer,
        ch_set_time, 4);

    return MDSI_RET_OK;

} /* end of MDSI_init_param */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_init_getphyfile                           */
/*  CALLING SEQ.    : short MDSI_init_getphyfile (void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 物理名情報ファイル読み出し処理                        */
/****************************************************************************/
short MDSI_init_getphyfile (void)
{
    short s_ret = 0;
    char  ch_pathmon_set_flag = MDSI_OFF;
    char  ch_phy_set_flag = MDSI_OFF;
    char  ch_file_no[5];
    char  ch_log_data_set_flag = MDSI_OFF;
    char  ch_lcn_data_set_flag = MDSI_OFF;
    char  ch_asgn_set_flag = MDSI_OFF;
    char  ch_tohan_set_flag = MDSI_OFF;
    char  ch_nw_set_flag = MDSI_OFF;
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
    memset( &phy_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts, 0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイルオープン */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_OPEN,
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
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    g_file_data.phy_file_no = phy_COM_IOM_arg_4_def.file_no;

    /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
    memset( &phy_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
    memset( &phy_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
    memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
    memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
    memset( ch_phy_sub_prog_sts, 0, sizeof(ch_phy_sub_prog_sts) );

    /* 物理名情報ファイル読込み開始処理 */
    memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(phy_COM_IOM_arg_3_def.prog_id));
    memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(phy_COM_IOM_arg_3_def.file_name,
        g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
    memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
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
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_phy_sub_prog_sts,
        &phy_COM_IOM_arg_3_def,
        &phy_COM_IOM_arg_4_def,
        &phy_COM_IOM_arg_5_def,
        &phy_COM_IOM_arg_6_def);

    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            phy_COM_IOM_arg_5_def.key_value,
            phy_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    /* 物理名情報ファイル展開処理 */
    while (memcmp(ch_phy_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_phy_sub_prog_sts)) == 0) {
        phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;

        /* 自サーバクラス論理ID確認 */
        if ( 0 == memcmp( (char *)g_myinfo_def.config_data.serverclass_name,
                     (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                     sizeof(g_myinfo_def.config_data.serverclass_name))){

            /* サーバクラス論理ID一致 */

            /* PATHMONプロセス名取得*/
            if (( MDSI_ON    != ch_pathmon_set_flag ) &&
                ( MDSI_SPACE != phy_tbl_local->srv_cls_info.pathmon_name[0] )) {
                memcpy( (char *)g_myinfo_def.pathmon_name,
                        (char *)phy_tbl_local->srv_cls_info.pathmon_name,
                        sizeof(g_myinfo_def.pathmon_name) );
                ch_pathmon_set_flag = MDSI_ON;
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
            if ( ( MDSI_ON != ch_phy_set_flag ) &&
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
                ch_phy_set_flag = MDSI_ON;

            }
            else if ( ( MDSI_ON != ch_asgn_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_RCV_DEN_FURI,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 受信電分振分先設定ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.assign_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.assign_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.assign_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_asgn_set_flag = MDSI_ON;

            }
            else if ( ( MDSI_ON != ch_tohan_set_flag ) &&
                      ( 0 == memcmp( (char *)DEF_FL_FURI_RATE,
                         (char *)phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind,
                         sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_kind)))){
                /* 東阪振分比率設定ファイル */
                /* 物理ファイル名を取得 */
                memcpy( (char *)g_file_data.tohan_rate_file_name,
                        (char *)phy_tbl_local->prc_file_info.prc_file_name,
                        sizeof(g_file_data.tohan_rate_file_name) );
                /* ファイル番号を取得 */
                memset( ch_file_no, 0, sizeof(ch_file_no) );
                memcpy( ch_file_no,
                        phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
                        sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
                g_file_data.tohan_rate_file_no = (short)atoi(ch_file_no);
                /* 物理ファイル名設定済みフラグON */
                ch_tohan_set_flag = MDSI_ON;

            }
            else if ( ( MDSI_ON != ch_nw_set_flag ) &&
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
                ch_nw_set_flag = MDSI_ON;

            }
        }
        else if ( 0 == memcmp( (char *)DEF_SC_LOG_OUT,
                          (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                          sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind))) {
            /* サーバクラス論理IDがログ出力 */
            /* ログ出力サーバー情報取得*/
            if ( MDSI_ON != ch_log_data_set_flag ){
                if ((0 == memcmp(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num,
                                 MDSI_REDUN_ZERO,
                                 sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num))) &&
                    ( MDSI_SPACE != phy_tbl_local->srv_cls_info.srv_cls_name[0] )) {
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
                    ch_log_data_set_flag = MDSI_ON;
                }
            }
        }
        else if ( 0 == memcmp( (char *)DEF_SC_GFP_LCN,
                          (char *)phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                          sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind))) {
            /* サーバクラス論理IDがGFP内部LCN採番 */
            /* GFP内部LCN採番サーバー情報取得*/
            if ( MDSI_ON != ch_lcn_data_set_flag ){
                if ((0 == memcmp(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num,
                                 MDSI_REDUN_ZERO,
                                 sizeof(phy_tbl_local->pri_key.srv_cls_key.srv_cls_mlt_num))) &&
                    ( MDSI_SPACE != phy_tbl_local->srv_cls_info.srv_cls_name[0] )) {
                    memcpy( (char *)g_lcncon_data.domain_name,
                            (char *)phy_tbl_local->srv_cls_info.domain_name,
                            sizeof(g_lcncon_data.domain_name) );
                    memcpy( (char *)g_lcncon_data.pathmon_name,
                            (char *)phy_tbl_local->srv_cls_info.pathmon_name,
                            sizeof(g_lcncon_data.pathmon_name) );
                    memcpy( (char *)g_lcncon_data.server_class,
                            (char *)phy_tbl_local->srv_cls_info.srv_cls_name,
                            sizeof(g_lcncon_data.server_class) );
                    g_lcncon_data.pathsend_timer = g_myinfo_def.config_data.send_timer;
                    g_lcncon_data.retry_cnt = (short)g_myinfo_def.config_data.send_retry_count;
                    ch_lcn_data_set_flag = MDSI_ON;
                }
            }
        }
        /* 対象全ファイル設定済み判定 */
        if ( ( MDSI_ON == ch_phy_set_flag )      &&
             ( MDSI_ON == ch_asgn_set_flag )     &&
             ( MDSI_ON == ch_tohan_set_flag )    &&
             ( MDSI_ON == ch_nw_set_flag )       &&
             ( MDSI_ON == ch_log_data_set_flag ) &&
             ( MDSI_ON == ch_lcn_data_set_flag ) ){
            /* 設定完了ループ終了 正常終了*/
            return MDSI_RET_OK;
        }

        /* 物理名情報ファイルNEXTREAD処理 */
        memset( &phy_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
        memset( &phy_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
        memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
        memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );

        /* 物理名情報ファイルNEXTREAD処理 */
        memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(phy_COM_IOM_arg_3_def.prog_id));
        memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_3_def.file_name,
            g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
        memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_READ,
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

        if ( MDSI_RET_OK != s_ret ) {
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_NEXTREAD,
                phy_COM_IOM_arg_5_def.key_value,
                phy_COM_IOM_arg_6_def.guardian_errcode);

            return MDSI_RET_NG;
        }

    }  /* end of while 物理名情報ファイル展開処理 */

    /* 設定未完了 異常終了*/
    return MDSI_RET_NG;

} /* end of MDSI_init_getphyfile */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_init_getnwfile                            */
/*  CALLING SEQ.    : short MDSI_init_getnwfile (void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : NW情報ファイル取得処理                                */
/****************************************************************************/
short MDSI_init_getnwfile (void)
{
    short   s_ret = 0;
    db_gfnwi_def *nw_tbl_local;
    char    ch_save_data[16];

    /* 電文項目位置情報数 初期化 */
    g_denbun_lct_info_cnt = 0;

    /* コネクション管理単位初期化 */
    g_connect_unit = MDSI_SPACE;

    /* 開局/閉局管理単位 */
    g_station_st_unit = MDSI_SPACE;

    /* 障害電文通知単位 */
    g_syogai_ind_unit = MDSI_SPACE;

    /* NW情報ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def nw_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def nw_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def nw_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def nw_COM_IOM_arg_6_def;
    char ch_nw_sub_prog_sts[2];

    /* NW情報ファイル プライマリーキー ポインタ*/
    t_filekey_gfnwi *gfnwi_p_key;

    /* NW情報ファイル用IOモジュールパラメータ初期化 */
    memset( &nw_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
    memset( &nw_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
    memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
    memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );
    memset( ch_nw_sub_prog_sts,    0, sizeof(ch_nw_sub_prog_sts) );

    /* NW情報ファイルオープン */
    memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(nw_COM_IOM_arg_3_def.prog_id));
    memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_3_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
    memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_OPEN,
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
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_NW_INFO,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            nw_COM_IOM_arg_6_def.guardian_errcode);

        return MDSI_RET_NG;
    }

    g_file_data.nw_file_no = nw_COM_IOM_arg_4_def.file_no;

    /* NW情報ファイル用IOモジュールパラメータ初期化 */
    memset( &nw_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
    memset( &nw_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
    memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
    memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );
    memset( ch_nw_sub_prog_sts,    0, sizeof(ch_nw_sub_prog_sts) );

    /* NW情報ファイル読込み開始処理 */
    memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(nw_COM_IOM_arg_3_def.prog_id));
    memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(nw_COM_IOM_arg_3_def.file_name,
        g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
    memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
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

    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_NW_INFO,
            DEF_COM_IOM_FUNC_STARTREAD,
            nw_COM_IOM_arg_5_def.key_value,
            nw_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    /* NW情報ファイル展開処理 */
    while (memcmp(ch_nw_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_nw_sub_prog_sts)) == 0) {

        nw_tbl_local = (db_gfnwi_def *)nw_COM_IOM_arg_6_def.rec_area;

        /* 設定テーブル数最大長チェック */
        if ( g_denbun_lct_info_cnt > MDSI_IF_MAX ) {
            /* 設定テーブル数最大長オーバー */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TBL MAX OVER",
                "");
            return MDSI_RET_NG;
        }

        /* コネクション管理単位設定 */
        if (g_connect_unit == MDSI_SPACE){
            g_connect_unit = nw_tbl_local->mng_lyr_info.connect_num_mng_lyr;
        }

        /* 開局/閉局管理単位設定 */
        if (g_station_st_unit == MDSI_SPACE){
            g_station_st_unit = nw_tbl_local->mng_lyr_info.open_close_mng_lyr;
        }

        /* 障害電文通知単位設定 */
        if (g_syogai_ind_unit == MDSI_SPACE){
            g_syogai_ind_unit = nw_tbl_local->shori_kbn_info.syogai_tuuchi_need;
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

        if ( g_nw_info[0].nw_segment[0] == MDSI_SPACE ){
            if ( nw_tbl_local->nw_id_info.nw_kubun[0] != MDSI_SPACE ){
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

        g_denbun_lct_info_cnt += 1;

        g_nw_info_cnt = g_denbun_lct_info_cnt;

        /* NW情報ファイルNEXTREAD処理 */
        memset( &nw_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_3_def) );
        memset( &nw_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(nw_COM_IOM_arg_4_def) );
        memset( &nw_COM_IOM_arg_5_def, 0, sizeof(nw_COM_IOM_arg_5_def) );
        memset( &nw_COM_IOM_arg_6_def, 0, sizeof(nw_COM_IOM_arg_6_def) );

        /* NW情報ファイルNEXTREAD処理 */
        memcpy(nw_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(nw_COM_IOM_arg_3_def.prog_id));
        memcpy(nw_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(nw_COM_IOM_arg_3_def.file_name,
            g_file_data.nw_file_name, sizeof(nw_COM_IOM_arg_3_def.file_name));
        memcpy(nw_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_READ,
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
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_nw_sub_prog_sts,
            &nw_COM_IOM_arg_3_def,
            &nw_COM_IOM_arg_4_def,
            &nw_COM_IOM_arg_5_def,
            &nw_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ) {
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_NEXTREAD,
                nw_COM_IOM_arg_5_def.key_value,
                nw_COM_IOM_arg_6_def.guardian_errcode);
            return MDSI_RET_NG;
        }
    }  /* end of while NW情報ファイル展開処理 */

    /* 設定完了 */
    return MDSI_RET_OK;

} /* end of MDSI_init_getnwfile */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_encdec_init                               */
/*  CALLING SEQ.    : short MDSI_encdec_init (void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 暗号・復号処理用初期処理                              */
/****************************************************************************/
short MDSI_encdec_init (void)
{
    short s_ret = MDSI_RET_NG;

    /* 個別モジュールパラメータ*/
    NWM_ENI_arg_1_def t_NWM_ENI_arg_1_def;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def_other;
    NWM_ENI_arg_3_def t_NWM_ENI_arg_3_def;
    NWM_ENI_arg_4_def t_NWM_ENI_arg_4_def;
    char ch_module_id[8+1];
    ems_info_add    ems_info_add_local;

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_ENI_arg_1_def, MDSI_SPACE, sizeof(t_NWM_ENI_arg_1_def) );
    memset( &t_NWM_ENI_arg_2_def,       0, sizeof(t_NWM_ENI_arg_2_def) );
    memset( &t_NWM_ENI_arg_2_def_other, 0, sizeof(t_NWM_ENI_arg_2_def_other) );
    memset( &t_NWM_ENI_arg_3_def,       0, sizeof(t_NWM_ENI_arg_3_def) );
    memset( &t_NWM_ENI_arg_4_def,       0, sizeof(t_NWM_ENI_arg_4_def) );
    memset( ch_module_id,               0, sizeof(ch_module_id) );
    memcpy( ch_module_id, g_myinfo_def.proc_data_sub.module_id, sizeof(ch_module_id) );
    memset( &ems_info_add_local,        0, sizeof(ems_info_add));

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
                    &t_NWM_ENI_arg_2_def,
                    &t_NWM_ENI_arg_2_def_other,
                    &t_NWM_ENI_arg_3_def,
                    &t_NWM_ENI_arg_4_def,
                    ch_module_id,
                    &g_cg010in_modle,
                    &ems_info_add_local
                    );

    /* 個別モジュール結果判定 */
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "NWM_ENI", s_ret);

        return MDSI_RET_NG;
    }

    /* グローバル領域初期化 */
    memset( &g_encdec_con, 0, sizeof(g_encdec_con) );

    /* 暗号・復号情報格納処理 */
    /* 鍵管理ファイルID */
    memcpy( g_encdec_con.dec_start_data.key_file_id, t_NWM_ENI_arg_2_def.file_id,
                          sizeof(g_encdec_con.dec_start_data.key_file_id) );
    /* 鍵管理ファイル名 */
    memcpy( g_encdec_con.dec_start_data.key_file_name, t_NWM_ENI_arg_2_def.file_name,
                          sizeof(g_encdec_con.dec_start_data.key_file_name) );
    /* 鍵管理ファイル番号 */
    g_encdec_con.dec_start_data.key_file_no    = t_NWM_ENI_arg_2_def.file_no;
    /* 鍵管理I/Oタイマー */
    g_encdec_con.dec_start_data.key_io_timer   = t_NWM_ENI_arg_2_def.io_timer;
    /* PATHMONプロセス名 */
    memcpy( g_encdec_con.dec_start_data.pathmon_name, t_NWM_ENI_arg_3_def.domain_name,
                          sizeof(g_encdec_con.dec_start_data.pathmon_name) );
    /* サーバクラス名 */
    memcpy( g_encdec_con.dec_start_data.server_class, t_NWM_ENI_arg_3_def.server_name,
                          sizeof(g_encdec_con.dec_start_data.server_class) );
    /* PATHSENDタイマー */
    g_encdec_con.dec_start_data.pathsend_timer = t_NWM_ENI_arg_3_def.pathsend_timer;
    /* PATHSENDリトライ回数 */
    g_encdec_con.dec_start_data.retry_cnt      = t_NWM_ENI_arg_3_def.pathsend_retry_cnt;

    return MDSI_RET_OK;

} /* end of MDSI_encdec_init */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_detour_tbl_make                           */
/*  CALLING SEQ.    : short MDSI_detour_tbl_make ( void )                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 受信電文振分先ファイル読み込み処理                    */
/****************************************************************************/
short MDSI_detour_tbl_make(void)
{
    short s_ret                 = MDSI_RET_NG;
    short loop_cnt              = 0;
    char  ch_phy_tokyo_set_flag = MDSI_OFF;
    char  ch_phy_osaka_set_flag = MDSI_OFF;
    db_gfqsw_def *detour_tbl_local;
                                /* 受信電分振分先一時設定テーブル */
    db_gfphi_def *phy_tbl_local;

    /* 受信電文振分先ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def ass_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def ass_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def ass_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def ass_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* 物理名情報ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def phy_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def phy_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def phy_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def phy_COM_IOM_arg_6_def;
    char ch_phy_sub_prog_sts[2];

    /* 受信電文振分先設定ファイル プライマリーキー ポインタ*/
    t_filekey_gfqsw *gfqsw_p_key;

    /* 物理名情報ファイル プライマリーキー ポインタ*/
    t_filekey_gfphi *gfphi_p_key;

    /* 受信電分振分先設定テーブル作成 */
    memset(&g_detour_tbl[0], 0x00, sizeof(g_detour_tbl));
  
    /* 東阪振分順リスト初期化 */
    memset(&g_tohan_list, 0x00, sizeof(g_tohan_list));

    /* IOモジュールパラメータ初期化 */
    memset( &ass_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_3_def) );
    memset( &ass_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_4_def) );
    memset( &ass_COM_IOM_arg_5_def, 0, sizeof(ass_COM_IOM_arg_5_def) );
    memset( &ass_COM_IOM_arg_6_def, 0, sizeof(ass_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 受信電分振分先設定ファイル読み込み処理 */
    /* 受信電文振分先ファイルのオープン */
    memcpy(ass_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(ass_COM_IOM_arg_3_def.prog_id));
    memcpy(ass_COM_IOM_arg_3_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
    memcpy(ass_COM_IOM_arg_3_def.file_name,
        g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_3_def.file_name));
    memcpy(ass_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_OPEN,
                                sizeof(ass_COM_IOM_arg_3_def.file_io_type));
    memcpy(ass_COM_IOM_arg_4_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
    memcpy(ass_COM_IOM_arg_4_def.file_name,
        g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_4_def.file_name));
    ass_COM_IOM_arg_4_def.file_no = g_file_data.assign_file_no;

    /* IOモジュール */
    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_OPEN,
        ch_sub_prog_sts,
        &ass_COM_IOM_arg_3_def,
        &ass_COM_IOM_arg_4_def,
        &ass_COM_IOM_arg_5_def,
        &ass_COM_IOM_arg_6_def);

    /* IOモジュール結果判定 */
    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_RCV_DEN_FURI,
            DEF_COM_IOM_FUNC_OPEN,
            "",
            ass_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    g_file_data.assign_file_no = ass_COM_IOM_arg_4_def.file_no;

    /* IOモジュールパラメータ初期化 */
    memset( &ass_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_3_def) );
    memset( &ass_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_4_def) );
    memset( &ass_COM_IOM_arg_5_def, 0, sizeof(ass_COM_IOM_arg_5_def) );
    memset( &ass_COM_IOM_arg_6_def, 0, sizeof(ass_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts,        0, sizeof(ch_sub_prog_sts) );

    /* 受信電文振分先ファイル読込み開始処理 */
    memcpy(ass_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(ass_COM_IOM_arg_3_def.prog_id));
    memcpy(ass_COM_IOM_arg_3_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
    memcpy(ass_COM_IOM_arg_3_def.file_name,
        g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_3_def.file_name));
    memcpy(ass_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
                                sizeof(ass_COM_IOM_arg_3_def.file_io_type));
    memcpy(ass_COM_IOM_arg_4_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
    memcpy(ass_COM_IOM_arg_4_def.file_name,
        g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_4_def.file_name));
    ass_COM_IOM_arg_4_def.file_no = g_file_data.assign_file_no;
    ass_COM_IOM_arg_5_def.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(ass_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(ass_COM_IOM_arg_5_def.key_type));
    gfqsw_p_key = (t_filekey_gfqsw *)&ass_COM_IOM_arg_5_def.key_value;
    gfqsw_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
    gfqsw_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfqsw_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gfqsw_p_key->pri_key_part1.grp_id));
    ass_COM_IOM_arg_5_def.key_len = sizeof(gfqsw_p_key->pri_key_part1);
    ass_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
    ass_COM_IOM_arg_5_def.lock_flg = DEF_COM_IOM_NOLOCK;
    ass_COM_IOM_arg_5_def.asc_desc_type = DEF_COM_IOM_ASCEND;
    ass_COM_IOM_arg_5_def.io_timer = g_myinfo_def.config_data.io_timer;
    ass_COM_IOM_arg_5_def.rec_len = sizeof(db_gfqsw_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_sub_prog_sts,
        &ass_COM_IOM_arg_3_def,
        &ass_COM_IOM_arg_4_def,
        &ass_COM_IOM_arg_5_def,
        &ass_COM_IOM_arg_6_def);

    if ( MDSI_RET_OK != s_ret ) {
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_RCV_DEN_FURI,
            DEF_COM_IOM_FUNC_STARTREAD,
            ass_COM_IOM_arg_5_def.key_value,
            ass_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    /* 受信電文振分先ファイル展開処理 */
    while (memcmp(ch_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) == 0) {
        detour_tbl_local = (db_gfqsw_def *)ass_COM_IOM_arg_6_def.rec_area;

        /* 設定テーブル数最大長チェック */
        if ( g_detour_tbl_cnt >= MDSI_DETOUR_TBL_MAX ) {
            /* 設定テーブル数最大長オーバー */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TBL MAX OVER",
                g_detour_tbl_cnt);

            return MDSI_RET_NG;
        }
        /* レコード項目チェック(MTI) */
        if (isdigit(detour_tbl_local->pri_key.mti_id[0]) &&
            isdigit(detour_tbl_local->pri_key.mti_id[1]) &&
            isdigit(detour_tbl_local->pri_key.mti_id[2]) &&
            isdigit(detour_tbl_local->pri_key.mti_id[3])) {
            /* MTI正常(数字) */
        }
        else {
            /* MTI確認 YYY1,YYY2,ZZZZ */
            if ( ( 0 == memcmp( detour_tbl_local->pri_key.mti_id, "YYY1", 4 )) ||
                 ( 0 == memcmp( detour_tbl_local->pri_key.mti_id, "YYY2", 4 )) ||
                 ( 0 == memcmp( detour_tbl_local->pri_key.mti_id, "ZZZZ", 4 ))) {
                /* MTI正常(数字以外) */
            }
            else
            {
                /* MTI異常 */
                /* エラー終了 */
                /* EMS出力 */
                MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                    "@L@C@f@X@K@E","","",
                    DEF_FL_RCV_DEN_FURI,
                    DEF_COM_IOM_FUNC_STARTREAD,
                    "MTI NG",
                    0);

                return MDSI_RET_NG;
            }
        }
        /* レコード項目チェック(東阪振分区分) */
        if ( ( detour_tbl_local->furiwake_cntrl_info.tky_osk_kubun == MDSI_TOHAN_DETOUR_1_char ) ||
             ( detour_tbl_local->furiwake_cntrl_info.tky_osk_kubun == MDSI_TOHAN_DETOUR_2_char ) ||
             ( detour_tbl_local->furiwake_cntrl_info.tky_osk_kubun == MDSI_TOHAN_DETOUR_3_char ) ) {
            /* 東阪振分区分正常 */
        }
        else{
            /* 東阪振分区分異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "TOHAN KUBUN NG",
                0);

            return MDSI_RET_NG;
        }
        /* レコード項目チェック(東京サイト サーバクラス種類) */
        if ( ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[0] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[1] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[2] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[3] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[4] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[5] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[6] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_kind[7] ) ) ) {
            /* サーバクラス種類正常 */
        }
        else{
            /* サーバクラス種類異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS NG",
                0);

            return MDSI_RET_NG;
        }
        /* レコード項目チェック(東京サイト サーバクラス論理番号) */
        if ( ( isdigit( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_num[0] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_num[1] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_num[2] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.tky_site_info.srv_cls_id.srv_cls_num[3] ) ) ) {
            /* サーバクラス論理番号正常 */
        }
        else{
            /* サーバクラス論理番号異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS NUM NG",
                0);
            return MDSI_RET_NG;
        }
        /* レコード項目チェック(大阪サイト サーバクラス種類) */
        if ( ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[0] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[1] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[2] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[3] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[4] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[5] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[6] ) ) &&
             ( isalpha( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_kind[7] ) ) ) {
            /* サーバクラス種類正常 */
        }
        else{
            /* サーバクラス種類異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS KIND NG",
                0);

            return MDSI_RET_NG;
        }
        /* レコード項目チェック(大阪サイト サーバクラス論理番号) */
        if ( ( isdigit( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_num[0] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_num[1] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_num[2] ) ) &&
             ( isdigit( detour_tbl_local->recv_qfile_info.osk_site_info.srv_cls_id.srv_cls_num[3] ) ) ) {
            /* サーバクラス論理番号正常 */
        }
        else{
            /* サーバクラス論理番号異常 */
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_STARTREAD,
                "SERVER CLASS NUM NG",
                0);

            return MDSI_RET_NG;
        }
        /* 受信電文振分先ファイルからグローバルテーブルに設定 */
        memcpy(&g_detour_tbl[g_detour_tbl_cnt].t_primary_key.mti[0],
               &detour_tbl_local->pri_key.mti_id,
               (sizeof(db_gfqsw_def)-sizeof(detour_tbl_local->future_use)-7));
        /* 設定テーブル数更新 */
        g_detour_tbl_cnt += 1;

        /* IOモジュールパラメータ初期化 */
        memset( &ass_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_3_def) );
        memset( &ass_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(ass_COM_IOM_arg_4_def) );
        memset( &ass_COM_IOM_arg_5_def, 0, sizeof(ass_COM_IOM_arg_5_def) );
        memset( &ass_COM_IOM_arg_6_def, 0, sizeof(ass_COM_IOM_arg_6_def) );

        /* 受信電文振分先ファイルNEXTREAD処理 */
        memcpy(ass_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(ass_COM_IOM_arg_3_def.prog_id));
        memcpy(ass_COM_IOM_arg_3_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
        memcpy(ass_COM_IOM_arg_3_def.file_name,
            g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_3_def.file_name));
        memcpy(ass_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_READ,
                                    sizeof(ass_COM_IOM_arg_3_def.file_io_type));
        memcpy(ass_COM_IOM_arg_4_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
        memcpy(ass_COM_IOM_arg_4_def.file_name,
            g_file_data.assign_file_name, sizeof(ass_COM_IOM_arg_4_def.file_name));
        ass_COM_IOM_arg_4_def.file_no          = g_file_data.assign_file_no;
        ass_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(ass_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                          sizeof(ass_COM_IOM_arg_5_def.key_type));
        gfqsw_p_key = (t_filekey_gfqsw *)&ass_COM_IOM_arg_5_def.key_value;
        gfqsw_p_key->pri_key_part1.site_id = g_myinfo_def.config_data.site_id;
        gfqsw_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfqsw_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfqsw_p_key->pri_key_part1.grp_id));
        ass_COM_IOM_arg_5_def.key_len = sizeof(gfqsw_p_key->pri_key_part1);
        ass_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_GENERIC;
        ass_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
        ass_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
        ass_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
        ass_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfqsw_def);

        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_NEXTREAD,
            ch_sub_prog_sts,
            &ass_COM_IOM_arg_3_def,
            &ass_COM_IOM_arg_4_def,
            &ass_COM_IOM_arg_5_def,
            &ass_COM_IOM_arg_6_def);

        /* 受信電文振分先ファイル情報異常終了 */
        if ( MDSI_RET_OK != s_ret ) {
            /* エラー終了 */
            /* ファイルI/Oエラー(EMS) */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_NEXTREAD,
                ass_COM_IOM_arg_5_def.key_value,
                ass_COM_IOM_arg_6_def.guardian_errcode);
                return MDSI_RET_NG;
        }

    } /* end of while 受信電文振分先ファイル展開処理 */

    /* サーバクラス情報取得 */
    for ( loop_cnt=0; loop_cnt<g_detour_tbl_cnt; loop_cnt++ ){
        /* 東京サーバクラス取得 */
        /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
        memset( &phy_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
        memset( &phy_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
        memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
        memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
        memset( ch_phy_sub_prog_sts, 0, sizeof(ch_phy_sub_prog_sts) );

        /* 物理名情報ファイル読込み開始処理 */
        memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                             sizeof(phy_COM_IOM_arg_3_def.prog_id));
        memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_3_def.file_name,
                g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
        memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
                                        sizeof(phy_COM_IOM_arg_3_def.file_io_type));
        memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_4_def.file_name,
                g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
        phy_COM_IOM_arg_4_def.file_no          = g_file_data.phy_file_no;
        phy_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                            sizeof(phy_COM_IOM_arg_5_def.key_type));
        gfphi_p_key = (t_filekey_gfphi *)&phy_COM_IOM_arg_5_def.key_value;
        gfphi_p_key->pri_key_part1.site_id = DEF_SITE_ID_TKY;
        gfphi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfphi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfphi_p_key->pri_key_part1.grp_id));
        memcpy((char *)&gfphi_p_key->srv_cls_key.srv_cls_id,
               (char *)&g_detour_tbl[loop_cnt].t_rcv_que.t_tokyo_site.t_tokyo_serverclass,
                                       sizeof(gfphi_p_key->srv_cls_key.srv_cls_id));
        memcpy(gfphi_p_key->srv_cls_key.srv_cls_mlt_num, MDSI_REDUN_ZERO,
                                  sizeof(gfphi_p_key->srv_cls_key.srv_cls_mlt_num));
        memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind, DEF_SC_NAME_DEFAULT,
                       sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind));
        memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT,
                        sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num));
        memcpy(gfphi_p_key->prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT,
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

        if (( s_ret != MDSI_RET_OK) ||
            ( memcmp(ch_phy_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_phy_sub_prog_sts)) != 0)) {
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_STARTREAD,
                phy_COM_IOM_arg_5_def.key_value,
                phy_COM_IOM_arg_6_def.guardian_errcode);
            return MDSI_RET_NG;
        }

        phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;
        memcpy((char *)&g_detour_tbl[loop_cnt].t_phy_data.t_tokyo_site_phy,
               (char *)&phy_tbl_local->srv_cls_info,
               sizeof(g_detour_tbl[loop_cnt].t_phy_data.t_tokyo_site_phy));

        /* 大阪サーバクラス取得 */
        /* 物理名情報ファイル用IOモジュールパラメータ初期化 */
        memset( &phy_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_3_def) );
        memset( &phy_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(phy_COM_IOM_arg_4_def) );
        memset( &phy_COM_IOM_arg_5_def, 0, sizeof(phy_COM_IOM_arg_5_def) );
        memset( &phy_COM_IOM_arg_6_def, 0, sizeof(phy_COM_IOM_arg_6_def) );
        memset( ch_phy_sub_prog_sts, 0, sizeof(ch_phy_sub_prog_sts) );

        /* 物理名情報ファイル読込み開始処理 */
        memcpy(phy_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                             sizeof(phy_COM_IOM_arg_3_def.prog_id));
        memcpy(phy_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_3_def.file_name,
                g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_3_def.file_name));
        memcpy(phy_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
                                        sizeof(phy_COM_IOM_arg_3_def.file_io_type));
        memcpy(phy_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(phy_COM_IOM_arg_4_def.file_name,
                g_file_data.phy_file_name, sizeof(phy_COM_IOM_arg_4_def.file_name));
        phy_COM_IOM_arg_4_def.file_no          = g_file_data.phy_file_no;
        phy_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
        memcpy(phy_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                            sizeof(phy_COM_IOM_arg_5_def.key_type));
        gfphi_p_key = (t_filekey_gfphi *)&phy_COM_IOM_arg_5_def.key_value;
        gfphi_p_key->pri_key_part1.site_id = DEF_SITE_ID_OSK;
        gfphi_p_key->pri_key_part1.nw_id = g_myinfo_def.config_data.network_id;
        memcpy(gfphi_p_key->pri_key_part1.grp_id, g_myinfo_def.config_data.group_id,
                                           sizeof(gfphi_p_key->pri_key_part1.grp_id));
        memcpy((char *)&gfphi_p_key->srv_cls_key.srv_cls_id,
               (char *)&g_detour_tbl[loop_cnt].t_rcv_que.t_tokyo_site.t_tokyo_serverclass,
                                       sizeof(gfphi_p_key->srv_cls_key.srv_cls_id));
        memcpy(gfphi_p_key->srv_cls_key.srv_cls_mlt_num, MDSI_REDUN_ZERO,
                                  sizeof(gfphi_p_key->srv_cls_key.srv_cls_mlt_num));
        memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind, DEF_SC_NAME_DEFAULT,
                       sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_kind));
        memcpy(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT,
                        sizeof(gfphi_p_key->prc_file_key.prc_file_id.prc_file_num));
        memcpy(gfphi_p_key->prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT,
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

        if (( s_ret != MDSI_RET_OK) ||
            ( memcmp(ch_phy_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_phy_sub_prog_sts)) != 0)) {
            /* エラー終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_STARTREAD,
                phy_COM_IOM_arg_5_def.key_value,
                phy_COM_IOM_arg_6_def.guardian_errcode);
            return MDSI_RET_NG;
        }

        phy_tbl_local = (db_gfphi_def *)phy_COM_IOM_arg_6_def.rec_area;
        memcpy((char *)&g_detour_tbl[loop_cnt].t_phy_data.t_osaka_site_phy,
               (char *)&phy_tbl_local->srv_cls_info,
               sizeof(g_detour_tbl[loop_cnt].t_phy_data.t_tokyo_site_phy));

    }  /* end of while (loop_cnt) */

    return MDSI_RET_OK;

} /* end of MDSI_detour_tbl_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_detourlist_make                           */
/*  CALLING SEQ.    : short MDSI_detourlist_make ( void )                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 東阪振分比率設定テーブル作成処理                      */
/****************************************************************************/
short MDSI_detourlist_make(void)
{
    short s_ret = MDSI_RET_NG;
    char ch_tokyo_ratio_char[4] = {0}; /* 東阪振分比率(東京) */
    char ch_osaka_ratio_char[4] = {0}; /* 東阪振分比率(大阪) */
    int  i_tokyo_ratio = 0;
    int  i_osaka_ratio = 0;
    int  i_ratio_total = 0;
    int  sort_list_cnt = 0;
    t_tohan_list tohan_list_local;    /* 東阪振分順リスト一時設定テーブル */
    db_gfnsw_def *tohan_tbl_local;    /* 東阪振分順ファイル一時設定テーブル */
    int  i_kouyaku = 0;
    int  i_kouyaku_tokyo = 0;
    int  i_kouyaku_osaka = 0;
    int  i_kouyaku_a = 0;
    int  i_kouyaku_b = 0;
    int  i_list_total = 0;
    int  i_rate = 0;
    int  i_kirisute_rate = 0;
    int  i_kirisute_kukan = 0;
    int  i_kirisute_kukan_cnt = 0;
    int  i_kiriage_rate = 0;
    int  i_kiriage_kukan = 0;
    int  i_kiriage_kukan_cnt = 0;
    int  i_kirisute_rate_cnt = 0;
    int  i_kiriage_rate_cnt = 0;

    /* 東阪振分比率設定ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def tohan_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def tohan_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def tohan_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def tohan_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* 東阪振分比率設定ファイル プライマリーキー ポインタ*/
    db_gfnsw_def *gfnsw_p_key;

    /* 東阪振分比率設定テーブル作成 */
    memset(&tohan_list_local, 0x00, sizeof(tohan_list_local));

    /* 東阪振分順リスト初期化 */
    memset(&g_tohan_list, 0x00, sizeof(g_tohan_list));

    /* 東阪振分比率設定ファイル読み込み処理 */
    /* IOモジュールパラメータ初期化 */
    memset( &tohan_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(tohan_COM_IOM_arg_3_def) );
    memset( &tohan_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(tohan_COM_IOM_arg_4_def) );
    memset( &tohan_COM_IOM_arg_5_def, 0, sizeof(tohan_COM_IOM_arg_5_def) );
    memset( &tohan_COM_IOM_arg_6_def, 0, sizeof(tohan_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 東阪振分比率設定ファイル読込み開始処理 */
    memcpy(tohan_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(tohan_COM_IOM_arg_3_def.prog_id));
    memcpy(tohan_COM_IOM_arg_3_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
    memcpy(tohan_COM_IOM_arg_3_def.file_name,
        g_file_data.tohan_rate_file_name, sizeof(tohan_COM_IOM_arg_3_def.file_name));
    memcpy(tohan_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
                                sizeof(tohan_COM_IOM_arg_3_def.file_io_type));
    memcpy(tohan_COM_IOM_arg_4_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
    memcpy(tohan_COM_IOM_arg_4_def.file_name,
        g_file_data.tohan_rate_file_name, sizeof(tohan_COM_IOM_arg_4_def.file_name));
    tohan_COM_IOM_arg_4_def.file_no          = g_file_data.tohan_rate_file_no;
    tohan_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(tohan_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(tohan_COM_IOM_arg_5_def.key_type));
    gfnsw_p_key = (db_gfnsw_def *)&tohan_COM_IOM_arg_5_def.key_value;
    gfnsw_p_key->pri_key.site_id = g_myinfo_def.config_data.site_id;
    gfnsw_p_key->pri_key.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gfnsw_p_key->pri_key.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gfnsw_p_key->pri_key.grp_id));
    tohan_COM_IOM_arg_5_def.key_len = sizeof(gfnsw_p_key->pri_key);
    tohan_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_EXACT;
    tohan_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    tohan_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    tohan_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    tohan_COM_IOM_arg_5_def.rec_len          = sizeof(db_gfnsw_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_sub_prog_sts,
        &tohan_COM_IOM_arg_3_def,
        &tohan_COM_IOM_arg_4_def,
        &tohan_COM_IOM_arg_5_def,
        &tohan_COM_IOM_arg_6_def);

    if (( s_ret != MDSI_RET_OK ) ||
        ( memcmp(ch_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) != 0)) {
        /* エラー終了 */
        /* ファイルI/Oエラー(EMS) */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_STARTREAD,
            tohan_COM_IOM_arg_5_def.key_value,
            tohan_COM_IOM_arg_6_def.guardian_errcode);
        strncpy(g_error_add, DEF_NERR_FILE_IO_ERR, sizeof(g_error_add));
        return MDSI_RET_NG;
    }

    /* 東阪振分比率設定ファイル展開処理 */
    tohan_tbl_local = (db_gfnsw_def *)tohan_COM_IOM_arg_6_def.rec_area;

    /* レコード項目チェック(振分比率(東京)) */
    memset( ch_tokyo_ratio_char, 0, sizeof(ch_tokyo_ratio_char));
    memcpy( ch_tokyo_ratio_char, tohan_tbl_local->furiwake_rate_tky, 3 );
    i_tokyo_ratio = atoi(ch_tokyo_ratio_char);
    if ( ( 0 <= i_tokyo_ratio) &&
         ( 100 >= i_tokyo_ratio) ) {
         /* 振分比率(東京)正常 */
         g_tohan_rate.tokyo_rate = (short)i_tokyo_ratio;
    }
    else{
        /* 振分比率(東京)異常 */
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_STARTREAD,
            "TOHAN RATE NG",
            0);

        return MDSI_RET_NG;
    }

    /* レコード項目チェック(振分比率(大阪)) */
    memset( ch_osaka_ratio_char, 0, sizeof(ch_osaka_ratio_char));
    memcpy( ch_osaka_ratio_char, tohan_tbl_local->furiwake_rate_osk, 3 );
    i_osaka_ratio = atoi(ch_osaka_ratio_char);
    if ( ( 0 <= i_osaka_ratio) &&
         ( 100 >= i_osaka_ratio) ) {
         /* 振分比率(大阪)正常 */
         g_tohan_rate.osaka_rate = (short)i_osaka_ratio;
    }
    else{
        /* 振分比率(大阪)異常 */
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_STARTREAD,
            "TOHAN RATE NG",
            0);

        return MDSI_RET_NG;
    }
    /* 振分比率の合計値確認(1以上100以下) */
    i_ratio_total = i_tokyo_ratio + i_osaka_ratio;
    if ( ( 0 <= i_ratio_total) &&
         ( 100 >= i_ratio_total) ) {
         /* 振分比率(大阪)正常 */
    }
    else{
        /* 振分比率の合計値異常 */
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_STARTREAD,
            "TOHAN RATE NG",
            0);
        return MDSI_RET_NG;
    }

    i_kouyaku_a = i_tokyo_ratio;
    i_kouyaku_b = i_osaka_ratio;

    if ( 0 == i_tokyo_ratio ){
        tohan_list_local.sort_list[0] = DEF_SITE_ID_OSK;
    }
    else if ( 0 == i_osaka_ratio ){
        tohan_list_local.sort_list[0] = DEF_SITE_ID_TKY;
    }
    else {
        while(i_kouyaku_b != 0){
            i_rate      = i_kouyaku_a % i_kouyaku_b;
            i_kouyaku_a = i_kouyaku_b;
            i_kouyaku_b = i_rate;
        }
        i_kouyaku       = i_kouyaku_a;
        i_kouyaku_tokyo = i_tokyo_ratio / i_kouyaku;
        i_kouyaku_osaka = i_osaka_ratio / i_kouyaku;

        i_list_total = i_kouyaku_tokyo + i_kouyaku_osaka;
        
        /* 東阪振分順リスト作成 */
        if ( i_kouyaku_tokyo >= i_kouyaku_osaka ){
            i_kirisute_rate = i_kouyaku_tokyo / i_kouyaku_osaka;
            i_kiriage_kukan = i_kouyaku_tokyo % i_kouyaku_osaka;
            if ( 0 != i_kiriage_kukan ){
                i_kiriage_rate = i_kirisute_rate+1;
            }
            else {
                i_kiriage_rate = 0;
            }
            i_kirisute_kukan = i_kouyaku_osaka - i_kiriage_kukan;
        }
        else {
            i_kirisute_rate = i_kouyaku_osaka / i_kouyaku_tokyo;
            i_kiriage_kukan = i_kouyaku_osaka % i_kouyaku_tokyo;
            if ( 0 != i_kiriage_kukan ){
                i_kiriage_rate = i_kirisute_rate+1;
            }
            else {
                i_kiriage_rate = 0;
            }
            i_kirisute_kukan = i_kouyaku_tokyo - i_kiriage_kukan;
        }

        while( sort_list_cnt < i_list_total){
            if ( i_kirisute_kukan_cnt < i_kirisute_kukan ){
                for ( i_kirisute_rate_cnt = 0;
                      i_kirisute_rate_cnt < i_kirisute_rate;
                      i_kirisute_rate_cnt++ ){
                    if ( i_kouyaku_tokyo > i_kouyaku_osaka ) {
                        tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_TKY;
                    }
                    else {
                        tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_OSK;
                    }
                    sort_list_cnt++;
                    if (sort_list_cnt >= (i_list_total-1)){
                        break;
                    }
                }
                if ( i_kouyaku_tokyo > i_kouyaku_osaka ) {
                    tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_OSK;
                }
                else {
                    tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_TKY;
                }
                sort_list_cnt++;
                if (sort_list_cnt >= (i_list_total-1)){
                    break;
                }
                i_kirisute_kukan_cnt++;
            }
            else {
                if ( i_kiriage_kukan_cnt >= i_kiriage_kukan ){
                    break;
                }
            }

            if ( i_kiriage_kukan_cnt < i_kiriage_kukan ){
                for ( i_kiriage_rate_cnt=0;
                      i_kiriage_rate_cnt < i_kiriage_rate;
                      i_kiriage_rate_cnt++ ){
                    if ( i_kouyaku_tokyo > i_kouyaku_osaka ) {
                        tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_TKY;
                    }
                    else {
                        tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_OSK;
                    }
                    sort_list_cnt++;
                    if (sort_list_cnt >= (i_list_total-1)){
                        break;
                    }
                }
                if ( i_kouyaku_tokyo > i_kouyaku_osaka ) {
                    tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_OSK;
                }
                else {
                    tohan_list_local.sort_list[sort_list_cnt] = DEF_SITE_ID_TKY;
                }
                sort_list_cnt++;
                if (sort_list_cnt >= (i_list_total-1)){
                    break;
                }
                i_kiriage_kukan_cnt++;
            }
            else {
                if ( i_kirisute_kukan_cnt >= i_kirisute_kukan ){
                    break;
                }
            }
        }
    }

    /* 東阪振分順リストの正規領域を更新 */
    memcpy( &g_tohan_list, &tohan_list_local, sizeof(g_tohan_list) );

    /* 東阪振分順リスト作成完了確認 */
    if ( 0 == tohan_list_local.sort_list[0] ) {
        /* 作成失敗 */
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_FURI_RATE,
            DEF_COM_IOM_FUNC_NEXTREAD,
            "TOHAN LIST MAKE ERR",
            0);
        return MDSI_RET_NG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_detourlist_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_main                                      */
/*  CALLING SEQ.    : void MDSI_main (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void MDSI_main(void)
{
    short s_ret = 0;

    /* 電文受信処理 */
    s_ret = MDSI_recv_read();
    if ( MDSI_RET_OK != s_ret ){
        g_myinfo_def.end_flag = MDSI_ON;
    }

} /* end of MDSI_main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_recv_read                                 */
/*  CALLING SEQ.    : short MDSI_recv_read ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : $RECEIVE処理                                          */
/****************************************************************************/
short MDSI_recv_read(void)
{
    short s_ret = 0;
    short s_err = 0;
    unsigned short s_read_cnt = 0;
    zsys_ddl_smsg_open_def *sys_msg_p;
    int i_CC = 0;

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
        s_ret = MDSI_req_recv((common_header_def *)g_recv_buf, (short)s_read_cnt);

        break;

    case ZFIL_ERR_SYSMESS :           /*SYSTEM MESSAGE receive*/
        sys_msg_p = (zsys_ddl_smsg_open_def *)g_recv_buf;
        switch ( sys_msg_p->u_z_msgnumber.z_msgnumber ) {
            case ZSYS_VAL_SMSG_OPEN :
                /* システムメッセージ受信処理 */
                MDSI_sys_open();
                break;
            case ZSYS_VAL_SMSG_CLOSE:
                /* システムメッセージ受信処理 */
                MDSI_sys_close();
                break;
            default :
                i_CC = REPLYX();
                if (_status_ne(i_CC)) {
                    FILE_GETINFO_(g_myinfo_def.recv_fno, &s_err);
                    MDSI_end();
                }
                break;
        }
        break;
    default :
        MDSI_message_output(DEF_EVT_REQ_ERR, DEF_NERR_IPC_SEISA_ERR, "@X@i",
            "ｱﾝｻﾎﾟｰﾄIPC", g_recv_buf);
        return MDSI_RET_NG;
    }
    return s_ret;

} /* end of MDSI_recv_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_req_recv                                  */
/*  CALLING SEQ.    : short MDSI_req_recv ( common_header_def *rcv_ipc,     */
/*                                           short rcv_data_len )           */
/*  ARGUMENT        : 1. rcv_ipc       (I) 受信IPC                          */
/*                  : 2. rcv_data_len  (I) 受信電文長                       */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信処理                                          */
/****************************************************************************/
short MDSI_req_recv (common_header_def *rcv_ipc, short rcv_data_len)
{
    short s_ret = 0;
    short reply_code = 0;
    char  ch_module_err_code[7+1];
    char  set_ems[80];
    c502_def *c502_rcv;

    memset(ch_module_err_code, '0', sizeof(ch_module_err_code));

    /* IPC内容精査 */
    s_ret = MDSI_ipc_chk(rcv_ipc, rcv_data_len);
    if ( MDSI_IPC_ERR_DATA_LEN == s_ret ){
        /* IPCチェック電文長異常 */
        //受信電文データ長チェックエラー(内部エラー)
        strncpy( g_internal_error_code,
                 DEF_NERR_IPC_SEISA_ERR,
                 sizeof(g_internal_error_code));

        /* EMS出力 */
        MDSI_message_output(DEF_EVT_REQ_ERR, DEF_NERR_IPC_SEISA_ERR, "@X@i",
            "ﾚﾝｸﾞｽ ｴﾗｰ", rcv_ipc);

    }
    else if ( MDSI_IPC_ERR_MSG == s_ret ){
        /* IPCチェックインタフェースコード異常 */
        /* 受信電文ヘッダ精査エラー(内部エラー) */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_HEADR_SEISA_ERR,DEF_NERR_IPC_SEISA_ERR ,
                           "@L@C@X@X",
                            "","","IPC KIND","interface_code");
        memset(set_ems, 0x00, sizeof(set_ems));
        memcpy(set_ems, rcv_ipc->interface_code, (sizeof(set_ems))-1);
    }
    else {
        if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_DEN_RCV_NT_REQ,
                        sizeof(rcv_ipc->interface_code))) {
            /* 電文受信通知要求 */
            s_ret = MDSI_con_req_recv( (c201_def *)rcv_ipc );
            /* 正常応答 */
            reply_code = MDSI_RET_OK;
        }
        else if ( 0 == memcmp( rcv_ipc->interface_code, DEF_IPC_IFCD_CMD_PRC_REQ,
                       sizeof(rcv_ipc->interface_code))) {
            c502_rcv = (c502_def*)rcv_ipc;
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_CMD_RCV,DEF_NERR_NOMAL,
                               "@L@C@X@X",
                                g_lcn,
                                &c502_rcv->command_info.connection_logical_name,
                                c502_rcv->common_header.interface_code,
                                c502_rcv->command_info.command_name);

            if (memcmp(c502_rcv->command_info.command_name,
                       DEF_IPC_CMD_FL_RE_READ,
                       sizeof(c502_rcv->command_info.command_name)) == 0){

                /* 接続構成変更 */
                s_ret = MDSI_file_up();
                if ( MDSI_RET_OK == s_ret ){
                    /* 正常応答 */
                    reply_code = MDSI_RET_OK;
                }
                else {
                    /* 異常応答 */
                    reply_code = MDSI_IPC_ERR;
                    memcpy(ch_module_err_code, g_internal_error_code, sizeof(g_internal_error_code));
                }
            }
            else {
                /* 異常応答 */
                reply_code = MDSI_IPC_ERR;
                memcpy(ch_module_err_code, DEF_NERR_IPC_SEISA_ERR, sizeof(g_internal_error_code));
            }
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_CMD,ch_module_err_code ,
                               "@L@C@X@2",
                                g_lcn,
                                &c502_rcv->command_info.connection_logical_name,
                                c502_rcv->common_header.interface_code,
                                reply_code);
        }
    }

    /* 電文受信しているので必ず正常応答を返却する */
    /* リプライ処理 */
    s_ret =  MDSI_recv_reply ((char *)rcv_ipc,
                              reply_code,
                              ch_module_err_code );

    /* リプライ処理異常時、次電文受信処理を行う */
    return MDSI_RET_OK;

} /* end of MDSI_req_recv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_ipc_chk                                   */
/*  CALLING SEQ.    : short MDSI_ipc_chk ( common_header_def *rcv_ipc,      */
/*                                         short rcv_data_len )             */
/*  ARGUMENT        : 1. rcv_ipc       (I) 受信IPC                          */
/*                  : 2. rcv_data_len  (I) 受信電文長                       */
/*  RETURN CODE     : 0:正常終了 1：インタフェースコード異常 2：データ長異常*/
/*  DESCRIPTION     : IPC内容精査処理                                       */
/****************************************************************************/
short MDSI_ipc_chk (common_header_def *rcv_ipc, short rcv_data_len)
{
    c502_def *c502_def_adr;

    /* データ長確認 */
    if ( rcv_data_len != (rcv_ipc->control_data_length + sizeof(common_header_def)) ){
        /* 電文長異常 */
        return MDSI_IPC_ERR_DATA_LEN;
    }

    if ( 0 == memcmp( &rcv_ipc->interface_code, DEF_IPC_IFCD_DEN_RCV_NT_REQ,
                    sizeof(rcv_ipc->interface_code))) {
        /* 電文受信通知要求 */
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
            return MDSI_IPC_ERR_MSG;
        }
    }
    else {
        /* インタフェースコード異常 */
        return MDSI_IPC_ERR_MSG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_ipc_chk */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_recv_reply                                */
/*  CALLING SEQ.    : short MDSI_recv_reply ( char *rcv_buff,               */
/*                                            short reply_err_code,         */
/*                                            char *module_err_code )       */
/*  ARGUMENT        : 1. rcv_buff         (I) 受信バッファ                  */
/*                  : 2. reply_err_code   (I) エラーコード                  */
/*                  : 3. module_err_code  (I) 内部エラーコード              */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信応答処理                                      */
/****************************************************************************/
short MDSI_recv_reply (char *rcv_buff,
                       short reply_err_code,
                       char *module_err_code )

{
    _cc_status i_ret = 0;
    c201_def *reply_ipc = (c201_def *)rcv_buff;
    unsigned short s_count_written = 0;
    short s_error_return = 0;
    short s_msg_size = 0;

    /* リプライIPC作成 */
    if ( 0 == memcmp( DEF_IPC_IFCD_DEN_RCV_NT_REQ, 
                      reply_ipc->common_header.interface_code,
                      sizeof(reply_ipc->common_header.interface_code))) {
        /* 電文受信通知要求 */
        memcpy( reply_ipc->common_header.interface_code,
                DEF_IPC_IFCD_DEN_RCV_NT_RSP,
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
                           g_recv_info.z_messagetag,
                           s_error_return );

    if (MDSI_RET_OK != i_ret) {
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "REPLY ERR",
                            rcv_buff);
    }

   return MDSI_RET_OK;

} /* end of MDSI_recv_reply */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_con_req_recv                              */
/*  CALLING SEQ.    : short MDSI_con_req_recv (c201_def *rcv_ipc)           */
/*  ARGUMENT        : 1. rcv_ipc          (I) 受信IPC                       */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信通知要求処理                                  */
/****************************************************************************/
short MDSI_con_req_recv (c201_def *rcv_ipc)
{
    short    s_ret          = 0;
    char     ch_get_lcn[16] = {0};
    char     ch_log_file_name[MDSI_LOG_FILE_NAME_SIZE];
    char     ch_tohan;
    char     ch_get_detour_tbl_no;
    char     ch_get_tohan_kind;
    c301_def set_send_data;
    c201_def rcv_ipc_save;
    short    loop_cnt          = 0;
    short    s_data_start_posi = 0;
    char     c_mti_data[4];
    char     c_mti_data_save[5];
    short    s_mti_cnt         = 0;
    char     ch_tohan_flag     = MDSI_OFF;
    char     ch_mti_codec_before[5];
    char     ch_mti_codec_after[5];
    char     hb_resp_send[MDSI_MAX_DATA_SIZE];
    short    s_hb_resp_leng    = 0;
    char     set_ems[80];
    char     ch_err_code[7];
    char     data_kind_set_flg = MDSI_OFF;

    memset( &set_send_data,   0,          sizeof(set_send_data) );
    memset( &rcv_ipc_save,    0,          sizeof(rcv_ipc_save) );
    memcpy( &rcv_ipc_save,    rcv_ipc,    sizeof(rcv_ipc_save) );
    memset( c_mti_data,       0,          sizeof(c_mti_data));
    memset( g_mti_data,       0,          sizeof(g_mti_data));
    memset( g_mti_save,       MDSI_SPACE, sizeof(g_mti_save));
    memset( ch_err_code,      MDSI_SPACE, sizeof(ch_err_code));
    memset( g_rcv_if_id,      MDSI_SPACE, sizeof(g_rcv_if_id) );
    memset( g_rcv_station_id, MDSI_SPACE, sizeof(g_rcv_station_id) );
    memset( &g_rcv_gflin,     MDSI_SPACE, sizeof(g_rcv_gflin) );

    /* 振分先東阪種別初期化 */
    g_tohan_kind = ' ';
    memset( g_tohan_proc_kind, ' ', sizeof(g_tohan_proc_kind));
    g_tohan_list.list_no_old = MDSI_NOT_SET;

    /* データ長保存 */
    g_myinfo_def.data_len = rcv_ipc->msg_info.msg_len;
    /* データ長削除済みフラグOFF */
    g_myinfo_def.data_len_del_flag = MDSI_OFF;

    /* 不正MTI受信フラグ OFF */
    g_mti_ng_flg = MDSI_OFF;

    /* インタフェースID保存 */
    memcpy( g_rcv_if_id, rcv_ipc_save.text_recv_notify.recv_con_id.interface_name,
            sizeof(g_rcv_if_id) );

    /* ステーション識別保存 */
    memcpy( g_rcv_station_id, rcv_ipc_save.text_recv_notify.recv_con_id.station_name,
            sizeof(g_rcv_station_id) );

    /* センターID保存 */
    memcpy( &g_rcv_gflin, &rcv_ipc_save.text_recv_notify.recv_con_id,
            sizeof(g_rcv_gflin) );

    g_mti_kind = 0;
    g_mti_judge = MDSI_MTI_JUDGE_ERR;

    g_denbun_lct_info_no = -1;

    /* 電文項目位置情報検索 */
    for ( loop_cnt=0; loop_cnt<g_denbun_lct_info_cnt; loop_cnt++ ){
        if (( 0 == memcmp( g_rcv_if_id,
                          g_denbun_lct_info[loop_cnt].interface_id,
                          sizeof(g_rcv_if_id) )) &&
            (g_denbun_lct_info[loop_cnt].data_len_start_lct != 0 )) {
            /* 電文項目位置情報配列番号設定 */
            g_denbun_lct_info_no = loop_cnt;
            break;
        }
    }

    if ( -1 == g_denbun_lct_info_no ) {
        /* 電文項目位置情報配列番号なし */
        /* 送信不可エラー終了 */
        /* 中継不能エラー(内部エラー) */
        memcpy( g_internal_error_code,
                DEF_NERR_DST_SELECT_ERR,
                sizeof(g_internal_error_code));
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_CHUKEI_ERR,
                            DEF_NERR_DST_SELECT_ERR,
                            "@L@C@X",
                            g_lcn,
                            &g_rcv_gflin,
                            "CENTER ID ERR");
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* GFP内部LCN取得 */
    s_ret = MDSI_lcn_get(ch_get_lcn);
    if ( MDSI_RET_OK != s_ret ){
        /* エラーログ出力 */
        s_ret = MDSI_err_log_set( rcv_ipc,
                                    DEF_ELG_INTERNAL,
                                    g_internal_error_code);
        /* 正常終了 */
        return MDSI_RET_OK;
    }
    memcpy( g_lcn, ch_get_lcn, sizeof(g_lcn) );

    /* 電文復号処理 */
    s_ret = MDSI_decode( &rcv_ipc_save );
    if ( MDSI_RET_OK != s_ret ){
        /* 電文ログ出力 */
        memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
        s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

        /* 障害電文通知作成依頼送信 */
        s_ret = MDSI_err_data_make ( rcv_ipc);

        /* エラーログ後正常応答 */
        return MDSI_RET_OK;
    }

    /* MTI取得対象メッセージ判定 */
    memset( hb_resp_send, 0x00, sizeof(hb_resp_send));
    g_mti_kind = NWM_MTI((char *)g_encdec_con.dec_conf_data.decode_data,
                          g_encdec_con.dec_conf_data.decode_data_length,
                          g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct,
                          &s_data_start_posi,
                          hb_resp_send,
                          &s_hb_resp_leng);

    /* 戻り値判定 */
    if ( g_mti_kind == MDSI_RET_NG ){
        g_mti_judge = MDSI_MTI_JUDGE_ERR;
        /* EMS出力 */
        memset(set_ems, 0x00, sizeof(set_ems));
        memcpy(set_ems, g_encdec_con.dec_conf_data.decode_data, (sizeof(set_ems))-1);
        MDSI_message_output(DEF_EVT_DENBUN_HAKI,DEF_NERR_RCV_DENBUN_HEADR_ERR,
                            "@L@X@D",
                            g_lcn,"DATA ERR",set_ems);

        /* 電文ログ出力 */
        memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
        s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

        /* MTI取得対象メッセージ判定エラー(内部エラー) */
        strncpy( g_internal_error_code,
                 DEF_NERR_RCV_DENBUN_HEADR_ERR,
                 sizeof(g_internal_error_code));

        /* 不正MTI受信フラグ ON */
        g_mti_ng_flg = MDSI_ON;

        /* 障害電文通知作成依頼送信 */
        s_ret = MDSI_err_data_make ( rcv_ipc);

        /* 障害電文通知作成依頼後正常応答 */
        return MDSI_RET_OK;
    }
    else if ( g_mti_kind == MDSI_MTI_JUDGE_IDL ){
        /* 電文ログ出力 */
        memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
        s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

        /* アイドルは破棄して終了 */
        return MDSI_RET_OK;
    }
    else if ( g_mti_kind == MDSI_MTI_JUDGE_QUE ){
        memcpy( c_mti_data, "YYY1", 4 );
    }
    else if ( g_mti_kind == MDSI_MTI_JUDGE_HB ){
        if ( s_hb_resp_leng != 0 ){
            /* ハートビート応答設定 */
            memcpy( g_encdec_con.dec_conf_data.decode_data, hb_resp_send, s_hb_resp_leng );
            g_encdec_con.dec_conf_data.decode_data_length = s_hb_resp_leng;
        }
        memcpy( c_mti_data, "YYY2", 4 );
    }

    /* MTI保存 */
    if ( c_mti_data[0] != 'Y' && c_mti_data[0] != 'Z' ){
        /* MTI取得 */
        s_mti_cnt = g_denbun_lct_info[g_denbun_lct_info_no].mti_start_lct - 1;

        if ( g_encdec_con.dec_conf_data.decode_data_length < (s_mti_cnt + 4) ){
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_MTI_HANTE_ERR, DEF_NERR_MTI_GET_ERR,
                                   "@L@C@X",
                                   g_lcn,
                                   &g_rcv_gflin,
                                   "    ");

            /* MTI取得対象メッセージ判定エラー(内部エラー) */
            strncpy( g_internal_error_code,
                     DEF_NERR_MTI_GET_ERR,
                     sizeof(g_internal_error_code));
            /* 電文ログ出力 */
            memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
            s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

            /* 不正MTI受信フラグ ON */
            g_mti_ng_flg = MDSI_ON;

            /* 障害電文通知作成依頼送信 */
            s_ret = MDSI_err_data_make ( rcv_ipc);

            /* 異常終了 */
            return MDSI_RET_OK;
        }

        if ( memcmp( g_denbun_lct_info[g_denbun_lct_info_no].mti_item_attribute,
                     DEF_MTI_ITEM_ATTR_BCD,
                     sizeof(g_denbun_lct_info[g_denbun_lct_info_no].mti_item_attribute)) == 0){
            memcpy( c_mti_data, &g_encdec_con.dec_conf_data.decode_data[s_mti_cnt], 2 );
            /* BCD文字変換 */
            memset(ch_mti_codec_before, 0, sizeof(ch_mti_codec_before));
            memset(ch_mti_codec_after, 0, sizeof(ch_mti_codec_after));

            memcpy(ch_mti_codec_before, c_mti_data, 2);
            /* 共通関数起動 */
            MDSI_BCD2CHAR(ch_mti_codec_before, ch_mti_codec_after,(short)2);
            if ( ch_mti_codec_after[0] == 0 ){
                /* 文字変換失敗 */
                /* EMS出力 */
                MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                                    DEF_NERR_MTI_GET_ERR,
                                    "@X@E", "BCD2CHAR", 0);
                /* MTI取得対象メッセージ判定エラー(内部エラー) */
                strncpy( g_internal_error_code,
                         DEF_NERR_MTI_GET_ERR,
                         sizeof(g_internal_error_code));
                /* 電文ログ出力 */
                memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
                s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

                /* 不正MTI受信フラグ ON */
                g_mti_ng_flg = MDSI_ON;

                /* 障害電文通知作成依頼送信 */
                s_ret = MDSI_err_data_make ( rcv_ipc);

                /* 異常終了 */
                return MDSI_RET_OK;
            }
            memcpy( c_mti_data, ch_mti_codec_after, 4);
        } else if( memcmp( g_denbun_lct_info[g_denbun_lct_info_no].mti_item_attribute,
                           DEF_MTI_ITEM_ATTR_EBC,
                           sizeof(g_denbun_lct_info[g_denbun_lct_info_no].mti_item_attribute)) == 0){
            memcpy( c_mti_data, &g_encdec_con.dec_conf_data.decode_data[s_mti_cnt], 4 );
            /* EBCDIC文字変換 */
            memset(ch_mti_codec_before, 0, sizeof(ch_mti_codec_before));
            memset(ch_mti_codec_after, 0, sizeof(ch_mti_codec_after));

            memcpy(ch_mti_codec_before, c_mti_data, 4);
            /* 共通関数起動 */
            EBCDIC2SJIS(ch_mti_codec_before, ch_mti_codec_after, (short)4);
            if ( ch_mti_codec_after[0] == 0 ){
                /* 文字変換失敗 */
                /* EMS出力 */
                MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                                    DEF_NERR_MTI_GET_ERR,
                                    "@X@E", "EBCDIC2SJIS", 0);
                /* MTI取得対象メッセージ判定エラー(内部エラー) */
                strncpy( g_internal_error_code,
                         DEF_NERR_MTI_GET_ERR,
                         sizeof(g_internal_error_code));
                /* 電文ログ出力 */
                memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
                s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

                /* 不正MTI受信フラグ ON */
                g_mti_ng_flg = MDSI_ON;

                /* 障害電文通知作成依頼送信 */
                s_ret = MDSI_err_data_make ( rcv_ipc);

                /* 異常終了 */
                return MDSI_RET_OK;
            }
            memcpy( c_mti_data, ch_mti_codec_after, 4);
        } else {
            /* MTIコード変換不要 */
            memcpy( c_mti_data, &g_encdec_con.dec_conf_data.decode_data[s_mti_cnt], 4 );
        }

        /* 受信MTIが受信電分振分先設定テーブル(ZZZZ,YYY1,YYY2以外)に無い場合はエラーとする */
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if (( g_detour_tbl[loop_cnt].t_primary_key.mti[0] == MDSI_ZERO) ||
                ( g_detour_tbl[loop_cnt].t_primary_key.mti[0] == 'Z')       ||
                ( g_detour_tbl[loop_cnt].t_primary_key.mti[0] == 'Y'))      {
                break;
            }
            if ( memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                         c_mti_data,
                         sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti)) == 0){
                /* 対象MTIテーブルあり */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */

        /* 受信電分振分先設定テーブル検索結果確認 */
        if (data_kind_set_flg != MDSI_ON){
            /* 受信電分振分先設定テーブルに受信MTIなし */
            /* EMS出力 */
            memset(c_mti_data_save, 0, sizeof(c_mti_data_save));
            memcpy(c_mti_data_save, c_mti_data, 4);
            MDSI_message_output(DEF_EVT_MTI_HANTE_ERR, DEF_NERR_MTI_GET_ERR,
                                   "@L@C@X",
                                   g_lcn,
                                   &g_rcv_gflin,
                                   c_mti_data_save);

            /* MTI取得対象メッセージ判定エラー(内部エラー) */
            strncpy( g_internal_error_code,
                     DEF_NERR_MTI_GET_ERR,
                     sizeof(g_internal_error_code));
            /* 電文ログ出力 */
            memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
            s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

            /* 不正MTI受信フラグ ON */
            g_mti_ng_flg = MDSI_ON;
            memcpy(g_mti_save, c_mti_data, sizeof(g_mti_save));

            /* 障害電文通知作成依頼送信 */
            s_ret = MDSI_err_data_make (rcv_ipc);

            /* 異常終了 */
            return MDSI_RET_OK;
        }

        memcpy( g_mti_data, c_mti_data, sizeof(g_mti_data));
        memcpy( g_mti_save, c_mti_data, sizeof(g_mti_save));
    }
    /* 電文開始位置を保存 */
    g_date_start_posi = s_data_start_posi;

    /* 振分先判定処理 */
    s_ret = MDSI_swich_judge (g_mti_kind,
                              c_mti_data,
                              rcv_ipc,
                              &ch_get_detour_tbl_no,
                              &ch_get_tohan_kind );
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* 障害電文通知依頼を行うため正常終了とする */
        return MDSI_RET_OK;
    }
    else {
        /* 振分先テーブル配列番号をグローバル領域に設定する */
        g_detour_data_no = ch_get_detour_tbl_no;
        /* 振分先東阪種別をグローバル領域に設定する */
        g_tohan_kind = ch_get_tohan_kind;
    }

    /* 電文ログ出力 */
    memset( ch_log_file_name, MDSI_SPACE, sizeof(ch_log_file_name));
    s_ret = MDSI_log_output( (char *)rcv_ipc, ch_get_lcn, &ch_tohan, ch_log_file_name );

    /* ログファイル名をグローバルデータに保存する */
    memcpy( g_log_file_name, ch_log_file_name, sizeof(g_log_file_name) );
    /* 二重受信判定 */
    if ( MDSI_RET_DOUBLE_SET == s_ret ) {
        /* 二重受信時の処理 */
        /* GFP内部LCNをログ出力結果から取得 */
        memcpy( g_lcn, ch_get_lcn, sizeof(g_lcn) );
        /* 前回東阪送信情報をログ出力結果から取得 */
        g_tohan_kind = ch_tohan;
        /* 東阪振分先検索済みの場合、検索前に戻す */
        if (g_tohan_list.list_no_old != MDSI_NOT_SET){
            g_tohan_list.list_no = g_tohan_list.list_no_old;
        }
    }
    else if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* エラーログ出力 処理継続 */
        s_ret = MDSI_err_log_set( rcv_ipc,
                                    DEF_ELG_INTERNAL,
                                    DEF_NERR_FILE_IO_ERR);
    }

    /* キュー登録要求作成処理 */
    s_ret = MDSI_data_make( MDSI_C301_KIND_NOMAL,
                            rcv_ipc,
                            &g_detour_tbl[g_detour_data_no],
                            g_tohan_kind,
                            ch_err_code,
                            &set_send_data);
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* キュー登録要求送信処理 */
    s_ret = MDSI_data_send(&g_detour_tbl[g_detour_data_no],
                           &set_send_data,
                           rcv_ipc);
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* エラーログ出力 */
        s_ret = MDSI_err_log_set( rcv_ipc,
                                    DEF_ELG_INTERNAL,
                                    DEF_NERR_SEND_ERR);
        return MDSI_RET_NG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_con_req_recv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_lcn_get                                   */
/*  CALLING SEQ.    : short MDSI_lcn_get (char *get_lcn)                    */
/*  ARGUMENT        : 1. get_lcn          (O) GFP内部LCN                    */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : GFP内部LCN取得処理                                    */
/****************************************************************************/
short MDSI_lcn_get (char *get_lcn)
{
    short s_ret = 0;
    c701_def st_lcn_ipc;          /* GFP内部LCN採番要求IPC設定テーブル */
    r701_def *r701_def_ipc;       /* GFP内部LCN採番応答IPC設定テーブル */

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* GFP内部LCN採番要求IPC設定テーブル初期化 */
    memset( &st_lcn_ipc, 0, sizeof(st_lcn_ipc) );

    /* IPC interface_code設定 */
    memcpy( st_lcn_ipc.common_header.interface_code, DEF_IPC_IFCD_LCN_NUM_REQ,
             sizeof(st_lcn_ipc.common_header.interface_code) );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( st_lcn_ipc.common_header.internal_error_code, MDSI_SPACE,
            sizeof(st_lcn_ipc.common_header.internal_error_code));
    memset( st_lcn_ipc.common_header.filler_1, MDSI_SPACE,
            sizeof(st_lcn_ipc.common_header.filler_1));

    /* IPC 呼び出し元プロセス名設定 */
    memcpy( st_lcn_ipc.process_name,
            g_myinfo_def.proc_data.my_pname,
            g_myinfo_def.proc_data.my_pname_len );

    /* IPC 採番システム(サイトコード)設定 */
    st_lcn_ipc.site_code = g_myinfo_def.config_data.site_id;

    /* IPC 場所(NW識別)設定 */
    st_lcn_ipc.network_code = g_myinfo_def.config_data.network_id;

    /* IPC データ長設定(26byte) */
    st_lcn_ipc.common_header.control_data_length =
           sizeof(st_lcn_ipc) - sizeof( st_lcn_ipc.common_header);

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, MDSI_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           MDSI_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));

    if ( g_lcncon_data.domain_name[0] != MDSI_SPACE){
        /* ドメイン名設定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_lcncon_data.domain_name,
                                 sizeof(g_lcncon_data.domain_name));
    }
    else {
        /* PATHMON名設定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_lcncon_data.pathmon_name,
                                 sizeof(g_lcncon_data.pathmon_name));
    }

    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_lcncon_data.server_class,
                       sizeof(g_lcncon_data.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &st_lcn_ipc, sizeof(st_lcn_ipc) );

    t_COM_PSD_arg_1_def.req_send_len    = sizeof(st_lcn_ipc);
    t_COM_PSD_arg_1_def.receive_max_len = MDSI_MAX_DATA_SIZE;
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_lcncon_data.pathsend_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = g_lcncon_data.retry_cnt;

    memcpy(t_COM_PSD_arg_2_def.prog_id,
           g_myinfo_def.proc_data_sub.module_id,
           sizeof(t_COM_PSD_arg_2_def.prog_id) );

    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_lcncon_data.server_class,
                       sizeof(g_lcncon_data.server_class));

    /* EMS出力情報設定 */

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
    if ( MDSI_RET_OK != s_ret ) {
        /* 異常終了 */
        /* GFP内部LCN取得エラー(EMS) */
        MDSI_message_output(DEF_EVT_LCN_GET_ERR, DEF_NERR_LCN_GET_ERR, "@L@C@E",
                            g_lcn, &g_rcv_gflin, s_ret);
        /* GFP内部LCN取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
                  DEF_NERR_LCN_GET_ERR,
                  sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }

    r701_def_ipc = (r701_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R701) */
    if ( memcmp(r701_def_ipc->common_header.interface_code,
                DEF_IPC_IFCD_LCN_NUM_RSP,
                sizeof(r701_def_ipc->common_header.interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (r701_def_ipc->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常 */
            s_ret = MDSI_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        s_ret = MDSI_RET_NG;
    }
    if ( s_ret == MDSI_RET_NG ) {
        /* EMS出力 応答エラー */
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_LCN_GET_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R701 ERROR          ",
                            r701_def_ipc);
        /* GFP内部LCN取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
                  DEF_NERR_LCN_GET_ERR,
                  sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }

    /* GFP内部LCNを受信バッファから取得 */
    memcpy( get_lcn, (char *)&r701_def_ipc->gfplcn, sizeof(r701_def_ipc->gfplcn) );

    return MDSI_RET_OK;

} /* end of MDSI_lcn_get */

/******************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_decode                                      */
/*  CALLING SEQ.    : short MDSI_decode ( c201_def *ch_rcv_data_adr)          */
/*  ARGUMENT        : 1. ch_rcv_data_adr  (I) 受信電文                        */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                  */
/*  DESCRIPTION     : 復号処理                                                */
/******************************************************************************/
short MDSI_decode ( c201_def *ch_rcv_data_adr)
{
    short s_ret;
    char  rcv_data[MAX_TEXT_BUF_LEN];
 
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
    memset( ch_module_id,         0, sizeof(ch_module_id) );
    memcpy( ch_module_id, g_myinfo_def.proc_data_sub.module_id, sizeof(ch_module_id) );

    /* グローバル領域初期化 */
    memset( &g_encdec_con.dec_conf_data, 0, sizeof(g_encdec_con.dec_conf_data) );

    /* 受信電文初期化 */
    memset( rcv_data, 0, sizeof(rcv_data));

    /* 復号処理 */
    memcpy(t_NWM_ENC_arg_2_def.file_id,
           g_encdec_con.dec_start_data.key_file_id,
           sizeof(t_NWM_ENC_arg_2_def.file_id));
    memcpy(t_NWM_ENC_arg_2_def.file_name,
           g_encdec_con.dec_start_data.key_file_name,
           sizeof(t_NWM_ENC_arg_2_def.file_name));
    t_NWM_ENC_arg_2_def.file_no = g_encdec_con.dec_start_data.key_file_no;
    t_NWM_ENC_arg_2_def.io_timer = g_myinfo_def.config_data.io_timer;

    memcpy( t_NWM_ENC_arg_3_def.domain_name, g_encdec_con.dec_start_data.pathmon_name,
               sizeof(t_NWM_ENC_arg_3_def.domain_name) );
    memcpy( t_NWM_ENC_arg_3_def.server_name, g_encdec_con.dec_start_data.server_class,
               sizeof(t_NWM_ENC_arg_3_def.server_name) );
    t_NWM_ENC_arg_3_def.pathsend_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_NWM_ENC_arg_3_def.pathsend_retry_cnt = (short)g_myinfo_def.config_data.send_retry_count;
    t_NWM_ENC_arg_4_def.site_id            = g_myinfo_def.config_data.site_id;
    t_NWM_ENC_arg_4_def.nw_id              = g_myinfo_def.config_data.network_id;
    memcpy( t_NWM_ENC_arg_4_def.grp_id, g_myinfo_def.config_data.group_id,
               sizeof(t_NWM_ENC_arg_4_def.grp_id) );
    memcpy( t_NWM_ENC_arg_4_def.if_id,
            ch_rcv_data_adr->text_recv_notify.recv_con_id.interface_name,
            sizeof(t_NWM_ENC_arg_4_def.if_id) );
    memcpy( t_NWM_ENC_arg_4_def.station_id,
            ch_rcv_data_adr->text_recv_notify.recv_con_id.station_name,
            sizeof(t_NWM_ENC_arg_4_def.station_id) );

    t_NWM_ENC_arg_5_def.before_msg = ch_rcv_data_adr->msg_info.msg_data;
    t_NWM_ENC_arg_5_def.before_len = ch_rcv_data_adr->msg_info.msg_len;
    t_NWM_ENC_arg_5_def.after_msg  = rcv_data;

    /* 個別モジュール */
    s_ret = NWM_ENC(MDSI_DECODE,
                    &t_NWM_ENC_arg_2_def,
                    &t_NWM_ENC_arg_3_def,
                    &t_NWM_ENC_arg_4_def,
                    &t_NWM_ENC_arg_5_def,
                    ch_module_id);
    /* 個別モジュール結果判定 */
    if ( s_ret >= 2 ) {
        switch(s_ret){
        case DEF_NWM_ENC_RTN_NG_AUTHORI:
            memcpy( g_internal_error_code,
                    DEF_NERR_KMAC_HANTE_ERR,
                    sizeof(g_internal_error_code));
            break;
        case DEF_NWM_ENC_RTN_NG_DIGITS_KMAC:
            memcpy( g_internal_error_code,
                    DEF_NERR_KMAC_HANTE_ERR,
                    sizeof(g_internal_error_code));
            break;
        case DEF_NWM_ENC_RTN_NG_DIGITS_KC:
            memcpy( g_internal_error_code,
                    DEF_NERR_KC_DEC_ERR,
                    sizeof(g_internal_error_code));
            break;
        case DEF_NWM_ENC_RTN_NG_ATALLA:
        case DEF_NWM_ENC_RTN_NG_IO:
        default:
            memcpy( g_internal_error_code,
                    DEF_NERR_DENBUN_DEC_ERR,
                    sizeof(g_internal_error_code));
            break;
        }
        /* エラー終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            g_internal_error_code,
                            "@X@E", "NWM_ENC", s_ret);

        return MDSI_RET_NG;
    }

    /* 復号結果格納処理 */
    /* 複合後データ格納 */
    memcpy( g_encdec_con.dec_conf_data.decode_data,
            t_NWM_ENC_arg_5_def.after_msg,
            t_NWM_ENC_arg_5_def.after_len );
    g_encdec_con.dec_conf_data.decode_data_length = t_NWM_ENC_arg_5_def.after_len;

    /* 正常終了 */
    return MDSI_RET_OK;

} /* end of MDSI_decode */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_log_output                                */
/*  CALLING SEQ.    : short MDSI_log_output ( c201_def *rcvdata,            */
/*                                            char *c_lcn,                  */
/*                                            char *c_site,                 */
/*                                            char *c_log_file_name )       */
/*  ARGUMENT        : 1. rcvdata          (I) ログ出力対象受信電文          */
/*                  : 2. c_lcn            (O) 前回送信GFP内部LCN            */
/*                  : 3. c_site           (O) 前回東阪送信情報              */
/*                  : 4. c_log_file_name  (O) 出力先電文ログファイル名      */
/*  RETURN CODE     : 0:正常終了 1:二重送信 -1:異常終了                     */
/*  DESCRIPTION     : 電文ログ出力処理（二重送信判定）                      */
/****************************************************************************/
short MDSI_log_output ( char *sendrcvdata,
                        char *c_lcn,
                        char *c_site,
                        char *c_log_file_name )
{
    short s_ret;

    c601_def st_log_ipc;         /* ログ出力要求IPC設定テーブル */
    r601_def *st_log_rcv_ipc;    /*ログ出力応答IPC設定テーブル */

    short    loop_cnt          = 0;
    char     station_sts       = 0;  /* 局状態設定領域 */
    char     data_kind_set_flg = 0;
    short    sh_nw_info_cnt    = 0;
    short    sh_nw_search_flg  = MDSI_OFF;
    c201_def *rcvdata          = (c201_def *)sendrcvdata;
    char ch_datalen_local[6];

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* ログ出力要求IPC設定テーブル初期化 */
    memset( &st_log_ipc, 0, sizeof(st_log_ipc) );
    memset( &g_st_log_ipc, 0, sizeof(g_st_log_ipc) );

    /* IPC interface_code設定 */
    memcpy( st_log_ipc.common_header.interface_code, DEF_IPC_IFCD_LG_OUT_REQ_DEN_REQ, 4 );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( st_log_ipc.common_header.internal_error_code, MDSI_SPACE,
            sizeof(st_log_ipc.common_header.internal_error_code));
    memset( st_log_ipc.common_header.filler_1, MDSI_SPACE,
            sizeof(st_log_ipc.common_header.filler_1));

    /* IPC ログファイル名にSPACEを設定 */
    memset( st_log_ipc.logfile_id, MDSI_SPACE,
            sizeof(st_log_ipc.logfile_id));

    /* IPC 登録/更新区分設定 */
    st_log_ipc.entry_update_cate = MDSI_LOG_SET;

    /* IPC プライマリーキー設定 */
    memcpy( &st_log_ipc.t_glnlg.pri_key.part_id[0],
            &rcvdata->text_recv_notify.recv_timestamp.time_stamp[18],
            sizeof(st_log_ipc.t_glnlg.pri_key.part_id) );

    /* IPC 送受信識別設定 (受信電文="1") */
    st_log_ipc.t_glnlg.send_recv_id = MDSI_RECEIVE_DATA;

    /* IPC 送受信電文長設定 */
    memset(ch_datalen_local, 0, sizeof(ch_datalen_local));
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%05d",
             rcvdata->msg_info.msg_len );
    memcpy( st_log_ipc.t_glnlg.send_recv_denbun_len,
            ch_datalen_local,
             sizeof(st_log_ipc.t_glnlg.send_recv_denbun_len));

    /* MTIあり電文判定 */
    if (( 0 != g_mti_data[0] )               &&
        ( 0 != memcmp(g_mti_data, "YYY", 3)) &&
        ( 0 != memcmp(g_mti_data, "ZZZZ", 4))&&
        (g_mti_kind != MDSI_MTI_JUDGE_IDL)){
        /* MTIあり電文 */
        /* IPC MTI設定 */
        memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id,
                g_mti_data,
                sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id) );
        memcpy( st_log_ipc.t_glnlg.mti_id,
                g_mti_data,
                sizeof(st_log_ipc.t_glnlg.mti_id) );
    }
    else {
        /* MTIなし電文 */
        /* IPC MTIにスペース設定 */
        memset( st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id,
                MDSI_SPACE,
                sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id) );
        memset( st_log_ipc.t_glnlg.mti_id,
                MDSI_SPACE,
                sizeof(st_log_ipc.t_glnlg.mti_id) );
    }

    /* IPC コネクション論理ID設定 */
    memcpy( &st_log_ipc.t_glnlg.connect_id,
            &rcvdata->text_recv_notify.recv_con_id,
            sizeof(st_log_ipc.t_glnlg.connect_id) );

    /* IPC 東阪振分情報 振分先 */
    st_log_ipc.t_glnlg.furiwake_info.furiwake_dst = g_tohan_kind;

    /* IPC 東阪振分情報 振分先選択処理区分 */
    memcpy( st_log_ipc.t_glnlg.furiwake_info.furiwake_kubun,
            g_tohan_proc_kind,
            sizeof(st_log_ipc.t_glnlg.furiwake_info.furiwake_kubun));

    /* IPC 電文送受信情報 電文受信時刻設定 */
    memcpy( &st_log_ipc.t_glnlg.pri_key.tushin_denbun_id,
            &rcvdata->text_recv_notify.recv_timestamp,
            sizeof(st_log_ipc.t_glnlg.pri_key.tushin_denbun_id) );
    memcpy( &st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_recv_time,
            &rcvdata->text_recv_notify.recv_timestamp,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_recv_time) );

    /* IPC 電文送受信情報 受信時局状態 */
    s_ret = MDSI_station_sts_read(rcvdata, &station_sts );
    if ( MDSI_RET_OK == s_ret ) {
        /* 局状態をIPCに設定 */
        st_log_ipc.t_glnlg.denbun_send_recv_info.recv_kyoku_sts = station_sts;
        g_station_sts = station_sts;
    }
    else {
        /* 局状態取得失敗 */
        strncpy( g_internal_error_code,
                 DEF_NERR_FILE_IO_ERR,
                 sizeof(g_internal_error_code));
        /* 電文ログ出力 */
        /* エラーログ出力 処理継続 */
        s_ret = MDSI_err_log_set( rcvdata,
                                  DEF_ELG_INTERNAL,
                                  DEF_NERR_FILE_IO_ERR);

        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* NW情報検索 */
    for ( sh_nw_info_cnt=0; sh_nw_info_cnt<g_nw_info_cnt; sh_nw_info_cnt++){
        if ( 0 == memcmp(g_nw_info[sh_nw_info_cnt].interface_id,
                         rcvdata->text_recv_notify.recv_con_id.interface_name,
                         sizeof(g_nw_info[sh_nw_info_cnt].interface_id))){
            /* NW情報あり */
            sh_nw_search_flg = MDSI_ON;
            break;
        }
    }
    if ( sh_nw_search_flg != MDSI_ON ){
        /* インタフェース識別なし */
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* IPC 電文送受信情報 NW区分設定 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.nw_kubun,
            g_nw_info[0].nw_segment,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.nw_kubun) );

    /* IPC 電文送受信情報 送信電文種別 */
    memset(g_rcv_log_msg_kind, ' ', 2);

    /* 送信電文種別未設定時スペース設定 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.send_denbun_shubetu = MDSI_SPACE;

    switch ( g_mti_kind ) {
    case MDSI_MTI_JUDGE_MTI:
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
                break;
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 st_log_ipc.t_glnlg.denbun_send_recv_info.mti_id,
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_ipc.t_glnlg.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_QUE:
        /* リジェクトを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_RJ, sizeof(g_rcv_log_msg_kind));
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 "YYY1",
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_ipc.t_glnlg.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_HB:
        /* ハートビートを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_HB, sizeof(g_rcv_log_msg_kind));
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 "YYY2",
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_ipc.t_glnlg.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_IDL:
        /* アイドルを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_IDL, sizeof(g_rcv_log_msg_kind));
        break;
     case MDSI_MTI_JUDGE_ERR:
        /* MTI異常を設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        break;
     default:
        break;
    }

    /* IPC メッセージ種別設定 */
    memcpy( st_log_ipc.t_glnlg.msg_shubetu,
            g_rcv_log_msg_kind, sizeof(st_log_ipc.t_glnlg.msg_shubetu) );

    /* IPC 電文送受信情報 電文ログKEY GFP内部LCN */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
            g_lcn,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id) );

    /* IPC 電文送受信情報 電文ログKEY 電文形態 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai = MDSI_SPACE;

    /* IPC 電文送受信情報 電文ログKEY 電文種別 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.denbun_shubetu = MDSI_SPACE;

    /* IPC 電文送受信情報 電文ログKEY 再送回数 */
    memset( st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.re_send_num,
             '0',
             sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_log_key.re_send_num) );

    /* IPC 電文送受信情報 電文フォーマット区分 */
    st_log_ipc.t_glnlg.denbun_send_recv_info.denbun_fmt_kubun = '0';

    /* IPC 電文送受信情報 通信ログ保存ファイル名 */
    memset( st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename,
            MDSI_SPACE,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename) );

    /* IPC 電文送受信情報 通信ログKEY */
    memcpy( &st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_key.part_id[0],
            &st_log_ipc.t_glnlg.pri_key.part_id[0],
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_key) );

    /* IPC 通信制御情報 インタフェース */
    memcpy( st_log_ipc.t_glnlg.tushin_cntrl_info.if_id,
            g_nw_info[sh_nw_info_cnt].interface_name,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.if_id) );

    /* IPC 通信制御情報 ステーション */
    memcpy( st_log_ipc.t_glnlg.tushin_cntrl_info.station_id,
            g_nw_info[sh_nw_info_cnt].station_name,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.station_id) );

    /* IPC 通信制御情報 受信コネクション論理ID */
    memcpy( &st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.recv_connect_id,
            &rcvdata->text_recv_notify.recv_con_id,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.recv_connect_id) );

    /* IPC 通信制御情報 受信コネクション情報 */
    memcpy( &st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.recv_connect_info,
            &rcvdata->text_recv_notify.recv_con_info,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.recv_connect_info) );

    /* IPC 通信制御情報 電文受信タイムスタンプ */
    memcpy( &st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.denbun_recv_time_stamp,
            &rcvdata->text_recv_notify.recv_timestamp,
            sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.line_info.denbun_recv_time_stamp) );

    /* IPC 電文長設定 */
    memset(ch_datalen_local, 0, sizeof(ch_datalen_local));
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%05d",
             g_encdec_con.dec_conf_data.decode_data_length );
    memcpy( st_log_ipc.t_glnlg.denbun_area.denbun_len,
            ch_datalen_local,
            sizeof(st_log_ipc.t_glnlg.denbun_area.denbun_len));

    /* データ長削除済みフラグ確認 */
    /* IPC MTI開始位置設定 */
    memset(ch_datalen_local, 0, sizeof(ch_datalen_local));

    if ((g_mti_kind == MDSI_MTI_JUDGE_QUE) ||
        (g_mti_kind == MDSI_MTI_JUDGE_IDL) ||
        (g_mti_kind == MDSI_MTI_JUDGE_HB)) {
        memset( st_log_ipc.t_glnlg.denbun_area.mti_start_lct, '0',
                sizeof(st_log_ipc.t_glnlg.denbun_area.mti_start_lct));
    }
    else{
        snprintf( ch_datalen_local,
                  sizeof(ch_datalen_local),
                 "%05d",
                 g_denbun_lct_info[g_denbun_lct_info_no].mti_start_lct );
        memcpy( st_log_ipc.t_glnlg.denbun_area.mti_start_lct,
                ch_datalen_local,
                sizeof(st_log_ipc.t_glnlg.denbun_area.mti_start_lct));
    }

    /* IPC 送受信電文設定 */
    memcpy( st_log_ipc.t_glnlg.denbun_area.denbun,
            g_encdec_con.dec_conf_data.decode_data,
            g_myinfo_def.data_len);

    /* IPC 送受信電文全体長設定 */
    st_log_ipc.common_header.control_data_length = 
         sizeof(c601_def) -
         sizeof(st_log_ipc.common_header) -
         sizeof(st_log_ipc.t_glnlg.denbun_area.denbun) +
         g_encdec_con.dec_conf_data.decode_data_length;

    /* ダミー領域スペース設定 */
    memset( st_log_ipc.t_glnlg.furiwake_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.furiwake_info.future_use));
    memset( st_log_ipc.t_glnlg.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.future_use));
    memset( st_log_ipc.t_glnlg.denbun_send_recv_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.future_use));
    memset( st_log_ipc.t_glnlg.tushin_cntrl_info.future_use, ' ',
                sizeof(st_log_ipc.t_glnlg.tushin_cntrl_info.future_use));

    /* ログ設定情報を保存 */
    memcpy( &g_st_log_ipc, &st_log_ipc, sizeof(g_st_log_ipc));

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, MDSI_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           MDSI_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));

    if (g_logcon_data.domain_name[0] != MDSI_SPACE){
        /* ドメイン名指定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.domain_name,
                                 sizeof(g_logcon_data.domain_name));
    }
    else{
        /* PATHMON名指定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.pathmon_name,
                                 sizeof(g_logcon_data.pathmon_name));
    }
    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_logcon_data.server_class,
                       sizeof(g_logcon_data.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &st_log_ipc, sizeof(st_log_ipc) );

    t_COM_PSD_arg_1_def.receive_max_len = sizeof(r601_def);
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_logcon_data.pathsend_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = g_logcon_data.retry_cnt;

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

    /* PATHSEND結果確認 */
    if ( MDSI_RET_OK != s_ret ) {
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_PSD", s_ret);

        memcpy( g_internal_error_code,
                DEF_NERR_PSEND_ERR_RE_OUT,
                sizeof(g_internal_error_code));

        return MDSI_RET_NG;
    }

    st_log_rcv_ipc =  (r601_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R601) */
    if ( memcmp(st_log_rcv_ipc->common_header.interface_code,
                DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
                sizeof(st_log_rcv_ipc->common_header.interface_code)) != 0 ) {
        /* 異常時処理実行 */
        /* EMS出力 応答エラー */
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_FILE_IO_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R601 ERROR          ",
                            st_log_rcv_ipc);
        return MDSI_RET_NG;
    }

    /* ログファイル名を受信バッファから取得 */
    memcpy( c_log_file_name, (char *)st_log_rcv_ipc->logfile_id,
            sizeof(st_log_rcv_ipc->logfile_id) );

    if ( MDSI_RET_LCN_DOUBLE == st_log_rcv_ipc->common_header.error_code ) {
        /* 二重送信 */
        /* 前回送信GFP内部LCNを受信バッファから取得 */
        memcpy( c_lcn, (char *)st_log_rcv_ipc->lcn,
                         sizeof(st_log_rcv_ipc->lcn) );
        /* 前回東阪送信情報を受信バッファから取得 */
        *c_site = st_log_rcv_ipc->thnkbn[0];
        return MDSI_RET_DOUBLE_SET;
    }
    else if ( MDSI_RET_OK != st_log_rcv_ipc->common_header.error_code ) {
        /* 異常終了 */
        /* EMS出力 応答エラー */
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_FILE_IO_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R601 ERROR          ",
                            &st_log_rcv_ipc);
        return MDSI_RET_NG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_log_output */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_swich_judge                                     */
/*  CALLING SEQ.    : short MDSI_swich_judge  ( short s_mti_judge,                */
/*                                              char  *ch_mti_data,               */
/*                                              c201_def *ch_dec_data_adr,        */
/*                                              char *ch_que_data_no )            */
/*                                              char *ch_tohan_kind )             */
/*  ARGUMENT        : 1. s_mti_judge      (I) 電文種別                            */
/*                  : 2. ch_mti_data      (I) MTI                                 */
/*                  : 3. ch_dec_data_adr  (I) 受信電文                            */
/*                  : 4. ch_que_data_no   (O) 受信電文振分先設定テーブルの配列番号*/
/*                  : 5. ch_tohan_kind    (O) 送信先東阪種別                      */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 振分先判定処理                                              */
/**********************************************************************************/
short MDSI_swich_judge  ( short s_mti_judge,
                          char  *ch_rcv_mti,
                          c201_def *ch_dec_data_adr,
                          char *ch_que_data_no,
                          char *ch_tohan_kind )
{
    short s_ret;
    char  ch_detour_tbl_no;
    char  tohan_detour_flag_char[2];
    char  tohan_detour_flag;
    char  data_kind_set_flg = MDSI_OFF;
    char  ch_log_file_name[MDSI_LOG_FILE_NAME_SIZE];
    char  ch_tohan          = 0;

    g_mti_judge = s_mti_judge;

    /* MTI取得対象メッセージ判定結果確認 */
    switch ( s_mti_judge ) {
    case MDSI_MTI_JUDGE_MTI:
    case MDSI_MTI_JUDGE_QUE:
    case MDSI_MTI_JUDGE_HB:
        /* 振分先キュー取得 */
        s_ret = MDSI_detour_que_get( ch_rcv_mti,
                                     &ch_detour_tbl_no);
        /* 振分先キュー取得結果確認 */
        if ( MDSI_RET_OK != s_ret ) {
            /* 異常終了 */
            //振分先判定エラー(内部エラー)
            strncpy( g_internal_error_code,
                     DEF_NERR_MTI_GET_ERR,
                     sizeof(g_internal_error_code));

            /* 受信電分振分先設定テーブルに対象MTIなし */
            g_mti_judge = MDSI_MTI_JUDGE_ERR;

            /* 不正MTI受信フラグ ON */
            g_mti_ng_flg = MDSI_ON;

            /* 障害電文通知作成依頼送信 */
            s_ret = MDSI_err_data_make ( ch_dec_data_adr);

            return MDSI_RET_NG;
        }
        /* MTIを保存 */
        memcpy(g_mti_data, ch_rcv_mti, sizeof(g_mti_data));

        /* OUTパラメータに受信電文振分先設定テーブルの配列番号を設定 */
        *ch_que_data_no = ch_detour_tbl_no;

        /* 東阪振分区分取得 */
        tohan_detour_flag_char[0]
               = g_detour_tbl[ch_detour_tbl_no].t_site_ctl.site_ctl;
        tohan_detour_flag_char[1] = 0;
        tohan_detour_flag = (char)atoi(tohan_detour_flag_char);
        /* 東阪振分処理 */
        s_ret = MDSI_tohan_detour( tohan_detour_flag );
        /* 東阪判定結果を判定する */
        if ( MDSI_TOHAN_TOKYO_INT == s_ret ) {
            /* 東京 */
            *ch_tohan_kind = DEF_SITE_ID_TKY;
        }
        else if ( MDSI_TOHAN_OSAKA_INT == s_ret ){
            /* 大阪 */
            *ch_tohan_kind = DEF_SITE_ID_OSK;
        }
        else {
            /* 異常終了 */
            //東阪振分エラー(内部エラー)
            strncpy( g_internal_error_code,
                     DEF_NERR_TO_FURIWAKE_FC2004_ERR,
                     sizeof(g_internal_error_code));
            return MDSI_RET_NG;
        }
        break;
    case MDSI_MTI_JUDGE_IDL:
        /* 送信しないのでキュー検索不要 */
        break;
    default:
        /* 設定されることはない */
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_swich_judge */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_detour_que_get                                  */
/*  CALLING SEQ.    : short MDSI_detour_que_get ( char *ch_rcv_mti,               */
/*                                                char *ch_detour_tbl_no)         */
/*  ARGUMENT        : 1. ch_rcv_mti       (I) MTI                                 */
/*                  : 2. ch_detour_tbl_no (O) 受信電分振分先設定テーブルの配列番号*/
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 振分先キュー取得                                            */
/**********************************************************************************/
short MDSI_detour_que_get ( char *ch_rcv_mti,
                            char *ch_detour_tbl_no )
{
    char  set_mti[4]        = {0};
    short loop_cnt          = 0;
    char  data_kind_set_flg = MDSI_OFF;

    /* MTI設定 */
    memcpy( set_mti, ch_rcv_mti, 4);

    /* 受信電分振分先設定テーブル検索 */
    for ( loop_cnt=0; loop_cnt<g_detour_tbl_cnt; loop_cnt++ ){
        if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                     set_mti,
                     sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
            /* 対象MTIテーブルあり */
            /* MTIをグローバル変数に設定する */
            memcpy( g_mti_data, set_mti, sizeof(g_mti_data) );
            /* 設定済フラグON */
            data_kind_set_flg = MDSI_ON;
            break;
        }
    }  /* end of while (loop_cnt) */
    /* 設定済フラグ確認 */
    if ( MDSI_ON != data_kind_set_flg ) {
        /* テーブルなし */
        /* 異常終了 */
        //振分先判定エラー(内部エラー)
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                            DEF_NERR_MTI_GET_ERR,
                            "@L@C@X@X",
                            g_lcn,
                            &g_rcv_gflin,
                            g_mti_data,
                            "FURIWAKE TABLE NONE");
        strncpy( g_internal_error_code,
                 DEF_NERR_MTI_GET_ERR,
                 sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }
    else {
        /* 受信電分振分先設定テーブル検索済み */
        /* 受信電分振分先設定テーブルの配列番号を返却 */
        *ch_detour_tbl_no = (char)loop_cnt;
    }

    return MDSI_RET_OK;

} /* end of MDSI_detour_que_get */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_tohan_detour                                    */
/*  CALLING SEQ.    : short MDSI_tohan_detour( char tohan_detour_flag )           */
/*  ARGUMENT        : 1. tohan_detour_flag      (I) 東阪振分区分                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了 1:東京 2:大阪                        */
/*  DESCRIPTION     : 東阪振分処理                                                */
/**********************************************************************************/
short MDSI_tohan_detour ( char tohan_detour_flag )
{
    short ret_tohan_kind = MDSI_RET_NG;

    /* 東阪振分区分確認 */
    switch ( tohan_detour_flag ) {
    case MDSI_TOHAN_DETOUR_1:
        /* 東阪振分の対象 */
        if ( DEF_SITE_ID_TKY == g_tohan_list.sort_list[g_tohan_list.list_no] ) {
            ret_tohan_kind = MDSI_TOHAN_TOKYO_INT;
        }
        else if ( DEF_SITE_ID_OSK == g_tohan_list.sort_list[g_tohan_list.list_no] ) {
            ret_tohan_kind = MDSI_TOHAN_OSAKA_INT;
        }

        /* 配列番号更新 */
        g_tohan_list.list_no_old = g_tohan_list.list_no;
        g_tohan_list.list_no += 1;
        if ( ( g_tohan_list.list_no >= MDSI_TOHAN_LIST_MAX ) ||
           ( MDSI_ZERO == g_tohan_list.sort_list[g_tohan_list.list_no] ) ){
            g_tohan_list.list_no = 0;
        }

        /* 振分先選択処理区分に常に東阪振分の対象電文(正常)を設定 */
        memcpy( g_tohan_proc_kind, MDSI_TOHAN_PROC_NOMAL, sizeof(g_tohan_proc_kind));

        break;
    case MDSI_TOHAN_DETOUR_2:
        /* エラー時のみ東阪振分の対象 */
        /* 自プロセスのサイトを設定 */
        if ( DEF_SITE_ID_TKY == g_myinfo_def.config_data.site_id ) {
            ret_tohan_kind = MDSI_TOHAN_TOKYO_INT;
        }
        else if ( DEF_SITE_ID_OSK == g_myinfo_def.config_data.site_id ) {
            ret_tohan_kind = MDSI_TOHAN_OSAKA_INT;
        }
        /* 振分先選択処理区分に常にエラー時のみ東阪振分の対象電文(正常)を設定 */
        memcpy( g_tohan_proc_kind, MDSI_TOHAN_PROC_ERR_NOMAL, sizeof(g_tohan_proc_kind));

        break;
    case MDSI_TOHAN_DETOUR_3:
        /* 東阪振分の対象外 */
        /* 自プロセスのサイトを設定 */
        if ( DEF_SITE_ID_TKY == g_myinfo_def.config_data.site_id ) {
            ret_tohan_kind = MDSI_TOHAN_TOKYO_INT;
        }
        else if ( DEF_SITE_ID_OSK == g_myinfo_def.config_data.site_id ) {
            ret_tohan_kind = MDSI_TOHAN_OSAKA_INT;
        }
        /* 振分先選択処理区分に常に東阪振分の対象外電文を設定 */
        memcpy( g_tohan_proc_kind, MDSI_TOHAN_PROC_NOT, sizeof(g_tohan_proc_kind));

        break;
    default:
        //東阪振分エラー(内部エラー)
        strncpy( g_internal_error_code,
                 DEF_NERR_TO_FURIWAKE_FC2004_ERR,
                 sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }

    return ret_tohan_kind;

} /* end of MDSI_tohan_detour */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_data_make                                       */
/*  CALLING SEQ.    : short MDSI_data_make ( char c301_data_kind,                 */
/*                                           c201_def *rcv_data_adr,              */
/*                                           t_detour_tbl *detour_tbl_adr,        */
/*                                           char tohan_kind,                     */
/*                                           char *err_code,                      */
/*                                           c301_def *send_data_adr)             */
/*  ARGUMENT        : 1. c301_data_kind (I) 作成データ種別                        */
/*                  : 2. rcv_data_adr   (I) 受信データ                            */
/*                  : 3. detour_tbl_adr (I) 送信先受信電文振分先設定テーブル      */
/*                  : 4. tohan_kind     (I) 送信先東阪種別                        */
/*                  : 5. err_code       (I) 内部エラーコード                      */
/*                  : 6. send_data_adr  (O) 送信電文                              */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 送信電文作成処理                                            */
/**********************************************************************************/
short MDSI_data_make ( char         c301_data_kind,
                       c201_def     *rcv_data_adr,
                       t_detour_tbl *detour_tbl_adr,
                       char         tohan_kind,
                       char         *err_code,
                       c301_def     *c301_send_data_adr )
{
    short        loop_cnt          = 0;
    c301_def     c301_send_data;
    char         *msg_data_adr     = NULL;
    short        msg_data_length   = 0;
    db_gqnwq_def *c301_que_file    = (db_gqnwq_def *)c301_send_data.que_rgs_info.msg_data;
    char         data_kind_set_flg = MDSI_OFF;
    short        sh_nw_info_cnt    = 0;
    char         ch_set_mti[4];
    char         ch_datalen_local[5];
    short        s_data_start_posi = 0;

    /* 送信電文設定領域クリア */
    memset( &c301_send_data, 0, sizeof(c301_send_data) );

    /* IPC interface_code設定 (C301) */
    memcpy( c301_send_data.common_header.interface_code,
                       DEF_IPC_IFCD_Q_RGST_REQ, 4 );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( c301_send_data.common_header.internal_error_code, MDSI_SPACE,
            sizeof(c301_send_data.common_header.internal_error_code));
    memset( c301_send_data.common_header.filler_1, MDSI_SPACE,
            sizeof(c301_send_data.common_header.filler_1));

    /* IPC GFP内部LCN */
    /* 業務応答の場合はスペース設定 */
    if (detour_tbl_adr->t_site_ctl.denbun_shubetu == DEF_SEND_DENBUN_RSP) {
        memset( c301_send_data.que_rgs_info.gfp_lcn,
                MDSI_SPACE,
                sizeof(c301_send_data.que_rgs_info.gfp_lcn) );
    }
    else{
        memcpy( c301_send_data.que_rgs_info.gfp_lcn,
                g_lcn,
                sizeof(c301_send_data.que_rgs_info.gfp_lcn) );
    }

    /* IPC サイト識別 */
    c301_send_data.que_rgs_info.serverclass_info.site_name =
          g_myinfo_def.config_data.site_id;

    /* IPC サーバクラス論理名   */
    memcpy( c301_send_data.que_rgs_info.serverclass_info.serverclass_id.serverclass_name,
            g_myinfo_def.config_data.serverclass_name,
            sizeof(c301_send_data.que_rgs_info.serverclass_info.serverclass_id.serverclass_name) );

    /* IPC サーバクラス論理番号 */
    memcpy( c301_send_data.que_rgs_info.serverclass_info.serverclass_id.serverclass_num,
            g_myinfo_def.config_data.serverclass_no,
            sizeof(c301_send_data.que_rgs_info.serverclass_info.serverclass_id.serverclass_num) );

    /* IPC 制御情報 処理コード区分設定 */
    if ( MDSI_C301_KIND_NOMAL == c301_data_kind ) {
        /* キュー登録要求 */
        memcpy( &c301_que_file->cntrl_info.shori_kubun, MDSI_C301_KIND_NOMAL_IPC, 2 );
    }
    else {
        /* 障害電文通知依頼 */
        memcpy( &c301_que_file->cntrl_info.shori_kubun, MDSI_C301_KIND_ERR_IPC, 2 );
    }

    /* IPC 制御情報 内部エラーコード設定 */
    memcpy( c301_que_file->cntrl_info.err_code, err_code,
               sizeof( c301_que_file->cntrl_info.err_code ) );

    /* IPC 制御情報 ACQ処理ノード区分設定 */
    c301_que_file->cntrl_info.acc_node_kubun = MDSI_SPACE;

    /* IPC 制御情報 ISS接続ノード区分設定 */
    c301_que_file->cntrl_info.iss_node_kubun = MDSI_SPACE;

    /* IPC 制御情報 再登録回数設定 */
    memset( &c301_que_file->cntrl_info.re_rgst_num, '0',
               sizeof( c301_que_file->cntrl_info.re_rgst_num ) );

    /* IPC 制御情報 予備設定 */
    memset( &c301_que_file->cntrl_info.future_use, MDSI_SPACE,
               sizeof( c301_que_file->cntrl_info.future_use ) );

    /* IPC 電文送受信情報 電文受信時刻設定 */
    memcpy( &c301_que_file->denbun_send_recv_info.denbun_recv_time,
            &rcv_data_adr->text_recv_notify.recv_timestamp,
            sizeof(c301_que_file->denbun_send_recv_info.denbun_recv_time) );

    /* IPC 電文送受信情報 受信時局状態 */
    /* 局状態をIPCに設定 */
    c301_que_file->denbun_send_recv_info.recv_kyoku_sts = g_station_sts;

    /* NW情報検索 */
    for ( sh_nw_info_cnt=0; sh_nw_info_cnt<MDSI_IF_MAX; sh_nw_info_cnt++){
        if ( 0 == memcmp(g_nw_info[sh_nw_info_cnt].interface_id,
                         rcv_data_adr->text_recv_notify.recv_con_id.interface_name,
                         sizeof(g_nw_info[sh_nw_info_cnt].interface_id))){
            /* NW情報あり */
            break;
        }
    }
    if ( sh_nw_info_cnt >= MDSI_IF_MAX){
        /* インタフェース識別なし */
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* IPC 電文送受信情報 NW区分設定 */
    memcpy( c301_que_file->denbun_send_recv_info.nw_kubun,
            g_nw_info[0].nw_segment,
            sizeof(c301_que_file->denbun_send_recv_info.nw_kubun) );

    /* IPC 電文送受信情報 MTI */
    /* IPC MTI設定 */
    memcpy( c301_que_file->denbun_send_recv_info.mti_id,
            g_mti_save,
            sizeof(c301_que_file->denbun_send_recv_info.mti_id) );

    /* IPC 電文送受信情報 送信電文種別 */
    switch ( g_mti_judge ) {
    case MDSI_MTI_JUDGE_MTI:
    case MDSI_MTI_JUDGE_ERR:

        if ( MDSI_C301_KIND_NOMAL == c301_data_kind ) {
            /* キュー登録要求 */
            memcpy( ch_set_mti, c301_que_file->denbun_send_recv_info.mti_id, sizeof(ch_set_mti));
        }
        else {
            /* 障害電文通知依頼 */
            memcpy( ch_set_mti, "ZZZZ", sizeof(ch_set_mti));
        }

        /* 不正MTI受信フラグ判定 */
        if (g_mti_ng_flg == MDSI_ON){
            /* MTI不正の場合は破棄通知を設定 */
            c301_que_file->denbun_send_recv_info.send_denbun_shubetu = MDSI_CTL_HAKI;
        }
        else {
            /* 受信電分振分先設定テーブル検索 */
            for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
                if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                    /* 受信電分振分先設定テーブルに対象MTIなし */
                    /* MTI異常エラー */
                    /* 振分先判定不可(EMS) */
                    /* EMS出力 */
                    MDSI_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                                        DEF_NERR_MTI_GET_ERR,
                                        "@L@C@X@X",
                                        g_lcn,
                                        &g_rcv_gflin,
                                        g_mti_data,
                                        "MTI NONE");
                    strncpy( g_internal_error_code,
                             DEF_NERR_MTI_GET_ERR,
                             sizeof(g_internal_error_code));

                    return MDSI_RET_NG;
                }
                if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                     ch_set_mti,
                     sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                    /* 対象MTIテーブルあり */
                    /* 受信電分振分先設定テーブル業務/制御設定 */
                    c301_que_file->denbun_send_recv_info.send_denbun_shubetu
                        = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;

                    /* 設定済フラグON */
                    data_kind_set_flg = MDSI_ON;
                    break;
                }
            }  /* end of while */
            /* 設定済フラグ確認 */
            if (MDSI_ON != data_kind_set_flg ){
                    /* 受信電分振分先設定テーブルに対象MTIなし */
                    /* MTI異常エラー */
                    /* 振分先判定不可(EMS) */
                    /* EMS出力 */
                    MDSI_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                                        DEF_NERR_MTI_GET_ERR,
                                        "@L@C@X@X",
                                        g_lcn,
                                        &g_rcv_gflin,
                                        g_mti_data,
                                        "MTI NONE");
                    //振分先判定エラー(内部エラー)
                    strncpy( g_internal_error_code,
                             DEF_NERR_MTI_GET_ERR,
                             sizeof(g_internal_error_code));

                    return MDSI_RET_NG;
            }
            break;
        case MDSI_MTI_JUDGE_QUE:
            /* リジェクトを設定 */
            /* 受信電分振分先設定テーブル検索 */
            for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
                if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                    /* 受信電分振分先設定テーブルに対象MTIなし */
                    memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
                }
                if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                     "YYY1",
                     sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                    /* 対象MTIテーブルあり */
                    /* 受信電分振分先設定テーブル業務/制御設定 */
                    c301_que_file->denbun_send_recv_info.send_denbun_shubetu
                        = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                    /* 設定済フラグON */
                    data_kind_set_flg = MDSI_ON;
                    break;
                }
            }  /* end of while */
            /* 設定済フラグ確認 */
            if (MDSI_ON != data_kind_set_flg ){
                    /* 受信電分振分先設定テーブルに対象MTIなし */
                    /* MTI異常エラー */
                    /* 振分先判定不可(EMS) */
                    /* EMS出力 */
                    MDSI_message_output(DEF_EVT_FURIWAKE_HANTE_ERR,
                                        DEF_NERR_MTI_GET_ERR,
                                        "@L@C@X@X",
                                        g_lcn,
                                        &g_rcv_gflin,
                                        g_mti_data,
                                        "MTI NONE");
                    //振分先判定エラー(内部エラー)
                    strncpy( g_internal_error_code,
                             DEF_NERR_MTI_GET_ERR,
                             sizeof(g_internal_error_code));

                    return MDSI_RET_NG;
            }
            break;

        case MDSI_MTI_JUDGE_HB:
            /* ハートビートを設定 */
            c301_que_file->denbun_send_recv_info.send_denbun_shubetu = MDSI_CTL_HB;
            break;
        default:
            return MDSI_RET_NG;
        }
    }

    /* IPC 電文送受信情報 電文ログKEY GFP内部LCN */
    /* 業務応答の場合はスペース設定 */
    if (detour_tbl_adr->t_site_ctl.denbun_shubetu == DEF_SEND_DENBUN_RSP) {
        memset( &c301_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
                MDSI_SPACE,
                sizeof(c301_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id) );
    }
    else{
        memcpy( &c301_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
                g_lcn,
                sizeof(c301_que_file->denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id) );
    }

    /* IPC 電文送受信情報 電文ログKEY 電文形態 */
    c301_que_file->denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai = MDSI_SPACE;

    /* IPC 電文送受信情報 電文ログKEY 電文種別 */
    c301_que_file->denbun_send_recv_info.denbun_log_key.denbun_shubetu = MDSI_SPACE;

    /* IPC 電文送受信情報 電文ログKEY 再送回数 */
    memset( c301_que_file->denbun_send_recv_info.denbun_log_key.re_send_num, '0',
            sizeof(c301_que_file->denbun_send_recv_info.denbun_log_key.re_send_num) );

    /* IPC 電文送受信情報 電文フォーマット区分 */
    c301_que_file->denbun_send_recv_info.denbun_fmt_kubun = '0';

    /* IPC 電文送受信情報 通信ログ保存ファイル名 */
    memcpy( c301_que_file->denbun_send_recv_info.tushin_log_save_filename,
            g_log_file_name,
            sizeof(c301_que_file->denbun_send_recv_info.tushin_log_save_filename) );

    /* IPC 電文送受信情報 通信ログKEY */
    memcpy( &c301_que_file->denbun_send_recv_info.tushin_log_key.part_id[0],
            &rcv_data_adr->text_recv_notify.recv_timestamp.time_stamp[18],
            sizeof(c301_que_file->denbun_send_recv_info.tushin_log_key.part_id) );
    memcpy( c301_que_file->denbun_send_recv_info.tushin_log_key.time_stamp,
            &rcv_data_adr->text_recv_notify.recv_timestamp,
            sizeof(rcv_data_adr->text_recv_notify.recv_timestamp) );

    /* IPC 通信制御情報 インタフェース */
    memcpy( c301_que_file->tushin_cntrl_info.if_id,
            g_nw_info[sh_nw_info_cnt].interface_name,
            sizeof(c301_que_file->tushin_cntrl_info.if_id) );

    /* IPC 通信制御情報 ステーション */
    memcpy( c301_que_file->tushin_cntrl_info.station_id,
            g_nw_info[sh_nw_info_cnt].station_name,
            sizeof(c301_que_file->tushin_cntrl_info.station_id) );

    /* IPC 通信制御情報 受信コネクション論理ID */
    memcpy( &c301_que_file->tushin_cntrl_info.line_info.recv_connect_id,
            &rcv_data_adr->text_recv_notify.recv_con_id,
            sizeof(c301_que_file->tushin_cntrl_info.line_info.recv_connect_id) );

    /* IPC 通信制御情報 受信コネクション情報 */
    memcpy( &c301_que_file->tushin_cntrl_info.line_info.recv_connect_info,
            &rcv_data_adr->text_recv_notify.recv_con_info,
            sizeof(c301_que_file->tushin_cntrl_info.line_info.recv_connect_info) );

    /* IPC 通信制御情報 電文受信タイムスタンプ */
    memcpy( &c301_que_file->tushin_cntrl_info.line_info.denbun_recv_time_stamp,
            &rcv_data_adr->text_recv_notify.recv_timestamp,
            sizeof(rcv_data_adr->text_recv_notify.recv_timestamp) );

    /* メッセージのデータアドレス設定 */
    msg_data_adr = &g_encdec_con.dec_conf_data.decode_data[0];
    /* データ長領域を削除する */
    /* 受信電文のデータ長削除処理 */
    if (g_date_start_posi == 1){
        msg_data_length = g_encdec_con.dec_conf_data.decode_data_length;
    }
    else {
        msg_data_adr += (g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct - 1);
        msg_data_length = g_encdec_con.dec_conf_data.decode_data_length
                           - g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct
                           + 1;
    }

    /* IPC 送受信電文設定 */
    memcpy( &c301_que_file->denbun_area.denbun[0],
            msg_data_adr, msg_data_length );

    /* データ長保存 */
    g_myinfo_def.data_len = msg_data_length;
    /* データ長削除済みフラグON */
    g_myinfo_def.data_len_del_flag = MDSI_ON;

    /* 予備領域にスペース設定 */
    memset( c301_que_file->denbun_send_recv_info.future_use, MDSI_SPACE,
               sizeof( c301_que_file->denbun_send_recv_info.future_use ) );
    memset( c301_que_file->tushin_cntrl_info.future_use, MDSI_SPACE,
               sizeof( c301_que_file->tushin_cntrl_info.future_use ) );

    /* IPC DATA部電文長設定 */
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%04d",
             msg_data_length );
    memcpy( c301_que_file->denbun_area.denbun_len,
            ch_datalen_local,
            sizeof(c301_que_file->denbun_area.denbun_len));

    /* IPC MTI開始位置設定 */
    /* 電文開始位置移動確認 */
    if (g_date_start_posi == 1){
        g_mti_start_posi = g_denbun_lct_info[g_denbun_lct_info_no].mti_start_lct;
    }
    else {
        s_data_start_posi = g_date_start_posi - 1;
        g_mti_start_posi = g_denbun_lct_info[g_denbun_lct_info_no].mti_start_lct - s_data_start_posi;
    }
    snprintf( ch_datalen_local,
              sizeof(ch_datalen_local),
             "%04d",
             g_mti_start_posi );
    memcpy( c301_que_file->denbun_area.mti_start_lct,
            ch_datalen_local,
            sizeof(c301_que_file->denbun_area.mti_start_lct));

    if ((g_mti_kind == MDSI_MTI_JUDGE_QUE) ||
        (g_mti_kind == MDSI_MTI_JUDGE_IDL) ||
        (g_mti_kind == MDSI_MTI_JUDGE_HB)) {
        memset( c301_que_file->denbun_area.mti_start_lct,
                '0',
                sizeof(c301_que_file->denbun_area.mti_start_lct));
    }

    /* IPC 送受信電文全体長設定 */
    c301_send_data.common_header.control_data_length =
          (unsigned short)((sizeof(c301_send_data.que_rgs_info) - 9999)
                          + (sizeof(db_gqnwq_def) - 9999)
                          + msg_data_length);

    /* 送信サイト保存 */
    g_tohan_kind = tohan_kind;

    /* C301データ設定 */
    memcpy( c301_send_data_adr, &c301_send_data, sizeof(c301_def) );

    return MDSI_RET_OK;

} /* end of MDSI_data_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_data_send                                 */
/*  CALLING SEQ.    : short MDSI_data_send( t_detour_tbl *detour_tbl,       */
/*                                          c301_def *send_deta_adr,        */
/*                                          c201_def *c201_msg)             */
/*  ARGUMENT        : 1. detour_tbl      (I) 電文振分先テーブル             */
/*                  : 2. send_deta_adr   (I) 送信電文のアドレス             */
/*                  : 3. c201_msg        (I) 受信電文のアドレス             */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : キュー登録要求送信処理                                */
/****************************************************************************/
short MDSI_data_send (  t_detour_tbl *detour_tbl,
                        c301_def *send_deta_adr,
                        c201_def *c201_msg)
{
    short s_ret;
    short loop_cnt = 0;

    c301_def send_data_save;
    r301_def *r301_data_save;

    short detour_end_flag = 0;          /* 東阪迂回済みフラグ */
    short send_retry_cnt  = 0;          /* 送信再送回数       */
    short ukai_flg        = MDSI_UKAI_EXEC; 
                                       /* 迂回可能フラグ     */

    memset( &send_data_save, 0, sizeof(send_data_save) );
    memcpy( &send_data_save, send_deta_adr, sizeof(send_data_save) );

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    for( loop_cnt=0; loop_cnt<2; loop_cnt++ ) {
        /* ログ出力処理 */
        if (( MDSI_MTI_JUDGE_MTI == g_mti_judge ) ||
            ( MDSI_MTI_JUDGE_QUE == g_mti_judge ) ||
            ( MDSI_MTI_JUDGE_HB  == g_mti_judge )) {
            /*迂回時のみNW通信ログ更新 */
            if (detour_end_flag == MDSI_ON){
                /* 電文ログ出力 */
                s_ret = MDSI_log_output_up( (char *)&send_data_save );
                if ( MDSI_RET_OK != s_ret ) {
                    /* 異常終了 エラーログ出力 処理継続 */
                    /* エラーログ出力 処理継続 */
                    s_ret = MDSI_err_log_set( c201_msg,
                                              DEF_ELG_INTERNAL,
                                              DEF_NERR_FILE_IO_ERR);
                }
            }
        }

        /* PATHSENDパラメータ初期化 */
        memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
        memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
        memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
        memset( &t_COM_PSD_arg_4_def, MDSI_SPACE, sizeof(t_COM_PSD_arg_4_def) );

        /* PATHSEND用情報設定 */

        /* 東阪種別により送信先設定 */
        memset(t_COM_PSD_arg_1_def.pathmon_name,
               MDSI_SPACE,
               sizeof(t_COM_PSD_arg_1_def.pathmon_name));
        if ( DEF_SITE_ID_TKY == g_tohan_kind ) {
            if (detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_domain_name[0] != MDSI_SPACE){
                /* ドメイン名指定 */
                memcpy(t_COM_PSD_arg_1_def.pathmon_name,
                       detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_domain_name,
                       sizeof(detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_domain_name));
            }
            else{
                /* PATHMON名指定 */
                memcpy(t_COM_PSD_arg_1_def.pathmon_name,
                       detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_pathmon,
                       sizeof(detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_pathmon));
            }
            memcpy(t_COM_PSD_arg_1_def.serverclass_name,
                   detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_serverclass,
                   sizeof(detour_tbl->t_phy_data.t_tokyo_site_phy.tokyo_serverclass));
        }
        else {
            if (detour_tbl->t_phy_data.t_osaka_site_phy.osaka_domain_name[0] != MDSI_SPACE){
                /* ドメイン名指定 */
                memcpy(t_COM_PSD_arg_1_def.pathmon_name,
                       detour_tbl->t_phy_data.t_osaka_site_phy.osaka_domain_name,
                       sizeof(detour_tbl->t_phy_data.t_osaka_site_phy.osaka_domain_name));
            }
            else{
                /* PATHMON名指定 */
                memcpy(t_COM_PSD_arg_1_def.pathmon_name,
                       detour_tbl->t_phy_data.t_osaka_site_phy.osaka_pathmon,
                       sizeof(detour_tbl->t_phy_data.t_osaka_site_phy.osaka_pathmon));
            }
            memcpy(t_COM_PSD_arg_1_def.serverclass_name,
                   detour_tbl->t_phy_data.t_osaka_site_phy.osaka_serverclass,
                   sizeof(detour_tbl->t_phy_data.t_osaka_site_phy.osaka_serverclass));
        }

        memcpy(t_COM_PSD_arg_1_def.msg_buf, &send_data_save, sizeof(send_data_save) );

        t_COM_PSD_arg_1_def.receive_max_len = MDSI_MAX_DATA_SIZE;
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

        for (send_retry_cnt=0;
             send_retry_cnt<=g_myinfo_def.config_data.send_retry_count;
             send_retry_cnt++) {
            memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
            /* PATHSEND共通処理実行 */
            s_ret = COM_PSD( &t_COM_PSD_arg_1_def,
                             &t_COM_PSD_arg_2_def,
                             &t_COM_PSD_arg_3_def,
                             &g_cg010in_modle,
                             &t_COM_PSD_arg_4_def );

            /* 応答電文異常終了判定 */
            if ( s_ret == MDSI_RET_OK ){
                r301_data_save = (r301_def *)t_COM_PSD_arg_1_def.msg_buf;
                /* 応答電文のインタフェースコード判定(R301) */
                if ( memcmp(r301_data_save->common_header.interface_code,
                            DEF_IPC_IFCD_Q_RGST_RSP,
                            sizeof(r301_data_save->common_header.interface_code)) == 0 ) {

                    /* 応答電文のエラーコード判定 */
                    if (r301_data_save->common_header.error_code == DEF_IPC_ERRCD_OK) {
                        /* 電文送信正常終了 */
                        return MDSI_RET_OK;
                    }
                }
                /* EMS出力 応答エラー */
                MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                                    g_lcn, g_myinfo_def.config_data.serverclass_name,
                                    "R301 ERROR          ",
                                    r301_data_save);
                break;
            }

            /* PATHSEND結果確認 */
            if ( s_ret == MDSI_RET_NG ) {
                MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                                    DEF_NERR_SEND_ERR,
                                    "@X@E",
                                    "COM_PSD",
                                    t_COM_PSD_arg_3_def.pathsend_errcode);

                if ((t_COM_PSD_arg_3_def.pathsend_errcode == MDSI_PATHSEND_ERR_905) ||
                    (t_COM_PSD_arg_3_def.pathsend_errcode == MDSI_PATHSEND_ERR_913) ||
                    (t_COM_PSD_arg_3_def.pathsend_errcode == MDSI_PATHSEND_ERR_915)) {
                    /* 異常時の迂回処理実行 */
                }
                else {
                    /* 迂回対象外エラー */
                    ukai_flg = MDSI_OFF;
                }
            }
        }  /* 再送のループ */

        if ( ukai_flg == MDSI_UKAI_EXEC ) {
            /* 迂回処理開始 */
            /* 迂回済み判定 */
            if ( MDSI_ON == detour_end_flag ) {
                /* 迂回済み */
                /* 異常終了 */
                MDSI_message_output(DEF_EVT_UKAI_ERR, DEF_NERR_TO_FURIWAKE_FC2004_ERR,
                                       "@L@C@X",
                                       g_lcn,
                                       t_COM_PSD_arg_1_def.serverclass_name,
                                       "ｳｶｲｿｳｼﾝｴﾗｰ");

                return MDSI_RET_NG;
            }
            else {
                /* 迂回処理 */
                s_ret = MDSI_err_detour( detour_tbl );
                if ( s_ret == MDSI_TOHAN_TOKYO_INT ) {
                    /* 迂回あり。迂回先を設定 */
                    g_tohan_kind = DEF_SITE_ID_TKY;
                    /* 迂回済みフラグON設定 */
                    detour_end_flag = MDSI_ON;
                }
                else if ( s_ret == MDSI_TOHAN_OSAKA_INT ){
                    /* 迂回あり。迂回先を設定 */
                    g_tohan_kind = DEF_SITE_ID_OSK;
                    /* 迂回済みフラグON設定 */
                    detour_end_flag = MDSI_ON;
                }
                else {
                    /* 迂回不可 */
                    /* 異常終了 */
                    /* EMS出力 */
                    /* PATHSENDエラー(EMS) */
                    MDSI_message_output(DEF_EVT_SITE_UKAI_ERR, DEF_NERR_SEND_ERR,
                                           "@L@C",
                                           g_lcn,
                                           t_COM_PSD_arg_1_def.serverclass_name,
                                           "ｳｶｲｿｳｼﾝﾌｶ ");

                    return MDSI_RET_NG;
                }
            }
        }
        else {
            /* 迂回不可 */
            return MDSI_RET_NG;
        }
    }  /* 迂回送信のループ */

    return MDSI_RET_OK;

} /* end of MDSI_data_send */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_log_output_up                             */
/*  CALLING SEQ.    : short MDSI_log_output_up ( char *sendrcvdata)         */
/*  ARGUMENT        : 1. sendrcvdata      (I) ログ出力対象送信電文          */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文ログ出力処理（更新）                              */
/****************************************************************************/
short MDSI_log_output_up ( char *sendrcvdata )
{
    short s_ret;

    c601_def      st_log_ipc;         /* ログ出力要求IPC設定テーブル */
    r601_def      *st_log_rcv_ipc;    /*ログ出力応答IPC設定テーブル */

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* ログ出力要求IPC設定テーブル初期化 */
    memset( &st_log_ipc, 0, sizeof(st_log_ipc) );
    memcpy( &st_log_ipc, &g_st_log_ipc, sizeof(st_log_ipc));

    /* IPC 登録/更新区分設定 */
    st_log_ipc.entry_update_cate = MDSI_LOG_UPDATE;

    /* IPC ログファイル名を設定 */
    memcpy( st_log_ipc.logfile_id,
            g_log_file_name,
            sizeof(st_log_ipc.logfile_id));

    /* IPC 東阪振分情報 振分先 */
    st_log_ipc.t_glnlg.furiwake_info.furiwake_dst = g_tohan_kind;

    /* IPC 東阪振分情報 振分先選択処理区分 */
    memcpy( st_log_ipc.t_glnlg.furiwake_info.furiwake_kubun,
            g_tohan_proc_kind,
            sizeof(st_log_ipc.t_glnlg.furiwake_info.furiwake_kubun));

    /* IPC 電文送受信情報 通信ログ保存ファイル名 */
    memcpy( st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename,
            g_log_file_name,
            sizeof(st_log_ipc.t_glnlg.denbun_send_recv_info.tushin_log_save_filename) );

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, MDSI_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           MDSI_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));

    if (g_logcon_data.domain_name[0] != MDSI_SPACE){
        /* ドメイン名設定 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.domain_name,
                                 sizeof(g_logcon_data.domain_name));
    }
    else{
        /* PATHMON名 */
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,g_logcon_data.pathmon_name,
                                 sizeof(g_logcon_data.pathmon_name));
    }

    memcpy(t_COM_PSD_arg_1_def.serverclass_name, g_logcon_data.server_class,
                       sizeof(g_logcon_data.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &st_log_ipc, sizeof(st_log_ipc) );

    t_COM_PSD_arg_1_def.receive_max_len = sizeof(r601_def);
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_logcon_data.pathsend_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = g_logcon_data.retry_cnt;

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

    /* PATHSEND結果確認 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_PSD", s_ret);

        memcpy( g_internal_error_code,
                DEF_NERR_PSEND_ERR_RE_OUT,
                sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }

    st_log_rcv_ipc =  (r601_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R601) */
    if ( memcmp(st_log_rcv_ipc->common_header.interface_code,
                DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
                sizeof(st_log_rcv_ipc->common_header.interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (st_log_rcv_ipc->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常時の迂回処理実行 */
            s_ret = MDSI_RET_NG;
        }
    }
    else {
        /* 異常時処理実行 */
        s_ret = MDSI_RET_NG;
    }
    if ( s_ret == MDSI_RET_NG ) {
        /* EMS出力 応答エラー */
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_FILE_IO_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "R601 ERROR log_up   ",
                            st_log_rcv_ipc);
        return MDSI_RET_NG;
    }

    /* ログファイル名を受信バッファから取得 */
    memset( g_log_file_name, MDSI_SPACE, sizeof(g_log_file_name));
    memcpy( g_log_file_name, (char *)st_log_rcv_ipc->logfile_id,
            sizeof(st_log_rcv_ipc->logfile_id) );

    return MDSI_RET_OK;

} /* end of MDSI_log_output_up */

/**********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_err_data_make                                   */
/*  CALLING SEQ.    : short MDSI_err_data_make ( c201_def *rcv_data_adr)          */
/*  ARGUMENT        : 1. rcv_data_adr     (I) 受信電文                            */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                      */
/*  DESCRIPTION     : 障害電文通知作成依頼作成送信処理                            */
/**********************************************************************************/
short MDSI_err_data_make ( c201_def *rcv_data_adr )
{
    short    s_ret                     = 0;
    c301_def c301_send_data;
    char     tohan_detour_flag_char[2] = {0};
    char     ch_tohan_kind             = 0;
    int      tohan_detour_flag         = 0;
    char     ch_detour_data_no         = 0;
    char     ch_rcv_mti[4];

    /* エラーログ出力 */
    s_ret = MDSI_err_log_set( rcv_data_adr,
                              DEF_ELG_INTERNAL,
                              g_internal_error_code);

    if ( g_syogai_ind_unit != MDSI_SYOGAITUCHI_ARI ) {
        /* CARDNET、J-LINK 以外は障害電文通知作成依頼を送信しない */
        return MDSI_RET_OK;
    }

    memcpy( ch_rcv_mti, "ZZZZ", 4);

    /* 送信先キュー検索 */
    s_ret = MDSI_detour_que_get ( ch_rcv_mti,
                                  &ch_detour_data_no );

    /* 送信先キュー検索結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        
        return MDSI_RET_NG;
    }

    /* 東阪振分区分取得 */
    tohan_detour_flag_char[0] = g_detour_tbl[ch_detour_data_no].t_site_ctl.site_ctl;
    tohan_detour_flag_char[1] = 0;
    tohan_detour_flag = (char)atoi(tohan_detour_flag_char);
    /* 東阪振分処理 */
    s_ret = MDSI_tohan_detour( (char)tohan_detour_flag );
    /* 東阪判定結果を判定する */
    if ( MDSI_TOHAN_TOKYO_INT == s_ret ) {
        /* 東京 */
        ch_tohan_kind = DEF_SITE_ID_TKY;
    }
    else if ( MDSI_TOHAN_OSAKA_INT == s_ret ) {
        /* 大阪 */
        ch_tohan_kind = DEF_SITE_ID_OSK;
    }
    else {
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* 障害電文通知依頼 電文作成処理 */
    s_ret = MDSI_data_make( MDSI_C301_KIND_ERR,
                            rcv_data_adr,
                            &g_detour_tbl[ch_detour_data_no],
                            ch_tohan_kind,
                            g_internal_error_code,
                            &c301_send_data );

    /* 送信先キュー検索結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    g_mti_judge = MDSI_MTI_JUDGE_ERR;

    /* キュー登録要求送信処理 */
    s_ret = MDSI_data_send(&g_detour_tbl[ch_detour_data_no],
                           &c301_send_data,
                           rcv_data_adr);

    /* 送信先キュー検索結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS発行 */
        
        return MDSI_RET_NG;
    }
    /* 障害電文通知作成依頼(EMS) */
    /* EMS出力 */
    MDSI_message_output(DEF_EVT_SHOGAI_NTF_CRE_REQ, DEF_NERR_HSMK_REQ_SEISA_ERR, 
                         "@L@C",
                         g_lcn,
                         &g_rcv_gflin);

    return MDSI_RET_OK;

} /* end of MDSI_err_data_make */

/***********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_err_detour                                       */
/*  CALLING SEQ.    : short MDSI_err_detour( t_detour_tbl *detour_tbl_adr )        */
/*  ARGUMENT        : 1. detour_tbl_adr   (I) 東阪振分区分                         */
/*  RETURN CODE     : 0:迂回不可 -1:異常終了 1:東京 2:大阪                         */
/*  DESCRIPTION     : 送信エラー時迂回先判定処理                                   */
/***********************************************************************************/
short MDSI_err_detour ( t_detour_tbl *detour_tbl_adr )
{
    char  tohan_detour_flag         = 0;
    char  tohan_detour_flag_char[2] = {0};
    short set_tohan_kind            = 0;

    /* 東阪振分区分取得 */
    tohan_detour_flag_char[0]
            = detour_tbl_adr->t_site_ctl.site_ctl;
    tohan_detour_flag_char[1] = 0;
    tohan_detour_flag = (char)atoi(tohan_detour_flag_char);

    /* 東阪振分区分確認 */
    switch ( tohan_detour_flag ) {
    case MDSI_TOHAN_DETOUR_1:
    case MDSI_TOHAN_DETOUR_2:
        /* 東阪送信済みフラグ判定 */
        if ( DEF_SITE_ID_TKY == g_tohan_kind ) {
            /* 東京送信済み */
            /* 大阪の比率判定 */
            if ( 0 < g_tohan_rate.osaka_rate ) {
                /* 迂回実行 */
                set_tohan_kind = MDSI_TOHAN_OSAKA_INT;
            }
            else {
                /* 迂回不可 */
                set_tohan_kind = MDSI_DETOUR_IMPOSSIBLE_INT;
            }
        }
        else {
            /* 大阪送信済み */
            /* 東京の比率判定 */
            if ( 0 < g_tohan_rate.tokyo_rate ) {
                /* 迂回実行 */
                set_tohan_kind = MDSI_TOHAN_TOKYO_INT;
            }
            else {
                /* 迂回不可 */
                set_tohan_kind = MDSI_DETOUR_IMPOSSIBLE_INT;
            }
        }
        /* 振分先選択処理区分設定 */
        if ( MDSI_DETOUR_IMPOSSIBLE_INT != set_tohan_kind ){
            if ( MDSI_TOHAN_DETOUR_1 == tohan_detour_flag ) {
                memcpy( g_tohan_proc_kind, MDSI_TOHAN_PROC_UKAI,
                                           sizeof(g_tohan_proc_kind));
            }
            else if ( MDSI_TOHAN_DETOUR_2 == tohan_detour_flag ) {
                memcpy( g_tohan_proc_kind, MDSI_TOHAN_PROC_ERR_UKAI,
                                           sizeof(g_tohan_proc_kind));
            }
        }

        break;
    case MDSI_TOHAN_DETOUR_3:
        /* 東阪振分の対象外 */
        /* 迂回不可 */
        set_tohan_kind = MDSI_DETOUR_IMPOSSIBLE_INT;
        break;
    default:
        /* ありえないルート処理不要 */
        break;
    }

    return set_tohan_kind;

} /* end of MDSI_err_detour */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_err_log_set                               */
/*  CALLING SEQ.    : short MDSI_err_log_set ( c201_def *rcvdata,           */
/*                                             char err_kind,               */
/*                                             char *err_code)              */
/*  ARGUMENT        : 1. rcvdata    (I) ログ出力対象電文                    */
/*                  : 2. err_kind   (I) ログ設定用エラー種別                */
/*                  : 3. err_code   (I) ログ設定用エラー番号                */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : エラーログ作成出力処理                                */
/****************************************************************************/
short MDSI_err_log_set ( c201_def *rcvdata,
                         char err_kind,
                         char *err_code)
{
    short s_ret=0;

    db_glelg_def st_log_tbl;         /* ログ出力テーブル */

    short sh_nw_info_cnt    = 0;
    short sh_nw_search_flg  = MDSI_OFF;

    unsigned int s_data_all_leng   = 0;
    short        loop_cnt          = 0;
    char         data_kind_set_flg = 0;
    char         ch_leng_set[6];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* ログ出力要求テーブル初期化 */
    memset( &st_log_tbl, 0, sizeof(st_log_tbl) );

    /* エラーログ プライマリーキー設定 */
    memcpy( &st_log_tbl.pri_key.part_id[0],
            &rcvdata->text_recv_notify.recv_timestamp.time_stamp[18],
            sizeof(st_log_tbl.pri_key.part_id) );

    /* エラーログ プライマリーキー タイムスタンプ設定 */
    memcpy( &st_log_tbl.pri_key.time_stamp[0],
            &rcvdata->text_recv_notify.recv_timestamp.time_stamp[0],
            sizeof(st_log_tbl.pri_key.time_stamp) );

    /* エラーログ プライマリーキー タイムスタンプ枝番設定 */
    memcpy( &st_log_tbl.pri_key.time_stamp_branch[0],
            &rcvdata->text_recv_notify.recv_timestamp.ts_unique_data[0],
            sizeof(st_log_tbl.pri_key.time_stamp_branch) );

    /* エラーログ エラー電文識別 */
    st_log_tbl.err_denbun_id = err_kind;

    /* MTI設定 */
    /* MTIあり電文判定 */
    if (( 0 != g_mti_data[0] ) &&
        ( 0 != memcmp(g_mti_data, "YYY", 3)) &&
        ( 0 != memcmp(g_mti_data, "ZZZZ", 4))){
        /* MTIあり電文 */
        /* IPC MTI設定 */
        memcpy( st_log_tbl.mti_id,
                g_mti_data,
                sizeof(st_log_tbl.mti_id) );
        memcpy( st_log_tbl.denbun_send_recv_info.mti_id,
                g_mti_data,
                sizeof(st_log_tbl.denbun_send_recv_info.mti_id) );
    }
    else {
        /* MTIなし電文 */
        /* IPC MTIにスペース設定 */
        memset( st_log_tbl.mti_id,
                MDSI_SPACE,
                sizeof(st_log_tbl.mti_id) );
        memset( st_log_tbl.denbun_send_recv_info.mti_id,
                MDSI_SPACE,
                sizeof(st_log_tbl.denbun_send_recv_info.mti_id) );
    }

    /* エラーログ レスポンスコード スペース設定 */
    memset( st_log_tbl.res_code, ' ', 
            sizeof(st_log_tbl.res_code) );

    /* エラーログ 内部エラーコード 設定 */
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
            &rcvdata->text_recv_notify.recv_timestamp,
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_recv_time) );

    /* エラーログ 電文送受信情報 受信時局状態 */
    /* 局状態を設定 */
    st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = g_station_sts;

    /* NW情報検索 */
    for ( sh_nw_info_cnt=0; sh_nw_info_cnt<g_nw_info_cnt; sh_nw_info_cnt++){
        if ( 0 == memcmp(g_nw_info[sh_nw_info_cnt].interface_id,
                         rcvdata->text_recv_notify.recv_con_id.interface_name,
                         sizeof(g_nw_info[sh_nw_info_cnt].interface_id))){
            /* NW情報あり */
            sh_nw_search_flg = MDSI_ON;
            break;
        }
    }
    if ( sh_nw_search_flg != MDSI_ON ){
        /* インタフェース識別なし */
        /* 異常終了 */
        return MDSI_RET_NG;
    }

    /* エラーログ 電文送受信情報 NW区分設定 */
    memcpy( st_log_tbl.denbun_send_recv_info.nw_kubun,
            g_nw_info[0].nw_segment,
            sizeof(st_log_tbl.denbun_send_recv_info.nw_kubun) );

    /* エラーログ 電文送受信情報 送信電文種別 */
    memset(g_rcv_log_msg_kind, ' ', 2);

    /* 送信電文種別未設定時スペース設定 */
    st_log_tbl.denbun_send_recv_info.send_denbun_shubetu = MDSI_SPACE;

    switch ( g_mti_kind ) {
    case MDSI_MTI_JUDGE_MTI:
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
                break;
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 st_log_tbl.denbun_send_recv_info.mti_id,
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_tbl.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_QUE:
        /* リジェクトを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_RJ, sizeof(g_rcv_log_msg_kind));
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 "YYY1",
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_tbl.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_HB:
        /* ハートビートを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_HB, sizeof(g_rcv_log_msg_kind));
        /* 受信電分振分先設定テーブル検索 */
        for ( loop_cnt=0; loop_cnt<MDSI_DETOUR_TBL_MAX; loop_cnt++ ){
            if ( 0 == g_detour_tbl[loop_cnt].t_primary_key.mti[0] ){
                /* 受信電分振分先設定テーブルに対象MTIなし */
                memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
            }
            if ( 0 == memcmp( g_detour_tbl[loop_cnt].t_primary_key.mti,
                 "YYY2",
                 sizeof(g_detour_tbl[loop_cnt].t_primary_key.mti) ) ){
                /* 対象MTIテーブルあり */
                /* 受信電分振分先設定テーブル業務/制御設定 */
                st_log_tbl.denbun_send_recv_info.send_denbun_shubetu
                    = g_detour_tbl[loop_cnt].t_site_ctl.denbun_shubetu;
                /* 設定済フラグON */
                data_kind_set_flg = MDSI_ON;
                break;
            }
        }  /* end of while */
        /* 設定済フラグ確認 */
        if (MDSI_ON != data_kind_set_flg ){
            /* 受信電分振分先設定テーブルに対象MTIなし */
            memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        }
        break;
    case MDSI_MTI_JUDGE_IDL:
        /* アイドルを設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_IDL, sizeof(g_rcv_log_msg_kind));
        break;
     case MDSI_MTI_JUDGE_ERR:
        /* MTI異常を設定 */
        memcpy( g_rcv_log_msg_kind, MDSI_LOG_ERR, sizeof(g_rcv_log_msg_kind));
        break;
     default:
        break;
    }

    /* エラーログ 電文送受信情報 電文ログKEY GFP内部LCN */
    memcpy( st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.lcn_id) );

    /* エラーログ 電文送受信情報 電文ログKEY 電文形態 */
    st_log_tbl.denbun_send_recv_info.denbun_log_key.tran_id.denbun_keitai = MDSI_SPACE;

    /* エラーログ 電文送受信情報 電文ログKEY 電文種別 */
    st_log_tbl.denbun_send_recv_info.denbun_log_key.denbun_shubetu = MDSI_SPACE;

    /* エラーログ 電文送受信情報 電文ログKEY 再送回数 */
    memset( st_log_tbl.denbun_send_recv_info.denbun_log_key.re_send_num,
            '0',
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key.re_send_num) );

    /* エラーログ 電文送受信情報 電文フォーマット区分 */
    st_log_tbl.denbun_send_recv_info.denbun_fmt_kubun = '0';

    /* エラーログ 電文送受信情報 通信ログ保存ファイル名 */
    memset( st_log_tbl.denbun_send_recv_info.tushin_log_save_filename,
            MDSI_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_save_filename) );

    /* エラーログ 電文送受信情報 通信ログKEY */
    memcpy( &st_log_tbl.denbun_send_recv_info.tushin_log_key.part_id[0],
            &st_log_tbl.pri_key.part_id[0],
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_key) );

    /* エラーログ 通信制御情報 インタフェース */
    memcpy( st_log_tbl.tushin_cntrl_info.if_id,
            g_nw_info[sh_nw_info_cnt].interface_name,
            sizeof(st_log_tbl.tushin_cntrl_info.if_id) );

    /* エラーログ 通信制御情報 ステーション */
    memcpy( st_log_tbl.tushin_cntrl_info.station_id,
            g_nw_info[sh_nw_info_cnt].station_name,
            sizeof(st_log_tbl.tushin_cntrl_info.station_id) );

    /* IPC 通信制御情報 受信コネクション論理ID */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id,
            &rcvdata->text_recv_notify.recv_con_id,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id) );

    /* IPC 通信制御情報 受信コネクション情報 */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_info,
            &rcvdata->text_recv_notify.recv_con_info,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info.recv_connect_info) );

    /* IPC 通信制御情報 電文受信タイムスタンプ */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp,
            &rcvdata->text_recv_notify.recv_timestamp,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp) );

    /* エラーログ 通信制御情報 回線情報 */
    memcpy( &st_log_tbl.tushin_cntrl_info.line_info,
            &rcvdata->text_recv_notify,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info) );

    /* ダミー領域クリア */
    memset( st_log_tbl.future_use, MDSI_SPACE, sizeof(st_log_tbl.future_use));
    memset( st_log_tbl.denbun_send_recv_info.future_use,
            MDSI_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.future_use));
    memset( st_log_tbl.tushin_cntrl_info.future_use,
            MDSI_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.future_use));

    /* エラーログ 電文長設定 */
    memset( ch_leng_set, 0, sizeof(ch_leng_set));
    snprintf( ch_leng_set,
              6,
             "%05d",
             rcvdata->msg_info.msg_len );
    memcpy( st_log_tbl.denbun_area.denbun_len, ch_leng_set, 5);

    /* エラーログ MTI開始位置設定 */
    /* データ長削除済みフラグ確認 */
    memset( ch_leng_set, 0, sizeof(ch_leng_set));
    if ( MDSI_ON == g_myinfo_def.data_len_del_flag ){
        /* IPC MTI開始位置設定 */
        snprintf( ch_leng_set,
                  6,
                 "%05d",
                 g_denbun_lct_info[g_denbun_lct_info_no].data_len_start_lct );
        memcpy( st_log_tbl.denbun_area.mti_start_lct, ch_leng_set, 5);
    }
    else {
        /* IPC MTI開始位置設定 */
        snprintf( ch_leng_set,
                  6,
                 "%05d",
                 g_denbun_lct_info[g_denbun_lct_info_no].denbun_start_lct );
        memcpy( st_log_tbl.denbun_area.mti_start_lct, ch_leng_set, 5);
    }

    /* エラーログ 送受信電文設定 */
    memcpy( st_log_tbl.denbun_area.denbun,
            rcvdata->msg_info.msg_data,
            rcvdata->msg_info.msg_len);

    /* エラーログ 送受信電文全体長設定 */
    s_data_all_leng = 
         rcvdata->msg_info.msg_len +
         db_glelg_def_Size -
         sizeof(st_log_tbl.denbun_area.denbun);

    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &t_COM_ERL_arg_3_def, MDSI_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = MDSI_ERR_LOG_WRITE;
    t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo_def.config_data.send_timer;
    t_COM_ERL_arg_1_def.data_len     = (short)s_data_all_leng;

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
           &rcvdata->text_recv_notify.recv_con_id,
           sizeof(rcvdata->text_recv_notify.recv_con_id));

    /* エラーログ共通処理実行 */
    s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                     &g_COM_ERL_arg_2_def,
                     &g_cg010in_modle,
                     &t_COM_ERL_arg_3_def,
                     g_myinfo_def.proc_data_sub.module_id);

    /* エラーログ結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_FILE_IO_ERR,
                            "@X@E", "COM_ERL", s_ret);
        return MDSI_RET_NG;
    }

    return MDSI_RET_OK;

} /* end of MDSI_err_log_set */

/***********************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_file_up                                          */
/*  CALLING SEQ.    : short MDSI_file_up( void )                                   */
/*  ARGUMENT        : void                                                         */
/*  RETURN CODE     : 0:正常 -1:異常                                               */
/*  DESCRIPTION     : 東阪振分比率設定ファイル更新                                 */
/***********************************************************************************/
short MDSI_file_up (void)
{
    short s_ret = 0;

    /*  東阪振分比率設定テーブル作成処理 */
    s_ret = MDSI_detourlist_make();

    /* 処理結果判定 */
    if ( MDSI_RET_OK != s_ret ){
        /* 異常終了 */
        memcpy(g_internal_error_code, DEF_NERR_FILE_IO_ERR, sizeof(g_internal_error_code));
        return MDSI_RET_NG;
    }

    /* 東阪振分比率変更 */
    MDSI_message_output(DEF_EVT_FURIWAKE_RATE_UPDT,
                        DEF_NERR_NOMAL,
                        "@L@2@2", "",
                        g_tohan_rate.tokyo_rate,
                        g_tohan_rate.osaka_rate);

    return MDSI_RET_OK;

} /* end of MDSI_file_up */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_sys_open                                  */
/*  CALLING SEQ.    : void MDSI_sys_open ( void )                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : OPENメッセージ処理                                    */
/****************************************************************************/
void MDSI_sys_open (void)
{
    short   s_rcv_err = 0;
    int     i_CC = 0;
    short   s_ret = 0;

    char    wk_pname[ZSYS_VAL_LEN_PROCESSNAME+1];
    short   wk_pname_len;
    short   wk_openid;
    short   wk_reply_cd;
    zsys_ddl_smsg_open_reply_def *rep_msg;
    zsys_ddl_smsg_open_def *sys_msg_p;
    
    memset( g_reply_info, 0, sizeof(g_reply_info));

    memset(wk_pname, 0x00, sizeof(wk_pname));
    wk_pname_len = 0;
    wk_openid    = 0;
    wk_reply_cd  = 0;

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
        wk_openid = MDSI_OPENID_ANCESTOR;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else if (memcmp(sys_msg_p->u_z_data.z_qualifier,"#GFPCI",6) == 0) {
        wk_openid = MDSI_OPENID_ROUT;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else if (memcmp(wk_pname, "$ZL", 3) == 0) {
        wk_openid = MDSI_OPENID_ROUT;
        wk_reply_cd = 0;
        g_myinfo_def.proc_data_sub.open_num++;
    } else {
        wk_reply_cd = 48;
    }

    /* オープナープロセス管理モジュール */
    s_ret = COM_STP_JUDGE(&g_openersinfo, g_recv_buf);
    
    if(s_ret < 0){
        /* EMS */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_NOMAL,
                            "@X@E", "COM_STP_JUDGE OPEN", s_ret);
    }
    
    /* リプライメッセージ */
    memset(g_resp_buf, 0, sizeof(g_resp_buf));

    rep_msg = (zsys_ddl_smsg_open_reply_def *)g_resp_buf;
    rep_msg->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
    rep_msg->z_openid = wk_openid;

    /* リプライ処理 */
    i_CC = REPLYX(g_resp_buf,
                  zsys_ddl_smsg_open_reply_def_Size,
                  /* count-written */,
                  ,
                  wk_reply_cd);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(g_myinfo_def.recv_fno, &s_rcv_err);
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "SYS OPEN REPLY ERR",
                            g_resp_buf);
    }

} /* end of MDSI_sys_open */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_sys_close                                 */
/*  CALLING SEQ.    : void MDSI_sys_close ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : CLOSEメッセージ処理                                   */
/****************************************************************************/
void MDSI_sys_close (void)
{
    short   s_rcv_err = 0;
    int     i_CC      = 0;
    short   s_ret     = 0;

    /* オープナープロセス管理モジュール */
    s_ret = COM_STP_JUDGE(&g_openersinfo, g_recv_buf);
    
    if(s_ret < 0){
        /* EMS */
        MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_NERR_NOMAL,
                            "@X@E", "COM_STP_JUDGE CLOSE", s_ret);
        g_myinfo_def.end_flag = MDSI_ON;
    }
    /* クローズ判定 0：停止不要 1:停止 */
    if (s_ret == 1){
        g_myinfo_def.end_flag = MDSI_NOMAL_END;
    }

    /* リプライ処理 */
    i_CC = REPLYX(g_recv_buf,,, , 0 /* ZFIL_ERR_OK */);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(g_myinfo_def.recv_fno, &s_rcv_err);
        MDSI_message_output(DEF_EVT_RSP_ERR,DEF_NERR_SEND_ERR , "@L@X@X@i",
                            g_lcn, g_myinfo_def.config_data.serverclass_name,
                            "SYS CLOSE REPLY ERR",
                            g_recv_buf);
    }

} /* end of MDSI_sys_close */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_end                                       */
/*  CALLING SEQ.    : void MDSI_end ( void )                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void MDSI_end (void)
{
    short s_ret = 0;
    char ch_mdsi_sub_prog_sts[2];
    lk_zac2001t_arg_1_def Trace_off;

    /* IOモジュールパラメータ*/
    COM_IOM_arg_3_def mdsi_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def mdsi_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def mdsi_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def mdsi_COM_IOM_arg_6_def;
    char ch_sub_prog_sts[2];

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* 局状態管理ファイルクローズ */
    if (g_file_data.station_sts_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdsi_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_3_def) );
        memset( &mdsi_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_4_def) );
        memset( &mdsi_COM_IOM_arg_5_def, 0, sizeof(mdsi_COM_IOM_arg_5_def) );
        memset( &mdsi_COM_IOM_arg_6_def, 0, sizeof(mdsi_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        /* 局状態管理ファイルクローズ */
        memcpy(mdsi_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdsi_COM_IOM_arg_3_def.prog_id));
        memcpy(mdsi_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(mdsi_COM_IOM_arg_3_def.file_name,
            g_file_data.station_sts_file_name, sizeof(mdsi_COM_IOM_arg_3_def.file_name));
        memcpy(mdsi_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_CLOSE,
                                    sizeof(mdsi_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdsi_COM_IOM_arg_4_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
        memcpy(mdsi_COM_IOM_arg_4_def.file_name,
            g_file_data.station_sts_file_name, sizeof(mdsi_COM_IOM_arg_4_def.file_name));
        mdsi_COM_IOM_arg_4_def.file_no = g_file_data.station_sts_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_sub_prog_sts,
            &mdsi_COM_IOM_arg_3_def,
            &mdsi_COM_IOM_arg_4_def,
            &mdsi_COM_IOM_arg_5_def,
            &mdsi_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_CEN_STS,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdsi_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 東阪振分比率設定ファイルクローズ */
    if (g_file_data.tohan_rate_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdsi_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_3_def) );
        memset( &mdsi_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_4_def) );
        memset( &mdsi_COM_IOM_arg_5_def, 0, sizeof(mdsi_COM_IOM_arg_5_def) );
        memset( &mdsi_COM_IOM_arg_6_def, 0, sizeof(mdsi_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdsi_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdsi_COM_IOM_arg_3_def.prog_id));
        memcpy(mdsi_COM_IOM_arg_3_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
        memcpy(mdsi_COM_IOM_arg_3_def.file_name,
            g_file_data.tohan_rate_file_name, sizeof(mdsi_COM_IOM_arg_3_def.file_name));
        memcpy(mdsi_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_CLOSE,
                                    sizeof(mdsi_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdsi_COM_IOM_arg_4_def.file_id, DEF_GFNSW, strlen(DEF_GFNSW));
        memcpy(mdsi_COM_IOM_arg_4_def.file_name,
            g_file_data.tohan_rate_file_name, sizeof(mdsi_COM_IOM_arg_4_def.file_name));
        mdsi_COM_IOM_arg_4_def.file_no = g_file_data.tohan_rate_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdsi_sub_prog_sts,
            &mdsi_COM_IOM_arg_3_def,
            &mdsi_COM_IOM_arg_4_def,
            &mdsi_COM_IOM_arg_5_def,
            &mdsi_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_FURI_RATE,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdsi_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 受信電分振分先設定ファイルクローズ */
    if (g_file_data.assign_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdsi_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_3_def) );
        memset( &mdsi_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_4_def) );
        memset( &mdsi_COM_IOM_arg_5_def, 0, sizeof(mdsi_COM_IOM_arg_5_def) );
        memset( &mdsi_COM_IOM_arg_6_def, 0, sizeof(mdsi_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdsi_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdsi_COM_IOM_arg_3_def.prog_id));
        memcpy(mdsi_COM_IOM_arg_3_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
        memcpy(mdsi_COM_IOM_arg_3_def.file_name,
            g_file_data.assign_file_name, sizeof(mdsi_COM_IOM_arg_3_def.file_name));
        memcpy(mdsi_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_CLOSE,
                                    sizeof(mdsi_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdsi_COM_IOM_arg_4_def.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
        memcpy(mdsi_COM_IOM_arg_4_def.file_name,
            g_file_data.assign_file_name, sizeof(mdsi_COM_IOM_arg_4_def.file_name));
        mdsi_COM_IOM_arg_4_def.file_no = g_file_data.assign_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdsi_sub_prog_sts,
            &mdsi_COM_IOM_arg_3_def,
            &mdsi_COM_IOM_arg_4_def,
            &mdsi_COM_IOM_arg_5_def,
            &mdsi_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_RCV_DEN_FURI,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdsi_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* NW情報ファイルクローズ */
    if (g_file_data.nw_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdsi_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_3_def) );
        memset( &mdsi_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_4_def) );
        memset( &mdsi_COM_IOM_arg_5_def, 0, sizeof(mdsi_COM_IOM_arg_5_def) );
        memset( &mdsi_COM_IOM_arg_6_def, 0, sizeof(mdsi_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdsi_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdsi_COM_IOM_arg_3_def.prog_id));
        memcpy(mdsi_COM_IOM_arg_3_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(mdsi_COM_IOM_arg_3_def.file_name,
            g_file_data.nw_file_name, sizeof(mdsi_COM_IOM_arg_3_def.file_name));
        memcpy(mdsi_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_CLOSE,
                                    sizeof(mdsi_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdsi_COM_IOM_arg_4_def.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
        memcpy(mdsi_COM_IOM_arg_4_def.file_name,
            g_file_data.nw_file_name, sizeof(mdsi_COM_IOM_arg_4_def.file_name));
        mdsi_COM_IOM_arg_4_def.file_no = g_file_data.nw_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdsi_sub_prog_sts,
            &mdsi_COM_IOM_arg_3_def,
            &mdsi_COM_IOM_arg_4_def,
            &mdsi_COM_IOM_arg_5_def,
            &mdsi_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_NW_INFO,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdsi_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    /* 物理名情報ファイルクローズ */
    if (g_file_data.phy_file_name[0] != 0 ){
        /* IOモジュールパラメータ初期化 */
        memset( &mdsi_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_3_def) );
        memset( &mdsi_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(mdsi_COM_IOM_arg_4_def) );
        memset( &mdsi_COM_IOM_arg_5_def, 0, sizeof(mdsi_COM_IOM_arg_5_def) );
        memset( &mdsi_COM_IOM_arg_6_def, 0, sizeof(mdsi_COM_IOM_arg_6_def) );
        memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

        memcpy(mdsi_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                                   sizeof(mdsi_COM_IOM_arg_3_def.prog_id));
        memcpy(mdsi_COM_IOM_arg_3_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(mdsi_COM_IOM_arg_3_def.file_name,
            g_file_data.phy_file_name, sizeof(mdsi_COM_IOM_arg_3_def.file_name));
        memcpy(mdsi_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_CLOSE,
                                    sizeof(mdsi_COM_IOM_arg_3_def.file_io_type));
        memcpy(mdsi_COM_IOM_arg_4_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
        memcpy(mdsi_COM_IOM_arg_4_def.file_name,
            g_file_data.phy_file_name, sizeof(mdsi_COM_IOM_arg_4_def.file_name));
        mdsi_COM_IOM_arg_4_def.file_no = g_file_data.phy_file_no;

        /* IOモジュール */
        s_ret = COM_IOM (
            DEF_COM_IOM_FUNC_CLOSE,
            ch_mdsi_sub_prog_sts,
            &mdsi_COM_IOM_arg_3_def,
            &mdsi_COM_IOM_arg_4_def,
            &mdsi_COM_IOM_arg_5_def,
            &mdsi_COM_IOM_arg_6_def);

        if ( MDSI_RET_OK != s_ret ){
            /* エラー時も処理継続 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
                "@L@C@f@X@K@E","","",
                DEF_FL_PHSIC_INFO,
                DEF_COM_IOM_FUNC_CLOSE,
                "",
                mdsi_COM_IOM_arg_6_def.guardian_errcode);
        }
    }

    if ( g_err_log.file_id != -1){
        /* エラーログクローズ */
        /* エラーログ用パラメータ初期化 */
        memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
        memset( &t_COM_ERL_arg_3_def, MDSI_SPACE, sizeof(t_COM_ERL_arg_3_def) );

        /* エラーログ用情報設定 */
        t_COM_ERL_arg_1_def.file_io_type = MDSI_ERR_LOG_CLOSE;
        t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo_def.config_data.send_timer;
        t_COM_ERL_arg_1_def.data_len     = 0;

        /* エラーログ共通処理実行 */
        s_ret = COM_ERL( &t_COM_ERL_arg_1_def,
                         &g_COM_ERL_arg_2_def,
                         &g_cg010in_modle,
                         &t_COM_ERL_arg_3_def,
                         g_myinfo_def.proc_data_sub.module_id);

        /* エラーログ結果判定 */
        if ( MDSI_RET_OK != s_ret ){
            /* 異常終了 */
            /* EMS出力 */
            MDSI_message_output(DEF_EVT_COMMON_MOD_ERR,
                                DEF_NERR_FILE_IO_ERR,
                                "@X@E", "COM_ERL CLOSE", s_ret);
        }
    }

    /* トレース終了 */
    Trace_off.func_flg = '2';    /*トレース終了処理*/
    TRACEOUT((char *)&Trace_off);

    if ( g_myinfo_def.end_flag == MDSI_NOMAL_END ){
        /* メッセージ出力 */
        MDSI_message_output(DEF_EVT_PROC_NORMAL_END, DEF_NERR_NOMAL, "@R",
                  g_myinfo_def.proc_data.my_pname);
        PROCESS_STOP_(,,MDSI_NORMAL_TERMINATION);
    }
    else {
        /* メッセージ出力 */
        MDSI_message_output(DEF_EVT_PROC_ABNORMAL_END, DEF_NERR_NOMAL, "@R",
                  g_myinfo_def.proc_data.my_pname);
        PROCESS_STOP_(,,MDSI_ABNORMAL_TERMINATION);
    }

} /* end of MDSI_end */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_message_output                            */
/*  CALLING SEQ.    : void MDSI_message_output ( short   s_event_code,      */
/*                                               char    *pch_inter_code,   */
/*                                               char    *pch_format, ...)  */
/*  ARGUMENT        : 1. s_event_code     (I) EMSイベントコード             */
/*                  : 2. pch_inter_code   (I) 内部エラーコード              */
/*                  : 3. pch_format       (I) EMS設定データ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ出力処理                                    */
/****************************************************************************/
void MDSI_message_output(
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

    memset((char *)&g_cg010in.emsinf.emsgkinf, MDSI_SPACE, sizeof(g_cg010in.emsinf.emsgkinf));

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
    memset( g_cg010in.emsinf.emsgkinf.s_nw_kbn,  MDSI_SPACE,
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
//    memset(ch_text, 0x00, sizeof(ch_text));
//    sprintf(ch_text, "%05d", s_event_code);
//    memcpy(g_cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
//        ch_text, strlen(ch_text));
//    s_param_cnt++;

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
    if ((g_cg010in.emsinf.rcd != MDSI_CG010_NOR) &&
        (g_cg010in.emsinf.rcd != MDSI_CG010_WARN)) {
    }
} /* end of MDSI_message_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_module_ems_make                           */
/*  CALLING SEQ.    : void MDSI_module_ems_make  ( oggz1in_def* t_cg010in)  */
/*  ARGUMENT        : 1. t_cg010in        (I) EMSデータ設定領域             */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ作成処理                                    */
/****************************************************************************/
void MDSI_module_ems_make( oggz1in_def *t_cg010in )
{
    memcpy( t_cg010in, &g_cg010in, sizeof(oggz1in_def));

    /* サブルーチンリプライコードにスペース設定 */
    t_cg010in->subrcd = MDSI_SPACE;

    /* リターンコードにスペース設定 */
    t_cg010in->emsinf.rcd = MDSI_SPACE;

    /* メッセージIDにスペース設定 */
    memset(t_cg010in->emsinf.msgid, MDSI_SPACE, sizeof(t_cg010in->emsinf.msgid));

    /* EMS出力情報クリア */
    memset((char *)&t_cg010in->emsinf.emsgkinf, MDSI_SPACE, sizeof(t_cg010in->emsinf.emsgkinf));

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
    memset( t_cg010in->emsinf.emsgkinf.s_nw_kbn,  MDSI_SPACE,
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
    memset((char *)&t_cg010in->emsinf.emsnninf, MDSI_SPACE, sizeof(t_cg010in->emsinf.emsnninf));

} /* end of MDSI_module_ems_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_station_sts_read                          */
/*  CALLING SEQ.    : void MDSI_station_sts_read ( c201_def*, char* )       */
/*  ARGUMENT        : 1. rcvdata       (I) 受信電文                         */
/*                  : 2. station_sts   (O) 局状態                           */
/*  RETURN CODE     : 0:正常終了 1:異常終了                                 */
/*  DESCRIPTION     : 局状態取得処理                                        */
/****************************************************************************/
short MDSI_station_sts_read ( c201_def *rcvdata,
                              char *station_sts )
{
    short s_ret                = 0;
    char  ch_sub_prog_sts[2];
    short s_station_st_tbl_cnt = 0;
    char  ch_st_get_flag       = MDSI_OFF;


    db_gcsst_def *station_st_tbl_local;

    /* 局状態管理ファイル用IOモジュールパラメータ*/
    COM_IOM_arg_3_def station_COM_IOM_arg_3_def;
    COM_IOM_arg_4_def station_COM_IOM_arg_4_def;
    COM_IOM_arg_5_def station_COM_IOM_arg_5_def;
    COM_IOM_arg_6_def station_COM_IOM_arg_6_def;

    /* 局状態管理ファイル プライマリーキー ポインタ*/
    db_gcsst_def *gcsst_p_key;

    /* IOモジュールパラメータ初期化 */
    memset( &station_COM_IOM_arg_3_def, MDSI_SPACE, sizeof(station_COM_IOM_arg_3_def) );
    memset( &station_COM_IOM_arg_4_def, MDSI_SPACE, sizeof(station_COM_IOM_arg_4_def) );
    memset( &station_COM_IOM_arg_5_def, 0, sizeof(station_COM_IOM_arg_5_def) );
    memset( &station_COM_IOM_arg_6_def, 0, sizeof(station_COM_IOM_arg_6_def) );
    memset( ch_sub_prog_sts, 0, sizeof(ch_sub_prog_sts) );

    /* 局状態管理ファイル読込み開始処理 */
    memcpy(station_COM_IOM_arg_3_def.prog_id, g_myinfo_def.proc_data_sub.module_id,
                               sizeof(station_COM_IOM_arg_3_def.prog_id));
    memcpy(station_COM_IOM_arg_3_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(station_COM_IOM_arg_3_def.file_name,
        g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_3_def.file_name));
    memcpy(station_COM_IOM_arg_3_def.file_io_type, MDSI_FILEIO_TYPE_START,
                                sizeof(station_COM_IOM_arg_3_def.file_io_type));
    memcpy(station_COM_IOM_arg_4_def.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(station_COM_IOM_arg_4_def.file_name,
        g_file_data.station_sts_file_name, sizeof(station_COM_IOM_arg_4_def.file_name));
    station_COM_IOM_arg_4_def.file_no = g_file_data.station_sts_file_no;
    station_COM_IOM_arg_5_def.part_key_type    = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(station_COM_IOM_arg_5_def.key_type, DEF_COM_IOM_KEYTYPE_PRI,
                                      sizeof(station_COM_IOM_arg_5_def.key_type));
    gcsst_p_key = (db_gcsst_def *)&station_COM_IOM_arg_5_def.key_value;
    memset((char *)&gcsst_p_key->pri_key, MDSI_SPACE, sizeof(gcsst_p_key->pri_key));
    gcsst_p_key->pri_key.site_id = g_myinfo_def.config_data.site_id;
    gcsst_p_key->pri_key.nw_id = g_myinfo_def.config_data.network_id;
    memcpy(gcsst_p_key->pri_key.grp_id, g_myinfo_def.config_data.group_id,
                                       sizeof(gcsst_p_key->pri_key.grp_id));
    memcpy(gcsst_p_key->pri_key.if_id,
           rcvdata->text_recv_notify.recv_con_id.interface_name,
           sizeof(gcsst_p_key->pri_key.if_id));
    if (( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_ST ) ||
        ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_CO )) { 
        memcpy((char *)&gcsst_p_key->pri_key.station_id,
               rcvdata->text_recv_notify.recv_con_id.station_name,
               sizeof(gcsst_p_key->pri_key.station_id));
    }
    if ( g_station_st_unit == DEF_OPN_CLS_MNG_LYR_CO ) { 
        memcpy(gcsst_p_key->pri_key.connect_id,
               rcvdata->text_recv_notify.recv_con_id.connection_name,
               sizeof(gcsst_p_key->pri_key.connect_id));
    }
    station_COM_IOM_arg_5_def.key_len = sizeof(gcsst_p_key->pri_key);
    station_COM_IOM_arg_5_def.positioning_mode = DEF_COM_IOM_EXACT;

    station_COM_IOM_arg_5_def.lock_flg         = DEF_COM_IOM_NOLOCK;
    station_COM_IOM_arg_5_def.asc_desc_type    = DEF_COM_IOM_ASCEND;
    station_COM_IOM_arg_5_def.io_timer         = g_myinfo_def.config_data.io_timer;
    station_COM_IOM_arg_5_def.rec_len          = sizeof(db_gcsst_def);

    s_ret = COM_IOM (
        DEF_COM_IOM_FUNC_STARTREAD,
        ch_sub_prog_sts,
        &station_COM_IOM_arg_3_def,
        &station_COM_IOM_arg_4_def,
        &station_COM_IOM_arg_5_def,
        &station_COM_IOM_arg_6_def);

    if (( s_ret != MDSI_RET_OK ) ||
        ( memcmp(ch_sub_prog_sts, MDSI_IO_NORMAL_END, sizeof(ch_sub_prog_sts)) != 0)) {
        /* 異常終了 */
        /* EMS出力 */
        MDSI_message_output(DEF_EVT_FILE_IO_ERR, DEF_NERR_FILE_IO_ERR,
            "@L@C@f@X@K@E","","",
            DEF_FL_CEN_STS,
            DEF_COM_IOM_FUNC_STARTREAD,
            station_COM_IOM_arg_5_def.key_value,
            station_COM_IOM_arg_6_def.guardian_errcode);
        return MDSI_RET_NG;
    }

    station_st_tbl_local = (db_gcsst_def *)&station_COM_IOM_arg_6_def.rec_area;
    memcpy(station_sts, station_st_tbl_local->state_sts_info.state_sts,
                 sizeof(station_st_tbl_local->state_sts_info.state_sts));

    return MDSI_RET_OK;

} /* end of MDSI_station_sts_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  MDSI_BCD2CHAR                                  */
/*  CALLING SEQ.    : void MDSI_BCD2CHAR ( unsigned char *, char*, short)   */
/*  ARGUMENT        : 1. bcd_p       (I) BCDデータ                          */
/*                  : 2. bcd_p       (O) ASCIIデータ                        */
/*  RETURN CODE     : なし                                                  */
/*  DESCRIPTION     : BSD⇒CHAR変換                                         */
/****************************************************************************/
void MDSI_BCD2CHAR(unsigned char *bcd_p, char *ascii_p,short s_len)
{
    short s_count;
    const char ToNUM_tbl[16] = {"0123456789******"};    /* ニューメリック変換テーブル */

    for (s_count = 0; s_count < s_len; s_count++) {
        ascii_p[s_count*2]   = ToNUM_tbl[bcd_p[s_count] >> 4];
        ascii_p[s_count*2+1] = ToNUM_tbl[bcd_p[s_count] & 0x0f];
    }
} /* end of MDSI_BCD2CHAR */
