/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD40                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               電文種別判定(Discover)                      */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-30                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/04/30 (J0680)新規作成                               */
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
#include "NWM_MSJ.h"  nolist
#include "msg_DI.h"   nolist
#include "common.h"   nolist
#include "file.h"     nolist
#include "GFPCSD40.h" nolist
#include "vproc.h"    nolist

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_MSJ                                        */
/*  CALLING SEQ.    : void   NWM_MSJ(char *,char *,char *)                  */
/*  ARGUMENT        : 1.pt_mti         (I)   MTI                            */
/*                  : 2.c_fixedform    (I)   固定フォーマットデータ         */
/*                  : 3.c_ctltext_type (O)   制御電文種別                   */
/*  RETURN CODE     : 処理結果                                              */
/*                      0     正常                                          */
/*                      1     エラー                                        */
/*  DESCRIPTION     : 電文種別判定を行う                                    */
/****************************************************************************/
short NWM_MSJ(char* pt_mti
             ,char* c_fixedform
             ,char* c_ctltext_type)
{
    // 制御電文種別ポインタ設定
    control_kind_def    *pt_ctltext_type = (control_kind_def*)c_ctltext_type;

    memset(pt_ctltext_type,' ',sizeof(control_kind_def)); // 制御電文種別クリア(space)

    // MTI="0800"のパターン
    if (memcmp(pt_mti,DEF_MTI_0800,DEF_MTI_SIZE) == 0) {
        MTI_0800    *data = (MTI_0800*)c_fixedform;

        // Network management information codeの有無チェック
        if(data->bit70.flag == false) {
            return DEF_MSJ_ERROR; // エラーでreturn
        }
        
        // Bit70 = "061"
        if (memcmp(data->bit70.data,DEF_BIT70_061,DEF_BIT70_SET_SIZE) == 0) {
            pt_ctltext_type->kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;     // '1':局状態・エコー制御
            pt_ctltext_type->req_res_kbn  = DEF_CTLMSG_REQUEST;         // '1':要求
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_OPN;         // '1':開局(サインオン)
            pt_ctltext_type->int_proc_kbn = DEF_CTLINT_NORMAL_ISSUER;   // '5':Discover:Issuer
        }
        // Bit70 = "101"
        else if (memcmp(data->bit70.data,DEF_BIT70_101,DEF_BIT70_SET_SIZE) == 0) {
            pt_ctltext_type->kinou_kbn    = DEF_CTLFNC_KEY_EXCH;        // '2':鍵交換制御
            pt_ctltext_type->req_res_kbn  = DEF_CTLMSG_REQUEST;         // '1':要求
            pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_KEY_EXC;         // '5':鍵交換
            pt_ctltext_type->int_proc_kbn = DEF_CTLINT_NORMAL_ISSUER;   // '5':Discover:Issuer
        }
        else {
            return DEF_MSJ_ERROR; // エラーでreturn
        }
    }
    // MTI="0820"のパターン
    else if (memcmp(pt_mti,DEF_MTI_0820,DEF_MTI_SIZE) == 0) {
        MTI_0820    *data = (MTI_0820*)c_fixedform;

        // Network management information codeの有無チェック
        if(data->bit70.flag == false) {
            return DEF_MSJ_ERROR; // エラーでreturn
        }

        //Response codeの有無チェック
        if(data->bit39.flag == false) {
            return DEF_MSJ_ERROR; // エラーでreturn
        }
        
        // Bit70 = "062"
        if (memcmp(data->bit70.data,DEF_BIT70_062,DEF_BIT70_SET_SIZE) == 0) {
            // Bit39 = "N1"
            if (memcmp(data->bit39.data,DEF_BIT39_N1,DEF_BIT39_SET_SIZE) == 0) {
                pt_ctltext_type->kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;     // '1':局状態・エコー制御
                pt_ctltext_type->req_res_kbn  = DEF_CTLMSG_REQUEST;         // '1':要求
                pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_OPN;         // '1':開局(サインオン)
                pt_ctltext_type->int_proc_kbn = DEF_CTLINT_NORMAL_ACQUIRER; // '4':Discover:Acquirer
            }
            // Bit39 = "N2" or "N3"
            else if (memcmp(data->bit39.data,DEF_BIT39_N2,DEF_BIT39_SET_SIZE) == 0 || 
                     memcmp(data->bit39.data,DEF_BIT39_N3,DEF_BIT39_SET_SIZE) == 0) {
                pt_ctltext_type->kinou_kbn    = DEF_CTLFNC_CNT_STS_ECH;     // '1':局状態・エコー制御
                pt_ctltext_type->req_res_kbn  = DEF_CTLMSG_REQUEST;         // '1':要求
                pt_ctltext_type->ctl_text_kbn = DEF_CTLTXT_CNT_CLS;         // '2':閉局(サインオフ)
                pt_ctltext_type->int_proc_kbn = DEF_CTLINT_NORMAL_ACQUIRER; // '4':Discover:Acquirer
            }
            else {
                return DEF_MSJ_ERROR; // エラーでreturn
            }
        }
        else {
            return DEF_MSJ_ERROR; // エラーでreturn
        }
    }
    //MTIがその他の電文の時(エラーリターン)
    else {
        return DEF_MSJ_ERROR; // エラーでreturn
    }
    // 正常リターン
    return DEF_MSJ_NORMAL; // 正常でreturn

} /* end of NWM_MSJ */
