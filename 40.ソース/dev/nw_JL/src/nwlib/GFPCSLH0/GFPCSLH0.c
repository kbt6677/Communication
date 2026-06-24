/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSLH0                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文送信時局状態判定(J-Link)                */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-25                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/04/25 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                            */
/*****************************************************************************/
#pragma ENV COMMON

/*****************************************************************************/
/*   INCLUDE定義                                                             */
/*****************************************************************************/
/* STANDARD HEADER */
#include <stdio.h>
#include <string.h>

/* USER HEADER     */
#include "NWM_NTE.h"
#include "common.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_NTE                                        */
/*  CALLING SEQ.    : short  NWM_NTE(char ,char ,char *)                    */
/*  ARGUMENT        : 1.送信電文種別                                        */
/*  ARGUMENT        : 2.制御電文種別                                        */
/*  ARGUMENT        : 3.局状態                                              */
/*  RETURN CODE     : 判定結果                                              */
/*  DESCRIPTION     : 送信電文種別と局状態から電文送信可/不可の判定を行う   */
/****************************************************************************/
short  NWM_NTE(char msg_type
            ,  char c_ctltext_type
            ,  char *station_state)
{
    switch(msg_type){
    case DEF_SEND_DENBUN_REQ: // 送信電文'1':業務要求
        // 局状態"10":開局
        if(memcmp(station_state,DEF_STTE_STS_OPN,DEF_SEND_DEN_TYP_LEN) == 0){
            return DEF_HDE_SEND_OK; // 送信可
        }
        // 局状態"11" or "90" or "91"
        if((memcmp(station_state,DEF_STTE_STS_OPNING,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLS,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLOSING,DEF_SEND_DEN_TYP_LEN) == 0)){
            return DEF_HDE_SEND_NG; // 送信不可
        }
        return DEF_HDE_SEND_CEN_ERR; // 局状態異常

    case DEF_SEND_DENBUN_CTR_REQ: // 送信電文'3':制御要求
        // 制御電文種別"1"開局 or "2"閉局
        if((c_ctltext_type == DEF_CTLTXT_CNT_OPN)
        || (c_ctltext_type == DEF_CTLTXT_CNT_CLS)){
            // 局状態"10" or "11" or "90" or "91"
            if((memcmp(station_state,DEF_STTE_STS_OPN,DEF_SEND_DEN_TYP_LEN) == 0)
            || (memcmp(station_state,DEF_STTE_STS_OPNING,DEF_SEND_DEN_TYP_LEN) == 0)
            || (memcmp(station_state,DEF_STTE_STS_CLS,DEF_SEND_DEN_TYP_LEN) == 0)
            || (memcmp(station_state,DEF_STTE_STS_CLOSING,DEF_SEND_DEN_TYP_LEN) == 0)){
                return DEF_HDE_SEND_OK; // 送信可
            }
            return DEF_HDE_SEND_CEN_ERR; // 局状態異常
        }
        // 制御電文種別"5"鍵交換 or "3"エコー
        // 局状態"10"：開局
        if((c_ctltext_type == DEF_CTLTXT_KEY_EXC)
        || (c_ctltext_type == DEF_CTLTXT_ECH_SND)){

            if(memcmp(station_state,DEF_STTE_STS_OPN,DEF_SEND_DEN_TYP_LEN) == 0){
                return DEF_HDE_SEND_OK; // 送信可
            }
            // 局状態"11" or "90" or "91"
            if((memcmp(station_state,DEF_STTE_STS_OPNING,DEF_SEND_DEN_TYP_LEN) == 0)
            || (memcmp(station_state,DEF_STTE_STS_CLS,DEF_SEND_DEN_TYP_LEN) == 0)
            || (memcmp(station_state,DEF_STTE_STS_CLOSING,DEF_SEND_DEN_TYP_LEN) == 0)){
                return DEF_HDE_SEND_NG; // 送信不可
            }
            return DEF_HDE_SEND_CEN_ERR; // 局状態異常
        }
        return DEF_HDE_SEND_TYP_ERR; // 送信電文種別異常

    case DEF_SEND_DENBUN_RSP: // 送信電文'2':業務応答
    case DEF_SEND_DENBUN_CTR_RSP: // 送信電文'4':制御応答
        // 局状態"10" or "11" or "90" or "91"
        if((memcmp(station_state,DEF_STTE_STS_OPN,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_OPNING,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLS,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLOSING,DEF_SEND_DEN_TYP_LEN) == 0)){
            return DEF_HDE_SEND_OK; // 送信可
        }
        return DEF_HDE_SEND_CEN_ERR; // 局状態異常

    case DEF_SEND_DENBUN_OTH:
        // 局状態"10" or "11" or "90" or "91"
        if((memcmp(station_state,DEF_STTE_STS_OPN,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_OPNING,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLS,DEF_SEND_DEN_TYP_LEN) == 0)
        || (memcmp(station_state,DEF_STTE_STS_CLOSING,DEF_SEND_DEN_TYP_LEN) == 0)){
            return DEF_HDE_SEND_OK; // 送信可
        }
        return DEF_HDE_SEND_CEN_ERR; // 局状態異常
    default:
        return DEF_HDE_SEND_TYP_ERR; // 送信電文種別異常
    }
}
/* end of NWM_NTE */
