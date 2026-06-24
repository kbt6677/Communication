/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSV30                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文ヘッダ編集(VisaNet)                     */
/*                                 指定された編集前電文に必要な場合          */
/*                                 VMLHを付与して編集後電文に設定を行う。    */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-01-24                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/01/24 (J0680)新規作成                               */
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
#include "common.h"
#include "msg_VI.h"
#include "NWM_HDE.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_HDE                                        */
/*  CALLING SEQ.    : void   NWM_HDE(strcut *, struct *, short, short)      */
/*  ARGUMENT        : 1.msg_in         (I)   編集前電文情報                 */
/*                  : 2.msg_out        (O)   編集後電文情報                 */
/*                  : 3.start_position (I)   電文開始位置                   */
/*                  : 4.send_msg_type  (I)   送信電文種別                   */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : 電文ヘッダの編集を行う                                */
/****************************************************************************/
void  NWM_HDE(NWM_HDE_arg_1_def *msg_in
            , NWM_HDE_arg_2_def *msg_out
            , short              start_position
            , short              send_msg_type)
{
    /* 型変換 インタフェースメッセージ */
    MSG_VMLH_VISA_def   *visa_vmlh  = (MSG_VMLH_VISA_def *)msg_out->msg_out_addr;
    MSG_HEADER_VISA_def *msg_header = (MSG_HEADER_VISA_def *)(msg_out->msg_out_addr + MSG_VMLH_VISA_def_Size);

    /* 送信電文種別確認 */
    if ( send_msg_type == DEF_SEND_DENBUN_HB ){
        /* ハートビートの場合 */
        memcpy(msg_out->msg_out_addr ,msg_in->msg_in_addr ,msg_in->msg_in_len);
        msg_out->msg_out_len = msg_in->msg_in_len;
    }
    else {
        /* VMLHのMessage Lengthを設定 */
        visa_vmlh->vmlh_msg_len = msg_in->msg_in_len;
        /* VMLHのReservedを設定 */
        visa_vmlh->vmlh_rsv = 0x00;
        /* VMLHのMessage Format and Platformを設定 */
        visa_vmlh->vmlh_msg_fmt_plt = 0x00;

        memcpy(&msg_header->mh_hdr_len ,msg_in->msg_in_addr ,msg_in->msg_in_len);
        /* 編集後電文長に編集前電文長 + VMLHヘッダ長 を設定 */
        msg_out->msg_out_len = msg_in->msg_in_len + MSG_VMLH_VISA_def_Size;
    }
    return;
}
/* end of NWM_HDE */
