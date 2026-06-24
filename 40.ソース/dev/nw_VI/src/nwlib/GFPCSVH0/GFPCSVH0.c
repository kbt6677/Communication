/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSVH0                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文送信時局状態判定(VISA)                  */
/*                                 送信電文種別と局状態から                  */
/*                                 電文送信可/不可の判定を行う。             */
/*        AUTHER            ････ HAS T.Sugisaki                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-12-04                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Sugisaki 2024/12/04 (J0680)新規作成                               */
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

/* USER HEADER     */
#include "common.h"
#include "NWM_NTE.h"
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_NTE                                        */
/*  CALLING SEQ.    : void   NWM_NTE(char , char *)                         */
/*  ARGUMENT        : 1.snd_msg_type   (I)   送信電文種別                   */
/*                  : 2.log_key_type   (I)   電文ログKEY電文種別            */
/*                  : 3.send_cen_sts   (I)   局状態                         */
/*  RETURN CODE     : 1.送信可                                              */
/*                    0.送信不可                                            */
/*                    2.電文種別異常                                        */
/*                    3.局状態異常                                          */
/*  DESCRIPTION     : 電文送信可/不可の判定                                 */
/****************************************************************************/
short    NWM_NTE(  char  snd_msg_type
                 , char  log_key_type
                 , char *send_cen_sts)
{

    //変数定義
    short   s_rtn_cd;         //戻り値

    switch(snd_msg_type)
    {
        case DEF_SEND_DENBUN_REQ:
            //----------------------------------------------------------------//
            // 業務要求
            //----------------------------------------------------------------//
            if( memcmp(send_cen_sts, DEF_STTE_STS_OPNING , DEF_SEND_DEN_TYP_LEN) == 0 ||
                memcmp(send_cen_sts, DEF_STTE_STS_CLS    , DEF_SEND_DEN_TYP_LEN) == 0 ||
                memcmp(send_cen_sts, DEF_STTE_STS_CLOSING, DEF_SEND_DEN_TYP_LEN) == 0 ){
                // "11"：開局処理中
                // "91"：閉局処理中
                // "90"：閉局
                //       送信不可
                s_rtn_cd = DEF_HDE_SEND_NG;
            }else if(memcmp(send_cen_sts, DEF_STTE_STS_OPN, DEF_SEND_DEN_TYP_LEN) == 0){
                // "10"：開局
                //       送信可
                s_rtn_cd = DEF_HDE_SEND_OK;
            }else{
                // その他・局状態異常
                s_rtn_cd = DEF_HDE_SEND_CEN_ERR;
            }
            break;

        case DEF_SEND_DENBUN_RSP:
        case DEF_SEND_DENBUN_CTR_REQ:
        case DEF_SEND_DENBUN_CTR_RSP:
        case DEF_SEND_DENBUN_HB:
        case DEF_SEND_DENBUN_OTH:
           //----------------------------------------------------------------//
            // 制御要求
            // 業務応答
            // 業務応答
            // 制御応答
            // 破棄通知
            //----------------------------------------------------------------//
            if( memcmp(send_cen_sts, DEF_STTE_STS_OPN    , DEF_SEND_DEN_TYP_LEN) == 0 ||
                memcmp(send_cen_sts, DEF_STTE_STS_OPNING , DEF_SEND_DEN_TYP_LEN) == 0 ||
                memcmp(send_cen_sts, DEF_STTE_STS_CLS    , DEF_SEND_DEN_TYP_LEN) == 0 ||
                memcmp(send_cen_sts, DEF_STTE_STS_CLOSING, DEF_SEND_DEN_TYP_LEN) == 0 ){
                // "11"：開局処理中
                // "10"：開局
                // "91"：閉局処理中
                // "90"：閉局
                //       送信可
                s_rtn_cd = DEF_HDE_SEND_OK;
            }else{
                // その他・局状態異常
                s_rtn_cd = DEF_HDE_SEND_CEN_ERR;
            }
            break;

        default:                        // その他
            // 電文種別異常
            s_rtn_cd = DEF_HDE_SEND_TYP_ERR;
        break;
    }

    return s_rtn_cd;
}
/* end of NWM_NTE */
