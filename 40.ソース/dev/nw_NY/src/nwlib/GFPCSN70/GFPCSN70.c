/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSN70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(NYCE)    */
/*                               処理を行う                                  */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-07-28                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/07/28 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
// #include "zspic"  nolist
// #include "zfilc"  nolist
#include "zsysc"  nolist

#include "msg_NY.h"                          /* 制御電文(nyce)               */
#include "file.h"
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVXZ2.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVX80.h"                        /* 局状態・エコー制御サーバ     */
#include "GFPCVX82.h"                        /* 局状態・エコー制御サーバ     */
#include "GFPCSN70.h"                        /* 局状態・エコー制御サーバ     */
#include "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_check_reqmsg                            */
/*  CALLING SEQ.    : short  NWM_STE_check_reqmsg(                           */
/*                          char*,char*,char*,char*,char*,char*,char*,char*, */
/*                          gflin_pkey_def*,t_rcv_info_def*,char*)           */
/*  ARGUMENT        : 1.p_rcv_data     (I)   受信応答                        */
/*                  : 2.p_rcv_data_len (I)   受信応答レングス                */
/*                  : 3.p_nw_info_gp   (I)   NW情報rec(Group)                */
/*                  : 4.p_nw_info_if   (I)   NW情報rec(InterFace)            */
/*                  : 5.p_cn_info_nw   (I)   接続先固有情報rec(NetWork)      */
/*                  : 6.p_cn_info_if   (I)   接続先固有情報rec(InterFace)    */
/*                  : 7.p_cn_info_st   (I)   接続先固有情報rec(Station)      */
/*                  : 8.p_cn_info_cn   (I)   接続先固有情報rec(connection)   */
/*                  : 9.p_con_lid      (I)   コネクション論理ID              */
/*                  : 10.p_rslt_info   (I/O) 処理結果情報                    */
/*                  : 11.p_gcsst )     (I)   局状態管理Fileレコード          */
/*  RETURN CODE     : 0：精査OK                                              */
/*                  : 1：精査エラー（拒否応答）                              */
/*                  : 2：精査エラー（障害電文通知）                          */
/*                  : 3：精査エラー（破棄）                                  */
/*  DESCRIPTION     : 開閉局･エコー要求電文精査                              */
/*****************************************************************************/
short  NWM_STE_check_reqmsg ( char           *p_rcv_data                      /* 受信応答                      */
                            , char           *p_rcv_data_len                  /* 受信応答レングス              */
                            , char           *p_nw_info_gp                    /* NW情報rec(Group)              */
                            , char           *p_nw_info_if                    /* NW情報rec(InterFace)          */
                            , char           *p_cn_info_nw                    /* 接続先固有情報rec(NetWork)    */
                            , char           *p_cn_info_if                    /* 接続先固有情報rec(InterFace)  */
                            , char           *p_cn_info_st                    /* 接続先固有情報rec(Station)    */
                            , char           *p_cn_info_cn                    /* 接続先固有情報rec(connection) */
                            , gflin_pkey_def *p_con_lid                       /* コネクション論理ID            */
                            , t_rcv_info_def *p_rslt_info                     /* 処理結果情報                  */
                            , char           *p_gcsst )                       /* 局状態管理Fileレコード        */
{
    short  ls_result;
    char   lc_wbuf[30];
    short  *wk_p     = NULL;

    msg_nyce_def      *lp_reqmsg;            /* 受信電文用P                 */

    fixedform_nyce_0800_def
                         *lp_mtidata;        /* 電文固定フォーマット        */

    lp_reqmsg         = (msg_nyce_def *)p_rcv_data;


/* ------------------------------------------------------------------------ */
/* (2) 被仕向要求電文精査                                                   */
/* ------------------------------------------------------------------------ */

    lp_mtidata = (fixedform_nyce_0800_def *)&lp_reqmsg->ffd;
/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if (memcmp (lp_reqmsg->mti , DEF_NY_MTI_0800_REQ   ,strlen(DEF_NY_MTI_0800_REQ)) !=0) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                     , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_HAKI;
    }

