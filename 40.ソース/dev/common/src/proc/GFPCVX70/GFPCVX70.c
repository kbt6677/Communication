/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX70                                    */
/*        FUNCTION          ････ 制御電文振分                                */
/*                               制御電文振分機能/制御電文マッチング機能     */
/*                                                                           */
/*        AUTHER            ････ HAS M.Matsumoto                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-02-03                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/02/03 (J0680)新規作成                               */
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
#include <string.h> nolist
#include <stdlib.h> nolist
#include <stdarg.h> nolist
#include <stdbool.h> nolist
#include <tal.h> nolist
#include <ctype.h> nolist
#include <cextdecs.h> nolist
#include <zsysc> nolist

/* USER HEADER     */
#include "file.h(db_gfphi)"             // 物理名情報ファイル
#include "file.h(db_gfnwi)"             // N/W情報ファイル
#include "file.h(db_glelg)"             // エラー出力ログ
#include "file.h(db_gfmtl)"             // 制御電文管理ファイル
#include "file.h(db_gfqsw)"             // 受信電文振分先設定ファイル
#include "file.h(queue_data)"           // 外部NW用キューデータ
#include "file.h(db_gqnwq)"             // 外部NW用キューファイル
#include "file.h(db_glnlg)"             // ← ipc.hで必要のため
#include "ipc.h"
#include "common.h"
#include "ems.h"
#include "errcd.h"
#include "GFPCGX20.h"                   // ビットマップ展開・組立
#include "GFPCGX40.h"                   // PATHSEND処理
#include "GFPCGX50.h"                   // システム日時取得
#include "GFPCGX80.h"                   // エラー出力ログ編集出力
#include "GFPCGXA0.h"                   // トランザクション管理
#include "GFPCGXB0.h"                   // IOモジュール
#include "GFPCGXC0.h"                   // オープナープロセス管理
#include "GFPCGXE0.h"                   // タイムアウト時刻算出
#include "NWM_MKM.h"                    // 要求応答マッチングキー生成
#include "NWM_MSJ.h"                    // 電文種別判定
#include "NWM_HDL.h"                    // 電文ヘッダ電文長編集

#include "DDLCTIC.h(timer_issue_rq)"    // タイマー制御
#include "DDLCTIC.h(timer_issue_resp)"  // タイマー制御
#include "DDLCTIC.h(timer_cancel_rq)"   // タイマー制御
#include "DDLCTIC.h(timer_cancel_resp)" // タイマー制御
#include "DDLCIFC.h(receivequeue_info)" // 業向系共通IF

#include "GFPCVXZ0.h"                   // 共通メイン
#include "GFPCVXZ2.h"                   // 共通メイン(グローバル)

#include "GFPCVX70.h"                   // 制御電文振分
#include "vproc.h"

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/
/* ------------------------------------------------------------------------ */
/*   グローバル変数定義                                                     */
/* ------------------------------------------------------------------------ */
t_kbt_file_data         g_kbt_file_data;                    /* 個別ファイル情報                 */
server_tbl_def          g_svrtbl[DEF_CMSD_SV_MAX];          /* サーバ管理テーブル               */
ctl_info_def            g_ctl_info;                         /* 制御電文管理情報                 */
format_info_def         g_format_info;                      /* フォーマット変換情報             */

/* ビットマップ展開組立 */
ISO8583context_t        g_iso8583_context;                  /* ISO8583コンテキスト              */
ISO8583object_t         g_iso8583_object;                   /* ISO8583オブジェクト              */
ffd_def                 g_ffd[DEF_CMSD_FFD_MAX];            /* 固定フォーマット定義             */
ffd_index_def           g_ffd_idx[DEF_CMSD_FFDIDX_MAX];     /* 固定フォーマット定義インデックス */
char                    g_ffmt_buf[DEF_CMSD_FFMT_SIZE_MAX]; /* 固定フォーマットバッファ         */
ffd_header_def          *g_ffmt_head;                       /* 固定フォーマットヘッダ           */
char                    g_iso8583_buf[DEF_CMD_DATA_SIZE];   /* ISO8583フォーマットバッファ      */

/* ファイルレコード */
db_gfnwi_def            g_gfnwi_ctl;                        /* インタフェースNW情報             */
db_gfmtl_def            g_gfmtl_ctl;                        /* 制御電文管理レコード             */
db_glelg_def            g_glelg_ctl;                        /* エラー出力                       */

/* 受信キュー */
db_gqnwq_def            *g_recvque;                         /* 受信キュー                       */
db_gqnwq_def            *g_sendque;                         /* 送信キュー                       */
receivequeue_info_def   *g_recvque_detail;                  /* 受信キュー(業務インタフェース)   */

/* IPC 制御電文IF */
common_header_def       *g_ipcreq_head;                     /* 要求IPCヘッダー                  */
common_header_def       *g_ipcres_head;                     /* 応答IPCヘッダー                  */
c301_def                *g_c301;                            /* キュー登録要求                   */
r301_def                *g_r301;                            /* キュー登録応答                   */
c302_def                *g_c302;                            /* キュー取出し通知要求             */
r302_def                *g_r302;                            /* キュー取出し通知応答             */
c502_def                *g_c502;                            /* コマンド処理要求                 */
r502_def                *g_r502;                            /* コマンド処理応答                 */

/* IPC 制御電文サーバ */
char                    g_serv_buf[DEF_BUF_LENGTH];         /* 制御電文サーバ用バッファ         */
cr401_def               *g_c401;                            /* NW受信電文要求                   */
cr401_def               *g_r401;                            /* NW受信電文応答                   */
cr402_def               *g_c402;                            /* 制御電文作成要求                 */
cr402_def               *g_r402;                            /* 制御電文作成応答                 */

/************************************************************************************************/
/*  制御電文共通メイン関数                                                                      */
/************************************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_kbt_set_prgid                             */
/*  CALLING SEQ.    : short CMIN_kbt_set_prgid(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 プログラムIDセット                               */
/****************************************************************************/
void  CMIN_kbt_set_prgid()
{
    memcpy(g_myinfo.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));

} /* end of CMIN_kbt_set_prgid */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_init                                  */
/*  CALLING SEQ.    : short CMIN_kbt_init(void)                             */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 初期処理                                         */
/****************************************************************************/
short CMIN_kbt_init(void)
{
    short           s_result;
    short           s_idx;

    /* データポインタ */
    g_ipcreq_head = (common_header_def *)g_recv_buf;
    g_ipcres_head = (common_header_def *)g_resp_buf;
    g_c301 = (c301_def *)g_send_buf;
    g_r301 = (r301_def *)g_send_buf;
    g_c302 = (c302_def *)g_recv_buf;
    g_r302 = (r302_def *)g_resp_buf;
    g_c502 = (c502_def *)g_recv_buf;
    g_r502 = (r502_def *)g_resp_buf;
    g_c401 = (cr401_def *)g_serv_buf;
    g_r401 = (cr401_def *)g_serv_buf;
    g_c402 = (cr402_def *)g_serv_buf;
    g_r402 = (cr402_def *)g_serv_buf;
    g_recvque = (db_gqnwq_def *)&g_c302->msg_data;
    g_recvque_detail = (receivequeue_info_def *)&g_c302->msg_data;
    g_sendque = (db_gqnwq_def *)&g_c301->que_rgs_info.msg_data;
    g_ffmt_head = (ffd_header_def *)g_ffmt_buf;

    /* サーバ管理テーブル */
    for (s_idx = 0; s_idx < DEF_CMSD_SV_MAX; s_idx++) {
        memset((char *)&g_svrtbl[s_idx], 0x00, sizeof(server_tbl_def));
    }

    /* 受信電文振分先情報取得処理 */
    s_result = CMSD_get_branch_info();
    if (s_result != DEF_RET_OK) {
        g_myinfo.end_flg = DEF_FLAG_ON;
        return DEF_RET_NG;
    }

    /* サーバクラス名取得処理 */
    s_result = CMSD_get_serverclass_names();
    if (s_result != DEF_RET_OK) {
        g_myinfo.end_flg = DEF_FLAG_ON;
        return DEF_RET_NG;
    }

    /* ISO8583エレメント情報取得処理 */
    s_result = CMSD_get_iso_elements();
    if (s_result != DEF_RET_OK) {
        g_myinfo.end_flg = DEF_FLAG_ON;
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* CMIN_kbt_init */

/****************************************************************************/
/*  FUNCTION        : 1.2.0  CMIN_kbt_get_physical_names                    */
/*  CALLING SEQ.    : short CMIN_kbt_get_physical_names(void)               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 物理名情報取得処理                               */
/****************************************************************************/
short CMIN_kbt_get_physical_names(void)
{
    short           s_result;
    db_gfphi_def    *db_gfph;
    t_gfphi_pri_key gfphi_pri_key;
    char            wkbuf[64];

    db_gfph = (db_gfphi_def *)g_com_iom_arg_6.rec_area;

    /* 検索情報初期化 */
    memset((char *)&gfphi_pri_key, 0x00, sizeof(gfphi_pri_key));
    gfphi_pri_key.site_id = g_myinfo.site_id;
    gfphi_pri_key.nw_id = g_myinfo.network_id;
    memcpy(gfphi_pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));
    memcpy(gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_NAME_DEFAULT, strlen(DEF_SC_NAME_DEFAULT));
    memcpy(gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num, DEF_SC_NUM_DEFAULT, strlen(DEF_SC_NUM_DEFAULT));
    memcpy(gfphi_pri_key.srv_cls_key.srv_cls_mlt_num, DEF_SC_DUP_DEFAULT, strlen(DEF_SC_DUP_DEFAULT));
    memcpy(gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_CTRL_DEN_LOG, strlen(DEF_FL_CTRL_DEN_LOG));
    memcpy(gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(gfphi_pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    /* 制御電文管理ファイル名取得 */
    memcpy(gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_CTRL_DEN_MG, strlen(DEF_FL_CTRL_DEN_MG));
    s_result = CMIN_get_phy_name((char *)&gfphi_pri_key);
    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, (char *)&gfphi_pri_key, sizeof(gfphi_pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_PHSIC_INFO
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_RET_NG;
    }
    memset(g_kbt_file_data.GFMTL_file_name, 0x00 ,sizeof(g_kbt_file_data.GFMTL_file_name));
    memcpy(g_kbt_file_data.GFMTL_file_name,
        db_gfph->prc_file_info.prc_file_name, sizeof(g_kbt_file_data.GFMTL_file_name));
    g_kbt_file_data.GFMTL_file_no = -1;

    /* 受信電文振分先設定ファイルファイル名取得 */
    memcpy(gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_RCV_DEN_FURI, strlen(DEF_FL_RCV_DEN_FURI));
    s_result = CMIN_get_phy_name((char *)&gfphi_pri_key);
    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, (char *)&gfphi_pri_key, sizeof(gfphi_pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_PHSIC_INFO
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_RET_NG;
    }
    memset(g_kbt_file_data.GFQSW_file_name, 0x00 ,sizeof(g_kbt_file_data.GFQSW_file_name));
    memcpy(g_kbt_file_data.GFQSW_file_name,
        db_gfph->prc_file_info.prc_file_name, sizeof(g_kbt_file_data.GFQSW_file_name));
    g_kbt_file_data.GFQSW_file_no = -1;

    /* 制御電文エレメント情報ファイルファイル名取得 */
    memcpy(gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_CTRL_MSG_ELM, strlen(DEF_FL_CTRL_MSG_ELM));
    s_result = CMIN_get_phy_name((char *)&gfphi_pri_key);
    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, (char *)&gfphi_pri_key, sizeof(gfphi_pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_PHSIC_INFO
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_RET_NG;
    }
    memset(g_kbt_file_data.GFELI_file_name, 0x00 ,sizeof(g_kbt_file_data.GFELI_file_name));
    memcpy(g_kbt_file_data.GFELI_file_name,
        db_gfph->prc_file_info.prc_file_name, sizeof(g_kbt_file_data.GFELI_file_name));
    g_kbt_file_data.GFELI_file_no = -1;

    return DEF_RET_OK;
} /* CMIN_kbt_get_physical_names */

/****************************************************************************/
/*  FUNCTION        : 1.3.0  CMIN_kbt_file_open                             */
/*  CALLING SEQ.    : short CMIN_kbt_file_open(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 ファイルオープン処理                             */
/****************************************************************************/
short CMIN_kbt_file_open(void)
{
    short       s_result;

    /* 制御電文管理ファイル */
    g_kbt_file_data.GFMTL_file_no = DEF_FNO_UNKOWN;
    s_result = CMIN_file_open(DEF_FL_CTRL_DEN_MG, g_kbt_file_data.GFMTL_file_name, &g_kbt_file_data.GFMTL_file_no);
    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_CTRL_DEN_MG
                            , DEF_FILEIO_OPEN
                            , ""
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* CMIN_kbt_file_open */

/****************************************************************************/
/*  FUNCTION        : 1.4.0  CMIN_kbt_file_close                            */
/*  CALLING SEQ.    : void CMIN_kbt_file_close(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 個別 ファイルクローズ処理                             */
/****************************************************************************/
void CMIN_kbt_file_close(void)
{
    if (g_kbt_file_data.GFMTL_file_no == DEF_FNO_UNKOWN) {
        return;
    }
    /* 制御電文管理ファイル */
    CMIN_file_close(DEF_FL_CTRL_DEN_MG, g_kbt_file_data.GFMTL_file_name, &g_kbt_file_data.GFMTL_file_no);

} /* CMIN_kbt_file_close */


/****************************************************************************/
/*  FUNCTION        : 1.5.0  CMIN_handle_req_msg                            */
/*  CALLING SEQ.    : void CMIN_handle_req_msg(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 電文受信処理                                     */
/****************************************************************************/
void CMIN_handle_req_msg(void)
{

    /* 制御電文管理情報初期化 */
    memset((char *)&g_ctl_info, 0x00, sizeof(g_ctl_info));
    memset(g_ctl_info.req_gfp_lcn, 0x20, sizeof(g_ctl_info.req_gfp_lcn));
    memset(g_ctl_info.qrecv_info.mti, 0x20, sizeof(g_ctl_info.qrecv_info.mti));
    memset(g_ctl_info.qsend_info.mti, 0x20, sizeof(g_ctl_info.qsend_info.mti));
    memset(g_ctl_info.pserv_info.mti, 0x20, sizeof(g_ctl_info.pserv_info.mti));
    memcpy(g_ctl_info.internal_err, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    memset((char *)&g_ctl_info.denbun_log_key, 0x20, sizeof(g_ctl_info.denbun_log_key));
    memset((char *)&g_ctl_info.connection_lid, 0x20, sizeof(g_ctl_info.connection_lid));

    /* インタフェースコード振分 */
    if (memcmp(g_ipcreq_head->interface_code, DEF_IPC_IFCD_Q_GET_NT_REQ, strlen(DEF_IPC_IFCD_Q_GET_NT_REQ))==0) {
        /* キュー取出し通知要求処理 */
        CMSD_inbound_request();
        return;
    }
    if (memcmp(g_ipcreq_head->interface_code, DEF_IPC_IFCD_CMD_PRC_REQ, strlen(DEF_IPC_IFCD_CMD_PRC_REQ))==0) {
        /* コマンド処理要求処理 */
        CMSD_command_request();
        return;
    }

    /* 非対応インタフェース */
    CMIN_message_output ( DEF_EVT_REQ_ERR
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_IPC_SEISA_ERR
                        , "@C@H"
                        , "ﾋﾀｲｵｳIPC"
                        , (char *)g_ipcreq_head);
    memcpy(g_ipcres_head->interface_code, g_ipcreq_head->interface_code, sizeof(common_header_def));
    g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
    memcpy(g_ipcres_head->internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    g_ipcres_head->control_data_length = 0;
    CMIN_send_reply((char *)g_ipcres_head, sizeof(common_header_def), 0);

} /* CMIN_handle_req_msg */
/************************************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 2.0.0  CMSD_get_branch_info                           */
/*  CALLING SEQ.    : short CMSD_get_branch_info(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 受信電文振分先情報取得処理                            */
/****************************************************************************/
short CMSD_get_branch_info(void)
{
    db_gfqsw_def    *db_gfqsw;
    short           s_result;
    char            wkbuf[64];

    db_gfqsw = (db_gfqsw_def *)&g_com_iom_arg_6.rec_area;

    /* 受信電文振分先設定ファイルオープン */
    s_result = CMIN_file_open(DEF_FL_RCV_DEN_FURI, g_kbt_file_data.GFQSW_file_name, &g_kbt_file_data.GFQSW_file_no);
    if (s_result != DEF_RET_OK) {
        return DEF_RET_NG;
    }

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* 受信電文振分先設定ファイル読込み */
    db_gfqsw->pri_key.site_id = g_myinfo.site_id;
    db_gfqsw->pri_key.nw_id = g_myinfo.network_id;
    memcpy(db_gfqsw->pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));
    memcpy(db_gfqsw->pri_key.mti_id, DEF_GFQSW_ILLEGAL_MTI, strlen(DEF_GFQSW_ILLEGAL_MTI));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_RCV_DEN_FURI, strlen(DEF_FL_RCV_DEN_FURI));
    memcpy(g_com_iom_arg_3.file_name, g_kbt_file_data.GFQSW_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFQSW, strlen(DEF_GFQSW));
    memcpy(g_com_iom_arg_4.file_name, g_kbt_file_data.GFQSW_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_kbt_file_data.GFQSW_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gfqsw->pri_key, sizeof(db_gfqsw->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len           = sizeof(db_gfqsw->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gfqsw->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfqsw_def_Size;

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        if (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_EOF_ERR, strlen(DEF_COM_IOM_EOF_ERR))==0) {
            return DEF_RET_OK;
        }
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_com_iom_arg_5.key_value, g_com_iom_arg_5.key_len);
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_RCV_DEN_FURI
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_RET_NG;
    }

    /* サーバクラス論理ID取得 */
    switch (g_myinfo.site_id) {
    case DEF_SITE_ID_TKY :
        memcpy(g_svrtbl[DEF_CMSD_SV_APL_IN].logicalname,
            (char *)&db_gfqsw->recv_qfile_info.tky_site_info.srv_cls_id,
            sizeof(db_gfqsw->recv_qfile_info.tky_site_info.srv_cls_id));
        break;
    case DEF_SITE_ID_OSK :
        memcpy(g_svrtbl[DEF_CMSD_SV_APL_IN].logicalname,
            (char *)&db_gfqsw->recv_qfile_info.osk_site_info.srv_cls_id,
            sizeof(db_gfqsw->recv_qfile_info.osk_site_info.srv_cls_id));
        break;
    default:
        CMIN_abend();
    }

    /* 受信電文振分先設定ファイルオープン */
    CMIN_file_close(DEF_FL_RCV_DEN_FURI, g_kbt_file_data.GFQSW_file_name, &g_kbt_file_data.GFQSW_file_no);

    return DEF_RET_OK;
} /* CMSD_get_branch_info */

/****************************************************************************/
/*  FUNCTION        : 3.0.0  CMSD_get_serverclass_names                     */
/*  CALLING SEQ.    : short CMSD_get_serverclass_names(void)                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : サーバクラス名取得処理                                */
/****************************************************************************/
short CMSD_get_serverclass_names(void)
{
    db_gfphi_def    *db_gfphi;
    short           s_result;
    char            wkbuf[64];
    short           s_idx;

    db_gfphi = (db_gfphi_def *)&g_com_iom_arg_6.rec_area;

    // 物理名情報ファイルのオープンクローズは共通メインで管理

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_PHSIC_INFO, strlen(DEF_FL_PHSIC_INFO));
    memcpy(g_com_iom_arg_3.file_name, g_com_file_data.phy_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(g_com_iom_arg_4.file_name, g_com_file_data.phy_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_com_file_data.phy_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gfphi->pri_key, sizeof(db_gfphi->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len            = sizeof(db_gfphi->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gfphi->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfphi_def_Size;

    /* 検索キー */
    for (s_idx = 0; s_idx < DEF_CMSD_SV_MAX; s_idx++) {
        db_gfphi->pri_key.site_id = g_myinfo.site_id;
        db_gfphi->pri_key.nw_id = g_myinfo.network_id;
        memcpy(db_gfphi->pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));
        memcpy(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_kind, "}}}}}}}}", 8);
        memcpy(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_num, "}}}}", 4);
        memcpy(db_gfphi->pri_key.prc_file_key.prc_file_mlt_num, "}}}}", 4);

        switch (s_idx) {
        case DEF_CMSD_SV_TIMER      :   /* タイマー制御サーバ           */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_TIMER, strlen(DEF_SC_TIMER));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_TIMER].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_TIMER_B    :   /* タイマー制御サーバ(BACKUP)   */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_TIMER, strlen(DEF_SC_TIMER));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0001", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_TIMER_B].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_SIGN_ECHO  :   /* 局状態・エコー制御サーバ     */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CNT_STS_ECH, strlen(DEF_SC_CNT_STS_ECH));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_SIGN_ECHO].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_SIGN_ECHO_AUTO :   /* 局状態・エコー制御サーバ(自動) */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CNT_STS_ECH, strlen(DEF_SC_CNT_STS_ECH));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0001", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_SIGN_ECHO_AUTO].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_KEYEXC     :   /* 鍵交換制御サーバ             */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_KEY_EXCH, strlen(DEF_SC_KEY_EXCH));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_KEYEXC].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_CUTOVER    :   /* カットオーバー制御サーバ     */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CUT_OVER, strlen(DEF_SC_CUT_OVER));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_CUTOVER].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_SAF        :   /* SAF送信制御サーバ            */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_SAF, strlen(DEF_SC_SAF));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_SAF].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_NOTICE     :   /* 通知電文制御サーバ           */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_NTF_MSG, strlen(DEF_SC_NTF_MSG));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_NOTICE].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_CTL_OUT    :   /* 制御電文IF(outbound)サーバ   */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CTRL_IF_O, strlen(DEF_SC_CTRL_IF_O));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_CTL_OUT].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        case DEF_CMSD_SV_APL_IN     :   /* 業務電文中継(inbound)サーバ  */
            if (g_svrtbl[DEF_CMSD_SV_APL_IN].logicalname[0] == 0) {
                continue;
            }
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
                &g_svrtbl[DEF_CMSD_SV_APL_IN].logicalname[0],
                sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,
                &g_svrtbl[DEF_CMSD_SV_APL_IN].logicalname[sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind)],
                sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            break;
        case DEF_CMSD_SV_APL_CTL    :   /* 制御電文中継(inbound)サーバ  */
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CTRL_CHU_I, strlen(DEF_SC_CTRL_CHU_I));
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, "0000", 4);
            memcpy(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", 4);
            memcpy(g_svrtbl[DEF_CMSD_SV_APL_CTL].logicalname,
                (char *)&db_gfphi->pri_key.srv_cls_key.srv_cls_id, sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id));
            break;
        default:
            CMIN_abend();
        }
        memcpy(g_com_iom_arg_5.key_value, (char *)&db_gfphi->pri_key, sizeof(db_gfphi->pri_key));

        /* 共通I/Oモジュール */
        s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                           , g_ch_sub_prog_sts
                           , &g_com_iom_arg_3
                           , &g_com_iom_arg_4
                           , &g_com_iom_arg_5
                           , &g_com_iom_arg_6);

        if ((s_result != DEF_RET_OK) ||
            (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
            if (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_EOF_ERR, strlen(DEF_COM_IOM_EOF_ERR))==0) {
                continue;
            }
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_com_iom_arg_5.key_value, sizeof(db_gfphi->pri_key));
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@C@C@C@C@C@U"
                                , ""
                                , ""
                                , DEF_FL_PHSIC_INFO
                                , DEF_FILEIO_READ
                                , wkbuf
                                , g_com_iom_arg_6.guardian_errcode);
            return DEF_RET_NG;
        }

        memcpy(g_svrtbl[s_idx].domainname,
            db_gfphi->srv_cls_info.domain_name, sizeof(db_gfphi->srv_cls_info.domain_name));
        memcpy(g_svrtbl[s_idx].monname,
            db_gfphi->srv_cls_info.pathmon_name, sizeof(db_gfphi->srv_cls_info.pathmon_name));
        memcpy(g_svrtbl[s_idx].scname,
            db_gfphi->srv_cls_info.srv_cls_name, sizeof(db_gfphi->srv_cls_info.srv_cls_name));
    }

    return DEF_RET_OK;
} /* CMSD_get_serverclass_names */


