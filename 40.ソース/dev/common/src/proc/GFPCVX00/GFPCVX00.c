/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX00                                    */
/*        FUNCTION          ････ リスナー                                    */
/*                               サーバ接続機能/リスナー機能                 */
/*                                                                           */
/*        AUTHER            ････ HAS M.Matsumoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-10-18                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2024/10/19 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
#pragma ENV COMMON

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <systype.h> nolist
#include <sys/socket.h> nolist
#include <errno.h> nolist
#include <netinet/in.h> nolist
#include <netdb.h> nolist
#include <string.h> nolist
#include <stdlib.h> nolist
#include <stdarg.h> nolist
#include <arpa/inet.h> nolist
#include <tal.h> nolist
#include <ctype.h> nolist
#include <cextdecs.h> nolist
#include <zsysc> nolist
#include <zspic> nolist

/* USER HEADER     */
#include "GFPCVX00.h"

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/
/* ------------------------------------------------------------------------ */
/*   グローバル変数定義                                                     */
/* ------------------------------------------------------------------------ */
myinfo_def      myinfo;                             /* 自プロセス情報テーブル                   */
short           s_exit_flag;                        /* 終了条件フラグ                           */
short           s_event;                            /* リスナーイベント                         */

iocmp_def       iocmp;                              /* I/O完了情報                              */
short           s_rcv_fd    = LSTN_FILE_CLOSED;     /* $RECEIVEファイル番号                     */
short           s_rcv_err   = 0;                    /* $RECEIVEエラーコード                     */

char            ch_rcv_buf[LSTN_MAX_TEXT_LEN+1];    /* $RECEIVEバッファ                         */
char            ch_rsp_buf[LSTN_MAX_TEXT_LEN+1];    /* 応答用バッファ                           */

zsys_ddl_smsg_open_def          *pch_open_msg    = (zsys_ddl_smsg_open_def      *)ch_rcv_buf;   /* OPENメッセージ           */
zsys_ddl_smsg_open_reply_def    *pch_open_reply  = (zsys_ddl_smsg_open_reply_def*)ch_rsp_buf;   /* OPENリプライメッセージ   */
zsys_ddl_smsg_close_def         *pch_close_msg   = (zsys_ddl_smsg_close_def     *)ch_rcv_buf;   /* CLOSEメッセージ          */
zsys_ddl_smsg_timesignal_def    *pch_timeout_msg = (zsys_ddl_smsg_timesignal_def*)ch_rcv_buf;   /* SIGNALTIMEOUTメッセージ  */
zsys_ddl_smsg_cpudown_def       *pch_cpudown_msg = (zsys_ddl_smsg_cpudown_def   *)ch_rcv_buf;   /* CPU-DOWNメッセージ       */

common_header_def   *pch_rcv_head =
                (common_header_def*)ch_rcv_buf;     /* 要求IPCヘッダー                          */
a001_def        *pch_a001 = (a001_def*)ch_rcv_buf;  /* 接続通知用非同期要求                     */
c103_def        *pch_c103 = (c103_def*)ch_rcv_buf;  /* コネクション接続開始要求                 */
c104_def        *pch_c104 = (c104_def*)ch_rcv_buf;  /* コネクション接続完了通知要求             */
c105_def        *pch_c105 = (c105_def*)ch_rcv_buf;  /* コネクション切断完了通知要求             */
c106_def        *pch_c106 = (c106_def*)ch_rcv_buf;  /* コネクション入替・切断完了通知要求       */
c502_def        *pch_c502 = (c502_def*)ch_rcv_buf;  /* コマンド要求                             */

common_header_def   *pch_rsp_head =
                (common_header_def*)ch_rsp_buf;     /* 応答IPCヘッダー                          */
n101_def        *pch_n101 = (n101_def*)ch_rsp_buf;  /* コネクション接続通知                     */
n102_def        *pch_n102 = (n102_def*)ch_rsp_buf;  /* コネクション入替・切断指示通知           */
r103_def        *pch_r103 = (r103_def*)ch_rsp_buf;  /* コネクション接続開始応答                 */
r104_def        *pch_r104 = (r104_def*)ch_rsp_buf;  /* コネクション接続完了通知応答             */
r105_def        *pch_r105 = (r105_def*)ch_rsp_buf;  /* コネクション切断完了通知応答             */
r106_def        *pch_r106 = (r106_def*)ch_rsp_buf;  /* コネクション入替・切断完了通知応答       */
r502_def        *pch_r502 = (r502_def*)ch_rsp_buf;  /* コマンド処理応答                         */

fileio_def      gflin_io;                           /* 回線管理ファイルI/O                      */
fileio_def      gclst_io;                           /* 回線ステータスファイルI/O                */
fileio_def      gcscn_io;                           /* 受信コネクション数管理ファイルI/O        */

db_gflin_def    *p_gflin_rec;                       /* 回線管理レコードバッファ                 */
db_gclst_def    *p_gclst_recin;                     /* 回線ステータスレコードバッファ           */
db_gclst_def    *p_gclst_recout;                    /* 回線ステータスレコードバッファ           */
db_gcscn_def    *p_gcscn_recin;                     /* 受信コネクション数管理レコードバッファ   */
db_gcscn_def    *p_gcscn_recout;                    /* 受信コネクション数管理レコードバッファ   */

_cc_status      i_CC;                               /* Condition Code */

/* 管理テーブル(オンライン&再編用) */
listenport_tbl_def      lpttbl  [LSTN_LISTENPORT_TBL_MAX];      /* リスンポート管理テーブル             */
controlserver_tbl_def   ctstbl  [LSTN_CONTROLSERVER_TBL_MAX];   /* コネクション制御管理テーブル         */
listener_tbl_def        lsntbl  [LSTN_LISTENER_TBL_MAX];        /* リスナー管理テーブル                 */
connection_tbl_def      contbl  [LSTN_CONNECTION_TBL_MAX];      /* コネクション管理テーブル             */
count_tbl_def           cnttbl  [LSTN_COUNT_TBL_MAX];           /* コネクション数管理テーブル           */
listenport_tbl_def      rlpttbl [LSTN_LISTENPORT_TBL_MAX];      /* リスンポート管理テーブル(再編用)     */
controlserver_tbl_def   rctstbl [LSTN_CONTROLSERVER_TBL_MAX];   /* コネクション制御管理テーブル(再編用) */
listener_tbl_def        rlsntbl [LSTN_LISTENER_TBL_MAX];        /* リスナー管理テーブル(再編用)         */
connection_tbl_def      rcontbl [LSTN_CONNECTION_TBL_MAX];      /* コネクション管理テーブル(再編用)     */
count_tbl_def           rcnttbl [LSTN_COUNT_TBL_MAX];           /* コネクション数管理テーブル(再編用)   */

/* サーバ停止判定 */
COM_STP_arg_1_def       opener_info;                /* オープナー情報テーブル */

/* メッセージ出力 */
oggz1in_def             cg010in;                    /* メッセージ出力 */

/* トランザクション管理 */
long    l_tranid = -1L;
char    ch_tranmodule[10];

/* トレース */
char    EXMYSRVCLSNAME[15];
char    EXMYPROCNAME[6];
short   EXTRACEMODE;
char    EXTRACEFILENAME[47];
short   EXTRACEFILENO;

char    ch_trc_buf[lk_zac2001i_arg_1_def_Size+1];
lk_zac2001i_arg_1_def   *p_trc_ipc = (lk_zac2001i_arg_1_def *)&ch_trc_buf;

/****************************************************************************/
/*  FUNCTION        : 1.0.0  main                                           */
/*  CALLING SEQ.    : int main(int, char[] *)                               */
/*  ARGUMENT        : int argc, char *argv[]                                */
/*  RETURN CODE     : int 0                                                 */
/*  DESCRIPTION     : メイン処理                                            */
/****************************************************************************/
int main( int argc, char *argv[] )
{
    /*----------------------------------------------*/
    /*    初期処理                                  */
    /*----------------------------------------------*/
    LSTN_init();

    /*----------------------------------------------*/
    /*    主処理                                    */
    /*----------------------------------------------*/
    LSTN_main();

    /*----------------------------------------------*/
    /*    終了処理                                  */
    /*----------------------------------------------*/
    LSTN_final();
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0  LSTN_init                                      */
/*  CALLING SEQ.    : void LSTN_init(void)                                  */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void LSTN_init(void)
{
    short           s_rtncd = 0;
    short           s_lptno = LSTN_TBL_NOT_ENTRY;

    /*----------------------------------------------*/
    /* メッセージ出力初期設定                       */
    /*----------------------------------------------*/
    memset((char *)&cg010in, 0x20, sizeof(cg010in));
    cg010in.subrcd = '0';
    cg010in.emsinf.rcd = '0';
    memcpy(cg010in.uytrminf.proctimer, "0002", 4); // temp
    memcpy(cg010in.uytrminf.uytrmmonlen, "00", 2); // temp
    memcpy(cg010in.uytrminf.uytrmsrvlen, "00", 2); // temp
    memcpy(cg010in.emsinf.emsgkinf.sysnm, DEF_EMS_SYSNM_GFP, strlen(DEF_EMS_SYSNM_GFP));
    memcpy(cg010in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM, strlen(DEF_EMS_SRV_KBN_COM));
    memcpy(cg010in.emsinf.emsgkinf.prgid, DEF_GFPCVX00, strlen(DEF_GFPCVX00));

    /*----------------------------------------------*/
    /* トランザクション管理初期処理                 */
    /*----------------------------------------------*/
    memset(ch_tranmodule, 0x00, sizeof(ch_tranmodule));
    memcpy(ch_tranmodule, DEF_GFPCVX00, strlen(DEF_GFPCVX00));

    /*----------------------------------------------*/
    /* グローバル情報初期化処理                     */
    /*----------------------------------------------*/
    /* 自プロセス情報テーブル */
    memset((char *)&myinfo, 0x00, sizeof(myinfo));
//  PROCESSHANDLE_NULLIT_(myinfo.process_info.my_phandle);
//  PROCESSHANDLE_NULLIT_(myinfo.process_info.creator_phandle);
//
//  s_rtncd = PROCESSHANDLE_GETMINE_(myinfo.process_info.my_phandle);
//  if (s_rtncd != 0) {
//      LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "PROCESSHANDLE_GETMINE_", s_rtncd);
//      LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
//  }
//  s_rtncd = PROCESSHANDLE_DECOMPOSE_(
//      myinfo.process_info.my_phandle,,,,,,,
//      myinfo.process_info.my_pname,
//      ZSYS_VAL_LEN_PROCESSNAME,
//      &myinfo.process_info.my_pname_len);
//  if (s_rtncd != 0) {
//      LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "PROCESSHANDLE_DECOMPOSE_", s_rtncd);
//      LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
//  }
//  memcpy(cg010in.emsinf.emsgkinf.trmnm,
//      myinfo.process_info.my_pname, myinfo.process_info.my_pname_len);
//
//  s_rtncd = PROCESS_GETPAIRINFO_(
//      myinfo.process_info.my_phandle,,,,,,,
//      myinfo.process_info.creator_phandle);
//  if (s_rtncd != 4 /* single named */ && s_rtncd != 5 /* process pair */) {
//      LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "PROCESS_GETPAIRINFO_", s_rtncd);
//      LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
//  }
//  s_rtncd = PROCESSHANDLE_DECOMPOSE_(
//          myinfo.process_info.creator_phandle,,,,,,,
//      myinfo.process_info.pathmon_name,
//      ZSYS_VAL_LEN_PROCESSNAME,
//      &myinfo.process_info.pathmon_name_len);
//  if (s_rtncd != 0) {
//      LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "PROCESSHANDLE_DECOMPOSE_", s_rtncd);
//      LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
//  }
    s_rtncd = COM_PRC(&myinfo.process_info);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "COM_PRC", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }
    memcpy(cg010in.emsinf.emsgkinf.trmnm,
        myinfo.process_info.my_pname, myinfo.process_info.my_pname_len);

    /* ファイルレコードポインタ */
    p_gflin_rec = (db_gflin_def *)&gflin_io.arg6.rec_area[0];
    p_gclst_recin = (db_gclst_def *)&gclst_io.arg6.rec_area[0];
    p_gclst_recout = (db_gclst_def *)&gclst_io.arg5.rec_area[0];
    p_gcscn_recin = (db_gcscn_def *)&gcscn_io.arg6.rec_area[0];
    p_gcscn_recout = (db_gcscn_def *)&gcscn_io.arg5.rec_area[0];

    /* リスンポート管理テーブル */
    LSTN_listenporttbl_init(LSTN_TBL_ONLINE);

    /* リスナー管理テーブル */
    LSTN_listenertbl_init(LSTN_TBL_ONLINE);

    /* コネクション制御管理テーブル */
    LSTN_controltbl_init(LSTN_TBL_ONLINE);

    /* コネクション管理テーブル */
    LSTN_connectiontbl_init(LSTN_TBL_ONLINE);

    /* コネクション数管理テーブル */
    LSTN_counttbl_init(LSTN_TBL_ONLINE);

    /*----------------------------------------------*/
    /* トレース出力初期設定                         */
    /*----------------------------------------------*/
    memset(ch_trc_buf, 0x20, lk_zac2001i_arg_1_def_Size);
    p_trc_ipc->func_flg = '0';
    TRACEOUT((char *)p_trc_ipc);

    /*----------------------------------------------*/
    /* コンフィグ情報取得処理                       */
    /*----------------------------------------------*/
    /* パラメータ取得処理 */
    LSTN_get_params();

    /* アサインファイル取得処理 */
    LSTN_get_assign();

    /* 物理名情報取得処理 */
    LSTN_get_phisical_name();

    /*----------------------------------------------*/
    /* 更新系ファイルオープン処理                   */
    /*----------------------------------------------*/
    /* 回線ステータスファイルOPEN処理 */
    LSTN_statusfile_open();

    /* 受信コネクション数管理ファイルOPEN処理 */
    LSTN_countfile_open();

    /*----------------------------------------------*/
    /* 管理テーブル情報取得処理                     */
    /*----------------------------------------------*/

    /* 回線管理ファイルオープン */
    s_rtncd = LSTN_linefile_open();
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* リスンポート情報取得処理 */
    s_rtncd = LSTN_get_listen_info(LSTN_TBL_ONLINE);
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* N/W情報取得処理 */
    s_rtncd = LSTN_get_network_info(LSTN_TBL_ONLINE);
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* コネクション数情報取得処理 */
    s_rtncd = LSTN_get_count_info(LSTN_TBL_ONLINE);
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* コネクション情報取得処理 */
    s_rtncd = LSTN_get_connection_info(LSTN_TBL_ONLINE);
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* コネクション制御管理テーブル(コネクション数管理対象外分) */
    s_rtncd = LSTN_get_controlserver_info(LSTN_TBL_ONLINE);
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 回線管理ファイルクローズ */
    LSTN_linefile_close();

    /* 回線ステータス反映処理 */
    s_rtncd = LSTN_get_status();
    if (s_rtncd != LSTN_GET_NORMAL) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 受信コネクション数管理情報更新処理 */
    LSTN_set_count_info();

    /*----------------------------------------------*/
    /* オープナー管理初期処理                       */
    /*----------------------------------------------*/
    COM_STP_INIT(&opener_info);

    /*----------------------------------------------*/
    /* $RECEIVEオープン処理                         */
    /*----------------------------------------------*/
    s_rcv_err = FILE_OPEN_(
        LSTN_RECEIVE_FILENAME,
        (short)strlen(LSTN_RECEIVE_FILENAME),
        &s_rcv_fd,
        ZSYS_VAL_OPENACC_READWRITE,
        ZSYS_VAL_OPENEXCL_SHARED,
        LSTN_NOWAITDEPTH,
        LSTN_RECVDEPTH);
    if (s_rcv_err != 0 /* ZFIL_ERR_OK */) {
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* $RECEIVE初回READUPDATE処理                   */
    /*----------------------------------------------*/
    i_CC = READUPDATEX(
        s_rcv_fd,
        (char *)&ch_rcv_buf,
        sizeof(ch_rcv_buf),
        /* count-read */,
        LSTN_TAG_RECV);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(s_rcv_fd, &s_rcv_err);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* MONITORCPUS開始処理                          */
    /*----------------------------------------------*/
    MONITORCPUS(0xffff);

    /*----------------------------------------------*/
    /* メッセージ出力処理                           */
    /*----------------------------------------------*/
    LSTN_message_output(DEF_EVT_PROC_START, '*', DEF_NERR_NOMAL, "@R", myinfo.process_info.my_pname);

    /*----------------------------------------------*/
    /* リスンポート初期処理                         */
    /*----------------------------------------------*/
    /* 起動状態復旧処理 */
    for (s_lptno = 0; s_lptno < myinfo.table_info[LSTN_TBL_ONLINE].listenporttbl_cnt; s_lptno++) {
        if (memcmp(lpttbl[s_lptno].ctl.listen_status, DEF_CONNECT_STS_LISTEN, strlen(DEF_CONNECT_STS_LISTEN))==0) {
            LSTN_port_open(s_lptno);
        }
    }
} /* LSTN_init */

/****************************************************************************/
/*  FUNCTION        : 3.0.0  LSTN_main                                      */
/*  CALLING SEQ.    : void LSTN_main(void)                                  */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void LSTN_main(void)
{
    s_exit_flag = LSTN_CONTINUE;

    while (s_exit_flag == LSTN_CONTINUE)
    {
        /*----------------------------------------------*/
        /* I/O完了待ち                                  */
        /*----------------------------------------------*/
        LSTN_io_wait();

        /*----------------------------------------------*/
        /* 完了I/O判定                                  */
        /*----------------------------------------------*/
         s_event = LSTN_event_judgement();

        /*----------------------------------------------*/
        /* イベント振分                                 */
        /*----------------------------------------------*/
        switch (s_event) {
        case LSTN_EV_NONE           :       /* イベント無し */
            break;
        case LSTN_EV_CTLASYNC       :       /* 接続通知用非同期要求 */
            LSTN_control_async();
            break;
        case LSTN_EV_CTLACCEPT      :       /* コネクション接続開始要求 */
            LSTN_control_accept();
            break;
        case LSTN_EV_CTLCONNECT     :       /* コネクション接続完了通知要求 */
            LSTN_control_connect();
            break;
        case LSTN_EV_CTLDISCONNECT  :       /* コネクション切断完了通知要求 */
            LSTN_control_disconnect();
            break;
        case LSTN_EV_CTLRECONNECT   :       /* コネクション入替・切断完了通知要求 */
            LSTN_control_reconnect();
            break;
        case LSTN_EV_SKTACCEPT      :       /* ACCEPT完了 */
            LSTN_accept_complete(iocmp.lpt_no);
            break;
        case LSTN_EV_SKTERROR       :       /* ACCEPTエラー */
            LSTN_accept_error(iocmp.lpt_no);
            break;
        case LSTN_EV_CMDLISTENSTART :       /* コマンド要求(リスナー開始) */
            LSTN_listen_start_command();
            break;
        case LSTN_EV_CMDLISTENSTOP  :       /* コマンド要求(リスナー終了) */
            LSTN_listen_stop_command();
            break;
        case LSTN_EV_CMDFILERELOAD  :       /* コマンド要求(ファイル再読込み) */
            LSTN_file_reload_command();
            break;
        default:
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
            break;
        }

        /*----------------------------------------------*/
        /* $RECEIVE READUPDATE処理                      */
        /*----------------------------------------------*/
        if (iocmp.fd == s_rcv_fd /* 0 */) {
            memset(ch_rcv_buf, 0x00, sizeof(ch_rcv_buf));
            i_CC = READUPDATEX(
                s_rcv_fd,
                (char *)&ch_rcv_buf,
                sizeof(ch_rcv_buf),
                /* count-read */,
                LSTN_TAG_RECV);
            if (_status_ne(i_CC)) {
                FILE_GETINFO_(s_rcv_fd, &s_rcv_err);
                LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
            }
        }
    }

} /* LSTN_main */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  LSTN_final                                     */
/*  CALLING SEQ.    : void LSTN_final(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void LSTN_final(void)
{
    /* 受信コネクション数管理ファイルクローズ */
    LSTN_countfile_close();

    /* 回線ステータスファイルクローズ */
    LSTN_statusfile_close();

    /* メッセージ出力 */
    LSTN_message_output(DEF_EVT_PROC_NORMAL_END, '*', DEF_NERR_NOMAL, "@R", myinfo.process_info.my_pname);

    /* プロセス停止 */
    PROCESS_STOP_(,, LSTN_NORMAL_TERMINATION);
} /* LSTN_final */

/****************************************************************************/
/*  FUNCTION        : 5.0.0  LSTN_get_params                                */
/*  CALLING SEQ.    : void LSTN_get_params(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void LSTN_get_params(void)
{
    char    ch_paramname    [32];
    char    ch_param        [64];
    short   s_result;
    char    ch_work         [10];

    /*----------------------------------------------*/
    /* サーバクラス論理ID                           */
    /*  0....+....1....+....2..                     */
    /*  T-D-G0001-SCLISTEN-0001                     */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_SRV_LOGICAL_ID, strlen(DEF_SRV_LOGICAL_ID));
    s_result = get_param_by_name(ch_paramname, ch_param, 23+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    if (strlen(ch_param) < 23) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_SRV_LOGICAL_ID, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    myinfo.config_info.site_id = ch_param[0];
    myinfo.config_info.network_id = ch_param[2];
    memcpy(myinfo.config_info.group_id, &ch_param[4], 5);
    memcpy(&myinfo.config_info.serverclass_id[0], &ch_param[10], 8);
    memcpy(&myinfo.config_info.serverclass_id[8], &ch_param[19], 4);

    /*----------------------------------------------*/
    /* ファイルI/Oタイマー                          */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_FILE_IO_TIMER_10MSECOND, strlen(DEF_FILE_IO_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    myinfo.config_info.fileio_timer = atol(ch_param);
    if (myinfo.config_info.fileio_timer == 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_FILE_IO_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* ソケットI/Oタイマー                          */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_SOCKET_IO_TIMER_10MSECOND, strlen(DEF_SOCKET_IO_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_SOCKET_IO_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    myinfo.config_info.socketio_timer = atol(ch_param);
    if (myinfo.config_info.socketio_timer == 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_SOCKET_IO_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* PATHSENDタイマー                             */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_PSEND_TIMER_10MSECOND, strlen(DEF_PSEND_TIMER_10MSECOND));
    s_result = get_param_by_name(ch_paramname, ch_param, 8+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_PSEND_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    myinfo.config_info.pathsendio_timer = atol(ch_param);
    if (myinfo.config_info.pathsendio_timer == 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@e", DEF_PSEND_TIMER_10MSECOND, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%04ld", myinfo.config_info.pathsendio_timer/100);
    memcpy(cg010in.uytrminf.proctimer, ch_work, strlen(ch_work));

    /*----------------------------------------------*/
    /* 運用監視端末出力PATHMON名                    */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_MSG_MON_NAME, strlen(DEF_MSG_MON_NAME));
    s_result = get_param_by_name(ch_paramname, ch_param,
        sizeof(cg010in.uytrminf.uytrmmon)+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_MON_NAME, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    memcpy(cg010in.uytrminf.uytrmmon, ch_param, strlen(ch_param));
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%02d", strlen(ch_param));
    memcpy(cg010in.uytrminf.uytrmmonlen, ch_work, strlen(ch_work));

    /*----------------------------------------------*/
    /* 運用監視端末出力サーバクラス名               */
    /*----------------------------------------------*/
    memset(ch_paramname, 0x00, sizeof(ch_paramname));
    memset(ch_param, 0x00, sizeof(ch_param));
    s_result = 0;

    memcpy(ch_paramname, DEF_MSG_SRV_NAME, strlen(DEF_MSG_SRV_NAME));
    s_result = get_param_by_name(ch_paramname, ch_param,
        sizeof(cg010in.uytrminf.uytrmsrv)+1);
    if (s_result < 0) {
        LSTN_message_output(DEF_EVT_PARAM_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR, "@X@e", DEF_MSG_SRV_NAME, s_result);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    memcpy(cg010in.uytrminf.uytrmsrv, ch_param, strlen(ch_param));
    memset(ch_work, 0x00, sizeof(ch_work));
    sprintf(ch_work, "%02d", strlen(ch_param));
    memcpy(cg010in.uytrminf.uytrmsrvlen, ch_work, strlen(ch_work));

} /* LSTN_get_params */

/****************************************************************************/
/*  FUNCTION        : 6.0.0  LSTN_get_assign                                */
/*  CALLING SEQ.    : void LSTN_get_assign(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : アサインファイル取得処理                              */
/****************************************************************************/
void LSTN_get_assign(void)
{
    char        ch_asname[32+1];
    char        ch_flname[48+1];

    /*----------------------------------------------*/
    /* 物理名情報ファイル名 */
    /*----------------------------------------------*/
    memset(ch_asname, 0x20, sizeof(ch_asname));
    ch_asname[sizeof(ch_asname)-1] = 0x00;
    memset(ch_flname, 0x20, sizeof(ch_flname));
    ch_flname[sizeof(ch_flname)-1] = 0x00;
    memcpy(ch_asname, DEF_ASN_GFPHI, strlen(DEF_ASN_GFPHI));
    COM_ASN(ch_asname, ch_flname, &myinfo.config_info.gfphi_fname_len);
    if (myinfo.config_info.gfphi_fname_len == 0) {
        LSTN_message_output(DEF_EVT_ASN_FILE_GET_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@X@E", DEF_ASN_GFPHI, 0);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
    }
    memcpy(myinfo.config_info.gfphi_fname, ch_flname, myinfo.config_info.gfphi_fname_len);

} /* LSTN_get_assign */

/****************************************************************************/
/*  FUNCTION        : 7.0.0  LSTN_get_phisical_name                         */
/*  CALLING SEQ.    : void LSTN_get_phisical_name(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 物理名情報取得処理                                    */
/****************************************************************************/
void LSTN_get_phisical_name(void)
{
    fileio_def       gfphi_io;
    db_gfphi_def     *p_gfphi_rec;
    char             ch_buf[64];

    p_gfphi_rec = (db_gfphi_def*)&gfphi_io.arg6.rec_area[0];

    /* IOモジュール情報 */
    memset((char *)&gfphi_io, ' ', sizeof(gfphi_io));
    gfphi_io.arg4.file_no = LSTN_FILE_CLOSED;
    gfphi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_io.arg5.part_key_position = 0;
    gfphi_io.arg5.part_key_len = 0;
    gfphi_io.arg5.key_len = 0;
    gfphi_io.arg5.compare_len = 0;
    gfphi_io.arg5.positioning_mode = 0;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gfphi_io.arg5.rec_len = 0;
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 物理名情報ファイルオープン                   */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_OPEN, strlen(DEF_COM_IOM_FUNC_OPEN));
    memcpy(gfphi_io.arg3.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
    memcpy(gfphi_io.arg3.file_id, DEF_FL_PHSIC_INFO, strlen(DEF_FL_PHSIC_INFO));
    memcpy(gfphi_io.arg3.file_name, myinfo.config_info.gfphi_fname, myinfo.config_info.gfphi_fname_len);
    memcpy(gfphi_io.arg3.file_io_type, DEF_FILEIO_OPEN, strlen(DEF_FILEIO_OPEN));
    memcpy(gfphi_io.arg4.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(gfphi_io.arg4.file_name, myinfo.config_info.gfphi_fname, myinfo.config_info.gfphi_fname_len);

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E",
            "", "", DEF_FL_PHSIC_INFO, DEF_COM_IOM_FUNC_OPEN, "", gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* 物理名情報ファイルREAD                       */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gfphi_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gfphi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gfphi_io.arg5.key_len = sizeof(p_gfphi_rec->pri_key);
    gfphi_io.arg5.compare_len = sizeof(p_gfphi_rec->pri_key);
    gfphi_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gfphi_io.arg5.rec_len = db_gfphi_def_Size;

    /* N/W情報ファイル名取得                        */
    p_gfphi_rec->pri_key.site_id = myinfo.config_info.site_id;
    p_gfphi_rec->pri_key.nw_id = myinfo.config_info.network_id;
    memcpy(p_gfphi_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memset((char *)&p_gfphi_rec->pri_key.srv_cls_key, '}', sizeof(p_gfphi_rec->pri_key.srv_cls_key));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_NW_INFO, strlen(DEF_FL_NW_INFO));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&p_gfphi_rec->pri_key, sizeof(p_gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
        return;
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gfphi_rec->prc_file_info.prc_file_name, sizeof(p_gfphi_rec->prc_file_info.prc_file_name));
    trim(ch_buf);
    myinfo.config_info.gfnwi_fname_len = (short)strlen(ch_buf);
    memcpy(myinfo.config_info.gfnwi_fname, ch_buf, myinfo.config_info.gfnwi_fname_len);

    /* 回線管理ファイル名取得                       */
    p_gfphi_rec->pri_key.site_id = myinfo.config_info.site_id;
    p_gfphi_rec->pri_key.nw_id = myinfo.config_info.network_id;
    memcpy(p_gfphi_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memset((char *)&p_gfphi_rec->pri_key.srv_cls_key, '}', sizeof(p_gfphi_rec->pri_key.srv_cls_key));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_LIN_MG, strlen(DEF_FL_LIN_MG));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&p_gfphi_rec->pri_key, sizeof(p_gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
        return;
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gfphi_rec->prc_file_info.prc_file_name, sizeof(p_gfphi_rec->prc_file_info.prc_file_name));
    trim(ch_buf);
    myinfo.config_info.gflin_fname_len = (short)strlen(ch_buf);
    memcpy(myinfo.config_info.gflin_fname, ch_buf, myinfo.config_info.gflin_fname_len);

    /* 回線ステータスファイル名取得                 */
    p_gfphi_rec->pri_key.site_id = myinfo.config_info.site_id;
    p_gfphi_rec->pri_key.nw_id = myinfo.config_info.network_id;
    memcpy(p_gfphi_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memset((char *)&p_gfphi_rec->pri_key.srv_cls_key, '}', sizeof(p_gfphi_rec->pri_key.srv_cls_key));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_LIN_STS, strlen(DEF_FL_LIN_STS));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&p_gfphi_rec->pri_key, sizeof(p_gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
        return;
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gfphi_rec->prc_file_info.prc_file_name, sizeof(p_gfphi_rec->prc_file_info.prc_file_name));
    trim(ch_buf);
    myinfo.config_info.gclst_fname_len = (short)strlen(ch_buf);
    memcpy(myinfo.config_info.gclst_fname, ch_buf, myinfo.config_info.gclst_fname_len);

    /* 受信コネクション数管理ファイル名取得         */
    p_gfphi_rec->pri_key.site_id = myinfo.config_info.site_id;
    p_gfphi_rec->pri_key.nw_id = myinfo.config_info.network_id;
    memcpy(p_gfphi_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memset((char *)&p_gfphi_rec->pri_key.srv_cls_key, '}', sizeof(p_gfphi_rec->pri_key.srv_cls_key));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_RCV_CON_NUM, strlen(DEF_FL_RCV_CON_NUM));
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(p_gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&p_gfphi_rec->pri_key, sizeof(p_gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_NORMAL_TERMINATION);
        return;
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            "READ",
            gfphi_io.arg5.key_value,
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gfphi_rec->prc_file_info.prc_file_name, sizeof(p_gfphi_rec->prc_file_info.prc_file_name));
    trim(ch_buf);
    myinfo.config_info.gcscn_fname_len = (short)strlen(ch_buf);
    memcpy(myinfo.config_info.gcscn_fname, ch_buf, myinfo.config_info.gcscn_fname_len);

    /*----------------------------------------------*/
    /* 物理名情報ファイルクローズ                   */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_CLOSE, strlen(DEF_COM_IOM_FUNC_CLOSE));
    memcpy(gfphi_io.arg3.file_io_type, DEF_FILEIO_CLOSE, strlen(DEF_FILEIO_CLOSE));
    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    gfphi_io.arg6.guardian_errcode = 0;

    COM_IOM(gfphi_io.func_type,
            gfphi_io.sub_prog_sts,
            &gfphi_io.arg3,
            &gfphi_io.arg4,
            &gfphi_io.arg5,
            &gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_PHSIC_INFO,
            DEF_COM_IOM_FUNC_CLOSE,
            "",
            gfphi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_get_phisical_name */

/****************************************************************************/
/*  FUNCTION        : 8.0.0  LSTN_get_listen_info                           */
/*  CALLING SEQ.    : short LSTN_get_listen_info(short)                     */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : リスンポート情報取得処理                              */
/****************************************************************************/
short LSTN_get_listen_info(short s_tbl_type)
{
    short   s_idx;
    short   s_lptno;

    /*----------------------------------------------*/
    /* 回線管理ファイル情報取得(自プロセス管理リスンポート) */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gflin_io.arg5.key_value,
        (char *)myinfo.config_info.serverclass_id,
        sizeof(myinfo.config_info.serverclass_id));
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_A1, strlen(DEF_COM_IOM_KEYTYPE_A1));
    gflin_io.arg5.key_len = sizeof(p_gflin_rec->alt1_key_info);
    gflin_io.arg5.compare_len = sizeof(p_gflin_rec->alt1_key_info);
    gflin_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;

    for (s_idx = 0; s_idx < LSTN_LISTENPORT_TBL_MAX; s_idx++) {
        /* 回線管理ファイルREAD */
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;

        COM_IOM(gflin_io.func_type,
                gflin_io.sub_prog_sts,
                &gflin_io.arg3,
                &gflin_io.arg4,
                &gflin_io.arg5,
                &gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_LIN_MG,
                "READ",
                gflin_io.arg5.key_value,
                gflin_io.arg6.guardian_errcode);
            LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
        }

        /* 無効レコードチェック */
        if (p_gflin_rec->invalid_flg == DEF_INVALID_FLG_ON) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* グループ識別チェック */
        if ((p_gflin_rec->pri_key.site_id != myinfo.config_info.site_id) ||
            (p_gflin_rec->pri_key.nw_id != myinfo.config_info.network_id) ||
            (memcmp(p_gflin_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id)) !=0)) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* リスンポート管理テーブル追加 */
        s_lptno = LSTN_listenporttbl_add(s_tbl_type, p_gflin_rec);
        if (s_lptno >= 0) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        } else {
            return(LSTN_GET_ERROR);
        }
    }

    if (myinfo.table_info[s_tbl_type].listenporttbl_cnt == 0) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LPTTBL NONE");
        return(LSTN_GET_ERROR);
    }

    return(LSTN_GET_NORMAL);
} /* LSTN_get_listen_info */

/****************************************************************************/
/*  FUNCTION        : 9.0.0  LSTN_get_network_info                          */
/*  CALLING SEQ.    : short LSTN_get_network_info(short)                    */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : N/W情報取得処理                                       */
/****************************************************************************/
short LSTN_get_network_info(short s_tbl_type)
{
    fileio_def          gfnwi_io;       /* N/W情報ファイルI/O               */
    db_gfnwi_def        *p_gfnwi_rec;
    layer_info_def      layer_info;
    char                ch_buf[32];
    short               s_idx;
    listenport_tbl_def  *p_lpttbl;

    p_gfnwi_rec = (db_gfnwi_def *)&gfnwi_io.arg6.rec_area[0];

    /* IOモジュール情報 */
    memset((char *)&gfnwi_io, ' ', sizeof(gfnwi_io));
    gfnwi_io.arg4.file_no = LSTN_FILE_CLOSED;
    gfnwi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_io.arg5.part_key_position = 0;
    gfnwi_io.arg5.part_key_len = 0;
    gfnwi_io.arg5.key_len = 0;
    gfnwi_io.arg5.compare_len = 0;
    gfnwi_io.arg5.positioning_mode = 0;
    gfnwi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfnwi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfnwi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gfnwi_io.arg5.rec_len = 0;
    gfnwi_io.arg6.guardian_errcode = 0;
    gfnwi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* N/W情報ファイルオープン                      */
    /*----------------------------------------------*/
    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_OPEN, strlen(DEF_COM_IOM_FUNC_OPEN));
    memcpy(gfnwi_io.arg3.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
    memcpy(gfnwi_io.arg3.file_id, DEF_FL_NW_INFO, strlen(DEF_FL_NW_INFO));
    memcpy(gfnwi_io.arg3.file_name, myinfo.config_info.gfnwi_fname, myinfo.config_info.gfnwi_fname_len);
    memcpy(gfnwi_io.arg3.file_io_type, DEF_FILEIO_OPEN, strlen(DEF_FILEIO_OPEN));
    memcpy(gfnwi_io.arg4.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(gfnwi_io.arg4.file_name, myinfo.config_info.gfnwi_fname, myinfo.config_info.gfnwi_fname_len);

    COM_IOM(gfnwi_io.func_type,
            gfnwi_io.sub_prog_sts,
            &gfnwi_io.arg3,
            &gfnwi_io.arg4,
            &gfnwi_io.arg5,
            &gfnwi_io.arg6);
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E",
            "", "", DEF_FL_NW_INFO, DEF_COM_IOM_FUNC_OPEN, "", gfnwi_io.arg6.guardian_errcode);
        return(LSTN_GET_ERROR);
    }

    /*----------------------------------------------*/
    /* N/W情報ファイル読込み処理(グループ単位)      */
    /*----------------------------------------------*/
    memset((char *)&layer_info, 0x20, sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memset(layer_info.interface_id, '}', sizeof(layer_info.interface_id));
    memset(layer_info.station_id, '}', sizeof(layer_info.station_id));

    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gfnwi_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gfnwi_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);
    memcpy(gfnwi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gfnwi_io.arg5.key_len = LSTN_KEYLEN_STATION;
    gfnwi_io.arg5.compare_len = LSTN_KEYLEN_STATION;
    gfnwi_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gfnwi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfnwi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfnwi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gfnwi_io.arg5.rec_len = db_gfnwi_def_Size;

    COM_IOM(gfnwi_io.func_type,
            gfnwi_io.sub_prog_sts,
            &gfnwi_io.arg3,
            &gfnwi_io.arg4,
            &gfnwi_io.arg5,
            &gfnwi_io.arg6);
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfnwi_io.sub_prog_sts)) == 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_NW_INFO,
            "READ",
            gfnwi_io.arg5.key_value,
            gfnwi_io.arg6.guardian_errcode);
        return(LSTN_GET_ERROR);
    }
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_NW_INFO,
            "READ",
            gfnwi_io.arg5.key_value,
            gfnwi_io.arg6.guardian_errcode);
        return(LSTN_GET_ERROR);
    }

    /* コネクション管理情報設定 */
    if ((p_gfnwi_rec->mng_lyr_info.connect_num_mng_lyr != DEF_CONNECT_NUM_MNG_LYR_IF) &&
        (p_gfnwi_rec->mng_lyr_info.connect_num_mng_lyr != DEF_CONNECT_NUM_MNG_LYR_ST)) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@f@K@X",
            DEF_FL_NW_INFO,
            gfnwi_io.arg5.key_value,
            "connect_num_mng_lyr");
        return(LSTN_GET_ERROR);
    }
    myinfo.config_info.connection_counter_layer = p_gfnwi_rec->mng_lyr_info.connect_num_mng_lyr;

    if ((p_gfnwi_rec->connect_num_mng_info.connect_num_mng_kind != DEF_CONNECT_NUM_MNG_KIND_A) &&
        (p_gfnwi_rec->connect_num_mng_info.connect_num_mng_kind != DEF_CONNECT_NUM_MNG_KIND_B)) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_PRM_RD_ERR_INV, "@f@K@X",
            DEF_FL_NW_INFO,
            gfnwi_io.arg5.key_value,
            "connect_num_mng_kind");
        return(LSTN_GET_ERROR);
    }
    myinfo.config_info.connection_counter_type = p_gfnwi_rec->connect_num_mng_info.connect_num_mng_kind;

    /* N/W識別情報(ログ用) */
    memcpy(cg010in.emsinf.emsgkinf.h_nw_kbn, p_gfnwi_rec->nw_id_info.nw_kubun, sizeof(cg010in.emsinf.emsgkinf.h_nw_kbn));

