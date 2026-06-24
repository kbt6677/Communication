/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSLC0                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               通知電文個別処理(J-Link)                    */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-12                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/03/12 (J0680)新規作成                               */
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
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <ctype.h>    nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h"
#include "NWM_NTC.h"
#include "msg_JL.h"
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_CTLTEXT_REQ         "51A1"          // 制御電文識別:Administrative要求電文
#define DEF_CTLTEXT_RES         "52A1"          // 制御電文識別:Administrative応答電文
#define DEF_ADMIN_DISABLE       '0'             // 応答電文編集要否判定：電文なし
#define DEF_FLG_OFF             false           // 固定フォーマットフラグオフ
#define DEF_CTLTEXT_SIZE        4               // 制御電文識別length
#define DEF_MTI_EBC_0630        0xf0f6f3f0      // MTI:0630 バイナリ
#define DEF_MTI_ASC_0630        "0630"          // MTI:0630 アスキー
#define DEF_MTI_SIZE            4               // MTIlength
#define DEF_BIT7_SIZE           10              // 固定フォーマットBit7length
#define DEF_BIT11_SIZE          6               // 固定フォーマットBit11length
#define DEF_BIT48_SIZE          255             // 固定フォーマットBit48length
#define DEF_BIT48_FLGOFF_SIZE   0x0000          // 固定フォーマットBit48のlength設定値

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_NTC_msg_check                              */
/*  CALLING SEQ.    : short  NWM_NTC_msg_check(char *,char *,struct *)      */
/*  ARGUMENT        : 1.c_ctltext_type  (I) 制御電文種別                    */
/*  ARGUMENT        : 2.precv           (I) 固定フォーマット                */
/*  ARGUMENT        : 3.request_data_len(I) リクエストデータ長              */
/*  ARGUMENT        : 4.receive_data_len(I) 受信電文長                      */
/*  ARGUMENT        : 5.nw_info         (I) N/W情報ファイル                 */
/*  RETURN CODE     : 1     通知電文                                        */
/*                  : 2     要求応答型要求電文                              */
/*                  : 3     要求応答型応答電文                              */
/*                  : 9     精査異常                                        */
/*  DESCRIPTION     : 通知電文内容を精査する                                */
/****************************************************************************/
short NWM_NTC_msg_check(
              char *c_ctltext_type
            , char *precv
            , short request_data_len
            , short receive_data_len
            , db_gfnwi_def *nw_info)
{
    // リクエストデータのデータ部を固定フォーマットに展開
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0620 *data = (MTI_0620*)&pt_fixedform_jlink->ffd;

    // 電文種別判定、応答有無判別
    // 52A1：Administrative応答電文
    if (memcmp(c_ctltext_type,DEF_CTLTEXT_REQ,DEF_CTLTEXT_SIZE) != 0){
        return DEF_RTN_RECEIVE_RESPONSE; // 要求応答型応答電文でreturn
    }
    // 51A1：Administrative要求電文
    // 応答なし
    if (nw_info->shori_kbn_info.admin_denbun_res_need == DEF_ADMIN_DISABLE){
        return DEF_RTN_RECEIVE_NOTICE; // 通知電文でreturn
    }
    // 応答あり
    // 電文長精査
    if(sizeof(MTI_0620) + DEF_MTI_SIZE > request_data_len){
        return DEF_RTN_ABNORMAL_SCRUTINY; // 精査異常でreturn
    }
    // Bit7精査:フラグON
    if(data->bit7.flag){
        // 送信日時精査
        for (int i=0;i<DEF_BIT7_SIZE;i++){
            if(!isdigit(data->bit7.data[i])){ // 数値じゃなかったら
                return DEF_RTN_ABNORMAL_SCRUTINY; // 精査異常でreturn
            }
        }
    // Bit7精査:フラグOFF
    }else{
        return DEF_RTN_ABNORMAL_SCRUTINY; // 精査異常でreturn
    }
    // Bit11精査:フラグON
    if(data->bit11.flag){
        // システムトレース監査番号精査
        for (int i=0;i<DEF_BIT11_SIZE;i++){
            if(!isdigit(data->bit11.data[i])){ // 数値じゃない
                return DEF_RTN_ABNORMAL_SCRUTINY;
            }
        }
    // Bit11精査:フラグOFF
    }else{
        return DEF_RTN_ABNORMAL_SCRUTINY; // 精査異常でreturn
    }
    // Bit48精査:フラグOFF
    if(data->bit48.flag == DEF_FLG_OFF){
        return DEF_RTN_ABNORMAL_SCRUTINY; // 精査異常でreturn
    }

    return DEF_RTN_RECEIVE_REQUEST; // 要求応答型要求電文でreturn
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_NTC_msg_edit                               */
/*  CALLING SEQ.    : short  NWM_NTC_msg_edit(char *,struct *,char *)       */
/*  ARGUMENT        : 1.precv              (I) 通知電文                     */
/*  ARGUMENT        : 2.precv_len          (I) 通知電文長                   */
/*  ARGUMENT        : 3.nw_info            (I) NW情報ファイル               */
/*  ARGUMENT        : 4.preply             (O) 応答電文                     */
/*  ARGUMENT        : 5.preply_buffer_len  (I) 応答電文バッファサイズ       */
/*  ARGUMENT        : 6.preply_len         (O) 応答電文長                   */
/*  ARGUMENT        : 7.setting_mti        (O) 応答電文MTI                  */
/*  RETURN CODE     : 1     応答電文なし                                    */
/*                  : 2     応答電文あり                                    */
/*                  : 9     編集エラー                                      */
/*  DESCRIPTION     : 応答電文の作成を行う                                  */
/****************************************************************************/
short NWM_NTC_msg_edit(
              char *precv
            , short precv_len
            , db_gfnwi_def *nw_info
            , char *preply
            , short preply_buff_len
            , short *preply_len
            , char *setting_mti)
{
    // リプライデータを固定フォーマットに展開
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)preply;
    MTI_0630 *reply_data = (MTI_0630*)&pt_fixedform_jlink->ffd;

    // 応答電文編集要否判定
    // 応答なし
    if (nw_info->shori_kbn_info.admin_denbun_res_need == DEF_ADMIN_DISABLE){
        return DEF_RTN_NO_RESPONSE_MSG; // 応答電文なしでreturn
    }
    // 応答あり
    // 電文長精査(通知電文)
    if(sizeof(MTI_0620) + DEF_MTI_SIZE > precv_len){
        return DEF_RTN_EDIT_ERROR; // 編集エラーでreturn
    }
    // 電文長精査(応答電文)
    if(sizeof(MTI_0630) + DEF_MTI_SIZE > preply_buff_len){
        return DEF_RTN_EDIT_ERROR; // 編集エラーでreturn
    }

    // 電文長設定
    *preply_len = sizeof(MTI_0630) + DEF_MTI_SIZE;
    // データ部設定
    memcpy(preply,precv,sizeof(MTI_0630) + DEF_MTI_SIZE);
    // MTI設定
    int mti = DEF_MTI_EBC_0630;
    memcpy(pt_fixedform_jlink->mti,&mti,DEF_MTI_SIZE);
    memcpy(setting_mti,DEF_MTI_ASC_0630,DEF_MTI_SIZE);
    // Bit48フラグオフ
    reply_data->bit48.flag = DEF_FLG_OFF;
    reply_data->bit48.data_len = DEF_BIT48_FLGOFF_SIZE;
    memset(reply_data->bit48.data,' ',DEF_BIT48_SIZE);

    return DEF_RTN_WITH_RESPONSE_MSG; // 応答電文ありでreturn
}
/* end of NWM_NTC */