/****************************************************************************/
/*  FUNCTION        : 4.0.0  CMSD_get_iso_elements                          */
/*  CALLING SEQ.    : short CMSD_get_iso_elements(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : ISO8583エレメント情報取得処理                         */
/****************************************************************************/
short CMSD_get_iso_elements(void)
{
    filename_l_t    gfeli_name_l;
    filename_p_t    gfeli_name_p;
    bool            bl_result;

    memset(&g_iso8583_context, 0, sizeof(ISO8583context_t));

    memset(gfeli_name_l, 0x00, sizeof(gfeli_name_l));
    memcpy(gfeli_name_l, DEF_FL_CTRL_MSG_ELM, strlen(DEF_FL_CTRL_MSG_ELM));
    memset(gfeli_name_p, 0x00, sizeof(gfeli_name_p));
    memcpy(gfeli_name_p, g_kbt_file_data.GFELI_file_name, sizeof(g_kbt_file_data.GFELI_file_name));

    /* 固定フォーマット初期化 */
    bl_result = com_btm_initial_ISO8583(
        &g_iso8583_context,
        DEF_GFPCVX70,
        gfeli_name_l,
        gfeli_name_p,
        g_myinfo.network_id,
        g_ffd_idx,
        DEF_CMSD_FFDIDX_MAX,
        g_ffd,
        DEF_CMSD_FFD_MAX);
    if (!bl_result) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@5"
                            , "com_btm_initial_ISO8583"
                            , g_iso8583_context.m_error_info.m_errCd);
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* CMSD_get_iso_elements */


/****************************************************************************/
/*  FUNCTION        : 5.0.0  CMSD_inbound_request                           */
/*  CALLING SEQ.    : void CMSD_inbound_request(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : キュー取出し通知要求処理                              */
/****************************************************************************/
void CMSD_inbound_request(void)
{
    /* 受信IPC長 */
    g_ctl_info.qrecv_info.ipc_len = g_recv_len;

    /* 電文受信通知 */
    if (memcmp(g_recvque->cntrl_info.shori_kubun,
        DEF_SHORI_KUBUN_RECV, strlen(DEF_SHORI_KUBUN_RECV))==0) {
        if (g_recvque->denbun_send_recv_info.send_denbun_shubetu == DEF_SEND_DENBUN_CTR_REQ) {
            /* 制御要求電文受信処理 */
            CMSD_recv_request();
            return;
        }
        if (g_recvque->denbun_send_recv_info.send_denbun_shubetu == DEF_SEND_DENBUN_CTR_RSP) {
            /* 制御応答電文受信処理 */
            CMSD_recv_response();
            return;
        }
    }
    /* 電文送信不可応答 */
    else if (memcmp(g_recvque->cntrl_info.shori_kubun,
        DEF_SHORI_KUBUN_SENDERROR, strlen(DEF_SHORI_KUBUN_SENDERROR))==0) {
        if (g_recvque->denbun_send_recv_info.send_denbun_shubetu == DEF_SEND_DENBUN_CTR_REQ) {
            /* 制御要求電文送信不可処理 */
            CMSD_request_send_error();
            return;
        }
        if (g_recvque->denbun_send_recv_info.send_denbun_shubetu == DEF_SEND_DENBUN_CTR_RSP) {
            /* 制御応答電文送信不可処理 */
            CMSD_response_send_error();
            return;
        }
    }
    /* T/O通知 */
    else if (memcmp(g_recvque->cntrl_info.shori_kubun,
        DEF_SHORI_KUBUN_TIMEOUT, strlen(DEF_SHORI_KUBUN_TIMEOUT))==0) {
        /* 制御応答電文タイムアウト処理 */
        CMSD_response_timeout();
        return;
    }

    /* 非対応処理コード区分 *//* 破棄 */
    CMIN_message_output ( DEF_EVT_REQ_ERR
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_IPC_SEISA_ERR
                        , "@C@H"
                        , "ｼｮﾘｺｰﾄﾞｸﾌﾞﾝ"
                        , (char *)g_ipcreq_head);
    memcpy(g_ipcres_head->interface_code, g_ipcreq_head->interface_code, sizeof(common_header_def));
    g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
    memcpy(g_ipcres_head->internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    g_ipcres_head->control_data_length = 0;
    CMIN_send_reply((char *)g_ipcres_head, sizeof(common_header_def), 0);

} /* CMSD_inbound_request */


/****************************************************************************/
/*  FUNCTION        : 6.0.0  CMSD_inbound_response                          */
/*  CALLING SEQ.    : void CMSD_inbound_response(void)                      */
/*  ARGUMENT        : 1.reply_kbn      (I)   リプライ区分                   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : キュー取出し通知応答処理                              */
/****************************************************************************/
void CMSD_inbound_response(short reply_kbn)
{
    /* キュー取出し通知応答編集 */
    memcpy(g_r302->common_header.interface_code, DEF_IPC_IFCD_Q_GET_NT_RSP, strlen(DEF_IPC_IFCD_Q_GET_NT_RSP));
    g_r302->common_header.error_code = reply_kbn;
    if (reply_kbn == DEF_IPC_ERRCD_OK) {
        memcpy(g_r302->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    } else {
        memcpy(g_r302->common_header.internal_error_code, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
    }
    g_r302->common_header.control_data_length = 0;

    /* キュー取出し通知応答リプライ */
    CMIN_send_reply((char *)g_r302, sizeof(r302_def), 0);

} /* CMSD_inbound_response */

/****************************************************************************/
/*  FUNCTION        : 7.0.0  CMSD_recv_request                              */
/*  CALLING SEQ.    : void CMSD_recv_request(void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御要求電文受信処理                                  */
/****************************************************************************/
void CMSD_recv_request(void)
{
    COM_SDT_arg_2_def   lcltime_c;
    COM_SDT_arg_3_def   lcltime_b;
    long long           lcltimestamp;
    short               s_result;
    char                wkbuf[64];

    /****************************************/
    /* 受信電文精査                         */
    /****************************************/
    /* 受信キューレコード長 */
    g_ctl_info.qrecv_info.qfile_len = g_ipcreq_head->control_data_length;
    /* MTI */
    memcpy((char *)&g_ctl_info.qrecv_info.mti,
        g_recvque->denbun_send_recv_info.mti_id,
        sizeof(g_recvque->denbun_send_recv_info.mti_id));
    /* 電文ログKEY */
    memcpy((char *)&g_ctl_info.denbun_log_key,
        (char *)&g_recvque->denbun_send_recv_info.denbun_log_key,
        sizeof(g_recvque->denbun_send_recv_info.denbun_log_key));
    /* GFP内部LCN */
    memcpy(g_ctl_info.req_gfp_lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    /* 受信コネクション論理ID */
    memcpy((char *)&g_ctl_info.connection_lid,
        (char *)&g_recvque->tushin_cntrl_info.line_info.recv_connect_id,
        sizeof(g_recvque->tushin_cntrl_info.line_info.recv_connect_id));

    /* 電文長 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_recvque->denbun_area.denbun_len, sizeof(g_recvque->denbun_area.denbun_len));
    g_ctl_info.qrecv_info.qdata_len = (unsigned short)atoi(wkbuf);

    /* インタフェース単位NW情報取得 */
    s_result = CMSD_read_netfile(
        DEF_CMSD_UNIT_INTERFACE,
        g_ctl_info.connection_lid.if_id,
        g_ctl_info.connection_lid.station_id);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* エラー出力ログ編集出力*/
        s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
        if (s_result != DEF_RET_OK) {
            CMSD_inbound_response(DEF_IPC_ERRCD_NG);
            return;
        }
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }

    /* MTI OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct));
    g_ctl_info.mti_off = (short)atoi(wkbuf)-1;

    /* MTI レングス */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_item_len, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_item_len));
    g_ctl_info.mti_len = (short)atoi(wkbuf);

    /* 電文データ OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct));
    g_ctl_info.msg_off = (short)atoi(wkbuf)-1;

    /* ビットマップ開始位置 */
    g_ctl_info.bitmap_off = (g_ctl_info.mti_off - g_ctl_info.msg_off) + g_ctl_info.mti_len;

    if (g_ctl_info.qrecv_info.mti[0] != ' ') {
        /****************************************/
        /* ビットマップ展開                     */
        /****************************************/
        g_format_info.iso8583_len = g_ctl_info.qrecv_info.qdata_len - g_ctl_info.bitmap_off;
        memset(g_iso8583_buf, 0x00, sizeof(g_iso8583_buf));
        memcpy(g_iso8583_buf,
            &g_recvque->denbun_area.denbun[g_ctl_info.bitmap_off], g_format_info.iso8583_len);
        s_result = CMSD_deploy_bitmap();
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_DEC_ERR, strlen(DEF_NERR_CMSD_BITMAP_DEC_ERR));
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_ctl_info.qrecv_info.mti, sizeof(g_ctl_info.qrecv_info.mti));
            CMIN_message_output ( DEF_EVT_BITMAP_EXPND_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_BITMAP_DEC_ERR
                                , "@L@C@3@H"
                                , g_ctl_info.req_gfp_lcn
                                , wkbuf
                                , g_iso8583_object.m_error_info.m_de_number
                                , (char *)g_ipcreq_head);
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            /* 障害電文通知要否 */
            if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
                /* 障害電文作成依頼処理 */
                CMSD_outbound_reject();
            }
            return;
        }

        /****************************************/
        /* 電文種別判定処理                     */
        /****************************************/
        s_result = NWM_MSJ(
            g_recvque->denbun_send_recv_info.mti_id,
            g_ffmt_buf,
            (char *)&g_ctl_info.qrecv_info.ctlkind);
        if (s_result != DEF_MSJ_NORMAL) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_HSMK_REQ_SEISA_ERR, strlen(DEF_NERR_HSMK_REQ_SEISA_ERR));
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_ctl_info.qrecv_info.mti, sizeof(g_ctl_info.qrecv_info.mti));
//          CMIN_message_output ( DEF_EVT_MTI_HANTE_ERR
//                              , DEF_MSGTTKB_GYOM_ERR
//                              , DEF_NERR_HSMK_REQ_SEISA_ERR
//                              , "@L@T@C"
//                              , g_ctl_info.req_gfp_lcn
//                              , (char *)&g_ctl_info.connection_lid
//                              , wkbuf);
            CMIN_message_output( DEF_EVT_DATA_FLD_SEISA_ERR
                               , DEF_MSGTTKB_GYOM_ERR
                               , DEF_NERR_HSMK_REQ_SEISA_ERR
                               , "@L@T@C@C"
                               , g_ctl_info.req_gfp_lcn
                               , (char *)&g_ctl_info.connection_lid
                               , wkbuf
                               , "NWM_MSJ(ﾃﾞﾝﾌﾞﾝｼｭﾍﾞﾂﾊﾝﾃｲ)ｴﾗｰ");
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            /* 障害電文通知要否 */
            if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
                /* 障害電文作成依頼処理 */
                CMSD_outbound_reject();
            }
            return;
        }
    } else {
        /* 通知電文扱い */
        g_ctl_info.qrecv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_NTF_MSG;
        g_ctl_info.qrecv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.qrecv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_FAL;
        g_ctl_info.qrecv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    }

    /****************************************/
    /* サーバ制御電文振分処理               */
    /****************************************/
    switch(g_ctl_info.qrecv_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :
        if ((g_ctl_info.qrecv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST) ||
            (g_ctl_info.qrecv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT)) {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO_AUTO;
        } else {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        }
        break;
    case DEF_CTLFNC_KEY_EXCH    :
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        break;
    case DEF_CTLFNC_CUT_OVER    :
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        break;
    case DEF_CTLFNC_SAF         :
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        break;
    case DEF_CTLFNC_NTF_MSG     :
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* サーバ制御電文受信通知処理           */
    /****************************************/
    if (g_ctl_info.qrecv_info.ctlkind.req_res_kbn == DEF_CTLMSG_RESPONSE) {
        g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_SIMUKE);
    } else {
        g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_HISIMUKE);
    }

    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_SENDDATA      :                   /* 送信電文あり     */
        /****************************************/
        /* ビットマップ組立                     */
        /****************************************/
        s_result = CMSD_create_bitmap();
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_ENC_ERR, strlen(DEF_NERR_CMSD_BITMAP_ENC_ERR));
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_ctl_info.qsend_info.mti, sizeof(g_ctl_info.qsend_info.mti));
            CMIN_message_output ( DEF_EVT_BITMAP_ASMBL_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_BITMAP_ENC_ERR
                                , "@L@C@3@H"
                                , g_ctl_info.req_gfp_lcn
                                , wkbuf
                                , g_iso8583_object.m_error_info.m_de_number
                                , (char *)g_ipcreq_head);
            g_ctl_info.server_sts = DEF_CMSD_STS_SENDDATAERROR;     /* 電文送信エラー */
            break;
        }
        break;
    case DEF_CMSD_STS_NORMAL        :                   /* 送信電文なし     */
        break;
    case DEF_CMSD_STS_PATHSENDERROR :                   /* PATHSENDエラー   */
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_ERRORRESPONSE :                   /* 異常応答         */
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_IFERROR       :
        /* エラー出力ログ編集出力*/
        s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
        if (s_result != DEF_RET_OK) {
            CMSD_inbound_response(DEF_IPC_ERRCD_NG);
            return;
        }
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    case DEF_CMSD_STS_FAULTTEXT     :                   /* 障害電文         */
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* 制御電文管理レコード登録             */
    /****************************************/
    if (g_ctl_info.server_sts != DEF_CMSD_STS_FAULTTEXT) {
        /* システム日時取得 */
        memset((char *)&lcltime_c, 0x00, sizeof(lcltime_c));
        memset((char *)&lcltime_b, 0x00, sizeof(lcltime_b));
        COM_SDT(2, &lcltime_c, &lcltime_b, &lcltimestamp);

        memset((char *)&g_gfmtl_ctl, 0x20, sizeof(g_gfmtl_ctl));
        g_gfmtl_ctl.pri_key.part_id[0] = '0';
        g_gfmtl_ctl.pri_key.part_id[1] = g_ctl_info.denbun_log_key.tran_id.gfp_lcn[14];
        memcpy(g_gfmtl_ctl.pri_key.lcn_id, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_gfmtl_ctl.pri_key.lcn_id));
        if (g_ctl_info.qrecv_info.ctlkind.req_res_kbn == DEF_CTLMSG_RESPONSE) {
            g_gfmtl_ctl.s_h_kubun = DEF_S_H_KUBUN_SIMUKE;
            memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_RCV, strlen(DEF_TRHK_STS_RSP_RCV));
            memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qrecv_info.ctlkind, sizeof(g_ctl_info.qrecv_info.ctlkind));
        } else {
            g_gfmtl_ctl.s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
            switch(g_ctl_info.server_sts) {
            case DEF_CMSD_STS_SENDDATA  :
                memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_SEND, strlen(DEF_TRHK_STS_RSP_SEND));
                memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qsend_info.ctlkind, sizeof(g_ctl_info.qsend_info.ctlkind));
                break;
            case DEF_CMSD_STS_NORMAL    :
                memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_NTF_RCV, strlen(DEF_TRHK_STS_NTF_RCV));
                memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qrecv_info.ctlkind, sizeof(g_ctl_info.qrecv_info.ctlkind));
                break;
            case DEF_CMSD_STS_FAULTTEXT :
                memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qrecv_info.ctlkind, sizeof(g_ctl_info.qrecv_info.ctlkind));
                break;
            case DEF_CMSD_STS_SENDDATAERROR :
                memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_SENDERR, strlen(DEF_TRHK_STS_RSP_SENDERR));
                memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qsend_info.ctlkind, sizeof(g_ctl_info.qsend_info.ctlkind));
                break;
            default:
                CMIN_abend();
            }
        }
        memcpy((char *)&g_gfmtl_ctl.torihiki_info, (char *)&g_ctl_info.connection_lid, sizeof(g_ctl_info.connection_lid));
        memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_gfmtl_ctl.naibu_err_id));
        memcpy(g_gfmtl_ctl.entry_timestamp, (char *)&lcltime_c, sizeof(g_gfmtl_ctl.entry_timestamp));

        s_result = CMSD_write_controlfile();
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            g_ctl_info.server_sts = DEF_CMSD_STS_SENDDATAERROR;     /* 電文送信エラー */
        }
    }

    /****************************************/
    /* キュー取出し通知応答処理             */
    /****************************************/
    if (g_ctl_info.server_sts != DEF_CMSD_STS_SENDDATAERROR) {
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
    } else {
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
    }

    /****************************************/
    /* NW電文受信通知後処理                 */
    /****************************************/
    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_SENDDATA      :       /* 送信電文有り */
        break;
    case DEF_CMSD_STS_NORMAL        :       /* 送信電文無し */
        return;
    case DEF_CMSD_STS_FAULTTEXT     :       /* 障害電文 */
        /* 障害電文通知要否 */
        if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
            /* 障害電文作成依頼処理 */
            CMSD_outbound_reject();
        }
        return;
    case DEF_CMSD_STS_SENDDATAERROR :       /* 電文送信エラー */
        /* 電文送信不可通知 */
        CMSD_recv_request_send_error();
        return;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* 送信キュー編集                       */
    /****************************************/
    memset((char *)g_sendque, 0x00, sizeof(db_gqnwq_def));
    memcpy((char *)g_sendque,
        (char *)g_recvque, (g_ctl_info.qrecv_info.qfile_len - g_ctl_info.qrecv_info.qdata_len));
    g_ctl_info.qsend_info.qdata_len = (unsigned short)(g_ctl_info.bitmap_off + g_format_info.iso8583_len);

    /* キューファイル制御情報 */
    memcpy(g_sendque->cntrl_info.shori_kubun, DEF_SHORI_KUBUN_SEND, strlen(DEF_SHORI_KUBUN_SEND));
    memset(g_sendque->cntrl_info.err_code, 0x20, sizeof(g_sendque->cntrl_info.err_code));
    /* キューファイル電文送受信情報 */
    memcpy(g_sendque->denbun_send_recv_info.mti_id,
        g_ctl_info.qsend_info.mti, sizeof(g_sendque->denbun_send_recv_info.mti_id));
    g_sendque->denbun_send_recv_info.send_denbun_shubetu = DEF_SEND_DENBUN_CTR_RSP;
    memset(g_sendque->denbun_send_recv_info.tushin_log_save_filename,
        0x20, sizeof(g_sendque->denbun_send_recv_info.tushin_log_save_filename));
    memset((char *)&g_sendque->denbun_send_recv_info.tushin_log_key,
        0x20, sizeof(g_sendque->denbun_send_recv_info.tushin_log_key));
    memcpy((char *)&g_sendque->denbun_send_recv_info.denbun_log_key,
        (char *)&g_r401->control_info.denbun_log_key, sizeof(g_sendque->denbun_send_recv_info.denbun_log_key));
    /* 通信制御情報(要求内容に同じ) */
    /* 電文長 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", g_ctl_info.qsend_info.qdata_len);
    memcpy(g_sendque->denbun_area.denbun_len, wkbuf, sizeof(g_sendque->denbun_area.denbun_len));
    /* MTI OFFSET(要求内容)に同じ */
    /* 送受信電文 */
    memcpy(&g_sendque->denbun_area.denbun[0],
        g_r401->data_bu.message_text, g_ctl_info.bitmap_off);
    memcpy(&g_sendque->denbun_area.denbun[g_ctl_info.bitmap_off],
        g_iso8583_buf, g_format_info.iso8583_len);

    /* 電文ヘッダ電文長編集 */
    s_result = NWM_HDL(g_sendque->denbun_area.denbun, (short)(g_format_info.iso8583_len + g_ctl_info.mti_len));
    if (s_result != DEF_HDL_NORMAL) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@C@5"
                            , "NWM_HDL"
                            , s_result);
        /* 電文送信不可通知 */
        CMSD_recv_request_send_error();
        return;
    }

    /****************************************/
    /* キュー登録処理                       */
    /****************************************/
    g_ctl_info.qsend_info.qfile_len =
        (db_gqnwq_def_Size - sizeof(g_sendque->denbun_area.denbun)) + g_ctl_info.qsend_info.qdata_len;
    s_result = CMSD_outbound_text(DEF_CMSD_SV_CTL_OUT);
    if (s_result != DEF_CMSD_STS_NORMAL) {
        /* 電文送信不可通知 */
        CMSD_recv_request_send_error();
        return;
    }

} /* CMSD_recv_request */

