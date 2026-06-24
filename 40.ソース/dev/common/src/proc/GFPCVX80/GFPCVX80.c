/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX80                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                                                                           */
/*                               制御電文共通となる常駐プロセス制御を行う。  */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-06                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/03/06 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/* STANDARD HEADER */
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <stdio.h>    nolist
#include <stdarg.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist
//#include "zspic"  nolist
//#include "zfilc"  nolist
#include "zsysc"  nolist

//#include "msg_CA.h"
#include "file.h"
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVXZ2.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVX80.h"                        /* 局状態・エコー制御サーバ     */
#include "GFPCVX81.h"                        /* 局状態・エコー制御サーバ     */
#include "GFPCGXH0.h"                        /* システム採番用PATHSEND       */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_set_prgid                             */
/*  CALLING SEQ.    : short CMIN_kbt_set_prgid(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 プログラムIDセット                               */
/****************************************************************************/
void  CMIN_kbt_set_prgid()
{
/* -------------------------------------------- */
/* 個別 プログラムIDをセット                    */
/* -------------------------------------------- */
    memcpy( g_myinfo.prog_id, DEF_GFPCVX80, strlen(DEF_GFPCVX80));

} /* end of CMIN_kbt_set_prgid */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_get_physical_names                    */
/*  CALLING SEQ.    : short CMIN_kbt_get_physical_names(void)               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 個別 物理名情報ファイル読み込み処理                   */
/****************************************************************************/
short  CMIN_kbt_get_physical_names()
{

    short            ls_result;
    db_gfphi_def    *phy_tbl_local;
    t_gfphi_pri_key  l_gfphi_pri_key;

/* ------------------------------------------------------ */
/* 接続先固有情報ファイル 物理名取得                      */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id, sizeof(l_gfphi_pri_key.grp_id));
    memset(&l_gfphi_pri_key.srv_cls_key
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CNCT_INFO , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO    , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO    , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_file_data.cnct_inf_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_kbt_file_data.cnct_inf_fname));

/* ------------------------------------------------------ */
/* 局状態管理ファイル  物理名取得                         */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id
          , sizeof(l_gfphi_pri_key.grp_id));
    memset(&l_gfphi_pri_key.srv_cls_key
          , DEF_BUF_NO_SET
          , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CEN_STS
          , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO
          , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO
          , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_file_data.stan_sts_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_kbt_file_data.stan_sts_fname));

/* ------------------------------------------------------ */
/* エコー状態管理ファイル  物理名取得                     */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id , sizeof(l_gfphi_pri_key.grp_id));
    memset(&l_gfphi_pri_key.srv_cls_key
          , DEF_BUF_NO_SET    , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_ECH_STS    , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO     , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO     , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    /* IOモジュール結果判定 */
    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;                   /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_file_data.echo_mng_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_kbt_file_data.echo_mng_fname));

/* ------------------------------------------------------ */
/* 制御電文管理ファイル  物理名取得                       */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id  , sizeof(l_gfphi_pri_key.grp_id));
    memset(&l_gfphi_pri_key.srv_cls_key
          , DEF_BUF_NO_SET     , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CTRL_DEN_LOG, sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_CZERO      , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_CZERO      , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    /* IOモジュール結果判定 */
    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;                   /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_file_data.ctrl_log_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_kbt_file_data.ctrl_log_fname));

/* ------------------------------------------------------ */
/* GFP内部LCN採番サーバ情報取得                           */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id, sizeof(l_gfphi_pri_key.grp_id));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind
          , DEF_SC_GFP_LCN   , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num
          , "0001"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num
          , "0000"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_svrcls.lcn_domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_kbt_svrcls.lcn_domain_name)-1);
    memcpy( g_kbt_svrcls.lcn_pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_kbt_svrcls.lcn_pathmon_name)-1);
    memcpy( g_kbt_svrcls.lcn_srvcls_name , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_kbt_svrcls.lcn_srvcls_name)-1);

/* ------------------------------------------------------ */
/* system採番生成サーバ情報取得                           */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id, sizeof(l_gfphi_pri_key.grp_id));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind
          , DEF_SC_SYS_NUM   , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num
          , "0000"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num
          , "0000"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_sys_no_data_pri.domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_sys_no_data_pri.domain_name));
    memcpy( g_sys_no_data_pri.pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_sys_no_data_pri.pathmon_name));
    memcpy( g_sys_no_data_pri.server_class , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_sys_no_data_pri.server_class));

    /* セカンダリ */
    l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num[3] = '1';
    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_sys_no_data_sec.domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_sys_no_data_sec.domain_name));
    memcpy( g_sys_no_data_sec.pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_sys_no_data_sec.pathmon_name));
    memcpy( g_sys_no_data_sec.server_class , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_sys_no_data_sec.server_class));

