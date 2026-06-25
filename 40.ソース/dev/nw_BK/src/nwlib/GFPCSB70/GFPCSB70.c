/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSB70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(BANKNET) */
/*                               処理を行う                                  */
/*        AUTHER            ････ HAS T.Sugisaki                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-07-02                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/07/02 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/* STANDARD HEADER */
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <stdbool.h> nolist
#include <string.h> nolist
#include <zsysc> nolist

/* USER HEADER     */
#include "ems.h"
#include "common.h"
#include "file.h"
#include "ipc.h"

#include "GFPOGGZ3_encode.h"
#include "GFPCGX50.h"                        /* システム日時取得             */
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVX80.h"                        /* 局状態・エコー制御サーバ     */

#include "msg_BK.h"                          /* 制御電文(BANKNET)            */
#include "GFPCSB70.h"
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
    char   lc_wbuf[64];

    msg_banknet_def      *lp_reqmsg;         /* 受信電文用P                 */

    fixedform_banknet_0800_def
                         *lp_mtidata;        /* 電文固定フォーマット        */

    lp_reqmsg         = (msg_banknet_def *)p_rcv_data;

/* ---------------------------------------------------------------------------------- */
/* 制御電文チェック                                                                   */
/* ---------------------------------------------------------------------------------- */

/* (a) MTI精査 */
    /* --- MTI -------------------- */
    EBCDIC2SJIS(lp_reqmsg->mti, lc_wbuf, sizeof(lp_reqmsg->mti));
    if ( memcmp( lc_wbuf, DEF_BK_MTI_0800_REQ, DEF_BK_MTI_LEN) != 0 ) {
        NWM_CTU_ERRORSET(DEF_CHK_ERR_MTI);
        return NWM_STE_SEISA_HAKI;
    }

/* (b) BITMAP部精査 */
    lp_mtidata = (fixedform_banknet_0800_def *)&lp_reqmsg->ffd;

    /* --- BIT02  ----------------- */
    if ( lp_mtidata->b002_pan.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_02);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT07  ----------------- */
    if ( lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_07);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT11 ----------------- */
    if ( lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_11);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    if ( lp_mtidata->b033_forwd_inst_id.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_33);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT63 ----------------- */
    if ( lp_mtidata->b063_network_data.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_63);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT70 ----------------- */
    if ( lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist != true ){
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_70);
        return NWM_STE_SEISA_HAKI;
    }

/* (c) データ部精査 */
    /* --- BIT02  ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b002_pan.ffd_data,
            lp_mtidata->b002_pan.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_02);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT07  ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy(lc_wbuf,"0000",4);
    memcpy(&lc_wbuf[4], lp_mtidata->b007_trans_date_time.ffd_data, DEF_BK_BIT007_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_07);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT11 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy(lc_wbuf, lp_mtidata->b011_system_audit_number.ffd_data, 
           lp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_11);
        return NWM_STE_SEISA_HAKI;
    }
    /* --- BIT33 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy(lc_wbuf, lp_mtidata->b033_forwd_inst_id.ffd_data,
           lp_mtidata->b033_forwd_inst_id.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_33);
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
    char   lc_wbuf[30];

    msg_banknet_def         *lp_rcvmsg;         /* 受信電文用P                 */
    msg_banknet_def         *lp_sreqmsg;        /* 仕向要求用P                 */
    fixedform_banknet_0810_def                  /* 電文固定フォーマット        */
                            *lp_mtidata;
    fixedform_banknet_0800_def                  /* 仕向電文固定フォーマット    */
                            *lp_smk_mtidata;

    lp_rcvmsg         = (msg_banknet_def *)p_rcv_data;
    lp_sreqmsg        = (msg_banknet_def *)p_snd_data;