/****************************************************************************/
/*  FUNCTION        : 8.0.0  CMSD_recv_request_send_error                   */
/*  CALLING SEQ.    : void CMSD_recv_request_send_error(void)               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御応答電文送信不可処理                              */
/****************************************************************************/
void CMSD_recv_request_send_error(void)
{
    long            tranid = -1L;
    short           s_result;

    /****************************************/
    /* 送信エラー処理                       */
    /****************************************/
    /* BEGINTRANSACTION */
    s_result = COM_TMF(DEF_COM_TMF_BEGIN, &tranid, DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_TMF_ERR
                            , "@C@U"
                            , "COM_TMF(BT)"
                            , s_result);
        CMIN_abend();
    }

    /* 制御電文管理ファイル更新 */
    s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* エラー出力ログ編集出力*/
        CMSD_errorlog(DEF_CMSD_ELG_SENDQUE, g_sendque->denbun_area.denbun, g_ctl_info.qsend_info.qdata_len);
        return;
    }
    memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_SENDERR, strlen(DEF_TRHK_STS_RSP_SENDERR));
    memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
    s_result = CMSD_update_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        CMIN_abend();
    }

    /* 送信不可通知処理 */
    s_result = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_HISIMUKE_ERROR);
    if (s_result != DEF_CMSD_STS_NORMAL) {
        /* ABORTTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_ABORT, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(AT)"
                                , s_result);
            CMIN_abend();
        }
        /* エラー出力ログ編集出力*/
        CMSD_errorlog(DEF_CMSD_ELG_SENDQUE, g_sendque->denbun_area.denbun, g_ctl_info.qsend_info.qdata_len);
        return;
    }

    /* ENDTRANSACTION */
    s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_TMF_ERR
                            , "@C@U"
                            , "COM_TMF(ET)"
                            , s_result);
        CMIN_abend();
    }
} /* CMSD_send_response_error */

/****************************************************************************/
/*  FUNCTION        : 9.0.0  CMSD_recv_response                             */
/*  CALLING SEQ.    : void CMSD_recv_response(void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御応答電文受信処理                                  */
/****************************************************************************/
void CMSD_recv_response(void)
{
    COM_SDT_arg_2_def   lcltime_c;
    COM_SDT_arg_3_def   lcltime_b;
    long long           lcltimestamp;
    short               s_result;
    char                wkbuf[64];

    /****************************************/
    /* 受信電文精査                         */
    /****************************************/
    /* 受信キューレコード長 */
    g_ctl_info.qrecv_info.qfile_len = g_ipcreq_head->control_data_length;
    /* MTI */
    memcpy((char *)&g_ctl_info.qrecv_info.mti,
        g_recvque->denbun_send_recv_info.mti_id,
        sizeof(g_recvque->denbun_send_recv_info.mti_id));
    /* 電文ログKEY */
    memcpy((char *)&g_ctl_info.denbun_log_key,
        (char *)&g_recvque->denbun_send_recv_info.denbun_log_key,
        sizeof(g_recvque->denbun_send_recv_info.denbun_log_key));
    /* GFP内部LCN */
    memcpy(g_ctl_info.req_gfp_lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    /* 受信コネクション論理ID */
    memcpy((char *)&g_ctl_info.connection_lid,
        (char *)&g_recvque->tushin_cntrl_info.line_info.recv_connect_id,
        sizeof(g_recvque->tushin_cntrl_info.line_info.recv_connect_id));

    /* 電文長 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_recvque->denbun_area.denbun_len, sizeof(g_recvque->denbun_area.denbun_len));
    g_ctl_info.qrecv_info.qdata_len = (unsigned short)atoi(wkbuf);

    /* インタフェース単位NW情報取得 */
    s_result = CMSD_read_netfile(
        DEF_CMSD_UNIT_INTERFACE,
        g_ctl_info.connection_lid.if_id,
        g_ctl_info.connection_lid.station_id);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* エラー出力ログ編集出力*/
        s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
        if (s_result != DEF_RET_OK) {
            CMSD_inbound_response(DEF_IPC_ERRCD_NG);
            return;
        }
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }

    /* MTI OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct));
    g_ctl_info.mti_off = (short)atoi(wkbuf)-1;

    /* MTI レングス */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_item_len, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_item_len));
    g_ctl_info.mti_len = (short)atoi(wkbuf);

    /* 電文データ OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct));
    g_ctl_info.msg_off = (short)atoi(wkbuf)-1;

    /* ビットマップ開始位置 */
    g_ctl_info.bitmap_off = (g_ctl_info.mti_off - g_ctl_info.msg_off) + g_ctl_info.mti_len;

    if (g_ctl_info.qrecv_info.mti[0] != ' ') {
        /****************************************/
        /* ビットマップ展開                     */
        /****************************************/
        g_format_info.iso8583_len = g_ctl_info.qrecv_info.qdata_len - g_ctl_info.bitmap_off;
        memset(g_iso8583_buf, 0x00, sizeof(g_iso8583_buf));
        memcpy(g_iso8583_buf,
            &g_recvque->denbun_area.denbun[g_ctl_info.bitmap_off], g_format_info.iso8583_len);
        s_result = CMSD_deploy_bitmap();
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_DEC_ERR, strlen(DEF_NERR_CMSD_BITMAP_DEC_ERR));
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_ctl_info.qrecv_info.mti, sizeof(g_ctl_info.qrecv_info.mti));
            CMIN_message_output ( DEF_EVT_BITMAP_EXPND_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_BITMAP_DEC_ERR
                                , "@L@C@3@H"
                                , g_ctl_info.req_gfp_lcn
                                , wkbuf
                                , g_iso8583_object.m_error_info.m_de_number
                                , (char *)g_ipcreq_head);
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            /* 障害電文通知要否 */
            if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
                /* 障害電文作成依頼処理 */
                CMSD_outbound_reject();
            }
            return;
        }

