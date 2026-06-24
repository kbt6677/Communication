/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU00                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               ヘッダーレイアウトチェック処理(UnionPay)    */
/*        AUTHER            ････ HAS Matsumoto                               */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-16                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Matsumoto  2025/09/16 (J0680)新規作成                               */
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
#include <string.h>   nolist

/* USER HEADER     */
#include "msg_UP.h"
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
    #define MSGLEN_SIZE      4
    short                    s_ret_code = 0;
    MSG_HEADER_UNIONPAY_def *t_recv_header;

    *o_data_offset = i_data_offset;

    /* 受信電文長サイズチェック */
    if (recv_len < MSGLEN_SIZE) {
        s_ret_code = DEF_NWM_MTI_RTN_INVALID; /* 電文長不正 */
        return(s_ret_code);
    }

    /* アイドルメッセージチェック/Message Length="0000" */
    if (memcmp(recv_msg, "0000", MSGLEN_SIZE)==0) {
        s_ret_code = DEF_NWM_MTI_RTN_IDLE; /* アイドルメッセージ */
        *o_data_offset = NWM_DATA_START_POSI;
        return(s_ret_code);
    }

    /* ヘッダ部長チェック */
    if (recv_len < (MSG_HEADER_UNIONPAY_def_Size + MSGLEN_SIZE)) {
        s_ret_code = DEF_NWM_MTI_RTN_INVALID; /* 電文長不正 */
        return(s_ret_code);
    }

    /* リジェクトコード判定 */
    t_recv_header = (MSG_HEADER_UNIONPAY_def *)(recv_msg + MSGLEN_SIZE);
    if (memcmp(t_recv_header->mh_rjct_code, "00000", sizeof(t_recv_header->mh_rjct_code))==0) {
        s_ret_code = DEF_NWM_MTI_RTN_GENERAL; /* 一般電文 */
    } else {
        s_ret_code = DEF_NWM_MTI_RTN_REJECT; /* リジェクトメッセージ */
        *o_data_offset = NWM_DATA_START_POSI;
    }
    return(s_ret_code);
}
/* end of NWM_MTI */
