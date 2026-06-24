/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD70                                    */
/*        FUNCTION          ････ 開局・閉局・エコー電文精査[Discover]        */
/*                                                                           */
/*                               ネットワーク経由の電文を受け取って          */
/*                               IPCを精査する                               */
/*                               処理結果を返す                              */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-30                                  */
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
#include  <stdbool.h>   nolist
#include  <stdio.h>     nolist
#include  <stdlib.h>    nolist
#include  <string.h>    nolist
#include  <ctype.h>     nolist
#include  <tal.h>       nolist
#include  <cextdecs.h>  nolist

/* USER HEADER */
#include "common.h"     nolist      // 共通ヘッダーファイル
#include "file.h"       nolist      // file
#include "ipc.h"        nolist      // ipc
#include "ems.h"
#include "NWM_CTU.h"
#include "GFPCVX80.h"               // 局状態・エコー制御サーバ
#include "GFPCSD70.h"   nolist      // 開局・閉局・エコー電文精査[Discover]
#include "GFPCGX50.h"   nolist      // システム日時取得・ヘッダファイル
#include "msg_DI.h"     nolist      // 共通ヘッダー(Discover)
#include "vproc.h"      nolist      // vproc


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
/*                      0：精査OK                                           */
/*                      3：精査エラー（破棄）                               */
/*  DESCRIPTION     : 開局・閉局・エコー要求電文精査                        */
/****************************************************************************/
short NWM_STE_check_reqmsg(
    char           *precv,                      // 受信電文（要求電文）
    char           *precv_len,                  // 受信電文長
    char           *nw_info_group,              // NW情報レコード(グループ単位)
    char           *nw_info_interface,          // NW情報レコード(インタフェース単位)
    char           *connect_inro_nw,            // 接続先固有情報レコード(NW単位)
    char           *connect_inro_interface,     // 接続先固有情報レコード(インタフェース単位)
    char           *connect_inro_station,       // 接続先固有情報レコード(ステーション単位)
    char           *connect_inro_connection,    // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def *connection_id,              // コネクション論理ID
    t_rcv_info_def *rcv_info,                   // 処理結果情報
    char           *station_record)             // 局状態管理ファイルレコード内容
{
    msg_discover_def      *pt_fixedform_discover = (msg_discover_def*)precv;
    db_gfnwi_def          *p_gfnwi_def           = (db_gfnwi_def *)nw_info_interface;
    dst_unq_info_discover *p_dst_unq_info        = (dst_unq_info_discover *)p_gfnwi_def->dst_unq_info;
    db_gcsst_def          *p_gcsst_def           = (db_gcsst_def *)station_record;
    char                   buff[256];

    // 制御電文種別4桁目="4"Acquirer
    if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL_ACQUIRER) {
        // 動作モードが"A" GFP as Acquirer 以外の場合は、リターン
        if (p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_A) {
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
    }
    // 制御電文種別4桁目="5"Issuer
    else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL_ISSUER) {
        // 動作モードが"I" GFP as Issuer 以外の場合は、リターン
        if (p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_I) {
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
    }
    // 制御電文種別が上記以外
    else {
        return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
    }

    // BITMAP精査
    // MTT=0800(局状態問合せ電文)の場合
    if (memcmp(pt_fixedform_discover->mti,DEF_MTI_0800,DEF_MTI_SIZE) == 0) {
        MTI_0800 *data   = (MTI_0800*)&pt_fixedform_discover->ffd;

        // bit.flag確認
        if ((data->bit7.flag   == false) ||
            (data->bit11.flag  == false) ||
            (data->bit32.flag  == false) ||
            (data->bit37.flag  == false) ||
            (data->bit70.flag  == false) ||
            (data->bit100.flag == false) ||
            (data->bit127.flag == false) ||
            (data->bit39.flag  == true)  ||
            (data->bit48.flag  == true)  ||
            (data->bit53.flag  == true)  ||
            (data->bit120.flag == true)   ) {
            memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }

        // bit7   日付（MMDDhhmmss）チェック
        memset(buff,NULL,sizeof(buff));
        memcpy(&buff[0],"0000",4);
        memcpy(&buff[4],data->bit7.data,data->bit7.data_len);
        if (CMIN_check_datetime(buff) != 0) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT7,sizeof(DEF_ERR_BIT7) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }

        // bit11  NUMERICチェック
        if (NWM_STE_isdigit(data->bit11.data,data->bit11.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT11,sizeof(DEF_ERR_BIT11) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit32  NUMERICチェック
        if (NWM_STE_isdigit(data->bit32.data,data->bit32.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT32,sizeof(DEF_ERR_BIT32) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit100 NUMERICチェック
        if (NWM_STE_isdigit(data->bit100.data,data->bit100.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT100,sizeof(DEF_ERR_BIT100) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }

    }
    // MTT=0820(局状態通知電文)の場合
    else {
        MTI_0820 *data   = (MTI_0820*)&pt_fixedform_discover->ffd;

        // bit.flag確認
        if ((data->bit7.flag   == false) ||
            (data->bit11.flag  == false) ||
            (data->bit32.flag  == false) ||
            (data->bit37.flag  == false) ||
            (data->bit39.flag  == false) ||
            (data->bit70.flag  == false) ||
            (data->bit100.flag == false) ||
            (data->bit127.flag == false) ||
            (data->bit48.flag  == true)  ||
            (data->bit53.flag  == true)  ||
            (data->bit120.flag == true)   ) {
            memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit7   日付（MMDDhhmmss）チェック
        memset(buff,NULL,sizeof(buff));
        memcpy(&buff[0],"0000",4);
        memcpy(&buff[4],data->bit7.data,data->bit7.data_len);
        if (CMIN_check_datetime(buff) != 0) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT7,sizeof(DEF_ERR_BIT7) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit11 NUMERICチェック
        if (NWM_STE_isdigit(data->bit11.data,data->bit11.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT11,sizeof(DEF_ERR_BIT11) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit32 NUMERICチェック
        if (NWM_STE_isdigit(data->bit32.data,data->bit32.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT32,sizeof(DEF_ERR_BIT32) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // bit100 NUMERICチェック
        if (NWM_STE_isdigit(data->bit100.data,data->bit100.data_len) == false) {
            memcpy(rcv_info->err_area,DEF_ERR_BIT100,sizeof(DEF_ERR_BIT100) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // 局状態と制御電文種別4桁目から制御電文種別3桁目を編集
        if ((memcmp(p_gcsst_def->state_sts_info.state_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) &&
            (memcmp(data->bit39.data,DEF_RES_CORD_N1,data->bit39.data_len) == 0)) {
            rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] = DEF_CTLTXT_ECH_SND;
        }   
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
/*  RETURN CODE     : 精査結果                                              */
/*                      0：精査OK                                           */
/*                      3：精査エラー（破棄）                               */
/*  DESCRIPTION     : 開局・閉局・エコー応答電文精査                        */
/****************************************************************************/
short NWM_STE_check_rspmsg(
    char            *precv,                     // 受信電文（応答電文）
    char            *precv_len,                 // 受信電文長
    char            *shimuke_data,              // 仕向要求電文
    char            *nw_info_group,             // NW情報レコード(グループ単位)
    char            *nw_info_interface,         // NW情報レコード(インタフェース単位)
    char            *connect_inro_nw,           // 接続先固有情報レコード(NW単位)
    char            *connect_inro_interface,    // 接続先固有情報レコード(インタフェース単位)
    char            *connect_inro_station,      // 接続先固有情報レコード(ステーション単位)
    char            *connect_inro_connection,   // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def  *connection_id,             // コネクション論理ID
    t_rcv_info_def  *rcv_info)                  // 処理結果情報
{
    msg_discover_def        *pt_fixedform_discover = (msg_discover_def*)precv;
    db_gfnwi_def            *p_gfnwi_def    = (db_gfnwi_def *)nw_info_interface;
    dst_unq_info_discover   *p_dst_unq_info = (dst_unq_info_discover *)p_gfnwi_def->dst_unq_info;
    
    // MTI＝"0810"
    if (memcmp(pt_fixedform_discover->mti,DEF_MTI_0810,DEF_MTI_SIZE) == 0) {
        MTI_0810  *data           = (MTI_0810*)&pt_fixedform_discover->ffd;
        
        // 動作モードが"A" GFP as Acquirer 以外の場合は、リターン
        if (p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_A) {
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // データ部精査
        if ((data->bit7.flag   == false) ||
            (data->bit11.flag  == false) ||
            (data->bit32.flag  == false) ||
            (data->bit37.flag  == false) ||
            (data->bit39.flag  == false) ||
            (data->bit48.flag  == true)  ||
            (data->bit53.flag  == true)  ||
            (data->bit70.flag  == false) ||
            (data->bit100.flag == false) ||
            (data->bit120.flag == true)  ||
            (data->bit127.flag == false)) {
            memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);   
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        //レスポンスコードチェック
        if (memcmp(data->bit39.data,DEF_RES_CORD_N1,data->bit39.data_len) == 0) {
            rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;
        }
        else{
            rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_DENY;
        }
    }
    // MTI="0830"
    else if (memcmp(pt_fixedform_discover->mti,DEF_MTI_0830,DEF_MTI_SIZE) == 0) {
        MTI_0830  *data           = (MTI_0830*)&pt_fixedform_discover->ffd;
        
        // 動作モードが"I" GFP as Issuer 以外の場合は、リターン
        if (p_dst_unq_info->move_mode != DEF_NW_MOVE_MODE_I) {
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        // データ部精査
        if ((data->bit7.flag   == false) ||
            (data->bit11.flag  == false) ||
            (data->bit32.flag  == false) ||
            (data->bit37.flag  == false) ||
            (data->bit39.flag  == false) ||
            (data->bit48.flag  == true)  ||
            (data->bit53.flag  == true)  ||
            (data->bit70.flag  == false) ||
            (data->bit100.flag == false) ||
            (data->bit120.flag == true)  ||
            (data->bit127.flag == false)) {
            memcpy(rcv_info->err_area,DEF_ERR_BITMAP,sizeof(DEF_ERR_BITMAP) - 1);
            return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
        }
        
        //レスポンスコードチェック
        if (memcmp(data->bit39.data,DEF_RES_CORD_OK,data->bit39.data_len) == 0) {
            rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;
        }
        else{
            rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_DENY;
        }
    }
    // 制御電文種別が上記以外
    else {
        return DEF_RET_DATA_BREAK; // 精査結果(電文破棄)でreturn
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
/*                      0：正常                                             */
/*                     -1：異常                                             */
/*  DESCRIPTION     : 開局・閉局・エコー要求電文編集                        */
/****************************************************************************/
short NWM_STE_edit_reqmsg(
    char                  *precv,                    // 送信電文（要求電文）
    char                  *nw_info_group,            // NW情報レコード(グループ単位)
    char                  *nw_info_interface,        // NW情報レコード(インタフェース単位)
    char                  *connect_info_nw,          // 接続先固有情報レコード(NW単位)
    char                  *connect_info_interface,   // 接続先固有情報レコード(インタフェース単位)
    char                  *connect_info_station,     // 接続先固有情報レコード(ステーション単位)
    char                  *connect_info_connection,  // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def        *connection_id,            // コネクション論理ID
    NWM_CTU_INI_arg_2_def *p_file_info_gccut,        // カット対象日付管理ファイル情報
    t_rcv_info_def        *rcv_info,                 // 処理結果情報
    oggz1in_def           *p_ems_info_cmn,           // EMS出力共通情報
    ems_info_add          *p_ems_info_add)           // EMS出力付加情報
{
    short              ret = 0;
    msg_discover_def  *pt_fixedform_discover = (msg_discover_def*)precv;
    COM_SDT_arg_2_def  SDT_arg2;        // システム日付取得アーギュメント2
    COM_SDT_arg_3_def  SDT_arg3;        // システム日付取得アーギュメント3
    long long          SDT_arg4;        // システム日付取得アーギュメント4
    char               total_days[4];   // 通算日
    char               buff[256];
    
    db_gfnwi_def          *p_gfnwi     = (db_gfnwi_def *)nw_info_interface;

    db_gfnws_def          *p_gfnws_if  = (db_gfnws_def *)connect_info_interface;
    db_gfnws_def          *p_gfnws_cn  = (db_gfnws_def *)connect_info_connection;

    dst_unq_info_discover *p_dst_unq_info = (dst_unq_info_discover *)p_gfnwi->dst_unq_info;
	
    nws_unq_info_di_def   *p_unq_info_if   = (nws_unq_info_di_def *)p_gfnws_if->dst_unq_info;
    nws_unq_info_di_def   *p_unq_info_cn   = (nws_unq_info_di_def *)p_gfnws_cn->dst_unq_info;
    
    // システム日付取得
    ret = COM_SDT(1             // 1：グリニッジ標準時
                 ,&SDT_arg2
                 ,&SDT_arg3
                 ,&SDT_arg4);
    if (ret != 0) {
        return DEF_RET_NG; // 異常でreturn
    }
    // 通算日計算
    memset(total_days ,NULL,sizeof(total_days));
    ret = CMIN_get_day_of_year((char*)&SDT_arg2
                              ,total_days);
    if (ret != 0) {
        return DEF_RET_NG; // 異常でreturn
    }
    
    
    // "A" GFP as Acquirer
    if(p_dst_unq_info->move_mode == DEF_NW_MOVE_MODE_A) {
        MTI_0800          *data = (MTI_0800*)&pt_fixedform_discover->ffd;
        
        memcpy(pt_fixedform_discover->mti,DEF_MTI_0800,DEF_MTI_SIZE);
        memset(data     ,NULL,sizeof(MTI_0800));    // 送信電文（要求電文）をNULLクリア
        
        // bitフラグ設定
        data->bit7.flag   = true;
        data->bit11.flag  = true;
        data->bit32.flag  = true;
        data->bit37.flag  = true;
        data->bit39.flag  = false;
        data->bit44.flag  = false;
        data->bit48.flag  = false;
        data->bit53.flag  = false;
        data->bit59.flag  = false;
        data->bit70.flag  = true;
        data->bit100.flag = true;
        data->bit120.flag = false;
        data->bit127.flag = true;
        
        // bit7設定
        memcpy(data->bit7.data
              ,SDT_arg2.mm
              ,sizeof(data->bit7.data));
        data->bit7.data_len = sizeof(data->bit7.data);
        
        // bit11設定
        memcpy(data->bit11.data,rcv_info->sys_no,sizeof(rcv_info->sys_no));
        data->bit11.data_len = sizeof(rcv_info->sys_no);
        
        // bit32設定
        memcpy(data->bit32.data,p_unq_info_if->acq_code,sizeof(p_unq_info_if->acq_code));
        data->bit32.data_len = sizeof(p_unq_info_if->acq_code);
        
        // bit37設定
        data->bit37.data[0] = SDT_arg2.yyyy[3];                             // 西暦下1桁
        memcpy(&data->bit37.data[1],total_days,strlen(total_days));         // 通算日3桁
        memcpy(&data->bit37.data[4],"00",strlen("00"));                     // "00"
        memcpy(&data->bit37.data[6],data->bit11.data,data->bit11.data_len); // システムトレースオーディットナンバー
        data->bit37.data_len = sizeof(data->bit37.data);
        
        // bit70
        memcpy(data->bit70.data
              ,DEF_NW_CORD_061          // "061"
              ,DEF_BIT70_SET_SIZE);
        data->bit70.data_len = DEF_BIT70_SET_SIZE;
        
        // bit100
        memcpy(data->bit100.data
              ,p_unq_info_cn->recv_code
              ,DEF_BIT100_SET_SIZE);
        data->bit100.data_len = DEF_BIT100_SET_SIZE;
        
        // bit127
        memcpy(data->bit127.data
              ,DEF_BIT127_SET_DATA
              ,DEF_BIT127_SET_SIZE);
        data->bit127.data_len = DEF_BIT127_SET_SIZE;
        
        // 処理結果情報編集
        memcpy(rcv_info->mti,pt_fixedform_discover->mti,DEF_MTI_SIZE);
        rcv_info->denbun_len = sizeof(MTI_0800) + DEF_MTI_SIZE;
    }
    // "I" GRP as Issuer
    else if(p_dst_unq_info->move_mode == DEF_NW_MOVE_MODE_I) {
        MTI_0820          *data = (MTI_0820*)&pt_fixedform_discover->ffd;
        
        memcpy(pt_fixedform_discover->mti,DEF_MTI_0820,DEF_MTI_SIZE);
        memset(data     ,NULL,sizeof(MTI_0820));    // 送信電文（要求電文）をNULLクリア
        
        // bitフラグ設定
        data->bit7.flag   = true;
        data->bit11.flag  = true;
        data->bit32.flag  = true;
        data->bit37.flag  = true;
        data->bit39.flag  = true;
        data->bit44.flag  = false;
        data->bit48.flag  = false;
        data->bit53.flag  = false;
        data->bit59.flag  = false;
        data->bit70.flag  = true;
        data->bit100.flag = true;
        data->bit120.flag = false;
        data->bit127.flag = true;
        
        // bit7設定
        memcpy(data->bit7.data
              ,SDT_arg2.mm
              ,sizeof(data->bit7.data));
        data->bit7.data_len = sizeof(data->bit7.data);
        
        // bit11設定
        memcpy(data->bit11.data,rcv_info->sys_no,sizeof(rcv_info->sys_no));
        data->bit11.data_len = sizeof(rcv_info->sys_no);
        
        // bit32設定
        memcpy(data->bit32.data,p_unq_info_if->acq_code,sizeof(p_unq_info_if->acq_code));
        data->bit32.data_len = sizeof(p_unq_info_if->acq_code);
        
        // bit37設定
        data->bit37.data[0] = SDT_arg2.yyyy[3];                             // 西暦下1桁
        memcpy(&data->bit37.data[1],total_days,strlen(total_days));         // 通算日3桁
        memcpy(&data->bit37.data[4],"00",strlen("00"));                     // "00"
        memcpy(&data->bit37.data[6],data->bit11.data,data->bit11.data_len); // システムトレースオーディットナンバー
        data->bit37.data_len = sizeof(data->bit37.data);
        
        // bit39
        // 制御電文種別3桁目 = 3(エコーテスト)の場合、"N1"(エコーテスト)をセット
        if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND) {
            memcpy(data->bit39.data,DEF_RES_CORD_N1,DEF_BIT39_SET_SIZE);
        }
        // 制御電文種別3桁目 = 2(閉局)の場合、"N3"(閉局)をセット
        else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
            memcpy(data->bit39.data,DEF_RES_CORD_N3,DEF_BIT39_SET_SIZE);
        }
        // 上記以外
        else{
            return DEF_RET_NG; // 異常でreturn
        }
        data->bit39.data_len = DEF_BIT39_SET_SIZE;
        
        // bit70
        memcpy(data->bit70.data
              ,DEF_NW_CORD_062          // "062"
              ,DEF_BIT70_SET_SIZE);
        data->bit70.data_len = DEF_BIT70_SET_SIZE;
        
        // bit100
        memcpy(data->bit100.data
              ,p_unq_info_cn->recv_code
              ,DEF_BIT100_SET_SIZE);
        data->bit100.data_len = DEF_BIT100_SET_SIZE;
        
        // bit127
        memcpy(data->bit127.data
              ,DEF_BIT127_SET_DATA
              ,DEF_BIT127_SET_SIZE);
        data->bit127.data_len = DEF_BIT127_SET_SIZE;
        
        // 処理結果情報編集
        memcpy(rcv_info->mti,pt_fixedform_discover->mti,DEF_MTI_SIZE);
        rcv_info->denbun_len = sizeof(MTI_0820) + DEF_MTI_SIZE;
    }
    // 制御電文種別が上記以外
    else {
        return DEF_RET_NG; // 異常でreturn
    }

    return DEF_RET_OK; // 正常でreturn
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
/*                      0：正常                                             */
/*                     -1：異常                                             */
/*  DESCRIPTION     : 開局・閉局・エコー応答電文編集                        */
/****************************************************************************/
short NWM_STE_edit_rspmsg(
    char                  *precv,                   // 送信電文（応答電文）
    char                  *prequest,                // 被仕向要求電文
    char                  *nw_info_group,           // NW情報レコード(グループ単位)
    char                  *nw_info_interface,       // NW情報レコード(インタフェース単位)
    char                  *connect_inro_nw,         // 接続先固有情報レコード(NW単位)
    char                  *connect_inro_interface,  // 接続先固有情報レコード(インタフェース単位)
    char                  *connect_inro_station,    // 接続先固有情報レコード(ステーション単位)
    char                  *connect_inro_connection, // 接続先固有情報レコード(コネクション単位)
    gflin_pkey_def        *connection_id,           // コネクション論理ID
    NWM_CTU_INI_arg_2_def *p_file_info_gccut,       // カット対象日付管理ファイル情報
    t_rcv_info_def        *rcv_info,                // 処理結果情報
    oggz1in_def           *p_ems_info_cmn,          // EMS出力共通情報
    ems_info_add          *p_ems_info_add)          // EMS出力付加情報
{
    msg_discover_def        *pt_fixedform_discover_preply  = (msg_discover_def*)precv;
    msg_discover_def        *pt_fixedform_discover_request = (msg_discover_def*)prequest;
    db_gfnwi_def            *p_gfnwi_def    = (db_gfnwi_def *)nw_info_interface;
    dst_unq_info_discover   *p_dst_unq_info = (dst_unq_info_discover *)p_gfnwi_def->dst_unq_info;


    // "I" GFP as Issuerの場合
    if (p_dst_unq_info->move_mode == DEF_NW_MOVE_MODE_I) {
        MTI_0800    *data_request   = (MTI_0800*)&pt_fixedform_discover_request->ffd;
        MTI_0810    *data_preply    = (MTI_0810*)&pt_fixedform_discover_preply->ffd;
        
        memcpy(pt_fixedform_discover_preply->mti,DEF_MTI_0810,DEF_MTI_SIZE);
        memset(data_preply,NULL,sizeof(MTI_0810));

        // bit7
        data_preply->bit7.flag     = data_request->bit7.flag;
        data_preply->bit7.data_len = data_request->bit7.data_len;
        memcpy(data_preply->bit7.data,data_request->bit7.data,sizeof(data_preply->bit7.data));
        
        // bit11
        data_preply->bit11.flag     = data_request->bit11.flag;
        data_preply->bit11.data_len = data_request->bit11.data_len;
        memcpy(data_preply->bit11.data,data_request->bit11.data,sizeof(data_preply->bit11.data));
        
        // bit32
        data_preply->bit32.flag     = data_request->bit32.flag;
        data_preply->bit32.data_len = data_request->bit32.data_len;
        memcpy(data_preply->bit32.data,data_request->bit32.data,sizeof(data_preply->bit32.data));
        
        // bit37
        data_preply->bit37.flag     = data_request->bit37.flag;
        data_preply->bit37.data_len = data_request->bit37.data_len;
        memcpy(data_preply->bit37.data,data_request->bit37.data,sizeof(data_preply->bit37.data));
        
        // bit39 "N1"を設定
        data_preply->bit39.flag     = true; 
        data_preply->bit39.data_len = DEF_BIT39_SET_SIZE;
        memcpy(data_preply->bit39.data,DEF_RES_CORD_N1,DEF_BIT39_SET_SIZE);
        
        // bit59
        data_preply->bit59.flag     = data_request->bit59.flag;
        data_preply->bit59.data_len = data_request->bit59.data_len;
        memcpy(data_preply->bit59.data,data_request->bit59.data,sizeof(data_preply->bit59.data));
        
        // bit70
        data_preply->bit70.flag     = data_request->bit70.flag;
        data_preply->bit70.data_len = data_request->bit70.data_len;
        memcpy(data_preply->bit70.data,data_request->bit70.data,sizeof(data_preply->bit70.data));
        
        // bit100
        data_preply->bit100.flag     = data_request->bit100.flag;
        data_preply->bit100.data_len = data_request->bit100.data_len;
        memcpy(data_preply->bit100.data,data_request->bit100.data,sizeof(data_preply->bit100.data));
        
        // bit127
        data_preply->bit127.flag     = data_request->bit127.flag;
        data_preply->bit127.data_len = data_request->bit127.data_len;
        memcpy(data_preply->bit127.data,data_request->bit127.data,sizeof(data_preply->bit127.data));
        
        // 処理結果情報編集
        memcpy(rcv_info->mti,pt_fixedform_discover_preply->mti,DEF_MTI_SIZE);
        rcv_info->denbun_len = sizeof(MTI_0810) + DEF_MTI_SIZE;
        rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;
    }
    // "A" GFP as Acquirer の場合
    else {
        MTI_0820    *data_request   = (MTI_0820*)&pt_fixedform_discover_request->ffd;
        MTI_0830    *data_preply    = (MTI_0830*)&pt_fixedform_discover_preply->ffd;
        
        memset(data_preply,NULL,sizeof(MTI_0830));
        memcpy(pt_fixedform_discover_preply->mti,DEF_MTI_0830,DEF_MTI_SIZE);

        // bit7
        data_preply->bit7.flag     = data_request->bit7.flag;
        data_preply->bit7.data_len = data_request->bit7.data_len;
        memcpy(data_preply->bit7.data,data_request->bit7.data,sizeof(data_preply->bit7.data));
        
        // bit11
        data_preply->bit11.flag     = data_request->bit11.flag;
        data_preply->bit11.data_len = data_request->bit11.data_len;
        memcpy(data_preply->bit11.data,data_request->bit11.data,sizeof(data_preply->bit11.data));
        
        // bit32
        data_preply->bit32.flag     = data_request->bit32.flag;
        data_preply->bit32.data_len = data_request->bit32.data_len;
        memcpy(data_preply->bit32.data,data_request->bit32.data,sizeof(data_preply->bit32.data));
        
        // bit37
        data_preply->bit37.flag     = data_request->bit37.flag;
        data_preply->bit37.data_len = data_request->bit37.data_len;
        memcpy(data_preply->bit37.data,data_request->bit37.data,sizeof(data_preply->bit37.data));
        
        // bit39 "00"を設定
        data_preply->bit39.flag     = true;
        data_preply->bit39.data_len = DEF_BIT39_SET_SIZE;
        memcpy(data_preply->bit39.data,DEF_RES_CORD_OK,DEF_BIT39_SET_SIZE);
        
        // bit59
        data_preply->bit59.flag     = data_request->bit59.flag;
        data_preply->bit59.data_len = data_request->bit59.data_len;
        memcpy(data_preply->bit59.data,data_request->bit59.data,sizeof(data_preply->bit59.data));
        
        // bit70
        data_preply->bit70.flag     = data_request->bit70.flag;
        data_preply->bit70.data_len = data_request->bit70.data_len;
        memcpy(data_preply->bit70.data,data_request->bit70.data,sizeof(data_preply->bit70.data));
        
        // bit100
        data_preply->bit100.flag     = data_request->bit100.flag;
        data_preply->bit100.data_len = data_request->bit100.data_len;
        memcpy(data_preply->bit100.data,data_request->bit100.data,sizeof(data_preply->bit100.data));
        
        // bit127
        data_preply->bit127.flag     = data_request->bit127.flag;
        data_preply->bit127.data_len = data_request->bit127.data_len;
        memcpy(data_preply->bit127.data,data_request->bit127.data,sizeof(data_preply->bit127.data));
        
        // 処理結果情報編集
        memcpy(rcv_info->mti,pt_fixedform_discover_preply->mti,DEF_MTI_SIZE);
        rcv_info->denbun_len = sizeof(MTI_0830) + DEF_MTI_SIZE;
        rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] = DEF_CTLINT_ALLOW;
    }

    return DEF_RET_OK; // 正常でreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_req_rcv                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_req_rcv                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 局状態判定結果結果                                    */
/*                      0：許可応答                                         */
/*                      2：電文破棄                                         */
/*  DESCRIPTION     : 開局・閉局・エコー局状態チェック（要求受信）          */
/****************************************************************************/
short NWM_STE_cst_check_req_rcv(
    char            *station_status,        // 局状態
    t_rcv_info_def  *rcv_info)              // 処理結果情報
{
    // space set    
    memset(rcv_info->new_stn_sts,' ',DEF_STTE_STS_SIZE);

    // 制御電文種別4桁目="4"Acquirerの場合
    if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL_ACQUIRER) {
        // 局状態:開局
        if(memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0){
            // サインオフ
            if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
            }
            // エコーテスト
            else if(rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND){
                // nop : 何も設定しない
            }
            return DEF_CHK_RES_OK; // 正常でreturn
        }
        // 局状態:開局処理中
        if(memcmp(station_status,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE) == 0){
            // サインオン
            if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN) {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);
            }
            // サインオフ
            else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
            }
            return DEF_CHK_RES_OK; // 正常でreturn
        }
        // 局状態:閉局
        if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
           // サインオン
           if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN) {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);
            }
            // サインオフ
            else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
                // nop : 何も設定しない
            }
            return DEF_CHK_RES_OK; // 正常でreturn
        }
        // 内部エラーコード設定(被仕向要求精査エラー)
        memcpy(rcv_info->naibu_errcd,DEF_NERR_HSMK_REQ_SEISA,sizeof(DEF_NERR_HSMK_REQ_SEISA) - 1);
    }
    // 制御電文種別4桁目="5"Issuerの場合
    else {
        // 局状態:開局
        if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0){
            return DEF_CHK_RES_OK; // 正常でreturn
        }
        // 局状態:閉局
        if(memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);
            return DEF_CHK_RES_OK; // 正常でreturn
        }
        // 局状態:閉局処理中
        if (memcmp(station_status,DEF_STTE_STS_CLOSING,DEF_STTE_STS_SIZE) == 0) {
            // 内部エラーコード設定(被仕向閉局要求精査エラー)
            memcpy(rcv_info->naibu_errcd,DEF_NERR_HSMK_CLS_REQ_SEISA,sizeof(DEF_NERR_HSMK_CLS_REQ_SEISA) - 1);
        }
        else {
            // 内部エラーコード設定(被仕向要求精査エラー)
            memcpy(rcv_info->naibu_errcd,DEF_NERR_HSMK_REQ_SEISA,sizeof(DEF_NERR_HSMK_REQ_SEISA) - 1);
        }
    }
    return DEF_CHK_RES_BREAK; // 電文破棄でreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_rsp_err                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_rsp_err                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 局状態判定結果                                        */
/*                      0：管理ファイル更新あり                             */
/*                      1：管理ファイル更新なし                             */
/*  DESCRIPTION     : 開局・閉局局状態チェック（応答送信不可）              */
/****************************************************************************/
short NWM_STE_cst_check_rsp_err(
    char            *station_status,        // 局状態
    t_rcv_info_def  *rcv_info)              // 処理結果情報
{
    memset(rcv_info->new_stn_sts,' ',DEF_STTE_STS_SIZE);

    // 内部処理区分が"A"許可応答以外
    if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] != DEF_CTLINT_ALLOW) {
        return DEF_RET_FILE_NO_UPDATE;  // 管理ファイル更新なしでreturn
    }
    // 局状態:開局
    if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0){
        memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
        return DEF_RET_FILE_UPDATE;     // 管理ファイル更新ありでreturn
    }
    // 局状態:閉局
    if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
        return DEF_RET_FILE_NO_UPDATE;      // 管理ファイル更新なしでreturn
    }
    return DEF_RET_FILE_NO_UPDATE;      // 管理ファイル更新なしでreturn
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_rsp_rcv                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_rsp_rcv                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : 要求種別                                              */
/*  ARGUMENT        : 受信電文                                              */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 局状態判定結果                                        */
/*                      0：管理ファイル更新あり                             */
/*                      1：管理ファイル更新なし                             */
/*                      2：管理ファイル更新あり（開局リトライ有り）         */
/*                      3：管理ファイル更新なし（開局リトライ有り）         */
/*  DESCRIPTION     : 開局・閉局局状態チェック（仕向応答）                  */
/****************************************************************************/
short NWM_STE_cst_check_rsp_rcv(
    char            *station_status,        // 局状態
    char            *request_type,          // 要求種別
    char            *receive_data,          // 受信電文
    t_rcv_info_def  *rcv_info)              // 処理結果情報
{
    short                simuke_kekka;                                // 仕向応答受信結果
    msg_discover_def    *pt_fixedform_discover = (msg_discover_def*)receive_data;
    
    // 変数初期化
    memset(rcv_info->new_stn_sts,' ',DEF_STTE_STS_SIZE);
    
    // 仕向応答受信結果の設定
    // "20"：仕向応答 の場合
    if (memcmp(request_type,DEF_CTLREQ_SIMUKE,sizeof(DEF_CTLREQ_SIMUKE) - 1) == 0) {
        // "A": 許可応答 の場合
        if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_ALLOW) {
            simuke_kekka = 0;       // 0：仕向応答OK
        }
        // "B": 拒否応答 の場合
        else {
            simuke_kekka = 1;       // 1：仕向応答NG
        }
    }
    // "30"：仕向応答タイムアウト の場合
    else if (memcmp(request_type,DEF_CTLREQ_TIMEOUT,sizeof(DEF_CTLREQ_TIMEOUT) - 1) == 0) {
        simuke_kekka = 2;       // 2：仕向応答NG（開局リトライ判定対象）
    }
    // "40"：仕向要求送信不可 の場合
    else {
        simuke_kekka = 1;       // 1：仕向応答NG
    }
    
    // 局状態判定結果と更新後局状態を設定
    
    // 制御電文種別3桁目:サインオン
    if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN) {
        // 0：仕向応答OK
        if (simuke_kekka == 0) {
            // 局状態:開局or閉局
            if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0
            ||  memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
            }
            // 局状態:開局処理中
            else {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);   // "10"：開局
                return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
            }
        }
        // 1：仕向応答NG
        else if (simuke_kekka == 1) {
            // 局状態:開局or閉局
            if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0
            ||  memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
            }
            // 局状態:開局処理中
            else {
                memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);   // "90"：閉局
                return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
            }
        }
        // 2：仕向応答NG（開局リトライ判定対象）
        else {
            // "7"：自動開局（コネクション確立） の場合
            if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_AUTO_CONNECT) {
                // 局状態:開局
                if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) {
                    return DEF_RET_FILE_NO_UPDATE_SIGNON_RETRY; // 管理ファイル更新なし（開局リトライ有り）でreturn
                }
                // 局状態:閉局
                if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                    return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
                }
                // 局状態:開局処理中
                else {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);   // "90"：閉局
                    return DEF_RET_FILE_UPDATE_SIGNON_RETRY; // 管理ファイル更新あり（開局リトライ有り）でreturn
                }
            }
            // 上記（"7"）以外
            else {
                // 局状態:開局or閉局
                if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0
                ||  memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                    return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
                }
                // 局状態:開局処理中
                else {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);   // "90"：閉局
                    return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
                }
            }
        }
    }
    // 制御電文種別3桁目:サインオフ
    else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
        // 局状態:開局or閉局
        if ((memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0)
        ||  (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0)) {
            return DEF_RET_FILE_NO_UPDATE; // 管理ファイル更新なしでreturn
        }
        else {
            memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);   // "90"：閉局
            return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
        }
    }
    // 制御電文種別3桁目:エコー
    else {
        return DEF_RET_FILE_UPDATE; // 管理ファイル更新ありでreturn
    }
}

