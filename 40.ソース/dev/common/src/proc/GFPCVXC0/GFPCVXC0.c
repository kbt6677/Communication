/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVXC0                                    */
/*        FUNCTION          ････ 通知電文制御                                */
/*                                                                           */
/*                               制御電文振分サーバからNW電文受信要求を      */
/*                               受信して通知電文(障害電文通知、             */
/*                               REJECTメッセージ)に対する処理を行う。       */
/*                                                                           */
/*        AUTHER            ････ HAS hashimoto                               */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-03-13                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 橋本   2025/03/13 (通知電文制御)新規作成                        */
/*****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <netdb.h>    nolist
#include <stdarg.h>   nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h"
#include "ems.h"
#include "errcd.h"
#include "GFPCGX80.h"
#include "GFPCVXZ0.h"
#include "GFPCVXZ2.h"
#include "GFPCGX90.h"
#include "GFPCVXA0.h"
#include "NWM_NTC.h"
#include "GFPCVXC0.h"                    /* 通知電文制御ヘッダーファイル     */
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/* 関数のﾌﾟﾛﾄﾀｲﾌﾟ宣言 */

/****************************************************************************/
/*  FUNCTION        : 4.1  CMIN_kbt_set_prgid                               */
/*  CALLING SEQ.    : void CMIN_kbt_set_prgid ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : プログラムIDセット処理                                */
/*                  : 本プログラムのIDを設定する。                          */
/****************************************************************************/
void CMIN_kbt_set_prgid ( void )
{

  /* (1) プログラムID設定処理 */
  memmove( g_myinfo.prog_id, DEF_GFPCVXC0, sizeof(g_myinfo.prog_id) - 1 );

} /*end of CMIN_kbt_set_prgid*/

/****************************************************************************/
/*  FUNCTION        : 4.2   CMIN_kbt_init                                   */
/*  CALLING SEQ.    : short CMIN_kbt_init ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     :  0:正常終了                                           */
/*                    -1:異常終了                                           */
/*  DESCRIPTION     : 個別初期処理                                          */
/*                  : 制御電文共通メインの初期処理以外で個別に              */
/*                    必要な初期処理を行う。                                */
/****************************************************************************/
short  CMIN_kbt_init( void )
{
  /* データポインタ */
  g_ipcreq_head = (common_header_def *)g_recv_buf;
  g_ipcres_head = (common_header_def *)g_resp_buf;
  g_c401 = (cr401_def *)g_recv_buf;
  g_r401 = (cr401_def *)g_resp_buf;

  /* (1) 正常終了で呼出し元にリターンする。 */
  return DEF_RET_OK;

} /*end of CMIN_kbt_init*/