/* (b) BITMAP部精査 */
    /* --- BIT007 ----------------- */
    if ( lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_007
                                   , strlen(DEF_CHK_ERR_BIT_007));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT011 ----------------- */
    if ( lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_011
                                   , strlen(DEF_CHK_ERR_BIT_011));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT070 ----------------- */
    if ( lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_070
                                   , strlen(DEF_CHK_ERR_BIT_070));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT096 ----------------- */
    if (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )||  /* 開局             */
        ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )) { /* 閉局             */
        if ( lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist != true ){
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_096
                                       , strlen(DEF_CHK_ERR_BIT_096));
            return NWM_STE_SEISA_HAKI;
        }
    } else 
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND  ) { /* エコーテスト     */
        if ( lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist != false ){
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_096
                                       , strlen(DEF_CHK_ERR_BIT_096));
            return NWM_STE_SEISA_HAKI;
        }
    }
        /* --- BIT125 ----------------- */
    if ( lp_mtidata->b125_nw_mng_info.ffd_header.m_flg_exist != false ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_125
                                   , strlen(DEF_CHK_ERR_BIT_125));
        return NWM_STE_SEISA_HAKI;
    }

/* (c) データ部精査 */
    /* --- BIT007 ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '0';
    lc_wbuf[1] = '0';
    lc_wbuf[2] = '0';
    lc_wbuf[3] = '0';
    memcpy (&lc_wbuf[4], lp_mtidata->b007_trans_date_time.ffd_data, DEF_BIT007_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_007
                                   , strlen(DEF_CHK_ERR_BIT_007));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT011 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b011_system_audit_number.ffd_data
                   , lp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_011
                                   , strlen(DEF_CHK_ERR_BIT_011));
        return NWM_STE_SEISA_HAKI;
    }

    return NWM_STE_SEISA_NORMAL;
} /* end of NWM_STE_check_reqmsg */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_check_rspmsg                           */
/*  CALLING SEQ.    : short  NWM_STE_check_rspmsg(                          */
/*                         char*,char*,char*,char*,char*,char*,char*,char*, */
/*                         char*,gflin_pkey_def*,t_rcv_info_def*)           */
/*  ARGUMENT        : 1.p_rcv_data     (I)   受信応答                       */
/*                  : 2.p_rcv_data_len (I)   受信応答レングス               */
/*                  : 3.p_snd_data     (I)   仕向要求                       */
/*                  : 4.p_nw_info_gp   (I)   NW情報rec(Group)               */
/*                  : 5.p_nw_info_if   (I)   NW情報rec(InterFace)           */
/*                  : 6.p_cn_info_nw   (I)   接続先固有情報rec(NetWork)     */
/*                  : 7.p_cn_info_if   (I)   接続先固有情報rec(InterFace)   */
/*                  : 8.p_cn_info_st   (I)   接続先固有情報rec(Station)     */
/*                  : 9.p_cn_info_cn   (I)   接続先固有情報rec(connection)  */
/*                  : 10.p_con_lid     (I)   コネクション論理ID             */
/*                  : 11.p_rslt_info   (I/O) 処理結果情報                   */
/*  RETURN CODE     : 0：精査OK                                             */
/*                  : 1：精査エラー（拒否応答）                             */
/*                  : 2：精査エラー（障害電文通知）                         */
/*                  : 3：精査エラー（破棄）                                 */
/*  DESCRIPTION     : 開閉局･エコー応答電文精査                             */
/****************************************************************************/
short  NWM_STE_check_rspmsg ( char           *p_rcv_data     /* 受信応答                      */
                            , char           *p_rcv_data_len /* 受信応答レングス              */
                            , char           *p_snd_data     /* 仕向要求                      */
                            , char           *p_nw_info_gp   /* NW情報rec(Group)              */
                            , char           *p_nw_info_if   /* NW情報rec(InterFace)          */
                            , char           *p_cn_info_nw   /* 接続先固有情報rec(NetWork)    */
                            , char           *p_cn_info_if   /* 接続先固有情報rec(InterFace)  */
                            , char           *p_cn_info_st   /* 接続先固有情報rec(Station)    */
                            , char           *p_cn_info_cn   /* 接続先固有情報rec(connection) */
                            , gflin_pkey_def *p_con_lid      /* コネクション論理ID            */
                            , t_rcv_info_def *p_rslt_info )  /* 処理結果情報                  */
{
    short  ls_result;
    short  *wk_p     = NULL;
    char   lc_wbuf[30];

    msg_nyce_def         *lp_rcvmsg;            /* 制御電文固定フォーマット（応答電文）       */
    fixedform_nyce_0810_def                      /* データ部固定フォーマット（応答電文）       */
                            *lp_mtidata;

    lp_rcvmsg         = (msg_nyce_def *)p_rcv_data;

    lp_mtidata     = (fixedform_nyce_0810_def *)&lp_rcvmsg->ffd;



/* ------------------------------------------------------------------------ */
/* (2) 仕向応答電文精査                                                     */
/* ------------------------------------------------------------------------ */

/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if (memcmp (lp_rcvmsg->mti , DEF_NY_MTI_0810_RSP, strlen(DEF_NY_MTI_0810_RSP)) !=0) {
         memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                      , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_HAKI;
    }

/* (b) BITMAP部精査 */

    /* --- BIT007 ----------------- */
    if ( lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_007
                                   , strlen(DEF_CHK_ERR_BIT_007));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT011 ----------------- */
    if ( lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_011
                                   , strlen(DEF_CHK_ERR_BIT_011));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT039 ----------------- */
    if ( lp_mtidata->b039_response_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_039
                                   , strlen(DEF_CHK_ERR_BIT_039));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT070 ----------------- */
    if ( lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_070
                                   , strlen(DEF_CHK_ERR_BIT_070));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT007 ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '0';
    lc_wbuf[1] = '0';
    lc_wbuf[2] = '0';
    lc_wbuf[3] = '0';
    memcpy (&lc_wbuf[4], lp_mtidata->b007_trans_date_time.ffd_data, DEF_BIT007_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_007
                                   , strlen(DEF_CHK_ERR_BIT_007));
        return NWM_STE_SEISA_HAKI;
    }

