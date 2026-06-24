/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVXH0                                    */
/*        FUNCTION          ････ 鍵管理ファイルIO                            */
/*                                                                           */
/*        AUTHER            ････ HAS T.Hashimoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2026-04-21                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== =========== ========== ============================================ */
/*  1.0  T.Hashimoto 2026/04/21 (xxxxx)新規作成                              */
/*****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* USER HEADER     */
#include "GFPCVXH0.h"

/********** グローバルデータ ***********/
myinfo_def                      myinfo;                                                 /* 自プロセス情報                           */
procinfo_def                    procinfo;                                               /* プロセス情報                             */
iocomp_def                      iocomp;                                                 /* I/O完了情報                              */
COM_STP_arg_1_def               openersinfo;                                            /* オープナー情報テーブル                   */
char                            recv_buf[32000];                                        /* $RECEIVE I/O用バッファ                   */
char                            resp_buf[sizeof(r301_def)];                             /* 応答メッセージ用バッファ                 */
char                            *sys_msg = recv_buf;                                    /* $RECEIVEバッファアドレス                 */
c801_def                        *req_msg = (c801_def *)recv_buf;                        /* 要求メッセージバッファアドレス           */
r801_def                        *rsp_msg = (r801_def *)resp_buf;                        /* 応答メッセージバッファアドレス           */
short                           req_dat_len;                                            /* 要求電文部データ長                       */
char                            naibu_tranid[41];                                       /* 内部トランザクションID                   */
char                            snd_pname[6+1];                                         /* 送信元プロセス名                         */
char                            sys_datetime[8+1];                                      /* システム日時(共通用)                     */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  main                                           */
/*  CALLING SEQ.    : void main (void)                                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メイン                                                */
/****************************************************************************/
int main (void)
{
    /* 初期化処理 */
    KYFU_init();

    /* 主処理 */
    /* フラグがONになるまでループ  */
    while ( myinfo.end_flg == KYFU_OFF ) {
        /* 主処理 */
        KYFU_main();
    }

    /* 終了処理 */
    KYFU_finish();

} /* end of main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_init                                      */
/*  CALLING SEQ.    : void KYFU_init (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void KYFU_init()
{
    short   wk_error;         /* WKエラーコード    */
    short   wk_result;        /* WK処理結果       */

    /* グローバル情報の初期化(自プロセス情報) */
    myinfo.recv_fno = -1;
    memset(&recv_buf, 0x20, sizeof(recv_buf));
    myinfo.recv_buf = recv_buf;
    memset(&resp_buf, 0x20, sizeof(resp_buf));
    memset(myinfo.pathmon_name, 0x20, sizeof(myinfo.pathmon_name));
    myinfo.end_flg = KYFU_OFF;
    memset(myinfo.trc_time_begin, 0x20, sizeof(myinfo.trc_time_begin));
    memset(myinfo.trc_time_end, 0x20, sizeof(myinfo.trc_time_end));
    myinfo.site_id = ' ';
    myinfo.network_id = ' ';
    memset(myinfo.nw_kubun, 0x20, sizeof(myinfo.nw_kubun));
    memset(myinfo.group_id, 0x20, sizeof(myinfo.group_id));
    memset(myinfo.serverclass_id, 0x20, sizeof(myinfo.serverclass_id));
    memset(myinfo.serverclass_kind, 0x20, sizeof(myinfo.serverclass_kind));
    memset(myinfo.serverclass_no, 0x20, sizeof(myinfo.serverclass_no));
    memset(myinfo.serverclass_mlt_num, 0x20, sizeof(myinfo.serverclass_mlt_num));
    memset(myinfo.gfphi_asn_fname, 0x20, sizeof(myinfo.gfphi_asn_fname));
    memset(myinfo.gfphi_fname, 0x20, sizeof(myinfo.gfphi_fname));
    myinfo.gfphi_fnum = -1;
    memset(myinfo.gckey_fname, 0x20, sizeof(myinfo.gckey_fname));
    myinfo.gckey_fnum = -1;
    myinfo.file_timer = 0L;
    myinfo.send_timer = 0L;
    memset(myinfo.msg_serverclass_name, 0x00, sizeof(myinfo.msg_serverclass_name));
    memset(myinfo.msg_pathmon_name, 0x00, sizeof(myinfo.msg_pathmon_name));
    memset(myinfo.module_id, 0x00, sizeof(myinfo.module_id));
    memset(myinfo.gfp_lcn, 0x20, sizeof(myinfo.gfp_lcn));

    /* グローバル情報の初期化(プロセス情報) */
    memset(&procinfo, 0x20, sizeof(procinfo));

    /* グローバル情報の初期化(オープナー情報テーブル) */
    memset(&openersinfo, 0x20, sizeof(openersinfo));

    /* EMS初期設定 */
    memset((char *)&cg010in, 0x20, sizeof(cg010in));
    cg010in.subrcd = '0';
    cg010in.emsinf.rcd = '0';
    memcpy(cg010in.uytrminf.proctimer, "0002", 4); /* temp */
    memcpy(cg010in.uytrminf.uytrmmonlen, "00", 2); /* temp */
    memcpy(cg010in.uytrminf.uytrmsrvlen, "00", 2); /* temp */
    memcpy(cg010in.emsinf.emsgkinf.prgid, DEF_GFPCVXH0, strlen(DEF_GFPCVXH0));

    /* グローバル情報の初期化(I/O完了情報) */
    iocomp.fno = -1;
    iocomp.addr = 0L;
    iocomp.len = 0;
    iocomp.tag = -1L;
    iocomp.ferr = 0;
    memset((char *)&iocomp.recv_info, 0x00, sizeof(iocomp.recv_info));

    /* グローバル情報の初期化 */
    memset(naibu_tranid, 0x20, sizeof(naibu_tranid));

    /* サーバクラス冗長化番号 */
    memcpy(myinfo.serverclass_mlt_num, "0000", sizeof(myinfo.serverclass_mlt_num));

    /* ローカル情報の初期化 */
    wk_error = KYFU_RET_OK;
    wk_result = KYFU_RET_OK;

    /* オープナープロセス管理モジュール初期処理 */
    COM_STP_INIT(&openersinfo);

    /* プロセス情報取得処理 */
    wk_result = COM_PRC(&procinfo);
    if (wk_result != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_PROC_OPN_ERR, "@X", "COM_PRC");
        /* 異常終了 */
        KYFU_abend();
    }

    /* モジュールID */
    memcpy(myinfo.module_id, DEF_GFPCVXH0, sizeof(DEF_GFPCVXH0));

    /* パラメータ取得処理 */
    KYFU_get_param();

    /* トレース出力初期処理 */
    memset((char *)&g_trc.recv, 0x20, sizeof(g_trc.recv));
    g_trc.recv.func_flg = FUNC_INIT;
    memcpy(g_trc.recv.trace_info.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memset(g_trc.recv.data_info.rec_len, '0', sizeof(char));
    TRACEOUT((char *)&g_trc.recv);

    /* コンフィグ情報取得処理 */
    memcpy(myinfo.gfphi_asn_fname,"GFPHI", 5);

    KYFU_get_config(myinfo.gfphi_asn_fname);

    /* $RECEIVE OPEN */
    wk_error = FILE_OPEN_(
        "$RECEIVE", 8,
        &myinfo.recv_fno,
        ZSYS_VAL_OPENACC_READWRITE,
        ZSYS_VAL_OPENEXCL_SHARED,
        0,  /* NOWAIT-DEPTH */
        2   /* RECEIVE-DEPTH */
        );
    if (wk_error != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", "$RECEIVE", "OPEN", "", wk_error);
        /* 異常終了 */
        KYFU_abend();
    }

    /* メッセージ出力 */
    KYFU_message_output(DEF_EVT_PROC_START, '*', DEF_NERR_NOMAL, "@R", procinfo.my_pname);

} /* end of KYFU_init */