//      /****************************************/
//      /* 電文種別判定処理                     */
//      /****************************************/
//      s_result = NWM_MSJ(
//          g_ctl_info.qrecv_info.mti,
//          g_ffmt_buf,
//          (char *)&g_ctl_info.qrecv_info.ctlkind);
//      if (s_result != DEF_MSJ_NORMAL) {
//          memcpy(g_ctl_info.internal_err, DEF_NERR_SMK_RSP_SEISA_ERR, strlen(DEF_NERR_SMK_RSP_SEISA_ERR));
//          memset(wkbuf, 0x00, sizeof(wkbuf));
//          memcpy(wkbuf, g_ctl_info.qrecv_info.mti, sizeof(g_ctl_info.qrecv_info.mti));
//          CMIN_message_output ( DEF_EVT_MTI_HANTE_ERR
//                              , DEF_MSGTTKB_GYOM_ERR
//                              , DEF_NERR_SMK_RSP_SEISA_ERR
//                              , "@L@T@C"
//                              , g_ctl_info.req_gfp_lcn
//                              , (char *)&g_ctl_info.connection_lid
//                              , wkbuf);
//          /* エラー出力ログ編集出力*/
//          s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
//          if (s_result != DEF_RET_OK) {
//              CMSD_inbound_response(DEF_IPC_ERRCD_NG);
//              return;
//          }
//          /* キュー取出し通知応答処理 */
//          CMSD_inbound_response(DEF_IPC_ERRCD_OK);
//          /* 障害電文通知要否 */
//          if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
//              /* 障害電文作成依頼処理 */
//              CMSD_outbound_reject();
//          }
//          return;
//      }
    } else {
        /* 通知電文扱い */
        g_ctl_info.qrecv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_NTF_MSG;
        g_ctl_info.qrecv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.qrecv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_FAL;
        g_ctl_info.qrecv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    }

    if (g_ctl_info.qrecv_info.ctlkind.kinou_kbn == DEF_CTLFNC_NTF_MSG) {
        /* システム日時取得 */
        memset((char *)&lcltime_c, 0x00, sizeof(lcltime_c));
        memset((char *)&lcltime_b, 0x00, sizeof(lcltime_b));
        COM_SDT(2, &lcltime_c, &lcltime_b, &lcltimestamp);

        memset((char *)&g_gfmtl_ctl, 0x20, sizeof(g_gfmtl_ctl));
        g_gfmtl_ctl.pri_key.part_id[0] = '0';
        g_gfmtl_ctl.pri_key.part_id[1] = g_ctl_info.req_gfp_lcn[14];
        memcpy(g_gfmtl_ctl.pri_key.lcn_id, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
        g_gfmtl_ctl.s_h_kubun = DEF_S_H_KUBUN_SIMUKE;
        memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_RCV, strlen(DEF_TRHK_STS_RSP_RCV));
        memcpy((char *)&g_gfmtl_ctl.torihiki_info, (char *)&g_ctl_info.connection_lid, sizeof(g_ctl_info.connection_lid));
        memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qsend_info.ctlkind, sizeof(g_ctl_info.qrecv_info.ctlkind));
        memcpy(g_gfmtl_ctl.naibu_err_id, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
        memcpy(g_gfmtl_ctl.entry_timestamp, (char *)&lcltime_c, sizeof(g_gfmtl_ctl.entry_timestamp));

        s_result = CMSD_write_controlfile();
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            return;
        }
    } else {
        /****************************************/
        /* 要求応答マッチングキー生成           */
        /****************************************/
        memset(g_ctl_info.matching_key, 0x20, sizeof(g_ctl_info.matching_key));
        s_result = NWM_MKM(
            g_ctl_info.qrecv_info.mti,
            (queue_data_def *)&g_recvque->denbun_area,
            g_ffmt_buf,
            g_ctl_info.matching_key,
            sizeof(g_ctl_info.matching_key),
            &g_ctl_info.matching_key_len);
        if (s_result != DEF_MKM_NORMAL) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_SMK_RSP_MCH_ERR, strlen(DEF_NERR_CMSD_SMK_RSP_MCH_ERR));
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_SMK_RSP_MCH_ERR
                                , "@C@U"
                                , "NWM_MKM"
                                , s_result);
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            /* 障害電文通知要否 */
            if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
                /* 障害電文作成依頼処理 */
                CMSD_outbound_reject();
            }
            return;
        }

        /****************************************/
        /* 制御電文管理ファイル取得             */
        /****************************************/
        s_result = CMSD_read_controlfile(DEF_CMSD_KEY_MATCH);
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            if (s_result != DEF_CMSD_FILE_EOF) {
                memcpy(g_ctl_info.internal_err, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_SMK_RSP_MCH_ERR, strlen(DEF_NERR_CMSD_SMK_RSP_MCH_ERR));
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, g_ctl_info.matching_key, g_ctl_info.matching_key_len);
            CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_SMK_RSP_MCH_ERR
                                , "@L@T@C"
                                , g_ctl_info.req_gfp_lcn
                                , (char *)&g_ctl_info.connection_lid
                                , wkbuf);
            /* エラー出力ログ編集出力*/
            s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
            if (s_result != DEF_RET_OK) {
                CMSD_inbound_response(DEF_IPC_ERRCD_NG);
                return;
            }
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            /* 障害電文通知要否 */
            if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
                /* 障害電文作成依頼処理 */
                CMSD_outbound_reject();
            }
            return;
        }
        /* 応答受信済み */
        if (memcmp(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_RCV, strlen(DEF_TRHK_STS_RSP_RCV))==0) {
            CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_CTL_DISCARD
                                , "@L@T@C"
                                , g_ctl_info.req_gfp_lcn
                                , (char *)&g_ctl_info.connection_lid
                                , "ｵｳﾄｳｼｮﾘｽﾞﾐ");
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            return;
        }
        /* 応答タイムアウト */
        if (memcmp(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_TIMEOUT, strlen(DEF_TRHK_STS_RSP_TIMEOUT))==0) {
            CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_CMSD_CTL_DISCARD
                                , "@L@T@C"
                                , g_ctl_info.req_gfp_lcn
                                , (char *)&g_ctl_info.connection_lid
                                , "ﾀｲﾑｱｳﾄｼｮﾘｽﾞﾐ");
            /* キュー取出し通知応答処理 */
            CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            return;
        }
        memcpy(g_ctl_info.req_gfp_lcn, g_gfmtl_ctl.pri_key.lcn_id, sizeof(g_gfmtl_ctl.pri_key.lcn_id));

        /* 制御電文種別作成 */
        memcpy((char *)&g_ctl_info.qrecv_info.ctlkind, g_gfmtl_ctl.control_kind, sizeof(g_ctl_info.qrecv_info.ctlkind));
        g_ctl_info.qrecv_info.ctlkind.req_res_kbn = DEF_CTLMSG_RESPONSE;

        /****************************************/
        /* 制御電文管理ファイル更新             */
        /****************************************/
        memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_RCV, strlen(DEF_TRHK_STS_RSP_RCV));
        memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
        s_result = CMSD_update_controlfile();
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            CMIN_abend();
        }

        /****************************************/
        /* タイマーキャンセル                   */
        /****************************************/
        memcpy(g_ctl_info.timer_key, g_gfmtl_ctl.timer_info_key, sizeof(g_ctl_info.timer_key));
        g_ctl_info.timer_entry_kbn = g_gfmtl_ctl.timer_entry_kbn;
        s_result = CMSD_timer_cancel();
        if (s_result != DEF_RET_OK) {
            /* キュー取出し通知応答処理 */
//          CMSD_inbound_response(DEF_IPC_ERRCD_OK);
            CMSD_inbound_response(DEF_IPC_ERRCD_NG);
            return;
        }
    }

    /****************************************/
    /* サーバ制御電文通知処理               */
    /****************************************/
    switch(g_ctl_info.qrecv_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :
        if ((g_ctl_info.qrecv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST) ||
            (g_ctl_info.qrecv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT)) {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO_AUTO;
        } else {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        }
        break;
    case DEF_CTLFNC_KEY_EXCH    :
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        break;
    case DEF_CTLFNC_CUT_OVER    :
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        break;
    case DEF_CTLFNC_SAF         :
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        break;
    case DEF_CTLFNC_NTF_MSG     :
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        break;
    default:
        CMIN_abend();
    }

    g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_SIMUKE);

    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_SENDDATA      :
        break;
    case DEF_CMSD_STS_NORMAL        :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    case DEF_CMSD_STS_PATHSENDERROR :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_ERRORRESPONSE :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_IFERROR       :
        /* エラー出力ログ編集出力*/
        s_result = CMSD_errorlog(DEF_CMSD_ELG_RECVQUE, g_recvque->denbun_area.denbun, g_ctl_info.qrecv_info.qdata_len);
        if (s_result != DEF_RET_OK) {
            CMSD_inbound_response(DEF_IPC_ERRCD_NG);
            return;
        }
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    case DEF_CMSD_STS_FAULTTEXT     :
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* キュー取出し通知応答処理             */
    /****************************************/
    CMSD_inbound_response(DEF_IPC_ERRCD_OK);

    /* 正常完了 */
    if (g_ctl_info.server_sts != DEF_CMSD_STS_FAULTTEXT) {
        return;
    }

    /* 障害電文通知要否 */
    if (g_gfnwi_ctl.shori_kbn_info.syogai_tuuchi_need == '1') {
        /* 障害電文作成依頼処理 */
        CMSD_outbound_reject();
    }

} /* CMSD_recv_response */

/****************************************************************************/
/*  FUNCTION        : 10.0.0  CMSD_command_request                          */
/*  CALLING SEQ.    : void CMSD_command_request(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンド要求処理                                      */
/****************************************************************************/
void CMSD_command_request(void)
{
    COM_SDT_arg_2_def   lcltime_c;
    COM_SDT_arg_3_def   lcltime_b;
    long long           lcltimestamp;
    char                wkbuf[64];
    long                tranid = -1L;
    short               s_result;
    short               s_retry;

    /* コマンド精査 */
    if ((g_c502->command_info.connection_logical_name.site_name != g_myinfo.site_id) ||
        (g_c502->command_info.connection_logical_name.nw_name != g_myinfo.network_id) ||
        (memcmp(g_c502->command_info.connection_logical_name.group_name,
            g_myinfo.group_id, sizeof(g_myinfo.group_id))!=0)) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        CMIN_message_output ( DEF_EVT_REQ_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@C@H"
                            , "ｻｲﾄ/NW/ｸﾞﾙｰﾌﾟｼｷﾍﾞﾂ"
                            , (char *)g_ipcreq_head);
        /* エラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        return;
    }

    /* 受信コネクション論理ID */
    memcpy((char *)&g_ctl_info.connection_lid,
        (char *)&g_c502->command_info.connection_logical_name,
        sizeof(g_c502->command_info.connection_logical_name));

    /****************************************/
    /* 制御電文種別設定処理                 */
    /****************************************/
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_OPN, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_OPN_ABS, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_FORCE;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_OPN_UPD, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_UPDATEONLY;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_OPN_AUT_REQ, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_AUTO_REQUEST;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_OPN_AUT_CON, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_AUTO_CONNECT;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_CLS, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_CLS;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_CLS_ABS, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_CLS;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_FORCE;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_CNT_CLS_UPD, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_CNT_CLS;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_UPDATEONLY;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_ECH_SND, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_ECH_SND;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_KEY_EXC_REQ, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_KEY_EXCH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_KEY_EXC_REQ;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_KEY_EXC, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_KEY_EXCH;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_KEY_EXC;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_SAF_SND_START, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_SAF;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_SAF_SND_START;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else
    if (memcmp(g_c502->command_info.command_name, DEF_IPC_CMD_SAF_SND_END, 4)==0) {
        g_ctl_info.pserv_info.ctlkind.kinou_kbn    = DEF_CTLFNC_SAF;
        g_ctl_info.pserv_info.ctlkind.req_res_kbn  = DEF_CTLMSG_REQUEST;
        g_ctl_info.pserv_info.ctlkind.ctl_text_kbn = DEF_CTLTXT_SAF_SND_END;
        g_ctl_info.pserv_info.ctlkind.int_proc_kbn = DEF_CTLINT_NORMAL;
    } else {
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        CMIN_message_output ( DEF_EVT_REQ_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@C@H"
                            , "ｺﾏﾝﾄﾞｼｷﾍﾞﾂ"
                            , (char *)g_ipcreq_head);
        /* エラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);

        return;
    }

    /****************************************/
    /* 制御電文機能有効チェック             */
    /****************************************/
    switch(g_ctl_info.pserv_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :   /* 局状態・エコー制御   */
        if ((g_ctl_info.pserv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST) ||
            (g_ctl_info.pserv_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT)) {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO_AUTO;
        } else {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        }
        if ((g_ctl_info.pserv_info.ctlkind.ctl_text_kbn == DEF_CTLTXT_CNT_OPN) ||
            (g_ctl_info.pserv_info.ctlkind.ctl_text_kbn == DEF_CTLTXT_CNT_CLS)) {
            g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr;
        } else
        if (g_ctl_info.pserv_info.ctlkind.ctl_text_kbn == DEF_CTLTXT_ECH_SND) {
            g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr;
        } else {
            CMIN_abend();
        }
        break;
    case DEF_CTLFNC_KEY_EXCH    :   /* 鍵交換制御           */
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.key_cng_mng_lyr;
        break;
    case DEF_CTLFNC_CUT_OVER    :   /* カットオーバー制御   */
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.cut_over_mng_lyr;
        break;
    case DEF_CTLFNC_SAF         :   /* SAF送信制御          */
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.saf_send_mng_lyr;
        break;
    case DEF_CTLFNC_NTF_MSG     :   /* 通知電文制御         */
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        g_ctl_info.mng_layer = g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.connect_num_mng_lyr;
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* N/W情報取得処理                      */
    /****************************************/
    s_result = CMSD_read_netfile(
        DEF_CMSD_UNIT_INTERFACE,
        g_c502->command_info.connection_logical_name.interface_name,
        g_c502->command_info.connection_logical_name.station_name);
    if (s_result != DEF_RET_OK) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        CMIN_message_output ( DEF_EVT_REQ_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@C@H"
                            , "NWI READ ERROR"
                            , (char *)g_ipcreq_head);
        /* エラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        return;
    }

    /* MTI OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_start_lct));
    g_ctl_info.mti_off = (short)atoi(wkbuf)-1;

    /* MTI レングス */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.mti_item_len, sizeof(g_gfnwi_ctl.denbun_item_lct_info.mti_item_len));
    g_ctl_info.mti_len = (short)atoi(wkbuf);

    /* 電文データ OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    memcpy(wkbuf, g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct, sizeof(g_gfnwi_ctl.denbun_item_lct_info.denbun_start_lct));
    g_ctl_info.msg_off = (short)atoi(wkbuf)-1;

    /* ビットマップ開始位置 */
    g_ctl_info.bitmap_off = (g_ctl_info.mti_off - g_ctl_info.msg_off) + g_ctl_info.mti_len;

    /****************************************/
    /* サーバ制御電文作成要求処理           */
    /****************************************/
    for (s_retry = -1; s_retry < g_myinfo.send_retry_count; s_retry++) {
        /* BEGINTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_BEGIN, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(BT)"
                                , s_result);
            CMIN_abend();
        }
        
        g_ctl_info.server_sts = CMSD_server_edit_request(g_ctl_info.server_idx);
        
        if (g_ctl_info.server_sts == DEF_CMSD_STS_PATHSENDERROR) {
            /* ABORTTRANSACTION */
            s_result = COM_TMF(DEF_COM_TMF_ABORT, &tranid, DEF_GFPCVX70);
            if (s_result != DEF_RET_OK) {
                CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                    , DEF_MSGTTKB_GYOM_ERR
                                    , DEF_NERR_TMF_ERR
                                    , "@C@U"
                                    , "COM_TMF(AT)"
                                    , s_result);
                CMIN_abend();
            }
        } else {
            break;
        }
    }
    
    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_REQANDRES :   /* 正常(要求電文あり、応答電文あり) */
    case DEF_CMSD_STS_REQONLY   :   /* 正常(要求電文あり、応答電文なし) */
        break;

    case DEF_CMSD_STS_NOREQ     :   /* 正常(要求電文なし) */
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンド正常応答 */
        CMSD_command_response(DEF_IPC_ERRCD_OK);
        return;

    case DEF_CMSD_STS_PATHSENDERROR :
        memcpy(g_ctl_info.internal_err, DEF_NERR_PSEND_ERR_RE_OUT, strlen(DEF_NERR_PSEND_ERR_RE_OUT));
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        return;

    default:
        /* ABORTTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_ABORT, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(AT)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        return;
    }

    /****************************************/
    /* 制御電文管理ファイル登録             */
    /****************************************/
    /* システム日時取得 */
    memset((char *)&lcltime_c, 0x00, sizeof(lcltime_c));
    memset((char *)&lcltime_b, 0x00, sizeof(lcltime_b));
    COM_SDT(2, &lcltime_c, &lcltime_b, &lcltimestamp);

    memset((char *)&g_gfmtl_ctl, 0x20, sizeof(g_gfmtl_ctl));
    g_gfmtl_ctl.pri_key.part_id[0] = '0';
    g_gfmtl_ctl.pri_key.part_id[1] = g_ctl_info.req_gfp_lcn[14];
    memcpy(g_gfmtl_ctl.pri_key.lcn_id, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    g_gfmtl_ctl.s_h_kubun = DEF_S_H_KUBUN_SIMUKE;
    memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_REQ_SEND, strlen(DEF_TRHK_STS_REQ_SEND));
    memcpy((char *)&g_gfmtl_ctl.torihiki_info, (char *)&g_ctl_info.connection_lid, sizeof(g_ctl_info.connection_lid));
    memcpy(g_gfmtl_ctl.control_kind, (char *)&g_ctl_info.qsend_info.ctlkind, sizeof(g_ctl_info.qrecv_info.ctlkind));
    memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_gfmtl_ctl.naibu_err_id));
    memcpy(g_gfmtl_ctl.entry_timestamp, (char *)&lcltime_c, sizeof(g_gfmtl_ctl.entry_timestamp));

    s_result = CMSD_write_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_OFF);
        return;
    }

    /****************************************/
    /* ビットマップ組立処理                 */
    /****************************************/
    s_result = CMSD_create_bitmap();
    if (s_result != DEF_RET_OK) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_ctl_info.qsend_info.mti, sizeof(g_ctl_info.qsend_info.mti));
        CMIN_message_output ( DEF_EVT_BITMAP_ASMBL_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_ENC_ERR
                            , "@L@C@3@H"
                            , g_ctl_info.req_gfp_lcn
                            , wkbuf
                            , g_iso8583_object.m_error_info.m_de_number
                            , (char *)g_ipcreq_head);
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_ON);
        return;
    }

    /****************************************/
    /* 応答待ちタイマ設定処理               */
    /****************************************/
    if (g_ctl_info.server_sts == DEF_CMSD_STS_REQANDRES) {  /* 正常(要求電文あり、応答電文あり) */
        s_result = CMSD_timer_entry(g_ctl_info.qsend_info.ctlkind.ctl_text_kbn);
        if (s_result != DEF_RET_OK) {
            /* ENDTRANSACTION */
            s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
            if (s_result != DEF_RET_OK) {
                CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                    , DEF_MSGTTKB_GYOM_ERR
                                    , DEF_NERR_TMF_ERR
                                    , "@C@U"
                                    , "COM_TMF(ET)"
                                    , s_result);
                CMIN_abend();
            }
            /* コマンドエラー応答 */
            CMSD_command_response(DEF_IPC_ERRCD_NG);
            /* 電文送信不可通知 */
            CMSD_command_request_send_error(DEF_FLAG_ON);
            return;
        }
    }

    /****************************************/
    /* 送信キュー編集                       */
    /****************************************/
    memset((char *)g_sendque, 0x00, sizeof(db_gqnwq_def));
    g_ctl_info.qsend_info.qdata_len = (unsigned short)(g_ctl_info.bitmap_off + g_format_info.iso8583_len);

    /* キューファイル制御情報 */
    memset((char *)&g_sendque->cntrl_info, 0x20, sizeof(g_sendque->cntrl_info));
    memcpy(g_sendque->cntrl_info.shori_kubun, DEF_SHORI_KUBUN_SEND, strlen(DEF_SHORI_KUBUN_SEND));
    /* キューファイル電文送受信情報 */
    memset((char *)&g_sendque->denbun_send_recv_info, 0x20, sizeof(g_sendque->denbun_send_recv_info));
    memcpy(g_sendque->denbun_send_recv_info.nw_kubun,
        g_myinfo.nw_kbn, sizeof(g_myinfo.nw_kbn));
    memcpy(g_sendque->denbun_send_recv_info.mti_id,
        g_ctl_info.qsend_info.mti, sizeof(g_sendque->denbun_send_recv_info.mti_id));
    g_sendque->denbun_send_recv_info.send_denbun_shubetu = DEF_SEND_DENBUN_CTR_REQ;
    memcpy((char *)&g_sendque->denbun_send_recv_info.denbun_log_key,
        (char *)&g_ctl_info.denbun_log_key, sizeof(g_ctl_info.denbun_log_key));
    g_sendque->denbun_send_recv_info.denbun_fmt_kubun = DEF_DENBUN_FMT_8583;
    memset(g_sendque->denbun_send_recv_info.tushin_log_save_filename,
        0x20, sizeof(g_sendque->denbun_send_recv_info.tushin_log_save_filename));
    memset((char *)&g_sendque->denbun_send_recv_info.tushin_log_key,
        0x20, sizeof(g_sendque->denbun_send_recv_info.tushin_log_key));
    /* 通信制御情報 */
    memset((char *)&g_sendque->tushin_cntrl_info, 0x20, sizeof(g_sendque->tushin_cntrl_info));
    memcpy((char *)&g_sendque->tushin_cntrl_info.if_id,
        (char *)&g_r402->control_info.interface_name, sizeof(g_r402->control_info.interface_name));
    memcpy((char *)&g_sendque->tushin_cntrl_info.station_id,
        (char *)&g_r402->control_info.station_name, sizeof(g_r402->control_info.station_name));
    memcpy((char *)&g_sendque->tushin_cntrl_info.line_info.recv_connect_id,
        (char *)&g_ctl_info.connection_lid, sizeof(g_ctl_info.connection_lid));

    /* 電文長 */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", g_ctl_info.qsend_info.qdata_len);
    memcpy(g_sendque->denbun_area.denbun_len, wkbuf, sizeof(g_sendque->denbun_area.denbun_len));

    /* MTI OFFSET */
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%04d", (g_ctl_info.mti_off - g_ctl_info.msg_off + 1));
    memcpy(g_sendque->denbun_area.mti_start_lct, wkbuf, sizeof(g_sendque->denbun_area.mti_start_lct));

    /* 送受信電文 */
    memcpy(&g_sendque->denbun_area.denbun[0],
        g_r402->data_bu.message_text, g_ctl_info.bitmap_off);
    memcpy(&g_sendque->denbun_area.denbun[g_ctl_info.bitmap_off],
        g_iso8583_buf, g_format_info.iso8583_len);

    /* 電文ヘッダ電文長編集 */
    s_result = NWM_HDL(g_sendque->denbun_area.denbun, (short)(g_format_info.iso8583_len + g_ctl_info.mti_len));
    if (s_result != DEF_HDL_NORMAL) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@C@5"
                            , "NWM_HDL"
                            , s_result);
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_ON);
        return;
    }

    /****************************************/
    /* 要求応答マッチングキー生成           */
    /****************************************/
    s_result = NWM_MKM(
        g_ctl_info.qsend_info.mti,
        (queue_data_def *)&g_sendque->denbun_area,
        g_ffmt_buf,
        g_ctl_info.matching_key,
        sizeof(g_ctl_info.matching_key),
        &g_ctl_info.matching_key_len);
    if (s_result != DEF_MKM_NORMAL) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@C@5"
                            , "NWM_MKM"
                            , s_result);
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_ON);
        return;
    }

    /****************************************/
    /* 制御電文管理ファイル更新             */
    /****************************************/
    s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* ENDTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(ET)"
                                , s_result);
            CMIN_abend();
        }
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_ON);
        return;
    }
    memcpy(g_gfmtl_ctl.req_res_match_key, g_ctl_info.matching_key, g_ctl_info.matching_key_len);
    memcpy((char *)&g_gfmtl_ctl.timer_info_key, g_ctl_info.timer_key, sizeof(g_ctl_info.timer_key));
    g_gfmtl_ctl.timer_entry_kbn = g_ctl_info.timer_entry_kbn;
    s_result = CMSD_update_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        CMIN_abend();
    }

    /* ENDTRANSACTION */
    s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_TMF_ERR
                            , "@C@U"
                            , "COM_TMF(ET)"
                            , s_result);
        CMIN_abend();
    }

    /****************************************/
    /* 制御電文キュー登録処理               */
    /****************************************/
    g_ctl_info.qsend_info.qfile_len =
        (db_gqnwq_def_Size - sizeof(g_sendque->denbun_area.denbun)) + g_ctl_info.qsend_info.qdata_len;
    s_result = CMSD_outbound_text(DEF_CMSD_SV_CTL_OUT);
    if (s_result != DEF_CMSD_STS_NORMAL) {
        /* コマンドエラー応答 */
        CMSD_command_response(DEF_IPC_ERRCD_NG);
        /* 電文送信不可通知 */
        CMSD_command_request_send_error(DEF_FLAG_ON);
        return;
    }

    /* 正常応答 */
    CMSD_command_response(DEF_IPC_ERRCD_OK);

} /* CMSD_command_request */

