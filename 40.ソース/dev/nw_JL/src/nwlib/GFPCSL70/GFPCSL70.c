/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSL70                                    */
/*        FUNCTION          ････ 開局・閉局・エコー電文精査[J-Link]          */
/*                                                                           */
/*                               ネットワーク経由の電文を受け取って          */
/*                               IPCを精査する                               */
/*                               処理結果を返す                              */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-02-19                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/02/19 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdbool.h>   nolist
#include  <stdio.h>     nolist
#include  <stdlib.h>    nolist
#include  <string.h>    nolist
#include  <ctype.h>     nolist

/* USER HEADER */
#include "common.h"     nolist      // 共通ヘッダーファイル
#include "file.h"       nolist      // file
#include "ipc.h"        nolist      // ipc
#include "ems.h"
#include "NWM_CTU.h"
#include "GFPCVX80.h"               // 局状態・エコー制御サーバ
#include "GFPCGX50.h"   nolist      // システム日時取得処理
#include "msg_JL.h"     nolist      // msg_JL
#include "vproc.h"      nolist      // vproc

/*****************************************************************************/
/*   DEFINE定義                                                              */
/*****************************************************************************/
#define DEF_NW_CORD_001                     "001"       // NW管理情報コード:開局
#define DEF_NW_CORD_002                     "002"       // NW管理情報コード:閉局
#define DEF_NW_CORD_301                     "301"       // NW管理情報コード:エコーテスト
#define DEF_RES_CORD_OK                     "00"        // RESPONCEコード:正常
#define DEF_MTI_EBC_0800                    0xf0f8f0f0  // MTI:0800 バイナリ
#define DEF_MTI_EBC_0810                    0xf0f8f1f0  // MTI:0810 バイナリ
#define DEF_MTI_ASC_0800                    "0800"      // MTI:0800 アスキー
#define DEF_MTI_ASC_0810                    "0810"      // MTI:0810 アスキー
#define DEF_MTI_SIZE                        4           // MTI SIZE
#define DEF_ERR_BIT7                        "BIT7"      // エラー発生エリア：7
#define DEF_ERR_BIT11                       "BIT11"     // エラー発生エリア：11
#define DEF_ERR_BIT33                       "BIT33"     // エラー発生エリア：33
#define DEF_ERR_BIT39                       "BIT39"     // エラー発生エリア：39
#define DEF_ERR_BIT53                       "BIT53"     // エラー発生エリア：53
#define DEF_ERR_BIT70                       "BIT70"     // エラー発生エリア：70
#define DEF_ERR_BIT96                       "BIT96"     // エラー発生エリア：96
#define DEF_ERR_BIT100                      "BIT100"    // エラー発生エリア：100
#define DEF_ERR_BIT105                      "BIT105"    // エラー発生エリア：105
#define DEF_ERR_BITMAP                      "BITMAP"    // エラー発生エリア：BITMAP
#define DEF_RET_OK                          0           // 返却値:OK / 許可
#define DEF_RET_NG                          -1          // 返却値:異常
#define DEF_RET_RES_NG                      1           // 返却値:精査結果(拒否応答)
#define DEF_RET_ERR_DATA                    2           // 返却値:精査結果(障害電文通知)
#define DEF_RET_DATA_BREAK                  4           // 返却値:精査結果(電文破棄)
#define DEF_CTLTEXT_TY_OFFSET               2           // 制御機能種別(制御電文区分)オフセット
#define DEF_CTLINT_TY_OFFSET                3           // 制御機能種別(内部処理区分)オフセット
#define DEF_RET_FILE_UPDATE                 0           // 管理ファイル:更新あり
#define DEF_RET_FILE_NO_UPDATE              1           // 管理ファイル:更新なし
#define DEF_RES_OK                          0           // 仕向応答:OK
#define DEF_RES_NG                          1           // 仕向応答:NG
#define DEF_STATUS_RESULT_OK                0           // コマンド受:付可（電文送信あり）
#define DEF_STATUS_RESULT_OK_NO_DATA        1           // コマンド受:付可（電文送信なし）
#define DEF_STATUS_RESULT_NG                2           // コマンド受:付不可
#define DEF_DATE_GMT                        1           // システム日時：グリニッジ標準時
#define DEF_DATE_BIT39                      "00"        // BIT39設定値
#define DEF_NERR_HSMK_REQ_SEISA             "SCDI001"   // 内部エラーコード:被仕向要求精査エラー
#define DEF_NERR_SMK_REQ_SEISA              "SCDI003"   // 内部エラーコード:仕向要求精査エラー
#define DEF_NERR_HSMK_CLS_REQ_SEISA         "SCDJ003"   // 内部エラーコード:被仕向閉局要求精査エラー
#define DEF_NERR_SMK_OPN_REQ_SEISA          "SCDJ009"   // 内部エラーコード:仕向開局要求精査エラー
#define DEF_NERR_SMK_OPN_STAT_CHK           "SCDJ010"   // 内部エラーコード:仕向開局処理中の局状態チェックエラー
#define DEF_NERR_SMK_CLS_REQ_SEISA          "SCDJ015"   // 内部エラーコード:仕向閉局要求精査エラー
#define DEF_NERR_SMK_CLS_STAT_CHK           "SCDJ016"   // 内部エラーコード:仕向閉局処理中の局状態チェックエラー
#define DEF_NERR_SMK_ECHO_STAT_CHK          "SCDJ024"   // 内部エラーコード:仕向エコーテスト局状態チェックエラー

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_check_reqmsg                           */
/*  CALLING SEQ.    : short NWM_STE_check_reqmsg                            */
/*  ARGUMENT        : 受信電文（要求電文）                                  */
/*  ARGUMENT        : 受信電文長                                            */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報レコード(NW単位)                        */
/*  ARGUMENT        : 接続先固有情報レコード(インタフェース単位)            */
/*  ARGUMENT        : 接続先固有情報レコード(ステーション単位)              */
/*  ARGUMENT        : 接続先固有情報レコード(コネクション単位)              */
/*  ARGUMENT        : コネクション論理ID                                    */
/*  ARGUMENT        : 処理結果情報                                          */
/*  ARGUMENT        : 局状態管理ファイルレコード内容                        */
/*  RETURN CODE     : 精査結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー要求電文精査                        */
/****************************************************************************/
short NWM_STE_check_reqmsg(
    char *precv,                                        // 受信電文（要求電文）
    char *precv_len,                                    // 受信電文長
    char *nw_info_group,                                // NW情報レコード(グループ単位)
    char *nw_info_interface,                            // NW情報レコード(インタフェース単位)
    char *connect_inro_nw,                              // 接続先固有情報レコード(NW単位)
    char *connect_inro_interface,                       // 接続先固有情報レコード(インタフェース単位)
    char *connect_inro_station,                         // 接続先固有情報レコード(ステーション単位)
    char *connect_inro_connection,                      // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def *connection_id,                      // コネクション論理ID
    t_rcv_info_def *rcv_info,                           // 処理結果情報
    char *station_record)                               // 局状態管理ファイルレコード内容