//    /*----------------------------------------------*/
//    /* N/W情報ファイル読込み処理(ステーション単位)  */
//    /*----------------------------------------------*/
//    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
//    memcpy(gfnwi_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
//    memcpy(gfnwi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
//    gfnwi_io.arg5.key_len = LSTN_KEYLEN_STATION;
//    gfnwi_io.arg5.compare_len = LSTN_KEYLEN_STATION;
//    gfnwi_io.arg5.positioning_mode = DEF_POSITION_EXACT;
//    gfnwi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
//    gfnwi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
//    gfnwi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
//    gfnwi_io.arg5.rec_len = db_gfnwi_def_Size;
//
//    /* テーブル範囲特定 */
//    if (s_tbl_type == LSTN_TBL_ONLINE) {
//        p_lpttbl = &lpttbl[0];
//    } else {
//        p_lpttbl = &rlpttbl[0];
//    }
//
//    for (s_idx = 0; s_idx < myinfo.table_info[s_tbl_type].listenporttbl_cnt; s_idx++) {
//        if ((memcmp(p_lpttbl[s_idx].interface_id, layer_info.interface_id, sizeof(layer_info.interface_id))==0) &&
//            (memcmp(p_lpttbl[s_idx].station_id, layer_info.station_id, sizeof(layer_info.station_id))==0)) {
//            p_lpttbl[s_idx].ctl.retry_timer = p_lpttbl[s_idx-1].ctl.retry_timer;
//            p_lpttbl[s_idx].ctl.retry_max = p_lpttbl[s_idx-1].ctl.retry_max;
//            continue;
//        }
//        memcpy(layer_info.interface_id, p_lpttbl[s_idx].interface_id, sizeof(p_lpttbl[s_idx].interface_id));
//        memcpy(layer_info.station_id, p_lpttbl[s_idx].station_id, sizeof(p_lpttbl[s_idx].station_id));
//        memcpy(gfnwi_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);
//
//        COM_IOM(gfnwi_io.func_type,
//                gfnwi_io.sub_prog_sts,
//                &gfnwi_io.arg3,
//                &gfnwi_io.arg4,
//                &gfnwi_io.arg5,
//                &gfnwi_io.arg6);
//        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfnwi_io.sub_prog_sts)) == 0) {
//            LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
//                "", "",
//                DEF_FL_NW_INFO,
//                "READ",
//                gfnwi_io.arg5.key_value,
//                gfnwi_io.arg6.guardian_errcode);
//            return(LSTN_GET_ERROR);
//        }
//        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
//            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
//                "", "",
//                DEF_FL_NW_INFO,
//                "READ",
//                gfnwi_io.arg5.key_value,
//                gfnwi_io.arg6.guardian_errcode);
//            return(LSTN_GET_ERROR);
//        }
//
//        /* リスナー制御情報設定 */
//        memset(ch_buf, 0x00, sizeof(ch_buf));
//        memcpy(ch_buf,
//            p_gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lst,
//            sizeof(p_gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lst));
//        p_lpttbl[s_idx].ctl.retry_timer = atol(ch_buf);
//
//        memset(ch_buf, 0x00, sizeof(ch_buf));
//        memcpy(ch_buf,
//            p_gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lst,
//            sizeof(p_gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lst));
//        p_lpttbl[s_idx].ctl.retry_max = atol(ch_buf);
//    }

    /*------------------------------------------------*/
    /* N/W情報ファイル読込み処理(インタフェース単位)  */
    /*------------------------------------------------*/
    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gfnwi_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gfnwi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gfnwi_io.arg5.key_len = LSTN_KEYLEN_STATION;
    gfnwi_io.arg5.compare_len = LSTN_KEYLEN_STATION;
    gfnwi_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gfnwi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfnwi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfnwi_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gfnwi_io.arg5.rec_len = db_gfnwi_def_Size;

    /* テーブル範囲特定 */
    if (s_tbl_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
    }

    for (s_idx = 0; s_idx < myinfo.table_info[s_tbl_type].listenporttbl_cnt; s_idx++) {
        if (memcmp(p_lpttbl[s_idx].interface_id, layer_info.interface_id, sizeof(layer_info.interface_id))==0) {
            p_lpttbl[s_idx].ctl.retry_timer = p_lpttbl[s_idx-1].ctl.retry_timer;
            p_lpttbl[s_idx].ctl.retry_max = p_lpttbl[s_idx-1].ctl.retry_max;
            continue;
        }
        memcpy(layer_info.interface_id, p_lpttbl[s_idx].interface_id, sizeof(p_lpttbl[s_idx].interface_id));
        memcpy(gfnwi_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);

        COM_IOM(gfnwi_io.func_type,
                gfnwi_io.sub_prog_sts,
                &gfnwi_io.arg3,
                &gfnwi_io.arg4,
                &gfnwi_io.arg5,
                &gfnwi_io.arg6);
        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfnwi_io.sub_prog_sts)) == 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR_WARN, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_NW_INFO,
                "READ",
                gfnwi_io.arg5.key_value,
                gfnwi_io.arg6.guardian_errcode);
            return(LSTN_GET_ERROR);
        }
        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_NW_INFO,
                "READ",
                gfnwi_io.arg5.key_value,
                gfnwi_io.arg6.guardian_errcode);
            return(LSTN_GET_ERROR);
        }

        /* リスナー制御情報設定 */
        memset(ch_buf, 0x00, sizeof(ch_buf));
        memcpy(ch_buf,
            p_gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lst,
            sizeof(p_gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lst));
        p_lpttbl[s_idx].ctl.retry_timer = atol(ch_buf);

        memset(ch_buf, 0x00, sizeof(ch_buf));
        memcpy(ch_buf,
            p_gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lst,
            sizeof(p_gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lst));
        p_lpttbl[s_idx].ctl.retry_max = atol(ch_buf);
    }

    /*----------------------------------------------*/
    /* N/W情報ファイルクローズ処理                  */
    /*----------------------------------------------*/
    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_CLOSE, strlen(DEF_COM_IOM_FUNC_CLOSE));
    memcpy(gfnwi_io.arg3.file_io_type, DEF_FILEIO_CLOSE, strlen(DEF_FILEIO_CLOSE));
    memset(gfnwi_io.sub_prog_sts, ' ', sizeof(gfnwi_io.sub_prog_sts));
    gfnwi_io.arg6.guardian_errcode = 0;

    COM_IOM(gfnwi_io.func_type,
            gfnwi_io.sub_prog_sts,
            &gfnwi_io.arg3,
            &gfnwi_io.arg4,
            &gfnwi_io.arg5,
            &gfnwi_io.arg6);
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_NW_INFO,
            DEF_COM_IOM_FUNC_CLOSE,
            "",
            gfnwi_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    return(LSTN_GET_NORMAL);
} /* LSTN_get_network_info */

/****************************************************************************/
/*  FUNCTION        : 10.0.0  LSTN_get_count_info                           */
/*  CALLING SEQ.    : short LSTN_get_count_info(short)                      */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : 受信コネクション数情報取得処理                        */
/****************************************************************************/
short LSTN_get_count_info(short s_tbl_type)
{
    listenport_tbl_def  *p_lpttbl;
    count_tbl_def       *p_cnttbl;
    short               s_rtncd;
    short               s_cntno = LSTN_TBL_NOT_ENTRY;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_tbl_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
        p_cnttbl = &cnttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
        p_cnttbl = &rcnttbl[0];
    }

    memset((char *)p_gcscn_recin, ' ', sizeof(db_gcscn_def));

    /*----------------------------------------------*/
    /* 受信コネクション数管理ファイル読込み         */
    /*----------------------------------------------*/
    for (s_idx = 0; s_idx < myinfo.table_info[s_tbl_type].listenporttbl_cnt; s_idx++) {
        /* コネクション数管理テーブル検索 */
        s_cntno = LSTN_counttbl_search(
            s_tbl_type,
            p_lpttbl[s_idx].interface_id,
            p_lpttbl[s_idx].station_id);

        if (s_cntno == LSTN_TBL_NOT_ENTRY) {
            /* 受信コネクション数管理ファイル読込み */
            s_rtncd = LSTN_countfile_read(
                p_lpttbl[s_idx].interface_id,
                p_lpttbl[s_idx].station_id,
                DEF_COM_IOM_NOLOCK);
            if (s_rtncd != LSTN_FILEIO_NORMAL) { /* EOF */
                continue;
            }
            /* コネクション数管理テーブル作成 */
            s_cntno = LSTN_counttbl_add(s_tbl_type, p_gcscn_recin);
            if (s_cntno < 0) {
                return(LSTN_GET_ERROR);
            }
        }

        p_lpttbl[s_idx].tbl.cnttbl_no = s_cntno;

        /* 再接続状態取得 */
        if ((memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts, DEF_RE_CONNECT_STS_ON, 2)==0) &&
            (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id,
             myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id))==0)) {
            p_lpttbl[s_idx].ctl.listen_state = LSTN_ST_RECONNECT;
            p_cnttbl[s_cntno].ctl.reconnect_lpttbl_no = s_idx;
        }
    }

    if (myinfo.table_info[s_tbl_type].counttbl_cnt == 0) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_RCV_CON_NUM, "", "CNTTBL NONE");
        return(LSTN_GET_ERROR);
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_get_count_info */

/****************************************************************************/
/*  FUNCTION        : 11.0.0  LSTN_get_connection_info                      */
/*  CALLING SEQ.    : short LSTN_get_connection_info(short)                 */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : コネクション情報取得処理                              */
/****************************************************************************/
short LSTN_get_connection_info(short s_tbl_type)
{
    layer_info_def      layer_info;
//  short               s_lsnno = LSTN_TBL_NOT_ENTRY;
    short               s_conno = LSTN_TBL_NOT_ENTRY;
    short               s_idx;
    listenport_tbl_def  *p_lpttbl;
    short               s_eof = 0;

    /* テーブル範囲特定 */
    if (s_tbl_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
    }

    /* 回線管理ファイルはオープン済み(リスンポート情報取得処理) */
    memset((char *)&layer_info, 0x20, sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));

    for (s_idx = 0; s_idx < myinfo.table_info[s_tbl_type].listenporttbl_cnt; s_idx++) {
        memset((char *)p_gflin_rec, 0x00, sizeof(db_gflin_def));
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
        memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));

        /* コネクション数管理単位 = インタフェース単位 */
        if (myinfo.config_info.connection_counter_layer == DEF_CONNECT_NUM_MNG_LYR_IF) {
            memcpy(layer_info.interface_id, p_lpttbl[s_idx].interface_id, sizeof(p_lpttbl[s_idx].interface_id));
            memcpy(gflin_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_INTERFACE);
            gflin_io.arg5.key_len = LSTN_KEYLEN_INTERFACE;
            gflin_io.arg5.compare_len = LSTN_KEYLEN_INTERFACE;
        }
        /* コネクション数管理単位 = ステーション単位 */
        if (myinfo.config_info.connection_counter_layer == DEF_CONNECT_NUM_MNG_LYR_ST) {
            memcpy(layer_info.interface_id, p_lpttbl[s_idx].interface_id, sizeof(p_lpttbl[s_idx].interface_id));
            memcpy(layer_info.station_id, p_lpttbl[s_idx].station_id, sizeof(p_lpttbl[s_idx].station_id));
            memcpy(gflin_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);
            gflin_io.arg5.key_len = LSTN_KEYLEN_STATION;
            gflin_io.arg5.compare_len = LSTN_KEYLEN_STATION;
        }
        memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
        gflin_io.arg5.positioning_mode = DEF_POSITION_GENERIC;
        gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
        gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
        gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
        gflin_io.arg5.rec_len = db_gflin_def_Size;

        /* 回線管理ファイル読込み */
        s_eof = 0;
        while (s_eof == 0) {
            memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
            gflin_io.arg6.guardian_errcode = 0;
            gflin_io.arg6.rec_len = 0;
            COM_IOM(gflin_io.func_type,
                    gflin_io.sub_prog_sts,
                    &gflin_io.arg3,
                    &gflin_io.arg4,
                    &gflin_io.arg5,
                    &gflin_io.arg6);
            if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
                s_eof = 1;
                break;
            }
            if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
                LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                    "", "",
                    DEF_FL_LIN_MG,
                    "READ",
                    gflin_io.arg5.key_value,
                    gflin_io.arg6.guardian_errcode);
                LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
            }

            /* 無効レコードチェック */
            if (p_gflin_rec->invalid_flg == DEF_INVALID_FLG_ON) {
                memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                continue;
            }

            /* コネクション識別コードチェック */
            if (memcmp((char *)p_gflin_rec->pri_key.connect_id, LSTN_CONNID_LISTNER, strlen(LSTN_CONNID_LISTNER))==0) {
                /* (CL)リスナー */
//              s_lsnno = LSTN_listenertbl_add(s_tbl_type, p_gflin_rec);
//              if (s_lsnno >= 0) {
                    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                    continue;
//              } else {
//                  return(LSTN_GET_ERROR);
//              }
            }
            else if (memcmp((char *)p_gflin_rec->pri_key.connect_id, LSTN_CONNID_SERVER, strlen(LSTN_CONNID_SERVER))==0) {
                /* (CS)コネクション制御(サーバ) */
                s_conno = LSTN_connectiontbl_add(s_tbl_type, p_gflin_rec);
                if (s_conno >= 0) {
                    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                    continue;
                } else {
                    return(LSTN_GET_ERROR);
                }
            }
            else {
                /* その他 */
                memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                continue;
            }
        }
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_get_connection_info */

/****************************************************************************/
/*  FUNCTION        : 11.0.0  LSTN_get_controlserver_info                   */
/*  CALLING SEQ.    : short LSTN_get_connection_info(short)                 */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : コネクション制御情報取得処理                          */
/****************************************************************************/
short LSTN_get_controlserver_info(short s_tbl_type)
{
    layer_info_def      layer_info;
    short               s_ctsno = LSTN_TBL_NOT_ENTRY;
    short               s_eof = 0;

    /* 回線管理ファイルはオープン済み(リスンポート情報取得処理) */
    memset((char *)&layer_info, 0x20, sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));

    memset((char *)p_gflin_rec, 0x00, sizeof(db_gflin_def));
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memset(gflin_io.arg5.key_value, 0x20, sizeof(gflin_io.arg5.key_value));
    memcpy(gflin_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_INTERFACE);
    gflin_io.arg5.key_len = LSTN_KEYLEN_GROUP;
    gflin_io.arg5.compare_len = LSTN_KEYLEN_GROUP;
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gflin_io.arg5.positioning_mode = DEF_POSITION_GENERIC;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;

    /* 回線管理ファイル読込み */
    s_eof = 0;
    while (s_eof == 0) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;
        COM_IOM(gflin_io.func_type,
                gflin_io.sub_prog_sts,
                &gflin_io.arg3,
                &gflin_io.arg4,
                &gflin_io.arg5,
                &gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
            s_eof = 1;
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_LIN_MG,
                "READ",
                gflin_io.arg5.key_value,
                gflin_io.arg6.guardian_errcode);
            LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
        }

        /* 無効レコードチェック */
        if (p_gflin_rec->invalid_flg == DEF_INVALID_FLG_ON) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* コネクション識別コードチェック */
        if (memcmp((char *)p_gflin_rec->pri_key.connect_id, LSTN_CONNID_SERVER, strlen(LSTN_CONNID_SERVER))==0) {
            /* (CS)コネクション制御(サーバ) */
            s_ctsno = LSTN_controltbl_add(s_tbl_type, p_gflin_rec);
            if (s_ctsno >= 0) {
                memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                continue;
            } else {
                return(LSTN_GET_ERROR);
            }
        }
        else {
            /* その他 */
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_get_controlserver_info */

/****************************************************************************/
/*  FUNCTION        : 12.0.0  LSTN_get_status                               */
/*  CALLING SEQ.    : void LSTN_get_status(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : 回線ステータス取得処理                                */
/****************************************************************************/
short LSTN_get_status(void)
{
    short       s_lptno = LSTN_TBL_NOT_ENTRY;
    short       s_conno = LSTN_TBL_NOT_ENTRY;
    short       s_rtncd;

    /* 回線ステータスファイル取得(リスンポート) */
    for (s_lptno = 0; s_lptno < myinfo.table_info[LSTN_TBL_ONLINE].listenporttbl_cnt; s_lptno++) {
        s_rtncd = LSTN_statusfile_read(
            lpttbl[s_lptno].interface_id,
            lpttbl[s_lptno].station_id,
            lpttbl[s_lptno].listen_id,
            DEF_COM_IOM_NOLOCK);
        if (s_rtncd != LSTN_FILEIO_NORMAL) {
            return(LSTN_GET_ERROR);
        }
        memcpy(lpttbl[s_lptno].ctl.listen_status,
            p_gclst_recin->connect_sts_info.connect_sts, sizeof(lpttbl[s_lptno].ctl.listen_status));
    }

    /* 回線ステータスファイル取得(データポート) */
    for (s_conno = 0; s_conno < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_conno++) {
        s_rtncd = LSTN_statusfile_read(
            contbl[s_conno].interface_id,
            contbl[s_conno].station_id,
            contbl[s_conno].connection_id,
            DEF_COM_IOM_NOLOCK);
        if (s_rtncd != LSTN_FILEIO_NORMAL) {
            return(LSTN_GET_ERROR);
        }
        memcpy(contbl[s_conno].ctl.connection_sts,
            p_gclst_recin->connect_sts_info.connect_sts, sizeof(contbl[s_conno].ctl.connection_sts));
        if (memcmp(contbl[s_conno].ctl.connection_sts, DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))==0) {
            ctstbl[contbl[s_conno].tbl.ctstbl_no].tbl.connection_cnt++;
            cnttbl[contbl[s_conno].tbl.cnttbl_no].tbl.cur_connection_cnt++;
        }
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_get_status */

/****************************************************************************/
/*  FUNCTION        : 13.0.0  LSTN_set_count_info                           */
/*  CALLING SEQ.    : void LSTN_set_count_info(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション数更新処理                                */
/****************************************************************************/
void LSTN_set_count_info(void)
{
    short   s_idx;
    short   s_rtncd;
    char    ch_buf[10];

    /*----------------------------------------------*/
    /* コネクション数管理ファイル更新               */
    /*----------------------------------------------*/
    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    for (s_idx = 0; s_idx < myinfo.table_info[LSTN_TBL_ONLINE].counttbl_cnt; s_idx++) {
        /* 受信コネクション数管理ファイル取得 */
        s_rtncd = LSTN_countfile_read(
            cnttbl[s_idx].interface_id,
            cnttbl[s_idx].station_id,
            DEF_COM_IOM_LOCK);
        if (s_rtncd != LSTN_FILEIO_NORMAL) { /* EOF */
            continue;
        }

        /* コネクション数更新 */
        memcpy((char *)p_gcscn_recout, (char *)p_gcscn_recin, sizeof(db_gcscn_def));
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf, "%04d", cnttbl[s_idx].tbl.max_connection_cnt);
        memcpy(p_gcscn_recout->connect_num_ctrl_info.max_connect_num,
            ch_buf, sizeof(p_gcscn_recout->connect_num_ctrl_info.max_connect_num));
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf, "%04d", cnttbl[s_idx].tbl.cur_connection_cnt);
        memcpy(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num,
            ch_buf, sizeof(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num));

        /* 再接続状態更新 */
        if ((memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts,
                DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON))==0) &&
            (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id,
                myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id))==0)) {
            if (cnttbl[s_idx].ctl.reconnect_cnt == 0) {
                memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_sts,
                    DEF_RE_CONNECT_STS_OFF, strlen(DEF_RE_CONNECT_STS_OFF));
                memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_start_time,
                    ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_start_time));
                memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_svr_cls_id,
                    ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id));
            }
        }

        /* 受信コネクション数管理ファイル更新 */
        LSTN_countfile_update();
    }

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_set_count_info */

/****************************************************************************/
/*  FUNCTION        : 14.0.0  LSTN_io_wait                                  */
/*  CALLING SEQ.    : void LSTN_io_wait(void)                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : I/O完了待ち処理                                       */
/****************************************************************************/
void LSTN_io_wait(void)
{
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memset((char *)&iocmp, 0x00, sizeof(iocmp));
    iocmp.fd = LSTN_FILE_CLOSED;
    iocmp.cts_no = LSTN_TBL_NOT_ENTRY;
    iocmp.lpt_no = LSTN_TBL_NOT_ENTRY;

    i_CC = AWAITIOX(
        &iocmp.fd,
        &iocmp.addr,
        (unsigned short *)&iocmp.len,
        &iocmp.tag,
        -1L);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(iocmp.fd, &iocmp.fs_err);
    }

} /* LSTN_io_wait */