/****************************************************************************/
/*  FUNCTION        : 11.0.0  CMSD_command_request_send_error               */
/*  CALLING SEQ.    : void CMSD_command_request_send_error(short)           */
/*  ARGUMENT        : 1.update_flg     (I)   ファイル更新要否               */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御要求電文送信エラー処理                            */
/****************************************************************************/
void CMSD_command_request_send_error(short update_flg)
{
    long            tranid = -1L;
    short           s_result;

    /* タイマーキャンセル処理 */
    s_result = CMSD_timer_cancel();
    if (s_result != DEF_RET_OK) {
        CMIN_abend();
    }

    /* BEGINTRANSACTION */
    s_result = COM_TMF(DEF_COM_TMF_BEGIN, &tranid, DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_TMF_ERR
                            , "@C@U"
                            , "COM_TMF(BT)"
                            , s_result);
        CMIN_abend();
    }

    if (update_flg == DEF_FLAG_ON) {
        /****************************************/
        /* 制御電文管理ファイル取得             */
        /****************************************/
        s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            /* ABORTTRANSACTION */
            s_result = COM_TMF(DEF_COM_TMF_ABORT, &tranid, DEF_GFPCVX70);
            if (s_result != DEF_RET_OK) {
                CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                    , DEF_MSGTTKB_GYOM_ERR
                                    , DEF_NERR_TMF_ERR
                                    , "@C@U"
                                    , "COM_TMF(AT)"
                                    , s_result);
                CMIN_abend();
            }
            /* エラー出力ログ編集出力*/
            CMSD_errorlog(DEF_CMSD_ELG_SENDQUE, g_sendque->denbun_area.denbun, g_ctl_info.qsend_info.qdata_len);
            return;
        }
        /****************************************/
        /* 制御電文管理ファイル更新             */
        /****************************************/
        memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_REQ_SENDERR, strlen(DEF_TRHK_STS_REQ_SENDERR));
        memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
        s_result = CMSD_update_controlfile();
        if (s_result != DEF_CMSD_FILE_NORMAL) {
            CMIN_abend();
        }
    }

    /****************************************/
    /* サーバ制御電文通知処理               */
    /****************************************/
    s_result = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_SIMUKE_ERROR);
    if (s_result != DEF_CMSD_STS_NORMAL) {
        /* ABORTTRANSACTION */
        s_result = COM_TMF(DEF_COM_TMF_ABORT, &tranid, DEF_GFPCVX70);
        if (s_result != DEF_RET_OK) {
            CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_TMF_ERR
                                , "@C@U"
                                , "COM_TMF(AT)"
                                , s_result);
            CMIN_abend();
        }
        /* エラー出力ログ編集出力*/
        CMSD_errorlog(DEF_CMSD_ELG_SENDQUE, g_sendque->denbun_area.denbun, g_ctl_info.qsend_info.qdata_len);
        return;
    }

    /* ENDTRANSACTION */
    s_result = COM_TMF(DEF_COM_TMF_END, &tranid, DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_TMF_ERR
                            , "@C@U"
                            , "COM_TMF(ET)"
                            , s_result);
        CMIN_abend();
    }

} /* CMSD_command_request_send_error */

/****************************************************************************/
/*  FUNCTION        : 12.0.0  CMSD_command_response                         */
/*  CALLING SEQ.    : void CMSD_command_response(short)                     */
/*  ARGUMENT        : 1.reply_kbn      (I)   リプライ区分                   */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンド応答処理                                      */
/****************************************************************************/
void CMSD_command_response(short reply_kbn)
{
    memcpy(g_r502->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP, strlen(DEF_IPC_IFCD_CMD_PRC_RSP));
    if (reply_kbn == 0) {
        g_r502->common_header.error_code = DEF_IPC_ERRCD_OK;
        memcpy(g_r302->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    } else {
        g_r502->common_header.error_code = DEF_IPC_ERRCD_NG;
        memcpy(g_r302->common_header.internal_error_code, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
    }
    g_r502->common_header.control_data_length = 0;

    CMIN_send_reply((char *)g_r502, sizeof(g_r502->common_header), 0);

} /* CMSD_command_response */


/****************************************************************************/
/*  FUNCTION        : 13.0.0  CMSD_request_send_error                       */
/*  CALLING SEQ.    : void CMSD_request_send_error(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御要求電文送信不可処理                              */
/****************************************************************************/
void CMSD_request_send_error(void)
{
    short           s_result;
    char            wkbuf[64];

    /* 送信キューレコード長 */
    g_ctl_info.qsend_info.qfile_len = g_ipcreq_head->control_data_length;
    /* MTI */
    memcpy((char *)&g_ctl_info.qsend_info.mti,
        g_recvque->denbun_send_recv_info.mti_id,
        sizeof(g_recvque->denbun_send_recv_info.mti_id));
    /* 電文ログKEY */
    memcpy((char *)&g_ctl_info.denbun_log_key,
        (char *)&g_recvque->denbun_send_recv_info.denbun_log_key,
        sizeof(g_recvque->denbun_send_recv_info.denbun_log_key));
    /* GFP内部LCN */
    memcpy(g_ctl_info.req_gfp_lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    /* コネクション論理ID */
    memcpy((char *)&g_ctl_info.connection_lid,
        (char *)&g_recvque->tushin_cntrl_info.line_info.recv_connect_id,
        sizeof(g_recvque->tushin_cntrl_info.line_info.recv_connect_id));
    /* 内部エラーコード */
    if (g_recvque->cntrl_info.err_code[0] == ' ') {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_CTL_SEND_ERR, strlen(DEF_NERR_CMSD_CTL_SEND_ERR));
    } else {
        memcpy(g_ctl_info.internal_err, g_recvque->cntrl_info.err_code, sizeof(g_recvque->cntrl_info.err_code));
    }

    /****************************************/
    /* 制御電文管理ファイル取得             */
    /****************************************/
    s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* 要求情報無し */
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
        CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@L@T@C"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , wkbuf);
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }
    memcpy((char *)&g_ctl_info.qsend_info.ctlkind, g_gfmtl_ctl.control_kind, sizeof(g_gfmtl_ctl.control_kind));

    /* 要求電文送信済み（応答電文待ち） */
    if (memcmp(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_REQ_SEND, strlen(DEF_TRHK_STS_REQ_SEND))!=0) {
        CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@L@T@C"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , "ｿｳｼﾝﾌｶ/ｵｳﾄｳｼｮﾘｽﾞﾐ");
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }

    /****************************************/
    /* 制御電文管理ファイル更新             */
    /****************************************/
    memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_REQ_SENDERR, strlen(DEF_TRHK_STS_REQ_SENDERR));
    memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
    s_result = CMSD_update_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        CMIN_abend();
    }

    /* タイマーキャンセル */
    memcpy(g_ctl_info.timer_key, g_gfmtl_ctl.timer_info_key, sizeof(g_ctl_info.timer_key));
    g_ctl_info.timer_entry_kbn = g_gfmtl_ctl.timer_entry_kbn;
    s_result = CMSD_timer_cancel();
    if (s_result != DEF_RET_OK) {
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }

    /****************************************/
    /* 制御電文振分                         */
    /****************************************/
    switch(g_ctl_info.qsend_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :
        if ((g_ctl_info.qsend_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST) ||
            (g_ctl_info.qsend_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT)) {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO_AUTO;
        } else {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        }
        break;
    case DEF_CTLFNC_KEY_EXCH    :
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        break;
    case DEF_CTLFNC_CUT_OVER    :
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        break;
    case DEF_CTLFNC_SAF         :
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        break;
    case DEF_CTLFNC_NTF_MSG     :
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* サーバ制御電文通知処理               */
    /****************************************/
    g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_SIMUKE_ERROR);

    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_NORMAL        :
        break;
    case DEF_CMSD_STS_PATHSENDERROR :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_ERRORRESPONSE :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* キュー取出し通知応答処理             */
    /****************************************/
    CMSD_inbound_response(DEF_IPC_ERRCD_OK);

} /* CMSD_request_send_error */


/****************************************************************************/
/*  FUNCTION        : 14.0.0  CMSD_response_send_error                      */
/*  CALLING SEQ.    : void CMSD_response_send_error(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御応答電文送信不可処理                              */
/****************************************************************************/
void CMSD_response_send_error(void)
{
    short           s_result;
    char            wkbuf[64];

    /* 送信キューレコード長 */
    g_ctl_info.qsend_info.qfile_len = g_ipcreq_head->control_data_length;
    /* MTI */
    memcpy((char *)&g_ctl_info.qsend_info.mti,
        g_recvque->denbun_send_recv_info.mti_id,
        sizeof(g_recvque->denbun_send_recv_info.mti_id));
    /* 電文ログKEY */
    memcpy((char *)&g_ctl_info.denbun_log_key,
        (char *)&g_recvque->denbun_send_recv_info.denbun_log_key,
        sizeof(g_recvque->denbun_send_recv_info.denbun_log_key));
    /* GFP内部LCN */
    memcpy(g_ctl_info.req_gfp_lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    /* コネクション論理ID */
    memcpy((char *)&g_ctl_info.connection_lid,
        (char *)&g_recvque->tushin_cntrl_info.line_info.recv_connect_id,
        sizeof(g_recvque->tushin_cntrl_info.line_info.recv_connect_id));
    /* 内部エラーコード */
    if (g_recvque->cntrl_info.err_code[0] == ' ') {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_CTL_SEND_ERR, strlen(DEF_NERR_CMSD_CTL_SEND_ERR));
    } else {
        memcpy(g_ctl_info.internal_err, g_recvque->cntrl_info.err_code, sizeof(g_recvque->cntrl_info.err_code));
    }

    /****************************************/
    /* 制御電文管理ファイル更新             */
    /****************************************/
    s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
        CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_CTL_SEND_ERR
                            , "@L@T@C"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , wkbuf);
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }
    memcpy(g_ctl_info.req_gfp_lcn, g_gfmtl_ctl.pri_key.lcn_id, sizeof(g_gfmtl_ctl.pri_key.lcn_id));
    memcpy((char *)&g_ctl_info.connection_lid, (char *)&g_gfmtl_ctl.torihiki_info, sizeof(g_gfmtl_ctl.torihiki_info));
    memcpy((char *)&g_ctl_info.qsend_info.ctlkind, g_gfmtl_ctl.control_kind, sizeof(g_gfmtl_ctl.control_kind));

    /****************************************/
    /* 制御電文管理ファイル更新             */
    /****************************************/
    memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_SENDERR, strlen(DEF_TRHK_STS_RSP_SENDERR));
    memcpy(g_gfmtl_ctl.naibu_err_id, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));
    s_result = CMSD_update_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        CMIN_abend();
    }

    /****************************************/
    /* 制御電文振分                         */
    /****************************************/
    switch(g_ctl_info.qsend_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :
        if ((g_ctl_info.qsend_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST) ||
            (g_ctl_info.qsend_info.ctlkind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT)) {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO_AUTO;
        } else {
            g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        }
        break;
    case DEF_CTLFNC_KEY_EXCH    :
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        break;
    case DEF_CTLFNC_CUT_OVER    :
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        break;
    case DEF_CTLFNC_SAF         :
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        break;
    case DEF_CTLFNC_NTF_MSG     :
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* サーバ制御電文通知処理               */
    /****************************************/
    g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_HISIMUKE_ERROR);

    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_NORMAL        :
        break;
    case DEF_CMSD_STS_PATHSENDERROR :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_ERRORRESPONSE :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* キュー取出し通知応答処理             */
    /****************************************/
    CMSD_inbound_response(DEF_IPC_ERRCD_OK);

} /* CMSD_response_send_error */

/****************************************************************************/
/*  FUNCTION        : 15.0.0  CMSD_response_timeout                         */
/*  CALLING SEQ.    : void CMSD_response_timeout(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御応答電文タイムアウト処理                          */
/****************************************************************************/
void CMSD_response_timeout(void)
{
    short           s_result;
    char            wkbuf[64];

    /* GFP内部LCN */
    memcpy(g_ctl_info.req_gfp_lcn,
        (char *)&g_recvque_detail->u_rcvgqnwq_data_ext.rcvgqnwq_data_rtn.gqnwq_data_rtn.gqnwq_data_rtn_info.biz_ctrl_info.transaction_id.gfp_lcn,
        sizeof(g_ctl_info.req_gfp_lcn));

    /* 内部エラーコード */
    memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_SMK_RSP_TIMEOUT, strlen(DEF_NERR_CMSD_SMK_RSP_TIMEOUT));

    /****************************************/
    /* 制御電文管理ファイル取得             */
    /****************************************/
    s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        /* 要求情報無し */
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
        CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_SMK_RSP_TIMEOUT
                            , "@L@T@C"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , wkbuf);
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }
    memcpy((char *)&g_ctl_info.connection_lid, (char *)&g_gfmtl_ctl.torihiki_info, sizeof(g_gfmtl_ctl.torihiki_info));
    memcpy((char *)&g_ctl_info.qsend_info.ctlkind, g_gfmtl_ctl.control_kind, sizeof(g_gfmtl_ctl.control_kind));

    /* 要求電文送信済み（応答電文待ち） */
    if (memcmp(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_REQ_SEND, strlen(DEF_TRHK_STS_REQ_SEND))!=0) {
        CMIN_message_output ( DEF_EVT_DENBUN_MATCH_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_SMK_RSP_TIMEOUT
                            , "@L@T@C"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , "ﾀｲﾑｱｳﾄ/ｵｳﾄｳｼｮﾘｽﾞﾐ");
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_OK);
        return;
    }

    /* メッセージ出力 */
    CMIN_message_output ( DEF_EVT_RSP_TIMEOUT_DETECT
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_CMSD_SMK_RSP_TIMEOUT
                        , "@L@T@C@C"
                        , g_ctl_info.req_gfp_lcn
                        , (char *)&g_ctl_info.connection_lid
                        , ""
                        , "");

    /****************************************/
    /* 制御電文管理ファイル更新             */
    /****************************************/
    memcpy(g_gfmtl_ctl.torihiki_sts, DEF_TRHK_STS_RSP_TIMEOUT, strlen(DEF_TRHK_STS_RSP_TIMEOUT));
    memcpy(g_gfmtl_ctl.naibu_err_id, DEF_NERR_CMSD_SMK_RSP_TIMEOUT, strlen(DEF_NERR_CMSD_SMK_RSP_TIMEOUT));
    s_result = CMSD_update_controlfile();
    if (s_result != DEF_CMSD_FILE_NORMAL) {
        CMIN_abend();
    }

    /****************************************/
    /* 制御電文振分                         */
    /****************************************/
    switch(g_ctl_info.qsend_info.ctlkind.kinou_kbn) {
    case DEF_CTLFNC_CNT_STS_ECH :
        g_ctl_info.server_idx = DEF_CMSD_SV_SIGN_ECHO;
        break;
    case DEF_CTLFNC_KEY_EXCH    :
        g_ctl_info.server_idx = DEF_CMSD_SV_KEYEXC;
        break;
    case DEF_CTLFNC_CUT_OVER    :
        g_ctl_info.server_idx = DEF_CMSD_SV_CUTOVER;
        break;
    case DEF_CTLFNC_SAF         :
        g_ctl_info.server_idx = DEF_CMSD_SV_SAF;
        break;
    case DEF_CTLFNC_NTF_MSG     :
        g_ctl_info.server_idx = DEF_CMSD_SV_NOTICE;
        break;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* サーバ制御電文通知処理               */
    /****************************************/
    g_ctl_info.server_sts = CMSD_server_recv_notice(g_ctl_info.server_idx, DEF_CTLREQ_TIMEOUT);

    switch(g_ctl_info.server_sts) {
    case DEF_CMSD_STS_NORMAL        :
        break;
    case DEF_CMSD_STS_PATHSENDERROR :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    case DEF_CMSD_STS_ERRORRESPONSE :
        /* キュー取出し通知応答処理 */
        CMSD_inbound_response(DEF_IPC_ERRCD_NG);
        return;
    default:
        CMIN_abend();
    }

    /****************************************/
    /* キュー取出し通知応答処理             */
    /****************************************/
    CMSD_inbound_response(DEF_IPC_ERRCD_OK);

} /* CMSD_response_timeout */