/* (3) 許可／拒否判定 */
    if ( memcmp( lp_mtidata->b039_response_code.ffd_data
               , DEF_ACT_NORMAL      , sizeof(lp_mtidata->b039_response_code.ffd_data)) != 0 ) {
        /* レスポンスコード(00以外) */
        /* 制御電文種別4桁目に"B"を設定 */
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
        return NWM_STE_SEISA_KYOHI;
    } else {
        /* レスポンスコード(00) */
        /* 制御電文種別4桁目に"A"を設定 */
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;
    }

    return NWM_STE_SEISA_NORMAL;
} /* end of NWM_STE_check_rspmsg */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_edit_reqmsg                            */
/*  CALLING SEQ.    : short  NWM_STE_edit_reqmsg(                           */
/*                         char*,char*,char*,char*,char*,char*,char*,       */
/*                         gflin_pkey_def*,NWM_CTU_INI_arg_2_def*,          */
/*                         t_rcv_info_def*,oggz1in_def*,ems_info_add*)      */
/*  ARGUMENT        : 1.p_rsp_data     (I)   送信電文                       */
/*                  : 2.p_nw_info_gp   (I)   NW情報rec(Group)               */
/*                  : 3.p_nw_info_if   (I)   NW情報rec(InterFace)           */
/*                  : 4.p_cn_info_nw   (I)   接続先固有情報rec(NetWork)     */
/*                  : 5.p_cn_info_if   (I)   接続先固有情報rec(InterFace)   */
/*                  : 6.p_cn_info_st   (I)   接続先固有情報rec(Station)     */
/*                  : 7.p_cn_info_cn   (I)   接続先固有情報rec(connection)  */
/*                  : 8.p_con_lid      (I)   コネクション論理ID             */
/*                  : 9.p_gccut_info   (I)   カット対象日付管理File情報     */
/*                  : 10.p_rslt_info   (I/O) 処理結果情報                   */
/*                  : 11.p_cg010in     (I)   EMS出力共通情報                */
/*                  : 12.p_ems_info_add(I)   EMS出力付加情報                */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 開閉局･エコー要求電文編集                             */
/****************************************************************************/
short  NWM_STE_edit_reqmsg ( char                  *p_rsp_data       /* 送信電文                      */
                           , char                  *p_nw_info_gp     /* NW情報rec(Group)              */
                           , char                  *p_nw_info_if     /* NW情報rec(InterFace)          */
                           , char                  *p_cn_info_nw     /* 接続先固有情報rec(NetWork)    */
                           , char                  *p_cn_info_if     /* 接続先固有情報rec(InterFace)  */
                           , char                  *p_cn_info_st     /* 接続先固有情報rec(Station)    */
                           , char                  *p_cn_info_cn     /* 接続先固有情報rec(connection) */
                           , gflin_pkey_def        *p_con_lid        /* コネクション論理ID            */
                           , NWM_CTU_INI_arg_2_def *p_gccut_info     /* カット対象日付管理File情報    */
                           , t_rcv_info_def        *p_rslt_info      /* 処理結果情報                  */
                           , oggz1in_def           *p_cg010in        /* EMS出力共通情報               */
                           , ems_info_add          *p_ems_info_add ) /* EMS出力付加情報               */
{
    long long                ll_date_time;
    char                     lc_wbuf[100];

    msg_nyce_def            *lp_sreqmsg;        /* 仕向要求用P                 */
    db_gfnws_def            *lp_gfnws_if;       /* 接続先固有情報NW用P         */
    nws_unq_info_ny_def     *lp_nws_nyce_if;    /* 接続先固有情報NW制御情報用P */
    fixedform_nyce_0800_def                      /* データ部p                   */
                            *lp_mtidata;

    COM_SDT_arg_2_def        com_sdt_arg_2;     /* 日時取得用データ            */
    COM_SDT_arg_3_def        com_sdt_arg_3;     /* 日時取得用データ            */

    lp_sreqmsg        = (msg_nyce_def *)p_rsp_data;
    lp_mtidata        = (fixedform_nyce_0800_def *)&lp_sreqmsg->ffd;

    lp_gfnws_if       = (db_gfnws_def *)p_cn_info_if;
    lp_nws_nyce_if    = (nws_unq_info_ny_def *)lp_gfnws_if->dst_unq_info;


/* ------------------------------------------------------------------------ */
/* 共通制御ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    /* 送信日時設定  */
    COM_SDT ( DEF_COM_SDT_arg1_gmt, &com_sdt_arg_2, &com_sdt_arg_3, &ll_date_time);
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));

    memset( (char *)&lp_sreqmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_nyce_0800_def));