{
    // 変数初期化
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0800 *data = (MTI_0800*)&pt_fixedform_jlink->ffd;

    // bit.flag確認
    if ((data->bit7.flag == false)
    || (data->bit11.flag == false)
    || (data->bit33.flag == false)
    || (data->bit53.flag == true)
    || (data->bit70.flag == false)
    || (data->bit96.flag == true)
    || (data->bit100.flag == false)
    || (data->bit105.flag == true)){
        memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);
        return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
    }

    // bit7 NUMERICチェック
    for (int i=0;i<data->bit7.data_len;i++){
        if(!isdigit(data->bit7.data[i])){ // 数値じゃなかったら
        memcpy(rcv_info->err_area,DEF_ERR_BIT7,sizeof(DEF_ERR_BIT7) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    }
    // bit11 NUMERICチェック
    for (int i=0;i<data->bit11.data_len;i++){
        if(!isdigit(data->bit11.data[i])){ // 数値じゃなかったら
            memcpy(rcv_info->err_area,DEF_ERR_BIT11,sizeof(DEF_ERR_BIT11) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    }
    // bit33 NUMERICチェック
    for (int i=0;i<data->bit33.data_len;i++){
        if(!isdigit(data->bit33.data[i])){ // 数値じゃなかったら
            memcpy(rcv_info->err_area,DEF_ERR_BIT33,sizeof(DEF_ERR_BIT33) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    }
    // bit70 値チェック
    // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        if (memcmp(data->bit70.data,DEF_NW_CORD_001,sizeof(DEF_NW_CORD_001)-1) != 0){
            memcpy(rcv_info->err_area,DEF_ERR_BIT70,sizeof(DEF_ERR_BIT70) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    // 閉局(サインオフ)
    } else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        if (memcmp(data->bit70.data,DEF_NW_CORD_002,sizeof(DEF_NW_CORD_002)-1) != 0){
            memcpy(rcv_info->err_area,DEF_ERR_BIT70,sizeof(DEF_ERR_BIT70) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    // エコーテスト
    } else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND){
        if (memcmp(data->bit70.data,DEF_NW_CORD_301,sizeof(DEF_NW_CORD_301)-1) != 0){
            memcpy(rcv_info->err_area,DEF_ERR_BIT70,sizeof(DEF_ERR_BIT70) - 1);
            return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
        }
    // その他
    } else {
        memcpy(rcv_info->err_area,DEF_ERR_BIT70,sizeof(DEF_ERR_BIT70) - 1);
        return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
    }
    return DEF_RET_OK; // 正常でreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_check_rspmsg                           */
/*  CALLING SEQ.    : short NWM_STE_check_rspmsg                            */
/*  ARGUMENT        : 受信電文（応答電文）                                  */
/*  ARGUMENT        : 受信電文長                                            */
/*  ARGUMENT        : 仕向要求電文                                          */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報レコード(NW単位)                        */
/*  ARGUMENT        : 接続先固有情報レコード(インタフェース単位)            */
/*  ARGUMENT        : 接続先固有情報レコード(ステーション単位)              */
/*  ARGUMENT        : 接続先固有情報レコード(コネクション単位)              */
/*  ARGUMENT        : コネクション論理ID                                    */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー応答電文精査                        */
/****************************************************************************/
short NWM_STE_check_rspmsg(
    char *precv,                                        // 受信電文（応答電文）
    char *precv_len,                                    // 受信電文長
    char *shimuke_data,                                 // 仕向要求電文
    char *nw_info_group,                                // NW情報レコード(グループ単位)
    char *nw_info_interface,                            // NW情報レコード(インタフェース単位)
    char *connect_inro_nw,                              // 接続先固有情報レコード(NW単位)
    char *connect_inro_interface,                       // 接続先固有情報レコード(インタフェース単位)
    char *connect_inro_station,                         // 接続先固有情報レコード(ステーション単位)
    char *connect_inro_connection,                      // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def *connection_id,                      // コネクション論理ID
    t_rcv_info_def *rcv_info)                           // 処理結果情報
{
    // 変数初期化
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0810 *data = (MTI_0810*)&pt_fixedform_jlink->ffd;
    
    // 制御電文種別4桁目にAを入れる
    rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;

    // bit.flag確認
    if ((data->bit7.flag == false)
    || (data->bit11.flag == false)
    || (data->bit33.flag == false)
    || (data->bit39.flag == false)
    || (data->bit53.flag == true)
    || (data->bit70.flag == false)
    || (data->bit96.flag == true)
    || (data->bit100.flag == false)
    || (data->bit105.flag == true)){
        memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);
        // 制御電文種別4桁目にBを入れる
        rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_DENY;
        return DEF_RET_ERR_DATA; // 精査結果(障害電文通知)でreturn
    }
    // bit39 RESPONCEコードチェック
    if (memcmp(data->bit39.data,DEF_RES_CORD_OK,data->bit39.data_len) != 0){
        // 制御電文種別4桁目にBを入れる
        rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_DENY;
        return DEF_RET_RES_NG; // 精査結果(拒否応答)でreturn
    }
    return DEF_RET_OK; // 正常でreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_edit_reqmsg                            */
/*  CALLING SEQ.    : short NWM_STE_edit_reqmsg                             */
/*  ARGUMENT        : 送信電文（要求電文）                                  */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報レコード(NW単位)                        */
/*  ARGUMENT        : 接続先固有情報レコード(インタフェース単位)            */
/*  ARGUMENT        : 接続先固有情報レコード(ステーション単位)              */
/*  ARGUMENT        : 接続先固有情報レコード(コネクション単位)              */
/*  ARGUMENT        : コネクション論理ID                                    */
/*  ARGUMENT        : カット対象日付管理ファイル情報                        */
/*  ARGUMENT        : 処理結果情報                                          */
/*  ARGUMENT        : EMS出力共通情報                                       */
/*  ARGUMENT        : EMS出力付加情報                                       */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー要求電文編集                        */
/****************************************************************************/
short NWM_STE_edit_reqmsg(
    char *precv,                                        // 送信電文（要求電文）
    char *nw_info_group,                                // NW情報レコード(グループ単位)
    char *nw_info_interface,                            // NW情報レコード(インタフェース単位)
    char *connect_inro_nw,                              // 接続先固有情報レコード(NW単位)
    char *connect_inro_interface,                       // 接続先固有情報レコード(インタフェース単位)
    char *connect_inro_station,                         // 接続先固有情報レコード(ステーション単位)
    char *connect_inro_connection,                      // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def *connection_id,                      // コネクション論理ID
    NWM_CTU_INI_arg_2_def *p_file_info_gccut,           // カット対象日付管理ファイル情報
    t_rcv_info_def *rcv_info,                           // 処理結果情報
    oggz1in_def *p_ems_info_cmn,                        // EMS出力共通情報
    ems_info_add *p_ems_info_add)                       // EMS出力付加情報
{
    // 変数初期化
    msg_jlink_def* pt_fixedform_jlink = (msg_jlink_def*)precv;
    MTI_0800 *data = (MTI_0800*)&pt_fixedform_jlink->ffd;
    db_gfnws_def *connect_info = (db_gfnws_def*)connect_inro_interface;
    dst_unq_info_JL *dst_unq_info =(dst_unq_info_JL *)connect_info->dst_unq_info;
    COM_SDT_arg_2_def SDT_arg2; // システム日付取得アーギュメント2
    COM_SDT_arg_3_def SDT_arg3; // システム日付取得アーギュメント3
    long long SDT_arg4; // システム日付取得アーギュメント4
    short ret;
    char wk_buf[12];//編集用バッファ

    memset(precv,' ',sizeof(MTI_0800) + DEF_MTI_SIZE);

    int mti = DEF_MTI_EBC_0800;

    // MTI設定
    memcpy(pt_fixedform_jlink->mti,&mti,DEF_MTI_SIZE);

    // bit7設定
    data->bit7.flag = true;
    // bit11設定
    data->bit11.flag = true;
    // bit33設定
    data->bit33.flag = true;
    // bit53設定
    data->bit53.flag = false;
    // bit70設定
    data->bit70.flag = true;
    // bit96設定
    data->bit96.flag = false;
    // bit100設定
    data->bit100.flag = true;
    // bit105設定
    data->bit105.flag = false;

    // システム日付取得(Bit7)
    data->bit7.data_len = sizeof(data->bit7.data);
    ret = COM_SDT(DEF_DATE_GMT,&SDT_arg2,&SDT_arg3,&SDT_arg4);
    if (ret) {
        return DEF_RET_NG; // 異常でreturn
    }
    memcpy(data->bit7.data,SDT_arg2.mm,data->bit7.data_len); // MMDDhhmmss の10桁
    // システム採番生成値(Bit11)
    memcpy(data->bit11.data,rcv_info->sys_no,sizeof(rcv_info->sys_no));
    data->bit11.data_len = sizeof(rcv_info->sys_no);
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
    
    // NW管理情報コード(Bit70)
    data->bit70.data_len = sizeof(DEF_NW_CORD_001)-1;
        // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        memcpy(data->bit70.data,DEF_NW_CORD_001,sizeof(DEF_NW_CORD_001)-1);
    // 閉局(サインオフ)
    } else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        memcpy(data->bit70.data,DEF_NW_CORD_002,sizeof(DEF_NW_CORD_002)-1);
    // エコーテスト
    } else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND){
        memcpy(data->bit70.data,DEF_NW_CORD_301,sizeof(DEF_NW_CORD_301)-1);
    // その他
    } else {
        return DEF_RET_NG; // 異常でreturn
    }

    // 処理結果情報編集
    memcpy(rcv_info->mti,DEF_MTI_ASC_0800,DEF_MTI_SIZE);
    rcv_info->denbun_len = sizeof(MTI_0800) + DEF_MTI_SIZE;

    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_edit_rspmsg                            */
/*  CALLING SEQ.    : short NWM_STE_edit_rspmsg                             */
/*  ARGUMENT        : 送信電文（応答電文）                                  */
/*  ARGUMENT        : 被仕向要求電文                                        */
/*  ARGUMENT        : NW情報レコード(グループ単位)                          */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 接続先固有情報レコード(NW単位)                        */
/*  ARGUMENT        : 接続先固有情報レコード(インタフェース単位)            */
/*  ARGUMENT        : 接続先固有情報レコード(ステーション単位)              */
/*  ARGUMENT        : 接続先固有情報レコード(コネクション単位)              */
/*  ARGUMENT        : コネクション論理ID                                    */
/*  ARGUMENT        : カット対象日付管理ファイル情報                        */
/*  ARGUMENT        : 処理結果情報                                          */
/*  ARGUMENT        : EMS出力共通情報                                       */
/*  ARGUMENT        : EMS出力付加情報                                       */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー応答電文編集                        */
/****************************************************************************/
short NWM_STE_edit_rspmsg(
    char *precv,                                        // 送信電文（応答電文）
    char *prequest,                                     // 被仕向要求電文
    char *nw_info_group,                                // NW情報レコード(グループ単位)
    char *nw_info_interface,                            // NW情報レコード(インタフェース単位)
    char *connect_inro_nw,                              // 接続先固有情報レコード(NW単位)
    char *connect_inro_interface,                       // 接続先固有情報レコード(インタフェース単位)
    char *connect_inro_station,                         // 接続先固有情報レコード(ステーション単位)
    char *connect_inro_connection,                      // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def *connection_id,                      // コネクション論理ID
    NWM_CTU_INI_arg_2_def *p_file_info_gccut,           // カット対象日付管理ファイル情報
    t_rcv_info_def *rcv_info,                           // 処理結果情報
    oggz1in_def *p_ems_info_cmn,                        // EMS出力共通情報
    ems_info_add *p_ems_info_add)                       // EMS出力付加情報
{
    // ポインタ設定
    msg_jlink_def* pt_fixedform_jlink_request = (msg_jlink_def*)prequest;
    MTI_0800 *request_data = (MTI_0800*)&pt_fixedform_jlink_request->ffd;

    msg_jlink_def* pt_fixedform_jlink_recve = (msg_jlink_def*)precv;
    MTI_0810 *recve_data = (MTI_0810*)&pt_fixedform_jlink_recve->ffd;

    memset(precv,' ',sizeof(MTI_0810) + DEF_MTI_SIZE);

    // MTI設定
    int mti = DEF_MTI_EBC_0810;
    memcpy(pt_fixedform_jlink_recve->mti,&mti,DEF_MTI_SIZE);

    // bit7設定
    recve_data->bit7.flag = request_data->bit7.flag;
    recve_data->bit7.data_len = request_data->bit7.data_len;
    memcpy(recve_data->bit7.data,request_data->bit7.data,request_data->bit7.data_len);
    // bit11設定
    recve_data->bit11.flag = request_data->bit11.flag;
    recve_data->bit11.data_len = request_data->bit11.data_len;
    memcpy(recve_data->bit11.data,request_data->bit11.data,request_data->bit11.data_len);
    // bit33設定
    recve_data->bit33.flag = request_data->bit33.flag;
    recve_data->bit33.data_len = request_data->bit33.data_len;
    memcpy(recve_data->bit33.data,request_data->bit33.data,request_data->bit33.data_len);
    // bit39設定
    recve_data->bit39.flag = true;
    recve_data->bit39.data_len = sizeof(recve_data->bit39.data);
    memcpy(recve_data->bit39.data,DEF_DATE_BIT39,sizeof(recve_data->bit39.data));
    // bit53設定
    recve_data->bit53.flag = request_data->bit53.flag;
    recve_data->bit53.data_len = request_data->bit53.data_len;
    memcpy(recve_data->bit53.data,request_data->bit53.data,request_data->bit53.data_len);
    // bit70設定
    recve_data->bit70.flag = request_data->bit70.flag;
    recve_data->bit70.data_len = request_data->bit70.data_len;
    memcpy(recve_data->bit70.data,request_data->bit70.data,request_data->bit70.data_len);
    // bit96設定
    recve_data->bit96.flag = request_data->bit96.flag;
    recve_data->bit96.data_len = request_data->bit96.data_len;
    memcpy(recve_data->bit96.data,request_data->bit96.data,request_data->bit96.data_len);
    // bit100設定
    recve_data->bit100.flag = request_data->bit100.flag;
    recve_data->bit100.data_len = request_data->bit100.data_len;
    memcpy(recve_data->bit100.data,request_data->bit100.data,request_data->bit100.data_len);
    // bit105設定
    recve_data->bit105.flag = request_data->bit105.flag;
    recve_data->bit105.data_len = request_data->bit105.data_len;
    memcpy(recve_data->bit105.data,request_data->bit105.data,request_data->bit105.data_len);

    // 処理結果情報編集
    memcpy(rcv_info->mti,DEF_MTI_ASC_0810,DEF_MTI_SIZE);
    rcv_info->denbun_len = sizeof(MTI_0810) + DEF_MTI_SIZE;
    rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;

    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_req_rcv                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_req_rcv                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー局状態チェック                      */
/****************************************************************************/
short NWM_STE_cst_check_req_rcv(
    char *station_status,                               // 局状態
    t_rcv_info_def *rcv_info)                           // 処理結果情報
{
    // space set
    memset(rcv_info->new_stn_sts,' ',sizeof(DEF_STTE_STS_OPN)-1);

    // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        // 局状態:開局
        if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
            return DEF_RET_OK; // 許可応答でreturn
        }
        // 局状態:閉局or開局処理中
        if (memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0
        || memcmp(station_status,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1) == 0){
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1);
            return DEF_RET_OK; // 許可応答でreturn
        }
        // 局状態:閉局処理中 内部エラーコード設定(被仕向閉局要求精査エラー)
        memcpy(rcv_info->naibu_errcd,DEF_NERR_HSMK_CLS_REQ_SEISA,sizeof(DEF_NERR_HSMK_CLS_REQ_SEISA)-1);
        return DEF_RET_DATA_BREAK; // 電文破棄でreturn
    } 
    // 閉局(サインオフ)
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        // 局状態:閉局
        if(memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
            return DEF_RET_OK; // 許可応答でreturn
        }
        // 局状態:開局or開局処理中or閉局処理中
        memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1);
        return DEF_RET_OK; // 許可応答でreturn
    }
    // エコーテスト
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND){
        return DEF_RET_OK; // 許可応答でreturn
    }
    // その他
    else {
        // 内部エラーコード設定(被仕向要求精査エラー)
        memcpy(rcv_info->naibu_errcd,DEF_NERR_HSMK_REQ_SEISA,sizeof(DEF_NERR_HSMK_REQ_SEISA)-1);
        return DEF_RET_DATA_BREAK; // 電文破棄でreturn
    }
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_rsp_err                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_rsp_err                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局局状態チェック（応答送信不可）              */
/****************************************************************************/
short NWM_STE_cst_check_rsp_err(
    char *station_status,                               // 局状態
    t_rcv_info_def *rcv_info)                           // 処理結果情報
{
    // space set
    memset(rcv_info->new_stn_sts,' ',sizeof(DEF_STTE_STS_OPN)-1);

    // 制御電文種別4桁目 = "A"：許可応答 以外の場合
    if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] != DEF_CTLINT_ALLOW){
        return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
    }
    // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        // 局状態:開局
        if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1);
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
        // 局状態:閉局or開局処理中or閉局処理中
        return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
    }
    // 閉局(サインオフ)
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        // 局状態:開局
        if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1);
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
        // 局状態:閉局or開局処理中or閉局処理中
        return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
    }
    else {
        return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
    }
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_rsp_rcv                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_rsp_rcv                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 要求種別                                              */
/*  ARGUMENT        : 受信電文                                              */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局局状態チェック（仕向応答）                  */
/****************************************************************************/
short NWM_STE_cst_check_rsp_rcv(
    char *station_status,                               // 局状態
    char *request_type,                                 // 要求種別
    char *receive_data,                                 // 受信電文
    t_rcv_info_def *rcv_info)                           // 処理結果情報
{
    // 内部変数定義
    short shimuke_result = 0;
    // space set
    memset(rcv_info->new_stn_sts,' ',sizeof(DEF_STTE_STS_OPN)-1);

    // 仕向応答受信結果の設定
    if ((memcmp(request_type,DEF_CTLREQ_SIMUKE,sizeof(DEF_CTLREQ_SIMUKE)-1) == 0)
    && (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_ALLOW)){
        shimuke_result = DEF_RES_OK; // 仕向応答OKを設定
    }
    else {
        shimuke_result = DEF_RES_NG; // 仕向応答NGを設定
    }

    // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        // 局状態:開局処理中 && 仕向応答OK
        if ((memcmp(station_status,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1) == 0)
        && (shimuke_result == DEF_RES_OK)){
            // 開局に更新
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1);
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
        // 局状態:開局処理中 && 仕向応答NG
        else if ((memcmp(station_status,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1) == 0)
        && (shimuke_result == DEF_RES_NG)){
            // 閉局に更新
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_OPN)-1);
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
        // それ以外
        else {
            return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
        }
    }
    // 閉局(サインオフ)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        // 局状態:閉局処理中
        if (memcmp(station_status,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1) == 0){
            // 閉局に更新
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1);
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
        // それ以外
        else {
            return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
        }
    } 
    // それ以外
    return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_command                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_command                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : 開局・閉局・エコー局状態チェック（コマンド）          */