/* ------------------------------------------------------ */
/* コマンドサーバ情報取得                                 */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id, sizeof(l_gfphi_pri_key.grp_id));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind
          , DEF_SC_CMD_SRV   , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num
          , "0000"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num
          , "0000"           , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_mlt_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset( l_gfphi_pri_key.prc_file_key.prc_file_mlt_num
          , DEF_BUF_NO_SET   , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_mlt_num));

    ls_result = CMIN_get_phy_name ((char *)&l_gfphi_pri_key);

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    phy_tbl_local = (db_gfphi_def *)g_com_iom_arg_6.rec_area;
    memcpy( g_kbt_svrcls.cmd_domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_kbt_svrcls.cmd_domain_name)-1);
    memcpy( g_kbt_svrcls.cmd_pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_kbt_svrcls.cmd_pathmon_name)-1);
    memcpy( g_kbt_svrcls.cmd_srvcls_name , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_kbt_svrcls.cmd_srvcls_name)-1);

    return DEF_RET_OK;
} /* end of CMIN_kbt_get_physical_names */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_init                                   */
/*  CALLING SEQ.    : short CMIN_kbt_init (void)                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 個別初期処理                                           */
/*****************************************************************************/
short  CMIN_kbt_init(void)
{
    short                ls_result;

// ---------------------------------------------------------------------------/
//  個別で必要な初期処理が存在する場合はこの関数を使用する。無ければこのまま  /
// ---------------------------------------------------------------------------/

/* ------------------------------------------------------ */
/* 接続先固有情報取得                                     */
/* ------------------------------------------------------ */
    memset((char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
          , DEF_BUF_NULL, sizeof(db_gfnws_def));

    /* 接続先固有情報ファイルレコード登録単位確認 */
    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].gfnws_info.rec_unit[DEF_FNWS_IDX_NETWORK] == DEF_CHR_FLG_ON ) {
        /* NW単位ファイルあり */

        /* IOモジュールパラメータ初期化 */
        memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
        memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
        memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
        memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
        memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

        memset(&g_gfnws_pkey           , DEF_BUF_SPACE , sizeof(g_gfnws_pkey));
        g_gfnws_pkey.nw_id  = g_myinfo.network_id;
        memset( g_gfnws_pkey.if_id     , DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.if_id     ));
        memset( g_gfnws_pkey.station_id, DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.station_id));
        memset( g_gfnws_pkey.connect_id, DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.connect_id));

        /* トレース情報設定 */
        memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                            , sizeof(g_com_iom_arg_3.prog_id     ));
        memcpy( g_com_iom_arg_3.file_id     , DEF_FL_NW_KOYU_INFO
                                            , sizeof(g_com_iom_arg_3.file_id     ));
        memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.cnct_inf_fname
                                            , sizeof(g_com_iom_arg_3.file_name   ));
        memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                            , sizeof(g_com_iom_arg_3.file_io_type));
        /* ファイル情報設定 */
        memcpy( g_com_iom_arg_4.file_id     , DEF_GFNWS
                                            , strlen(DEF_GFNWS));
        memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.cnct_inf_fname
                                            , sizeof(g_com_iom_arg_4.file_name));
        g_com_iom_arg_4.file_no             = g_kbt_file_data.cnct_inf_fno;
        /* 入力情報 */
        g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
        g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
        g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
        memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gfnws_pkey
                                            , sizeof(g_gfnws_pkey));
        memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                            , strlen(DEF_COM_IOM_KEYTYPE_PRI));
        g_com_iom_arg_5.key_len             = sizeof(g_gfnws_pkey);
        g_com_iom_arg_5.compare_len         = sizeof(g_gfnws_pkey);
        g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
        g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
        g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
        g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
        g_com_iom_arg_5.rec_len             = sizeof(db_gfnwi_def);

        ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                            , g_ch_sub_prog_sts
                            ,&g_com_iom_arg_3
                            ,&g_com_iom_arg_4
                            ,&g_com_iom_arg_5
                            ,&g_com_iom_arg_6);

        if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
            ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@C@C@C@C@C@U"
                                , ""
                                , ""
                                , DEF_FL_CNCT_INFO
                                , DEF_COM_IOM_FUNC_STARTREAD
                                , g_com_iom_arg_5.key_value
                                , g_com_iom_arg_6.guardian_errcode );
            return DEF_RET_NG;
        }

        memcpy((char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
              , g_com_iom_arg_6.rec_area, sizeof(db_gfnws_def));
    }

    return DEF_RET_OK;
} /* CMIN_kbt_init */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_file_open                              */
/*  CALLING SEQ.    : short CMIN_kbt_file_open (void)                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 個別ファイルオープン処理                               */
/*****************************************************************************/
short CMIN_kbt_file_open()
{
    short   ls_result;                            /* WK処理結果              */

    /* 接続先固有情報ファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_CNCT_INFO
                              , g_kbt_file_data.cnct_inf_fname
                              ,&g_kbt_file_data.cnct_inf_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 局状態管理ファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_CEN_STS
                              , g_kbt_file_data.stan_sts_fname
                              ,&g_kbt_file_data.stan_sts_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* エコー状態管理ファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_ECH_STS
                              , g_kbt_file_data.echo_mng_fname
                              ,&g_kbt_file_data.echo_mng_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 制御電文ログファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_CTRL_DEN_LOG
                              , g_kbt_file_data.ctrl_log_fname
                              ,&g_kbt_file_data.ctrl_log_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* end of CMIN_kbt_file_open */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_kbt_file_close                             */
/*  CALLING SEQ.    : void CMIN_kbt_file_close(void)                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 個別ファイルクローズ処理                               */
/*****************************************************************************/
void CMIN_kbt_file_close()
{
    /* 接続先固有情報ファイルクローズ */
    CMIN_file_close( DEF_FL_CNCT_INFO   , g_kbt_file_data.cnct_inf_fname
                                        ,&g_kbt_file_data.cnct_inf_fno);

    /* 局状態管理ファイルクローズ */
    CMIN_file_close( DEF_FL_CEN_STS     , g_kbt_file_data.stan_sts_fname
                                        ,&g_kbt_file_data.stan_sts_fno);

    /* エコー状態管理ファイルクローズ */
    CMIN_file_close( DEF_FL_ECH_STS     , g_kbt_file_data.echo_mng_fname
                                        ,&g_kbt_file_data.echo_mng_fno);

    /* 制御電文ログファイルクローズ */
    CMIN_file_close( DEF_FL_CTRL_DEN_LOG, g_kbt_file_data.ctrl_log_fname
                                        ,&g_kbt_file_data.ctrl_log_fno);

} /* end of CMIN_kbt_file_close */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_handle_req_msg                             */
/*  CALLING SEQ.    : void CMIN_handle_req_msg (void)                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 電文受信処理                                           */
/*****************************************************************************/
void CMIN_handle_req_msg(void)
{
    common_header_def *lp_cmn_head;
    cr401_def         *lp_r401;
    t_rcv_info_def     l_rslt_info;

    lp_cmn_head  = (common_header_def *)g_recv_buf;
    lp_r401      = (cr401_def *)g_resp_buf;

    memset(&l_rslt_info, DEF_BUF_NULL, sizeof(t_rcv_info_def));
    memcpy( l_rslt_info.naibu_errcd, DEF_NERR_NOMAL, strlen(DEF_NERR_NOMAL));
    l_rslt_info.repl_data_len = DEF_BUF_NULL;
    l_rslt_info.repl_code     = DEF_BUF_NULL;
    g_son_retry = DEF_CSTE_RETRY_OFF;

    if ( memcmp( lp_cmn_head->interface_code
               , DEF_IPC_IFCD_NW_MSG_REQ, strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == DEF_RET_OK) {
        /* ---------------------------------------------- */
        /* NW電文受信要求処理                             */
        /* ---------------------------------------------- */
        CSTE_handle_req_msg_c401(&l_rslt_info);
    } else
    if ( memcmp( lp_cmn_head->interface_code
               , DEF_IPC_IFCD_CTRL_MSG_REQ, strlen(DEF_IPC_IFCD_CTRL_MSG_REQ)) == DEF_RET_OK) {
        /* ---------------------------------------------- */
        /* 制御電文作成要求処理                           */
        /* ---------------------------------------------- */
        CSTE_handle_req_msg_c402(&l_rslt_info);
    } else {
        CMIN_message_output ( DEF_EVT_REQ_ERR         /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_IPC_SEISA_ERR
                            , "@C@H"
                            , "ｲﾝﾀﾌｪｰｽｺｰﾄﾞｴﾗｰ"
                            , lp_cmn_head);
        l_rslt_info.repl_data_len = 0;
        return;
    }

    /* ---------------------------------------------- */
    /* 応答送信                                       */
    /* ---------------------------------------------- */
    CMIN_send_reply((char *)g_resp_buf, l_rslt_info.repl_data_len, l_rslt_info.repl_code);

    /* ------------------------------------------ */
    /* 自動開局コマンド送信                       */
    /* ------------------------------------------ */
    if ((( memcmp( lp_r401->common_header.interface_code
                 , DEF_IPC_IFCD_NW_MSG_RSP, strlen(DEF_IPC_IFCD_NW_MSG_RSP)) == DEF_RET_OK         )&&
         ( lp_r401->common_header.error_code                                 == DEF_IPC_ERRCD_OK   )&&
         ( lp_r401->control_info.control_kind.ctl_text_kbn                   == DEF_CTLTXT_CNT_OPN )&&
         ( memcmp( lp_r401->control_info.request_kind
                 , DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE))         == DEF_RET_OK         )&&
         ( g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].shori_kbn_info.trigger_open_need
                                                                             == DEF_FNWS_KYOKU_SEND)&&
         ( l_rslt_info.rsp_result == DEF_RSP_TYPE_NORMAL)) ||
        ( g_son_retry == DEF_CSTE_RETRY_ON )){
        CSTE_send_signon( &l_rslt_info );
    }

} /* end of STE_handle_req_msg */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0 CSTE_handle_req_msg_c401                         */
/*  CALLING SEQ.    : short CSTE_handle_req_msg_c401(short *)                */
/*  ARGUMENT        :                                                        */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : NW電文受信要求処理                                     */
/*****************************************************************************/
void  CSTE_handle_req_msg_c401(t_rcv_info_def *p_rslt_info)
{
    short           ls_result;
    short           ls_lock_flg;
    char            lc_wbuf[20];
    cr401_def      *lp_c401;

    lp_c401     = (cr401_def *)g_recv_buf;

    memcpy( p_rslt_info->ctrl_type,&lp_c401->control_info.control_kind
                                  , sizeof(control_kind_def));

    for(;;){
        /* ---------------------------------------------- */
        /* NW情報ファイル読込処理                         */
        /* ---------------------------------------------- */
        ls_result = CSTE_read_gfnwi( p_rslt_info );

        if ( ls_result != DEF_RET_OK ) {
            p_rslt_info->repl_data_len = 0;
            break;
        }
        /* ---------------------------------------------- */
        /* 接続先固有情報ファイル読込処理                 */
        /* ---------------------------------------------- */

        ls_result = CSTE_read_gfnws( p_rslt_info );

        if ( ls_result != DEF_RET_OK ) {
            p_rslt_info->repl_data_len = 0;
            break;
        }

        /* ---------------------------------------------- */
        /* 局状態取得(開閉局電文のみ実施)                 */
        /* ---------------------------------------------- */
        if (( lp_c401->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_CNT_OPN )||
            ( lp_c401->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_CNT_CLS ))  {
            /* 局状態管理ファイル読込み */
            ls_result = CSTE_read_gcsst( DEF_COM_IOM_LOCK, p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                p_rslt_info->repl_data_len = 0;
                break;
            }

            ls_lock_flg = DEF_FLAG_ON;
        }

        /* ---------------------------------------------- */
        /* 受信電文精査                                   */
        /* ---------------------------------------------- */
        if ( memcmp( lp_c401->control_info.request_kind    /* 被仕向要求受信      */
                   , DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE)) == DEF_RET_OK ){
            /* 開局・閉局・エコー要求電文精査 */
            ls_result = NWM_STE_check_reqmsg( (char *)&lp_c401->data_bu.message_text
                                            , (char *)&lp_c401->control_info.denbun_len
                                            , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                                            , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                                            , (gflin_pkey_def *)&lp_c401->control_info.connection_lid
                                            , p_rslt_info
                                            , (char *)&g_gcsst );

            /* 0,1 0:正常、以外は8:障害 */
            if ( ls_result != DEF_RSP_TYPE_NORMAL &&
                 ls_result != DEF_RSP_TYPE_KYOHI ) {
                p_rslt_info->rsp_result = DEF_RSP_TYPE_OBST;
                memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
                memcpy( lc_wbuf, lp_c401->control_info.mti, DEF_MTI_LEN);
                CMIN_message_output( DEF_EVT_DATA_FLD_SEISA_ERR
                                   , DEF_MSGTTKB_GYOM_ERR
                                   , DEF_NERR_HSMK_REQ_SEISA_ERR
                                   , "@L@T@C@C"
                                   , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                                   , &lp_c401->control_info.connection_lid
                                   , lc_wbuf
                                   , p_rslt_info->err_area);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_REQ_SEISA_ERR
                                                , strlen(DEF_NERR_HSMK_REQ_SEISA_ERR));
            }
        } else
        if ( memcmp( lp_c401->control_info.request_kind    /* 仕向応答受信        */
                   , DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == DEF_RET_OK ){
            /* 制御電文ログ取得 */
            ls_result = CSTE_read_glmlg( DEF_GLMLG_READ_REQ
                                       , DEF_COM_IOM_NOLOCK
                                       , DEF_S_H_KUBUN_SIMUKE
                                       , p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                if ( ls_lock_flg == DEF_FLAG_ON ) {
                    /* 局状態管理ファイルUNLOCK */
                    ls_result = CSTE_unlock_gcsst( p_rslt_info );
                    ls_lock_flg = DEF_FLAG_OFF;
                }
                p_rslt_info->repl_data_len = 0;
                break;
            }

            /* 開局・閉局・エコー応答電文精査 */
            ls_result = NWM_STE_check_rspmsg( (char *)&lp_c401->data_bu.message_text
                                            , (char *)&lp_c401->control_info.denbun_len
                                            , (char *)&g_glmlg.denbun_area.denbun
                                            , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                                            , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                                            , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                                            , (gflin_pkey_def *)&lp_c401->control_info.connection_lid
                                            , p_rslt_info);

            if ( ls_result != DEF_RSP_TYPE_NORMAL &&
                 ls_result != DEF_RSP_TYPE_KYOHI ) {
                p_rslt_info->rsp_result = DEF_RSP_TYPE_OBST;

                memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
                memcpy( lc_wbuf, lp_c401->control_info.mti, DEF_MTI_LEN);
                CMIN_message_output( DEF_EVT_DATA_FLD_SEISA_ERR
                                   , DEF_MSGTTKB_GYOM_ERR
                                   , DEF_NERR_SMK_RSP_SEISA_ERR
                                   , "@L@T@C@C"
                                   , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                                   , &lp_c401->control_info.connection_lid
                                   , lc_wbuf
                                   , p_rslt_info->err_area);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_RSP_SEISA_ERR
                                                , strlen(DEF_NERR_SMK_RSP_SEISA_ERR));
            }
        }

        if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_OBST ) {
            CSTE_error_msg_out( p_rslt_info );                  /* エラー出力ログ出力処理   */

            /* 開閉局_仕向応答受信判定        */
            if (( memcmp( lp_c401->control_info.request_kind
                   , DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == DEF_RET_OK ) &&
                (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )||
                 ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )))  {
               /* 開閉局_仕向応答受信の場合 */
               /* 制御機能種別・内部処理区分に'B'設定し、処理継続 */
               p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
            } 
            else {
                if ( ls_lock_flg == DEF_FLAG_ON ){
                    ls_result = CSTE_unlock_gcsst( p_rslt_info );   /* 局状態管理ファイルUNLOCK */
                    ls_lock_flg = DEF_FLAG_OFF;
                }
                break;
            }
        }

        memcpy( p_rslt_info->lcn_no, lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                                   , DEF_LCN_NO_LEN);

        if (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )||
            ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ))  {
            /* 開閉局処理 */
            if ( memcmp( lp_c401->control_info.request_kind /* 被仕向要求         */
                       , DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE)) == DEF_RET_OK ){

                /* 開閉局_被仕向要求受信処理 */
                ls_result = CSTE_sonof_hsm_req_rcv( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向応答           */
                       , DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == DEF_RET_OK ){

                /* 開閉局_仕向応答受信処理   */
                ls_result = CSTE_sonof_sim_rsp_rcv( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向応答Timeout    */
                       , DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT)) == DEF_RET_OK ){
                /* 開閉局_仕向応答待ちタイムアウト処理 */
                ls_result = CSTE_sonof_sim_rsp_timeout( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向要求送信不可   */
                       , DEF_CTLREQ_SIMUKE_ERROR, strlen(DEF_CTLREQ_SIMUKE_ERROR)) == DEF_RET_OK ){
                /* 開閉局_仕向要求送信不可処理 */
                ls_result = CSTE_sonof_sim_req_snd_err( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 被仕向応答送信不可 */
                       , DEF_CTLREQ_HISIMUKE_ERROR, strlen(DEF_CTLREQ_HISIMUKE_ERROR)) == DEF_RET_OK ){
                /* 開閉局_被仕向応答送信不可処理 */
                ls_result = CSTE_sonof_hsm_rsp_snd_err( p_rslt_info );
            }

            if ( ls_result != DEF_RET_OK ) {
                if ( ls_result == DEF_RET_NG_UNLOCK ) {
                    /* 局状態管理ファイルUNLOCK */
                    ls_result = CSTE_unlock_gcsst( p_rslt_info );
                }
                p_rslt_info->repl_data_len = 0;
            }
        } else {
            /* エコー処理 */
            /* 局状態管理ファイルUNLOCK */
            if ( ls_lock_flg == DEF_FLAG_ON ){
                ls_result = CSTE_unlock_gcsst( p_rslt_info );

                if ( ls_result != DEF_RET_OK ) {
                    p_rslt_info->repl_data_len = 0;
                    break;
                }
            }

            if ( memcmp( lp_c401->control_info.request_kind /* 被仕向要求         */
                       , DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE)) == DEF_RET_OK ){

                /* エコー_被仕向要求受信処理 */
                ls_result = CSTE_echo_hsm_req_rcv( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向応答           */
                       , DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == DEF_RET_OK ){

                /* エコー_仕向応答受信処理   */
                ls_result = CSTE_echo_sim_rsp_rcv( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向応答Timeout    */
                       , DEF_CTLREQ_TIMEOUT
                       , strlen(DEF_CTLREQ_TIMEOUT)) == DEF_RET_OK ){
                /* エコー_仕向応答待ちタイムアウト処理 */
                ls_result = CSTE_echo_sim_rsp_timeout( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 仕向要求送信不可   */
                       , DEF_CTLREQ_SIMUKE_ERROR
                       , strlen(DEF_CTLREQ_SIMUKE_ERROR)) == DEF_RET_OK ){
                /* エコー_仕向要求送信不可処理 */
                ls_result = CSTE_echo_sim_req_snd_err( p_rslt_info );
            } else
            if ( memcmp( lp_c401->control_info.request_kind /* 被仕向応答送信不可 */
                       , DEF_CTLREQ_HISIMUKE_ERROR, strlen(DEF_CTLREQ_HISIMUKE_ERROR)) == DEF_RET_OK ){
                /* エコー_被仕向応答送信不可処理 */
                ls_result = CSTE_echo_hsm_rsp_snd_err( p_rslt_info );
            }

            if ( ls_result != DEF_RET_OK ) {
                p_rslt_info->repl_data_len = 0;
            }
        }

        break;
    }

    /* NW電文受信応答IPC(R401)編集処理 */
    p_rslt_info->repl_data_len = CSTE_r401_edit( p_rslt_info );

} /* end of CSTE_handle_req_msg_c401 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_hsm_req_rcv                          */
/*  CALLING SEQ.    : short  CSTE_sonof_hsm_req_rcv()                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_被仕向要求受信処理                              */
/*****************************************************************************/
short  CSTE_sonof_hsm_req_rcv(t_rcv_info_def *p_rslt_info)
{
    short         ls_result;
    cr401_def    *lp_c401;
    cr401_def    *lp_r401;
    char          ems_data[12];
    char          ch_wk_data[8];

    lp_c401     = (cr401_def *)g_recv_buf;
    lp_r401     = (cr401_def *)g_resp_buf;

    memset(ems_data, 0, sizeof(ems_data));

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_SONF_H_RQ_RV, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG_UNLOCK;
    }

    /* 制御電文種別 要求応答区分 -> 応答設定 */
    p_rslt_info->ctrl_type[1] = DEF_REQRSP_KBN_RSP;

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(要求受信)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_req_rcv( g_gcsst.state_sts_info.state_sts
                                         , p_rslt_info );
    if ( ls_result != NWM_STE_SST_KYOKA ){
        /* EMS出力 (局状態エラー)*/
        memset(ch_wk_data, 0x00, sizeof(ch_wk_data));
        memcpy(ch_wk_data,
               g_gcsst.state_sts_info.state_sts,
               sizeof(g_gcsst.state_sts_info.state_sts));
        /* 局状態変更(開局処理中/閉局処理中) */
        if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {
            /* 開局 */
            CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_HSMK_SST_OPNING,
                                "@L@T@C",
                                lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
                                &lp_c401->control_info.connection_lid,
                                ch_wk_data);
        } else {
            /* 閉局 */
            CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_HSMK_SST_CLSING,
                                "@L@T@C",
                                lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
                                &lp_c401->control_info.connection_lid,
                                ch_wk_data);
        }
    }

    switch(ls_result) {
    case NWM_STE_SST_KYOKA :  /* 0:許可応答 */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_NORMAL;

        if ( memcmp(p_rslt_info->new_stn_sts, DEF_CSTE_STS_SPACE, sizeof(p_rslt_info->new_stn_sts)) != 0 ) {
            /* EMS出力 局状態変更(開局処理中/閉局処理中) */
            if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {
                /* 開局 */
                /* 開局処理中メッセージ */
                memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
            } else {
                /* 閉局 */
                /* 閉局処理中メッセージ */
                memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
            }
            /* EMS出力 局状態変更(開局処理中/閉局処理中) */
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                               , &lp_c401->control_info.connection_lid
                               , ems_data);

            /* 局状態管理ファイル更新 */
            ls_result = CSTE_update_gcsst( p_rslt_info);
            if ( ls_result != DEF_RET_OK ) {
                return DEF_RET_NG;
            }

            /* EMS出力 局状態変更(更新後局状態) */
            memset(ems_data, 0, sizeof(ems_data));

            if ( memcmp(p_rslt_info->new_stn_sts,
                        DEF_STTE_STS_OPN,
                        sizeof(p_rslt_info->new_stn_sts)) == 0) {
                /* 開局 */
                memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
            }
            else if ( memcmp(p_rslt_info->new_stn_sts,
                                          DEF_STTE_STS_CLS,
                                          sizeof(p_rslt_info->new_stn_sts)) == 0) {
                /* 閉局 */
                memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
            }
            else if ( memcmp(p_rslt_info->new_stn_sts,
                                          DEF_STTE_STS_OPNING,
                                          sizeof(p_rslt_info->new_stn_sts)) == 0) {
                /* 開局処理中 */
                memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
            }
            else if ( memcmp(p_rslt_info->new_stn_sts,
                                          DEF_STTE_STS_CLOSING,
                                          sizeof(p_rslt_info->new_stn_sts)) == 0) {
                /* 閉局処理中 */
                memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
            }

            /* 局状態更新EMS出力 */
            if (ems_data[0] != 0){
                CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                                   , DEF_MSGTTKB_NORMAL
                                   , DEF_NERR_NOMAL
                                   , "@L@T@C"
                                   , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                                   , &lp_c401->control_info.connection_lid
                                   , ems_data);
            }
        } else {
            /* 局状態管理ファイルUNLOCK */
            ls_result = CSTE_unlock_gcsst( p_rslt_info );
            if ( ls_result != DEF_RET_OK ) {
                return DEF_RET_NG;
            }
        }

        break;
    case NWM_STE_SST_KYOHI  :  /* 1:拒否応答 */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
        /* 局状態管理ファイルUNLOCK */
        ls_result = CSTE_unlock_gcsst( p_rslt_info );
        if ( ls_result != DEF_RET_OK ) {
            return DEF_RET_NG;
        }
        break;
    case NWM_STE_SST_HAKI  :  /* 3:電文破棄 */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_HAKI;
        /* 局状態管理ファイルUNLOCK */
        ls_result = CSTE_unlock_gcsst( p_rslt_info );
        if ( ls_result != DEF_RET_OK ) {
            return DEF_RET_NG;
        }
        return DEF_RET_OK;
    default:
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        /* 局状態管理ファイルUNLOCK */
        ls_result = CSTE_unlock_gcsst( p_rslt_info );
        if ( ls_result != DEF_RET_OK ) {
            return DEF_RET_NG;
        }
        return DEF_RET_NG;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー応答電文編集                 */
    /* ---------------------------------------------- */
    NWM_STE_edit_rspmsg( (char *)&lp_r401->data_bu.message_text
                       , (char *)&lp_c401->data_bu.message_text
                       , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                       , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                       , (gflin_pkey_def *)&lp_c401->control_info.connection_lid
                       ,&g_NWM_CTU_INI_arg_2
                       , p_rslt_info
                       ,&g_cg010in_modle
                       ,(ems_info_add*)&g_ems_add );

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_SONF_H_RQ_SD, p_rslt_info ) ;

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_sim_rsp_rcv                          */
/*  CALLING SEQ.    : short  CSTE_sonof_sim_rsp_rcv()                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_仕向応答受信処理                                */
/*****************************************************************************/
short   CSTE_sonof_sim_rsp_rcv(t_rcv_info_def *p_rslt_info)
{
    short       ls_result;
    cr401_def  *lp_c401;
    short       ls_autoopen_flg;
    short       update_flg = DEF_FLAG_OFF;
    char        ems_data[12];

    lp_c401     = (cr401_def *)g_recv_buf;
    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    /* 電文精査NG時はログ出力しない */
    if (p_rslt_info->rsp_result != DEF_RSP_TYPE_OBST) {
        ls_result = CSTE_put_glmlg( DEF_SONF_S_RP_RV, p_rslt_info );

        if ( ls_result != DEF_RET_OK ) {
            return DEF_RET_NG_UNLOCK;
        }
    }

    /* 制御電文種別 要求応答区分 -> 応答設定 */
    p_rslt_info->ctrl_type[1] = DEF_REQRSP_KBN_RSP;

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(仕向応答)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_rsp_rcv( g_gcsst.state_sts_info.state_sts
                                         , (char *)&lp_c401->control_info.request_kind
                                         , (char *)&lp_c401->data_bu.message_text
                                         , p_rslt_info );

    /* 自動開局判定 */
    ls_autoopen_flg = CSTE_is_auto_signon();

    /* 処理結果判定 */
    if ( ls_autoopen_flg == DEF_CSTE_OPEN_AUTO ) {  /* 自動開局 */
        if ( ls_result == NWM_STE_SST_UPDATE ) {
            memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
            memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        } else {
            memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
        }
    } else {
        if ( ls_result == NWM_STE_SST_UPDATE ) {
            memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        } else {
            ls_result = CSTE_unlock_gcsst( p_rslt_info );
        }
    }

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* EMS出力 局状態変更(開局/閉局) */
    if (update_flg == DEF_FLAG_ON) {
        /* EMS出力 局状態変更(更新後局状態) */
        memset(ems_data, 0, sizeof(ems_data));

        if ( memcmp(p_rslt_info->new_stn_sts,
                    DEF_STTE_STS_OPN,
                    sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局 */
            memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLS,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局 */
            memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_OPNING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局処理中 */
            memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLOSING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局処理中 */
            memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }

        /* 局状態更新EMS出力 */
        if (ems_data[0] != 0){
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                               , &lp_c401->control_info.connection_lid
                               , ems_data);
        }
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_sim_rsp_timeout                      */
/*  CALLING SEQ.    : short  CSTE_sonof_sim_rsp_timeout()                    */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_仕向応答待ちタイムアウト処理                    */
/*****************************************************************************/
short   CSTE_sonof_sim_rsp_timeout(t_rcv_info_def *p_rslt_info)
{
    short       ls_result;
    cr401_def   *lp_c401;
    char        lc_wkbuf[10];
    long        ll_retry_cnt;
    long        ll_retry_max;
    short       ls_autoopen_flg;
    short       update_flg = DEF_FLAG_OFF;
    char        ems_data[12];

    lp_c401     = (cr401_def *)g_recv_buf;
    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_SONF_S_RP_TIMEOUT_RV, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG_UNLOCK;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(仕向応答)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_rsp_rcv( g_gcsst.state_sts_info.state_sts
                                         , (char *)&lp_c401->control_info.request_kind
                                         , (char *)&lp_c401->data_bu.message_text
                                         , p_rslt_info );

    /* 自動開局判定 */
    ls_autoopen_flg = CSTE_is_auto_signon();

    if (ls_autoopen_flg == DEF_CSTE_OPEN_AUTO) {
        /* 開局リトライ回数判定 */
        memset( lc_wkbuf, 0x00, sizeof(lc_wkbuf) );
        memcpy( lc_wkbuf, g_gcsst.state_sts_info.open_state_retry_num, 
            sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
        ll_retry_cnt = atol( lc_wkbuf );
        memset( lc_wkbuf, 0x00, sizeof(lc_wkbuf) );
        memcpy( lc_wkbuf, g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].cntrl_denbun_cnt_info.open_retry_num, 
            sizeof( g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].cntrl_denbun_cnt_info.open_retry_num ));
        ll_retry_max = atol( lc_wkbuf );
        if ( ll_retry_cnt < ll_retry_max ) {
            g_son_retry = DEF_CSTE_RETRY_ON;
        } else {
            /* リトライオーバー */
            CMIN_message_output( DEF_EVT_SIGN_ON_RETRY_OVER
                               , DEF_MSGTTKB_GYOM_ERR
                               , DEF_NERR_SIGN_ON_RETRY_OVER
                               , "@T"
                               , &lp_c401->control_info.connection_lid);
        }
    }

    /* 処理結果判定 */
    if ( ls_autoopen_flg == DEF_CSTE_OPEN_AUTO ) {  /* 自動開局 */
        if ( g_son_retry == DEF_CSTE_RETRY_ON ) {
            switch ( ls_result ) {
            case NWM_STE_SST_UPDATE :
                memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
                memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
                g_son_retry = DEF_CSTE_RETRY_OFF;
                ls_result = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
                break;
            case NWM_STE_SST_NOUPDATE :
                memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
                g_son_retry = DEF_CSTE_RETRY_OFF;
                ls_result = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
                break;
            case NWM_STE_SST_UPDATE_RETRY :
                memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
                ls_result = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
                break;
            case NWM_STE_SST_NOUPDATE_RETRY :
                ls_result = CSTE_unlock_gcsst( p_rslt_info );
                break;
            default :
                break;
            }
        } else {
            switch ( ls_result ) {
            case NWM_STE_SST_UPDATE :
            case NWM_STE_SST_UPDATE_RETRY :
                memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
                memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
                ls_result = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
                break;
            case NWM_STE_SST_NOUPDATE :
            case NWM_STE_SST_NOUPDATE_RETRY :
                memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
                ls_result = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
                break;
            default :
                break;
            }
        }
    } else {
        if ( ls_result == NWM_STE_SST_UPDATE ) {
            memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        } else {
            ls_result = CSTE_unlock_gcsst( p_rslt_info );
        }
    }

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    if (update_flg == DEF_FLAG_ON) {
        /* EMS出力 局状態変更(更新後局状態) */
        memset(ems_data, 0, sizeof(ems_data));

        if ( memcmp(p_rslt_info->new_stn_sts,
                    DEF_STTE_STS_OPN,
                    sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局 */
            memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLS,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局 */
            memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_OPNING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局処理中 */
            memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLOSING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局処理中 */
            memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }

        /* 局状態更新EMS出力 */
        if (ems_data[0] != 0){
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                               , &lp_c401->control_info.connection_lid
                               , ems_data);
        }
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_sim_req_snd_err                      */
/*  CALLING SEQ.    : short  CSTE_sonof_sim_req_snd_err()                    */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_仕向要求送信不可処理                            */
/*****************************************************************************/
short   CSTE_sonof_sim_req_snd_err(t_rcv_info_def *p_rslt_info)
{
    short ls_result;
    short ls_autoopen_flg;
    short update_flg = DEF_FLAG_OFF;
    char  ems_data[12];

    cr401_def *lp_c401;

    lp_c401     = (cr401_def *)g_recv_buf;
    /* ---------------------------------------------- */
    /* 制御電文ログ更新処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_update_glmlg( DEF_S_H_KUBUN_SIMUKE, DEF_GLMLG_READ_REQ, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG_UNLOCK;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(仕向応答)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_rsp_rcv( g_gcsst.state_sts_info.state_sts
                                         , (char *)&lp_c401->control_info.request_kind
                                         , (char *)&lp_c401->data_bu.message_text
                                         , p_rslt_info );

    /* 自動開局判定 */
    ls_autoopen_flg = CSTE_is_auto_signon();

    /* 処理結果判定 */
    if ( ls_autoopen_flg == DEF_CSTE_OPEN_AUTO ) {  /* 自動開局 */
        if ( ls_result == NWM_STE_SST_UPDATE ) {
            memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
            memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        } else {
            memset( g_gcsst.state_sts_info.open_state_retry_num, '0', sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        }
        /* リトライ中断 */
        CMIN_message_output( DEF_EVT_SIGN_ON_RETRY_ERR
                           , DEF_MSGTTKB_GYOM_ERR
                           , DEF_NERR_SIGN_ON_RETRY_ERR
                           , "@T"
                           , &lp_c401->control_info.connection_lid);
    } else {
        if ( ls_result == NWM_STE_SST_UPDATE ) {
            memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts, sizeof( p_rslt_info->new_stn_sts ));
            ls_result = CSTE_update_gcsst( p_rslt_info );
            update_flg = DEF_FLAG_ON;
        } else {
            ls_result = CSTE_unlock_gcsst( p_rslt_info );
        }
    }

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    if (update_flg == DEF_FLAG_ON) {
        /* EMS出力 局状態変更(更新後局状態) */
        memset(ems_data, 0, sizeof(ems_data));

        if ( memcmp(p_rslt_info->new_stn_sts,
                    DEF_STTE_STS_OPN,
                    sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局 */
            memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLS,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局 */
            memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_OPNING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局処理中 */
            memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLOSING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局処理中 */
            memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }

        /* 局状態更新EMS出力 */
        if (ems_data[0] != 0){
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                               , &lp_c401->control_info.connection_lid
                               , ems_data);
        }
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_hsm_rsp_snd_err                      */
/*  CALLING SEQ.    : short  CSTE_sonof_hsm_rsp_snd_err()                    */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_被仕向応答送信不可処理                          */
/*****************************************************************************/
short   CSTE_sonof_hsm_rsp_snd_err(t_rcv_info_def *p_rslt_info)
{
    short      ls_result;
    short      update_flg = DEF_FLAG_OFF;
    char       ems_data[12];
    cr401_def *lp_c401;

    lp_c401   = (cr401_def *)g_recv_buf;

    /* ---------------------------------------------- */
    /* 制御電文ログ更新処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_update_glmlg( DEF_S_H_KUBUN_HISIMUKE, DEF_GLMLG_READ_RSP, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG_UNLOCK;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(仕向応答)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_rsp_err( g_gcsst.state_sts_info.state_sts
                                         , p_rslt_info );

    if ( ls_result == NWM_STE_SST_UPDATE ) {                    /* ファイル更新有り   */
        /* 局状態管理ファイル更新 */
        ls_result = CSTE_update_gcsst( p_rslt_info );
        update_flg = DEF_FLAG_ON;
    } else {                                            /* ファイル更新なし   */
        /* 局状態管理ファイルUNLOCK */
        ls_result = CSTE_unlock_gcsst( p_rslt_info );
    }

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    if (update_flg == DEF_FLAG_ON) {
        /* EMS出力 局状態変更(更新後局状態) */
        memset(ems_data, 0, sizeof(ems_data));

        if ( memcmp(p_rslt_info->new_stn_sts,
                    DEF_STTE_STS_OPN,
                    sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局 */
            memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLS,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局 */
            memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_OPNING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局処理中 */
            memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLOSING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局処理中 */
            memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }

        /* 局状態更新EMS出力 */
        if (ems_data[0] != 0){
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                               , &lp_c401->control_info.connection_lid
                               , ems_data);
        }
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_hsm_req_rcv                           */
/*  CALLING SEQ.    : short  CSTE_echo_hsm_req_rcv()                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_被仕向要求受信処理                              */
/*****************************************************************************/
short  CSTE_echo_hsm_req_rcv(t_rcv_info_def *p_rslt_info)
{
    short ls_result;
    cr401_def    *lp_c401;
    cr401_def    *lp_r401;
    char          ch_wk_data[8];

    lp_c401     = (cr401_def *)g_recv_buf;
    lp_r401     = (cr401_def *)g_resp_buf;

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_ECHO_H_RQ_RV, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 制御電文種別 要求応答区分 -> 応答設定 */
    p_rslt_info->ctrl_type[1] = DEF_REQRSP_KBN_RSP;


    /* ---------------------------------------------- */
    /* 局状態管理ファイル読込み                       */
    /* ---------------------------------------------- */
    ls_result = CSTE_read_gcsst( DEF_COM_IOM_NOLOCK, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(要求受信)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_req_rcv( g_gcsst.state_sts_info.state_sts
                                         , p_rslt_info );

    if ( ls_result != NWM_STE_SEISA_NORMAL ) {
        p_rslt_info->rsp_result = DEF_RSP_TYPE_NORMAL;
        /* EMS出力 (局状態エラー)*/
        memset(ch_wk_data, 0x00, sizeof(ch_wk_data));
        memcpy(ch_wk_data,
               g_gcsst.state_sts_info.state_sts,
               sizeof(g_gcsst.state_sts_info.state_sts));
        CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_HSMK_SST_ECHO,
                            "@L@T@C",
                            lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &lp_c401->control_info.connection_lid,
                            ch_wk_data);
    }

    /* エコー態管理ファイル更新 */
    ls_result = CSTE_update_gcest( p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー応答電文編集                 */
    /* ---------------------------------------------- */
    NWM_STE_edit_rspmsg( (char *)&lp_r401->data_bu.message_text
                       , (char *)&lp_c401->data_bu.message_text
                       , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                       , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                       , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                       , (gflin_pkey_def *)&lp_c401->control_info.connection_lid
                       ,&g_NWM_CTU_INI_arg_2
                       , p_rslt_info
                       ,&g_cg010in_modle
                       ,(ems_info_add*)&g_ems_add );

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_ECHO_H_RQ_SD, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_sim_rsp_rcv                           */
/*  CALLING SEQ.    : short  CSTE_echo_sim_rsp_rcv()                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_仕向応答受信処理                                */
/*****************************************************************************/
short   CSTE_echo_sim_rsp_rcv(t_rcv_info_def *p_rslt_info)
{
    short ls_result;

    /* 電文精査結果設定 */
    if (p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW){
        p_rslt_info->rsp_result = DEF_RSP_TYPE_NORMAL;
    }
    else {
        p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
    }

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_ECHO_S_RP_RV, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 制御電文種別 要求応答区分 -> 応答設定 */
    p_rslt_info->ctrl_type[1] = DEF_REQRSP_KBN_RSP;

    /* エコー態管理ファイル更新 */
    ls_result = CSTE_update_gcest( p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_sim_rsp_timeout                       */
/*  CALLING SEQ.    : short  CSTE_echo_sim_rsp_timeout()                     */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_仕向応答待ちタイムアウト処理                    */
/*****************************************************************************/
short   CSTE_echo_sim_rsp_timeout(t_rcv_info_def *p_rslt_info)
{
    short ls_result;

    /* ---------------------------------------------- */
    /* 制御電文ログ出力処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_put_glmlg( DEF_ECHO_S_RP_TIMEOUT_RV, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL) {
            p_rslt_info->rsp_result = ls_result;
        }
        return DEF_RET_NG;
    }

    /* エコー態管理ファイル更新 */
    ls_result = CSTE_update_gcest( p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_sim_req_snd_err                       */
/*  CALLING SEQ.    : short  CSTE_echo_sim_req_snd_err()                     */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_仕向要求送信不可処理                            */
/*****************************************************************************/
short   CSTE_echo_sim_req_snd_err(t_rcv_info_def *p_rslt_info)
{
    short ls_result;

    /* ---------------------------------------------- */
    /* 制御電文ログ更新処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_update_glmlg( DEF_S_H_KUBUN_SIMUKE, DEF_GLMLG_READ_REQ, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* エコー態管理ファイル更新 */
    ls_result = CSTE_update_gcest( p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_hsm_rsp_snd_err                       */
/*  CALLING SEQ.    : short  CSTE_echo_hsm_rsp_snd_err()                     */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_被仕向応答送信不可処理                          */
/*****************************************************************************/
short   CSTE_echo_hsm_rsp_snd_err(t_rcv_info_def *p_rslt_info)
{
    short ls_result;

    /* ---------------------------------------------- */
    /* 制御電文ログ更新処理                           */
    /* ---------------------------------------------- */
    ls_result = CSTE_update_glmlg( DEF_S_H_KUBUN_HISIMUKE, DEF_GLMLG_READ_RSP, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL) {
            p_rslt_info->rsp_result = ls_result;
        }
        return DEF_RET_NG;
    }

    /* エコー態管理ファイル更新 */
    ls_result = CSTE_update_gcest( p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_r401_edit                                  */
/*  CALLING SEQ.    : short  CSTE_r401_edit()                                */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : NW電文受信応答IPC(R401)編集処理                        */
/*****************************************************************************/
short  CSTE_r401_edit(t_rcv_info_def *p_rslt_info)
{
    short            ls_total_length;
    cr401_def       *lp_r401;
    cr401_def       *lp_c401;

    lp_r401   = (cr401_def *)g_resp_buf;
    lp_c401   = (cr401_def *)g_recv_buf;
/* ----------------------------------------------------------------- */
/* 応答電文エリアには既に受信電文データが設定してある状態。          */
/* この関数では、応答電文として編集が必要な箇所のみを設定する。      */
/* ----------------------------------------------------------------- */

    memcpy( lp_r401->common_header.interface_code
          , DEF_IPC_IFCD_NW_MSG_RSP     , strlen(DEF_IPC_IFCD_NW_MSG_RSP));

    switch(p_rslt_info->rsp_result) {
        case DEF_RSP_TYPE_NORMAL: /* 正常応答   */
            lp_r401->common_header.error_code = DEF_IPC_ERRCD_OK;

            if ( p_rslt_info->denbun_len == 0 ) {
                memcpy( lp_r401->control_info.response_kind
                      , DEF_CTLRSP_NOSEND , strlen(DEF_CTLRSP_NOSEND));
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_NORMAL;
            } else {
                memcpy( lp_r401->control_info.response_kind
                      , DEF_CTLRSP_SEND   , strlen(DEF_CTLRSP_SEND));
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;
            }
            break;

        case DEF_RSP_TYPE_KYOHI:  /* 拒否応答   */
            lp_r401->common_header.error_code = DEF_IPC_ERRCD_OK;

            if ( p_rslt_info->denbun_len == 0 ) {
                memcpy( lp_r401->control_info.response_kind
                      , DEF_CTLRSP_NOSEND , strlen(DEF_CTLRSP_NOSEND));
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_NORMAL;
            } else {
                memcpy( lp_r401->control_info.response_kind
                      , DEF_CTLRSP_SEND   , strlen(DEF_CTLRSP_SEND));
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
            }

            memcpy( lp_r401->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd
                  , sizeof(lp_r401->common_header.internal_error_code));
            break;

        case DEF_RSP_TYPE_HAKI:   /* 破棄       */
            lp_r401->common_header.error_code = DEF_IPC_ERRCD_OK;

            p_rslt_info->denbun_len = 0;
            memcpy( lp_r401->control_info.response_kind
                  , DEF_CTLRSP_NOSEND , strlen(DEF_CTLRSP_NOSEND));
            p_rslt_info->ctrl_type[3] = DEF_CTLINT_NORMAL;

            memcpy( lp_r401->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd
                  , sizeof(lp_r401->common_header.internal_error_code));
            break;

        case DEF_RSP_TYPE_OBST:   /* 障害電文 精査エラー系  */
            lp_r401->common_header.error_code = DEF_IPC_ERRCD_FAILMSG;

            p_rslt_info->denbun_len = 0;
            memcpy( lp_r401->control_info.response_kind
                  , DEF_CTLRSP_NOSEND , strlen(DEF_CTLRSP_NOSEND));
            p_rslt_info->ctrl_type[3] = DEF_CTLINT_NORMAL;

            memcpy( lp_r401->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd, DEF_NAIBU_ERR_CD_LEN);
            break;

        default:                  /* その他(エラー応答) */
            lp_r401->common_header.error_code = DEF_IPC_ERRCD_NG;

            p_rslt_info->denbun_len = 0;
            memcpy( lp_r401->control_info.response_kind
                  , DEF_CTLRSP_NOSEND , strlen(DEF_CTLRSP_NOSEND));
            p_rslt_info->ctrl_type[3] = DEF_CTLINT_NORMAL;

            memcpy( lp_r401->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd, DEF_NAIBU_ERR_CD_LEN);
            break;
    }

    lp_r401->common_header.control_data_length = (unsigned short)(sizeof(lp_r401->control_info)
                                               + p_rslt_info->denbun_len);

    memcpy(&lp_r401->control_info.control_kind
          , p_rslt_info->ctrl_type, sizeof(lp_r401->control_info.control_kind));

    lp_r401->control_info.denbun_log_key.denbun_shubetu = p_rslt_info->ctrl_type[2];

    memcpy( lp_r401->control_info.send_naibu_err_code
          , lp_c401->control_info.send_naibu_err_code
          , sizeof(lp_r401->control_info.send_naibu_err_code));

    if ( p_rslt_info->denbun_len != 0 ) {
        memcpy( lp_r401->control_info.mti
              , p_rslt_info->mti, sizeof(lp_r401->control_info.mti));
    }

    ls_total_length = (short)lp_r401->common_header.control_data_length
                    + sizeof(common_header_def);

    return ls_total_length;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_handle_req_msg_c402                        */
/*  CALLING SEQ.    : short  CSTE_handle_req_msg_c402()                      */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文作成要求処理                                   */
/*****************************************************************************/
void  CSTE_handle_req_msg_c402(t_rcv_info_def *p_rslt_info)
{
    short           ls_result;
    cr402_def      *lp_c402;

    lp_c402 = (cr402_def *)g_recv_buf;

    for(;;){

        /* ---------------------------------------------- */
        /* NW情報ファイル読込処理                         */
        /* ---------------------------------------------- */
        ls_result = CSTE_read_gfnwi( p_rslt_info );

        if ( ls_result != DEF_RET_OK ) {
            p_rslt_info->repl_data_len = 0;
            break;
        }
        /* ---------------------------------------------- */
        /* 接続先固有情報ファイル読込処理                 */
        /* ---------------------------------------------- */
        ls_result = CSTE_read_gfnws( p_rslt_info );

        if ( ls_result != DEF_RET_OK ) {
            p_rslt_info->repl_data_len = 0;
            break;
        }

        memcpy( p_rslt_info->ctrl_type, &lp_c402->control_info.control_kind
                                      , sizeof(lp_c402->control_info.control_kind));

        if (( lp_c402->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_CNT_OPN )||
            ( lp_c402->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_CNT_CLS ))  {
            /* 開閉局_仕向要求送信処理 */
            CSTE_sonof_sim_req_snd( p_rslt_info );

        } else {
            /* エコー_仕向要求送信処理 */
            CSTE_echo_sim_req_snd( p_rslt_info );

        }

        break;
    }

    /* 制御電文作成応答IPC(R402)編集処理 */
    p_rslt_info->repl_data_len = CSTE_r402_edit( p_rslt_info );

}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_sonof_sim_req_snd                          */
/*  CALLING SEQ.    : short  CSTE_sonof_sim_req_snd()                        */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開閉局_仕向要求送信処理                                */
/*****************************************************************************/
short  CSTE_sonof_sim_req_snd(t_rcv_info_def *p_rslt_info)
{
    short         ls_result;
    short         ls_ret_code;
    cr402_def    *lp_c402;
    cr402_def    *lp_r402;
    char          lc_wkbuf[10];
    long          ll_retry_cnt;
    short         update_flg = DEF_FLAG_OFF;
    char          ems_data[12];

    ls_ret_code = DEF_RET_OK;

    lp_c402     = (cr402_def *)g_recv_buf;
    lp_r402     = (cr402_def *)g_resp_buf;

    /* ---------------------------------------------- */
    /* 局状態管理ファイル読込み                       */
    /* ---------------------------------------------- */
    ls_ret_code = CSTE_read_gcsst( DEF_COM_IOM_LOCK, p_rslt_info );

    if ( ls_ret_code != DEF_RET_OK ) {
        return ls_ret_code;
    }
    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(コマンド)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_command( g_gcsst.state_sts_info.state_sts
                                         , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                                         , p_rslt_info );

    switch(ls_result) {
        case NWM_STE_CMD_OK_SEND:
            /* GFP内部LCN取得処理 */
            ls_ret_code = CSTE_get_lcn( p_rslt_info );

            if ( ls_ret_code != DEF_RET_OK ) {
                return ls_ret_code;
            }

            /* システム通番取得処理 */
            ls_ret_code = CSTE_get_sysnum( p_rslt_info );

            if ( ls_ret_code != DEF_RET_OK ) {
                return ls_ret_code;
            }

            /* 自動開局判定 */
            if (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) &&
                (( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_REQUEST ) ||
                 ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_CONNECT ))) {
                /* 開局リトライ回数更新 */
                memset( lc_wkbuf, 0x00, sizeof(lc_wkbuf) );
                memcpy( lc_wkbuf, g_gcsst.state_sts_info.open_state_retry_num, 
                    sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
                ll_retry_cnt = atol( lc_wkbuf ) + 1L;
                memset( lc_wkbuf, 0x00, sizeof(lc_wkbuf) );
                sprintf( lc_wkbuf, "%08ld", ll_retry_cnt );
                memcpy( g_gcsst.state_sts_info.open_state_retry_num, lc_wkbuf, 
                    sizeof( g_gcsst.state_sts_info.open_state_retry_num ));
            }

            if ( memcmp(p_rslt_info->new_stn_sts, DEF_CSTE_STS_SPACE, sizeof(p_rslt_info->new_stn_sts)) != 0 ) {
                /* EMS出力 */
                memset(ems_data, 0, sizeof(ems_data));

                /* EMS出力 局状態変更(開局処理中/閉局処理中) */
                if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {
                    /* 開局 */
                    /* 開局処理中メッセージ */
                    memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
                } else {
                    /* 閉局 */
                    /* 閉局処理中メッセージ */
                    memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
                }
                /* EMS出力 局状態変更(開局処理中/閉局処理中) */
                CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                                   , DEF_MSGTTKB_NORMAL
                                   , DEF_NERR_NOMAL
                                   , "@L@T@C"
                                   , p_rslt_info->lcn_no
                                   , &lp_c402->control_info.connection_lid
                                   , ems_data);

                /* 局状態管理ファイル更新 */
                ls_ret_code = CSTE_update_gcsst( p_rslt_info );

                if ( ls_ret_code != DEF_RET_OK ) {
                    return ls_ret_code;
                }
                update_flg = DEF_FLAG_ON;
            } else {
                /* 局状態管理ファイルUNLOCK */
                ls_ret_code = CSTE_unlock_gcsst( p_rslt_info );

                if ( ls_result != DEF_RET_OK ) {
                    return ls_ret_code;
                }
                update_flg = DEF_FLAG_OFF;
            }

            /* 開局・閉局・エコー要求電文編集 */
            NWM_STE_edit_reqmsg( (char *)&lp_r402->data_bu.message_text
                               , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                               , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                               , (gflin_pkey_def *)&lp_c402->control_info.connection_lid
                               ,&g_NWM_CTU_INI_arg_2
                               , p_rslt_info
                               ,&g_cg010in_modle
                               ,(ems_info_add*)&g_ems_add );

            /* 制御電文ログ出力処理 */
            ls_ret_code = CSTE_put_glmlg( DEF_SONF_S_RQ_SD, p_rslt_info );
            break;
        case NWM_STE_CMD_OK_NOSEND:
            /* 局状態管理ファイル更新 */
            if ( memcmp(p_rslt_info->new_stn_sts, DEF_CSTE_STS_SPACE, sizeof(p_rslt_info->new_stn_sts)) != 0 ) {
                ls_ret_code = CSTE_update_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_ON;
            } else {
                ls_ret_code = CSTE_unlock_gcsst( p_rslt_info );
                update_flg = DEF_FLAG_OFF;
            }
            break;
        case NWM_STE_CMD_NG:
            /* 局状態管理ファイルUNLOCK */
            ls_ret_code = CSTE_unlock_gcsst( p_rslt_info );
            p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
            break;
        default:
            /* 論理矛盾  後でメッセージ追加 */
            break;
    }

    if ((update_flg == DEF_FLAG_ON) &&
        (ls_ret_code != DEF_RET_OK)){
        /* EMS出力 局状態変更(更新後局状態) */
        memset(ems_data, 0, sizeof(ems_data));

        if ( memcmp(p_rslt_info->new_stn_sts,
                    DEF_STTE_STS_OPN,
                    sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局 */
            memcpy(ems_data, "ｶｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLS,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局 */
            memcpy(ems_data, "ﾍｲｷｮｸ      ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_OPNING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 開局処理中 */
            memcpy(ems_data, "ｶｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }
        else if ( memcmp(p_rslt_info->new_stn_sts,
                                      DEF_STTE_STS_CLOSING,
                                      sizeof(p_rslt_info->new_stn_sts)) == 0) {
            /* 閉局処理中 */
            memcpy(ems_data, "ﾍｲｷｮｸｼｮﾘﾁｭｳ", 11);
        }

        /* 局状態更新EMS出力 */
        if (ems_data[0] != 0){
            CMIN_message_output( DEF_EVT_KYOKU_STS_UPDATE
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_NOMAL
                               , "@L@T@C"
                               , p_rslt_info->lcn_no
                               , &lp_c402->control_info.connection_lid
                               , ems_data);
        }
    }

    return ls_ret_code;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_echo_sim_req_snd                           */
/*  CALLING SEQ.    : short  CSTE_echo_sim_req_snd()                         */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー_仕向要求送信処理                                */
/*****************************************************************************/
short  CSTE_echo_sim_req_snd(t_rcv_info_def *p_rslt_info)
{
    short ls_result;
    cr402_def    *lp_c402;
    cr402_def    *lp_r402;

    ls_result = DEF_RET_OK;

    lp_c402     = (cr402_def *)g_recv_buf;
    lp_r402     = (cr402_def *)g_resp_buf;

    /* ---------------------------------------------- */
    /* 局状態管理ファイル読込み                       */
    /* ---------------------------------------------- */
    ls_result = CSTE_read_gcsst( DEF_COM_IOM_NOLOCK, p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* ---------------------------------------------- */
    /* 開局・閉局・エコー局状態チェック(コマンド)     */
    /* ---------------------------------------------- */
    ls_result = NWM_STE_cst_check_command( g_gcsst.state_sts_info.state_sts
                                         , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                                         , p_rslt_info );

    switch(ls_result) {
        case NWM_STE_CMD_OK_SEND:
            /* GFP内部LCN取得処理 */
            ls_result = CSTE_get_lcn( p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                return ls_result;
            }

            /* システム通番取得処理 */
            ls_result = CSTE_get_sysnum( p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                return ls_result;
            }

            /* エコー状態管理ファイル更新 */
            ls_result = CSTE_update_gcest( p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                return ls_result;
            }

            /* 開局・閉局・エコー要求電文編集 */
            NWM_STE_edit_reqmsg( (char *)&lp_r402->data_bu.message_text
                               , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
                               , (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_NETWORK]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_INTERFACE]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_STATION]
                               , (char *)&g_db_gfnws_tbl[DEF_FNWS_IDX_CNNNECTION]
                               , (gflin_pkey_def *)&lp_c402->control_info.connection_lid
                               ,&g_NWM_CTU_INI_arg_2
                               , p_rslt_info
                               ,&g_cg010in_modle
                               ,(ems_info_add*)&g_ems_add );

            /* 制御電文ログ出力処理 */
            ls_result = CSTE_put_glmlg( DEF_ECHO_S_RQ_SD, p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                return ls_result;
            }
            break;
        case NWM_STE_CMD_OK_NOSEND:
            p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
            break;
        case NWM_STE_CMD_NG:
            p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
            break;
        default:
            /* 論理矛盾  後でメッセージ追加 */
            break;

    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_r402_edit                                  */
/*  CALLING SEQ.    : short  CSTE_r402_edit()                                */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文作成応答IPC(R402)編集処理                      */
/*****************************************************************************/
short  CSTE_r402_edit(t_rcv_info_def *p_rslt_info)
{
    short            ls_total_length;
    cr402_def       *lp_r402;

    lp_r402   = (cr402_def *)g_resp_buf;
/* ----------------------------------------------------------------- */
/* 応答電文エリアには既に受信電文データが設定してある状態。          */
/* この関数では、応答電文として編集が必要な箇所のみを設定する。      */
/* ----------------------------------------------------------------- */

    memcpy( lp_r402->common_header.interface_code
          , DEF_IPC_IFCD_CTRL_MSG_RSP, sizeof(lp_r402->common_header.interface_code));

    switch( p_rslt_info->rsp_result) {
        case DEF_RSP_TYPE_NORMAL: /* 正常応答   */
            lp_r402->common_header.error_code = DEF_IPC_ERRCD_OK;
            memcpy( lp_r402->common_header.internal_error_code
                  , DEF_NERR_NOMAL        , sizeof(lp_r402->common_header.internal_error_code));

            if ( p_rslt_info->denbun_len == 0 ) {
                memcpy( lp_r402->control_info.response_kind
                      , DEF_CTLRSP_NOSEND , sizeof(lp_r402->control_info.response_kind));
            } else {
                memcpy( lp_r402->control_info.response_kind
                      , DEF_CTLRSP_SEND   , sizeof(lp_r402->control_info.response_kind));
            }
            break;
        case DEF_RSP_TYPE_KYOHI:  /* 拒否応答   */
            lp_r402->common_header.error_code = DEF_IPC_ERRCD_NG;

            memcpy( lp_r402->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd
                  , sizeof(lp_r402->common_header.internal_error_code));
            if ( p_rslt_info->denbun_len == 0 ) {
                memcpy( lp_r402->control_info.response_kind
                      , DEF_CTLRSP_NOSEND , sizeof(lp_r402->control_info.response_kind));
            } else {
                p_rslt_info->ctrl_type[3]         = DEF_CTLINT_ALLOW;
                memcpy( lp_r402->control_info.response_kind
                      , DEF_CTLRSP_SEND   , sizeof(lp_r402->control_info.response_kind));
            }
            break;
        case DEF_RSP_TYPE_OBST:   /* 障害電文   */
            lp_r402->common_header.error_code = DEF_IPC_ERRCD_FAILMSG;

            memcpy( lp_r402->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd, sizeof(lp_r402->common_header.internal_error_code));
            break;
        default:                  /* その他(エラー応答、破棄) */
            lp_r402->common_header.error_code = DEF_IPC_ERRCD_NG;

            memcpy( lp_r402->common_header.internal_error_code
                  , p_rslt_info->naibu_errcd, sizeof(lp_r402->common_header.internal_error_code));

            break;
    }

    lp_r402->common_header.control_data_length = (unsigned short)(sizeof(lp_r402->control_info)
                                               + p_rslt_info->denbun_len);

    memcpy(&lp_r402->control_info.control_kind
          , p_rslt_info->ctrl_type, sizeof(lp_r402->control_info.control_kind));

    if (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )||
        ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ))  {
        if (g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF) {
            memset( lp_r402->control_info.connection_lid.station_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.station_name));
            memset( lp_r402->control_info.connection_lid.connection_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.connection_name));
        } else
        if (g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_ST) {
            memset( lp_r402->control_info.connection_lid.connection_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.connection_name));
        }
    } else {
        if (g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF) {
            memset( lp_r402->control_info.connection_lid.station_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.station_name));
            memset( lp_r402->control_info.connection_lid.connection_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.connection_name));
        } else
        if (g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr == DEF_OPN_CLS_MNG_LYR_ST) {
            memset( lp_r402->control_info.connection_lid.connection_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_r402->control_info.connection_lid.connection_name));
        }
    }

    memcpy(&lp_r402->control_info.denbun_log_key.tran_id.gfp_lcn
          , p_rslt_info->lcn_no   , sizeof(p_rslt_info->lcn_no));

    lp_r402->control_info.denbun_log_key.denbun_shubetu = p_rslt_info->ctrl_type[2];

    if ( p_rslt_info->denbun_len != 0 ) {
        memcpy( lp_r402->control_info.mti
              , p_rslt_info->mti, sizeof(lp_r402->control_info.mti));
    }

    ls_total_length = (short)lp_r402->common_header.control_data_length
                    + sizeof(common_header_def);

    return ls_total_length;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_is_auto_signon                             */
/*  CALLING SEQ.    : short  CSTE_is_auto_signon()                           */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : 0:自動開局以外  1:自動開局                             */
/*  DESCRIPTION     : 自動開局判定                                           */
/*****************************************************************************/
short   CSTE_is_auto_signon(void)
{
    cr401_def   *lp_c401;

    lp_c401     = (cr401_def *)g_recv_buf;
    if (( lp_c401->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_CNT_OPN ) &&
        (( lp_c401->control_info.control_kind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST ) ||
         ( lp_c401->control_info.control_kind.int_proc_kbn == DEF_CTLINT_AUTO_CONNECT ))) {
        return DEF_CSTE_OPEN_AUTO;
    } else {
        return DEF_CSTE_OPEN_NOAUTO;
    }
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_send_signon                                */
/*  CALLING SEQ.    : short  CSTE_send_signon()                              */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 開局コマンド送信処理                                   */
/*****************************************************************************/
void  CSTE_send_signon(t_rcv_info_def *p_rslt_info)
{
    short              ls_result;

    c501_def          *lp_c501;                    /* コマンド要求           */
    cr401_def         *lp_c401;                    /* NW受信電文             */
    COM_PSD_arg_1_def  l_com_psd_arg1;             /* PATHSENDモジュール引数 */
    COM_PSD_arg_2_def  l_com_psd_arg2;             /* PATHSENDモジュール引数 */
    COM_PSD_arg_3_def  l_com_psd_arg3;             /* PATHSENDモジュール引数 */
    COM_PSD_arg_4_def  l_com_psd_arg4;             /* PATHSENDモジュール引数 */

/* -------------------------------------------------- */
/* コマンド要求編集                                   */
/* -------------------------------------------------- */
    lp_c501 = (c501_def *)g_send_buf;
    lp_c401 = (cr401_def *)g_recv_buf;

    memset( g_send_buf, DEF_BUF_SPACE, sizeof(g_send_buf));

    /* 共通ヘッダ                     */
    memcpy( lp_c501->common_header.interface_code
          , DEF_IPC_IFCD_CMD_REQ, sizeof(lp_c501->common_header.interface_code));

    lp_c501->common_header.error_code = DEF_BUF_NULL;

    memset( lp_c501->common_header.internal_error_code
          , DEF_BUF_CZERO       , sizeof(lp_c501->common_header.internal_error_code));

    lp_c501->common_header.control_data_length = (unsigned short)( sizeof(lp_c501->command_info)
                                                                 + 2);

    /* コマンド情報                   */
    if ( g_son_retry == DEF_CSTE_RETRY_ON ) {
        if ( lp_c401->control_info.control_kind.int_proc_kbn == DEF_CTLINT_AUTO_REQUEST ) {
            memcpy( lp_c501->command_info.command_name
                  , DEF_IPC_CMD_CNT_OPN_AUT_REQ
                  , sizeof(lp_c501->command_info.command_name));
        } else {
            memcpy( lp_c501->command_info.command_name
                  , DEF_IPC_CMD_CNT_OPN_AUT_CON
                  , sizeof(lp_c501->command_info.command_name));
        }
    } else {
        memcpy( lp_c501->command_info.command_name
              , DEF_IPC_CMD_CNT_OPN_AUT_REQ
              , sizeof(lp_c501->command_info.command_name));
    }

    lp_c501->command_info.connection_logical_name.site_name
                                = lp_c401->control_info.connection_lid.site_name;
    lp_c501->command_info.connection_logical_name.nw_name
                                = lp_c401->control_info.connection_lid.nw_name;

    memcpy( lp_c501->command_info.connection_logical_name.group_name
          , lp_c401->control_info.connection_lid.group_name
          , sizeof(lp_c501->command_info.connection_logical_name.group_name));
    memcpy( lp_c501->command_info.connection_logical_name.interface_name
          , lp_c401->control_info.connection_lid.interface_name
          , sizeof(lp_c501->command_info.connection_logical_name.interface_name));

    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF ) {
        memset( lp_c501->command_info.connection_logical_name.station_name
              , DEF_BUF_SPACE
              , sizeof(lp_c501->command_info.connection_logical_name.station_name));
        memset( lp_c501->command_info.connection_logical_name.connection_name
              , DEF_BUF_SPACE
              , sizeof(lp_c501->command_info.connection_logical_name.connection_name));
    } else {
        memcpy( lp_c501->command_info.connection_logical_name.station_name
              , lp_c401->control_info.connection_lid.station_name
              , sizeof(lp_c501->command_info.connection_logical_name.station_name));

        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_ST ) {
            memset( lp_c501->command_info.connection_logical_name.connection_name
                  , DEF_BUF_SPACE
                  , sizeof(lp_c501->command_info.connection_logical_name.connection_name));
        } else {
            memcpy( lp_c501->command_info.connection_logical_name.connection_name
                  , lp_c401->control_info.connection_lid.connection_name
                  , sizeof(lp_c501->command_info.connection_logical_name.connection_name));
        }
    }

    memcpy( lp_c501->command_info.interface_ext_name
          , lp_c401->control_info.interface_name
          , sizeof(lp_c501->command_info.interface_ext_name));

    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF ) {
        memset( lp_c501->command_info.station_ext_name
              , DEF_BUF_SPACE
              , sizeof(lp_c501->command_info.station_ext_name));
    } else {
        memcpy( lp_c501->command_info.station_ext_name
              , lp_c401->control_info.station_name
              , sizeof(lp_c501->command_info.station_ext_name));
    }

    memset( lp_c501->command_info.srv_cls_id
          , DEF_BUF_SPACE
          , sizeof(lp_c501->command_info.srv_cls_id));

    lp_c501->record_count = DEF_BUF_NULL;

/* -------------------------------------------------- */
/* コマンドサーバPATHSEND情報編集                     */
/* -------------------------------------------------- */
    memset(&l_com_psd_arg1, DEF_BUF_NULL  , sizeof(l_com_psd_arg1));
    memset(&l_com_psd_arg2, DEF_BUF_NULL  , sizeof(l_com_psd_arg2));
    memset(&l_com_psd_arg3, DEF_BUF_NULL  , sizeof(l_com_psd_arg3));
    memset(&l_com_psd_arg4, DEF_BUF_SPACE , sizeof(l_com_psd_arg4));

    memset( l_com_psd_arg1.pathmon_name, ' ', sizeof(l_com_psd_arg1.pathmon_name) );
    memset( l_com_psd_arg1.serverclass_name, ' ', sizeof(l_com_psd_arg1.serverclass_name) );
    if ( g_kbt_svrcls.cmd_domain_name[0] != ' ' ) {
        memcpy( l_com_psd_arg1.pathmon_name    , g_kbt_svrcls.cmd_domain_name
                                               , sizeof(g_kbt_svrcls.cmd_domain_name));
    } else {
        memcpy( l_com_psd_arg1.pathmon_name    , g_kbt_svrcls.cmd_pathmon_name
                                               , sizeof(l_com_psd_arg1.pathmon_name));
    }
    memcpy( l_com_psd_arg1.serverclass_name, g_kbt_svrcls.cmd_srvcls_name
                                           , sizeof(g_kbt_svrcls.cmd_srvcls_name));
    l_com_psd_arg1.req_send_len    = (short)(lp_c501->common_header.control_data_length 
                                             + sizeof(common_header_def));
    memcpy( l_com_psd_arg1.msg_buf         , lp_c501
                                           , l_com_psd_arg1.req_send_len);
    l_com_psd_arg1.receive_max_len = sizeof(r501_def);
    l_com_psd_arg1.send_timer_msec = g_myinfo.send_timer;
    l_com_psd_arg1.retry_cnt       = (short)g_myinfo.send_retry_count;

    memcpy( l_com_psd_arg2.prog_id       , g_myinfo.prog_id
                                         , sizeof(l_com_psd_arg2.prog_id)   );
    memcpy( l_com_psd_arg4.srv_logical_id, g_myinfo.serverclass_name
                                         , sizeof(g_myinfo.serverclass_name));

/* -------------------------------------------------- */
/* PATHSEND共通処理実行                               */
/* -------------------------------------------------- */
    ls_result = COM_PSD( &l_com_psd_arg1
                       , &l_com_psd_arg2
                       , &l_com_psd_arg3
                       , &g_cg010in_modle
                       , &l_com_psd_arg4 );

    if ( ls_result != DEF_RET_OK ) {
        if ( g_kbt_svrcls.cmd_domain_name[0] != ' ' ) {
            CMIN_message_output( DEF_EVT_PSEND_ERR_DETECT
                               , DEF_MSGTTKB_GYOM_ERR
                               , DEF_NERR_CMD_HAKKO_ERR
                               , "@S@L@C@C@X"
                               , DEF_SC_CMD_SRV
                               , ""
                               , g_kbt_svrcls.cmd_domain_name
                               , g_kbt_svrcls.cmd_srvcls_name
                               , l_com_psd_arg3.guardian_errcode);
        } else {
            CMIN_message_output( DEF_EVT_PSEND_ERR_DETECT
                               , DEF_MSGTTKB_GYOM_ERR
                               , DEF_NERR_CMD_HAKKO_ERR
                               , "@S@L@C@C@X"
                               , DEF_SC_CMD_SRV
                               , ""
                               , g_kbt_svrcls.cmd_pathmon_name
                               , g_kbt_svrcls.cmd_srvcls_name
                               , l_com_psd_arg3.guardian_errcode);
        }
    }

}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_read_gfnwi                                 */
/*  CALLING SEQ.    : short  CSTE_read_gfnwi()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : NW情報ファイル読込処理                                 */
/*****************************************************************************/
short  CSTE_read_gfnwi(t_rcv_info_def *p_rslt_info)
{
    short                ls_result;
    t_gfnwi_pri_key_def  l_gfnwi_pkey;       /* NW情報ファイルプライマリKey  */
    cr401_def           *lp_c401;            /* NW電文                       */
    cr402_def           *lp_c402;            /* 制御電文                     */

    lp_c401 = (cr401_def *)g_recv_buf;
    lp_c402 = (cr402_def *)g_recv_buf;
/* ------------------------------------------------------ */
/* インターフェイス識別単位取得                           */
/* ------------------------------------------------------ */
    memset(&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE], DEF_BUF_SPACE, sizeof(db_gfnwi_def));
    memset(&l_gfnwi_pkey                       , DEF_BUF_SPACE, sizeof(l_gfnwi_pkey));
    /* プライマリKey設定 */
    if ( memcmp( lp_c401->common_header.interface_code
               , DEF_IPC_IFCD_NW_MSG_REQ, strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == 0 ) {
        l_gfnwi_pkey.site_id = lp_c401->control_info.connection_lid.site_name;
        l_gfnwi_pkey.nw_id   = lp_c401->control_info.connection_lid.nw_name;
        memcpy( l_gfnwi_pkey.grp_id    , lp_c401->control_info.connection_lid.group_name
                                       , sizeof(l_gfnwi_pkey.grp_id    ));
        memcpy( l_gfnwi_pkey.if_id     , lp_c401->control_info.connection_lid.interface_name
                                       , sizeof(l_gfnwi_pkey.if_id     ));
    } else {
        l_gfnwi_pkey.site_id = lp_c402->control_info.connection_lid.site_name;
        l_gfnwi_pkey.nw_id   = lp_c402->control_info.connection_lid.nw_name;
        memcpy( l_gfnwi_pkey.grp_id    , lp_c402->control_info.connection_lid.group_name
                                       , sizeof(l_gfnwi_pkey.grp_id    ));
        memcpy( l_gfnwi_pkey.if_id     , lp_c402->control_info.connection_lid.interface_name
                                       , sizeof(l_gfnwi_pkey.if_id     ));
    }
    memset( l_gfnwi_pkey.station_id, DEF_BUF_NO_SET     , sizeof(l_gfnwi_pkey.station_id));

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_NW_INFO
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_com_file_data.nw_file_name
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                        , sizeof(g_com_iom_arg_3.file_io_type));

    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GFNWI
                                        , strlen(DEF_GFNWI));
    memcpy( g_com_iom_arg_4.file_name   , g_com_file_data.nw_file_name
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_com_file_data.nw_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&l_gfnwi_pkey
                                        , sizeof(l_gfnwi_pkey));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(l_gfnwi_pkey);
    g_com_iom_arg_5.compare_len         = sizeof(l_gfnwi_pkey);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gfnwi_def);

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
        ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_NW_INFO
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    memcpy( (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE]
          , g_com_iom_arg_6.rec_area, sizeof(db_gfnwi_def));


    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_read_gfnws                                 */
/*  CALLING SEQ.    : short  CSTE_read_gfnws()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 接続先固有情報ファイル読込処理                         */
/*****************************************************************************/
short  CSTE_read_gfnws(t_rcv_info_def *p_rslt_info)
{
    short  ls_idx;
    short  ls_result;

    cr401_def        *lp_c401;               /* NW電文                       */
    cr402_def        *lp_c402;               /* 制御電文                     */
    gflin_pkey_def   *lp_con_lid;            /* コネクション論理ID           */

    lp_c401 = (cr401_def *)g_recv_buf;
    lp_c402 = (cr402_def *)g_recv_buf;

    if ( memcmp( lp_c401->common_header.interface_code
                , DEF_IPC_IFCD_NW_MSG_REQ, strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == 0 ) {
        lp_con_lid   = (gflin_pkey_def *)&lp_c401->control_info.connection_lid;
    } else {
        lp_con_lid   = (gflin_pkey_def *)&lp_c402->control_info.connection_lid;
    }

    for (ls_idx=1; ls_idx < DEF_FNWS_IDX_MAX;ls_idx++) {

        memset(&g_gfnws_pkey           , DEF_BUF_SPACE , sizeof(g_gfnws_pkey));

        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].gfnws_info.rec_unit[ls_idx] == DEF_CHR_FLG_ON ) {

            g_gfnws_pkey.nw_id  = lp_con_lid->nw_name;

            if ( ls_idx == DEF_FNWS_IDX_INTERFACE ) {
                memcpy( g_gfnws_pkey.if_id     , lp_con_lid->interface_name
                                               , sizeof(g_gfnws_pkey.if_id     ));
                memset( g_gfnws_pkey.station_id, DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.station_id));
                memset( g_gfnws_pkey.connect_id, DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.connect_id));
            } else
            if ( ls_idx == DEF_FNWS_IDX_STATION ) {
                memcpy( g_gfnws_pkey.if_id     , lp_con_lid->interface_name
                                               , sizeof(g_gfnws_pkey.if_id     ));
                memcpy( g_gfnws_pkey.station_id, lp_con_lid->station_name
                                               , sizeof(g_gfnws_pkey.station_id));
                memset( g_gfnws_pkey.connect_id, DEF_BUF_NO_SET, sizeof(g_gfnws_pkey.connect_id));
            } else
            if ( ls_idx == DEF_FNWS_IDX_CNNNECTION ) {
                memcpy( g_gfnws_pkey.if_id     , lp_con_lid->interface_name
                                               , sizeof(g_gfnws_pkey.if_id     ));
                memcpy( g_gfnws_pkey.station_id, lp_con_lid->station_name
                                               , sizeof(g_gfnws_pkey.station_id));
                memcpy( g_gfnws_pkey.connect_id, lp_con_lid->connection_name
                                               , sizeof(g_gfnws_pkey.connect_id));
            }

            ls_result = CSTE_read_gfnws_io( p_rslt_info );

            if ( ls_result != DEF_RET_OK ) {
                return DEF_RET_NG;
            }

            memcpy( (char *)&g_db_gfnws_tbl[ls_idx], g_com_iom_arg_6.rec_area, sizeof(db_gfnws_def));
        }
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_read_gfnws_io                              */
/*  CALLING SEQ.    : short  CSTE_read_gfnws_io()                            */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 接続先固有情報ファイルI/O処理                          */
/*****************************************************************************/
short  CSTE_read_gfnws_io(t_rcv_info_def *p_rslt_info)
{
    short ls_result;

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_NW_KOYU_INFO
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.cnct_inf_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GFNWS
                                        , strlen(DEF_GFNWS));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.cnct_inf_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.cnct_inf_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gfnws_pkey
                                        , sizeof(g_gfnws_pkey));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gfnws_pkey);
    g_com_iom_arg_5.compare_len         = sizeof(g_gfnws_pkey);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gfnws_def);

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
        ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_CNCT_INFO
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_read_gcsst                                 */
/*  CALLING SEQ.    : short  CSTE_read_gcsst()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 局状態管理ファイル取得処理                             */
/*****************************************************************************/
short  CSTE_read_gcsst(short p_lock_type, t_rcv_info_def *p_rslt_info)
{
    short           ls_result;
    cr401_def      *lp_c401;                 /* NW電文                       */
    cr402_def      *lp_c402;                 /* 制御電文                     */
    gflin_pkey_def *lp_con_lid;              /* コネクション論理ID           */

    lp_c401 = (cr401_def *)&g_recv_buf;
    lp_c402 = (cr402_def *)&g_recv_buf;

    if ( memcmp( lp_c401->common_header.interface_code
                , DEF_IPC_IFCD_NW_MSG_REQ, strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == 0 ) {
        lp_con_lid   = (gflin_pkey_def *)&lp_c401->control_info.connection_lid;
    } else {
        lp_con_lid   = (gflin_pkey_def *)&lp_c402->control_info.connection_lid;
    }

    memset(&g_gcsst.pri_key, DEF_BUF_SPACE, sizeof(g_gcsst.pri_key));

    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF ) {
        g_gcsst.pri_key.site_id = lp_con_lid->site_name;
        g_gcsst.pri_key.nw_id   = lp_con_lid->nw_name;
        memcpy( g_gcsst.pri_key.grp_id    , lp_con_lid->group_name
                                          , sizeof(g_gcsst.pri_key.grp_id));
        memcpy( g_gcsst.pri_key.if_id     , lp_con_lid->interface_name
                                          , sizeof(g_gcsst.pri_key.if_id));
    } else
    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_ST ) {
        g_gcsst.pri_key.site_id = lp_con_lid->site_name;
        g_gcsst.pri_key.nw_id   = lp_con_lid->nw_name;
        memcpy( g_gcsst.pri_key.grp_id    , lp_con_lid->group_name
                                          , sizeof(g_gcsst.pri_key.grp_id));
        memcpy( g_gcsst.pri_key.if_id     , lp_con_lid->interface_name
                                          , sizeof(g_gcsst.pri_key.if_id));
        memcpy( g_gcsst.pri_key.station_id, lp_con_lid->station_name
                                          , sizeof(g_gcsst.pri_key.station_id));
    } else
    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_CO ) {
        g_gcsst.pri_key.site_id = lp_con_lid->site_name;
        g_gcsst.pri_key.nw_id   = lp_con_lid->nw_name;
        memcpy( g_gcsst.pri_key.grp_id    , lp_con_lid->group_name
                                          , sizeof(g_gcsst.pri_key.grp_id));
        memcpy( g_gcsst.pri_key.if_id     , lp_con_lid->interface_name
                                          , sizeof(g_gcsst.pri_key.if_id));
        memcpy( g_gcsst.pri_key.station_id, lp_con_lid->station_name
                                          , sizeof(g_gcsst.pri_key.station_id));
        memcpy( g_gcsst.pri_key.connect_id, lp_con_lid->connection_name
                                          , sizeof(g_gcsst.pri_key.connect_id));
    }

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CEN_STS
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GCSST
                                        , strlen(DEF_GCSST));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.stan_sts_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gcsst.pri_key
                                        , sizeof(g_gcsst.pri_key));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.compare_len         = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = p_lock_type;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gcsst_def);

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
        ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    memcpy( (char *)&g_gcsst, g_com_iom_arg_6.rec_area, sizeof(db_gcsst_def));

    return DEF_RET_OK;
} /* CSTE_read_gcsst */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_update_gcsst                               */
/*  CALLING SEQ.    : short  CSTE_update_gcsst()                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 局状態管理ファイル更新処理                             */
/*****************************************************************************/
short  CSTE_update_gcsst(t_rcv_info_def *p_rslt_info)
{
    short         ls_result;
    long long     ll_datetime;
    db_gcsst_def  db_gcsst_local;

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CEN_STS
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_UPDATE
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GCSST
                                        , strlen(DEF_GCSST));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.stan_sts_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gcsst.pri_key
                                        , sizeof(g_gcsst.pri_key));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.compare_len         = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gcsst_def);

    if (( memcmp(p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
        ( memcmp(p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
        ( memcmp(p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 )||
        ( memcmp(p_rslt_info->new_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
        memcpy( g_gcsst.state_sts_info.state_sts, p_rslt_info->new_stn_sts
                                                , sizeof(g_gcsst.state_sts_info.state_sts));

        COM_SDT ( DEF_COM_SDT_arg1_jpn, &g_com_sdt_arg_2, &g_com_sdt_arg_3, &ll_datetime);
        memcpy  ( g_gcsst.state_sts_info.state_sts_update_time
                , (char *)&g_com_sdt_arg_2, sizeof(g_gcsst.state_sts_info.state_sts_update_time));
    }

    memcpy( g_com_iom_arg_5.rec_area, (char *)&g_gcsst, sizeof(db_gcsst_def));

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_UPDATE
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_IF ) {
        /* 局状態管理　他サイトレコードの更新処理 */
        memcpy( (char *)&db_gcsst_local.pri_key
              , (char *)&g_gcsst.pri_key
              , sizeof(db_gcsst_local.pri_key));

        if ( g_gcsst.pri_key.site_id == DEF_SITE_ID_TKY ) {
            db_gcsst_local.pri_key.site_id = DEF_SITE_ID_OSK;
        }
        else {
            db_gcsst_local.pri_key.site_id = DEF_SITE_ID_TKY;
        }

        /* READLOCK処理 */
        /* IOモジュールパラメータ初期化 */
        memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
        memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
        memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
        memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
        memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

        /* トレース情報設定 */
        memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                            , sizeof(g_com_iom_arg_3.prog_id     ));
        memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CEN_STS
                                            , sizeof(g_com_iom_arg_3.file_id     ));
        memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.stan_sts_fname
                                            , sizeof(g_com_iom_arg_3.file_name   ));
        memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                            , sizeof(g_com_iom_arg_3.file_io_type));
        /* ファイル情報設定 */
        memcpy( g_com_iom_arg_4.file_id     , DEF_GCSST
                                            , strlen(DEF_GCSST));
        memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.stan_sts_fname
                                            , sizeof(g_com_iom_arg_4.file_name));
        g_com_iom_arg_4.file_no             = g_kbt_file_data.stan_sts_fno;
        /* 入力情報 */
        g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
        g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
        g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
        memcpy( g_com_iom_arg_5.key_value   , (char *)&db_gcsst_local.pri_key
                                            , sizeof(db_gcsst_local.pri_key));
        memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                            , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
        g_com_iom_arg_5.key_len             = sizeof(db_gcsst_local.pri_key);
        g_com_iom_arg_5.compare_len         = sizeof(db_gcsst_local.pri_key);
        g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
        g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCK;
        g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
        g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
        g_com_iom_arg_5.rec_len             = sizeof(db_gcsst_def);

        ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                            , g_ch_sub_prog_sts
                            ,&g_com_iom_arg_3
                            ,&g_com_iom_arg_4
                            ,&g_com_iom_arg_5
                            ,&g_com_iom_arg_6);

        if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
            ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@C@C@C@C@C@U"
                                , ""
                                , ""
                                , DEF_FL_CEN_STS
                                , DEF_COM_IOM_FUNC_STARTREAD
                                , g_com_iom_arg_5.key_value
                                , g_com_iom_arg_6.guardian_errcode );
            p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
            return DEF_RET_NG;
        }

        memcpy( (char *)&db_gcsst_local, g_com_iom_arg_6.rec_area, sizeof(db_gcsst_local));

        /* 局状態情報を自サイトと同じ内容に更新 */
        memcpy( (char *)&db_gcsst_local.state_sts_info
              , (char *)&g_gcsst.state_sts_info
              , sizeof(db_gcsst_local.state_sts_info));

        /* UPDATE処理 */
        /* IOモジュールパラメータ初期化 */
        memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
        memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
        memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
        memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
        memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

        /* トレース情報設定 */
        memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                            , sizeof(g_com_iom_arg_3.prog_id     ));
        memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CEN_STS
                                            , sizeof(g_com_iom_arg_3.file_id     ));
        memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.stan_sts_fname
                                            , sizeof(g_com_iom_arg_3.file_name   ));
        memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_UPDATE
                                            , sizeof(g_com_iom_arg_3.file_io_type));
        /* ファイル情報設定 */
        memcpy( g_com_iom_arg_4.file_id     , DEF_GCSST
                                            , strlen(DEF_GCSST));
        memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.stan_sts_fname
                                            , sizeof(g_com_iom_arg_4.file_name));
        g_com_iom_arg_4.file_no             = g_kbt_file_data.stan_sts_fno;
        /* 入力情報 */
        g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
        g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
        g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
        memcpy( g_com_iom_arg_5.key_value   , (char *)&db_gcsst_local.pri_key
                                            , sizeof(db_gcsst_local.pri_key));
        memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                            , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
        g_com_iom_arg_5.key_len             = sizeof(db_gcsst_local.pri_key);
        g_com_iom_arg_5.compare_len         = sizeof(db_gcsst_local.pri_key);
        g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
        g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCKFREE;
        g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
        g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
        g_com_iom_arg_5.rec_len             = sizeof(db_gcsst_def);

        memcpy( g_com_iom_arg_5.rec_area, (char *)&db_gcsst_local, sizeof(db_gcsst_def));

        ls_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                            , g_ch_sub_prog_sts
                            ,&g_com_iom_arg_3
                            ,&g_com_iom_arg_4
                            ,&g_com_iom_arg_5
                            ,&g_com_iom_arg_6);

        if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
            CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                                , DEF_MSGTTKB_GYOM_ERR
                                , DEF_NERR_FILE_IO_ERR
                                , "@C@C@C@C@C@U"
                                , ""
                                , ""
                                , DEF_FL_CEN_STS
                                , DEF_COM_IOM_FUNC_UPDATE
                                , g_com_iom_arg_5.key_value
                                , g_com_iom_arg_6.guardian_errcode );
            p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR, strlen(DEF_NERR_FILE_IO_ERR));
            return DEF_RET_NG;
        }
    }

    return DEF_RET_OK;
} 

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_unlock_gcsst                               */
/*  CALLING SEQ.    : short  CSTE_unlock_gcsst()                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 局状態管理ファイルUNLOCK処理                           */
/*****************************************************************************/
short  CSTE_unlock_gcsst(t_rcv_info_def *p_rslt_info)
{
    short         ls_result;

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_CEN_STS
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_UNLOCKREC
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GCSST
                                        , strlen(DEF_GCSST));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.stan_sts_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.stan_sts_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gcsst.pri_key
                                        , sizeof(g_gcsst.pri_key));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.compare_len         = sizeof(g_gcsst.pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gcsst_def);
    memcpy( g_com_iom_arg_5.rec_area, (char *)&g_gcsst, sizeof(db_gcsst_def));

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_UNLOC
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_UNLOC
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_update_gcest                               */
/*  CALLING SEQ.    : short  CSTE_update_gcest()                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エコー状態管理ファイル更新処理                         */
/*****************************************************************************/
short  CSTE_update_gcest(t_rcv_info_def *p_rslt_info)
{
    short              ls_result;
    long long          ll_datetime;
    cr401_def         *lp_rcvif_401;
    cr402_def         *lp_rcvif_402;
    common_header_def *lp_cmn_head;

/* NW情報ファイルのエコーテスト管理単位でKeyを確定させる。            */
/* 確定したKey情報を受信電文の制御情報から設定、対象レコードをREAD    */
/* チェック結果を設定して更新                                         */
    memset(&g_gcest     , DEF_BUF_SPACE, sizeof(g_gcest)     );
    memset(&g_gcest_pkey, DEF_BUF_SPACE, sizeof(g_gcest_pkey));

    lp_cmn_head = (common_header_def *)g_recv_buf;

    if ( memcmp( lp_cmn_head->interface_code, DEF_IPC_IFCD_NW_MSG_REQ
                                            , strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == DEF_RET_OK) {
        lp_rcvif_401 = (cr401_def *)g_recv_buf;

        g_gcest_pkey.site_id = lp_rcvif_401->control_info.connection_lid.site_name;
        g_gcest_pkey.nw_id   = lp_rcvif_401->control_info.connection_lid.nw_name;
        memcpy( g_gcest_pkey.gp_id
              , lp_rcvif_401->control_info.connection_lid.group_name
              , sizeof(g_gcest_pkey.gp_id));
        memcpy( g_gcest_pkey.if_id
              , lp_rcvif_401->control_info.connection_lid.interface_name
              , sizeof(g_gcest_pkey.if_id));

        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr
                                                      == DEF_ECHO_TEST_MNG_LYR_ST ) {
            memcpy( g_gcest_pkey.st_id
                  , lp_rcvif_401->control_info.connection_lid.station_name
                  , sizeof(g_gcest_pkey.st_id));
        } else
        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr
                                                      == DEF_ECHO_TEST_MNG_LYR_CO ) {
            memcpy( g_gcest_pkey.st_id
                  , lp_rcvif_401->control_info.connection_lid.station_name
                  , sizeof(g_gcest_pkey.st_id));
            memcpy( g_gcest_pkey.cn_id
                  , lp_rcvif_401->control_info.connection_lid.connection_name
                  , sizeof(g_gcest_pkey.cn_id));
        }
    } else
    if ( memcmp( lp_cmn_head->interface_code, DEF_IPC_IFCD_CTRL_MSG_REQ
                                            , strlen(DEF_IPC_IFCD_CTRL_MSG_REQ)) == DEF_RET_OK) {
        lp_rcvif_402 = (cr402_def *)g_recv_buf;

        g_gcest_pkey.site_id = lp_rcvif_402->control_info.connection_lid.site_name;
        g_gcest_pkey.nw_id   = lp_rcvif_402->control_info.connection_lid.nw_name;
        memcpy( g_gcest_pkey.gp_id
              , lp_rcvif_402->control_info.connection_lid.group_name
              , sizeof(g_gcest_pkey.gp_id));
        memcpy( g_gcest_pkey.if_id
              , lp_rcvif_402->control_info.connection_lid.interface_name
              , sizeof(g_gcest_pkey.if_id));

        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr
                                                      == DEF_ECHO_TEST_MNG_LYR_ST ) {
            memcpy( g_gcest_pkey.st_id
                  , lp_rcvif_402->control_info.connection_lid.station_name
                  , sizeof(g_gcest_pkey.st_id));
        } else
        if ( g_gfnwi_tbl[DEF_FNWI_IDX_SITE].mng_lyr_info.echo_test_mng_lyr
                                                      == DEF_ECHO_TEST_MNG_LYR_CO ) {
            memcpy( g_gcest_pkey.st_id
                  , lp_rcvif_402->control_info.connection_lid.station_name
                  , sizeof(g_gcest_pkey.st_id));
            memcpy( g_gcest_pkey.cn_id
                  , lp_rcvif_402->control_info.connection_lid.connection_name
                  , sizeof(g_gcest_pkey.cn_id));
        }
    }
