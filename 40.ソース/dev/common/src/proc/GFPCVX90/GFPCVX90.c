/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX90                                    */
/*        FUNCTION          ････ 鍵交換制御                                  */
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
#include "GFPCVX90.h"           // 鍵交換

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
    memcpy( g_myinfo.prog_id, DEF_GFPCVX90, strlen(DEF_GFPCVX90));

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
    db_gfphi_def     *phy_tbl_local;
    t_gfphi_pri_key  l_gfphi_pri_key;
    char             ch_file_no[5];

    /* グローバル領域初期化 */
    memset( &g_file_info, 0, sizeof(g_file_info) );

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
          , DEF_FL_NW_KOYU_INFO , sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
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
    memcpy( g_file_info.gfnws_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_file_info.gfnws_fname));
   /* ファイル番号を取得 */
   memset( ch_file_no, 0, sizeof(ch_file_no) );
   memcpy( ch_file_no,
           phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
           sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
   g_file_info.gfnws_fname_len = (short)atoi(ch_file_no);

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
    memcpy( g_file_info.gcsst_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_file_info.gcsst_fname));
   /* ファイル番号を取得 */
   memset( ch_file_no, 0, sizeof(ch_file_no) );
   memcpy( ch_file_no,
           phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
           sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
   g_file_info.gcsst_fno = (short)atoi(ch_file_no);

/* ------------------------------------------------------ */
/* 鍵管理管理ファイル  物理名取得                         */
/* ------------------------------------------------------ */
    /* 鍵管理情報・ATALLA情報取得処理 */
    /* 個別モジュールパラメータ*/
    NWM_ENI_arg_1_def t_NWM_ENI_arg_1_def;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def_own;
    NWM_ENI_arg_2_def t_NWM_ENI_arg_2_def_other;
    NWM_ENI_arg_3_def t_NWM_ENI_arg_3_def;
    NWM_ENI_arg_4_def t_NWM_ENI_arg_4_def;
    char ch_module_id[8+1];
    ems_info_add    ems_info_add_local;

    /* 個別モジュールパラメータ初期化 */
    memset( &t_NWM_ENI_arg_1_def,       CKYX_SPACE, sizeof(t_NWM_ENI_arg_1_def) );
    memset( &t_NWM_ENI_arg_2_def_own,   0, sizeof(t_NWM_ENI_arg_2_def_own) );
    memset( &t_NWM_ENI_arg_2_def_other, 0, sizeof(t_NWM_ENI_arg_2_def_other) );
    memset( &t_NWM_ENI_arg_3_def,       0, sizeof(t_NWM_ENI_arg_3_def) );
    memset( &t_NWM_ENI_arg_4_def,       0, sizeof(t_NWM_ENI_arg_4_def) );
    memset( &ems_info_add_local,        0, sizeof(ems_info_add));
    memset( ch_module_id,               0, sizeof(ch_module_id) );
    memcpy( ch_module_id, g_myinfo.prog_id,sizeof(ch_module_id) );

    /* 鍵管理情報・ATALLA情報取得処理 */
    memcpy(t_NWM_ENI_arg_1_def.file_id, DEF_GFPHI, strlen(DEF_GFPHI));
    memcpy(t_NWM_ENI_arg_1_def.file_name,
        g_com_file_data.phy_file_name, sizeof(t_NWM_ENI_arg_1_def.file_name));
    t_NWM_ENI_arg_1_def.file_no  = g_com_file_data.phy_file_no;
    t_NWM_ENI_arg_1_def.io_timer = g_myinfo.io_timer;
    t_NWM_ENI_arg_3_def.pathsend_retry_cnt = (short)g_myinfo.send_retry_count;
    t_NWM_ENI_arg_4_def.site_id  = g_myinfo.site_id;
    t_NWM_ENI_arg_4_def.nw_id    = g_myinfo.network_id;
    memcpy( t_NWM_ENI_arg_4_def.grp_id, g_myinfo.group_id,
               sizeof(t_NWM_ENI_arg_4_def.grp_id) );

    /* 個別モジュール */
    ls_result = NWM_ENI(DEF_NWM_ENI_OWN_NODE_ONLY,
                        &t_NWM_ENI_arg_1_def,
                        &t_NWM_ENI_arg_2_def_own,
                        &t_NWM_ENI_arg_2_def_other,
                        &t_NWM_ENI_arg_3_def,
                        &t_NWM_ENI_arg_4_def,
                        ch_module_id,
                        &g_cg010in_modle,
                        &ems_info_add_local
                        );

    /* 個別モジュール結果判定 */
    if ( ls_result != DEF_RET_OK) {
        /* エラー終了 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }

    /* 鍵管理情報(自ノード)格納処理 */
    /* 鍵管理ファイルID */
    memcpy( g_file_info.own_gckey_fid, t_NWM_ENI_arg_2_def_own.file_id,
                          sizeof(g_file_info.own_gckey_fid) );
    /* 鍵管理ファイル名 */
    memcpy( g_file_info.own_gckey_fname, t_NWM_ENI_arg_2_def_own.file_name,
                          sizeof(g_file_info.own_gckey_fname) );
    /* 鍵管理ファイル番号 */
    g_file_info.own_gckey_fno    = t_NWM_ENI_arg_2_def_own.file_no;

    /* ATALLA情報設定 */
    /* PATHMONプロセス名 */
    memcpy( g_atalla_con.pathmon_name, t_NWM_ENI_arg_3_def.domain_name,
                          sizeof(g_atalla_con.pathmon_name) );
    /* サーバクラス名 */
    memcpy( g_atalla_con.server_class, t_NWM_ENI_arg_3_def.server_name,
                          sizeof(g_atalla_con.server_class) );
    /* PATHSENDタイマー */
    g_atalla_con.pathsend_timer = g_myinfo.send_timer;
    /* PATHSENDリトライ回数 */
    g_atalla_con.retry_cnt      = t_NWM_ENI_arg_3_def.pathsend_retry_cnt;

/* ------------------------------------------------------ */
/* 制御電文ログ管理ファイル  物理名取得                   */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id     , g_myinfo.group_id
                                       , sizeof(l_gfphi_pri_key.grp_id));
// 物理名のKey設定が未記載
    memset(&l_gfphi_pri_key.srv_cls_key
          , DEF_BUF_NO_SET    , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_CTRL_DEN_LOG, sizeof(l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind));
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
    memcpy( g_file_info.glmlg_fname
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_file_info.glmlg_fname));
   /* ファイル番号を取得 */
   memset( ch_file_no, 0, sizeof(ch_file_no) );
   memcpy( ch_file_no,
           phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num,
           sizeof(phy_tbl_local->pri_key.prc_file_key.prc_file_id.prc_file_num) );
   g_file_info.glmlg_fno = (short)atoi(ch_file_no);

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
    memcpy( g_lcncon_data.domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_lcncon_data.domain_name));
    memcpy( g_lcncon_data.pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_lcncon_data.pathmon_name));
    memcpy( g_lcncon_data.server_class , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_lcncon_data.server_class));