/****************************************************************************/
/*  FUNCTION        : 0.0.0  NWM_STE_cst_check_command                      */
/*  CALLING SEQ.    : short NWM_STE_cst_check_command                       */
/*  ARGUMENT        : 局状態                                                */
/*  ARGUMENT        : NW情報レコード(インタフェース単位)                    */
/*  ARGUMENT        : 処理結果情報                                          */
/*  RETURN CODE     : 局状態判定結果                                        */
/*                      0：コマンド受付可（電文送信あり）                   */
/*                      1：コマンド受付可（電文送信なし）                   */
/*                      2：コマンド受付不可                                 */
/*  DESCRIPTION     : 開局・閉局・エコー局状態チェック（コマンド）          */
/****************************************************************************/
short NWM_STE_cst_check_command(
    char            *station_status,        // 局状態
    char            *nw_info_interface,     // NW情報レコード(インタフェース単位)
    t_rcv_info_def  *rcv_info)              // 処理結果情報
{
    db_gfnwi_def            *p_gfnwi_def    = (db_gfnwi_def *)nw_info_interface;
    dst_unq_info_discover   *p_dst_unq_info = (dst_unq_info_discover *)p_gfnwi_def->dst_unq_info;
    
    memset(rcv_info->new_stn_sts,' ',DEF_STTE_STS_SIZE);
    
    // "A" GFP as Acquirerの場合
    if (p_dst_unq_info->move_mode == DEF_NW_MOVE_MODE_A) {
        // 開局(サインオン)
        if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN) {
            // 通常
            if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL) {
                // 局状態:開局
                if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) {
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 局状態:閉局
                else if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 上記以外:内部エラーコード設定(仕向開局処理中の局状態チェックエラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK) - 1);
                }
            }
            // 強制実行
            else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_FORCE) {
                // 局状態:開局or閉局
                if ((memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 上記以外:内部エラーコード設定(仕向開局処理中の局状態チェックエラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK) - 1);
                }
            }
            // 状態更新のみ
            else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY) {
                // 局状態:開局
                if(memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0){
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 局状態:閉局or開局処理中
                if ((memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 上記以外:内部エラーコード設定(仕向開局処理中の局状態チェックエラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK) - 1);
                }
            }
            //自動開局
            else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_AUTO_CONNECT) {
                // 局状態:開局
                if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) {
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 局状態:閉局
                else if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 上記以外:内部エラーコード設定(仕向開局処理中の局状態チェックエラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK) - 1);
                }
            }
            // 上記以外:内部エラーコード設定(仕向開局要求精査エラー)
            else{
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_REQ_SEISA,sizeof(DEF_NERR_SMK_OPN_REQ_SEISA) - 1);
            }
        }
        // 閉局(サインオフ)
        else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
            // 状態更新のみ
            if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY) {
                // 局状態:閉局
                if(memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0){
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 局状態:開局or開局処理中
                if ((memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_OPNING,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 上記以外:内部エラーコード設定(仕向閉局要求精査エラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
                }
            }
            // 上記以外:内部エラーコード設定(仕向閉局処理中の局状態チェックエラー)
            else{
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
            } 
        }
        // 上記以外:内部エラーコード設定(仕向要求精査エラー)
        else{
            memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_REQ_SEISA,sizeof(DEF_NERR_SMK_REQ_SEISA) - 1);
        }
    }
    // "I" GFP as Issuerの場合
    else if (p_dst_unq_info->move_mode == DEF_NW_MOVE_MODE_I) {
        // 開局(サインオン)
        if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_OPN) {
            // 状態更新のみ
            if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY) {
                // 局状態:開局
                if(memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0){
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 局状態:閉局or閉局処理中
                if ((memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_CLOSING,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 上記以外:内部エラーコード設定(仕向開局処理中の局状態チェックエラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_STAT_CHK,sizeof(DEF_NERR_SMK_OPN_STAT_CHK) - 1);
                }
            }
            // 上記以外:内部エラーコード設定(仕向開局要求精査エラー)
            else{
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_OPN_REQ_SEISA,sizeof(DEF_NERR_SMK_OPN_REQ_SEISA) - 1);
            }
        }
        // 閉局(サインオフ)
        else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_CNT_CLS) {
            // 通常
            if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_NORMAL) {
                // 局状態:開局
                if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLOSING,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 局状態:閉局
                else if (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 上記以外:内部エラーコード設定(仕向閉局要求精査エラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
                }
            }
            // 強制実行
            else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_FORCE) {
                // 局状態:開局 or 閉局
                if ((memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLOSING,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
                }
                // 上記以外:内部エラーコード設定(仕向閉局要求精査エラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
                }
            }
            // 状態更新のみ
            else if (rcv_info->ctrl_type[DEF_CTLINT_TY_OFFSET] == DEF_CTLINT_UPDATEONLY) {
                // 局状態:閉局
                if(memcmp(station_status,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE) == 0){
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 局状態:開局or閉局処理中
                if ((memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0)
                ||  (memcmp(station_status,DEF_STTE_STS_CLOSING,DEF_STTE_STS_SIZE) == 0)) {
                    memcpy(rcv_info->new_stn_sts,DEF_STTE_STS_CLS,DEF_STTE_STS_SIZE);
                    return DEF_STATUS_RESULT_OK_NO_DATA; // コマンド受付可（電文送信なし）
                }
                // 上記以外:内部エラーコード設定(仕向閉局要求精査エラー)
                else{
                    memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
                }
            }
            // 上記以外:内部エラーコード設定(仕向閉局処理中の局状態チェックエラー)
            else{
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_CLS_STAT_CHK,sizeof(DEF_NERR_SMK_CLS_STAT_CHK) - 1);
            }
        }
        // エコーテスト
        else if (rcv_info->ctrl_type[DEF_CTLTEXT_TY_OFFSET] == DEF_CTLTXT_ECH_SND) {
            // 局状態:開局
            if (memcmp(station_status,DEF_STTE_STS_OPN,DEF_STTE_STS_SIZE) == 0) {
                return DEF_STATUS_RESULT_OK; // コマンド受付可（電文送信あり）
            }
            // 上記以外:内部エラーコード設定(仕向エコーテスト局状態チェックエラー)
            else{
                memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_ECHO_STAT_CHK,sizeof(DEF_NERR_SMK_ECHO_STAT_CHK) - 1);
            }
        }
        // 上記以外:内部エラーコード設定(仕向要求精査エラー)
        else{
            memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_REQ_SEISA,sizeof(DEF_NERR_SMK_REQ_SEISA) - 1);
        }
    }
    // 上記以外:内部エラーコード設定(仕向要求精査エラー)
    else{
        memcpy(rcv_info->naibu_errcd,DEF_NERR_SMK_REQ_SEISA,sizeof(DEF_NERR_SMK_REQ_SEISA) - 1);
    }
    return DEF_STATUS_RESULT_NG; // コマンド受付不可でreturn
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_STE_isdigit                                */
/*  CALLING SEQ.    : short  NWM_STE_isdigit(char*,short)                   */
/*  ARGUMENT        : 判定する文字列                                        */
/*  ARGUMENT        : 文字列の長さ                                          */
/*  RETURN CODE     : 処理結果                                              */
/*                      true ：数字                                         */
/*                      false：数字以外が含まれている                       */
/*  DESCRIPTION     : 文字列が数字であるかを判定                            */
/****************************************************************************/
bool NWM_STE_isdigit(
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
