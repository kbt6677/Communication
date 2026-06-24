/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(AEGN)    */
/*                               処理を行う                                  */
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
#include <zsysc>    nolist
#include <string.h> nolist

#include "common.h"
#include "file.h"
#include "ipc.h"
#include "ems.h"

#include "GFPCGX50.h"                        /* システム日時取得             */
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVX80.h"                        /* 局状態・エコー制御サーバ     */

#include "msg_AX.h"                          /* 制御電文(AEGN)               */
#include "GFPCSA70.h"
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

    msg_aegn_def      *lp_reqmsg;            /* 受信電文用P                 */

    fixedform_aegn_1804_def
                         *lp_mtidata;        /* 電文固定フォーマット        */

    lp_reqmsg         = (msg_aegn_def *)p_rcv_data;


/* ------------------------------------------------------------------------ */
/* 制御電文チェック                                                         */
/* ------------------------------------------------------------------------ */
/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if (memcmp (lp_reqmsg->mti , c_ax_mti_1804_req_ebc ,sizeof(c_ax_mti_1804_req_ebc)) !=0) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                     , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_HAKI;
    }
    lp_mtidata = (fixedform_aegn_1804_def *)&lp_reqmsg->ffd;

/* (b) BITMAP部精査 */
    /* --- BIT03 ----------------- */
    if ( lp_mtidata->b03_processing_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_03
                                   , strlen(DEF_CHK_ERR_BIT_03));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT11 ----------------- */
    if ( lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                     , strlen(DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT12 ----------------- */
    if ( lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                   , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT24 ----------------- */
    if ( lp_mtidata->b24_function_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_24
                                   , strlen(DEF_CHK_ERR_BIT_24));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    if ( lp_mtidata->b33_forwd_inst_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_33
                                   , strlen(DEF_CHK_ERR_BIT_33));
        return NWM_STE_SEISA_HAKI;
    }

/* (c) データ部精査 */
    /* --- BIT03 ----------------- */
    memcpy( lc_wbuf , DEF_PROC_CODE_000000 ,DEF_AX_BIT03_DATA_LEN);
    if ( memcmp (lp_mtidata->b03_processing_code.ffd_data , lc_wbuf ,DEF_AX_BIT03_DATA_LEN) != 0  ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_03
                                   , strlen(DEF_CHK_ERR_BIT_03));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT11 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b11_system_audit_number.ffd_data
                   , lp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                   , strlen(DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT12 ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '2';
    lc_wbuf[1] = '0';
    memcpy (&lc_wbuf[2], lp_mtidata->b12_local_tran_time.ffd_data, DEF_AX_BIT12_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                   , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b33_forwd_inst_id.ffd_data
                   , lp_mtidata->b33_forwd_inst_id.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_33
                                   , strlen(DEF_CHK_ERR_BIT_33));
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
    short  *wk_p     = NULL;
    char   lc_wbuf[30];

    msg_aegn_def         *lp_rcvmsg;            /* 受信電文用P                 */
    msg_aegn_def         *lp_sreqmsg;           /* 仕向要求用P                 */
    fixedform_aegn_1814_def                     /* 電文固定フォーマット        */
                            *lp_mtidata;
    fixedform_aegn_1804_def                     /* 仕向電文固定フォーマット    */
                            *lp_smk_mtidata;

    lp_rcvmsg         = (msg_aegn_def *)p_rcv_data;
    lp_sreqmsg        = (msg_aegn_def *)p_snd_data;

/* ------------------------------------------------------------------------ */
/* 制御電文チェック                                                         */
/* ------------------------------------------------------------------------ */
/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if (memcmp (lp_rcvmsg->mti , c_ax_mti_1814_rsp_ebc ,sizeof(c_ax_mti_1814_rsp_ebc)) !=0) {
         memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                      , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_HAKI;
    }

    lp_mtidata     = (fixedform_aegn_1814_def *)&lp_rcvmsg->ffd;
    lp_smk_mtidata = (fixedform_aegn_1804_def *)&lp_sreqmsg->ffd;

/* (b) BITMAP部精査 */
    /* --- BIT03 ----------------- */
    if ( lp_mtidata->b03_processing_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_03
                                   , strlen(DEF_CHK_ERR_BIT_03));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT11 ----------------- */
    if ( lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                     , strlen( DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT12 ----------------- */
    if ( lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                     , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT24 ----------------- */
    if ( lp_mtidata->b24_function_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_24
                                     , strlen(DEF_CHK_ERR_BIT_24));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    if ( lp_mtidata->b33_forwd_inst_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_33
                                   , strlen(DEF_CHK_ERR_BIT_33));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT39 ----------------- */
    if ( lp_mtidata->b39_action_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_39
                                   , strlen(DEF_CHK_ERR_BIT_39));
        return NWM_STE_SEISA_HAKI;
    }

/* (c) データ部精査 */
    /* --- BIT03 ----------------- */
    memcpy( lc_wbuf , DEF_PROC_CODE_000000 ,DEF_AX_BIT03_DATA_LEN);
    if ( memcmp (lp_mtidata->b03_processing_code.ffd_data , lc_wbuf ,DEF_AX_BIT03_DATA_LEN) != 0  ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_03
                                   , strlen(DEF_CHK_ERR_BIT_03));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    if ( memcmp( lp_mtidata->b33_forwd_inst_id.ffd_data
               , lp_smk_mtidata->b33_forwd_inst_id.ffd_data, DEF_AX_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_33
                                     , strlen(DEF_CHK_ERR_BIT_33));
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT39 ----------------- */
    if ( memcmp( lp_mtidata->b39_action_code.ffd_data
               , DEF_ACT_NORMAL      , sizeof(lp_mtidata->b39_action_code.ffd_data)) != 0 ) {
        /* アクションコード(800以外) */
        /* 制御電文種別4桁目に"B"を設定 */
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
        return NWM_STE_SEISA_KYOHI;
    } else {
        /* アクションコード(800) */
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
                            , t_rcv_info_def       *p_rslt_info      /* 処理結果情報                  */
                           , oggz1in_def           *p_cg010in        /* EMS出力共通情報               */
                           , ems_info_add          *p_ems_info_add ) /* EMS出力付加情報               */
{
    long long                ll_date_time;
    char                     lc_wbuf[100];

    msg_aegn_def         *lp_sreqmsg;           /* 仕向要求用P                 */
    db_gfnws_def            *lp_gfnws_if;       /* 接続先固有情報NW用P         */
    nws_unq_info_ax_def     *lp_nws_aegn_if;    /* 接続先固有情報NW制御情報用P */
    fixedform_aegn_1804_def                     /* データ部p                   */
                            *lp_mtidata;

    COM_SDT_arg_2_def        com_sdt_arg_2;     /* 日時取得用データ            */
    COM_SDT_arg_3_def        com_sdt_arg_3;     /* 日時取得用データ            */

    lp_sreqmsg = (msg_aegn_def *)p_rsp_data;

    lp_gfnws_if       = (db_gfnws_def *)p_cn_info_if;
    lp_nws_aegn_if    = (nws_unq_info_ax_def *)lp_gfnws_if->dst_unq_info;


/* ------------------------------------------------------------------------ */
/* 共通制御ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    /* 送信日時設定  */
    COM_SDT ( DEF_COM_SDT_arg1_jpn, &com_sdt_arg_2, &com_sdt_arg_3, &ll_date_time);
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_sreqmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_aegn_1804_def));

    memcpy( lp_sreqmsg->mti , c_ax_mti_1804_req_ebc, sizeof(c_ax_mti_1804_req_ebc));
    memcpy( p_rslt_info->mti, DEF_AX_MTI_1804_REQ, strlen(DEF_AX_MTI_1804_REQ));

    lp_mtidata = (fixedform_aegn_1804_def *)&lp_sreqmsg->ffd;

    lp_mtidata->b03_processing_code.ffd_header.m_flg_exist             = true;
    lp_mtidata->b03_processing_code.ffd_header.m_fixvalue_length       = DEF_AX_BIT03_DATA_LEN;
    memcpy( lp_mtidata->b03_processing_code.ffd_data
          , DEF_PROC_CODE_000000 , strlen(DEF_PROC_CODE_000000));

    lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length = DEF_AX_BIT11_DATA_LEN;
    memcpy( lp_mtidata->b11_system_audit_number.ffd_data
          , p_rslt_info->sys_no, DEF_AX_BIT11_DATA_LEN);

    lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist           = true;
    lp_mtidata->b12_local_tran_time.ffd_header.m_fixvalue_length     = DEF_AX_BIT12_DATA_LEN;

    memcpy( lp_mtidata->b12_local_tran_time.ffd_data
          ,&com_sdt_arg_2.yyyy[2]    , DEF_AX_BIT12_DATA_LEN);

    lp_mtidata->b24_function_code.ffd_header.m_flg_exist             = true;
    lp_mtidata->b24_function_code.ffd_header.m_fixvalue_length       = DEF_AX_BIT24_DATA_LEN;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {  /* 開局 */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_AX_F24_FUNCTION_801_OPN  , strlen(DEF_AX_F24_FUNCTION_801_OPN));
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_AX_F24_FUNCTION_802_CLS  , strlen(DEF_AX_F24_FUNCTION_802_CLS));
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND ) {  /* エコー */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_AX_F24_FUNCTION_831_ECH  , strlen(DEF_AX_F24_FUNCTION_831_ECH));
    }

    lp_mtidata->b33_forwd_inst_id.ffd_header.m_flg_exist             = true;
    lp_mtidata->b33_forwd_inst_id.ffd_header.m_fixvalue_length       = DEF_AX_CNTR_ID_LEN;
    memcpy( lp_mtidata->b33_forwd_inst_id.ffd_data
          , lp_nws_aegn_if->send_src_code, DEF_AX_SEND_SRC_CODE_LEN);

    lp_mtidata->b48_stand_in_param_info.ffd_header.m_flg_exist             = false;
    lp_mtidata->b48_stand_in_param_info.ffd_header.m_fixvalue_length       = 0;

    memset( lp_mtidata->b48_stand_in_param_info.ffd_data
          , DEF_BUF_SPACE , sizeof(lp_mtidata->b48_stand_in_param_info.ffd_data));

    /* データ長算出 */
    p_rslt_info->denbun_len = sizeof(lp_sreqmsg->mti)
                            + sizeof(fixedform_aegn_1804_def);

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
    msg_aegn_def         *lp_rspmsg;         /* 送信電文用P                 */
    msg_aegn_def         *lp_rcvmsg;         /* 受信電文用P                 */
    fixedform_aegn_1814_def                  /* 応答電文データ部p           */
                            *lp_rsp_mtidata;
    fixedform_aegn_1804_def                  /* 受信電文データ部p           */
                            *lp_rcv_mtidata;

    lp_rspmsg         = (msg_aegn_def *)p_snd_data;
    lp_rcvmsg         = (msg_aegn_def *)p_rcv_data;

/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_rspmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_aegn_1814_def));

    memcpy( lp_rspmsg->mti  , c_ax_mti_1814_rsp_ebc, sizeof(c_ax_mti_1814_rsp_ebc));
    memcpy( p_rslt_info->mti, DEF_AX_MTI_1814_RSP, strlen(DEF_AX_MTI_1814_RSP));

    lp_rsp_mtidata = (fixedform_aegn_1814_def *)&lp_rspmsg->ffd;
    lp_rcv_mtidata = (fixedform_aegn_1804_def *)&lp_rcvmsg->ffd;

    lp_rsp_mtidata->b03_processing_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b03_processing_code.ffd_header.m_fixvalue_length = DEF_AX_BIT03_DATA_LEN;
    memcpy( lp_rsp_mtidata->b03_processing_code.ffd_data
          , lp_rcv_mtidata->b03_processing_code.ffd_data, DEF_AX_BIT03_DATA_LEN);

    lp_rsp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length = DEF_AX_BIT11_DATA_LEN;
    memcpy( lp_rsp_mtidata->b11_system_audit_number.ffd_data
          , lp_rcv_mtidata->b11_system_audit_number.ffd_data, DEF_AX_BIT11_DATA_LEN);

    lp_rsp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b12_local_tran_time.ffd_header.m_fixvalue_length = DEF_AX_BIT12_DATA_LEN;
    memcpy( lp_rsp_mtidata->b12_local_tran_time.ffd_data
          , lp_rcv_mtidata->b12_local_tran_time.ffd_data   , DEF_AX_BIT12_DATA_LEN);

    lp_rsp_mtidata->b24_function_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b24_function_code.ffd_header.m_fixvalue_length = DEF_AX_BIT24_DATA_LEN;
    memcpy( lp_rsp_mtidata->b24_function_code.ffd_data
          , lp_rcv_mtidata->b24_function_code.ffd_data     , DEF_AX_BIT24_DATA_LEN);

    lp_rsp_mtidata->b33_forwd_inst_id.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b33_forwd_inst_id.ffd_header.m_fixvalue_length = DEF_AX_BIT33_DATA_LEN;
    memcpy( lp_rsp_mtidata->b33_forwd_inst_id.ffd_data
          , lp_rcv_mtidata->b33_forwd_inst_id.ffd_data     , DEF_AX_BIT33_DATA_LEN);

    lp_rsp_mtidata->b39_action_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b39_action_code.ffd_header.m_fixvalue_length = DEF_AX_BIT39_DATA_LEN;

    memcpy( lp_rsp_mtidata->b39_action_code.ffd_data
          , DEF_ACT_NORMAL , DEF_AX_BIT39_DATA_LEN);
    p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;

    /* データ長算出 */
    p_rslt_info->denbun_len = sizeof(lp_rspmsg->mti)
                            + sizeof(fixedform_aegn_1814_def);

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
        if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
            memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN , strlen(DEF_STTE_STS_OPN));
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 ){

            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_CLS,
                                 sizeof(p_rslt_info->naibu_errcd));  /*       局状態エラー(被仕向閉局)*/
            ls_retcode = NWM_STE_SST_HAKI;
        } else
        if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_OPNING,
                                 sizeof(p_rslt_info->naibu_errcd));  /*       局状態エラー(被仕向開局処理中)*/
            ls_retcode = NWM_STE_SST_HAKI;
        } else
        if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {

            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_CLSING,
                                 sizeof(p_rslt_info->naibu_errcd));  /*       局状態エラー(被仕向閉局処理中)*/
            ls_retcode = NWM_STE_SST_HAKI;
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {
        if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN, strlen(DEF_STTE_STS_OPN)        ) == 0 )||
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
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;
    short  naibu_rslt = DEF_CST_CHK_OK;  /* 仕向応答受信結果(内部) */

    ls_retcode     = NWM_STE_SST_NOUPDATE;

    if ( memcmp( p_req_type, DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == 0 ) {
        /* "20"：仕向応答 */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            /* 許可応答 */
            /* 仕向応答 OK (内部) */
            naibu_rslt = DEF_CST_CHK_OK;
        } else {
            /* 拒否応答 */
            /* 仕向応答 NG (内部) */
            naibu_rslt = DEF_CST_CHK_NG;
        }
    } else if ( memcmp( p_req_type, DEF_CTLREQ_TIMEOUT, strlen(DEF_CTLREQ_TIMEOUT)) == 0 ) {
        /* "30"：仕向応答タイムアウト */
            naibu_rslt = DEF_CST_CHK_NG_RETRYCHECK;
    } else {
        /* "40"：仕向要求送信不可 */
        /* 仕向応答 NG (内部) */
        naibu_rslt = DEF_CST_CHK_NG;
    }

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( naibu_rslt == DEF_CST_CHK_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        } else
        if ( naibu_rslt == DEF_CST_CHK_NG ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        } else {
            if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_REQUEST ) {
                if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN) ) == 0 ) {
                    memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_NOUPDATE_RETRY;     /* 管理ファイル更新なし（開局リトライ有り） */
                } else {
                    memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
                } 
            } else
            if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_CONNECT ) {
                if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN) ) == 0 ) {
                    memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_NOUPDATE_RETRY;     /* 管理ファイル更新なし（開局リトライ有り） */
                } else
                if ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                    memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_UPDATE_RETRY;     /* 管理ファイル更新あり（開局リトライ有り） */
                } else {
                    memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
                } 
            } else {
                if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                    memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
                } else {
                    memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                    ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
                }
            }
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {
        if ( naibu_rslt == DEF_CST_CHK_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        } else {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
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
short  NWM_STE_cst_check_rsp_err ( char           *p_stn_sts     /* 局状態       */
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_SST_NOUPDATE ;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
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

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {                     /* 開局要求                           */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_NORMAL )  {             /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                       , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理                */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_FORCE )  {              /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
                                         sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {         /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN               /*       ->10:開局                    */
                                               , DEF_SSTS_LEN);
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_REQUEST )  {      /*   自動開局(開局要求電文受信)       */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )  {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )    {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLS
                                   ,  sizeof(p_rslt_info->naibu_errcd));     /*       -> 局状態エラー(仕向閉局)    */
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                     /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )  {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING
                                    , sizeof(p_rslt_info->naibu_errcd));     /*       -> 局状態エラー(仕向開局処理中) */
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                    /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )    {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING
                                    , sizeof(p_rslt_info->naibu_errcd));     /*       -> 局状態エラー(仕向閉局処理中) */
            } 
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_AUTO_CONNECT )  {      /*   自動開局(コネクション確立)       */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )  {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )   {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                     /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 ){
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING
                                    , sizeof(p_rslt_info->naibu_errcd));     /*       -> 局状態エラー(仕向開局処理中) */
            } else
            if  ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE             /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING
                                    , sizeof(p_rslt_info->naibu_errcd));     /*       -> 局状態エラー(仕向閉局処理中) */
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                 /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR
                                , sizeof(p_rslt_info->naibu_errcd));        /*       -> 局状態エラー(仕向処理区分エラー) */
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {                    /* 閉局                               */
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_NORMAL )  {             /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLOSING       /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING
                                    , sizeof(p_rslt_info->naibu_errcd));    /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING
                                    , sizeof(p_rslt_info->naibu_errcd));    /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_FORCE )  {              /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLOSING       /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_OPNING
                                       , sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CLSING
                                       , sizeof(p_rslt_info->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rslt_info->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {         /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS               /*       ->90:閉局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                 /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR
                                , sizeof(p_rslt_info->naibu_errcd));        /*       -> 局状態エラー(仕向処理区分エラー) */
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {                     /* エコー                             */
        if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                           /*     開局状態                       */
                              , strlen(DEF_STTE_STS_OPN     )) == 0 )|| 
            ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                           /*     閉局状態                       */
                               , strlen(DEF_STTE_STS_CLS    )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                        /*     開局処理中                     */
                               , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                       /*     閉局処理中                     */
                               , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            ls_retcode = NWM_STE_CMD_OK_SEND;                            /*       ->0:コマンド受付可(送信あり) */
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
        }
    }
    return ls_retcode;
} /* end of NWM_STE_cst_check_command */