/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_get_param                                 */
/*  CALLING SEQ.    : void KYFU_get_param (void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void KYFU_get_param (void)
{
    char    wk_paramname    [32];
    char    wk_param        [64];
    short   wk_result;
    char    ch_work[10];

    /* サーバクラス論理ID */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;

    memcpy(wk_paramname, DEF_SRV_LOGICAL_ID, sizeof(DEF_SRV_LOGICAL_ID) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 23+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_SRV_LOGICAL_ID, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    if (strlen(wk_param) != 23) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_SRV_LOGICAL_ID, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    /* サイト識別 */
    myinfo.site_id = wk_param[0];
    /* N/W識別 */
    myinfo.network_id = wk_param[2];
    /* グループ識別 */
    memcpy(myinfo.group_id, wk_param+4, 5);
    /* サーバクラス種類 */
    memcpy(myinfo.serverclass_kind, wk_param+10, 8);
    /* サーバクラス論理番号 */
    memcpy(myinfo.serverclass_no, wk_param+19, 4);
    /* サーバクラス論理ID */
    memcpy(&myinfo.serverclass_id[0], &myinfo.serverclass_kind, 8);
    memcpy(&myinfo.serverclass_id[8], &myinfo.serverclass_no,   4);

    /* 運用監視端末出力サーバ.PATHMON名 */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;

    memcpy(wk_paramname, DEF_MSG_MON_NAME, sizeof(DEF_MSG_MON_NAME) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 13+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_MON_NAME, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    if (strlen(wk_param) > 13) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_MSG_MON_NAME, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }
    memcpy(myinfo.msg_pathmon_name, wk_param, sizeof(myinfo.msg_pathmon_name));

    memcpy(cg010in.uytrminf.uytrmmon, wk_param, strlen(wk_param));
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%02d", strlen(wk_param));
    memcpy(cg010in.uytrminf.uytrmmonlen, ch_work, strlen(ch_work));

    /* 運用監視端末出力サーバ.サーバクラス名 */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;

    memcpy(wk_paramname, DEF_MSG_SRV_NAME, sizeof(DEF_MSG_SRV_NAME) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 12+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_SRV_NAME, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    if (strlen(wk_param) > 12) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_MSG_SRV_NAME, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }
    memcpy(myinfo.msg_serverclass_name, wk_param, sizeof(myinfo.msg_serverclass_name));

    memcpy(cg010in.uytrminf.uytrmsrv, wk_param, strlen(wk_param));
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%02d", strlen(wk_param));
    memcpy(cg010in.uytrminf.uytrmsrvlen, ch_work, strlen(ch_work));

    /* ファイルI/Oタイマー(単位:ミリ秒) */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;

    memcpy(wk_paramname, DEF_FILE_IO_TIMER_10MSECOND, sizeof(DEF_FILE_IO_TIMER_10MSECOND) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 8+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    if (strlen(wk_param) > 8) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }
    myinfo.file_timer = atol(wk_param);

    /* PATHSENDタイマー(単位:ミリ秒) */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;

    memcpy(wk_paramname, DEF_PSEND_TIMER_10MSECOND, sizeof(DEF_PSEND_TIMER_10MSECOND) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 8+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_PSEND_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }

    if (strlen(wk_param) > 8) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_PSEND_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        KYFU_abend();
    }
    myinfo.send_timer = atol(wk_param);

    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%04ld", myinfo.send_timer/100);
    memcpy(cg010in.uytrminf.proctimer, ch_work, strlen(ch_work));

} /* end of KYFU_get_param */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_get_config                                */
/*  CALLING SEQ.    : void KYFU_get_config (char)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コンフィグ情報取得処理                                */
/****************************************************************************/
void KYFU_get_config (
    char*    asn_filename
)
{
    /* --- --- --- --- */
    /* ローカル変数    */
    /* --- --- ---  ---*/
    char    wk_filename[48];
    short   wk_filename_len = 0;
    char    wk_gckey_filename[48];
    long    wk_gckey_filename_len = 0;
    char    *wk_gckey_space_posi  =  0;

    memset(wk_filename, 0x20, sizeof(wk_filename));
    memset(wk_gckey_filename, 0x20, sizeof(wk_gckey_filename));

    /* --- --- --- --- --- --- */
    /* プライマリキー用構造体  */
    /* --- --- --- --- --- --- */
    /* 物理名情報ファイル.プライマリキー */
    typedef struct __gfphi_pri_key_def
    {
         char                            site_id;
         char                            nw_id;
         char                            grp_id[5];
         struct
         {
            struct
            {
               char                            srv_cls_kind[8];
               char                            srv_cls_num[4];
            } srv_cls_id;
            char                            srv_cls_mlt_num[4];
         } srv_cls_key;
         struct
         {
            struct
            {
               char                            prc_file_kind[8];
               char                            prc_file_num[4];
            } prc_file_id;
            char                            prc_file_mlt_num[4];
         } prc_file_key;
    } gfphi_pri_key_def;

    /* 鍵管理ファイル.プライマリキー */
    typedef struct __gckey_pri_key_def
    {
        char                            site_id;
        char                            nw_id;
        char                            grp_id[5];
        char                            if_id[5];
        char                            station_id[6];
        char                            key_type[4];
    } gckey_pri_key_def;

    /* --- --- --- --- --- --- --- --- --- */
    /* ローカル変数.物理名情報ファイル用   */
    /* --- --- --- --- --- --- --- --- --- */
    /* 物理名情報ファイル用IOモジュールパラメータ */
    char                gfphi_sub_prog_sts[2];
    COM_IOM_arg_3_def   gfphi_COM_IOM_arg_3;
    COM_IOM_arg_4_def   gfphi_COM_IOM_arg_4;
    COM_IOM_arg_5_def   gfphi_COM_IOM_arg_5;
    COM_IOM_arg_6_def   gfphi_COM_IOM_arg_6;

    memset(&gfphi_sub_prog_sts,     0x20,   sizeof(gfphi_sub_prog_sts));
    memset(&gfphi_COM_IOM_arg_3,    0x20,   sizeof(gfphi_COM_IOM_arg_3));
    memset(&gfphi_COM_IOM_arg_4,    0x20,   sizeof(gfphi_COM_IOM_arg_4));
    memset(&gfphi_COM_IOM_arg_5,    0x20,   sizeof(gfphi_COM_IOM_arg_5));
    memset(&gfphi_COM_IOM_arg_6,    0x20,   sizeof(gfphi_COM_IOM_arg_6));

    /* 物理名情報ファイル用レコード */
    db_gfphi_def        gfphi_rec;
    memset(&gfphi_rec, 0x20, sizeof(gfphi_rec));

    /* 物理名情報ファイル.プライマリキー */
    gfphi_pri_key_def    wk_gfphi_pri_key;
    memset(&wk_gfphi_pri_key, 0x20, sizeof(wk_gfphi_pri_key));

    /* --- --- --- --- --- --- --- --- */
    /* ローカル変数.鍵管理ファイル用   */
    /* --- --- --- --- --- --- --- --- */
    /* 鍵管理ファイル用IOモジュールパラメータ */
    char                gckey_sub_prog_sts[2];
    COM_IOM_arg_3_def   gckey_COM_IOM_arg_3;
    COM_IOM_arg_4_def   gckey_COM_IOM_arg_4;
    COM_IOM_arg_5_def   gckey_COM_IOM_arg_5;
    COM_IOM_arg_6_def   gckey_COM_IOM_arg_6;

    memset(&gckey_sub_prog_sts,     0x20,   sizeof(gckey_sub_prog_sts));
    memset(&gckey_COM_IOM_arg_3,    0x20,   sizeof(gckey_COM_IOM_arg_3));
    memset(&gckey_COM_IOM_arg_4,    0x20,   sizeof(gckey_COM_IOM_arg_4));
    memset(&gckey_COM_IOM_arg_5,    0x20,   sizeof(gckey_COM_IOM_arg_5));
    memset(&gckey_COM_IOM_arg_6,    0x20,   sizeof(gckey_COM_IOM_arg_6));

    /* 鍵管理ファイル用レコード */
    db_gckey_def        gckey_rec;
    memset(&gckey_rec, 0x20, sizeof(gckey_rec));

    /* 鍵管理ファイル.プライマリキー */
    gckey_pri_key_def    wk_gckey_pri_key;
    memset(&wk_gckey_pri_key, 0x20, sizeof(wk_gckey_pri_key));

    /* --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.オープン */
    /* --- --- --- --- --- --- --- */
    /* 物理名情報ファイル物理ファイル名取得(ASSIGN情報) */
    COM_ASN(asn_filename, wk_filename, &wk_filename_len );
    if ( (0 == wk_filename_len) || (48 < wk_filename_len) ){ /* 物理名長異常 */
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_ASN_FILE_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@E", DEF_ASN_GFPHI, 0);
        /* 異常終了 */
        KYFU_abend();
    }

    memcpy(myinfo.gfphi_fname, wk_filename, sizeof(myinfo.gfphi_fname));

    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));

    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(gfphi_COM_IOM_arg_3.prog_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    KYFU_FILEIO_OPEN,   sizeof(KYFU_FILEIO_OPEN) -1);

    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,    0x20,   sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id, DEF_GFPHI, sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name, myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no =  myinfo.gfphi_fnum;

    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,                0x20,   sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type           = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len            = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gfphi_COM_IOM_arg_5.key_value,       0x20,   sizeof(gfphi_COM_IOM_arg_5.key_value));
    memset(gfphi_COM_IOM_arg_5.key_type,        0x20,   sizeof(gfphi_COM_IOM_arg_5.key_type));
    gfphi_COM_IOM_arg_5.key_len                 = 0;
    gfphi_COM_IOM_arg_5.compare_len             = 0;
    gfphi_COM_IOM_arg_5.positioning_mode        = 0;
    gfphi_COM_IOM_arg_5.lock_flg                = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type           = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer                = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len                 = 0;
    memset(gfphi_COM_IOM_arg_5.rec_area,        0x20,   sizeof(gfphi_COM_IOM_arg_5.rec_area));

    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,            0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_6.rec_area));

    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_OPEN,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    /* オープン完了 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", DEF_FL_PHSIC_INFO, "OPEN", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        KYFU_abend();
    }
    /* FILE情報格納 */
    myinfo.gfphi_fnum = gfphi_COM_IOM_arg_4.file_no;

    /* --- --- --- --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.鍵管理ファイル名取得 */
    /* --- --- --- --- --- --- --- --- --- --- */

    /* キー設定 */
    memset(&wk_gfphi_pri_key, 0x20, sizeof(wk_gfphi_pri_key));
    memcpy(&wk_gfphi_pri_key.site_id, &myinfo.site_id, sizeof(wk_gfphi_pri_key.site_id));
    memcpy(&wk_gfphi_pri_key.nw_id, &myinfo.network_id, sizeof(wk_gfphi_pri_key.nw_id));
    memcpy(&wk_gfphi_pri_key.grp_id, &myinfo.group_id, sizeof(wk_gfphi_pri_key.grp_id));
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_NAME_DEFAULT, sizeof(DEF_SC_NAME_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num, DEF_SC_NUM_DEFAULT, sizeof(DEF_SC_NUM_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num, DEF_SC_DUP_DEFAULT, sizeof(DEF_SC_DUP_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_KEY_MG, sizeof(DEF_FL_KEY_MG) -1);
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num, KYFU_NON_MLT_NUM, sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num, KYFU_NON_MLT_NUM, sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));

    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));

    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,    0x20,   sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id, myinfo.module_id,sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id, DEF_GFPHI, sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_3.file_name, myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type, KYFU_FILEIO_READ, sizeof(KYFU_FILEIO_READ) -1);

    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,    0x20,   sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id, DEF_GFPHI, sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name, myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no =  myinfo.gfphi_fnum;

    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,    0x20,   sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len      = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(gfphi_COM_IOM_arg_5.key_value, &wk_gfphi_pri_key, sizeof(wk_gfphi_pri_key));
    memcpy(gfphi_COM_IOM_arg_5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI) -1);
    gfphi_COM_IOM_arg_5.key_len           = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.compare_len       = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    gfphi_COM_IOM_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer          = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len           = db_gfphi_def_Size;
    memset(gfphi_COM_IOM_arg_5.rec_area, 0x20, sizeof(gfphi_COM_IOM_arg_5.rec_area));

    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,    0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode  = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc, 0x20,sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name, 0x20,sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len           = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area, 0x20,sizeof(gfphi_COM_IOM_arg_6.rec_area));

    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_STARTREAD,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );

    /* 読み込み終了 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", DEF_FL_KEY_MG, "READ", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        KYFU_abend();
    }
    /* 正常読み込み.読み込み情報格納 */
    memset(&gfphi_rec, 0x20, sizeof(gfphi_rec));
    memcpy(&gfphi_rec, &gfphi_COM_IOM_arg_6.rec_area, sizeof(gfphi_rec));
    memcpy(&wk_gckey_filename, gfphi_rec.prc_file_info.prc_file_name, sizeof(gfphi_rec.prc_file_info.prc_file_name));
    /* 物理ファイル名チェック */
    wk_gckey_filename_len = sizeof(wk_gckey_filename);
    if ( (0 == wk_gckey_filename_len) || (48 < wk_gckey_filename_len) ){ /* 物理名長異常 */
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X", DEF_FL_PHSIC_INFO, wk_gfphi_pri_key, "GCKEY FILENAME ERR");
        /* 異常終了 */
        KYFU_abend();
    }
    /*鍵管理ファイル.物理ファイル名長 */
    wk_gckey_space_posi = strchr(wk_gckey_filename,' ');
    if (wk_gckey_space_posi == NULL) {
        wk_gckey_filename_len = sizeof(wk_gckey_filename);
    } else {
        wk_gckey_filename_len = wk_gckey_space_posi - wk_gckey_filename;
    }
    /* 鍵管理ファイル.物理ファイル名格納 */
    memcpy(myinfo.gckey_fname, wk_gckey_filename, wk_gckey_filename_len);

    /* --- --- --- --- --- --- --- ---*/
    /* 物理名情報ファイル.クローズ   */
    /* --- --- --- --- --- --- --- ---*/
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));

    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,    0x20,   sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    KYFU_FILEIO_CLOSE,  sizeof(KYFU_FILEIO_CLOSE) -1);

    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,    0x20,   sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,     DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,   myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no             = myinfo.gfphi_fnum;

    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,    0x20,   sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gfphi_COM_IOM_arg_5.key_value,   0x20,   sizeof(gfphi_COM_IOM_arg_5.key_value));
    memset(gfphi_COM_IOM_arg_5.key_type,    0x20,   sizeof(gfphi_COM_IOM_arg_5.key_type));
    gfphi_COM_IOM_arg_5.key_len             = 0;
    gfphi_COM_IOM_arg_5.compare_len         = 0;
    gfphi_COM_IOM_arg_5.positioning_mode    = 0;
    gfphi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_5.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_5.rec_area));

    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,    0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode =  0;
    memset(gfphi_COM_IOM_arg_6.err_proc, 0x20,sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name, 0x20,sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len =  0;
    memset(gfphi_COM_IOM_arg_6.rec_area, 0x20,sizeof(gfphi_COM_IOM_arg_6.rec_area));

    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_CLOSE,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    /* クローズ完了 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", DEF_FL_PHSIC_INFO, "CLOSE", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        KYFU_abend();
    }
    /* FILE情報格納 */
    myinfo.gfphi_fnum = KYFU_FILE_CLOSED;

    /* --- --- --- --- --- --- */
    /* 鍵管理ファイル.オープン */
    /* --- --- --- --- --- --- */
    /* arg2_サブプログラムステータス */
    memset(gckey_sub_prog_sts, 0x20, sizeof(gckey_sub_prog_sts));

    /* arg3_トレース情報 */
    memset(&gckey_COM_IOM_arg_3,    0x20,   sizeof(gckey_COM_IOM_arg_3));
    memcpy(gckey_COM_IOM_arg_3.prog_id, myinfo.module_id,sizeof(gckey_COM_IOM_arg_3.prog_id));
    memcpy(gckey_COM_IOM_arg_3.file_id, DEF_GCKEY, sizeof(DEF_GCKEY) -1);
    memcpy(gckey_COM_IOM_arg_3.file_name, myinfo.gckey_fname, sizeof(myinfo.gckey_fname));
    memcpy(gckey_COM_IOM_arg_3.file_io_type, KYFU_FILEIO_OPEN, sizeof(KYFU_FILEIO_OPEN) -1);

    /* arg4_ファイル情報 */
    memset(&gckey_COM_IOM_arg_4,    0x20,   sizeof(gckey_COM_IOM_arg_4));
    memcpy(gckey_COM_IOM_arg_4.file_id, DEF_GCKEY, sizeof(DEF_GCKEY) -1);
    memcpy(gckey_COM_IOM_arg_4.file_name, myinfo.gckey_fname, sizeof(myinfo.gckey_fname));
    gckey_COM_IOM_arg_4.file_no =  myinfo.gckey_fnum;

    /* arg5_入力情報 */
    memset(&gckey_COM_IOM_arg_5,    0x20,   sizeof(gckey_COM_IOM_arg_5));
    gckey_COM_IOM_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_position = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_len      = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gckey_COM_IOM_arg_5.key_value, 0x20, sizeof(gckey_COM_IOM_arg_5.key_value));
    memset(gckey_COM_IOM_arg_5.key_type, 0x20, sizeof(gckey_COM_IOM_arg_5.key_type));
    gckey_COM_IOM_arg_5.key_len           = 0;
    gckey_COM_IOM_arg_5.compare_len       = 0;
    gckey_COM_IOM_arg_5.positioning_mode  = 0;
    gckey_COM_IOM_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    gckey_COM_IOM_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    gckey_COM_IOM_arg_5.io_timer          = myinfo.file_timer;
    gckey_COM_IOM_arg_5.rec_len           = 0;
    memset(gckey_COM_IOM_arg_5.rec_area, 0x20, sizeof(gckey_COM_IOM_arg_5.rec_area));

    /* arg6_出力情報 */
    memset(&gckey_COM_IOM_arg_6,    0x20,   sizeof(gckey_COM_IOM_arg_6));
    gckey_COM_IOM_arg_6.guardian_errcode  = 0;
    memset(gckey_COM_IOM_arg_6.err_proc, 0x20, sizeof(gckey_COM_IOM_arg_6.err_proc));
    memset(gckey_COM_IOM_arg_6.file_name, 0x20,sizeof(gckey_COM_IOM_arg_6.file_name));
    gckey_COM_IOM_arg_6.rec_len           = 0;
    memset(gckey_COM_IOM_arg_6.rec_area, 0x20,sizeof(gckey_COM_IOM_arg_6.rec_area));

    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_OPEN,
        gckey_sub_prog_sts,
        &gckey_COM_IOM_arg_3,
        &gckey_COM_IOM_arg_4,
        &gckey_COM_IOM_arg_5,
        &gckey_COM_IOM_arg_6
    );
    /* オープン完了 */
    if (memcmp(gckey_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", DEF_FL_PHSIC_INFO, "OPEN", gckey_COM_IOM_arg_5.key_value, gckey_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        KYFU_abend();
    }
    /* FILE情報格納 */
    myinfo.gckey_fnum = gckey_COM_IOM_arg_4.file_no;

} /* end of KYFU_get_config */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_main                                      */
/*  CALLING SEQ.    : void KYFU_main (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void KYFU_main(void)
{
    /* 初期化(I/O完了情報) */
    iocomp.fno = -1;
    iocomp.addr = 0;
    iocomp.len = 0;
    iocomp.tag = -1L;
    iocomp.ferr = 0L;
    memset((char *)&iocomp.recv_info, 0x00, sizeof(iocomp.recv_info));

    memset(naibu_tranid, 0x20, sizeof(naibu_tranid));

    /* $RECEIVE READUPDATE処理 */
    KYFU_recv_read();

} /* end of KYFU_main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_recv_read                                 */
/*  CALLING SEQ.    : void KYFU_recv_read (void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : $RECEIVE READUPDATE処理                               */
/****************************************************************************/
void KYFU_recv_read(void)
{
    short       sdt_func_type   = 0;
    short       wk_err_cd       = 0;

    COM_SDT_arg_2_def       wk_myinfo_trc_time_begin;
    COM_SDT_arg_3_def       open_begin_COM_SDT_arg_3;
    COM_SDT_arg_3_def       open_end_COM_SDT_arg_3;
    long long               wk_begin_datetime_l = 0LL;

    memset(&wk_myinfo_trc_time_begin, 0x20, sizeof(wk_myinfo_trc_time_begin));
    memset(&open_begin_COM_SDT_arg_3, 0x20, sizeof(open_begin_COM_SDT_arg_3));
    memset(&open_end_COM_SDT_arg_3, 0x20, sizeof(open_end_COM_SDT_arg_3));

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_myinfo_trc_time_begin, &open_begin_COM_SDT_arg_3, &wk_begin_datetime_l);

    /* $RECEIVEのREADUPDATE */
    READUPDATEX (
        myinfo.recv_fno,
        recv_buf,
        (unsigned short)sizeof(recv_buf),
        (unsigned short *)&iocomp.len);

    /* 受信メッセージ取得(FILE番号が0の場合) */
    if ( myinfo.recv_fno == 0) {
        FILE_GETRECEIVEINFO_((short _far *)&iocomp.recv_info);
    }

    FILE_GETINFO_(myinfo.recv_fno, &iocomp.ferr);
    if (iocomp.ferr != 0      &&
        iocomp.ferr != 6) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", "$RECEIVE", "READUPDATE", "", iocomp.ferr);
        /* 異常終了 */
        KYFU_abend();
    }

    /* エラーコード判定 */
    switch(iocomp.ferr) {
    case 0 :
        /* 要求受信処理 */
        KYFU_req_recv();
        break;
    case 6 :
        /* システムメッセージ処理 */
        wk_err_cd = COM_STP_JUDGE(&openersinfo, sys_msg);

        /* 戻り値異常 */
        if(wk_err_cd < 0){
            /* メッセージ出力 */
            KYFU_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_PROC_OPN_ERR, "@R@e", procinfo.my_pname, wk_err_cd);
            /* 異常終了 */
            KYFU_abend();
        }
        /* クローズ判定 */
        if (wk_err_cd == 1){
            myinfo.end_flg = 1;
        }
        /* リプライ */
        KYFU_send_reply(resp_buf, 0, 0);
        break;
    default:
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", "$RECEIVE", "READUPDATE", "", iocomp.ferr);
        /* 異常終了 */
        KYFU_abend();
    }

} /* end of KYFU_recv_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_send_reply                                */
/*  CALLING SEQ.    : void KYFU_send_reply (char*, short, short)            */
/*  ARGUMENT        : IN リプライバッファ                                   */
/*                    IN リプライレングス                                   */
/*                    IN リプライコード                                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void KYFU_send_reply (
    char    *reply_buf,
    short   reply_len,
    short   reply_cd)
{
    _cc_status  wk_cc;

    short       sdt_func_type   = 0;
    COM_SDT_arg_2_def       wk_myinfo_trc_time_end;
    COM_SDT_arg_2_def       wk_trc_time_begin;
    COM_SDT_arg_2_def       wk_trc_time_end;
    COM_SDT_arg_3_def       reply_begin_COM_SDT_arg_3;
    COM_SDT_arg_3_def       reply_end_COM_SDT_arg_3;
    long long               wk_begin_datetime_l = 0LL;
    long long               wk_end_datetime_l = 0LL;

    memset(&wk_myinfo_trc_time_end, 0x20, sizeof(wk_myinfo_trc_time_end));
    memset(&wk_trc_time_begin, 0x20, sizeof(wk_trc_time_begin));
    memset(&wk_trc_time_end, 0x20, sizeof(wk_trc_time_end));
    memset(&reply_begin_COM_SDT_arg_3, 0x20, sizeof(reply_begin_COM_SDT_arg_3));
    memset(&reply_end_COM_SDT_arg_3, 0x20, sizeof(reply_end_COM_SDT_arg_3));

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_trc_time_begin, &reply_begin_COM_SDT_arg_3, &wk_begin_datetime_l);

    /* リプライ */
    wk_cc = REPLYX (
        reply_buf,
        reply_len,,
        iocomp.recv_info.z_messagetag,
        reply_cd);

    if (_status_ne(wk_cc)) {
        FILE_GETINFO_(myinfo.recv_fno, &iocomp.ferr);
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@X@E", "REPLYX", iocomp.ferr);
        /* 異常終了 */
        KYFU_abend();
    }

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_myinfo_trc_time_end, &reply_end_COM_SDT_arg_3, &wk_end_datetime_l);

    /* トレース出力 */

} /* end of KYFU_send_reply */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_req_recv                                  */
/*  CALLING SEQ.    : void KYFU_req_recv (void)                             */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求受信処理                                          */
/****************************************************************************/
void KYFU_req_recv (void)
{

    short   wk_result       = 0;            /* wk_処理結果 */
    short   wk_reply_cd     = 0;            /* wk_リプライコード */
    char    ch_timeout_ipc_total_len[8];    /* ch_タイムアウトIPC.DATA制御情報.全体長 */
    memset(ch_timeout_ipc_total_len, 0x00, sizeof(ch_timeout_ipc_total_len));

    /* IPCサイズチェック */
    if (iocomp.len <
        (sizeof(req_msg->common_header))) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｱﾝｻﾎﾟｰﾄIPC", recv_buf);
        /* エラーリプライ */
        KYFU_send_reply(resp_buf, 0, 0);
        return;
    }

    /* メッセージ.インターフェース種別判定 */
    if (memcmp(req_msg->common_header.interface_code, DEF_IPC_IFCD_GCKEY_UPD_REQ, sizeof(req_msg->common_header.interface_code)) == 0) {
        if ((req_msg->req_info.upd_rec_cnt == KYFU_REC_1) || (req_msg->req_info.upd_rec_cnt == KYFU_REC_2)) {
        } else {
            /* メッセージ出力 */
            KYFU_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "", recv_buf);
            /* エラー応答 */
            memcpy(rsp_msg->common_header.interface_code, DEF_IPC_IFCD_Q_RGST_RSP,sizeof(rsp_msg->common_header.interface_code)); /* R301 */
            rsp_msg->common_header.error_code = KYFU_RET_ERR;
            memcpy(rsp_msg->common_header.internal_error_code, DEF_NERR_FILE_IO_ERR, sizeof(rsp_msg->common_header.internal_error_code));
            rsp_msg->common_header.control_data_length = 0;
            KYFU_send_reply(resp_buf, 0, 0);
            return;
        }
    } else {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｱﾝｻﾎﾟｰﾄIPC", recv_buf);
        /* エラー応答 */
        memcpy(rsp_msg->common_header.interface_code, DEF_IPC_IFCD_Q_RGST_RSP,sizeof(rsp_msg->common_header.interface_code)); /* R301 */
        rsp_msg->common_header.error_code = KYFU_RET_ERR;
        memcpy(rsp_msg->common_header.internal_error_code, DEF_NERR_FILE_IO_ERR, sizeof(rsp_msg->common_header.internal_error_code));
        rsp_msg->common_header.control_data_length = 0;
        KYFU_send_reply(resp_buf, 0, 0);
        return;
    }

    /* 鍵管理更新処理 */
    wk_result = KYFU_gckey_update(KYFU_REC_1);
    if (wk_result == 0) {
        if (req_msg->req_info.upd_rec_cnt == KYFU_REC_2) {
            wk_result = KYFU_gckey_update(KYFU_REC_2);
        }
    }

    switch(wk_result) {
    case KYFU_RET_OK :
        /* 正常応答編集 */
        memcpy(rsp_msg->common_header.interface_code, DEF_IPC_IFCD_GCKEY_UPD_RSP,sizeof(rsp_msg->common_header.interface_code)); /* R301 */
        rsp_msg->common_header.error_code = KYFU_RET_OK;
        memcpy(rsp_msg->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(rsp_msg->common_header.internal_error_code));
        rsp_msg->common_header.control_data_length = 0;
        break;

    case KYFU_RET_ERR :
        /* エラー応答編集 */
        memcpy(rsp_msg->common_header.interface_code, DEF_IPC_IFCD_GCKEY_UPD_RSP,sizeof(rsp_msg->common_header.interface_code)); /* R301 */
        rsp_msg->common_header.error_code = KYFU_RET_ERR;
        memcpy(rsp_msg->common_header.internal_error_code, DEF_NERR_FILE_IO_ERR, sizeof(rsp_msg->common_header.internal_error_code));
        rsp_msg->common_header.control_data_length = 0;
        break;
    default:
        /* 異常終了 */
        KYFU_abend();
    }

    /* 応答送信 */
    KYFU_send_reply((char *)resp_buf, sizeof(r801_def), wk_reply_cd);
    return;

} /* end of KYFU_req_recv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_gckey_update                              */
/*  CALLING SEQ.    : short KYFU_gckey_update (void)                        */
/*  ARGUMENT        : 1. 更新レコード番号                                   */
/*  RETURN CODE     : 鍵管理ファイル更新エラーコード                        */
/*  DESCRIPTION     : 鍵管理ファイル更新処理                                */
/****************************************************************************/
short KYFU_gckey_update (short s_rec_count)
{
    short           s_result;
    db_gckey_def    *db_gckey;

    /* --- --- --- --- --- --- --- --- */
    /* ローカル変数.鍵管理ファイル用   */
    /* --- --- --- --- --- --- --- --- */
    /* 鍵管理ファイル用IOモジュールパラメータ */
    char                gckey_sub_prog_sts[2];
    COM_IOM_arg_3_def   gckey_COM_IOM_arg_3;
    COM_IOM_arg_4_def   gckey_COM_IOM_arg_4;
    COM_IOM_arg_5_def   gckey_COM_IOM_arg_5;
    COM_IOM_arg_6_def   gckey_COM_IOM_arg_6;

    if (s_rec_count == KYFU_REC_1) {
        db_gckey = (db_gckey_def *)&req_msg->upd_rec_1;
    } else {
        db_gckey = (db_gckey_def *)&req_msg->upd_rec_2;
    }

    /* 共通I/Oモジュール情報初期化 */
    memset(gckey_sub_prog_sts ,   0x00, sizeof(gckey_sub_prog_sts));
    memset(&gckey_COM_IOM_arg_3 , 0x00, sizeof(gckey_COM_IOM_arg_3));
    memset(&gckey_COM_IOM_arg_4 , 0x00, sizeof(gckey_COM_IOM_arg_4));
    memset(&gckey_COM_IOM_arg_5 , 0x00, sizeof(gckey_COM_IOM_arg_5));
    memset(&gckey_COM_IOM_arg_6 , 0x00, sizeof(gckey_COM_IOM_arg_6));

    /* トレース情報 */
    memcpy(gckey_COM_IOM_arg_3.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memcpy(gckey_COM_IOM_arg_3.file_id, DEF_GCKEY, strlen(DEF_GCKEY));
    memcpy(gckey_COM_IOM_arg_3.file_name, myinfo.gckey_fname, sizeof(gckey_COM_IOM_arg_3.file_name));
    memcpy(gckey_COM_IOM_arg_3.file_io_type, KYFU_FILEIO_READ, strlen(KYFU_FILEIO_READ));

    /* ファイル情報 */
    memcpy(gckey_COM_IOM_arg_4.file_id, DEF_FL_KEY_MG, strlen(DEF_FL_KEY_MG));

    memcpy(gckey_COM_IOM_arg_4.file_name, myinfo.gckey_fname, sizeof(gckey_COM_IOM_arg_4.file_name));
    gckey_COM_IOM_arg_4.file_no = myinfo.gckey_fnum;

    /* 入力情報 */
    gckey_COM_IOM_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_position = 0;
    gckey_COM_IOM_arg_5.part_key_len      = 0;
    memcpy(gckey_COM_IOM_arg_5.key_value  , (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));
    memcpy(gckey_COM_IOM_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gckey_COM_IOM_arg_5.key_len           = sizeof(db_gckey->pri_key);
    gckey_COM_IOM_arg_5.compare_len       = sizeof(db_gckey->pri_key);
    gckey_COM_IOM_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    gckey_COM_IOM_arg_5.lock_flg          = DEF_COM_IOM_LOCK;
    gckey_COM_IOM_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    gckey_COM_IOM_arg_5.io_timer          = myinfo.file_timer;
    gckey_COM_IOM_arg_5.rec_len           = db_gckey_def_Size;

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , gckey_sub_prog_sts
                       , &gckey_COM_IOM_arg_3
                       , &gckey_COM_IOM_arg_4
                       , &gckey_COM_IOM_arg_5
                       , &gckey_COM_IOM_arg_6);

    if ((s_result != KYFU_RET_OK) ||
        (memcmp( gckey_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        KYFU_message_output ( DEF_EVT_FILE_IO_ERR
                            , 'E'
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@C@f@X@K@E"
                            , myinfo.gfp_lcn
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , gckey_COM_IOM_arg_5.key_value
                            , gckey_COM_IOM_arg_6.guardian_errcode);
        return KYFU_RET_ERR;
    }

    /* 共通I/Oモジュール情報初期化 */
    memset(gckey_sub_prog_sts, 0x00, sizeof(gckey_sub_prog_sts));
    memset(&gckey_COM_IOM_arg_3 , 0x00, sizeof(gckey_COM_IOM_arg_3));
    memset(&gckey_COM_IOM_arg_4 , 0x00, sizeof(gckey_COM_IOM_arg_4));
    memset(&gckey_COM_IOM_arg_5 , 0x00, sizeof(gckey_COM_IOM_arg_5));
    memset(&gckey_COM_IOM_arg_6 , 0x00, sizeof(gckey_COM_IOM_arg_6));

    /* トレース情報 */
    memcpy(gckey_COM_IOM_arg_3.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memcpy(gckey_COM_IOM_arg_3.file_id, DEF_GCKEY, strlen(DEF_GCKEY));
    memcpy(gckey_COM_IOM_arg_3.file_name, myinfo.gckey_fname, sizeof(gckey_COM_IOM_arg_3.file_name));
    memcpy(gckey_COM_IOM_arg_3.file_io_type, KYFU_FILEIO_UPDATE, strlen(KYFU_FILEIO_UPDATE));

    /* ファイル情報 */
    memcpy(gckey_COM_IOM_arg_4.file_id, DEF_FL_KEY_MG, strlen(DEF_FL_KEY_MG));

    memcpy(gckey_COM_IOM_arg_4.file_name, myinfo.gckey_fname, sizeof(gckey_COM_IOM_arg_4.file_name));
    gckey_COM_IOM_arg_4.file_no = myinfo.gckey_fnum;

    /* 入力情報 */
    gckey_COM_IOM_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_position = 0;
    gckey_COM_IOM_arg_5.part_key_len      = 0;
    memcpy(gckey_COM_IOM_arg_5.key_value  , (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));
    memcpy(gckey_COM_IOM_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gckey_COM_IOM_arg_5.key_len           = sizeof(db_gckey->pri_key);
    gckey_COM_IOM_arg_5.compare_len       = sizeof(db_gckey->pri_key);
    gckey_COM_IOM_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    gckey_COM_IOM_arg_5.lock_flg          = DEF_COM_IOM_LOCKFREE;
    gckey_COM_IOM_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    gckey_COM_IOM_arg_5.io_timer          = myinfo.file_timer;
    gckey_COM_IOM_arg_5.rec_len           = db_gckey_def_Size;

    /* 鍵管理更新データ設定 */
    memcpy(gckey_COM_IOM_arg_5.rec_area, (char*)db_gckey, db_gckey_def_Size);

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                       , gckey_sub_prog_sts
                       , &gckey_COM_IOM_arg_3
                       , &gckey_COM_IOM_arg_4
                       , &gckey_COM_IOM_arg_5
                       , &gckey_COM_IOM_arg_6);

    if (s_result != KYFU_RET_OK) {
        KYFU_message_output ( DEF_EVT_FILE_IO_ERR
                            , 'E'
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@C@f@X@K@E"
                            , myinfo.gfp_lcn
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_UPDATE
                            , gckey_COM_IOM_arg_5.key_value
                            , gckey_COM_IOM_arg_6.guardian_errcode);
        return KYFU_RET_ERR;
    }

    return KYFU_RET_OK;

} /* end of KYFU_gckey_update */

/****************************************************************************/
/*  FUNCTION        : x.x.0  KYFU_message_output                            */
/*  CALLING SEQ.    : void KYFU_message_output( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ出力処理                                    */
/****************************************************************************/
void KYFU_message_output(
    short   s_event_code,
    char    ch_kubun,
    char    *pch_inter_code,
    char    *pch_format, ...)
{
    char        ch_text[99];
    char        ch_fmt[99];
    char        *pch_ep, *pch_sp;
    short       s_var, s_vcnt = 0, s_cnt, s_idx, s_param_cnt = 0, loop_flg = 1;
    va_list     va_ap;

    /* --- --- --- --- --- --- --- --- */
    /* サブルーチンリプライコード      */
    /* --- --- --- --- --- --- --- --- */
    cg010in.subrcd = '0';

    /* --- --- --- --- --- --- --- --- */
    /* 運用監視端末出力情報            */
    /* --- --- --- --- --- --- --- --- */
    /* KYFU_get_param()で事前設定 */

    /* --- --- --- --- --- --- --- --- */
    /* EMS出力情報                     */
    /* --- --- --- --- --- --- --- --- */
    /* リターンコード */
    cg010in.subrcd = '0';

    /* イベント番号/メッセージID */
    memset(ch_text, 0x00, sizeof(ch_text));
    sprintf(ch_text, "%05d", s_event_code);
    memcpy(cg010in.emsinf.msgid, ch_text, strlen(ch_text));

    /* --- --- --- --- --- --- --- --- */
    /* EMS出力情報.業務共通メッセージ  */
    /* --- --- --- --- --- --- --- --- */
    memset(&cg010in.emsinf.emsgkinf, 0x20, sizeof(cg010in.emsinf.emsgkinf));

    /* メッセージ通知区分 */
    cg010in.emsinf.emsgkinf.msgttkb = ch_kubun;

    /* システム名 */
    memcpy(cg010in.emsinf.emsgkinf.sysnm,
        DEF_EMS_SYSNM_GFP, sizeof(cg010in.emsinf.emsgkinf.sysnm));

    /* SERVER分類 */
    memcpy(cg010in.emsinf.emsgkinf.srv_kbn,
        DEF_EMS_SRV_KBN_COM, sizeof(cg010in.emsinf.emsgkinf.srv_kbn));

    /* NW識別(被仕向) */
    memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn,
        myinfo.nw_kubun, sizeof(myinfo.nw_kubun));

    /* NW識別(仕向) */
    /* 設定なし*/

    /* プログラム名 */
    memcpy(cg010in.emsinf.emsgkinf.prgid,
        DEF_GFPCVXH0, sizeof(cg010in.emsinf.emsgkinf.prgid));

    /* プロセス名 */
    memcpy(cg010in.emsinf.emsgkinf.trmnm,
        procinfo.my_pname, sizeof(procinfo.my_pname));

    /* GFP内部エラーコード */
    memcpy(cg010in.emsinf.emsgkinf.inter_errcd,
        pch_inter_code, sizeof(cg010in.emsinf.emsgkinf.inter_errcd));

    /* --- --- --- --- --- --- --- --- */
    /* EMS出力情報.任意メッセージ      */
    /* --- --- --- --- --- --- --- --- */
    memset((char *)&cg010in.emsinf.emsnninf, 0x20, sizeof(cg010in.emsinf.emsnninf));

    /* サーバークラス論理ID */
    if (myinfo.serverclass_id[0] != 0x00) {
        memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
            myinfo.serverclass_id, 12);
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
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'i':       /* IPCヘッダ */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            HEX2CHAR(pch_ep, ch_text, 24);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'K':       /* キー */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, strlen(pch_ep));
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '1':       /* 1桁BINARY(フラグ) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%01d", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '2':       /* 2桁BINARY(CPU番号) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%02d", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '3':       /* 3桁BINARY(内部テーブル数) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%03d", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '4':       /* 4桁BINARY(システムエラーコード) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%04d", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '5':       /* 5桁BINARY */
        case 'e':       /* エラーコード */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%05d", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
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
            s_var = (short)va_arg(va_ap, unsigned short);
            sprintf(ch_text,"%05u", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case '8':       /* 8桁BINARY */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%08", s_var);
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
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
    GFPOGGZ1(&cg010in);

} /* KYFU_message_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_finish                                    */
/*  CALLING SEQ.    : void KYFU_finish (void)                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void KYFU_finish (void)
{

    /* --- --- --- --- --- --- */
    /* 鍵管理ファイル.クローズ */
    /* --- --- --- --- --- --- */
    /* 鍵管理ファイル用IOモジュールパラメータ */
    char                gckey_sub_prog_sts[2];
    COM_IOM_arg_3_def   gckey_COM_IOM_arg_3;
    COM_IOM_arg_4_def   gckey_COM_IOM_arg_4;
    COM_IOM_arg_5_def   gckey_COM_IOM_arg_5;
    COM_IOM_arg_6_def   gckey_COM_IOM_arg_6;

    /* arg2_サブプログラムステータス */
    memset(gckey_sub_prog_sts, 0x20, sizeof(gckey_sub_prog_sts));

    /* arg3_トレース情報 */
    memset(&gckey_COM_IOM_arg_3,                0x20,               sizeof(gckey_COM_IOM_arg_3));
    memcpy(gckey_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gckey_COM_IOM_arg_3.file_id,         DEF_GCKEY,          sizeof(DEF_GCKEY) -1);
    memset(gckey_COM_IOM_arg_3.file_name,       0x20,               sizeof(gckey_COM_IOM_arg_3.file_name));
    memcpy(gckey_COM_IOM_arg_3.file_io_type,    KYFU_FILEIO_CLOSE,  sizeof(KYFU_FILEIO_CLOSE) -1);

    /* arg4_ファイル情報 */
    memset(&gckey_COM_IOM_arg_4,            0x20,               sizeof(gckey_COM_IOM_arg_4));
    memcpy(gckey_COM_IOM_arg_4.file_id,     DEF_GCKEY,          sizeof(DEF_GCKEY) -1);
    memcpy(gckey_COM_IOM_arg_4.file_name,   myinfo.gckey_fname, sizeof(myinfo.gckey_fname));
    gckey_COM_IOM_arg_4.file_no             = myinfo.gckey_fnum;

    /* arg5_入力情報 */
    memset(&gckey_COM_IOM_arg_5,            0x20,   sizeof(gckey_COM_IOM_arg_5));
    gckey_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gckey_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gckey_COM_IOM_arg_5.key_value,   0x20,   sizeof(gckey_COM_IOM_arg_5.key_value));
    memset(gckey_COM_IOM_arg_5.key_type,    0x20,   sizeof(gckey_COM_IOM_arg_5.key_type));
    gckey_COM_IOM_arg_5.key_len             = 0;
    gckey_COM_IOM_arg_5.compare_len         = 0;
    gckey_COM_IOM_arg_5.positioning_mode    = 0;
    gckey_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gckey_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gckey_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gckey_COM_IOM_arg_5.rec_len             = 0;
    memset(gckey_COM_IOM_arg_5.rec_area,    0x20,   sizeof(gckey_COM_IOM_arg_5.rec_area));

    /* arg6_出力情報 */
    memset(&gckey_COM_IOM_arg_6,            0x20,   sizeof(gckey_COM_IOM_arg_6));
    gckey_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gckey_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gckey_COM_IOM_arg_6.err_proc));
    memset(gckey_COM_IOM_arg_6.file_name,   0x20,   sizeof(gckey_COM_IOM_arg_6.file_name));
    gckey_COM_IOM_arg_6.rec_len             = 0;
    memset(gckey_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gckey_COM_IOM_arg_6.rec_area));

    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_CLOSE,
        gckey_sub_prog_sts,
        &gckey_COM_IOM_arg_3,
        &gckey_COM_IOM_arg_4,
        &gckey_COM_IOM_arg_5,
        &gckey_COM_IOM_arg_6
    );
    /* クローズ完了 */
    if (memcmp(gckey_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != KYFU_RET_OK) {
        /* メッセージ出力 */
        KYFU_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", myinfo.gfp_lcn, "", DEF_FL_PHSIC_INFO, "CLOSE", gckey_COM_IOM_arg_5.key_value, gckey_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        KYFU_abend();
    }
    /* FILE情報格納 */
    myinfo.gckey_fnum = KYFU_FILE_CLOSED;

    /* トレース終了処理 */
    memset((char *)&g_trc.recv, 0x20, sizeof(g_trc.recv));
    g_trc.recv.func_flg = FUNC_END;
    memcpy(g_trc.recv.trace_info.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memset(g_trc.recv.trace_info.guardian_errcode, '0', sizeof(char));
    memset(g_trc.recv.data_info.rec_len, '0', sizeof(char));
    TRACEOUT((char *)&g_trc.recv);

    /* メッセージ出力 */
    KYFU_message_output(DEF_EVT_PROC_NORMAL_END, '*', DEF_NERR_NOMAL, "@R", procinfo.my_pname);

    /* PROCESS_STOP */
    PROCESS_STOP_(,, KYFU_NORMAL_TERMINATION);

} /* end of KYFU_finish */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  KYFU_abend                                     */
/*  CALLING SEQ.    : void KYFU_abend (void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void KYFU_abend (void)
{
    /* メッセージ出力 */
    KYFU_message_output(DEF_EVT_PROC_ABNORMAL_END, 'E', DEF_NERR_NOMAL, "@R", procinfo.my_pname);

    /* PROCESS_ABEND */
    PROCESS_STOP_(,, KYFU_ABNORMAL_TERMINATION);

} /* end of KYFU_abend */
