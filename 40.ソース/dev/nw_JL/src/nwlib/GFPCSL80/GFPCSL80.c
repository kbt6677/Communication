/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSL80                                    */
/*        FUNCTION          ････ 鍵交換個別処理[J-link]                      */
/*                                                                           */
/*                               鍵交換制御のNW個別処理(J-Link)の処理を行う  */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-10                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/03/10 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdbool.h>  nolist
#include  <stdio.h>    nolist
#include  <stdlib.h>   nolist
#include  <string.h>   nolist
#include  <ctype.h>    nolist
/* USER HEADER     */
#include  "file.h"
#include  "ipc.h"
#include  "ems.h"
#include  "msg_JL.h"
#include  "vproc.h"
#include  "common.h"
#include  "NWM_KYX.h"
#include  "GFPCGX50.h"              // システム日時取得処理
#include  "GFPOGGZ3_encode.h"       // 文字コード変換モジュール

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define  DEF_COM_STD_GMT            1                       // システム日時：グリニッジ標準時
#define  DEF_COM_STD_J              2                       // システム日時：日本時間
#define  DEF_ERR_BIT7               "0007"                  // エラー発生ビット番号：7
#define  DEF_ERR_BIT11              "0011"                  // エラー発生ビット番号：11
#define  DEF_ERR_BIT33              "0033"                  // エラー発生ビット番号：33
#define  DEF_ERR_BIT39              "0039"                  // エラー発生ビット番号：39
#define  DEF_ERR_BIT53              "0053"                  // エラー発生ビット番号：53
#define  DEF_ERR_BIT70              "0070"                  // エラー発生ビット番号：70
#define  DEF_ERR_BIT96              "0096"                  // エラー発生ビット番号：96
#define  DEF_ERR_BIT100             "0100"                  // エラー発生ビット番号：100
#define  DEF_ERR_BIT105             "0105"                  // エラー発生ビット番号：105
#define  DEF_ERR_BITSIZE            4                       // エラー発生ビット番号サイズ
#define  DEF_BCD_LEN                2                       // char型1byteのBCDコード桁数
#define  DEF_MTI_EBC_0800           0xf0f8f0f0              // MTI:0800
#define  DEF_MTI_EBC_0810           0xf0f8f1f0              // MTI:0810
#define  DEF_MTI_ASC_0800           "0800"                  // MTI:0800
#define  DEF_MTI_ASC_0810           "0810"                  // MTI:0810
#define  DEF_MTI_SIZE               4                       // MTI SIZE
#define  DEF_VARIANT                "VA"                    // Variant（ANSI X9.17）
#define  DEF_BIT39_00               "00"                    // BIT39精査値00
#define  DEF_BIT39_96               "96"                    // BIT39精査値96
#define  DEF_IN_ERR_CODE_OK         "0000000"               // 内部エラーコード：正常
#define  DEF_IN_ERR_GET_DAY_ERR     "SCBL001"               // 日時取得エラー
#define  DEF_BIT7_LEN               10                      // BIT7サイズ
#define  DEF_BIT11_LEN              6                       // BIT11サイズ
#define  DEF_BIT33_LEN              11                      // BIT33サイズ
#define  DEF_BIT39_LEN              2                       // BIT39サイズ
#define  DEF_BIT53_LEN              8                       // BIT53サイズ
#define  DEF_BIT53_CHK_LEN          5                       // BIT53精査サイズ
#define  DEF_BIT70_LEN              3                       // BIT70サイズ
#define  DEF_BIT100_LEN             11                      // BIT100サイズ
#define  DEF_BIT53_RES              0x00                    // BIT53reserved精査値
#define  DEF_BIT53_VA_LEN           4                       // BIT53check digitサイズ:VA
#define  DEF_BIT53_KB_LEN           6                       // BIT53check digitサイズ:KB
#define  DEF_BIT70_101              "101"                   // BIT70精査値101
#define  DEF_BIT70_102              "102"                   // BIT70精査値102
#define  DEF_CHECK_DIGIT_OFFSET     5                       // check_digitBI53内開始位置
#define  DEF_RESERVED_OFFSET        7                       // reservedBI53内開始位置
#define  DEF_CHECK_DIGIT_SIZE_VA    4                       // check_digitサイズ（ANSI X9.17）
#define  DEF_CHECK_DIGIT_SIZE_KB    6                       // check_digitサイズ（TR-31 Key Block）
#define  DEF_KEY_MAX_SIZE           32                      // キー(EBC)最大長
const char DEF_BIT53_01[5] = {0x99,0x01,0x01,0x00,0x00};    // BIT53精査値01
const char DEF_BIT53_02[5] = {0x99,0x01,0x02,0x00,0x00};    // BIT53精査値02

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_req                          */
/*  CALLING SEQ.    : short  NWM_KYX_msg_check_req( )                       */
/*  ARGUMENT        : リクエストデータ                                      */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(NW単位)                                */
/*  ARGUMENT        : 接続先固有情報(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(ステーション単位)                      */
/*  ARGUMENT        : 接続先固有情報(コネクション単位)                      */
/*  ARGUMENT        : 鍵管理情報レコード                                    */
/*  ARGUMENT        : 精査処理結果                                          */
/*  RETURN CODE     : 精査結果                                              */
/*  DESCRIPTION     : 被仕向要求電文の精査を行う                            */
/****************************************************************************/
short NWM_KYX_msg_check_req(
    char *precv,                        // リクエストデータ
    char *p_rcv_data_len,               // 受信電文長
    db_gfnwi_def *nw_info_group,        // NW情報レコード(グループ単位)
    db_gfnwi_def *nw_info_interface,    // NW情報レコード(インタフェース単位)
    char *connect_nw,                   // 接続先固有情報(NW単位)
    char *connect_interface,            // 接続先固有情報(インタフェース単位)
    char *connect_station,              // 接続先固有情報(ステーション単位)
    char *connect_connetion,            // 接続先固有情報(コネクション単位)
    db_gckey_def *gckey_info,           // 鍵管理情報レコード
    NWM_KYX_arg_1_def *check_result)    // 精査処理結果
{
    // 変数初期化
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0800 *data = (MTI_0800*)&pt_fixedform_jlink->ffd;

    char buff[64];
    memset(check_result->err_bit,' ',sizeof(check_result->err_bit));

    // bit精査
    if (data->bit7.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit11.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit33.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT33,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit53.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit70.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit100.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT100,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    // bit7 NUMERICチェック
    for (int i=0;i<data->bit7.data_len;i++){
        if(!isdigit(data->bit7.data[i])){ // 数値じゃなかったら
            memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
    }
    // bit11 NUMERICチェック
    for (int i=0;i<data->bit11.data_len;i++){
        if(!isdigit(data->bit11.data[i])){ // 数値じゃなかったら
            memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
    }
    // bit33 NUMERICチェック
    for (int i=0;i<data->bit33.data_len;i++){
        if(!isdigit(data->bit33.data[i])){ // 数値じゃなかったら
            memcpy(check_result->err_bit,DEF_ERR_BIT33,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
    }
    // Variant（ANSI X9.17）
    if (memcmp(gckey_info->key_info.key_format,DEF_VARIANT,sizeof(DEF_VARIANT)-1) == 0){
        if (memcmp(data->bit53.data,DEF_BIT53_01,DEF_BIT53_CHK_LEN) != 0){
            memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        if (data->bit53.data[DEF_RESERVED_OFFSET] != DEF_BIT53_RES){
            memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        if (memcmp(data->bit70.data,DEF_BIT70_101,sizeof(DEF_BIT70_101)-1) != 0){
            memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        // single DES
        if (memcmp(gckey_info->key_info.key_use_alg,DEF_KEY_USE_ALG_DES1,sizeof(DEF_KEY_USE_ALG_DES1)-1) == 0){
            if (data->bit96.flag == false){
                memcpy(check_result->err_bit,DEF_ERR_BIT96,DEF_ERR_BITSIZE);
                return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
            }
            if (data->bit105.flag){
                memcpy(check_result->err_bit,DEF_ERR_BIT105,DEF_ERR_BITSIZE);
                return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
            }
            // 精査処理結果編集 bit96
            memset(buff,NULL,sizeof(buff));
            for (int i = 0; i < data->bit96.data_len ; i++) { // 16進数文字列に変換
                sprintf(&buff[i*2], "%02X", data->bit96.data[i]);
            }
            check_result->key_leng = (short)data->bit96.data_len * 2;
            memcpy(check_result->key,buff,check_result->key_leng);
        }
        // triple DES(Double Key)
        else if(memcmp(gckey_info->key_info.key_use_alg,DEF_KEY_USE_ALG_DES2,sizeof(DEF_KEY_USE_ALG_DES2)-1) == 0){
            if (data->bit96.flag){
                memcpy(check_result->err_bit,DEF_ERR_BIT96,DEF_ERR_BITSIZE);
                return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
            }
            if (data->bit105.flag == false){
                memcpy(check_result->err_bit,DEF_ERR_BIT105,DEF_ERR_BITSIZE);
                return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
            }
            // 精査処理結果編集 bit105
            memset(buff,NULL,sizeof(buff));
            for (int i = 0; i < data->bit105.data_len ; i++) { // 16進数文字列に変換
                sprintf(&buff[i * 2], "%02X", data->bit105.data[i]);
            }
            check_result->key_leng = (short)data->bit105.data_len * 2;
            memcpy(check_result->key,buff,check_result->key_leng);
        }
        // triple DES(Triple Key)
        else{
            return DEF_NWM_KYX_RTN_ERR;
        }
        // チェックデジット長設定
        check_result->checkdigit_leng = DEF_CHECK_DIGIT_SIZE_VA;
    }
    // Key Block（ANSI X9 TR31 key Block）
    else{
        if (memcmp(data->bit53.data,DEF_BIT53_02,DEF_BIT53_CHK_LEN) != 0){
            memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        if (memcmp(data->bit70.data,DEF_BIT70_102,sizeof(DEF_BIT70_102)-1) != 0){
            memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        if (data->bit96.flag){
            memcpy(check_result->err_bit,DEF_ERR_BIT96,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        if (data->bit105.flag == false){
            memcpy(check_result->err_bit,DEF_ERR_BIT105,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
        }
        // 精査処理結果編集 bit105
        check_result->key_leng = (short)data->bit105.data_len;
        memcpy(check_result->key,data->bit105.data,data->bit105.data_len);
        // チェックデジット長設定
        check_result->checkdigit_leng = DEF_CHECK_DIGIT_SIZE_KB;
    }

    // 精査処理結果編集    
    memset(buff,NULL,sizeof(buff));
    for (int i = 0; i < check_result->checkdigit_leng / 2; i++) { // 16進数文字列に変換
        sprintf(&buff[i * 2], "%02X", data->bit53.data[DEF_CHECK_DIGIT_OFFSET + i]);
    }
    memcpy(check_result->checkdigit,buff,check_result->checkdigit_leng);
    memcpy(check_result->key_kind,DEF_KEY_TYPE_KPE,sizeof(DEF_KEY_TYPE_KPE)-1);
    return DEF_NWM_KYX_RTN_OK;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_rsp                          */
/*  CALLING SEQ.    : short  NWM_KYX_msg_check_rsp( )                       */
/*  ARGUMENT        : 鍵交換電文種別                                        */
/*  ARGUMENT        : 受信データ                                            */
/*  ARGUMENT        : 仕向要求電文                                          */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(NW単位)                                */
/*  ARGUMENT        : 接続先固有情報(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(ステーション単位)                      */
/*  ARGUMENT        : 接続先固有情報(コネクション単位)                      */
/*  ARGUMENT        : 鍵管理情報レコード                                    */
/*  ARGUMENT        : 精査処理結果                                          */
/*  RETURN CODE     : 精査結果                                              */
/*  DESCRIPTION     : 仕向応答電文の精査を行う                              */
/****************************************************************************/
short NWM_KYX_msg_check_rsp(
    short exchange_type,                // 鍵交換電文種別
    char *precv,                        // 受信データ
    char *p_rcv_data_len,               // 受信電文長
    db_gfnwi_def *nw_info_group,        // NW情報レコード(グループ単位)
    db_gfnwi_def *nw_info_interface,    // NW情報レコード(インタフェース単位)
    char *connect_nw,                   // 接続先固有情報(NW単位)
    char *connect_interface,            // 接続先固有情報(インタフェース単位)
    char *connect_station,              // 接続先固有情報(ステーション単位)
    char *connect_connetion,            // 接続先固有情報(コネクション単位)
    db_gckey_def *gckey_info,           // 鍵管理情報レコード
    NWM_KYX_arg_2_def *check_result)    // 精査処理結果
{
    // 変数初期化
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0810 *data = (MTI_0810*)&pt_fixedform_jlink->ffd;

    char buff[10];
    memset(buff,NULL,sizeof(buff));
    memset(check_result,0x20,sizeof(check_result));
    check_result->key_leng=0;
    check_result->checkdigit_leng=0;
    memset(check_result->err_bit,' ',sizeof(check_result->err_bit));

    // bit精査
    if (data->bit7.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit11.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit33.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT33,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit39.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT39,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit53.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit70.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit96.flag){
        memcpy(check_result->err_bit,DEF_ERR_BIT96,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit100.flag == false){
        memcpy(check_result->err_bit,DEF_ERR_BIT100,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    if (data->bit105.flag){
        memcpy(check_result->err_bit,DEF_ERR_BIT105,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_FAULT_MSG; // 精査結果(障害電文通知)でreturn
    }
    // Bit39 レスポンスコード精査
    if (memcmp(data->bit39.data,DEF_BIT39_00,sizeof(DEF_BIT39_00)-1) != 0){
        return DEF_NWM_KYX_RTN_NG_REJ; // 精査結果(拒否応答)でreturn
    }

    memcpy(check_result->key_kind,DEF_KEY_TYPE_KPE,sizeof(DEF_KEY_TYPE_KPE)-1);

    return DEF_NWM_KYX_RTN_OK;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_edit_req                           */
/*  CALLING SEQ.    : short  NWM_KYX_msg_edit_req( )                        */
/*  ARGUMENT        : 鍵交換電文種別                                        */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(NW単位)                                */
/*  ARGUMENT        : 接続先固有情報(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(ステーション単位)                      */
/*  ARGUMENT        : 接続先固有情報(コネクション単位)                      */
/*  ARGUMENT        : 鍵管理情報レコード                                    */
/*  ARGUMENT        : EMS出力共通情報                                       */
/*  ARGUMENT        : EMS出力付加情報                                       */
/*  ARGUMENT        : 設定データ                                            */
/*  ARGUMENT        : 電文情報                                              */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 要求電文の編集を行う                                  */
/****************************************************************************/
short NWM_KYX_msg_edit_req(
    short exchange_type,                // 鍵交換電文種別
    db_gfnwi_def *nw_info_group,        // NW情報レコード(グループ単位)
    db_gfnwi_def *nw_info_interface,    // NW情報レコード(インタフェース単位)
    char *connect_nw,                   // 接続先固有情報(NW単位)
    char *connect_interface,            // 接続先固有情報(インタフェース単位)
    char *connect_station,              // 接続先固有情報(ステーション単位)
    char *connect_connetion,            // 接続先固有情報(コネクション単位)
    db_gckey_def *gckey_info,           // 鍵管理情報レコード
    oggz1in_def *oggz1in,               // EMS出力共通情報
    NWM_KYX_ems_add *ems_add,           // EMS出力付加情報
    NWM_KYX_arg_3_def *setting_data,    // 設定データ
    NWM_KYX_arg_4_def *data_info)       // 電文情報
{
    // 変数初期化
    int ret = 0 ,check_digit_value[DEF_BIT53_KB_LEN],key_value[DEF_KEY_MAX_SIZE];
    long buff = 0x0;
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)data_info->message;
    MTI_0800 *data = (MTI_0800*)&pt_fixedform_jlink->ffd;

    memset(data_info->message,' ',sizeof(MTI_0800) + DEF_MTI_SIZE);
    data_info->message_leng = sizeof(MTI_0800) + DEF_MTI_SIZE;
    int mti = DEF_MTI_EBC_0800;
    memcpy(pt_fixedform_jlink->mti,&mti,DEF_MTI_SIZE);
    memcpy(data_info->mti,DEF_MTI_ASC_0800,DEF_MTI_SIZE);
    COM_SDT_arg_2_def SDT_arg2; // システム日付取得アーギュメント2
    COM_SDT_arg_3_def SDT_arg3; // システム日付取得アーギュメント3
    long long SDT_arg4; // システム日付取得アーギュメント4
    dst_unq_info_JL *dst_unq_info =(dst_unq_info_JL *)connect_interface;
    char wk_buf[12];//編集用バッファ

    //鍵交換電文種別チェック
    if (exchange_type != DEF_NWM_KYX_MSG_KIND_KEYREQ) {
        return DEF_NWM_KYX_RTN_ERR;
    }

    // bit設定
    data->bit7.flag = true;
    data->bit11.flag = true;
    data->bit33.flag = true;
    data->bit53.flag = true;
    data->bit70.flag = true;
    data->bit100.flag = true;

    // length設定
    data->bit7.data_len = DEF_BIT7_LEN;
    data->bit11.data_len = DEF_BIT11_LEN;
    data->bit33.data_len = DEF_BIT33_LEN;
    data->bit53.data_len = DEF_BIT53_LEN;
    data->bit70.data_len = DEF_BIT70_LEN;
    data->bit100.data_len = DEF_BIT100_LEN;

    // システム日付取得(Bit7設定)
    ret = COM_SDT(DEF_COM_STD_GMT,&SDT_arg2,&SDT_arg3,&SDT_arg4);
    if (ret != 0) {
        // エラーコード返却廃止
        // memcpy(data_info->err_code,DEF_IN_ERR_GET_DAY_ERR,sizeof(DEF_IN_ERR_GET_DAY_ERR)-1); 
        return DEF_NWM_KYX_RTN_ERR;
    }
    
    memcpy(data->bit7.data,&SDT_arg2.mm,data->bit7.data_len); // MMDDhhmmss の10桁

    // システム採番生成値(Bit11)
    memcpy(data->bit11.data,setting_data->sysytem_no,data->bit11.data_len);

    // 送信元識別コード(Bit33)
    memset(wk_buf,NULL,sizeof(wk_buf));

    /* NULL止めワークに接続先固有情報レコード(インタフェース単位)に */
    /* 設定されている送信先識別コードをコピー                       */
    memcpy(wk_buf,dst_unq_info->forwarding_inst_id
        ,sizeof(dst_unq_info->forwarding_inst_id));

    /* ワークの先頭からスペースもしくはNULLが見つかるまでの         */
    /* 長さ(可変長の実データ長)を固定フォーマットのデータ長に設定   */
    data->bit33.data_len = strcspn(wk_buf, " ");

    /* スペースクリア済みの固定フォーマットに送信先識別コードを可変長レングスで設定 */
    memcpy(data->bit33.data,dst_unq_info->forwarding_inst_id
        ,data->bit33.data_len);

    // 受信機関識別コード(Bit100)
    memset(wk_buf,NULL,sizeof(wk_buf));
    /* NULL止めワークに接続先固有情報レコード(インタフェース単位)に */
    /* 設定されている送信先識別コードをコピー                       */
    memcpy(wk_buf,dst_unq_info->receiving_inst_id
        ,sizeof(dst_unq_info->receiving_inst_id));

    /* ワークの先頭からスペースもしくはNULLが見つかるまでの         */
    /* 長さ(可変長の実データ長)を固定フォーマットのデータ長に設定   */
    data->bit100.data_len = strcspn(wk_buf, " ");

    /* スペースクリア済みの固定フォーマットに送信先識別コードを可変長レングスで設定 */
    memcpy(data->bit100.data,dst_unq_info->receiving_inst_id
        ,data->bit100.data_len);

    // 鍵設定部初期化
    memset(&data->bit96,NULL,sizeof(data->bit96));
    memset(&data->bit105,NULL,sizeof(data->bit105));
    data->bit96.flag = false;
    data->bit105.flag = false;

    // Variant（ANSI X9.17）
    if (memcmp(gckey_info->key_info.key_format,DEF_VARIANT,sizeof(DEF_VARIANT)-1) == 0){
        // Bit53上5桁
        memcpy(data->bit53.data,DEF_BIT53_01,DEF_BIT53_CHK_LEN);
        CHAR2HEX(setting_data->checkdigit, &data->bit53.data[DEF_CHECK_DIGIT_OFFSET],DEF_CHECK_DIGIT_SIZE_VA );
//        for (int i = 0; i < DEF_BIT53_VA_LEN; i++) {
//            check_digit_value[i] = setting_data->checkdigit[i] - '0';
//        }
//        for (int i = 0;i < (DEF_CHECK_DIGIT_SIZE_VA / 2);i++){
//            data->bit53.data[DEF_CHECK_DIGIT_OFFSET + i] = (char)((check_digit_value[i * 2] << 4) | check_digit_value[(i * 2) + 1]);
//        }
        data->bit53.data[DEF_CHECK_DIGIT_OFFSET + 2] = DEF_BIT53_RES;

        // NW管理情報コード(bit70)
        memcpy(data->bit70.data,DEF_BIT70_101,data->bit70.data_len);

        // single DES
        if (memcmp(gckey_info->key_info.key_use_alg,DEF_KEY_USE_ALG_DES1,sizeof(DEF_KEY_USE_ALG_DES1)-1) == 0){
            data->bit96.flag = true;
            // KEY値 bit96
            data->bit96.data_len = (setting_data->key_leng) / 2;
            //文字変換モジュールでCHARをBinaryに変換
            CHAR2HEX(setting_data->key, data->bit96.data,setting_data->key_leng );
//            for (int i = 0; i < setting_data->key_leng; i++) {
//                key_value[i] = setting_data->key[i] - '0';
//            }
//            for (int i = 0;i < data->bit96.data_len;i++){
//                data->bit96.data[i] = (char)((key_value[i * 2] << 4) | key_value[(i * 2) + 1]);
//            }
        }
        // triple DES(Double key)
        else if(memcmp(gckey_info->key_info.key_use_alg,DEF_KEY_USE_ALG_DES2,sizeof(DEF_KEY_USE_ALG_DES2)-1) == 0){
            data->bit105.flag = true;
            // KEY値 bit105
            data->bit105.data_len = (setting_data->key_leng) / 2;
            //文字変換モジュールでCHARをBinaryに変換
            CHAR2HEX(setting_data->key, data->bit105.data,setting_data->key_leng);
//            for (int i = 0; i < setting_data->key_leng; i++) {
//                key_value[i] = setting_data->key[i] - '0';
//            }
//            for (int i = 0;i < data->bit105.data_len;i++){
//                data->bit105.data[i] = (char)((key_value[i * 2] << 4) | key_value[(i * 2) + 1]);
//            }
        }
        else{
            return DEF_NWM_KYX_RTN_ERR;
        }
    }
    // Key Block（ANSI X9 TR31 key Block）
    else{
        // Bit53上5桁
        memcpy(data->bit53.data,DEF_BIT53_02,DEF_BIT53_CHK_LEN);
        for (int i = 0; i < DEF_BIT53_KB_LEN; i++) {
            check_digit_value[i] = setting_data->checkdigit[i] - '0';
        }
        for (int i = 0;i < (DEF_CHECK_DIGIT_SIZE_KB / 2);i++){
            data->bit53.data[DEF_CHECK_DIGIT_OFFSET + i] = (char)((check_digit_value[i * 2] << 4) | check_digit_value[(i * 2) + 1]);
        }

        // NW管理情報コード(bit70)
        memcpy(data->bit70.data,DEF_BIT70_102,data->bit70.data_len);

        data->bit105.flag = true;

        // KEY値 bit105
        data->bit105.data_len = setting_data->key_leng;
        memcpy(data->bit105.data,setting_data->key,setting_data->key_leng);
    }
    return DEF_NWM_KYX_RTN_OK;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_edit_rsp                           */
/*  CALLING SEQ.    : short  NWM_KYX_msg_edit_rsp( )                        */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(NW単位)                                */
/*  ARGUMENT        : 接続先固有情報(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(ステーション単位)                      */
/*  ARGUMENT        : 接続先固有情報(コネクション単位)                      */
/*  ARGUMENT        : 鍵管理情報レコード                                    */
/*  ARGUMENT        : カット対象日付管理ファイル情報                        */
/*  ARGUMENT        : EMS出力共通情報                                       */
/*  ARGUMENT        : EMS出力付加情報                                       */
/*  ARGUMENT        : 設定データ                                            */
/*  ARGUMENT        : 電文情報                                              */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 応答電文の編集を行う                                  */
/****************************************************************************/
short NWM_KYX_msg_edit_rsp(
    db_gfnwi_def *nw_info_group,        // NW情報レコード(グループ単位)
    db_gfnwi_def *nw_info_interface,    // NW情報レコード(インタフェース単位)
    char *connect_nw,                   // 接続先固有情報(NW単位)
    char *connect_interface,            // 接続先固有情報(インタフェース単位)
    char *connect_station,              // 接続先固有情報(ステーション単位)
    char *connect_connetion,            // 接続先固有情報(コネクション単位)
    db_gckey_def *gckey_info,           // 鍵管理情報レコード
    file_info_gccut *info_gccut,        // カット対象日付管理ファイル情報
    oggz1in_def *oggz1in,               // EMS出力共通情報
    NWM_KYX_ems_add *ems_add,           // EMS出力付加情報
    NWM_KYX_arg_5_def *setting_data,    // 設定データ
    NWM_KYX_arg_6_def *data_info)       // 電文情報
{
    // 変数初期化
    char buff[100];
    msg_jlink_def* pt_fixedform_jlink_receive = (msg_jlink_def*)data_info->rcv_message;
    MTI_0800 *rcv_data = (MTI_0800*)&pt_fixedform_jlink_receive->ffd;
    msg_jlink_def* pt_fixedform_jlink_reply = (msg_jlink_def*)data_info->message;
    MTI_0810 *reply_data = (MTI_0810*)&pt_fixedform_jlink_reply->ffd;

    memset(data_info->message,' ',sizeof(MTI_0810) + DEF_MTI_SIZE);
    data_info->message_leng = sizeof(MTI_0810) + DEF_MTI_SIZE;
    int mti = DEF_MTI_EBC_0810;
    memcpy(pt_fixedform_jlink_reply->mti,&mti,DEF_MTI_SIZE);
    memcpy(data_info->mti,DEF_MTI_ASC_0810,DEF_MTI_SIZE);

    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit7,sizeof(rcv_data->bit7));
    memcpy(&reply_data->bit7,buff,sizeof(rcv_data->bit7));
    
    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit11,sizeof(rcv_data->bit11));
    memcpy(&reply_data->bit11,buff,sizeof(rcv_data->bit11));
    
    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit33,sizeof(rcv_data->bit33));
    memcpy(&reply_data->bit33,buff,sizeof(rcv_data->bit33));
    
    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit53,sizeof(rcv_data->bit53));
    memcpy(&reply_data->bit53,buff,sizeof(rcv_data->bit53));
    
    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit70,sizeof(rcv_data->bit70));
    memcpy(&reply_data->bit70,buff,sizeof(rcv_data->bit70));
    
    memset(buff,NULL,sizeof(buff));
    // 応答編集時はbitOFF
    memcpy(&reply_data->bit96,buff,sizeof(rcv_data->bit96));
    
    memset(buff,NULL,sizeof(buff));
    memcpy(buff,&rcv_data->bit100,sizeof(rcv_data->bit100));
    memcpy(&reply_data->bit100,buff,sizeof(rcv_data->bit100));
    
    memset(buff,NULL,sizeof(buff));
    // 応答編集時はbitOFF
    memcpy(&reply_data->bit105,buff,sizeof(rcv_data->bit105));
    
    // bit39設定
    reply_data->bit39.flag = true;
    reply_data->bit39.data_len = DEF_BIT39_LEN;
    // 内部エラーコード異常
    if(memcmp(setting_data->err_code,DEF_IN_ERR_CODE_OK,sizeof(setting_data->err_code)) != 0){
        memcpy(reply_data->bit39.data,DEF_BIT39_96,DEF_BIT39_LEN);
        return DEF_NWM_KYX_RTN_NG_REJ; // 拒否応答でreturn
    }
    // 内部エラーコード正常
    memcpy(reply_data->bit39.data,DEF_BIT39_00,DEF_BIT39_LEN);
    return DEF_NWM_KYX_RTN_OK; // 正常でreturn
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_req                          */
/*  CALLING SEQ.    : short  NWM_KYX_cst_check_req( )                       */
/*  ARGUMENT        : 局状態                                                */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 要求受信時に局状態が処理可能状態であることを判定      */
/****************************************************************************/
short NWM_KYX_cst_check_req(
    char *station_status)               // 局状態
{
    if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
        return DEF_NWM_KYX_RTN_OK; // 正常でreturn
    }
    return DEF_NWM_KYX_RTN_HAKI_MSG; // 電文破棄でreturn
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_rsp                          */
/*  CALLING SEQ.    : short  NWM_KYX_cst_check_rsp( )                       */
/*  ARGUMENT        : 鍵交換電文種別                                        */
/*  ARGUMENT        : 局状態                                                */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 応答受信時に局状態が処理可能状態であることを判定      */
/****************************************************************************/
short NWM_KYX_cst_check_rsp(
    short exchange_type,                // 鍵交換電文種別
    char *station_status)               // 局状態
{
    //鍵交換電文種別チェック
    if (exchange_type != DEF_NWM_KYX_MSG_KIND_KEYRSP) {
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 電文破棄でreturn
    }
    return DEF_NWM_KYX_RTN_OK; // 正常でreturn
}
