/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD50                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               要求応答マッチングキー生成(Discover)        */
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
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <stdbool.h>  nolist
#include <string.h>   nolist

/* USER HEADER     */
#include "file.h(queue_data)"
#include "NWM_MKM.h"
#include "msg_DI.h"
#include "GFPCSD50.h"
#include "vproc.h"


/**********************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_MKM                                              */
/*  CALLING SEQ.    : short  NWM_MKM(char *,char *,char *,char *,short,short *)   */
/*  ARGUMENT        : mti              (I) MTI                                    */
/*  ARGUMENT        : queue_data       (I) キューファイル電文情報                 */
/*  ARGUMENT        : data_discover    (I) 固定フォーマットデータ                 */
/*  ARGUMENT        : key_info         (O) マッチングキー情報                     */
/*  ARGUMENT        : key_len_max      (I) マッチングキーレングスの最大長         */
/*  ARGUMENT        : key_len          (O) マッチングキーレングス                 */
/*  RETURN CODE     : 処理結果                                                    */
/*                      0     正常                                                */
/*                      1     エラー                                              */
/*  DESCRIPTION     : 要求応答マッチングキーを生成する                            */
/**********************************************************************************/
short  NWM_MKM(char           *mti
            ,  queue_data_def *queue_data
            ,  char           *data_discover
            ,  char           *key_info
            ,  short           key_len_max
            ,  short          *key_len)
{
    // マッチングキーポインタ設定
    key_info_discover_def   *pt_key = (key_info_discover_def*)key_info;

    // 引数のマッチングキー情報(key_info)のサイズがキー長より小さい場合はエラー
    if (key_len_max < sizeof(key_info_discover_def)) {
        return DEF_RETURN_ERROR; // エラーでreturn
    }

    memset(pt_key,' ',key_len_max);
    
    // MTI:0800のケース
    if (memcmp(mti,DEF_MTI_0800,DEF_MTI_SIZE) == 0) {
        MTI_0800* pt_data_discover = (MTI_0800*)data_discover;
        // 項目判定チェック(Bit7)
        if (pt_data_discover->bit7.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_discover->bit11.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }

        // マッチングキー編集
        // MTI
        memcpy(pt_key->MTI,DEF_MTI_0800,DEF_MTI_SIZE);  // MTI"0800"の場合は"0800"をセット
        // Bit7
        memcpy(pt_key->tsm_date_and_time
              ,pt_data_discover->bit7.data
              ,sizeof(pt_key->tsm_date_and_time));
        // Bit11
        memcpy(pt_key->audit_number
              ,pt_data_discover->bit11.data
              ,sizeof(pt_key->audit_number));
        // Bit32 フラグオンの場合に設定
        if (pt_data_discover->bit32.flag == true) {
            memcpy(pt_key->A_I_identification_code
                  ,pt_data_discover->bit32.data
                  ,pt_data_discover->bit32.data_len);
        }
        // Bit100 フラグオンの場合に設定
        if (pt_data_discover->bit100.flag == true) {
            memcpy(pt_key->R_I_identification_code
                  ,pt_data_discover->bit100.data
                  ,pt_data_discover->bit100.data_len);
        }
        *key_len = sizeof(key_info_discover_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:0810のケース
    else if (memcmp(mti,DEF_MTI_0810,DEF_MTI_SIZE) == 0) {
        MTI_0810* pt_data_discover = (MTI_0810*)data_discover;
        // 項目判定チェック(Bit7)
        if (pt_data_discover->bit7.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_discover->bit11.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        
        // マッチングキー編集
        // MTI
        memcpy(pt_key->MTI,DEF_MTI_0800,DEF_MTI_SIZE);  // MTI"0810"の場合は"0800"をセット
        // Bit7
        memcpy(pt_key->tsm_date_and_time
              ,pt_data_discover->bit7.data
              ,sizeof(pt_key->tsm_date_and_time));
        // Bit11
        memcpy(pt_key->audit_number,pt_data_discover->bit11.data
                    , sizeof(pt_key->audit_number));
        // Bit32 フラグオンの場合に設定
        if (pt_data_discover->bit32.flag == true) {
            memcpy(pt_key->A_I_identification_code
                  ,pt_data_discover->bit32.data
                  ,pt_data_discover->bit32.data_len);
        }
        // Bit100 フラグオンの場合に設定
        if (pt_data_discover->bit100.flag == true) {
            memcpy(pt_key->R_I_identification_code
                  ,pt_data_discover->bit100.data
                  ,pt_data_discover->bit100.data_len);
        }
        *key_len = sizeof(key_info_discover_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:0820のケース
    else if (memcmp(mti,DEF_MTI_0820,DEF_MTI_SIZE) == 0) {
        MTI_0820* pt_data_discover = (MTI_0820*)data_discover;
        // 項目判定チェック(Bit7)
        if (pt_data_discover->bit7.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_discover->bit11.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        
        // マッチングキー編集
        // MTI
        memcpy(pt_key->MTI,DEF_MTI_0820,DEF_MTI_SIZE);  // MTI"0820"の場合は"0820"をセット
        // Bit7
        memcpy(pt_key->tsm_date_and_time
              ,pt_data_discover->bit7.data
              ,sizeof(pt_key->tsm_date_and_time));
        // Bit11
        memcpy(pt_key->audit_number
              ,pt_data_discover->bit11.data
              ,sizeof(pt_key->audit_number));
        // Bit32 フラグオンの場合に設定
        if (pt_data_discover->bit32.flag == true) {
            memcpy(pt_key->A_I_identification_code
                  ,pt_data_discover->bit32.data
                  ,pt_data_discover->bit32.data_len);
        }
        // Bit100 フラグオンの場合に設定
        if (pt_data_discover->bit100.flag == true) {
            memcpy(pt_key->R_I_identification_code
                  ,pt_data_discover->bit100.data
                  ,pt_data_discover->bit100.data_len);
        }
        *key_len = sizeof(key_info_discover_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:0830のケース
    else if (memcmp(mti,DEF_MTI_0830,DEF_MTI_SIZE) == 0) {
        MTI_0830* pt_data_discover = (MTI_0830*)data_discover;
        // 項目判定チェック(Bit7)
        if (pt_data_discover->bit7.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        // 項目判定チェック(Bit11)
        if (pt_data_discover->bit11.flag == false) {
            return DEF_RETURN_ERROR; // エラーでreturn
        }
        
        // マッチングキー編集
        // MTI
        memcpy(pt_key->MTI,DEF_MTI_0820,DEF_MTI_SIZE);  // MTI"0830"の場合は"0820"をセット
        // Bit7
        memcpy(pt_key->tsm_date_and_time
              ,pt_data_discover->bit7.data
              ,sizeof(pt_key->tsm_date_and_time));
        // Bit11
        memcpy(pt_key->audit_number
              ,pt_data_discover->bit11.data
              , sizeof(pt_key->audit_number));
        // Bit32 フラグオンの場合に設定
        if (pt_data_discover->bit32.flag == true) {
            memcpy(pt_key->A_I_identification_code
                  ,pt_data_discover->bit32.data
                  ,pt_data_discover->bit32.data_len);
        }
        // Bit100 フラグオンの場合に設定
        if (pt_data_discover->bit100.flag == true) {
            memcpy(pt_key->R_I_identification_code
                  ,pt_data_discover->bit100.data
                  ,pt_data_discover->bit100.data_len);
        }
        *key_len = sizeof(key_info_discover_def);
        return DEF_RETURN_NORMAL; // 正常でreturn
    }
    // MTI:その他
    else {
        return DEF_RETURN_ERROR; // エラーでreturn
    }
}
/* end of NWM_MKM */
