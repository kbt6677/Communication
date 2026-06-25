/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVXZ0                                    */
/*        FUNCTION          ････ 制御電文共通メイン処理                      */
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
#include "zspic"  nolist
#include "zfilc"  nolist
#include "zsysc"  nolist

#include "file.h"

#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVXZ1.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  main                                            */
/*  CALLING SEQ.    : void main (void)                                       */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : メイン                                                 */
/*****************************************************************************/
int main (void)
{
    CMIN_init();                  /* 初期処理   */

    CMIN_main();                  /* 主処理     */

    CMIN_finish();                /* 終了処理   */

} /* end of main */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_init                                       */
/*  CALLING SEQ.    : void CMIN_init (void)                                  */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 初期処理                                               */
/*****************************************************************************/
void CMIN_init()
{

    short   ls_error;                             /* WKエラーコード          */
    short   ls_result;                            /* WK処理結果              */

/* --------------------------------------- */
/* グローバル情報の初期化(自プロセス情報)  */
/* --------------------------------------- */
    g_myinfo.site_id          = DEF_BUF_NULL;
    g_myinfo.network_id       = DEF_BUF_NULL;
    memset ( g_myinfo.nw_kbn                , DEF_BUF_NULL , sizeof(g_myinfo.nw_kbn              ));
    memset ( g_myinfo.group_id              , DEF_BUF_NULL , sizeof(g_myinfo.group_id            ));
    memset ( g_myinfo.prog_id               , DEF_BUF_NULL , sizeof(g_myinfo.prog_id             ));
    memset ( g_myinfo.serverclass_name      , DEF_BUF_NULL , sizeof(g_myinfo.serverclass_name    ));
    memset ( g_myinfo.serverclass_no        , DEF_BUF_NULL , sizeof(g_myinfo.serverclass_no      ));
    g_myinfo.io_timer         = DEF_TIME_INI;
    memset ( g_myinfo.ems_serverclass_name  , DEF_BUF_NULL , sizeof(g_myinfo.ems_serverclass_name));
    memset ( g_myinfo.ems_pathmon           , DEF_BUF_NULL , sizeof(g_myinfo.ems_pathmon         ));
    g_myinfo.proc_io_timer    = DEF_TIME_INI;
    g_myinfo.send_timer       = DEF_TIME_INI;
    g_myinfo.send_retry_count = DEF_LCNT_INI;
    memset ( g_myinfo.my_prcname            , DEF_BUF_NULL , sizeof(g_myinfo.my_prcname          ));
    memset ( g_myinfo.my_prcno              , DEF_BUF_NULL , sizeof(g_myinfo.my_prcno            ));
    g_myinfo.recv_fno          = DEF_FNO_UNKOWN ;
    memset (&g_recv_buf                     , DEF_BUF_SPACE, sizeof(g_recv_buf                   ));
    g_myinfo.recv_buf          = g_recv_buf;
    memset ( g_myinfo.pathmon_name          , DEF_BUF_SPACE, sizeof(g_myinfo.pathmon_name        ));
    g_myinfo.end_flg           = DEF_FLAG_OFF;
    g_myinfo.data_len          = DEF_BUF_NULL;
    g_myinfo.data_len_del_flag = DEF_FLAG_OFF;

    /* グローバル情報の初期化(プロセス情報) */
    memset(&g_procinfo, DEF_BUF_SPACE, sizeof(g_procinfo));

    /* グローバル情報の初期化(PATHSEND情報) */
    g_sendinfo.psend_data     = g_send_buf;
    g_sendinfo.psend_send_len = DEF_BUF_NULL;
    g_sendinfo.psend_resp_len = DEF_BUF_NULL;
    g_sendinfo.psend_err      = DEF_BUF_NULL;
    g_sendinfo.gerr           = DEF_BUF_NULL;

    /* EMS初期設定 */
    memset( (char *)&g_cg010in_modle, DEF_BUF_SPACE, sizeof(g_cg010in_modle));
    g_cg010in_modle.subrcd         = DEF_BUF_CZERO;
    g_cg010in_modle.emsinf.rcd     = DEF_BUF_CZERO;
    memcpy( g_cg010in_modle.uytrminf.proctimer  , "0002", 4); /* temp */
    memcpy( g_cg010in_modle.uytrminf.uytrmmonlen, "00"  , 2); /* temp */
    memcpy( g_cg010in_modle.uytrminf.uytrmsrvlen, "00"  , 2); /* temp */
//    memset( (char *)&g_cg010in_modle      , DEF_BUF_SPACE, sizeof(g_cg010in_modle));

/* --------------------------------------- */
/* 個別 プログラムIDセット                 */
/* --------------------------------------- */
    CMIN_kbt_set_prgid();
    memcpy( g_cg010in_modle.emsinf.emsgkinf.prgid, g_myinfo.prog_id
                                           , strlen(g_myinfo.prog_id));
/* --------------------------------------- */
/* プロセス情報取得処理                    */
/* --------------------------------------- */
    ls_result = COM_PRC(&g_procinfo);

    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR             /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C"
                            , "COM_PRC" );
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* オープナプロセス管理モジュール初期処理  */
/* --------------------------------------- */
    COM_STP_INIT(&g_openersinfo);

/* --------------------------------------- */
/* トレース出力初期処理                    */
/* --------------------------------------- */
    gp_trc_st = (lk_zac2001r_arg_1_def *)&g_trc_buf;
    memset( g_trc_buf, DEF_BUF_SPACE, sizeof(g_trc_buf));
    gp_trc_st->func_flg = DEF_TRC_FUNC_INIT;
    memcpy( gp_trc_st->trace_info.prog_id, g_myinfo.prog_id, sizeof(gp_trc_st->trace_info.prog_id));
    memset( gp_trc_st->data_info.rec_len , DEF_BUF_CZERO   , sizeof(gp_trc_st->data_info.rec_len ));
    TRACEOUT((char *)g_trc_buf);

/* --------------------------------------- */
/* パラメータ取得処理                      */
/* --------------------------------------- */
    ls_result = CMIN_get_params();

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* 共通 物理名情報ファイル取得処理         */
/* --------------------------------------- */
    ls_result = CMIN_com_get_physical_names();

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* 個別 物理名情報ファイル取得処理         */
/* --------------------------------------- */
    ls_result = CMIN_kbt_get_physical_names();

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* 共通ファイルオープン                    */
/* --------------------------------------- */
    /* NW情報ファイルオープン */
    ls_result = CMIN_file_open( DEF_FL_NW_INFO
                              , g_com_file_data.nw_file_name
                              ,&g_com_file_data.nw_file_no);

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* 個別ファイルオープン                    */
/* --------------------------------------- */
    ls_result = CMIN_kbt_file_open();

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* N/Wグループ情報取得処理                 */
/* --------------------------------------- */
    ls_result = CMIN_get_group_info();

    if ( ls_result != DEF_RET_OK ) {
        g_myinfo.end_flg = DEF_FLAG_ON;   /* 終了フラグON */
        return;
    }

/* --------------------------------------- */
/* エラー出力ログ編集出力コントロール処理  */
/* --------------------------------------- */
    /* パラメータ初期化 */
    memset( &g_com_erl_arg_1, DEF_BUF_NULL , sizeof(g_com_erl_arg_1) );
    memset( &g_com_erl_arg_2, DEF_BUF_SPACE, sizeof(g_com_erl_arg_2) );
    memset( &g_ems_add      , DEF_BUF_SPACE, sizeof(g_ems_add      ) );
    /* エラーログ用情報設定 */
    g_com_erl_arg_1.file_io_type = DEF_ERL_IOTYPE_OPEN;
    g_com_erl_arg_1.io_timer     = g_myinfo.send_timer;
    g_com_erl_arg_1.data_len     = DEF_BUF_NULL;
    g_com_erl_arg_2.file_no      = DEF_FNO_UNKOWN;
    memset( g_com_erl_arg_2.file_name, DEF_BUF_NULL, sizeof(g_com_erl_arg_2.file_name));

    ls_result = COM_ERL( &g_com_erl_arg_1                   /* エラーログ共通処理実行 */
                       , &g_com_erl_arg_2
                       , &g_cg010in_modle
                       , &g_com_erl_arg_3
                       , g_myinfo.prog_id );

    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR        /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C"
                            , "COM_ERL" );
        CMIN_abend();                                       /* 異常終了            */
    }

    /* エラー出力ログ.ファイル情報取得 */
    g_com_file_data.erlg_file_no          = g_com_erl_arg_2.file_no;