/****************************************************************************/
/*  FUNCTION        : 16.0.0  CMSD_server_recv_notice                       */
/*  CALLING SEQ.    : short CMSD_server_recv_notice(short, char*)           */
/*  ARGUMENT        : 1.server_no      (I)   サーバ管理テーブル番号         */
/*  ARGUMENT        : 2.req_kind       (I)   要求区分                       */
/*  RETURN CODE     : 処理ステータス                                        */
/*  DESCRIPTION     : サーバ制御電文通知処理                                */
/****************************************************************************/
short CMSD_server_recv_notice(short server_no, char *req_kind)
{
    short           ffmt_off;
    short           data_len;
    short           s_result;

    /****************************************/
    /* NW電文受信通知要求編集処理           */
    /****************************************/
    g_svrtbl[server_no].send_len = 0;
    g_svrtbl[server_no].recv_len = 0;
    memset(g_serv_buf, 0x00, sizeof(g_serv_buf));
    memset((char *)g_c401, 0x20, sizeof(cr401_def));
    memcpy(g_c401->common_header.interface_code, DEF_IPC_IFCD_NW_MSG_REQ, strlen(DEF_IPC_IFCD_NW_MSG_REQ));
    memcpy(g_c401->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    g_c401->common_header.error_code = DEF_IPC_ERRCD_OK;

    memcpy(g_c401->control_info.request_kind, req_kind, sizeof(g_c401->control_info.request_kind));
    memcpy((char *)&g_c401->control_info.connection_lid,
        (char *)&g_ctl_info.connection_lid, sizeof(g_c401->control_info.connection_lid));
    memcpy((char *)&g_c401->control_info.denbun_log_key,
        (char *)&g_ctl_info.denbun_log_key, sizeof(g_c401->control_info.denbun_log_key));
    if (memcmp(req_kind, DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT))!=0) {
        memcpy((char *)&g_c401->control_info.interface_name,
            (char *)&g_recvque->tushin_cntrl_info.if_id, sizeof(g_recvque->tushin_cntrl_info.if_id));
        memcpy((char *)&g_c401->control_info.station_name,
            (char *)&g_recvque->tushin_cntrl_info.station_id, sizeof(g_recvque->tushin_cntrl_info.station_id));
    }
    if ((memcmp(req_kind, DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE))==0)) {
        memcpy((char *)&g_c401->control_info.control_kind,
            (char *)&g_ctl_info.qrecv_info.ctlkind, sizeof(g_c401->control_info.control_kind));
        memcpy(g_c401->control_info.mti, g_ctl_info.qrecv_info.mti, sizeof(g_c401->control_info.mti));
        memcpy(g_c401->control_info.req_gfp_lcn, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    }
    if ((memcmp(req_kind, DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_SIMUKE_ERROR, strlen(DEF_CTLREQ_SIMUKE_ERROR))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_HISIMUKE_ERROR, strlen(DEF_CTLREQ_HISIMUKE_ERROR))==0)) {
        memcpy((char *)&g_c401->control_info.control_kind,
            (char *)&g_ctl_info.qsend_info.ctlkind, sizeof(g_c401->control_info.control_kind));
        memcpy(g_c401->control_info.mti, g_ctl_info.qsend_info.mti, sizeof(g_c401->control_info.mti));
        memcpy(g_c401->control_info.req_gfp_lcn, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    }
    if ((memcmp(req_kind, DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_SIMUKE_ERROR, strlen(DEF_CTLREQ_SIMUKE_ERROR))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_HISIMUKE_ERROR, strlen(DEF_CTLREQ_HISIMUKE_ERROR))==0)) {
        memcpy(g_c401->control_info.send_naibu_err_code,
            g_ctl_info.internal_err, sizeof(g_c401->control_info.send_naibu_err_code));
    }
    if ((memcmp(req_kind, DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE))==0) ||
        (memcmp(req_kind, DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE))==0)) {
        memcpy((char *)&g_c401->control_info.denbun_len,
            (char *)&g_recvque->denbun_area.denbun_len, sizeof(g_recvque->denbun_area.denbun_len));
        memcpy(&g_c401->data_bu.message_text[0], g_recvque->denbun_area.denbun, g_ctl_info.bitmap_off);
        memcpy(&g_c401->data_bu.message_text[g_ctl_info.bitmap_off], g_ffmt_buf, g_format_info.ffmt_len);
        data_len = g_ctl_info.bitmap_off + (short)g_format_info.ffmt_len;
        g_c401->common_header.control_data_length = sizeof(g_c401->control_info) + data_len;
        g_svrtbl[server_no].send_len = sizeof(g_c401->common_header) + g_c401->common_header.control_data_length;
    } else {
        g_c401->common_header.control_data_length = sizeof(g_c401->control_info);
        g_svrtbl[server_no].send_len = sizeof(g_c401->common_header) + sizeof(g_c401->control_info);
    }

    /* PATHSEND */
    s_result = CMSD_pathsend(server_no, g_serv_buf, 0);
    if (s_result != DEF_RET_OK) {
        return DEF_CMSD_STS_PATHSENDERROR;
    }

    /****************************************/
    /* NW電文受信通知応答処理               */
    /****************************************/
    if (memcmp(g_r401->common_header.interface_code, DEF_IPC_IFCD_NW_MSG_RSP, strlen(DEF_IPC_IFCD_NW_MSG_RSP))!=0) {
        /* エラーメッセージ出力 */
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ｲﾝﾀﾌｪｰｽｺｰﾄﾞｴﾗｰ"
                            , (char *)&g_r401->common_header);
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_IFERROR;
    }
//  if (g_svrtbl[server_no].recv_len < (sizeof(g_r401->common_header)+sizeof(g_r401->control_info))) {
    if (g_svrtbl[server_no].recv_len < sizeof(g_r401->common_header)) {
        /* エラーメッセージ出力 */
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ｲﾝﾀﾌｪｰｽｻｲｽﾞｴﾗｰ"
                            , (char *)&g_r401->common_header);
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_IFERROR;
    }

    switch (g_r401->common_header.error_code) {
    case DEF_IPC_ERRCD_OK       :       /* 正常応答 */
        memcpy(g_ctl_info.internal_err, g_r401->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        if (memcmp(g_r401->control_info.response_kind, DEF_CTLRSP_SEND, strlen(DEF_CTLRSP_SEND))==0) {
            /* 送信電文有り */
            memcpy((char *)&g_ctl_info.qsend_info.mti,
                g_r401->control_info.mti, sizeof(g_r401->control_info.mti));
            memcpy((char *)&g_ctl_info.qsend_info.ctlkind,
                (char *)&g_r401->control_info.control_kind, sizeof(g_r401->control_info.control_kind));

            /* 固定フォーマット */
            ffmt_off = sizeof(g_r401->common_header) + sizeof(g_r401->control_info) + g_ctl_info.bitmap_off;
            g_format_info.ffmt_len = g_svrtbl[server_no].recv_len - ffmt_off;
            memset(g_ffmt_buf, 0x00, sizeof(g_ffmt_buf));
            memcpy(g_ffmt_buf, &g_serv_buf[ffmt_off], g_format_info.ffmt_len);

            return DEF_CMSD_STS_SENDDATA;
        } else {
            /* 送信電文無し */
            return DEF_CMSD_STS_NORMAL;
        }

    case DEF_IPC_ERRCD_FAILMSG  :       /* 障害電文 */
        memcpy(g_ctl_info.internal_err, g_r401->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        return DEF_CMSD_STS_FAULTTEXT;

    case DEF_IPC_ERRCD_NG       :       /* 拒否(異常)応答 */
        memcpy(g_ctl_info.internal_err, g_r401->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        return DEF_CMSD_STS_ERRORRESPONSE;

    default:                            /* 不明応答 */
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ﾌﾒｲｴﾗｰｺｰﾄﾞ"
                            , (char *)&g_r401->common_header);

        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_ERRORRESPONSE;
    }

} /* CMSD_server_recv_notice */


/****************************************************************************/
/*  FUNCTION        : 17.0.0  CMSD_server_edit_request                      */
/*  CALLING SEQ.    : short CMSD_server_edit_request(short)                 */
/*  ARGUMENT        : 1.server_no      (I)   サーバ管理テーブル番号         */
/*  RETURN CODE     : 処理ステータス                                        */
/*  DESCRIPTION     : サーバ制御電文作成要求処理                            */
/****************************************************************************/
short CMSD_server_edit_request(short server_no)
{
    short           ffmt_off;
    short           s_result;

    /****************************************/
    /* 制御電文作成要求編集処理             */
    /****************************************/
    g_svrtbl[server_no].send_len = 0;
    g_svrtbl[server_no].recv_len = 0;
    memset(g_serv_buf, 0x00, sizeof(g_serv_buf));
    memset((char *)g_c402, 0x20, sizeof(cr402_def));
    memcpy(g_c402->common_header.interface_code, DEF_IPC_IFCD_CTRL_MSG_REQ, strlen(DEF_IPC_IFCD_CTRL_MSG_REQ));
    memcpy(g_c402->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    g_c402->common_header.error_code = DEF_IPC_ERRCD_OK;
    g_c402->common_header.control_data_length = sizeof(g_c402->control_info);

    memcpy((char *)&g_c402->control_info.control_kind,
        (char *)&g_ctl_info.pserv_info.ctlkind, sizeof(g_c402->control_info.control_kind));
    memcpy((char *)&g_c402->control_info.connection_lid,
        (char *)&g_ctl_info.connection_lid, sizeof(g_c402->control_info.connection_lid));
    memcpy((char *)&g_c402->control_info.interface_name,
        (char *)&g_c502->command_info.interface_ext_name, sizeof(g_c502->command_info.interface_ext_name));
    memcpy((char *)&g_c402->control_info.station_name,
        (char *)&g_c502->command_info.station_ext_name, sizeof(g_c502->command_info.station_ext_name));

    /* PATHSEND */
    g_svrtbl[server_no].send_len = sizeof(g_c402->common_header) + sizeof(g_c402->control_info);
    s_result = CMSD_pathsend(server_no, g_serv_buf, 0);
    if (s_result != DEF_RET_OK) {
        return DEF_CMSD_STS_PATHSENDERROR;
    }

    /****************************************/
    /* 制御電文作成応答処理                 */
    /****************************************/
    if (memcmp(g_r402->common_header.interface_code, DEF_IPC_IFCD_CTRL_MSG_RSP, strlen(DEF_IPC_IFCD_CTRL_MSG_RSP))!=0) {
        /* エラーメッセージ出力 */
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ｲﾝﾀﾌｪｰｽｺｰﾄﾞｴﾗｰ"
                            , (char *)&g_r402->common_header);

        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_IFERROR;
    }
//  if (g_svrtbl[server_no].recv_len < (sizeof(g_r402->common_header)+sizeof(g_r402->control_info))) {
    if (g_svrtbl[server_no].recv_len < sizeof(g_r402->common_header)) {
        /* エラーメッセージ出力 */
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ｲﾝﾀﾌｪｰｽｻｲｽﾞｴﾗｰ"
                            , (char *)&g_r402->common_header);
        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_IFERROR;
    }

    switch (g_r402->common_header.error_code) {
    case DEF_IPC_ERRCD_OK       :
        memcpy(g_ctl_info.internal_err, g_r402->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        if (memcmp(g_r402->control_info.response_kind, DEF_CTLRSP_SEND, strlen(DEF_CTLRSP_SEND))==0) {
            /* 送信電文有り */
            memcpy((char *)&g_ctl_info.qsend_info.mti,
                g_r402->control_info.mti, sizeof(g_r402->control_info.mti));
            memcpy((char *)&g_ctl_info.qsend_info.ctlkind,
                (char *)&g_r402->control_info.control_kind, sizeof(g_r402->control_info.control_kind));
            /* 電文ログKEY */
            memcpy((char *)&g_ctl_info.denbun_log_key,
                (char *)&g_r402->control_info.denbun_log_key, sizeof(g_r402->control_info.denbun_log_key));
            /* GFP内部LCN */
            memcpy(g_ctl_info.req_gfp_lcn,
                (char *)&g_r402->control_info.denbun_log_key.tran_id.gfp_lcn,
                sizeof(g_r402->control_info.denbun_log_key.tran_id.gfp_lcn));
            /* コネクション論理ID */
            memcpy((char *)&g_ctl_info.connection_lid,
                (char *)&g_r402->control_info.connection_lid, sizeof(g_r402->control_info.connection_lid));
            /* 固定フォーマット */
            ffmt_off = sizeof(g_r402->common_header) + sizeof(g_r402->control_info) + g_ctl_info.bitmap_off;
            g_format_info.ffmt_len = g_svrtbl[server_no].recv_len - ffmt_off;
            memset(g_ffmt_buf, 0x00, sizeof(g_ffmt_buf));
            memcpy(g_ffmt_buf, &g_serv_buf[ffmt_off], g_format_info.ffmt_len);

            return DEF_CMSD_STS_REQANDRES;
        } else {
            /* 送信電文無し */
            return DEF_CMSD_STS_NOREQ;
        }

    case DEF_IPC_ERRCD_NG       :
        memcpy(g_ctl_info.internal_err, g_r402->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        return DEF_CMSD_STS_ERRORRESPONSE;

    default:
        CMIN_message_output ( DEF_EVT_RSP_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@L@S@C@H"
                            , g_ctl_info.req_gfp_lcn
                            , g_svrtbl[server_no].scname
                            , "ﾌﾒｲｴﾗｰｺｰﾄﾞ"
                            , (char *)&g_r402->common_header);

        memcpy(g_ctl_info.internal_err, DEF_NERR_IPC_SEISA_ERR, strlen(DEF_NERR_IPC_SEISA_ERR));
        return DEF_CMSD_STS_ERRORRESPONSE;
    }

} /* CMSD_server_edit_request */

/****************************************************************************/
/*  FUNCTION        : 18.0.0  CMSD_outbound_text                            */
/*  CALLING SEQ.    : short CMSD_outbound_text(short)                       */
/*  ARGUMENT        : 1.server_no      (I)   サーバ管理テーブル番号         */
/*  RETURN CODE     : 処理ステータス                                        */
/*  DESCRIPTION     : 制御電文キュー登録処理                                */
/****************************************************************************/
short CMSD_outbound_text(short server_no)
{
    short           s_result;

    /****************************************/
    /* キュー登録要求処理                   */
    /****************************************/
    /* 共通ヘッダ */
    memcpy(g_c301->common_header.interface_code, DEF_IPC_IFCD_Q_RGST_REQ, strlen(DEF_IPC_IFCD_Q_RGST_REQ));
    memcpy(g_c301->common_header.internal_error_code, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    g_c301->common_header.error_code = DEF_IPC_ERRCD_OK;
    g_c301->common_header.control_data_length =
        (sizeof(g_c301->que_rgs_info) - sizeof(g_c301->que_rgs_info.msg_data)) + g_ctl_info.qsend_info.qfile_len;
    g_ctl_info.qsend_info.ipc_len =
        sizeof(g_c301->common_header) + g_c301->common_header.control_data_length;

    /* キュー登録情報部*/
    memset((char *)&g_c301->que_rgs_info,
        0x20, (sizeof(g_c301->que_rgs_info) - sizeof(g_c301->que_rgs_info.msg_data)));
    memcpy((char *)&g_c301->que_rgs_info.gfp_lcn,
        (char *)&g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_c301->que_rgs_info.gfp_lcn));
    g_c301->que_rgs_info.serverclass_info.site_name = g_myinfo.site_id;
    memcpy((char *)g_c301->que_rgs_info.serverclass_info.serverclass_id.serverclass_name,
        (char *)g_myinfo.serverclass_name, sizeof(g_c301->que_rgs_info.serverclass_info.serverclass_id.serverclass_name));
    memcpy((char *)g_c301->que_rgs_info.serverclass_info.serverclass_id.serverclass_num,
        (char *)g_myinfo.serverclass_no, sizeof(g_c301->que_rgs_info.serverclass_info.serverclass_id.serverclass_num));

    /* PATHSEND */
    g_svrtbl[server_no].send_len = (short)g_ctl_info.qsend_info.ipc_len;
    s_result = CMSD_pathsend(server_no, g_send_buf, (short)g_myinfo.send_retry_count);
    if (s_result != DEF_RET_OK) {
        return DEF_CMSD_STS_ERRORRESPONSE;
    }

    if (g_r301->common_header.error_code == DEF_IPC_ERRCD_OK) {
        return DEF_CMSD_STS_NORMAL;
    } else {
        memcpy(g_ctl_info.internal_err, g_r301->common_header.internal_error_code, sizeof(g_ctl_info.internal_err));
        CMIN_message_output ( DEF_EVT_ERR_REP_RCV
                            , DEF_MSGTTKB_GYOM_ERR
                            , g_ctl_info.internal_err
                            , "@L@T@S"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , g_svrtbl[server_no].scname);
        return DEF_CMSD_STS_ERRORRESPONSE;
    }

} /* CMSD_outbound_text */

/****************************************************************************/
/*  FUNCTION        : 19.0.0  CMSD_outbound_reject                          */
/*  CALLING SEQ.    : void CMSD_outbound_reject(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 障害電文作成依頼キュー登録処理                        */
/****************************************************************************/
void CMSD_outbound_reject(void)
{
    short           s_result;

    /****************************************/
    /* 障害電文通知作成依頼処理             */
    /****************************************/
    memset(g_send_buf, 0x00, sizeof(g_send_buf));
    memcpy((char *)g_sendque, (char *)g_recvque, g_ctl_info.qrecv_info.qfile_len);
    g_ctl_info.qsend_info.qfile_len = g_ctl_info.qrecv_info.qfile_len;
    memcpy(g_sendque->cntrl_info.shori_kubun, DEF_SHORI_KUBUN_FAULTTEXT, strlen(DEF_SHORI_KUBUN_FAULTTEXT));
    memcpy(g_sendque->cntrl_info.err_code, g_ctl_info.internal_err, sizeof(g_ctl_info.internal_err));

    /* 業務電文キュー登録 */
    s_result = CMSD_outbound_text(DEF_CMSD_SV_APL_IN);
    if (s_result != DEF_CMSD_STS_NORMAL) {
        /* エラー出力ログ編集出力*/
        CMSD_errorlog(DEF_CMSD_ELG_SENDQUE, g_sendque->denbun_area.denbun, g_ctl_info.qsend_info.qdata_len);
    }

} /* CMSD_outbound_reject */

/****************************************************************************/
/*  FUNCTION        : 20.0.0  CMSD_timer_entry                              */
/*  CALLING SEQ.    : short CMSD_timer_entry(char)                          */
/*  ARGUMENT        : 1.text_kbn       (I)   電文区分                       */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : タイマー発行処理                                      */
/****************************************************************************/
short CMSD_timer_entry(char text_kbn)
{
    COM_SDT_arg_2_def       lcltime_c;
    COM_SDT_arg_3_def       lcltime_b;
    long long               lcltimestamp;
    long                    timer_val;
    char                    timeout_c[20+1];
    char                    l_serv_buf[sizeof(timer_issue_rq_def)+1];
    timer_issue_rq_def      *timer_issue_req;
    timer_issue_resp_def    *timer_issue_res;
    short                   s_result;
    char                    wkbuf1[64];

    memset(l_serv_buf, 0x00, sizeof(l_serv_buf));
    timer_issue_req = (timer_issue_rq_def *)l_serv_buf;
    timer_issue_res = (timer_issue_resp_def *)l_serv_buf;

    /* タイマー情報(N/W情報ファイル) */
    memset(wkbuf1, 0x00, sizeof(wkbuf1));
    switch (text_kbn) {
    case DEF_CTLTXT_CNT_OPN         :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.open_res_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.open_res_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_CNT_CLS         :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.close_res_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.close_res_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_ECH_SND         :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.echo_test_res_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.echo_test_res_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_KEY_EXC_REQ     :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.key_cng_req_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.key_cng_req_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_KEY_EXC         :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.key_cng_res_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.key_cng_res_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_CUT_START       :
    case DEF_CTLTXT_CUT_END         :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.cut_over_res_wait_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.cut_over_res_wait_tmr));
        timer_val = atol(wkbuf1);
        break;
    case DEF_CTLTXT_SAF_SND_START   :
    case DEF_CTLTXT_SAF_SND_END     :
        memcpy(wkbuf1, g_gfnwi_ctl.cntrl_denbun_tmr_info.saf_start_end_res_tmr,
            sizeof(g_gfnwi_ctl.cntrl_denbun_tmr_info.saf_start_end_res_tmr));
        timer_val = atol(wkbuf1);
        break;
    default:
        CMIN_abend();
    }

    /* システム日時取得 */
    memset((char *)&lcltime_c, 0x00, sizeof(lcltime_c));
    memset((char *)&lcltime_b, 0x00, sizeof(lcltime_b));
    COM_SDT(2, &lcltime_c, &lcltime_b, &lcltimestamp);

    /* タイマー満了日時算出 */
    memset(timeout_c, 0x00, sizeof(timeout_c));
    COM_DTC((char *)&lcltime_c, timer_val, timeout_c);

    /* タイマー発行要求 */
    memcpy(timer_issue_req->unique_msg_id, DEF_IPC_IFCD_TIMER_ADD_REQ, strlen(DEF_IPC_IFCD_TIMER_ADD_REQ));
    memcpy(timer_issue_req->process_result_code, "0000", sizeof(timer_issue_req->process_result_code));
    memset((char *)&timer_issue_req->transaction_id, 0x20, sizeof(timer_issue_req->transaction_id));
    memcpy((char *)&timer_issue_req->transaction_id, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    memcpy(timer_issue_req->timer_exp_yymmddhhmmsscc,
        timeout_c, sizeof(timer_issue_req->timer_exp_yymmddhhmmsscc));
    memset(timer_issue_req->queue_pathmon_primary, 0x20, sizeof(timer_issue_req->queue_pathmon_primary));
    if (g_svrtbl[DEF_CMSD_SV_APL_CTL].domainname[0] != ' ') {
        memcpy(timer_issue_req->queue_pathmon_primary,
            g_svrtbl[DEF_CMSD_SV_APL_CTL].domainname, strlen(g_svrtbl[DEF_CMSD_SV_APL_CTL].domainname));
    } else {
        memcpy(timer_issue_req->queue_pathmon_primary,
            g_svrtbl[DEF_CMSD_SV_APL_CTL].monname, strlen(g_svrtbl[DEF_CMSD_SV_APL_CTL].monname));
    }
    memset((char *)&timer_issue_req->queue_server_primary, 0x20, sizeof(timer_issue_req->queue_server_primary));
    memcpy((char *)&timer_issue_req->queue_server_primary,
        g_svrtbl[DEF_CMSD_SV_APL_CTL].scname, strlen(g_svrtbl[DEF_CMSD_SV_APL_CTL].scname));
    memset(timer_issue_req->queue_pathmon_backup, 0x20, sizeof(timer_issue_req->queue_pathmon_backup));
    memset(timer_issue_req->queue_server_backup, 0x20, sizeof(timer_issue_req->queue_server_backup));

    /* PATHSEND */
    g_ctl_info.timer_entry_kbn = DEF_TIMER_NOENTRY;
    g_svrtbl[DEF_CMSD_SV_TIMER].send_len = timer_issue_rq_def_Size;
    s_result = CMSD_pathsend(DEF_CMSD_SV_TIMER, l_serv_buf, 0);
    if (s_result != DEF_RET_OK) {
        g_svrtbl[DEF_CMSD_SV_TIMER_B].send_len = timer_cancel_rq_def_Size;
        s_result = CMSD_pathsend(DEF_CMSD_SV_TIMER_B, l_serv_buf, 0);
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_TIMER_ERR, strlen(DEF_NERR_CMSD_TIMER_ERR));
            return DEF_RET_NG;
        }
        g_ctl_info.timer_entry_kbn = DEF_TIMER_ENTRY_BACKUP;
    } else {
        g_ctl_info.timer_entry_kbn = DEF_TIMER_ENTRY_PRIMARY;
    }

    /* 処理結果判定 */
    if (memcmp(timer_issue_res->process_result_code, "0000", sizeof(timer_issue_res->process_result_code)) != 0) {
        CMIN_message_output ( DEF_EVT_ERR_REP_RCV
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_TIMER_ERR
                            , "@L@T@S"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , g_svrtbl[DEF_CMSD_SV_TIMER].scname);
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_TIMER_ERR, strlen(DEF_NERR_CMSD_TIMER_ERR));
        return DEF_RET_NG;
    }
    memset(g_ctl_info.timer_key, 0x00, sizeof(g_ctl_info.timer_key));
    memcpy(g_ctl_info.timer_key, timer_issue_res->timer_ctrl_addr1,
        (sizeof(timer_issue_res->timer_ctrl_addr1)+
         sizeof(timer_issue_res->timer_ctrl_cpu1)+
         sizeof(timer_issue_res->timer_ctrl_pin1)+
         sizeof(timer_issue_res->timer_ctrl_addr2)+
         sizeof(timer_issue_res->timer_ctrl_cpu2)+
         sizeof(timer_issue_res->timer_ctrl_pin2)));

    return DEF_RET_OK;
} /* CMSD_timer_entry */