/****************************************************************************/
short NWM_STE_cst_check_command(
    char *station_status,                               // 局状態
    char *nw_info_interface,                            // NW情報レコード(インタフェース単位)
    t_rcv_info_def *rcv_info)                           // 処理結果情報
{
    // space set
    memset(rcv_info->new_stn_sts,' ',sizeof(DEF_STTE_STS_OPN)-1);

    // 開局(サインオン)
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN){
        // 通常
        if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL){
            // 局状態:開局
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // 局状態:閉局
            else if (memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1);
                return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向開局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // 強制実行
        else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_FORCE){
            // 局状態:開局or閉局
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1);
                return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向開局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // 状態更新のみ
        else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY){
            // 局状態:開局
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // 局状態:閉局or開局処理中or閉局処理中
            if(memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1);
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向開局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // その他
        else {
            // 内部エラー設定(仕向開局要求精査エラー)
            memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_REQ_SEISA,sizeof(DEF_NERR_SMK_OPN_REQ_SEISA)-1);
            return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
        }
    } 
    // 閉局(サインオフ)
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS){
        // 通常
        if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL){
            // 局状態:開局
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1);
                return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）でreturn
            }
            // 局状態:閉局
            else if (memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向閉局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // 強制実行
        else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_FORCE){
            // 局状態:開局or閉局
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1);
                return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向閉局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // 状態更新のみ
        else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY){
            // 局状態:閉局
            if(memcmp(station_status,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1) == 0){
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // 局状態:開局or開局処理中or閉局処理中
            if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_OPNING,sizeof(DEF_STTE_STS_OPNING)-1) == 0
            || memcmp(station_status,DEF_STTE_STS_CLOSING,sizeof(DEF_STTE_STS_CLOSING)-1) == 0){
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,sizeof(DEF_STTE_STS_CLS)-1);
                return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）でreturn
            }
            // その他
            else {
                // 内部エラー設定(仕向閉局処理中の局状態チェックエラー)
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK)-1);
                return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
            }
        }
        // その他
        else {
            // 内部エラー設定(仕向閉局要求精査エラー)
            memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_REQ_SEISA,sizeof(DEF_NERR_SMK_CLS_REQ_SEISA)-1);
            return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
        }
    }
    // エコーテスト
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND){
        // 局状態:開局
        if (memcmp(station_status,DEF_STTE_STS_OPN,sizeof(DEF_STTE_STS_OPN)-1) == 0){
            return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）でreturn
        }
        // その他
        else {
            // 内部エラー設定(仕向エコーテスト局状態チェックエラー)
            memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_ECHO_STAT_CHK,sizeof(DEF_NERR_SMK_ECHO_STAT_CHK)-1);
            return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
        }
    }
    // その他
    // 内部エラー設定(仕向要求精査エラー)
    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_REQ_SEISA,sizeof(DEF_NERR_SMK_REQ_SEISA)-1);
    return DEF_RET_RES_NG; // コマンド受付不可でreturn
}