/* ---------------------------------------------- */
/* エコー状態管理ファイルREAD                     */
/* ---------------------------------------------- */

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_ECH_STS
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.echo_mng_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GCEST
                                        , strlen(DEF_GCEST));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.echo_mng_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.echo_mng_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gcest_pkey
                                        , sizeof(g_gcest_pkey));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gcest_pkey);
    g_com_iom_arg_5.compare_len         = sizeof(g_gcest_pkey);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCK;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(g_gcest);

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
        ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_ECH_STS
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    memcpy( (char *)&g_gcest, g_com_iom_arg_6.rec_area, sizeof(db_gcest_def));

/* ---------------------------------------------- */
/* エコー状態管理データ編集                       */
/* ---------------------------------------------- */
    ls_result = COM_SDT( DEF_COM_SDT_arg1_jpn
                       ,&g_com_sdt_arg_2
                       ,&g_com_sdt_arg_3
                       ,&ll_datetime );

    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR             /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C"
                            , "COM_SDT" );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_PROC_OPN_ERR
                                        , strlen(DEF_NERR_PROC_OPN_ERR));
        return DEF_RET_NG;
    }

    /* C401受信 */
    if ( memcmp( lp_cmn_head->interface_code, DEF_IPC_IFCD_NW_MSG_REQ
                                            , strlen(DEF_IPC_IFCD_NW_MSG_REQ)) == DEF_RET_OK) {
        if ( memcmp( lp_rcvif_401->control_info.request_kind             /* 被仕向要求受信      */
                   , DEF_CTLREQ_HISIMUKE, strlen(DEF_CTLREQ_HISIMUKE)) == 0 ) {
            if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL ) {  /* 許可 */
                memcpy( g_gcest.echo_info.dst_echo_info.last_echo_result
                      , DEF_LAST_ECHO_OK
                      , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_result));
                memcpy( g_gcest.echo_info.dst_echo_info.last_echo_ok_time
                      , (char *)&g_com_sdt_arg_2
                      , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_ok_time));
            } else {                                      /* 拒否 */
                memcpy( g_gcest.echo_info.dst_echo_info.last_echo_result
                      , DEF_LAST_ECHO_KYOHI_SEND
                      , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_result));
            }
            memcpy( g_gcest.echo_info.dst_echo_info.last_echo_req_recv_time
                  , (char *)&g_com_sdt_arg_2
                  , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_req_recv_time));
        } else 
        if ( memcmp( lp_rcvif_401->control_info.request_kind            /* 仕向応答受信        */
                   , DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == 0 ) {
            if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL ) {  /* 許可 */
                memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                      , DEF_LAST_ECHO_OK
                      , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
                memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_ok_time
                      , (char *)&g_com_sdt_arg_2
                      , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_ok_time));
            } else {                                      /* 拒否 */
                memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                      , DEF_LAST_ECHO_KYOHI_RCV
                      , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
            }
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_end_time
                  , (char *)&g_com_sdt_arg_2
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_end_time));
        } else 
        if ( memcmp( lp_rcvif_401->control_info.request_kind        /* 仕向応答Timeout    */
                   , DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT)) == 0 ) {
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                  , DEF_LAST_ECHO_TIMEOUT
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_end_time
                  , (char *)&g_com_sdt_arg_2
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_end_time));
        } else 
        if ( memcmp( lp_rcvif_401->control_info.request_kind        /* 仕向要求送信不可   */
                   , DEF_CTLREQ_SIMUKE_ERROR, strlen(DEF_CTLREQ_SIMUKE_ERROR)) == 0 ) {
                memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                      , DEF_LAST_ECHO_SEND_ERR
                      , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
        } else 
        if ( memcmp( lp_rcvif_401->control_info.request_kind        /* 被仕向要求送信不可 */
                   , DEF_CTLREQ_HISIMUKE_ERROR, strlen(DEF_CTLREQ_HISIMUKE_ERROR)) == 0 ) {
                memcpy( g_gcest.echo_info.dst_echo_info.last_echo_result
                      , DEF_LAST_ECHO_SEND_ERR
                      , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_result));
                memcpy( g_gcest.echo_info.dst_echo_info.last_echo_req_recv_time
                      , (char *)&g_com_sdt_arg_2
                      , sizeof(g_gcest.echo_info.dst_echo_info.last_echo_req_recv_time));
        }
    } else { /* c402 */
        if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL ) {  /* 許可 */
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                  , DEF_LAST_ECHO_OK
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_start_time
                  , (char *)&g_com_sdt_arg_2
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_start_time));
        } else {                                      /* 拒否 */
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_start_time
                  , (char *)&g_com_sdt_arg_2
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_start_time));
            memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_result
                  , DEF_LAST_ECHO_KYOHI_RCV
                  , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_result));
        }
        memcpy( g_gcest.echo_info.cbs_echo_info.last_echo_end_time
              , (char *)&g_com_sdt_arg_2
              , sizeof(g_gcest.echo_info.cbs_echo_info.last_echo_end_time));
    }