/****************************************************************************/
/*  FUNCTION        : 21.0.0  CMSD_timer_cancel                             */
/*  CALLING SEQ.    : short CMSD_timer_cancel(void)                         */
/*  ARGUMENT        : 1.text_kbn       (I)   電文区分                       */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : タイマーキャンセル処理                                */
/****************************************************************************/
short CMSD_timer_cancel(void)
{
    char                    l_serv_buf[sizeof(timer_issue_rq_def)+1];
    timer_cancel_rq_def     *timer_cancel_req;
    timer_cancel_resp_def   *timer_cancel_res;
    short                   s_result;

    if (g_ctl_info.timer_entry_kbn == DEF_TIMER_NOENTRY) {
        return DEF_RET_OK;
    }
    
    memset(l_serv_buf, 0x00, sizeof(l_serv_buf));
    timer_cancel_req = (timer_cancel_rq_def *)l_serv_buf;
    timer_cancel_res = (timer_cancel_resp_def *)l_serv_buf;

    /* タイマーキャンセル要求 */
    memset(l_serv_buf, 0x00, sizeof(l_serv_buf));
    memcpy(timer_cancel_req->unique_msg_id, DEF_IPC_IFCD_TIMER_CAN_REQ, strlen(DEF_IPC_IFCD_TIMER_CAN_REQ));
    memcpy(timer_cancel_req->process_result_code, "0000", sizeof(timer_cancel_req->process_result_code));
    memset((char *)&timer_cancel_req->transaction_id, 0x20, sizeof(timer_cancel_req->transaction_id));
    memcpy((char *)&timer_cancel_req->transaction_id, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
    memcpy(timer_cancel_req->timer_ctrl_addr1, g_ctl_info.timer_key,
        (sizeof(timer_cancel_req->timer_ctrl_addr1)+
         sizeof(timer_cancel_req->timer_ctrl_cpu1)+
         sizeof(timer_cancel_req->timer_ctrl_pin1)+
         sizeof(timer_cancel_req->timer_ctrl_addr2)+
         sizeof(timer_cancel_req->timer_ctrl_cpu2)+
         sizeof(timer_cancel_req->timer_ctrl_pin2)));

    /* PATHSEND */
    if (g_ctl_info.timer_entry_kbn == DEF_TIMER_ENTRY_PRIMARY) {
        g_svrtbl[DEF_CMSD_SV_TIMER].send_len = timer_cancel_rq_def_Size;
        s_result = CMSD_pathsend(DEF_CMSD_SV_TIMER, l_serv_buf, 0);
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_TIMER_CNSL_ERR, strlen(DEF_NERR_CMSD_TIMER_CNSL_ERR));
            return DEF_RET_NG;
        }
    } else {
        g_svrtbl[DEF_CMSD_SV_TIMER_B].send_len = timer_cancel_rq_def_Size;
        s_result = CMSD_pathsend(DEF_CMSD_SV_TIMER_B, l_serv_buf, 0);
        if (s_result != DEF_RET_OK) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_TIMER_CNSL_ERR, strlen(DEF_NERR_CMSD_TIMER_CNSL_ERR));
            return DEF_RET_NG;
        }
    }

    /* 処理結果判定 */
    if (memcmp(timer_cancel_res->process_result_code, "0000", sizeof(timer_cancel_res->process_result_code)) != 0) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_TIMER_CNSL_ERR, strlen(DEF_NERR_CMSD_TIMER_CNSL_ERR));
        CMIN_message_output ( DEF_EVT_ERR_REP_RCV
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_TIMER_CNSL_ERR
                            , "@L@T@S"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , g_svrtbl[DEF_CMSD_SV_TIMER].scname);
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* CMSD_timer_cancel */

/****************************************************************************/
/*  FUNCTION        : 22.0.0  CMSD_pathsend                                 */
/*  CALLING SEQ.    : short CMSD_pathsend(short, char*, short)              */
/*  ARGUMENT        : 1.server_no      (I)   サーバ管理テーブル番号         */
/*  ARGUMENT        : 2.pathsend_buf   (I)   PATHSENDバッファ               */
/*  ARGUMENT        : 3.backup_no      (I)   BACKUPサーバ管理テーブル番号   */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : サーバPATHSEND処理                                    */
/****************************************************************************/
short CMSD_pathsend(short server_no, char *pathsend_buf, short retry_cnt)
{
    COM_PSD_arg_1_def   COM_PSD_arg_1;
    COM_PSD_arg_2_def   COM_PSD_arg_2;
    COM_PSD_arg_3_def   COM_PSD_arg_3;
    COM_PSD_arg_4_def   COM_PSD_arg_4;
    short               s_result;

    memset((char *)&COM_PSD_arg_1, 0x00, sizeof(COM_PSD_arg_1));
    memset((char *)&COM_PSD_arg_2, 0x00, sizeof(COM_PSD_arg_2));
    memset((char *)&COM_PSD_arg_3, 0x00, sizeof(COM_PSD_arg_3));
    memset((char *)&COM_PSD_arg_4, 0x20, sizeof(COM_PSD_arg_4));

    /* PATHSEND情報 */
    memset(COM_PSD_arg_1.pathmon_name, 0x20, sizeof(COM_PSD_arg_1.pathmon_name));
    if (g_svrtbl[server_no].domainname[0] != ' ') {
        memcpy(COM_PSD_arg_1.pathmon_name, g_svrtbl[server_no].domainname, strlen(g_svrtbl[server_no].domainname));
    } else {
        memcpy(COM_PSD_arg_1.pathmon_name, g_svrtbl[server_no].monname, strlen(g_svrtbl[server_no].monname));
    }
    memset(COM_PSD_arg_1.serverclass_name, 0x20, sizeof(COM_PSD_arg_1.serverclass_name));
    memcpy(COM_PSD_arg_1.serverclass_name, g_svrtbl[server_no].scname, strlen(g_svrtbl[server_no].scname));

    memcpy(COM_PSD_arg_1.msg_buf, pathsend_buf, g_svrtbl[server_no].send_len);
    COM_PSD_arg_1.req_send_len = g_svrtbl[server_no].send_len;
    COM_PSD_arg_1.receive_max_len = DEF_BUF_LENGTH;
    COM_PSD_arg_1.send_timer_msec = g_myinfo.send_timer;
//  COM_PSD_arg_1.retry_cnt = (short)g_myinfo.send_retry_count;
    COM_PSD_arg_1.retry_cnt = retry_cnt;

    memcpy(COM_PSD_arg_2.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));

    memcpy(COM_PSD_arg_4.srv_logical_id, g_myinfo.serverclass_name, sizeof(COM_PSD_arg_4.srv_logical_id));
    memcpy(COM_PSD_arg_4.lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(COM_PSD_arg_4.lcn));

    /* PATHSEND */
    s_result = COM_PSD(
        &COM_PSD_arg_1,
        &COM_PSD_arg_2,
        &COM_PSD_arg_3,
        &g_cg010in_modle,
        &COM_PSD_arg_4);
    if (s_result != DEF_RET_OK) {
        if ((memcmp(g_cg010in_modle.emsinf.emsgkinf.inter_errcd, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL))==0) ||
            (g_cg010in_modle.emsinf.emsgkinf.inter_errcd[0] == ' ')) {
            memcpy(g_ctl_info.internal_err, DEF_NERR_PSEND_ERR_RE_OK, strlen(DEF_NERR_PSEND_ERR_RE_OK));
        } else {
            memcpy(g_ctl_info.internal_err, g_cg010in_modle.emsinf.emsgkinf.inter_errcd, sizeof(g_ctl_info.internal_err));
        }
        return DEF_RET_NG;
    }

    /* PATHSEND応答 */
    g_svrtbl[server_no].recv_len = COM_PSD_arg_1.receive_len;
    memcpy(pathsend_buf, COM_PSD_arg_1.msg_buf, g_svrtbl[server_no].recv_len);

    return DEF_RET_OK;
} /* CMSD_pathsend */