/* --------------------------------------- */
/* カット対象日付取得・更新用初期処理      */
/* --------------------------------------- */
    memset(&g_NWM_CTU_INI_arg_1, DEF_BUF_NULL, sizeof(g_NWM_CTU_INI_arg_1));
    memset(&g_NWM_CTU_INI_arg_2, DEF_BUF_NULL, sizeof(g_NWM_CTU_INI_arg_2));
    memset(&g_NWM_CTU_INI_arg_3, DEF_BUF_NULL, sizeof(g_NWM_CTU_INI_arg_3));
    memset(&g_NWM_CTU_INI_arg_6, DEF_BUF_NULL, sizeof(g_NWM_CTU_INI_arg_6));

    memcpy( g_NWM_CTU_INI_arg_1.file_id  , DEF_FL_PHSIC_INFO
                                     , sizeof(g_NWM_CTU_INI_arg_1.file_id));
    memcpy( g_NWM_CTU_INI_arg_1.file_name, g_com_file_data.phy_file_name
                                     , sizeof(g_NWM_CTU_INI_arg_1.file_name));
    g_NWM_CTU_INI_arg_1.file_no      = g_com_file_data.phy_file_no;
    g_NWM_CTU_INI_arg_1.io_timer     = g_myinfo.io_timer;

    g_NWM_CTU_INI_arg_2.io_timer     = g_myinfo.io_timer;

    g_NWM_CTU_INI_arg_3.site_id      = g_myinfo.site_id;
    g_NWM_CTU_INI_arg_3.nw_id        = g_myinfo.network_id;
    memcpy( g_NWM_CTU_INI_arg_3.grp_id, g_myinfo.group_id
                                      , sizeof(g_NWM_CTU_INI_arg_3.grp_id));

    ls_result = NWM_CTU_INIT(&g_NWM_CTU_INI_arg_1
                            ,&g_NWM_CTU_INI_arg_2
                            ,&g_NWM_CTU_INI_arg_3
                            , g_myinfo.prog_id
                            ,&g_cg010in_modle
                            ,(NWM_CTU_INI_arg_6_def *)&g_ems_add );

    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR        /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C"
                            , "NWM_CTU_INIT" );
        CMIN_abend();                                       /* 異常終了            */
    }

/* --------------------------------------- */
/* 個別初期処理                            */
/* --------------------------------------- */
/*  メッセージは内部で実行                 */
    ls_result = CMIN_kbt_init();

    if ( ls_result != DEF_RET_OK ) {
        CMIN_abend();                                       /* 異常終了            */
    }

/* --------------------------------------- */
/* 共通 物理名情報ファイルCLOSE            */
/* --------------------------------------- */
    CMIN_file_close( DEF_FL_PHSIC_INFO                      /* 物理名情報ファイルクローズ */
                   , g_com_file_data.phy_file_name
                   ,&g_com_file_data.phy_file_no );