/****************************************************************************/
/*  FUNCTION        : 15.0.0  LSTN_event_judgement                          */
/*  CALLING SEQ.    : short LSTN_event_judgement(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : イベント判定処理                                      */
/****************************************************************************/
short   LSTN_event_judgement(void)
{
    /*----------------------------------------------*/
    /* $RECEIVE READUPDATE完了                      */
    /*----------------------------------------------*/
    if (iocmp.fd == s_rcv_fd /* 0 */) {
        s_rcv_err = FILE_GETRECEIVEINFO_((short *)&iocmp.rinf);
        if (s_rcv_err != 0) {
            LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@X@E",
                "FILE_GETRECEIVEINFO_", s_rcv_err);
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }

        /*----------------------------------------------*/
        /* システムメッセージ                           */
        /*----------------------------------------------*/
        if (iocmp.fs_err == 6) {
            switch (pch_open_msg->u_z_msgnumber.z_msgnumber) {
            case ZSYS_VAL_SMSG_OPEN:            /* オープンメッセージ */
                LSTN_open_message();
                break;
            case ZSYS_VAL_SMSG_CLOSE:           /* クローズメッセージ */
                LSTN_close_message();
                break;
            case ZSYS_VAL_SMSG_CPUDOWN:         /* CPUダウンメッセージ */
                LSTN_cpudown_message();
                break;
            case ZSYS_VAL_SMSG_TIMESIGNAL:      /* シグナルタイムアウトメッセージ */
                LSTN_timeout_message();
                break;
            default:
                LSTN_reply(ch_rsp_buf, 0, iocmp.rinf.z_messagetag, 0, LSTN_FLG_OFF);
                break;
            }
            return(LSTN_EV_NONE);
        } else if (iocmp.fs_err != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "", LSTN_RECEIVE_FILENAME, "READUPDATE", "", iocmp.fs_err);
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }

        /*----------------------------------------------*/
        /* システムメッセージ以外                       */
        /*----------------------------------------------*/
        if (iocmp.rinf.z_openlabel == -1) {
            /*----------------------------------------------*/
            /* コマンドサーバインタフェース                 */
            /*----------------------------------------------*/
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_CMD_PRC_REQ, strlen(DEF_IPC_IFCD_CMD_PRC_REQ))==0) {
                if (memcmp(pch_c502->command_info.command_name,
                    DEF_IPC_CMD_LSN_START, strlen(DEF_IPC_CMD_LSN_START))==0) {
                    return(LSTN_EV_CMDLISTENSTART);
                }
                if (memcmp(pch_c502->command_info.command_name,
                    DEF_IPC_CMD_LSN_END, strlen(DEF_IPC_CMD_LSN_END))==0) {
                    return(LSTN_EV_CMDLISTENSTOP);
                }
                if (memcmp(pch_c502->command_info.command_name,
                    DEF_IPC_CMD_FL_RE_READ, strlen(DEF_IPC_CMD_FL_RE_READ))==0) {
                    return(LSTN_EV_CMDFILERELOAD);
                }
            }
            memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
            memcpy(ch_rsp_buf, ch_rcv_buf, iocmp.len);
//          pch_rsp_head->interface_code[0] = 'R';
            pch_rsp_head->error_code = DEF_IPC_ERRCD_NG;
            memcpy(pch_rsp_head->internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
            pch_rsp_head->control_data_length = 0;
        } else {
            /* オープンラベルからコネクション制御管理テーブル番号取得 */
            if ((iocmp.rinf.z_openlabel >= 0) &&
                (iocmp.rinf.z_openlabel < myinfo.table_info[LSTN_TBL_ONLINE].controlservertbl_cnt)) {
                if (memcmp((char *)&iocmp.rinf.z_sender,
                    (char *)ctstbl[iocmp.rinf.z_openlabel].ctl.phandle, sizeof(iocmp.rinf.z_sender))==0) {
                    iocmp.cts_no = iocmp.rinf.z_openlabel;
                } else {
                    LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
                }
            } else {
                LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
            }
            /*----------------------------------------------*/
            /* コネクション制御インタフェース */
            /*----------------------------------------------*/
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_CON_NT_ACTL_REQ, sizeof(DEF_IPC_IFCD_CON_NT_ACTL_REQ))==0) {
                return(LSTN_EV_CTLASYNC);
            }
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_CON_START_REQ, sizeof(DEF_IPC_IFCD_CON_START_REQ))==0) {
                return(LSTN_EV_CTLACCEPT);
            }
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_CON_NT_REQ, sizeof(DEF_IPC_IFCD_CON_NT_REQ))==0) {
                return(LSTN_EV_CTLCONNECT);
            }
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_DISCON_NT_REQ, sizeof(DEF_IPC_IFCD_DISCON_NT_REQ))==0) {
                return(LSTN_EV_CTLDISCONNECT);
            }
            if (memcmp(pch_rcv_head->interface_code,
                DEF_IPC_IFCD_CON_SW_NT_REQ, sizeof(DEF_IPC_IFCD_CON_SW_NT_REQ))==0) {
                return(LSTN_EV_CTLRECONNECT);
            }
            memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
            memcpy(ch_rsp_buf, ch_rcv_buf, iocmp.len);
//          pch_rsp_head->interface_code[0] = 'R';
            pch_rsp_head->error_code = DEF_IPC_ERRCD_NG;
            memcpy(pch_rsp_head->internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
            pch_rsp_head->control_data_length = 0;
        }

        /* 非対応IPC */
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｱﾝｻﾎﾟｰﾄIPC", ch_rcv_buf);
        LSTN_reply(ch_rsp_buf, iocmp.len, iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return(LSTN_EV_NONE);
    } /* (iocmp.fd == 0) */

    /*----------------------------------------------*/
    /* ソケットI/O完了                              */
    /*----------------------------------------------*/
    COM_TGA(iocmp.tag,
        (unsigned short *)&iocmp.compo,
        (unsigned short *)&iocmp.lpt_no,
        (unsigned short *)&iocmp.event);
    if ((iocmp.compo == LSTN_COMPONENT_LISTEN) &&
        (iocmp.event == LSTN_TAG_ACCEPT)) {
        if (iocmp.fs_err == 0 /* ZFIL_ERR_OK */) {
            return(LSTN_EV_SKTACCEPT);
        } else {
            return(LSTN_EV_SKTERROR);
        }
    }

    /* 不明I/O完了 */
    LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "IPCｶｲｾｷｴﾗｰ", ch_rcv_buf);
    LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    return(LSTN_EV_NONE);

} /* LSTN_event_judgement */

/****************************************************************************/
/*  FUNCTION        : 16.0.0  LSTN_open_message                             */
/*  CALLING SEQ.    : void LSTN_open_message(void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープンメッセージ処理                                */
/****************************************************************************/
void LSTN_open_message(void)
{
    short   s_cpuno = -1;
    char    ch_proc_name[ZSYS_VAL_LEN_PROCESSNAME+1];
    short   s_proc_name_len = 0;
    char    ch_logic_name[16];
    short   s_replen = 0;
    short   s_reperr = 0;
    short   s_rtncd;

    /* オープナー情報取得 */
    memset(ch_proc_name, 0x00, sizeof(ch_proc_name));
    PROCESSHANDLE_DECOMPOSE_(
        (short *)&iocmp.rinf.z_sender,
        (short *)&s_cpuno,,,,,,
        ch_proc_name,
        ZSYS_VAL_LEN_PROCESSNAME,
        &s_proc_name_len);

    /* オープンクオリファイア判定 */
    if (pch_open_msg->z_qualifier_len >= 14) {
        if (memcmp(pch_open_msg->u_z_data.z_qualifier,
            LSTN_CTLSERVER_QUALIFY, strlen(LSTN_CTLSERVER_QUALIFY))==0) {
            /* コネクション制御プロセスオープン */
            memset(ch_logic_name, 0x00, sizeof(ch_logic_name));
            memcpy(&ch_logic_name[0], &pch_open_msg->u_z_data.z_qualifier[1], 6);
            memcpy(&ch_logic_name[6], &pch_open_msg->u_z_data.z_qualifier[8], 6);

            /* コネクション制御管理テーブル検索 */
            iocmp.cts_no = LSTN_controltbl_search(LSTN_TBL_ONLINE, ch_logic_name);
            if (iocmp.cts_no == LSTN_TBL_NOT_ENTRY) {
                /* テーブル不一致 */
                LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｵｰﾌﾟﾝｸｵﾘﾌｧｲｴﾗｰ", ch_rcv_buf);
                s_reperr = LSTN_UNRELATED_TABLE;
            } else {
                /* コネクション制御管理テーブル更新 */
                memcpy((char *)&ctstbl[iocmp.cts_no].ctl.phandle[0],
                    (char *)&iocmp.rinf.z_sender,
                    zsys_ddl_phandle_def_Size);
                ctstbl[iocmp.cts_no].ctl.cpu_no = s_cpuno;
                memset(ctstbl[iocmp.cts_no].ctl.proc_name, 0x00, sizeof(ctstbl[iocmp.cts_no].ctl.proc_name));
                memcpy(ctstbl[iocmp.cts_no].ctl.proc_name, ch_proc_name, s_proc_name_len);
                ctstbl[iocmp.cts_no].ctl.proc_name_len = s_proc_name_len;
                ctstbl[iocmp.cts_no].ctl.control_status = LSTN_CONTROL_STS_OPEN;

                /* リプライメッセージ(オープンラベル設定) */
                pch_open_reply->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
                pch_open_reply->z_openid = iocmp.cts_no;
                s_replen = zsys_ddl_smsg_open_reply_def_Size;
                s_reperr = 0 /* ZFIL_ERR_OK */;
            }
        } else {
            /* テーブル登録なし */
            LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｵｰﾌﾟﾝｸｵﾘﾌｧｲｴﾗｰ", ch_rcv_buf);
            s_reperr = LSTN_UNRELATED_TABLE;
        }
    } else {
        /* クリエイタープロセス判定 */
        if (PROCESSHANDLE_COMPARE_(iocmp.rinf.z_sender.u_z_data.z_word, myinfo.process_info.ans_phandle) != 0) {
            /* リプライメッセージ(オープンラベル設定) */
            pch_open_reply->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
            pch_open_reply->z_openid = LSTN_LABEL_CREATOR;
            s_replen = zsys_ddl_smsg_open_reply_def_Size;
            s_reperr = 0 /* ZFIL_ERR_OK */;
        } else {
            s_reperr = 0 /* ZFIL_ERR_OK */;
        }
    }

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, s_replen, iocmp.rinf.z_messagetag, s_reperr, LSTN_FLG_OFF);

    if (s_reperr != 0) return;

    // オーナープロセス管理 */
    s_rtncd = COM_STP_JUDGE(&opener_info, ch_rcv_buf);
    if (s_rtncd < 0) {
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_open_message */

/****************************************************************************/
/*  FUNCTION        : 17.0.0  LSTN_close_message                            */
/*  CALLING SEQ.    : void LSTN_close_message(void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : クローズメッセージ処理                                */
/****************************************************************************/
void LSTN_close_message(void)
{
    short       s_rtncd;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, 0, iocmp.rinf.z_messagetag, 0, LSTN_FLG_OFF);

    /* クリエイタープロセスクローズ処理 */
    if (iocmp.rinf.z_openlabel == LSTN_LABEL_CREATOR) {
        s_exit_flag = LSTN_TERMINATE;
        return;
    }

    // オーナープロセス管理 */
    s_rtncd = COM_STP_JUDGE(&opener_info, ch_rcv_buf);
    if (s_rtncd < 0) {
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }
    if (s_rtncd == 1) {
        s_exit_flag = LSTN_TERMINATE;
        return;
    }

    /* コネクション制御クローズ処理 */
    if ((iocmp.rinf.z_openlabel >= 0) &&
        (iocmp.rinf.z_openlabel < myinfo.table_info[LSTN_TBL_ONLINE].controlservertbl_cnt)) {
        if (memcmp((char *)&iocmp.rinf.z_sender,
            (char *)ctstbl[iocmp.rinf.z_openlabel].ctl.phandle, sizeof(iocmp.rinf.z_sender))==0) {
            iocmp.cts_no = iocmp.rinf.z_openlabel;
        } else {
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }
        LSTN_control_close(iocmp.cts_no);
    }

} /* LSTN_close_message */

/****************************************************************************/
/*  FUNCTION        : 18.0.0  LSTN_cpudown_message                          */
/*  CALLING SEQ.    : void LSTN_cpudown_message(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : CPUダウンメッセージ処理                               */
/****************************************************************************/
void LSTN_cpudown_message(void)
{
    short       s_ctsno;
    short       s_rtncd;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, 0, iocmp.rinf.z_messagetag, 0, LSTN_FLG_OFF);

    // オーナープロセス管理 */
    s_rtncd = COM_STP_JUDGE(&opener_info, ch_rcv_buf);
    if (s_rtncd < 0) {
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }
    if (s_rtncd == 1) {
        s_exit_flag = LSTN_TERMINATE;
        return;
    }

    /* コネクション制御クローズ処理 */
    for (s_ctsno = 0; s_ctsno < myinfo.table_info[LSTN_TBL_ONLINE].controlservertbl_cnt; s_ctsno++) {
        if (ctstbl[s_ctsno].ctl.cpu_no == pch_cpudown_msg->z_cpunumber) {
            LSTN_control_close(s_ctsno);
        }
    }

} /* LSTN_cpudown_message */

/****************************************************************************/
/*  FUNCTION        : 19.0.0  LSTN_timeout_message                          */
/*  CALLING SEQ.    : void LSTN_timeout_message(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : タイムアウトメッセージ処理                            */
/****************************************************************************/
void LSTN_timeout_message(void)
{
    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, 0, iocmp.rinf.z_messagetag, 0, LSTN_FLG_OFF);

    /* リスンポート管理テーブル番号取得 */
    iocmp.lpt_no = pch_timeout_msg->z_parm1;
    lpttbl[iocmp.lpt_no].ctl.retry_tag = LSTN_TAG_NULL;

    /* リスンポートリトライ処理 */
    LSTN_port_open(iocmp.lpt_no);

} /* LSTN_timeout_message */

/****************************************************************************/
/*  FUNCTION        : 20.0.0  LSTN_listen_start_command                     */
/*  CALLING SEQ.    : void LSTN_listen_start_command(void)                  */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー開始コマンド処理                              */
/****************************************************************************/
void LSTN_listen_start_command(void)
{
    short   s_lptno = LSTN_TBL_NOT_ENTRY;
    short   s_rtncd;
//  char    ch_cmdprm[64];

    /* コマンド精査 */
    if ((pch_c502->command_info.connection_logical_name.site_name != myinfo.config_info.site_id) ||
        (pch_c502->command_info.connection_logical_name.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c502->command_info.connection_logical_name.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r502->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* リスンポート管理テーブル取得 */
    s_lptno = LSTN_listenporttbl_search(
        LSTN_TBL_ONLINE,
        pch_c502->command_info.connection_logical_name.interface_name,
        pch_c502->command_info.connection_logical_name.station_name,
        pch_c502->command_info.connection_logical_name.connection_name);
    if (s_lptno == LSTN_TBL_NOT_ENTRY) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ﾘｽﾝﾎﾟｰﾄﾃｰﾌﾞﾙﾐﾄｳﾛｸ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r502->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コマンド受信メッセージ */
//  memset(ch_cmdprm, 0x00, sizeof(ch_cmdprm));
//  memcpy(ch_cmdprm, (char *)&pch_c502->command_info.connection_logical_name,
//      sizeof(pch_c502->command_info.connection_logical_name));
//  LSTN_message_output(DEF_EVT_CMD_RCV, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", ch_cmdprm, DEF_IPC_CMD_LSN_START, "");

    /* リスンポートステートチェック */
    switch (lpttbl[s_lptno].ctl.listen_state) {
    case LSTN_ST_DISCONNECT: /* 切断状態 */
        /* リスンポートオープン処理 */
        s_rtncd = LSTN_port_open(s_lptno);
        if (s_rtncd != 0) {
//          /* コマンド結果メッセージ */
//          LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_CMD_ERR_DONE, "@L@C@X@X",
//              "", "", DEF_IPC_CMD_LSN_START, "ERROR");

            /* エラー応答 */
            memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
            memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
            memcpy(pch_r502->common_header.interface_code,
                DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
            pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
            memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_LISTEN_ERR, strlen(DEF_NERR_LISTEN_ERR));
            pch_r502->common_header.control_data_length = 0;

            /* リプライ処理 */
            LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
            return;
        }
        break;

    case LSTN_ST_ACCEPT     : /* 接続待ち状態     */
        /* 開始済み/正常応答 */
        break;

    case LSTN_ST_RETRY     : /* リトライ待ち状態     */
        /* タイマキャンセル */
        i_CC = CANCELTIMEOUT(lpttbl[s_lptno].ctl.retry_tag);
        if (_status_ne(i_CC)) {
            LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "CANCELTIMEOUT", i_CC);
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }
        lpttbl[s_lptno].ctl.retry_tag = LSTN_TAG_NULL;
        lpttbl[s_lptno].ctl.retry_cnt = 0;

        /* リスンポートオープン処理 */
        s_rtncd = LSTN_port_open(s_lptno);
        if (s_rtncd != 0) {
//          /* コマンド結果メッセージ */
//          LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_CMD_ERR_DONE, "@L@C@X@X",
//              "", "", DEF_IPC_CMD_LSN_START, "ERROR");

            /* エラー応答 */
            memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
            memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
            memcpy(pch_r502->common_header.interface_code,
                DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
            pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
            memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_LISTEN_ERR, strlen(DEF_NERR_LISTEN_ERR));
            pch_r502->common_header.control_data_length = 0;

            /* リプライ処理 */
            LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
            return;
        }
        break;

    case LSTN_ST_RECONNECT  : /* 切断待ち状態     */
        /* リスンポートオープン処理 */
        s_rtncd = LSTN_port_open(s_lptno);
        if (s_rtncd != 0) {
//          /* コマンド結果メッセージ */
//          LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_CMD_ERR_DONE, "@L@C@X@X",
//              "", "", DEF_IPC_CMD_LSN_START, "ERROR");

            /* エラー応答 */
            memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
            memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
            memcpy(pch_r502->common_header.interface_code,
                DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
            pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
            memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_LISTEN_ERR, strlen(DEF_NERR_LISTEN_ERR));
            pch_r502->common_header.control_data_length = 0;

            /* リプライ処理 */
            LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
            return;
        }
        break;

    default:
        break;
    }

//  /* コマンド結果メッセージ */
//  LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", "", DEF_IPC_CMD_LSN_START, "OK");

    /* リスンポートオープン処理後応答 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
    memcpy(pch_r502->common_header.interface_code,
        DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
    pch_r502->common_header.control_data_length = 0;
    pch_r502->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

} /* LSTN_listen_start_command */

/****************************************************************************/
/*  FUNCTION        : 21.0.0  LSTN_listen_stop_command                      */
/*  CALLING SEQ.    : void LSTN_listen_stop_command(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー終了コマンド処理                              */
/****************************************************************************/
void LSTN_listen_stop_command(void)
{
    short   s_lptno = LSTN_TBL_NOT_ENTRY;
//  char    ch_cmdprm[64];

    /* コマンド精査 */
    if ((pch_c502->command_info.connection_logical_name.site_name != myinfo.config_info.site_id) ||
        (pch_c502->command_info.connection_logical_name.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c502->command_info.connection_logical_name.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r502->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* リスンポート管理テーブル取得 */
    s_lptno = LSTN_listenporttbl_search(
        LSTN_TBL_ONLINE,
        pch_c502->command_info.connection_logical_name.interface_name,
        pch_c502->command_info.connection_logical_name.station_name,
        pch_c502->command_info.connection_logical_name.connection_name);
    if (s_lptno == LSTN_TBL_NOT_ENTRY) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ﾘｽﾝﾎﾟｰﾄﾃｰﾌﾞﾙﾐﾄｳﾛｸ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r502->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コマンド受信メッセージ */
//  memset(ch_cmdprm, 0x00, sizeof(ch_cmdprm));
//  memcpy(ch_cmdprm, (char *)&pch_c502->command_info.connection_logical_name,
//      sizeof(pch_c502->command_info.connection_logical_name));
//  LSTN_message_output(DEF_EVT_CMD_RCV, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", ch_cmdprm, DEF_IPC_CMD_LSN_END, "");

    /* リスンポートステートチェック */
    switch (lpttbl[s_lptno].ctl.listen_state) {
    case LSTN_ST_DISCONNECT: /* 切断状態 */
        break;

    case LSTN_ST_ACCEPT     : /* 接続待ち状態     */
        /* リスンポートクローズ処理 */
        LSTN_port_close(s_lptno);
        break;

    case LSTN_ST_RETRY     : /* リトライ待ち状態     */
        /* リスンポートクローズ処理 */
        LSTN_port_close(s_lptno);
        break;

    case LSTN_ST_RECONNECT  : /* 切断待ち状態     */
        /* リスンポートクローズ処理 */
        LSTN_port_close(s_lptno);
        break;

    default:
        break;
    }

//  /* コマンド結果メッセージ */
//  LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", "", DEF_IPC_CMD_LSN_END, "OK");

    /* リスンポートクローズ処理後応答 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
    memcpy(pch_r502->common_header.interface_code,
        DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
    pch_r502->common_header.control_data_length = 0;
    pch_r502->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

} /* LSTN_listen_stop_command */

/****************************************************************************/
/*  FUNCTION        : 22.0.0  LSTN_file_reload_command                      */
/*  CALLING SEQ.    : void LSTN_file_reload_command(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ファイル再読込みコマンド処理                          */
/****************************************************************************/
void LSTN_file_reload_command(void)
{
    short   s_cmdlvl = LSTN_CMDLVL_GROUP;
    short   s_rtncd;
    short   s_reidx;
    short   s_onidx;
//  short   s_wkidx;
//  char    ch_cmdprm[64];
    char    ch_buf[100];

    /* コマンド精査 */
    if ((pch_c502->command_info.connection_logical_name.site_name != myinfo.config_info.site_id) ||
        (pch_c502->command_info.connection_logical_name.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c502->command_info.connection_logical_name.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ-NW-ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r502->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コマンド受信メッセージ */
//  memset(ch_cmdprm, 0x00, sizeof(ch_cmdprm));
//  memcpy(ch_cmdprm, (char *)&pch_c502->command_info.connection_logical_name,
//      sizeof(pch_c502->command_info.connection_logical_name));
//  LSTN_message_output(DEF_EVT_CMD_RCV, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", ch_cmdprm, DEF_IPC_CMD_FL_RE_READ, "");

    /* コマンドレベル */
    if (pch_c502->command_info.connection_logical_name.interface_name[0] != ' ') {
        if (pch_c502->command_info.connection_logical_name.station_name[0] != ' ') {
            s_cmdlvl = LSTN_CMDLVL_STATION;
        } else {
            s_cmdlvl = LSTN_CMDLVL_INTERFACE;
        }
    } else {
        s_cmdlvl = LSTN_CMDLVL_GROUP;
    }

    /*----------------------------------------------*/
    /* リスンポート状態チェック                     */
    /*----------------------------------------------*/
    for (s_onidx = 0; s_onidx < myinfo.table_info[LSTN_TBL_ONLINE].listenporttbl_cnt; s_onidx++) {
        if ((s_cmdlvl == LSTN_CMDLVL_GROUP) ||
            ((s_cmdlvl == LSTN_CMDLVL_INTERFACE) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, sizeof(lpttbl[s_onidx].interface_id))==0)) ||
            ((s_cmdlvl == LSTN_CMDLVL_STATION) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, sizeof(lpttbl[s_onidx].interface_id))==0) &&
             (memcmp(lpttbl[s_onidx].station_id,
                pch_c502->command_info.connection_logical_name.station_name, sizeof(lpttbl[s_onidx].station_id))==0))) {
            if (lpttbl[s_onidx].ctl.listen_fd != LSTN_FILE_CLOSED) {
//              /* オープン中で異なるIPやポート番号への変更は不可 */
//              if ((memcmp(rlpttbl[s_reidx].ctl.src_ip_text,
//                      lpttbl[s_onidx].ctl.src_ip_text, sizeof(lpttbl[s_onidx].ctl.src_ip_text)) != 0) ||
//                  (rlpttbl[s_reidx].ctl.src_port_no != lpttbl[s_onidx].ctl.src_port_no)) {
                    memset(ch_buf, 0x00, sizeof(ch_buf));
                    sprintf(ch_buf, "%.1s%.1s%.5s%.5s%.6s%.6s",
                        &myinfo.config_info.site_id,
                        &myinfo.config_info.network_id,
                        myinfo.config_info.group_id,
                        lpttbl[s_onidx].interface_id,
                        lpttbl[s_onidx].station_id,
                        lpttbl[s_onidx].listen_id);
                    /* エラーメッセージ出力 */
                    LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_RE_READ_ERR, "@f@K@X",
                        DEF_FL_LIN_MG, ch_buf, " ｾﾂｿﾞｸﾁｭｳ");
//                  LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//                      "", "", DEF_IPC_CMD_FL_RE_READ, "NG");

                    /* エラー応答 */
                    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
                    memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
                    memcpy(pch_r502->common_header.interface_code,
                        DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
                    pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
                    memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
                    pch_r502->common_header.control_data_length = 0;
                    LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
                    return;
//              }
            }
        }
    }

    /*----------------------------------------------*/
    /* 再編用管理テーブル初期化処理                 */
    /*----------------------------------------------*/
    /* テーブル件数リセット */
    myinfo.table_info[LSTN_TBL_RELOAD].listenporttbl_cnt = 0;
    myinfo.table_info[LSTN_TBL_RELOAD].controlservertbl_cnt = 0;
    myinfo.table_info[LSTN_TBL_RELOAD].listenertbl_cnt = 0;
    myinfo.table_info[LSTN_TBL_RELOAD].connectiontbl_cnt = 0;
    myinfo.table_info[LSTN_TBL_RELOAD].counttbl_cnt = 0;

    /* リスンポート管理テーブル初期化 */
    LSTN_listenporttbl_init(LSTN_TBL_RELOAD);

    /* リスナー管理テーブル初期化 */
    LSTN_listenertbl_init(LSTN_TBL_RELOAD);

    /* コネクション制御管理テーブル初期化 */
    LSTN_controltbl_init(LSTN_TBL_RELOAD);

    /* コネクション管理テーブル初期化 */
    LSTN_connectiontbl_init(LSTN_TBL_RELOAD);

    /* コネクション数管理テーブル初期化 */
    LSTN_counttbl_init(LSTN_TBL_RELOAD);

    /*----------------------------------------------*/
    /* 回線管理ファイルオープン                     */
    /*----------------------------------------------*/
    s_rtncd = LSTN_linefile_open();
    if (s_rtncd != LSTN_GET_NORMAL) {
//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /*----------------------------------------------*/
    /* 再編用管理テーブル作成処理                   */
    /*----------------------------------------------*/
    /* リスンポート情報取得処理 */
    s_rtncd = LSTN_file_reload_listen(s_cmdlvl);
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* 回線管理ファイルクローズ */
        LSTN_linefile_close();

//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* リスンポート管理テーブル登録済みチェック */
    for (s_reidx = 0; s_reidx < myinfo.table_info[LSTN_TBL_RELOAD].listenporttbl_cnt; s_reidx++) {
        /* オンライン状態チェック */
        s_onidx = LSTN_listenporttbl_search(
            LSTN_TBL_ONLINE,
            rlpttbl[s_reidx].interface_id,
            rlpttbl[s_reidx].station_id,
            rlpttbl[s_reidx].listen_id);
        if (s_onidx > LSTN_TBL_NOT_ENTRY) {
            memcpy((char *)&rlpttbl[s_reidx].ctl, (char *)&lpttbl[s_onidx].ctl, sizeof(lpttbl[s_onidx].ctl));
        }
    }

    /* リスンポート管理テーブル削除チェック */
    for (s_onidx = 0; s_onidx < myinfo.table_info[LSTN_TBL_ONLINE].listenporttbl_cnt; s_onidx++) {
        if ((s_cmdlvl == LSTN_CMDLVL_GROUP) ||
            ((s_cmdlvl == LSTN_CMDLVL_INTERFACE) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, sizeof(lpttbl[s_onidx].interface_id))==0)) ||
            ((s_cmdlvl == LSTN_CMDLVL_STATION) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, sizeof(lpttbl[s_onidx].interface_id))==0) &&
             (memcmp(lpttbl[s_onidx].station_id,
                pch_c502->command_info.connection_logical_name.station_name, sizeof(lpttbl[s_onidx].station_id))==0))) {
            if (lpttbl[s_onidx].ctl.listen_fd != LSTN_FILE_CLOSED) {
                /* オープン中でテーブル削除は不可 */
                s_reidx = LSTN_listenporttbl_search(
                    LSTN_TBL_RELOAD,
                    lpttbl[s_onidx].interface_id,
                    lpttbl[s_onidx].station_id,
                    lpttbl[s_onidx].listen_id);
                if (s_reidx == LSTN_TBL_NOT_ENTRY) {
                    /* 回線管理ファイルクローズ */
                    LSTN_linefile_close();

                    memset(ch_buf, 0x00, sizeof(ch_buf));
                    sprintf(ch_buf, "%.1s%.1s%.5s%.5s%.6s%.6s",
                        &myinfo.config_info.site_id,
                        &myinfo.config_info.network_id,
                        myinfo.config_info.group_id,
                        lpttbl[s_onidx].interface_id,
                        lpttbl[s_onidx].station_id,
                        lpttbl[s_onidx].listen_id);
                    /* エラーメッセージ出力 */
                    LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_RE_READ_ERR, "@f@K@X",
                        DEF_FL_LIN_MG, ch_buf, " ｾﾂｿﾞｸﾁｭｳ");
//                  LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_CMD_ERR_DONE, "@L@C@X@X",
//                      "", "", DEF_IPC_CMD_FL_RE_READ, "NG");

                    /* エラー応答 */
                    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
                    memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
                    memcpy(pch_r502->common_header.interface_code,
                        DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
                    pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
                    memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
                    pch_r502->common_header.control_data_length = 0;
                    LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
                    return;
                }
            }
        }
    }

    /*----------------------------------------------*/
    /* N/W情報取得処理                              */
    /*----------------------------------------------*/
    s_rtncd = LSTN_get_network_info(LSTN_TBL_RELOAD);
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* 回線管理ファイルクローズ */
        LSTN_linefile_close();

//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /*----------------------------------------------*/
    /* コネクション数情報取得処理                   */
    /*----------------------------------------------*/
    s_rtncd = LSTN_get_count_info(LSTN_TBL_RELOAD);
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* 回線管理ファイルクローズ */
        LSTN_linefile_close();

//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション数管理テーブル登録済みチェック */
    for (s_reidx = 0; s_reidx < myinfo.table_info[LSTN_TBL_RELOAD].counttbl_cnt; s_reidx++) {
        s_onidx = LSTN_counttbl_search(
            LSTN_TBL_ONLINE,
            rcnttbl[s_reidx].interface_id,
            rcnttbl[s_reidx].station_id);
        if (s_onidx > LSTN_TBL_NOT_ENTRY) {
            memcpy((char *)&rcnttbl[s_reidx].ctl, (char *)&cnttbl[s_onidx].ctl, sizeof(cnttbl[s_onidx].ctl));
        }
    }

    /*----------------------------------------------*/
    /* コネクション情報取得処理                     */
    /*----------------------------------------------*/
    s_rtncd = LSTN_file_reload_connection(s_cmdlvl);
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* 回線管理ファイルクローズ */
        LSTN_linefile_close();

//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル登録済みチェック */
    for (s_reidx = 0; s_reidx < myinfo.table_info[LSTN_TBL_RELOAD].connectiontbl_cnt; s_reidx++) {
        s_onidx = LSTN_connectiontbl_search(
            LSTN_TBL_ONLINE,
            rcontbl[s_reidx].interface_id,
            rcontbl[s_reidx].station_id,
            rcontbl[s_reidx].connection_id);
        if (s_onidx > LSTN_TBL_NOT_ENTRY) {
            memcpy((char *)&rcontbl[s_reidx].ctl, (char *)&contbl[s_onidx].ctl, sizeof(contbl[s_onidx].ctl));
        }
    }

    /* コネクション制御管理テーブル(コネクション数管理対象外分) */
    s_rtncd = LSTN_get_controlserver_info(LSTN_TBL_RELOAD);
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* 回線管理ファイルクローズ */
        LSTN_linefile_close();

//      /* エラーメッセージ出力 */
//      LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_RE_READ_ERR, "@L@C@X@X",
//          "", "", DEF_IPC_CMD_FL_RE_READ, "ERROR");

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション制御管理テーブル登録済みチェック */
    for (s_reidx = 0; s_reidx < myinfo.table_info[LSTN_TBL_RELOAD].controlservertbl_cnt; s_reidx++) {
        s_onidx = LSTN_controltbl_search(
            LSTN_TBL_ONLINE,
            rctstbl[s_reidx].serverclass_id);
        if (s_onidx > LSTN_TBL_NOT_ENTRY) {
            memcpy((char *)&rctstbl[s_reidx].ctl, (char *)&ctstbl[s_onidx].ctl, sizeof(ctstbl[s_onidx].ctl));
        }
    }

//  /* リスナー管理テーブル登録済みチェック */
//  for (s_reidx = 0; s_reidx < myinfo.table_info[LSTN_TBL_RELOAD].listenertbl_cnt; s_reidx++) {
//      s_onidx = LSTN_listenertbl_search(
//          LSTN_TBL_ONLINE,
//          rlsntbl[s_reidx].interface_id,
//          rlsntbl[s_reidx].station_id,
//          rlsntbl[s_reidx].listen_id);
//      if (s_onidx > LSTN_TBL_NOT_ENTRY) {
//          memcpy((char *)&rlsntbl[s_reidx].ctl, (char *)&lsntbl[s_onidx].ctl, sizeof(lsntbl[s_onidx].ctl));
//      }
//  }

   /*----------------------------------------------*/
    /* 回線管理ファイルクローズ                     */
    /*----------------------------------------------*/
    LSTN_linefile_close();

    /* テーブル入替え */
    memcpy((char *)&lpttbl[0], (char *)&rlpttbl[0], sizeof(listenport_tbl_def)*LSTN_LISTENPORT_TBL_MAX);
    memcpy((char *)&ctstbl[0], (char *)&rctstbl[0], sizeof(controlserver_tbl_def)*LSTN_CONTROLSERVER_TBL_MAX);
    memcpy((char *)&lsntbl[0], (char *)&rlsntbl[0], sizeof(listener_tbl_def)*LSTN_LISTENER_TBL_MAX);
    memcpy((char *)&contbl[0], (char *)&rcontbl[0], sizeof(connection_tbl_def)*LSTN_CONNECTION_TBL_MAX);
    memcpy((char *)&cnttbl[0], (char *)&rcnttbl[0], sizeof(count_tbl_def)*LSTN_COUNT_TBL_MAX);
    memcpy((char *)&myinfo.table_info[LSTN_TBL_ONLINE],
        (char *)&myinfo.table_info[LSTN_TBL_RELOAD], sizeof(myinfo.table_info[LSTN_TBL_RELOAD]));

    /* 回線ステータス反映処理 */
    s_rtncd = LSTN_get_status();
    if (s_rtncd != LSTN_GET_NORMAL) {
        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
        memcpy(pch_r502->common_header.interface_code,
            DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
        pch_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r502->common_header.internal_error_code,
            DEF_NERR_RE_READ_ERR, strlen(DEF_NERR_RE_READ_ERR));
        pch_r502->common_header.control_data_length = 0;
        LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* 受信コネクション数管理情報更新処理 */
    LSTN_set_count_info();

    /* コマンド結果メッセージ */
    LSTN_message_output(DEF_EVT_CONF_RE_READ, '*', DEF_NERR_NOMAL, "@L", "");
//  LSTN_message_output(DEF_EVT_CMD, '*', DEF_NERR_NOMAL, "@L@C@X@X", "", "", DEF_IPC_CMD_FL_RE_READ, "OK");

    /* 応答 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r502, (char *)pch_c502, sizeof(r502_def));
    memcpy(pch_r502->common_header.interface_code,
        DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
    pch_r502->common_header.control_data_length = 0;
    pch_r502->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r502->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(pch_r502->common_header), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

} /* LSTN_file_reload_command */

/****************************************************************************/
/*  FUNCTION        : 11.0.0  LSTN_file_reload_listen                       */
/*  CALLING SEQ.    : short LSTN_file_reload_listen(void)                   */
/*  ARGUMENT        : 1.s_cmdlvl       (I)   コマンド指定レベル             */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : リスンポート情報再取得処理                            */
/****************************************************************************/
short LSTN_file_reload_listen(short s_cmdlvl)
{
    short               s_lptno = LSTN_TBL_NOT_ENTRY;
    short               s_onidx;
//  short               s_reidx;
    short               s_eof = 0;

    /* グループ指定(全件再構築) */
    if (s_cmdlvl == LSTN_CMDLVL_GROUP) {
        return LSTN_get_listen_info(LSTN_TBL_RELOAD);
    }

    /* オンライン用コネクション管理テーブル取得 */
    for (s_onidx = 0; s_onidx < myinfo.table_info[LSTN_TBL_ONLINE].listenporttbl_cnt; s_onidx++) {
        // 指定レコード以外を再構築テーブルにコピーする
        if (((s_cmdlvl == LSTN_CMDLVL_INTERFACE) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_INTERFACE)==0)) ||
            ((s_cmdlvl == LSTN_CMDLVL_STATION) &&
             (memcmp(lpttbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_STATION))==0)) {
            continue;
        }
        s_lptno = LSTN_listenporttbl_move(s_onidx);
        if (s_lptno == LSTN_TBL_NOT_ENTRY) {
            return(LSTN_GET_ERROR);
        }
    }

    /* 再構築用回線管理ファイル取得 */
    memset((char *)p_gflin_rec, 0x00, sizeof(db_gflin_def));
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gflin_io.arg5.key_value,
        (char *)myinfo.config_info.serverclass_id,
        sizeof(myinfo.config_info.serverclass_id));
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_A1, strlen(DEF_COM_IOM_KEYTYPE_A1));
    gflin_io.arg5.key_len = sizeof(p_gflin_rec->alt1_key_info);
    gflin_io.arg5.compare_len = sizeof(p_gflin_rec->alt1_key_info);
    gflin_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;

    /* 回線管理ファイル読込み */
    s_eof = 0;
    while (s_eof == 0) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;
        COM_IOM(gflin_io.func_type,
                gflin_io.sub_prog_sts,
                &gflin_io.arg3,
                &gflin_io.arg4,
                &gflin_io.arg5,
                &gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
            s_eof = 1;
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_LIN_MG,
                "READ",
                gflin_io.arg5.key_value,
                gflin_io.arg6.guardian_errcode);
            return(LSTN_GET_ERROR);
        }

        /* 無効レコードチェック */
        if (p_gflin_rec->invalid_flg == DEF_INVALID_FLG_ON) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* グループ識別チェック */
        if ((p_gflin_rec->pri_key.site_id != myinfo.config_info.site_id) ||
            (p_gflin_rec->pri_key.nw_id != myinfo.config_info.network_id) ||
            (memcmp(p_gflin_rec->pri_key.grp_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id)) !=0)) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* リスンポート管理テーブル追加 */
        if (((s_cmdlvl == LSTN_CMDLVL_INTERFACE) &&
             (memcmp(p_gflin_rec->pri_key.if_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_INTERFACE)==0)) ||
            ((s_cmdlvl == LSTN_CMDLVL_STATION) &&
             (memcmp(p_gflin_rec->pri_key.if_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_STATION))==0)) {
            s_lptno = LSTN_listenporttbl_add(LSTN_TBL_RELOAD, p_gflin_rec);
            if (s_lptno >= 0) {
                memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
                memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
                continue;
            } else {
                return(LSTN_GET_ERROR);
            }
        } else {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_file_reload_listen */

/****************************************************************************/
/*  FUNCTION        : 11.0.0  LSTN_file_reload_connection                   */
/*  CALLING SEQ.    : short LSTN_file_reload_connection(void)               */
/*  ARGUMENT        : 1.s_cmdlvl       (I)   コマンド指定レベル             */
/*  RETURN CODE     : 処理結果(0:正常 -1:異常)                              */
/*  DESCRIPTION     : コネクション情報再取得処理                            */
/****************************************************************************/
short LSTN_file_reload_connection(short s_cmdlvl)
{
    layer_info_def      layer_info;
//  short               s_lsnno = LSTN_TBL_NOT_ENTRY;
    short               s_conno = LSTN_TBL_NOT_ENTRY;
    short               s_onidx;
    short               s_eof = 0;

    /* グループ指定(全件再構築) */
    if (s_cmdlvl == LSTN_CMDLVL_GROUP) {
        return LSTN_get_connection_info(LSTN_TBL_RELOAD);
    }

    /* オンライン用コネクション管理テーブル取得 */
    for (s_onidx = 0; s_onidx < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_onidx++) {
        // 指定レコード以外を再構築テーブルにコピーする
        if (((s_cmdlvl == LSTN_CMDLVL_INTERFACE) &&
             (memcmp(contbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_INTERFACE)==0)) ||
            ((s_cmdlvl == LSTN_CMDLVL_STATION) &&
             (memcmp(contbl[s_onidx].interface_id,
                pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_STATION))==0)) {
            continue;
        }
        s_conno = LSTN_connectiontbl_move(s_onidx);
        if (s_conno == LSTN_TBL_NOT_ENTRY) {
            return(LSTN_GET_ERROR);
        }
    }

    /* 再構築用回線管理ファイル取得 */
    memset((char *)&layer_info, 0x20, sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));

    memset((char *)p_gflin_rec, 0x00, sizeof(db_gflin_def));
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));

    /* コネクション数管理単位 = インタフェース単位 */
    if (s_cmdlvl == LSTN_CMDLVL_INTERFACE) {
        memcpy(layer_info.interface_id, 
            pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_INTERFACE);
        memcpy(gflin_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_INTERFACE);
        gflin_io.arg5.key_len = LSTN_KEYLEN_INTERFACE;
        gflin_io.arg5.compare_len = LSTN_KEYLEN_INTERFACE;
    }
    /* コネクション数管理単位 = ステーション単位 */
    if (s_cmdlvl == LSTN_CMDLVL_STATION) {
        memcpy(layer_info.interface_id, 
            pch_c502->command_info.connection_logical_name.interface_name, LSTN_TBLKEYLEN_STATION);
        memcpy(gflin_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);
        gflin_io.arg5.key_len = LSTN_KEYLEN_STATION;
        gflin_io.arg5.compare_len = LSTN_KEYLEN_STATION;
    }
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gflin_io.arg5.positioning_mode = DEF_POSITION_GENERIC;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;

    /* 回線管理ファイル読込み */
    s_eof = 0;
    while (s_eof == 0) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;
        COM_IOM(gflin_io.func_type,
                gflin_io.sub_prog_sts,
                &gflin_io.arg3,
                &gflin_io.arg4,
                &gflin_io.arg5,
                &gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
            s_eof = 1;
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
            LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
                "", "",
                DEF_FL_LIN_MG,
                "READ",
                gflin_io.arg5.key_value,
                gflin_io.arg6.guardian_errcode);
            return(LSTN_GET_ERROR);
        }

        /* 無効レコードチェック */
        if (p_gflin_rec->invalid_flg == DEF_INVALID_FLG_ON) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* コネクション識別コードチェック */
        if (memcmp((char *)p_gflin_rec->pri_key.connect_id, LSTN_CONNID_SERVER, strlen(LSTN_CONNID_SERVER))!=0) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        }

        /* コネクション管理テーブル追加 */
        s_conno = LSTN_connectiontbl_add(LSTN_TBL_RELOAD, p_gflin_rec);
        if (s_conno >= 0) {
            memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, strlen(DEF_COM_IOM_FUNC_NEXTREAD));
            memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));
            continue;
        } else {
            return(LSTN_GET_ERROR);
        }
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_file_reload_connection */

