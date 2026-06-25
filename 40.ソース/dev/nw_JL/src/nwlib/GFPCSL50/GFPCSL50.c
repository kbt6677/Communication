/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSL50                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               要求応答マッチングキー生成(J-Link)          */
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
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <stdbool.h>  nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h(queue_data)"
#include "NWM_MKM.h"
#include "msg_JL.h"
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_MTI_NW_REQ          "0800"
#define DEF_MTI_NW_RES          "0810"
#define DEF_BITMAP_FLGOFF       false
#define DEF_PROC_TYPE_REQ       1
#define DEF_RETURN_NORMAL       0
#define DEF_RETURN_ERROR        1
/****************************************************************************/
/* 構造体のtypedef定義                                                      */
/****************************************************************************/
typedef struct __key_info_jlink
{
    char    MTI[4];                     // MTI
    char    tsm_date_and_time[10];      // 取引日時(BIT 7)
    char    audit_number[6];            // システムトレースオーディットナンバー(BIT 11)
    char    identification_code[11];    // 送信元識別コード(BIT 33)
} key_info_jlink_def;

/**********************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_MKM                                              */
/*  CALLING SEQ.    : short  NWM_MKM(char *,char *,char *,char *,short,short *)   */
/*  ARGUMENT        : mti              (I) MTI                                    */
/*  ARGUMENT        : queue_data       (I) キューファイル電文情報                 */
/*  ARGUMENT        : data_jlink       (I) 固定フォーマットデータ                 */
/*  ARGUMENT        : key_info         (O) マッチングキー情報                     */
/*  ARGUMENT        : key_len_max      (I) マッチングキーレングスの最大長         */
/*  ARGUMENT        : key_len          (O) マッチングキーレングス                 */
/*  RETURN CODE     : 0     正常                                                  */
/*                  : 1     エラー                                                */
/*  DESCRIPTION     : 要求応答マッチングキーを生成する                            */
/**********************************************************************************/

short  NWM_MKM(char           *mti
            ,  queue_data_def *queue_data
            ,  char           *data_jlink
            ,  char           *key_info
            ,  short           key_len_max
            ,  short          *key_len)
{
    // マッチングキー読込
    key_info_jlink_def* pt_key = (key_info_jlink_def*)key_info;
    
    // マッチングキー長精査
    if(key_len_max < sizeof(key_info_jlink_def)){
        return DEF_RETURN_ERROR; // エラーでreturn
    }

    // MTI:0800(要求)のケース
    if (memcmp(mti,DEF_MTI_NW_REQ,sizeof(DEF_MTI_NW_REQ)-1) == 0){
        MTI_0800* pt_data_jlink = (MTI_0800*)data_jlink;
        // 項目判定チェック(Bit7)
        if (pt_data_jlink->bit7.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_jlink->bit11.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit33)
        if (pt_data_jlink->bit33.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }

        // マッチングキー編集
        memset(key_info,' ',sizeof(key_info_jlink_def));
        memcpy(pt_key->MTI,DEF_MTI_NW_REQ,sizeof(pt_key->MTI)); // 入力値は0800固定
        memcpy(pt_key->tsm_date_and_time,pt_data_jlink->bit7.data
                    , sizeof(pt_key->tsm_date_and_time));
        memcpy(pt_key->audit_number,pt_data_jlink->bit11.data
                    , sizeof(pt_key->audit_number));
        memcpy(pt_key->identification_code,pt_data_jlink->bit33.data
                    , pt_data_jlink->bit33.data_len);

        *key_len = sizeof(key_info_jlink_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:0810(応答)のケース
    else if (memcmp(mti,DEF_MTI_NW_RES,sizeof(DEF_MTI_NW_RES)-1) == 0){
        MTI_0810* pt_data_jlink = (MTI_0810*)data_jlink;
        // 項目判定チェック(Bit7)
        if (pt_data_jlink->bit7.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_jlink->bit11.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit33)
        if (pt_data_jlink->bit33.flag == DEF_BITMAP_FLGOFF){
            return DEF_RETURN_ERROR; // エラーでreturn
        }

        // マッチングキー編集
        memset(key_info,' ',sizeof(key_info_jlink_def));
        memcpy(pt_key->MTI,DEF_MTI_NW_REQ,sizeof(pt_key->MTI)); // 入力値は0800固定
        memcpy(pt_key->tsm_date_and_time,pt_data_jlink->bit7.data
                    , sizeof(pt_key->tsm_date_and_time));
        memcpy(pt_key->audit_number,pt_data_jlink->bit11.data
                    , sizeof(pt_key->audit_number));
        memcpy(pt_key->identification_code,pt_data_jlink->bit33.data
                    , pt_data_jlink->bit33.data_len);

        *key_len = sizeof(key_info_jlink_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:その他
    else {
        return DEF_RETURN_ERROR; // エラーでreturn
    }

}
/* end of NWM_MKM */