/* --------------------------------------- */
/* $RECEIVE OPEN                           */
/* --------------------------------------- */
    ls_error = FILE_OPEN_( DEF_RECEIVE_FNAME
                         , (short)strlen(DEF_RECEIVE_FNAME)
                         ,&g_myinfo.recv_fno
                         , ZSYS_VAL_OPENACC_READWRITE
                         , ZSYS_VAL_OPENEXCL_SHARED
                         , DEF_RCV_WAITDEPTH
                         , 2   /* RECEIVE-DEPTH */ );

    if ( ls_error != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR           /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_RECEIVE_FNAME
                            , DEF_FILEIO_OPEN
                            , ""
                            , ls_error );
        CMIN_abend();                                       /* 異常終了            */
    }

    CMIN_message_output ( DEF_EVT_PROC_START                /* メッセージ出力処理  */
                        , DEF_MSGTTKB_NORMAL
                        , DEF_NERR_NOMAL
                        , "@C"
                        , g_procinfo.my_pname );

} /* end of CMIN_init */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_get_params                                */
/*  CALLING SEQ.    : short CMIN_get_params (void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
short  CMIN_get_params()
{
    char    lc_paramname    [DEF_PRMNM_MAX_LEN+1];
    char    lc_param        [DEF_PRMVL_MAX_LEN+1];
    char    lc_phy_name     [ZSYS_VAL_LEN_FILENAME+1];
    char    lc_wkbuf        [10];
    short   ls_get_len;
    short   ls_result;

    /* --------------------------------------- */
    /* 物理名情報ファイル取得(ASSIGN情報)      */
    /* --------------------------------------- */
    memset (lc_phy_name , DEF_BUF_NULL , sizeof(lc_phy_name  ));
    memset (lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname ));
    memset (lc_paramname, DEF_BUF_SPACE, sizeof(lc_paramname )-1);
    memcpy (lc_paramname, DEF_ASN_GFPHI, sizeof(DEF_ASN_GFPHI)-1);

    COM_ASN(lc_paramname, lc_phy_name, &ls_get_len );

    if ( (ls_get_len == 0) || (ls_get_len > 47) ){
        CMIN_message_output ( DEF_EVT_COMMON_MOD_ERR             /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PROC_OPN_ERR
                            , "@C"
                            , "COM_ASN" );
        return DEF_RET_NG;
    }
    /* 物理名設定 */
    memset( g_com_file_data.phy_file_name, DEF_BUF_NULL, sizeof(g_com_file_data.phy_file_name));
    memcpy( g_com_file_data.phy_file_name, lc_phy_name, sizeof(g_com_file_data.phy_file_name)-1);

    /* --------------------------------------- */
    /* サーバクラス論理IDの取得                */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_SRV_LOGICAL_ID, 14);
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result < DEF_NORMAL_END ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR    /* メッセージ出力処理       */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_SRV_LOGICAL_ID
                            , ls_result );
        CMIN_abend();                                  /* 異常終了                 */
    }

    if ( strlen(lc_param) != DEF_SVRCLS_CHK_LEN ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR    /* メッセージ出力処理       */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR_INV
                            , "@C@5"
                            , DEF_SRV_LOGICAL_ID
                            , ls_result );

        CMIN_abend();                                  /* 異常終了                 */
    }

    g_myinfo.site_id      = lc_param[0];               /* サイト識別               */
    g_myinfo.network_id   = lc_param[2];               /* N/W識別                  */

    memcpy(g_myinfo.group_id        , &lc_param[4] , sizeof(g_myinfo.group_id        ));
    memcpy(g_myinfo.serverclass_name, &lc_param[10], sizeof(g_myinfo.serverclass_name));
    memcpy(g_myinfo.serverclass_no  , &lc_param[19], sizeof(g_myinfo.serverclass_no  ));

    /* --------------------------------------- */
    /* ファイルI/Oタイマ                       */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_FILE_IO_TIMER_10MSECOND, 23);
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result        < DEF_NORMAL_END ||
         strlen(lc_param) > 8                ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_FILE_IO_TIMER_10MSECOND
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    /* 数字文字チェック */
    ls_result = CMIN_num_check(lc_param);

    if ( ls_result < DEF_NORMAL_END ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR_INV
                            , "@C@5"
                            , DEF_FILE_IO_TIMER_10MSECOND
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    g_myinfo.io_timer = atol(lc_param);

    /* --------------------------------------- */
    /* 運用監視端末出力サーバ・サーバクラス名  */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_MSG_SRV_NAME, 12);
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result < DEF_NORMAL_END ||
         strlen(lc_param) > sizeof(g_myinfo.ems_serverclass_name)-1 ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_MSG_SRV_NAME
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    memcpy (g_myinfo.ems_serverclass_name    , lc_param, strlen(lc_param));
    memcpy (g_cg010in_modle.uytrminf.uytrmsrv, lc_param, strlen(lc_param));
    memset (lc_wkbuf, DEF_BUF_NULL , sizeof(lc_wkbuf));
    sprintf(lc_wkbuf, "%02d"       , strlen(lc_param));
    memcpy (g_cg010in_modle.uytrminf.uytrmsrvlen, lc_wkbuf
                                                , sizeof(g_cg010in_modle.uytrminf.uytrmsrvlen));

    /* --------------------------------------- */
    /* 運用監視端末出力サーバのPATHMON名       */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_MSG_MON_NAME, 12);
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result < DEF_NORMAL_END ||
         strlen(lc_param) > sizeof(g_myinfo.ems_pathmon)-1 ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_MSG_MON_NAME
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    memcpy (g_myinfo.ems_pathmon             , lc_param, strlen(lc_param));
    memcpy (g_cg010in_modle.uytrminf.uytrmmon, lc_param, strlen(lc_param));
    memset (lc_wkbuf, DEF_BUF_NULL , sizeof(lc_wkbuf));
    sprintf(lc_wkbuf, "%02d"       , strlen(lc_param));
    memcpy (g_cg010in_modle.uytrminf.uytrmmonlen, lc_wkbuf, sizeof(g_cg010in_modle.uytrminf.uytrmmonlen));
    /* --------------------------------------- */
    /* PATHSENDタイマ                          */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_PSEND_TIMER_10MSECOND, 21);
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result        < DEF_NORMAL_END ||
         strlen(lc_param) > 8                ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_PSEND_TIMER_10MSECOND
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    /* 数字文字チェック */
    ls_result = CMIN_num_check(lc_param);

    if ( ls_result < DEF_NORMAL_END ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR_INV
                            , "@C@5"
                            , DEF_PSEND_TIMER_10MSECOND
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }
    g_myinfo.send_timer = atol(lc_param);

    /* --------------------------------------- */
    /* PATHSENDリトライ回数                    */
    /* --------------------------------------- */
    memset(lc_paramname, DEF_BUF_NULL , sizeof(lc_paramname));
    memset(lc_param    , DEF_BUF_NULL , sizeof(lc_param    ));
    ls_result = DEF_BUF_NULL;

    memcpy(lc_paramname, DEF_PSEND_RETRY_CNT, sizeof(DEF_PSEND_RETRY_CNT));
    ls_result = get_param_by_name (lc_paramname, lc_param, DEF_PRMVL_MAX_LEN+1);

    if ( ls_result        < DEF_NORMAL_END ||
         strlen(lc_param) > 8                ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR
                            , "@C@5"
                            , DEF_PSEND_RETRY_CNT
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    /* 数字文字チェック */
    ls_result = CMIN_num_check(lc_param);

    if ( ls_result < DEF_NORMAL_END ) {
        CMIN_message_output ( DEF_EVT_PARAM_GET_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_PRM_RD_ERR_INV
                            , "@C@5"
                            , DEF_PSEND_RETRY_CNT
                            , ls_result );
        CMIN_abend();                                            /* 異常終了            */
    }

    g_myinfo.send_retry_count = atol(lc_param);

    memcpy (g_cg010in_modle.uytrminf.proctimer, lc_param, strlen(lc_param));
    return DEF_RET_OK;

} /* end of CMIN_get_params */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_com_get_physical_names                    */
/*  CALLING SEQ.    : short CMIN_com_get_physical_names(void)               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 物理ファイル名取得処理(共通)                          */
/****************************************************************************/
short  CMIN_com_get_physical_names()
{
    short            ls_result;
    t_gfphi_pri_key  l_gfphi_pri_key;
    db_gfphi_def    *phy_tbl_local;

/* ------------------------------------------------------ */
/* 物理名情報ファイルオープン                             */
/* ------------------------------------------------------ */
    ls_result = CMIN_file_open( DEF_FL_PHSIC_INFO
                              , g_com_file_data.phy_file_name
                              ,&g_com_file_data.phy_file_no );

    if ( ls_result != DEF_RET_OK ) {              /* IOモジュール結果判定 */
        g_myinfo.end_flg = DEF_FLAG_ON;           /* 終了フラグON */
        return DEF_RET_NG;
    }
/* ------------------------------------------------------ */
/* NW情報ファイル  物理名取得                             */
/* ------------------------------------------------------ */
    l_gfphi_pri_key.site_id = g_myinfo.site_id;
    l_gfphi_pri_key.nw_id   = g_myinfo.network_id;
    memcpy( l_gfphi_pri_key.grp_id     , g_myinfo.group_id
          , sizeof(l_gfphi_pri_key.grp_id));
    memset(&l_gfphi_pri_key.srv_cls_key, DEF_BUF_NO_SET
          , sizeof(l_gfphi_pri_key.srv_cls_key));
    memcpy( l_gfphi_pri_key.prc_file_key.prc_file_id.prc_file_kind
          , DEF_FL_NW_INFO
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
    memcpy( g_com_file_data.nw_file_name
          , phy_tbl_local->prc_file_info.prc_file_name
          , sizeof(g_com_file_data.nw_file_name)-1);

    return DEF_RET_OK;
} /* end of CMIN_com_get_physical_names */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_get_phy_name                              */
/*  CALLING SEQ.    : short CMIN_get_phy_name (char *)                      */
/*  ARGUMENT        : char *p_key_value                                     */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 物理名情報ファイル読み込み処理                        */
/****************************************************************************/
short CMIN_get_phy_name (char *p_key_value)
{
    short         ls_result;

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    /* 物理名情報ファイル読込 */
    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id
                                        , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_id     , DEF_FL_PHSIC_INFO
                                        , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , g_com_file_data.phy_file_name
                                        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_START
                                        , sizeof(g_com_iom_arg_3.file_io_type));

    memcpy( g_com_iom_arg_4.file_id     , DEF_GFPHI
                                        , strlen(DEF_GFPHI));
    memcpy( g_com_iom_arg_4.file_name   , g_com_file_data.phy_file_name
                                        , sizeof(g_com_iom_arg_4.file_name));
    g_com_iom_arg_4.file_no             = g_com_file_data.phy_file_no;

    g_com_iom_arg_5.part_key_type       = DEF_COM_IOM_PARTITION_KEY_NOT;
    g_com_iom_arg_5.part_key_position   = DEF_BUF_NULL;
    g_com_iom_arg_5.part_key_len        = DEF_BUF_NULL;
    memcpy( g_com_iom_arg_5.key_value   , p_key_value, strlen(p_key_value));
    memcpy( g_com_iom_arg_5.key_type    , DEF_COM_IOM_KEYTYPE_PRI
                                        , sizeof(g_com_iom_arg_5.key_type));
    g_com_iom_arg_5.key_len             = sizeof(t_gfphi_pri_key);
    g_com_iom_arg_5.positioning_mode    = DEF_COM_IOM_EXACT;
    g_com_iom_arg_5.lock_flg            = DEF_COM_IOM_NOLOCK;
    g_com_iom_arg_5.asc_desc_type       = DEF_COM_IOM_ASCEND;
    g_com_iom_arg_5.io_timer            = g_myinfo.io_timer;
    g_com_iom_arg_5.rec_len             = sizeof(db_gfphi_def);

    /* IOモジュール */
    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_STARTREAD
                        , g_ch_sub_prog_sts
                        , &g_com_iom_arg_3
                        , &g_com_iom_arg_4
                        , &g_com_iom_arg_5
                        , &g_com_iom_arg_6);

    /* IOモジュール結果判定 */
    if ( ls_result != DEF_RET_OK ) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_FL_PHSIC_INFO
                            , DEF_COM_IOM_FUNC_STARTREAD
                            , g_com_iom_arg_5.key_value
                            , g_com_iom_arg_6.guardian_errcode );
        return DEF_RET_NG;
    }

    return DEF_RET_OK;
} /* end of CMIN_get_phy_name */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_file_open                                 */
/*  CALLING SEQ.    : short  CMIN_file_open (short)                         */
/*  ARGUMENT        : short *p_ope_file_name                                */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : ファイルオープン処理(共通)                            */
/****************************************************************************/
short CMIN_file_open ( char  *p_file_id
                     , char  *p_file_name
                     , short *p_file_no )
{
    short         ls_result;

    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id   , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_OPEN    , sizeof(g_com_iom_arg_3.file_io_type));
    memcpy( g_com_iom_arg_3.file_id     , p_file_id          , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , p_file_name        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_4.file_id     , &p_file_id[3]      , sizeof(g_com_iom_arg_4.file_id     )-3);
    memcpy( g_com_iom_arg_4.file_name   , p_file_name        , sizeof(g_com_iom_arg_4.file_name   ));
    g_com_iom_arg_4.file_no             = g_com_file_data.phy_file_no;

    ls_result = COM_IOM ( DEF_COM_IOM_FUNC_OPEN        /* IOモジュール         */
                        , g_ch_sub_prog_sts
                        , &g_com_iom_arg_3
                        , &g_com_iom_arg_4
                        , &g_com_iom_arg_5
                        , &g_com_iom_arg_6);

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR      /* メッセージ出力処理   */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            ,&p_file_id
                            , DEF_FILEIO_OPEN
                            , ""
                            , g_com_iom_arg_6.guardian_errcode );
        return DEF_RET_NG;
    }

    *p_file_no = g_com_iom_arg_4.file_no;

    return DEF_RET_OK;
} /* end of CMIN_file_open */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_file_close                                */
/*  CALLING SEQ.    : void   CMIN_file_close (short)                        */
/*  ARGUMENT        : short  p_file_no                                      */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ファイルクローズ処理(共通)                            */
/****************************************************************************/
void CMIN_file_close ( char  *p_file_id
                     , char  *p_file_name
                     , short *p_file_no )
{
    /* IOモジュールパラメータ初期化 */
    memset( &g_com_iom_arg_3 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_3  ));
    memset( &g_com_iom_arg_4 , DEF_BUF_SPACE, sizeof(g_com_iom_arg_4  ));
    memset( &g_com_iom_arg_5 , DEF_BUF_NULL, sizeof(g_com_iom_arg_5  ));
    memset( &g_com_iom_arg_6 , DEF_BUF_NULL, sizeof(g_com_iom_arg_6  ));
    memset( g_ch_sub_prog_sts, DEF_BUF_NULL, sizeof(g_ch_sub_prog_sts));

    memcpy( g_com_iom_arg_3.prog_id     , g_myinfo.prog_id   , sizeof(g_com_iom_arg_3.prog_id     ));
    memcpy( g_com_iom_arg_3.file_io_type, DEF_FILEIO_CLOSE   , sizeof(g_com_iom_arg_3.file_io_type));
    memcpy( g_com_iom_arg_3.file_id     , p_file_id          , sizeof(g_com_iom_arg_3.file_id     ));
    memcpy( g_com_iom_arg_3.file_name   , p_file_name        , sizeof(g_com_iom_arg_3.file_name   ));
    memcpy( g_com_iom_arg_4.file_id     , &p_file_id[3]      , sizeof(g_com_iom_arg_4.file_id     )-3);
    memcpy( g_com_iom_arg_4.file_name   , p_file_name        , sizeof(g_com_iom_arg_4.file_name   ));
    g_com_iom_arg_4.file_no             = *p_file_no;

    COM_IOM ( DEF_COM_IOM_FUNC_CLOSE /* IOモジュール         */
            , g_ch_sub_prog_sts
            , &g_com_iom_arg_3
            , &g_com_iom_arg_4
            , &g_com_iom_arg_5
            , &g_com_iom_arg_6);

    *p_file_no = DEF_FNO_UNKOWN;

} /* end of CMIN_file_close */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_get_group_info                            */
/*  CALLING SEQ.    : short CMIN_get_group_info (void)                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : NW情報グループ単位取得処理                            */
/****************************************************************************/
short  CMIN_get_group_info()
{
    short                ls_result;
    t_gfnwi_pri_key_def  l_gfnwi_pkey;       /* NW情報ファイルプライマリKey */
/* ------------------------------------------------------ */
/* N/Wグループ情報取得                                    */
/* ------------------------------------------------------ */
    memset(&g_gfnwi_tbl[DEF_FNWI_IDX_SITE], DEF_BUF_SPACE, sizeof(db_gfnwi_def));
    memset(&l_gfnwi_pkey                  , DEF_BUF_SPACE, sizeof(l_gfnwi_pkey));
    /* プライマリKey設定 */
    l_gfnwi_pkey.site_id = g_myinfo.site_id;
    l_gfnwi_pkey.nw_id   = g_myinfo.network_id;
    memcpy( l_gfnwi_pkey.grp_id    , g_myinfo.group_id  , sizeof(l_gfnwi_pkey.grp_id    ));
    memset( l_gfnwi_pkey.if_id     , DEF_BUF_NO_SET     , sizeof(l_gfnwi_pkey.if_id     ));
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

    if ( ls_result != DEF_RET_OK ) {                   /* IOモジュール結果判定 */
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
        return DEF_RET_NG;
    }

    memcpy( (char *)&g_gfnwi_tbl[DEF_FNWI_IDX_SITE]
          , g_com_iom_arg_6.rec_area
          , sizeof(db_gfnwi_def));
    memcpy(&g_myinfo.nw_kbn
          , g_gfnwi_tbl[DEF_FNWI_IDX_SITE].nw_id_info.nw_kubun
          , sizeof(g_gfnwi_tbl[DEF_FNWI_IDX_SITE].nw_id_info.nw_kubun));

    return DEF_RET_OK;
} /* end of CMIN_get_group_info */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_main                                      */
/*  CALLING SEQ.    : void CMIN_main (void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/*  FUNCTION        : 1.1.0  CMIN_main                                      */
/*****************************************************************************/
void CMIN_main(void)
{
    short  ls_error;
    short  ls_result;

    /* フラグがONになるまでループ  */
    while ( g_myinfo.end_flg == DEF_FLAG_OFF ) {
        memset((char *)&g_iocomp.recv_info, DEF_BUF_NULL, sizeof(g_iocomp.recv_info));
        memset( g_recv_buf                , DEF_BUF_NULL, DEF_BUF_LENGTH            );

        CMIN_read_recv();                         /* $RECEIVE READUPDATE処理  */

        if ( g_myinfo.recv_fno == DEF_RECVE_NO ) {
            /* $RECEIVE IO */
            ls_error = FILE_GETRECEIVEINFO_((short *)&g_iocomp.recv_info);

            if ( ls_error != ZFIL_ERR_OK ) {
                CMIN_message_output ( DEF_EVT_PROCEDURE_ERR /* メッセージ出力処理  */
                                    , DEF_MSGTTKB_GYOM_ERR
                                    , DEF_NERR_FILE_IO_ERR
                                    , "@C@5"
                                    , DEF_FILEIO_GETRINFO
                                    , ls_error );
                CMIN_abend();                               /* 異常終了            */
            }

            PROCESSHANDLE_DECOMPOSE_( (short *)&g_iocomp.recv_info.z_sender
                              ,,,,,,, g_iocomp.proc_name
                                    , ZSYS_VAL_LEN_PROCESSNAME
                                    ,&g_iocomp.proc_name_len);

            g_iocomp.proc_name[g_iocomp.proc_name_len] = DEF_BUF_NULL;
            memset( g_resp_buf, DEF_BUF_NULL, DEF_BUF_LENGTH );
            memcpy( g_resp_buf, g_recv_buf  , g_recv_len     );

            /* 受信メッセージ振分け */
            switch ( g_iocomp.ferror ) {
                case ZFIL_ERR_OK:
                    /* $RECEIVE ->電文受信処理 */
                    CMIN_handle_req_msg();
// 内部の処理で実施(サーバによってはreply後の処理も発生するため)
//                    CMIN_send_reply(g_resp_buf, 0, 0);
                    break;
                case ZFIL_ERR_SYSMESS:
                    /* SYSTEMメッセージ ->SYSTEMメッセージ受信処理 */
                    ls_result = COM_STP_JUDGE(&g_openersinfo, gp_sys_msg);

                    if ( ls_result == DEF_RET_NORMAL_END ) { /* 正常終了 */
                        g_myinfo.end_flg = DEF_FLAG_ON;
                    } else if ( ls_result != DEF_RET_OK ) {
                        CMIN_message_output( DEF_EVT_COMMON_MOD_ERR
                                           , DEF_MSGTTKB_NORMAL
                                           , DEF_NERR_PROC_OPN_ERR
                                           , "@C@5"
                                           , g_procinfo.my_pname
                                           , ls_result );
                        CMIN_abend();                        /* 異常終了 */
                    }

                    CMIN_send_reply(g_resp_buf, 0, 0);       /* リプライ */
                    break;
                default:
                    CMIN_message_output( DEF_EVT_COMMON_MOD_ERR
                                       , DEF_MSGTTKB_NORMAL
                                       , DEF_NERR_PROC_OPN_ERR
                                       , "@C@5"
                                       , g_procinfo.my_pname
                                       , ls_result );
                    CMIN_abend();                           /* 異常終了 */
                    break;
            }
        } else {
            CMIN_message_output( DEF_EVT_COMMON_MOD_ERR
                               , DEF_MSGTTKB_NORMAL
                               , DEF_NERR_PROC_OPN_ERR
                               , "@C@5"
                               , g_procinfo.my_pname
                               , ls_result );
            CMIN_abend();                        /* 異常終了 */
        }
    }
} /* end of CMIN_main */
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_read_recv                                  */
/*  CALLING SEQ.    : void CMIN_read_recv (void)                             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : $RECEIVE処理                                           */
/*****************************************************************************/
void CMIN_read_recv(void)
{

    /* $RECEIVEのREADUPDATE */
    g_recv_len = 0;
    READUPDATEX ( g_myinfo.recv_fno
                , g_recv_buf
                , (unsigned short)sizeof(g_recv_buf)
                ,&g_recv_len);
    FILE_GETINFO_(g_myinfo.recv_fno, &g_iocomp.ferror);

    if ( g_iocomp.ferror != ZFIL_ERR_OK &&
         g_iocomp.ferror != ZFIL_ERR_SYSMESS ) {
        CMIN_message_output ( DEF_EVT_FILE_IO_ERR
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@C@C@C@C@U"
                            , ""
                            , ""
                            , DEF_RECEIVE_FNAME
                            , DEF_FILEIO_RUPDX
                            , ""
                            , g_iocomp.ferror );
        CMIN_abend();                                            /* 異常終了            */
    }
} /* end of CMIN_read_recv */
/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_send_reply                                 */
/*  CALLING SEQ.    : void CMIN_send_reply (char*, short, short)             */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : リプライ処理                                           */
/*****************************************************************************/
void CMIN_send_reply ( char  *reply_buf
                     , short  reply_len
                     , short  reply_cd )
{
    _cc_status  i_CC;

    i_CC = REPLYX ( reply_buf
                  , reply_len
                 ,, g_iocomp.recv_info.z_messagetag
                  , reply_cd );

    if ( _status_ne(i_CC) ) {
        FILE_GETINFO_(g_myinfo.recv_fno, &g_iocomp.ferror);
        CMIN_message_output ( DEF_EVT_PROCEDURE_ERR              /* メッセージ出力処理  */
                            , DEF_MSGTTKB_GYOM_ERR
                            , DEF_NERR_FILE_IO_ERR
                            , "@C@U"
                            , DEF_FILEIO_REPLY
                            , g_iocomp.ferror );
        CMIN_abend();                                            /* 異常終了            */
    }
} /* end of CMIN_send_reply */