/* ---------------------------------------------------------------------------------- */
/* 制御電文チェック                                                                   */
/* ---------------------------------------------------------------------------------- */

        /* --- MTI -------------------- */
        EBCDIC2SJIS(lp_rcvmsg->mti, lc_wbuf, sizeof(lp_rcvmsg->mti));
        if ( memcmp( lc_wbuf, DEF_BK_MTI_0810_RSP, DEF_BK_MTI_LEN) != 0 ) {
             NWM_CTU_ERRORSET(DEF_CHK_ERR_MTI);
            return NWM_STE_SEISA_HAKI;
        }

        lp_mtidata     = (fixedform_banknet_0810_def *)&lp_rcvmsg->ffd;
        lp_smk_mtidata = (fixedform_banknet_0800_def *)&lp_sreqmsg->ffd;

        /* --- BIT02 ----------------- */
        if ( lp_mtidata->b002_pan.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_02);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT07 ----------------- */
        if ( lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_07);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT11 ----------------- */
        if ( lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_11);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT33 ----------------- */
        if ( lp_mtidata->b033_forwd_inst_id.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_33);
            return NWM_STE_SEISA_HAKI;
        }

        if ( memcmp( lp_mtidata->b033_forwd_inst_id.ffd_data
                   , lp_smk_mtidata->b033_forwd_inst_id.ffd_data, DEF_BK_BIT033_DATA_LEN) != 0 ) {
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_33);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT39 ----------------- */
        if ( lp_mtidata->b039_response_code.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_39);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT63 ----------------- */
        if ( lp_mtidata->b063_network_data.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_63);
            return NWM_STE_SEISA_HAKI;
        }

        /* --- BIT70 ----------------- */
        if ( lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist != true ){
            NWM_CTU_ERRORSET(DEF_CHK_ERR_BIT_70);
            return NWM_STE_SEISA_HAKI;
        }

        if ( memcmp( lp_mtidata->b039_response_code.ffd_data
                   , DEF_RESP_CD_ALLOW_CHK      , strlen(DEF_RESP_CD_ALLOW_CHK)) != 0 ) {
            /* アクションコード(00以外) */
            /* 制御電文種別4桁目に"B"を設定 */
            p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
            return NWM_STE_SEISA_KYOHI;
        } else {
            /* アクションコード(00) */
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
    char                     lc_wbuf[100];
    COM_SDT_arg_2_def datetime_b;
    COM_SDT_arg_3_def datetime_c;
    long long         datetime_ll;
    msg_banknet_def         *lp_sreqmsg;        /* 仕向要求用P                           */
    db_gfnws_def            *lp_gfnws_cn;       /* 接続先固有情報NW用P(CONN単位)         */
    db_gfnws_def            *lp_gfnws_if;       /* 接続先固有情報NW用P(IF単位)           */
    nws_unq_info_bk_def     *lp_nws_banknet_cn; /* 接続先固有情報NW制御情報用P(CONN単位) */
    nws_unq_info_bk_def     *lp_nws_banknet_if; /* 接続先固有情報NW制御情報用P(IF単位)   */
    fixedform_banknet_0800_def                  /* データ部p                             */
                            *lp_mtidata;

    lp_sreqmsg = (msg_banknet_def *)p_rsp_data;

    lp_gfnws_cn       = (db_gfnws_def *)p_cn_info_cn;
    lp_nws_banknet_cn = (nws_unq_info_bk_def *)lp_gfnws_cn->dst_unq_info;
    lp_gfnws_if       = (db_gfnws_def *)p_cn_info_if;
    lp_nws_banknet_if = (nws_unq_info_bk_def *)lp_gfnws_if->dst_unq_info;

/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_sreqmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_banknet_0810_def));

    memcpy( p_rslt_info->mti, DEF_BK_MTI_0800_REQ, DEF_BK_MTI_LEN);
    SJIS2EBCDIC(DEF_BK_MTI_0800_REQ, lp_sreqmsg->mti, DEF_BK_MTI_LEN);

    lp_mtidata = (fixedform_banknet_0800_def *)&lp_sreqmsg->ffd;

    // BIT002
    lp_mtidata->b002_pan.ffd_header.m_flg_exist       = true;
    lp_mtidata->b002_pan.ffd_header.m_fixvalue_length = sizeof(lp_nws_banknet_cn->grp_signon_id);
    memcpy( lp_mtidata->b002_pan.ffd_data
          , lp_nws_banknet_cn->grp_signon_id, sizeof(lp_nws_banknet_cn->grp_signon_id));

    // BIT007
    /* 共通制御・送信日時 */
    COM_SDT ( DEF_COM_SDT_arg1_gmt, &datetime_b, &datetime_c, &datetime_ll);
    snprintf(lc_wbuf,sizeof(lc_wbuf),"%2.2s%2.2s%2.2s%2.2s%2.2s",
            datetime_b.mm,
            datetime_b.dd,
            datetime_b.hh,
            datetime_b.md,
            datetime_b.ss);

    lp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist           = true;
    lp_mtidata->b007_trans_date_time.ffd_header.m_fixvalue_length     = DEF_BK_BIT007_DATA_LEN;
    memcpy( lp_mtidata->b007_trans_date_time.ffd_data, lc_wbuf, DEF_BK_BIT007_DATA_LEN);

    // BIT011
    lp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_BK_BIT011_DATA_LEN;
    memcpy( lp_mtidata->b011_system_audit_number.ffd_data,
            p_rslt_info->sys_no, DEF_BK_BIT011_DATA_LEN);

    // BIT020
    memset(&lp_mtidata->b020_pan_country, ' ', sizeof(lp_mtidata->b020_pan_country));
    lp_mtidata->b020_pan_country.ffd_header.m_flg_exist       = false;
    lp_mtidata->b020_pan_country.ffd_header.m_fixvalue_length = 0;


    // BIT033
    lp_mtidata->b033_forwd_inst_id.ffd_header.m_flg_exist       = true;
    lp_mtidata->b033_forwd_inst_id.ffd_header.m_fixvalue_length = DEF_BK_BIT033_DATA_LEN;
    memcpy( lp_mtidata->b033_forwd_inst_id.ffd_data
          , lp_nws_banknet_if->send_src_code, DEF_BK_BIT033_DATA_LEN);

    // BIT053
    memset(&lp_mtidata->b053_security_ctl_info, ' ', sizeof(lp_mtidata->b053_security_ctl_info));
    lp_mtidata->b053_security_ctl_info.ffd_header.m_flg_exist       = false;
    lp_mtidata->b053_security_ctl_info.ffd_header.m_fixvalue_length = 0;

    // BIT063
    memset(&lp_mtidata->b063_network_data, ' ', sizeof(lp_mtidata->b011_system_audit_number));
    lp_mtidata->b063_network_data.ffd_header.m_flg_exist       = false;
    lp_mtidata->b063_network_data.ffd_header.m_fixvalue_length = 0;

    // BIT070
    lp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist       = true;
    lp_mtidata->b070_nw_mng_code.ffd_header.m_fixvalue_length = DEF_BK_BIT070_DATA_LEN;
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {          /* 開局 */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data, DEF_BK_F70_INFOCODE_061_SON  , DEF_BK_BIT070_DATA_LEN);
    } else if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data, DEF_BK_F70_INFOCODE_062_SOF  , DEF_BK_BIT070_DATA_LEN);
    } else if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND ) {   /* エコー */
        memcpy( lp_mtidata->b070_nw_mng_code.ffd_data, DEF_BK_F70_INFOCODE_270_ECH  , DEF_BK_BIT070_DATA_LEN);
    }

    // BIT094
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ||    /* 開局 */
         p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        lp_mtidata->b094_service_indicator.ffd_header.m_flg_exist       = true;
        lp_mtidata->b094_service_indicator.ffd_header.m_fixvalue_length = DEF_BK_BIT094_DATA_LEN;
        memcpy( lp_mtidata->b094_service_indicator.ffd_data,
                lp_nws_banknet_cn->service_indicator, DEF_BK_BIT094_DATA_LEN);
    } else {
        memset(&lp_mtidata->b094_service_indicator, ' ', sizeof(lp_mtidata->b094_service_indicator));
        lp_mtidata->b094_service_indicator.ffd_header.m_flg_exist       = false;
        lp_mtidata->b094_service_indicator.ffd_header.m_fixvalue_length = 0;
    }
    // BIT096
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ||    /* 開局 */
         p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist       = true;
        lp_mtidata->b096_msg_security_code.ffd_header.m_fixvalue_length = DEF_BK_BIT096_DATA_LEN;
        memcpy( lp_mtidata->b096_msg_security_code.ffd_data,
                lp_nws_banknet_cn->msg_sec_code, DEF_BK_BIT096_DATA_LEN);
    } else {
        memset(&lp_mtidata->b096_msg_security_code, ' ', sizeof(lp_mtidata->b094_service_indicator));
        lp_mtidata->b096_msg_security_code.ffd_header.m_flg_exist       = false;
        lp_mtidata->b096_msg_security_code.ffd_header.m_fixvalue_length = 0;
    }

    // BIT127
    memset(&lp_mtidata->b127_private_data, ' ', sizeof(lp_mtidata->b127_private_data));
    lp_mtidata->b127_private_data.ffd_header.m_flg_exist       = false;
    lp_mtidata->b127_private_data.ffd_header.m_fixvalue_length = 0;

    /* データ長算出 */
    p_rslt_info->denbun_len = DEF_BK_MTI_LEN
                            + sizeof(fixedform_banknet_0810_def);

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
                            , t_rcv_info_def       *p_rslt_info      /* 処理結果情報               */
                           , oggz1in_def           *p_cg010in        /* EMS出力共通情報            */
                           , ems_info_add          *p_ems_info_add ) /* EMS出力付加情報            */
{
    msg_banknet_def         *lp_rspmsg;      /* 送信電文用P                 */
    msg_banknet_def         *lp_rcvmsg;      /* 受信電文用P                 */
    fixedform_banknet_0810_def               /* 応答電文データ部p           */
                            *lp_rsp_mtidata;
    fixedform_banknet_0800_def               /* 受信電文データ部p           */
                            *lp_rcv_mtidata;

    lp_rspmsg         = (msg_banknet_def *)p_snd_data;
    lp_rcvmsg         = (msg_banknet_def *)p_rcv_data;

/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_rspmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_banknet_0800_def));

    memcpy( p_rslt_info->mti, DEF_BK_MTI_0810_RSP, DEF_BK_MTI_LEN);
    SJIS2EBCDIC(DEF_BK_MTI_0810_RSP, lp_rspmsg->mti, DEF_BK_MTI_LEN);

    lp_rsp_mtidata = (fixedform_banknet_0810_def *)&lp_rspmsg->ffd;
    lp_rcv_mtidata = (fixedform_banknet_0800_def *)&lp_rcvmsg->ffd;

    // BIT002
    lp_rsp_mtidata->b002_pan.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b002_pan.ffd_header.m_fixvalue_length = DEF_BK_BIT002_DATA_LEN;
    memcpy( lp_rsp_mtidata->b002_pan.ffd_data
          , lp_rcv_mtidata->b002_pan.ffd_data, DEF_BK_BIT002_DATA_LEN);
    // BIT007
    lp_rsp_mtidata->b007_trans_date_time.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b007_trans_date_time.ffd_header.m_fixvalue_length = DEF_BK_BIT007_DATA_LEN;
    memcpy( lp_rsp_mtidata->b007_trans_date_time.ffd_data
          , lp_rcv_mtidata->b007_trans_date_time.ffd_data, DEF_BK_BIT007_DATA_LEN);
    // BIT011
    lp_rsp_mtidata->b011_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b011_system_audit_number.ffd_header.m_fixvalue_length = DEF_BK_BIT011_DATA_LEN;
    memcpy( lp_rsp_mtidata->b011_system_audit_number.ffd_data
          , lp_rcv_mtidata->b011_system_audit_number.ffd_data, DEF_BK_BIT011_DATA_LEN);
    // BIT033
    lp_rsp_mtidata->b033_forwd_inst_id.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b033_forwd_inst_id.ffd_header.m_fixvalue_length = DEF_BK_BIT033_DATA_LEN;
    memcpy( lp_rsp_mtidata->b033_forwd_inst_id.ffd_data
          , lp_rcv_mtidata->b033_forwd_inst_id.ffd_data, DEF_BK_BIT033_DATA_LEN);
    // BIT039
    lp_rsp_mtidata->b039_response_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b039_response_code.ffd_header.m_fixvalue_length = DEF_BK_BIT039_DATA_LEN;
    memcpy( lp_rsp_mtidata->b039_response_code.ffd_data, "00", DEF_BK_BIT039_DATA_LEN);
    // BIT044
    memset(&lp_rsp_mtidata->b044_add_rsp_data, ' ', sizeof(lp_rsp_mtidata->b044_add_rsp_data));
    lp_rsp_mtidata->b044_add_rsp_data.ffd_header.m_flg_exist       = false;
    lp_rsp_mtidata->b044_add_rsp_data.ffd_header.m_fixvalue_length = 0;
    // BIT063
    lp_rsp_mtidata->b063_network_data.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b063_network_data.ffd_header.m_fixvalue_length = DEF_BK_BIT063_DATA_LEN;
    memcpy( lp_rsp_mtidata->b063_network_data.ffd_data
          , lp_rcv_mtidata->b063_network_data.ffd_data, DEF_BK_BIT063_DATA_LEN);
    // BIT070
    lp_rsp_mtidata->b070_nw_mng_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b070_nw_mng_code.ffd_header.m_fixvalue_length = DEF_BK_BIT070_DATA_LEN;
    memcpy( lp_rsp_mtidata->b070_nw_mng_code.ffd_data
          , lp_rcv_mtidata->b070_nw_mng_code.ffd_data, DEF_BK_BIT070_DATA_LEN);
    // BIT127
    memset(&lp_rsp_mtidata->b127_private_data, ' ', sizeof(lp_rsp_mtidata->b127_private_data));
    lp_rsp_mtidata->b127_private_data.ffd_header.m_flg_exist       = false;
    lp_rsp_mtidata->b127_private_data.ffd_header.m_fixvalue_length = 0;

    if ( memcmp( lp_rsp_mtidata->b039_response_code.ffd_data
               , DEF_RESP_CD_ALLOW_CHK      , sizeof(DEF_RESP_CD_ALLOW_CHK)) != 0 ) {
        /* 制御電文種別4桁目に"B"を設定 */
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_DENY;
        return DEF_RSP_TYPE_KYOHI;
    } else {
        /* 制御電文種別4桁目に"A"を設定 */
        p_rslt_info->ctrl_type[3] = DEF_CTLINT_ALLOW;
    }

    /* データ長算出 */
    p_rslt_info->denbun_len = DEF_BK_MTI_LEN
                            + sizeof(fixedform_banknet_0800_def);

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

    ls_retcode = NWM_STE_SST_KYOHI ;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {
        if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN, strlen(DEF_STTE_STS_OPN))         == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS))     == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING))  == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        }else{
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_ECHO
                                            , strlen(DEF_NERR_HSMK_SST_ECHO));
            ls_retcode = NWM_STE_SST_KYOHI;
        }
    }else{
        memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
        memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_ECHO
                                        , strlen(DEF_NERR_HSMK_SST_ECHO));
        ls_retcode = NWM_STE_SST_KYOHI;
    }

    return ls_retcode;  /* 許可応答 */

} /* end of NWM_STE_cst_check_req_rcv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_err                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_err(                     */
/*                                      char*, t_rcv_info_def*)             */
/*  ARGUMENT        : 1. p_stn_sts      (I)   局状態                        */
/*                  : 2. p_rsp_rslt     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(応答送信不可)             */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_err ( char           *p_stn_sts    /* 局状態       */
                                 , t_rcv_info_def *p_rsp_rslt ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode     = NWM_STE_SST_NOUPDATE ;

    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_ALLOW ) {

            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 ){
                // 開局 ->閉局
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
            ls_retcode = DEF_CST_CHK_NG;     /* 管理ファイル更新なし */
        }
    } else if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_ALLOW ) {
            if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 ) {
                // 開局 ->閉局
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
        }
    }

    return ls_retcode;
} /* end of NWM_STE_cst_check_rsp_rcv */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_rcv                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_rcv(                     */
/*                              char*, char*, char*, t_rcv_info_def*)       */
/*  ARGUMENT        : 1. p_stn_sts      (I)   局状態                        */
/*                  : 2. p_req_type     (I)   要求種別                      */
/*                  : 3. p_rcv_data     (I)   受信電文                      */
/*                  : 4. p_rsp_rslt     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(仕向応答)                 */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_rcv ( char           *p_stn_sts     /* 局状態       */
                                 , char           *p_req_type    /* 要求種別     */
                                 , char           *p_rcv_data    /* 受信電文     */
                                 , t_rcv_info_def *p_rsp_rslt )  /* 処理結果情報 */
{
    short  ls_retcode;
    short  naibu_rslt = DEF_CST_CHK_OK;  /* 仕向応答受信結果(内部) */

    ls_retcode     = NWM_STE_SST_NOUPDATE ;

    if ( memcmp( p_req_type, DEF_CTLREQ_SIMUKE, strlen(DEF_CTLREQ_SIMUKE)) == 0 ) {
        /* "20"：仕向応答 */
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            /* 許可応答 */
            /* 仕向応答 OK (内部) */
            naibu_rslt = DEF_CST_CHK_OK;
        } else {
            /* 拒否応答 */
            /* 仕向応答 NG (内部) */
            naibu_rslt = DEF_CST_CHK_NG;
        }
    } else {
        /* "30"：仕向応答タイムアウト  "40"：仕向要求送信不可 */
        /* 仕向応答 NG (内部) */
        naibu_rslt = DEF_CST_CHK_NG;
    }

    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if(naibu_rslt == DEF_CST_CHK_OK &&  memcmp(p_stn_sts, DEF_STTE_STS_OPNING, strlen(DEF_STTE_STS_OPNING)) == 0){
            memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPN , strlen(DEF_STTE_STS_OPN));
            ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
        }else if(naibu_rslt == DEF_CST_CHK_NG &&  memcmp(p_stn_sts, DEF_STTE_STS_OPNING, strlen(DEF_STTE_STS_OPNING)) == 0){
            memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
            ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
        }else{
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
        }
    } else if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if(naibu_rslt == DEF_CST_CHK_OK &&  memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0){
            memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
            ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
        }else if(naibu_rslt == DEF_CST_CHK_NG &&  memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0){
            memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
            ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
        }else{
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
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
/*                  : 3. p_rsp_rslt     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: コマンド受付可（電文送信あり）                     */
/*                  : 1: コマンド受付可（電文送信なし）                     */
/*                  : 2: コマンド受付不可                                   */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(コマンド)                 */
/****************************************************************************/
short  NWM_STE_cst_check_command ( char           *p_stn_sts    /* 局状態       */
                                 , char           *p_nwi_if     /* NW情報レコード(インタフェース単位)  */
                                 , t_rcv_info_def *p_rsp_rslt ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_CMD_NG;
    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_NOMAL, sizeof(p_rsp_rslt->naibu_errcd));

    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {                     /* 開局要求                           */
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_NORMAL )  {             /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                 /*     閉局状態                       */
                       , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理                */
                                               , DEF_SSTS_LEN);
            } else if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING             /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_CLSING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_FORCE )  {              /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_CLSING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {         /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPN               /*       ->10:開局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                 sizeof(p_rsp_rslt->naibu_errcd));          /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    } else
    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {                    /* 閉局                               */
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_NORMAL )  {             /*   オプション無し                   */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                                  , strlen(DEF_STTE_STS_OPN)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLOSING       /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_CLSING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_FORCE )  {              /*   強制実行                         */
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPN                       /*     開局状態                       */
                                   , strlen(DEF_STTE_STS_OPN)) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                       /*     閉局状態                       */
                                   , strlen(DEF_STTE_STS_CLS)) == 0 )) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLOSING       /*       ->91:閉局処理中              */
                                               , DEF_SSTS_LEN);
            } else
            if (( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                                   , strlen(DEF_STTE_STS_OPNING )) == 0 )||
                ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                                   , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
                if ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                 /*     開局処理中                     */
                                       , strlen(DEF_STTE_STS_OPNING )) == 0 ) {
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_HSMK_SST_CLSING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向閉局処理中)*/
                }
            }
        } else
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_UPDATEONLY )  {         /*   状態更新                         */
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                                  , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                               , DEF_SSTS_LEN);
            } else {
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                           /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS               /*       ->90:閉局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                 sizeof(p_rsp_rslt->naibu_errcd));          /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    } else
    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {                     /* エコー                             */
        if ((memcmp( p_stn_sts, DEF_STTE_STS_OPN                        /*     開局状態                       */
                              , strlen(DEF_STTE_STS_OPN)) == 0 )||
            (memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                              , strlen(DEF_STTE_STS_CLS)) == 0 )||
            (memcmp( p_stn_sts, DEF_STTE_STS_OPNING                    /*     開局処理中                     */
                               , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            (memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                   /*     閉局処理中                     */
                               , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            ls_retcode = NWM_STE_CMD_OK_SEND;                                /*       ->0:コマンド受付可(送信あり) */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                      /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
        } else {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                 sizeof(p_rsp_rslt->naibu_errcd));          /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    }

    return ls_retcode;
} /* end of NWM_STE_cst_check_command */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_char2hex                               */
/*  CALLING SEQ.    : short NWM_STE_char2hex      (const char     *         */
/*                                                 ,char           *        */
/*                                                 ,short           )       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.hex_p          (O)   変換先バッファ                 */
/*                  : 3.s_len          (I)   変換長                         */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 16進文字->BINARY変換を行う                            */
/****************************************************************************/
short NWM_STE_char2hex(char *ascii_p, char *hex_p, short s_len)
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
/*  DESCRIPTION     : BCDデータの数値チェックを行う                      */
/****************************************************************************/
short NWM_bcd_check(char *p_bcd_buf, short p_len)
{
    short  ls_idx;
    short  ls_target;

    /* 全体電文長                      */
    for(ls_idx=0; ls_idx < p_len; ls_idx++){
        /* BCDチェック */
        ls_target = (short)(p_bcd_buf[ls_idx] & 0x0f);
        if (ls_target > 9){
            return DEF_RET_NG;
        }

        ls_target = (short)((p_bcd_buf[ls_idx] >> 4) & 0x0f);
        if (ls_target > 9){
            return DEF_RET_NG;
        }
    }

    return DEF_RET_OK;
} /* end of NWM_bcd_check */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_bcd_to_char                                */
/*  CALLING SEQ.    : void   NWM_bcd_to_char                                */
/*  ARGUMENT        : char *p_bcd_buf   (I)   変換元バッファ                */
/*                  : char *char_buf    (O)   変換先バッファ                */
/*                  : short p_len       (I)   変換元レングス                */
/*  RETURN CODE     : 無し                                                  */
/*  DESCRIPTION     : BCDをcharに変換                                       */
/****************************************************************************/
void NWM_bcd_to_char(char *p_bcd_buf, char *p_char_buf, short p_len)
{
    short  ls_idx;
    short  ls_idx2;
    short  ls_ret1;
    short  ls_ret2;
    /* 全体電文長                      */
    for(ls_idx=0,ls_idx2=0; ls_idx < p_len; ls_idx++,ls_idx2++){
        /* 上位BIT 取得 */
        ls_ret1 = (short)((p_bcd_buf[ls_idx] >> 4) & 0x0f);
        ls_ret2 = (short)(p_bcd_buf[ls_idx] & 0x0f);
        sprintf(&p_char_buf[ls_idx2], "%x%x", ls_ret1, ls_ret2);
        /* BCDをそのまま文字として出力する        */
        /* (数値チェックせず、変換のみ実施)       */
        ls_idx2++;
    }
} /* end of NWM_bcd_check */