/****************************************************************************/
/*  FUNCTION        : 23.0.0  LSTN_control_async                            */
/*  CALLING SEQ.    : void LSTN_control_async(void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション非同期要求処理                            */
/****************************************************************************/
void LSTN_control_async(void)
{
    /* 非同期タグチェック */
    if (ctstbl[iocmp.cts_no].ctl.async_tag != LSTN_TAG_NULL) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ﾋﾄﾞｳｷﾆｼﾞｭｳｼﾞｭｼﾝ", ch_rcv_buf);

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, 0, iocmp.rinf.z_messagetag, LSTN_ASYNC_USED, LSTN_FLG_ON);
        return;
    }

    /* コネクション入替・切断指示デキューイング */
    if (ctstbl[iocmp.cts_no].ctl.queue_cnt > 0) {
        LSTN_control_queue_reply(iocmp.cts_no);
        return;
    }

    /* 非同期タグ保管 */
    ctstbl[iocmp.cts_no].ctl.async_tag = iocmp.rinf.z_messagetag;

} /* LSTN_control_async */

/****************************************************************************/
/*  FUNCTION        : 24.0.0  LSTN_control_accept                           */
/*  CALLING SEQ.    : void LSTN_control_accept(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション接続開始要求処理                          */
/****************************************************************************/
void LSTN_control_accept(void)
{
//  short   s_ctsno = LSTN_TBL_NOT_ENTRY;
    short   s_conno = LSTN_TBL_NOT_ENTRY;
    short   s_cntno = LSTN_TBL_NOT_ENTRY;
    short   s_update_cnt = 0;
//  short   s_lptno = LSTN_TBL_NOT_ENTRY;
    short   s_rtncd;

    /* インタフェース精査 */
    if ((pch_c103->line_info.site_name != myinfo.config_info.site_id) ||
        (pch_c103->line_info.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c103->line_info.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r103, (char *)pch_c103, sizeof(r103_def));
        memcpy(pch_r103->common_header.interface_code,
            DEF_IPC_IFCD_CON_START_RSP, strlen(DEF_IPC_IFCD_CON_START_RSP));
        pch_r103->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r103->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r103->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r103_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル取得 */
    s_conno = LSTN_connectiontbl_search(
        LSTN_TBL_ONLINE,
        pch_c103->line_info.interface_name,
        pch_c103->line_info.station_name,
        pch_c103->line_info.connection_name);
    if (s_conno == LSTN_TBL_NOT_ENTRY) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｺﾈｸｼｮﾝﾃｰﾌﾞﾙﾐﾄｳﾛｸ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r103, (char *)pch_c103, sizeof(r103_def));
        memcpy(pch_r103->common_header.interface_code,
            DEF_IPC_IFCD_CON_START_RSP, strlen(DEF_IPC_IFCD_CON_START_RSP));
        pch_r103->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r103->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r103->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r103_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }
    if (contbl[s_conno].tbl.ctstbl_no != iocmp.cts_no) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｶﾝﾘﾃｰﾌﾞﾙﾌｲｯﾁ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r103, (char *)pch_c103, sizeof(r103_def));
        memcpy(pch_r103->common_header.interface_code,
            DEF_IPC_IFCD_CON_START_RSP, strlen(DEF_IPC_IFCD_CON_START_RSP));
        pch_r103->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r103->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r103->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r103_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション状態更新 */
//  memcpy(contbl[s_conno].ctl.connection_sts, DEF_CONNECT_STS_LISTEN, strlen(DEF_CONNECT_STS_LISTEN));

    s_cntno = contbl[s_conno].tbl.cnttbl_no;
//  s_ctsno = contbl[s_conno].tbl.ctstbl_no;

    s_update_cnt = LSTN_connectiontbl_status(
        s_conno,
//      DEF_CONNECT_STS_LISTEN,
        pch_c103->status_info.connection_status,
        pch_c103->status_info.connection_status_time);

//  /* リスンポートステートチェック */
//  s_lptno = contbl[s_conno].tbl.lpttbl_no;
//  switch (lpttbl[s_lptno].ctl.listen_state) {
//  case LSTN_ST_DISCONNECT: /* 切断状態 */
//      break;
//
//  case LSTN_ST_RETRY     : /* リトライ待ち状態     */
//      /* タイマキャンセル */
//      i_CC = CANCELTIMEOUT(lpttbl[s_lptno].ctl.retry_tag);
//      if (_status_ne(i_CC)) {
//          LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "CANCELTIMEOUT", i_CC);
//          LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
//      }
//      lpttbl[s_lptno].ctl.retry_tag = LSTN_TAG_NULL;
//
//      /* リスンポートオープン処理 */
//      s_rtncd = LSTN_port_open(s_lptno);
//      if (s_rtncd != 0) {
//          /* エラー応答 */
//          memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
//          memcpy((char *)pch_r103, (char *)pch_c103, sizeof(r103_def));
//          memcpy(pch_r103->common_header.interface_code,
//              DEF_IPC_IFCD_CON_START_RSP, strlen(DEF_IPC_IFCD_CON_START_RSP));
//          pch_r103->common_header.error_code = DEF_IPC_ERRCD_NG;
//          memcpy(pch_r103->common_header.internal_error_code, DEF_NERR_ACCEPT_ERR, strlen(DEF_NERR_ACCEPT_ERR));
//          pch_r103->common_header.control_data_length = 0;
//
//          /* リプライ処理 */
//          LSTN_reply(ch_rsp_buf, sizeof(r103_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
//          return;
//      }
//      break;
//
//  default:
//      break;
//  }

    /* 応答編集 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r103, (char *)pch_c103, sizeof(r103_def));
    memcpy(pch_r103->common_header.interface_code,
        DEF_IPC_IFCD_CON_START_RSP, strlen(DEF_IPC_IFCD_CON_START_RSP));
    pch_r103->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r103->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_r103->common_header.control_data_length = 0;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(r103_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

    /* カウント要否 */
    if (s_update_cnt == 0) {
        return;
    }

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_update_count(s_cntno, s_update_cnt);

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_control_accept */

/****************************************************************************/
/*  FUNCTION        : 25.0.0  LSTN_control_connect                          */
/*  CALLING SEQ.    : void LSTN_control_connect(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション接続完了処理                              */
/****************************************************************************/
void LSTN_control_connect(void)
{
    short   s_conno = LSTN_TBL_NOT_ENTRY;
    short   s_cntno = LSTN_TBL_NOT_ENTRY;
    short   s_update_cnt = 0;
//  char    ch_buf[64];
    short   s_rtncd = 0;

    if ((pch_c104->line_info.site_name != myinfo.config_info.site_id) ||
        (pch_c104->line_info.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c104->line_info.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r104, (char *)pch_c104, sizeof(r104_def));
        memcpy(pch_r104->common_header.interface_code,
            DEF_IPC_IFCD_CON_NT_RSP, strlen(DEF_IPC_IFCD_CON_NT_RSP));
        pch_r104->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r104->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r104->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r104_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル取得 */
    s_conno = LSTN_connectiontbl_search(
        LSTN_TBL_ONLINE,
        pch_c104->line_info.interface_name,
        pch_c104->line_info.station_name,
        pch_c104->line_info.connection_name);
    if (s_conno == LSTN_TBL_NOT_ENTRY) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｺﾈｸｼｮﾝﾃｰﾌﾞﾙﾐﾄｳﾛｸ" , ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r104, (char *)pch_c104, sizeof(r104_def));
        memcpy(pch_r104->common_header.interface_code,
            DEF_IPC_IFCD_CON_NT_RSP, strlen(DEF_IPC_IFCD_CON_NT_RSP));
        pch_r104->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r104->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r104->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r104_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }
    if (contbl[s_conno].tbl.ctstbl_no != iocmp.cts_no) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｶﾝﾘﾃｰﾌﾞﾙﾌｲｯﾁ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r104, (char *)pch_c104, sizeof(r104_def));
        memcpy(pch_r104->common_header.interface_code,
            DEF_IPC_IFCD_CON_NT_RSP, strlen(DEF_IPC_IFCD_CON_NT_RSP));
        pch_r104->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r104->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r104->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r104_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル更新 */
    s_cntno = contbl[s_conno].tbl.cnttbl_no;
    s_update_cnt = LSTN_connectiontbl_status(
        s_conno,
        pch_c104->status_info.connection_status,
        pch_c104->status_info.connection_status_time);

    /* 応答編集 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r104, (char *)pch_c104, sizeof(r104_def));
    memcpy(pch_r104->common_header.interface_code,
        DEF_IPC_IFCD_CON_NT_RSP, strlen(DEF_IPC_IFCD_CON_NT_RSP));
    pch_r104->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r104->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_r104->common_header.control_data_length = 0;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(r104_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

    /* カウント要否 */
    if (s_update_cnt == 0) {
        return;
    }

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_update_count(s_cntno, s_update_cnt);

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_control_connect */

/****************************************************************************/
/*  FUNCTION        : 26.0.0  LSTN_control_disconnect                       */
/*  CALLING SEQ.    : void LSTN_control_disconnect(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション切断完了処理                              */
/****************************************************************************/
void LSTN_control_disconnect(void)
{
    short   s_conno = LSTN_TBL_NOT_ENTRY;
    short   s_cntno = LSTN_TBL_NOT_ENTRY;
    short   s_update_cnt = 0;
//  char    ch_buf[64];
    short   s_rtncd = 0;

    if ((pch_c105->line_info.site_name != myinfo.config_info.site_id) ||
        (pch_c105->line_info.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c105->line_info.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r105, (char *)pch_c105, sizeof(r105_def));
        memcpy(pch_r105->common_header.interface_code,
            DEF_IPC_IFCD_DISCON_NT_RSP, strlen(DEF_IPC_IFCD_DISCON_NT_RSP));
        pch_r105->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r105->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r105->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r105_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル取得 */
    s_conno = LSTN_connectiontbl_search(
        LSTN_TBL_ONLINE,
        pch_c105->line_info.interface_name,
        pch_c105->line_info.station_name,
        pch_c105->line_info.connection_name);
    if (s_conno == LSTN_TBL_NOT_ENTRY) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｺﾈｸｼｮﾝﾃｰﾌﾞﾙﾐﾄｳﾛｸ" , ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r105, (char *)pch_c105, sizeof(r105_def));
        memcpy(pch_r105->common_header.interface_code,
            DEF_IPC_IFCD_DISCON_NT_RSP, strlen(DEF_IPC_IFCD_DISCON_NT_RSP));
        pch_r105->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r105->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r105->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r105_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }
    if (contbl[s_conno].tbl.ctstbl_no != iocmp.cts_no) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｶﾝﾘﾃｰﾌﾞﾙﾌｲｯﾁ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_r105, (char *)pch_c105, sizeof(r105_def));
        memcpy(pch_r105->common_header.interface_code,
            DEF_IPC_IFCD_DISCON_NT_RSP, strlen(DEF_IPC_IFCD_DISCON_NT_RSP));
        pch_r105->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_r105->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_r105->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r105_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* コネクション管理テーブル更新 */
    s_cntno = contbl[s_conno].tbl.cnttbl_no;
    s_update_cnt = LSTN_connectiontbl_status(
        s_conno,
        pch_c105->status_info.connection_status,
        pch_c105->status_info.connection_status_time);

    /* 応答編集 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r105, (char *)pch_c105, sizeof(r105_def));
    memcpy(pch_r105->common_header.interface_code,
        DEF_IPC_IFCD_DISCON_NT_RSP, strlen(DEF_IPC_IFCD_DISCON_NT_RSP));
    pch_r105->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r105->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_r105->common_header.control_data_length = 0;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(r105_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

    /* カウント要否 */
    if (s_update_cnt == 0) {
        return;
    }

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

//    /* 受信コネクション数管理ファイル読込み */
//    LSTN_countfile_read(cnttbl[s_cntno].interface_id, cnttbl[s_cntno].station_id, DEF_COM_IOM_LOCK);
//
//    /* コネクション数更新 */
//    memcpy((char *)p_gcscn_recout, (char *)p_gcscn_recin, sizeof(db_gcscn_def));
//    memset(ch_buf, 0x00, sizeof(ch_buf));
//    memcpy(ch_buf, p_gcscn_recin->connect_num_ctrl_info.crt_connect_num, sizeof(p_gcscn_recin->connect_num_ctrl_info.crt_connect_num));
//    cnttbl[s_cntno].tbl.cur_connection_cnt = _max(0, (short)(atoi(ch_buf) -1));
//    memset(ch_buf, 0x00, sizeof(ch_buf));
//    sprintf(ch_buf, "%04d", cnttbl[s_cntno].tbl.cur_connection_cnt);
//    memcpy(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num, ch_buf, sizeof(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num));
//
//    /* 再接続状態更新 */
//    if ((memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts,
//            DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON))==0) &&
//        (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id,
//            myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id))==0)) {
//        if (cnttbl[s_cntno].ctl.reconnect_cnt == 0) {
//            memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_sts,
//                DEF_RE_CONNECT_STS_OFF, strlen(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts));
//            memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_start_time,
//                ' ', strlen(p_gcscn_recin->connect_num_ctrl_info.re_connect_start_time));
//            memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_svr_cls_id,
//                ' ', strlen(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id));
//        }
//    }
//
//    /* 受信コネクション数管理ファイル更新 */
//    LSTN_countfile_update();

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_update_count(s_cntno, s_update_cnt);

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_control_disconnect */

