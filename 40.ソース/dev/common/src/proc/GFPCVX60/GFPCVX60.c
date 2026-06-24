/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX60                                    */
/*        FUNCTION          ････ 電文中継(GET)                               */
/*                                                                           */
/*        AUTHER            ････ HAS S.Makino                                */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-11-07                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Makino    2024/11/07 (xxxxx)新規作成                              */
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* USER HEADER     */
#include "GFPCVX60.h"

/********** グローバルデータ ***********/
myinfo_def                      myinfo;                                                 /* 自プロセス情報                         */
procinfo_def                    procinfo;                                               /* プロセス情報                           */
qfileinfo_def                   qfileinfo;                                              /* キューファイル情報                     */
iocomp_def                      iocomp;                                                 /* I/O完了情報                            */
COM_STP_arg_1_def               openersinfo;                                            /* オープナー情報テーブル                 */
char                            recv_buf[32000];                                        /* $RECEIVE I/O用バッファ                 */
short                           recv_buf_size;                                          /* $RECEIVE I/O用バッファのサイズ         */
char                            resp_buf[32000];                                        /* 応答メッセージ用バッファ               */
short                           qfile_read_buf_size;                                    /* キューファイル読み込みバッファのサイズ */
char                            qfile_read_buf[db_gqnwq_def_Size];                      /* キューファイル読み込みバッファ         */
char                            *sys_msg = recv_buf;                                    /* $RECEIVEバッファアドレス               */
char                            send_buf[32000];                                        /* PATHSEND用バッファ                     */
short                           send_buf_size;                                          /* PATHSEND用バッファ                     */
c302_def                        *req_msg = (c302_def *)send_buf;                        /* 要求メッセージバッファアドレス         */
r302_def                        *rsp_msg = (r302_def *)resp_buf;                        /* 応答メッセージバッファアドレス         */
char                            naibu_tranid[41];                                       /* 内部トランザクションID                 */

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
    QGET_init();

    /* 主処理 */
    /* フラグがONになるまでループ  */
    while ( myinfo.end_flg == QGET_OFF ) {
        /* 主処理 */
        QGET_main();
    }

    /* 終了処理 */
    QGET_finish();

} /* end of main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_init                                      */
/*  CALLING SEQ.    : void QGET_init (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void QGET_init()
{
    short   wk_error;         /* WKエラーコード($RECEIVE)   */
    short   wk_result;        /* WK処理結果                 */
    short   wk_err_code = 0;  /* WKエラーコード             */
    long    wk_naibu_tranid;  /* WKトランザクションID       */
    char    wk_fname[47];     /* WKファイル名               */
    
    memset(wk_fname, 0x20, sizeof(wk_fname));

    /* グローバル情報の初期化(自プロセス情報) */
    myinfo.recv_fno = -1;
    memset(&recv_buf, 0x20, sizeof(recv_buf));
    memset(&myinfo.recv_buf, 0x20, sizeof(myinfo.recv_buf));
    recv_buf_size = 0;
    memset(resp_buf, 0x20, sizeof(resp_buf));
    memset(myinfo.psend_serverclass_name, 0x20, sizeof(myinfo.psend_serverclass_name));
    memset(myinfo.psend_pathmon_name, 0x20, sizeof(myinfo.psend_pathmon_name));
    myinfo.end_flg = 0;
    myinfo.read_end_flg = 0;
    memset(myinfo.trc_time_begin, 0x20, sizeof(myinfo.trc_time_begin));
    memset(myinfo.trc_time_end, 0x20, sizeof(myinfo.trc_time_end));
    myinfo.site_id = ' ';
    myinfo.network_id = ' ';
    memset(myinfo.nw_kubun, 0x20, sizeof(myinfo.nw_kubun));
    memset(myinfo.group_id, 0x20, sizeof(myinfo.group_id));
    memset(myinfo.serverclass_kind, 0x20, sizeof(myinfo.serverclass_kind));
    memset(myinfo.serverclass_no, 0x20, sizeof(myinfo.serverclass_no));
    memset(myinfo.serverclass_name, 0x20, sizeof(myinfo.serverclass_name));
    myinfo.file_timer = 0L;
    myinfo.send_timer = 0L;
    myinfo.send_retry = 0;
    memset(myinfo.msg_serverclass_name, 0x00, sizeof(myinfo.msg_serverclass_name));
    memset(myinfo.msg_pathmon_name, 0x00, sizeof(myinfo.msg_pathmon_name));
    myinfo.expiry_second = 0L;
    myinfo.qfile_read_wait_timer = 0L;
    memset(myinfo.module_id, 0x00, sizeof(myinfo.module_id));
    memset(myinfo.gfphi_asn_fname, 0x20, sizeof(myinfo.gfphi_asn_fname));
    memset(myinfo.gfphi_fname, 0x20, sizeof(myinfo.gfphi_fname));
    myinfo.gfphi_fnum = -1;
    memset(myinfo.gfnwi_fname, 0x20, sizeof(myinfo.gfnwi_fname));
    myinfo.gfnwi_fnum = -1;
    myinfo.erl_fnum = -1;
    myinfo.abort_delay_timer = 0L;

    /* グローバル情報の初期化(プロセス情報) */
    memset(&procinfo, 0x20, sizeof(procinfo));

    /* グローバル情報の初期化(キューファイル情報) */
    memset(qfileinfo.fname, 0x00, sizeof(qfileinfo.fname));
    qfileinfo.fno = 0;
    qfileinfo.fnum = -1;
    qfileinfo.ferr = 0;
    memset(qfileinfo.rec_key, 0x00, sizeof(qfileinfo.rec_key));

    /* グローバル情報の初期化(オープナー情報テーブル) */
    memset(&openersinfo, 0x20, sizeof(openersinfo));

    /* EMS初期設定 */
    memset((char *)&cg010in, 0x20, sizeof(cg010in));
    cg010in.subrcd = '0';
    cg010in.emsinf.rcd = '0';
    memcpy(cg010in.uytrminf.proctimer, "0002", 4); /* temp */
    memcpy(cg010in.uytrminf.uytrmmonlen, "00", 2); /* temp */
    memcpy(cg010in.uytrminf.uytrmsrvlen, "00", 2); /* temp */
    memcpy(cg010in.emsinf.emsgkinf.prgid, DEF_GFPCVX60, strlen(DEF_GFPCVX60));

    /* グローバル情報の初期化(I/O完了情報) */
    iocomp.fno = -1;
    iocomp.addr = 0L;
    iocomp.len = 0;
    iocomp.tag = -1L;
    iocomp.ferr = 0;
    memset((char *)&iocomp.recv_info, 0x00, sizeof(iocomp.recv_info));
        
    /* グローバル情報の初期化 */
    memset(&recv_buf, 0x20, sizeof(recv_buf));
    recv_buf_size = 0;
    memset(&resp_buf, 0x20, sizeof(resp_buf));
    qfile_read_buf_size = 0;
    memset(qfile_read_buf, 0x20, sizeof(qfile_read_buf));
    memset(send_buf, 0x20, sizeof(send_buf));
    send_buf_size = 0;
    memset(naibu_tranid, 0x20, sizeof(naibu_tranid));

    /* ローカル情報の初期化 */
    wk_error = QGET_RET_OK;
    wk_result = QGET_RET_OK;

    /* エラー出力ログ用アーギュメントの初期化 */
    memset(&timeup_COM_ERL_arg_1,   0,   sizeof(timeup_COM_ERL_arg_1));
    memset(&timeup_COM_ERL_arg_2,   0x20,   sizeof(timeup_COM_ERL_arg_2));
    memset(&timeup_COM_ERL_arg_3,   0x20,   sizeof(timeup_COM_ERL_arg_3));
    
    /* オープナープロセス管理モジュール初期処理 */
    COM_STP_INIT(&openersinfo);

    /* プロセス情報取得処理 */
    wk_result = COM_PRC(&procinfo);
    if (wk_result != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_PROC_OPN_ERR, "@X", "COM_PRC");
        /* 異常終了 */
        QGET_abend();
    }

    /* モジュールID */
    memcpy(myinfo.module_id, DEF_GFPCVX60, sizeof(DEF_GFPCVX60));

    /* パラメータ取得処理 */
    QGET_get_param();
    
    /* トレース出力初期処理 */
    memset((char *)&g_trc.recv, 0x20, sizeof(g_trc.recv));
    g_trc.recv.func_flg = FUNC_INIT;
    memcpy(g_trc.recv.trace_info.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memset(g_trc.recv.data_info.rec_len, '0', sizeof(char));
    TRACEOUT((char *)&g_trc.recv);

    /* コンフィグ情報取得処理 */
    memcpy(myinfo.gfphi_asn_fname,"GFPHI", 5);
    
    QGET_get_config(myinfo.gfphi_asn_fname);

    /* キューファイルオープン */
    QGET_qfile_open();
    
    /* --- --- --- --- --- --- --- --- --- --- */
    /* エラー出力ログ編集出力モジュール(OPEN)  */
    /* --- --- --- --- --- --- --- --- --- --- */
    /* arg1.入力情報 */
    timeup_COM_ERL_arg_1.file_io_type   = DEF_COM_ERL_ARG1_OPEN;
    timeup_COM_ERL_arg_1.io_timer       = myinfo.file_timer;
    
    
    /* arg2.エラー出力ログファイル情報 */
    memset(&timeup_COM_ERL_arg_2,       0x20,   sizeof(timeup_COM_ERL_arg_2));
    /* timeup_COM_ERL_arg_2.file_name ->OPEN時の設定なし*/
    timeup_COM_ERL_arg_2.file_no        = QGET_FILE_CLOSED;
    
    /* arg3.EMS出力共通情報 */
    /* 運用監視端末出力情報 ->QGET_get_paramで事前設定 */
    /* EMS出力情報.業務共通メッセージ */
    memset(&cg010in.emsinf,                 0x20,                   sizeof(cg010in.emsinf));
    memcpy(cg010in.emsinf.emsgkinf.sysnm,   DEF_EMS_SYSNM_GFP,      sizeof(cg010in.emsinf.emsgkinf.sysnm));
    memcpy(cg010in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM,    sizeof(cg010in.emsinf.emsgkinf.srv_kbn));
    memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn,myinfo.nw_kubun,        sizeof(myinfo.nw_kubun));
    memcpy(cg010in.emsinf.emsgkinf.prgid,   DEF_GFPCVX60,           sizeof(cg010in.emsinf.emsgkinf.prgid));
    memcpy(cg010in.emsinf.emsgkinf.trmnm,   procinfo.my_pname,      sizeof(procinfo.my_pname));
    
    /* arg4.EMS出力付加情報 */
    memset(&timeup_COM_ERL_arg_3,                   0x20,                       sizeof(timeup_COM_ERL_arg_3));
    memcpy(&timeup_COM_ERL_arg_3.srv_logical_id,    myinfo.serverclass_kind,    sizeof(myinfo.serverclass_kind));
    
    wk_err_code = COM_ERL(&timeup_COM_ERL_arg_1, &timeup_COM_ERL_arg_2, &cg010in, &timeup_COM_ERL_arg_3, DEF_GFPCVX60);
    if (wk_err_code != 0) {
        /* 異常終了 */
        QGET_abend();
    }
    
    /* エラー出力ログ.ファイル情報取得 */
    memcpy(myinfo.erl_fname,    timeup_COM_ERL_arg_2.file_name,   sizeof(myinfo.erl_fname));
    myinfo.erl_fnum             = timeup_COM_ERL_arg_2.file_no;

    /* $RECEIVE OPEN */
    wk_error = FILE_OPEN_(
        "$RECEIVE", 8,
        &myinfo.recv_fno,
        ZSYS_VAL_OPENACC_READWRITE,
        ZSYS_VAL_OPENEXCL_SHARED,
        1,  /* NOWAIT-DEPTH */
        1   /* RECEIVE-DEPTH */
        );
    if (wk_error != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E", "", "", "$RECEIVE", "OPEN", "", wk_error);
        /* 異常終了 */
        QGET_abend();
    }
    
    /* $RECEIVE READUPDATE */
    READUPDATEX (
        myinfo.recv_fno,
        recv_buf,
        (unsigned short)sizeof(recv_buf),
        (unsigned short *)&iocomp.len);

    /* BIGINTRANSACTION */
    wk_naibu_tranid =  atol(naibu_tranid);
    wk_result = COM_TMF(DEF_COM_TMF_BEGIN, &wk_naibu_tranid, myinfo.module_id);
    if (wk_result != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", wk_result);
        /* 異常終了 */
        QGET_abend();
        
    }
    /* $QUEUE-FILE READUPDATELOCK */
    READUPDATELOCKX(qfileinfo.fnum
                   ,qfile_read_buf
                   ,db_gqnwq_def_Size
                   ,(unsigned short *)&iocomp.len);

    /* メッセージ出力 */
    QGET_message_output(DEF_EVT_PROC_START, '*', DEF_NERR_NOMAL, "@R", procinfo.my_pname);
    
} /* end of QGET_init */


