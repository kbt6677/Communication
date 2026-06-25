/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVXA0                                    */
/*        FUNCTION          ････ カットオーバー制御                          */
/*                                                                           */
/*                               制御電文振り分けから受信したI/Fに従い、     */
/*                               カット日付の更新を行う。                    */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-31                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Sugisaki 2025/03/31 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* STANDARD HEADER */
#include <stdio.h>    nolist
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist
#include <stdarg.h>

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"
#include "ipc.h"
#include "GFPCGXG0.h"
#include "GFPCGXC0.h"
#include "GFPCGX80.h"
#include "NWM_ENI.h"

#include "GFPCGX90.h"
#include "NWM_CTO.h"
#include "NWM_CTU.h"

#include "GFPCVXA0.h"               /* カットオーバー制御メインヘッダーファイル */
#include "GFPCVXZ0.h"               /* 制御電文機能共通メインヘッダーファイル */
#include "GFPCVXZ2.h"               /* 制御電文機能共通メインヘッダーファイル */
#include "vproc.h"

/*****************************************************************************/
/* 共通呼出・処理個別部 開始                                                 */
/*****************************************************************************/
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_set_prgid                              */
/*  CALLING SEQ.    : void CMIN_kbt_set_prgid (void)                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 個別プログラムID設定処理                               */
/*****************************************************************************/
void CMIN_kbt_set_prgid()
{
    /*  */
    memcpy(g_myinfo.prog_id, DEF_GFPCVXA0, strlen(DEF_GFPCVXA0));

    return;

} /* end of CMIN_kbt_set_prgid */
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_init                                   */
/*  CALLING SEQ.    : void CMIN_kbt_init (void)                              */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:正常                                                */
/*                    -1:異常                                                */
/*  DESCRIPTION     : 個別 初期処理                                          */
/*****************************************************************************/
short CMIN_kbt_init()
{
    short ls_result;

    // EMS用項目設定
    CCUT_ems_arg_ini();

    // 初期処理(局状態管理ファイル)
    ls_result = CCUT_kbt_ini_GCSST();
    if(ls_result != DEF_RET_OK){
        return DEF_RET_NG;
    }

    // 初期処理(接続先固有情報ファイル)
    ls_result = CCUT_kbt_ini_GFNWS();
    if(ls_result != DEF_RET_OK){
        return DEF_RET_NG;
    }

    // 初期処理(NW情報ファイル/インターフェイス識別単位)
    ls_result = CCUT_kbt_ini_GFNWI();
    if(ls_result != DEF_RET_OK){
        return DEF_RET_NG;
    }

    // 初期処理(制御電文ログファイル)
    ls_result = CCUT_kbt_ini_GLMLG();
    if(ls_result != DEF_RET_OK){
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* end of CMIN_kbt_init */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_kbt_ini_GCSST                              */
/*  CALLING SEQ.    : void CCUT_kbt_ini_GCSST (void)                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:正常                                                */
/*                    -1:異常                                                */
/*  DESCRIPTION     : 個別 初期処理(局状態管理ファイル)                      */
/*****************************************************************************/
static short CCUT_kbt_ini_GCSST()
{
    short ls_result;
    short file_no;
    t_gfphi_pri_key     st_gfphi_key;
    db_gfphi_def        *pst_gfphi;
    char                 pname[sizeof(db_gfphi_def)];   // 物理名用バッファ
                                                        // 設定用ダミーバッファ
    char                 buf[sizeof(g_GCSST_in_inf.key_value)];
    /* ------------------------------------------------------ */
    /* 局状態管理ファイル  物理名取得                         */
    /* ------------------------------------------------------ */
    st_gfphi_key.site_id = g_myinfo.site_id;
    st_gfphi_key.nw_id   = g_myinfo.network_id;
    memcpy( st_gfphi_key.grp_id     , g_myinfo.group_id
          , sizeof(st_gfphi_key.grp_id));
    memset(&st_gfphi_key.srv_cls_key, DEF_BUF_NO_SET
          , sizeof(st_gfphi_key.srv_cls_key));
    memcpy( st_gfphi_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CEN_STS
          , strlen(DEF_FL_CEN_STS));
    memset( st_gfphi_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_id.prc_file_num));
    memset( st_gfphi_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name((char *)&st_gfphi_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    // 取得物理名設定(エイリアス)
    pst_gfphi = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memset(pname, 0x00, sizeof(pst_gfphi->prc_file_info));
    memcpy(pname, pst_gfphi->prc_file_info.prc_file_name, sizeof(pst_gfphi->prc_file_info.prc_file_name));

    //------------------------------------------------------------------------//
    // 局状態管理ファイルOPEN                                                 //
    //------------------------------------------------------------------------//
    ls_result = CMIN_file_open(DEF_FL_NW_KOYU_INFO, pname, &file_no);
    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    //------------------------------------------------------------------------//
    // 局状態管理ファイル操作情報編集                                         //
    //------------------------------------------------------------------------//
    //--------------------------------//
    // 検索キー設定(ダミー）
    //--------------------------------//
    memset(buf, ' ', sizeof(buf));

    //----------------------------------------//
    // その他COM_IOM定義編集（キー値は未設定）
    //----------------------------------------//
    set_comiom_arg(pname, DEF_FL_CEN_STS, file_no, buf, DEF_COM_IOM_NOLOCK,
                   sizeof(g_GCSST_rec.pri_key), sizeof(db_gcsst_def),
                   &g_GCSST_tace_inf, &g_GCSST_file_inf, &g_GCSST_in_inf);

    return DEF_RET_OK;

} /* end of CCUT_kbt_ini_GCSST */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_kbt_ini_GFNWS                              */
/*  CALLING SEQ.    : void CCUT_kbt_ini_GFNWS (void)                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:正常                                                */
/*                    -1:異常                                                */
/*  DESCRIPTION     : 個別 初期処理(接続先固有情報ファイル)                  */
/*****************************************************************************/
static short CCUT_kbt_ini_GFNWS()
{
    int i;
    short ls_result;
    short file_no;
    t_gfphi_pri_key     st_gfphi_key;
    db_gfphi_def        *pst_gfphi;
    db_gfnws_def        *pst_gfnws;
    char                 pname[sizeof(db_gfphi_def)];   // 物理名用バッファ
    COM_IOM_arg_5_def   st_gfnws_in;                    // 接続先固有情報・入力情報
    COM_IOM_arg_6_def   st_gfnws_out;                   // 接続先固有情報・出力情報

    memcpy(gch_gfnws_rec_unit, g_gfnwi_tbl[0].gfnws_info.rec_unit, DEF_REC_UNIT_FLG_CNT);

    /* ------------------------------------------------------ */
    /* 接続先固有情報ファイル  物理名取得                     */
    /* ------------------------------------------------------ */
    st_gfphi_key.site_id = g_myinfo.site_id;
    st_gfphi_key.nw_id   = g_myinfo.network_id;
    memcpy( st_gfphi_key.grp_id     , g_myinfo.group_id
          , sizeof(st_gfphi_key.grp_id));
    memset(&st_gfphi_key.srv_cls_key, DEF_BUF_NO_SET
          , sizeof(st_gfphi_key.srv_cls_key));
    memcpy( st_gfphi_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_NW_KOYU_INFO
          , strlen(DEF_FL_NW_KOYU_INFO));
    memset( st_gfphi_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_id.prc_file_num));
    memset( st_gfphi_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name((char *)&st_gfphi_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }
    pst_gfphi = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memset(pname, 0x00, sizeof(pst_gfphi->prc_file_info));
    memcpy(pname, pst_gfphi->prc_file_info.prc_file_name, sizeof(pst_gfphi->prc_file_info.prc_file_name));

    //------------------------------------------------------------------------//
    // 接続先固有情報ファイルOPEN                                             //
    //------------------------------------------------------------------------//
    ls_result = CMIN_file_open(DEF_FL_NW_KOYU_INFO, pname, &file_no);
    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    //------------------------------------------------------------------------//
    // 接続先固有情報ファイル操作情報編集                                     //
    //------------------------------------------------------------------------//

    /* COM_IOM定義編集 */
    set_comiom_arg(pname, DEF_FL_NW_KOYU_INFO, file_no, g_GFNWS_key, DEF_COM_IOM_NOLOCK,
                   sizeof(pst_gfnws->pri_key), sizeof(db_gfnws_def),
                   &g_GFNWS_tace_inf, &g_GFNWS_file_inf, &g_GFNWS_in_inf);

    memcpy(&st_gfnws_in, &g_GFNWS_in_inf,sizeof(g_GFNWS_in_inf));

    memcpy(g_GFNWS_tace_inf.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    // 入力情報・KEY-VALUE
    // 検索キー設定
    pst_gfnws = (db_gfnws_def *)g_GFNWS_key;

    // NW識別
    pst_gfnws->pri_key.nw_id = g_gfnwi_tbl->pri_key.nw_id;

    memcpy(st_gfnws_in.key_value, pst_gfnws, sizeof(st_gfnws_in.key_value));

    pst_gfnws = (db_gfnws_def *)st_gfnws_in.key_value;

    // NW単位のみ
    if(g_gfnwi_tbl->gfnws_info.rec_unit[0] == '0'){
        // 未使用の場合はスキップ
        memset(&gst_gfnws_inf[0], 0x00, sizeof(db_gfnws_def));
    }
    else {
        // 接続先固有情報ファイル取得
        // 入力情報・キー編集
        // NW単位はインターフェース識別以降をDEF_BUF_NO_SET
        memset(pst_gfnws->pri_key.if_id, DEF_BUF_NO_SET,
               sizeof(pst_gfnws->pri_key.if_id) +
               sizeof(pst_gfnws->pri_key.station_id) +
               sizeof(pst_gfnws->pri_key.connect_id));


        memset(&st_gfnws_out, 0x00, sizeof(st_gfnws_out));

        ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                            , g_ch_sub_prog_sts
                            ,&g_GFNWS_tace_inf
                            ,&g_GFNWS_file_inf
                            ,&st_gfnws_in
                            ,&st_gfnws_out);
                                                            /* IOモジュール結果判定 */

        if ( memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != DEF_RET_OK ) {
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@L@T@C@C@C@5"
                                , g_add_ems_info.lcn
                                , g_add_ems_info.connect
                                , DEF_FL_NW_KOYU_INFO
                                , DEF_COM_IOM_FUNC_STARTREAD
                                , st_gfnws_in.key_value
                                , st_gfnws_out.guardian_errcode );
            return DEF_RET_NG;
        }

        memcpy( (char *)&gst_gfnws_inf[0], st_gfnws_out.rec_area, sizeof(db_gfnws_def));
    }

    return DEF_RET_OK;

} /* end of CCUT_kbt_ini_GFNWS */
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_kbt_ini_GFNWI                              */
/*  CALLING SEQ.    : short  CCUT_kbt_ini_GFNWI()                            */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:正常                                                */
/*                    -1:異常                                                */
/*  DESCRIPTION     : 個別 初期処理(NW情報ファイル)                          */
/*****************************************************************************/
static short  CCUT_kbt_ini_GFNWI()
{
    //------------------------------------------------------------------------//
    // NW情報管理ファイル操作情報編集                                         //
    //------------------------------------------------------------------------//
    //--------------------------------//
    // 検索キー設定
    //--------------------------------//
    memset(g_GFNWI_key, ' ', sizeof(g_GFNWI_key));

    //--------------------------------//
    // その他COM_IOM定義編集
    //--------------------------------//
    set_comiom_arg(g_com_file_data.nw_file_name,
                   DEF_FL_NW_INFO,
                   g_com_file_data.nw_file_no,
                   g_GFNWI_key,
                   DEF_COM_IOM_NOLOCK,
                   sizeof(g_gfnwi_tbl[DEF_FNWI_IDX_SITE].pri_key),
                   sizeof(db_gfnwi_def),
                   &g_GFNWI_tace_inf,
                   &g_GFNWI_file_inf,
                   &g_GFNWI_in_inf);

    /* トレース情報設定・READ */
    memcpy( g_GFNWI_tace_inf.file_io_type, DEF_FILEIO_READ, sizeof(g_GFNWI_tace_inf.file_io_type));

    return DEF_RET_OK;

} /* end of CCUT_kbt_ini_GFNWI */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_kbt_ini_GLMLG                              */
/*  CALLING SEQ.    : short  CCUT_kbt_ini_GLMLG()                            */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:正常                                                */
/*                    -1:異常                                                */
/*  DESCRIPTION     : 制御電文ログファイル初期処理                           */
/*  DESCRIPTION     : 個別 初期処理(制御電文ログファイル)                    */
/*****************************************************************************/
static short  CCUT_kbt_ini_GLMLG()
{
    db_glmlg_def      st_glmlg;
    short             ls_result;
    short             file_no;
    t_gfphi_pri_key     st_gfphi_key;
    char                 pname[sizeof(db_gfphi_def)];   // 物理名用バッファ
    db_gfphi_def        *pst_gfphi;

    /* ------------------------------------------------------ */
    /* 接続先固有情報ファイル  物理名取得                     */
    /* ------------------------------------------------------ */
    st_gfphi_key.site_id = g_myinfo.site_id;
    st_gfphi_key.nw_id   = g_myinfo.network_id;
    memcpy( st_gfphi_key.grp_id     , g_myinfo.group_id
          , sizeof(st_gfphi_key.grp_id));
    memset(&st_gfphi_key.srv_cls_key, DEF_BUF_NO_SET
          , sizeof(st_gfphi_key.srv_cls_key));
    memcpy( st_gfphi_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CTRL_DEN_LOG
          , strlen(DEF_FL_CTRL_DEN_LOG));
    memset( st_gfphi_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_id.prc_file_num));
    memset( st_gfphi_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO
          , sizeof(st_gfphi_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name((char *)&st_gfphi_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }
    pst_gfphi = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memset(pname, 0x00, sizeof(pst_gfphi->prc_file_info));
    memcpy(pname, pst_gfphi->prc_file_info.prc_file_name, sizeof(pst_gfphi->prc_file_info.prc_file_name));

    //--------------------------------//
    // 検索キー設定
    //--------------------------------//
    memset(&st_glmlg, ' ', sizeof(st_glmlg));

    //------------------------------------------------------------------------//
    // 制御電文ログファイルOPEN                                               //
    //------------------------------------------------------------------------//
    ls_result = CMIN_file_open(DEF_FL_CTRL_DEN_LOG, pname, &file_no);
    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    //--------------------------------//
    // その他COM_IOM定義編集
    //--------------------------------//
    set_comiom_arg(pname,
                   DEF_FL_CTRL_DEN_LOG,
                   file_no,
                   (char *)&st_glmlg.pri_key,
                   DEF_COM_IOM_NOLOCK,
                   sizeof(st_glmlg.pri_key),
                   sizeof(st_glmlg),
                   &g_GLMLG_tace_inf,
                   &g_GLMLG_file_inf,
                   &g_GLMLG_in_inf);

    return DEF_RET_OK;

} /* end of CCUT_kbt_ini_GLMLG */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_ems_arg_ini                                */
/*  CALLING SEQ.    : void CCUT_ems_arg_ini (void)                           */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : EMS用共通項目設定処理                                  */
/*****************************************************************************/
static void CCUT_ems_arg_ini()
{
    char szbuf[16];
    memset(&g_cmn_ems_info, 0x00, sizeof(g_cmn_ems_info));

    // IOタイマ
    snprintf(szbuf, sizeof(szbuf),
             "%0*.*d",
             sizeof(g_cmn_ems_info.uytrminf.proctimer),
             sizeof(g_cmn_ems_info.uytrminf.proctimer),
              g_myinfo.io_timer);
    memcpy(g_cmn_ems_info.uytrminf.proctimer,
           szbuf,
           sizeof(g_cmn_ems_info.uytrminf.proctimer));

    // 運用監視端末出力PATHMON名
    snprintf(szbuf, sizeof(szbuf),
             "%-*.*s",
             sizeof(g_cmn_ems_info.uytrminf.uytrmmon),
             sizeof(g_cmn_ems_info.uytrminf.uytrmmon),
             g_myinfo.ems_pathmon);
    memcpy(g_cmn_ems_info.uytrminf.uytrmmon,
           szbuf,
           sizeof(g_cmn_ems_info.uytrminf.uytrmmon));

    // 運用監視端末出力PATHMON名長
    snprintf(szbuf, sizeof(szbuf),
             "%0*.*d",
             sizeof(g_cmn_ems_info.uytrminf.uytrmmonlen),
             sizeof(g_cmn_ems_info.uytrminf.uytrmmonlen),
             strlen(g_myinfo.ems_pathmon));
    memcpy(g_cmn_ems_info.uytrminf.uytrmmonlen,
           szbuf,
           sizeof(g_cmn_ems_info.uytrminf.uytrmmonlen));

    // 運用監視端末出力サーバ名
    snprintf(szbuf, sizeof(szbuf),
             "%-*.*s",
             sizeof(g_cmn_ems_info.uytrminf.uytrmsrv),
             sizeof(g_cmn_ems_info.uytrminf.uytrmsrv),
             g_myinfo.ems_serverclass_name);
    memcpy(g_cmn_ems_info.uytrminf.uytrmsrv,
           szbuf,
           sizeof(g_cmn_ems_info.uytrminf.uytrmsrv));
    // 運用監視端末出力サーバ名長
    snprintf(szbuf, sizeof(szbuf),
            "%0*.*d",
            sizeof(g_cmn_ems_info.uytrminf.uytrmsrvlen),
            sizeof(g_cmn_ems_info.uytrminf.uytrmsrvlen),
             strlen(g_myinfo.ems_serverclass_name));
    memcpy(g_cmn_ems_info.uytrminf.uytrmsrvlen,
           szbuf,
           sizeof(g_cmn_ems_info.uytrminf.uytrmsrvlen));

    // EMS出力情報・システム名
    memcpy( g_cmn_ems_info.emsinf.emsgkinf.sysnm,
            DEF_EMS_SYSNM_GFP  ,
            strlen(DEF_EMS_SYSNM_GFP));
    // EMS出力情報・SERVER分類
    memcpy( g_cmn_ems_info.emsinf.emsgkinf.srv_kbn,
            DEF_EMS_SRV_KBN_COM  ,
            strlen(DEF_EMS_SRV_KBN_COM));
    // EMS出力情報・IF識別子
    memcpy( g_cmn_ems_info.emsinf.emsgkinf.h_nw_kbn,
            g_myinfo.nw_kbn  ,
            sizeof(g_cmn_ems_info.emsinf.emsgkinf.h_nw_kbn));
    // EMS出力情報・差出センタID
    // EMS出力情報・メッセージ出力元プログラム名
    memcpy( g_cmn_ems_info.emsinf.emsgkinf.prgid,
            g_myinfo.prog_id  ,
            sizeof(g_cmn_ems_info.emsinf.emsgkinf.prgid));

    memset(&g_add_ems_info, ' ', sizeof(g_add_ems_info));

    // EMS用サーバクラス論理名
    memcpy(g_add_ems_info.srv_logical_id, g_myinfo.serverclass_name,sizeof(g_add_ems_info.srv_logical_id));

} /* end of CCUT_ems_arg_ini */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_get_physical_names                     */
/*  CALLING SEQ.    : void CMIN_kbt_get_physical_names (void)                */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 物理ファイル名取得処理(個別)                           */
/*****************************************************************************/
short CMIN_kbt_get_physical_names()
{
    return DEF_RET_OK;
} /* end of CMIN_kbt_get_physical_names */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_file_open                              */
/*  CALLING SEQ.    : void CMIN_kbt_file_open (void)                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : ファイルオープン処理(個別)                             */
/*****************************************************************************/
short CMIN_kbt_file_open()
{

    return DEF_RET_OK;
} /* end of CMIN_kbt_file_open */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_file_close                             */
/*  CALLING SEQ.    : void CMIN_kbt_file_close (void)                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : ファイルクローズ処理(個別)                             */
/*****************************************************************************/
void CMIN_kbt_file_close()
{

    //------------------------------------------------------------------------//
    // 局状態管理ファイルCLOSE                                                 //
    //------------------------------------------------------------------------//
    CMIN_file_close(g_GCSST_file_inf.file_id, g_GCSST_file_inf.file_name, &g_GCSST_file_inf.file_no);

    //------------------------------------------------------------------------//
    // 接続先固有情報ファイルCLOSE                                             //
    //------------------------------------------------------------------------//
    CMIN_file_close(g_GFNWS_file_inf.file_id, g_GFNWS_file_inf.file_name, &g_GFNWS_file_inf.file_no);

    //------------------------------------------------------------------------//
    // 制御電文ログファイルCLOSE                                              //
    //------------------------------------------------------------------------//
    CMIN_file_close(g_GLMLG_file_inf.file_id, g_GLMLG_file_inf.file_name, &g_GLMLG_file_inf.file_no);

    return;
} /* end of CMIN_kbt_file_open */

/*****************************************************************************/
/* 共通呼出・処理個別部 終了                                                 */
/*****************************************************************************/

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_handle_req_msg                             */
/*  CALLING SEQ.    : void CMIN_handle_req_msg (void)                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : リクエスタメッセージ処理                               */
/*****************************************************************************/
void CMIN_handle_req_msg(void)
{
    short         s_resp_len;               // 送信電文長
    short         s_ret_code;
    char          c_inner_errcd[7+1];       // 内部エラーコード
    char          c_cut_date[8+1];          // カット日付
    cr401_def     *pst_rec_rcv = (cr401_def *)g_recv_buf;  // 受信電文
    cr401_def     *pst_rsp_snd = (cr401_def *)g_resp_buf;  // 送信電文
    short         error_code = 0;
    char          c_err_bit[32];           // 電文精査エラービット設定領域
    char          ch_wk_data[16];

    memset(c_inner_errcd, 0x00, sizeof(c_inner_errcd));
    memset(c_cut_date,    0x00, sizeof(c_cut_date));

    memset(c_inner_errcd, 0x00, sizeof(c_inner_errcd));
    memcpy(c_inner_errcd,DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));

    memset(c_err_bit,     0x00, sizeof(c_err_bit));

    // EMS用・LCN設定
    memcpy(g_add_ems_info.lcn,
           &pst_rec_rcv->control_info.denbun_log_key.tran_id.gfp_lcn,
           sizeof(g_add_ems_info.lcn));

    // EMS用・接続先（サイト識別～コネクション識別） (24バイト)
    memcpy(g_add_ems_info.connect,
           &pst_rec_rcv->control_info.connection_lid,
           sizeof(g_add_ems_info.connect));

    // 接続先固有情報(コネクション単位)取得判定
    // 接続先固有情報ファイル・コネクション単位取得
    s_ret_code = CCUT_get_cnct_gfnws( pst_rec_rcv,
                                      g_gfnwi_tbl->gfnws_info.rec_unit,
                                     &gst_gfnws_inf[DEF_REC_UNIT_FLG_IF],
                                     &gst_gfnws_inf[DEF_REC_UNIT_FLG_ST],
                                     &gst_gfnws_inf[DEF_REC_UNIT_FLG_CNN],
                                      c_inner_errcd);
    if(s_ret_code != DEF_CCUT_OK){
        /* 電文編集 */
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        pst_rsp_snd->common_header.error_code = DEF_IPC_ERRCD_NG;
        /* 応答送信 */
        CMIN_send_reply((char *)g_resp_buf, (short)s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    // NW情報ファイル・インターフェイス単位取得
    s_ret_code = CCUT_read_gfnwi(c_inner_errcd);
    if(s_ret_code != DEF_CCUT_OK){
        /* 電文編集 */
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        pst_rsp_snd->common_header.error_code = DEF_IPC_ERRCD_NG;
        MCR_CCUT_CPY(pst_rsp_snd->common_header.internal_error_code, c_inner_errcd);
        /* 応答送信 */
        CMIN_send_reply((char *)g_resp_buf, (short)s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    // 処理スキップ確認（"50"：被仕向応答送信不可）
    if(memcmp(pst_rec_rcv->control_info.request_kind,
              DEF_CTLREQ_HISIMUKE_ERROR,
              sizeof(pst_rec_rcv->control_info.request_kind)) == 0){
        //--------------------------------------------------------------------*/
        // "50"：被仕向応答送信不可の場合、制御電文ログ更新で終了
        //--------------------------------------------------------------------*/
        // 制御電文ログ更新
        error_code = CCUT_update_log(pst_rec_rcv,
                                     &g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                                     c_inner_errcd,
                                     (short)g_recv_len);
        // 送信電文編集
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        pst_rsp_snd->common_header.error_code = error_code;

        // 電文送信
        CMIN_send_reply((char *)g_resp_buf, s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    /* 電文精査 */
    error_code = NWM_CTO_msg_check(g_recv_buf,
                                   (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE],
                                   (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                                   (char *)&gst_gfnws_inf[DEF_REC_UNIT_IDX_NW],
                                   (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_IF],
                                   (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_ST],
                                   (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_CNN],
                                    c_cut_date,
                                    c_err_bit);

    /* 戻り値判定 */
    if ( error_code != DEF_CCUT_OK) {
        /* 精査異常 */
        // 内部エラーコード設定
        memcpy(c_inner_errcd,DEF_NWM_CTO_MSG_FMT_NG, strlen(DEF_NWM_CTO_MSG_FMT_NG));

        /* EMS出力 電文精査エラー */
        memset(ch_wk_data, 0x00, sizeof(ch_wk_data));
        memcpy(ch_wk_data,
               pst_rec_rcv->control_info.mti,
               sizeof(pst_rec_rcv->control_info.mti));
        CMIN_message_output(DEF_EVT_DATA_FLD_SEISA_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_HSMK_REQ_SEISA_ERR,
                            "@L@T@C@C",
                            pst_rec_rcv->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &pst_rec_rcv->control_info.connection_lid,
                            ch_wk_data,
                            c_err_bit);

        // エラー出力ログ出力処理
        CCUT_put_errlog(DEF_NERR_HSMK_REQ_SEISA_ERR);

        if (error_code == DEF_NWM_CTO_ERR_HAKI){
            /* 電文破棄 */
            NWM_CTO_msg_edit(g_recv_buf,
                            (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE],
                            (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                            (char *)&gst_gfnws_inf[DEF_REC_UNIT_IDX_NW],
                            (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_IF],
                            (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_ST],
                            (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_CNN],
                            c_inner_errcd,
                            g_resp_buf,
                            &s_resp_len);
        }
        else {
            /* 障害電文通知とその他エラー */
            // 送信電文編集
            CCUT_msg_edit(DEF_CCUT_MTI_REQ, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
            pst_rsp_snd->common_header.error_code = error_code;
        }
        // 電文送信
        CMIN_send_reply((char *)g_resp_buf, s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    //--------------------------------------------------------------------//
    // 制御電文ログ出力
    //--------------------------------------------------------------------//
    s_ret_code = CCUT_put_log(DEF_CCUT_RCV_ID,
                              pst_rec_rcv,
                              &g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                              c_inner_errcd,
                              (short)g_recv_len);
    if(s_ret_code != DEF_CCUT_OK) {
        // 送信電文編集
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        pst_rsp_snd->common_header.error_code = DEF_IPC_ERRCD_NG;

        // 電文送信
        CMIN_send_reply((char *)g_resp_buf, s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    //--------------------------------------------------------------------//
    // CCUT_cutover(カットオーバー制御処理)呼出
    //--------------------------------------------------------------------//
    error_code = CCUT_cutover(pst_rec_rcv, c_inner_errcd, c_cut_date);
    if(error_code != DEF_CCUT_OK){
        /* 電文編集(エラー時) */
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        pst_rsp_snd->common_header.error_code = error_code;

        // 電文送信
        CMIN_send_reply((char *)g_resp_buf, s_resp_len, DEF_IPC_ERRCD_OK);

        return;
    }

    /* 電文編集 */
    NWM_CTO_msg_edit(g_recv_buf,
                    (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE],
                    (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                    (char *)&gst_gfnws_inf[DEF_REC_UNIT_IDX_NW],
                    (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_IF],
                    (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_ST],
                    (char *)&gst_gfnws_inf[DEF_REC_UNIT_FLG_CNN],
                    c_inner_errcd,
                    g_resp_buf,
                    &s_resp_len);

    //--------------------------------------------------------------------//
    // 制御電文ログ出力
    //--------------------------------------------------------------------//
    s_ret_code = CCUT_put_log(DEF_CCUT_SND_ID,
                              pst_rsp_snd,
                              &g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE],
                              c_inner_errcd,
                              s_resp_len);
    if(s_ret_code != DEF_CCUT_OK){
        /* 電文編集(エラー時) */
        CCUT_msg_edit(DEF_CCUT_MTI_UPD, g_recv_buf, c_inner_errcd, g_resp_buf, &s_resp_len);
        error_code = DEF_IPC_ERRCD_NG;
    }

    /* 応答送信 */
    if(error_code < 0){
        pst_rsp_snd->common_header.error_code = DEF_IPC_ERRCD_NG;
    }else{
        pst_rsp_snd->common_header.error_code = error_code;
    }
    MCR_CCUT_CPY(pst_rsp_snd->common_header.internal_error_code, c_inner_errcd);
    CMIN_send_reply((char *)g_resp_buf, s_resp_len, DEF_IPC_ERRCD_OK);

    return;

} /* end of CMIN_handle_req_msg */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_get_cnct_gfnws                             */
/*  CALLING SEQ.    : db_gfnws_def * CCUT_get_cnct_gfnws (void)              */
/*  ARGUMENT        : 1.pst_rec_rcv     (I) 電文                             */
/*                  : 2.rec_unit        (I) 接続先固有情報レコード登録単位   */
/*                  : 3.pch_cninf_if    (O) 接続先固有情報(IF単位)           */
/*                  : 4.pch_cninf_st    (O) 接続先固有情報(Station単位)      */
/*                  : 5.pch_cninf_cn    (O) 接続先固有情報(コネクション単位) */
/*                  : 6.intrlr_err_code (O) 内部エラーコード                 */
/*  RETURN CODE     : 0:正常                                                 */
/*                    0以外:異常                                             */
/*  DESCRIPTION     : 接続先固有情報(コネクション単位)取得処理               */
/*****************************************************************************/
static short CCUT_get_cnct_gfnws(
    cr401_def    *pst_rec_rcv,
    char         *rec_unit,
    db_gfnws_def *pch_cninf_if,
    db_gfnws_def *pch_cninf_st,
    db_gfnws_def *pch_cninf_cn,
    char         *intrlr_err_code
)
{
    int            i;
    db_gfnws_def        *pst_gfnws;         // 接続先固有情報・キー情報
    COM_IOM_arg_5_def   st_gfnws_in;        // 接続先固有情報・ファイル入力情報
    COM_IOM_arg_6_def   st_gfnws_out;       // 接続先固有情報・ファイル出力情報
                                            // 接続先固有情報設定用配列
                                            // (配列1個目はダミー
    db_gfnws_def        *pst_gfnws_arr[4] = {NULL, pch_cninf_if,pch_cninf_st, pch_cninf_cn};

    memset(pch_cninf_if, 0x00, sizeof(db_gfnws_def));
    memset(pch_cninf_st, 0x00, sizeof(db_gfnws_def));
    memset(pch_cninf_cn, 0x00, sizeof(db_gfnws_def));

    /*--------------------------------------*/
    /* 接続先固有情報(コネクション単位)取得 */
    /*--------------------------------------*/
    memcpy(&st_gfnws_in, &g_GFNWS_in_inf, sizeof(st_gfnws_in));

    for(i = DEF_REC_UNIT_FLG_IF; i <= DEF_REC_UNIT_FLG_CNN; i++){
        if(rec_unit[i] == '0'){
            // 設定不要の場合スキップ
            continue;
        }

        memset(st_gfnws_in.key_value, 0x00, sizeof(pst_gfnws->pri_key));

        // 接続先固有情報ファイル取得
        // 入力情報・キー編集
        pst_gfnws = (db_gfnws_def *)st_gfnws_in.key_value;
        //--------------------------------------------------------------------//
        // キー項目編集
        //--------------------------------------------------------------------//
        // NW ID
        pst_gfnws->pri_key.nw_id = pst_rec_rcv->control_info.connection_lid.nw_name;
        // IF ID
        memcpy(pst_gfnws->pri_key.if_id,
               pst_rec_rcv->control_info.connection_lid.interface_name,
               sizeof(pst_gfnws->pri_key.if_id));
        // 接続ID
        if(i == DEF_REC_UNIT_FLG_IF){
            // インタフェース単位
            // ステーションID
            memset(pst_gfnws->pri_key.station_id,
                   DEF_BUF_NO_SET,
                   sizeof(pst_gfnws->pri_key.station_id));
            // コネクションID
            memset(pst_gfnws->pri_key.connect_id,
                   DEF_BUF_NO_SET,
                   sizeof(pst_gfnws->pri_key.connect_id));
        }
        else if(i == DEF_REC_UNIT_FLG_ST){
            // ステーション単位
            // ステーションID
            memcpy(pst_gfnws->pri_key.station_id,
                   pst_rec_rcv->control_info.connection_lid.station_name,
                   sizeof(pst_gfnws->pri_key.station_id));
            // コネクションID
            memset(pst_gfnws->pri_key.connect_id,
                   DEF_BUF_NO_SET,
                   sizeof(pst_gfnws->pri_key.connect_id));
        }else{
            // コネクション単位
            // ステーションID
            memcpy(pst_gfnws->pri_key.station_id,
               pst_rec_rcv->control_info.connection_lid.station_name,
               sizeof(pst_gfnws->pri_key.station_id));
            // コネクションID
            memcpy(pst_gfnws->pri_key.connect_id,
                   pst_rec_rcv->control_info.connection_lid.connection_name,
                   sizeof(pst_gfnws->pri_key.connect_id));
        }

        memset(&st_gfnws_out, 0x00, sizeof(st_gfnws_out));
        COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                 , g_ch_sub_prog_sts
                 ,&g_GFNWS_tace_inf
                 ,&g_GFNWS_file_inf
                 ,&st_gfnws_in
                 ,&st_gfnws_out);

                                                            /* IOモジュール結果判定 */
        if ( memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != DEF_RET_OK ) {
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@L@T@C@C@C@5"
                                , g_add_ems_info.lcn
                                , g_add_ems_info.connect
                                , DEF_FL_CEN_STS
                                , DEF_COM_IOM_FUNC_STARTREAD
                                , st_gfnws_in.key_value
                                , st_gfnws_out.guardian_errcode );
            memcpy(intrlr_err_code, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR));
            return DEF_CCUT_NG ;
        }else{
            // 取得情報コピー
            memcpy(pst_gfnws_arr[i], st_gfnws_out.rec_area, sizeof(db_gfnws_def));
        }
    }
    return DEF_CCUT_OK ;
} /* end of CCUT_get_cnct_gfnws */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CCUT_read_gfnwi                                 */
/*  CALLING SEQ.    : short  CCUT_read_gfnwi()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : NW情報ファイル・インターフェイス識別単位読込処理       */
/*****************************************************************************/
static short  CCUT_read_gfnwi(
    char *pc_inner_errcd
)
{
    t_gfnwi_pri_key_def  l_gfnwi_pkey;       /* NW情報ファイルプライマリKey  */
    cr401_def           *lp_c401;            /* NW電文                       */
    COM_IOM_arg_6_def    st_outinf;          /* COM_IOM出力情報              */

    lp_c401 = (cr401_def *)g_recv_buf;

    /* ------------------------------------------------------ */
    /* インターフェイス識別単位取得                           */
    /* ------------------------------------------------------ */
    memset(&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE], DEF_BUF_SPACE, sizeof(db_gfnwi_def));
    memset(&l_gfnwi_pkey                       , DEF_BUF_SPACE, sizeof(l_gfnwi_pkey));
    /* プライマリKey設定 */
    l_gfnwi_pkey.site_id = lp_c401->control_info.connection_lid.site_name;
    l_gfnwi_pkey.nw_id   = lp_c401->control_info.connection_lid.nw_name;
    memcpy( l_gfnwi_pkey.grp_id    , lp_c401->control_info.connection_lid.group_name
                                   , sizeof(l_gfnwi_pkey.grp_id    ));
    memcpy( l_gfnwi_pkey.if_id     , lp_c401->control_info.connection_lid.interface_name
                                   , sizeof(l_gfnwi_pkey.if_id     ));
    memset( l_gfnwi_pkey.station_id, DEF_BUF_NO_SET     , sizeof(l_gfnwi_pkey.station_id));

    /* IOモジュールパラメータ初期化 */
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));
    memset( &st_outinf,        DEF_BUF_NULL, sizeof(st_outinf));

    /* 入力情報 */
    memcpy( g_GFNWI_in_inf.key_value, (char *)&l_gfnwi_pkey, sizeof(l_gfnwi_pkey));

    COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
            , g_ch_sub_prog_sts
            ,&g_GFNWI_tace_inf
            ,&g_GFNWI_file_inf
            ,&g_GFNWI_in_inf
            ,&st_outinf);
                                                        /* IOモジュール結果判定 */
    if ( memcmp(g_ch_sub_prog_sts, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR       /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@5"
                            , g_add_ems_info.lcn
                            , g_add_ems_info.connect
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_GFNWI_in_inf.key_value
                            , st_outinf.guardian_errcode );
        memcpy(pc_inner_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    /* 取得レコード格納 */
    memcpy( (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE], st_outinf.rec_area, sizeof(db_gfnwi_def));

    return DEF_RET_OK;
} /* end of CCUT_read_gfnwi */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_cutover                                    */
/*  CALLING SEQ.    : short CCUT_cutover(cr401_def,char *,char *)            */
/*  ARGUMENT        : 1.pcr401msg      (I)   NW電文受信形式電文              */
/*                  : 2.c_inner_errcd  (I)   内部エラーコード                */
/*                  : 3.c_cut_date     (I)   カット日付                      */
/*  RETURN CODE     :  0:正常                                                */
/*                     9:FILE IOエラー                                       */
/*  DESCRIPTION     : カットオーバー制御処理                                 */
/*****************************************************************************/
short CCUT_cutover( cr401_def       *pcr401msg
                   , char           *c_inner_errcd
                   , char           *c_cut_date)
{
    short s_ret_cd;
    char sub_status[DEF_CCUT_STATUS_LEN +1];
    char state_sts[DEF_STATE_STS_LEN + 1];
    COM_IOM_arg_5_def st_in_inf;        /* IOモジュール・入力情報 */
    COM_IOM_arg_6_def st_out_inf;       /* IOモジュール・出力情報 */
    db_gcsst_def     *gcsst_inf;
    /* 変数初期化 */
    memcpy(c_inner_errcd, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    s_ret_cd = DEF_CCUT_OK;

    /*------------------------------------------------------------------------*/
    /* 初期処理                                                               */
    /*------------------------------------------------------------------------*/
    // 受信メッセージ出力
    CMIN_message_output ( DEF_EVT_CUT_OVER_RCV      /* カットオーバー受信   */
                        , DEF_MSGTTKB_NORMAL
                        , DEF_NERR_NOMAL
                        , "@L@T@C"
                        , g_add_ems_info.lcn
                        , g_add_ems_info.connect
                        , c_cut_date);

    /*------------------------------------------------------------------------*/
    /* 局状態取得                                                             */
    /*------------------------------------------------------------------------*/
    // 構造体初期化
    memset(&st_out_inf,   0x00, sizeof(st_out_inf));
    memcpy(&st_in_inf,    &g_GCSST_in_inf, sizeof(st_in_inf));

    // IO TYPE更新（READ)
    memset(g_GCSST_tace_inf.file_io_type, ' ', sizeof(g_GCSST_tace_inf.file_io_type));
    memcpy(g_GCSST_tace_inf.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    // キー編集
    CCUT_edit_gfnws_key(&st_in_inf, pcr401msg);

    COM_IOM( DEF_COM_IOM_FUNC_STARTREAD,
             sub_status,
             &g_GCSST_tace_inf,
             &g_GCSST_file_inf,
             &st_in_inf,
             &st_out_inf);
                                                        /* IOモジュール結果判定 */
    if ( memcmp(sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR       /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@L@T@C@C@C@5"
                            , g_add_ems_info.lcn
                            , g_add_ems_info.connect
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , st_in_inf.key_value
                            , st_out_inf.guardian_errcode );

        memcpy(c_inner_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));

        return DEF_IPC_ERRCD_NG;
    }

    gcsst_inf = (db_gcsst_def *)st_out_inf.rec_area;

    /*------------------------------------------------------------------------*/
    /* 局状態チェック                                                         */
    /*------------------------------------------------------------------------*/
    s_ret_cd = NWM_CTO_cst_check_req_rcv(gcsst_inf->state_sts_info.state_sts,
                                         c_inner_errcd);

    if(s_ret_cd < 0){
        memcpy(state_sts, gcsst_inf->state_sts_info.state_sts, DEF_STATE_STS_LEN);
        state_sts[DEF_STATE_STS_LEN] = 0x00;
        CMIN_message_output ( DEF_EVT_KYOKU_STS_ERR         /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , c_inner_errcd
                            , "@L@T@C"
                            , g_add_ems_info.lcn             // GFP内部LCN (15バイト)
                            , g_add_ems_info.connect         // 接続先（サイト識別～コネクション識別） (24バイト)
                            , state_sts);                    // 局状態

        return DEF_IPC_ERRCD_NG;
    }

    /*------------------------------------------------------------------------*/
    /* カット対象日付更新                                                     */
    /*------------------------------------------------------------------------*/
    memcpy(g_NWM_CTU_INI_arg_3.if_id,
           pcr401msg->control_info.connection_lid.interface_name,
           sizeof(g_NWM_CTU_INI_arg_3.if_id));

    memset(g_NWM_CTU_INI_arg_3.connect_id, ' ',
           sizeof(g_NWM_CTU_INI_arg_3.connect_id) +
           sizeof(g_NWM_CTU_INI_arg_3.station_id));

    // 入力情報・KEY-VALUE
    switch(g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.cut_over_mng_lyr){
        case DEF_CUT_OVER_MNG_LYR_CNCT:
            //------------------------//
            // コネクション単位
            //------------------------//
            // コネクションID設定
            memcpy(g_NWM_CTU_INI_arg_3.connect_id,
                   pcr401msg->control_info.connection_lid.connection_name,
                   sizeof(g_NWM_CTU_INI_arg_3.connect_id));

            // ステーションID設定
            memcpy(g_NWM_CTU_INI_arg_3.station_id,
                   pcr401msg->control_info.connection_lid.station_name,
                   sizeof(g_NWM_CTU_INI_arg_3.station_id));

        case DEF_CUT_OVER_MNG_LYR_ST:
            //------------------------//
            // ステーション単位
            //------------------------//
            // ステーションID設定
            memcpy(g_NWM_CTU_INI_arg_3.station_id,
                   pcr401msg->control_info.connection_lid.station_name,
                   sizeof(g_NWM_CTU_INI_arg_3.station_id));

        default:    // 上記以外はそのまま
                break;
    }

    s_ret_cd = NWM_CTU_UPDATE(&g_NWM_CTU_INI_arg_2, // カット対象日付管理ファイル情報
                              &g_NWM_CTU_INI_arg_3, // ネットワーク特定情報
                              DEF_PROG_ID,          // モジュールID
                              &g_cmn_ems_info,      // EMS共通情報
                              &g_add_ems_info,      // EMS追加情報
                              c_cut_date);          // カット日付
    if(s_ret_cd < 0){
        memcpy(c_inner_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_IPC_ERRCD_NG;
    }

    /*------------------------------------------------------------------------*/
    /* 終了処理                                                               */
    /*------------------------------------------------------------------------*/
    // 受信メッセージ出力
    CMIN_message_output ( DEF_EVT_CUT_OVER_DATE_UPDT  /* カットオーバー更新   */
                        , DEF_MSGTTKB_NORMAL
                        , DEF_NERR_NOMAL
                        , "@L@T@C"
                        , g_add_ems_info.lcn
                        , g_add_ems_info.connect
                        , c_cut_date);

    // 正常終了
    return DEF_IPC_ERRCD_OK;

} /* end of CCUT_cutover */
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_edit_gfnws_key                             */
/*  CALLING SEQ.    : void CCUT_edit_gfnws_key(COM_IOM_arg_5_def *,          */
/*                                             cr401_def *)                  */
/*  ARGUMENT        : 1.pc_key_val     (O)   検索キー                        */
/*                  : 2.pcr401msg      (I)   電文                            */
/*                  : 3.pst_nw_inf     (I)   NW情報（ステーション単位）      */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 局状態検索キー編集処理                                 */
/*****************************************************************************/
static void CCUT_edit_gfnws_key(
    COM_IOM_arg_5_def   *pst_in_inf,
    cr401_def           *pcr401msg
)
{
    db_gcsst_def st_wkgcsst;            // 編集用NW情報ファイルレコード領域

    // キー初期化
    memset(&st_wkgcsst.pri_key, ' ',sizeof(st_wkgcsst.pri_key));

    // キー共通部
    st_wkgcsst.pri_key.site_id      = pcr401msg->control_info.connection_lid.site_name;
    st_wkgcsst.pri_key.nw_id        = pcr401msg->control_info.connection_lid.nw_name;
    memcpy(st_wkgcsst.pri_key.grp_id,
           pcr401msg->control_info.connection_lid.group_name,
           sizeof(st_wkgcsst.pri_key.grp_id));
    memcpy(st_wkgcsst.pri_key.if_id,
           pcr401msg->control_info.connection_lid.interface_name,
           sizeof(st_wkgcsst.pri_key.if_id));

    // 入力情報・KEY-VALUE
    switch(g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.cut_over_mng_lyr){
        case DEF_CUT_OVER_MNG_LYR_CNCT:
            // コネクション単位
            memcpy(st_wkgcsst.pri_key.connect_id,
                   pcr401msg->control_info.connection_lid.connection_name,
                   sizeof(st_wkgcsst.pri_key.connect_id));

        case DEF_CUT_OVER_MNG_LYR_ST:
            // ステーション単位
            memcpy(st_wkgcsst.pri_key.station_id,
                   pcr401msg->control_info.connection_lid.station_name,
                   sizeof(st_wkgcsst.pri_key.station_id));

        default:    // 上記以外はそのまま
                break;
    }

    // キーのコピー
    memcpy(pst_in_inf->key_value, &st_wkgcsst.pri_key, sizeof(st_wkgcsst.pri_key));

    return;
} /* end of CCUT_edit_gfnws_key */
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_put_log                                    */
/*  CALLING SEQ.    : void CCUT_put_log()                                    */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログ出力処理                                   */
/*****************************************************************************/
static short CCUT_put_log(
    char            send_recv_id,
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *inner_err_cd,
    short           len
)
{
    db_glmlg_def      st_glmlg;
    short             ret = 0;
    short             substatus[1];

    // 業務ログ情報設定
    CCUT_make_glmlg(send_recv_id,pcr401msg, pstgfnwi, inner_err_cd, len, &st_glmlg);

    /* ログ出力 */
    ret = CMIN_put_glmlg(g_GLMLG_file_inf.file_name,
                         g_GLMLG_file_inf.file_no,
                         (char *)&st_glmlg.pri_key,
                         DEF_COM_IOM_NOLOCK,
                         (char *)&st_glmlg,
                         substatus);

    return ret;
} /* end of CCUT_put_log */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_update_log                                 */
/*  CALLING SEQ.    : void CCUT_update_log()                                 */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログ更新処理                                   */
/*****************************************************************************/
static short CCUT_update_log(
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *inner_err_cd,
    short            len
)
{
    db_glmlg_def      st_glmlg;
    db_glmlg_def      st_glmlg_old;
    short             ret;
    short             substatus[1];

    // 業務ログ情報設定
    CCUT_make_glmlg(DEF_CCUT_SND_ID,  pcr401msg, pstgfnwi, inner_err_cd, len, &st_glmlg);

    memset(&st_glmlg_old, 0x00, sizeof(st_glmlg_old));
    // 制御電文ログ読込
    ret = CMIN_read_glmlg(g_GLMLG_file_inf.file_name,
                          g_GLMLG_file_inf.file_no,
                          (char *)&st_glmlg.pri_key,
                          DEF_COM_IOM_LOCK,
                          (char *)&st_glmlg_old,
                          substatus);
    if(ret != DEF_CCUT_OK ){
        return DEF_IPC_ERRCD_NG;
    }


    // 業務ログ情報設定
    CCUT_make_glmlg(DEF_CCUT_SND_ID, pcr401msg, pstgfnwi, inner_err_cd, len, &st_glmlg);

    // 更新制御電文情報更新

    // 制御電文ログ更新
    ret = CMIN_update_glmlg(g_GLMLG_file_inf.file_name,
                            g_GLMLG_file_inf.file_no,
                            (char *)&st_glmlg.pri_key,
                            DEF_COM_IOM_LOCK,
                            (char *)&st_glmlg,
                            substatus);
    if(ret != DEF_CCUT_OK ){
        return DEF_IPC_ERRCD_NG;
    }

    return 0;
} /* end of CCUT_update_log */
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_put_log                                    */
/*  CALLING SEQ.    : void CCUT_put_log(char,cr401_def *,db_gfnwi_def *,     */
/*                                      short,db_glmlg_def*)                 */
/*  ARGUMENT        : 1.send_recv_id   (I)   送受信区分                      */
/*                  : 2.pcr401msg      (I)   送受信電文                      */
/*                  : 3.pstgfnwi       (I)   NW情報                          */
/*                  : 4.inner_err_cd   (I)   内部エラーコード                */
/*                  : 5.len            (I)   電文長                          */
/*                  : 6.pst_glmlg      (O)   編集後制御電文情報              */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログレコード編集処理                           */
/*****************************************************************************/
static void CCUT_make_glmlg(
    char             send_recv_id,
    cr401_def       *pcr401msg,
    db_gfnwi_def    *pstgfnwi,
    char            *inner_err_cd,
    short            len,
    db_glmlg_def    *pst_glmlg
)
{
    char                        datetime_hex[64];   /* COM_UNQ用バッファ */
    char                        szbuf[32];          /* 編集用バッファ */

    char  txt[32];
    char *pch_gfp_lcn = pcr401msg->control_info.denbun_log_key.tran_id.gfp_lcn;
    size_t  gfp_len  = sizeof(pcr401msg->control_info.denbun_log_key.tran_id.gfp_lcn);
    memset(pst_glmlg, ' ', sizeof(db_glmlg_def));
    //------------------------------------------------------------------------//
    // PK
    //------------------------------------------------------------------------//
    // パーティションID
    pst_glmlg->pri_key.part_id[0]='0';
    pst_glmlg->pri_key.part_id[1]=pch_gfp_lcn[gfp_len - 1];
    // LCN
    MCR_CCUT_CPY(pst_glmlg->pri_key.lcn_id, pch_gfp_lcn);
    // 仕向・被仕向区分
    pst_glmlg->pri_key.s_h_kubun    = DEF_CCUT_SND_TYPE;
    // 送受信識別
    pst_glmlg->pri_key.send_recv_id = send_recv_id;

    //------------------------------------------------------------------------//
    // コネクション論理ID
    //------------------------------------------------------------------------//
    memcpy(&pst_glmlg->connect_id, &pcr401msg->control_info.connection_lid,
            sizeof(pst_glmlg->connect_id));

    //------------------------------------------------------------------------//
    // MTI
    //------------------------------------------------------------------------//
    MCR_CCUT_CPY(pst_glmlg->mti_id, pcr401msg->control_info.mti);

    //------------------------------------------------------------------------//
    // 内部エラーコード
    //------------------------------------------------------------------------//
    MCR_CCUT_CPY(pst_glmlg->naibu_err_code, inner_err_cd);

    //------------------------------------------------------------------------//
    //制御電文サーバクラス情報
    //------------------------------------------------------------------------//
    snprintf(txt,sizeof(txt),"%8.8s%4.4s", g_myinfo.serverclass_name,g_myinfo.serverclass_no);
    MCR_CCUT_CPY(pst_glmlg->cntrl_denbun_srv_cls_info.srv_cls_id, txt);
    memset(pst_glmlg->cntrl_denbun_srv_cls_info.srv_cls_mlt_num,
           ' ',
           sizeof(pst_glmlg->cntrl_denbun_srv_cls_info.srv_cls_mlt_num));

    //------------------------------------------------------------------------//
    // 制御電文識別
    //------------------------------------------------------------------------//
    MCR_CCUT_CPY(pst_glmlg->control_kind, &pcr401msg->control_info.control_kind);

    //------------------------------------------------------------------------//
    // 送信不可情報
    //------------------------------------------------------------------------//
    MCR_CCUT_CPY(pst_glmlg->send_naibu_err_code, pcr401msg->control_info.send_naibu_err_code);

    //------------------------------------------------------------------------//
    // 送信時時間
    //------------------------------------------------------------------------//
    /* 共通制御・送信日時 */
    memset(szbuf, 0x00, sizeof(szbuf));
    memset(datetime_hex, 0x00, sizeof(datetime_hex));
    COM_UNQ((COM_UNQ_arg_1_def *)szbuf, datetime_hex);
    MCR_CCUT_CPY(pst_glmlg->entry_timestamp, szbuf);

    //------------------------------------------------------------------------//
    // 電文情報有無
    //------------------------------------------------------------------------//
    if(len == 0){
        pst_glmlg->denbun_info_exist = '0';
    }else{
        pst_glmlg->denbun_info_exist = '1';
    }
    //------------------------------------------------------------------------//
    // 電文部
    //------------------------------------------------------------------------//
    snprintf(txt,sizeof(txt),"%05.5d",len);
    MCR_CCUT_CPY(pst_glmlg->denbun_area.denbun_len, txt);
    MCR_CCUT_CPY(pst_glmlg->denbun_area.denbun, pcr401msg->data_bu.message_text);

    return;

} /* end of CCUT_make_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_msg_edit                                   */
/*  CALLING SEQ.    : void CCUT_msg_edit(short,char *,char *,char *,short *) */
/*  ARGUMENT        : 1.psh_reqmsg      (I)   MTI更新フラグ                  */
/*                  : 2.psh_reqmsg      (I)   被仕向要求電文                 */
/*                  : 2.pch_inn_errcd   (I)   内部エラーコード               */
/*                  : 3.pch_rspmsg      (O)   応答メッセージ                 */
/*                  : 4.psh_len         (O)   応答メッセージ長               */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : カットオーバー応答電文編集処理(電文なし)               */
/*****************************************************************************/
static void CCUT_msg_edit(
    short        sh_mti_flg,        /* MTI更新フラグ */
    char         *psh_reqmsg,       /* 被仕向要求電文 */
    char         *pch_inn_errcd,    /* 内部エラーコード */
    char         *pch_rspmsg,       /* 応答メッセージ */
    short        *psh_len           /* 応答メッセージ長 */
)
{
    /* C401 制御電文処理要求 */
    cr401_def                    *pst_req_cr401_inf = (cr401_def *)psh_reqmsg;

    /* C401 制御電文処理要求 */
    cr401_def                    *pst_rsp_cr401_inf = (cr401_def *)pch_rspmsg;

    /* -----------------------------------------------------------------------*/
    /* 電文編集・ヘッダ部                                                     */
    /* -----------------------------------------------------------------------*/
    /* インターフェースコード */
    memcpy(pst_rsp_cr401_inf->common_header.interface_code,
           DEF_IPC_IFCD_NW_MSG_RSP,
           strlen(DEF_IPC_IFCD_NW_MSG_RSP));

    // エラーコード
    pst_rsp_cr401_inf->common_header.error_code = DEF_IPC_ERRCD_OK;

    // 内部エラーコード
    memcpy(pst_rsp_cr401_inf->common_header.internal_error_code,
           pch_inn_errcd,
           strlen(pch_inn_errcd));

    // データ長
    pst_rsp_cr401_inf->common_header.control_data_length =
                              (short)sizeof(pst_rsp_cr401_inf->control_info);

    /* -----------------------------------------------------------------------*/
    // 制御電文情報
    /* -----------------------------------------------------------------------*/
    memcpy(&pst_rsp_cr401_inf->control_info,&pst_req_cr401_inf->control_info,
           sizeof(pst_rsp_cr401_inf->control_info));

    // 応答種別
    memcpy(pst_rsp_cr401_inf->control_info.response_kind,
           DEF_CTLRSP_NOSEND,
           strlen(DEF_CTLRSP_NOSEND));

    // 制御電文種別
    /* 異常応答 */
    pst_rsp_cr401_inf->control_info.control_kind.req_res_kbn  = DEF_CTLMSG_RESPONSE;
    pst_rsp_cr401_inf->control_info.control_kind.int_proc_kbn = DEF_CTLINT_DENY;

    //  要求種別
    // 要求値保障

    // 制御電文種別
    // 要求値保障

    // コネクション論理ID
    // 要求値保障

    // インタフェース名
    // 要求値保障

    // ステーション名
    // 要求値保障

    // 電文ログKEY・電文種別
    pst_rsp_cr401_inf->control_info.denbun_log_key.denbun_shubetu =
                pst_rsp_cr401_inf->control_info.control_kind.ctl_text_kbn;
    // 電文ログKEY・トランザクションID、再送回数は要求値保障

    // MTI
    if(sh_mti_flg == DEF_CCUT_MTI_UPD){
        // MTI更新
        memcpy(pst_rsp_cr401_inf->control_info.mti,
               DEF_NWM_CTO_CTLMSG_RES,
               strlen(DEF_NWM_CTO_CTLMSG_RES));
    }
    // 仕向要求電文GFP内部LCN
    // 要求値保障

    // 送信不可時内部エラーコード
    // 要求値保障

    // 受信電文長
    memset(pst_rsp_cr401_inf->control_info.denbun_len, ' ', sizeof(pst_rsp_cr401_inf->control_info.denbun_len));

    /* -----------------------------------------------------------------------*/
    // 応答電文長
    /* -----------------------------------------------------------------------*/
    *psh_len =   (short)(sizeof(pst_rsp_cr401_inf->common_header) +
                         sizeof(pst_rsp_cr401_inf->control_info));

    return;
} /* end of CCUT_msg_edit */

/****************************************************************************/
/*  FUNCTION        : 4.13 CCUT_put_errlog                                  */
/*  CALLING SEQ.    : void CCUT_put_errlog(char *)                          */
/*  ARGUMENT        : char *: 内部エラーコード                              */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : エラー出力ログ出力処理                                */
/****************************************************************************/
void CCUT_put_errlog( char *internal_errcode )
{
    cr401_def       *pst_rec_rcv = (cr401_def *)g_recv_buf;  // 受信電文
    db_glelg_def    glelg_rec;
    short           wklen;
    char            wkbuf1[10];
    short           s_result;

    /* エラー出力ログ編集 */
    memset( (char *)&glelg_rec, DEF_BUF_SPACE, sizeof(glelg_rec));
    glelg_rec.err_denbun_id = DEF_ELG_INTERNAL;
    memcpy(glelg_rec.mti_id, pst_rec_rcv->control_info.mti, sizeof(pst_rec_rcv->control_info.mti));
    memcpy(glelg_rec.naibu_err_code, internal_errcode, sizeof(glelg_rec.naibu_err_code));
    
    // 電文送受信情報
    memcpy(glelg_rec.srv_cls_info.srv_cls_id, g_myinfo.serverclass_name, sizeof(glelg_rec.srv_cls_info.srv_cls_id));
    memcpy(glelg_rec.srv_cls_info.srv_cls_mlt_num, "0000", sizeof(glelg_rec.srv_cls_info.srv_cls_mlt_num));
    memcpy(glelg_rec.denbun_send_recv_info.nw_kubun, g_myinfo.nw_kbn, sizeof(g_myinfo.nw_kbn));
    memcpy(glelg_rec.denbun_send_recv_info.mti_id, pst_rec_rcv->control_info.mti, sizeof(pst_rec_rcv->control_info.mti));
    glelg_rec.denbun_send_recv_info.send_denbun_shubetu = DEF_SEND_DENBUN_CTR_REQ;
    memcpy((char *)&glelg_rec.denbun_send_recv_info.denbun_log_key, (char *)&pst_rec_rcv->control_info.denbun_log_key, sizeof(pst_rec_rcv->control_info.denbun_log_key));
    glelg_rec.denbun_send_recv_info.denbun_fmt_kubun = DEF_DENBUN_FMT_8583;

    // 通信制御情報
    memcpy(glelg_rec.tushin_cntrl_info.if_id, pst_rec_rcv->control_info.interface_name, sizeof(pst_rec_rcv->control_info.interface_name));
    memcpy(glelg_rec.tushin_cntrl_info.station_id, pst_rec_rcv->control_info.station_name, sizeof(pst_rec_rcv->control_info.station_name));
    memcpy(glelg_rec.tushin_cntrl_info.line_info.recv_connect_id, (char *)&pst_rec_rcv->control_info.connection_lid, sizeof(pst_rec_rcv->control_info.connection_lid));

    // 電文部
    wklen = (short)(pst_rec_rcv->common_header.control_data_length - sizeof(pst_rec_rcv->control_info));
    memset(wkbuf1, 0x00, sizeof(wkbuf1));
    sprintf(wkbuf1, "%05d", wklen);
    memcpy(&glelg_rec.denbun_area.denbun_len, wkbuf1, strlen(wkbuf1));
    memcpy(&glelg_rec.denbun_area.mti_start_lct, g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].denbun_item_lct_info.mti_start_lct, sizeof(glelg_rec.denbun_area.mti_start_lct));
    memcpy(&glelg_rec.denbun_area.denbun, pst_rec_rcv->data_bu.message_text, wklen);
    wklen = (sizeof(glelg_rec) - sizeof(glelg_rec.denbun_area.denbun)) + wklen;

    // エラー出力ログ出力
    memset( &g_com_erl_arg_1, DEF_BUF_NULL , sizeof(g_com_erl_arg_1) );
    memset( &g_cg010in_modle.emsinf, DEF_BUF_SPACE, sizeof(g_cg010in_modle.emsinf) );
    memset( &g_com_erl_arg_3, DEF_BUF_SPACE, sizeof(g_com_erl_arg_3) );
    g_com_erl_arg_1.file_io_type = DEF_COM_ERL_ARG1_WRITE;
    g_com_erl_arg_1.io_timer     = g_myinfo.send_timer;
    g_com_erl_arg_1.data_len     = wklen;
    g_com_erl_arg_1.data_area    = (char *)&glelg_rec;
    s_result = COM_ERL( &g_com_erl_arg_1                   /* エラーログ共通処理実行 */
                      , &g_com_erl_arg_2
                      , &g_cg010in_modle
                      , &g_com_erl_arg_3
                      , g_myinfo.prog_id );
    if ( s_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR        /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@U"
                            , "COM_ERL"
                            , s_result );
        CMIN_abend();                                       /* 異常終了            */
    }
} /* CCUT_put_errlog */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_read_glmlg                                 */
/*  CALLING SEQ.    : short set_comiom_arg(char *,short,char *,short,short,  */
/*                                         short, char *, char *)            */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.pch_lname       (I)   論理ファイル名                 */
/*                  : 3.sh_fie_no       (I)   ファイル番号                   */
/*                  : 4.pch_rec_key     (I)   読込みキー                     */
/*                  : 5.sh_lock         (I)   LOCK有無                       */
/*                  : 6.sh_key_len      (I)   キー長                         */
/*                  : 7.sh_rec_len      (I)   レコード長                     */
/*                  : 8.ptrace_inf      (O)   トレース情報                   */
/*                  : 9.pfile_inf       (O)   ファイル情報                   */
/*                  :10.pin_inf         (O)   入力情報                       */
/*  RETURN CODE     :  void                                                  */
/*  DESCRIPTION     : 制御電文ログCOM_IOM用引数設定処理                      */
/*****************************************************************************/
static void set_comiom_arg(
    char                    *pch_pname,         // 物理ファイル名
    char                    *pch_lname,         // 論理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    short                    sh_key_len,        // キー長
    short                    sh_rec_len,        // レコード長
    COM_IOM_arg_3_def       *ptrace_inf,        // トレース情報
    COM_IOM_arg_4_def       *pfile_inf,         // ファイル情報
    COM_IOM_arg_5_def       *pin_inf            // 入力情報
)
{
    //------------------------------------------------------------------------//
    // トレース情報の編集                                                     //
    //------------------------------------------------------------------------//
    // トレース情報・モジュールID
    ccut_stb_cpy(ptrace_inf->prog_id,   g_myinfo.prog_id,         sizeof(ptrace_inf->prog_id));
    // トレース情報・ファイルID
    ccut_stb_cpy(ptrace_inf->file_id,   pch_lname,   sizeof(ptrace_inf->file_id));
    // トレース情報・ファイル名
    ccut_stb_cpy(ptrace_inf->file_name, pch_pname,  sizeof(ptrace_inf->file_name));

    //------------------------------------------------------------------------//
    // ファイル情報の編集                                                     //
    //------------------------------------------------------------------------//
    // ファイル情報・論理ファイル名
    ccut_stb_cpy(pfile_inf->file_id,   pch_lname,   strlen(pch_lname));
    // ファイル情報・物理ファイル名
    ccut_stb_cpy(pfile_inf->file_name, pch_pname,   sizeof(pfile_inf->file_name));
    // ファイル情報・ファイル番号
    pfile_inf->file_no = sh_fie_no;

    //------------------------------------------------------------------------//
    // 入力情報の編集                                                         //
    //------------------------------------------------------------------------//
    // 入力情報・パーティション識別
    pin_inf->part_key_type =0;
    // 入力情報・パーティション情報開始位置
    pin_inf->part_key_position =0;
    // 入力情報・パーティション情報長
    pin_inf->part_key_len =0;
    // 入力情報・KEY-VALUE
    memcpy(pin_inf->key_value, pch_rec_key, sh_key_len);
    // 入力情報・KEY識別
    memcpy(pin_inf->key_type,"10", 2);
    // 入力情報・KEY-LENGTH
    pin_inf->key_len = sh_key_len;
    // 入力情報・COMPARE-LENGTH
    pin_inf->compare_len = sh_key_len;
    // 入力情報・POSITIONNING-MODE
    pin_inf->positioning_mode =2;
    // 入力情報・LOCKフラグ
    pin_inf->lock_flg =sh_lock;
    // 入力情報・昇順/降順識別
    pin_inf->asc_desc_type =0;
    // 入力情報・I/Oタイマ
    pin_inf->io_timer =(long)g_myinfo.io_timer;
    // 入力情報・レコード長
    pin_inf->rec_len =sh_rec_len;

    return;
} /* end of set_comiom_arg */
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  ccut_stb_cpy                                    */
/*  CALLING SEQ.    : short ccut_stb_cpy(void *,void *size_t)                */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.dst             (O)   コピー先                       */
/*                  : 2.src             (I)   コピー元                       */
/*                  : 3.len             (I)   出力サイズ                     */
/*  RETURN CODE     :  void                                                  */
/*  DESCRIPTION     : 指定長空白埋めコピー処理                               */
/*****************************************************************************/
static void ccut_stb_cpy(void *dst, void *src, size_t len)
{
    size_t cplen = len;
    size_t i;
    char *psrc = src;
    char *pdst = dst;

    // 全空白で初期化
    memset(pdst, DEF_BUF_SPACE, len);

    // NULL文字検索、あればその位置までコピー指定
    for(i = 0; i < len; i++){
        if(psrc[i] == 0x00){
            // NULL文字あり
            cplen = i;
            break;
        }
    }
    memcpy(pdst, psrc, cplen);

    return;
} /* end of ccut_stb_cpy */