/****************************************************************************/
/*  FUNCTION        : 27.0.0  LSTN_control_reconnect                        */
/*  CALLING SEQ.    : void LSTN_control_reconnect(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション入替切断完了処理                          */
/****************************************************************************/
void LSTN_control_reconnect(void)
{
    short   s_ctsno = LSTN_TBL_NOT_ENTRY;
    short   s_conno = LSTN_TBL_NOT_ENTRY;
    short   s_cntno = LSTN_TBL_NOT_ENTRY;
    short   s_idx;
    short   s_update_cnt = 0;
    short   s_rtncd = 0;

    s_ctsno = iocmp.cts_no;

    if ((pch_c106->line_info.site_name != myinfo.config_info.site_id) ||
        (pch_c106->line_info.nw_name != myinfo.config_info.network_id) ||
        (memcmp(pch_c106->line_info.group_name,
            myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id))!=0)) {
        LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｴﾗｰ", ch_rcv_buf);

        /* エラー応答 */
        memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
        memcpy((char *)pch_c106, (char *)pch_c106, sizeof(r104_def));
        memcpy(pch_c106->common_header.interface_code,
            DEF_IPC_IFCD_CON_NT_RSP, strlen(DEF_IPC_IFCD_CON_NT_RSP));
        pch_c106->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(pch_c106->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        pch_c106->common_header.control_data_length = 0;

        /* リプライ処理 */
        LSTN_reply(ch_rsp_buf, sizeof(r104_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
        return;
    }

    /* 入替コネクション管理テーブル取得 */
    if (pch_c106->line_info.connection_name[0] == ' ') {
        s_conno = LSTN_TBL_NOT_ENTRY;
    } else {
        s_conno = LSTN_connectiontbl_search(
            LSTN_TBL_ONLINE,
            pch_c106->line_info.interface_name,
            pch_c106->line_info.station_name,
            pch_c106->line_info.connection_name);
//      if (s_conno == LSTN_TBL_NOT_ENTRY) {
//          LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｺﾈｸｼｮﾝﾃｰﾌﾞﾙﾐﾄｳﾛｸ" , ch_rcv_buf);
//
//          /* エラー応答 */
//          memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
//          memcpy((char *)pch_r106, (char *)pch_c106, sizeof(r106_def));
//          memcpy(pch_r106->common_header.interface_code,
//              DEF_IPC_IFCD_CON_SW_NT_RSP, strlen(DEF_IPC_IFCD_CON_SW_NT_RSP));
//          pch_r106->common_header.error_code = DEF_IPC_ERRCD_NG;
//          memcpy(pch_r106->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
//          pch_r106->common_header.control_data_length = 0;
//          LSTN_reply(ch_rsp_buf, sizeof(r106_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
//          return;
//      }
//      if (contbl[s_conno].tbl.ctstbl_no != s_ctsno) {
//          LSTN_message_output(DEF_EVT_REQ_ERR, 'E', DEF_NERR_IPC_SEISA_ERR, "@X@i", "ｶﾝﾘﾃｰﾌﾞﾙﾌｲｯﾁ", ch_rcv_buf);
//
//          /* エラー応答 */
//          memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
//          memcpy((char *)pch_r106, (char *)pch_c106, sizeof(r106_def));
//          memcpy(pch_r106->common_header.interface_code,
//              DEF_IPC_IFCD_CON_SW_NT_RSP, strlen(DEF_IPC_IFCD_CON_SW_NT_RSP));
//          pch_r106->common_header.error_code = DEF_IPC_ERRCD_NG;
//          memcpy(pch_r106->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
//          pch_r106->common_header.control_data_length = 0;
//          LSTN_reply(ch_rsp_buf, sizeof(r106_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);
//          return;
//      }
    }

    /* 再接続状態確認 */
    s_cntno = ctstbl[s_ctsno].ctl.reconnect_cnttbl_no;
    if (s_cntno == LSTN_TBL_NOT_ENTRY) {
        s_cntno = LSTN_counttbl_search(
            LSTN_TBL_ONLINE,
            pch_c106->line_info.interface_name,
            pch_c106->line_info.station_name);
    }
    if (s_cntno != LSTN_TBL_NOT_ENTRY) {
        /* コネクション状態更新 */
        for (s_idx = 0; s_idx < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_idx++) {
            if ((contbl[s_idx].tbl.cnttbl_no == s_cntno) &&
                (contbl[s_idx].tbl.ctstbl_no == s_ctsno)) {
                if (s_idx == s_conno) {
                    s_update_cnt = s_update_cnt +
                        LSTN_connectiontbl_status(
                            s_idx,
                            pch_c106->status_info.connection_status,
                            pch_c106->status_info.connection_status_time);
                } else {
                    if (memcmp(contbl[s_idx].ctl.connection_sts,
                        DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))==0) {
                        s_update_cnt = s_update_cnt +
                            LSTN_connectiontbl_status(
                                s_idx,
                                DEF_CONNECT_STS_LISTEN,
                                pch_c106->status_info.connection_status_time);
                    } else {
                        s_update_cnt = s_update_cnt +
                            LSTN_connectiontbl_status(
                                s_idx,
                                DEF_CONNECT_STS_DISCONN,
                                pch_c106->status_info.connection_status_time);
                    }
                }
            }
        }
    }

    /* 応答編集 */
    memset(ch_rsp_buf, 0x00, sizeof(ch_rsp_buf));
    memcpy((char *)pch_r106, (char *)pch_c106, sizeof(r106_def));
    memcpy(pch_r106->common_header.interface_code,
        DEF_IPC_IFCD_CON_SW_NT_RSP, strlen(DEF_IPC_IFCD_CON_SW_NT_RSP));
    pch_r106->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_r106->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_r106->common_header.control_data_length = 0;

    /* リプライ処理 */
    LSTN_reply(ch_rsp_buf, sizeof(r106_def), iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

    /* カウント要否 */
    if (s_update_cnt == 0) {
        /* 再接続終了処理 */
        LSTN_reconnect_end(s_ctsno);
        return;
    }

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_update_count(s_cntno, s_update_cnt);

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 再接続終了処理 */
    LSTN_reconnect_end(s_ctsno);

} /* LSTN_control_reconnect */

/****************************************************************************/
/*  FUNCTION        : 28.0.0  LSTN_control_close                            */
/*  CALLING SEQ.    : void LSTN_control_close(short)                        */
/*  ARGUMENT        : 1.ctstbl_no      (I)   コネクション制御テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御プロセスクローズ処理                  */
/****************************************************************************/
void LSTN_control_close(short s_ctstbl_no)
{
    short           s_cntno = LSTN_TBL_NOT_ENTRY;
    short           s_conno = LSTN_TBL_NOT_ENTRY;
    short           s_update_cnt[LSTN_COUNT_TBL_MAX];
    short           s_rtncd = 0;
    short           s_time[8];
    char            sdt_time[21];
    long long       ll_time;

    for (s_cntno = 0; s_cntno < LSTN_COUNT_TBL_MAX; s_cntno++) {
        s_update_cnt[s_cntno] = 0;
    }
    memset(sdt_time, 0x00, sizeof(sdt_time));
    COM_SDT(2, (COM_SDT_arg_2_def *)sdt_time, (COM_SDT_arg_3_def *)s_time, &ll_time);

    /* 非同期I/Oリプライ */
    if (ctstbl[s_ctstbl_no].ctl.async_tag != LSTN_TAG_NULL) {
        LSTN_reply(ch_rsp_buf, 0, ctstbl[s_ctstbl_no].ctl.async_tag, 0, LSTN_FLG_OFF);
        ctstbl[s_ctstbl_no].ctl.async_tag = LSTN_TAG_NULL;
    }

    /* コネクション制御状態更新 */
    ctstbl[s_ctstbl_no].ctl.control_status = LSTN_CONTROL_STS_CLOSE;
    ctstbl[s_ctstbl_no].ctl.queue_cnt = 0;
    ctstbl[s_ctstbl_no].ctl.queue_idx = 0;

    /* コネクション状態更新 */
    for (s_conno = 0; s_conno < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_conno++) {
        if (contbl[s_conno].tbl.ctstbl_no == s_ctstbl_no) {
            s_cntno = contbl[s_conno].tbl.cnttbl_no;
            s_update_cnt[s_cntno] = s_update_cnt[s_cntno] +
                LSTN_connectiontbl_status(s_conno, DEF_CONNECT_STS_DISCONN, sdt_time);
        }
    }

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* コネクション数管理ファイル更新 */
    for (s_cntno = 0; s_cntno < myinfo.table_info[LSTN_TBL_ONLINE].counttbl_cnt; s_cntno++) {
        if (s_update_cnt[s_cntno] != 0) {
            LSTN_countfile_update_count(s_cntno, s_update_cnt[s_cntno]);
        }
    }

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 再接続終了処理 */
    LSTN_reconnect_end(s_ctstbl_no);

} /* LSTN_control_close */

/****************************************************************************/
/*  FUNCTION        : 67.0.0  LSTN_control_queue                            */
/*  CALLING SEQ.    : void LSTN_control_queue(short,char*,short)            */
/*  ARGUMENT        : 1.ctstbl_no      (I)   コネクション制御テーブル番号   */
/*  ARGUMENT        : 2.que_msg        (I)   キューメッセージ               */
/*  ARGUMENT        : 3.que_len        (I)   キューメッセージ長             */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御プロセスキューイング処理              */
/****************************************************************************/
void LSTN_control_queue(short s_ctstbl_no, char *que_msg, short s_que_len)
{
    short       que_no;

    if ((ctstbl[s_ctstbl_no].ctl.queue_cnt >= LSTN_CONTROLSERVER_QUE_MAX) ||
        (s_que_len <= 0)) {
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }
    que_no = (short)((ctstbl[s_ctstbl_no].ctl.queue_idx + ctstbl[s_ctstbl_no].ctl.queue_cnt) % LSTN_CONTROLSERVER_QUE_MAX);
    memcpy(ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_buf, que_msg, s_que_len);
    ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_len = s_que_len;

    ctstbl[s_ctstbl_no].ctl.queue_cnt++;
} /* LSTN_control_queue */

/****************************************************************************/
/*  FUNCTION        : 67.0.0  LSTN_control_dequeue                          */
/*  CALLING SEQ.    : void LSTN_control_dequeue(short)                      */
/*  ARGUMENT        : 1.ctstbl_no      (I)   コネクション制御テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御プロセスデキューリプライ処理          */
/****************************************************************************/
void LSTN_control_queue_reply(short s_ctstbl_no)
{
    short       que_no;

    if (ctstbl[s_ctstbl_no].ctl.queue_cnt == 0) {
        return;
    }
    que_no = ctstbl[s_ctstbl_no].ctl.queue_idx;
    LSTN_reply(ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_buf,
        ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_len,
        iocmp.rinf.z_messagetag, 0, LSTN_FLG_ON);

    memset(ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_buf, 0x00, 
        sizeof(ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_buf));
    ctstbl[s_ctstbl_no].ctl.queue_tbl[que_no].queue_len = 0;

    ctstbl[s_ctstbl_no].ctl.queue_idx++;
    if (ctstbl[s_ctstbl_no].ctl.queue_idx >= LSTN_CONTROLSERVER_QUE_MAX) {
        ctstbl[s_ctstbl_no].ctl.queue_idx = 0;
    }
    ctstbl[s_ctstbl_no].ctl.queue_cnt--;
} /* LSTN_control_queue_reply */

/****************************************************************************/
/*  FUNCTION        : 29.0.0  LSTN_port_open                                */
/*  CALLING SEQ.    : short LSTN_port_open(short)                           */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : リスンポートオープン処理                              */
/****************************************************************************/
short LSTN_port_open(short s_lpttbl_no)
{
    int         i_optvalue = 1; /* ON */
    short       s_time[8];
    long long   ll_time;
    short       s_rtncd = 0;
    char        ch_buf[10];

    /* オープン済みチェック */
    if (lpttbl[s_lpttbl_no].ctl.listen_fd != LSTN_FILE_CLOSED) {
        return(LSTN_OPEN_ALREADY);
    }

    /* リトライチェック */
    if (lpttbl[s_lpttbl_no].ctl.retry_tag != LSTN_TAG_NULL) {
        /* タイマキャンセル */
        i_CC = CANCELTIMEOUT(lpttbl[s_lpttbl_no].ctl.retry_tag);
        if (_status_ne(i_CC)) {
            LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "CANCELTIMEOUT", i_CC);
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }
        lpttbl[s_lpttbl_no].ctl.retry_tag = LSTN_TAG_NULL;
    }

    /* TCP/IPプロセス */
    socket_set_inet_name(lpttbl[s_lpttbl_no].ctl.tcpip_name);

    /* SOCKET処理 */
    lpttbl[s_lpttbl_no].ctl.listen_fd = (short)socket_nw(
        AF_INET,                /* address_family */
        SOCK_STREAM,            /* socket_type */
        0,                      /* protocol */
        LSTN_SO_NOWAIT_DEPTH,   /* flags */
        0);                     /* sync */
    if (lpttbl[s_lpttbl_no].ctl.listen_fd == LSTN_FILE_CLOSED) {
        lpttbl[s_lpttbl_no].ctl.so_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SOCKET_GEN_ERR, "@X@E", "socket_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        LSTN_port_open_retry(s_lpttbl_no);
        return(LSTN_OPEN_ERROR);
    }

    /* SOCKETOPTION処理(REUSEADDR) */
    lpttbl[s_lpttbl_no].ctl.so_error = (short)setsockopt_nw(
        lpttbl[s_lpttbl_no].ctl.listen_fd,  /* socket */
        SOL_SOCKET,                     /* level */
        SO_REUSEADDR,                   /* optname */
        (char *)&i_optvalue,            /* optval */
        sizeof(i_optvalue),             /* optlen */
        LSTN_TAG_SOCKET);               /* tag */
    if (lpttbl[s_lpttbl_no].ctl.so_error == -1) {
        lpttbl[s_lpttbl_no].ctl.so_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SETSOCKOPT_ERR, "@X@e", "setsockopt_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    AWAITIOX(&lpttbl[s_lpttbl_no].ctl.listen_fd,,,, myinfo.config_info.socketio_timer);
    FILE_GETINFO_(lpttbl[s_lpttbl_no].ctl.listen_fd, &lpttbl[s_lpttbl_no].ctl.so_error);
    if (lpttbl[s_lpttbl_no].ctl.so_error) {
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SOCKET_GEN_ERR, "@X@E", "setsockopt_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        FILE_CLOSE_(lpttbl[s_lpttbl_no].ctl.listen_fd);
        lpttbl[s_lpttbl_no].ctl.listen_fd = LSTN_FILE_CLOSED;
        LSTN_port_open_retry(s_lpttbl_no);
        return(LSTN_OPEN_ERROR);
    }

    /* BIND処理 */
    lpttbl[s_lpttbl_no].ctl.src_sockaddr.sin_family = AF_INET;
    lpttbl[s_lpttbl_no].ctl.src_sockaddr.sin_addr.s_addr = inet_addr(lpttbl[s_lpttbl_no].ctl.src_ip_text);
    lpttbl[s_lpttbl_no].ctl.src_sockaddr.sin_port = lpttbl[s_lpttbl_no].ctl.src_port_no;
    lpttbl[s_lpttbl_no].ctl.so_error = (short)bind_nw(
        lpttbl[s_lpttbl_no].ctl.listen_fd,                      /* socket */
        (struct sockaddr*)&lpttbl[s_lpttbl_no].ctl.src_sockaddr,/* address_ptr */
        sizeof(lpttbl[s_lpttbl_no].ctl.src_sockaddr),           /* address_len */
        LSTN_TAG_SOCKET);                                       /* tag */
    if (lpttbl[s_lpttbl_no].ctl.so_error == -1) {
        lpttbl[s_lpttbl_no].ctl.so_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_BIND_ERR, "@X@e", "bind_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    AWAITIOX(&lpttbl[s_lpttbl_no].ctl.listen_fd,,,, myinfo.config_info.socketio_timer);
    FILE_GETINFO_(lpttbl[s_lpttbl_no].ctl.listen_fd, &lpttbl[s_lpttbl_no].ctl.so_error);
    if (lpttbl[s_lpttbl_no].ctl.so_error) {
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SOCKET_GEN_ERR, "@X@E", "bind_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        FILE_CLOSE_(lpttbl[s_lpttbl_no].ctl.listen_fd);
        lpttbl[s_lpttbl_no].ctl.listen_fd = LSTN_FILE_CLOSED;
        LSTN_port_open_retry(s_lpttbl_no);
        return(LSTN_OPEN_ERROR);
    }

    /* LISTEN処理 */
    lpttbl[s_lpttbl_no].ctl.so_error = (short)listen(
        lpttbl[s_lpttbl_no].ctl.listen_fd,  /* socket */
        LSTN_MIN_LISTEN);               /* queue_length */

    /* ACCEPT処理 */
    LSTN_accept(s_lpttbl_no);

    /* リスンポート管理テーブル更新 */
    if (s_lpttbl_no == cnttbl[lpttbl[s_lpttbl_no].tbl.cnttbl_no].ctl.reconnect_lpttbl_no) {
        lpttbl[s_lpttbl_no].ctl.listen_state = LSTN_ST_RECONNECT;
    } else {
        lpttbl[s_lpttbl_no].ctl.listen_state = LSTN_ST_ACCEPT;
    }
    memcpy(lpttbl[s_lpttbl_no].ctl.listen_status, DEF_CONNECT_STS_LISTEN, strlen(DEF_CONNECT_STS_LISTEN));
    COM_SDT(2, (COM_SDT_arg_2_def *)lpttbl[s_lpttbl_no].ctl.connect_time, (COM_SDT_arg_3_def *)s_time, &ll_time);

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 回線ステータスファイルREAD処理 */
    LSTN_statusfile_read(
        lpttbl[s_lpttbl_no].interface_id,
        lpttbl[s_lpttbl_no].station_id,
        lpttbl[s_lpttbl_no].listen_id,
        DEF_COM_IOM_LOCK);

    /* 回線ステータスファイル編集処理 */
    memcpy((char *)p_gclst_recout, (char *)p_gclst_recin, sizeof(db_gclst_def));
    memset((char *)&p_gclst_recout->connect_sts_info, 0x20, sizeof(p_gclst_recout->connect_sts_info));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts,
        lpttbl[s_lpttbl_no].ctl.listen_status,
        sizeof(lpttbl[s_lpttbl_no].ctl.listen_status));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts_update_time,
        lpttbl[s_lpttbl_no].ctl.connect_time,
        sizeof(lpttbl[s_lpttbl_no].ctl.connect_time));
    memset((char *)&p_gclst_recout->connect_info, 0x20, sizeof(p_gclst_recout->connect_info));
    memcpy(p_gclst_recout->connect_info.ip_adress_src,
        lpttbl[s_lpttbl_no].ctl.src_ip_text,
        strlen(lpttbl[s_lpttbl_no].ctl.src_ip_text));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%05u", lpttbl[s_lpttbl_no].ctl.src_port_no);
    memcpy(p_gclst_recout->connect_info.port_num_src, ch_buf, strlen(ch_buf));

    /* 回線ステータスファイルUPDATE処理 */
    LSTN_statusfile_update();

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* リスンポートオープン正常完了 */
//  LSTN_message_output();
    return(LSTN_OPEN_NORMAL);

} /* LSTN_port_open */

/****************************************************************************/
/*  FUNCTION        : 30.0.0  LSTN_port_open_retry                          */
/*  CALLING SEQ.    : void LSTN_port_open_retry(short)                      */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスンポートオープンリトライ処理                      */
/****************************************************************************/
void LSTN_port_open_retry(short s_lpttbl_no)
{
    /* リトライ回数チェック */
    if (lpttbl[s_lpttbl_no].ctl.retry_cnt < lpttbl[s_lpttbl_no].ctl.retry_max) {
        lpttbl[s_lpttbl_no].ctl.retry_cnt++;
    } else {
//      LSTN_message_output(DEF_EVT_CONN_SYOGAI, 'E', DEF_NERR_LSTN_ACC_RETRY_OVER, "@X@E", "RETRY OVER", lpttbl[s_lpttbl_no].ctl.retry_cnt);
        return;
    }

    /* リトライタイマー処理 */
    i_CC = SIGNALTIMEOUT(
        lpttbl[s_lpttbl_no].ctl.retry_timer,
        s_lpttbl_no,
        LSTN_TAG_SIGNAL,
        &lpttbl[s_lpttbl_no].ctl.retry_tag);
    if (_status_ne(i_CC)) {
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "SIGNALTIMEOUT", i_CC);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /* リスンポートステート変更 */
    lpttbl[s_lpttbl_no].ctl.listen_state = LSTN_ST_RETRY;

} /* LSTN_port_open_retry */

/****************************************************************************/
/*  FUNCTION        : 31.0.0  LSTN_port_close                               */
/*  CALLING SEQ.    : void LSTN_port_close(short)                           */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスンポートクローズ処理                              */
/****************************************************************************/
void LSTN_port_close(short s_lpttbl_no)
{
    short       s_time[8];
    long long   ll_time;
    short       s_rtncd = 0;

    /* リトライチェック */
    if (lpttbl[s_lpttbl_no].ctl.listen_state == LSTN_ST_RETRY) {
        /* タイマキャンセル */
        i_CC = CANCELTIMEOUT(lpttbl[s_lpttbl_no].ctl.retry_tag);
        if (_status_ne(i_CC)) {
            LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "CANCELTIMEOUT", i_CC);
            LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
        }
        lpttbl[s_lpttbl_no].ctl.retry_tag = LSTN_TAG_NULL;
        lpttbl[s_lpttbl_no].ctl.retry_cnt = 0;
    }

    /* ソケット未オープン */
    if (lpttbl[s_lpttbl_no].ctl.listen_fd == LSTN_FILE_CLOSED) {
        return;
    }

    /* ソケットクローズ */
    FILE_CLOSE_(lpttbl[s_lpttbl_no].ctl.listen_fd);

    /* リスンポート管理テーブル更新 */
    lpttbl[s_lpttbl_no].ctl.listen_fd = LSTN_FILE_CLOSED;
    lpttbl[s_lpttbl_no].ctl.so_tag = LSTN_TAG_NULL;
    lpttbl[s_lpttbl_no].ctl.listen_state = LSTN_ST_DISCONNECT;
    memcpy(lpttbl[s_lpttbl_no].ctl.listen_status, DEF_CONNECT_STS_DISCONN, strlen(DEF_CONNECT_STS_DISCONN));
    COM_SDT(2, (COM_SDT_arg_2_def *)lpttbl[s_lpttbl_no].ctl.disconnect_time, (COM_SDT_arg_3_def *)s_time, &ll_time);

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 回線ステータスファイルREAD処理 */
    LSTN_statusfile_read(
        lpttbl[s_lpttbl_no].interface_id,
        lpttbl[s_lpttbl_no].station_id,
        lpttbl[s_lpttbl_no].listen_id,
        DEF_COM_IOM_LOCK);

    /* 回線ステータスファイル編集処理 */
    memcpy((char *)p_gclst_recout, (char *)p_gclst_recin, sizeof(db_gclst_def));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts,
        lpttbl[s_lpttbl_no].ctl.listen_status,
        sizeof(lpttbl[s_lpttbl_no].ctl.listen_status));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts_update_time,
        lpttbl[s_lpttbl_no].ctl.disconnect_time,
        sizeof(lpttbl[s_lpttbl_no].ctl.disconnect_time));
//  memset((char *)&p_gclst_recout->connect_info, 0x20, sizeof(p_gclst_recout->connect_info));
    memset(p_gclst_recout->connect_info.err_code, 0x20, sizeof(p_gclst_recout->connect_info.err_code));

    /* 回線ステータスファイルUPDATE処理 */
    LSTN_statusfile_update();

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* リスンポートクローズ正常完了 */
//  LSTN_message_output();

} /* LSTN_port_close */

/****************************************************************************/
/*  FUNCTION        : 32.0.0  LSTN_accept                                   */
/*  CALLING SEQ.    : void LSTN_accept(short)                               */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ACCEPT処理                                            */
/****************************************************************************/
void LSTN_accept(short s_lpttbl_no)
{
    short   s_rtncd = 0;

    /* タグ生成 */
    s_rtncd = COM_TGM(
        LSTN_COMPONENT_LISTEN,
        (unsigned short)s_lpttbl_no,
        (unsigned short)LSTN_TAG_ACCEPT,
        (unsigned long *)&lpttbl[s_lpttbl_no].ctl.so_tag);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "COM_TGM", s_rtncd);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /* ACCEPT処理 */
    lpttbl[s_lpttbl_no].ctl.acc_sockaddr_len = sizeof(lpttbl[s_lpttbl_no].ctl.acc_sockaddr);
    lpttbl[s_lpttbl_no].ctl.so_error = (short)accept_nw(
        lpttbl[s_lpttbl_no].ctl.listen_fd,                      /* socket */
        (struct sockaddr*)&lpttbl[s_lpttbl_no].ctl.acc_sockaddr,/* from_ptr */
        (int *)&lpttbl[s_lpttbl_no].ctl.acc_sockaddr_len,       /* from_len1 */
        lpttbl[s_lpttbl_no].ctl.so_tag);                        /* tag */
    if (lpttbl[s_lpttbl_no].ctl.so_error == -1) {
        lpttbl[s_lpttbl_no].ctl.so_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_ACCEPT_ERR, "@X@E", "accept_nw", lpttbl[s_lpttbl_no].ctl.so_error);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }
} /* LSTN_accept */

/****************************************************************************/
/*  FUNCTION        : 33.0.0  LSTN_accept_complete                          */
/*  CALLING SEQ.    : void LSTN_accept_complete(short)                      */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ACCEPT完了処理                                        */
/****************************************************************************/
void LSTN_accept_complete(
    short   s_lpttbl_no         /* リスンポート管理テーブル番号 */
    )
{
    unsigned long       ul_sinaddr;
    char                *pch_ntoa;
    short               s_cntno = LSTN_TBL_NOT_ENTRY;
    short               s_ctsno = LSTN_TBL_NOT_ENTRY;
    short               s_conno = LSTN_TBL_NOT_ENTRY;
    char                ch_buf[32];
    COM_SDT_arg_2_def   sdt_chdate;
    COM_SDT_arg_3_def   sdt_sdate;
    long long           ll_tmstamp;
    short               s_rtncd = 0;
    short               s_acccnt = 0;

    /* ACCEPT正常完了 */
    lpttbl[s_lpttbl_no].ctl.so_tag = LSTN_TAG_NULL;
    lpttbl[s_lpttbl_no].ctl.retry_cnt = 0;

    /* 接続先判定 */
    memset(lpttbl[s_lpttbl_no].ctl.acc_ip_text, 0x00, sizeof(lpttbl[s_lpttbl_no].ctl.acc_ip_text));
    pch_ntoa = inet_ntoa(lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_addr);
    memcpy(lpttbl[s_lpttbl_no].ctl.acc_ip_text, pch_ntoa, strlen(pch_ntoa));

    if (strlen(lpttbl[s_lpttbl_no].ctl.dst_ip_text) > 0) {
        ul_sinaddr = inet_addr(lpttbl[s_lpttbl_no].ctl.dst_ip_text);
        if (lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_addr.s_addr != ul_sinaddr) {
            memset(ch_buf, 0x00, sizeof(ch_buf));
            sprintf(ch_buf, "%c-%c-%.5s-%.5s-%.6s-%.6s",
                myinfo.config_info.site_id, 
                myinfo.config_info.network_id, 
                myinfo.config_info.group_id, 
                lpttbl[s_lpttbl_no].interface_id,
                lpttbl[s_lpttbl_no].station_id,
                lpttbl[s_lpttbl_no].listen_id);
            LSTN_message_output(DEF_EVT_ACCEPT_REJECT, 'E', DEF_NERR_LSTN_DST_ADRS_CHK_ERR, "@X@T@A@P@a@p@X",
                ch_buf,
                lpttbl[s_lpttbl_no].ctl.tcpip_name, 
                lpttbl[s_lpttbl_no].ctl.src_ip_text, 
                lpttbl[s_lpttbl_no].ctl.src_port_no,
                lpttbl[s_lpttbl_no].ctl.acc_ip_text, 
                lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
                DEF_REJECT_ADDR_CHK_ERR
                );
            LSTN_accept_reject(s_lpttbl_no);
            return;
        }
    }

    /* 受信コネクション数管理ファイル読込み */
    LSTN_countfile_read(lpttbl[s_lpttbl_no].interface_id, lpttbl[s_lpttbl_no].station_id, DEF_COM_IOM_NOLOCK);

    s_cntno = lpttbl[s_lpttbl_no].tbl.cnttbl_no;
    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gcscn_recin->connect_num_ctrl_info.crt_connect_num, sizeof(p_gcscn_recin->connect_num_ctrl_info.crt_connect_num));
    cnttbl[s_cntno].tbl.cur_connection_cnt = (short)atoi(ch_buf);

    /* コネクション数判定 */
    if (cnttbl[s_cntno].tbl.cur_connection_cnt >= cnttbl[s_cntno].tbl.max_connection_cnt) {
        /* コネクション数オーバー */
        LSTN_connection_over(s_lpttbl_no);
        return;
    }

    /* 接続待ちコネクション管理テーブル取得 */
    for (s_conno = 0; s_conno < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_conno++) {
        if (contbl[s_conno].tbl.lpttbl_no == s_lpttbl_no) {
            if (memcmp(contbl[s_conno].ctl.connection_sts, DEF_CONNECT_STS_LISTEN, strlen(DEF_CONNECT_STS_LISTEN))==0) {
                /* 接続待ちコネクション */
                break;
            }
            if (memcmp(contbl[s_conno].ctl.connection_sts, DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))==0) {
                /* 接続済みコネクション */
                s_acccnt++;
            }
        }
    }
    if (s_conno >= myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt) {
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf, "%c-%c-%.5s-%.5s-%.6s-%.6s",
            myinfo.config_info.site_id, 
            myinfo.config_info.network_id, 
            myinfo.config_info.group_id, 
            lpttbl[s_lpttbl_no].interface_id,
            lpttbl[s_lpttbl_no].station_id,
            lpttbl[s_lpttbl_no].listen_id);
        if (s_conno == s_acccnt) {
            /* 全て接続済み */
            LSTN_message_output(DEF_EVT_ACCEPT_REJECT, 'E', DEF_NERR_LSTN_ALL_TBL_CON, "@X@T@A@P@a@p@X",
                ch_buf,
                lpttbl[s_lpttbl_no].ctl.tcpip_name, 
                lpttbl[s_lpttbl_no].ctl.src_ip_text, 
                lpttbl[s_lpttbl_no].ctl.src_port_no,
                lpttbl[s_lpttbl_no].ctl.acc_ip_text, 
                lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
                DEF_REJECT_ALL_TBL_CON
                );
        } else {
            /* 接続待ちコネクション無し */
            LSTN_message_output(DEF_EVT_ACCEPT_REJECT, 'E', DEF_NERR_LSTN_FREE_CON_NON, "@X@T@A@P@a@p@X",
                ch_buf,
                lpttbl[s_lpttbl_no].ctl.tcpip_name, 
                lpttbl[s_lpttbl_no].ctl.src_ip_text, 
                lpttbl[s_lpttbl_no].ctl.src_port_no,
                lpttbl[s_lpttbl_no].ctl.acc_ip_text, 
                lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
                DEF_REJECT_FREE_CON_NON
            );
        }   
        LSTN_accept_reject(s_lpttbl_no);
        return;
    }

    /* コネクション制御選択 */
    s_ctsno = contbl[s_conno].tbl.ctstbl_no;

    /* コネクション接続通知編集 */
    memset(ch_rsp_buf, ' ', sizeof(ch_rsp_buf));
    memcpy(pch_n101->common_header.interface_code, DEF_IPC_IFCD_CON_NT, strlen(DEF_IPC_IFCD_CON_NT));
    pch_n101->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_n101->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_n101->common_header.control_data_length = 0;

    /* コネクション識別情報 */
    pch_n101->line_info.site_name = myinfo.config_info.site_id;
    pch_n101->line_info.nw_name = myinfo.config_info.network_id;
    memcpy(pch_n101->line_info.group_name,
        myinfo.config_info.group_id,
        sizeof(myinfo.config_info.group_id));
    memcpy(pch_n101->line_info.interface_name,
        contbl[s_conno].interface_id,
        sizeof(contbl[s_conno].interface_id));
    memcpy(pch_n101->line_info.station_name,
        contbl[s_conno].station_id,
        sizeof(contbl[s_conno].station_id));
    memcpy(pch_n101->line_info.connection_name,
        contbl[s_conno].connection_id,
        sizeof(contbl[s_conno].connection_id));

    /* リスンポート識別情報 */
    pch_n101->listen_info.site_name = myinfo.config_info.site_id;
    pch_n101->listen_info.nw_name = myinfo.config_info.network_id;
    memcpy(pch_n101->listen_info.group_name,
        myinfo.config_info.group_id,
        sizeof(myinfo.config_info.group_id));
    memcpy(pch_n101->listen_info.interface_name,
        lpttbl[s_lpttbl_no].interface_id,
        sizeof(lpttbl[s_lpttbl_no].interface_id));
    memcpy(pch_n101->listen_info.station_name,
        lpttbl[s_lpttbl_no].station_id,
        sizeof(lpttbl[s_lpttbl_no].station_id));
    memcpy(pch_n101->listen_info.connection_name,
        lpttbl[s_lpttbl_no].listen_id,
        sizeof(lpttbl[s_lpttbl_no].listen_id));

    /* コネクション情報 */
    memcpy(pch_n101->connection_info.src_ip_address,
        lpttbl[s_lpttbl_no].ctl.src_ip_text,
        strlen(lpttbl[s_lpttbl_no].ctl.src_ip_text));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%05d", lpttbl[s_lpttbl_no].ctl.src_port_no);
    memcpy(pch_n101->connection_info.src_port, ch_buf, strlen(ch_buf));
    memcpy(pch_n101->connection_info.dest_ip_address,
        lpttbl[s_lpttbl_no].ctl.acc_ip_text,
        strlen(lpttbl[s_lpttbl_no].ctl.acc_ip_text));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%05d", lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port);
    memcpy(pch_n101->connection_info.dest_port, ch_buf, strlen(ch_buf));

    /* ソケット情報 */
    memcpy((char *)&pch_n101->socket_info,
        (char *)&lpttbl[s_lpttbl_no].ctl.acc_sockaddr,
        sizeof(lpttbl[s_lpttbl_no].ctl.acc_sockaddr));

    /* コネクション接続通知(非同期応答) */
    if (ctstbl[s_ctsno].ctl.async_tag != LSTN_TAG_NULL) {
        LSTN_reply(ch_rsp_buf, sizeof(n101_def), ctstbl[s_ctsno].ctl.async_tag, 0, LSTN_FLG_ON);
        ctstbl[s_ctsno].ctl.async_tag = LSTN_TAG_NULL;
    } else {
        LSTN_control_queue(s_ctsno, ch_rsp_buf, sizeof(n101_def));
    }

    /* コネクション管理テーブル更新 */
    if (COM_SDT(2, &sdt_chdate, &sdt_sdate, &ll_tmstamp) != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_SDT", 0);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }
    LSTN_connectiontbl_status(s_conno, DEF_CONNECT_STS_CONNECT, (char *)&sdt_chdate);

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    LSTN_countfile_update_count(s_cntno, 1);

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* ACCEPT処理 */
    LSTN_accept(s_lpttbl_no);

} /* LSTN_accept_complete */