/****************************************************************************/
/*  FUNCTION        : 23.0.0  CMSD_errorlog                                 */
/*  CALLING SEQ.    : short CMSD_errorlog(void)                              */
/*  ARGUMENT        : 1.err_type       (I)   エラータイプ                   */
/*  ARGUMENT        : 2.err_data       (I)   エラーデータ                   */
/*  ARGUMENT        : 3.err_len        (I)   エラーデータ長                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : エラー出力ログ出力処理                                */
/****************************************************************************/
short CMSD_errorlog(short err_type, char *err_data, unsigned short err_len)
{
    short           wklen = 0;
    char            wkbuf[64];
    short           s_result;

    /* エラー出力ログ編集 */
    memset((char *)&g_glelg_ctl, 0x20, sizeof(g_glelg_ctl));
    g_glelg_ctl.err_denbun_id = '2'; /* 内部エラー検知 */
    memcpy(g_glelg_ctl.naibu_err_code, g_ctl_info.internal_err, sizeof(g_glelg_ctl.naibu_err_code));
    memcpy(g_glelg_ctl.srv_cls_info.srv_cls_id, g_myinfo.serverclass_name, sizeof(g_glelg_ctl.srv_cls_info.srv_cls_id));
    memcpy(g_glelg_ctl.srv_cls_info.srv_cls_mlt_num, "0000", sizeof(g_glelg_ctl.srv_cls_info.srv_cls_mlt_num));

    switch (err_type) {
    case DEF_CMSD_ELG_RECVQUE:
        memcpy(g_glelg_ctl.mti_id, g_ctl_info.qrecv_info.mti, sizeof(g_ctl_info.qrecv_info.mti));
        memcpy((char *)&g_glelg_ctl.denbun_send_recv_info, (char *)&g_recvque->denbun_send_recv_info, sizeof(g_recvque->denbun_send_recv_info));
        memcpy((char *)&g_glelg_ctl.tushin_cntrl_info, (char *)&g_recvque->tushin_cntrl_info, sizeof(g_recvque->tushin_cntrl_info));
        break;

    case DEF_CMSD_ELG_SENDQUE:
        memcpy(g_glelg_ctl.mti_id, g_ctl_info.qsend_info.mti, sizeof(g_ctl_info.qsend_info.mti));
        memcpy((char *)&g_glelg_ctl.denbun_send_recv_info, (char *)&g_sendque->denbun_send_recv_info, sizeof(g_sendque->denbun_send_recv_info));
        memcpy((char *)&g_glelg_ctl.tushin_cntrl_info, (char *)&g_sendque->tushin_cntrl_info, sizeof(g_sendque->tushin_cntrl_info));
        break;

    case DEF_CMSD_ELG_SENDRES:
        memcpy(g_glelg_ctl.mti_id, g_r401->control_info.mti, sizeof(g_r401->control_info.mti));
        memcpy((char *)&g_glelg_ctl.denbun_send_recv_info.denbun_log_key,
            (char *)&g_r401->control_info.denbun_log_key, sizeof(g_r401->control_info.denbun_log_key));
        break;

    case DEF_CMSD_ELG_SENDREQ:
        memcpy(g_glelg_ctl.mti_id, g_r402->control_info.mti, sizeof(g_r402->control_info.mti));
        memcpy((char *)&g_glelg_ctl.denbun_send_recv_info.denbun_log_key,
            (char *)&g_r402->control_info.denbun_log_key, sizeof(g_r402->control_info.denbun_log_key));
        break;

    default:
        CMIN_abend();
    }
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%05d", err_len);
    memcpy(&g_glelg_ctl.denbun_area.denbun_len, wkbuf, strlen(wkbuf));
    memset(wkbuf, 0x00, sizeof(wkbuf));
    sprintf(wkbuf, "%05d", (g_ctl_info.mti_off - g_ctl_info.msg_off + 1));
    memcpy(&g_glelg_ctl.denbun_area.mti_start_lct, wkbuf, strlen(wkbuf));
    memcpy(&g_glelg_ctl.denbun_area.denbun, err_data, err_len);
    wklen = (sizeof(g_glelg_ctl) - sizeof(g_glelg_ctl.denbun_area.denbun)) + err_len;

    /* エラー出力共通インタフェース */
    g_com_erl_arg_1.file_io_type = DEF_COM_ERL_ARG1_WRITE;
    g_com_erl_arg_1.io_timer = g_myinfo.io_timer;
    g_com_erl_arg_1.data_len = wklen;
    g_com_erl_arg_1.data_area = (char *)&g_glelg_ctl;
    memcpy(g_com_erl_arg_3.srv_logical_id, g_myinfo.serverclass_name, sizeof(g_com_erl_arg_3.srv_logical_id));
    memcpy(g_com_erl_arg_3.lcn, g_ctl_info.denbun_log_key.tran_id.gfp_lcn, sizeof(g_com_erl_arg_3.lcn));
    memcpy(g_com_erl_arg_3.connect, (char *)&g_ctl_info.connection_lid, sizeof(g_com_erl_arg_3.connect));

    /* エラー出力ログ出力 */
    s_result = COM_ERL(
        &g_com_erl_arg_1,
        &g_com_erl_arg_2,
        &g_cg010in_modle,
        &g_com_erl_arg_3,
        DEF_GFPCVX70);
    if (s_result != DEF_RET_OK) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* CMSD_errorlog */

/****************************************************************************/
/*  FUNCTION        : 24.0.0  CMSD_deploy_bitmap                            */
/*  CALLING SEQ.    : short CMSD_deploy_bitmap(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : ビットマップ展開処理                                  */
/****************************************************************************/
short CMSD_deploy_bitmap(void)
{
    bool        bitmap_result;

    memset(g_ffmt_buf, 0x00, sizeof(g_ffmt_buf));
    g_format_info.ffmt_len = 0;

    /* バージョン設定 */
    if (g_gfnwi_ctl.denbun_item_lct_info.text_format_version == ' ') {
        bitmap_result = com_btm_set_ipc_version(&g_iso8583_context, '1');
    } else {
        bitmap_result = com_btm_set_ipc_version(&g_iso8583_context, g_gfnwi_ctl.denbun_item_lct_info.text_format_version);
    }
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_DEC_ERR, strlen(DEF_NERR_CMSD_BITMAP_DEC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_DEC_ERR
                            , "@C@5"
                            , "com_btm_set_ipc_version"
                            , g_iso8583_context.m_error_info.m_errCd);
        return DEF_RET_NG;
    }

    /* ビットマップオブジェクト */
    memset(&g_iso8583_object, 0, sizeof(g_iso8583_object));
    bitmap_result = com_btm_create_ISO8583_object(
        &g_iso8583_context,
        &g_iso8583_object,
        g_ctl_info.qrecv_info.mti);
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_DEC_ERR, strlen(DEF_NERR_CMSD_BITMAP_DEC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_DEC_ERR
                            , "@C@5"
                            , "com_btm_create_ISO8583_object"
                            , g_iso8583_context.m_error_info.m_errCd);
        return DEF_RET_NG;
    }

    /* ビットマップ展開 */
    bitmap_result = com_btm_render_fixformat(
        &g_iso8583_object,
        g_ffmt_buf,
        g_iso8583_buf,
        g_format_info.iso8583_len);
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_DEC_ERR, strlen(DEF_NERR_CMSD_BITMAP_DEC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_DEC_ERR
                            , "@C@5"
                            , "com_btm_render_fixformat"
                            , g_iso8583_object.m_error_info.m_errCd);
        return DEF_RET_NG;
    }
    g_format_info.ffmt_len = com_btm_get_fixformat_length(&g_iso8583_object);

    return DEF_RET_OK;
} /* CMSD_deploy_bitmap */

/****************************************************************************/
/*  FUNCTION        : 25.0.0  CMSD_create_bitmap                            */
/*  CALLING SEQ.    : short CMSD_create_bitmap(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : ビットマップ組立処理                                  */
/****************************************************************************/
short CMSD_create_bitmap(void)
{
    bool        bitmap_result;

    memset(g_iso8583_buf, 0x00, sizeof(g_iso8583_buf));
    g_format_info.iso8583_len = 0;

    /* バージョン設定 */
    if (g_gfnwi_ctl.denbun_item_lct_info.text_format_version == ' ') {
        bitmap_result = com_btm_set_ipc_version(&g_iso8583_context, '1');
    } else {
        bitmap_result = com_btm_set_ipc_version(&g_iso8583_context, g_gfnwi_ctl.denbun_item_lct_info.text_format_version);
    }
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_ENC_ERR, strlen(DEF_NERR_CMSD_BITMAP_ENC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_ENC_ERR
                            , "@C@5"
                            , "com_btm_set_ipc_version"
                            , g_iso8583_context.m_error_info.m_errCd);
        return DEF_RET_NG;
    }

    /* ビットマップオブジェクト */
    memset(&g_iso8583_object, 0, sizeof(g_iso8583_object));
    bitmap_result = com_btm_create_ISO8583_object(
        &g_iso8583_context,
        &g_iso8583_object,
        g_ctl_info.qsend_info.mti);
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_ENC_ERR, strlen(DEF_NERR_CMSD_BITMAP_ENC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_ENC_ERR
                            , "@C@5"
                            , "com_btm_create_ISO8583_object"
                            , g_iso8583_context.m_error_info.m_errCd);
        return DEF_RET_NG;
    }

    /* ビットマップ組立 */
    bitmap_result = com_btm_render_ISO8583_message(
        &g_iso8583_object,
        g_iso8583_buf,
        sizeof(g_iso8583_buf),
        g_ffmt_buf);
    if (!bitmap_result) {
        memcpy(g_ctl_info.internal_err, DEF_NERR_CMSD_BITMAP_ENC_ERR, strlen(DEF_NERR_CMSD_BITMAP_ENC_ERR));
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_CMSD_BITMAP_ENC_ERR
                            , "@C@5"
                            , "com_btm_render_ISO8583_message"
                            , g_iso8583_object.m_error_info.m_errCd);
        return DEF_RET_NG;
    }
    g_format_info.iso8583_len = com_btm_get_ISO8583_message_length(&g_iso8583_object);

    return DEF_RET_OK;
} /* CMSD_create_bitmap */

/****************************************************************************/
/*  FUNCTION        : 26.0.0  CMSD_read_netfile                             */
/*  CALLING SEQ.    : short CMSD_read_netfile(void)                         */
/*  ARGUMENT        : 1.unit_kbn       (I)   単位区分                       */
/*  ARGUMENT        : 2.interface_id   (I)   インタフェース識別             */
/*  ARGUMENT        : 3.station_id     (I)   ステーション識別               */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : N/W情報ファイルREAD処理                               */
/****************************************************************************/
short CMSD_read_netfile(
    short           unit_kbn,
    char            *interface_id,
    char            *station_id)
{
    short           s_result;
    db_gfnwi_def    *db_gfnwi;
    char            wkbuf[64];

    db_gfnwi = (db_gfnwi_def *)&g_com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_NW_INFO, strlen(DEF_FL_NW_INFO));
    memcpy(g_com_iom_arg_3.file_name, g_com_file_data.nw_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, "READ    ", 8);

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(g_com_iom_arg_4.file_name, g_com_file_data.nw_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_com_file_data.nw_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gfnwi->pri_key, sizeof(db_gfnwi->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len            = sizeof(db_gfnwi->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gfnwi->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfnwi_def_Size;

    /* 検索キー */
    db_gfnwi->pri_key.site_id = g_myinfo.site_id;
    db_gfnwi->pri_key.nw_id = g_myinfo.network_id;
    memcpy(db_gfnwi->pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));

    if (unit_kbn == DEF_CMSD_UNIT_INTERFACE) {
        memcpy(db_gfnwi->pri_key.if_id, interface_id, 5);
        memcpy(db_gfnwi->pri_key.station_id, "}}}}}}", 6);
    }
    if (unit_kbn == DEF_CMSD_UNIT_STATION) {
        memcpy(db_gfnwi->pri_key.if_id, interface_id, 5);
        memcpy(db_gfnwi->pri_key.station_id, station_id, 6);
    }
    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gfnwi->pri_key, sizeof(db_gfnwi->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_com_iom_arg_5.key_value, g_com_iom_arg_5.key_len);
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@U"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , DEF_FL_NW_INFO
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        if (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_EOF_ERR, strlen(DEF_COM_IOM_EOF_ERR))==0) {
            return DEF_CMSD_FILE_EOF;
        } else {
            return DEF_CMSD_FILE_ERROR;
        }
    }

    /* レコード情報取得 */
    memcpy((char *)&g_gfnwi_ctl, (char *)db_gfnwi, db_gfnwi_def_Size);

    return DEF_CMSD_FILE_NORMAL;
} /* CMSD_read_netfile */

/****************************************************************************/
/*  FUNCTION        : 27.0.0  CMSD_read_controlfile                         */
/*  CALLING SEQ.    : short CMSD_read_controlfile(void)                     */
/*  ARGUMENT        : 1.select_kbn     (I)   検索区分                       */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 制御電文管理ファイルREAD処理                          */
/****************************************************************************/
short CMSD_read_controlfile(short select_kbn)
{
    short           s_result;
    char            wkbuf[64];

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_CTRL_DEN_MG, strlen(DEF_FL_CTRL_DEN_MG));
    memcpy(g_com_iom_arg_3.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, "READ    ", 8);

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFMTL, strlen(DEF_GFMTL));
    memcpy(g_com_iom_arg_4.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_kbt_file_data.GFMTL_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_LOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfmtl_def_Size;

    /* 検索キー */
    if (select_kbn == DEF_CMSD_KEY_LCN) {
        g_gfmtl_ctl.pri_key.part_id[0] = '0';
        g_gfmtl_ctl.pri_key.part_id[1] = g_ctl_info.req_gfp_lcn[14];
        memcpy(g_gfmtl_ctl.pri_key.lcn_id, g_ctl_info.req_gfp_lcn, sizeof(g_ctl_info.req_gfp_lcn));
        memcpy(g_com_iom_arg_5.key_value  , (char *)&g_gfmtl_ctl.pri_key, sizeof(g_gfmtl_ctl.pri_key));
        memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
        g_com_iom_arg_5.key_len            = sizeof(g_gfmtl_ctl.pri_key);
        g_com_iom_arg_5.compare_len       = sizeof(g_gfmtl_ctl.pri_key);
    }
    if (select_kbn == DEF_CMSD_KEY_MATCH) {
        memset(g_gfmtl_ctl.req_res_match_key, 0x20, sizeof(g_gfmtl_ctl.req_res_match_key));
        memcpy(g_gfmtl_ctl.req_res_match_key, g_ctl_info.matching_key, sizeof(g_ctl_info.matching_key));
        memcpy(g_com_iom_arg_5.key_value  , (char *)&g_gfmtl_ctl.req_res_match_key, sizeof(g_gfmtl_ctl.req_res_match_key));
        memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_A1, strlen(DEF_COM_IOM_KEYTYPE_A1));
        g_com_iom_arg_5.key_len            = sizeof(g_gfmtl_ctl.req_res_match_key);
        g_com_iom_arg_5.compare_len       = sizeof(g_gfmtl_ctl.req_res_match_key);
    }
    if (select_kbn == DEF_CMSD_KEY_TIMER) {
        memcpy(g_gfmtl_ctl.timer_info_key, g_ctl_info.timer_key, sizeof(g_gfmtl_ctl.timer_info_key));
        memcpy(g_com_iom_arg_5.key_value  , (char *)&g_gfmtl_ctl.timer_info_key, sizeof(g_gfmtl_ctl.timer_info_key));
        memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_A2, strlen(DEF_COM_IOM_KEYTYPE_A2));
        g_com_iom_arg_5.key_len            = sizeof(g_gfmtl_ctl.timer_info_key);
        g_com_iom_arg_5.compare_len       = sizeof(g_gfmtl_ctl.timer_info_key);
    }

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_com_iom_arg_5.key_value, g_com_iom_arg_5.key_len);
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@U"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , DEF_FL_CTRL_DEN_MG
                            , DEF_FILEIO_READ
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        if (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_EOF_ERR, strlen(DEF_COM_IOM_EOF_ERR))==0) {
            return DEF_CMSD_FILE_EOF;
        } else {
            return DEF_CMSD_FILE_ERROR;
        }
    }

    /* レコード設定 */
    memcpy((char *)&g_gfmtl_ctl, g_com_iom_arg_6.rec_area, db_gfmtl_def_Size);

    return DEF_CMSD_FILE_NORMAL;
} /* CMSD_read_controlfile */

/****************************************************************************/
/*  FUNCTION        : 28.0.0  CMSD_write_controlfile                        */
/*  CALLING SEQ.    : short CMSD_write_controlfile(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 制御電文管理ファイルWRITE処理                         */
/****************************************************************************/
short CMSD_write_controlfile()
{
    db_gfmtl_def    g_gfmtl_tmp;    /* 退避制御電文管理レコード */
    short           s_result;
    char            wkbuf[64];

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_CTRL_DEN_MG, strlen(DEF_FL_CTRL_DEN_MG));
    memcpy(g_com_iom_arg_3.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, "WRITE   ", 8);

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFMTL, strlen(DEF_GFMTL));
    memcpy(g_com_iom_arg_4.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_kbt_file_data.GFMTL_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    g_com_iom_arg_5.key_len           = 0;
    g_com_iom_arg_5.compare_len       = 0;
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfmtl_def_Size;
    memcpy(g_com_iom_arg_5.rec_area, (char *)&g_gfmtl_ctl, db_gfmtl_def_Size);

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_ADD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if (s_result != DEF_RET_OK) {
        // 重複の場合は更新
        if (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_DUPLICATE_ERR, strlen(g_ch_sub_prog_sts))==0) {
            memcpy((char *)&g_gfmtl_tmp, (char *)&g_gfmtl_ctl, sizeof(g_gfmtl_ctl));
            s_result = CMSD_read_controlfile(DEF_CMSD_KEY_LCN);
            if (s_result != DEF_CMSD_FILE_NORMAL) {
                return DEF_CMSD_FILE_ERROR;
            }
            memcpy((char *)&g_gfmtl_ctl, (char *)&g_gfmtl_tmp, sizeof(g_gfmtl_tmp));
            s_result = CMSD_update_controlfile();
            if (s_result != DEF_CMSD_FILE_NORMAL) {
                return DEF_CMSD_FILE_ERROR;
            }
        } else {
            memset(wkbuf, 0x00, sizeof(wkbuf));
            memcpy(wkbuf, (char *)&g_gfmtl_ctl.pri_key, sizeof(g_gfmtl_ctl.pri_key));
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@L@T@C@C@C@U"
                                , g_ctl_info.req_gfp_lcn
                                , (char *)&g_ctl_info.connection_lid
                                , DEF_FL_CTRL_DEN_MG
                                , DEF_FILEIO_WRITE
                                , wkbuf
                                , g_com_iom_arg_6.guardian_errcode);
            return DEF_CMSD_FILE_ERROR;
        }
    }

    return DEF_CMSD_FILE_NORMAL;
} /* CMSD_write_controlfile */

/****************************************************************************/
/*  FUNCTION        : 29.0.0  CMSD_update_controlfile                       */
/*  CALLING SEQ.    : void CMSD_update_controlfile(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 制御電文管理ファイルUPDATE処理                        */
/****************************************************************************/
short CMSD_update_controlfile(void)
{
    short           s_result;
    char            wkbuf[64];

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_CTRL_DEN_MG, strlen(DEF_FL_CTRL_DEN_MG));
    memcpy(g_com_iom_arg_3.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, "REWRITE ", 8);

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFMTL, strlen(DEF_GFMTL));
    memcpy(g_com_iom_arg_4.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_kbt_file_data.GFMTL_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    g_com_iom_arg_5.key_len           = 0;
    g_com_iom_arg_5.compare_len       = 0;
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfmtl_def_Size;
    memcpy(g_com_iom_arg_5.rec_area, (char *)&g_gfmtl_ctl, db_gfmtl_def_Size);

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_com_iom_arg_5.key_value, g_com_iom_arg_5.key_len);
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@U"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , DEF_FL_CTRL_DEN_MG
                            , DEF_FILEIO_UPDATE
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        return DEF_CMSD_FILE_ERROR;
    }

    return DEF_CMSD_FILE_NORMAL;
} /* CMSD_update_controlfile */


/****************************************************************************/
/*  FUNCTION        : 30.0.0  CMSD_unlock_controlfile                       */
/*  CALLING SEQ.    : void CMSD_unlock_controlfile(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御電文管理ファイルUNLOCK処理                        */
/****************************************************************************/
void CMSD_unlock_controlfile(void)
{
    short           s_result;
    char            wkbuf[64];

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x20, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x20, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x20, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, DEF_GFPCVX70, strlen(DEF_GFPCVX70));
    memcpy(g_com_iom_arg_3.file_id, DEF_FL_CTRL_DEN_MG, strlen(DEF_FL_CTRL_DEN_MG));
    memcpy(g_com_iom_arg_3.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, "UNLOCK  ", 8);

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_GFMTL, strlen(DEF_GFMTL));
    memcpy(g_com_iom_arg_4.file_name, g_kbt_file_data.GFMTL_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_kbt_file_data.GFMTL_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    g_com_iom_arg_5.key_len           = 0;
    g_com_iom_arg_5.compare_len       = 0;
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfmtl_def_Size;

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_UNLOC
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR))!=0)) {
        memset(wkbuf, 0x00, sizeof(wkbuf));
        memcpy(wkbuf, g_com_iom_arg_5.key_value, g_com_iom_arg_5.key_len);
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@U"
                            , g_ctl_info.req_gfp_lcn
                            , (char *)&g_ctl_info.connection_lid
                            , DEF_FL_CTRL_DEN_MG
                            , DEF_FILEIO_UNLOCKREC
                            , wkbuf
                            , g_com_iom_arg_6.guardian_errcode);
        CMIN_abend();
    }

} /* CMSD_unlock_controlfile */

