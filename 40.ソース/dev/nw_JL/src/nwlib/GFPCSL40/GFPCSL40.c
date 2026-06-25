/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSL40                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文種別判定(J-Link)                        */
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
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h"     nolist
#include "ipc.h"      nolist
#include "NWM_MSJ.h"  nolist
#include "msg_JL.h"   nolist
#include "common.h"   nolist
#include "vproc.h"    nolist

/*****************************************************************************/
/*   DEFINE定義                                                              */
/*****************************************************************************/
#define DEF_MTI_NW_REQ                      "0800"      // MTI0800
#define DEF_MTI_NW_RES                      "0810"      // MTI0810
#define DEF_MTI_ADMINI_REQ                  "0620"      // MTI0620
#define DEF_MTI_ADMINI_RES                  "0630"      // MTI0630
#define DEF_SIGN_ON_NW_MNG_CODE             "001"       // 固定フォーマットbit70：001
#define DEF_SIGN_OFF_NW_MNG_CODE            "002"       // 固定フォーマットbit70：002
#define DEF_KEY_REQ_AXSIX_NW_MNG_CODE       "101"       // 固定フォーマットbit70：101
#define DEF_KEY_REQ_KB_NW_MNG_CODE          "102"       // 固定フォーマットbit70：102
#define DEF_ECHO_NW_MNG_CODE                "301"       // 固定フォーマットbit70：301

#define DEF_BITMAP_FLGOFF                   false       // 固定フォーマットフラグOFF
#define DEF_MTI_SIZE                        4           // MTIlength
#define DEF_CONTROL_KIND_SIZE               4           // 制御電文識別length
#define DEF_NW_MNG_CODE_SIZE                3           // 固定フォーマットBit70length
#define DEF_RETURN_NORMAL                   0           // 戻り値：正常
#define DEF_RETURN_ERROR                    1           // 戻り値：異常

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_MSJ                                        */
/*  CALLING SEQ.    : void   NWM_MSJ(char *,char *,char *)                  */
/*  ARGUMENT        : 1.pt_mti         (I)   MTI                            */
/*                  : 2.c_fixedform    (I)   固定フォーマットデータ         */
/*                  : 3.c_ctltext_type (O)   制御電文種別                   */
/*  RETURN CODE     : 0     正常                                            */
/*                  : 1     エラー                                          */
/*  DESCRIPTION     : 電文種別判定を行う                                    */
/****************************************************************************/
short NWM_MSJ(char* pt_mti,char* c_fixedform,char* c_ctltext_type)
{
    // 制御電文種別読込
    control_kind_def* pt_ctltext_type = (control_kind_def*)c_ctltext_type;

    //MTIチェック
    //MTIが制御電文要求(0800)のパターン
    if (memcmp(pt_mti,DEF_MTI_NW_REQ,DEF_MTI_SIZE) ==0 ){
        //固定フォーマットアドレス設定
        MTI_0800* pt_fixedform_jlink = (MTI_0800*)c_fixedform;

        // Network management information codeの有無チェック
        if(pt_fixedform_jlink->bit70.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 2桁目設定:要求応答区分
        pt_ctltext_type->req_res_kbn = DEF_CTLMSG_REQUEST;
        // 1桁目設定：制御機能区分
        // 3桁目設定：制御電文区分
        // 開局パターン
        if(memcmp(pt_fixedform_jlink->bit70.data,DEF_SIGN_ON_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        }
        // 閉局パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_SIGN_OFF_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_CLS;
        }
        // エコーテストパターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_ECHO_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_ECH_SND;
        }
        // 鍵交換(ANSIX 9.17)パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_KEY_REQ_AXSIX_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_KEY_EXCH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_KEY_EXC;
        }
        // 鍵交換(TR-31 Key Block)パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_KEY_REQ_KB_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_KEY_EXCH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_KEY_EXC;
        }
        // その他（エラー）
        else {
            memset(pt_ctltext_type,' ',DEF_CONTROL_KIND_SIZE); // space埋め
            return DEF_RETURN_ERROR; // エラーでreturn
        }
    //MTIが制御電文応答(0810)のパターン
    }else if(memcmp(pt_mti,DEF_MTI_NW_RES,DEF_MTI_SIZE) ==0 ){
        //固定フォーマットアドレス設定
        MTI_0810* pt_fixedform_jlink = (MTI_0810*)c_fixedform;

        //Network management information codeの有無チェック
        if(pt_fixedform_jlink->bit70.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 2桁目設定:要求応答区分
        pt_ctltext_type->req_res_kbn = DEF_CTLMSG_RESPONSE;
        // 1桁目設定：制御機能区分
        // 3桁目設定：制御電文区分
        // 開局パターン
        if(memcmp(pt_fixedform_jlink->bit70.data,DEF_SIGN_ON_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_OPN;
        }
        // 閉局パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_SIGN_OFF_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_CLS;
        }
        // エコーテストパターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_ECHO_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_CNT_STS_ECH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_ECH_SND;
        }
        // 鍵交換(ANSIX 9.17)パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_KEY_REQ_AXSIX_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_KEY_EXCH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_KEY_EXC;
        }
        // 鍵交換(TR-31 Key Block)パターン
        else if(memcmp(pt_fixedform_jlink->bit70.data,DEF_KEY_REQ_KB_NW_MNG_CODE,DEF_NW_MNG_CODE_SIZE) ==0 ){
            pt_ctltext_type->kinou_kbn = DEF_CTLFNC_KEY_EXCH;
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_KEY_EXC;
        }
        // その他（エラー）
        else {
            memset(pt_ctltext_type,' ',DEF_CONTROL_KIND_SIZE); // space埋め
            return DEF_RETURN_ERROR; // エラーでreturn
        }
    }
    //MTIがAdministrative要求(0620)の時
    else if(memcmp(pt_mti,DEF_MTI_ADMINI_REQ,DEF_MTI_SIZE) ==0 ){
        // 1桁目設定：制御機能区分
        pt_ctltext_type->kinou_kbn = DEF_CTLFNC_NTF_MSG;
        // 2桁目設定:要求応答区分
        pt_ctltext_type->req_res_kbn = DEF_CTLMSG_REQUEST;
        // 3桁目設定：制御電文区分
        pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_FAL;
    }
    //MTIがAdministrative応答(0630)の時
    else if(memcmp(pt_mti,DEF_MTI_ADMINI_RES,DEF_MTI_SIZE) ==0 ){
        // 1桁目設定：制御機能区分
        pt_ctltext_type->kinou_kbn = DEF_CTLFNC_NTF_MSG;
        // 2桁目設定:要求応答区分
        pt_ctltext_type->req_res_kbn = DEF_CTLMSG_RESPONSE;
        // 3桁目設定：制御電文区分
        pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_FAL;
    }
    //MTIがその他の電文の時(エラーリターン)
    else{
        memset(pt_ctltext_type,' ',DEF_CONTROL_KIND_SIZE);
        return DEF_RETURN_ERROR; // エラーでreturn
    }
    // 4桁目設定:内部処理区分
    pt_ctltext_type->int_proc_kbn = DEF_CTLINT_NORMAL;
    // 正常リターン
    return DEF_RETURN_NORMAL; // 正常でreturn

}
/* end of NWM_MSJ */
