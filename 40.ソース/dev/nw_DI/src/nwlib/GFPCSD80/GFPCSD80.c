/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD80                                    */
/*        FUNCTION          ････ 鍵交換個別処理[Discover]                    */
/*                                                                           */
/*                               鍵交換制御のNW個別処理(Discover)の処理を行う*/
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-20                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/04/30 (J0680)新規作成                               */
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
#include  "common.h"
#include  "msg_DI.h"
#include  "NWM_KYX.h"
#include  "GFPCGX50.h"              // システム日時取得処理
#include  "GFPCSD80.h"
#include  "vproc.h"


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
/*  RETURN CODE     : 処理結果                                              */
/*                      0：精査OK                                           */
/*                      3：精査エラー(電文破棄)                             */
/*  DESCRIPTION     : 被仕向要求電文の精査を行う                            */
/****************************************************************************/
short NWM_KYX_msg_check_req(
    char                *precv,             // リクエストデータ
    char                *p_rcv_data_len,    // 受信電文長
    db_gfnwi_def        *nw_info_group,     // NW情報レコード(グループ単位)
    db_gfnwi_def        *nw_info_interface, // NW情報レコード(インタフェース単位)
    char                *connect_nw,        // 接続先固有情報(NW単位)
    char                *connect_interface, // 接続先固有情報(インタフェース単位)
    char                *connect_station,   // 接続先固有情報(ステーション単位)
    char                *connect_connetion, // 接続先固有情報(コネクション単位)
    db_gckey_def        *gckey_info,        // 鍵管理情報レコード
    NWM_KYX_arg_1_def   *check_result)      // 精査処理結果
{
    // 変数初期化
    msg_discover_def        *pt_fixedform_discover = (msg_discover_def*)precv;
    MTI_0800                *data = (MTI_0800*)&pt_fixedform_discover->ffd;
    dst_unq_info_discover   *p_dst_unq_info = (dst_unq_info_discover *)nw_info_interface->dst_unq_info;
    char                     buff[256];

    // エラービット番号クリア
    memset(check_result->err_bit,' ',sizeof(check_result->err_bit));
    
    //動作モード精査
    if(p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_I) {
        return DEF_NWM_KYX_RTN_HAKI_MSG;
    }

    // 精査：Bit7
    if (data->bit7.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // 日付チェック
        memset(buff,NULL,sizeof(buff));
        memcpy(&buff[0],"0000",4);
        memcpy(&buff[4],data->bit7.data,data->bit7.data_len);
        if (CMIN_check_datetime(buff) != 0) {
            memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }
    
    // 精査：Bit11
    if (data->bit11.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // NUMERICチェック
        if (NWM_KYX_isdigit(data->bit11.data,data->bit11.data_len) == false) {
            memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }
    
    // 精査：Bit32
    if (data->bit32.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT32,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // NUMERICチェック
        if (NWM_KYX_isdigit(data->bit32.data,data->bit32.data_len) == false) {
            memcpy(check_result->err_bit,DEF_ERR_BIT32,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }
    
    // 精査：Bit37
    if (data->bit37.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT37,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    // 精査：Bit39
    if (data->bit39.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT39,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    // 精査：Bit44
    if (data->bit44.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT44,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    // 精査：Bit48
    if (data->bit48.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT48,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    // 精査：Bit53
    if (data->bit53.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // bit53 値チェック
        if(memcmp(&data->bit53.data[2],DEF_BIT53_04,DEF_PIN_ENC_SET_SIZE) != 0){
            memcpy(check_result->err_bit,DEF_ERR_BIT53,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }
    // 精査：Bit70
    if (data->bit70.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // bit70 値チェック
        if (memcmp(data->bit70.data,DEF_BIT70_101,DEF_BIT70_SET_SIZE) != 0) {
            memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }

    // 精査：Bit100
    if (data->bit100.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT100,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // NUMERICチェック
        if (NWM_KYX_isdigit(data->bit100.data,data->bit100.data_len) == false) {
            memcpy(check_result->err_bit,DEF_ERR_BIT100,DEF_ERR_BITSIZE);
            return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
        }
    }
    
    // 精査：Bit120
    if (data->bit120.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT120,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    
    // 精査：Bit127
    if (data->bit127.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT127,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査処理結果を編集
    memcpy(check_result->key_kind                       // KEY種別
          ,DEF_KEY_TYPE_KPE                             // "KPE "
          ,sizeof(check_result->key_kind));
    check_result->key_leng = (short)data->bit120.data_len;
    memcpy(check_result->key                            // KEY
          ,data->bit120.data                            // Bit120
          ,data->bit120.data_len);
    check_result->checkdigit_leng = DEF_BIT53_CK_DI_SIZE;
    memcpy(check_result->checkdigit                     // チェックディジット
          ,&data->bit53.data[DEF_BIT53_CK_DI_OFSET]
          ,DEF_BIT53_CK_DI_SIZE);

    return DEF_NWM_KYX_RTN_OK; // 正常でreturn
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_msg_check_rsp                          */
/*  CALLING SEQ.    : short  NWM_KYX_msg_check_rsp( )                       */
/*  ARGUMENT        : 鍵交換電文種別                                        */
/*  ARGUMENT        : 受信データ                                            */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(NW単位)                                */
/*  ARGUMENT        : 接続先固有情報(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報(ステーション単位)                      */
/*  ARGUMENT        : 接続先固有情報(コネクション単位)                      */
/*  ARGUMENT        : 鍵管理情報レコード                                    */
/*  ARGUMENT        : 精査処理結果                                          */
/*  RETURN CODE     : 処理結果                                              */
/*                      0：精査OK                                           */
/*                      1：精査エラー(拒否応答)                             */
/*                      3：精査エラー(電文破棄)                             */
/*  DESCRIPTION     : 仕向応答電文の精査を行う                              */
/****************************************************************************/
short NWM_KYX_msg_check_rsp(
    short               exchange_type,      // 鍵交換電文種別
    char                *precv,             // 受信データ
    char                *p_rcv_data_len,    // 受信電文長
    db_gfnwi_def        *nw_info_group,     // NW情報レコード(グループ単位)
    db_gfnwi_def        *nw_info_interface, // NW情報レコード(インタフェース単位)
    char                *connect_nw,        // 接続先固有情報(NW単位)
    char                *connect_interface, // 接続先固有情報(インタフェース単位)
    char                *connect_station,   // 接続先固有情報(ステーション単位)
    char                *connect_connetion, // 接続先固有情報(コネクション単位)
    db_gckey_def        *gckey_info,        // 鍵管理情報レコード
    NWM_KYX_arg_2_def   *check_result)      // 精査処理結果
{
    msg_discover_def    *pt_fixedform_discover = (msg_discover_def*)precv;
    MTI_0810            *data = (MTI_0810*)&pt_fixedform_discover->ffd;

    // 変数初期化
    memset(check_result,NULL,sizeof(check_result));

    // 精査：Bit7
    if (data->bit7.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT7,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit11
    if (data->bit11.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT11,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit32
    if (data->bit32.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT32,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit37
    if (data->bit37.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT37,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit39
    if (data->bit39.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT39,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }
    else {
        // レスポンスコード精査
        if (memcmp(data->bit39.data,DEF_BIT39_00,sizeof(DEF_BIT39_00) - 1) != 0) {
            return DEF_NWM_KYX_RTN_NG_REJ; // 精査結果(拒否応答)でreturn
        }
    }

    // 精査：Bit48
    if (data->bit48.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT48,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit59
    if (data->bit59.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT59,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit70
    if (data->bit70.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT70,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit100
    if (data->bit100.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT100,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit120
    if (data->bit120.flag == true) {
        memcpy(check_result->err_bit,DEF_ERR_BIT120,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    // 精査：Bit127
    if (data->bit127.flag == false) {
        memcpy(check_result->err_bit,DEF_ERR_BIT127,DEF_ERR_BITSIZE);
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 精査結果(電文破棄)でreturn
    }

    memcpy(check_result->key_kind,DEF_KEY_TYPE_KPE,sizeof(DEF_KEY_TYPE_KPE)-1);

    return DEF_NWM_KYX_RTN_OK; // 正常でreturn
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
/*                      0：正常                                             */
/*                      9：異常                                             */
/*  DESCRIPTION     : 要求電文の編集を行う                                  */
/****************************************************************************/
short NWM_KYX_msg_edit_req(
    short               exchange_type,      // 鍵交換電文種別
    db_gfnwi_def        *nw_info_group,     // NW情報レコード(グループ単位)
    db_gfnwi_def        *nw_info_interface, // NW情報レコード(インタフェース単位)
    char                *connect_nw,        // 接続先固有情報(NW単位)
    char                *connect_interface, // 接続先固有情報(インタフェース単位)
    char                *connect_station,   // 接続先固有情報(ステーション単位)
    char                *connect_connetion, // 接続先固有情報(コネクション単位)
    db_gckey_def        *gckey_info,        // 鍵管理情報レコード
    oggz1in_def         *oggz1in,           // EMS出力共通情報
    NWM_KYX_ems_add     *ems_add,           // EMS出力付加情報
    NWM_KYX_arg_3_def   *setting_data,      // 設定データ
    NWM_KYX_arg_4_def   *data_info)         // 電文情報
{
    int                      ret;
    msg_discover_def        *pt_fixedform_discover = (msg_discover_def*)data_info->message;
    MTI_0800                *data = (MTI_0800*)&pt_fixedform_discover->ffd;
    COM_SDT_arg_2_def        SDT_arg2; // システム日付取得アーギュメント2
    COM_SDT_arg_3_def        SDT_arg3; // システム日付取得アーギュメント3
    long long                SDT_arg4; // システム日付取得アーギュメント4
    char                     total_days[4];   // 通算日
    dst_unq_info_discover   *p_dst_unq_info = (dst_unq_info_discover *)nw_info_interface->dst_unq_info;
    
    //鍵交換電文種別チェック
    if (exchange_type != DEF_NWM_KYX_MSG_KIND_KEYREQ) {
        return DEF_NWM_KYX_RTN_ERR;
    }
    
    //動作モード精査            "A" GFP as Acquirer 以外は異常終了
    if (p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_A) {
        return DEF_NWM_KYX_RTN_ERR;
    }
    
    // 鍵管理情報レコード精査   "KB"以外は異常終了
    if (memcmp(gckey_info->key_info.key_format
              ,DEF_KEY_FMT_KB
              ,sizeof(gckey_info->key_info.key_format) != 0)) {
        return DEF_NWM_KYX_RTN_ERR;
    }
    
    // 鍵仕様アルゴリズム精査   "DS2" or "DS3"以外は異常終了
    if ((memcmp(gckey_info->key_info.key_use_alg
               ,DEF_KEY_DS2
               ,sizeof(gckey_info->key_info.key_use_alg) != 0)) &&
        (memcmp(gckey_info->key_info.key_use_alg
               ,DEF_KEY_DS3
               ,sizeof(gckey_info->key_info.key_use_alg) != 0))) {
        return DEF_NWM_KYX_RTN_ERR;
    }
    
    data_info->message_leng = sizeof(MTI_0800) + DEF_MTI_SIZE;
    memcpy(data_info->mti,DEF_MTI_0800,DEF_MTI_SIZE);               // MTI:0800
    
    memcpy(pt_fixedform_discover->mti,DEF_MTI_0800,DEF_MTI_SIZE);   // MTI:0800
    
    memset(data,NULL,sizeof(MTI_0800));
    // bit7
    data->bit7.flag      = true;
    data->bit7.data_len  = DEF_BIT7_SET_SIZE;
    // システム日付取得
    ret = COM_SDT(1             // 1：グリニッジ標準時
                 ,&SDT_arg2
                 ,&SDT_arg3
                 ,&SDT_arg4);
    if (ret != 0) {
        return DEF_NWM_KYX_RTN_ERR; //異常でreturn
    }
    memcpy(data->bit7.data,SDT_arg2.mm,data->bit7.data_len); // MMDDhhmmss の10桁
    
    // bit11
    data->bit11.flag      = true;
    data->bit11.data_len  = DEF_BIT11_SET_SIZE;
    
    memcpy(data->bit11.data             // システム採番生成値(Bit11)
          ,setting_data->sysytem_no
          ,data->bit11.data_len);
    
    // bit32
    data->bit32.flag      = false;
    
    // bit37
    data->bit37.flag      = true;
    
    // 通算日計算
    memset(total_days ,NULL,sizeof(total_days));
    ret = CMIN_get_day_of_year((char*)&SDT_arg2
                              ,total_days);
    if (ret != 0) {
        return DEF_NWM_KYX_RTN_ERR; // 異常でreturn
    }
    data->bit37.data[0] = SDT_arg2.yyyy[3];                             // 西暦下1桁
    memcpy(&data->bit37.data[1],total_days,strlen(total_days));         // 通算日3桁
    memcpy(&data->bit37.data[4],"00",strlen("00"));                     // "00"
    memcpy(&data->bit37.data[6],data->bit11.data,data->bit11.data_len); // システムトレースオーディットナンバー
    data->bit37.data_len = sizeof(data->bit37.data);
    
    // bit39
    data->bit39.flag      = false;
    
    // bit44
    data->bit44.flag      = false;
    
    // bit48
    data->bit48.flag      = false;
    
    // bit53
    data->bit53.flag      = true;
    data->bit53.data_len  = DEF_BIT53_SET_SIZE;
    // bit53
    memcpy( data->bit53.data,DEF_BIT53_DATA,DEF_BIT53_SET_SIZE - DEF_CHECK_DIGIT_SIZE_KB);
    memcpy(&data->bit53.data[DEF_BIT53_SET_SIZE - DEF_CHECK_DIGIT_SIZE_KB]
          ,setting_data->checkdigit
          ,DEF_CHECK_DIGIT_SIZE_KB);
    
    // bit59
    data->bit59.flag      = false;
    
    // bit70
    data->bit70.flag      = true;
    data->bit70.data_len  = DEF_BIT70_SET_SIZE;
    memcpy(data->bit70.data,DEF_BIT70_101,DEF_BIT70_SET_SIZE);
    
    // bit100
    data->bit100.flag     = false;
    
    // bit120
    data->bit120.flag     = true;
    data->bit120.data_len = setting_data->key_leng;
    memcpy(data->bit120.data,setting_data->key,setting_data->key_leng);
    
    // bit127
    data->bit127.flag     = true;
    data->bit127.data_len = DEF_BIT127_SET_SIZE;
    memcpy(data->bit127.data,DEF_BIT127_SET_DATA,DEF_BIT127_SET_SIZE);

    return DEF_NWM_KYX_RTN_OK; // 正常でreturn
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
/*                      0：正常                                             */
/*                      1：拒否応答                                         */
/*  DESCRIPTION     : 応答電文の編集を行う                                  */
/****************************************************************************/
short NWM_KYX_msg_edit_rsp(
    db_gfnwi_def        *nw_info_group,     // NW情報レコード(グループ単位)
    db_gfnwi_def        *nw_info_interface, // NW情報レコード(インタフェース単位)
    char                *connect_nw,        // 接続先固有情報(NW単位)
    char                *connect_interface, // 接続先固有情報(インタフェース単位)
    char                *connect_station,   // 接続先固有情報(ステーション単位)
    char                *connect_connetion, // 接続先固有情報(コネクション単位)
    db_gckey_def        *gckey_info,        // 鍵管理情報レコード
    file_info_gccut     *info_gccut,        // カット対象日付管理ファイル情報
    oggz1in_def         *oggz1in,           // EMS出力共通情報
    NWM_KYX_ems_add     *ems_add,           // EMS出力付加情報
    NWM_KYX_arg_5_def   *setting_data,      // 設定データ
    NWM_KYX_arg_6_def   *data_info)         // 電文情報
{
    msg_discover_def    *pt_fixedform_discover_receive = (msg_discover_def*)data_info->rcv_message;
    MTI_0800            *rcv_data                      = (MTI_0800*)&pt_fixedform_discover_receive->ffd;
    msg_discover_def    *pt_fixedform_discover_reply   = (msg_discover_def*)data_info->message;
    MTI_0810            *reply_data                    = (MTI_0810*)&pt_fixedform_discover_reply->ffd;
    short                ret_code;
    
    // 電文情報
    data_info->message_leng = sizeof(MTI_0810) + DEF_MTI_SIZE;
    memcpy(data_info->mti
          ,DEF_MTI_0810         // MTIに"0810"を設定
          ,DEF_MTI_SIZE);
    
    memcpy(pt_fixedform_discover_reply->mti
          ,DEF_MTI_0810         // MTIに"0810"を設定
          ,DEF_MTI_SIZE);
    memset(reply_data,NULL,sizeof(MTI_0810));
    
    ret_code = DEF_NWM_KYX_RTN_OK;
    // bit7設定
    memcpy(&reply_data->bit7
          ,&rcv_data->bit7
          ,sizeof(bit7_def));
    // bit11設定
    memcpy(&reply_data->bit11
          ,&rcv_data->bit11
          ,sizeof(bit11_def));
    // bit32設定
    memcpy(&reply_data->bit32
          ,&rcv_data->bit32
          ,sizeof(bit32_def));
    // bit37設定
    memcpy(&reply_data->bit37
          ,&rcv_data->bit37
          ,sizeof(bit37_def));
    // bit39設定
    reply_data->bit39.flag     = true;
    reply_data->bit39.data_len = DEF_BIT39_SET_SIZE;
    // 内部エラーコードが正常の場合
    if (memcmp(setting_data->err_code,DEF_IN_ERR_CODE_OK,sizeof(setting_data->err_code)) == 0) {
        memcpy(reply_data->bit39.data,DEF_BIT39_00,DEF_BIT39_SET_SIZE);
    }
    else {
        memcpy(reply_data->bit39.data,DEF_BIT39_96,DEF_BIT39_SET_SIZE);
        ret_code = DEF_NWM_KYX_RTN_NG_REJ;  // 処理結果に拒否応答をセット
    }
    
    // bit44設定
    memcpy(&reply_data->bit44
          ,&rcv_data->bit44
          ,sizeof(bit44_def));
    // bit48設定
    memcpy(&reply_data->bit48
          ,&rcv_data->bit48
          ,sizeof(bit48_def));
    reply_data->bit48.flag  = false;
    // bit53設定
    memcpy(&reply_data->bit53
          ,&rcv_data->bit53
          ,sizeof(bit53_def));
    reply_data->bit53.flag  = false;
    // bit59設定
    memcpy(&reply_data->bit59
          ,&rcv_data->bit59
          ,sizeof(bit59_def));
    // bit70設定
    memcpy(&reply_data->bit70
          ,&rcv_data->bit70
          ,sizeof(bit70_def));
    // bit100設定
    memcpy(&reply_data->bit100
          ,&rcv_data->bit100
          ,sizeof(bit100_def));
    // bit120設定
    memcpy(&reply_data->bit120
          ,&rcv_data->bit120
          ,sizeof(bit120_def));
    reply_data->bit120.flag = false;
    // bit127設定
    memcpy(&reply_data->bit127
          ,&rcv_data->bit127
          ,sizeof(bit127_def));
    
    return ret_code;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_req                          */
/*  CALLING SEQ.    : short  NWM_KYX_cst_check_req( )                       */
/*  ARGUMENT        : 局状態                                                */
/*  RETURN CODE     : 処理結果                                              */
/*                      0：正常                                             */
/*                      3：電文破棄                                         */
/*  DESCRIPTION     : 要求受信時に局状態が処理可能状態であることを判定      */
/****************************************************************************/
short NWM_KYX_cst_check_req(
    char    *station_status)            // 局状態
{
    // 局状態:開局
    if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0) {
        return DEF_NWM_KYX_RTN_OK; // 正常でreturn
    }
    // 局状態:その他
    else {
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 電文破棄でreturn
    }
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_cst_check_rsp                          */
/*  CALLING SEQ.    : short  NWM_KYX_cst_check_rsp( )                       */
/*  ARGUMENT        : 鍵交換電文種別                                        */
/*  ARGUMENT        : 局状態                                                */
/*  RETURN CODE     : 処理結果                                              */
/*                      0：正常                                             */
/*                      3：電文破棄                                         */
/*  DESCRIPTION     : 応答受信時に局状態が処理可能状態であることを判定      */
/****************************************************************************/
short NWM_KYX_cst_check_rsp(
    short   exchange_type,              // 鍵交換電文種別
    char    *station_status)            // 局状態
{
    //鍵交換電文種別チェック
    if (exchange_type != DEF_NWM_KYX_MSG_KIND_KEYRSP) {
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 電文破棄でreturn
    }
    
    // 局状態:閉局処理中
    if (memcmp(station_status,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1) == 0) {
        return DEF_NWM_KYX_RTN_HAKI_MSG; // 電文破棄でreturn
    }
    else {
        return DEF_NWM_KYX_RTN_OK; // 正常でreturn
    }
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_KYX_isdigit                                */
/*  CALLING SEQ.    : short  NWM_KYX_isdigit(char*,short)                   */
/*  ARGUMENT        : 判定する文字列                                        */
/*  ARGUMENT        : 文字列の長さ                                          */
/*  RETURN CODE     : 処理結果                                              */
/*                      true ：数字                                         */
/*                      false：数字以外が含まれている                       */
/*  DESCRIPTION     : 文字列が数字であるかを判定                            */
/****************************************************************************/
bool NWM_KYX_isdigit(
    char            *src
   ,unsigned long    len)
{
    int i;
    
    for (i = 0; i < len; i++) {
        if (!isdigit(src[i])) { // 数字以外
            return false;
        }
    }
    return true;
}
