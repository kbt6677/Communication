/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSV00                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               ヘッダーレイアウトチェック処理(VisaNet)     */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-01-20                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/01/20 (J0680)新規作成                               */
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
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist

/* USER HEADER     */
#include "msg_VI.h"
#include "NWM_MTI.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_MTI                                        */
/*  CALLING SEQ.    : short NWM_MTI(char*,short,short,short*,char*,short*)  */
/*  ARGUMENT        : 1.recv_msg        (I)   受信電文                      */
/*                  : 2.recv_len        (I)   受信電文長                    */
/*                  : 3.i_data_offset   (I)   電文データ開始位置(入力)      */
/*                  : 4.o_data_offset   (O)   電文データ開始位置(判定結果)  */
/*                  : 5.hb_resp_send    (O)   送信電文(ハートビート応答)    */
/*                  : 6.s_hb_resp_leng  (O)   送信電文長(ハートビート応答)  */
/*  RETURN CODE     :  1 :一般電文                                          */
/*                  :  2 :リジェクトメッセージ                              */
/*                  :  3 :ハートビートメッセージ                            */
/*                  :  4 :アイドルメッセージ正常                            */
/*                  : -1 :電文不正                                          */
/*  DESCRIPTION     : ヘッダーレイアウトのチェックを行う                    */
/****************************************************************************/
short NWM_MTI(char   *recv_msg
             ,short   recv_len
             ,short   i_data_offset
             ,short  *o_data_offset
             ,char   *hb_resp_send
             ,short  *s_hb_resp_leng)
{
    short s_ret_code = 0;
    MSG_VMLH_VISA_def           *t_recv_header;
    MSG_HEADER_REJECT_VISA_def  *t_recv_msg;

    /* 電文データ開始位置（判定結果）に電文データ開始位置（入力値）を設定する */
    *o_data_offset = i_data_offset;

    /* VMLHの構造体ポインタに 受信電文の先頭アドレスを設定 */
    t_recv_header = (MSG_VMLH_VISA_def *)recv_msg;

    /* Message Headerの構造体ポインタに  受信電文の先頭アドレス + VMLHヘッダ長を設定 */
    t_recv_msg = (MSG_HEADER_REJECT_VISA_def *)(recv_msg + MSG_VMLH_VISA_def_Size);

    /* 受信電文長がVMLHヘッダ長以上であることを確認 */
    if (recv_len < MSG_VMLH_VISA_def_Size ){
        /* 3Byte以下 */
        /* 電文不正 */
        s_ret_code = DEF_NWM_MTI_RTN_INVALID;
    }
    /* 電文長確認 */
    else if (t_recv_header->vmlh_msg_len == DEF_VI_VMLH_LENG_ZERO){
        /* 電文長が0 */
        /* 判定結果（戻り値）に「4：アイドルメッセージ」を設定 */
        s_ret_code = DEF_NWM_MTI_RTN_IDLE;
        /* 電文データ開始位置（判定結果）に「1」を設定 */
        *o_data_offset = NWM_DATA_START_POSI;
    }
    else if (t_recv_header->vmlh_msg_fmt_plt == DEF_VI_VMLH_FORM_HB_20){
        /* Message Format and Platformが0x20(ハートビート) */
        /* 判定結果（戻り値）に「3：ハートビートメッセージ」を設定 */
        s_ret_code = DEF_NWM_MTI_RTN_HEARTBEAT;
        /* ハートビート応答電文作成 */
        memcpy( hb_resp_send, recv_msg, recv_len);
        /* ハートビート応答電文長設定 */
        *s_hb_resp_leng = recv_len;
        /* 電文データ開始位置（判定結果）に「1」を設定 */
        *o_data_offset = NWM_DATA_START_POSI;
    }
    /* 受信電文長が30(リジェクトメッセージのVMLH + Message Headerのサイズ)以上かつ */
    /*  Message HeaderのHeader Flag and Format が x'81'(リジェクト)                */
    else if ((recv_len >= MSG_HEADER_REJDATA_LENG ) &&
             (t_recv_msg->mh_hdr_flg_fmt == DEF_VI_HD_FLGFRM_RJ_81)){
        /* 判定結果（戻り値）に「2：リジェクトメッセージ」を設定 */
        s_ret_code = DEF_NWM_MTI_RTN_REJECT;
        /* 電文データ開始位置（判定結果）に「1」を設定 */
        *o_data_offset = NWM_DATA_START_POSI;
    }
    /* 受信電文長が 28(一般電文のVMLH + Message Header + MTI のサイズ)以上であることを確認 */
    else if (recv_len >= MSG_HEADER_MTI_LENG ){
        /* 判定結果（戻り値）に「1：一般電文」」を設定 */
        s_ret_code = DEF_NWM_MTI_RTN_GENERAL;
    }
    else{
        /* 電文不正 */
        s_ret_code = DEF_NWM_MTI_RTN_INVALID;
    }

    return(s_ret_code);
}
/* end of NWM_MTI */