/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_get_param                                 */
/*  CALLING SEQ.    : void QGET_get_param (void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void QGET_get_param (void)
{
    char    wk_paramname    [32];
    char    wk_param        [64];
    short   wk_result;
    char    ch_work[10];

    /* サーバクラス論理ID取得 */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_SRV_LOGICAL_ID, sizeof(DEF_SRV_LOGICAL_ID) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 23+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_SRV_LOGICAL_ID, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) != 23) {
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_SRV_LOGICAL_ID, wk_result);
        /* 異常終了 */
        QGET_abend();
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
    memcpy(&myinfo.serverclass_name[0], &myinfo.serverclass_kind, 8);
    memcpy(&myinfo.serverclass_name[8], &myinfo.serverclass_no,   4);

    /* 運用監視端末出力サーバ.PATHMON名 */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_MSG_MON_NAME, sizeof(DEF_MSG_MON_NAME) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 13+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_MON_NAME, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 13) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_MSG_MON_NAME, wk_result);
        /* 異常終了 */
        QGET_abend();
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
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_SRV_NAME, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 12) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_MSG_SRV_NAME, wk_result);
        /* 異常終了 */
        QGET_abend();
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
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    if (strlen(wk_param) > 8) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
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
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_PSEND_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 8) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_PSEND_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    myinfo.send_timer = atol(wk_param);
    
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%04ld", myinfo.send_timer/100);
    memcpy(cg010in.uytrminf.proctimer, ch_work, strlen(ch_work));

    /* PATHSENDリトライ回数 */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_PSEND_RETRY_CNT, sizeof(DEF_PSEND_RETRY_CNT) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 4+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_PSEND_RETRY_CNT, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 4) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_PSEND_RETRY_CNT, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    myinfo.send_retry = (short)atoi(wk_param);

    /* 送信期限切れタイマー */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_DENBUN_EXPIRY_SECOND, sizeof(DEF_DENBUN_EXPIRY_SECOND) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 6+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_DENBUN_EXPIRY_SECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 6) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_DENBUN_EXPIRY_SECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    myinfo.expiry_second = atol(wk_param);

    /* キューファイル読み込み完了待ちタイマー */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_QUE_READ_TIMER_10MSECOND, sizeof(DEF_QUE_READ_TIMER_10MSECOND) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 6+1);
    if (wk_result < 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_QUE_READ_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    if (strlen(wk_param) > 6) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_QUE_READ_TIMER_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    
    myinfo.qfile_read_wait_timer = atol(wk_param);

    /* ABORT TRAN DELAYタイマー(単位:ミリ秒) */
    memset(wk_paramname, 0x00, sizeof(wk_paramname));
    memset(wk_param, 0x00, sizeof(wk_param));
    wk_result = 0;
    
    memcpy(wk_paramname, DEF_ABORT_DELAY_10MSECOND, sizeof(DEF_ABORT_DELAY_10MSECOND) -1);
    wk_result = get_param_by_name (wk_paramname, wk_param, 8+1);
    if ((wk_result == -2) || (strlen(wk_param) > 8)) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_ABORT_DELAY_10MSECOND, wk_result);
        /* 異常終了 */
        QGET_abend();
    }
    if (wk_result == 0) {
        myinfo.abort_delay_timer = atol(wk_param);
    } else {
        myinfo.abort_delay_timer = 0L;
    }

} /* end of QGET_get_param */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_get_config                                */
/*  CALLING SEQ.    : void QGET_get_config (char)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コンフィグ情報取得処理                                */
/****************************************************************************/
void QGET_get_config (
    char*    asn_filename
)
{
    /* --- --- --- --- */
    /* ローカル変数    */
    /* --- --- ---  ---*/

    char    wk_filename[48];
    short   wk_filename_len = 0;
    char    wk_gfnwi_filename[48];
    long    wk_gfnwi_filename_len = 0;
    char    *wk_gfnwi_space_posi  = 0;
    
    memset(wk_filename, 0x20, sizeof(wk_filename));
    memset(&wk_gfnwi_filename, 0x20, sizeof(wk_gfnwi_filename));

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
    
    /* NW情報ファイル.プライマリキー */
    typedef struct __gfnwi_pri_key_def
    {
        char                            site_id;
        char                            nw_id;
        char                            grp_id[5];
        char                            if_id[5];
        char                            station_id[6];
    } gfnwi_pri_key_def;
    
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
    /* ローカル変数.NW情報ファイル用   */
    /* --- --- --- --- --- --- --- --- */
    /* NW情報ファイル用IOモジュールパラメータ */
    char                gfnwi_sub_prog_sts[2];
    COM_IOM_arg_3_def   gfnwi_COM_IOM_arg_3;
    COM_IOM_arg_4_def   gfnwi_COM_IOM_arg_4;
    COM_IOM_arg_5_def   gfnwi_COM_IOM_arg_5;
    COM_IOM_arg_6_def   gfnwi_COM_IOM_arg_6;
    
    memset(&gfnwi_sub_prog_sts,     0x20,   sizeof(gfnwi_sub_prog_sts));
    memset(&gfnwi_COM_IOM_arg_3,    0x20,   sizeof(gfnwi_COM_IOM_arg_3));
    memset(&gfnwi_COM_IOM_arg_4,    0x20,   sizeof(gfnwi_COM_IOM_arg_4));
    memset(&gfnwi_COM_IOM_arg_5,    0x20,   sizeof(gfnwi_COM_IOM_arg_5));
    memset(&gfnwi_COM_IOM_arg_6,    0x20,   sizeof(gfnwi_COM_IOM_arg_6));
    
    /* NW情報ファイル用レコード */
    db_gfnwi_def        gfnwi_rec;
    memset(&gfnwi_rec, 0x20, sizeof(gfnwi_rec));
    
    /* NW情報ファイル.プライマリキー */
    gfnwi_pri_key_def    wk_gfnwi_pri_key;
    memset(&wk_gfnwi_pri_key, 0x20, sizeof(wk_gfnwi_pri_key));

    /* --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.オープン */
    /* --- --- --- --- --- --- --- */
    /* 物理名情報ファイル物理ファイル名取得(ASSIGN情報) */
    COM_ASN(asn_filename, wk_filename, &wk_filename_len );
    if ( (0 == wk_filename_len) || (47 < wk_filename_len) ){ /* 物理名長異常 */
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_ASN_FILE_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@E", DEF_ASN_GFPHI, 0);
        /* 異常終了 */
        QGET_abend();
        /* 異常終了 */
        QGET_abend();
    }
    
    memcpy(myinfo.gfphi_fname, wk_filename, sizeof(myinfo.gfphi_fname));
    
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(gfphi_COM_IOM_arg_3.prog_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(gfphi_COM_IOM_arg_3.file_id));
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_OPEN,   sizeof(QGET_FILEIO_OPEN) -1);
        
    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,                0x20,               sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,       myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no                 = myinfo.gfphi_fnum;
    
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
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, "OPEN", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* FILE情報格納 */
    myinfo.gfphi_fnum = gfphi_COM_IOM_arg_4.file_no;
    
    /* --- --- --- --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.キューファイル名取得 */
    /* --- --- --- --- --- --- --- --- --- --- */
    
    /* キー設定 */
    memset(&wk_gfphi_pri_key,           0x20,               sizeof(wk_gfphi_pri_key));
    memcpy(&wk_gfphi_pri_key.site_id,   &myinfo.site_id,    sizeof(wk_gfphi_pri_key.site_id));
    memcpy(&wk_gfphi_pri_key.nw_id,     &myinfo.network_id, sizeof(wk_gfphi_pri_key.nw_id));
    memcpy(&wk_gfphi_pri_key.grp_id,    &myinfo.group_id,   sizeof(wk_gfphi_pri_key.grp_id));
    
    memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,    myinfo.serverclass_kind
                                                          , sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num,     myinfo.serverclass_no
                                                          , sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num,            QGET_NON_MLT_NUM
                                                          , sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num));
    
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_NW_QUE
                                                          , sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num,  QGET_NON_MLT_NUM
                                                          , sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num,          myinfo.serverclass_no
                                                          , sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num));
    
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_READ,   sizeof(QGET_FILEIO_READ) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,            0x20,                   sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,     DEF_GFPHI,              sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,   myinfo.gfphi_fname,     sizeof(myinfo.gfphi_fname) -1);
    gfphi_COM_IOM_arg_4.file_no             = myinfo.gfphi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,            0x20,                       sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(gfphi_COM_IOM_arg_5.key_value,   &wk_gfphi_pri_key,          sizeof(wk_gfphi_pri_key));
    memcpy(gfphi_COM_IOM_arg_5.key_type,    DEF_COM_IOM_KEYTYPE_PRI,    sizeof(DEF_COM_IOM_KEYTYPE_PRI) -1);
    gfphi_COM_IOM_arg_5.key_len             = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.compare_len         = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    gfphi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len             = db_gfphi_def_Size;
    memset(gfphi_COM_IOM_arg_5.rec_area,    0x20,                       sizeof(gfphi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,            0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_STARTREAD,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    
    /* 読み込み結果判定 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_sub_prog_sts)) == 0) {
        /* ファイル情報格納 */
        memcpy(&gfphi_rec, &gfphi_COM_IOM_arg_6.rec_area, sizeof(gfphi_rec));
        memcpy(qfileinfo.fname, gfphi_rec.prc_file_info.prc_file_name, sizeof(qfileinfo.fname));
        
    }else if(memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_sub_prog_sts)) == 0) {
        /* 終了フラグON(対象なし) */
        myinfo.end_flg = QGET_ON;
        
    }else{
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR , "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, DEF_COM_IOM_FUNC_STARTREAD, gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
        
    }

    /* --- --- --- --- --- --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.PATHSEND先サーバクラス名取得 */
    /* --- --- --- --- --- --- --- --- --- --- --- --- */
    
    /* キー設定 */
    memset(&wk_gfphi_pri_key, 0x20, sizeof(wk_gfphi_pri_key));
    memcpy(&wk_gfphi_pri_key.site_id, &myinfo.site_id, sizeof(wk_gfphi_pri_key.site_id));
    memcpy(&wk_gfphi_pri_key.nw_id, &myinfo.network_id, sizeof(wk_gfphi_pri_key.nw_id));
    memcpy(&wk_gfphi_pri_key.grp_id, &myinfo.group_id, sizeof(wk_gfphi_pri_key.grp_id));
    
    if (memcmp(myinfo.serverclass_kind, DEF_SC_CTRL_IF_I, sizeof(myinfo.serverclass_kind)) == 0) {
        /* 制御電文振分にPATHSEND */
        memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CTRL_FURI, sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    }else{
        /* 電文振分(outbound)にPATHSEND */
        memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_FURI_O, sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    }
    memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num, QGET_NON_MLT_NUM, sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    
    memcpy(wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num, QGET_NON_MLT_NUM, sizeof(wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num));
    
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_SC_NAME_DEFAULT, sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT, sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT, sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num)); /* DEF_SC_DUP_DEFAULT */
    
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_READ,   sizeof(QGET_FILEIO_READ) -1);
        
    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,            0x20,               sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,     DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,   myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no             = myinfo.gfphi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,            0x20,                       sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(gfphi_COM_IOM_arg_5.key_value,   &wk_gfphi_pri_key,          sizeof(wk_gfphi_pri_key));
    memcpy(gfphi_COM_IOM_arg_5.key_type,    DEF_COM_IOM_KEYTYPE_PRI,    sizeof(DEF_COM_IOM_KEYTYPE_PRI) -1);
    gfphi_COM_IOM_arg_5.key_len             = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.compare_len         = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    gfphi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len             = db_gfphi_def_Size;
    memset(gfphi_COM_IOM_arg_5.rec_area,    0x20,                       sizeof(gfphi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,            0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_STARTREAD,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    
    /* 読み込み結果判定 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_sub_prog_sts)) == 0) {
        /* PATHSEND先.サーバクラス情報/PATHMON名 */
        memcpy(&gfphi_rec, &gfphi_COM_IOM_arg_6.rec_area, sizeof(gfphi_rec));
        if (gfphi_rec.srv_cls_info.domain_name[0] != ' ') {     /* ドメイン指定有り */
            memset(myinfo.psend_pathmon_name, ' ', sizeof(myinfo.psend_pathmon_name));
            memcpy(myinfo.psend_pathmon_name, gfphi_rec.srv_cls_info.domain_name, sizeof(gfphi_rec.srv_cls_info.domain_name));
        }else{                                                  /* PATHMON指定 */
            memcpy(myinfo.psend_pathmon_name, gfphi_rec.srv_cls_info.pathmon_name, sizeof(myinfo.psend_pathmon_name));
        }
        memcpy(myinfo.psend_serverclass_name, gfphi_rec.srv_cls_info.srv_cls_name, sizeof(myinfo.psend_serverclass_name));
        
    }else if(memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_sub_prog_sts)) == 0) {
        /* 終了フラグON(対象なし) */
        myinfo.end_flg = QGET_ON;
        
    }else{
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR , "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, DEF_COM_IOM_FUNC_STARTREAD, gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了処理 */
        QGET_abend();
        
    }

    /* --- --- --- --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.NW情報ファイル名取得 */
    /* --- --- --- --- --- --- --- --- --- --- */
    
    /* キー設定 */
    memset(&wk_gfphi_pri_key,               0x20,               sizeof(wk_gfphi_pri_key));
    memcpy(&wk_gfphi_pri_key.site_id,       &myinfo.site_id,    sizeof(wk_gfphi_pri_key.site_id));
    memcpy(&wk_gfphi_pri_key.nw_id,         &myinfo.network_id, sizeof(wk_gfphi_pri_key.nw_id));
    memcpy(&wk_gfphi_pri_key.grp_id,        &myinfo.group_id,   sizeof(wk_gfphi_pri_key.grp_id));
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,       DEF_SC_NAME_DEFAULT
                                                              , sizeof(DEF_SC_NAME_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num,        DEF_SC_NUM_DEFAULT
                                                              , sizeof(DEF_SC_NUM_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num,               DEF_SC_DUP_DEFAULT
                                                              , sizeof(DEF_SC_DUP_DEFAULT) -1);
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind,    DEF_FL_NW_INFO
                                                              , sizeof(DEF_FL_NW_INFO) -1);
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num,     QGET_NON_MLT_NUM
                                                              , sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(&wk_gfphi_pri_key.prc_file_key.prc_file_mlt_num,             QGET_NON_MLT_NUM
                                                              , sizeof(wk_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_3.file_name,       myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_READ,   sizeof(QGET_FILEIO_READ) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,            0x20,               sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,     DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,   myinfo.gfphi_fname, sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no             = myinfo.gfphi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,            0x20,                       sizeof(gfphi_COM_IOM_arg_5));
    gfphi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(gfphi_COM_IOM_arg_5.key_value,   &wk_gfphi_pri_key,          sizeof(wk_gfphi_pri_key));
    memcpy(gfphi_COM_IOM_arg_5.key_type,    DEF_COM_IOM_KEYTYPE_PRI,    sizeof(DEF_COM_IOM_KEYTYPE_PRI) -1);
    gfphi_COM_IOM_arg_5.key_len             = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.compare_len         = sizeof(wk_gfphi_pri_key);
    gfphi_COM_IOM_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    gfphi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfphi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfphi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfphi_COM_IOM_arg_5.rec_len             = db_gfphi_def_Size;
    memset(gfphi_COM_IOM_arg_5.rec_area,    0x20,                       sizeof(gfphi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfphi_COM_IOM_arg_6,            0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_STARTREAD,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    
    /* 読み込み終了 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_INFO, "READ", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* 正常読み込み.読み込み情報格納 */
    memset(&gfphi_rec, 0x20, sizeof(gfphi_rec));
    memcpy(&gfphi_rec, &gfphi_COM_IOM_arg_6.rec_area, sizeof(gfphi_rec));
    memcpy(&wk_gfnwi_filename, gfphi_rec.prc_file_info.prc_file_name, sizeof(gfphi_rec.prc_file_info.prc_file_name));
    /* 物理ファイル名チェック */
    wk_gfnwi_filename_len = sizeof(wk_gfnwi_filename);
    if ( (0 == wk_gfnwi_filename_len) || (48 < wk_gfnwi_filename_len) ){ /* 物理名長異常 */
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X", DEF_FL_PHSIC_INFO, wk_gfphi_pri_key, "GFNWI FILENAME ERR");
        /* 異常終了 */
        QGET_abend();
    }
    /* NW情報ファイル.物理ファイル名長 */
    wk_gfnwi_space_posi = strchr(wk_gfnwi_filename,' ');
    if (wk_gfnwi_space_posi == NULL) {
        wk_gfnwi_filename_len = sizeof(wk_gfnwi_filename);
    } else {
        wk_gfnwi_filename_len = wk_gfnwi_space_posi - wk_gfnwi_filename;
    }
    /* NW情報ファイル.物理ファイル名格納 */
    memcpy(myinfo.gfnwi_fname, wk_gfnwi_filename, wk_gfnwi_filename_len);

    /* --- --- --- --- --- --- --- */
    /* 物理名情報ファイル.クローズ */
    /* --- --- --- --- --- --- --- */
    
    /* arg2_サブプログラムステータス */
    memset(gfphi_sub_prog_sts, 0x20, sizeof(gfphi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfphi_COM_IOM_arg_3,                0x20,               sizeof(gfphi_COM_IOM_arg_3));
    memcpy(gfphi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfphi_COM_IOM_arg_3.file_id,         DEF_GFPHI,          sizeof(DEF_GFPHI) -1);
    memset(gfphi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfphi_COM_IOM_arg_3.file_name));
    memcpy(gfphi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_CLOSE,  sizeof(QGET_FILEIO_CLOSE) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfphi_COM_IOM_arg_4,            0x20,                   sizeof(gfphi_COM_IOM_arg_4));
    memcpy(gfphi_COM_IOM_arg_4.file_id,     DEF_GFPHI,              sizeof(DEF_GFPHI) -1);
    memcpy(gfphi_COM_IOM_arg_4.file_name,   myinfo.gfphi_fname,     sizeof(myinfo.gfphi_fname));
    gfphi_COM_IOM_arg_4.file_no             = myinfo.gfphi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfphi_COM_IOM_arg_5,            0x20,   sizeof(gfphi_COM_IOM_arg_5));
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
    memset(&gfphi_COM_IOM_arg_6,            0x20,   sizeof(gfphi_COM_IOM_arg_6));
    gfphi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfphi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfphi_COM_IOM_arg_6.err_proc));
    memset(gfphi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfphi_COM_IOM_arg_6.file_name));
    gfphi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfphi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfphi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_CLOSE,
        gfphi_sub_prog_sts,
        &gfphi_COM_IOM_arg_3,
        &gfphi_COM_IOM_arg_4,
        &gfphi_COM_IOM_arg_5,
        &gfphi_COM_IOM_arg_6
    );
    /* クローズ完了 */
    if (memcmp(gfphi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, "CLOSE", gfphi_COM_IOM_arg_5.key_value, gfphi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* FILE情報格納 */
    myinfo.gfphi_fnum = QGET_FILE_CLOSED;
    
    
    myinfo.gfphi_fnum = QGET_FILE_CLOSED;
    
    /* --- --- --- --- --- --- */
    /* NW情報ファイル.オープン */
    /* --- --- --- --- --- --- */
    /* arg2_サブプログラムステータス */
    memset(gfnwi_sub_prog_sts, 0x20, sizeof(gfnwi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfnwi_COM_IOM_arg_3,                0x20,               sizeof(gfnwi_COM_IOM_arg_3));
    memcpy(gfnwi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfnwi_COM_IOM_arg_3.file_id,         DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memcpy(gfnwi_COM_IOM_arg_3.file_name,       myinfo.gfnwi_fname, sizeof(myinfo.gfnwi_fname));
    memcpy(gfnwi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_OPEN,   sizeof(QGET_FILEIO_OPEN) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfnwi_COM_IOM_arg_4,            0x20,               sizeof(gfnwi_COM_IOM_arg_4));
    memcpy(gfnwi_COM_IOM_arg_4.file_id,     DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memcpy(gfnwi_COM_IOM_arg_4.file_name,   myinfo.gfnwi_fname, sizeof(myinfo.gfnwi_fname));
    gfnwi_COM_IOM_arg_4.file_no             = myinfo.gfnwi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfnwi_COM_IOM_arg_5,            0x20,   sizeof(gfnwi_COM_IOM_arg_5));
    gfnwi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gfnwi_COM_IOM_arg_5.key_value,   0x20,   sizeof(gfnwi_COM_IOM_arg_5.key_value));
    memset(gfnwi_COM_IOM_arg_5.key_type,    0x20,   sizeof(gfnwi_COM_IOM_arg_5.key_type));
    gfnwi_COM_IOM_arg_5.key_len             = 0;
    gfnwi_COM_IOM_arg_5.compare_len         = 0;
    gfnwi_COM_IOM_arg_5.positioning_mode    = 0;
    gfnwi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfnwi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfnwi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfnwi_COM_IOM_arg_5.rec_len             = 0;
    memset(gfnwi_COM_IOM_arg_5.rec_area,    0x20,   sizeof(gfnwi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfnwi_COM_IOM_arg_6,    0x20,   sizeof(gfnwi_COM_IOM_arg_6));
    gfnwi_COM_IOM_arg_6.guardian_errcode  = 0;
    memset(gfnwi_COM_IOM_arg_6.err_proc, 0x20, sizeof(gfnwi_COM_IOM_arg_6.err_proc));
    memset(gfnwi_COM_IOM_arg_6.file_name, 0x20,sizeof(gfnwi_COM_IOM_arg_6.file_name));
    gfnwi_COM_IOM_arg_6.rec_len           = 0;
    memset(gfnwi_COM_IOM_arg_6.rec_area, 0x20,sizeof(gfnwi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_OPEN,
        gfnwi_sub_prog_sts,
        &gfnwi_COM_IOM_arg_3,
        &gfnwi_COM_IOM_arg_4,
        &gfnwi_COM_IOM_arg_5,
        &gfnwi_COM_IOM_arg_6
    );
    /* オープン完了 */
    if (memcmp(gfnwi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, "OPEN", gfnwi_COM_IOM_arg_5.key_value, gfnwi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* FILE情報格納 */
    myinfo.gfnwi_fnum = gfnwi_COM_IOM_arg_4.file_no;
    
    /* --- --- --- --- --- --- --- */
    /* NW情報ファイル.NW区分取得   */
    /* --- --- --- --- --- --- --- */
    /* キー設定 */
    memset(&wk_gfnwi_pri_key,               0x20,                   sizeof(wk_gfnwi_pri_key));
    memcpy(&wk_gfnwi_pri_key.site_id,       &myinfo.site_id,        sizeof(wk_gfnwi_pri_key.site_id));
    memcpy(&wk_gfnwi_pri_key.nw_id,         &myinfo.network_id,     sizeof(wk_gfnwi_pri_key.nw_id));
    memcpy(&wk_gfnwi_pri_key.grp_id,        &myinfo.group_id,       sizeof(wk_gfnwi_pri_key.grp_id));
    memcpy(&wk_gfnwi_pri_key.if_id,         DEF_IF_ID_DEFAULT,      sizeof(DEF_IF_ID_DEFAULT) -1);
    memcpy(&wk_gfnwi_pri_key.station_id,    DEF_STATION_ID_DEFAULT, sizeof(DEF_STATION_ID_DEFAULT) -1);
    
    /* arg2_サブプログラムステータス */
    memset(gfnwi_sub_prog_sts, 0x20, sizeof(gfnwi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfnwi_COM_IOM_arg_3,                0x20,               sizeof(gfnwi_COM_IOM_arg_3));
    memcpy(gfnwi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfnwi_COM_IOM_arg_3.file_id,         DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memcpy(gfnwi_COM_IOM_arg_3.file_name,       myinfo.gfnwi_fname, sizeof(myinfo.gfnwi_fname));
    memcpy(gfnwi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_READ,   sizeof(QGET_FILEIO_READ) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfnwi_COM_IOM_arg_4,            0x20,               sizeof(gfnwi_COM_IOM_arg_4));
    memcpy(gfnwi_COM_IOM_arg_4.file_id,     DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memcpy(gfnwi_COM_IOM_arg_4.file_name,   myinfo.gfnwi_fname, sizeof(myinfo.gfnwi_fname));
    gfnwi_COM_IOM_arg_4.file_no             = myinfo.gfnwi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfnwi_COM_IOM_arg_5,            0x20,                       sizeof(gfnwi_COM_IOM_arg_5));
    gfnwi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(gfnwi_COM_IOM_arg_5.key_value,   &wk_gfnwi_pri_key,          sizeof(wk_gfnwi_pri_key));
    memcpy(gfnwi_COM_IOM_arg_5.key_type,    DEF_COM_IOM_KEYTYPE_PRI,    sizeof(DEF_COM_IOM_KEYTYPE_PRI) -1);
    gfnwi_COM_IOM_arg_5.key_len             = sizeof(wk_gfnwi_pri_key);
    gfnwi_COM_IOM_arg_5.compare_len         = sizeof(wk_gfnwi_pri_key);
    gfnwi_COM_IOM_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    gfnwi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfnwi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfnwi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfnwi_COM_IOM_arg_5.rec_len             = db_gfnwi_def_Size;
    memset(gfnwi_COM_IOM_arg_5.rec_area,    0x20,                       sizeof(gfnwi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfnwi_COM_IOM_arg_6,            0x20,   sizeof(gfnwi_COM_IOM_arg_6));
    gfnwi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfnwi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfnwi_COM_IOM_arg_6.err_proc));
    memset(gfnwi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfnwi_COM_IOM_arg_6.file_name));
    gfnwi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfnwi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfnwi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_STARTREAD,
        gfnwi_sub_prog_sts,
        &gfnwi_COM_IOM_arg_3,
        &gfnwi_COM_IOM_arg_4,
        &gfnwi_COM_IOM_arg_5,
        &gfnwi_COM_IOM_arg_6
    );
    
    /* 読み込み終了 */
    if (memcmp(gfnwi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_INFO, "READ", gfnwi_COM_IOM_arg_5.key_value, gfnwi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* 正常読み込み.読み込み情報格納 */
    memset(&gfnwi_rec,          0x20,                           sizeof(gfnwi_rec));
    memcpy(&gfnwi_rec,          &gfnwi_COM_IOM_arg_6.rec_area,  sizeof(gfnwi_rec));
    memcpy(&myinfo.nw_kubun,    &gfnwi_rec.nw_id_info.nw_kubun, sizeof(gfnwi_rec.nw_id_info.nw_kubun));
    
    /* --- --- --- --- --- --- */
    /* NW情報ファイル.クローズ */
    /* --- --- --- --- --- --- */
    /* arg2_サブプログラムステータス */
    memset(gfnwi_sub_prog_sts, 0x20, sizeof(gfnwi_sub_prog_sts));
    
    /* arg3_トレース情報 */
    memset(&gfnwi_COM_IOM_arg_3,                0x20,               sizeof(gfnwi_COM_IOM_arg_3));
    memcpy(gfnwi_COM_IOM_arg_3.prog_id,         myinfo.module_id,   sizeof(myinfo.module_id));
    memcpy(gfnwi_COM_IOM_arg_3.file_id,         DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memset(gfnwi_COM_IOM_arg_3.file_name,       0x20,               sizeof(gfnwi_COM_IOM_arg_3.file_name));
    memcpy(gfnwi_COM_IOM_arg_3.file_io_type,    QGET_FILEIO_CLOSE,  sizeof(QGET_FILEIO_CLOSE) -1);
    
    /* arg4_ファイル情報 */
    memset(&gfnwi_COM_IOM_arg_4,            0x20,               sizeof(gfnwi_COM_IOM_arg_4));
    memcpy(gfnwi_COM_IOM_arg_4.file_id,     DEF_GFNWI,          sizeof(DEF_GFNWI) -1);
    memcpy(gfnwi_COM_IOM_arg_4.file_name,   myinfo.gfnwi_fname, sizeof(myinfo.gfnwi_fname));
    gfnwi_COM_IOM_arg_4.file_no             = myinfo.gfnwi_fnum;
    
    /* arg5_入力情報 */
    memset(&gfnwi_COM_IOM_arg_5,            0x20,   sizeof(gfnwi_COM_IOM_arg_5));
    gfnwi_COM_IOM_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_position   = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_COM_IOM_arg_5.part_key_len        = DEF_COM_IOM_PARTITION_KEY_NOT;
    memset(gfnwi_COM_IOM_arg_5.key_value,   0x20,   sizeof(gfnwi_COM_IOM_arg_5.key_value));
    memset(gfnwi_COM_IOM_arg_5.key_type,    0x20,   sizeof(gfnwi_COM_IOM_arg_5.key_type));
    gfnwi_COM_IOM_arg_5.key_len             = 0;
    gfnwi_COM_IOM_arg_5.compare_len         = 0;
    gfnwi_COM_IOM_arg_5.positioning_mode    = 0;
    gfnwi_COM_IOM_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    gfnwi_COM_IOM_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    gfnwi_COM_IOM_arg_5.io_timer            = myinfo.file_timer;
    gfnwi_COM_IOM_arg_5.rec_len             = 0;
    memset(gfnwi_COM_IOM_arg_5.rec_area,    0x20,   sizeof(gfnwi_COM_IOM_arg_5.rec_area));
    
    /* arg6_出力情報 */
    memset(&gfnwi_COM_IOM_arg_6,            0x20,   sizeof(gfnwi_COM_IOM_arg_6));
    gfnwi_COM_IOM_arg_6.guardian_errcode    = 0;
    memset(gfnwi_COM_IOM_arg_6.err_proc,    0x20,   sizeof(gfnwi_COM_IOM_arg_6.err_proc));
    memset(gfnwi_COM_IOM_arg_6.file_name,   0x20,   sizeof(gfnwi_COM_IOM_arg_6.file_name));
    gfnwi_COM_IOM_arg_6.rec_len             = 0;
    memset(gfnwi_COM_IOM_arg_6.rec_area,    0x20,   sizeof(gfnwi_COM_IOM_arg_6.rec_area));
    
    /* 共通モジュール */
    COM_IOM (DEF_COM_IOM_FUNC_CLOSE,
        gfnwi_sub_prog_sts,
        &gfnwi_COM_IOM_arg_3,
        &gfnwi_COM_IOM_arg_4,
        &gfnwi_COM_IOM_arg_5,
        &gfnwi_COM_IOM_arg_6
    );
    /* クローズ完了 */
    if (memcmp(gfnwi_sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_PHSIC_INFO, "CLOSE", gfnwi_COM_IOM_arg_5.key_value, gfnwi_COM_IOM_arg_6.guardian_errcode);
        /* 異常終了 */
        QGET_abend();
    }
    /* FILE情報格納 */
    myinfo.gfnwi_fnum = QGET_FILE_CLOSED;
    

} /* end of QGET_get_config */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_main                                      */
/*  CALLING SEQ.    : void QGET_main (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void QGET_main(void)
{
    /* 初期化(I/O完了情報) */
    iocomp.fno = -1;
    iocomp.addr = 0;
    iocomp.len = 0;
    iocomp.tag = -1L;
    iocomp.ferr = 0L;
    memset((char *)&iocomp.recv_info, 0x00, sizeof(iocomp.recv_info));
    
    short   wk_err_cd = 0;
    short   wk_result = 0;
    long    wk_naibu_tranid = 0;

    memset(naibu_tranid, 0x20, sizeof(naibu_tranid));


    /* AWAITIOX */
    AWAITIOX (&iocomp.fno, (long *)&iocomp.addr, (unsigned short *)&iocomp.len);
    /* AWAITIOX結果.ファイル番号判定 */
    if (iocomp.fno == myinfo.recv_fno) {
        
        /* $RECEIVE READUPDATE完了後処理 */
        FILE_GETRECEIVEINFO_((short _far *)&iocomp.recv_info);
        FILE_GETINFO_(myinfo.recv_fno, &iocomp.ferr);
        /* エラーコード判定 */
        switch(iocomp.ferr) {
        case 0 :
            QGET_send_reply(resp_buf, 0, 0);
            break;
        case 6 :
            /* システムメッセージ処理 */
            wk_err_cd = COM_STP_JUDGE(&openersinfo, sys_msg);
            
            /* 戻り値異常 */
            if(wk_err_cd < 0){
                /* メッセージ出力 */
                QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_PROC_OPN_ERR, "@R@e", procinfo.my_pname, wk_err_cd);
                /* 異常終了 */
                QGET_abend();
            }
            /* クローズ判定 */
            if (wk_err_cd == 1){
                myinfo.end_flg = 1;
            }
            /* リプライ */
            QGET_send_reply(resp_buf, 0, 0);
            break;
        default:
            /* 異常終了 */
            QGET_abend();
        }
        
        /* $RECEIVE READUPDATE */
        READUPDATEX (
            myinfo.recv_fno,
            recv_buf,
            (unsigned short)sizeof(recv_buf),
            (unsigned short *)&iocomp.len);
    }else{
        
        /* キューファイル READUPDATELOCK完了後処理 */
        QGET_qfile_read();
        if (myinfo.read_end_flg == QGET_ON) {
            /* キューファイル読み込み正常 */
            QGET_req_send();
        }
        /* 読み込み完了フラグ.初期化 */
        myinfo.read_end_flg = QGET_OFF;
        
        /* BIGINTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        wk_result = COM_TMF(DEF_COM_TMF_BEGIN, &wk_naibu_tranid, myinfo.module_id);
        if (wk_result != QGET_RET_OK) {
            /* メッセージ出力 */
            QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", wk_result);
            /* 異常終了 */
            QGET_abend();
            
        }
        
        /* $QUEUE-FILE READUPDATELOCK */
        READUPDATELOCKX(qfileinfo.fnum
                       ,qfile_read_buf
                       ,db_gqnwq_def_Size
                       ,(unsigned short *)&iocomp.len);
    }
    
} /* end of QGET_main */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_send_reply                                */
/*  CALLING SEQ.    : void QGET_send_reply (char*, short, short)            */
/*  ARGUMENT        : IN リプライバッファ                                   */
/*                    IN リプライレングス                                   */
/*                    IN リプライコード                                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void QGET_send_reply (
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
    long long               wk_begin_datetime_l = 0;
    long long               wk_end_datetime_l = 0;
    
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
        reply_len,,,
        reply_cd);

    if (_status_ne(wk_cc)) {
        FILE_GETINFO_(myinfo.recv_fno, &iocomp.ferr);
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@X@E", "REPLYX", iocomp.ferr);
        /* 異常終了 */
        QGET_abend();
    }

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_myinfo_trc_time_end, &reply_end_COM_SDT_arg_3, &wk_end_datetime_l);
    


} /* end of QGET_send_reply */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_req_send                                  */
/*  CALLING SEQ.    : void QGET_req_send (void)                             */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求送信処理                                          */
/****************************************************************************/
void QGET_req_send (void)
{
    short   wk_result       = 0;            /* wk_完了フラグ */
    long    wk_naibu_tranid = 0;
    
    /* キュー取り出し通知要求用PATHSENDモジュールパラメータ */
    COM_PSD_arg_1_def   c302_COM_PSD_arg_1;
    COM_PSD_arg_2_def   c302_COM_PSD_arg_2;
    COM_PSD_arg_3_def   c302_COM_PSD_arg_3;
    COM_PSD_arg_4_def   c302_COM_PSD_arg_4;
    
    memset(&c302_COM_PSD_arg_1, 0x20, sizeof(c302_COM_PSD_arg_1));
    memset(&c302_COM_PSD_arg_2, 0x20, sizeof(c302_COM_PSD_arg_2));
    memset(&c302_COM_PSD_arg_3, 0x20, sizeof(c302_COM_PSD_arg_3));
    memset(&c302_COM_PSD_arg_4, 0x20, sizeof(c302_COM_PSD_arg_4));

    /* IPC編集 */
    memcpy(req_msg->common_header.interface_code, DEF_IPC_IFCD_Q_GET_NT_REQ, sizeof(req_msg->common_header.interface_code));
    req_msg->common_header.error_code = 0;
    memcpy(req_msg->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL));
    memset(req_msg->common_header.filler_1, 0x20, sizeof(req_msg->common_header.filler_1));
	req_msg->common_header.control_data_length = iocomp.len;
    
    memcpy(req_msg->msg_data, &qfile_read_buf, iocomp.len);

    /* PATHSEND処理 */
    
    /* PATHSEND共通モジュールパラメータ */
    /* arg1 */
    memcpy(c302_COM_PSD_arg_1.pathmon_name, myinfo.psend_pathmon_name, sizeof(myinfo.psend_pathmon_name));
    memcpy(c302_COM_PSD_arg_1.serverclass_name, myinfo.psend_serverclass_name, sizeof(myinfo.psend_serverclass_name));
    memcpy(c302_COM_PSD_arg_1.msg_buf, &send_buf, sizeof(req_msg->common_header) + iocomp.len);
    c302_COM_PSD_arg_1.req_send_len = sizeof(req_msg->common_header) + iocomp.len;
    c302_COM_PSD_arg_1.receive_max_len = sizeof(r302_def);
    c302_COM_PSD_arg_1.send_timer_msec = myinfo.send_timer;
    c302_COM_PSD_arg_1.retry_cnt = myinfo.send_retry;
    /* arg2 */
    memcpy(&c302_COM_PSD_arg_2.prog_id, &myinfo.module_id, sizeof(c302_COM_PSD_arg_2.prog_id));
    /* arg3 */
    c302_COM_PSD_arg_3.guardian_errcode = 0;
    c302_COM_PSD_arg_3.pathsend_errcode = 0;
    /* arg4 */
    /* 運用監視端末出力情報 ->QGET_get_paramで事前設定 */
    /* EMS出力情報.業務共通メッセージ */
    memset(&cg010in.emsinf,                 0x20,                   sizeof(cg010in.emsinf));
    memcpy(cg010in.emsinf.emsgkinf.sysnm,   DEF_EMS_SYSNM_GFP,      sizeof(cg010in.emsinf.emsgkinf.sysnm));
    memcpy(cg010in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM,    sizeof(cg010in.emsinf.emsgkinf.srv_kbn));
    memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn,myinfo.nw_kubun,        sizeof(myinfo.nw_kubun));
    memcpy(cg010in.emsinf.emsgkinf.prgid,   DEF_GFPCVX60,           sizeof(cg010in.emsinf.emsgkinf.prgid));
    memcpy(cg010in.emsinf.emsgkinf.trmnm,   procinfo.my_pname,      sizeof(procinfo.my_pname));
    
    /* arg5 */
    memcpy(&c302_COM_PSD_arg_4.srv_logical_id,  myinfo.serverclass_kind,    sizeof(myinfo.serverclass_kind));
    
    /* PATHSEND共通モジュール */
    wk_result = COM_PSD(&c302_COM_PSD_arg_1, &c302_COM_PSD_arg_2, &c302_COM_PSD_arg_3, &cg010in, &c302_COM_PSD_arg_4);
    if (wk_result != QGET_RET_OK) {
        /* リトライディレイ */
        DELAY ( myinfo.abort_delay_timer );
        /* ABORTTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
        return;
    }
    
    memcpy(rsp_msg, &c302_COM_PSD_arg_1.msg_buf, c302_COM_PSD_arg_1.receive_len);

    /* REPLYメッセージ判定 */
    if (memcmp(rsp_msg->common_header.interface_code, DEF_IPC_IFCD_Q_GET_NT_RSP, sizeof(req_msg->common_header.interface_code)) != 0) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@L@S@X@i", "", myinfo.psend_serverclass_name, "ｱﾝｻﾎﾟｰﾄIPC", resp_buf);
        /* リトライディレイ */
        DELAY ( myinfo.abort_delay_timer );
        /* ABORTTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
        return;
    }
    
    /* キュー取り出し通知応答.共通ヘッダ.エラーコード判定 */
    if (rsp_msg->common_header.error_code == 0) {
        /* ENDTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        wk_result = COM_TMF(DEF_COM_TMF_END, &wk_naibu_tranid, myinfo.module_id);
        if (wk_result != QGET_RET_OK) {
            /* メッセージ出力 */
            QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", wk_result);
            /* 異常終了 */
            QGET_abend();
        }
    }else{
        /* リトライディレイ */
        DELAY ( myinfo.abort_delay_timer );
        /* ABORTTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
        return;
    }

} /* end of QGET_req_send */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_qfile_open                                */
/*  CALLING SEQ.    : void QGET_qfile_open (char)                           */
/*  ARGUMENT        : IN 物理ファイル名                                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : キューファイルオープン処理                            */
/****************************************************************************/
void QGET_qfile_open()
{
    char    wk_fname[48];
    long    wk_fname_len    = 0;
    short   wk_fnum         = -1;
    short   wk_err_code     = 0;
    char    *wk_space_posi  = 0;
    _cc_status  wk_cc;
    lk_zac2001f_arg_1_def file_trace;
    memset(&file_trace, 0x20, lk_zac2001f_arg_1_def_Size);
    
    short   sdt_func_type   = 0;
    COM_SDT_arg_2_def       wk_trc_time_begin;
    COM_SDT_arg_2_def       wk_trc_time_end;
    COM_SDT_arg_3_def       qopen_begin_COM_SDT_arg_3;
    COM_SDT_arg_3_def       qopen_end_COM_SDT_arg_3;
    long long               wk_begin_datetime_l = 0;
    long long               wk_end_datetime_l = 0;
    
    char wk_text[10];
    memset(&wk_text, 0x20, sizeof(wk_text));
    
    
    memset(&wk_trc_time_begin, 0x20, sizeof(wk_trc_time_begin));
    memset(&wk_trc_time_end, 0x20, sizeof(wk_trc_time_end));
    memset(&qopen_begin_COM_SDT_arg_3, 0x20, sizeof(qopen_begin_COM_SDT_arg_3));
    memset(&qopen_end_COM_SDT_arg_3, 0x20, sizeof(qopen_end_COM_SDT_arg_3));
    
    memset(&wk_fname, 0x20, sizeof(wk_fname));
    memset(wk_fname, 0x20, sizeof(wk_fname));

    memcpy(wk_fname, qfileinfo.fname, sizeof(qfileinfo.fname));
    wk_space_posi = strchr(wk_fname,' ');
    if (wk_space_posi == NULL) {
        wk_fname_len = sizeof(qfileinfo.fname);
    } else {
        wk_fname_len = wk_space_posi - wk_fname;
    }

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_trc_time_begin, &qopen_begin_COM_SDT_arg_3, &wk_begin_datetime_l);
    /* FILE_OPEN_ */
    wk_err_code = FILE_OPEN_(wk_fname, (short)wk_fname_len, &wk_fnum,,,1); /* NOWAIT */
    if (wk_fnum == QGET_FILE_CLOSED) {
        iocomp.ferr = wk_err_code;
    } else {
        iocomp.ferr = 0;
    }

    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_trc_time_end, &qopen_end_COM_SDT_arg_3, &wk_end_datetime_l);
    /* トレース出力初期処理 */
    memset((char *)&file_trace, 0x20, sizeof(file_trace));
    file_trace.func_flg = '1';
    memcpy((char *)&file_trace.trace_info.prog_id, myinfo.module_id
                                                ,sizeof(myinfo.module_id));
    memcpy((char *)&file_trace.trace_info.file_id, QGET_FILE_LOGI_NAME
                                                ,sizeof(QGET_FILE_LOGI_NAME) -1);
    memcpy((char *)&file_trace.trace_info.file_name, qfileinfo.fname
                                                ,sizeof(qfileinfo.fname));
    memcpy((char *)&file_trace.trace_info.file_io_type, QGET_FILEIO_OPEN
                                                ,sizeof(QGET_FILEIO_OPEN) -1);
    sprintf(wk_text,"%04d", iocomp.ferr);
    memcpy((char *)&file_trace.data_info.guardian_errcode,wk_text
                                                ,sizeof(wk_text));
    memcpy((char *)&file_trace.data_info.shori_start_time,&wk_trc_time_begin.hh
                                                ,sizeof(file_trace.data_info.shori_start_time));
    memcpy((char *)&file_trace.data_info.shori_end_time,&wk_trc_time_end.hh
                                                ,sizeof(file_trace.data_info.shori_end_time));
    TRACEOUT((char *)&file_trace);

    /* WK_エラーコードの判定 */
    if (wk_err_code != QGET_RET_OK) {
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_QUE    , "OPEN", "", wk_err_code);
        /* 異常終了 */
        QGET_abend();
        
    }

    /* キュー読み込み完了待ちタイマ設定 */
    short *s_wait_timer = (short *)&myinfo.qfile_read_wait_timer;
    wk_cc = SETMODE(wk_fnum
                     ,128 /* Queue files timeout periods */
                     ,s_wait_timer[0]
                     ,s_wait_timer[1]
                     );
    
    if (_status_ne(wk_cc)) {
        FILE_GETINFO_(myinfo.recv_fno, &iocomp.ferr);
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@X@E", "SETMODE", iocomp.ferr);

        /* 異常終了 */
        QGET_abend();
    }
    
    /*キューファイル情報更新*/
    memcpy(qfileinfo.fname, wk_fname, wk_fname_len);
    qfileinfo.fnum    = wk_fnum;
    
} /* end of QGET_qfile_open */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_qfile_read                                */
/*  CALLING SEQ.    : short QGET_qfile_read (void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : キューファイル読み込みエラーコード                    */
/*  DESCRIPTION     : キューファイル読み込み処理                            */
/****************************************************************************/
void QGET_qfile_read (void)
{
    long    wk_naibu_tranid = 0;
    short   wk_result = 0;
    short   wk_err_code = 0;
    long long   wk_timestamp = 0LL;
    long long   *wk_ll;
    char    wk_c_timestamp[18+1];
    char    wk_head_timestamp[10+1];
    db_gqnwq_def qfile_rec;
    db_glelg_def glelg_rec;
    char    wk_ch_len[8];
    char    ch_dst_text[24+1];
    short   wk_denbun_len;
    char    ch_wklcn[32];
    char    ch_wkdat[64];
    
    memset(&qfile_rec, 0x20, sizeof(qfile_rec));
    memset(&glelg_rec, 0x20, sizeof(glelg_rec));
    memset(&timeup_COM_ERL_arg_2, 0x20, sizeof(timeup_COM_ERL_arg_2));
    
    memset(&wk_c_timestamp, 0x20, sizeof(wk_c_timestamp));
    memset(&wk_head_timestamp, 0x20, sizeof(wk_head_timestamp));
    memset(&wk_ch_len, 0x20, sizeof(wk_ch_len));
    memset(&ch_dst_text, 0x20, sizeof(ch_dst_text));

    /* キューファイル.エラーコード判定 */
    FILE_GETINFO_(qfileinfo.fnum, &wk_err_code);
    switch (wk_err_code) {
    case 0:
        memcpy(&qfile_rec, &qfile_read_buf, sizeof(qfile_read_buf));
        break;
    case 162:
        /* ABORTTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
        return;
    case 200:
    case 201:
    case 210:
    case 211:
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_QFILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_QUE, "READUPDATELOCKX", "", wk_err_code);
        
        /* ABORTTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
        /* キューファイル再オープン */
        QGET_qfile_close();
        QGET_qfile_open();
        return;
    case 48:
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_QFILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_QUE, "READUPDATELOCKX", "", wk_err_code);
        /* 正常終了 */
        QGET_finish();
    default:
        /* メッセージ出力 */
        QGET_message_output(DEF_EVT_QFILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_NW_QUE, "READUPDATELOCKX", "", wk_err_code);
        /* 異常終了 */
        QGET_abend();
    }

    /* 送信期限切れ判定 */
    wk_timestamp = JULIANTIMESTAMP();
    wk_ll = (long long *)&qfile_rec.pri_key.key_time_stamp[0];
    if (wk_timestamp - *wk_ll <= (myinfo.expiry_second *1000000)) {
        /* 読み込み正常終了 */
        myinfo.read_end_flg = QGET_ON;
        return;
    }else{
        /* 送信期限切れエラー出力 */
        
        /* エラー出力ログレコード編集 */
        
        glelg_rec.err_denbun_id = DEF_ELG_INTERNAL;                                                             /* エラー電文識別 */
        memcpy(&glelg_rec.mti_id, qfile_rec.denbun_send_recv_info.mti_id, sizeof(glelg_rec.mti_id));            /* MTI */
        memcpy(&glelg_rec.naibu_err_code, DEF_NERR_QGET_EXPIRE, strlen(DEF_NERR_QGET_EXPIRE));                  /* 内部エラーコード */
        memcpy(&glelg_rec.srv_cls_info.srv_cls_id, myinfo.serverclass_name, sizeof(myinfo.serverclass_name));   /* サーバクラス論理ID */
        memcpy(&glelg_rec.srv_cls_info.srv_cls_mlt_num, QGET_NON_MLT_NUM, strlen(QGET_NON_MLT_NUM));            /* サーバクラス冗長化番号 */
        memcpy((char *)&glelg_rec.denbun_send_recv_info, (char *)&qfile_rec.denbun_send_recv_info, sizeof(glelg_rec.denbun_send_recv_info));
        memcpy((char *)&glelg_rec.tushin_cntrl_info, (char *)&qfile_rec.tushin_cntrl_info, sizeof(glelg_rec.tushin_cntrl_info));
        if (iocomp.len > MAX_TEXT_BUF_LEN) {
            /* エラー出力ログ・電文部・電文レコードサイズ超過 */
            sprintf(wk_ch_len, "%05d", MAX_TEXT_BUF_LEN);
            memcpy(glelg_rec.denbun_area.denbun_len, &wk_ch_len, sizeof(glelg_rec.denbun_area.denbun_len));     /* 電文長(9999) */
            memcpy(glelg_rec.denbun_area.denbun, &qfile_read_buf, MAX_TEXT_BUF_LEN);                            /* 電文(9999桁まで) */
        }else{
            /* エラー出力ログ・電文部・電文レコードサイズ以内 */
            memset(glelg_rec.denbun_area.denbun_len, '0', sizeof(glelg_rec.denbun_area.denbun_len));            /* 電文長 */
            memcpy(&glelg_rec.denbun_area.denbun_len[1], qfile_rec.denbun_area.denbun_len, sizeof(qfile_rec.denbun_area.denbun_len));
            memset(glelg_rec.denbun_area.mti_start_lct, '0', sizeof(glelg_rec.denbun_area.mti_start_lct));      /* MTI開始位置 */
            memcpy(wk_ch_len, qfile_rec.denbun_area.denbun_len, sizeof(qfile_rec.denbun_area.denbun_len));
            wk_denbun_len = (short)atoi(wk_ch_len);
            memcpy(&glelg_rec.denbun_area.mti_start_lct[1], qfile_rec.denbun_area.mti_start_lct, sizeof(qfile_rec.denbun_area.mti_start_lct));
            memcpy(glelg_rec.denbun_area.denbun, qfile_rec.denbun_area.denbun, wk_denbun_len);
        }
        
        /* --- --- --- --- --- --- --- --- --- --- */
        /* エラー出力ログ編集出力モジュール(WRITE) */
        /* --- --- --- --- --- --- --- --- --- --- */
        /* arg1.入力情報 */
        timeup_COM_ERL_arg_1.file_io_type       = DEF_COM_ERL_ARG1_WRITE;
        timeup_COM_ERL_arg_1.io_timer           = myinfo.file_timer;
        if (iocomp.len > MAX_TEXT_BUF_LEN) {
            /* エラー出力ログ・電文部・電文レコードサイズ超過 */
            timeup_COM_ERL_arg_1.data_len           = db_glelg_def_Size - sizeof(glelg_rec.denbun_area.denbun) + MAX_TEXT_BUF_LEN;
        }else{
            /* エラー出力ログ・電文部・電文レコードサイズ以内 */
            timeup_COM_ERL_arg_1.data_len           = db_glelg_def_Size - sizeof(glelg_rec.denbun_area.denbun) + iocomp.len;
        }
        timeup_COM_ERL_arg_1.data_area          = (char *)&glelg_rec;
        
        /* arg2.エラー出力ログファイル情報 */
        memset(&timeup_COM_ERL_arg_2,           0x20,               sizeof(timeup_COM_ERL_arg_2));
        memcpy(timeup_COM_ERL_arg_2.file_name,  myinfo.erl_fname,   sizeof(myinfo.erl_fname));
        timeup_COM_ERL_arg_2.file_no            = myinfo.erl_fnum;
        
        /* arg3.EMS出力共通情報 */
        /* 運用監視端末出力情報 ->QGET_get_paramで事前設定 */
        /* EMS出力情報.業務共通メッセージ */
        memset(&cg010in.emsinf,                 0x20,                   sizeof(cg010in.emsinf));
        memcpy(cg010in.emsinf.emsgkinf.sysnm,   DEF_EMS_SYSNM_GFP,      sizeof(cg010in.emsinf.emsgkinf.sysnm));
        memcpy(cg010in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM,    sizeof(cg010in.emsinf.emsgkinf.srv_kbn));
        memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn,myinfo.nw_kubun,        sizeof(myinfo.nw_kubun));
        memcpy(cg010in.emsinf.emsgkinf.prgid,   DEF_GFPCVX60,           sizeof(cg010in.emsinf.emsgkinf.prgid));
        memcpy(cg010in.emsinf.emsgkinf.trmnm,   procinfo.my_pname,      sizeof(procinfo.my_pname));
        
        /* arg4.EMS出力付加情報 */
        memset(&timeup_COM_ERL_arg_3,                   0x20,                       sizeof(timeup_COM_ERL_arg_3));
        memcpy(&timeup_COM_ERL_arg_3.srv_logical_id,    myinfo.serverclass_kind,    sizeof(myinfo.serverclass_kind));
        
        wk_err_code = COM_ERL(&timeup_COM_ERL_arg_1, &timeup_COM_ERL_arg_2, &cg010in, &timeup_COM_ERL_arg_3, DEF_GFPCVX60);
        if (wk_err_code != 0) {
            /* メッセージ出力 */
            QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E", "", "", DEF_FL_ERR_LOG, "WRITE", "", wk_err_code);
            /* 異常終了 */
            QGET_abend();
        }
        /* メッセージ出力 */
        memset(ch_wklcn, 0x00, sizeof(ch_wklcn));
        memset(ch_wkdat, 0x00, sizeof(ch_wkdat));
        memcpy(ch_wklcn, 
            (char *)&qfile_rec.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id, 
            sizeof(qfile_rec.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id));
        memcpy(ch_wkdat, (char *)&qfile_rec.cntrl_info, sizeof(qfile_rec.cntrl_info));
        QGET_message_output(DEF_EVT_DENBUN_HAKI, 'E', DEF_NERR_QGET_EXPIRE, "@L@X@X", ch_wklcn, "DENBUN-EXPIRY", ch_wkdat);

        /* ENDTRANSACTION */
        wk_naibu_tranid =  atol(naibu_tranid);
        wk_result = COM_TMF(DEF_COM_TMF_END, &wk_naibu_tranid, myinfo.module_id);
        if (wk_result != QGET_RET_OK) {
            /* メッセージ出力 */
            QGET_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", wk_result);
            /* 異常終了 */
            QGET_abend();
        }
    }
}
/* end of QGET_qfile_read */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_qfile_close                               */
/*  CALLING SEQ.    : void QGET_qfile_close (void)                          */
/*  ARGUMENT        : IN ファイル番号                                       */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : キューファイルクローズ処理                            */
/****************************************************************************/
void QGET_qfile_close()
{
    short   wk_err_code = 0;
    short   sdt_func_type   = 0;
    COM_SDT_arg_2_def       wk_trc_time_begin;
    COM_SDT_arg_2_def       wk_trc_time_end;
    COM_SDT_arg_3_def       qclose_begin_COM_SDT_arg_3;
    COM_SDT_arg_3_def       qclose_end_COM_SDT_arg_3;
    long long               wk_begin_datetime_l = 0;
    long long               wk_end_datetime_l = 0;
    
    char wk_text[10];
    memset(&wk_text, 0x20, sizeof(wk_text));
    
    
    lk_zac2001f_arg_1_def file_trace;
    memset(&file_trace, 0x20, lk_zac2001f_arg_1_def_Size);
    
    memset(&wk_trc_time_begin, 0x20, sizeof(wk_trc_time_begin));
    memset(&wk_trc_time_end, 0x20, sizeof(wk_trc_time_end));
    memset(&qclose_begin_COM_SDT_arg_3, 0x20, sizeof(qclose_begin_COM_SDT_arg_3));
    memset(&qclose_end_COM_SDT_arg_3, 0x20, sizeof(qclose_end_COM_SDT_arg_3));
    
    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_trc_time_begin, &qclose_begin_COM_SDT_arg_3, &wk_begin_datetime_l);
    /* FILE_CLOSE_ */
    wk_err_code = FILE_CLOSE_(qfileinfo.fnum);
    if (wk_err_code != QGET_RET_OK) {
        /* 異常終了 */
        QGET_abend();
    }
    
    /* システム日時取得 */
    sdt_func_type = 2;
    COM_SDT(sdt_func_type, &wk_trc_time_end, &qclose_end_COM_SDT_arg_3, &wk_end_datetime_l);
    /* トレース出力処理 */
    memset((char *)&file_trace, 0x20, sizeof(file_trace));
    file_trace.func_flg = '1';
    memcpy((char *)&file_trace.trace_info.prog_id, myinfo.module_id
                                                ,sizeof(myinfo.module_id));
    memcpy((char *)&file_trace.trace_info.file_id, QGET_FILE_LOGI_NAME
                                                ,sizeof(QGET_FILE_LOGI_NAME) -1);
    memcpy((char *)&file_trace.trace_info.file_name, qfileinfo.fname
                                                ,sizeof(qfileinfo.fname));
    memcpy((char *)&file_trace.trace_info.file_io_type, QGET_FILEIO_CLOSE
                                                ,sizeof(QGET_FILEIO_CLOSE) -1);
    sprintf(wk_text,"%04d", wk_err_code);
    memcpy((char *)&file_trace.data_info.guardian_errcode,wk_text
                                                ,sizeof(wk_text));
    memcpy((char *)&file_trace.data_info.shori_start_time,&wk_trc_time_begin.hh
                                                ,sizeof(file_trace.data_info.shori_start_time));
    memcpy((char *)&file_trace.data_info.shori_end_time,&wk_trc_time_end.hh
                                                ,sizeof(file_trace.data_info.shori_end_time));
    TRACEOUT((char *)&file_trace);

    /* キューファイル情報の更新 */
    qfileinfo.fnum = -1;
    qfileinfo.ferr =  0;

} /* end of QGET_qfile_close */