/****************************************************************************/
/*  FUNCTION        : 34.0.0  LSTN_accept_error                             */
/*  CALLING SEQ.    : void LSTN_accept_error(short)                         */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ACCEPTエラー処理                                      */
/****************************************************************************/
void LSTN_accept_error(short s_lpttbl_no)
{
    char                *pch_ntoa;
    short               s_rtncd = 0;
    char                ch_buf[32];

    /* タグクリア */
    lpttbl[s_lpttbl_no].ctl.so_tag = LSTN_TAG_NULL;

    /* 接続元情報取得 */
    memset(lpttbl[s_lpttbl_no].ctl.acc_ip_text, 0x00, sizeof(lpttbl[s_lpttbl_no].ctl.acc_ip_text));
    pch_ntoa = inet_ntoa(lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_addr);
    memcpy(lpttbl[s_lpttbl_no].ctl.acc_ip_text, pch_ntoa, strlen(pch_ntoa));

    /* エラーメッセージ出力 */
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%c-%c-%.5s-%.5s-%.6s-%.6s",
        myinfo.config_info.site_id, 
        myinfo.config_info.network_id, 
        myinfo.config_info.group_id, 
        lpttbl[s_lpttbl_no].interface_id,
        lpttbl[s_lpttbl_no].station_id,
        lpttbl[s_lpttbl_no].listen_id);
    LSTN_message_output(DEF_EVT_CONN_SYOGAI, 'E', DEF_NERR_ACCEPT_ERR, "@L@T@a@p@A@P@E",
        ch_buf,
        lpttbl[s_lpttbl_no].ctl.tcpip_name,
        lpttbl[s_lpttbl_no].ctl.acc_ip_text,
        lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
        lpttbl[s_lpttbl_no].ctl.src_ip_text,
        lpttbl[s_lpttbl_no].ctl.src_port_no,
        iocmp.fs_err);

    /* リスンポートクローズ */
    LSTN_port_close(s_lpttbl_no);

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 回線ステータスファイルREAD処理 */
    LSTN_statusfile_read(
        lpttbl[s_lpttbl_no].interface_id,
        lpttbl[s_lpttbl_no].station_id,
        lpttbl[s_lpttbl_no].listen_id,
        DEF_COM_IOM_LOCK);

    /* 回線ステータスファイル編集処理 */
    memcpy((char *)p_gclst_recout, (char *)p_gclst_recin, sizeof(db_gclst_def));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts,
        lpttbl[s_lpttbl_no].ctl.listen_status,
        sizeof(lpttbl[s_lpttbl_no].ctl.listen_status));
    memcpy(p_gclst_recout->connect_sts_info.connect_sts_update_time,
        lpttbl[s_lpttbl_no].ctl.disconnect_time,
        sizeof(lpttbl[s_lpttbl_no].ctl.disconnect_time));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%04d", iocmp.fs_err);
    memcpy(p_gclst_recout->connect_info.err_code, ch_buf, strlen(ch_buf));

    /* 回線ステータスファイルUPDATE処理 */
    LSTN_statusfile_update();

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* リトライタイマー発行処理 */
    i_CC = SIGNALTIMEOUT(
        lpttbl[s_lpttbl_no].ctl.retry_timer,
        s_lpttbl_no,
        LSTN_TAG_SIGNAL,
        &lpttbl[s_lpttbl_no].ctl.retry_tag);
    if (_status_ne(i_CC)) {
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SYSIF_LGC_ERR, "@X@E", "SIGNALTIMEOUT", i_CC);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_accept_error */

/****************************************************************************/
/*  FUNCTION        : 35.0.0  LSTN_accept_reject                            */
/*  CALLING SEQ.    : void LSTN_accept_reject(short)                        */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ACCEPT拒否処理                                        */
/****************************************************************************/
void LSTN_accept_reject(short s_lpttbl_no)
{
    short               s_error;
    short               s_socket_fd;

    /* サーバコネクション用ソケット生成 */
    s_socket_fd = (short)socket_nw(
        AF_INET,                /* address_family */
        SOCK_STREAM,            /* socket_type */
        0,                      /* protocol */
        LSTN_SO_NOWAIT_DEPTH,   /* flags */
        0);                     /* sync */
    if (s_socket_fd == -1) {
        s_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_SOCKET_GEN_ERR, "@X@E", "socket_nw", s_error);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /* サーバコネクション接続処理 */
    s_error = (short)accept_nw2(s_socket_fd,
        (struct sockaddr*)&lpttbl[s_lpttbl_no].ctl.acc_sockaddr, LSTN_TAG_SOCKET);
    if (s_error == -1) {
        s_error = (short)errno;
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_ACCEPT2_ERR, "@X@E", "accept_nw2", s_error);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }
    AWAITIOX(&s_socket_fd,,,, myinfo.config_info.socketio_timer);
    FILE_GETINFO_(s_socket_fd, &s_error);
    if (s_error) {
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_ACCEPT2_ERR, "@X@E", "accept_nw2", s_error);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* サーバコネクション切断処理 */
    FILE_CLOSE_(s_socket_fd);
//  LSTN_message_output();

    /* ACCEPT処理 */
    LSTN_accept(s_lpttbl_no);

} /* LSTN_accept_reject */

/****************************************************************************/
/*  FUNCTION        : 36.0.0  LSTN_connection_over                          */
/*  CALLING SEQ.    : void LSTN_connection_over(short)                      */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクションオーバー処理                              */
/****************************************************************************/
void LSTN_connection_over(short s_lpttbl_no)
{
    char                ch_buf[32];

    /* エラーメッセージ出力 */
    LSTN_message_output(DEF_EVT_CONN_NUM_OVER, 'E', DEF_NERR_LSTN_CON_NUM_OVER, "@L@T@a@p@A@P@3",
        "",
        lpttbl[s_lpttbl_no].ctl.tcpip_name,
        lpttbl[s_lpttbl_no].ctl.acc_ip_text,
        lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
        lpttbl[s_lpttbl_no].ctl.src_ip_text,
        lpttbl[s_lpttbl_no].ctl.src_port_no,
        cnttbl[lpttbl[s_lpttbl_no].tbl.cnttbl_no].tbl.max_connection_cnt);

    /* コネクション数オーバ時処理パターン */
    switch(myinfo.config_info.connection_counter_type) {
    case DEF_CONNECT_NUM_MNG_KIND_A :
        /* 接続拒否 */
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf, "%c-%c-%.5s-%.5s-%.6s-%.6s",
            myinfo.config_info.site_id, 
            myinfo.config_info.network_id, 
            myinfo.config_info.group_id, 
            lpttbl[s_lpttbl_no].interface_id,
            lpttbl[s_lpttbl_no].station_id,
            lpttbl[s_lpttbl_no].listen_id);
        LSTN_message_output(DEF_EVT_ACCEPT_REJECT, 'E', DEF_NERR_LSTN_CON_NUM_OVER, "@X@T@A@P@a@p@X",
            ch_buf,
            lpttbl[s_lpttbl_no].ctl.tcpip_name, 
            lpttbl[s_lpttbl_no].ctl.src_ip_text, 
            lpttbl[s_lpttbl_no].ctl.src_port_no,
            lpttbl[s_lpttbl_no].ctl.acc_ip_text, 
            lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port,
            DEF_REJECT_CON_NUM_OVER
            );
        LSTN_accept_reject(s_lpttbl_no);
        break;
    case DEF_CONNECT_NUM_MNG_KIND_B :
        /* 全コネクション再接続 */
        LSTN_reconnect_start(s_lpttbl_no);
        break;
    default:
        break;
    }
} /* LSTN_connection_over */

/****************************************************************************/
/*  FUNCTION        : 37.0.0  LSTN_reconnect_start                          */
/*  CALLING SEQ.    : void LSTN_reconnect_start(short)                      */
/*  ARGUMENT        : 1.lpttbl_no      (I)   リスンポート管理テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 再接続開始処理                                        */
/****************************************************************************/
void LSTN_reconnect_start(short s_lpttbl_no)
{
    short               s_cntno = LSTN_TBL_NOT_ENTRY;
    short               s_conno = LSTN_TBL_NOT_ENTRY;
    short               s_ctsno = LSTN_TBL_NOT_ENTRY;
    char                ch_buf[64];
    COM_SDT_arg_2_def   sdt_chdate;
    COM_SDT_arg_3_def   sdt_sdate;
    long long           ll_tmstamp;
    short               s_rtncd = 0;

    /* コネクション数管理テーブル取得 */
    s_cntno = lpttbl[s_lpttbl_no].tbl.cnttbl_no;

    if (COM_SDT(2, &sdt_chdate, &sdt_sdate, &ll_tmstamp) != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "COM_SDT", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }
    cnttbl[s_cntno].ctl.reconnect_lpttbl_no = s_lpttbl_no;
    memcpy(cnttbl[s_cntno].ctl.reconnect_status, DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON));
    memcpy(cnttbl[s_cntno].ctl.reconnect_starttime, (char *)&sdt_chdate, sizeof(sdt_chdate));

    /*----------------------------------------------*/
    /* 受信コネクション数管理ファイル更新           */
    /*----------------------------------------------*/
    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }
    LSTN_countfile_read(
        lpttbl[s_lpttbl_no].interface_id,
        lpttbl[s_lpttbl_no].station_id,
        DEF_COM_IOM_LOCK);

    /* 再接続中判定 */
    if (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts,
            DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON))==0) {
        memcpy(cnttbl[s_cntno].ctl.reconnect_serverclass,
            p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id, sizeof(cnttbl[s_cntno].ctl.reconnect_serverclass));

        LSTN_countfile_unlock();

        /* ENDTRANSACTION */
        s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
        if (s_rtncd != 0) {
            LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
            LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
        }

        /* コネクション制御切断通知待ち */
        for (s_conno = 0; s_conno < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_conno++) {
            if (contbl[s_conno].tbl.cnttbl_no == s_cntno) {
                s_ctsno = contbl[s_conno].tbl.ctstbl_no;
                if (ctstbl[s_ctsno].ctl.reconnect_cnttbl_no == LSTN_TBL_NOT_ENTRY) {
                    ctstbl[s_ctsno].ctl.reconnect_cnttbl_no = s_cntno;
                    cnttbl[s_cntno].ctl.reconnect_cnt++;
                }
            }
        }
        return;
    }

    memcpy((char *)p_gcscn_recout, (char *)p_gcscn_recin, sizeof(db_gcscn_def));
    memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_sts, DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON));
    memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_start_time, (char *)&sdt_chdate, sizeof(sdt_chdate));
    memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_svr_cls_id,
        myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id));

    LSTN_countfile_update();

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /*----------------------------------------------*/
    /* コネクション入替&切断指示                    */
    /*----------------------------------------------*/
    /* コネクション入替&切断指示通知編集(最初の1件のみ) */
    memset(ch_rsp_buf, ' ', sizeof(ch_rsp_buf));
    memcpy(pch_n102->common_header.interface_code, DEF_IPC_IFCD_CON_SW_NT, strlen(DEF_IPC_IFCD_CON_SW_NT));
    pch_n102->common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(pch_n102->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    pch_n102->common_header.control_data_length = 0;

    /* リスンポート識別情報 */
    pch_n102->listen_info.site_name = myinfo.config_info.site_id;
    pch_n102->listen_info.nw_name = myinfo.config_info.network_id;
    memcpy(pch_n102->listen_info.group_name,
        myinfo.config_info.group_id,
        sizeof(myinfo.config_info.group_id));
    memcpy(pch_n102->listen_info.interface_name,
        lpttbl[s_lpttbl_no].interface_id,
        sizeof(lpttbl[s_lpttbl_no].interface_id));
    memcpy(pch_n102->listen_info.station_name,
        lpttbl[s_lpttbl_no].station_id,
        sizeof(lpttbl[s_lpttbl_no].station_id));
    memcpy(pch_n102->listen_info.connection_name,
        lpttbl[s_lpttbl_no].listen_id,
        sizeof(lpttbl[s_lpttbl_no].listen_id));

    /* コネクション情報 */
    memcpy(pch_n102->connection_info.src_ip_address,
        lpttbl[s_lpttbl_no].ctl.src_ip_text,
        strlen(lpttbl[s_lpttbl_no].ctl.src_ip_text));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%05d", lpttbl[s_lpttbl_no].ctl.src_port_no);
    memcpy(pch_n102->connection_info.src_port, ch_buf, strlen(ch_buf));
    memcpy(pch_n102->connection_info.dest_ip_address,
        lpttbl[s_lpttbl_no].ctl.acc_ip_text,
        strlen(lpttbl[s_lpttbl_no].ctl.acc_ip_text));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%05d", lpttbl[s_lpttbl_no].ctl.acc_sockaddr.sin_port);
    memcpy(pch_n102->connection_info.dest_port, ch_buf, strlen(ch_buf));

    /* ソケット情報 */
    memcpy((char *)&pch_n102->socket_info,
        (char *)&lpttbl[s_lpttbl_no].ctl.acc_sockaddr,
        sizeof(lpttbl[s_lpttbl_no].ctl.acc_sockaddr));

    /* コネクション切断情報 */
    pch_n102->disconnect_info.disconnect_info = myinfo.config_info.connection_counter_layer;

    /* コネクション制御 */
    for (s_conno = 0; s_conno < myinfo.table_info[LSTN_TBL_ONLINE].connectiontbl_cnt; s_conno++) {
        if (contbl[s_conno].tbl.cnttbl_no == s_cntno) {
            s_ctsno = contbl[s_conno].tbl.ctstbl_no;
            if (ctstbl[s_ctsno].ctl.reconnect_cnttbl_no == LSTN_TBL_NOT_ENTRY) {
                ctstbl[s_ctsno].ctl.reconnect_cnttbl_no = s_cntno;

                /* コネクション識別情報(入替コネクション) */
                if (cnttbl[s_cntno].ctl.reconnect_cnt == 0) {
                    pch_n102->line_info.site_name = myinfo.config_info.site_id;
                    pch_n102->line_info.nw_name = myinfo.config_info.network_id;
                    memcpy(pch_n102->line_info.group_name,
                        myinfo.config_info.group_id,
                        sizeof(myinfo.config_info.group_id));
                    memcpy(pch_n102->line_info.interface_name,
                        contbl[s_conno].interface_id,
                        sizeof(contbl[s_conno].interface_id));
                    memcpy(pch_n102->line_info.station_name,
                        contbl[s_conno].station_id,
                        sizeof(contbl[s_conno].station_id));
                    memcpy(pch_n102->line_info.connection_name,
                        contbl[s_conno].connection_id,
                        sizeof(contbl[s_conno].connection_id));
                }

                /* コネクション入替・切断指示通知送信 */
                if (ctstbl[s_ctsno].ctl.async_tag != LSTN_TAG_NULL) {
                    LSTN_reply(ch_rsp_buf, sizeof(n102_def), ctstbl[s_ctsno].ctl.async_tag, 0, LSTN_FLG_ON);
                    ctstbl[s_ctsno].ctl.async_tag = LSTN_TAG_NULL;
                } else {
                    /* 非同期通知があるまでキューイング */
//                  ctstbl[s_ctsno].ctl.reconnect_queue_flg = LSTN_FLG_ON;
//                  memcpy(ctstbl[s_ctsno].ctl.reconnect_queue_buf, (char *)pch_n102, sizeof(n102_def));
                    LSTN_control_queue(s_ctsno, (char *)pch_n102, sizeof(n102_def));
                }

                /* 入替情報クリア */
                if (cnttbl[s_cntno].ctl.reconnect_cnt == 0) {
                    memset((char *)&pch_n102->line_info, ' ', sizeof(pch_n102->line_info));
//                  memset((char *)&pch_n102->listen_info, ' ', sizeof(pch_n102->listen_info));
                    memset((char *)&pch_n102->connection_info, ' ', sizeof(pch_n102->connection_info));
                    memset((char *)&pch_n102->socket_info, 0x00, sizeof(pch_n102->socket_info));
                }
                cnttbl[s_cntno].ctl.reconnect_cnt++;
            }
        }
    }

    /* リスナーステート */
    lpttbl[s_lpttbl_no].ctl.listen_state = LSTN_ST_RECONNECT;

} /* LSTN_reconnect_start */

/****************************************************************************/
/*  FUNCTION        : 38.0.0  LSTN_reconnect_end                            */
/*  CALLING SEQ.    : void LSTN_reconnect_end(short)                        */
/*  ARGUMENT        : 1.ctstbl_no      (I)   コネクション制御テーブル番号   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 再接続終了処理                                        */
/****************************************************************************/
void LSTN_reconnect_end(short s_ctstbl_no)
{
    short               s_cntno = LSTN_TBL_NOT_ENTRY;
    COM_SDT_arg_2_def   sdt_chdate;
    COM_SDT_arg_3_def   sdt_sdate;
    long long           ll_tmstamp;
    short               s_rtncd = 0;
    char                ch_buf[32];

    /* 再接続処理解除(コネクション制御単位) */
    if (ctstbl[s_ctstbl_no].ctl.reconnect_cnttbl_no == LSTN_TBL_NOT_ENTRY) {
        return;
    }
    s_cntno = ctstbl[s_ctstbl_no].ctl.reconnect_cnttbl_no;
    ctstbl[s_ctstbl_no].ctl.reconnect_cnttbl_no = LSTN_TBL_NOT_ENTRY;
    cnttbl[s_cntno].ctl.reconnect_cnt--;

    /* コネクション制御数確認 */
    if (cnttbl[s_cntno].ctl.reconnect_cnt > 0) {
        return;
    }

    /* コネクション数管理テーブル更新 */
    memcpy(cnttbl[s_cntno].ctl.reconnect_status, DEF_RE_CONNECT_STS_OFF, strlen(DEF_RE_CONNECT_STS_OFF));
    if (COM_SDT(2, &sdt_chdate, &sdt_sdate, &ll_tmstamp) != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_NOMAL, "@X@E", "COM_SDT", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }
    memset(cnttbl[s_cntno].ctl.reconnect_serverclass, 0x20, sizeof(cnttbl[s_cntno].ctl.reconnect_serverclass));
    memcpy(cnttbl[s_cntno].ctl.reconnect_endtime, (char *)&sdt_chdate, sizeof(sdt_chdate));

    /* BEGINTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_BEGIN, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(BT)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_read(cnttbl[s_cntno].interface_id, cnttbl[s_cntno].station_id, DEF_COM_IOM_LOCK);

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gcscn_recin->connect_num_ctrl_info.crt_connect_num, sizeof(p_gcscn_recin->connect_num_ctrl_info.crt_connect_num));
    cnttbl[s_cntno].tbl.cur_connection_cnt = (short)atoi(ch_buf);

    if (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id,
        myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id))==0) {
        memcpy((char *)p_gcscn_recout, (char *)p_gcscn_recin, sizeof(db_gcscn_def));
        memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_sts,
            DEF_RE_CONNECT_STS_OFF, strlen(DEF_RE_CONNECT_STS_OFF));
        memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_start_time,
            ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_start_time));
        memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_svr_cls_id,
            ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id));
        LSTN_countfile_update();
    } else {
        /* 他リスナーによる制御 */
        LSTN_countfile_unlock();
    }

    /* ENDTRANSACTION */
    s_rtncd = COM_TMF(DEF_COM_TMF_END, &l_tranid, ch_tranmodule);
    if (s_rtncd != 0) {
        LSTN_message_output(DEF_EVT_COMMON_MOD_ERR, 'E', DEF_NERR_TMF_ERR, "@X@E", "COM_TMF(ET)", s_rtncd);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

    /* ACCEPT処理 */
    LSTN_accept(cnttbl[s_cntno].ctl.reconnect_lpttbl_no);
    cnttbl[s_cntno].ctl.reconnect_lpttbl_no = LSTN_TBL_NOT_ENTRY;

    /* リスナーステート */
    lpttbl[cnttbl[s_cntno].ctl.reconnect_lpttbl_no].ctl.listen_state = LSTN_ST_ACCEPT;

} /* LSTN_reconnect_end */

/****************************************************************************/
/*  FUNCTION        : 39.0.0  LSTN_linefile_open                            */
/*  CALLING SEQ.    : short LSTN_linefile_open(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 処理結果(0:正常 1:EOF -1:エラー)                      */
/*  DESCRIPTION     : 回線管理ファイルOPEN処理                              */
/****************************************************************************/
short LSTN_linefile_open(void)
{
    /*----------------------------------------------*/
    /* ファイルオープン処理                         */
    /*----------------------------------------------*/
    memset((char *)&gflin_io, ' ', sizeof(gflin_io));
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_OPEN, strlen(DEF_COM_IOM_FUNC_OPEN));
    memcpy(gflin_io.arg3.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
    memcpy(gflin_io.arg3.file_id, DEF_FL_LIN_MG, strlen(DEF_FL_LIN_MG));
    memcpy(gflin_io.arg3.file_name, myinfo.config_info.gflin_fname, myinfo.config_info.gflin_fname_len);
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_OPEN, strlen(DEF_FILEIO_OPEN));
    memcpy(gflin_io.arg4.file_id, DEF_GFLIN, strlen(DEF_GFLIN));
    memcpy(gflin_io.arg4.file_name, myinfo.config_info.gflin_fname, myinfo.config_info.gflin_fname_len);
    gflin_io.arg4.file_no = LSTN_FILE_CLOSED;
    gflin_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gflin_io.arg5.part_key_position = 0;
    gflin_io.arg5.part_key_len = 0;
    gflin_io.arg5.key_len = 0;
    gflin_io.arg5.compare_len = 0;
    gflin_io.arg5.positioning_mode = 0;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gflin_io.arg5.rec_len = 0;
    gflin_io.arg6.guardian_errcode = 0;
    gflin_io.arg6.rec_len = 0;

    COM_IOM(gflin_io.func_type,
            gflin_io.sub_prog_sts,
            &gflin_io.arg3,
            &gflin_io.arg4,
            &gflin_io.arg5,
            &gflin_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E",
            "", "", DEF_FL_LIN_MG, DEF_COM_IOM_FUNC_OPEN, "", gflin_io.arg6.guardian_errcode);
        return(LSTN_GET_ERROR);
    }

    return(LSTN_GET_NORMAL);

} /* LSTN_linefile_open */