/****************************************************************************/
/*  FUNCTION        : 4.3   CMIN_kbt_get_physical_names                     */
/*  CALLING SEQ.    : short CMIN_kbt_get_physical_names ( void )            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     :  0:正常終了                                           */
/*                    -1:異常終了                                           */
/*  DESCRIPTION     : 個別物理名情報取得処理                                */
/*                  : 制御電文共通メインの物理名取得処理以外で個別に必要な  */
/*                    物理名取得処理を行う。                                */
/****************************************************************************/
short  CMIN_kbt_get_physical_names( void )
{
  short            ls_result;
  t_gfphi_pri_key  l_gfphi_pri_key;
  db_gfphi_def    *phy_tbl_local;

  /* ① WK検索キー(プライマリキー情報)を編集する。 */

  l_gfphi_pri_key.site_id = g_myinfo.site_id;
  l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
  memmove( l_gfphi_pri_key.grp_id     , g_myinfo.group_id
         , sizeof(l_gfphi_pri_key.grp_id));
  memset(&l_gfphi_pri_key.srv_cls_key, DEF_BUF_NO_SET
        , sizeof(l_gfphi_pri_key.srv_cls_key));
  memmove( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
         , DEF_FL_CTRL_DEN_LOG
         , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
  memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
        , DEF_BUF_CZERO
        , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
  memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
        , DEF_BUF_CZERO
        , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

  /* ② 「物理ファイル名取得処理(CMIN_get_phy_name)」を使用して、*/
  /*     制御電文ログファイル名を取得する。 */
  ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

  /* ③ 処理結果を判定する。 */
  if ( ls_result == DEF_RET_OK ) {              /* IOモジュール結果判定 */
    memset(g_kbt_file_data.log_file_name, 0x00 ,sizeof(g_kbt_file_data.log_file_name));
    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memmove( g_kbt_file_data.log_file_name
           , phy_tbl_local->prc_file_info.prc_file_name
           , sizeof(g_kbt_file_data.log_file_name));
    g_kbt_file_data.log_file_no = -1;
    return DEF_RET_OK;
  } else {
    g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
    return DEF_RET_NG;
  }

} /*end of CMIN_kbt_get_physical_names*/

/****************************************************************************/
/*  FUNCTION        : 4.4   CMIN_kbt_file_open                              */
/*  CALLING SEQ.    : short CMIN_kbt_file_open(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 ファイルオープン処理                             */
/****************************************************************************/
short CMIN_kbt_file_open(void)
{
  short       s_result;

  /* 制御電文管理ファイル */
  s_result = CMIN_file_open( DEF_FL_CTRL_DEN_LOG,
                             g_kbt_file_data.log_file_name,
                            &g_kbt_file_data.log_file_no );
  if( s_result != DEF_RET_OK ) {
    g_myinfo.end_flg = DEF_FLAG_ON;
    return DEF_RET_NG;
  }

  return DEF_RET_OK;
} /* CMIN_kbt_file_open */

/****************************************************************************/
/*  FUNCTION        : 4.x  CMIN_kbt_file_close                              */
/*  CALLING SEQ.    : void CMIN_kbt_file_close(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 個別 ファイルクローズ処理                             */
/****************************************************************************/
void   CMIN_kbt_file_close(void)
{
  CMIN_file_close( DEF_FL_CTRL_DEN_LOG,
                   g_kbt_file_data.log_file_name,
                  &g_kbt_file_data.log_file_no );
}

/****************************************************************************/
/*  FUNCTION        : 4.5  CMIN_handle_req_msg                              */
/*  CALLING SEQ.    : void CMIN_handle_req_msg(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 個別 電文受信処理                                     */
/****************************************************************************/
void CMIN_handle_req_msg(void)
{

  /* 個別グローバル情報初期化 */
  memset( (char *)&g_db_gfnwi, NULL, sizeof(db_gfnwi_def) );
//memset( (char *)&g_glmlg_io, NULL, sizeof(g_glmlg_io) );
  memset( (char *)&g_wk_glmlg_pri_key, NULL, sizeof(g_wk_glmlg_pri_key) );
  memset( (char *)&g_ctrl_info, NULL, sizeof(g_ctrl_info) );
  memmove( g_ctrl_info.internal_error_code,
           DEF_NERR_NOMAL,
           sizeof(g_ctrl_info.internal_error_code) );

  /* (1) リクエストデータのインターフェースコード(先頭4バイト)で処理を振分ける。 */
  if (memcmp( g_ipcreq_head->interface_code, DEF_IPC_IFCD_NW_MSG_REQ, 
              strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == 0 ) {
    /* 電文受信通知処理を呼び出す。 */
    CNTF_receive_req_data();
  } else {
    /* a) IPC精査エラーのEMS出力を行う。 */
    CMIN_message_output( DEF_EVT_REQ_ERR
                       , DEF_MSGTTKB_GYOM_ERR
                       , DEF_NERR_IPC_SEISA_ERR
                       , "@C@H"
                       , "ﾋﾀｲｵｳIPC"
                       , (char *)g_ipcreq_head );

    /* b) 応答バッファをNULLで初期化後、リプライデータを編集する。 *//* 破棄 */
    memset( g_ipcres_head, NULL, sizeof(common_header_def) );
    memmove( g_ipcres_head->interface_code,
             DEF_IPC_IFCD_NW_MSG_RSP,
             sizeof(g_ipcres_head->interface_code ));
    g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
    memcpy( g_ipcres_head->internal_error_code,
            DEF_NERR_NOMAL,
            sizeof(g_ipcres_head->internal_error_code) );
    g_ipcres_head->control_data_length = 0;

    /* c) CMIN_send_replyを使用してリプライメッセージを返す。 */
    CMIN_send_reply((char *)g_ipcres_head, (short)sizeof(common_header_def), 0);
  }

} /* CMIN_handle_req_msg */

/****************************************************************************/
/*  FUNCTION        : 4.6  CNTF_receive_req_data                            */
/*  CALLING SEQ.    : void CNTF_receive_req_data(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 電文受信通知処理                                      */
/****************************************************************************/
void CNTF_receive_req_data(void)
{
  short       s_result;
  char        wkbuf1[64];
  short       s_dtlen;

/* (1) IPC精査処理 */
  /* ①  ｢CNTF_validate_request(電文精査処理)｣を呼び出して、IPCを精査する。 */
  s_result = CNTF_validate_request( );

  /* ② 処理結果を確認する。 */
  if( s_result == DEF_RET_OK ) {
  } else {
    /* a) 応答バッファをNULLで初期化後、リプライデータを編集する。 */
    memset( g_ipcres_head, NULL, sizeof(common_header_def) );
    memmove( g_ipcres_head->interface_code,
             DEF_IPC_IFCD_NW_MSG_RSP,
             sizeof(g_ipcres_head->interface_code ));
    g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
    memcpy( g_ipcres_head->internal_error_code,
            DEF_NERR_NOMAL,
            sizeof(g_ipcres_head->internal_error_code) );
    g_ipcres_head->control_data_length = 0;

    /* b) CMIN_send_replyを使用してリプライメッセージを返す。 */
    CMIN_send_reply((char *)g_ipcres_head, (short)sizeof(common_header_def), 0);

    /* c) 呼出し元にリターンする。 */
    return;
  }

/* (2) 要求種別振分 */
  /* ① IPCの要求種別から処理を振分ける。 */
  if( memcmp( g_c401->control_info.request_kind, DEF_CTLREQ_HISIMUKE_ERROR,
              sizeof(g_c401->control_info.request_kind) ) == 0 ) {
    CNTF_send_response_error( );
    return;
  }

/* (3) 通知電文精査 */
  /* ① 「N/W情報ファイル取得処理(CNTF_get_network_info)」を呼び出して、 */
  /*    障害電文通知管理単位のN/W情報を取得する。 */
  CNTF_get_network_info( DEF_GFNWI_GET_IF,
                         (char *)&g_c401->control_info.connection_lid.interface_name,
                         DEF_STATION_ID_DEFAULT,
                         (db_gfnwi_def *)&g_db_gfnwi );
  /* ② NW個別モジュール「通知電文精査（NWM_NTC_msg_check）」を */
  /*    呼び出して、通知電文内容を精査する。 */
  memset(wkbuf1, 0x00, sizeof(wkbuf1));
  memcpy(wkbuf1, g_c401->control_info.denbun_len, sizeof(g_c401->control_info.denbun_len));
  s_dtlen = (short)atoi(wkbuf1);
  g_ctrl_info.scrutiny_result = NWM_NTC_msg_check(
                                (char *)&g_ctrl_info.rcv_control_kind,
                                (char *)&g_c401->data_bu,
                                (short)g_ctrl_info.rcv_msg_len,
                                s_dtlen,
                                (db_gfnwi_def *)&g_db_gfnwi );
  /* ③ NW個別モジュール「通知電文精査（NWM_NTC_msg_check）」の */
  /*    処理結果を判定する。 */
  switch( g_ctrl_info.scrutiny_result ) {
    case DEF_RTN_RECEIVE_NOTICE:      /* 1：通知電文 */
      g_glmlg_pri_key.s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
      g_glmlg_pri_key.send_recv_id = DEF_REQ_RCV;
      CNTF_receive_notice( );
      break;
    case DEF_RTN_RECEIVE_REQUEST:     /* 2：要求応答型要求電文 */
      g_glmlg_pri_key.s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
      g_glmlg_pri_key.send_recv_id = DEF_REQ_RCV;
      CNTF_receive_request( );
      break;
    case DEF_RTN_RECEIVE_RESPONSE:    /* 3：要求応答型応答電文 */
      g_glmlg_pri_key.s_h_kubun = DEF_S_H_KUBUN_SIMUKE;
      g_glmlg_pri_key.send_recv_id = DEF_RSP_RCV;
      CNTF_receive_response( );
      break;
    case DEF_RTN_ABNORMAL_SCRUTINY:   /* 9： 検査異常 */
      CNTF_validate_error( );
      break;
  }

} /* CNTF_receive_req_data */

/****************************************************************************/
/*  FUNCTION        : 4.7  CNTF_validate_request                            */
/*  CALLING SEQ.    : short CNTF_validate_request(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0：正常, 1：エラー                                    */
/*  DESCRIPTION     : 電文精査処理                                          */
/****************************************************************************/
short CNTF_validate_request(void)
{
  /* (1) 要求種別精査 */
  if(  memcmp( g_c401->control_info.request_kind, DEF_CTLREQ_HISIMUKE,
               sizeof(g_c401->control_info.request_kind) ) == 0
    || memcmp( g_c401->control_info.request_kind, DEF_CTLREQ_SIMUKE,
               sizeof(g_c401->control_info.request_kind) ) == 0
    || memcmp( g_c401->control_info.request_kind, DEF_CTLREQ_HISIMUKE_ERROR,
               sizeof(g_c401->control_info.request_kind) ) == 0 ) {
    memmove( &g_ctrl_info.rcv_control_kind,
             &g_c401->control_info.control_kind,
             sizeof(g_ctrl_info.rcv_control_kind) );
  } else {
    CMIN_message_output( DEF_EVT_REQ_ERR
                       , DEF_MSGTTKB_GYOM_ERR
                       , DEF_NERR_IPC_SEISA_ERR
                       , "@C@H"
                       , "ﾖｳｷｭｳｼｭﾍﾞﾂ"
                       , (char *)g_c401 );
    memmove( g_ctrl_info.internal_error_code,
             DEF_NERR_IPC_SEISA_ERR,
    sizeof(g_ctrl_info.internal_error_code) );
    return DEF_RET_NG;
  }

  /* (2) 制御電文種別精査  */
  if(   g_c401->control_info.control_kind.kinou_kbn == DEF_CTLFNC_NTF_MSG
    &&  (  g_c401->control_info.control_kind.req_res_kbn == DEF_CTLMSG_REQUEST
        || g_c401->control_info.control_kind.req_res_kbn == DEF_CTLMSG_RESPONSE
        || g_c401->control_info.control_kind.req_res_kbn == DEF_CTLMSG_NOTICE )
    &&  g_c401->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_FAL ) {
    /* (3)に遷移 */
  } else {
    CMIN_message_output( DEF_EVT_REQ_ERR
                       , DEF_MSGTTKB_GYOM_ERR
                       , DEF_NERR_IPC_SEISA_ERR
                       , "@C@H"
                       , "ｾｲｷﾞｮﾃﾞﾝﾌﾞﾝｼｭﾍﾞﾂ"
                       , (char *)g_c401 );
    memmove( g_ctrl_info.internal_error_code,
             DEF_NERR_IPC_SEISA_ERR,
    sizeof(g_ctrl_info.internal_error_code) );
    return DEF_RET_NG;
  }

  /* (3) データ長精査 */
  g_ctrl_info.rcv_msg_len = g_c401->common_header.control_data_length
                             - (unsigned short)sizeof(g_c401->control_info);
  if( g_ctrl_info.rcv_msg_len >= g_recv_len 
                                  - (unsigned short)(sizeof(g_c401->common_header)
                                                     + sizeof(g_c401->control_info)) ) {
    return DEF_RET_OK;
  } else {
    CMIN_message_output( DEF_EVT_REQ_ERR
                       , DEF_MSGTTKB_GYOM_ERR
                       , DEF_NERR_IPC_SEISA_ERR
                       , "@C@H"
                       , "ﾃﾞｰﾀﾁｮｳ"
                       , (char *)g_c401 );
    memmove( g_ctrl_info.internal_error_code,
             DEF_NERR_IPC_SEISA_ERR,
    sizeof(g_ctrl_info.internal_error_code) );
    return DEF_RET_NG;
  }

} /* CNTF_validate_request */

/****************************************************************************/
/*  FUNCTION        : 4.8  CNTF_validate_error                              */
/*  CALLING SEQ.    : void CNTF_validate_error(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 電文精査エラー処理                                    */
/****************************************************************************/
void CNTF_validate_error(void)
{
  short  s_result;

  /* (1) エラーメッセージ出力 */
  CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                      , DEF_MSGTTKB_GYOM_ERR
                      , DEF_NERR_NTC_RECV_SEISA
                      , "@C@U"
                      , "NWM_NTC_msg_check"
                      , DEF_RET_NG );

  /* (2) エラー出力ログ出力処理 */
  CNTF_put_errlog(DEF_ELG_INTERNAL, DEF_NERR_NTC_RECV_SEISA);    // 内部エラー検知

  /* (3) NW電文受信応答編集 */
  /* ① 応答バッファをNULLで初期化後、リプライデータを編集する。 */
  memset( g_ipcres_head, NULL, sizeof(common_header_def) );
  memmove( g_ipcres_head->interface_code,
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(g_ipcres_head->interface_code ));
  g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
  memcpy( g_ipcres_head->internal_error_code,
          DEF_NERR_NOMAL,
          sizeof(g_ipcres_head->internal_error_code) );
  g_ipcres_head->control_data_length = 0;

  /* c) CMIN_send_replyを使用してリプライメッセージを返す。 */
  CMIN_send_reply( (char *)g_ipcres_head, (short)sizeof(common_header_def), 0 );

} /* CNTF_validate_error */

/****************************************************************************/
/*  FUNCTION        : 4.9  CNTF_receive_notice                              */
/*  CALLING SEQ.    : void CNTF_receive_notice(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 通知型電文受信処理                                    */
/****************************************************************************/
void CNTF_receive_notice(void)
{
  short           s_result;
  char            wkbuf1[64];
  char            wkbuf2[64];
  char            wkbuf3[64];

  /* (1) 「メッセージ出力処理(CNTF_message_output)」を使用し、 */
  /*     通知受信メッセージを出力する。 */
  memset( wkbuf1, 0x00, sizeof(wkbuf1) );
  memmove( wkbuf1,
           g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
           sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
  memset( wkbuf2, 0x00, sizeof(wkbuf2) );
  memmove( wkbuf2,
           &g_c401->control_info.connection_lid,
           sizeof(g_c401->control_info.connection_lid) );
  memset( wkbuf3, 0x00, sizeof(wkbuf3) );
  memmove( wkbuf3, "00000", 5 );
  CMIN_message_output( DEF_EVT_NTF_DENBUN_RCV        /* メッセージ出力処理  */
                     , DEF_MSGTTKB_NORMAL
                     , DEF_NERR_NOMAL
                     , "@L@T@C"
                     , wkbuf1
                     , wkbuf2
                     , wkbuf3 );

  /* (2) 「CNTF_put_log(制御電文ログ出力処理)」を呼び出し、 */
  /*     制御電文ログ出力処理を行う。 */
  CNTF_put_log( DEF_HSMK_REQ_RCV,
                DEF_NERR_NOMAL,
                (short)g_ctrl_info.rcv_msg_len,
    (char *)&g_c401->data_bu );

  /*     エラー出力ログ出力処理を行う。 */
  CNTF_put_errlog(DEF_ELG_NOTICE, DEF_NERR_NTC_RECV);  // 接続先から受信したエラー電文通知

  /* (3) NW電文受信応答リプライ */
  /* ヘッダ部編集 */
  memset( g_resp_buf, ' ', sizeof(g_resp_buf) );
  memmove( g_r401->common_header.interface_code,            /* インターフェースコード */
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(g_r401->common_header.interface_code) );
  g_r401->common_header.error_code = DEF_IPC_ERRCD_OK;      /* エラーコード */
  memmove( g_r401->common_header.internal_error_code,       /* 内部エラーコード */
           DEF_NERR_NOMAL,
           sizeof(g_r401->common_header.internal_error_code) );
  g_r401->common_header.control_data_length = 
    (unsigned short)sizeof(g_r401->control_info);
  
  /* 制御電文情報編集 */
  memmove( g_r401->control_info.request_kind,               /* 要求種別 */
           g_c401->control_info.request_kind,
           sizeof(g_r401->control_info.request_kind) );
  memmove( g_r401->control_info.response_kind,              /* 応答種別 */
           DEF_CTLRSP_NOSEND,
           sizeof(g_r401->control_info.response_kind) );
  memmove( &g_r401->control_info.control_kind,               /* 制御電文種別 */
           &g_c401->control_info.control_kind,
           sizeof(g_r401->control_info.control_kind) );
  memmove( &g_r401->control_info.connection_lid,             /* コネクション論理ID */
           &g_c401->control_info.connection_lid,
           sizeof(g_r401->control_info.connection_lid) );
  memmove( &g_r401->control_info.interface_name,             /* インタフェース名 */
           &g_c401->control_info.interface_name,
           sizeof(g_r401->control_info.interface_name) );
  memmove( &g_r401->control_info.station_name,               /* ステーション名 */
           &g_c401->control_info.station_name,
           sizeof(g_r401->control_info.station_name) );
  memmove( &g_r401->control_info.denbun_log_key,             /* 電文ログKEY */
           &g_c401->control_info.denbun_log_key,
           sizeof(g_r401->control_info.denbun_log_key) );

  /*② 「リプライ処理(CMIN_send_reply)」を呼び出してリプライする。 */
  CMIN_send_reply( (char *)g_ipcres_head,
                   (short)(sizeof(common_header_def) + sizeof(g_c401->control_info)),
                   0 );

} /* CNTF_receive_notice */

/****************************************************************************/
/*  FUNCTION        : 4.10 CNTF_receive_request                             */
/*  CALLING SEQ.    : void CNTF_receive_request(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求応答型要求電文受信処理                            */
/****************************************************************************/
void CNTF_receive_request(void)
{
  char            wkbuf1[64];
  char            wkbuf2[64];
  char            wkbuf3[64];
  short           s_result;
  unsigned short  us_resp_msg_len;

  /* (1) 「メッセージ出力処理(CNTF_message_output)」を使用し、 */
  /*     通知受信メッセージを出力する。 */
  memset( wkbuf1, 0x00, sizeof(wkbuf1) );
  memmove( wkbuf1,
           g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
           sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
  memset( wkbuf2, 0x00, sizeof(wkbuf2) );
  memmove( wkbuf2,
           &g_c401->control_info.connection_lid,
           sizeof(g_c401->control_info.connection_lid) );
  memset( wkbuf3, 0x00, sizeof(wkbuf1) );
  memmove( wkbuf3, "00000", 5 );
  CMIN_message_output( DEF_EVT_NTF_DENBUN_RCV        /* メッセージ出力処理  */
                     , DEF_MSGTTKB_NORMAL
                     , DEF_NERR_NOMAL
                     , "@L@T@C"
                     , wkbuf1
                     , wkbuf2
                     , wkbuf3 );

  /* (2) 「CNTF_put_log(制御電文ログ出力処理)」を呼び出し、 */
  /*     制御電文ログ出力処理を行う。 */
  CNTF_put_log( DEF_HSMK_REQ_RCV,
                DEF_NERR_NOMAL,
                (short)g_ctrl_info.rcv_msg_len,
                (char *)&g_c401->data_bu );

  /*     エラー出力ログ出力処理を行う。 */
  CNTF_put_errlog(DEF_ELG_NOTICE, DEF_NERR_REQ_RECV);  // 接続先から受信したエラー電文通知

  /* (3) NW電文受信応答編集 */
  /* ① NW個別モジュール「通知電文編集（NWM_NTC_msg_edit）」を呼び出して、 */
  /*    応答電文を編集する。 */
  memset( g_resp_buf, ' ', sizeof(g_resp_buf) );
  s_result = NWM_NTC_msg_edit( (char *)g_c401->data_bu.message_text,
                               (short )g_ctrl_info.rcv_msg_len,
                                       &g_db_gfnwi,
                               (char *)&g_r401->data_bu,
                                       sizeof(g_r401->data_bu),
                               (short*)&us_resp_msg_len,
                               (char *)g_ctrl_info.send_mti );
  /* ② NW個別モジュール「通知電文編集（NWM_NTC_msg_edit）」の */
  /*    処理結果を判定する。 */
  if( s_result == DEF_RTN_EDIT_ERROR ) {
    /* a) 電文編集エラー(共通モジュールエラー)のEMS出力を行う。 */
    memset( wkbuf1, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf1,
             g_procinfo.my_pname,
             sizeof(g_procinfo.my_pname) );
    CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_NTC_RESP_EDIT
                        , "@C@U"
                        , wkbuf1
                        , DEF_RET_NG );
    /* b) 応答バッファをNULLで初期化後、リプライデータを編集する。 */
    memset( g_ipcres_head, NULL, sizeof(common_header_def) );
    memmove( g_ipcres_head->interface_code,
             DEF_IPC_IFCD_NW_MSG_RSP,
             sizeof(g_ipcres_head->interface_code ));
    g_ipcres_head->error_code = DEF_IPC_ERRCD_OK;
    memcpy( g_ipcres_head->internal_error_code,
            DEF_NERR_NOMAL,
            sizeof(g_ipcres_head->internal_error_code) );
    g_ipcres_head->control_data_length = 0;
    /* c) CMIN_send_replyを使用してリプライメッセージを返す。 */
    CMIN_send_reply( (char *)g_ipcres_head, (short)sizeof(common_header_def), 0 );
    /* d) 呼出し元にリターンする。 */
    return;
  }

  /* (4) 「CNTF_put_log(制御電文ログ出力処理)」を呼び出し、 */
  /*     制御電文ログ出力処理を行う。 */
  CNTF_put_log( DEF_HSMK_REP_SND,
                DEF_NERR_NOMAL,
                (short)us_resp_msg_len,
                (char *)&g_r401->data_bu );

  /* (5) NW電文受信応答リプライ */
  /* ヘッダ部編集 */
  memmove( g_r401->common_header.interface_code,            /* インターフェースコード */
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(g_r401->common_header.interface_code) );
  g_r401->common_header.error_code = DEF_IPC_ERRCD_OK;      /* エラーコード */
  memcpy( g_r401->common_header.internal_error_code,        /* 内部エラーコード */
          DEF_NERR_NOMAL,
          sizeof(g_r401->common_header.internal_error_code) );
  g_r401->common_header.control_data_length = 
    (unsigned short)sizeof(g_r401->control_info) + us_resp_msg_len;
  
  /* 制御電文情報編集 */
  memmove( g_r401->control_info.request_kind,               /* 要求種別 */
           g_c401->control_info.request_kind,
           sizeof(g_r401->control_info.request_kind) );
  memmove( g_r401->control_info.response_kind,              /* 応答種別 */
           DEF_CTLRSP_SEND,
           sizeof(g_r401->control_info.response_kind) );
                                                            /* 制御電文種別 */
  g_r401->control_info.control_kind.kinou_kbn    = DEF_CTLFNC_NTF_MSG;
  g_r401->control_info.control_kind.req_res_kbn  = DEF_CTLMSG_RESPONSE;
  g_r401->control_info.control_kind.ctl_text_kbn = DEF_CTLTXT_FAL;
  g_r401->control_info.control_kind.int_proc_kbn = DEF_CTLINT_NORMAL;
  memmove( &g_r401->control_info.connection_lid,             /* コネクション論理ID */
           &g_c401->control_info.connection_lid,
           sizeof(g_r401->control_info.connection_lid) );
  memmove( g_r401->control_info.interface_name,             /* インタフェース名 */
           g_c401->control_info.interface_name,
           sizeof(g_r401->control_info.interface_name) );
  memmove( g_r401->control_info.station_name,               /* ステーション名 */
           g_c401->control_info.station_name,
           sizeof(g_r401->control_info.station_name) );
  memmove( &g_r401->control_info.denbun_log_key,             /* 電文ログKEY */
           &g_c401->control_info.denbun_log_key,
           sizeof(g_r401->control_info.denbun_log_key) );
  g_r401->control_info.denbun_log_key.denbun_shubetu = g_r401->control_info.control_kind.ctl_text_kbn;
  memmove( g_r401->control_info.mti,                        /* MTI */
           g_ctrl_info.send_mti,
           sizeof(g_r401->control_info.mti) );
  memmove( g_r401->control_info.req_gfp_lcn,                /* 仕向要求電文GFP内部LCN */
           g_c401->control_info.req_gfp_lcn,
           sizeof(g_r401->control_info.req_gfp_lcn) );

  /*② 「リプライ処理(CMIN_send_reply)」を呼び出してリプライする。 */
  CMIN_send_reply( (char *)g_ipcres_head,
                   (short)(sizeof(common_header_def) + g_r401->common_header.control_data_length),
                   0 );

} /* CNTF_receive_request */

/****************************************************************************/
/*  FUNCTION        : 4.11 CNTF_receive_response                            */
/*  CALLING SEQ.    : void CNTF_receive_response(void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求応答型応答電文受信処理                            */
/****************************************************************************/
void CNTF_receive_response(void)
{

  /* (1) 「CNTF_put_log(制御電文ログ出力処理)」を呼び出し、 */
  /*     制御電文ログ出力処理を行う。 */
  CNTF_put_log( DEF_SMK_REP_RCV,
                DEF_NERR_NOMAL,
                (short)g_ctrl_info.rcv_msg_len,
                (char *)&g_c401->data_bu );

  /* (2) NW電文受信応答リプライ */
  /* ヘッダ部編集 */
  memset( g_resp_buf, ' ', sizeof(g_resp_buf) );
  memmove( g_r401->common_header.interface_code,            /* インターフェースコード */
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(g_r401->common_header.interface_code) );
  g_r401->common_header.error_code = DEF_IPC_ERRCD_OK;      /* エラーコード */
  memcpy( g_r401->common_header.internal_error_code,        /* 内部エラーコード */
          DEF_NERR_NOMAL,
          sizeof(g_r401->common_header.internal_error_code) );
  g_r401->common_header.control_data_length =
    (unsigned short)sizeof(g_r401->control_info);
  
  /* 制御電文情報編集 */
  memmove( &g_r401->control_info.request_kind,               /* 要求種別 */
           &g_c401->control_info.request_kind,
           sizeof(g_r401->control_info.request_kind) );
  memmove( &g_r401->control_info.response_kind,              /* 応答種別 */
           DEF_CTLRSP_NOSEND,
           sizeof(g_r401->control_info.response_kind) );
  memmove( &g_r401->control_info.control_kind,               /* 制御電文種別 */
           &g_c401->control_info.control_kind,
           sizeof(g_r401->control_info.control_kind) );
  memmove( &g_r401->control_info.connection_lid,             /* コネクション論理ID */
           &g_c401->control_info.connection_lid,
           sizeof(g_r401->control_info.connection_lid) );
  memmove( g_r401->control_info.interface_name,             /* インタフェース名 */
           g_c401->control_info.interface_name,
           sizeof(g_r401->control_info.interface_name) );
  memmove( g_r401->control_info.station_name,               /* ステーション名 */
           g_c401->control_info.station_name,
           sizeof(g_r401->control_info.station_name) );
  memmove( &g_r401->control_info.denbun_log_key,             /* 電文ログKEY */
           &g_c401->control_info.denbun_log_key,
           sizeof(g_r401->control_info.denbun_log_key) );
  memmove( g_r401->control_info.mti,                        /* MTI */
           g_ctrl_info.send_mti,
           sizeof(g_r401->control_info.mti) );
  memmove( g_r401->control_info.req_gfp_lcn,                /* 仕向要求電文GFP内部LCN */
           g_c401->control_info.req_gfp_lcn,
           sizeof(g_r401->control_info.req_gfp_lcn) );
  /*② 「リプライ処理(CMIN_send_reply)」を呼び出してリプライする。 */
  CMIN_send_reply( (char *)g_ipcres_head,
                   (short)(sizeof(common_header_def) + sizeof(g_r401->control_info)),
                   0 );

} /* CNTF_receive_response */

/****************************************************************************/
/*  FUNCTION        : 4.12 CNTF_send_response_error                         */
/*  CALLING SEQ.    : void CNTF_send_response_error(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 応答電文送信不可処理                                  */
/****************************************************************************/
void CNTF_send_response_error(void)
{
  char            wkbuf1[64];
  char            wkbuf2[64];
  char            wkbuf3[64];
  char            wkbuf4[64];

  /* (1) 「メッセージ出力処理(CNTF_message_output)」を使用し、 */
  /*     送信エラーメッセージを出力する。 */
  memset( wkbuf1, 0x00, sizeof(wkbuf1) );
  memmove( wkbuf1,
           g_c401->control_info.send_naibu_err_code,
           sizeof(g_c401->control_info.send_naibu_err_code) );
  memset( wkbuf2, 0x00, sizeof(wkbuf2) );
  memmove( wkbuf2,
           g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
           sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
  memset( wkbuf3, 0x00, sizeof(wkbuf3) );
  memmove( wkbuf3,
           &g_c401->control_info.connection_lid,
           sizeof(g_c401->control_info.connection_lid) );
  memset( wkbuf4, 0x00, sizeof(wkbuf4) );
  memmove( wkbuf4,
           g_c401->control_info.mti,
           sizeof(g_c401->control_info.mti) );
  CMIN_message_output( DEF_EVT_SEND_ERR_NTF_RCV      /* メッセージ出力処理  */
                     , DEF_MSGTTKB_GYOM_ERR
                     , wkbuf1
                     , "@L@T@C"
                     , wkbuf2
                     , wkbuf3
                     , wkbuf4 );

  /* (2) 「CNTF_put_log(制御電文ログ出力処理)」を呼び出し、 */
  /*     制御電文ログ出力処理を行う。 */
  CNTF_put_log( DEF_HSMK_REP_SND_FUKA,
                g_c401->control_info.send_naibu_err_code,
                (short)g_ctrl_info.rcv_msg_len,
                (char *)&g_c401->data_bu );

  /* (3) NW電文受信応答リプライ */
  /* ヘッダ部編集 */
  memset( g_resp_buf, ' ', sizeof(g_resp_buf) );
  memmove( g_r401->common_header.interface_code,            /* インターフェースコード */
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(g_r401->common_header.interface_code) );
  g_r401->common_header.error_code = DEF_IPC_ERRCD_OK;      /* エラーコード */
  memcpy( g_r401->common_header.internal_error_code,        /* 内部エラーコード */
          DEF_NERR_NOMAL,
          sizeof(g_r401->common_header.internal_error_code) );
  g_r401->common_header.control_data_length = (unsigned short)sizeof(g_r401->control_info);
  
  /* 制御電文情報編集 */
  memmove( &g_r401->control_info.request_kind,               /* 要求種別 */
           &g_c401->control_info.request_kind,
           sizeof(g_r401->control_info.request_kind) );
  memmove( &g_r401->control_info.response_kind,              /* 応答種別 */
           DEF_CTLRSP_NOSEND,
           sizeof(g_r401->control_info.response_kind) );
  memmove( &g_r401->control_info.control_kind,               /* 制御電文種別 */
           &g_c401->control_info.control_kind,
           sizeof(g_r401->control_info.control_kind) );
  memmove( &g_r401->control_info.connection_lid,             /* コネクション論理ID */
           &g_c401->control_info.connection_lid,
           sizeof(g_r401->control_info.connection_lid) );
  memmove( &g_r401->control_info.interface_name,             /* インタフェース名 */
           &g_c401->control_info.interface_name,
           sizeof(g_r401->control_info.interface_name) );
  memmove( &g_r401->control_info.station_name,               /* ステーション名 */
           &g_c401->control_info.station_name,
           sizeof(g_r401->control_info.station_name) );
  memmove( &g_r401->control_info.denbun_log_key,             /* 電文ログKEY */
           &g_c401->control_info.denbun_log_key,
           sizeof(g_r401->control_info.denbun_log_key) );
  memset( g_r401->control_info.mti,                        /* MTI */
          ' ',
          sizeof(g_r401->control_info.mti) );
  memset( g_r401->control_info.req_gfp_lcn,                /* 仕向要求電文GFP内部LCN */
          ' ',
          sizeof(g_r401->control_info.req_gfp_lcn) );
  /*② 「リプライ処理(CMIN_send_reply)」を呼び出してリプライする。 */
  CMIN_send_reply( (char *)g_ipcres_head,
                   (short)(sizeof(common_header_def) + sizeof(g_r401->control_info)),
                   0 );

} /* CNTF_send_response_error */

/****************************************************************************/
/*  FUNCTION        : 4.13 CNTF_get_network_info                            */
/*  CALLING SEQ.    : void CNTF_get_network_info(short, char *,             */
/*                                               char *, db_gfnwi_def *)    */
/*  ARGUMENT        : short : 取得区分(1： グループ単位、                   */
/*                                     2： インタフェース単位、             */
/*                                     3： ステーション単位)                */
/*                    char *: インタフェース識別                            */
/*                    char *: ステーション識別                              */
/*                    db_gfnwi_def *: N/W情報レコード                       */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : N/W情報ファイル取得処理                                  */
/****************************************************************************/
void CNTF_get_network_info( short get_kbn,
                            char *if_id,
                            char *station_id,
                            db_gfnwi_def *db_gfnwi)
{
  short                s_result;
  t_gfnwi_pri_key_def  gfnwi_pkey;       /* NW情報ファイルプライマリKey */
  char                 wkbuf1[64];
  char                 wkbuf2[64];
  char                 wkbuf3[64];

/* ------------------------------------------------------ */
/* N/Wグループ情報取得                                    */
/* ------------------------------------------------------ */
  memset( db_gfnwi,    DEF_BUF_SPACE, sizeof(db_gfnwi_def) );
  memset( &gfnwi_pkey, DEF_BUF_SPACE, sizeof(gfnwi_pkey) );
  /* プライマリKey設定 */
  gfnwi_pkey.site_id = g_myinfo.site_id;
  gfnwi_pkey.nw_id   = g_myinfo.network_id;
  memmove( gfnwi_pkey.grp_id, g_myinfo.group_id, sizeof(gfnwi_pkey.grp_id) );
  if( get_kbn == DEF_GFNWI_GET_GP ) {
    memmove( gfnwi_pkey.if_id, DEF_IF_ID_DEFAULT, sizeof(gfnwi_pkey.if_id) );
  } else {
    memmove( gfnwi_pkey.if_id, if_id, sizeof(gfnwi_pkey.if_id) );
  }
  if( get_kbn == DEF_GFNWI_GET_ST ) {
    memmove( gfnwi_pkey.station_id, station_id, sizeof(gfnwi_pkey.station_id) );
  } else {
    memmove( gfnwi_pkey.station_id, DEF_STATION_ID_DEFAULT, sizeof(gfnwi_pkey.station_id) );
  }

  /* IOモジュールパラメータ初期化 */
  memset( &g_com_iom_arg_3 , DEF_BUF_NULL, sizeof(g_com_iom_arg_3  ));
  memset( &g_com_iom_arg_4 , DEF_BUF_NULL, sizeof(g_com_iom_arg_4  ));
  memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
  memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
  memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

  /* トレース情報設定 */
  memcpy( g_com_iom_arg_3.prog_id     , g_procinfo.my_pname
                                      , sizeof(g_com_iom_arg_3.prog_id     ));
  memcpy( g_com_iom_arg_3.file_id     , DEF_FL_NW_INFO
                                      , sizeof(g_com_iom_arg_3.file_id     ));
  memcpy( g_com_iom_arg_3.file_name   , g_com_file_data.nw_file_name
                                      , sizeof(g_com_iom_arg_3.file_name   ));
  memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                      , sizeof(g_com_iom_arg_3.file_io_type));

  /* ファイル情報設定 */
  memcpy( g_com_iom_arg_4.file_id     , DEF_FL_NW_INFO
                                      , sizeof(g_com_iom_arg_4.file_id  ));
  memcpy( g_com_iom_arg_4.file_name   , g_com_file_data.nw_file_name
                                      , sizeof(g_com_iom_arg_4.file_name));
  g_com_iom_arg_4.file_no             = g_com_file_data.nw_file_no;

  /* 入力情報 */
  g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
  g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
  g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
  memcpy( g_com_iom_arg_5.key_value   , (char *)&gfnwi_pkey
                                      , sizeof(gfnwi_pkey));
  memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                      , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
  g_com_iom_arg_5.key_len             = sizeof(gfnwi_pkey);
  g_com_iom_arg_5.compare_len         = sizeof(gfnwi_pkey);
  g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
  g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
  g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
  g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
  g_com_iom_arg_5.rec_len             = sizeof(db_gfnwi_def);

  s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                      , g_ch_sub_prog_sts
                      ,&g_com_iom_arg_3
                      ,&g_com_iom_arg_4
                      ,&g_com_iom_arg_5
                      ,&g_com_iom_arg_6);

  if( memcmp( g_ch_sub_prog_sts,
              DEF_COM_IOM_NO_ERR,
              sizeof(g_ch_sub_prog_sts) ) == 0 ) {/* IOモジュール結果判定 */
    memmove( (char *)db_gfnwi,
             g_com_iom_arg_6.rec_area,
             sizeof(db_gfnwi_def) );
  } else {
    memset( wkbuf1, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf1,
             g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
             sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
    memset( wkbuf2, 0x00, sizeof(wkbuf2) );
    memmove( wkbuf2,
             &g_c401->control_info.connection_lid,
             sizeof(g_c401->control_info.connection_lid) );
    memset( wkbuf3, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf3,
             g_com_iom_arg_5.key_value,
             sizeof(g_com_iom_arg_5.key_value) );
    CMIN_message_output ( DEF_EVT_FILE_IO_ERR     /* メッセージ出力処理   */
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_FILE_IO_ERR
                        , "@C@C@C@C@C@U"
                        , wkbuf1
                        , wkbuf2
                        , DEF_FL_NW_INFO
                        , DEF_COM_IOM_FUNC_STARTREAD
                        , wkbuf3
                        , g_com_iom_arg_6.guardian_errcode );
    CMIN_abend();                                 /* 異常終了            */
  }

} /* CNTF_get_network_info */

/****************************************************************************/
/*  FUNCTION        : 4.13 CNTF_put_log                                     */
/*  CALLING SEQ.    : void CNTF_put_log(short, char *, short, char *)       */
/*  ARGUMENT        : short : 処理区分(13：仕向応答電文受信、               */
/*                                     21：被仕向要求電文受信、             */
/*                                     22：被仕向応答電文送信、             */
/*                                     23：被仕向応答電文送信不可)          */
/*                    char *: 内部エラーコード                              */
/*                    short : 電文長                                        */
/*                    char *: 電文                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御電文ログ出力処理                                  */
/****************************************************************************/
void CNTF_put_log( short syori_kbn,
                   char *internal_error_code,
                   short msg_len,
                   char *mag )
{
  struct {
     char                            part_id[2];
     char                            lcn_id[15];
     char                            s_h_kubun;
     char                            send_recv_id;
  } wk_pri_key;
  db_glmlg_def      wk_glmlg;
  char              wk_s_h_kubun;
  char              wk_send_recv_id;
  short             s_result;
  char              wkbuf1[64];
  char              wkbuf2[64];
  char              wkbuf3[64];
  char              datetime_hex[64];   /* COM_UNQ用バッファ */
  char              szbuf[32];          /* 編集用バッファ */
  short             s_rec_len;

  /* (1) 追加・更新判定処理 */
  switch( syori_kbn ) {
  case DEF_SMK_REP_RCV:
    wk_s_h_kubun = DEF_S_H_KUBUN_SIMUKE;
    wk_send_recv_id = DEF_RSP_RCV;
    break;
  case DEF_HSMK_REQ_RCV:
    wk_s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
    wk_send_recv_id = DEF_REQ_SEND;
    break;
  case DEF_HSMK_REP_SND:
    wk_s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
    wk_send_recv_id = DEF_RSP_RCV;
    break;
  case DEF_HSMK_REP_SND_FUKA:
    wk_s_h_kubun = DEF_S_H_KUBUN_HISIMUKE;
    wk_send_recv_id = DEF_RSP_RCV;
    /* (2) 制御電文ログファイル更新処理 */
    /*  ① 検索キーを編集する。 */
    wk_pri_key.part_id[0] = '0';
    wk_pri_key.part_id[1] =
      g_c401->control_info.denbun_log_key.tran_id.gfp_lcn[14];
    memmove( &wk_pri_key.lcn_id,
             g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
             sizeof(wk_pri_key.lcn_id) );
    wk_pri_key.s_h_kubun = wk_s_h_kubun;
    wk_pri_key.send_recv_id = wk_send_recv_id;
    
    /*  ② 制御ログファイルを取得する */
    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_NULL, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_NULL, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_procinfo.my_pname
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CTRL_DEN_LOG
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.log_file_name
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                        , sizeof(g_com_iom_arg_3.file_io_type));

    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_FL_CTRL_DEN_LOG
                                        , sizeof(g_com_iom_arg_4.file_id  ));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.log_file_name
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.log_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memmove( g_com_iom_arg_5.key_value,
             (char *)&wk_pri_key, sizeof(wk_pri_key));
    memmove( g_com_iom_arg_5.key_type,
             DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(wk_pri_key);
    g_com_iom_arg_5.compare_len         = sizeof(wk_pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCK;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = db_glmlg_def_Size;
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);
    if( memcmp( g_ch_sub_prog_sts,
                DEF_COM_IOM_NO_ERR,
                sizeof(g_ch_sub_prog_sts) ) == 0 ) {/* IOモジュール結果判定 */
      memmove( (char *)&wk_glmlg,
               g_com_iom_arg_6.rec_area,
               sizeof(wk_glmlg) );
      s_rec_len = g_com_iom_arg_6.rec_len;
    } else {
      memset( wkbuf1, 0x00, sizeof(wkbuf1) );
      memmove( wkbuf1,
               g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
               sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
      memset( wkbuf2, 0x00, sizeof(wkbuf2) );
      memmove( wkbuf2,
               &g_c401->control_info.connection_lid,
               sizeof(g_c401->control_info.connection_lid) );
      memset( wkbuf3, 0x00, sizeof(wkbuf3) );
      memmove( wkbuf3,
               g_com_iom_arg_5.key_value,
               sizeof(g_com_iom_arg_5.key_value) );
      CMIN_message_output ( DEF_EVT_FILE_IO_ERR     /* メッセージ出力処理   */
                          , DEF_MSGTTKB_GYOM_ERR
                          , DEF_NERR_FILE_IO_ERR
                          , "@C@C@C@C@C@U"
                          , wkbuf1
                          , wkbuf2
                          , DEF_FL_CTRL_DEN_LOG
                          , DEF_COM_IOM_FUNC_STARTREAD
                          , wkbuf3
                          , g_com_iom_arg_6.guardian_errcode );
      CMIN_abend();                                 /* 異常終了            */
    }

    /*  ③ 制御電文ログレコードを編集する。 */
    memmove( wk_glmlg.send_naibu_err_code,
             g_c401->control_info.send_naibu_err_code,
             sizeof(wk_glmlg.send_naibu_err_code) );
    memmove( wk_glmlg.naibu_err_code,
             g_ctrl_info.internal_error_code,
             sizeof(wk_glmlg.naibu_err_code) );
    memmove( wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id,
             g_myinfo.serverclass_name,
             sizeof(wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id) );
    /* memmove( wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num,
             g_myinfo.serverclass_no,
             sizeof(wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num) ); */

    /*  ④ 制御ログファイルを更新する。 */
    memset( g_ch_sub_prog_sts, ' ', sizeof(g_ch_sub_prog_sts) );
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memmove( g_com_iom_arg_3.file_io_type,
             DEF_FILEIO_REWRITE,
             sizeof(g_com_iom_arg_3.file_io_type) );
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCKFREE;
    memmove( g_com_iom_arg_5.rec_area,
             (char *)&wk_glmlg,
             sizeof(wk_glmlg) );
    g_com_iom_arg_5.rec_len             = s_rec_len;
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);
    if( memcmp( g_ch_sub_prog_sts,
                DEF_COM_IOM_NO_ERR,
                sizeof(g_ch_sub_prog_sts) ) == 0 ) {/* IOモジュール結果判定 */
      return;
    } else {
      memset( wkbuf1, 0x00, sizeof(wkbuf1) );
      memmove( wkbuf1,
               g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
               sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
      memset( wkbuf2, 0x00, sizeof(wkbuf2) );
      memmove( wkbuf2,
               &g_c401->control_info.connection_lid,
               sizeof(g_c401->control_info.connection_lid) );
      memset( wkbuf3, 0x00, sizeof(wkbuf1) );
      memmove( wkbuf3,
               g_com_iom_arg_5.key_value,
               sizeof(g_com_iom_arg_5.key_value) );
      CMIN_message_output ( DEF_EVT_FILE_IO_ERR     /* メッセージ出力処理   */
                          , DEF_MSGTTKB_GYOM_ERR
                          , DEF_NERR_FILE_IO_ERR
                          , "@C@C@C@C@C@U"
                          , wkbuf1
                          , wkbuf2
                          , DEF_FL_CTRL_DEN_LOG
                          , DEF_COM_IOM_FUNC_UPDATE
                          , wkbuf3
                          , g_com_iom_arg_6.guardian_errcode );
      CMIN_abend();                                 /* 異常終了            */
    }

    break;
  default:
    memset( wkbuf1, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf1,
             g_ctrl_info.internal_error_code,
             sizeof(g_ctrl_info.internal_error_code) );
    memset( wkbuf2, 0x00, sizeof(wkbuf2) );
    memmove( wkbuf2,
             g_procinfo.my_pname,
             sizeof(g_procinfo.my_pname) );
    CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR
                        , DEF_MSGTTKB_GYOM_ERR
                        , wkbuf1
                        , "@C@U"
                        , wkbuf2
                        , DEF_RET_NG );
      CMIN_abend();                                 /* 異常終了            */
  }

  /* (3) 制御電文ログファイル登録処理 */
  /*  ① 制御電文ログレコードを編集する。 */
  memset( &wk_glmlg, ' ', sizeof(wk_glmlg) );
  wk_glmlg.pri_key.part_id[0] = '0';              /* パーティションID */
  wk_glmlg.pri_key.part_id[1] =
    g_c401->control_info.denbun_log_key.tran_id.gfp_lcn[14];
  memmove( &wk_glmlg.pri_key.lcn_id,              /* LCN */
           g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
           sizeof(wk_glmlg.pri_key.lcn_id) );
  wk_glmlg.pri_key.s_h_kubun = wk_s_h_kubun;      /* 仕向・被仕向区分 */
  wk_glmlg.pri_key.send_recv_id = wk_send_recv_id;/* 送受信識別 */
  memmove ( &wk_glmlg.connect_id,                  /* コネクション論理ID */
            &g_c401->control_info.connection_lid,
            sizeof(wk_glmlg.connect_id) );
  if( syori_kbn == DEF_HSMK_REP_SND ) {
    memmove( wk_glmlg.mti_id,                      /* MTI */
             g_ctrl_info.send_mti,
             sizeof(wk_glmlg.mti_id) );
    memmove( wk_glmlg.control_kind,                /* 制御電文種別 */
             g_ctrl_info.send_mti,
             sizeof(wk_glmlg.control_kind) );
    memset( wk_glmlg.send_naibu_err_code,         /* 送信不可情報 */
            ' ', sizeof(wk_glmlg.send_naibu_err_code) );
  } else {
    memmove( wk_glmlg.mti_id,                      /* MTI */
             g_c401->control_info.mti,
             sizeof(wk_glmlg.mti_id) );
    memmove( wk_glmlg.control_kind,                /* 制御電文種別 */
             &g_c401->control_info.control_kind,
             sizeof(wk_glmlg.control_kind) );
    memmove( wk_glmlg.send_naibu_err_code,         /* 送信不可情報 */
             g_c401->control_info.send_naibu_err_code,
             sizeof(wk_glmlg.send_naibu_err_code) );
  }
  memmove( wk_glmlg.naibu_err_code,               /* 内部エラーコード */
           g_ctrl_info.internal_error_code,
           sizeof(wk_glmlg.naibu_err_code) );
  memmove( wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id,/* 制御電文サーバクラス情報 */
           g_myinfo.serverclass_name,
           sizeof(wk_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id) );
  memset(szbuf, 0x00, sizeof(szbuf));             /* 共通制御・送信日時 */
  memset(datetime_hex, 0x00, sizeof(datetime_hex));
  COM_UNQ((COM_UNQ_arg_1_def *)szbuf, datetime_hex);
  MCR_CCUT_CPY(wk_glmlg.entry_timestamp, szbuf);
  wk_glmlg.denbun_info_exist = DEF_DENBUN_IFO_ON; /* 電文情報有無 */
  sprintf( wk_glmlg.denbun_area.denbun_len,       /* 電文長 */
           "%05d", g_ctrl_info.rcv_msg_len );
  memmove( wk_glmlg.denbun_area.denbun,           /* 電文 */
           g_c401->data_bu.message_text,
            g_ctrl_info.rcv_msg_len );
  s_rec_len = g_ctrl_info.rcv_msg_len + DEF_GLMLG_DENBUN_OFFSET;

  /*  ② 制御ログファイルへ書き込む。 */
  memset( &g_com_iom_arg_3 , DEF_BUF_NULL, sizeof(g_com_iom_arg_3  ));
  memset( &g_com_iom_arg_4 , DEF_BUF_NULL, sizeof(g_com_iom_arg_4  ));
  memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
  memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
  memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

  /* トレース情報設定 */
  memcpy( g_com_iom_arg_3.prog_id     , g_procinfo.my_pname
                                      , sizeof(g_com_iom_arg_3.prog_id     ));
  memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CTRL_DEN_LOG
                                      , sizeof(g_com_iom_arg_3.file_id     ));
  memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.log_file_name
                                      , sizeof(g_com_iom_arg_3.file_name   ));
  memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_WRITE
                                      , sizeof(g_com_iom_arg_3.file_io_type));

  /* ファイル情報設定 */
  memcpy( g_com_iom_arg_4.file_id     , DEF_FL_CTRL_DEN_LOG
                                      , sizeof(g_com_iom_arg_4.file_id  ));
  memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.log_file_name
                                      , sizeof(g_com_iom_arg_4.file_name));
  g_com_iom_arg_4.file_no             = g_kbt_file_data.log_file_no;

  /* 入力情報 */
  g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
  g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
  g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
  memset( g_com_iom_arg_5.key_value, ' ', sizeof(g_com_iom_arg_5.key_value));
  memset( g_com_iom_arg_5.key_type, ' ', sizeof(g_com_iom_arg_5.key_type));
  g_com_iom_arg_5.key_len             = 0;
  g_com_iom_arg_5.compare_len         = 0;
  g_com_iom_arg_5.positioning_mode    = 0;
  g_com_iom_arg_5.lock_flg            = 0;
  g_com_iom_arg_5.asc_desc_type       = 0;
  g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
  g_com_iom_arg_5.rec_len             = s_rec_len;
  memmove( g_com_iom_arg_5.rec_area, &wk_glmlg, db_glmlg_def_Size );
  s_result = COM_IOM ( DEF_COM_IOM_FUNC_ADD
                      , g_ch_sub_prog_sts
                      ,&g_com_iom_arg_3
                      ,&g_com_iom_arg_4
                      ,&g_com_iom_arg_5
                      ,&g_com_iom_arg_6);
  if( memcmp( g_ch_sub_prog_sts,
              DEF_COM_IOM_NO_ERR,
              sizeof(g_ch_sub_prog_sts) ) == 0 ) {/* IOモジュール結果判定 */
    return;
  } else {
    memset( wkbuf1, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf1,
             g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
             sizeof(g_c401->control_info.denbun_log_key.tran_id.gfp_lcn) );
    memset( wkbuf2, 0x00, sizeof(wkbuf2) );
    memmove( wkbuf2,
             &g_c401->control_info.connection_lid,
             sizeof(g_c401->control_info.connection_lid) );
    wk_pri_key.part_id[0] = '0';
    wk_pri_key.part_id[1] =
      g_c401->control_info.denbun_log_key.tran_id.gfp_lcn[14];
    memmove( &wk_pri_key.lcn_id,
             g_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
             sizeof(wk_pri_key.lcn_id) );
    wk_pri_key.s_h_kubun = wk_s_h_kubun;
    wk_pri_key.send_recv_id = wk_send_recv_id;
    memset( wkbuf3, 0x00, sizeof(wkbuf1) );
    memmove( wkbuf3, &wk_pri_key, sizeof(wk_pri_key) );
    CMIN_message_output ( DEF_EVT_FILE_IO_ERR     /* メッセージ出力処理   */
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_FILE_IO_ERR
                        , "@C@C@C@C@C@U"
                        , wkbuf1
                        , wkbuf2
                        , DEF_FL_CTRL_DEN_LOG
                        , DEF_COM_IOM_FUNC_ADD
                        , wkbuf3
                        , g_com_iom_arg_6.guardian_errcode );
    CMIN_abend();                                 /* 異常終了            */
  }

} /* CNTF_put_log */