/****************************************************************************/
/*  FUNCTION        : x.x.0  QGET_message_output                            */
/*  CALLING SEQ.    : void QGET_message_output( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ出力処理                                    */
/****************************************************************************/
void QGET_message_output(
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
    /* QGET_get_paramで事前設定 */

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
        DEF_GFPCVX60, sizeof(cg010in.emsinf.emsgkinf.prgid));
    
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
    if (myinfo.serverclass_name[0] != 0x00) {
        memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
            myinfo.serverclass_name, 12);
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
        case 'S':       /* サーバークラス論理ID(送信先) */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, strlen(pch_ep));
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
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
            s_var = (short)va_arg(va_ap, short);
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
    
} /* QGET_message_output */


/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_finish                                    */
/*  CALLING SEQ.    : void QGET_finish (void)                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void QGET_finish (void)
{
    short   wk_err_code     = 0;
    long    wk_naibu_tranid = 0;
    
    /* ABORTTRANSACTION */
    wk_naibu_tranid =  atol(naibu_tranid);
    COM_TMF(DEF_COM_TMF_ABORT, &wk_naibu_tranid, myinfo.module_id);
    
    /* キューファイルクローズ処理 */
    QGET_qfile_close();

    /* --- --- --- --- --- --- --- --- --- --- */
    /* エラー出力ログ編集出力モジュール(CLOSE) */
    /* --- --- --- --- --- --- --- --- --- --- */
    /* arg1.入力情報 */
    timeup_COM_ERL_arg_1.file_io_type   = DEF_COM_ERL_ARG1_CLOSE;
    timeup_COM_ERL_arg_1.io_timer       = myinfo.file_timer;
    
    /* arg2.エラー出力ログファイル情報 */
    memset(&timeup_COM_ERL_arg_2,           0x20,               sizeof(timeup_COM_ERL_arg_2));
    memcpy(timeup_COM_ERL_arg_2.file_name,  myinfo.erl_fname,   sizeof(myinfo.erl_fname));
    timeup_COM_ERL_arg_2.file_no            = myinfo.erl_fnum;
    
    /* arg3.EMS出力共通情報 */
    /* 運用監視端末出力情報 ->QGET_get_paramで事前設定 */
    /* EMS出力情報.業務共通メッセージ */
    memset(&cg010in.emsinf,                 0x20,                   sizeof(cg010in.emsinf));
    memcpy(cg010in.emsinf.emsgkinf.sysnm,   DEF_EMS_SYSNM_GFP,      sizeof(cg010in.emsinf.emsgkinf.sysnm));
    memcpy(cg010in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM,    sizeof(cg010in.emsinf.emsgkinf.srv_kbn));
    memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn,myinfo.nw_kubun,        sizeof(myinfo.nw_kubun));
    memcpy(cg010in.emsinf.emsgkinf.prgid,   DEF_GFPCVX60,           sizeof(cg010in.emsinf.emsgkinf.prgid));
    memcpy(cg010in.emsinf.emsgkinf.trmnm,   procinfo.my_pname,      sizeof(procinfo.my_pname));
    
    /* arg4.EMS出力付加情報 */
    memset(&timeup_COM_ERL_arg_3,                   0x20,                       sizeof(timeup_COM_ERL_arg_3));
    memcpy(&timeup_COM_ERL_arg_3.srv_logical_id,    myinfo.serverclass_kind,    sizeof(myinfo.serverclass_kind));
    
    wk_err_code = COM_ERL(&timeup_COM_ERL_arg_1, &timeup_COM_ERL_arg_2, &cg010in, &timeup_COM_ERL_arg_3, DEF_GFPCVX60);
    if (wk_err_code != 0) {
        /* 異常終了 */
        QGET_abend();
    }

    /* トレース終了処理 */
    memset((char *)&g_trc.recv, 0x20, sizeof(g_trc.recv));
    g_trc.recv.func_flg = FUNC_END;
    memcpy(g_trc.recv.trace_info.prog_id, myinfo.module_id, sizeof(myinfo.module_id));
    memset(g_trc.recv.trace_info.guardian_errcode, '0', sizeof(char));
    memset(g_trc.recv.data_info.rec_len, '0', sizeof(char));
    TRACEOUT((char *)&g_trc.recv);

    /* メッセージ出力 */
    QGET_message_output(DEF_EVT_PROC_NORMAL_END, '*', DEF_NERR_NOMAL, "@R", procinfo.my_pname);

    
    /* PROCESS_STOP */
    PROCESS_STOP_(,, QGET_NORMAL_TERMINATION);

} /* end of QGET_finish */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  QGET_abend                                     */
/*  CALLING SEQ.    : void QGET_abend (void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void QGET_abend (void)
{
    /* メッセージ出力 */
    QGET_message_output(DEF_EVT_PROC_ABNORMAL_END, 'E', DEF_NERR_NOMAL, "@R", procinfo.my_pname);

    /* PROCESS_ABEND */
    PROCESS_STOP_(,, QGET_ABNORMAL_TERMINATION);

} /* end of QGET_abend */