/* ------------------------------------------------------------------------ */
/* (a) MTI編集                                                              */
/* ------------------------------------------------------------------------ */
    memcpy( lp_sreqmsg->mti , DEF_NY_MTI_0800_REQ, DEF_MTI_LEN);
    memcpy( p_rslt_info->mti, DEF_NY_MTI_0800_REQ, DEF_MTI_LEN);
/* ------------------------------------------------------------------------ */
/* (b) データ部編集                                                         */
/* ------------------------------------------------------------------------ */

    /* --- BIT007 ----------------- */
    lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist           = true;
    lp_mtidata->b007_trans_date_time.ffd_header.m_fixvalue_length     = DEF_BIT007_DATA_LEN;
    memcpy( lp_mtidata->b007_trans_date_time.ffd_data
          ,&com_sdt_arg_2.mm[0]    , DEF_BIT007_DATA_LEN);

    /* --- BIT011 ----------------- */
    lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_BIT011_DATA_LEN;
    memcpy( lp_mtidata->b011_system_audit_number.ffd_data
          ,p_rslt_info->sys_no     , DEF_BIT011_DATA_LEN);

    /* --- BIT070 ----------------- */
    lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist               = true;
    lp_mtidata->b070_nw_mng_code.ffd_header.m_fixvalue_length         = DEF_BIT070_DATA_LEN;
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {  /* 開局 */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data
              ,DEF_NY_F70_INFOCODE_061_SON   , DEF_BIT070_DATA_LEN);
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data
              ,DEF_NY_F70_INFOCODE_062_SOF   , DEF_BIT070_DATA_LEN);
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND ) {  /* エコー */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data
              ,DEF_NY_F70_INFOCODE_361_ECH   , DEF_BIT070_DATA_LEN);
    }
    /* --- BIT096 ----------------- */
    if (( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) ||    /* 開局 */
       ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )) {  /* 閉局 */
        lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist         = true;
        lp_mtidata->b096_msg_security_code.ffd_header.m_fixvalue_length   = DEF_BIT096_DATA_LEN;
        memcpy( lp_mtidata->b096_msg_security_code.ffd_data
              , lp_nws_nyce_if->msg_sec_code, DEF_BIT096_DATA_LEN);
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND ) {  /* エコー */
        lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist         = false;
        lp_mtidata->b096_msg_security_code.ffd_header.m_fixvalue_length   = 0;
        memset( lp_mtidata->b096_msg_security_code.ffd_data
              ,DEF_BUF_SPACE    , DEF_BIT096_DATA_LEN);
    }
    /* --- BIT125 ----------------- */
    lp_mtidata->b125_nw_mng_info.ffd_header.m_flg_exist               = false;
    lp_mtidata->b125_nw_mng_info.ffd_header.m_fixvalue_length         = 0;
    memset( lp_mtidata->b125_nw_mng_info.ffd_data
          ,DEF_BUF_SPACE    , DEF_BIT125_DATA_LEN);

    /* データ長算出 */
    p_rslt_info->denbun_len = DEF_MTI_LEN
                            + sizeof(fixedform_nyce_0800_def);

    return NWM_STE_EDIT_NORMAL;
} /* end of NWM_STE_edit_reqmsg */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_edit_rspmsg                            */
/*  CALLING SEQ.    : short  NWM_STE_edit_rspmsg(                           */
/*                         char*,char*,char*,char*,char*,char*,char*,char*, */
/*                         gflin_pkey_def*,NWM_CTU_INI_arg_2_def*,          */
/*                         t_rcv_info_def*,oggz1in_def*,ems_info_add*)      */
/*  ARGUMENT        : 1.p_snd_data     (I)   送信応答                       */
/*                  : 2.p_rcv_data     (I)   被仕向要求                     */
/*                  : 3.p_nw_info_gp   (I)   NW情報rec(Group)               */
/*                  : 4.p_nw_info_if   (I)   NW情報rec(InterFace)           */
/*                  : 5.p_cn_info_nw   (I)   接続先固有情報rec(NetWork)     */
/*                  : 6.p_cn_info_if   (I)   接続先固有情報rec(InterFace)   */
/*                  : 7.p_cn_info_st   (I)   接続先固有情報rec(Station)     */
/*                  : 8.p_cn_info_cn   (I)   接続先固有情報rec(connection)  */
/*                  : 9.p_con_lid      (I)   コネクション論理ID             */
/*                  : 10.p_gccut_info  (I)   カット対象日付管理File情報     */
/*                  : 11.p_rslt_info   (I/O) 処理結果情報                   */
/*                  : 12.p_cg010in     (I)   EMS出力共通情報                */
/*                  : 13.p_ems_info_add(I)   EMS出力付加情報                */
/*  RETURN CODE     : 0:正常終了  -1:異常終了                               */
/*  DESCRIPTION     : 開閉局･エコー応答電文編集                             */
/****************************************************************************/
short  NWM_STE_edit_rspmsg ( char                  *p_snd_data       /* 送信応答                   */
                           , char                  *p_rcv_data       /* 被仕向要求                 */
                           , char                  *p_nw_info_gp     /* NW情報rec(GP)              */
                           , char                  *p_nw_info_if     /* NW情報rec(IF)              */
                           , char                  *p_cn_info_nw     /* 接続先固有情報rec(NW)      */
                           , char                  *p_cn_info_if     /* 接続先固有情報rec(IF)      */
                           , char                  *p_cn_info_st     /* 接続先固有情報rec(ST)      */
                           , char                  *p_cn_info_cn     /* 接続先固有情報rec(CN)      */
                           , gflin_pkey_def        *p_con_lid        /* コネクション論理ID         */
                           , NWM_CTU_INI_arg_2_def *p_gccut_info     /* カット対象日付管理File情報 */
                           , t_rcv_info_def        *p_rslt_info      /* 処理結果情報               */
                           , oggz1in_def           *p_cg010in        /* EMS出力共通情報            */
                           , ems_info_add          *p_ems_info_add ) /* EMS出力付加情報            */
{
    msg_nyce_def         *lp_rspmsg;         /* 送信電文用P                 */
    msg_nyce_def         *lp_rcvmsg;         /* 受信電文用P                 */
    fixedform_nyce_0810_def                  /* 応答電文データ部p           */
                            *lp_rsp_mtidata;
    fixedform_nyce_0800_def                  /* 受信電文データ部p           */
                            *lp_rcv_mtidata;

    lp_rspmsg         = (msg_nyce_def *)p_snd_data;
    lp_rcvmsg         = (msg_nyce_def *)p_rcv_data;

/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_rspmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_nyce_0810_def));

    memcpy( lp_rspmsg->mti  , DEF_NY_MTI_0810_RSP, DEF_MTI_LEN);
    memcpy( p_rslt_info->mti, DEF_NY_MTI_0810_RSP, DEF_MTI_LEN);


    memcpy( lp_rspmsg->mti  , DEF_NY_MTI_0810_RSP, DEF_MTI_LEN);
    memcpy( p_rslt_info->mti, DEF_NY_MTI_0810_RSP, DEF_MTI_LEN);

    lp_rsp_mtidata = (fixedform_nyce_0810_def *)&lp_rspmsg->ffd;
    lp_rcv_mtidata = (fixedform_nyce_0800_def *)&lp_rcvmsg->ffd;

    /* --- BIT007 ----------------- */
    lp_rsp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b007_trans_date_time.ffd_header.m_fixvalue_length = DEF_BIT007_DATA_LEN;
    memcpy( lp_rsp_mtidata->b007_trans_date_time.ffd_data
          , lp_rcv_mtidata->b007_trans_date_time.ffd_data, DEF_BIT007_DATA_LEN);
    /* --- BIT011 ----------------- */
    lp_rsp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_BIT011_DATA_LEN;
    memcpy( lp_rsp_mtidata->b011_system_audit_number.ffd_data
          , lp_rcv_mtidata->b011_system_audit_number.ffd_data, DEF_BIT011_DATA_LEN);
    /* --- BIT039 ----------------- */
    lp_rsp_mtidata->b039_response_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b039_response_code.ffd_header.m_fixvalue_length = DEF_BIT039_DATA_LEN;

    memcpy( lp_rsp_mtidata->b039_response_code.ffd_data
          , DEF_ACT_NORMAL , DEF_BIT039_DATA_LEN);

    /* --- BIT070 ----------------- */
    lp_rsp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b070_nw_mng_code.ffd_header.m_fixvalue_length = DEF_BIT070_DATA_LEN;
    memcpy( lp_rsp_mtidata->b070_nw_mng_code.ffd_data
          , lp_rcv_mtidata->b070_nw_mng_code.ffd_data, DEF_BIT070_DATA_LEN);

    /* (2) 許可／拒否判定 */
    if ( memcmp(lp_rsp_mtidata->b039_response_code.ffd_data, DEF_ACT_NORMAL    , strlen(DEF_ACT_NORMAL)   ) == 0 ) {
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;
    } else {
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
    }
    /* データ長算出 */
    p_rslt_info->denbun_len = DEF_MTI_LEN
                            + sizeof(fixedform_nyce_0810_def);

    return NWM_STE_EDIT_NORMAL;
} /* end of NWM_STE_edit_rspmsg */

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_req_rcv                       */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_req_rcv(                      */
/*                              char*, t_rcv_info_def*)                      */
/*  ARGUMENT        : 1. p_stn_sts       (I)   局状態                        */
/*                  : 2. p_rslt_info     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 許可応答                                            */
/*                  : 1: 拒否応答                                            */
/*                  : 2: 電文破棄                                            */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(要求)                      */
/*****************************************************************************/
short  NWM_STE_cst_check_req_rcv ( char           *p_stn_sts     /* 局状態       */
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_SST_HAKI ;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)     ) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if (( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 )) {
            memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN , strlen(DEF_STTE_STS_OPN));
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_HAKI;
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_CLSING
                        , sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if ((memcmp(p_stn_sts, DEF_STTE_STS_OPN     , strlen(DEF_STTE_STS_OPN)    ) == 0 )  || 
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )  ||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
            ls_retcode = NWM_STE_SST_KYOKA;
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {
        if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        }
    }

    return ls_retcode;  /* 許可応答 */

} /* end of NWM_STE_cst_check_req_rcv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_rcv                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_rcv(                     */
/*                                       char*,char*,char*,t_rcv_info_def*) */
/*  ARGUMENT        : 1.p_stn_sts      (I)   局状態                         */
/*                  : 2.p_req_type     (I)   要求種別                       */
/*                  : 3.p_rcv_data     (I)   受信電文                       */
/*                  : 4.p_rslt_info    (I/O) 処理結果情報                   */
/*  RETURN CODE     : 0：管理ファイル更新あり                               */
/*                  : 1：管理ファイル更新なし                               */
/*                  : 2：管理ファイル更新あり（開局リトライ有り）           */
/*                  : 3：管理ファイル更新なし（開局リトライ有り）           */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(仕向応答)                 */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_rcv ( char           *p_stn_sts     /* 局状態       */
                                 , char           *p_req_type    /* 要求種別     */
                                 , char           *p_rcv_data    /* 受信電文     */
                                 , t_rcv_info_def *p_rslt_info )  /* 処理結果情報 */
{
    short  ls_retcode;
    short  naibu_rslt = DEF_RET_OK;  /* 仕向応答受信結果(内部) */

    ls_retcode     = NWM_STE_SST_NOUPDATE;

    if ( memcmp( p_req_type, DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == 0 ) {
        /* "20"：仕向応答 */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            /* 許可応答 */
            /* 仕向応答 OK (内部) */
            naibu_rslt = DEF_RET_OK;
        } else {
            /* 拒否応答 */
            /* 仕向応答 NG (内部) */
            naibu_rslt = DEF_RET_NG;
        }
    } else {
        /* "30"：仕向応答タイムアウト  "40"：仕向要求送信不可 */
        /* 仕向応答 NG (内部) */
        naibu_rslt = DEF_RET_NG;
    }

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( naibu_rslt == DEF_RET_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            } 
        } else {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            } 
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {
        if ( naibu_rslt == DEF_RET_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            } 
        } else {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            }
        }
    }

    return ls_retcode;
} /* end of NWM_STE_cst_check_rsp_rcv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_err                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_err(                     */
/*                                      char*, t_rcv_info_def*)             */
/*  ARGUMENT        : 1. p_stn_sts      (I)   局状態                        */
/*                  : 2. p_rslt_info    (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(応答送信不可)             */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_err ( char           *p_stn_sts    /* 局状態       */
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_SST_NOUPDATE;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            } else {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;             /* 管理ファイル更新なし */
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;         /* 管理ファイル更新あり */
            } else {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;             /* 管理ファイル更新なし */
        }
    }

    return ls_retcode;  /* 許可応答 */

} /* end of NWM_STE_cst_check_rsp_err */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_command                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_command()                    */
/*                              char*, char*, t_rcv_info_def*)              */
/*  ARGUMENT        : 1. p_stn_sts      (I)   局状態                        */
/*                  : 2. p_nwi_if       (I)   NW情報(インタフェース単位)    */
/*                  : 3. p_rslt_info    (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: コマンド受付可（電文送信あり）                     */
/*                  : 1: コマンド受付可（電文送信なし）                     */
/*                  : 2: コマンド受付不可                                   */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(コマンド)                 */
/****************************************************************************/
short  NWM_STE_cst_check_command ( char           *p_stn_sts    /* 局状態       */
                                 , char           *p_nwi_if     /* NW情報レコード(インタフェース単位)  */
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_CMD_NG;
    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_NOMAL, sizeof(p_rslt_info->naibu_errcd));

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {               /* 開局要求                           */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_NORMAL )  {            /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                       , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING       /*       ->11:開局処理                */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_FORCE )  {             /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING       /*       ->11:開局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {        /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN              /*       ->10:開局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                 /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                 sizeof(p_rslt_info->naibu_errcd));         /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {               /* 閉局                               */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_NORMAL )  {            /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLOSING      /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_FORCE )  {             /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLOSING      /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd)); /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {        /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS              /*       ->90:閉局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                 /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                 sizeof(p_rslt_info->naibu_errcd));         /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {               /* エコー                             */
        if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                           /*     開局状態                       */
                              , strlen(DEF_STTE_STS_OPN     )) == 0 )|| 
            ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                           /*     閉局状態                       */
                               , strlen(DEF_STTE_STS_CLS    )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                        /*     開局処理中                     */
                               , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                       /*     閉局処理中                     */
                               , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            ls_retcode = NWM_STE_CMD_OK_SEND;                            /*       ->0:コマンド受付可(送信あり) */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                 /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
        }
    }
    return ls_retcode;
} /* end of NWM_STE_cst_check_command */