/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_finish                                     */
/*  CALLING SEQ.    : void CMIN_finish (void)                                */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 終了処理                                               */
/*****************************************************************************/
void CMIN_finish (void)
{
    short  ls_result;

/* --------------------------------------- */
/* 共通 ファイルCLOSE                      */
/* --------------------------------------- */
    CMIN_file_close( DEF_FL_NW_INFO             /* NW情報ファイルクローズ     */
                   , g_com_file_data.nw_file_name
                   ,&g_com_file_data.nw_file_no );

/* --------------------------------------- */
/* 個別 ファイルCLOSE                      */
/* --------------------------------------- */
    CMIN_kbt_file_close();

/* --------------------------------------- */
/* エラー出力ログ編集出力モジュール(CLOSE) */
/* --------------------------------------- */
    g_com_erl_arg_1.file_io_type     = DEF_COM_ERL_ARG1_CLOSE;
    g_com_erl_arg_1.io_timer         = g_myinfo.io_timer;
    memset(&g_com_erl_arg_2          , DEF_BUF_SPACE, sizeof(g_com_erl_arg_2));
    memset( g_com_erl_arg_2.file_name, DEF_BUF_NULL, sizeof(g_com_erl_arg_2.file_name));
    g_com_erl_arg_2.file_no          = g_com_file_data.erlg_file_no;

    memset(&g_cg010in_modle.emsinf                  , DEF_BUF_SPACE            , sizeof(g_cg010in_modle.emsinf                  ));
    memcpy( g_cg010in_modle.emsinf.emsgkinf.sysnm   , DEF_EMS_SYSNM_GFP        , sizeof(g_cg010in_modle.emsinf.emsgkinf.sysnm   ));
    memcpy( g_cg010in_modle.emsinf.emsgkinf.srv_kbn , DEF_EMS_SRV_KBN_COM      , sizeof(g_cg010in_modle.emsinf.emsgkinf.srv_kbn ));
    memcpy( g_cg010in_modle.emsinf.emsgkinf.h_nw_kbn, g_myinfo.nw_kbn          , sizeof(g_cg010in_modle.emsinf.emsgkinf.h_nw_kbn));
    memcpy( g_cg010in_modle.emsinf.emsgkinf.prgid   , g_myinfo.prog_id         , sizeof(g_cg010in_modle.emsinf.emsgkinf.prgid   ));
    memcpy( g_cg010in_modle.emsinf.emsgkinf.trmnm   , g_procinfo.my_pname      , sizeof(g_procinfo.my_pname               ));

    memset(&g_com_erl_arg_3                   , DEF_BUF_SPACE            , sizeof(g_com_erl_arg_3                   ));
    memcpy(&g_com_erl_arg_3.srv_logical_id    , g_myinfo.serverclass_name, sizeof(g_myinfo.serverclass_name         ));

    ls_result = COM_ERL(&g_com_erl_arg_1
                       ,&g_com_erl_arg_2
                       ,&g_cg010in_modle
                       ,&g_com_erl_arg_3
                       , g_myinfo.prog_id);

    if ( ls_result != DEF_RET_OK ) {
        CMIN_abend();                                            /* 異常終了            */
    }

/* --------------------------------------- */
/* トレース出力終了処理                    */
/* --------------------------------------- */
    gp_trc_ed = (lk_zac2001r_arg_1_def *)&g_trc_buf;
    memset( g_trc_buf, DEF_BUF_SPACE, sizeof(g_trc_buf));
    gp_trc_ed->func_flg = DEF_TRC_FUNC_END;
    memcpy( gp_trc_ed->trace_info.prog_id, g_myinfo.prog_id
                                         , sizeof(gp_trc_ed->trace_info.prog_id));
    memset( gp_trc_ed->data_info.rec_len , DEF_BUF_CZERO
                                         , sizeof(gp_trc_ed->data_info.rec_len ));
    TRACEOUT((char *)g_trc_buf);

    CMIN_message_output ( DEF_EVT_PROC_NORMAL_END       /* メッセージ出力処理  */
                        , DEF_MSGTTKB_NORMAL
                        , DEF_NERR_NOMAL
                        , "@C"
                        , g_procinfo.my_pname );

    /* PROCESS_STOP */
    PROCESS_STOP_(,, DEF_NORMAL_END);

} /* end of CMIN_finish */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_read_glmlg                                 */
/*  CALLING SEQ.    : short CMIN_read_glmlg(char *,short,char *,short,       */
/*                                          char *, short *)                 */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.sh_fie_no       (I)   ファイル番号                   */
/*                  : 3.pch_rec_key     (I)   読込みキー                     */
/*                  : 4.sh_lock         (I)   LOCK有無                       */
/*                  : 5.pch_rec         (O)   読込んだレコード               */
/*                  : 6.psh_error       (O)   I/Oエラーコード                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：COM_IOM異常終了                                    */
/*  DESCRIPTION     : 制御電文ログ読込処理                                   */
/*****************************************************************************/
short CMIN_read_glmlg(
    char                    *pch_pname,         // 物理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    char                    *pch_rec,           // 読込んだレコード
    short                   *psh_error          // I/Oエラーコード
)
{
    char              pch_sub_status[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    //------------------------------------------------------------------------//
    // 初期化
    //------------------------------------------------------------------------//
    memset(&trace_inf,   0x00, sizeof(trace_inf));
    memset(&file_inf,    0x00, sizeof(file_inf));
    memset(&in_inf,      0x00, sizeof(in_inf));
    memset(&out_inf,     0x00, sizeof(out_inf));

    //------------------------------------------------------------------------//
    /* COM_IOM引数定義・共通 */
    //------------------------------------------------------------------------//
    CMIN_set_comiom_arg(pch_pname,
                        DEF_GLMLG,
                        sh_fie_no,
                        pch_rec_key,
                        sh_lock,
                        DEF_GLMLG_KEY_LEN,
                        sizeof(db_glmlg_def),
                        &trace_inf,
                        &file_inf,
                        &in_inf,
                        DEF_FILEIO_READ);

    //------------------------------------------------------------------------//
    // IOモジュール呼出
    //------------------------------------------------------------------------//
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD,
            pch_sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    *psh_error = out_inf.guardian_errcode;

    if(memcmp(pch_sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
    memcpy(pch_rec, out_inf.rec_area, sizeof(db_glmlg_def));

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}/* end of CMIN_read_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_put_glmlg                                  */
/*  CALLING SEQ.    : short CMIN_put_glmlg(char *,short,char *,short,char *, */
/*                                          short *)                         */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.sh_fie_no       (I)   ファイル番号                   */
/*                  : 3.pch_rec_key     (I)   書込みレコード                 */
/*                  : 4.sh_lock         (I)   LOCK有無                       */
/*                  : 5.pch_rec         (O)   読込んだレコード               */
/*                  : 6.psh_error       (O)   I/Oエラーコード                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：COM_IOM異常終了                                    */
/*  DESCRIPTION     : 制御電文ログ出力処理                                   */
/*****************************************************************************/
short CMIN_put_glmlg(
    char                    *pch_pname,         // 物理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    char                    *pch_rec,           // 読込んだレコード
    short                   *psh_error          // I/Oエラーコード
)
{
    char              pch_sub_status[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    db_glmlg_def     *glmlg_rec;
    short             s_rec_len;
    short             s_denbun_len;
    char              ch_denbun_len_char[6];

    // 初期化
    memset(&trace_inf,   0x00, sizeof(trace_inf));
    memset(&file_inf,    0x00, sizeof(file_inf));
    memset(&in_inf,      0x00, sizeof(in_inf));
    memset(&out_inf,     0x00, sizeof(out_inf));

    /* レコード長算出 */
    glmlg_rec = (db_glmlg_def *)pch_rec_key;
    memset(ch_denbun_len_char, 0x00, sizeof(ch_denbun_len_char));
    memcpy(ch_denbun_len_char,
           glmlg_rec->denbun_area.denbun_len,
           sizeof(glmlg_rec->denbun_area.denbun_len));
    s_denbun_len = (short)atoi(ch_denbun_len_char);
    s_rec_len = s_denbun_len + DEF_GLMLG_DENBUN_OFFSET;

    //------------------------------------------------------------------------//
    /* COM_IOM引数定義・共通 */
    //------------------------------------------------------------------------//
    CMIN_set_comiom_arg(pch_pname,
                        DEF_GLMLG,
                        sh_fie_no,
                        pch_rec_key,
                        sh_lock,
                        DEF_GLMLG_KEY_LEN,
                        s_rec_len,
                        &trace_inf,
                        &file_inf,
                        &in_inf,
                        DEF_FILEIO_WRITE);

    // 入力レコード設定
    memcpy(in_inf.rec_area, pch_rec_key, s_rec_len);

    //------------------------------------------------------------------------//
    // IOモジュール呼出
    //------------------------------------------------------------------------//
    COM_IOM(DEF_COM_IOM_FUNC_ADD,
            pch_sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    *psh_error = out_inf.guardian_errcode;

    if(memcmp(pch_sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
#if 0
    memcpy(pch_sub_status, out_inf.rec_area, sizeof(db_glmlg_def));
#endif

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}/* end of CMIN_put_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_update_glmlg                               */
/*  CALLING SEQ.    : short CMIN_update_glmlg(char *,short,char *,short,     */
/*                                          char *, short *)                 */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.sh_fie_no       (I)   ファイル番号                   */
/*                  : 3.pch_rec_key     (I)   更新レコード                   */
/*                  : 4.sh_lock         (I)   LOCK有無                       */
/*                  : 5.pch_rec         (O)   読込んだレコード               */
/*                  : 6.psh_error       (O)   I/Oエラーコード                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：COM_IOM異常終了                                    */
/*  DESCRIPTION     : 制御電文ログ更新処理                                   */
/*****************************************************************************/
short CMIN_update_glmlg(
    char                    *pch_pname,         // 物理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    char                    *pch_rec,           // 読込んだレコード
    short                   *psh_error          // I/Oエラーコード
)
{
    char              pch_sub_status[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    db_glmlg_def     *glmlg_rec;
    short             s_rec_len;
    short             s_denbun_len;
    char              ch_denbun_len_char[6];

    //------------------------------------------------------------------------//
    // 初期化
    //------------------------------------------------------------------------//
    memset(&trace_inf,   0x00, sizeof(trace_inf));
    memset(&file_inf,    0x00, sizeof(file_inf));
    memset(&in_inf,      0x00, sizeof(in_inf));
    memset(&out_inf,     0x00, sizeof(out_inf));

    /* レコード長算出 */
    glmlg_rec = (db_glmlg_def *)pch_rec_key;
    memset(ch_denbun_len_char, 0x00, sizeof(ch_denbun_len_char));
    memcpy(ch_denbun_len_char,
           glmlg_rec->denbun_area.denbun_len,
           sizeof(glmlg_rec->denbun_area.denbun_len));
    s_denbun_len = (short)atoi(ch_denbun_len_char);
    s_rec_len = s_denbun_len + DEF_GLMLG_DENBUN_OFFSET;

    //------------------------------------------------------------------------//
    // COM_IOM引数定義・共通
    //------------------------------------------------------------------------//
    CMIN_set_comiom_arg(pch_pname,
                        DEF_GLMLG,
                        sh_fie_no,
                        pch_rec_key,
                        sh_lock,
                        DEF_GLMLG_KEY_LEN,
                        s_rec_len,
                        &trace_inf,
                        &file_inf,
                        &in_inf,
                        DEF_FILEIO_UPDATE);

    // 入力レコード設定
    memcpy(in_inf.rec_area, pch_rec_key, s_rec_len);

    //------------------------------------------------------------------------//
    // IOモジュール呼出
    //------------------------------------------------------------------------//
    COM_IOM(DEF_COM_IOM_FUNC_UPDATE,
            pch_sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    *psh_error = out_inf.guardian_errcode;

    if(memcmp(pch_sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
#if 0
    memcpy(pch_sub_status, out_inf.rec_area, sizeof(db_glmlg_def));
#endif

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}/* end of CMIN_update_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_unlock_glmlg                               */
/*  CALLING SEQ.    : short CMIN_unlock_glmlg(char *,short,char *,short,     */
/*                                          char *,short *)                  */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.sh_fie_no       (I)   ファイル番号                   */
/*                  : 3.pch_rec_key     (I)   読込みキー                     */
/*                  : 4.sh_lock         (I)   LOCK有無                       */
/*                  : 5.pch_rec         (O)   読込んだレコード               */
/*                  : 6.psh_error       (O)   I/Oエラーコード                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：COM_IOM異常終了                                    */
/*  DESCRIPTION     : 制御電文ログUNLOCK処理                                 */
/*****************************************************************************/
short CMIN_unlock_glmlg(
    char                    *pch_pname,         // 物理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    char                    *pch_rec,           // 読込んだレコード
    short                   *psh_error          // I/Oエラーコード
)
{
    char              pch_sub_status[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    //------------------------------------------------------------------------//
    // 初期化
    //------------------------------------------------------------------------//
    memset(&trace_inf,   0x00, sizeof(trace_inf));
    memset(&file_inf,    0x00, sizeof(file_inf));
    memset(&in_inf,      0x00, sizeof(in_inf));
    memset(&out_inf,     0x00, sizeof(out_inf));

    //------------------------------------------------------------------------//
    // COM_IOM引数定義・共通
    //------------------------------------------------------------------------//
    CMIN_set_comiom_arg(pch_pname,
                        DEF_GLMLG,
                        sh_fie_no,
                        pch_rec_key,
                        sh_lock,
                        DEF_GLMLG_KEY_LEN,
                        sizeof(db_glmlg_def),
                        &trace_inf,
                        &file_inf,
                        &in_inf,
                        DEF_FILEIO_UNLOCKREC);

    //------------------------------------------------------------------------//
    // IOモジュール呼出
    //------------------------------------------------------------------------//
    COM_IOM(DEF_COM_IOM_FUNC_UNLOC,
            pch_sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    *psh_error = out_inf.guardian_errcode;

    if(memcmp(pch_sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
#if 0
    memcpy(pch_sub_status, out_inf.rec_area, sizeof(db_glmlg_def));
#endif

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}/* end of CMIN_unlock_glmlg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_read_gfnws                                 */
/*  CALLING SEQ.    : short CMIN_read_gfnws(char *,short,char *,short,       */
/*                                          char *, short *)                 */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pch_pname       (I)   物理ファイル名                 */
/*                  : 2.sh_fie_no       (I)   ファイル番号                   */
/*                  : 3.pch_rec_key     (I)   読込みキー                     */
/*                  : 4.sh_lock         (I)   LOCK有無                       */
/*                  : 5.pch_rec         (O)   読込んだレコード               */
/*                  : 6.psh_error       (O)   I/Oエラーコード                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：COM_IOM異常終了                                    */
/*  DESCRIPTION     : 接続先固有情報ファイル読込処理                         */
/*****************************************************************************/
short CMIN_read_gfnws(
    char                    *pch_pname,         // 物理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    char                    *pch_rec,           // 読込んだレコード
    short                   *psh_error          // I/Oエラーコード
)
{
    char              pch_sub_status[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    //------------------------------------------------------------------------//
    // 初期化
    //------------------------------------------------------------------------//
    memset(&trace_inf,   0x00, sizeof(trace_inf));
    memset(&file_inf,    0x00, sizeof(file_inf));
    memset(&in_inf,      0x00, sizeof(in_inf));
    memset(&out_inf,     0x00, sizeof(out_inf));

    //------------------------------------------------------------------------//
    // COM_IOM引数定義・共通
    //------------------------------------------------------------------------//
    CMIN_set_comiom_arg(pch_pname,
                        DEF_GFNWS,
                        sh_fie_no,
                        pch_rec_key,
                        sh_lock,
                        DEF_GFNWS_KEY_LEN,
                        sizeof(db_gfnws_def),
                        &trace_inf,
                        &file_inf,
                        &in_inf,
                        DEF_FILEIO_READ);

    // 入力レコード設定
    memcpy(in_inf.rec_area, pch_rec_key, sizeof(db_gfnws_def));

    //------------------------------------------------------------------------//
    // IOモジュール呼出
    //------------------------------------------------------------------------//
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD,
            pch_sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    *psh_error = out_inf.guardian_errcode;

    if(memcmp(pch_sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
    memcpy(out_inf.rec_area, pch_rec_key, sizeof(db_gfnws_def));

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}/* end of CMIN_read_gfnws */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_set_comiom_arg                             */
/*  CALLING SEQ.    : short CMIN_set_comiom_arg(char *,char *,short,         */
/*                                         char *,short, short, short,       */
/*                      COM_IOM_arg_3_def *, COM_IOM_arg_4_def *,            */
/*                      COM_IOM_arg_5_def *, char *)                         */
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
/*                  :11.pch_iotype      (I)   ファイルI/Oタイプ              */
/*  RETURN CODE     :  void                                                  */
/*  DESCRIPTION     : 制御電文ログCOM_IOM用引数設定処理                      */
/*****************************************************************************/
static void CMIN_set_comiom_arg(
    char                    *pch_pname,         // 物理ファイル名
    char                    *pch_lname,         // 論理ファイル名
    short                    sh_fie_no,         // ファイル番号
    char                    *pch_rec_key,       // 読込みキー
    short                    sh_lock,           // LOCK有無
    short                    sh_key_len,        // キー長
    short                    sh_rec_len,        // レコード長
    COM_IOM_arg_3_def       *ptrace_inf,        // トレース情報
    COM_IOM_arg_4_def       *pfile_inf,         // ファイル情報
    COM_IOM_arg_5_def       *pin_inf,           // 入力情報
    char                    *pch_iotype         // ファイルI/Oタイプ
)
{
    //------------------------------------------------------------------------//
    // 物理名情報ファイルよりレコードを取得                                   //
    //------------------------------------------------------------------------//
    // トレース情報・モジュールID
    memset((char *)ptrace_inf, 0x20, sizeof(COM_IOM_arg_3_def));
    CMIN_spctub_cpy(ptrace_inf->prog_id,   g_myinfo.prog_id,         sizeof(ptrace_inf->prog_id));
    // トレース情報・ファイルID
    CMIN_spctub_cpy(ptrace_inf->file_id,   pch_lname,   sizeof(ptrace_inf->file_id));
    // トレース情報・ファイル名
    CMIN_spctub_cpy(ptrace_inf->file_name, pch_pname,  sizeof(ptrace_inf->file_name));
    // トレース情報・ファイルI/Oタイプ
    CMIN_spctub_cpy(ptrace_inf->file_io_type, pch_iotype,  sizeof(ptrace_inf->file_io_type));

    // ファイル情報・論理ファイル名
    memset((char *)pfile_inf, 0x20, sizeof(COM_IOM_arg_4_def));
    CMIN_spctub_cpy(pfile_inf->file_id,   pch_lname,   strlen(pch_lname));
    // ファイル情報・物理ファイル名
    CMIN_spctub_cpy(pfile_inf->file_name, pch_pname,   sizeof(pfile_inf->file_name));
    // ファイル情報・ファイル番号
    pfile_inf->file_no = sh_fie_no;

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
}/* end of CMIN_set_comiom_arg */

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CMIN_spctub_cpy                                 */
/*  CALLING SEQ.    : void CMIN_spctub_cpy(char *char *,size_t)              */
/*  ARGUMENT        : 1.dst                 (I)   コピー先                   */
/*                  : 2.src                 (I)   コピー元                   */
/*                  : 3.len                 (I)   コピー長                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 空白埋めコピー処理                                     */
/*****************************************************************************/
static void CMIN_spctub_cpy(
    char    *dst,
    char    *src,
    size_t  len

)
{
    // 最大255バイトまで
    char szbuf[256];

    snprintf(szbuf,sizeof(szbuf),"%-255.255s", src);
    memcpy(dst, szbuf, len);

    return;
}/* end of CMIN_spctub_cpy */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  CMIN_num_check                                  */
/*  CALLING SEQ.    : void CMIN_num_check (char)                             */
/*  ARGUMENT        : char  チェック配列                                     */
/*  RETURN CODE     : short チェック結果                                     */
/*  DESCRIPTION     : 数字文字チェック処理                                   */
/*****************************************************************************/
short CMIN_num_check (char *check_buf)
{
    short ls_idx;

    /* NULL ストッパーがない場合のチェック 100桁まで */
    if ( strlen(check_buf) > DEF_NUMCHK_MAX_LEN ) {
            return DEF_RET_NG;
    }

    for (ls_idx=0;ls_idx < strlen(check_buf);ls_idx++){
        if ( NULL == isdigit(check_buf[ls_idx])) {
            return DEF_RET_NG;
        }
    }

    return DEF_RET_OK;

} /* end of CMIN_num_check */

/*****************************************************************************/
/*  FUNCTION        : 1.0  通算日算出処理                                    */
/*                                                                           */
/*  CALLING SEQ.    : short CMIN_get_day_of_year(char*, char*)               */
/*  ARGUMENT        : 1.date_p   (I)    年月日 "YYYYMMDD" 指定域アドレス     */
/*                  : 2.tday_p   (I/O)  通算日 "999" 格納域アドレス          */
/*                                                                           */
/*  RETURN CODE     : 0:正常 1:エラー                                        */
/*  DESCRIPTION     :                                                        */
/*****************************************************************************/
short   CMIN_get_day_of_year(char *date_p, char *tday_p)
{
    short     rtncd;                             // 返却値格納域
    long long timestamp_vday;                    // 確認対象日付通算時間
    long long timestamp_bday;                    // 基準（当年1/1）日付通算時間
    int       total_day;                         // 通算日数算出域
    short     curr_date[8];                      // 現日付取得域
    short     base_date[8];                      // 基準日付指定域
    short     vari_date[8];                      // 確認日付指定域
    short     errmask;                           //
    char      yer[4+1];                          // 確認対象年格納域
    char      mon[2+1];                          // 確認対象月格納域
    char      day[2+1];                          // 確認対象日格納域

    /* 初期化                  */
    memset(base_date,0x00,sizeof(base_date));
    memset(vari_date,0x00,sizeof(vari_date));
    memset(curr_date,0x00,sizeof(curr_date));
    memset(yer,0x00,sizeof(yer));
    memset(mon,0x00,sizeof(mon));
    memset(day,0x00,sizeof(day));
    timestamp_vday = 0;
    timestamp_bday = 0;
    total_day      = 0;
    errmask        = 0;
    rtncd          = DEF_RTN_ERROR;

    /* 現年月日取得            */
    TIME(curr_date);

    /* 基準年月日通算秒数取得  */
    base_date[0] = curr_date[0];
    base_date[1] = (short)1;
    base_date[2] = (short)1;
    timestamp_bday = COMPUTETIMESTAMP(base_date,&errmask);

    /* 確認年月日通算秒数取得  */
    memcpy(yer,(date_p + 0),sizeof(yer)-1);
    memcpy(mon,(date_p + 4),sizeof(mon)-1);
    memcpy(day,(date_p + 6),sizeof(day)-1);
    vari_date[0] = (short)atoi(yer);
    vari_date[1] = (short)atoi(mon);
    vari_date[2] = (short)atoi(day);
    timestamp_vday = COMPUTETIMESTAMP(vari_date,&errmask);

    /* 基準日（当年1/1)からの通算日算出 */
    sprintf(tday_p,"%03d",total_day);
    if (timestamp_bday <= timestamp_vday) {
        total_day = (int)((timestamp_vday - timestamp_bday) / DEF_DAY_USEC + 1);
        sprintf(tday_p,"%03d",total_day);
        rtncd = DEF_RTN_NORMAL;
    }
    return(rtncd);
} /* end of CMIN_get_day_of_year */

/****************************************************************************/
/*  FUNCTION        : 4.0.0  CMIN_check_datetime                            */
/*  CALLING SEQ.    : short CMIN_check_datetime(char *date_time)            */
/*  ARGUMENT        : 1.day_time  (I)日時(文字列)  YYYYMMDDhhmmss           */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : 日付形式チェック                                      */
/****************************************************************************/
short CMIN_check_datetime(char  *date_time)
{
    short  ts[8];
    short  get_ts[16];
    char   day_time_save[4+1];
    short  errmask      = 0;
    short  ts_cnt       = 0;
    short  day_cnt      = 4;
    short  loop_cnt     = 0;
    long long timestamp = 0;

    memset(ts,            0, sizeof(ts));
    memset(day_time_save, 0, sizeof(day_time_save));
    memset(get_ts,        0, sizeof(get_ts));

    memcpy(day_time_save, date_time, 4);
    ts[ts_cnt] = (short)atoi(day_time_save);
    memset(day_time_save, 0, sizeof(day_time_save));

    for(loop_cnt=1; loop_cnt<6 ; loop_cnt++){
        memcpy(day_time_save, &date_time[day_cnt], 2);
        ts[loop_cnt] = (short)atoi(day_time_save);
        day_cnt+=2;
    }

    /* YYYYが0の場合はYYYYを設定する */
    if (ts[0] == 0){
        TIME(get_ts);
        ts[0] = get_ts[0];
    }

    /* 日時チェック */
    timestamp = COMPUTETIMESTAMP(ts,&errmask);
    if (errmask != DEF_RET_OK){
        /* 異常終了 */
        return DEF_RET_NG;
    }

    return DEF_RET_OK;

} /* end of CMIN_check_datetime */

/*****************************************************************************/
/*  FUNCTION        : x.x.x  CMIN_message_output                             */
/*  CALLING SEQ.    : void CMIN_message_output()                             */
/*  ARGUMENT        :                                                        */
/*                  :                                                        */
/*                  :                                                        */
/*                  :                                                        */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : メッセージ出力処理                                     */
/*****************************************************************************/
void CMIN_message_output ( short   s_event_code
                         , char    ch_kubun
                         , char   *pch_inter_code
                         , char   *pch_format, ...)
{
    char        ch_text[99];
    char        ch_fmt[99];
    char        *pch_ep, *pch_sp;
    short       s_var, s_vcnt = 0, s_cnt, s_idx, s_param_cnt = 0, loop_flg = 1;
    va_list     va_ap;


    g_cg010in_modle.subrcd     = '0';                       /* サブルーチンリプライコード */
    g_cg010in_modle.emsinf.rcd = '0';                       /* リターンコード             */
                                                            /* EMS出力情報                */
    memset ( ch_text, 0x00  , sizeof(ch_text));             /*  イベント番号/メッセージID */
    sprintf( ch_text, "%05d", s_event_code);
    memcpy ( g_cg010in_modle.emsinf.msgid, ch_text, strlen(ch_text));
    memset (&g_cg010in_modle.emsinf.emsgkinf                /*  業務共通メッセージ        */
           , DEF_BUF_SPACE      , sizeof(g_cg010in_modle.emsinf.emsgkinf));
    g_cg010in_modle.emsinf.emsgkinf.msgttkb = ch_kubun;     /*  メッセージ通知区分        */
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.sysnm          /*  システム名                */
           , DEF_EMS_SYSNM_GFP  , sizeof(g_cg010in_modle.emsinf.emsgkinf.sysnm));
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.srv_kbn        /*  SERVER分類                */
           , DEF_EMS_SRV_KBN_COM, sizeof(g_cg010in_modle.emsinf.emsgkinf.srv_kbn));
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.h_nw_kbn       /*  NW識別(被仕向)            */
           , g_myinfo.nw_kbn, sizeof(g_myinfo.nw_kbn));
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.prgid          /*  プログラムID              */
           , g_myinfo.prog_id   , sizeof(g_cg010in_modle.emsinf.emsgkinf.prgid));
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.trmnm          /*  プロセス名                */
           , g_procinfo.my_pname, sizeof(g_procinfo.my_pname));
    memcpy ( g_cg010in_modle.emsinf.emsgkinf.inter_errcd    /*  GFP内部エラーコード       */
           , pch_inter_code     , sizeof(g_cg010in_modle.emsinf.emsgkinf.inter_errcd));
    memset ( (char *)&g_cg010in_modle.emsinf.emsnninf       /*  任意メッセージ            */
           , DEF_BUF_SPACE      , sizeof(g_cg010in_modle.emsinf.emsnninf));

    if ( g_myinfo.serverclass_name[0] != DEF_BUF_NULL ) {   /* サーバークラス論理ID       */
        memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
               , g_myinfo.serverclass_name, 12);
    }
    s_param_cnt++;

    memset ( ch_fmt, DEF_BUF_NULL, sizeof(ch_fmt));
    memcpy ( ch_fmt, pch_format  , strlen(pch_format));

    for ( s_cnt=0, s_idx=0; s_idx < strlen(ch_fmt); s_idx++) {
        if (ch_fmt[s_idx]==0x40) s_cnt++;
    }

    va_start(va_ap, pch_format);
    pch_sp = ch_fmt;
    while ( pch_sp[0] && loop_flg) {
        switch (pch_sp[0]) {
        case DEF_MSG_TYPE_SKIP:    /* skip */
            pch_sp++;
            break;
        case DEF_MSG_TYPE_CHAR:    /* charデータ */
            pch_ep = (char *)va_arg(va_ap, char *);
            memset( ch_text, 0x00, sizeof(ch_text));
            memcpy( ch_text, pch_ep, strlen(pch_ep));
            memcpy( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                  , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_HEX:     /* charデータ(HEX表示) */
            pch_ep = (char *)va_arg(va_ap, char *);
            memset  ( ch_text, 0x00, sizeof(ch_text));
            HEX2CHAR( pch_ep, ch_text, 24 );
            memcpy  ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                    , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_LCN:     /* charデータ(LCN)15桁 */
            pch_ep = (char *)va_arg(va_ap, char *);
            memset( ch_text, 0x00, sizeof(ch_text));
            memcpy( ch_text, pch_ep, DEF_MSG_TYPE_LCN_LEN);
            memcpy( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                  , ch_text, DEF_MSG_TYPE_LCN_LEN);
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_SVRCLS:   /* charデータ(ServerClass)8桁 */
            pch_ep = (char *)va_arg(va_ap, char *);
            memset( ch_text, 0x00, sizeof(ch_text));
            memcpy( ch_text, pch_ep, DEF_MSG_TYPE_SVRCLS_LEN);
            memcpy( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                  , ch_text, DEF_MSG_TYPE_SVRCLS_LEN);
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_CNECT:     /* charデータ(接続先)24桁 */
            pch_ep = (char *)va_arg(va_ap, char *);
            memset( ch_text, 0x00, sizeof(ch_text));
            memcpy( ch_text, pch_ep, DEF_MSG_TYPE_CNECT_LEN);
            memcpy( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                  , ch_text, DEF_MSG_TYPE_CNECT_LEN);
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_1:   /* 1桁BINARY(フラグ) */
            s_var = (short)va_arg(va_ap, short);
            sprintf(ch_text,"%01d", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_2:   /* 2桁BINARY(CPU番号) */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%02d", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_3:   /* 3桁BINARY(内部テーブル数) */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%03d", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_4:   /* 4桁BINARY(システムエラーコード) */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%04d", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_5:   /* 5桁BINARY、エラーコード */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%05d", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_U5:  /* 5桁BINARY(UNSIGN)、メッセージ番号、エラーコード */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%05u", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        case DEF_MSG_TYPE_BIN_8:   /* 8桁BINARY */
            s_var = (short)va_arg(va_ap, short);
            sprintf( ch_text,"%08", s_var);
            memcpy ( g_cg010in_modle.emsinf.emsnninf.msgtbl[s_param_cnt].msgtbl_vl
                   , ch_text, strlen(ch_text));
            pch_sp++;
            s_param_cnt++;
            s_vcnt++;
            break;
        default:
            pch_sp++;
            break;
        }
        if (s_vcnt==s_cnt) break;
    }
    va_end(va_ap);

    /* 運用監視端末出力 */
    GFPOGGZ1(&g_cg010in_modle);

} /* CMIN_message_output */

/*****************************************************************************/
/*  FUNCTION        : x.x.x  CMIN_abend                                      */
/*  CALLING SEQ.    : void CMIN_abend(void)                                  */
/*  ARGUMENT        : void                                                   */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : ABEND処理                                              */
/*****************************************************************************/
void CMIN_abend (void)
{
    CMIN_message_output ( DEF_EVT_PROC_ABNORMAL_END              /* メッセージ出力処理  */
                        , DEF_MSGTTKB_GYOM_ERR
                        , DEF_NERR_SYSIF_LGC_ERR
                        , "@C"
                        , g_procinfo.my_pname );
    PROCESS_STOP_(,, DEF_ABNORMAL_END);

} /* end of CMIN_abend */