/* ------------------------------------------------------ */
/* システム採番生成サーバ情報取得                         */
/* ------------------------------------------------------ */
    /* プライマリ */
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
/* 鍵管理ファイル更新サーバ情報取得                       */
/* ------------------------------------------------------ */
    if (g_myinfo.site_id == DEF_SITE_ID_TKY){
        l_gfphi_pri_key.site_id = DEF_SITE_ID_OSK;
    }
    else{
        l_gfphi_pri_key.site_id = DEF_SITE_ID_TKY;
    }
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id
          , g_myinfo.group_id, sizeof(l_gfphi_pri_key.grp_id));
    memcpy( l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind
          , DEF_SC_GCKEY_UPD , sizeof(l_gfphi_pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
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
    memcpy( g_gckey_upd_srv.domain_name , phy_tbl_local->srv_cls_info.domain_name
          , sizeof(g_gckey_upd_srv.domain_name));
    memcpy( g_gckey_upd_srv.pathmon_name, phy_tbl_local->srv_cls_info.pathmon_name
          , sizeof(g_gckey_upd_srv.pathmon_name));
    memcpy( g_gckey_upd_srv.server_class , phy_tbl_local->srv_cls_info.srv_cls_name
          , sizeof(g_gckey_upd_srv.server_class));

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
//    short                ls_result;

// ---------------------------------------------------------------------------/
//  個別で必要な初期処理が存在する場合はこの関数を使用する。無ければこのまま  /
// ---------------------------------------------------------------------------/
    /* カット対象日付管理ファイル情報取得 */
    memcpy((char *)&g_file_info_gccut,
           (char *)&g_NWM_CTU_INI_arg_2,
           sizeof(g_file_info_gccut));

    g_file_info_gccut.io_timer = g_myinfo.io_timer;

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
    ls_result = CMIN_file_open( DEF_FL_NW_KOYU_INFO
                              , g_file_info.gfnws_fname
                              ,&g_file_info.gfnws_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 局状態管理ファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_CEN_STS
                              , g_file_info.gcsst_fname
                              ,&g_file_info.gcsst_fno);

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

    /* 制御電文ログファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_CTRL_DEN_LOG
                              , g_file_info.glmlg_fname
                              ,&g_file_info.glmlg_fno);

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
    CMIN_file_close( DEF_FL_NW_KOYU_INFO
                    , g_file_info.gfnws_fname
                    ,&g_file_info.gfnws_fno);

    /* 局状態管理ファイルクローズ */
    CMIN_file_close( DEF_FL_CEN_STS
                    , g_file_info.gcsst_fname
                    ,&g_file_info.gcsst_fno);


    /* 制御電文ログファイルクローズ */
//    CMIN_file_close( DEF_FL_CTRL_DEN_LOG,
//                    , g_file_info.glmlg_fname
//                    , &g_file_info.glmlg_fno);

} /* end of CMIN_kbt_file_close */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_handle_req_msg                             */
/*  CALLING SEQ.    : short CMIN_handle_req_msg (void)                       */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : リクエスタメッセージ処理                               */
/*****************************************************************************/
void  CMIN_handle_req_msg()
{
    short ls_result = 0;
    common_header_def *rcv_ipc;

    memset(g_send_err_code, CKYX_SPACE, sizeof(g_send_err_code));
    g_request_trigger = CKYX_REQUEST_TRIGGER_UNDEFINED;

    rcv_ipc = (common_header_def *)&g_recv_buf;
    if (memcmp(rcv_ipc->interface_code,
        DEF_IPC_IFCD_NW_MSG_REQ,
        sizeof(rcv_ipc->interface_code)) == 0) {
        /* C401 */
        ls_result = CKYX_handle_req_msg_c401((cr401_def*)rcv_ipc);
        if (ls_result != CKYX_RET_OK){
            /* 処理なし */
        }
    }
    else if (memcmp(rcv_ipc->interface_code,
             DEF_IPC_IFCD_CTRL_MSG_REQ,
             sizeof(rcv_ipc->interface_code)) == 0) {
        /* C402 */
        g_request_trigger = CKYX_REQUEST_TRIGGER_GFP;
        ls_result = CKYX_handle_req_msg_c402((cr402_def*)rcv_ipc);
        if (ls_result != CKYX_RET_OK){
            /* 処理なし */
        }
    }
    else {
        /* IPC精査エラー */
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_IPC_SEISA_ERR ,
                            "@L@C@C@C",
                            "",
                            "",
                            rcv_ipc->interface_code,
                            "interface ");
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        /* IPCエラー設定 */
        rcv_ipc->error_code = DEF_IPC_ERRCD_NG;
        /* IPC内部エラーコード設定 */
        memcpy( rcv_ipc->internal_error_code,
                DEF_NERR_IPC_SEISA_ERR,
                sizeof(rcv_ipc->internal_error_code));
    }

    /* リプライ処理 */
    ls_result =  CKYX_reply();

} /* end of CMIN_handle_req_msg */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_handle_req_msg_c401                        */
/*  CALLING SEQ.    : short CKYX_handle_req_msg_c401 (cr401_def *c401_msg)   */
/*  ARGUMENT        : 1. c401_data       (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : C401受信処理                                           */
/*****************************************************************************/
short CKYX_handle_req_msg_c401(cr401_def *c401_msg)
{
    short   ls_result = CKYX_RET_OK;              /* WK処理結果              */

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    memcpy(g_internal_error_code, DEF_NERR_NOMAL, sizeof(g_internal_error_code));

    /* 局状態クリア */
    memset(g_station_sts,         CKYX_SPACE, sizeof(g_station_sts));

    memcpy(&g_rcv_con_info,
           &c401_msg->control_info.connection_lid,
           sizeof(g_rcv_con_info));

    /* 鍵種別にKPEを設定 */
    memcpy(g_key_kind, CKYX_KEY_DB_KPE, sizeof(g_key_kind));

    for(;;){
        ls_result = CKYX_all_file_read(&c401_msg->control_info.connection_lid);

        /* 読み込み結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            break;
        }

        /* 制御電文識別により処理を振分ける */
        if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_RESPONSE)&&
            (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC_REQ)){
            /* CTL_KIND x24x */
            /* 鍵交換依頼応答処理起動 */
            ls_result = CKYX_key_req_rsp(c401_msg);
        }
        else if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
            (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC_REQ)){
            /* CTL_KIND x14x */
            /* 鍵交換依頼応答(仕向応答タイムアウトまたは仕向要求送信不可)処理起動 */
            ls_result = CKYX_key_req_rsp(c401_msg);
        }
        else if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
                 (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC)){
            /* CTL_KIND x15x */
            if ((memcmp(c401_msg->control_info.request_kind,
                        DEF_CTLREQ_TIMEOUT,
                        sizeof(c401_msg->control_info.request_kind)) == 0) ||
                (memcmp(c401_msg->control_info.request_kind,
                        DEF_CTLREQ_SIMUKE_ERROR,
                        sizeof(c401_msg->control_info.request_kind)) == 0)){
                /* 仕向応答タイムアウトまたは仕向要求送信不可 */
                /* GFP契機鍵交換応答処理 (暗号鍵送信) 起動 */
                ls_result = CKYX_gfp_key_send_rsp(c401_msg);
            }
            else {
                /* 接続先契機鍵交換 (暗号鍵受信)起動 */
                ls_result = CKYX_key_exchange_req(c401_msg);
            }
        }
        else if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_RESPONSE)&&
                 (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC)){
            /* CTL_KIND x25x */
            if (memcmp(c401_msg->control_info.request_kind,
                       DEF_CTLREQ_HISIMUKE_ERROR,
                       sizeof(c401_msg->control_info.request_kind)) == 0){
                /* 被仕向応答送信不可 */
                /* 接続先契機鍵交換応答電文送信不可 (暗号鍵送信) */
                ls_result = CKYX_key_exchange_rsp_err(c401_msg);
            }
            else{
                /* 仕向応答 */
                /* GFP契機鍵交換応答処理 (暗号鍵送信) 起動 */
                ls_result = CKYX_gfp_key_send_rsp(c401_msg);
             }
        }
        else {
            /* IPC精査エラー */
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_IPC_SEISA_ERR ,
                                "@L@T@C@C",
                                c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                                &c401_msg->control_info.connection_lid,
                                "C401      ",
                                "controlkin");

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_IPC_SEISA_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
        }

        break;
    }

    /* R401作成処理 */
    ls_result = CKYX_r401_make(c401_msg, g_resp_kind);

    return ls_result;

} /* end of CKYX_handle_req_msg_c401 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_handle_req_msg_c402                        */
/*  CALLING SEQ.    : short CKYX_handle_req_msg_c402 (cr402_def *c402_msg)   */
/*  ARGUMENT        : 1. c402_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : C402受信処理                                           */
/*****************************************************************************/
short CKYX_handle_req_msg_c402(cr402_def *c402_msg)
{
    short   ls_result = CKYX_RET_OK;              /* WK処理結果              */
    short   req_kind  = CKYX_RET_NG;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    memcpy(g_internal_error_code, DEF_NERR_NOMAL, sizeof(g_internal_error_code));

    memcpy(&g_rcv_con_info,
           &c402_msg->control_info.connection_lid,
           sizeof(g_rcv_con_info));

    /* 鍵種別にKPEを設定 */
    memcpy(g_key_kind, CKYX_KEY_DB_KPE, sizeof(g_key_kind));

    for(;;){
        ls_result = CKYX_all_file_read(&c402_msg->control_info.connection_lid);

        /* 読み込み結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            break;
        }

        /* 制御電文識別により処理を振分ける */
        if ((c402_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
            (c402_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC_REQ)){
            /* CTL_KIND x14x */
            /* 鍵交換依頼要求処理起動 */
            ls_result = CKYX_key_exchange_make_req(c402_msg);
            req_kind = DEF_NWM_KYX_MSG_KIND_IRAIREQ;
        }
        else if ((c402_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
                 (c402_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC)){
            /* CTL_KIND x15x */
            /* GFP契機鍵交換要求処理 (暗号鍵送信)起動 */
            ls_result = CKYX_gfp_key_exchange_send_req(c402_msg);
            req_kind = DEF_NWM_KYX_MSG_KIND_KEYREQ;
        }
        else {
            /* IPC精査エラー */
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_IPC_SEISA_ERR ,
                                "@L@T@C@C",
                                c402_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                                &c402_msg->control_info.connection_lid,
                                "C402      ",
                                "controlkin");

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_IPC_SEISA_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
        }

        break;
    }

    /* R402作成処理 */
    ls_result = CKYX_r402_make(req_kind, c402_msg, g_resp_kind);

    return ls_result;
} /* end of CKYX_handle_req_msg_c402 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_exchange_make_req                      */
/*  CALLING SEQ.    : short CKYX_key_exchange_make_req (cr402_def *c402_msg) */
/*  ARGUMENT        : 1. c402_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 鍵交換依頼要求処理                                     */
/*****************************************************************************/
short CKYX_key_exchange_make_req(cr402_def *c402_msg)
{
    short   ls_result = CKYX_RET_OK;              /* WK処理結果              */

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* EMS出力 鍵交換依頼開始 */
    CMIN_message_output(DEF_EVT_KEY_EXCH_START,
                        DEF_MSGTTKB_GYOM_ERR,
                        DEF_NERR_NOMAL ,
                        "@L@T@C",
                        c402_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                        &c402_msg->control_info.connection_lid,
                        g_key_kind );

    /* GFP内部LCN取得処理 */
    ls_result = CKYX_get_lcn(g_lcn);

    /* 処理結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 仕向鍵交換LCN取得エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
//             DEF_NERR_SMK_LCN_GET_ERR,
               DEF_NERR_LCN_GET_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return ls_result;
} /* end of CKYX_key_exchange_make_req */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_req_rsp                                */
/*  CALLING SEQ.    : short CKYX_key_req_rsp (cr401_def *c401_msg)           */
/*  ARGUMENT        : 1. c401_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 鍵交換依頼応答処理(C401)                               */
/*****************************************************************************/
short CKYX_key_req_rsp(cr401_def *c401_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    char    rcv_message_text[MAX_TEXT_BUF_LEN];
    char    ch_wk_data[16];
    NWM_KYX_arg_2_def NWM_KYX_arg_2;

    memset(&NWM_KYX_arg_2, CKYX_SPACE, sizeof(NWM_KYX_arg_2));

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    memset(g_station_sts   , CKYX_SPACE, sizeof(g_station_sts));
    memset(rcv_message_text, 0         , sizeof(rcv_message_text)); 

    memcpy(rcv_message_text,
           c401_msg->data_bu.message_text, 
           sizeof(rcv_message_text));

    /* 送信不可時内部エラーコード設定 */
    memcpy(g_send_err_code, c401_msg->control_info.send_naibu_err_code, sizeof(g_send_err_code));

    /* C401の要求種別確認 */
    if ( memcmp(c401_msg->control_info.request_kind,
                DEF_CTLREQ_SIMUKE,
                sizeof(c401_msg->control_info.request_kind)) == 0){
        /* "20"：仕向応答 */
        memset(g_rcv_key       , CKYX_SPACE, sizeof(g_rcv_key));
        memset(g_rcv_checkdigit, CKYX_SPACE, sizeof(g_rcv_checkdigit));
        NWM_KYX_arg_2.key              = g_rcv_key;
        NWM_KYX_arg_2.checkdigit       = g_rcv_checkdigit;
        NWM_KYX_arg_2.key_leng         = 0;
        NWM_KYX_arg_2.checkdigit_leng  = 0;

        /* 鍵交換電文精査(応答受信) */
        ls_result = NWM_KYX_msg_check_rsp(DEF_NWM_KYX_MSG_KIND_REQRSP /* 要求種別                 */
                                        , rcv_message_text            /* 受信データ(応答電文)     */
                                        , c401_msg->control_info.denbun_len  /* 電文長            */
                                        , &g_nwi_g         /* NW情報レコード(グループ単位)        */
                                        , &g_nwi_i         /* NW情報レコード(インタフェース単位)  */
                                        , g_nws_n.dst_unq_info /* 接続先固有情報(NW単位)              */
                                        , g_nws_i.dst_unq_info /* 接続先固有情報(インタフェース単位)  */
                                        , g_nws_s.dst_unq_info /* 接続先固有情報(ステーション単位)    */
                                        , g_nws_c.dst_unq_info /* 接続先固有情報(コネクション単位)    */
                                        , &g_gckey         /* 鍵管理情報レコード                  */
                                        , &NWM_KYX_arg_2); /* 精査処理結果                        */

        /* 鍵交換電文精査結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK){
            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;
            if (ls_result == CKYX_RSP_KIND_REJ){
                /* 拒否応答 */
                /* 制御電文種別に拒否応答を設定 */
                c401_msg->control_info.control_kind.int_proc_kbn = DEF_CTLINT_DENY;
                /* 制御電文ログ出力処理 */
                ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                                CKYX_LOG_SET,
                                                CKYX_ERR_NASHI,
                                                g_internal_error_code);

                /* 制御電文ログ出力結果判定 */
                if ( ls_result != CKYX_RET_OK ){
                    /* 処理異常終了 */
                    /* 制御ログ出力エラー(処理継続) */
                }
            }
            else {
                /* 障害電文通知/電文破棄 */
                /* EMS出力 鍵交換電文精査エラー */
                memset(ch_wk_data, CKYX_ZERO, sizeof(ch_wk_data));
                memcpy(ch_wk_data,
                       c401_msg->control_info.mti,
                       sizeof(c401_msg->control_info.mti));
                CMIN_message_output(DEF_EVT_DATA_FLD_SEISA_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_SMK_SEISA_ERR,
                                    "@L@T@C@C",
                                    c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                                    &c401_msg->control_info.connection_lid,
                                    ch_wk_data,
                                    &NWM_KYX_arg_2.err_bit[1]);

                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_SMK_SEISA_ERR,
                       sizeof(g_internal_error_code));
            }
            return CKYX_RET_NG;
        }

        /* 制御電文種別に許可応答を設定 */
        c401_msg->control_info.control_kind.int_proc_kbn = DEF_CTLINT_ALLOW;

        /* 精査処理結果保存 */
        memcpy(&g_rsp_seisa_result, &NWM_KYX_arg_2, sizeof(g_rsp_seisa_result));
        memcpy(g_key_kind, g_rsp_seisa_result.key_kind, sizeof(g_key_kind));
        /* 鍵管理ファイル読込処理 */
        ls_result = CKYX_key_read(g_key_kind,
                                  DEF_COM_IOM_NOLOCK,
                                  CKYX_OWN_FILE_OWN_SITE,
                                  &c401_msg->control_info.connection_lid,
                                  &g_gckey);

        /* 読み込み結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 鍵管理ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_SET,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 局状態取得処理 */
        ls_result = CKYX_read_station_st(&c401_msg->control_info.connection_lid,
                                         g_station_sts);

        /* 局状態取得結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 局状態エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
        /* 鍵交換局状態チェック（応答受信）*/
        ls_result = NWM_KYX_cst_check_rsp(DEF_NWM_KYX_MSG_KIND_REQRSP, g_station_sts);

        /* 鍵交換局状態チェック（応答受信)結果判定 */
        if ( ls_result != DEF_NWM_KYX_RTN_OK ){
            /* EMS出力 (局状態エラー)*/
            CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_SMK_STATION_ST_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c401_msg->control_info.connection_lid,
                                g_station_sts);

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_SMK_STATION_ST_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;
            return CKYX_RET_NG;
        }

        /* EMS出力 鍵交換依頼終了 */
        CMIN_message_output(DEF_EVT_KEY_EXCH_END,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_NOMAL ,
                            "@L@T@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            DEF_NWM_KYX_KIND_KPE );

    }
    else if (memcmp(c401_msg->control_info.request_kind,
             DEF_CTLREQ_TIMEOUT,
             sizeof(c401_msg->control_info.request_kind)) == 0){
        /* "30"：仕向応答タイムアウト */
        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_SET,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }
    else if(memcmp(c401_msg->control_info.request_kind,
              DEF_CTLREQ_SIMUKE_ERROR,
              sizeof(c401_msg->control_info.request_kind)) == 0) {
        /* "40"：仕向要求送信不可 */
        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_UPDATE,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }
    else {
        /* IPC精査エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_SEISA_ERR,
               sizeof(g_internal_error_code));
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_IPC_SEISA_ERR ,
                            "@L@T@C@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            "C401      ",
                            "req_kind  ");
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;
} /* end of CKYX_key_req_rsp */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_exchange_req                           */
/*  CALLING SEQ.    : short CKYX_key_exchange_req (cr401_def *c401_msg)      */
/*  ARGUMENT        : 1. c401_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 接続先契機鍵交換要求処理(C401)                         */
/*****************************************************************************/
short CKYX_key_exchange_req(cr401_def *c401_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    char    rcv_message_text[MAX_TEXT_BUF_LEN];
    char    ch_wk_data[16];
    NWM_KYX_arg_1_def NWM_KYX_arg_1;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    memset(g_station_sts,          CKYX_SPACE, sizeof(g_station_sts));
    memset((char *)&NWM_KYX_arg_1, CKYX_SPACE, sizeof(NWM_KYX_arg_1));
    memset(rcv_message_text      , 0         , sizeof(rcv_message_text)); 

    memcpy(rcv_message_text,
           c401_msg->data_bu.message_text, 
           sizeof(rcv_message_text));

    /* C401の要求種別確認 */
    if ( memcmp(c401_msg->control_info.request_kind,
                DEF_CTLREQ_HISIMUKE,
                sizeof(c401_msg->control_info.request_kind)) == 0){
        /* "10"：被仕向要求 */
        memset(g_rcv_key       , CKYX_SPACE, sizeof(g_rcv_key));
        memset(g_rcv_checkdigit, CKYX_SPACE, sizeof(g_rcv_checkdigit));
        NWM_KYX_arg_1.key              = g_rcv_key;
        NWM_KYX_arg_1.checkdigit       = g_rcv_checkdigit;
        NWM_KYX_arg_1.key_leng         = 0;
        NWM_KYX_arg_1.checkdigit_leng  = 0;

        /* GFP内部LCN格納 */
        memcpy(g_lcn,
               c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
               sizeof(g_lcn));

        /* 鍵交換電文精査(要求受信) */
        ls_result = NWM_KYX_msg_check_req(rcv_message_text     /* 受信データ(要求電文)                */
                                        , c401_msg->control_info.denbun_len  /* 電文長                */
                                        , &g_nwi_g             /* NW情報レコード(グループ単位)        */
                                        , &g_nwi_i             /* NW情報レコード(インタフェース単位)  */
                                        , g_nws_n.dst_unq_info /* 接続先固有情報(NW単位)              */
                                        , g_nws_i.dst_unq_info /* 接続先固有情報(インタフェース単位)  */
                                        , g_nws_s.dst_unq_info /* 接続先固有情報(ステーション単位)    */
                                        , g_nws_c.dst_unq_info /* 接続先固有情報(コネクション単位)    */
                                        , &g_gckey             /* 鍵管理情報レコード                  */
                                        , &NWM_KYX_arg_1);     /* 精査処理結果                        */

        /* 鍵交換電文精査結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK){
            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;

            if (ls_result == DEF_NWM_KYX_RTN_NG_REJ) {
                /* 拒否応答 */
                /* 制御電文ログ出力処理 */
                ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                               CKYX_LOG_SET,
                                               CKYX_ERR_NASHI,
                                               g_internal_error_code);
                /* チェックディジット精査エラー */
                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_CHK_DIGIT_ERR,
                       sizeof(g_internal_error_code));
            }
            else {
                /* 障害電文通知/電文破棄 */
                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_HSMK_SEISA_ERR,
                       sizeof(g_internal_error_code));
            }
            /* EMS出力 鍵交換電文精査エラー */
            memset(ch_wk_data, CKYX_ZERO, sizeof(ch_wk_data));
            memcpy(ch_wk_data,
                   c401_msg->control_info.mti,
                   sizeof(c401_msg->control_info.mti));
            CMIN_message_output(DEF_EVT_DATA_FLD_SEISA_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                g_internal_error_code,
                                "@L@T@C@C",
                                c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                                &c401_msg->control_info.connection_lid,
                                ch_wk_data,
                                &NWM_KYX_arg_1.err_bit[1]);
            return CKYX_RET_NG;
        }

        /* 精査処理結果保存 */
        memcpy(&g_req_seisa_result, &NWM_KYX_arg_1, sizeof(g_rsp_seisa_result));
        memcpy(g_key_kind, g_req_seisa_result.key_kind, sizeof(g_key_kind));
        memcpy(g_atalla_info.key_usage_11B, NWM_KYX_arg_1.key_usage, sizeof(g_atalla_info.key_usage_11B));

        /* 鍵管理ファイル読込処理 */
        ls_result = CKYX_key_read(g_key_kind,
                                  DEF_COM_IOM_NOLOCK,
                                  CKYX_OWN_FILE_OWN_SITE,
                                  &c401_msg->control_info.connection_lid,
                                  &g_gckey);

        /* 読み込み結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 鍵管理ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                       CKYX_LOG_SET,
                                       CKYX_ERR_NASHI,
                                       g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* EMS出力 鍵交換開始 */
        CMIN_message_output(DEF_EVT_KEY_EXCH_START,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_NOMAL ,
                            "@L@T@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            g_key_kind );

        /* 局状態取得処理 */
        ls_result = CKYX_read_station_st(&c401_msg->control_info.connection_lid,
                                         g_station_sts);

        /* 局状態取得結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 接続先固有情報ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
        /* 鍵交換局状態チェック（要求受信）*/
        ls_result = NWM_KYX_cst_check_req(g_station_sts);

        /* 鍵交換局状態チェック（要求受信)結果判定 */
        if ( ls_result != DEF_NWM_KYX_RTN_OK ){
            /* EMS出力 (局状態エラー)*/
            CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_HSMK_STATION_ST_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c401_msg->control_info.connection_lid,
                                g_station_sts);

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_HSMK_STATION_ST_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;
            return CKYX_RET_NG;
        }

        /* 鍵復号処理 */
        ls_result = CKYX_key_cmd_dec(c401_msg);

        /* 復号処理結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK ){
            return CKYX_RET_NG;
        }

        /* 鍵管理ファイル更新処理 */
        ls_result = CKYX_key_file_update(CKYX_KEY_IDX_UPD_YES);

        /* 処理結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK ){
            return CKYX_RET_NG;
        }

        /* EMS出力 鍵交換終了 */
        CMIN_message_output(DEF_EVT_KEY_EXCH_END,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_NOMAL ,
                            "@L@T@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            &g_req_seisa_result.key_kind );

    }
    else {
        /* IPC精査エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_SEISA_ERR,
               sizeof(g_internal_error_code));
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_IPC_SEISA_ERR ,
                            "@L@T@C@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            "C401      ",
                            "req_kind  ");
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;
} /* end of CKYX_key_exchange_req */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_exchange_rsp_err                       */
/*  CALLING SEQ.    : short CKYX_key_exchange_rsp_err (cr401_def *c401_msg)  */
/*  ARGUMENT        : 1. c401_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 接続先契機鍵交換応答電文送信不可(C401)                 */
/*****************************************************************************/
short CKYX_key_exchange_rsp_err(cr401_def *c401_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* 制御電文ログ出力処理 */
    ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_UPDATE,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

    /* 制御電文ログ出力結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* 制御ログ出力エラー */

        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_CTL_LOG_OUTPUT_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 鍵管理ファイル入れ替え処理(鍵情報を旧情報に戻す) */
    ls_result = CKYX_key_file_num_chg();

    /* 処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;
} /* end of CKYX_key_exchange_rsp_err */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_gfp_key_exchange_send_req                  */
/*  CALLING SEQ.    : short CKYX_gfp_key_exchange_send_req                   */
/*                                         (cr402_def *c402_msg)             */
/*  ARGUMENT        : 1. c402_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : GFP契機鍵交換要求処理 (暗号鍵送信)(C402)               */
/*****************************************************************************/
short CKYX_gfp_key_exchange_send_req(cr402_def *c402_msg)
{
    short   ls_result = CKYX_RET_OK;              /* WK処理結果              */

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* EMS出力 鍵交換開始 */
    CMIN_message_output(DEF_EVT_KEY_EXCH_START,
                        DEF_MSGTTKB_GYOM_ERR,
                        DEF_NERR_NOMAL ,
                        "@L@T@C",
                        c402_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                        &c402_msg->control_info.connection_lid,
                        g_key_kind );

    /* GFP内部LCN取得処理 */
    ls_result = CKYX_get_lcn(g_lcn);

    /* 処理結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 仕向鍵交換LCN取得エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
//             DEF_NERR_SMK_LCN_GET_ERR,
               DEF_NERR_LCN_GET_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 鍵生成&暗号化処理 */
    ls_result = CKYX_key_cmd_enc(c402_msg);

    /* 鍵生成&暗号化処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        return CKYX_RET_NG;
    }

    /* 鍵管理ファイル更新処理 */
    ls_result = CKYX_key_file_update(CKYX_KEY_IDX_UPD_NO);

    /* 処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        return CKYX_RET_NG;
    }


    return ls_result;
} /* end of CKYX_gfp_key_exchange_send_req */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_gfp_key_send_rsp                           */
/*  CALLING SEQ.    : short CKYX_gfp_key_send_rsp (cr401_def *c401_msg)      */
/*  ARGUMENT        : 1. c401_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : GFP契機鍵交換応答処理 (暗号鍵送信)(C401)               */
/*****************************************************************************/
short CKYX_gfp_key_send_rsp(cr401_def *c401_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    char    rcv_message_text[MAX_TEXT_BUF_LEN];
    char    ch_wk_data[16];
    NWM_KYX_arg_2_def  NWM_KYX_arg_2;

    memset((char *)&NWM_KYX_arg_2, CKYX_SPACE, sizeof(NWM_KYX_arg_2));

    g_resp_kind = CKYX_RSP_KIND_NORMAL;
    memset(g_station_sts   , CKYX_SPACE, sizeof(g_station_sts));
    memset(rcv_message_text, 0         , sizeof(rcv_message_text)); 

    memcpy(rcv_message_text,
           c401_msg->data_bu.message_text, 
           sizeof(rcv_message_text));

    /* C401の要求種別確認 */
    if ( memcmp(c401_msg->control_info.request_kind,
                DEF_CTLREQ_SIMUKE,
                sizeof(c401_msg->control_info.request_kind)) == 0){
        /* "20"：仕向応答 */

        memset(g_rcv_key       , CKYX_SPACE, sizeof(g_rcv_key));
        memset(g_rcv_checkdigit, CKYX_SPACE, sizeof(g_rcv_checkdigit));
        NWM_KYX_arg_2.key              = g_rcv_key;
        NWM_KYX_arg_2.checkdigit       = g_rcv_checkdigit;
        NWM_KYX_arg_2.key_leng         = 0;
        NWM_KYX_arg_2.checkdigit_leng  = 0;

        /* 鍵交換電文精査(応答受信) */
        ls_result = NWM_KYX_msg_check_rsp(DEF_NWM_KYX_MSG_KIND_KEYRSP        /* 要求種別              */
                                        , rcv_message_text     /* 受信データ(応答電文)                */
                                        , c401_msg->control_info.denbun_len  /* 電文長                */
                                        , &g_nwi_g             /* NW情報レコード(グループ単位)        */
                                        , &g_nwi_i             /* NW情報レコード(インタフェース単位)  */
                                        , g_nws_n.dst_unq_info /* 接続先固有情報(NW単位)              */
                                        , g_nws_i.dst_unq_info /* 接続先固有情報(インタフェース単位)  */
                                        , g_nws_s.dst_unq_info /* 接続先固有情報(ステーション単位)    */
                                        , g_nws_c.dst_unq_info /* 接続先固有情報(コネクション単位)    */
                                        , &g_gckey             /* 鍵管理情報レコード                  */
                                        , &NWM_KYX_arg_2);     /* 精査処理結果                        */

        /* 鍵交換電文精査結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK){
            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;
            if (ls_result == CKYX_RSP_KIND_REJ){
                /* 拒否応答 */
                /* 制御電文種別に拒否応答を設定 */
                c401_msg->control_info.control_kind.int_proc_kbn = DEF_CTLINT_DENY;
                /* 制御電文ログ出力処理 */
                ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                                CKYX_LOG_SET,
                                                CKYX_ERR_NASHI,
                                                g_internal_error_code);

                /* 制御電文ログ出力結果判定 */
                if ( ls_result != CKYX_RET_OK ){
                    /* 処理異常終了 */
                    /* 制御ログ出力エラー(処理継続) */
                }
            }
            else {
                /* 障害電文通知/電文破棄 */
                /* EMS出力 鍵交換電文精査エラー */
                memset(ch_wk_data, CKYX_ZERO, sizeof(ch_wk_data));
                memcpy(ch_wk_data,
                       c401_msg->control_info.mti,
                       sizeof(c401_msg->control_info.mti));
                CMIN_message_output(DEF_EVT_DATA_FLD_SEISA_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_SMK_SEISA_ERR,
                                    "@L@T@C@C",
                                    c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                                    &c401_msg->control_info.connection_lid,
                                    ch_wk_data,
                                    &NWM_KYX_arg_2.err_bit[1]);

                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_SMK_SEISA_ERR,
                       sizeof(g_internal_error_code));
            }
            return CKYX_RET_NG;
        }

        /* 制御電文種別に許可応答を設定 */
        c401_msg->control_info.control_kind.int_proc_kbn = DEF_CTLINT_ALLOW;

        /* 精査処理結果保存 */
        memcpy(&g_rsp_seisa_result, &NWM_KYX_arg_2, sizeof(g_rsp_seisa_result));
        /* キー種別格納 */
        memcpy(g_key_kind, g_rsp_seisa_result.key_kind, sizeof(g_key_kind));

        /* 鍵管理ファイル読込処理 */
        ls_result = CKYX_key_read(g_key_kind,
                                  DEF_COM_IOM_NOLOCK,
                                  CKYX_OWN_FILE_OWN_SITE,
                                  &c401_msg->control_info.connection_lid,
                                  &g_gckey);

        /* 読み込み結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 鍵管理ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 局状態取得処理 */
        ls_result = CKYX_read_station_st(&c401_msg->control_info.connection_lid,
                                         g_station_sts);

        /* 局状態取得結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 接続先固有情報ファイル読込エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
        /* 鍵交換局状態チェック（応答受信）*/
        ls_result = NWM_KYX_cst_check_rsp(DEF_NWM_KYX_MSG_KIND_KEYRSP, g_station_sts);

        /* 鍵交換局状態チェック（応答受信)結果判定 */
        if ( ls_result != DEF_NWM_KYX_RTN_OK ){
            /* EMS出力 (局状態エラー)*/
            CMIN_message_output(DEF_EVT_KYOKU_STS_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_SMK_STATION_ST_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c401_msg->control_info.connection_lid,
                                g_station_sts);

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_SMK_STATION_ST_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別に戻り値を設定 */
            g_resp_kind = ls_result;
            return CKYX_RET_NG;
        }

        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_SET,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 鍵管理ファイル入れ替え処理(鍵情報を旧情報に戻す) */
        ls_result = CKYX_key_file_num_chg();

        /* 処理結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK ){
            return CKYX_RET_NG;
        }

        /* EMS出力 鍵交換終了 */
        CMIN_message_output(DEF_EVT_KEY_EXCH_END,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_NOMAL ,
                            "@L@T@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            &g_rsp_seisa_result.key_kind );
    }
    else if (memcmp(c401_msg->control_info.request_kind,
             DEF_CTLREQ_TIMEOUT,
             sizeof(c401_msg->control_info.request_kind)) == 0){
        /* "30"：仕向応答タイムアウト */
        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_SET,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }
    else if (memcmp(c401_msg->control_info.request_kind,
             DEF_CTLREQ_SIMUKE_ERROR,
             sizeof(c401_msg->control_info.request_kind)) == 0) {
        /* "40"：仕向要求送信不可 */
        /* 制御電文ログ出力処理 */
        ls_result = CKYX_ctl_log_output((char*)c401_msg,
                                        CKYX_LOG_UPDATE,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);

        /* 制御電文ログ出力結果判定 */
        if ( ls_result != CKYX_RET_OK ){
            /* 処理異常終了 */
            /* 制御ログ出力エラー */
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_CTL_LOG_OUTPUT_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }
    else {
        /* IPC精査エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_SEISA_ERR,
               sizeof(g_internal_error_code));
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_HEADR_SEISA_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SMK_SEISA_ERR ,
                            "@L@T@C@C",
                            c401_msg->control_info.denbun_log_key.tran_id.gfp_lcn,
                            &c401_msg->control_info.connection_lid,
                            "C401      ",
                            "req_kind  ");
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_gfp_key_send_rsp */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_cmd_dec                                */
/*  CALLING SEQ.    : short CKYX_key_cmd_dec (cr401_def *c401_msg)           */
/*  ARGUMENT        : 1. c401_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 鍵復号処理                                             */
/*****************************************************************************/
short CKYX_key_cmd_dec(cr401_def *c401_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    char    atalla_msg[8192];
    char    atalla_rsp[8192];
    short   atalla_msg_len   = 0;
    short   chkdigit_chk_len = 0;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* ATALLAコマンド作成 */
    ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_DEC, atalla_msg, &atalla_msg_len);
    /* 正常リターンのみ */

    /* ATALLAコマンドPATHSEND処理 */
    ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != CKYX_RET_OK ){
        /* EMS出力 (ATALLA応答エラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_HSMK_DECODE_ERR,
                            "@L@T@C",
                            g_lcn,
                            &c401_msg->control_info.connection_lid,
                            "ATALLA_RSP_ERR");

        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_HSMK_DECODE_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別に拒否応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_REJ;
        return CKYX_RET_NG;
    }

    /* ATALLA応答データ解析処理 */
    ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_DEC, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        /* 応答種別に拒否応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_REJ;
        return CKYX_RET_NG;
    }

    /* チェックディジット生成要不要判定 */
    if (g_key_info_save.chk_digit_len != CKYX_CHKDIGIT_LEN){
        /* チェックディジット長が6でなければ生成する */
        memset(atalla_msg, 0, sizeof(atalla_msg));
        memset(atalla_rsp, 0, sizeof(atalla_rsp));

        /* チェックディジット6桁化 */
        /* ATALLAコマンド作成(チェックディジット生成) */
        ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_CKDIGIT,atalla_msg, &atalla_msg_len );
        /* 正常リターンのみ */

        /* ATALLAコマンドPATHSEND処理 */
        ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

        /* 処理結果判定 */
        if (ls_result != CKYX_RET_OK ){
            /* EMS出力 (ATALLA応答エラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_HSMK_DECODE_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c401_msg->control_info.connection_lid,
                                "");

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_HSMK_DECODE_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別に拒否応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_REJ;
            return CKYX_RET_NG;
        }

        /* ATALLA応答データ解析処理 */
        ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_CKDIGIT, atalla_rsp);

        /* 処理結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK ){
            /* EMS出力 (ATALLA応答エラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_HSMK_DECODE_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c401_msg->control_info.connection_lid,
                                "");

            /* 応答種別に拒否応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_REJ;
            return CKYX_RET_NG;
        }
    }

    /* Check Digits判定 */
    if (g_req_seisa_result.checkdigit_leng >= g_key_info_save.chk_digit_len){
        chkdigit_chk_len = g_key_info_save.chk_digit_len;
    }
    else{
        chkdigit_chk_len = g_req_seisa_result.checkdigit_leng;
    }

    if (memcmp(g_key_info_save.chk_digit,
               g_req_seisa_result.checkdigit,
               chkdigit_chk_len)  != 0) {
        /* EMS出力 (チェックデジットエラー) */
        CMIN_message_output(DEF_EVT_CHECK_DIGIT,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_CHK_DIGIT_ERR,
                            "@L@T@H@H",
                            g_lcn,
                            &g_rcv_con_info,
                            g_key_info_save.chk_digit,
                            g_req_seisa_result.checkdigit);
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_CHK_DIGIT_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別に拒否応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_REJ;

        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_key_cmd_dec */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_cmd_enc                                */
/*  CALLING SEQ.    : short CKYX_key_cmd_enc (cr402_def *c402_msg)           */
/*  ARGUMENT        : 1. c402_msg        (I) 受信IPC                         */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 暗号鍵生成＆暗号化処理                                 */
/*****************************************************************************/
short CKYX_key_cmd_enc(cr402_def *c402_msg)
{
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    char    atalla_msg[8192];
    char    atalla_rsp[8192];
    short   atalla_msg_len = 0;
    char    key_format_save[2];                      /* g_gckey.key_info.key_format退避用 */
    key_info_save_def    key_info_save;              /* 生成した暗号鍵情報保存領域  */

    memset(g_save_checkdigit, 0x00,sizeof(g_save_checkdigit));
    g_save_checkdigit_len = 0;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    memset(atalla_msg, 0, sizeof(atalla_msg));
    memset(atalla_rsp, 0, sizeof(atalla_rsp));

    /* ATALLAコマンド作成(鍵生成) */
    ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_MAKE, atalla_msg, &atalla_msg_len);
    /* 正常リターンのみ */

    /* ATALLAコマンドPATHSEND処理 */
    ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != CKYX_RET_OK ){
        /* EMS出力 (ATALLA応答エラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SMK_KEY_ENC_ERR,
                            "@L@T@C",
                            g_lcn,
                            &c402_msg->control_info.connection_lid,
                            "");

        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別に異常を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* ATALLA応答データ解析処理 */
    ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_MAKE, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        /* EMS出力 (ATALLA応答エラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SMK_KEY_ENC_ERR,
                            "@L@T@C",
                            g_lcn,
                            &c402_msg->control_info.connection_lid,
                            "");

        /* 応答種別に異常を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* Exportability用処理 */
    /* g_gckey.key_info.exportabilityが'S'の場合、Exportability用処理を行う */
    if (g_gckey.key_info.exportability == CKYX_KEY_EXPORT_S){

       /* 生成した暗号鍵情報保存(20で取得した内容) */
       memcpy(&key_info_save,&g_key_info_save,sizeof(g_key_info_save));

       /* 鍵管理ファイル読込処理(内部変換用) */
       ls_result = CKYX_key_read_internal(g_key_kind,
                                          DEF_COM_IOM_NOLOCK,
                                          CKYX_OWN_FILE_OWN_SITE,
                                          &c402_msg->control_info.connection_lid,
                                          &g_gckey_conv);

       /* 読み込み結果判定 */
       if ( ls_result != CKYX_RET_OK ){
           /* 処理異常終了 */
           /* 鍵管理ファイル読込エラー */
           /* 内部エラーコード設定 */
           memcpy(g_internal_error_code,
                  DEF_NERR_FILE_IO_ERR,
                  sizeof(g_internal_error_code));

           /* 応答種別にエラー応答を設定 */
           g_resp_kind = CKYX_RSP_KIND_ERR;
           return CKYX_RET_NG;
       }

       /* ATALLALコマンド発行 【Command 1A】ANSI X9.17形式に変換 */
       memset(atalla_msg, 0, sizeof(atalla_msg));
       memset(atalla_rsp, 0, sizeof(atalla_rsp));

       /* Command 1Aを使うためg_gckey.key_info.key_formatの値をスペースからVAに変更 */
       memcpy(key_format_save,g_gckey.key_info.key_format,sizeof(g_gckey.key_info.key_format));
       memcpy(g_gckey.key_info.key_format, CKYX_KEY_FMT_VA, sizeof(g_gckey.key_info.key_format));

       /* ATALLAコマンド作成(鍵生成) */
       ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_ENC, atalla_msg, &atalla_msg_len);
       /* 正常リターンのみ */

       /* ATALLAコマンドPATHSEND処理 */
       ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

       /* 処理結果判定 */
       if (ls_result != CKYX_RET_OK ){
           /* EMS出力 (ATALLA応答エラー)*/
           CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                               DEF_MSGTTKB_GYOM_ERR,
                               DEF_NERR_SMK_KEY_ENC_ERR,
                               "@L@T@C",
                               g_lcn,
                               &c402_msg->control_info.connection_lid,
                               "");

           /* 内部エラーコード設定 */
           memcpy(g_internal_error_code,
                  DEF_NERR_SMK_KEY_ENC_ERR,
                  sizeof(g_internal_error_code));

           /* 応答種別に異常を設定 */
           g_resp_kind = CKYX_RSP_KIND_ERR;
           return CKYX_RET_NG;
       }

       /* ATALLA応答データ解析処理 */
       ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_ENC, atalla_rsp);

       /* 処理結果判定 */
       if (ls_result != DEF_NWM_KYX_RTN_OK ){
           /* EMS出力 (ATALLA応答エラー)*/
           CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                               DEF_MSGTTKB_GYOM_ERR,
                               DEF_NERR_SMK_KEY_ENC_ERR,
                               "@L@T@C",
                               g_lcn,
                               &c402_msg->control_info.connection_lid,
                               "");

           /* 応答種別に異常を設定 */
           g_resp_kind = CKYX_RSP_KIND_ERR;
           return CKYX_RET_NG;
       }

       /* チェックデジット退避 */
       memcpy(g_save_checkdigit, g_key_info_save.chk_digit, sizeof(g_key_info_save.chk_digit));
       g_save_checkdigit_len = g_key_info_save.chk_digit_len;

       /* ATALLALコマンド発行 【Command 11B】AKB形式(Exportability S)に変換 */
       memset(atalla_msg, 0, sizeof(atalla_msg));
       memset(atalla_rsp, 0, sizeof(atalla_rsp));

       /* Command 11Bを使うためg_gckey.key_info.key_formatの値をスペースからVAに変更(1Aの処理で設定済だが再度設定) */
       memcpy(g_gckey.key_info.key_format, CKYX_KEY_FMT_VA, sizeof(g_gckey.key_info.key_format));

       /* Key Usage 設定 スペースを判定し'0'が編集される */
       g_atalla_info.key_usage_11B[0] = CKYX_SPACE;

       /* ATALLAコマンド作成(鍵生成) */
       ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_DEC, atalla_msg, &atalla_msg_len);
       /* 正常リターンのみ */

       /* ATALLAコマンドPATHSEND処理 */
       ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

       /* 処理結果判定 */
       if (ls_result != CKYX_RET_OK ){
           /* EMS出力 (ATALLA応答エラー)*/
           CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                               DEF_MSGTTKB_GYOM_ERR,
                               DEF_NERR_HSMK_DECODE_ERR,
                               "@L@T@C",
                               g_lcn,
                               &c402_msg->control_info.connection_lid,
                               "");

           /* 内部エラーコード設定 */
           memcpy(g_internal_error_code,
                  DEF_NERR_SMK_KEY_ENC_ERR,
                  sizeof(g_internal_error_code));

           /* 応答種別に異常を設定 */
           g_resp_kind = CKYX_RSP_KIND_ERR;
           return CKYX_RET_NG;
       }

       /* ATALLA応答データ解析処理 */
       ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_DEC, atalla_rsp);

       /* 処理結果判定 */
       if (ls_result != DEF_NWM_KYX_RTN_OK ){
           /* EMS出力 (ATALLA応答エラー)*/
           CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                               DEF_MSGTTKB_GYOM_ERR,
                               DEF_NERR_HSMK_DECODE_ERR,
                               "@L@T@C",
                               g_lcn,
                               &c402_msg->control_info.connection_lid,
                               "");

           /* 応答種別に異常を設定 */
           g_resp_kind = CKYX_RSP_KIND_ERR;
           return CKYX_RET_NG;
       }

       /* g_gckey.key_info.key_formatの値を元に戻す */
       memcpy(g_gckey.key_info.key_format, key_format_save, sizeof(g_gckey.key_info.key_format));

       /* 生成した暗号鍵情報を元に戻す(20で取得した内容) */
       memcpy(&g_key_info_save.file_set_wk_key,
              &key_info_save.file_set_wk_key,
              sizeof(g_key_info_save.file_set_wk_key));
       g_key_info_save.file_set_wk_key_len = key_info_save.file_set_wk_key_len;

    }  /* Exportability用処理 end */

    memset(atalla_msg, 0, sizeof(atalla_msg));
    memset(atalla_rsp, 0, sizeof(atalla_rsp));

    /* ATALLAコマンド作成(鍵暗号化) */
    ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_ENC,atalla_msg, &atalla_msg_len );
    /* 正常リターンのみ */

    /* ATALLAコマンドPATHSEND処理 */
    ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != CKYX_RET_OK ){
        /* EMS出力 (ATALLA応答エラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SMK_KEY_ENC_ERR,
                            "@L@T@C",
                            g_lcn,
                            &c402_msg->control_info.connection_lid,
                            "");

        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別に異常を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* ATALLA応答データ解析処理 */
    ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_ENC, atalla_rsp);

    /* 処理結果判定 */
    if (ls_result != DEF_NWM_KYX_RTN_OK ){
        /* EMS出力 (ATALLA応答エラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SMK_KEY_ENC_ERR,
                            "@L@T@C",
                            g_lcn,
                            &c402_msg->control_info.connection_lid,
                            "");

        /* 応答種別に異常を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* g_gckey.key_info.exportabilityが'E'の場合、チェックデジットを戻す */
    if ((g_gckey.key_info.exportability == CKYX_KEY_EXPORT_S)&&
        (g_request_trigger == CKYX_REQUEST_TRIGGER_GFP)){
        memcpy(g_key_info_save.chk_digit, g_save_checkdigit, sizeof(g_key_info_save.chk_digit));
        g_key_info_save.chk_digit_len = g_save_checkdigit_len;
    }

    /* チェックディジット生成要不要判定 */
    if (g_key_info_save.chk_digit_len != CKYX_CHKDIGIT_LEN){
        /* チェックディジット長が6でなければ生成する */
        memset(atalla_msg, 0, sizeof(atalla_msg));
        memset(atalla_rsp, 0, sizeof(atalla_rsp));

        /* チェックディジット6桁化 */
        /* ATALLAコマンド作成(チェックディジット生成) */
        ls_result = CKYX_atalla_cmd_make(CKYX_CMD_KEY_CKDIGIT,atalla_msg, &atalla_msg_len );
        /* 正常リターンのみ */

        /* ATALLAコマンドPATHSEND処理 */
        ls_result = CKYX_atalla_pathsend(atalla_msg, atalla_msg_len, atalla_rsp);

        /* 処理結果判定 */
        if (ls_result != CKYX_RET_OK ){
            /* EMS出力 (ATALLA応答エラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_SMK_KEY_ENC_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c402_msg->control_info.connection_lid,
                                "");

            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_SMK_KEY_ENC_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別に異常を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* ATALLA応答データ解析処理 */
        ls_result = CKYX_atalla_rsp_chk(CKYX_CMD_KEY_CKDIGIT, atalla_rsp);

        /* 処理結果判定 */
        if (ls_result != DEF_NWM_KYX_RTN_OK ){
            /* EMS出力 (ATALLA応答エラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_SMK_KEY_ENC_ERR,
                                "@L@T@C",
                                g_lcn,
                                &c402_msg->control_info.connection_lid,
                                "");

            /* 応答種別に異常を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }

    return CKYX_RET_OK;

} /* end of CKYX_key_cmd_enc */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_cmd_make                            */
/*  CALLING SEQ.    : short CKYX_atalla_cmd_make (short cmd_type)            */
/*  ARGUMENT        : 1. cmd_type        (I) コマンド種別                    */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : ATALLAコマンド作成処理                                 */
/*****************************************************************************/
short CKYX_atalla_cmd_make(short cmd_type,
                           char  *atalla_msg,
                           short *atalla_msg_len)
{
    char    ch_key_fmt[2];

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* キーフォーマット取得 */
    memcpy(ch_key_fmt, g_gckey.key_info.key_format, sizeof(ch_key_fmt));

    /* コマンド種別で処理振分 */
    if ( cmd_type == CKYX_CMD_KEY_DEC ){
        /* 鍵復号 */
        /* 鍵管理情報ファイルのキーフォーマット判定 */
        if (memcmp(ch_key_fmt, CKYX_KEY_FMT_VA, sizeof(ch_key_fmt)) == 0){
            /* Command 11B 作成 */
            CKYX_atalla_make_11B(atalla_msg, atalla_msg_len);
        }
        else{
            /* Command 119 作成 */
            CKYX_atalla_make_119(atalla_msg, atalla_msg_len);
        }
    }
    else if ( cmd_type == CKYX_CMD_KEY_MAKE ){
        /* 鍵生成 */
        /* Command 10 作成 */
        CKYX_atalla_make_10(atalla_msg, atalla_msg_len);
    }
    else if ( cmd_type == CKYX_CMD_KEY_ENC ){
        /* 鍵暗号化 */
        /* 鍵管理情報ファイルのキーフォーマット判定 */
        if (memcmp(ch_key_fmt, CKYX_KEY_FMT_VA, sizeof(ch_key_fmt)) == 0){
            /* Command 1A 作成 */
            CKYX_atalla_make_1A(atalla_msg, atalla_msg_len);

        }
        else{
            /* Command 11A 作成 */
            CKYX_atalla_make_11A(atalla_msg, atalla_msg_len);
        }
    }
    else if ( cmd_type == CKYX_CMD_KEY_CKDIGIT ){
        /* チェックディジット生成 */
        /* Command 7E 作成 */
        CKYX_atalla_make_7E(atalla_msg, atalla_msg_len);
    }

    return CKYX_RET_OK;

} /* end of CKYX_atalla_cmd_make */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_10                             */
/*  CALLING SEQ.    : short CKYX_atalla_make_10 (void)                       */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 10 作成  暗号鍵の生成                          */
/*****************************************************************************/
short CKYX_atalla_make_10(char *atalla_msg,
                          short *atalla_msg_len)
{
    short   cmd_cnt      = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    memset((char*)&g_atalla_info, 0, sizeof(g_atalla_info));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_10, sizeof(CKYX_ATALLA_CMD_10));

    /* # 設定 */
    command_msg[2] = '#';
    cmd_cnt = 3;

    /* Key Usage 設定 */
    memcpy(g_atalla_info.key_use_10, CKYX_ATALLA_1PUNE, sizeof(g_atalla_info.key_use_10));
//  g_atalla_info.key_use_10[4] = g_gckey.key_info.exportability;
    memcpy(&command_msg[cmd_cnt], g_atalla_info.key_use_10, 8);
    /* # 設定 */
    cmd_cnt = cmd_cnt + 8;
    command_msg[cmd_cnt] = '#';
    cmd_cnt += 1;

    /* Key Exchange Key 設定 (省略) */
    /* # 設定 */
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Key Length 設定 */
    if (memcmp(g_gckey.key_info.key_use_alg,
               CKYX_KEY_DS1,
               sizeof(g_gckey.key_info.key_use_alg)) == 0){
        command_msg[cmd_cnt] = CKYX_KEY_ALG_S;
        cmd_cnt = cmd_cnt + 1;
        command_msg[cmd_cnt] = '#';
    }
    else if (memcmp(g_gckey.key_info.key_use_alg,
               CKYX_KEY_DS2,
               sizeof(g_gckey.key_info.key_use_alg)) == 0){
        command_msg[cmd_cnt] = CKYX_KEY_ALG_D;
        cmd_cnt = cmd_cnt + 1;
        command_msg[cmd_cnt] = '#';
    }
    else if (memcmp(g_gckey.key_info.key_use_alg,
               CKYX_KEY_DS3,
               sizeof(g_gckey.key_info.key_use_alg)) == 0){
        command_msg[cmd_cnt] = CKYX_KEY_ALG_T;
        cmd_cnt = cmd_cnt + 1;
        command_msg[cmd_cnt] = '#';
    }

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_10 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_1A                             */
/*  CALLING SEQ.    : short CKYX_atalla_make_1A (void)                       */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 1A 作成 暗号鍵の暗号化(出力形式：ANSI X9.17)   */
/*****************************************************************************/
short CKYX_atalla_make_1A(char *atalla_msg,
                          short *atalla_msg_len)
{
    short   cmd_cnt      = 0;
    short   loop_cnt     = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_1A, sizeof(CKYX_ATALLA_CMD_1A));

    /* # 設定 */
    command_msg[2] = '#';
    cmd_cnt = 3;

    /* Key Usage 設定 */
    command_msg[cmd_cnt] = '0';
    cmd_cnt = cmd_cnt + 1;
    /* # 設定 */
    command_msg[cmd_cnt] = '#';
    cmd_cnt += 1;
    /* Exportability判定 */
    if ((g_gckey.key_info.exportability == CKYX_KEY_EXPORT_S)&&
        (g_request_trigger == CKYX_REQUEST_TRIGGER_GFP)){
        /* Key Exchange Key 設定 */
        for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
            if (g_gckey_conv.key_info.send_key_info.key_value[loop_cnt] == CKYX_SPACE){
                break;
            }
        }
        if (loop_cnt != 0){
            /* KEYあり */
            memcpy(&command_msg[cmd_cnt], g_gckey_conv.key_info.send_key_info.key_value, loop_cnt);
            cmd_cnt = cmd_cnt + loop_cnt;
        }
    }
    else{
        /* Key Exchange Key 設定 */
        for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
            if (g_gckey.key_info.send_key_info.key_value[loop_cnt] == CKYX_SPACE){
                break;
            }
        }
        if (loop_cnt != 0){
            /* KEYあり */
            memcpy(&command_msg[cmd_cnt], g_gckey.key_info.send_key_info.key_value, loop_cnt);
            cmd_cnt = cmd_cnt + loop_cnt;
        }
    }

    /* # 設定 */
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Working Key or Key Spec 設定 */
    memcpy(&command_msg[cmd_cnt],
           g_key_info_save.key,
           g_key_info_save.key_len);
    cmd_cnt = cmd_cnt + g_key_info_save.key_len;
    command_msg[cmd_cnt] = '#';

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_1A */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_11A                            */
/*  CALLING SEQ.    : short CKYX_atalla_make_11A (void)                      */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 11A 作成 暗号鍵の暗号化(ANSI X9 TR31 key Block)*/
/*****************************************************************************/
short CKYX_atalla_make_11A(char  *atalla_msg,
                           short *atalla_msg_len)
{
    short   cmd_cnt      = 0;
    short   loop_cnt     = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_11A, sizeof(CKYX_ATALLA_CMD_11A));

    /* # 設定 */
    command_msg[3] = '#';

    /* Version 設定 */
    command_msg[4] = CKYX_ATALLA_11A_VER_B;
    command_msg[5] = '#';

    /* Key Usage 省略 */
    command_msg[6] = '#';

    /* Exportability 省略 */
    command_msg[7] = '#';

    /* Key Block Protection Key 設定 */
    for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
        if (g_gckey.key_info.send_key_info.key_value[loop_cnt] == CKYX_SPACE){
            break;
        }
    }
    cmd_cnt = 8;
    if (loop_cnt != 0){
        /* KEYあり */
        memcpy(&command_msg[cmd_cnt], g_gckey.key_info.send_key_info.key_value, loop_cnt);
        cmd_cnt = cmd_cnt + loop_cnt;
    }
    /* # 設定 */
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Working Key or Key Spec 設定 */
    memcpy(&command_msg[cmd_cnt],
           g_key_info_save.key,
           g_key_info_save.key_len);
    cmd_cnt = cmd_cnt +  g_key_info_save.key_len;
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Key Version Number 設定 */
    memset(&command_msg[cmd_cnt], '0', 2);
    cmd_cnt = cmd_cnt + 2;
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Number of Optional blocks 設定 */
    memset(&command_msg[cmd_cnt], '0', 2);
    cmd_cnt = cmd_cnt + 2;
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Optional Block Data 省略 */
    command_msg[cmd_cnt] = '#';

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_11A */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_11B                            */
/*  CALLING SEQ.    : short CKYX_atalla_make_11B (void)                      */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 11B 作成 暗号鍵の復号（入力形式：ANSI X9.17）  */
/*****************************************************************************/
short CKYX_atalla_make_11B(char  *atalla_msg,
                           short *atalla_msg_len)
{
    short   cmd_cnt      = 0;
    short   loop_cnt     = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_11B, sizeof(CKYX_ATALLA_CMD_11B));

    /* # 設定 */
    command_msg[3] = '#';

    cmd_cnt = 4;

    /* Key Usage 設定 */
    if (g_atalla_info.key_usage_11B[0] == CKYX_SPACE){
        command_msg[cmd_cnt] = '0';
        cmd_cnt = cmd_cnt + 1;
    }
    else{
        command_msg[cmd_cnt] = g_atalla_info.key_usage_11B[0];
        cmd_cnt = cmd_cnt + 1;
        if (g_atalla_info.key_usage_11B[1] != CKYX_SPACE){
            command_msg[cmd_cnt] = g_atalla_info.key_usage_11B[1];
            cmd_cnt += 1;
        }
    }

    command_msg[cmd_cnt] = '#';
    cmd_cnt += 1;

    /* Working Key 設定 */
    /* Exportability判定 */
    if ((g_gckey.key_info.exportability == CKYX_KEY_EXPORT_S)&&
        (g_request_trigger == CKYX_REQUEST_TRIGGER_GFP)){
        memcpy(&command_msg[cmd_cnt],
               g_key_info_save.key,
               g_key_info_save.key_len);
        cmd_cnt = cmd_cnt + g_key_info_save.key_len;
        command_msg[cmd_cnt] = '#';
        cmd_cnt = cmd_cnt + 1;
    }
    else {
        memcpy(&command_msg[cmd_cnt],
               g_req_seisa_result.key,
               g_req_seisa_result.key_leng);
        cmd_cnt = cmd_cnt + g_req_seisa_result.key_leng;
        command_msg[cmd_cnt] = '#';
        cmd_cnt = cmd_cnt + 1;
    }

    /* Exportability判定 */
    if ((g_gckey.key_info.exportability == CKYX_KEY_EXPORT_S)&&
        (g_request_trigger == CKYX_REQUEST_TRIGGER_GFP)){
        /* Key Exchange Key 設定 */
        for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
            if (g_gckey_conv.key_info.recv_key_info.key_value[loop_cnt] == CKYX_SPACE){
                break;
            }
        }
        if (loop_cnt != 0){
            /* KEYあり */
            memcpy(&command_msg[cmd_cnt], g_gckey_conv.key_info.recv_key_info.key_value, loop_cnt);
            cmd_cnt = cmd_cnt + loop_cnt;
        }
    }
    else{
        /* Key Exchange Key 設定 */
        for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
            if (g_gckey.key_info.recv_key_info.key_value[loop_cnt] == CKYX_SPACE){
                break;
            }
        }
        if (loop_cnt != 0){
            /* KEYあり */
            memcpy(&command_msg[cmd_cnt], g_gckey.key_info.recv_key_info.key_value, loop_cnt);
            cmd_cnt = cmd_cnt + loop_cnt;
        }
    }
    /* # 設定 */
    command_msg[cmd_cnt] = '#';

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_11B */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_119                            */
/*  CALLING SEQ.    : short CKYX_atalla_make_119 (void)                      */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 119 作成 暗号鍵の復号(ANSI X9 TR31 key Block)  */
/*****************************************************************************/
short CKYX_atalla_make_119(char  *atalla_msg,
                           short *atalla_msg_len)
{
    short   cmd_cnt      = 0;
    short   loop_cnt     = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_119, sizeof(CKYX_ATALLA_CMD_119));

    /* # 設定 */
    command_msg[3] = '#';

    cmd_cnt = 4;

    /* AKB Version 設定 */
    command_msg[cmd_cnt] = '1';
    cmd_cnt = cmd_cnt + 1;
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* Algorithm/Identifier U設定 */
    command_msg[cmd_cnt] = 'U';
    command_msg[cmd_cnt+1] = '#';
    cmd_cnt = cmd_cnt + 2;

    /* Key Block Protection Key 設定 */
    for(loop_cnt=0; loop_cnt<CKYX_KEK_LEN; loop_cnt++){
        if (g_gckey.key_info.recv_key_info.key_value[loop_cnt] == CKYX_SPACE){
            break;
        }
    }
    if (loop_cnt != 0){
        /* KEYあり */
        memcpy(&command_msg[cmd_cnt], g_gckey.key_info.recv_key_info.key_value, loop_cnt);
        cmd_cnt = cmd_cnt + loop_cnt;
    }
    /* # 設定 */
    command_msg[cmd_cnt] = '#';
    cmd_cnt = cmd_cnt + 1;

    /* TR-31 Key Block 設定 */
    memcpy(&command_msg[cmd_cnt],
           g_req_seisa_result.key,
           g_req_seisa_result.key_leng);
    cmd_cnt = cmd_cnt + g_req_seisa_result.key_leng;
    command_msg[cmd_cnt] = '#';

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_119 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_7E                             */
/*  CALLING SEQ.    : short CKYX_atalla_make_7E (void)                       */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Command 7E 作成  チェックディジットの生成              */
/*****************************************************************************/
short CKYX_atalla_make_7E(char *atalla_msg,
                          short *atalla_msg_len)
{
    short   cmd_cnt      = 0;

    char command_msg[CKYX_MAX_ATALLA_CMD_LEN];

    memset(command_msg, CKYX_SPACE, sizeof(command_msg));

    memset((char*)&g_atalla_info, 0, sizeof(g_atalla_info));

    /* Command identifier 設定 */
    memcpy(&command_msg[0], CKYX_ATALLA_CMD_7E, sizeof(CKYX_ATALLA_CMD_7E));

    /* # 設定 */
    command_msg[2] = '#';
    cmd_cnt = 3;

    /* Method 設定 */
    command_msg[cmd_cnt] = 'V';
    /* # 設定 */
    command_msg[cmd_cnt+1] = '#';
    /* Key Usage 省略 */
    command_msg[cmd_cnt+2] = '#';
    cmd_cnt = cmd_cnt+3;

    /* WorkingKey設定 */
    memcpy(&command_msg[cmd_cnt],
           g_key_info_save.file_set_wk_key,
           g_key_info_save.file_set_wk_key_len);

    /* # 設定 */
    cmd_cnt = cmd_cnt+g_key_info_save.file_set_wk_key_len;
    command_msg[cmd_cnt] = '#';

    atalla_msg[0] = '<';
    memcpy(&atalla_msg[1], command_msg, (cmd_cnt+1));
    atalla_msg[cmd_cnt+2] = '>';
    *atalla_msg_len = cmd_cnt+3;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_make_7E */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_rsp_chk                             */
/*  CALLING SEQ.    : short CKYX_atalla_rsp_chk (char cmd_type,              */
/*                                               char *atalla_rsp)           */
/*  ARGUMENT        : 1. cmd_type        (I) コマンド種別                    */
/*                  : 2. atalla_rsp      (I) ATALLAレスポンス                */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : ATALLA応答データ解析処理                               */
/*****************************************************************************/
short CKYX_atalla_rsp_chk(char cmd_type, char *atalla_rsp)
{
    char    ch_key_fmt[2];
    char    ch_rsp_id[4];
    char    ch_rsp_id_leng = 0;
    short   ls_result    = CKYX_RET_OK;              /* WK処理結果      */
    short   loop_cnt     = 0;

    atalla_info_20_def  atalla_info_20;
    atalla_info_2A_def  atalla_info_2A;
    atalla_info_21A_def atalla_info_21A;
    atalla_info_21B_def atalla_info_21B;
    atalla_info_219_def atalla_info_219;
    atalla_info_8E_def  atalla_info_8E;

    memset((char*)&atalla_info_20 , 0, sizeof(atalla_info_20) );
    memset((char*)&atalla_info_2A , 0, sizeof(atalla_info_2A) );
    memset((char*)&atalla_info_21A, 0, sizeof(atalla_info_21A));
    memset((char*)&atalla_info_21B, 0, sizeof(atalla_info_21B));
    memset((char*)&atalla_info_219, 0, sizeof(atalla_info_219));
    memset((char*)&atalla_info_8E , 0, sizeof(atalla_info_8E) );

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* ATALLAレスポンスのResponse identifier取得 */
    for(loop_cnt=0; loop_cnt<4; loop_cnt++){
        /* レスポンスID保存 */
        ch_rsp_id[loop_cnt] = atalla_rsp[loop_cnt+1];
        if (ch_rsp_id[loop_cnt] == '#'){
            /* #を0に変換 */
            ch_rsp_id[loop_cnt] = 0;
            ch_rsp_id_leng = (char)loop_cnt;
            break;
        }
    }

    if (ch_rsp_id_leng == 0){
        /* Response identifier異常 */
        /* EMS出力 (ATALLAレスポンスエラー)*/
        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_ATALLA_RSP_ERR,
                            "@L@T@C",
                            g_lcn,
                            &g_rcv_con_info,
                            ch_rsp_id);

        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_ATALLA_RSP_ERR,
               sizeof(g_internal_error_code));

        /* 異常終了 */
        return CKYX_RET_NG;
    }

    /* キーフォーマット取得 */
    memcpy(ch_key_fmt, g_gckey.key_info.key_format, sizeof(ch_key_fmt));

    /* コマンド種別で処理振分 */
    if ( cmd_type == CKYX_CMD_KEY_DEC ){
        /* 鍵復号 */
        /* 鍵管理情報ファイルのキーフォーマット判定 */
        if (memcmp(ch_key_fmt, CKYX_KEY_FMT_VA, sizeof(ch_key_fmt)) == 0){
            /* ATALLAレスポンス判定 */
            if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_21B, ch_rsp_id_leng) == 0){
                /* Response 21B 解析 */
                ls_result = CKYX_atalla_rsp_21B(&atalla_rsp[1], &atalla_info_21B);
                if (ls_result != CKYX_RET_OK){
                    if (memcmp(g_internal_error_code,
                               DEF_NERR_CHK_DIGIT_ERR,
                               sizeof(g_internal_error_code)) != 0) {
                        /* EMS出力 (ATALLAレスポンスエラー)*/
                        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                            DEF_MSGTTKB_GYOM_ERR,
                                            DEF_NERR_ATALLA_RSP_ERR,
                                            "@L@T@C",
                                            g_lcn,
                                            &g_rcv_con_info,
                                            ch_rsp_id);
                    }
                    /* 異常終了 */
                    return CKYX_RET_NG;
                }
                /* キー情報格納 */
                memcpy(&g_key_info_save, &atalla_info_21B, sizeof(atalla_info_21B));
                memcpy(g_key_info_save.file_set_wk_key,
                       atalla_info_21B.key,
                       sizeof(g_key_info_save.file_set_wk_key));
               g_key_info_save.file_set_wk_key_len = atalla_info_21B.key_len;
            }
            else {
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);
                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_ATALLA_RSP_ERR,
                       sizeof(g_internal_error_code));

                /* 異常終了 */
                return CKYX_RET_NG;
            }
        }
        else{ /* KB */
            /* ATALLAレスポンス判定 */
            if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_219, ch_rsp_id_leng) == 0){
                /* Response 219 解析 */
                ls_result = CKYX_atalla_rsp_219(&atalla_rsp[1], &atalla_info_219);
                if (ls_result != CKYX_RET_OK){
                    if (memcmp(g_internal_error_code,
                               DEF_NERR_CHK_DIGIT_ERR,
                               sizeof(g_internal_error_code)) != 0) {
                        /* EMS出力 (ATALLAレスポンスエラー)*/
                        CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                            DEF_MSGTTKB_GYOM_ERR,
                                            DEF_NERR_ATALLA_RSP_ERR,
                                            "@L@T@C",
                                            g_lcn,
                                            &g_rcv_con_info,
                                            ch_rsp_id);
                    }                    /* 異常終了 */
                    return CKYX_RET_NG;
                }

                /* キー情報格納 */
                memcpy(&g_key_info_save, &atalla_info_219, sizeof(atalla_info_219));
                memcpy(g_key_info_save.file_set_wk_key,
                       atalla_info_219.key,
                       sizeof(g_key_info_save.file_set_wk_key));
                g_key_info_save.file_set_wk_key_len = atalla_info_219.key_len;
            }
            else {
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);
                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_ATALLA_RSP_ERR,
                       sizeof(g_internal_error_code));
                /* 異常終了 */
                return CKYX_RET_NG;
            }
        }
    }
    else if ( cmd_type == CKYX_CMD_KEY_MAKE ){
        /* 鍵生成 */
        if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_20, ch_rsp_id_leng) == 0){
            /* Response 20 解析 */
            ls_result = CKYX_atalla_rsp_20(&atalla_rsp[1], &atalla_info_20);
            if (ls_result != CKYX_RET_OK){
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);
                /* 異常終了 */
                return CKYX_RET_NG;
            }
            /* キー情報格納 */
            memcpy(&g_key_info_save,
                   &atalla_info_20,
                   sizeof(atalla_info_20));
            memcpy(g_key_info_save.file_set_wk_key,
                   atalla_info_20.key_mfk,
                   sizeof(g_key_info_save.file_set_wk_key));
            g_key_info_save.file_set_wk_key_len = atalla_info_20.key_mfk_len;
        }
        else {
            /* EMS出力 (ATALLAレスポンスエラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_ATALLA_RSP_ERR,
                                "@L@T@C",
                                g_lcn,
                                &g_rcv_con_info,
                                ch_rsp_id);
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_ATALLA_RSP_ERR,
                   sizeof(g_internal_error_code));

            /* 異常終了 */
            return CKYX_RET_NG;
        }
    }
    else if ( cmd_type == CKYX_CMD_KEY_ENC ){
        /* 鍵暗号化 */
        /* 鍵管理情報ファイルのキーフォーマット判定 */
        if (memcmp(ch_key_fmt, CKYX_KEY_FMT_VA, sizeof(ch_key_fmt)) == 0){
            /* ATALLAレスポンス判定 */
            if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_2A, ch_rsp_id_leng) == 0){
                /* Response 2A 解析 */
                ls_result = CKYX_atalla_rsp_2A(&atalla_rsp[1], &atalla_info_2A);
                if (ls_result != CKYX_RET_OK){
                    /* EMS出力 (ATALLAレスポンスエラー)*/
                    CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                        DEF_MSGTTKB_GYOM_ERR,
                                        DEF_NERR_ATALLA_RSP_ERR,
                                        "@L@T@C",
                                        g_lcn,
                                        &g_rcv_con_info,
                                        ch_rsp_id);
                    /* 異常終了 */
                    return CKYX_RET_NG;
                }
                /* キー情報格納 */
                memcpy(&g_key_info_save, &atalla_info_2A, sizeof(atalla_info_2A));
            }
            else {
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);

                 /* 内部エラーコード設定 */
                 memcpy(g_internal_error_code,
                        DEF_NERR_ATALLA_RSP_ERR,
                        sizeof(g_internal_error_code));

                /* 異常終了 */
                return CKYX_RET_NG;
            }
        }
        else{ /* KB */
            /* ATALLAレスポンス判定 */
            if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_21A, ch_rsp_id_leng) == 0){
                /* Response 21A 解析 */
                ls_result = CKYX_atalla_rsp_21A(&atalla_rsp[1], &atalla_info_21A);
                if (ls_result != CKYX_RET_OK){
                    /* EMS出力 (ATALLAレスポンスエラー)*/
                    CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                        DEF_MSGTTKB_GYOM_ERR,
                                        DEF_NERR_ATALLA_RSP_ERR,
                                        "@L@T@C",
                                        g_lcn,
                                        &g_rcv_con_info,
                                        ch_rsp_id);
                    /* 異常終了 */
                    return CKYX_RET_NG;
                }
                /* キー情報格納 */
                g_key_info_save.chk_digit_len = atalla_info_21A.chk_digit_len;
                memcpy(g_key_info_save.chk_digit, atalla_info_21A.chk_digit, sizeof(g_key_info_save.chk_digit));
                g_key_info_save.key_akb_len = atalla_info_21A.key_akb_len;
                memcpy(g_key_info_save.key_akb, atalla_info_21A.key_akb, sizeof(g_key_info_save.key_akb));
                g_key_info_save.key_len = atalla_info_21A.key_blk_len;
                memcpy(g_key_info_save.key, atalla_info_21A.key_blk, sizeof(g_key_info_save.key_blk));
            }
            else {
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);

                 /* 内部エラーコード設定 */
                 memcpy(g_internal_error_code,
                        DEF_NERR_ATALLA_RSP_ERR,
                        sizeof(g_internal_error_code));

                /* 異常終了 */
                return CKYX_RET_NG;
            }
        }
    }
    else if ( cmd_type == CKYX_CMD_KEY_CKDIGIT ){
        /* チェックディジット生成 */
        /* ATALLAレスポンス判定 */
        if (memcmp(ch_rsp_id, CKYX_ATALLA_RSP_8E, ch_rsp_id_leng) == 0){
            /* Response 8E 解析 */
            ls_result = CKYX_atalla_rsp_8E(&atalla_rsp[1], &atalla_info_8E);
            if (ls_result != CKYX_RET_OK){
                /* EMS出力 (ATALLAレスポンスエラー)*/
                CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_ATALLA_RSP_ERR,
                                    "@L@T@C",
                                    g_lcn,
                                    &g_rcv_con_info,
                                    ch_rsp_id);
                /* 異常終了 */
                return CKYX_RET_NG;
            }
            /* キー情報格納 */
            g_key_info_save.chk_digit_len = atalla_info_8E.chk_digit_len;
            memcpy(g_key_info_save.chk_digit, atalla_info_8E.chk_digit, sizeof(g_key_info_save.chk_digit));
        }
        else {
            /* EMS出力 (ATALLAレスポンスエラー)*/
            CMIN_message_output(DEF_EVT_ATALLA_RSP_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_ATALLA_RSP_ERR,
                                "@L@T@C",
                                g_lcn,
                                &g_rcv_con_info,
                                ch_rsp_id);

             /* 内部エラーコード設定 */
             memcpy(g_internal_error_code,
                    DEF_NERR_ATALLA_RSP_ERR,
                    sizeof(g_internal_error_code));

            /* 異常終了 */
            return CKYX_RET_NG;
        }
    }

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_chk */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_rsp_20                              */
/*  CALLING SEQ.    : short CKYX_atalla_rsp_20 (char *atalla_rsp)            */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_20  (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 20 受信 暗号鍵の生成                          */
/*****************************************************************************/
short CKYX_atalla_rsp_20(char *atalla_rsp, atalla_info_20_def *atalla_info_20)
{
    short   cmd_cnt      = 3;
    short   loop_cnt     = 0;
    char    set_end_flg  = CKYX_FLG_OFF;

    /* Working Key (MFK encrypted) 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_20->key_mfk[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_20->key_mfk_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Working Key (KEK encrypted) 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_20->key_kek[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_20->key_kek_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_20->chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_20->chk_digit_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_20 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_2A                             */
/*  CALLING SEQ.    : short CKYX_atalla_make_2A (char *atalla_rsp)           */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_2A  (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 2A 受信 暗号鍵の暗号化(出力形式：ANSI X9.17)  */
/*****************************************************************************/
short CKYX_atalla_rsp_2A(char *atalla_rsp, atalla_info_2A_def *atalla_info_2A)
{
    short   cmd_cnt      = 3;
    short   loop_cnt     = 0;
    char    set_end_flg  = CKYX_FLG_OFF;

    /* Working Key (MFK encrypted) 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_2A->key[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_2A->key_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_2A->chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_2A->chk_digit_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Working Key AKB 参照 */
    if (atalla_rsp[cmd_cnt] == '>'){
        /* 電文終了 */
        return CKYX_RET_OK;
    }

    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if ((atalla_rsp[cmd_cnt+loop_cnt] != '#') &&
            (atalla_rsp[cmd_cnt+loop_cnt] != '>')) {
            atalla_info_2A->key_akb[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_2A->key_akb_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_2A */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_rsp_21A                             */
/*  CALLING SEQ.    : short CKYX_atalla_rsp_21A (char *atalla_rsp)           */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_21A (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 21A 受信 暗号鍵の暗号化(ANSI X9 TR31 keyBlock)*/
/*****************************************************************************/
short CKYX_atalla_rsp_21A(char *atalla_rsp, atalla_info_21A_def *atalla_info_21A)
{
    short   cmd_cnt      = 4;
    short   loop_cnt     = 0;
    char    set_end_flg  = CKYX_FLG_OFF;

    /* TR-31 Key Block 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_TR31_KEY_BLK_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_21A->key_blk[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_21A->key_blk_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_21A->chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_21A->chk_digit_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Working Key AKB 参照 */
    if (atalla_rsp[cmd_cnt] == '>'){
        /* 電文終了 */
        return CKYX_RET_OK;
    }

    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_21A->key_akb[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_21A->key_akb_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_21A */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_rsp_21B                             */
/*  CALLING SEQ.    : short CKYX_atalla_rsp_21B (char *atalla_rsp)           */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_21B (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 21B 受信 暗号鍵の復号(入力形式：ANSI X9.17)   */
/*****************************************************************************/
short CKYX_atalla_rsp_21B(char *atalla_rsp, atalla_info_21B_def *atalla_info_21B)
{
    short   cmd_cnt          = 4;
    char    chk_digit[CKYX_CHKDIGIT_LEN];
    short   loop_cnt         = 0;
    char    set_end_flg  = CKYX_FLG_OFF;

    /* Working Key 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_21B->key[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_21B->key_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_HSMK_DECODE_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_21B->chk_digit_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_HSMK_DECODE_ERR,
               sizeof(g_internal_error_code));

        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* チェックデジットを保存 */
    memcpy(atalla_info_21B->chk_digit, chk_digit, atalla_info_21B->chk_digit_len);

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_21B */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_219                            */
/*  CALLING SEQ.    : short CKYX_atalla_make_219 (char *atalla_rsp)          */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_219 (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 219 受信 暗号鍵の復号(ANSI X9 TR31 key Block) */
/*****************************************************************************/
short CKYX_atalla_rsp_219(char *atalla_rsp, atalla_info_219_def *atalla_info_219)
{
    short   cmd_cnt          = 4;
    short   loop_cnt         = 0;
    char    chk_digit[CKYX_CHKDIGIT_LEN];
    char    set_end_flg  = CKYX_FLG_OFF;

    /* Working Key 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_KEY_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_219->key[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_219->key_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_HSMK_DECODE_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            set_end_flg = CKYX_FLG_ON;
            break;
        }
    }
    atalla_info_219->chk_digit_len = loop_cnt;
    cmd_cnt = cmd_cnt + loop_cnt + 1;
    if (set_end_flg != CKYX_FLG_ON){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_HSMK_DECODE_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }
    set_end_flg = CKYX_FLG_OFF;

    /* チェックデジットを保存 */
    memcpy(atalla_info_219->chk_digit, chk_digit, atalla_info_219->chk_digit_len);

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_219 */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_atalla_make_8E                             */
/*  CALLING SEQ.    : short CKYX_atalla_make_8E (char *atalla_rsp)           */
/*  ARGUMENT        : 1. atalla_rsp      (I) ATALLA応答                      */
/*                  : 2. atalla_info_8E  (O) ATALLA結果格納                  */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : Response 8E 受信 チェックディジットの生成              */
/*****************************************************************************/
short CKYX_atalla_rsp_8E(char *atalla_rsp, atalla_info_8E_def *atalla_info_8E)
{
    short   cmd_cnt      = 3;
    short   loop_cnt     = 0;

    /* Method 長確認 */
    if  (atalla_rsp[cmd_cnt+1] != '#'){
        /* データ長異常 */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    cmd_cnt = cmd_cnt+2;

    /* Check Digits 参照 */
    for(loop_cnt=0; loop_cnt<=CKYX_CHKDIGIT_LEN; loop_cnt++){
        if (atalla_rsp[cmd_cnt+loop_cnt] != '#'){
            atalla_info_8E->chk_digit[loop_cnt] = atalla_rsp[cmd_cnt+loop_cnt];
        }
        else{
            break;
        }
    }

    /* Check Digits 長確認 */
    if ( loop_cnt != CKYX_CHKDIGIT_LEN ){
        /* Check Digitsが6ではない */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_SMK_KEY_ENC_ERR,
               sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    /* Check Digits 長設定 */
    atalla_info_8E->chk_digit_len = loop_cnt;

    return CKYX_RET_OK;

} /* end of CKYX_atalla_rsp_8E */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_file_update                            */
/*  CALLING SEQ.    : short CKYX_key_file_update (short key_idx_upd)         */
/*  ARGUMENT        : 1. key_idx_upd      (I) キーインデックス更新有無       */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 鍵管理ファイル更新処理                                 */
/*****************************************************************************/
short CKYX_key_file_update(short key_idx_upd)
{
    short   ls_result    = CKYX_RET_OK;          /* WK処理結果              */
    short   loop_cnt     = 0;
    short   ls_loop_end  = 0;

    db_gckey_def       get_kye_file_data;        /* KEYファイル読み込み領域 */

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */

    /* 日時取得用データ初期化 */
    memset(&COM_SDT_arg_2_data, CKYX_SPACE, sizeof(COM_SDT_arg_2_data));
    memset(&COM_SDT_arg_3_data, CKYX_ZERO , sizeof(COM_SDT_arg_2_data));
    date_time = CKYX_ZERO;

    /* 他ノード鍵管理ファイル更新レコード情報初期化 */
    memset(&g_gckey_upd_rec, CKYX_SPACE, sizeof(g_gckey_upd_rec));
    g_gckey_upd_rec.upd_rec_cnt = 0;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* システム日時取得 */
    COM_SDT(DEF_COM_SDT_arg1_jpn, &COM_SDT_arg_2_data,
            &COM_SDT_arg_3_data,  &date_time);

    /* 自ノードファイルの自サイトレコード、他サイトレコードを更新 */
    /*   ※他サイトレコードは鍵管理単位がインタエース単位の場合   */
    if (g_nwi_g.mng_lyr_info.key_cng_mng_lyr == DEF_KEY_CHANGE_MNG_LYR_IF){
        ls_loop_end = CKYX_OWN_FILE_OTHER_SITE;
    }
    else {
        ls_loop_end = CKYX_OWN_FILE_OWN_SITE;
    }

    for(loop_cnt=CKYX_OWN_FILE_OWN_SITE; loop_cnt<=ls_loop_end; loop_cnt++){
        /* NW情報の鍵交換管理単位確認 */
        /* 鍵管理ファイル読込処理 */
        ls_result = CKYX_key_read(g_key_kind,
                                  DEF_COM_IOM_LOCK,
                                  (char)loop_cnt,
                                  &g_rcv_con_info,
                                  &get_kye_file_data);

        if (ls_result != CKYX_RET_OK){
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 鍵情報のインデックス確認 */
        if (memcmp(get_kye_file_data.key_info.new_key_index,
                   DEF_NEW_KEY_INDEX01,
                   sizeof(get_kye_file_data.key_info.new_key_index)) == 0){
            /* 鍵情報をキー情報(2)に設定 */
            /* キー値設定 */
            memset(get_kye_file_data.key_info.key_info_02.key_value,
                   CKYX_SPACE,
                   sizeof(get_kye_file_data.key_info.key_info_02.key_value));
            memcpy(get_kye_file_data.key_info.key_info_02.key_value,
                   g_key_info_save.file_set_wk_key,
                   g_key_info_save.file_set_wk_key_len);
            /* チェックデジット設定 */
            memset(get_kye_file_data.key_info.key_info_02.check_digit,
                   CKYX_SPACE,
                   sizeof(get_kye_file_data.key_info.key_info_02.check_digit));
            memcpy(get_kye_file_data.key_info.key_info_02.check_digit,
                   g_key_info_save.chk_digit,
                   g_key_info_save.chk_digit_len);
            /* 更新日時設定 */
            memcpy(get_kye_file_data.key_info.key_info_02.key_update_time,
                   &COM_SDT_arg_2_data,
                   sizeof(get_kye_file_data.key_info.key_info_02.key_update_time));
            /* 鍵情報のインデックス設定 */
            if ( key_idx_upd == CKYX_KEY_IDX_UPD_YES ) {
                memcpy(get_kye_file_data.key_info.new_key_index,
                       DEF_NEW_KEY_INDEX02,
                       sizeof(get_kye_file_data.key_info.new_key_index));
            }
        }
        else {
            /* 鍵情報をキー情報(1)に設定 */
            /* キー値設定 */
            memset(get_kye_file_data.key_info.key_info_01.key_value,
                   CKYX_SPACE,
                   sizeof(get_kye_file_data.key_info.key_info_01.key_value));
            memcpy(get_kye_file_data.key_info.key_info_01.key_value,
                   g_key_info_save.file_set_wk_key,
                   g_key_info_save.file_set_wk_key_len);
            /* チェックデジット設定 */
            memset(get_kye_file_data.key_info.key_info_01.check_digit,
                   CKYX_SPACE,
                   sizeof(get_kye_file_data.key_info.key_info_01.check_digit));
            memcpy(get_kye_file_data.key_info.key_info_01.check_digit,
                   g_key_info_save.chk_digit,
                   g_key_info_save.chk_digit_len);
            /* 更新日時設定 */
            memcpy(get_kye_file_data.key_info.key_info_01.key_update_time,
                   &COM_SDT_arg_2_data,
                   sizeof(get_kye_file_data.key_info.key_info_01.key_update_time));
            /* 鍵情報のインデックス設定 */
            if ( key_idx_upd == CKYX_KEY_IDX_UPD_YES ) {
                memcpy(get_kye_file_data.key_info.new_key_index,
                       DEF_NEW_KEY_INDEX01,
                       sizeof(get_kye_file_data.key_info.new_key_index));
            }
        }

        /* 他ノード鍵管理ファイル更新レコード情報設定 */
        memcpy(g_gckey_upd_rec.rectbl[loop_cnt].upd_rec,
               &get_kye_file_data, 
               sizeof(g_gckey_upd_rec.rectbl[loop_cnt].upd_rec));
        g_gckey_upd_rec.upd_rec_cnt += 1;

        /* 鍵管理ファイルの鍵情報更新 */
        ls_result = CKYX_key_write((char)loop_cnt, &get_kye_file_data);
        if (ls_result != CKYX_RET_OK){
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }

    /* 他ノードファイルの更新 */
    ls_result = CKYX_other_node_gckey_upd();
    if (ls_result != CKYX_RET_OK){
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_key_file_update */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_key_file_num_chg                           */
/*  CALLING SEQ.    : short CKYX_key_file_num_chg (void)                     */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 鍵管理ファイル入れ替え処理                             */
/*****************************************************************************/
short CKYX_key_file_num_chg()
{
    short   ls_result    = CKYX_RET_OK;          /* WK処理結果              */
    short   loop_cnt     = 0;
    short   ls_loop_end  = 0;

    db_gckey_def       get_kye_file_data;        /* KEYファイル読み込み領域 */

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */

    /* 日時取得用データ初期化 */
    memset(&COM_SDT_arg_2_data, CKYX_SPACE, sizeof(COM_SDT_arg_2_data));
    memset(&COM_SDT_arg_3_data, CKYX_ZERO , sizeof(COM_SDT_arg_3_data));
    date_time = CKYX_ZERO;

    /* 他ノード鍵管理ファイル更新レコード情報初期化 */
    memset(&g_gckey_upd_rec, CKYX_SPACE, sizeof(g_gckey_upd_rec));
    g_gckey_upd_rec.upd_rec_cnt = 0;

    g_resp_kind = CKYX_RSP_KIND_NORMAL;

    /* システム日時取得 */
    COM_SDT(DEF_COM_SDT_arg1_jpn, &COM_SDT_arg_2_data,
            &COM_SDT_arg_3_data,  &date_time);

    /* 自ノードファイルの自サイトレコード、他サイトレコードを更新 */
    /*   ※他サイトレコードは鍵管理単位がインタエース単位の場合   */
    if (g_nwi_g.mng_lyr_info.key_cng_mng_lyr == DEF_KEY_CHANGE_MNG_LYR_IF){
        ls_loop_end = CKYX_OWN_FILE_OTHER_SITE;
    }
    else {
        ls_loop_end = CKYX_OWN_FILE_OWN_SITE;
    }

    for(loop_cnt=CKYX_OWN_FILE_OWN_SITE; loop_cnt<=ls_loop_end; loop_cnt++){
        /* 鍵管理ファイル読込処理 */
        ls_result = CKYX_key_read(g_key_kind,
                                  DEF_COM_IOM_LOCK,
                                  (char)loop_cnt,
                                  &g_rcv_con_info,
                                  &get_kye_file_data);

        if (ls_result != CKYX_RET_OK){
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }

        /* 鍵情報のインデックス確認 */
        if (memcmp(get_kye_file_data.key_info.new_key_index,
                   DEF_NEW_KEY_INDEX01,
                   sizeof(get_kye_file_data.key_info.new_key_index)) == 0){
            /* 鍵情報のインデックスを2に設定 */
            memcpy(get_kye_file_data.key_info.new_key_index,
                   DEF_NEW_KEY_INDEX02,
                   sizeof(get_kye_file_data.key_info.new_key_index));
            /* 更新日時設定 */
            memcpy(get_kye_file_data.key_info.key_info_02.key_update_time,
                   &COM_SDT_arg_2_data,
                   sizeof(get_kye_file_data.key_info.key_info_02.key_update_time));
        }
        else {
            /* 鍵情報のインデックスを1に設定 */
            memcpy(get_kye_file_data.key_info.new_key_index,
                   DEF_NEW_KEY_INDEX01,
                   sizeof(get_kye_file_data.key_info.new_key_index));
            /* 更新日時設定 */
            memcpy(get_kye_file_data.key_info.key_info_01.key_update_time,
                   &COM_SDT_arg_2_data,
                   sizeof(get_kye_file_data.key_info.key_info_01.key_update_time));
        }

        /* 他ノード鍵管理ファイル更新レコード情報設定 */
        memcpy(g_gckey_upd_rec.rectbl[loop_cnt].upd_rec,
               &get_kye_file_data, 
               sizeof(g_gckey_upd_rec.rectbl[loop_cnt].upd_rec));
        g_gckey_upd_rec.upd_rec_cnt += 1;

        /* 鍵管理ファイルの鍵情報更新 */
        ls_result = CKYX_key_write((char)loop_cnt, &get_kye_file_data);
        if (ls_result != CKYX_RET_OK){
            /* 内部エラーコード設定 */
            memcpy(g_internal_error_code,
                   DEF_NERR_FILE_IO_ERR,
                   sizeof(g_internal_error_code));

            /* 応答種別にエラー応答を設定 */
            g_resp_kind = CKYX_RSP_KIND_ERR;
            return CKYX_RET_NG;
        }
    }

    /* 他ノードファイルの更新 */
    ls_result = CKYX_other_node_gckey_upd();
    if (ls_result != CKYX_RET_OK){
        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_key_file_num_chg */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_other_node_gckey_upd                      */
/*  CALLING SEQ.    : short CKYX_other_node_gckey_upd (void)                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 他ノード鍵管理ファイル更新                            */
/****************************************************************************/
short CKYX_other_node_gckey_upd()
{
    short ls_result = 0;
    c801_def c801_ipc_data;       /* 鍵管理ファイル更新要求IPC */
    r801_def *r801_ipc_ptr;       /* 鍵管理ファイル更新応答IPC */

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* 鍵管理ファイル更新要求IPC初期化 */
    memset( &c801_ipc_data, 0, sizeof(c801_ipc_data) );

    /* IPC interface_code設定 */
    memcpy( c801_ipc_data.common_header.interface_code, DEF_IPC_IFCD_GCKEY_UPD_REQ,
             sizeof(c801_ipc_data.common_header.interface_code) );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( c801_ipc_data.common_header.internal_error_code, CKYX_SPACE,
            sizeof(c801_ipc_data.common_header.internal_error_code));
    memset( c801_ipc_data.common_header.filler_1, CKYX_SPACE,
            sizeof(c801_ipc_data.common_header.filler_1));
    memset( (char *)&c801_ipc_data.req_info, CKYX_SPACE,
            sizeof(c801_ipc_data.req_info));

    /* IPC GFP内部LCN設定 */
    memcpy( c801_ipc_data.req_info.gfp_lcn,
            g_lcn,
            sizeof(c801_ipc_data.req_info.gfp_lcn));

    /* IPC サイト識別設定 */
    c801_ipc_data.req_info.serverclass_info.site_name = g_myinfo.site_id;

    /* IPC サーバクラス論理名設定 */
    memcpy( c801_ipc_data.req_info.serverclass_info.serverclass_id.serverclass_name,
            g_myinfo.serverclass_name,
            sizeof(c801_ipc_data.req_info.serverclass_info.serverclass_id.serverclass_name));

    /* IPC サーバクラス論理番号設定 */
    memcpy( c801_ipc_data.req_info.serverclass_info.serverclass_id.serverclass_num,
            g_myinfo.serverclass_no,
            sizeof(c801_ipc_data.req_info.serverclass_info.serverclass_id.serverclass_num));

    /* IPC 更新レコード数 */
    c801_ipc_data.req_info.upd_rec_cnt = g_gckey_upd_rec.upd_rec_cnt;

    /* IPC 更新レコード1設定 */
    memcpy( c801_ipc_data.upd_rec_1,
            g_gckey_upd_rec.rectbl[0].upd_rec,
            sizeof(c801_ipc_data.upd_rec_1));

    /* IPC 更新レコード2設定 */
    memcpy( c801_ipc_data.upd_rec_2,
            g_gckey_upd_rec.rectbl[1].upd_rec,
            sizeof(c801_ipc_data.upd_rec_2));

    /* IPC データ長設定(26byte) */
    c801_ipc_data.common_header.control_data_length =
           sizeof(c801_ipc_data) - sizeof( c801_ipc_data.common_header);

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, CKYX_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           CKYX_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));
    if (g_gckey_upd_srv.domain_name[0] != CKYX_SPACE){
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               g_gckey_upd_srv.domain_name,
               sizeof(g_gckey_upd_srv.domain_name));
    }
    else{
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               g_gckey_upd_srv.pathmon_name,
               sizeof(g_gckey_upd_srv.pathmon_name));
    }

    memcpy(t_COM_PSD_arg_1_def.serverclass_name,
           g_gckey_upd_srv.server_class,
           sizeof(g_gckey_upd_srv.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &c801_ipc_data, sizeof(c801_ipc_data) );

    t_COM_PSD_arg_1_def.req_send_len    = sizeof(c801_ipc_data);
    t_COM_PSD_arg_1_def.receive_max_len = CKYX_MAX_DATA_SIZE;
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_myinfo.send_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = (short)g_myinfo.send_retry_count;

    memcpy(t_COM_PSD_arg_2_def.prog_id,
           g_myinfo.prog_id,
           sizeof(t_COM_PSD_arg_2_def.prog_id) );

    /* サーバークラス論理ID設定 */
    memcpy( t_COM_PSD_arg_4_def.srv_logical_id,
            g_myinfo.serverclass_name,
            sizeof(g_myinfo.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy( t_COM_PSD_arg_4_def.lcn,
            g_lcn,
            sizeof(g_lcn));

    /* PATHSEND共通処理実行 */
    ls_result = COM_PSD( &t_COM_PSD_arg_1_def,
                         &t_COM_PSD_arg_2_def,
                         &t_COM_PSD_arg_3_def,
                         &g_cg010in_modle,
                         &t_COM_PSD_arg_4_def );

    /* PATHSEND結果確認 */
    if ( ls_result != CKYX_RET_OK ) {
        /* 異常終了 */
        /* PATHSEND共通処理にてEMS出力済みのためEMS出力は行わない */
        /* PATHMON SHUTDOWNの場合、他ノードメンテナンス中と判断し正常とする */
        if (( t_COM_PSD_arg_3_def.pathsend_errcode == CKYX_PSD_ERR_SHUTDOWN ) ||
            ( t_COM_PSD_arg_3_def.pathsend_errcode == CKYX_PSD_ERR_CONNECT && 
              t_COM_PSD_arg_3_def.guardian_errcode == CKYX_FIL_ERR_NOSUCHDEV )) {
            return CKYX_RET_OK;
        }
        else {
            strncpy( g_internal_error_code,
                     DEF_NERR_OPPSITE_KEY_UPD_ERR,
                     sizeof(g_internal_error_code));
            return CKYX_RET_NG;
        }
    }

    r801_ipc_ptr = (r801_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R801) */
    if ( memcmp(r801_ipc_ptr->common_header.interface_code,
                DEF_IPC_IFCD_GCKEY_UPD_RSP,
                sizeof(r801_ipc_ptr->common_header.interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (r801_ipc_ptr->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常 */
            ls_result = CKYX_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        ls_result = CKYX_RET_NG;
    }
    if ( ls_result == CKYX_RET_NG ) {
        /* EMS出力 応答エラー */
        CMIN_message_output(DEF_EVT_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_OPPSITE_KEY_UPD_ERR,
                            "@L@C@C@H",
                            g_lcn, g_myinfo.serverclass_name,
                            "R801 ERROR          ",
                            r801_ipc_ptr);
        /* GFP内部LCN取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
                 DEF_NERR_OPPSITE_KEY_UPD_ERR,
                 sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_other_node_gckey_upd */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_r401_make                                  */
/*  CALLING SEQ.    : short CKYX_r401_make (cr401_def *c401_msg,             */
/*                                          short     rsp_kind)              */
/*  ARGUMENT        : 1. c401_msg        (I) 受信メッセージ                  */
/*                  : 2. rsp_kind        (I) 応答種別                        */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : IPC(R401)作成処理                                      */
/*****************************************************************************/
short CKYX_r401_make(cr401_def *c401_msg, short rsp_kind)
{
    short   ls_result    = CKYX_RET_OK;          /* WK処理結果              */

    cr401_def          r401_msg;                 /* R401作成領域            */
    NWM_KYX_ems_add    ems_info_add_data;        /* EMS出力付加情報         */
    NWM_KYX_arg_5_def  NWM_KYX_arg_5;
    NWM_KYX_arg_6_def  NWM_KYX_arg_6;
    char               rsp_data[MAX_TEXT_BUF_LEN]; /* 作成電文設定領域      */
    char               sv_internal_error_code[7];

    memset(&r401_msg,      CKYX_ZERO, sizeof(r401_msg) );
    memset(&NWM_KYX_arg_5, CKYX_ZERO, sizeof(NWM_KYX_arg_5) );
    memset(&NWM_KYX_arg_6, CKYX_ZERO, sizeof(NWM_KYX_arg_6) );

    memcpy(sv_internal_error_code, g_internal_error_code, sizeof(sv_internal_error_code));

    /* 電文種別初期化 */
    memcpy( NWM_KYX_arg_5.denbun_kind,
            &c401_msg->control_info.control_kind,
            sizeof(NWM_KYX_arg_5.denbun_kind));

    /* 制御電文識別確認 */
    if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
        (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC)&&
        (memcmp(c401_msg->control_info.request_kind,
                DEF_CTLREQ_HISIMUKE,
                sizeof(c401_msg->control_info.request_kind)) == 0)) {
        /* CTL_KIND x15x */
        /* 接続先契機鍵交換要求 */
        /* 応答種別確認 */
        if ((rsp_kind == CKYX_RSP_KIND_NORMAL) ||
            (rsp_kind == CKYX_RSP_KIND_REJ   )) {
            /* 正常応答または拒否応答 */
            /* 鍵交換電文(応答電文)編集処理 */
            /* サーバークラス論理ID設定 */
            memcpy( ems_info_add_data.srv_logical_id,
                    g_myinfo.serverclass_name,
                    sizeof(g_myinfo.serverclass_name));

            /* GFP内部LCN設定 */
            memcpy( ems_info_add_data.lcn,
                    g_lcn,
                    sizeof(g_lcn));

            /* 電文種別設定 */
            NWM_KYX_arg_5.denbun_kind[0] = DEF_CTLFNC_KEY_EXCH;
            NWM_KYX_arg_5.denbun_kind[1] = DEF_CTLMSG_RESPONSE;
            NWM_KYX_arg_5.denbun_kind[2] = DEF_CTLTXT_KEY_EXC;
            NWM_KYX_arg_5.denbun_kind[3] = DEF_CTLINT_NORMAL;

            /* コネクション論理ID設定 */
            memcpy( &NWM_KYX_arg_5.connection_lid,
                    &c401_msg->control_info.connection_lid,
                    sizeof(NWM_KYX_arg_5.connection_lid));

            /* チェックデジット設定 */
            NWM_KYX_arg_5.checkdigit_leng = (char)g_key_info_save.chk_digit_len;
            NWM_KYX_arg_5.checkdigit = g_key_info_save.chk_digit;

            /* システム採番設定 */
            memcpy( NWM_KYX_arg_5.sysytem_no,
                    g_sys_num,
                    sizeof(NWM_KYX_arg_5.sysytem_no));

            /* キー識別設定 */
            memcpy( NWM_KYX_arg_5.key_kind,
                    g_key_kind,
                    sizeof(NWM_KYX_arg_5.key_kind));

            /* キー長設定 */
            NWM_KYX_arg_5.key_leng =(char)g_key_info_save.key_len;

            /* キー設定 */
            NWM_KYX_arg_5.key = g_key_info_save.key;

            /* 内部エラーコード設定 */
            memcpy( NWM_KYX_arg_5.err_code,
                    g_internal_error_code,
                    sizeof(NWM_KYX_arg_5.err_code));

            /* 被仕向要求電文設定 */
            NWM_KYX_arg_6.rcv_message_leng = 
               (c401_msg->common_header.control_data_length - sizeof(c401_msg->control_info));
            NWM_KYX_arg_6.rcv_message = c401_msg->data_bu.message_text;

            /* 作成電文設定領域 */
            memset(rsp_data, CKYX_ZERO, sizeof(rsp_data));
            NWM_KYX_arg_6.message = rsp_data;

            /* MTI設定領域初期化 */
            memset(NWM_KYX_arg_6.mti, CKYX_SPACE, sizeof(NWM_KYX_arg_6.mti));

            ls_result = NWM_KYX_msg_edit_rsp(
                                             &g_nwi_g         /* NW情報レコード(グループ単位)        */
                                           , &g_nwi_i         /* NW情報レコード(インタフェース単位)  */
                                           , g_nws_n.dst_unq_info /* 接続先固有情報(NW単位)              */
                                           , g_nws_i.dst_unq_info /* 接続先固有情報(インタフェース単位)  */
                                           , g_nws_s.dst_unq_info /* 接続先固有情報(ステーション単位)    */
                                           , g_nws_c.dst_unq_info /* 接続先固有情報(コネクション単位)    */
                                           , &g_gckey         /* 鍵管理情報レコード                  */
                                           , &g_file_info_gccut
                                           , &g_cg010in_modle
                                           , &ems_info_add_data
                                           , &NWM_KYX_arg_5
                                           , &NWM_KYX_arg_6);
            if ((ls_result == DEF_NWM_KYX_RTN_OK) ||
                (ls_result == DEF_NWM_KYX_RTN_NG_REJ)){
                if (rsp_kind == CKYX_RSP_KIND_NORMAL) {
                    /* 正常応答 */
                    /* 電文種別設定 */
                    NWM_KYX_arg_5.denbun_kind[3] = DEF_CTLINT_ALLOW;
                }
                else {
                    /* 拒否応答 */
                    /* 電文種別設定 */
                    NWM_KYX_arg_5.denbun_kind[3] = DEF_CTLINT_DENY;
                }
            }
            else if (ls_result == DEF_NWM_KYX_RTN_HAKI_MSG){
                /* 破棄 */
                /* 応答種別にエラー応答を設定 */
                rsp_kind = CKYX_RSP_KIND_HAKI;
            }
            /* 送信不可時内部エラーコードにスペース設定 */
            memset(c401_msg->control_info.send_naibu_err_code,
                   CKYX_SPACE,
                   sizeof(c401_msg->control_info.send_naibu_err_code));
            /* 受信電文長にスペース設定 */
            memset(c401_msg->control_info.denbun_len,
                   CKYX_SPACE,
                   sizeof(c401_msg->control_info.denbun_len));
        }
    }

    /* R401のIPCヘッダ作成 */
    memcpy(&r401_msg, c401_msg, sizeof(r401_msg));

    /* インタフェースコード設定 */
    memcpy(r401_msg.common_header.interface_code,
           DEF_IPC_IFCD_NW_MSG_RSP,
           sizeof(r401_msg.common_header.interface_code));

    /* 応答電文なしで初期化 */
     memcpy(r401_msg.control_info.response_kind,
            CKYX_SEND_DATA_NASHI,
            sizeof(r401_msg.control_info.response_kind));

    /* 制御電文種別C401で初期化 */
    memcpy(&r401_msg.control_info.control_kind,
           &c401_msg->control_info.control_kind,
           sizeof(r401_msg.control_info.control_kind));

    /* 応答種別確認 */
    switch(rsp_kind){
    case CKYX_RSP_KIND_NORMAL:
    case CKYX_RSP_KIND_REJ:
        /* 応答電文有無設定 */
        if (NWM_KYX_arg_6.message_leng != 0){
            memcpy(r401_msg.control_info.response_kind,
                   CKYX_SEND_DATA_ARI,
                   sizeof(r401_msg.control_info.response_kind));
        }
        /* エラーコード設定 */
        r401_msg.common_header.error_code = DEF_IPC_ERRCD_OK;
        /* 内部エラーコード設定 */
        memcpy(r401_msg.common_header.internal_error_code,
               DEF_NERR_NOMAL,
               sizeof(r401_msg.common_header.internal_error_code));
        /* データ長設定 */
        r401_msg.common_header.control_data_length = 
           sizeof(r401_msg.control_info) + NWM_KYX_arg_6.message_leng;

        /* 制御電文設定 */
        memset(r401_msg.data_bu.message_text,
               CKYX_ZERO,
               sizeof(r401_msg.data_bu.message_text));
        memcpy(r401_msg.data_bu.message_text,
               rsp_data,
               sizeof(r401_msg.data_bu.message_text));
        /* 制御電文種別 */
        memcpy(&r401_msg.control_info.control_kind,
               NWM_KYX_arg_5.denbun_kind,
               sizeof(r401_msg.control_info.control_kind));

        break;
    case CKYX_RSP_KIND_FAULT:
        /* エラーコード設定 */
        r401_msg.common_header.error_code = DEF_IPC_ERRCD_FAILMSG;
        /* 内部エラーコード設定 */
        memcpy(r401_msg.common_header.internal_error_code,
               g_internal_error_code,
               sizeof(r401_msg.common_header.internal_error_code));
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_SHOGAI_NTF_CRE_REQ,
                            DEF_MSGTTKB_GYOM_ERR,
                            g_internal_error_code,
                            "@L@T",
                            g_lcn,
                            &g_rcv_con_info);
        break;
    case CKYX_RSP_KIND_HAKI:
        /* 応答電文有無設定 */
        memcpy(r401_msg.control_info.response_kind,
               CKYX_SEND_DATA_NASHI,
               sizeof(r401_msg.control_info.response_kind));
        /* エラーコード設定 */
        r401_msg.common_header.error_code = DEF_IPC_ERRCD_OK;
        /* 内部エラーコード設定 */
        memcpy(r401_msg.common_header.internal_error_code,
               g_internal_error_code,
               sizeof(r401_msg.common_header.internal_error_code));
        /* データ長設定(データ削除) */
        r401_msg.common_header.control_data_length = 
           sizeof(r401_msg.control_info);
        break;
    case CKYX_RSP_KIND_ERR:
    default:
        /* エラーコード設定 */
        r401_msg.common_header.error_code = DEF_IPC_ERRCD_NG;
        /* 内部エラーコード設定 */
        memcpy(r401_msg.common_header.internal_error_code,
               g_internal_error_code,
               sizeof(r401_msg.common_header.internal_error_code));
        break;
    }

    /* 電文種別 */
    r401_msg.control_info.denbun_log_key.denbun_shubetu
                = r401_msg.control_info.control_kind.ctl_text_kbn;

    /* MTI設定 */
    memcpy(r401_msg.control_info.mti,
           NWM_KYX_arg_6.mti,
           sizeof(r401_msg.control_info.mti));


    /* 受信IPC情報格納 */
    memset(g_key_rsp_msg, 0, sizeof(g_key_rsp_msg));
    memcpy(g_key_rsp_msg, (char*)&r401_msg, sizeof(r401_msg));

    /* 制御電文識別確認 */
    if ((c401_msg->control_info.control_kind.req_res_kbn  == DEF_CTLMSG_REQUEST)&&
        (c401_msg->control_info.control_kind.ctl_text_kbn == DEF_CTLTXT_KEY_EXC)&&
        (memcmp(c401_msg->control_info.request_kind,
                DEF_CTLREQ_HISIMUKE,
                sizeof(c401_msg->control_info.request_kind)) == 0)) {
        /* CTL_KIND x15x */
        /* 接続先契機鍵交換 */
        /* 応答種別確認 */
        if ((rsp_kind == CKYX_RSP_KIND_NORMAL) ||
            (rsp_kind == CKYX_RSP_KIND_REJ )) {

            /* 制御ログ出力 */
            ls_result = CKYX_ctl_log_output((char*)&r401_msg,
                                    CKYX_LOG_SET,
                                    CKYX_ERR_NASHI,
                                    g_internal_error_code);

            if (ls_result != CKYX_RET_OK){
                /* 電文削除 */
                /* エラーコード設定 */
                r401_msg.common_header.error_code = DEF_IPC_ERRCD_NG;
                /* 内部エラーコード設定 */
                memcpy(r401_msg.common_header.internal_error_code,
                       g_internal_error_code,
                       sizeof(r401_msg.common_header.internal_error_code));
                /* データ長設定 */
                r401_msg.common_header.control_data_length = 
                   sizeof(r401_msg.control_info);

                /* 応答種別にエラー応答を設定 */
                rsp_kind = CKYX_RSP_KIND_ERR;
            }
        }
    }
    /* 応答種別確認 */
    if ((rsp_kind == CKYX_RSP_KIND_ERR)   ||
        (rsp_kind == CKYX_RSP_KIND_FAULT) ||
        (rsp_kind == CKYX_RSP_KIND_HAKI)) {
        /* エラーログ出力 処理継続 */
        ls_result = CKYX_err_log_output( (char*)c401_msg,
                                         CKYX_ERR_LOG_NAIBU_ERR,
                                         g_internal_error_code);
    }
    g_key_rsp_leng =
        r401_msg.common_header.control_data_length + sizeof(r401_msg.common_header);

    return CKYX_RET_OK;

} /* end of CKYX_r401_make */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_r402_make                                  */
/*  CALLING SEQ.    : short CKYX_r402_make (short req_kind,                   */
/*                                          cr402_def *c402_msg,             */
/*                                          short     rsp_kind)              */
/*  ARGUMENT        : 1. req_kind        (I) 要求種別                        */
/*                  : 2. c402_msg        (I) 受信メッセージ                  */
/*                  : 3. rsp_kind        (I) 応答種別                        */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : IPC(R402)作成処理                                      */
/*****************************************************************************/
short CKYX_r402_make(short req_kind, cr402_def *c402_msg, short rsp_kind)
{
    short   ls_result    = CKYX_RET_OK;          /* WK処理結果              */

    cr402_def          r402_msg;                 /* R402作成領域            */
    NWM_KYX_ems_add    ems_info_add_data;        /* EMS出力付加情報         */
    NWM_KYX_arg_3_def  NWM_KYX_arg_3;
    NWM_KYX_arg_4_def  NWM_KYX_arg_4;
    char               rsp_data[MAX_TEXT_BUF_LEN]; /* 作成電文設定領域      */
    char               sv_internal_error_code[7];

    memset(&r402_msg,      CKYX_ZERO, sizeof(r402_msg) );
    memset(&NWM_KYX_arg_3, CKYX_ZERO, sizeof(NWM_KYX_arg_3) );
    memset(&NWM_KYX_arg_4, CKYX_ZERO, sizeof(NWM_KYX_arg_4) );

    memcpy(sv_internal_error_code, g_internal_error_code, sizeof(sv_internal_error_code));

    /* 制御電文種別初期値設定 */
    memcpy(NWM_KYX_arg_3.denbun_kind,
           &c402_msg->control_info.control_kind,
           sizeof(NWM_KYX_arg_3.denbun_kind));

    /* 応答種別確認 */
    if (rsp_kind == CKYX_RSP_KIND_NORMAL) {
        /* 正常応答 */
        /* システム採番生成処理 */
        memset(g_sys_num, 0, sizeof(g_sys_num));
        ls_result = CKYX_system_num_get(g_sys_num);

        if (ls_result != CKYX_RET_OK){
            if (rsp_kind == CKYX_RSP_KIND_NORMAL){
                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_SYS_NO_MAKE_ERR,
                       sizeof(g_internal_error_code));
            }
            else {
                memcpy(g_internal_error_code,
                       sv_internal_error_code,
                       sizeof(sv_internal_error_code));
            }
            /* 応答種別にエラー応答を設定 */
            rsp_kind    = CKYX_RSP_KIND_ERR;
        }
        else {
            /* CTL_KIND x15x / x14x */
            /* 鍵交換電文(要求電文)編集処理 */
            /* サーバークラス論理ID設定 */
            memcpy( ems_info_add_data.srv_logical_id,
                    g_myinfo.serverclass_name,
                    sizeof(g_myinfo.serverclass_name));

            /* GFP内部LCN設定 */
            memcpy( ems_info_add_data.lcn,
                    g_lcn,
                    sizeof(g_lcn));

            /* 電文種別設定 */
            memcpy(NWM_KYX_arg_3.denbun_kind,
                   &c402_msg->control_info.control_kind,
                   sizeof(NWM_KYX_arg_3.denbun_kind));
            NWM_KYX_arg_3.denbun_kind[1] = DEF_CTLMSG_REQUEST;
            NWM_KYX_arg_3.denbun_kind[3] = DEF_CTLINT_NORMAL;

            /* コネクション論理ID設定 */
            memcpy( &NWM_KYX_arg_3.connection_lid,
                    &c402_msg->control_info.connection_lid,
                    sizeof(NWM_KYX_arg_3.connection_lid));

            /* チェックデジット設定 */
            NWM_KYX_arg_3.checkdigit_leng = (char)g_key_info_save.chk_digit_len;
            NWM_KYX_arg_3.checkdigit = g_key_info_save.chk_digit;

            /* システム採番設定 */
            memcpy( NWM_KYX_arg_3.sysytem_no,
                    g_sys_num,
                    sizeof(NWM_KYX_arg_3.sysytem_no));

            /* キー識別設定 */
            memcpy( NWM_KYX_arg_3.key_kind,
                    g_key_kind,
                    sizeof(NWM_KYX_arg_3.key_kind));

            /* キー長設定 */
            NWM_KYX_arg_3.key_leng = (char)g_key_info_save.key_len;

            /* キー設定 */
            NWM_KYX_arg_3.key = g_key_info_save.key;

            /* 内部エラーコード設定 */
            memcpy( NWM_KYX_arg_3.err_code,
                    g_internal_error_code,
                    sizeof(NWM_KYX_arg_3.err_code));

            /* 作成電文設定領域 */
            memset(rsp_data, CKYX_ZERO, sizeof(rsp_data));
            NWM_KYX_arg_4.message = rsp_data;

            /* MTI設定領域初期化 */
            memset(NWM_KYX_arg_4.mti, CKYX_SPACE, sizeof(NWM_KYX_arg_4.mti));

            ls_result = NWM_KYX_msg_edit_req(req_kind
                                           , &g_nwi_g         /* NW情報レコード(グループ単位)        */
                                           , &g_nwi_i         /* NW情報レコード(インタフェース単位)  */
                                           , g_nws_n.dst_unq_info /* 接続先固有情報(NW単位)              */
                                           , g_nws_i.dst_unq_info /* 接続先固有情報(インタフェース単位)  */
                                           , g_nws_s.dst_unq_info /* 接続先固有情報(ステーション単位)    */
                                           , g_nws_c.dst_unq_info /* 接続先固有情報(コネクション単位)    */
                                           , &g_gckey         /* 鍵管理情報レコード                  */
                                           , &g_cg010in_modle
                                           ,  &ems_info_add_data
                                           ,  &NWM_KYX_arg_3
                                           ,  &NWM_KYX_arg_4);
            if (ls_result != CKYX_RET_OK){
                /* EMS出力 */
                CMIN_message_output(DEF_EVT_COMMON_MOD_ERR,
                                    DEF_MSGTTKB_GYOM_ERR,
                                    DEF_NERR_MSG_MAKE_ERR ,
                                    "@C@2",
                                    "NWM_KYX_msg_edit_req",
                                     ls_result);

                /* 内部エラーコード設定 */
                memcpy(g_internal_error_code,
                       DEF_NERR_MSG_MAKE_ERR,
                       sizeof(g_internal_error_code));

                /* 応答種別にエラー応答を設定 */
                rsp_kind = CKYX_RSP_KIND_ERR;
            }
        }
    }

    /* R402のIPCヘッダ作成 */
    memcpy(&r402_msg, c402_msg, sizeof(r402_msg));

    /* インタフェースコード設定 */
    memcpy(r402_msg.common_header.interface_code,
           DEF_IPC_IFCD_CTRL_MSG_RSP,
           sizeof(r402_msg.common_header.interface_code));

    /* 応答電文なしで初期化 */
    memcpy(r402_msg.control_info.response_kind,
           CKYX_SEND_DATA_NASHI,
           sizeof(r402_msg.control_info.response_kind));

    /* 応答種別確認 */
    if (rsp_kind == CKYX_RSP_KIND_NORMAL){
        /* 正常 */
        /* 応答電文有無設定 */
        if (NWM_KYX_arg_4.message_leng != 0){
            memcpy(r402_msg.control_info.response_kind,
                   CKYX_SEND_DATA_ARI,
                   sizeof(r402_msg.control_info.response_kind));
        }
        /* エラーコード設定 */
        r402_msg.common_header.error_code = DEF_IPC_ERRCD_OK;
        /* 内部エラーコード設定 */
        memcpy(r402_msg.common_header.internal_error_code,
               DEF_NERR_NOMAL,
               sizeof(r402_msg.common_header.internal_error_code));
    }
    else {
        /* 正常以外 */
        /* エラーコード設定 */
        r402_msg.common_header.error_code = DEF_IPC_ERRCD_NG;
        /* 内部エラーコード設定 */
        memcpy(r402_msg.common_header.internal_error_code,
               g_internal_error_code,
               sizeof(r402_msg.common_header.internal_error_code));
    }

    /* 制御電文種別 */
    memcpy(&r402_msg.control_info.control_kind,
           NWM_KYX_arg_3.denbun_kind,
           sizeof(r402_msg.control_info.control_kind));

    /* 電文種別 */
    r402_msg.control_info.denbun_log_key.denbun_shubetu
                = r402_msg.control_info.control_kind.ctl_text_kbn;

    /* MTI設定 */
    memcpy(r402_msg.control_info.mti,
           NWM_KYX_arg_4.mti,
           sizeof(r402_msg.control_info.mti));

    /* GFP内部LCN設定 */
    memcpy(r402_msg.control_info.denbun_log_key.tran_id.gfp_lcn,
           g_lcn,
           sizeof(r402_msg.control_info.denbun_log_key.tran_id.gfp_lcn));

    /* 制御電文設定 */
    memset(r402_msg.data_bu.message_text,
           CKYX_ZERO,
           sizeof(r402_msg.data_bu.message_text));
    memcpy(r402_msg.data_bu.message_text,
           rsp_data,
           sizeof(r402_msg.data_bu.message_text));

    /* 応答種別確認 */
    if (rsp_kind == CKYX_RSP_KIND_NORMAL) {
        /* データ長設定 */
        r402_msg.common_header.control_data_length = 
           sizeof(r402_msg.control_info) + NWM_KYX_arg_4.message_leng;

        /* 制御ログ出力 */
        ls_result = CKYX_ctl_log_output((char*)&r402_msg,
                                        CKYX_LOG_SET,
                                        CKYX_ERR_NASHI,
                                        g_internal_error_code);
        if (ls_result != CKYX_RET_OK){
            /* 応答種別にエラー応答を設定 */
            rsp_kind    = CKYX_RSP_KIND_ERR;
            /* エラーコード設定 */
            r402_msg.common_header.error_code = DEF_IPC_ERRCD_NG;
            /* 内部エラーコード設定 */
            memcpy(r402_msg.common_header.internal_error_code,
                   g_internal_error_code,
                   sizeof(r402_msg.common_header.internal_error_code));
            /* 電文削除 */
            /* データ長設定 */
            r402_msg.common_header.control_data_length = 
               sizeof(r402_msg.control_info);
        }
    }
    else {
        /* データ長設定 */
        r402_msg.common_header.control_data_length = sizeof(r402_msg.control_info);
    }

    /* エラーログ出力なし */

    /* 受信IPC情報格納 */
    memset(g_key_rsp_msg, 0, sizeof(g_key_rsp_msg));
    memcpy(g_key_rsp_msg, (char*)&r402_msg, sizeof(r402_msg));

    g_key_rsp_leng =
        r402_msg.common_header.control_data_length + sizeof(r402_msg.common_header);

    return CKYX_RET_OK;

} /* end of CKYX_r402_make */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_reply                                     */
/*  CALLING SEQ.    : short CKYX_reply ( void )                             */
/*  ARGUMENT        : なし                                                  */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 電文受信応答処理                                      */
/****************************************************************************/
short CKYX_reply ()
{
    _cc_status i_ret     = 0;
    unsigned short s_count_written = 0;
    short s_error_return = 0;

    /* リプライ処理 */
    i_ret = REPLYX((char *)g_key_rsp_msg,
                           g_key_rsp_leng,
                           &s_count_written,
                           g_iocomp.recv_info.z_messagetag,
                           s_error_return );

    if (i_ret != CKYX_RET_OK) {
        /* 異常終了 */
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_KEY_SEND_ERR ,
                             "@L@C@C@H",
                            g_lcn,
                            g_myinfo.serverclass_name,
                            "REPLY ERR",
                            g_key_rsp_msg);
        return CKYX_RET_NG;
    }

   return CKYX_RET_OK;

} /* end of CKYX_reply */

/****************************************************************************/
/*  FUNCTION        : 2.3.2  CKYX_atalla_pathsend                           */
/*  CALLING SEQ.    : short CKYX_atalla_pathsend(struct *, short  */
/*                                          ,char * )                       */
/*  ARGUMENT        : 1.snd_req        (I/O) PATHSEND情報                   */
/*                  : 2.atalla_srv_info(I)   ATALLA振分サーバ情報           */
/*                  : 3.cmd_len        (I)   コマンド長                     */
/*                  : 4.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : 5 :異常(PATHSENDエラー／ファイルIOエラー)             */
/*  DESCRIPTION     : ATALLAへのPATHSENDを行う                              */
/****************************************************************************/
short CKYX_atalla_pathsend(char  *snd_req
                           ,short cmd_len
                           ,char *cmd_rsp)
{
    short  err = 0;
    COM_PSD_arg_1_def arg1_data;
    COM_PSD_arg_2_def snd_trc;               /* PATHSENDモジュール引数 */
    COM_PSD_arg_3_def snd_res;               /* PATHSENDモジュール引数 */
    COM_PSD_arg_4_def ems_add;               /* EMS付加情報            */

    memset(&arg1_data, CKYX_SPACE, sizeof(arg1_data));
    memset(&snd_trc,   CKYX_SPACE, sizeof(snd_trc));
    memset(&snd_res,   CKYX_ZERO,  sizeof(snd_res));
    memset(&ems_add,   CKYX_ZERO,  sizeof(ems_add));

    memcpy(arg1_data.pathmon_name ,g_atalla_con.pathmon_name ,sizeof(g_atalla_con.pathmon_name));
    memcpy(arg1_data.serverclass_name ,g_atalla_con.server_class ,sizeof(g_atalla_con.server_class));
    memcpy(arg1_data.msg_buf ,snd_req ,cmd_len);
    arg1_data.req_send_len = cmd_len;
    arg1_data.receive_max_len = CKYX_MAX_DATA_SIZE;
    arg1_data.send_timer_msec = (long)g_atalla_con.pathsend_timer;
    arg1_data.retry_cnt = g_atalla_con.retry_cnt;
    arg1_data.receive_len = CKYX_ZERO;
    memcpy(snd_trc.prog_id,g_myinfo.prog_id ,sizeof(snd_trc.prog_id));

    /* サーバークラス論理ID設定 */
    memcpy( ems_add.srv_logical_id,
            g_myinfo.serverclass_name,
            sizeof(g_myinfo.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy( ems_add.lcn,
            g_lcn,
            sizeof(g_lcn));

    err = COM_PSD(&arg1_data ,&snd_trc ,&snd_res , &g_cg010in_modle, &ems_add);

    /* 処理結果判定 */
    if( err == 0 ){
        memcpy(cmd_rsp, arg1_data.msg_buf, arg1_data.receive_len);
        return(CKYX_RET_OK);
    } else {
        return(CKYX_RET_NG);
    }
} /* end of CKYX_atalla_pathsend */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_get_lcn                                   */
/*  CALLING SEQ.    : short CKYX_get_lcn (char *get_lcn)                    */
/*  ARGUMENT        : 1. get_lcn          (O) GFP内部LCN                    */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : GFP内部LCN取得処理                                    */
/****************************************************************************/
short CKYX_get_lcn (char *get_lcn)
{
    short ls_result = 0;
    c701_def st_lcn_ipc;          /* GFP内部LCN採番要求IPC設定テーブル */
    r701_def *r701_def_ipc;       /* GFP内部LCN採番応答IPC設定テーブル */

    /* PATHSENDパラメータ */
    COM_PSD_arg_1_def t_COM_PSD_arg_1_def;
    COM_PSD_arg_2_def t_COM_PSD_arg_2_def;
    COM_PSD_arg_3_def t_COM_PSD_arg_3_def;
    COM_PSD_arg_4_def t_COM_PSD_arg_4_def;

    /* GFP内部LCN採番要求IPC設定テーブル初期化 */
    memset( &st_lcn_ipc, 0, sizeof(st_lcn_ipc) );

    /* IPC interface_code設定 */
    memcpy( st_lcn_ipc.common_header.interface_code, DEF_IPC_IFCD_LCN_NUM_REQ,
             sizeof(st_lcn_ipc.common_header.interface_code) );

    /* IPC 内部エラーコードと予備にSPACEを設定 */
    memset( st_lcn_ipc.common_header.internal_error_code, CKYX_SPACE,
            sizeof(st_lcn_ipc.common_header.internal_error_code));
    memset( st_lcn_ipc.common_header.filler_1, CKYX_SPACE,
            sizeof(st_lcn_ipc.common_header.filler_1));

    /* IPC 呼び出し元プロセス名設定 */
    memcpy( st_lcn_ipc.process_name,
            g_myinfo.serverclass_name,
            sizeof(g_myinfo.serverclass_name));

    /* IPC 採番システム(サイトコード)設定 */
    st_lcn_ipc.site_code = g_myinfo.site_id;

    /* IPC 場所(NW識別)設定 */
    st_lcn_ipc.network_code = DEF_NW_ID_GFP;

    /* IPC データ長設定(26byte) */
    st_lcn_ipc.common_header.control_data_length =
           sizeof(st_lcn_ipc) - sizeof( st_lcn_ipc.common_header);

    /* PATHSENDパラメータ初期化 */
    memset( &t_COM_PSD_arg_1_def, 0, sizeof(t_COM_PSD_arg_1_def) );
    memset( &t_COM_PSD_arg_2_def, 0, sizeof(t_COM_PSD_arg_2_def) );
    memset( &t_COM_PSD_arg_3_def, 0, sizeof(t_COM_PSD_arg_3_def) );
    memset( &t_COM_PSD_arg_4_def, CKYX_SPACE, sizeof(t_COM_PSD_arg_4_def) );

    /* PATHSEND用情報設定 */
    memset(t_COM_PSD_arg_1_def.pathmon_name,
           CKYX_SPACE,
           sizeof(t_COM_PSD_arg_1_def.pathmon_name));
    if (g_lcncon_data.domain_name[0] != CKYX_SPACE){
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               g_lcncon_data.domain_name,
               sizeof(g_lcncon_data.domain_name));
    }
    else{
        memcpy(t_COM_PSD_arg_1_def.pathmon_name,
               g_lcncon_data.pathmon_name,
               sizeof(g_lcncon_data.pathmon_name));
    }

    memcpy(t_COM_PSD_arg_1_def.serverclass_name,
           g_lcncon_data.server_class,
           sizeof(g_lcncon_data.server_class));

    memcpy(t_COM_PSD_arg_1_def.msg_buf, &st_lcn_ipc, sizeof(st_lcn_ipc) );

    t_COM_PSD_arg_1_def.req_send_len    = sizeof(st_lcn_ipc);
    t_COM_PSD_arg_1_def.receive_max_len = CKYX_MAX_DATA_SIZE;
    t_COM_PSD_arg_1_def.send_timer_msec = (long)g_myinfo.send_timer;
    t_COM_PSD_arg_1_def.retry_cnt       = (short)g_myinfo.send_retry_count;

    memcpy(t_COM_PSD_arg_2_def.prog_id,
           g_myinfo.prog_id,
           sizeof(t_COM_PSD_arg_2_def.prog_id) );

    /* サーバークラス論理ID設定 */
    memcpy( t_COM_PSD_arg_4_def.srv_logical_id,
            g_myinfo.serverclass_name,
            sizeof(g_myinfo.serverclass_name));

    /* GFP内部LCN設定 */
    memcpy( t_COM_PSD_arg_4_def.lcn,
            g_lcn,
            sizeof(g_lcn));

    /* PATHSEND共通処理実行 */
    ls_result = COM_PSD( &t_COM_PSD_arg_1_def,
                         &t_COM_PSD_arg_2_def,
                         &t_COM_PSD_arg_3_def,
                         &g_cg010in_modle,
                         &t_COM_PSD_arg_4_def );

    /* PATHSEND結果確認 */
    if ( CKYX_RET_OK != ls_result ) {
        /* 異常終了 */
        /* GFP内部LCN取得エラー(EMS) */
        CMIN_message_output(DEF_EVT_LCN_GET_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
//                          DEF_NERR_KEY_LCN_GET_ERR,
                            DEF_NERR_LCN_GET_ERR,
                            "@L@T@2",
                            g_lcn,
                            &g_rcv_con_info,
                            ls_result);
        /* GFP内部LCN取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
//                DEF_NERR_KEY_LCN_GET_ERR,
                  DEF_NERR_LCN_GET_ERR,
                  sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    r701_def_ipc = (r701_def *)t_COM_PSD_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(R701) */
    if ( memcmp(r701_def_ipc->common_header.interface_code,
                DEF_IPC_IFCD_LCN_NUM_RSP,
                sizeof(r701_def_ipc->common_header.interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if (r701_def_ipc->common_header.error_code != DEF_IPC_ERRCD_OK) {
            /* 異常 */
            ls_result = CKYX_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        ls_result = CKYX_RET_NG;
    }
    if ( ls_result == CKYX_RET_NG ) {
        /* EMS出力 応答エラー */
        CMIN_message_output(DEF_EVT_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
//                          DEF_NERR_KEY_LCN_GET_ERR,
                            DEF_NERR_LCN_GET_ERR,
                            "@L@C@C@H",
                            g_lcn, g_myinfo.serverclass_name,
                            "R701 ERROR          ",
                            r701_def_ipc);
        /* GFP内部LCN取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
//                DEF_NERR_KEY_LCN_GET_ERR,
                  DEF_NERR_LCN_GET_ERR,
                  sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    /* GFP内部LCNを受信バッファから取得 */
    memcpy( get_lcn, (char *)&r701_def_ipc->gfplcn, CKYX_LCN_LENG );

    return CKYX_RET_OK;

} /* end of CKYX_get_lcn */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_system_num_get                            */
/*  CALLING SEQ.    : short CKYX_system_num_get (char *get_sys_no)          */
/*  ARGUMENT        : 1. get_sys_no     (O) システム採番                    */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : システム採番生成処理：                                */
/****************************************************************************/
short CKYX_system_num_get (char *get_sys_no)
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
    memset( &t_COM_PSD_SYS_arg_1_def, 0, sizeof(t_COM_PSD_SYS_arg_1_def) );
    memset( &t_COM_PSD_SYS_arg_2_def, 0, sizeof(t_COM_PSD_SYS_arg_2_def) );
    memset( &t_COM_PSD_SYS_arg_3_def, 0, sizeof(t_COM_PSD_SYS_arg_3_def) );
    memset( &t_COM_PSD_SYS_arg_4_def, CKYX_SPACE, sizeof(t_COM_PSD_SYS_arg_4_def) );

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
    t_COM_PSD_SYS_arg_1_def.receive_max_len = CKYX_MAX_DATA_SIZE;
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
            g_lcn,
            sizeof(g_lcn));

    /* PATHSEND共通処理実行 */
    ls_result = COM_PSD_SYS( &t_COM_PSD_SYS_arg_1_def,
                             &t_COM_PSD_SYS_arg_2_def,
                             &t_COM_PSD_SYS_arg_3_def,
                             &g_cg010in_modle,
                             &t_COM_PSD_SYS_arg_4_def );

    /* PATHSEND結果確認 */
    if ( CKYX_RET_OK != ls_result ) {
        /* 異常終了 */
        /* システム採番取得エラー(EMS) */
        CMIN_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SYS_NO_MAKE_ERR,
                            "@C@2",
                            "COM_PSD_SYS",
                            t_COM_PSD_SYS_arg_3_def.guardian_errcode);

        /* システム採番取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
                 DEF_NERR_SYS_NO_MAKE_ERR,
                 sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    ss02_def_ipc = (ss02_def *)t_COM_PSD_SYS_arg_1_def.msg_buf;
    /* 応答電文のインタフェースコード判定(SS02) */
    if ( memcmp(ss02_def_ipc->interface_code,
                DEF_IPC_IFCD_SYSTEM_NUM_RSP,
                sizeof(ss02_def_ipc->interface_code)) == 0 ) {

        /* 応答電文のエラーコード判定 */
        if ((ss02_def_ipc->internal_error_code[0] != CKYX_SPACE) &&
            (ss02_def_ipc->internal_error_code[0] != CKYX_CHAR_ZERO)){
            /* 異常 */
            ls_result = CKYX_RET_NG;
        }
    }
    else {
        /* 異常処理実行 */
        ls_result = CKYX_RET_NG;
    }
    if ( ls_result == CKYX_RET_NG ) {
        /* EMS出力 応答エラー */
        CMIN_message_output(DEF_EVT_RSP_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_SYS_NO_MAKE_ERR,
                            "@L@C@C@H",
                            g_lcn,
                            g_myinfo.serverclass_name,
                            "SS02 ERROR          ",
                            ss02_def_ipc);
        /* システム採番取得エラー(内部エラー) */
        strncpy( g_internal_error_code,
                  DEF_NERR_SYS_NO_MAKE_ERR,
                  sizeof(g_internal_error_code));
        return CKYX_RET_NG;
    }

    /* システム採番を受信バッファから取得 */
    memcpy( get_sys_no, ss02_def_ipc->numbering_value, sizeof(ss02_def_ipc->numbering_value) );

    return CKYX_RET_OK;

} /* end of CKYX_system_num_get */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_ctl_log_output                            */
/*  CALLING SEQ.    : short CKYX_ctl_log_output ( char rcv_ipc,             */
/*                                                char write_flg,           */
/*                                                char err_kind,            */
/*                                                char *err_code)           */
/*  ARGUMENT        : 1. rcv_ipc       (I) 受信IPC種別                      */
/*                  : 2. write_flg     (I) 書き込みフラグ                   */
/*                  : 2. err_kind      (I) 送受信識別                       */
/*                  : 3. err_code      (I) ログ設定用エラー番号             */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : 制御ログ作成出力処理                                  */
/****************************************************************************/
short CKYX_ctl_log_output ( char *rcv_ipc,
                            char write_flg,
                            char err_kind,
                            char *err_code)
{
    short ls_result = 0;

    cr401_def *c401_ipc;
    cr402_def *c402_ipc;
    common_header_def *ipc_head;
    short      rcv_ipc_flg  = 0;
    short      io_err_code  = 0;

    db_glmlg_def st_log_tbl;         /* ログ出力テーブル */
    db_glmlg_def dmy_tbl;            /* ダミーテーブル   */

    char         ch_leng_set[6];
    short        data_len          = 0;

    COM_SDT_arg_2_def  COM_SDT_arg_2_data;       /* 日時取得用データ        */
    COM_SDT_arg_3_def  COM_SDT_arg_3_data;       /* 日時取得用データ        */
    long long          date_time;                /* 日時取得用データ        */

    /* 日時取得用データ初期化 */
    memset(&COM_SDT_arg_2_data, CKYX_SPACE, sizeof(COM_SDT_arg_2_data));
    memset(&COM_SDT_arg_3_data, CKYX_ZERO , sizeof(COM_SDT_arg_3_data));
    date_time = CKYX_ZERO;

    /* 受信IPC種別確認 */
    ipc_head = (common_header_def *)rcv_ipc;
    if ((memcmp(ipc_head->interface_code,
               DEF_IPC_IFCD_NW_MSG_REQ,
               sizeof(ipc_head->interface_code)) == 0 ) ||
        (memcmp(ipc_head->interface_code,
               DEF_IPC_IFCD_NW_MSG_RSP,
               sizeof(ipc_head->interface_code)) == 0 )){
        rcv_ipc_flg = CKYX_C401_FLG;
        c401_ipc = (cr401_def*)rcv_ipc;
    }
    else {
        rcv_ipc_flg = CKYX_C402_FLG;
        c402_ipc = (cr402_def*)rcv_ipc;
    }

    /* ログ出力要求テーブル初期化 */
    memset( &st_log_tbl, CKYX_SPACE, sizeof(st_log_tbl) );

    /* 書き込みフラグ判定 */
    if (write_flg == CKYX_LOG_SET){
        /* 新規 */
        if (rcv_ipc_flg == CKYX_C401_FLG ){
            /* プライマリーキー GFP内部LCN設定 */
            if ((memcmp(c401_ipc->control_info.request_kind,
                        DEF_CTLREQ_SIMUKE,
                        sizeof(c401_ipc->control_info.request_kind)) == 0) ||
                (memcmp(c401_ipc->control_info.request_kind,
                        DEF_CTLREQ_TIMEOUT,
                        sizeof(c401_ipc->control_info.request_kind)) == 0) ||
                (memcmp(c401_ipc->control_info.request_kind,
                        DEF_CTLREQ_SIMUKE_ERROR,
                        sizeof(c401_ipc->control_info.request_kind)) == 0)){
                /* 仕向応答 */
                /* プライマリーキー パーティションID設定 */
                st_log_tbl.pri_key.part_id[0] = '0';
                st_log_tbl.pri_key.part_id[1] = c401_ipc->control_info.req_gfp_lcn[14];
                /* プライマリーキー LCN設定 */
                memcpy( st_log_tbl.pri_key.lcn_id,
                        c401_ipc->control_info.req_gfp_lcn,
                        sizeof(st_log_tbl.pri_key.lcn_id) );
            }
            else {
                /* 被仕向/ 仕向要求 */
                /* プライマリーキー パーティションID設定 */
                st_log_tbl.pri_key.part_id[0] = '0';
                st_log_tbl.pri_key.part_id[1] = c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn[14];
                /* プライマリーキー LCN設定 */
                memcpy( st_log_tbl.pri_key.lcn_id,
                        c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn,
                        sizeof(st_log_tbl.pri_key.lcn_id) );
            }

            /* プライマリーキー 仕向・被仕向区分設定 */
            if (memcmp(c401_ipc->control_info.request_kind,
                       DEF_CTLREQ_HISIMUKE,
                       sizeof(c401_ipc->control_info.request_kind)) == 0) {
                /* 被仕向要求 */
                st_log_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_HISIMUKE;
                if (c401_ipc->control_info.response_kind[0] == CKYX_SPACE){
                    /* プライマリーキー 送受信識別 */
                    st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_REQUEST;
                }
                else {
                    /* プライマリーキー 送受信識別 */
                    st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_RESPONSE;
                }
            }
            else if (memcmp(c401_ipc->control_info.request_kind,
                       DEF_CTLREQ_SIMUKE,
                       sizeof(c401_ipc->control_info.request_kind)) == 0) {
                /* 仕向応答*/
                st_log_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_SIMUKE;
                /* プライマリーキー 送受信識別 */
                st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_RESPONSE;
            }
            else if (memcmp(c401_ipc->control_info.request_kind,
                       DEF_CTLREQ_TIMEOUT,
                       sizeof(c401_ipc->control_info.request_kind)) == 0) {
                /* 仕向応答タイムアウト*/
                st_log_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_SIMUKE;
                /* プライマリーキー 送受信識別 */
                st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_RESPONSE;
            }
            else {
                /* 動作しないルート */
                st_log_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_SIMUKE;
                /* プライマリーキー 送受信識別 */
                st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_REQUEST;
            }

            /* コネクション論理ID設定 */
            memcpy( &st_log_tbl.connect_id,
                    &c401_ipc->control_info.connection_lid,
                    sizeof(st_log_tbl.connect_id));

            /* MTI */
            memcpy( st_log_tbl.mti_id,
                    c401_ipc->control_info.mti,
                    sizeof(st_log_tbl.mti_id) );

            /* 制御電文種別 */
            memcpy(st_log_tbl.control_kind,
                    &c401_ipc->control_info.control_kind,
                    sizeof(st_log_tbl.control_kind));

            /* 送信不可情報 */
            memcpy(st_log_tbl.send_naibu_err_code,
                    c401_ipc->control_info.send_naibu_err_code,
                    sizeof(st_log_tbl.send_naibu_err_code));

            /* 内部エラーコード */
            memcpy(st_log_tbl.naibu_err_code,
                    g_internal_error_code,
                    sizeof(st_log_tbl.naibu_err_code));

            /* サーバークラス論理ID設定 */
            memcpy( st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_id,
                    g_myinfo.serverclass_name, 
                    sizeof(st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_id) );

            /* サーバクラス冗長化番号 */
            memset( st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_mlt_num,
                    '0',
                    sizeof(st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_mlt_num) );

            /* 電文長取得 */
            data_len = c401_ipc->common_header.control_data_length -
                       sizeof(c401_ipc->control_info);

            if (data_len <= 0){
                /* 電文なし */
                /* 電文情報有無設定 電文なし */
                st_log_tbl.denbun_info_exist = CKYX_CTL_LOG_DATA_NASHI;
                /* 電文長設定 */
                memset( st_log_tbl.denbun_area.denbun_len, '0',
                        sizeof(st_log_tbl.denbun_area.denbun_len));
            }
            else{
                /* 電文情報有無設定 電文あり */
                st_log_tbl.denbun_info_exist = CKYX_CTL_LOG_DATA_ARI;
                /* 電文長設定 */
                memset( ch_leng_set, 0, sizeof(ch_leng_set));
                snprintf( ch_leng_set,
                          6,
                         "%05d",
                         data_len);
                memcpy( st_log_tbl.denbun_area.denbun_len, ch_leng_set, 5);
                /* 電文設定 */
                memcpy( st_log_tbl.denbun_area.denbun,
                        c401_ipc->data_bu.message_text,
                        data_len);
            }
        }
        else{
            /* プライマリーキー パーティションID設定 */
            st_log_tbl.pri_key.part_id[0] = '0';
            st_log_tbl.pri_key.part_id[1] = c402_ipc->control_info.denbun_log_key.tran_id.gfp_lcn[14];

            /* プライマリーキー GFP内部LCN設定 */
            memcpy( st_log_tbl.pri_key.lcn_id,
                    c402_ipc->control_info.denbun_log_key.tran_id.gfp_lcn,
                    sizeof(g_lcn) );

            /* 仕向設定 */
            st_log_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_SIMUKE;
            st_log_tbl.pri_key.send_recv_id = DEF_CTLMSG_REQUEST;

            /* コネクション論理ID設定 */
            memcpy( &st_log_tbl.connect_id,
                    &c402_ipc->control_info.connection_lid,
                    sizeof(st_log_tbl.connect_id));

            /* MTI */
            memcpy( st_log_tbl.mti_id,
                    c402_ipc->control_info.mti,
                    sizeof(st_log_tbl.mti_id) );

            /* 制御電文種別 */
            memcpy(st_log_tbl.control_kind,
                    &c402_ipc->control_info.control_kind,
                    sizeof(st_log_tbl.control_kind));

            /* 内部エラーコード */
            memcpy(st_log_tbl.naibu_err_code,
                    g_internal_error_code,
                    sizeof(st_log_tbl.naibu_err_code));

            /* サーバークラス論理ID設定 */
            memcpy( st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_id,
                    g_myinfo.serverclass_name, 
                    sizeof(st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_id) );

            /* サーバクラス冗長化番号 */
            memset( st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_mlt_num,
                    '0',
                    sizeof(st_log_tbl.cntrl_denbun_srv_cls_info.srv_cls_mlt_num) );

            /* 電文長取得 */
            data_len = c402_ipc->common_header.control_data_length -
                       sizeof(c402_ipc->control_info);

            if (data_len <= 0){
                /* 電文なし */
                /* 電文情報有無設定 電文なし */
                st_log_tbl.denbun_info_exist = CKYX_CTL_LOG_DATA_NASHI;
                /* 電文長設定 */
                memset( st_log_tbl.denbun_area.denbun_len, '0',
                        sizeof(st_log_tbl.denbun_area.denbun_len));
            }
            else{
                /* 電文情報有無設定 電文あり */
                st_log_tbl.denbun_info_exist = CKYX_CTL_LOG_DATA_ARI;
                /* 電文長設定 */
                memset( ch_leng_set, 0, sizeof(ch_leng_set));
                snprintf( ch_leng_set,
                          6,
                         "%05d",
                         data_len);
                memcpy( st_log_tbl.denbun_area.denbun_len, ch_leng_set, 5);
                /* 電文設定 */
                memcpy( st_log_tbl.denbun_area.denbun,
                        c402_ipc->data_bu.message_text,
                        data_len);
            }
        }

        /* 制御ログ登録時間を設定 */
        /* システム日時取得 */
        COM_SDT(DEF_COM_SDT_arg1_jpn, &COM_SDT_arg_2_data,
                &COM_SDT_arg_3_data,  &date_time);

        memcpy(st_log_tbl.entry_timestamp,
               (char *)&COM_SDT_arg_2_data,
               sizeof(st_log_tbl.entry_timestamp));

        /* 鍵種類を設定 */
        memcpy(st_log_tbl.proc_result_info.key_type,
               g_key_kind,
               sizeof(g_key_kind));
        memset( st_log_tbl.proc_result_info.future_use, CKYX_SPACE, sizeof(st_log_tbl.proc_result_info.future_use));

        /* ダミー領域にスペースを設定 */
        memset( st_log_tbl.future_use, CKYX_SPACE, sizeof(st_log_tbl.future_use));

        io_err_code = 0;

        ls_result = CMIN_put_glmlg( g_file_info.glmlg_fname,  // 物理ファイル名
                                     g_file_info.glmlg_fno,   // ファイル番号
                                     (char*)&st_log_tbl,      // 読込みキー
                                     DEF_COM_IOM_LOCK,        // LOCK有無
                                     (char*)&dmy_tbl,         // 読込んだレコード
                                     &io_err_code);           // I/Oエラーコード
        if (ls_result != CKYX_RET_OK){
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_FILE_IO_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_CTL_LOG_OUTPUT_ERR,
                                "@L@T@C@C@C@5",
                                g_lcn,
                                &st_log_tbl.connect_id,
                                DEF_FL_CTRL_DEN_LOG,
                                DEF_COM_IOM_FUNC_ADD,
                                "",
                                io_err_code);
            return CKYX_RET_NG;
        }
        ls_result = CMIN_unlock_glmlg( g_file_info.glmlg_fname,  // 物理ファイル名
                                       g_file_info.glmlg_fno,    // ファイル番号
                                       (char*)&st_log_tbl,       // 読込みキー
                                       DEF_COM_IOM_LOCKFREE,     // LOCK有無
                                       (char*)&dmy_tbl,          // 読込んだレコード
                                       &io_err_code);            // I/Oエラーコード

        if (ls_result != CKYX_RET_OK){
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_FILE_IO_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_CTL_LOG_OUTPUT_ERR,
                                "@L@T@C@C@C@5",
                                g_lcn,
                                &st_log_tbl.connect_id,
                                DEF_FL_CTRL_DEN_LOG,
                                DEF_COM_IOM_FUNC_UNLOC,
                                "",
                                io_err_code);
            return CKYX_RET_NG;
        }
    }
    else{
        /* 更新 */
        memset((char*)&dmy_tbl, 0, sizeof(dmy_tbl));

        /* プライマリーキー 仕向・被仕向区分設定 */
        if (memcmp(c401_ipc->control_info.request_kind,
                   DEF_CTLREQ_HISIMUKE_ERROR,
                   sizeof(c401_ipc->control_info.request_kind)) == 0) {
            /* 被仕向応答送信不可 */
            dmy_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_HISIMUKE;
            /* プライマリーキー 送受信識別 */
            dmy_tbl.pri_key.send_recv_id = DEF_CTLMSG_RESPONSE;
            /* プライマリーキー パーティションID設定 */
            dmy_tbl.pri_key.part_id[0] = '0';
            dmy_tbl.pri_key.part_id[1] = c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn[14];
            /* プライマリーキー GFP内部LCN設定 */
            memcpy( dmy_tbl.pri_key.lcn_id,
                    c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn,
                    sizeof(g_lcn) );
        }
        else {
            /* 仕向要求送信不可 または 仕向タイムアウト */
            dmy_tbl.pri_key.s_h_kubun = CKYX_CTL_LOG_SIMUKE;
            /* プライマリーキー 送受信識別 */
            dmy_tbl.pri_key.send_recv_id = DEF_CTLMSG_REQUEST;
            /* プライマリーキー パーティションID設定 */
            dmy_tbl.pri_key.part_id[0] = '0';
            dmy_tbl.pri_key.part_id[1] = c401_ipc->control_info.req_gfp_lcn[14];
            /* プライマリーキー GFP内部LCN設定 */
            memcpy( dmy_tbl.pri_key.lcn_id,
                    c401_ipc->control_info.req_gfp_lcn,
                    sizeof(g_lcn) );
            if (memcmp(c401_ipc->control_info.request_kind,
                    DEF_CTLREQ_TIMEOUT,
                    sizeof(c401_ipc->control_info.request_kind)) == 0) {
                /* 仕向タイムアウト */
                /* プライマリーキー 送受信識別 */
                dmy_tbl.pri_key.send_recv_id = DEF_CTLMSG_RESPONSE;
                
            }
        }

        ls_result = CMIN_read_glmlg( g_file_info.glmlg_fname,  // 物理ファイル名
                                     g_file_info.glmlg_fno,    // ファイル番号
                                     (char*)&dmy_tbl,          // 読込みキー
                                     DEF_COM_IOM_LOCK,         // LOCK有無
                                     (char*)&st_log_tbl,       // 読込んだレコード
                                     &io_err_code);            // I/Oエラーコード

        if (ls_result != CKYX_RET_OK){
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_FILE_IO_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_CTL_LOG_OUTPUT_ERR,
                                "@L@T@C@C@C@5",
                                g_lcn,
                                &st_log_tbl.connect_id,
                                DEF_FL_CTRL_DEN_LOG,
                                DEF_COM_IOM_FUNC_STARTREAD,
                                "",
                                io_err_code);
            return CKYX_RET_NG;
        }

        /* 鍵種類取得 */
        memcpy(g_key_kind,
               st_log_tbl.proc_result_info.key_type,
               sizeof(g_key_kind));

        /* 送信不可情報設定 */
        memcpy(st_log_tbl.send_naibu_err_code,
               c401_ipc->control_info.send_naibu_err_code,
                sizeof(st_log_tbl.send_naibu_err_code));

        ls_result = CMIN_update_glmlg( g_file_info.glmlg_fname,  // 物理ファイル名
                                       g_file_info.glmlg_fno,    // ファイル番号
                                       (char*)&st_log_tbl,       // 読込みキー
                                       DEF_COM_IOM_LOCK,         // LOCK有無
                                       (char*)&dmy_tbl,          // 読込んだレコード
                                       &io_err_code);            // I/Oエラーコード
        if (ls_result != CKYX_RET_OK){
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_FILE_IO_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_CTL_LOG_OUTPUT_ERR,
                                "@L@T@C@C@C@5",
                                g_lcn,
                                &st_log_tbl.connect_id,
                                DEF_FL_CTRL_DEN_LOG,
                                DEF_COM_IOM_FUNC_UPDATE,
                                "",
                                io_err_code);
            return CKYX_RET_NG;
        }
        ls_result = CMIN_unlock_glmlg( g_file_info.glmlg_fname,  // 物理ファイル名
                                       g_file_info.glmlg_fno,    // ファイル番号
                                       (char*)&st_log_tbl,       // 読込みキー
                                       DEF_COM_IOM_LOCKFREE,     // LOCK有無
                                       (char*)&dmy_tbl,          // 読込んだレコード
                                       &io_err_code);            // I/Oエラーコード

        if (ls_result != CKYX_RET_OK){
            /* EMS出力 */
            CMIN_message_output(DEF_EVT_FILE_IO_ERR,
                                DEF_MSGTTKB_GYOM_ERR,
                                DEF_NERR_CTL_LOG_OUTPUT_ERR,
                                "@L@T@C@C@C@5",
                                g_lcn,
                                &st_log_tbl.connect_id,
                                DEF_FL_CTRL_DEN_LOG,
                                DEF_COM_IOM_FUNC_UNLOC,
                                "",
                                io_err_code);
            return CKYX_RET_NG;
        }
    }

    return CKYX_RET_OK;

} /* end of CKYX_ctl_log_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_err_log_output                            */
/*  CALLING SEQ.    : short CKYX_err_log_output ( char *rcv_ipc,            */
/*                                                char err_kind,            */
/*                                                char *err_code)           */
/*  ARGUMENT        : 1. rcv_ipc    (I) 受信IPC種別                         */
/*                  : 3. err_kind   (I) ログ設定用エラー種別                */
/*                  : 4. err_code   (I) ログ設定用エラー番号                */
/*  RETURN CODE     : 0:正常終了 -1:異常終了                                */
/*  DESCRIPTION     : エラーログ作成出力処理                                */
/****************************************************************************/
short CKYX_err_log_output ( char *rcv_ipc,
                            char err_kind,
                            char *err_code)
{
    short ls_result   = 0;
    char  rcv_ipc_flg = 0;
    cr401_def *c401_ipc;
    cr402_def *c402_ipc;

    db_glelg_def      st_log_tbl;         /* ログ出力テーブル */
    common_header_def *ipc_head;

    unsigned int s_data_all_leng   = 0;
    char         ch_leng_set[6];
    short        data_len          = 0;

    /* タイムスタンプパラメータ */
    COM_UNQ_arg_1_def t_COM_UNQ_arg_1_def;
    char ch_datetime_hex[16+1];

    /* 受信IPC種別確認 */
    ipc_head = (common_header_def *)rcv_ipc;
    if (memcmp(ipc_head->interface_code,
               DEF_IPC_IFCD_NW_MSG_REQ,
               sizeof(ipc_head->interface_code)) == 0 ){
        rcv_ipc_flg = CKYX_C401_FLG;
        c401_ipc = (cr401_def*)rcv_ipc;
    }
    else {
        rcv_ipc_flg = CKYX_C402_FLG;
        c402_ipc = (cr402_def*)rcv_ipc;
    }

    /* エラーログ出力パラメータ */
    COM_ERL_arg_1_def t_COM_ERL_arg_1_def;
    COM_ERL_arg_3_def t_COM_ERL_arg_3_def;

    /* ログ出力要求テーブル初期化 */
    memset( &st_log_tbl, 0, sizeof(st_log_tbl) );

    /* エラーログ プライマリーキー設定 */
    memset( (char *)&t_COM_UNQ_arg_1_def, 0, sizeof(t_COM_UNQ_arg_1_def));
    memset( ch_datetime_hex, 0, sizeof(ch_datetime_hex));
    COM_UNQ( &t_COM_UNQ_arg_1_def, ch_datetime_hex );

    /* エラーログ プライマリーキー パーティションIP設定 */
    memcpy( &st_log_tbl.pri_key.part_id[0],
            &t_COM_UNQ_arg_1_def.cc[1],
            2 );

    /* エラーログ プライマリーキー タイムスタンプ設定 */
    memcpy( st_log_tbl.pri_key.time_stamp,
            &t_COM_UNQ_arg_1_def,
            sizeof(st_log_tbl.pri_key.time_stamp) );

    /* エラーログ プライマリーキー タイムスタンプ枝番設定 */
    memcpy( st_log_tbl.pri_key.time_stamp_branch,
            &ch_datetime_hex[8],
            sizeof(st_log_tbl.pri_key.time_stamp_branch) );

    /* エラーログ エラー電文識別 */
    st_log_tbl.err_denbun_id = err_kind;

    /* エラーログ MTI設定 */
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        memcpy( st_log_tbl.mti_id,
                c401_ipc->control_info.mti,
                sizeof(st_log_tbl.mti_id) );
    }
    else {
        memcpy( st_log_tbl.mti_id,
                c402_ipc->control_info.mti,
                sizeof(st_log_tbl.mti_id) );
    }

    /* エラーログ レスポンスコード スペース設定 */
    memset( st_log_tbl.res_code, ' ', 
            sizeof(st_log_tbl.res_code) );

    /* エラーログ 内部エラーコード 設定 */
    memcpy( st_log_tbl.naibu_err_code,
            g_internal_error_code, 
            sizeof(st_log_tbl.naibu_err_code) );

    /* エラーログ サーバークラス論理ID設定 */
    memcpy( st_log_tbl.srv_cls_info.srv_cls_id,
            g_myinfo.serverclass_name, 
            sizeof(st_log_tbl.srv_cls_info.srv_cls_id) );

    /* エラーログ サーバクラス冗長化番号 */
    memset( st_log_tbl.srv_cls_info.srv_cls_mlt_num,
            '0',
            sizeof(st_log_tbl.srv_cls_info.srv_cls_mlt_num) );

    /* エラーログ 電文送受信情報 電文受信時刻設定 */
    memcpy( st_log_tbl.denbun_send_recv_info.denbun_recv_time,
            st_log_tbl.pri_key.time_stamp,
            sizeof(st_log_tbl.denbun_send_recv_info.denbun_recv_time) );

    /* エラーログ 電文送受信情報 受信時局状態 */
    /* 局状態未取得の場合取得 */
    if (g_station_sts[0] == CKYX_SPACE){
        /* 局状態取得処理 */
        if (rcv_ipc_flg == CKYX_C401_FLG ){
            ls_result = CKYX_read_station_st(&c401_ipc->control_info.connection_lid,
                                             g_station_sts);
        }
        else {
            ls_result = CKYX_read_station_st(&c402_ipc->control_info.connection_lid,
                                             g_station_sts);
        }
        if (ls_result != CKYX_RET_OK){
            /* 局状態取得失敗時はスペースをそのまま設定 */
        }
    }

    /* 局状態を設定 */
    if (memcmp(g_station_sts, DEF_STTE_STS_OPN, sizeof(g_station_sts)) ==0){
        st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = CKYX_STATION_ST_OPEN;
    }
    else if (memcmp(g_station_sts, DEF_STTE_STS_OPNING, sizeof(g_station_sts)) ==0){
        st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = CKYX_STATION_ST_OPEN_PROC;
    }
    else if (memcmp(g_station_sts, DEF_STTE_STS_CLOSING, sizeof(g_station_sts)) ==0){
        st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = CKYX_STATION_ST_CLOSE_PROC;
    }
    else if (memcmp(g_station_sts, DEF_STTE_STS_CLS, sizeof(g_station_sts)) ==0){
        st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = CKYX_STATION_ST_CLOSE;
    }
    else {
        st_log_tbl.denbun_send_recv_info.recv_kyoku_sts = CKYX_SPACE;
    }

    /* エラーログ 電文送受信情報 NW区分設定 */
    memcpy( st_log_tbl.denbun_send_recv_info.nw_kubun,
            g_nwi_g.nw_id_info.nw_kubun,
            sizeof(st_log_tbl.denbun_send_recv_info.nw_kubun) );

    /* エラーログ 電文送受信情報 MTI */
    memcpy( st_log_tbl.denbun_send_recv_info.mti_id,
            st_log_tbl.mti_id,
            sizeof(st_log_tbl.denbun_send_recv_info.mti_id));

    /* エラーログ 電文送受信情報 送信電文種別 */
    st_log_tbl.denbun_send_recv_info.send_denbun_shubetu = CKYX_SPACE;

    /* エラーログ 電文送受信情報 電文ログKEY GFP内部LCN */
    /* エラーログ 電文送受信情報 電文ログKEY 電文形態 */
    /* エラーログ 電文送受信情報 電文ログKEY 電文種別 */
    /* エラーログ 電文送受信情報 電文ログKEY 再送回数 */
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        memcpy( &st_log_tbl.denbun_send_recv_info.denbun_log_key,
                &c401_ipc->control_info.denbun_log_key,
                sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key));
    }
    else {
        memcpy( &st_log_tbl.denbun_send_recv_info.denbun_log_key,
                &c402_ipc->control_info.denbun_log_key,
                sizeof(st_log_tbl.denbun_send_recv_info.denbun_log_key));
    }
    /* エラーログ 電文送受信情報 電文フォーマット区分 */
    st_log_tbl.denbun_send_recv_info.denbun_fmt_kubun = '0';

    /* エラーログ 電文送受信情報 通信ログ保存ファイル名 */
    memset( st_log_tbl.denbun_send_recv_info.tushin_log_save_filename,
            CKYX_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_save_filename) );

    /* エラーログ 電文送受信情報 通信ログKEY */
    memset( &st_log_tbl.denbun_send_recv_info.tushin_log_key,
            CKYX_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.tushin_log_key));

    /* エラーログ 通信制御情報 インタフェース */
    memset( st_log_tbl.tushin_cntrl_info.if_id,
            CKYX_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.if_id) );

    /* エラーログ 通信制御情報 ステーション */
    memset( st_log_tbl.tushin_cntrl_info.station_id,
            CKYX_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.station_id) );

    /* エラーログ 通信制御情報 回線情報 受信コネクション論理ID */
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id,
                &c401_ipc->control_info.connection_lid,
                sizeof(st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id) );
    }
    else{
        memcpy( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id,
                &c402_ipc->control_info.connection_lid,
                sizeof(st_log_tbl.tushin_cntrl_info.line_info.recv_connect_id) );
    }

    /* エラーログ 通信制御情報 回線情報 受信コネクション情報 */
    memset( &st_log_tbl.tushin_cntrl_info.line_info.recv_connect_info,
            CKYX_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info.recv_connect_info) );

    /* エラーログ 通信制御情報 回線情報 電文受信タイムスタンプ */
    memset( &st_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp,
            CKYX_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.line_info.denbun_recv_time_stamp));

    /* ダミー領域クリア */
    memset( st_log_tbl.future_use, CKYX_SPACE, sizeof(st_log_tbl.future_use));
    memset( st_log_tbl.denbun_send_recv_info.future_use,
            CKYX_SPACE,
            sizeof(st_log_tbl.denbun_send_recv_info.future_use));
    memset( st_log_tbl.tushin_cntrl_info.future_use,
            CKYX_SPACE,
            sizeof(st_log_tbl.tushin_cntrl_info.future_use));

    /* エラーログ 電文長設定 */
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        data_len = c401_ipc->common_header.control_data_length -
                   sizeof(c401_ipc->control_info);
    }
    else{
        data_len = c402_ipc->common_header.control_data_length -
                   sizeof(c402_ipc->control_info);
    }

    memset( ch_leng_set, 0, sizeof(ch_leng_set));
    snprintf( ch_leng_set,
              6,
             "%05d",
             data_len);
    memcpy( st_log_tbl.denbun_area.denbun_len, ch_leng_set, 5);

    /* エラーログ MTI開始位置設定 */
    memset( ch_leng_set, 0, sizeof(ch_leng_set));
    if (data_len == 0){
        /* データ部なし */
        /* NW情報ファイルのMTI開始位置に0を設定 */
        memset( st_log_tbl.denbun_area.mti_start_lct, '0', 5);
    }
    else {
        /* NW情報ファイルのMTI開始位置を設定 */
        memcpy( st_log_tbl.denbun_area.mti_start_lct,
                g_nwi_g.denbun_item_lct_info.mti_start_lct, 5);
    }

    /* エラーログ 送受信電文設定 */
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        memcpy( st_log_tbl.denbun_area.denbun,
                c401_ipc->data_bu.message_text,
                data_len);
    }
    else {
        memcpy( st_log_tbl.denbun_area.denbun,
                c402_ipc->data_bu.message_text,
                data_len);
    }

    /* エラーログ 送受信電文全体長設定 */
    s_data_all_leng = (unsigned int)
         (sizeof(st_log_tbl) - sizeof(st_log_tbl.denbun_area.denbun) + data_len);

    /* エラーログ用パラメータ初期化 */
    memset( &t_COM_ERL_arg_1_def, 0, sizeof(t_COM_ERL_arg_1_def) );
    memset( &t_COM_ERL_arg_3_def, CKYX_SPACE, sizeof(t_COM_ERL_arg_3_def) );

    /* エラーログ用情報設定 */
    t_COM_ERL_arg_1_def.file_io_type = CKYX_ERR_LOG_WRITE;
    t_COM_ERL_arg_1_def.io_timer     = (long)g_myinfo.send_timer;
    t_COM_ERL_arg_1_def.data_len     = (short)s_data_all_leng;

    t_COM_ERL_arg_1_def.data_area = (char *)&st_log_tbl;

    /* サーバークラス論理ID設定 */
    memcpy(t_COM_ERL_arg_3_def.srv_logical_id,
           g_myinfo.serverclass_name,
           sizeof(g_myinfo.serverclass_name));
    if (rcv_ipc_flg == CKYX_C401_FLG ){
        /* GFP内部LCN設定 */
        memcpy(t_COM_ERL_arg_3_def.lcn,
                c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn,
                sizeof(c401_ipc->control_info.denbun_log_key.tran_id.gfp_lcn));

        /* 接続先(サイト識別～コネクション識別) */
        memcpy(t_COM_ERL_arg_3_def.connect,
               &c401_ipc->control_info.connection_lid,
               sizeof(c401_ipc->control_info.connection_lid));
    }
    else {
        /* GFP内部LCN設定 */
        memcpy(t_COM_ERL_arg_3_def.lcn,
                c402_ipc->control_info.denbun_log_key.tran_id.gfp_lcn,
                sizeof(c402_ipc->control_info.denbun_log_key.tran_id.gfp_lcn));

        /* 接続先(サイト識別～コネクション識別) */
        memcpy(t_COM_ERL_arg_3_def.connect,
               &c402_ipc->control_info.connection_lid,
               sizeof(c402_ipc->control_info.connection_lid));
    }

    /* エラーログ共通処理実行 */
    ls_result = COM_ERL( &t_COM_ERL_arg_1_def,
                         &g_com_erl_arg_2,
                         &g_cg010in_modle,
                         &t_COM_ERL_arg_3_def,
                         g_myinfo.prog_id);

    /* エラーログ結果判定 */
    if ( CKYX_RET_OK != ls_result ){
        /* 異常終了 */
        /* EMS出力 */
        CMIN_message_output(DEF_EVT_COMMON_MOD_ERR,
                            DEF_MSGTTKB_GYOM_ERR,
                            DEF_NERR_ERRLOG_OUTPUT_ERR,
                            "@C@2", "COM_ERL", ls_result);
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* end of CKYX_err_log_output */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_all_file_read                              */
/*  CALLING SEQ.    : short CKYX_all_file_read(gflin_pkey_def *connection_lid)*/
/*  ARGUMENT        : 1. connection_lid       (I) センターID                 */
/*  RETURN CODE     :  0:Normal End                                          */
/*  RETURN CODE     : -1:Error End                                           */
/*  DESCRIPTION     : 全ファイル読み込み処理                                 */
/*****************************************************************************/
short CKYX_all_file_read(gflin_pkey_def *connection_lid)
{
    short   ls_result = CKYX_RET_OK;              /* WK処理結果              */

    /* NW情報ファイル(グループ単位)読込処理 */
    ls_result = CKYX_read_nwfile(CKYX_UNIT_GROUP, 
                                 connection_lid,
                                 &g_nwi_g);

    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* NW情報ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* NW情報ファイル(インタフェース単位)読込処理 */
    ls_result = CKYX_read_nwfile(CKYX_UNIT_INTERFACE, 
                                 connection_lid,
                                 &g_nwi_i);

    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* NW情報ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 接続先固有情報ファイル(NW単位)読込処理 */
    memset((char *)&g_nws_n, CKYX_ZERO, sizeof(g_nws_n));
    ls_result = CKYX_read_nwsfile(CKYX_UNIT_NW, 
                                   connection_lid,
                                   &g_nws_n);
    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* 接続先固有情報ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 接続先固有情報ファイル(インタフェース単位)読込処理 */
    memset((char *)&g_nws_i, CKYX_ZERO, sizeof(g_nws_i));
    ls_result = CKYX_read_nwsfile(CKYX_UNIT_INTERFACE, 
                                   connection_lid,
                                   &g_nws_i);
    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* 接続先固有情報ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 接続先固有情報ファイル(ステーション単位)読込処理 */
    memset((char *)&g_nws_s, CKYX_ZERO, sizeof(g_nws_s));
    ls_result = CKYX_read_nwsfile(CKYX_UNIT_STATION, 
                                   connection_lid,
                                   &g_nws_s);
    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* 接続先固有情報ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    /* 接続先固有情報ファイル(コネクション単位)は読み込み不要 */
    memset((char *)&g_nws_c, CKYX_ZERO, sizeof(g_nws_c));

    /* インタフェース単位 */
    /* 鍵管理ファイル読込処理 */
    ls_result = CKYX_key_read(g_key_kind,
                              DEF_COM_IOM_NOLOCK,
                              CKYX_OWN_FILE_OWN_SITE,
                              connection_lid,
                              &g_gckey);

    /* 読み込み結果判定 */
    if ( ls_result != CKYX_RET_OK ){
        /* 処理異常終了 */
        /* 鍵管理ファイル読込エラー */
        /* 内部エラーコード設定 */
        memcpy(g_internal_error_code,
               DEF_NERR_FILE_IO_ERR,
               sizeof(g_internal_error_code));

        /* 応答種別にエラー応答を設定 */
        g_resp_kind = CKYX_RSP_KIND_ERR;
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* CKYX_all_file_read */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_read_nwfile                               */
/*  CALLING SEQ.    : short CKYX_read_nwfile(short unit_kbn,                */
/*                                           gflin_pkey_def *connection_lid,*/
/*                                           db_gfnwi_def *set_db)          */
/*  ARGUMENT        : 1.unit_kbn       (I)DB種別                            */
/*                  : 2.connection_lid (I)センターID                        */
/*                  : 3.set_db         (O)DB設定領域                        */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : N/W情報ファイル読み込み処理                           */
/****************************************************************************/
short CKYX_read_nwfile(short          unit_kbn,
                       gflin_pkey_def *connection_lid,
                       db_gfnwi_def   *set_db)
{
    short           s_result;
    db_gfnwi_def    *db_gfnwi;

    db_gfnwi = (db_gfnwi_def *)&g_com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GFNWI, strlen(DEF_GFNWI));
    memcpy(g_com_iom_arg_3.file_name, g_com_file_data.nw_file_name, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_NW_INFO, strlen(DEF_FL_NW_INFO));
    memcpy(g_com_iom_arg_4.file_name, g_com_file_data.nw_file_name, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_com_file_data.nw_file_no;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gfnwi->pri_key, sizeof(db_gfnwi->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len            = sizeof(db_gfnwi->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gfnwi->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfnwi_def_Size;

    /* 検索キー */
    db_gfnwi->pri_key.site_id = g_myinfo.site_id;
    db_gfnwi->pri_key.nw_id = g_myinfo.network_id;
    memcpy(db_gfnwi->pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));

    if (unit_kbn == CKYX_UNIT_GROUP) {
        memcpy(db_gfnwi->pri_key.if_id, "}}}}}", 5);
        memcpy(db_gfnwi->pri_key.station_id, "}}}}}}", 6);
    }
    if (unit_kbn == CKYX_UNIT_INTERFACE) {
        memcpy(db_gfnwi->pri_key.if_id, connection_lid->interface_name, 5);
        memcpy(db_gfnwi->pri_key.station_id, "}}}}}}", 6);
    }

    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gfnwi->pri_key, sizeof(db_gfnwi->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_NW_INFO
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    /* レコード情報取得 */
    memcpy((char *)set_db, (char *)db_gfnwi, db_gfnwi_def_Size);

    return CKYX_RET_OK;

} /* CKYX_read_nwfile */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_read_nwsfile                              */
/*  CALLING SEQ.    : short CKYX_read_nwsfile(short unit_kbn,               */
/*                                           gflin_pkey_def *connection_lid,*/
/*                                           db_gfnws_def   *set_db)        */
/*  ARGUMENT        : 1.unit_kbn       (I)DB種別                            */
/*                  : 2.connection_lid (I)センターID                        */
/*                  : 3.set_db         (O)DB設定領域                        */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 接続先固有ファイル読み込み処理                        */
/****************************************************************************/
short CKYX_read_nwsfile(short          unit_kbn,
                        gflin_pkey_def *connection_lid,
                        db_gfnws_def   *set_db)
{
    short           s_result;
    db_gfnws_def    *db_gfnws;

    db_gfnws = (db_gfnws_def *)&g_com_iom_arg_6.rec_area;

    /* 接続先固有ファイル登録有無フラグ判定 */
    if ( g_nwi_g.gfnws_info.rec_unit[unit_kbn-1] == '0'){
        /* ファイル登録なし */
        
        return CKYX_RET_OK;
    }

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GFNWS, strlen(DEF_GFNWS));
    memcpy(g_com_iom_arg_3.file_name, g_file_info.gfnws_fname, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_NW_KOYU_INFO, strlen(DEF_FL_NW_KOYU_INFO));
    memcpy(g_com_iom_arg_4.file_name, g_file_info.gfnws_fname, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_file_info.gfnws_fno;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gfnws->pri_key, sizeof(db_gfnws->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len            = sizeof(db_gfnws->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gfnws->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gfnws_def_Size;

    /* 検索キー */
    db_gfnws->pri_key.nw_id = g_myinfo.network_id;

    if (unit_kbn == CKYX_UNIT_NW) {
        memcpy(db_gfnws->pri_key.if_id, "}}}}}", 5);
        memcpy(db_gfnws->pri_key.station_id, "}}}}}}", 6);
        memcpy(db_gfnws->pri_key.connect_id, "}}}}}}", 6);
    }
    if (unit_kbn == CKYX_UNIT_INTERFACE) {
        memcpy(db_gfnws->pri_key.if_id,
               connection_lid->interface_name,
               sizeof(db_gfnws->pri_key.if_id));
        memcpy(db_gfnws->pri_key.station_id, "}}}}}}", 6);
        memcpy(db_gfnws->pri_key.connect_id, "}}}}}}", 6);
    }
    if (unit_kbn == CKYX_UNIT_STATION) {
        memcpy(db_gfnws->pri_key.if_id,
               connection_lid->interface_name,
               sizeof(db_gfnws->pri_key.if_id));
        memcpy(db_gfnws->pri_key.station_id,
               connection_lid->station_name,
               sizeof(db_gfnws->pri_key.station_id));
        memcpy(db_gfnws->pri_key.connect_id, "}}}}}}", 6);
    }
    if (unit_kbn == CKYX_UNIT_CONNECTION) {
        memcpy(db_gfnws->pri_key.if_id,
               connection_lid->interface_name,
               sizeof(db_gfnws->pri_key.if_id));
        memcpy(db_gfnws->pri_key.station_id,
               connection_lid->station_name,
               sizeof(db_gfnws->pri_key.station_id));
        memcpy(db_gfnws->pri_key.connect_id,
               connection_lid->connection_name,
               sizeof(db_gfnws->pri_key.connect_id));
        memcpy(db_gfnws->pri_key.connect_id,
               connection_lid->connection_name,
               sizeof(db_gfnws->pri_key.connect_id));
    }
    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gfnws->pri_key, sizeof(db_gfnws->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_NW_KOYU_INFO
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    /* レコード情報取得 */
    memcpy((char *)set_db, (char *)db_gfnws, db_gfnws_def_Size);

    return CKYX_RET_OK;

} /* CKYX_read_nwsfile */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_read_station_st                           */
/*  CALLING SEQ.    : short CKYX_read_station_st(                           */
/*                                   gflin_pkey_def *connection_lid,        */
/*                                   char *station_sts)                     */
/*  ARGUMENT        : 1.connection_lid      (I)センターID                   */
/*                  : 2.station_sts         (O)局状態                       */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 局状態取得処理                                        */
/****************************************************************************/
short CKYX_read_station_st(gflin_pkey_def *connection_lid, char *station_sts)
{
    short           s_result;
    db_gcsst_def    *db_gcsst;

    db_gcsst = (db_gcsst_def *)&g_com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GCSST, strlen(DEF_GCSST));
    memcpy(g_com_iom_arg_3.file_name, g_file_info.gfnwi_fname, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_CEN_STS, strlen(DEF_FL_CEN_STS));
    memcpy(g_com_iom_arg_4.file_name, g_file_info.gcsst_fname, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_file_info.gcsst_fno;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gcsst->pri_key, sizeof(db_gcsst->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len           = sizeof(db_gcsst->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gcsst->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gcsst_def_Size;

    /* 検索キー */
    memset(&db_gcsst->pri_key, CKYX_SPACE, sizeof(db_gcsst->pri_key));

    db_gcsst->pri_key.site_id = connection_lid->site_name;
    db_gcsst->pri_key.nw_id = connection_lid->nw_name;
    memcpy(db_gcsst->pri_key.grp_id, connection_lid->group_name, sizeof(connection_lid->group_name));
    memcpy(db_gcsst->pri_key.if_id, connection_lid->interface_name, sizeof(connection_lid->interface_name));

    /* 局状態管理単位判定 */
    if (g_nwi_g.mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_ST){
        memcpy(db_gcsst->pri_key.station_id, connection_lid->station_name, sizeof(connection_lid->station_name));
    }
    else if (g_nwi_g.mng_lyr_info.open_close_mng_lyr == DEF_OPN_CLS_MNG_LYR_CO){
        memcpy(db_gcsst->pri_key.station_id, connection_lid->station_name, sizeof(connection_lid->station_name));
        memcpy(db_gcsst->pri_key.connect_id, connection_lid->connection_name, sizeof(connection_lid->connection_name));
    }

    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gcsst->pri_key, sizeof(db_gcsst->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@2"
                            , ""
                            , ""
                            , DEF_FL_CEN_STS
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    /* 局状態設定 */
    memcpy( station_sts,
            db_gcsst->state_sts_info.state_sts,
            sizeof(db_gcsst->state_sts_info.state_sts));

    return CKYX_RET_OK;

} /* CKYX_read_station_st */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_key_read                                  */
/*  CALLING SEQ.    : short CKYX_key_read(char  *key_kind,                  */
/*                                        char  node_type,                  */
/*                                        gflin_pkey_def *connection_lid,   */
/*                                        db_gckey_def   *set_db)           */
/*  ARGUMENT        : 1.key_kind       (I)キー種別                          */
/*                  : 2.node_type      (I)ノード種別                        */
/*                  : 3.connection_lid (I)センターID                        */
/*                  : 4.set_db         (O)DB設定領域                        */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 鍵管理ファイル読み込み処理                            */
/****************************************************************************/
short CKYX_key_read(char           *key_kind,
                    char           readlock,
                    char           node_type,
                    gflin_pkey_def *connection_lid,
                    db_gckey_def   *set_db)
{
    short           s_result;
    db_gckey_def    *db_gckey;
    char            key_kind_set[5];

    db_gckey = (db_gckey_def *)&g_com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GCKEY, strlen(DEF_GCKEY));
    memcpy(g_com_iom_arg_3.file_name, g_file_info.gfnwi_fname, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_KEY_MG, strlen(DEF_FL_KEY_MG));

    /* 自ノードファイル */
    memcpy(g_com_iom_arg_4.file_name, g_file_info.own_gckey_fname, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_file_info.own_gckey_fno;

    memcpy(key_kind_set, key_kind, 4);
    key_kind_set[4] = 0;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len           = sizeof(db_gckey->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gckey->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = readlock;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gckey_def_Size;

    /* 検索キー */
    if (node_type == CKYX_OWN_FILE_OWN_SITE) {
        /* 自サイト */
        db_gckey->pri_key.site_id = g_myinfo.site_id;
    }
    else{
        /* 他サイト */
        if (g_myinfo.site_id == DEF_SITE_ID_TKY){
            db_gckey->pri_key.site_id = DEF_SITE_ID_OSK;
        }
        else{
            db_gckey->pri_key.site_id = DEF_SITE_ID_TKY;
        }
    }
    db_gckey->pri_key.nw_id = g_myinfo.network_id;
    memcpy(db_gckey->pri_key.grp_id, g_myinfo.group_id, sizeof(g_myinfo.group_id));

    if (g_nwi_g.mng_lyr_info.key_cng_mng_lyr == DEF_KEY_CHANGE_MNG_LYR_IF){
       /* インタフェース単位 */
        memcpy(db_gckey->pri_key.if_id,
               connection_lid->interface_name,
               sizeof(db_gckey->pri_key.if_id));
        memset(db_gckey->pri_key.station_id, CKYX_SPACE, 6);
    }
    else if (g_nwi_g.mng_lyr_info.key_cng_mng_lyr == DEF_KEY_CHANGE_MNG_LYR_ST){
       /* ステーション単位 */
        memcpy(db_gckey->pri_key.if_id,
               connection_lid->interface_name,
               sizeof(db_gckey->pri_key.if_id));
        memcpy(db_gckey->pri_key.station_id,
               connection_lid->station_name,
               sizeof(db_gckey->pri_key.station_id));
    }
    else {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    memcpy(db_gckey->pri_key.key_type, key_kind, sizeof(db_gckey->pri_key.key_type));

    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    /* レコード情報取得 */
    memcpy((char *)set_db, (char *)db_gckey, db_gckey_def_Size);

    return CKYX_RET_OK;

} /* CKYX_key_read */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_key_read_internal                         */
/*  CALLING SEQ.    : short CKYX_key_read_internal(                         */
/*                                      char  *key_kind,                    */
/*                                      char  node_type,                    */
/*                                      gflin_pkey_def *connection_lid,     */
/*                                      db_gckey_def   *set_db)             */
/*  ARGUMENT        : 1.key_kind       (I)キー種別                          */
/*                  : 2.node_type      (I)ノード種別                        */
/*                  : 3.connection_lid (I)センターID                        */
/*                  : 4.set_db         (O)DB設定領域                        */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 鍵管理ファイル読み込み処理(内部変換用)                */
/****************************************************************************/
short CKYX_key_read_internal(char           *key_kind,
                             char           readlock,
                             char           node_type,
                             gflin_pkey_def *connection_lid,
                             db_gckey_def   *set_db)
{
    short           s_result;
    db_gckey_def    *db_gckey;
    char            key_kind_set[5];

    db_gckey = (db_gckey_def *)&g_com_iom_arg_6.rec_area;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GCKEY, strlen(DEF_GCKEY));
    memcpy(g_com_iom_arg_3.file_name, g_file_info.gfnwi_fname, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_READ, strlen(DEF_FILEIO_READ));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_KEY_MG, strlen(DEF_FL_KEY_MG));

    /* 自ノードファイル */
    memcpy(g_com_iom_arg_4.file_name, g_file_info.own_gckey_fname, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_file_info.own_gckey_fno;

    memcpy(key_kind_set, key_kind, 4);
    key_kind_set[4] = 0;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memset((char *)&db_gckey->pri_key , ' ', sizeof(db_gckey->pri_key));
    memcpy(db_gckey->pri_key.key_type , CKYX_KEY_DB_CONV, sizeof(db_gckey->pri_key.key_type));
    memcpy(g_com_iom_arg_5.key_value  , (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len           = sizeof(db_gckey->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(db_gckey->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = readlock;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gckey_def_Size;

    /* 検索キー */
    if (node_type == CKYX_OWN_FILE_OWN_SITE) {
        /* 自サイト */
        db_gckey->pri_key.site_id = g_myinfo.site_id;
    }
    else{
        /* 他サイト */
        if (g_myinfo.site_id == DEF_SITE_ID_TKY){
            db_gckey->pri_key.site_id = DEF_SITE_ID_OSK;
        }
        else{
            db_gckey->pri_key.site_id = DEF_SITE_ID_TKY;
        }
    }
    db_gckey->pri_key.nw_id = g_myinfo.network_id;

    memcpy(g_com_iom_arg_5.key_value, (char *)&db_gckey->pri_key, sizeof(db_gckey->pri_key));

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if ((s_result != DEF_RET_OK) ||
        (memcmp( g_ch_sub_prog_sts , DEF_COM_IOM_EOF_ERR , strlen( DEF_COM_IOM_EOF_ERR ) ) == 0 )) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    /* レコード情報取得 */
    memcpy((char *)set_db, (char *)db_gckey, db_gckey_def_Size);

    return CKYX_RET_OK;

} /* CKYX_key_read_internal */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CKYX_key_write                                 */
/*  CALLING SEQ.    : short CKYX_key_write(short unit_kbn,                  */
/*                                         char  node_type,                 */
/*                                         db_gckey_def   *set_db)          */
/*  ARGUMENT        : 2.node_type      (I)ノード種別                        */
/*                  : 4.set_db         (O)DB設定領域                        */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 鍵管理ファイル書き込み処理                           */
/****************************************************************************/
short CKYX_key_write(char           node_type,
                    db_gckey_def   *set_db)
{
    short           s_result;

    /* 共通I/Oモジュール情報初期化 */
    memset(g_ch_sub_prog_sts, 0x00, sizeof(g_ch_sub_prog_sts));
    memset(&g_com_iom_arg_3 , 0x00, sizeof(g_com_iom_arg_3));
    memset(&g_com_iom_arg_4 , 0x00, sizeof(g_com_iom_arg_4));
    memset(&g_com_iom_arg_5 , 0x00, sizeof(g_com_iom_arg_5));
    memset(&g_com_iom_arg_6 , 0x00, sizeof(g_com_iom_arg_6));

    /* トレース情報 */
    memcpy(g_com_iom_arg_3.prog_id, g_myinfo.prog_id, sizeof(g_myinfo.prog_id));
    memcpy(g_com_iom_arg_3.file_id, DEF_GCKEY, strlen(DEF_GCKEY));
    memcpy(g_com_iom_arg_3.file_name, g_file_info.own_gckey_fname, sizeof(g_com_iom_arg_3.file_name));
    memcpy(g_com_iom_arg_3.file_io_type, DEF_FILEIO_UPDATE, strlen(DEF_FILEIO_UPDATE));

    /* ファイル情報 */
    memcpy(g_com_iom_arg_4.file_id, DEF_FL_KEY_MG, strlen(DEF_FL_KEY_MG));

    /* 自ノードファイル */
    memcpy(g_com_iom_arg_4.file_name, g_file_info.own_gckey_fname, sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no = g_file_info.own_gckey_fno;

    /* 入力情報 */
    g_com_iom_arg_5.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position = 0;
    g_com_iom_arg_5.part_key_len      = 0;
    memcpy(g_com_iom_arg_5.key_value  , (char *)&set_db->pri_key, sizeof(set_db->pri_key));
    memcpy(g_com_iom_arg_5.key_type   , DEF_COM_IOM_KEYTYPE_PRI, strlen(DEF_COM_IOM_KEYTYPE_PRI));
    g_com_iom_arg_5.key_len           = sizeof(set_db->pri_key);
    g_com_iom_arg_5.compare_len       = sizeof(set_db->pri_key);
    g_com_iom_arg_5.positioning_mode  = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg          = DEF_COM_IOM_LOCKFREE;
    g_com_iom_arg_5.asc_desc_type     = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer          = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len           = db_gckey_def_Size;
    memcpy(g_com_iom_arg_5.rec_area, (char*)set_db, db_gckey_def_Size);

    /* 共通I/Oモジュール */
    s_result = COM_IOM ( DEF_COM_IOM_FUNC_UPDATE
                       , g_ch_sub_prog_sts
                       , &g_com_iom_arg_3
                       , &g_com_iom_arg_4
                       , &g_com_iom_arg_5
                       , &g_com_iom_arg_6);

    if (s_result != DEF_RET_OK) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@4"
                            , ""
                            , ""
                            , DEF_FL_KEY_MG
                            , DEF_COM_IOM_FUNC_UPDATE
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode);
        return CKYX_RET_NG;
    }

    return CKYX_RET_OK;

} /* CKYX_key_write */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CKYX_char2hex                                  */
/*  CALLING SEQ.    : short CKYX_char2hex         (const char     *         */
/*                                                 ,char           *        */
/*                                                 ,short           )       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.hex_p          (O)   変換先バッファ                 */
/*                  : 3.s_len          (I)   変換長                         */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 16進文字->BINARY変換を行う                            */
/****************************************************************************/
short CKYX_char2hex(char *ascii_p, char *hex_p, short s_len)
{
    short var,base,half,iix,oix;

    base = (short)(s_len & 0x01);
    for (iix = 0,oix = 0,var = 0;iix < s_len;iix++) {

        if        (ascii_p[iix] >= '0' && ascii_p[iix] <= '9') {
           half = ascii_p[iix] - '0';
        } else if (ascii_p[iix] >= 'A' && ascii_p[iix] <= 'F') {
           half = ascii_p[iix] - 'A' + 10;
        } else if (ascii_p[iix] >= 'a' && ascii_p[iix] <= 'f') {
           half = ascii_p[iix] - 'a' + 10;
        } else {
          memset(hex_p,0x20,s_len/2);
          return (1);
        }
        if (base == 0) {
            var = half;
            base = 1;
        } else {
            var = (short)((var << 4) + half);
            hex_p[oix] = (char)var;
            oix++;
            base = 0;
        }
    }
    return (0);

} /* end of CKYX_char2hex */