/* ---------------------------------------------- */
/* エコー状態管理ファイルUPDATE                   */
/* ---------------------------------------------- */
    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* トレース情報設定 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_ECH_STS
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_kbt_file_data.echo_mng_fname
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_UPDATE
                                        , sizeof(g_com_iom_arg_3.file_io_type));
    /* ファイル情報設定 */
    memcpy( g_com_iom_arg_4.file_id     , DEF_GCEST
                                        , strlen(DEF_GCEST));
    memcpy( g_com_iom_arg_4.file_name   , g_kbt_file_data.echo_mng_fname
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_kbt_file_data.echo_mng_fno;
    /* 入力情報 */
    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , (char *)&g_gcest.pri_key
                                        , sizeof(g_gcest.pri_key));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len             = sizeof(g_gcest.pri_key);
    g_com_iom_arg_5.compare_len         = sizeof(g_gcest.pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gcest_def);
    memcpy( g_com_iom_arg_5.rec_area, (char *)&g_gcest, sizeof(db_gcest_def));

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                        , g_ch_sub_prog_sts
                        ,&g_com_iom_arg_3
                        ,&g_com_iom_arg_4
                        ,&g_com_iom_arg_5
                        ,&g_com_iom_arg_6);

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_ECH_STS
                            , DEF_COM_IOM_FUNC_UPDATE
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} 

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_put_glmlg                                  */
/*  CALLING SEQ.    : short  CSTE_put_glmlg()                                */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログ出力処理                                   */
/*****************************************************************************/
short  CSTE_put_glmlg(short p_data_type, t_rcv_info_def *p_rslt_info)
{
    short         ls_result;
    short         ls_data_len;
    long long     ll_datetime;

    short         ls_guardian_error;
    char          lc_wbuf[40];

    cr401_def    *lp_c401;
    cr401_def    *lp_r401;
    cr402_def    *lp_r402;

    memset(&g_glmlg, DEF_BUF_SPACE, sizeof(db_glmlg_def));
                                                     /* 401 受信電文の登録             */
    if ( p_data_type == DEF_SONF_H_RQ_RV         ||  /* 開閉局_被仕向要求受信          */
         p_data_type == DEF_ECHO_H_RQ_RV         ||  /* エコー_被仕向要求受信          */
         p_data_type == DEF_SONF_S_RP_RV         ||  /* 開閉局_仕向応答受信            */
         p_data_type == DEF_ECHO_S_RP_RV         ||  /* エコー_仕向応答受信            */
         p_data_type == DEF_SONF_S_RP_TIMEOUT_RV ||  /* 開閉局_仕向応答待ちTimeout受信 */
         p_data_type == DEF_ECHO_S_RP_TIMEOUT_RV ) { /* エコー_仕向応答待ちTimeout受信 */
        /* 受信エリアのアドレスセット */
        lp_c401 = (cr401_def *)g_recv_buf;

        /* Key情報セット */
        g_glmlg.pri_key.part_id[0] = '0';
        g_glmlg.pri_key.part_id[1] = lp_c401->control_info.req_gfp_lcn[14];
        memcpy ( g_glmlg.pri_key.lcn_id , lp_c401->control_info.req_gfp_lcn
                                        , sizeof(g_glmlg.pri_key.lcn_id));

        if ( p_data_type == DEF_SONF_H_RQ_RV ||         /* 開閉局_被仕向要求受信 */
             p_data_type == DEF_ECHO_H_RQ_RV   ) {      /* エコー_被仕向要求受信 */
            g_glmlg.pri_key.s_h_kubun    = DEF_S_H_KUBUN_HISIMUKE;
            g_glmlg.pri_key.send_recv_id = DEF_REQ_RCV;
        } else {
            g_glmlg.pri_key.s_h_kubun    = DEF_S_H_KUBUN_SIMUKE;
            g_glmlg.pri_key.send_recv_id = DEF_RSP_RCV;
        }

        /* コネクション論理IDセット */
        g_glmlg.connect_id.site_id   = lp_c401->control_info.connection_lid.site_name;
        g_glmlg.connect_id.nw_id     = lp_c401->control_info.connection_lid.nw_name;
        memcpy( g_glmlg.connect_id.grp_id    , lp_c401->control_info.connection_lid.group_name
                                             , sizeof(g_glmlg.connect_id.grp_id));
        memcpy( g_glmlg.connect_id.if_id     , lp_c401->control_info.connection_lid.interface_name
                                             , sizeof(g_glmlg.connect_id.if_id));
        memcpy( g_glmlg.connect_id.station_id, lp_c401->control_info.connection_lid.station_name
                                             , sizeof(g_glmlg.connect_id.station_id));
        memcpy( g_glmlg.connect_id.connect_id, lp_c401->control_info.connection_lid.connection_name
                                             , sizeof(g_glmlg.connect_id.connect_id));

        memcpy( g_glmlg.mti_id               , lp_c401->control_info.mti
                                             , sizeof(g_glmlg.mti_id));
        memcpy( g_glmlg.control_kind         ,&lp_c401->control_info.control_kind
                                             , sizeof(g_glmlg.control_kind));

        memcpy( g_glmlg.naibu_err_code       , p_rslt_info->naibu_errcd
                                             , sizeof(g_glmlg.naibu_err_code));
        memcpy( g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id
              , g_myinfo.serverclass_name
              , sizeof(g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id));

        if ( p_data_type == DEF_SONF_H_RQ_RV ||  /* 開閉局_被仕向要求受信 */
             p_data_type == DEF_ECHO_H_RQ_RV ||  /* エコー_被仕向要求受信 */
             p_data_type == DEF_SONF_S_RP_RV ||  /* 開閉局_仕向応答受信   */
             p_data_type == DEF_ECHO_S_RP_RV ) { /* エコー_仕向応答受信   */
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_ON;

            ls_data_len = (short)lp_c401->common_header.control_data_length
                        - sizeof(lp_c401->control_info);
            sprintf( g_glmlg.denbun_area.denbun_len, "%05d", ls_data_len);
            memcpy( g_glmlg.denbun_area.denbun    , lp_c401->data_bu.message_text
                                                  , ls_data_len);
        } else {
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_OFF;
            memset( g_glmlg.denbun_area.denbun_len, DEF_BUF_CZERO
                                                  , sizeof(g_glmlg.denbun_area.denbun_len));
        }

        memcpy( g_glmlg.control_kind          , p_rslt_info->ctrl_type
                                              , sizeof(g_glmlg.control_kind));
    } else                                           /* 401 送信電文の登録             */
    if ( p_data_type == DEF_SONF_H_RQ_SD ||          /* 開閉局_被仕向応答送信          */
         p_data_type == DEF_ECHO_H_RQ_SD  ) {        /* エコー_被仕向応答送信          */
        /* 送信エリアのアドレスセット */
        lp_r401 = (cr401_def *)g_resp_buf;
        lp_c401 = (cr401_def *)g_recv_buf;

        g_glmlg.pri_key.part_id[0] = '0';
        g_glmlg.pri_key.part_id[1] = lp_c401->control_info.req_gfp_lcn[14];
        memcpy ( g_glmlg.pri_key.lcn_id , lp_c401->control_info.req_gfp_lcn
                                        , sizeof(g_glmlg.pri_key.lcn_id));

        g_glmlg.pri_key.s_h_kubun    = DEF_S_H_KUBUN_HISIMUKE;
        g_glmlg.pri_key.send_recv_id = DEF_RSP_SEND;

        /* コネクション論理IDセット */
        g_glmlg.connect_id.site_id   = lp_r401->control_info.connection_lid.site_name;
        g_glmlg.connect_id.nw_id     = lp_r401->control_info.connection_lid.nw_name;
        memcpy( g_glmlg.connect_id.grp_id    , lp_r401->control_info.connection_lid.group_name
                                             , sizeof(g_glmlg.connect_id.grp_id));
        memcpy( g_glmlg.connect_id.if_id     , lp_r401->control_info.connection_lid.interface_name
                                             , sizeof(g_glmlg.connect_id.if_id));
        memcpy( g_glmlg.connect_id.station_id, lp_r401->control_info.connection_lid.station_name
                                             , sizeof(g_glmlg.connect_id.station_id));
        memcpy( g_glmlg.connect_id.connect_id, lp_r401->control_info.connection_lid.connection_name
                                             , sizeof(g_glmlg.connect_id.connect_id));

        memcpy( g_glmlg.mti_id               , p_rslt_info->mti
                                             , sizeof(g_glmlg.mti_id));

        if ( p_rslt_info->denbun_len != 0 ) {
            if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_NORMAL ) {
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;
            } else
            if ( p_rslt_info->rsp_result == DEF_RSP_TYPE_KYOHI ) {
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
            }
        }
        memcpy( g_glmlg.control_kind         , p_rslt_info->ctrl_type
                                             , sizeof(g_glmlg.control_kind));
        memcpy( g_glmlg.naibu_err_code       , p_rslt_info->naibu_errcd
                                             , sizeof(g_glmlg.naibu_err_code));
        memcpy( g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id
              , g_myinfo.serverclass_name
              , sizeof(g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id));
//      memcpy( g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num
//            , "0000"
//            , sizeof(g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num));

         if ( p_rslt_info->denbun_len != 0 ) {
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_ON;
            sprintf( g_glmlg.denbun_area.denbun_len, "%05d", p_rslt_info->denbun_len);
            memcpy ( g_glmlg.denbun_area.denbun    , lp_r401->data_bu.message_text
                                                   , p_rslt_info->denbun_len);
        } else {
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_OFF;
            memset( g_glmlg.denbun_area.denbun_len, DEF_BUF_CZERO
                                                  , sizeof(g_glmlg.denbun_area.denbun_len));
        }
    } else                                           /* 402 送信電文の登録             */
    if ( p_data_type == DEF_SONF_S_RQ_SD ||          /* 開閉局_仕向要求送信            */
         p_data_type == DEF_ECHO_S_RQ_SD   ) {       /* エコー_仕向要求送信            */

        /* 送信エリアのアドレスセット */
        lp_r402 = (cr402_def *)g_resp_buf;

        g_glmlg.pri_key.part_id[0] = '0';
        g_glmlg.pri_key.part_id[1] = p_rslt_info->lcn_no[14];
        memcpy ( g_glmlg.pri_key.lcn_id , p_rslt_info->lcn_no
                                        , sizeof(g_glmlg.pri_key.lcn_id));
        g_glmlg.pri_key.s_h_kubun    = DEF_S_H_KUBUN_SIMUKE;
        g_glmlg.pri_key.send_recv_id = DEF_REQ_SEND;

        /* コネクション論理IDセット */
        g_glmlg.connect_id.site_id   = lp_r402->control_info.connection_lid.site_name;
        g_glmlg.connect_id.nw_id     = lp_r402->control_info.connection_lid.nw_name;
        memcpy( g_glmlg.connect_id.grp_id    , lp_r402->control_info.connection_lid.group_name
                                             , sizeof(g_glmlg.connect_id.grp_id));
        memcpy( g_glmlg.connect_id.if_id     , lp_r402->control_info.connection_lid.interface_name
                                             , sizeof(g_glmlg.connect_id.if_id));
        memcpy( g_glmlg.connect_id.station_id, lp_r402->control_info.connection_lid.station_name
                                             , sizeof(g_glmlg.connect_id.station_id));
        memcpy( g_glmlg.connect_id.connect_id, lp_r402->control_info.connection_lid.connection_name
                                             , sizeof(g_glmlg.connect_id.connect_id));

        memcpy( g_glmlg.mti_id               , p_rslt_info->mti
                                             , sizeof(g_glmlg.mti_id));

        if ( p_rslt_info->denbun_len != 0 ) {
            if ( p_rslt_info->rsp_result != DEF_RSP_TYPE_NORMAL ) {
                p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
            }
        }

        memcpy( g_glmlg.control_kind         , p_rslt_info->ctrl_type
                                             , sizeof(g_glmlg.control_kind));
        memcpy( g_glmlg.naibu_err_code       , p_rslt_info->naibu_errcd
                                             , sizeof(g_glmlg.naibu_err_code));
        memcpy( g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id
              , g_myinfo.serverclass_name
              , sizeof(g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_id));
//      memcpy( g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num
//            , "0000"
//            , sizeof(g_glmlg.cntrl_denbun_srv_cls_info.srv_cls_mlt_num));

        if ( p_rslt_info->denbun_len != 0 ) {
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_ON;
            sprintf( g_glmlg.denbun_area.denbun_len, "%05d", p_rslt_info->denbun_len);
            memcpy ( g_glmlg.denbun_area.denbun    , lp_r402->data_bu.message_text
                                                   , p_rslt_info->denbun_len);
        } else {
            g_glmlg.denbun_info_exist = DEF_DENBUN_IFO_OFF;
            memset( g_glmlg.denbun_area.denbun_len, DEF_BUF_CZERO
                                                  , sizeof(g_glmlg.denbun_area.denbun_len));
        }
    }

    COM_SDT( DEF_COM_SDT_arg1_jpn, &g_com_sdt_arg_2, &g_com_sdt_arg_3, &ll_datetime);
    memcpy ( g_glmlg.entry_timestamp, (char *)&g_com_sdt_arg_2, sizeof(g_glmlg.entry_timestamp));

    ls_result = CMIN_put_glmlg ( g_kbt_file_data.ctrl_log_fname     /* 物理ファイル名         */
                               , g_kbt_file_data.ctrl_log_fno       /* ファイル番号           */
                               , (char *)&g_glmlg.pri_key           /* 読込みキー             */
                               , DEF_COM_IOM_NOLOCK                 /* LOCK有無               */
                               , (char *)&g_glmlg                   /* 読込んだレコード       */
                               , &ls_guardian_error  );             /* I/Oエラーコード        */
    
    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        memset( lc_wbuf, DEF_BUF_NULL            , sizeof(lc_wbuf));
        memcpy( lc_wbuf, (char *)&g_glmlg.pri_key, sizeof(g_glmlg.pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@5"
                            , ""
                            , ""
                            , DEF_FL_CTRL_DEN_LOG
                            , DEF_COM_IOM_FUNC_ADD
                            , lc_wbuf
                            , ls_guardian_error );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_read_glmlg                                 */
/*  CALLING SEQ.    : short  CSTE_read_glmlg()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログ取得処理                                   */
/*****************************************************************************/
short  CSTE_read_glmlg(short p_read_kbn, short p_lockmode, char p_s_h_kbn, t_rcv_info_def *p_rslt_info)
{
    short         ls_result;
    short         ls_guardian_error;
    char          lc_wbuf[40];
    cr401_def    *lp_c401;

    lp_c401     = (cr401_def *)g_recv_buf;

    /* ---------------------------------------------- */
    /* 制御電文ログKey編集                            */
    /* ---------------------------------------------- */
    g_glmlg.pri_key.part_id[0] = '0';
    g_glmlg.pri_key.part_id[1] = lp_c401->control_info.req_gfp_lcn[14];
    memcpy ( g_glmlg.pri_key.lcn_id , lp_c401->control_info.req_gfp_lcn
                                    , sizeof(g_glmlg.pri_key.lcn_id));
    g_glmlg.pri_key.s_h_kubun    = p_s_h_kbn;

    if ( p_read_kbn == DEF_GLMLG_READ_REQ ) {
        g_glmlg.pri_key.send_recv_id = DEF_REQ_SEND;
    } else {
        g_glmlg.pri_key.send_recv_id = DEF_RSP_RCV;
    }

    ls_result = CMIN_read_glmlg( g_kbt_file_data.ctrl_log_fname
                               , g_kbt_file_data.ctrl_log_fno
                               , (char *)&g_glmlg.pri_key
                               , p_lockmode
                               , (char *)&g_glmlg
                               , &ls_guardian_error );

    if (( ls_result != DEF_RET_OK ) ||                 /* IOモジュール結果判定 */
        ( memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        memset( lc_wbuf, DEF_BUF_NULL            , sizeof(lc_wbuf));
        memcpy( lc_wbuf, (char *)&g_glmlg.pri_key, sizeof(g_glmlg.pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@5"
                            , ""
                            , ""
                            , DEF_FL_CTRL_DEN_LOG
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , lc_wbuf
                            , ls_guardian_error );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_update_glmlg                               */
/*  CALLING SEQ.    : short  CSTE_update_glmlg()                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 制御電文ログ更新処理                                   */
/*****************************************************************************/
short  CSTE_update_glmlg(char p_s_h_kbn,short p_req_rcv, t_rcv_info_def *p_rslt_info)
{
    short  ls_result;
    short  ls_guardian_error;
    char   lc_wbuf[40];
    cr401_def *lp_c401;

    lp_c401     = (cr401_def *)g_recv_buf;

    ls_result = CSTE_read_glmlg( p_req_rcv
                               , DEF_COM_IOM_LOCK
                               , p_s_h_kbn
                               , p_rslt_info );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    memcpy( g_glmlg.control_kind       , p_rslt_info->ctrl_type
                                       , sizeof(g_glmlg.control_kind));
    memcpy( g_glmlg.send_naibu_err_code, lp_c401->control_info.send_naibu_err_code
                                       , sizeof(g_glmlg.send_naibu_err_code));

    ls_result = CMIN_update_glmlg ( g_kbt_file_data.ctrl_log_fname
                                  , g_kbt_file_data.ctrl_log_fno
                                  , (char *)&g_glmlg.pri_key
                                  , DEF_COM_IOM_LOCKFREE
                                  , (char *)&g_glmlg
                                  , &ls_guardian_error );

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        memset( lc_wbuf, DEF_BUF_NULL            , sizeof(lc_wbuf));
        memcpy( lc_wbuf, (char *)&g_glmlg.pri_key, sizeof(g_glmlg.pri_key));
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@5"
                            , ""
                            , ""
                            , DEF_FL_CTRL_DEN_LOG
                            , DEF_COM_IOM_FUNC_UPDATE
                            , lc_wbuf
                            , ls_guardian_error );
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_FILE_IO_ERR
                                        , strlen(DEF_NERR_FILE_IO_ERR));
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* end of CSTE_update_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_get_lcn                                    */
/*  CALLING SEQ.    : short  CSTE_get_lcn()                                  */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : GFP内部LCN取得処理                                     */
/*****************************************************************************/
short  CSTE_get_lcn(t_rcv_info_def *p_rslt_info)
{
    short     ls_result;
    c701_def  l_c701;                   /* GFP内部LCN採番要求IPC設定テーブル */
    r701_def *l_r701;                   /* GFP内部LCN採番応答IPC設定テーブル */

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def l_com_psd_arg1;
    COM_PSD_arg_2_def l_com_psd_arg2;
    COM_PSD_arg_3_def l_com_psd_arg3;
    COM_PSD_arg_4_def l_com_psd_arg4;

    l_r701 = (r701_def *)l_com_psd_arg1.msg_buf;

    /* GFP内部LCN採番要求IPC設定テーブル初期化 */
    memset( &l_c701, DEF_BUF_NULL, sizeof(l_c701));

    /* IPC interface_code設定 */
    memcpy( l_c701.common_header.interface_code, DEF_IPC_IFCD_LCN_NUM_REQ
          , sizeof(l_c701.common_header.interface_code));

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( l_c701.common_header.internal_error_code, DEF_BUF_SPACE
                                                    , sizeof(l_c701.common_header.internal_error_code));
    memset( l_c701.common_header.filler_1           , DEF_BUF_SPACE
                                                    , sizeof(l_c701.common_header.filler_1));
    memcpy( l_c701.process_name                     , g_procinfo.my_pname
                                                    , g_procinfo.my_pname_len );
    l_c701.site_code                         = g_myinfo.site_id;
    l_c701.network_code                      = DEF_NW_ID_GFP;
    l_c701.common_header.control_data_length = sizeof(l_c701) - sizeof(l_c701.common_header);

    /* PATHSENDパラメータ初期化 */
    memset( &l_com_psd_arg1, DEF_BUF_NULL  , sizeof(l_com_psd_arg1));
    memset( &l_com_psd_arg2, DEF_BUF_NULL  , sizeof(l_com_psd_arg2));
    memset( &l_com_psd_arg3, DEF_BUF_NULL  , sizeof(l_com_psd_arg3));
    memset( &l_com_psd_arg4, DEF_BUF_SPACE , sizeof(l_com_psd_arg4));

    /* PATHSEND用情報設定 */
    if ( g_kbt_svrcls.lcn_domain_name[0] != ' ' ) {
        memcpy( l_com_psd_arg1.pathmon_name    , g_kbt_svrcls.lcn_domain_name
                                               , sizeof(l_com_psd_arg1.pathmon_name));
    } else {
        memcpy( l_com_psd_arg1.pathmon_name    , g_kbt_svrcls.lcn_pathmon_name
                                               , sizeof(l_com_psd_arg1.pathmon_name));
    }
    memcpy( l_com_psd_arg1.serverclass_name, g_kbt_svrcls.lcn_srvcls_name
                                           , sizeof(l_com_psd_arg1.serverclass_name));
    memcpy( l_com_psd_arg1.msg_buf, &l_c701, sizeof(l_c701));
    l_com_psd_arg1.req_send_len    = sizeof(l_c701);
    l_com_psd_arg1.receive_max_len = sizeof(r701_def);
    l_com_psd_arg1.send_timer_msec = g_myinfo.send_timer;
    l_com_psd_arg1.retry_cnt       = (short)g_myinfo.send_retry_count;

    memcpy( l_com_psd_arg2.prog_id       , g_myinfo.prog_id
                                         , sizeof(l_com_psd_arg2.prog_id)   );
    memcpy( l_com_psd_arg4.srv_logical_id, g_myinfo.serverclass_name
                                         , sizeof(g_myinfo.serverclass_name));

    /* PATHSEND共通処理実行 */
    ls_result = COM_PSD( &l_com_psd_arg1
                       , &l_com_psd_arg2
                       , &l_com_psd_arg3
                       , &g_cg010in_modle
                       , &l_com_psd_arg4 );

    /* PATHSEND結果確認 */
    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output( DEF_EVT_LCN_GET_ERR
                           , DEF_MSGTTKB_GYOM_ERR
                           , DEF_NERR_LCN_GET_ERR
                           , "@L@S@U"
                           , l_com_psd_arg4.lcn
                           , l_com_psd_arg4.srv_logical_id
                           , l_com_psd_arg3.pathsend_errcode);
        /* GFP内部LCN取得エラー(内部エラー) */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_LCN_GET_ERR, strlen(DEF_NERR_LCN_GET_ERR));
        return DEF_RET_NG;
    }

    /* 応答電文のインタフェースコード判定(R701) */
    if ( memcmp( l_r701->common_header.interface_code
               , DEF_IPC_IFCD_LCN_NUM_RSP, strlen(DEF_IPC_IFCD_LCN_NUM_RSP)) == 0 ) {
        /* 応答電文のエラーコード判定 */
        if (l_r701->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常 */
            ls_result = DEF_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        ls_result = DEF_RET_NG;
    }
    if ( ls_result == DEF_RET_NG ) {
        /* EMS出力 応答エラー */
        CMIN_message_output( DEF_EVT_RSP_ERR
                           , DEF_MSGTTKB_GYOM_ERR
                           , DEF_NERR_LCN_GET_ERR
                           , "@L@S@C"
                           , l_com_psd_arg4.lcn
                           , g_myinfo.serverclass_name
                           , "R701 ERROR          ");
        /* GFP内部LCN取得エラー(内部エラー) */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_ERROR;
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_LCN_GET_ERR, strlen(DEF_NERR_LCN_GET_ERR));
        return DEF_RET_NG;
    }

    /* GFP内部LCNを受信バッファから取得 */
    memcpy( p_rslt_info->lcn_no, (char *)&l_r701->gfplcn, sizeof(l_r701->gfplcn) );

    return DEF_RET_OK;
} /* end of CSTE_get_lcn */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_get_sysnum                                 */
/*  CALLING SEQ.    : short  CSTE_get_sysnum()                               */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : システム通番取得処理                                   */
/*****************************************************************************/
short  CSTE_get_sysnum(t_rcv_info_def *p_rslt_info)
{
    short ls_result = 0;
    ss02_def ipc_msg;               /* システム採番要求IPC設定テーブル */
    ss01_def *ss01_def_ipc;         /* システム採番応答IPC設定テーブル */
    ss02_def *ss02_def_ipc;         /* システム採番応答IPC設定テーブル */

    ss01_def_ipc = (ss01_def*)&ipc_msg;

    /* PATHSENDパラメータ */
    COM_PSD_SYS_arg_1_def t_COM_PSD_SYS_arg_1_def;
    COM_PSD_SYS_arg_2_def t_COM_PSD_SYS_arg_2_def;
    COM_PSD_SYS_arg_3_def t_COM_PSD_SYS_arg_3_def;
    COM_PSD_SYS_arg_4_def t_COM_PSD_SYS_arg_4_def;

    /* システム採番要求IPC設定テーブル初期化 */
    memset( &ipc_msg, 0, sizeof(ipc_msg) );

    /* IPC interface_code設定 */
    memcpy( ss01_def_ipc->interface_code,
            DEF_IPC_IFCD_SYSTEM_NUM_REQ,
            sizeof(ss01_def_ipc->interface_code) );

    /* IPC 内部通番区分を設定 */
    memcpy( ss01_def_ipc->serial_number_kbn,
            DEF_STAN6,
            sizeof(ss01_def_ipc->serial_number_kbn));

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_SYS_arg_1_def, DEF_BUF_NULL , sizeof(t_COM_PSD_SYS_arg_1_def) );
    memset( &t_COM_PSD_SYS_arg_2_def, DEF_BUF_NULL , sizeof(t_COM_PSD_SYS_arg_2_def) );
    memset( &t_COM_PSD_SYS_arg_3_def, DEF_BUF_NULL , sizeof(t_COM_PSD_SYS_arg_3_def) );
    memset( &t_COM_PSD_SYS_arg_4_def, DEF_BUF_SPACE, sizeof(t_COM_PSD_SYS_arg_4_def) );

    /* PATHSEND用情報設定 */
    memcpy(t_COM_PSD_SYS_arg_1_def.pathmon_name_pri,
           g_sys_no_data_pri.pathmon_name,
           sizeof(g_sys_no_data_pri.pathmon_name));

    memcpy(t_COM_PSD_SYS_arg_1_def.serverclass_name_pri,
           g_sys_no_data_pri.server_class,
           sizeof(g_sys_no_data_pri.server_class));

    memcpy(t_COM_PSD_SYS_arg_1_def.pathmon_name_sec,
           g_sys_no_data_sec.pathmon_name,
           sizeof(g_sys_no_data_sec.pathmon_name));

    memcpy(t_COM_PSD_SYS_arg_1_def.serverclass_name_sec,
           g_sys_no_data_sec.server_class,
           sizeof(g_sys_no_data_sec.server_class));

    memcpy(t_COM_PSD_SYS_arg_1_def.msg_buf, ss01_def_ipc, sizeof(ss01_def) );

    t_COM_PSD_SYS_arg_1_def.req_send_len    = sizeof(ss01_def);
    t_COM_PSD_SYS_arg_1_def.receive_max_len = DEF_PSEND_DATA_MAX;
    t_COM_PSD_SYS_arg_1_def.send_timer_msec = (long)g_myinfo.send_timer;
    t_COM_PSD_SYS_arg_1_def.retry_cnt       = (short)g_myinfo.send_retry_count;

    memcpy(t_COM_PSD_SYS_arg_2_def.prog_id,
           g_myinfo.prog_id,
           sizeof(t_COM_PSD_SYS_arg_2_def.prog_id) );

    /* サーバークラス論理ID設定 */
    memcpy( t_COM_PSD_SYS_arg_4_def.srv_logical_id,
            g_myinfo.serverclass_name,
            sizeof(g_myinfo.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy( t_COM_PSD_SYS_arg_4_def.lcn,
            p_rslt_info->lcn_no,
            sizeof(p_rslt_info->lcn_no));

    /* PATHSEND共通処理実行 */
    ls_result = COM_PSD_SYS( &t_COM_PSD_SYS_arg_1_def,
                             &t_COM_PSD_SYS_arg_2_def,
                             &t_COM_PSD_SYS_arg_3_def,
                             &g_cg010in_modle,
                             &t_COM_PSD_SYS_arg_4_def );

    /* PATHSEND結果確認 */
    if ( ls_result != DEF_RET_OK ) {
        /* 異常終了 */
        /* システム採番取得エラー(EMS) */
        CMIN_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SYS_NO_MAKE_ERR,
                            "@C@2",
                            "COM_PSD_SYS",
                            t_COM_PSD_SYS_arg_3_def.guardian_errcode);

        /* システム採番取得エラー(内部エラー) */
        strncpy( p_rslt_info->naibu_errcd,
                 DEF_NERR_SYS_NO_MAKE_ERR,
                 sizeof(p_rslt_info->naibu_errcd));
        /* 拒否応答設定 */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;

        return DEF_RET_NG;
    }

    ss02_def_ipc = (ss02_def *)t_COM_PSD_SYS_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(SS02) */
    if ( memcmp(ss02_def_ipc->interface_code,
                DEF_IPC_IFCD_SYSTEM_NUM_RSP,
                sizeof(ss02_def_ipc->interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (( memcmp( ss02_def_ipc->internal_error_code, DEF_NERR_NOMAL     , strlen(DEF_NERR_NOMAL)) != 0 )&&
            ( memcmp( ss02_def_ipc->internal_error_code, DEF_NERR_INIT_SPACE, strlen(DEF_NERR_INIT_SPACE)) != 0 )) {

            /* 異常 */
            ls_result = DEF_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        ls_result = DEF_RET_NG;
    }
    if ( ls_result == DEF_RET_NG ) {
        /* EMS出力 応答エラー */
        CMIN_message_output(DEF_EVT_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SYS_NO_MAKE_ERR,
                            "@L@C@C@H",
                            p_rslt_info->lcn_no,
                            g_myinfo.serverclass_name,
                            "SS02 ERROR          ",
                            ss02_def_ipc);
        /* システム採番取得エラー(内部エラー) */
        strncpy( p_rslt_info->naibu_errcd,
                  DEF_NERR_SYS_NO_MAKE_ERR,
                  sizeof(p_rslt_info->naibu_errcd));
        /* 拒否応答設定 */
        p_rslt_info->rsp_result = DEF_RSP_TYPE_KYOHI;
        return DEF_RET_NG;
    }

    /* システム採番を受信バッファから取得 */
    memcpy( p_rslt_info->sys_no, ss02_def_ipc->numbering_value, sizeof(ss02_def_ipc->numbering_value) );

    return DEF_RET_OK;
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CSTE_error_msg_out                              */
/*  CALLING SEQ.    : short  CSTE_error_msg_out()                            */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : エラー出力ログ出力処理                                 */
/*****************************************************************************/
void  CSTE_error_msg_out(t_rcv_info_def *p_rslt_info)
{
    short              ls_result;
    short              ls_data_len;
    short              ls_erlg_len;

    char               lc_datetime_hex[16+1];
    char               lc_wklen[6];


    cr401_def         *lp_c401;
    db_glelg_def       l_log_tbl;                 /* ログ出力テーブル         */
    COM_UNQ_arg_1_def  l_com_unq_arg1;            /* ユニーク日時取得         */

    lp_c401 = (cr401_def *)g_recv_buf;

    memset( &l_log_tbl             , DEF_BUF_NULL, sizeof(l_log_tbl)      ); /* ログ出力要求テーブル */
    memset( (char *)&l_com_unq_arg1, DEF_BUF_NULL, sizeof(l_com_unq_arg1 )); /* ユニーク日時取得arg1 */
    memset( lc_datetime_hex        , DEF_BUF_NULL, sizeof(lc_datetime_hex)); /* 日時エリア           */

    /* ---------------------------------------- */
    /* ユニーク日時取得                         */
    /* ---------------------------------------- */
    COM_UNQ( &l_com_unq_arg1, lc_datetime_hex );  

    /* エラーログ P-Key セット */
    memcpy( l_log_tbl.pri_key.part_id          , l_com_unq_arg1.cc
                                               , sizeof(l_log_tbl.pri_key.part_id));
    memcpy( l_log_tbl.pri_key.time_stamp       ,&l_com_unq_arg1
                                               , sizeof(l_log_tbl.pri_key.time_stamp));
    memcpy( l_log_tbl.pri_key.time_stamp_branch, l_com_unq_arg1.ss
                                               , sizeof(l_log_tbl.pri_key.time_stamp_branch));

    l_log_tbl.err_denbun_id = DEF_ERR_ID_NAIBU;
    memcpy( l_log_tbl.mti_id                   , lp_c401->control_info.mti
                                               , sizeof(l_log_tbl.mti_id));
    memset( l_log_tbl.res_code                 , DEF_BUF_SPACE
                                               , sizeof(l_log_tbl.res_code));
    memcpy( l_log_tbl.naibu_err_code           , p_rslt_info->naibu_errcd
                                               , sizeof(l_log_tbl.naibu_err_code));
    memcpy( l_log_tbl.srv_cls_info.srv_cls_id  , g_myinfo.serverclass_name
                                               , sizeof(l_log_tbl.srv_cls_info.srv_cls_id));

//  memset( l_log_tbl.srv_cls_info.srv_cls_mlt_num, DEF_BUF_CZERO
//                                             , sizeof(l_log_tbl.srv_cls_info.srv_cls_mlt_num));
    memcpy( l_log_tbl.denbun_send_recv_info.denbun_recv_time, l_log_tbl.pri_key.time_stamp
                                               , sizeof(l_log_tbl.denbun_send_recv_info.denbun_recv_time));

    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_OPN, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_OPN;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_CLS, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_CLS;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_OPNING, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_OPNING;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_CLOSING, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_CLSING;
    }

    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_OPN, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_OPN;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_CLS, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_CLS;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_OPNING, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_OPNING;
    } else
    if ( memcmp( g_gcsst.state_sts_info.state_sts
               , DEF_STTE_STS_CLOSING, sizeof(g_gcsst.state_sts_info.state_sts)) == 0 ) {
        l_log_tbl.denbun_send_recv_info.recv_kyoku_sts = DEF_RCV_KYOKU_STS_CLSING;
    }

    memcpy( l_log_tbl.denbun_send_recv_info.nw_kubun, g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].nw_id_info.nw_kubun
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.nw_kubun) );
    memcpy( l_log_tbl.denbun_send_recv_info.mti_id  , l_log_tbl.mti_id
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.mti_id));

    l_log_tbl.denbun_send_recv_info.send_denbun_shubetu = DEF_BUF_SPACE;
    memcpy(&l_log_tbl.denbun_send_recv_info.denbun_log_key,&lp_c401->control_info.denbun_log_key
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.denbun_log_key));

    l_log_tbl.denbun_send_recv_info.denbun_fmt_kubun = DEF_BUF_CZERO;

    memset( l_log_tbl.denbun_send_recv_info.tushin_log_save_filename, DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.tushin_log_save_filename));
    memset(&l_log_tbl.denbun_send_recv_info.tushin_log_key, DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.tushin_log_key));
    memset( l_log_tbl.tushin_cntrl_info.if_id       , DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.if_id));
    memset( l_log_tbl.tushin_cntrl_info.station_id  , DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.station_id));
    memcpy(&l_log_tbl.tushin_cntrl_info.line_info.recv_connect_id, &lp_c401->control_info.connection_lid
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.line_info.recv_connect_id));
    memset(&l_log_tbl.tushin_cntrl_info.line_info.recv_connect_info, DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.line_info.recv_connect_info));
    memset(&l_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp, DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp));
    memset( l_log_tbl.future_use                    , DEF_BUF_SPACE, sizeof(l_log_tbl.future_use));
    memset( l_log_tbl.denbun_send_recv_info.future_use, DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.denbun_send_recv_info.future_use));
    memset( l_log_tbl.tushin_cntrl_info.future_use  , DEF_BUF_SPACE
                                                    , sizeof(l_log_tbl.tushin_cntrl_info.future_use));
    /* 電文長 */
    ls_data_len = (short)(lp_c401->common_header.control_data_length - (short)sizeof(lp_c401->control_info));
    memset  ( lc_wklen, DEF_BUF_NULL, sizeof(lc_wklen));
    snprintf( lc_wklen, sizeof(lc_wklen), "%05d" , ls_data_len );
    memcpy  ( l_log_tbl.denbun_area.denbun_len   , lc_wklen, sizeof(l_log_tbl.denbun_area.denbun_len));

    /* MTI開始位置設定 */
    memset  ( l_log_tbl.denbun_area.mti_start_lct, DEF_BUF_CZERO
                                                 , sizeof(l_log_tbl.denbun_area.mti_start_lct));
    if ( ls_data_len != 0 ) {
        memset  ( lc_wklen, DEF_BUF_NULL, sizeof(lc_wklen));
        snprintf( lc_wklen, sizeof(lc_wklen), "%05d"
                , g_gfnwi_tbl[DEF_FNWI_IDX_INTERFACE].denbun_item_lct_info.mti_start_lct);
        memcpy  ( l_log_tbl.denbun_area.mti_start_lct, lc_wklen
                , sizeof(l_log_tbl.denbun_area.mti_start_lct));
    }

    memcpy( l_log_tbl.denbun_area.denbun, lp_c401->data_bu.message_text, ls_data_len);

    /* エラーログ用パラメータ初期化 */
    memset(&g_com_erl_arg_1, DEF_BUF_NULL , sizeof(COM_ERL_arg_1_def));
    memset(&g_com_erl_arg_3, DEF_BUF_SPACE, sizeof(COM_ERL_arg_3_def));

    /* エラーログ用情報設定 */
    g_com_erl_arg_1.file_io_type = DEF_COM_ERL_ARG1_WRITE;
    g_com_erl_arg_1.io_timer     = g_myinfo.send_timer;

    ls_erlg_len = (short)(sizeof(l_log_tbl) - sizeof(l_log_tbl.denbun_area.denbun)
                                            + ls_data_len);
    g_com_erl_arg_1.data_len  = ls_erlg_len;
    g_com_erl_arg_1.data_area = (char *)&l_log_tbl;

    memcpy( g_com_erl_arg_3.srv_logical_id, g_myinfo.serverclass_name
                                         , sizeof(g_myinfo.serverclass_name));
    memcpy( g_com_erl_arg_3.lcn           , lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn
                                         , sizeof(lp_c401->control_info.denbun_log_key.tran_id.gfp_lcn));
    memcpy( g_com_erl_arg_3.connect       , &lp_c401->control_info.connection_lid
                                         , sizeof(lp_c401->control_info.connection_lid));

    ls_result = COM_ERL( &g_com_erl_arg_1,                   /* エラーログ共通処理実行   */
                         &g_com_erl_arg_2,
                         &g_cg010in_modle,
                         &g_com_erl_arg_3,
                         g_myinfo.prog_id);

    if ( ls_result != DEF_RET_OK ) {                        /* エラーログ結果判定       */
        /* 異常終了 */
        /* EMS出力 */
        CMIN_message_output( DEF_EVT_COMMON_MOD_ERR
                           , DEF_MSGTTKB_GYOM_ERR
                           , DEF_NERR_ERRLOG_OUTPUT_ERR
                           , "@C@5", "COM_ERL", ls_result);
        return;
    }

} /* end of CSTE_error_msg_out */