/****************************************************************************/
/*  FUNCTION        : 4.13 CNTF_put_errlog                                  */
/*  CALLING SEQ.    : void CNTF_put_errlog(char, char *)                    */
/*  ARGUMENT        : char  : エラー電文識別                                */
/*                    char *: 内部エラーコード                              */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : エラー出力ログ出力処理                                */
/****************************************************************************/
void CNTF_put_errlog( char err_kbn, char *internal_errcode )
{
    db_glelg_def    glelg_rec;
    short           wklen;
    char            wkbuf1[10];
    short           s_result;

    /* エラー出力ログ編集 */
    memset( (char *)&glelg_rec, DEF_BUF_SPACE, sizeof(glelg_rec));
    glelg_rec.err_denbun_id = err_kbn;
    memcpy(glelg_rec.mti_id, g_c401->control_info.mti, sizeof(g_c401->control_info.mti));
    memcpy(glelg_rec.naibu_err_code, internal_errcode, sizeof(glelg_rec.naibu_err_code));
    
    // 電文送受信情報
    memcpy(glelg_rec.srv_cls_info.srv_cls_id, g_myinfo.serverclass_name, sizeof(glelg_rec.srv_cls_info.srv_cls_id));
    memcpy(glelg_rec.srv_cls_info.srv_cls_mlt_num, "0000", sizeof(glelg_rec.srv_cls_info.srv_cls_mlt_num));
    memcpy(glelg_rec.denbun_send_recv_info.nw_kubun, g_myinfo.nw_kbn, sizeof(g_myinfo.nw_kbn));
    memcpy(glelg_rec.denbun_send_recv_info.mti_id, g_c401->control_info.mti, sizeof(g_c401->control_info.mti));
    glelg_rec.denbun_send_recv_info.send_denbun_shubetu = DEF_SEND_DENBUN_OTH;
    memcpy((char *)&glelg_rec.denbun_send_recv_info.denbun_log_key, (char *)&g_c401->control_info.denbun_log_key, sizeof(g_c401->control_info.denbun_log_key));
    glelg_rec.denbun_send_recv_info.denbun_fmt_kubun = DEF_DENBUN_FMT_8583;

    // 通信制御情報
    memcpy(glelg_rec.tushin_cntrl_info.if_id, g_c401->control_info.interface_name, sizeof(g_c401->control_info.interface_name));
    memcpy(glelg_rec.tushin_cntrl_info.station_id, g_c401->control_info.station_name, sizeof(g_c401->control_info.station_name));
    memcpy(glelg_rec.tushin_cntrl_info.line_info.recv_connect_id, (char *)&g_c401->control_info.connection_lid, sizeof(g_c401->control_info.connection_lid));

    // 電文部
    wklen = (short)g_ctrl_info.rcv_msg_len;
    memset(wkbuf1, 0x00, sizeof(wkbuf1));
    sprintf(wkbuf1, "%05d", wklen);
    memcpy(&glelg_rec.denbun_area.denbun_len, wkbuf1, strlen(wkbuf1));
    memcpy(&glelg_rec.denbun_area.mti_start_lct, g_db_gfnwi.denbun_item_lct_info.mti_start_lct, sizeof(g_db_gfnwi.denbun_item_lct_info.mti_start_lct));
    memcpy(&glelg_rec.denbun_area.denbun, g_c401->data_bu.message_text, wklen);
    wklen = (sizeof(glelg_rec) - sizeof(glelg_rec.denbun_area.denbun)) + wklen;

    // エラー出力ログ出力
    memset( &g_com_erl_arg_1, DEF_BUF_NULL , sizeof(g_com_erl_arg_1) );
    memset( &g_cg010in_modle.emsinf, DEF_BUF_SPACE, sizeof(g_cg010in_modle.emsinf) );
    memset( &g_com_erl_arg_3, DEF_BUF_SPACE, sizeof(g_com_erl_arg_3) );
    g_com_erl_arg_1.file_io_type = DEF_COM_ERL_ARG1_WRITE;
    g_com_erl_arg_1.io_timer     = g_myinfo.send_timer;
    g_com_erl_arg_1.data_len     = wklen;
    g_com_erl_arg_1.data_area    = (char *)&glelg_rec;
    s_result = COM_ERL( &g_com_erl_arg_1                   /* エラーログ共通処理実行 */
                      , &g_com_erl_arg_2
                      , &g_cg010in_modle
                      , &g_com_erl_arg_3
                      , g_myinfo.prog_id );
    if ( s_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR        /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@U"
                            , "COM_ERL"
                            , s_result );
        CMIN_abend();                                       /* 異常終了            */
    }
} /* CNTF_put_errlog */