/****************************************************************************/
/*  FUNCTION        : 42.0.0  LSTN_linefile_close                           */
/*  CALLING SEQ.    : void LSTN_linefile_close(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイルCLOSE処理                       */
/****************************************************************************/
void LSTN_linefile_close(void)
{
    if (gflin_io.arg4.file_no <= 0) {
        return;
    }

    /*----------------------------------------------*/
    /* ファイルクローズ処理                         */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_CLOSE, strlen(DEF_COM_IOM_FUNC_CLOSE));
    memcpy(gflin_io.arg3.file_io_type, DEF_FILEIO_CLOSE, strlen(DEF_FILEIO_CLOSE));
    memset(gflin_io.sub_prog_sts, ' ', sizeof(gclst_io.sub_prog_sts));
    gflin_io.arg6.guardian_errcode = 0;

    COM_IOM(gflin_io.func_type,
            gflin_io.sub_prog_sts,
            &gflin_io.arg3,
            &gflin_io.arg4,
            &gflin_io.arg5,
            &gflin_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_LIN_MG,
            DEF_COM_IOM_FUNC_CLOSE,
            "", gflin_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_linefile_close */

/****************************************************************************/
/*  FUNCTION        : 39.0.0  LSTN_statusfile_open                          */
/*  CALLING SEQ.    : void LSTN_statusfile_open(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイルOPEN処理                        */
/****************************************************************************/
void LSTN_statusfile_open(void)
{
    /*----------------------------------------------*/
    /* ファイルオープン処理                         */
    /*----------------------------------------------*/
    memset((char *)&gclst_io,' ',sizeof(gclst_io));
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_OPEN, strlen(DEF_COM_IOM_FUNC_OPEN));
    memcpy(gclst_io.arg3.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
    memcpy(gclst_io.arg3.file_id, DEF_FL_LIN_STS, strlen(DEF_FL_LIN_STS));
    memcpy(gclst_io.arg3.file_name, myinfo.config_info.gclst_fname, myinfo.config_info.gclst_fname_len);
    memcpy(gclst_io.arg3.file_io_type, DEF_FILEIO_OPEN, strlen(DEF_FILEIO_OPEN));
    memcpy(gclst_io.arg4.file_id, DEF_GCLST, strlen(DEF_GCLST));
    memcpy(gclst_io.arg4.file_name, myinfo.config_info.gclst_fname, myinfo.config_info.gclst_fname_len);
    gclst_io.arg4.file_no = LSTN_FILE_CLOSED;
    gclst_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gclst_io.arg5.part_key_position = 0;
    gclst_io.arg5.part_key_len = 0;
    gclst_io.arg5.key_len = 0;
    gclst_io.arg5.compare_len = 0;
    gclst_io.arg5.positioning_mode = 0;
    gclst_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gclst_io.arg5.rec_len = 0;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    COM_IOM(gclst_io.func_type,
            gclst_io.sub_prog_sts,
            &gclst_io.arg3,
            &gclst_io.arg4,
            &gclst_io.arg5,
            &gclst_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E",
            "", "", DEF_FL_LIN_STS, DEF_COM_IOM_FUNC_OPEN, "", gclst_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_statusfile_open */

/****************************************************************************/
/*  FUNCTION        : 40.0.0  LSTN_statusfile_read                          */
/*  CALLING SEQ.    : void LSTN_statusfile_read(char*,char*,char*,short)    */
/*  ARGUMENT        : 1.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 2.station_id     (I)   ステーション識別               */
/*  ARGUMENT        : 3.connection_id  (I)   コネクション識別               */
/*  ARGUMENT        : 4.lock_mode      (I)   ロックモード                   */
/*  RETURN CODE     : 処理結果(0:正常 1:EOF -1:エラー)                      */
/*  DESCRIPTION     : 回線ステータスファイルREAD処理                        */
/****************************************************************************/
short LSTN_statusfile_read(
    char    *pch_interface_id,
    char    *pch_station_id,
    char    *pch_connection_id,
    short   s_lock_mode)
{
    layer_info_def      layer_info;

    memset((char *)p_gclst_recin, ' ', sizeof(db_gclst_def));
    memset((char *)p_gclst_recout, ' ', sizeof(db_gclst_def));

    /* キー編集 */
    memset((char *)&layer_info, ' ', sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memcpy(layer_info.interface_id, pch_interface_id, sizeof(layer_info.interface_id));
    memcpy(layer_info.station_id, pch_station_id, sizeof(layer_info.station_id));
    memcpy(layer_info.connection_id, pch_connection_id, sizeof(layer_info.connection_id));

    /* ファイルREAD処理 */
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gclst_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memcpy(gclst_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    memcpy(gclst_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_CONNECTION);
    gclst_io.arg5.key_len = LSTN_KEYLEN_CONNECTION;
    gclst_io.arg5.compare_len = LSTN_KEYLEN_CONNECTION;
    gclst_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gclst_io.arg5.lock_flg = s_lock_mode;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gclst_io.arg5.rec_len = db_gclst_def_Size;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    COM_IOM(gclst_io.func_type,
            gclst_io.sub_prog_sts,
            &gclst_io.arg3,
            &gclst_io.arg4,
            &gclst_io.arg5,
            &gclst_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_LIN_STS,
            DEF_COM_IOM_FUNC_STARTREAD,
            gclst_io.arg5.key_value,
            gclst_io.arg6.guardian_errcode);
        return(LSTN_FILEIO_ERROR);
    }
    return(LSTN_FILEIO_NORMAL);

} /* LSTN_statusfile_read */

/****************************************************************************/
/*  FUNCTION        : 41.0.0  LSTN_statusfile_update                        */
/*  CALLING SEQ.    : void LSTN_statusfile_update(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイルUPDATE処理                      */
/****************************************************************************/
void LSTN_statusfile_update(void)
{
    /* ファイルUPDATE処理 */
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_UPDATE, strlen(DEF_COM_IOM_FUNC_UPDATE));
    memcpy(gclst_io.arg3.file_io_type, DEF_FILEIO_REWRITE, strlen(DEF_FILEIO_REWRITE));
    gclst_io.arg5.key_len = sizeof(p_gclst_recin->pri_key);
    gclst_io.arg5.compare_len = sizeof(p_gclst_recin->pri_key);
    gclst_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gclst_io.arg5.lock_flg = DEF_COM_IOM_LOCKFREE;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gclst_io.arg5.rec_len = db_gclst_def_Size;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    COM_IOM(gclst_io.func_type,
            gclst_io.sub_prog_sts,
            &gclst_io.arg3,
            &gclst_io.arg4,
            &gclst_io.arg5,
            &gclst_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_LIN_STS,
            DEF_COM_IOM_FUNC_UPDATE,
            gclst_io.arg5.key_value,
            gclst_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_statusfile_update */

/****************************************************************************/
/*  FUNCTION        : 42.0.0  LSTN_statusfile_close                          */
/*  CALLING SEQ.    : void LSTN_statusfile_close(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイルCLOSE処理                       */
/****************************************************************************/
void LSTN_statusfile_close(void)
{
    if (gclst_io.arg4.file_no <= 0) {
        return;
    }

    /*----------------------------------------------*/
    /* ファイルクローズ処理                         */
    /*----------------------------------------------*/
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_CLOSE, strlen(DEF_COM_IOM_FUNC_CLOSE));
    memcpy(gclst_io.arg3.file_io_type, DEF_FILEIO_CLOSE, strlen(DEF_FILEIO_CLOSE));
    memset(gclst_io.sub_prog_sts, ' ', sizeof(gclst_io.sub_prog_sts));
    gclst_io.arg6.guardian_errcode = 0;

    COM_IOM(gclst_io.func_type,
            gclst_io.sub_prog_sts,
            &gclst_io.arg3,
            &gclst_io.arg4,
            &gclst_io.arg5,
            &gclst_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_LIN_STS,
            DEF_COM_IOM_FUNC_CLOSE,
            "", gclst_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_statusfile_close */

/****************************************************************************/
/*  FUNCTION        : 43.0.0  LSTN_countfile_open                           */
/*  CALLING SEQ.    : void LSTN_countfile_open(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 受信コネクション数管理ファイルOPEN処理                */
/****************************************************************************/
void LSTN_countfile_open(void)
{
    /*----------------------------------------------*/
    /* 受信コネクション数管理ファイルオープン処理   */
    /*----------------------------------------------*/
    memset((char *)&gcscn_io, ' ', sizeof(gcscn_io));
    memcpy(gcscn_io.func_type, DEF_COM_IOM_FUNC_OPEN, strlen(DEF_COM_IOM_FUNC_OPEN));
    memcpy(gcscn_io.arg3.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
    memcpy(gcscn_io.arg3.file_id, DEF_FL_RCV_CON_NUM, strlen(DEF_FL_RCV_CON_NUM));
    memcpy(gcscn_io.arg3.file_name, myinfo.config_info.gcscn_fname, myinfo.config_info.gcscn_fname_len);
    memcpy(gcscn_io.arg3.file_io_type, DEF_FILEIO_OPEN, strlen(DEF_FILEIO_OPEN));
    memcpy(gcscn_io.arg4.file_id, DEF_GCSCN, strlen(DEF_GCSCN));
    memcpy(gcscn_io.arg4.file_name, myinfo.config_info.gcscn_fname, myinfo.config_info.gcscn_fname_len);
    gcscn_io.arg4.file_no = LSTN_FILE_CLOSED;
    gcscn_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gcscn_io.arg5.part_key_position = 0;
    gcscn_io.arg5.part_key_len = 0;
    gcscn_io.arg5.key_len = 0;
    gcscn_io.arg5.compare_len = 0;
    gcscn_io.arg5.positioning_mode = 0;
    gcscn_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gcscn_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gcscn_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gcscn_io.arg5.rec_len = 0;
    gcscn_io.arg6.guardian_errcode = 0;
    gcscn_io.arg6.rec_len = 0;

    COM_IOM(gcscn_io.func_type,
            gcscn_io.sub_prog_sts,
            &gcscn_io.arg3,
            &gcscn_io.arg4,
            &gcscn_io.arg5,
            &gcscn_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_OPN_ERR, "@L@C@f@X@K@E",
            "", "", DEF_FL_RCV_CON_NUM, DEF_COM_IOM_FUNC_OPEN, "", gcscn_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_countfile_open */

/****************************************************************************/
/*  FUNCTION        : 44.0.0  LSTN_countfile_read                           */
/*  CALLING SEQ.    : short LSTN_countfile_read(char*,char*,short)          */
/*  ARGUMENT        : 1.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 2.station_id     (I)   ステーション識別               */
/*  ARGUMENT        : 3.lock_mode      (I)   ロックモード                   */
/*  RETURN CODE     : 処理結果(0:正常 1:EOF -1:エラー)                      */
/*  DESCRIPTION     : 受信コネクション数管理ファイルREAD処理                */
/****************************************************************************/
short LSTN_countfile_read(
    char    *pch_interface_id,
    char    *pch_station_id,
    short   s_lock_mode
    )
{
    layer_info_def      layer_info;

    /* キー編集 */
    memset((char *)&layer_info, ' ', sizeof(layer_info));
    layer_info.site_id = myinfo.config_info.site_id;
    layer_info.network_id = myinfo.config_info.network_id;
    memcpy(layer_info.group_id, myinfo.config_info.group_id, sizeof(myinfo.config_info.group_id));
    memcpy(layer_info.interface_id, pch_interface_id, sizeof(layer_info.interface_id));
    if (myinfo.config_info.connection_counter_layer == DEF_CONNECT_NUM_MNG_LYR_ST) {
        memcpy(layer_info.station_id, pch_station_id, sizeof(layer_info.station_id));
    }
    memcpy(gcscn_io.arg5.key_value, (char *)&layer_info, LSTN_KEYLEN_STATION);
    gcscn_io.arg5.key_len = LSTN_KEYLEN_STATION;
    gcscn_io.arg5.compare_len = LSTN_KEYLEN_STATION;
    memcpy(gcscn_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, strlen(DEF_COM_IOM_FUNC_STARTREAD));
    memcpy(gcscn_io.arg3.file_io_type, DEF_FILEIO_START, strlen(DEF_FILEIO_START));
    memset(gcscn_io.sub_prog_sts, ' ', sizeof(gcscn_io.sub_prog_sts));
    memcpy(gcscn_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gcscn_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gcscn_io.arg5.lock_flg = s_lock_mode;
    gcscn_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gcscn_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gcscn_io.arg5.rec_len = db_gcscn_def_Size;
    gcscn_io.arg6.guardian_errcode = 0;
    gcscn_io.arg6.rec_len = 0;

    COM_IOM(gcscn_io.func_type,
            gcscn_io.sub_prog_sts,
            &gcscn_io.arg3,
            &gcscn_io.arg4,
            &gcscn_io.arg5,
            &gcscn_io.arg6);

    /* 処理結果判定 */
//  if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, 2) == 0) {
//      return(LSTN_FILEIO_EOF);
//  }
    if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_RCV_CON_NUM,
            DEF_COM_IOM_FUNC_STARTREAD,
            gcscn_io.arg5.key_value,
            gcscn_io.arg6.guardian_errcode);
        return(LSTN_FILEIO_ERROR);
    }
    return(LSTN_FILEIO_NORMAL);

} /* LSTN_countfile_read */

/****************************************************************************/
/*  FUNCTION        : 45.0.0  LSTN_countfile_update                         */
/*  CALLING SEQ.    : void LSTN_countfile_update(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 受信コネクション数管理ファイルUPDATE処理              */
/****************************************************************************/
void LSTN_countfile_update(void)
{
    /* ファイルUPDATE処理 */
    memcpy(gcscn_io.func_type, DEF_COM_IOM_FUNC_UPDATE, strlen(DEF_COM_IOM_FUNC_UPDATE));
    memcpy(gcscn_io.arg3.file_io_type, DEF_FILEIO_REWRITE, strlen(DEF_FILEIO_REWRITE));

    memcpy(gcscn_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    gcscn_io.arg5.key_len = sizeof(p_gcscn_recin->pri_key);
    gcscn_io.arg5.compare_len = sizeof(p_gcscn_recin->pri_key);
    gcscn_io.arg5.positioning_mode = DEF_POSITION_EXACT;
    gcscn_io.arg5.lock_flg = DEF_COM_IOM_LOCKFREE;
    gcscn_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gcscn_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gcscn_io.arg5.rec_len = db_gcscn_def_Size;
    gcscn_io.arg6.guardian_errcode = 0;
    gcscn_io.arg6.rec_len = 0;

    COM_IOM(gcscn_io.func_type,
            gcscn_io.sub_prog_sts,
            &gcscn_io.arg3,
            &gcscn_io.arg4,
            &gcscn_io.arg5,
            &gcscn_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_RCV_CON_NUM,
            DEF_FILEIO_REWRITE,
            gcscn_io.arg5.key_value,
            gcscn_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_countfile_update */

/****************************************************************************/
/*  FUNCTION        : 46.0.0  LSTN_countfile_unlock                         */
/*  CALLING SEQ.    : void LSTN_countfile_unlock(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 受信コネクション数管理ファイルUNLOCK処理              */
/****************************************************************************/
void LSTN_countfile_unlock(void)
{
    /* ファイルUNLOCK処理 */
    memcpy(gcscn_io.func_type, DEF_COM_IOM_FUNC_UNLOC, strlen(DEF_COM_IOM_FUNC_UNLOC));
    memcpy(gcscn_io.arg3.file_io_type, DEF_FILEIO_UNLOCK, strlen(DEF_FILEIO_UNLOCK));
    gcscn_io.arg5.key_len = 0;
    gcscn_io.arg5.compare_len = 0;
    gcscn_io.arg5.positioning_mode = 0;
    gcscn_io.arg5.positioning_mode = 0;
    gcscn_io.arg5.lock_flg = DEF_COM_IOM_LOCKFREE;
    gcscn_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gcscn_io.arg5.io_timer = myinfo.config_info.fileio_timer;
    gcscn_io.arg5.rec_len = 0;
    gcscn_io.arg6.guardian_errcode = 0;
    gcscn_io.arg6.rec_len = 0;

    COM_IOM(gcscn_io.func_type,
            gcscn_io.sub_prog_sts,
            &gcscn_io.arg3,
            &gcscn_io.arg4,
            &gcscn_io.arg5,
            &gcscn_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_RCV_CON_NUM,
            DEF_FILEIO_UNLOCK,
            gcscn_io.arg5.key_value,
            gcscn_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_countfile_unlock */

/****************************************************************************/
/*  FUNCTION        : 47.0.0  LSTN_countfile_close                          */
/*  CALLING SEQ.    : void LSTN_countfile_close(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 受信コネクション数管理ファイルCLOSE処理               */
/****************************************************************************/
void LSTN_countfile_close(void)
{
    if (gcscn_io.arg4.file_no <= 0) {
        return;
    }

    /*----------------------------------------------*/
    /* ファイルクローズ処理                         */
    /*----------------------------------------------*/
    memcpy(gcscn_io.func_type, DEF_COM_IOM_FUNC_CLOSE, strlen(DEF_COM_IOM_FUNC_CLOSE));
    memcpy(gcscn_io.arg3.file_io_type, DEF_FILEIO_CLOSE, strlen(DEF_FILEIO_CLOSE));
    memset(gcscn_io.sub_prog_sts, ' ', sizeof(gcscn_io.sub_prog_sts));
    gcscn_io.arg6.guardian_errcode = 0;

    COM_IOM(gcscn_io.func_type,
            gcscn_io.sub_prog_sts,
            &gcscn_io.arg3,
            &gcscn_io.arg4,
            &gcscn_io.arg5,
            &gcscn_io.arg6);

    /* 処理結果判定 */
    if (memcmp(gcscn_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, 2) != 0) {
        LSTN_message_output(DEF_EVT_FILE_IO_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@L@C@f@X@K@E",
            "", "",
            DEF_FL_RCV_CON_NUM,
            DEF_COM_IOM_FUNC_CLOSE,
            "",
            gcscn_io.arg6.guardian_errcode);
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_countfile_close */

/****************************************************************************/
/*  FUNCTION        : 48.0.0  LSTN_countfile_update_count                   */
/*  CALLING SEQ.    : short LSTN_countfile_update_count(short, short)       */
/*  ARGUMENT        : 1.table_no       (I)   テーブル番号                   */
/*  ARGUMENT        : 2.sub_cnt        (I)   増減数                         */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 受信コネクション数管理ファイルカウント処理            */
/****************************************************************************/
void LSTN_countfile_update_count(short s_table_no, short s_sub_cnt)
{
    char    ch_buf[64];

    /* 受信コネクション数管理ファイル読込み */
    LSTN_countfile_read(cnttbl[s_table_no].interface_id, cnttbl[s_table_no].station_id, DEF_COM_IOM_LOCK);

    /* コネクション数更新 */
    memcpy((char *)p_gcscn_recout, (char *)p_gcscn_recin, sizeof(db_gcscn_def));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gcscn_recin->connect_num_ctrl_info.crt_connect_num, sizeof(p_gcscn_recin->connect_num_ctrl_info.crt_connect_num));
    cnttbl[s_table_no].tbl.cur_connection_cnt =
        _min(cnttbl[s_table_no].tbl.max_connection_cnt, _max(0, (short)(atoi(ch_buf) + s_sub_cnt)));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    sprintf(ch_buf, "%04d", cnttbl[s_table_no].tbl.cur_connection_cnt);
    memcpy(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num, ch_buf, sizeof(p_gcscn_recout->connect_num_ctrl_info.crt_connect_num));

    /* 再接続状態更新 */
    if ((memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_sts,
            DEF_RE_CONNECT_STS_ON, strlen(DEF_RE_CONNECT_STS_ON))==0) &&
        (memcmp(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id,
            myinfo.config_info.serverclass_id, sizeof(myinfo.config_info.serverclass_id))==0)) {
        if (cnttbl[s_table_no].ctl.reconnect_cnt <= 0) {
            memcpy(p_gcscn_recout->connect_num_ctrl_info.re_connect_sts,
                DEF_RE_CONNECT_STS_OFF, strlen(DEF_RE_CONNECT_STS_OFF));
            memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_start_time,
                ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_start_time));
            memset(p_gcscn_recout->connect_num_ctrl_info.re_connect_svr_cls_id,
                ' ', sizeof(p_gcscn_recin->connect_num_ctrl_info.re_connect_svr_cls_id));
        }
    }

    /* 受信コネクション数管理ファイル更新 */
    LSTN_countfile_update();

} /* LSTN_countfile_update_count */

/****************************************************************************/
/*  FUNCTION        : 49.0.0  LSTN_listenporttbl_init                       */
/*  CALLING SEQ.    : void LSTN_listenporttbl_init(short)                   */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスンポート管理テーブル初期処理                      */
/****************************************************************************/
void LSTN_listenporttbl_init(short s_table_type)
{
    listenport_tbl_def  *p_lpttbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
    }

    /* テーブル初期化処理 */
    for (s_idx = 0; s_idx < LSTN_LISTENPORT_TBL_MAX; s_idx++) {
        memset((char *)&p_lpttbl[s_idx], 0x00, sizeof(listenport_tbl_def));
        p_lpttbl[s_idx].ctl.listen_state = LSTN_ST_DISCONNECT;
        memcpy(p_lpttbl[s_idx].ctl.listen_status, DEF_CONNECT_STS_LISTEN, strlen(DEF_CONNECT_STS_LISTEN));
        p_lpttbl[s_idx].ctl.listen_fd = LSTN_FILE_CLOSED;
        p_lpttbl[s_idx].ctl.retry_tag = LSTN_TAG_NULL;
        p_lpttbl[s_idx].tbl.lsntbl_no = LSTN_TBL_NOT_ENTRY;
        p_lpttbl[s_idx].tbl.cnttbl_no = LSTN_TBL_NOT_ENTRY;
    }

} /* LSTN_listenporttbl_init */

/****************************************************************************/
/*  FUNCTION        : 50.0.0  LSTN_listenporttbl_search                     */
/*  CALLING SEQ.    : short LSTN_listenporttbl_search(short,char*,char*,char*) */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 3.station_id     (I)   ステーション識別               */
/*  ARGUMENT        : 4.listen_id      (I)   リスンポート識別               */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : リスンポート管理テーブル検索処理                      */
/****************************************************************************/
short LSTN_listenporttbl_search(
    short   s_table_type,
    char    *ch_interface_id,
    char    *ch_station_id,
    char    *ch_listen_id)
{
    listenport_tbl_def  *p_lpttbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
    }

    /* テーブル検索処理 */
    for (s_idx = 0; s_idx < myinfo.table_info[s_table_type].listenporttbl_cnt; s_idx++) {
        if ((memcmp(ch_interface_id, p_lpttbl[s_idx].interface_id, sizeof(p_lpttbl[s_idx].interface_id))==0) &&
            (memcmp(ch_station_id, p_lpttbl[s_idx].station_id, sizeof(p_lpttbl[s_idx].station_id))==0) &&
            (memcmp(ch_listen_id, p_lpttbl[s_idx].listen_id, sizeof(p_lpttbl[s_idx].listen_id))==0)) {
            /* 該当テーブル有り */
            return(s_idx);
        }
    }
    /* 該当テーブル無し */
    return(LSTN_TBL_NOT_ENTRY);

} /* LSTN_listenporttbl_search */

/****************************************************************************/
/*  FUNCTION        : 51.0.0  LSTN_listenporttbl_add                        */
/*  CALLING SEQ.    : short LSTN_listenporttbl_add(short, db_gflin_def*)    */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.gflin_rec      (I)   回線管理ファイル情報           */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : リスンポート管理テーブル追加処理                      */
/****************************************************************************/
short LSTN_listenporttbl_add(
    short           s_table_type,
    db_gflin_def    *p_gflin_rec)
{
    listenport_tbl_def  *p_lpttbl;
    short               s_idx;
    char                ch_buf[32];

    /* テーブル検索チェック */
    s_idx = LSTN_listenporttbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id,
        p_gflin_rec->pri_key.connect_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* テーブル数チェック */
    if (myinfo.table_info[s_table_type].listenporttbl_cnt >= LSTN_LISTENPORT_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LPTTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_LISTENPORT_TBL_MAX; s_idx++) {
        if (p_lpttbl[s_idx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_LISTENPORT_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LPTTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(p_lpttbl[s_idx].interface_id, p_gflin_rec->pri_key.if_id, sizeof(p_gflin_rec->pri_key.if_id));
    memcpy(p_lpttbl[s_idx].station_id, p_gflin_rec->pri_key.station_id, sizeof(p_gflin_rec->pri_key.station_id));
    memcpy(p_lpttbl[s_idx].listen_id, p_gflin_rec->pri_key.connect_id, sizeof(p_gflin_rec->pri_key.connect_id));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gflin_rec->tcpip_prc_name, sizeof(p_gflin_rec->tcpip_prc_name));
    trim(ch_buf);
    memcpy(p_lpttbl[s_idx].ctl.tcpip_name, ch_buf, strlen(ch_buf));

    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gflin_rec->ip_adress_src, sizeof(p_gflin_rec->ip_adress_src));
    trim(ch_buf);
    memcpy(p_lpttbl[s_idx].ctl.src_ip_text, ch_buf, strlen(ch_buf));
    memset(ch_buf, 0x00, sizeof(ch_buf));
    memcpy(ch_buf, p_gflin_rec->port_num_src, sizeof(p_gflin_rec->port_num_src));
    p_lpttbl[s_idx].ctl.src_port_no = (short)atoi(ch_buf);

    if (p_gflin_rec->ip_adress_dst[0] != ' ') {
        memset(ch_buf, 0x00, sizeof(ch_buf));
        memcpy(ch_buf, p_gflin_rec->ip_adress_dst, sizeof(p_gflin_rec->ip_adress_dst));
        trim(ch_buf);
        memcpy(p_lpttbl[s_idx].ctl.dst_ip_text, ch_buf, strlen(ch_buf));
        memset(ch_buf, 0x00, sizeof(ch_buf));
        memcpy(ch_buf, p_gflin_rec->port_num_dst, sizeof(p_gflin_rec->port_num_dst));
        p_lpttbl[s_idx].ctl.dst_port_no = (short)atoi(ch_buf);
    }

    /* テーブル数加算 */
    myinfo.table_info[s_table_type].listenporttbl_cnt++;
    return(s_idx);

} /* LSTN_listenporttbl_add */

/****************************************************************************/
/*  FUNCTION        : 51.0.0  LSTN_listenporttbl_move                       */
/*  CALLING SEQ.    : short LSTN_listenporttbl_move(short)                  */
/*  ARGUMENT        : 1.s_online_no    (I)   テーブル番号                   */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : リスンポート管理テーブル追加処理                      */
/****************************************************************************/
short LSTN_listenporttbl_move(short s_online_no)
{
    short               s_idx;

    /* テーブル検索チェック */
    s_idx = LSTN_listenporttbl_search(
        LSTN_TBL_RELOAD,
        lpttbl[s_online_no].interface_id,
        lpttbl[s_online_no].station_id,
        lpttbl[s_online_no].listen_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* テーブル数チェック */
    if (myinfo.table_info[LSTN_TBL_RELOAD].listenporttbl_cnt >= LSTN_LISTENPORT_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LPTTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_LISTENPORT_TBL_MAX; s_idx++) {
        if (rlpttbl[s_idx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_LISTENPORT_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LPTTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(rlpttbl[s_idx].interface_id, lpttbl[s_online_no].interface_id, sizeof(lpttbl[s_online_no].interface_id));
    memcpy(rlpttbl[s_idx].station_id, lpttbl[s_online_no].station_id, sizeof(lpttbl[s_online_no].station_id));
    memcpy(rlpttbl[s_idx].listen_id, lpttbl[s_online_no].listen_id, sizeof(lpttbl[s_online_no].listen_id));
    memcpy((char *)&rlpttbl[s_idx].ctl, (char *)&lpttbl[s_online_no].ctl, sizeof(lpttbl[s_online_no].ctl));
    
    /* テーブル数加算 */
    myinfo.table_info[LSTN_TBL_RELOAD].listenporttbl_cnt++;
    return(s_idx);

} /* LSTN_listenporttbl_move */

/****************************************************************************/
/*  FUNCTION        : 52.0.0  LSTN_listenertbl_init                         */
/*  CALLING SEQ.    : void LSTN_listenertbl_init(short)                     */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー管理テーブル初期処理                          */
/****************************************************************************/
void LSTN_listenertbl_init(short s_table_type)
{
    listener_tbl_def    *p_lsntbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lsntbl = &lsntbl[0];
    } else {
        p_lsntbl = &rlsntbl[0];
    }

    /* テーブル初期化処理 */
    for (s_idx = 0; s_idx < LSTN_LISTENER_TBL_MAX; s_idx++) {
        memset((char *)&p_lsntbl[s_idx], 0x00, sizeof(listener_tbl_def));
        p_lsntbl[s_idx].tbl.lpttbl_no = LSTN_TBL_NOT_ENTRY;
    }

} /* LSTN_listenertbl_init */

/****************************************************************************/
/*  FUNCTION        : 53.0.0  LSTN_listenertbl_search                       */
/*  CALLING SEQ.    : short LSTN_listenertbl_search(short,char*,char*,char*) */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 3.station_id     (I)   ステーション識別               */
/*  ARGUMENT        : 4.listen_id      (I)   リスンポート識別               */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : リスナー管理テーブル検索処理                          */
/****************************************************************************/
short LSTN_listenertbl_search(
    short       s_table_type,
    char        *pch_interface_id,
    char        *pch_station_id,
    char        *pch_listen_id)
{
    listener_tbl_def    *p_lsntbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lsntbl = &lsntbl[0];
    } else {
        p_lsntbl = &rlsntbl[0];
    }

    /* テーブル検索処理 */
    for (s_idx = 0; s_idx < myinfo.table_info[s_table_type].listenertbl_cnt; s_idx++) {
        if ((memcmp(pch_interface_id, p_lsntbl[s_idx].interface_id, sizeof(p_lsntbl[s_idx].interface_id))==0) &&
            (memcmp(pch_station_id, p_lsntbl[s_idx].station_id, sizeof(p_lsntbl[s_idx].station_id))==0) &&
            (memcmp(pch_listen_id, p_lsntbl[s_idx].listen_id, sizeof(p_lsntbl[s_idx].listen_id))==0)) {
            /* 該当テーブル有り */
            return(s_idx);
        }
    }
    /* 該当テーブル無し */
    return(LSTN_TBL_NOT_ENTRY);

} /* LSTN_listenertbl_search */

/****************************************************************************/
/*  FUNCTION        : 54.0.0  LSTN_listenertbl_add                          */
/*  CALLING SEQ.    : short LSTN_listenertbl_add(short,db_gflin_def*)       */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.gflin_rec      (I)   回線管理ファイル情報           */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : リスナー管理テーブル追加処理                          */
/****************************************************************************/
short LSTN_listenertbl_add(
    short           s_table_type,
    db_gflin_def    *p_gflin_rec)
{
    listenport_tbl_def  *p_lpttbl;
    listener_tbl_def    *p_lsntbl;
    short               s_idx;
    short               s_lptidx = LSTN_TBL_NOT_ENTRY;

    /* テーブル検索チェック */
    s_idx = LSTN_listenertbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id,
        p_gflin_rec->pri_key.connect_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* テーブル数チェック */
    if (myinfo.table_info[s_table_type].listenertbl_cnt >= LSTN_LISTENER_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LSNTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_lpttbl = &lpttbl[0];
        p_lsntbl = &lsntbl[0];
    } else {
        p_lpttbl = &rlpttbl[0];
        p_lsntbl = &rlsntbl[0];
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_LISTENER_TBL_MAX; s_idx++) {
        if (p_lsntbl[s_idx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_LISTENER_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "LSNTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(p_lsntbl[s_idx].interface_id, p_gflin_rec->pri_key.if_id, sizeof(p_gflin_rec->pri_key.if_id));
    memcpy(p_lsntbl[s_idx].station_id, p_gflin_rec->pri_key.station_id, sizeof(p_gflin_rec->pri_key.station_id));
    memcpy(p_lsntbl[s_idx].listen_id, p_gflin_rec->pri_key.connect_id, sizeof(p_gflin_rec->pri_key.connect_id));
    memcpy(p_lsntbl[s_idx].ctl.serverclass_id,
        (char *)&p_gflin_rec->alt1_key_info.srv_cls_id, sizeof(p_gflin_rec->alt1_key_info.srv_cls_id));

    /* リスンポート管理テーブル検索 *//* 自プロセス管理対象のみ */
    s_lptidx = LSTN_listenporttbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id,
        p_gflin_rec->pri_key.connect_id);
    if (s_lptidx > LSTN_TBL_NOT_ENTRY) {
        p_lsntbl[s_idx].tbl.lpttbl_no = s_lptidx;
        p_lpttbl[s_lptidx].tbl.lsntbl_no = s_idx;
    }

    /* テーブル数加算 */
    myinfo.table_info[s_table_type].listenertbl_cnt++;
    return(s_idx);

} /* LSTN_listenertbl_add */

/****************************************************************************/
/*  FUNCTION        : 55.0.0  LSTN_controltbl_init                          */
/*  CALLING SEQ.    : void LSTN_controltbl_init(short)                      */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御管理テーブル初期処理                  */
/****************************************************************************/
void LSTN_controltbl_init(short s_table_type)
{
    controlserver_tbl_def   *p_ctstbl;
    short                   s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_ctstbl = &ctstbl[0];
    } else {
        p_ctstbl = &rctstbl[0];
    }

    /* テーブル初期化処理 */
    for (s_idx = 0; s_idx < LSTN_CONTROLSERVER_TBL_MAX; s_idx++) {
        memset((char *)&p_ctstbl[s_idx], 0x00, sizeof(controlserver_tbl_def));
        PROCESSHANDLE_NULLIT_(p_ctstbl[s_idx].ctl.phandle);
        p_ctstbl[s_idx].ctl.control_status = LSTN_CONTROL_STS_CLOSE;
        p_ctstbl[s_idx].ctl.cpu_no = -1;
        p_ctstbl[s_idx].ctl.async_tag = LSTN_TAG_NULL;
        p_ctstbl[s_idx].ctl.reconnect_cnttbl_no = LSTN_TBL_NOT_ENTRY;
        p_ctstbl[s_idx].ctl.queue_cnt = 0;
        p_ctstbl[s_idx].ctl.queue_idx = 0;
    }

} /* LSTN_controltbl_init */

/****************************************************************************/
/*  FUNCTION        : 56.0.0  LSTN_controltbl_search                        */
/*  CALLING SEQ.    : short LSTN_controltbl_search(short,char*)             */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.serverclass_id (I)   サーバクラス論理ID             */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション制御管理テーブル検索処理                  */
/****************************************************************************/
short LSTN_controltbl_search(
    short   s_table_type,
    char    *ch_serverclass_id)
{
    controlserver_tbl_def   *p_ctstbl;
    short                   s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_ctstbl = &ctstbl[0];
    } else {
        p_ctstbl = &rctstbl[0];
    }

    /* テーブル検索処理 */
    for (s_idx = 0; s_idx < myinfo.table_info[s_table_type].controlservertbl_cnt; s_idx++) {
        if (memcmp(ch_serverclass_id, p_ctstbl[s_idx].serverclass_id, sizeof(p_ctstbl[s_idx].serverclass_id))==0) {
            /* 該当テーブル有り */
            return(s_idx);
        }
    }
    /* 該当テーブル無し */
    return(LSTN_TBL_NOT_ENTRY);

} /* LSTN_controltbl_search */

/****************************************************************************/
/*  FUNCTION        : 57.0.0  LSTN_controltbl_add                           */
/*  CALLING SEQ.    : short LSTN_controltbl_add(short,db_gflin_def*)        */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.gflin_rec      (I)   回線管理ファイル情報           */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション制御管理テーブル追加処理                  */
/****************************************************************************/
short LSTN_controltbl_add(
    short           s_table_type,
    db_gflin_def    *p_gflin_rec)
{
    controlserver_tbl_def   *p_ctstbl;
    short                   s_idx;

    /* テーブル検索チェック */
    s_idx = LSTN_controltbl_search(s_table_type, (char *)&p_gflin_rec->alt1_key_info.srv_cls_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* テーブル数チェック */
    if (myinfo.table_info[s_table_type].controlservertbl_cnt >= LSTN_CONTROLSERVER_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CTSTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_ctstbl = &ctstbl[0];
    } else {
        p_ctstbl = &rctstbl[0];
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_CONTROLSERVER_TBL_MAX; s_idx++) {
        if (p_ctstbl[s_idx].serverclass_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_CONTROLSERVER_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CTSTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(p_ctstbl[s_idx].serverclass_id,
        (char *)&p_gflin_rec->alt1_key_info.srv_cls_id, sizeof(p_gflin_rec->alt1_key_info.srv_cls_id));

    /* テーブル数加算 */
    myinfo.table_info[s_table_type].controlservertbl_cnt++;
    return(s_idx);

} /* LSTN_controltbl_add */

/****************************************************************************/
/*  FUNCTION        : 57.0.0  LSTN_controltbl_move                          */
/*  CALLING SEQ.    : short LSTN_controltbl_move(short)                     */
/*  ARGUMENT        : 1.s_online_no    (I)   テーブル番号                   */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション制御管理テーブル移動処理                  */
/****************************************************************************/
short LSTN_controltbl_move(
    short           s_online_no)
{
    short                   s_idx;

    /* テーブル検索チェック */
    s_idx = LSTN_controltbl_search(LSTN_TBL_RELOAD, ctstbl[s_online_no].serverclass_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* テーブル数チェック */
    if (myinfo.table_info[LSTN_TBL_RELOAD].controlservertbl_cnt >= LSTN_CONTROLSERVER_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CTSTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_CONTROLSERVER_TBL_MAX; s_idx++) {
        if (rctstbl[s_idx].serverclass_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_CONTROLSERVER_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CTSTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(rctstbl[s_idx].serverclass_id,
        ctstbl[s_online_no].serverclass_id, sizeof(ctstbl[s_online_no].serverclass_id));
    memcpy((char *)&rctstbl[s_idx].ctl,
        (char *)&ctstbl[s_online_no].ctl, sizeof(ctstbl[s_online_no].ctl));

    /* テーブル数加算 */
    myinfo.table_info[LSTN_TBL_RELOAD].controlservertbl_cnt++;
    return(s_idx);

} /* LSTN_controltbl_move */

/****************************************************************************/
/*  FUNCTION        : 58.0.0  LSTN_connectiontbl_init                       */
/*  CALLING SEQ.    : void LSTN_connectiontbl_init(short)                   */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション管理テーブル初期処理                      */
/****************************************************************************/
void LSTN_connectiontbl_init(short s_table_type)
{
    connection_tbl_def  *p_contbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_contbl = &contbl[0];
    } else {
        p_contbl = &rcontbl[0];
    }

    /* テーブル初期化処理 */
    for (s_idx = 0; s_idx < LSTN_CONNECTION_TBL_MAX; s_idx++) {
        memset((char *)&p_contbl[s_idx], 0x00, sizeof(connection_tbl_def));
        memcpy(p_contbl[s_idx].ctl.connection_sts, DEF_CONNECT_STS_DISCONN, strlen(DEF_CONNECT_STS_DISCONN));
        p_contbl[s_idx].tbl.ctstbl_no = LSTN_TBL_NOT_ENTRY;
        p_contbl[s_idx].tbl.lsntbl_no = LSTN_TBL_NOT_ENTRY;
        p_contbl[s_idx].tbl.lpttbl_no = LSTN_TBL_NOT_ENTRY;
        p_contbl[s_idx].tbl.cnttbl_no = LSTN_TBL_NOT_ENTRY;
    }

} /* LSTN_connectiontbl_init */

/****************************************************************************/
/*  FUNCTION        : 59.0.0  LSTN_connectiontbl_search                     */
/*  CALLING SEQ.    : short LSTN_connectiontbl_search(short,char*,char*,char*) */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 3.station_id     (I)   ステーション識別               */
/*  ARGUMENT        : 4.connection_id  (I)   コネクション識別               */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション管理テーブル検索処理                      */
/****************************************************************************/
short LSTN_connectiontbl_search(
    short   s_table_type,
    char    *pch_interface_id,
    char    *pch_station_id,
    char    *pch_connection_id)
{
    connection_tbl_def  *p_contbl;
    short               s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_contbl = &contbl[0];
    } else {
        p_contbl = &rcontbl[0];
    }

    /* テーブル検索処理 */
    for (s_idx = 0; s_idx < myinfo.table_info[s_table_type].connectiontbl_cnt; s_idx++) {
        if ((memcmp(pch_interface_id, p_contbl[s_idx].interface_id, sizeof(p_contbl[s_idx].interface_id))==0) &&
            (memcmp(pch_station_id, p_contbl[s_idx].station_id, sizeof(p_contbl[s_idx].station_id))==0) &&
            (memcmp(pch_connection_id, p_contbl[s_idx].connection_id, sizeof(p_contbl[s_idx].connection_id))==0)) {
            /* 該当テーブル有り */
            return(s_idx);
        }
    }
    /* 該当テーブル無し */
    return(LSTN_TBL_NOT_ENTRY);

} /* LSTN_connectiontbl_search */

/****************************************************************************/
/*  FUNCTION        : 60.0.0  LSTN_connectiontbl_add                        */
/*  CALLING SEQ.    : short LSTN_connectiontbl_add(short, db_gflin_def*)    */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.gflin_rec      (I)   回線管理ファイル情報           */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション管理テーブル追加処理                      */
/****************************************************************************/
short LSTN_connectiontbl_add(
    short           s_table_type,
    db_gflin_def    *p_gflin_rec)
{
    controlserver_tbl_def   *p_ctstbl;
    connection_tbl_def      *p_contbl;
//  listener_tbl_def        *p_lsntbl;
    count_tbl_def           *p_cnttbl;
    short   s_conidx = LSTN_TBL_NOT_ENTRY;
    short   s_ctsidx = LSTN_TBL_NOT_ENTRY;
    short   s_lsnidx = LSTN_TBL_NOT_ENTRY;
    short   s_lptidx = LSTN_TBL_NOT_ENTRY;
    short   s_cntidx = LSTN_TBL_NOT_ENTRY;

    /* コネクション管理テーブル検索チェック */
    s_conidx = LSTN_connectiontbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id,
        p_gflin_rec->pri_key.connect_id);
    if (s_conidx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_conidx);
    }

    /* コネクション管理テーブル数チェック */
    if (myinfo.table_info[s_table_type].connectiontbl_cnt >= LSTN_CONNECTION_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CONTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_ctstbl = &ctstbl[0];
        p_contbl = &contbl[0];
//      p_lsntbl = &lsntbl[0];
        p_cnttbl = &cnttbl[0];
    } else {
        p_ctstbl = &rctstbl[0];
        p_contbl = &rcontbl[0];
//      p_lsntbl = &rlsntbl[0];
        p_cnttbl = &rcnttbl[0];
    }

    /* 空きテーブル取得 */
    for (s_conidx = 0; s_conidx < LSTN_CONNECTION_TBL_MAX; s_conidx++) {
        if (p_contbl[s_conidx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_conidx >= LSTN_CONNECTION_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CONTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* コネクション制御管理テーブル検索 */
    s_ctsidx = LSTN_controltbl_search(s_table_type, (char *)&p_gflin_rec->alt1_key_info.srv_cls_id);
    if (s_ctsidx == LSTN_TBL_NOT_ENTRY) {
        /* コネクション制御管理テーブル登録 */
        s_ctsidx = LSTN_controltbl_add(s_table_type, p_gflin_rec);
        if (s_ctsidx == LSTN_TBL_NOT_ENTRY) {
            return(LSTN_TBL_NOT_ENTRY);
        }
    }

    /* コネクション制御管理テーブル更新 */
    p_ctstbl[s_ctsidx].tbl.connection_max++;

//  /* リスナー管理テーブル検索 */
//  s_lsnidx = LSTN_listenertbl_search(
//      s_table_type,
//      p_gflin_rec->pri_key.if_id,
//      p_gflin_rec->pri_key.station_id,
//      p_gflin_rec->alt2_key_info.lst_connect_id);
//  if (s_lsnidx != LSTN_TBL_NOT_ENTRY) {
//      p_lsntbl[s_lsnidx].tbl.connection_max++;
//  }

    /* リスンポート管理テーブル検索 *//* 自プロセス管理対象のみ */
    s_lptidx = LSTN_listenporttbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id,
        p_gflin_rec->alt2_key_info.lst_connect_id);

    /* コネクション数管理テーブル検索 *//* 自プロセス管理対象のみ */
    s_cntidx = LSTN_counttbl_search(
        s_table_type,
        p_gflin_rec->pri_key.if_id,
        p_gflin_rec->pri_key.station_id);
    if (s_cntidx != LSTN_TBL_NOT_ENTRY) {
        p_cnttbl[s_cntidx].tbl.max_connection_cnt++;
    }

    /* コネクション管理テーブル情報設定 */
    memcpy(p_contbl[s_conidx].interface_id, p_gflin_rec->pri_key.if_id, sizeof(p_gflin_rec->pri_key.if_id));
    memcpy(p_contbl[s_conidx].station_id, p_gflin_rec->pri_key.station_id, sizeof(p_gflin_rec->pri_key.station_id));
    memcpy(p_contbl[s_conidx].connection_id, p_gflin_rec->pri_key.connect_id, sizeof(p_gflin_rec->pri_key.connect_id));
    p_contbl[s_conidx].tbl.ctstbl_no = s_ctsidx;
    p_contbl[s_conidx].tbl.lsntbl_no = s_lsnidx;
    p_contbl[s_conidx].tbl.lpttbl_no = s_lptidx;
    p_contbl[s_conidx].tbl.cnttbl_no = s_cntidx;

    /* コネクション管理テーブル数加算 */
    myinfo.table_info[s_table_type].connectiontbl_cnt++;
    return(s_conidx);

} /* LSTN_connectiontbl_add */

/****************************************************************************/
/*  FUNCTION        : 60.0.0  LSTN_connectiontbl_move                       */
/*  CALLING SEQ.    : short LSTN_connectiontbl_move(short)                  */
/*  ARGUMENT        : 1.s_online_no    (I)   テーブル番号                   */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション管理テーブル移動処理                      */
/****************************************************************************/
short LSTN_connectiontbl_move(
    short           s_online_no)
{
    short   s_conidx = LSTN_TBL_NOT_ENTRY;
    short   s_ctsidx = LSTN_TBL_NOT_ENTRY;
    short   s_lsnidx = LSTN_TBL_NOT_ENTRY;
    short   s_lptidx = LSTN_TBL_NOT_ENTRY;
    short   s_cntidx = LSTN_TBL_NOT_ENTRY;

    /* コネクション管理テーブル検索チェック */
    s_conidx = LSTN_connectiontbl_search(
        LSTN_TBL_RELOAD,
        contbl[s_online_no].interface_id,
        contbl[s_online_no].station_id,
        contbl[s_online_no].connection_id);
    if (s_conidx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_conidx);
    }

    /* コネクション管理テーブル数チェック */
    if (myinfo.table_info[LSTN_TBL_RELOAD].connectiontbl_cnt >= LSTN_CONNECTION_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CONTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* 空きテーブル取得 */
    for (s_conidx = 0; s_conidx < LSTN_CONNECTION_TBL_MAX; s_conidx++) {
        if (rcontbl[s_conidx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_conidx >= LSTN_CONNECTION_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_LIN_MG, "", "CONTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* コネクション制御管理テーブル検索 */
    s_ctsidx = LSTN_controltbl_search(
        LSTN_TBL_RELOAD,
        ctstbl[contbl[s_online_no].tbl.ctstbl_no].serverclass_id);
    if (s_ctsidx == LSTN_TBL_NOT_ENTRY) {
        /* コネクション制御管理テーブル登録 */
        s_ctsidx = LSTN_controltbl_move(contbl[s_online_no].tbl.ctstbl_no);
        if (s_ctsidx == LSTN_TBL_NOT_ENTRY) {
            return(LSTN_TBL_NOT_ENTRY);
        }
    }

    /* コネクション制御管理テーブル更新 */
    rctstbl[s_ctsidx].tbl.connection_max++;

//  /* リスナー管理テーブル検索 */
//  if (contbl[s_online_no].tbl.lsntbl_no != LSTN_TBL_NOT_ENTRY) {
//      s_lsnidx = LSTN_listenertbl_search(
//          LSTN_TBL_RELOAD,
//          lsntbl[contbl[s_online_no].tbl.lsntbl_no].interface_id,
//          lsntbl[contbl[s_online_no].tbl.lsntbl_no].station_id,
//          lsntbl[contbl[s_online_no].tbl.lsntbl_no].listen_id);
//      if (s_lsnidx != LSTN_TBL_NOT_ENTRY) {
//          rlsntbl[s_lsnidx].tbl.connection_max++;
//      }
//  }

    /* リスンポート管理テーブル検索 *//* 自プロセス管理対象のみ */
    if (contbl[s_online_no].tbl.lpttbl_no != LSTN_TBL_NOT_ENTRY) {
        s_lptidx = LSTN_listenporttbl_search(
            LSTN_TBL_RELOAD,
            lpttbl[contbl[s_online_no].tbl.lpttbl_no].interface_id,
            lpttbl[contbl[s_online_no].tbl.lpttbl_no].station_id,
            lpttbl[contbl[s_online_no].tbl.lpttbl_no].listen_id);
    }

    /* コネクション数管理テーブル検索 *//* 自プロセス管理対象のみ */
    if (contbl[s_online_no].tbl.cnttbl_no != LSTN_TBL_NOT_ENTRY) {
        s_cntidx = LSTN_counttbl_search(
            LSTN_TBL_RELOAD,
            cnttbl[contbl[s_online_no].tbl.cnttbl_no].interface_id,
            cnttbl[contbl[s_online_no].tbl.cnttbl_no].station_id);
        if (s_cntidx != LSTN_TBL_NOT_ENTRY) {
            rcnttbl[s_cntidx].tbl.max_connection_cnt++;
        }
    }

    /* コネクション管理テーブル情報設定 */
    memcpy(rcontbl[s_conidx].interface_id, contbl[s_online_no].interface_id, sizeof(contbl[s_online_no].interface_id));
    memcpy(rcontbl[s_conidx].station_id, contbl[s_online_no].station_id, sizeof(contbl[s_online_no].station_id));
    memcpy(rcontbl[s_conidx].connection_id, contbl[s_online_no].connection_id, sizeof(contbl[s_online_no].connection_id));
    rcontbl[s_conidx].tbl.ctstbl_no = s_ctsidx;
    rcontbl[s_conidx].tbl.lsntbl_no = s_lsnidx;
    rcontbl[s_conidx].tbl.lpttbl_no = s_lptidx;
    rcontbl[s_conidx].tbl.cnttbl_no = s_cntidx;

    /* コネクション管理テーブル数加算 */
    myinfo.table_info[LSTN_TBL_RELOAD].connectiontbl_cnt++;
    return(s_conidx);

} /* LSTN_connectiontbl_move */

/****************************************************************************/
/*  FUNCTION        : 61.0.0  LSTN_connectiontbl_status                     */
/*  CALLING SEQ.    : void LSTN_connectiontbl_status(short, char*, char*)   */
/*  ARGUMENT        : 1.contbl_no      (I)   コネクション管理テーブル番号   */
/*  ARGUMENT        : 2.connect_sts    (I)   コネクション状態               */
/*  ARGUMENT        : 3.connect_time   (I)   コネクション状態更新時間       */
/*  RETURN CODE     : 更新コネクション数(0:無し ≠0:有り)                   */
/*  DESCRIPTION     : コネクション管理テーブルコネクション状態更新処理      */
/****************************************************************************/
short LSTN_connectiontbl_status(short s_contbl_no, char *connect_sts, char *connect_time)
{
    short   s_ctsno         = LSTN_TBL_NOT_ENTRY;
    short   s_update_cnt    = 0;

    s_ctsno = contbl[s_contbl_no].tbl.ctstbl_no;

    /* 接続->非接続 */
    if (memcmp(contbl[s_contbl_no].ctl.connection_sts, DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))==0) {
        if (memcmp(connect_sts, DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))!=0) {
            ctstbl[s_ctsno].tbl.connection_cnt = _max(0, (ctstbl[s_ctsno].tbl.connection_cnt-1));
            if (contbl[s_contbl_no].tbl.lpttbl_no != LSTN_TBL_NOT_ENTRY) {
                s_update_cnt--;
            }
        }
    /* 非接続->接続 */
    } else {
        if (memcmp(connect_sts, DEF_CONNECT_STS_CONNECT, strlen(DEF_CONNECT_STS_CONNECT))==0) {
            ctstbl[s_ctsno].tbl.connection_cnt = _min(ctstbl[s_ctsno].tbl.connection_max, (ctstbl[s_ctsno].tbl.connection_cnt+1));
            if (contbl[s_contbl_no].tbl.lpttbl_no != LSTN_TBL_NOT_ENTRY) {
                s_update_cnt++;
            }
        }
    }
    memcpy(contbl[s_contbl_no].ctl.connection_sts, connect_sts, sizeof(contbl[s_contbl_no].ctl.connection_sts));
    memcpy(contbl[s_contbl_no].ctl.connect_time, connect_time, sizeof(contbl[s_contbl_no].ctl.disconnect_time));

    return s_update_cnt;

} /* LSTN_connectiontbl_status */

/****************************************************************************/
/*  FUNCTION        : 62.0.0  LSTN_counttbl_init                            */
/*  CALLING SEQ.    : void LSTN_counttbl_init(short)                        */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション数管理テーブル初期処理                    */
/****************************************************************************/
void LSTN_counttbl_init(short s_table_type)
{
    count_tbl_def   *p_cnttbl;
    short           s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_cnttbl = &cnttbl[0];
    } else {
        p_cnttbl = &rcnttbl[0];
    }

    /* テーブル初期化処理 */
    for (s_idx = 0; s_idx < LSTN_COUNT_TBL_MAX; s_idx++) {
        memset((char *)&p_cnttbl[s_idx], 0x00, sizeof(count_tbl_def));
        p_cnttbl[s_idx].ctl.reconnect_lpttbl_no = LSTN_TBL_NOT_ENTRY;
    }

} /* LSTN_counttbl_init */

/****************************************************************************/
/*  FUNCTION        : 63.0.0  LSTN_counttbl_search                          */
/*  CALLING SEQ.    : short LSTN_counttbl_search(short,char*,char*)         */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 3.station_id     (I)   ステーション識別               */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション数管理テーブル検索処理                    */
/****************************************************************************/
short LSTN_counttbl_search(
    short   s_table_type,
    char    *pch_interface_id,
    char    *pch_station_id)
{
    count_tbl_def   *p_cnttbl;
    short           s_idx;

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_cnttbl = &cnttbl[0];
    } else {
        p_cnttbl = &rcnttbl[0];
    }

    /* テーブル検索処理 */
    for (s_idx = 0; s_idx < myinfo.table_info[s_table_type].counttbl_cnt; s_idx++) {
        if (myinfo.config_info.connection_counter_layer == DEF_CONNECT_NUM_MNG_LYR_IF) {
            if (memcmp(pch_interface_id, p_cnttbl[s_idx].interface_id, sizeof(p_cnttbl[s_idx].interface_id))==0) {
                /* 該当テーブル有り */
                return(s_idx);
            }
        }
        if (myinfo.config_info.connection_counter_layer == DEF_CONNECT_NUM_MNG_LYR_ST) {
            if ((memcmp(pch_interface_id, p_cnttbl[s_idx].interface_id, sizeof(p_cnttbl[s_idx].interface_id))==0) &&
                (memcmp(pch_station_id, p_cnttbl[s_idx].station_id, sizeof(p_cnttbl[s_idx].station_id))==0)) {
                /* 該当テーブル有り */
                return(s_idx);
            }
        }
    }
    /* 該当テーブル無し */
    return(LSTN_TBL_NOT_ENTRY);

} /* LSTN_counttbl_search */

/****************************************************************************/
/*  FUNCTION        : 64.0.0  LSTN_counttbl_add                             */
/*  CALLING SEQ.    : short LSTN_counttbl_add(short,db_gcscn_def*)          */
/*  ARGUMENT        : 1.table_type     (I)   テーブルタイプ                 */
/*  ARGUMENT        : 2.gflin_rec      (I)   コネクション数管理ファイル情報 */
/*  RETURN CODE     : 0>:テーブル番号 -1:エラー                             */
/*  DESCRIPTION     : コネクション数管理テーブル追加処理                    */
/****************************************************************************/
short LSTN_counttbl_add(
    short           s_table_type,
    db_gcscn_def    *p_gcscn_rec)
{
    count_tbl_def   *p_cnttbl;
    short           s_idx;
//  char            ch_buf[64];

    /* コネクション数管理テーブル検索チェック */
    s_idx = LSTN_counttbl_search(
        s_table_type,
        p_gcscn_rec->pri_key.if_id,
        p_gcscn_rec->pri_key.station_id);
    if (s_idx > LSTN_TBL_NOT_ENTRY) {
        /* 登録済み */
        return(s_idx);
    }

    /* コネクション数管理テーブル数チェック */
    if (myinfo.table_info[s_table_type].counttbl_cnt >= LSTN_COUNT_TBL_MAX) {
        /* テーブルフル */
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_RCV_CON_NUM, "", "CNTTBL MAX");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル範囲特定 */
    if (s_table_type == LSTN_TBL_ONLINE) {
        p_cnttbl = &cnttbl[0];
    } else {
        p_cnttbl = &rcnttbl[0];
    }

    /* 空きテーブル取得 */
    for (s_idx = 0; s_idx < LSTN_COUNT_TBL_MAX; s_idx++) {
        if (p_cnttbl[s_idx].interface_id[0] == NULL) {
            break;
        }
    }
    if (s_idx >= LSTN_COUNT_TBL_MAX) {
        LSTN_message_output(DEF_EVT_CONFIG_ERR, 'E', DEF_NERR_NOMAL, "@f@K@X",
            DEF_FL_RCV_CON_NUM, "", "CNTTBL OVER");
        return(LSTN_TBL_NOT_ENTRY);
    }

    /* テーブル情報設定 */
    memcpy(p_cnttbl[s_idx].interface_id, p_gcscn_rec->pri_key.if_id, sizeof(p_gcscn_rec->pri_key.if_id));
    memcpy(p_cnttbl[s_idx].station_id, p_gcscn_rec->pri_key.station_id, sizeof(p_gcscn_rec->pri_key.station_id));

    memcpy(p_cnttbl[s_idx].ctl.reconnect_status,
        p_gcscn_rec->connect_num_ctrl_info.re_connect_sts,
        sizeof(p_gcscn_rec->connect_num_ctrl_info.re_connect_sts));
    memcpy(p_cnttbl[s_idx].ctl.reconnect_starttime,
        p_gcscn_rec->connect_num_ctrl_info.re_connect_start_time,
        sizeof(p_gcscn_rec->connect_num_ctrl_info.re_connect_start_time));
    memcpy(p_cnttbl[s_idx].ctl.reconnect_serverclass,
        p_gcscn_rec->connect_num_ctrl_info.re_connect_svr_cls_id,
        sizeof(p_gcscn_rec->connect_num_ctrl_info.re_connect_svr_cls_id));

    /* テーブル数加算 */
    myinfo.table_info[s_table_type].counttbl_cnt++;
    return(s_idx);

} /* LSTN_counttbl_add */

/****************************************************************************/
/*  FUNCTION        : 65.0.0  LSTN_reply                                    */
/*  CALLING SEQ.    : void LSTN_reply(char*,short,short,short,short)        */
/*  ARGUMENT        : 1.rep_buf        (I)   リプライバッファ               */
/*  ARGUMENT        : 2.rep_len        (I)   リプライレングス               */
/*  ARGUMENT        : 3.rep_tag        (I)   リプライタグ                   */
/*  ARGUMENT        : 4.rep_err        (I)   リプライエラーコード           */
/*  ARGUMENT        : 5.trace_flg      (I)   トレース出力要否               */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void LSTN_reply(
    char    *pch_rep_buf,
    short   s_rep_len,
    short   s_rep_tag,
    short   s_rep_err,
    short   s_trace_flg)
{
    COM_SDT_arg_2_def   sdt_chdate;
    COM_SDT_arg_3_def   sdt_sdate;
    long long           ll_tmstamp;
    char                ch_buf[10];

    if (s_trace_flg == LSTN_FLG_ON) {
        COM_SDT(2, &sdt_chdate, &sdt_sdate, &ll_tmstamp);
    }

    /* リプライ */
    i_CC = REPLYX(pch_rep_buf, s_rep_len, /* count-written */, s_rep_tag, s_rep_err);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(s_rcv_fd, &s_rcv_err);
        LSTN_message_output(DEF_EVT_PROCEDURE_ERR, 'E', DEF_NERR_FILE_IO_ERR, "@X@E", "REPLYX", s_rcv_err);
        LSTN_internal_error(LSTN_MSG_INTERNAL_ERR, LSTN_ABNORMAL_TERMINATION);
    }

    /* トレース */
    if (s_trace_flg == LSTN_FLG_ON) {
        memset(ch_trc_buf, 0x20, lk_zac2001i_arg_1_def_Size);
        p_trc_ipc->func_flg = '1';
        memcpy(p_trc_ipc->trace_info.prog_id, DEF_GFPCVX00, strlen(DEF_GFPCVX00));
        memcpy(p_trc_ipc->trace_info.file_id, LSTN_RECEIVE_FILENAME, strlen(LSTN_RECEIVE_FILENAME));
        memcpy(p_trc_ipc->trace_info.file_name, LSTN_RECEIVE_FILENAME, strlen(LSTN_RECEIVE_FILENAME));
        memcpy(p_trc_ipc->trace_info.file_io_type, "WRITE", 5);
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf, "%04d", s_rep_err);
        memcpy(p_trc_ipc->trace_info.guardian_errcode, ch_buf, 4);
        memcpy(p_trc_ipc->trace_info.shori_start_time,
            (char *)sdt_chdate.hh, sizeof(p_trc_ipc->trace_info.shori_start_time));
        COM_SDT(2, &sdt_chdate, &sdt_sdate, &ll_tmstamp);
        memcpy(p_trc_ipc->trace_info.shori_end_time,
            (char *)sdt_chdate.hh, sizeof(p_trc_ipc->trace_info.shori_end_time));
        memset(ch_buf, 0x00, sizeof(ch_buf));
        sprintf(ch_buf,"%05d", s_rep_len);
        memcpy(p_trc_ipc->data_info.rec_len, ch_buf, 5);
        memcpy(p_trc_ipc->data_info.rec_area, pch_rep_buf, s_rep_len);
        TRACEOUT((char *)p_trc_ipc);
    }

} /* LSTN_internal_error */

/****************************************************************************/
/*  FUNCTION        : 66.0.0  LSTN_message_output                           */
/*  CALLING SEQ.    : void LSTN_message_output(short,char,char*,char*, ...) */
/*  ARGUMENT        : 1.event_code     (I)   メッセージ番号                 */
/*  ARGUMENT        : 2.kubun          (I)   メッセージ通知区分             */
/*  ARGUMENT        : 3.inter_code     (I)   内部エラーコード               */
/*  ARGUMENT        : 4.format         (I)   編集書式                       */
/*  ARGUMENT        : 5....            (I)   可変パラメータ                 */
/*  ARGUMENT        : 5.trace_flg      (I)   トレース出力要否               */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ出力処理                                    */
/****************************************************************************/
void LSTN_message_output(
    short   s_event_code,
    char    ch_kubun,
    char    *pch_inter_code,
    char    *pch_format, ...)
{
    char            ch_text[99];
    char            ch_fmt[99];
    char            *pch_ep, *pch_sp;
    unsigned short  s_var;
    short           s_vcnt = 0, s_cnt, s_idx, s_param_cnt = 0, loop_flg = 1;
    va_list         va_ap;

    /* イベントコード*/
    memset(ch_text, 0x00, sizeof(ch_text));
    sprintf(ch_text, "%05d", s_event_code);
    memcpy(cg010in.emsinf.msgid, ch_text, strlen(ch_text));

    /* メッセージ通知区分 */
    cg010in.emsinf.emsgkinf.msgttkb = ch_kubun;

    /* 内部エラーコード */
    memcpy(cg010in.emsinf.emsgkinf.inter_errcd,
        pch_inter_code, sizeof(cg010in.emsinf.emsgkinf.inter_errcd));

    /* 任意メッセージ */
    memset((char *)&cg010in.emsinf.emsnninf, 0x20, sizeof(cg010in.emsinf.emsnninf));

//  /* ①GFP通信制御メッセージ番号 */
//  memset(ch_text, 0x00, sizeof(ch_text));
//  sprintf(ch_text, "%05d", s_event_code);
//  memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
//      ch_text, strlen(ch_text));
//  s_param_cnt++;

    /* ②サーバークラス論理ID */
    if (myinfo.config_info.serverclass_id[0] != 0x00) {
        memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
            myinfo.config_info.serverclass_id, 12);
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
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl, ch_text, 48);
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case 'K':       /* キー */
            pch_ep = (char *)va_arg(va_ap,char *);
            memset(ch_text, 0x00, sizeof(ch_text));
            memcpy(ch_text, pch_ep, strlen(pch_ep));
            memcpy(cg010in.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl,
                ch_text, _min(strlen(ch_text), 40));
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
    if (cg010in.subrcd == DEF_CG010_PRMERR) {
        LSTN_internal_error(LSTN_MSG_NONE, LSTN_ABNORMAL_TERMINATION);
    }

} /* LSTN_message_output */

/****************************************************************************/
/*  FUNCTION        : 67.0.0  LSTN_internal_error                           */
/*  CALLING SEQ.    : void LSTN_internal_error(short,short)                 */
/*  ARGUMENT        : 1.stop_msg       (I)   メッセージ出力要否             */
/*  ARGUMENT        : 2.stop_flag      (I)   停止フラグ(0:正常 1:異常)      */
/*  ARGUMENT        : 終了フラグ                                            */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 内部エラー処理                                        */
/****************************************************************************/
void LSTN_internal_error(
    short   s_stop_msg,
    short   s_stop_flag)
{
    /* メッセージ出力 */
    if (s_stop_msg != LSTN_MSG_NONE) {
        LSTN_message_output(DEF_EVT_PROC_ABNORMAL_END, 'E', DEF_NERR_NOMAL, "@R", myinfo.process_info.my_pname);
    }

    /* プロセス停止 */
    PROCESS_STOP_(,, s_stop_flag);

} /* LSTN_internal_error */

/****************************************************************************/
/*  FUNCTION        : 99.0.0  trim                                          */
/*  CALLING SEQ.    : char* trim(const char*)                               */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : char*                                                 */
/*  DESCRIPTION     : トリム処理                                            */
/****************************************************************************/
char* trim (const char* string) {
    return ltrim(rtrim(string));
}

char* ltrim (const char* string) {
    char* tmp = (char*) string;
    for (; *tmp == 0x20 && *tmp != 0x00; tmp++);
    return tmp;
}

char* rtrim (const char* string) {
    char* tmp = (char*) string;
    int s = (int)(strlen(tmp) - 1);
    for (; s > 0 && tmp[s] == 0x20; s--);
    tmp[s + 1] = 0x00;
    return tmp;
}
/****************************************************************************/

