/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(CARDNET) */
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

#include "msg_CA.h"                          /* 制御電文(CARDNET)            */
#include "GFPCSJ70.h"                        /* 局状態・エコー制御サーバ     */
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
/*  DESCRIPTION     : 開閉局・エコー要求電文精査                             */
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

    msg_cardnet_def      *lp_reqmsg;         /* 受信電文用P                 */

    db_gfnwi_def         *lp_gfnwi_if;       /* NW情報I/F用P                */
    nwi_unq_info_ca_def  *lp_nwi_cardnet_if; /* NW情報I/F制御情報用P        */
    db_gfnws_def         *lp_gfnws_nw;       /* 接続先固有情報NW用P         */
    nws_unq_info_ca_def  *lp_nws_cardnet_nw; /* 接続先固有情報NW制御情報用P */
    db_gfnws_def         *lp_gfnws_st;       /* 接続先固有情報ST用P         */
    nws_unq_info_ca_def  *lp_nws_cardnet_st; /* 接続先固有情報ST制御情報用P */
    fixedform_cardnet_1804_def
                         *lp_mtidata;        /* 電文固定フォーマット        */

    lp_reqmsg         = (msg_cardnet_def *)p_rcv_data;

    lp_gfnwi_if       = (db_gfnwi_def *)p_nw_info_if;
    lp_nwi_cardnet_if = (nwi_unq_info_ca_def *)lp_gfnwi_if->dst_unq_info;

    lp_gfnws_nw       = (db_gfnws_def *)p_cn_info_nw;
    lp_nws_cardnet_nw = (nws_unq_info_ca_def *)lp_gfnws_nw->dst_unq_info;

    lp_gfnws_st       = (db_gfnws_def *)p_cn_info_st;
    lp_nws_cardnet_st = (nws_unq_info_ca_def *)lp_gfnws_st->dst_unq_info;

/* -------------------------------------------------------------------------- */
/* 制御電文 共通制御ヘッダチェック                                            */
/* -------------------------------------------------------------------------- */
    /* 制御電文識別 → 値check */
    if (memcmp( lp_reqmsg->header.ctrl_hdr_type
              , DEF_CA_CMHD_TYPE_F1     , strlen(DEF_CA_CMHD_TYPE_F1)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CTRLHD_TYPE
                                     , strlen(DEF_CHK_ERR_CTRLHD_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 全体電文長 → Numericチェック */
    ls_result = NWM_bcd_check( lp_reqmsg->header.ctrl_msg_len, DEF_CA_CTRL_MSG_LEN );

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_TOTAL_LEN
                                     , strlen(DEF_CHK_ERR_TOTAL_LEN));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 差出センタID → センタID比較 */
    if (memcmp( lp_reqmsg->header.ctrl_src_id
              , lp_nws_cardnet_nw->dst_center_id, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_DST_CENT_ID
                                     , strlen(DEF_CHK_ERR_DST_CENT_ID));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 宛先センタID → センタID比較 */
    if (memcmp( lp_reqmsg->header.ctrl_dst_id
              , lp_nws_cardnet_st->src_center_id, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_SRC_CENT_ID
                                     , strlen(DEF_CHK_ERR_SRC_CENT_ID));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 送信日時:日付形式チェック */
    memset ( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_bcd_to_char ( lp_reqmsg->header.ctrl_snd_time
                    , lc_wbuf
                    , DEF_CA_SND_TIME_LEN);
    ls_result = CMIN_check_datetime( lc_wbuf ) ;

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_SEND_TIME
                                     , strlen(DEF_CHK_ERR_SEND_TIME));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* モードフラグ → 設定値check */
    if ( lp_nwi_cardnet_if->mode_flg == DEF_MODE_FLG_OFF  ) {
        if( lp_reqmsg->header.ctrl_mode_flg != DEF_CA_APHD_MODE_00_HONBAN ) {
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MODE_FLG
                                         , strlen(DEF_CHK_ERR_MODE_FLG));
            return NWM_STE_SEISA_SHOGAI;
        }
    } else
    if ( lp_nwi_cardnet_if->mode_flg == DEF_MODE_FLG_ON  ) {
        if( lp_reqmsg->header.ctrl_mode_flg != DEF_CA_APHD_MODE_10_TEST ) {
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MODE_FLG
                                       , strlen(DEF_CHK_ERR_MODE_FLG));
            return NWM_STE_SEISA_SHOGAI;
        }
    }

    /* ヘッダータイプ → 設定値check */
    if ( memcmp( lp_reqmsg->header.bh_hdr_type
               , DEF_CA_APHD_TYPE_A1, strlen(DEF_CA_APHD_TYPE_A1)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_HD_TYPE
                                   , strlen(DEF_CHK_ERR_HD_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 電文種別コード → 設定値check */
    if ( memcmp( lp_reqmsg->header.bh_msg_type
               , DEF_CA_APHD_MSGCODE_C804_REQ   , strlen(DEF_CA_APHD_MSGCODE_C804_REQ)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MSG_TYPE
                                     , strlen(DEF_CHK_ERR_MSG_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* カット対象日付:日付形式チェック */
    memset ( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_bcd_to_char ( lp_reqmsg->header.bh_cut_date
                    , lc_wbuf
                    , DEF_CA_CUT_DATE_LEN );
    ls_result = CMIN_check_datetime( lc_wbuf );

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CUT_DATE
                                     , strlen(DEF_CHK_ERR_CUT_DATE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* BODY部電文長 → Numericチェック */
    ls_result = NWM_bcd_check( lp_reqmsg->header.bh_body_len, DEF_CA_BODY_LEN);

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BODY_LEN
                                     , strlen(DEF_CHK_ERR_BODY_LEN));
        return NWM_STE_SEISA_SHOGAI;
    }

/* ------------------------------------------------------------------------ */
/* 制御電文チェック                                                         */
/* ------------------------------------------------------------------------ */
/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if ( memcmp( lp_reqmsg->mti, DEF_CA_MTI_1804_REQ, DEF_CA_MTI_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                     , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_SHOGAI;
    }

/* (b) BITMAP部精査 */
    lp_mtidata = (fixedform_cardnet_1804_def *)&lp_reqmsg->ffd;

    /* --- BIT11 ----------------- */
    if ( lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                     , strlen(DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT12 ----------------- */
    if ( lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                   , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT24 ----------------- */
    if ( lp_mtidata->b24_function_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_24
                                   , strlen(DEF_CHK_ERR_BIT_24));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT93 ----------------- */
    if ( lp_mtidata->b93_src_center_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_93
                                   , strlen(DEF_CHK_ERR_BIT_93));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT94 ----------------- */
    if ( lp_mtidata->b94_dst_center_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_94
                                   , strlen(DEF_CHK_ERR_BIT_94));
        return NWM_STE_SEISA_SHOGAI;
    }

/* (c) データ部精査 */
    /* --- BIT11 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b11_system_audit_number.ffd_data
                   , lp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                   , strlen(DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT12 ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '2';
    lc_wbuf[1] = '0';
    memcpy (&lc_wbuf[2], lp_mtidata->b12_local_tran_time.ffd_data, DEF_CA_BIT12_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                   , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_SHOGAI;
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
/*  DESCRIPTION     : 開閉局・エコー応答電文精査                            */
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
    char   lc_wbuf[30];

    msg_cardnet_def         *lp_rcvmsg;         /* 受信電文用P                 */
    msg_cardnet_def         *lp_sreqmsg;        /* 仕向要求用P                 */
    db_gfnwi_def            *lp_gfnwi_if;       /* NW情報I/F用P                */
    nwi_unq_info_ca_def     *lp_nwi_cardnet_if; /* NW情報I/F制御情報用P        */
    db_gfnws_def            *lp_gfnws_nw;       /* 接続先固有情報NW用P         */
    nws_unq_info_ca_def     *lp_nws_cardnet_nw; /* 接続先固有情報NW制御情報用P */
    db_gfnws_def            *lp_gfnws_st;       /* 接続先固有情報ST用P         */
    nws_unq_info_ca_def     *lp_nws_cardnet_st; /* 接続先固有情報ST制御情報用P */
    fixedform_cardnet_1814_def                  /* 電文固定フォーマット        */
                            *lp_mtidata;
    fixedform_cardnet_1804_def                  /* 仕向電文固定フォーマット    */
                            *lp_smk_mtidata;

    lp_rcvmsg         = (msg_cardnet_def *)p_rcv_data;
    lp_sreqmsg        = (msg_cardnet_def *)p_snd_data;

    lp_gfnwi_if       = (db_gfnwi_def *)p_nw_info_if;
    lp_nwi_cardnet_if = (nwi_unq_info_ca_def *)lp_gfnwi_if->dst_unq_info;

    lp_gfnws_nw       = (db_gfnws_def *)p_cn_info_nw;
    lp_nws_cardnet_nw = (nws_unq_info_ca_def *)lp_gfnws_nw->dst_unq_info;

    lp_gfnws_st       = (db_gfnws_def *)p_cn_info_st;
    lp_nws_cardnet_st = (nws_unq_info_ca_def *)lp_gfnws_st->dst_unq_info;

/* -------------------------------------------------------------------------- */
/* 制御電文 共通制御ヘッダチェック                                            */
/* -------------------------------------------------------------------------- */
    /* 制御電文識別 → 値check */
    if ( memcmp( lp_rcvmsg->header.ctrl_hdr_type, DEF_CA_CMHD_TYPE_F1
                                                , strlen(DEF_CA_CMHD_TYPE_F1)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CTRLHD_TYPE
                                     , strlen(DEF_CHK_ERR_CTRLHD_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 全体電文長 → Numericチェック */
    ls_result = NWM_bcd_check( lp_rcvmsg->header.ctrl_msg_len
                             , DEF_CA_CTRL_MSG_LEN);

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_TOTAL_LEN
                                     , strlen(DEF_CHK_ERR_TOTAL_LEN));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 差出センタID → センタID比較 */
    if ( memcmp(lp_rcvmsg->header.ctrl_src_id, lp_nws_cardnet_nw->dst_center_id, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_DST_CENT_ID
                                     , strlen(DEF_CHK_ERR_DST_CENT_ID));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 宛先センタID → センタID比較 */
    if ( memcmp(lp_rcvmsg->header.ctrl_dst_id, lp_nws_cardnet_st->src_center_id, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_SRC_CENT_ID
                                     , strlen(DEF_CHK_ERR_SRC_CENT_ID));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 加盟店契約会社コード → センタID比較 */
    if ( memcmp(lp_rcvmsg->header.ctrl_merch_code, lp_sreqmsg->header.ctrl_merch_code
                                                 , DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CTL_MERCH_CD
                                     , strlen(DEF_CHK_ERR_CTL_MERCH_CD));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 送信日時:日付形式チェック */
    memset ( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_bcd_to_char ( lp_rcvmsg->header.ctrl_snd_time
                    , lc_wbuf
                    , DEF_CA_SND_TIME_LEN);
    ls_result = CMIN_check_datetime( lc_wbuf ) ;

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_SEND_TIME
                                     , strlen(DEF_CHK_ERR_SEND_TIME));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* モードフラグ → 設定値check */
    if ( lp_nwi_cardnet_if->mode_flg == DEF_MODE_FLG_OFF  ) {
        if( lp_rcvmsg->header.ctrl_mode_flg != DEF_CA_APHD_MODE_00_HONBAN ) {
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MODE_FLG
                                         , strlen(DEF_CHK_ERR_MODE_FLG));
            return NWM_STE_SEISA_SHOGAI;
        }
    } else
    if ( lp_nwi_cardnet_if->mode_flg == DEF_MODE_FLG_ON  ) {
        if( lp_rcvmsg->header.ctrl_mode_flg != DEF_CA_APHD_MODE_10_TEST ) {
            memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MODE_FLG
                                       , strlen(DEF_CHK_ERR_MODE_FLG));
            return NWM_STE_SEISA_SHOGAI;
        }
    }

    /* ヘッダタイプ → 設定値check */
    if ( memcmp( lp_rcvmsg->header.bh_hdr_type, DEF_CA_APHD_TYPE_A1
                                              , strlen(DEF_CA_APHD_TYPE_A1)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_HD_TYPE
                                     , strlen(DEF_CHK_ERR_HD_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 電文種別コード → 設定値check */
    if ( memcmp( lp_rcvmsg->header.bh_msg_type, DEF_CA_APHD_MSGCODE_C814_RSP
                                              , strlen(DEF_CA_APHD_MSGCODE_C814_RSP)) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MSG_TYPE
                                     , strlen(DEF_CHK_ERR_MSG_TYPE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* 仕向区分 → 仕向比較 */
    if ( lp_rcvmsg->header.bh_dst_type != lp_sreqmsg->header.bh_dst_type ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_SMK_KUBN
                                     , strlen(DEF_CHK_ERR_SMK_KUBN));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* カット対象日付:日付形式チェック */
    memset ( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_bcd_to_char ( lp_rcvmsg->header.bh_cut_date
                    , lc_wbuf
                    , DEF_CA_CUT_DATE_LEN);
    ls_result = CMIN_check_datetime( lc_wbuf ) ;

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CUT_DATE
                                     , strlen(DEF_CHK_ERR_CUT_DATE));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* BODY部電文長 → Numericチェック */
    ls_result = NWM_bcd_check( lp_rcvmsg->header.bh_body_len
                             , DEF_CA_BODY_LEN);

    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BODY_LEN
                                   , strlen(DEF_CHK_ERR_BODY_LEN));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* カードネット取引識別 → 仕向比較 */
   if ( memcmp( lp_rcvmsg->header.bh_cardnet_id
              , lp_sreqmsg->header.bh_cardnet_id, DEF_CA_CARDNET_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CDNT_ID
                                     , strlen(DEF_CHK_ERR_CDNT_ID));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* カードネット取引通番 → 仕向比較 */
    if ( memcmp( lp_rcvmsg->header.bh_cardnet_seq
               , lp_sreqmsg->header.bh_cardnet_seq, DEF_CA_CARDNET_SEQ_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CDNT_SEQ
                                     , strlen(DEF_CHK_ERR_CDNT_SEQ));
        return NWM_STE_SEISA_SHOGAI;
    }

    /* カードネット使用域 → 仕向比較 */
   if ( memcmp( lp_rcvmsg->header.bh_cardnet_area
              , lp_sreqmsg->header.bh_cardnet_area, DEF_CA_CARDNET_AREA_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_CDNT_AREA
                                     , strlen(DEF_CHK_ERR_CDNT_AREA));
        return NWM_STE_SEISA_SHOGAI;
    }

/* ------------------------------------------------------------------------ */
/* 制御電文チェック                                                         */
/* ------------------------------------------------------------------------ */
/* (a) MTI精査 */
    /* --- MTI -------------------- */
    if ( memcmp( lp_rcvmsg->mti, DEF_CA_MTI_1814_RSP, DEF_CA_MTI_LEN) != 0 ) {
         memcpy( p_rslt_info->err_area, DEF_CHK_ERR_MTI
                                      , strlen(DEF_CHK_ERR_MTI));
        return NWM_STE_SEISA_SHOGAI;
    }

/* (b) BITMAP部精査 */
    lp_mtidata     = (fixedform_cardnet_1814_def *)&lp_rcvmsg->ffd;
    lp_smk_mtidata = (fixedform_cardnet_1804_def *)&lp_sreqmsg->ffd;

    /* --- BIT11 ----------------- */
    if ( lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                     , strlen( DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT12 ----------------- */
    if ( lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                     , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT24 ----------------- */
    if ( lp_mtidata->b24_function_code.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_24
                                     , strlen(DEF_CHK_ERR_BIT_24));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT93 ----------------- */
    if ( lp_mtidata->b93_src_center_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_93
                                     , strlen(DEF_CHK_ERR_BIT_93));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT94 ----------------- */
    if ( lp_mtidata->b94_dst_center_id.ffd_header.m_flg_exist != true ){
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_94
                                     , strlen(DEF_CHK_ERR_BIT_94));
        return NWM_STE_SEISA_SHOGAI;
    }

/* (c) データ部精査 */
    /* --- BIT11 ----------------- */
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    memcpy( lc_wbuf, lp_mtidata->b11_system_audit_number.ffd_data
                   , lp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length);
    ls_result = CMIN_num_check ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_11
                                     , strlen(DEF_CHK_ERR_BIT_11));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT12 ----------------- */
    memset ( lc_wbuf , DEF_BUF_NULL, sizeof(lc_wbuf));
    lc_wbuf[0] = '2';
    lc_wbuf[1] = '0';
    memcpy (&lc_wbuf[2], lp_mtidata->b12_local_tran_time.ffd_data, DEF_CA_BIT12_DATA_LEN);
    ls_result = CMIN_check_datetime ( lc_wbuf );
    if ( ls_result != DEF_RET_OK ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_12
                                     , strlen(DEF_CHK_ERR_BIT_12));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT93 ----------------- */
    if ( memcmp( lp_mtidata->b93_src_center_id.ffd_data
               , lp_smk_mtidata->b93_src_center_id.ffd_data, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_93
                                     , strlen(DEF_CHK_ERR_BIT_93));
        return NWM_STE_SEISA_SHOGAI;
    }
    /* --- BIT94 ----------------- */
    if ( memcmp( lp_mtidata->b94_dst_center_id.ffd_data
               , lp_smk_mtidata->b94_dst_center_id.ffd_data, DEF_CA_CNTR_ID_LEN) != 0 ) {
        memcpy( p_rslt_info->err_area, DEF_CHK_ERR_BIT_94
                                     , strlen(DEF_CHK_ERR_BIT_94));
        return NWM_STE_SEISA_SHOGAI;
    }

    if ( memcmp( lp_mtidata->b39_action_code.ffd_data
               , DEF_ACT_NORMAL      , sizeof(DEF_ACT_NORMAL)) != 0 ) {
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
    short                    ls_result;
    long long                ll_date_time;
    char                     lc_wbuf[100];
    db_gccut_def             l_gccut;           /* カット対象日付管理F         */

    msg_cardnet_def         *lp_sreqmsg;        /* 仕向要求用P                 */
    db_gfnwi_def            *lp_gfnwi_if;       /* NW情報I/F用P                */
    nwi_unq_info_ca_def     *lp_nwi_cardnet_if; /* NW情報I/F制御情報用P        */
    db_gfnws_def            *lp_gfnws_nw;       /* 接続先固有情報NW用P         */
    nws_unq_info_ca_def     *lp_nws_cardnet_nw; /* 接続先固有情報NW制御情報用P */
    db_gfnws_def            *lp_gfnws_st;       /* 接続先固有情報ST用P         */
    nws_unq_info_ca_def     *lp_nws_cardnet_st; /* 接続先固有情報ST制御情報用P */
    fixedform_cardnet_1804_def                  /* データ部p                   */
                            *lp_mtidata;
    COM_SDT_arg_2_def        com_sdt_arg_2;     /* 日時取得用データ            */
    COM_SDT_arg_3_def        com_sdt_arg_3;     /* 日時取得用データ            */

    lp_sreqmsg = (msg_cardnet_def *)p_rsp_data;

    lp_gfnwi_if       = (db_gfnwi_def *)p_nw_info_if;
    lp_nwi_cardnet_if = (nwi_unq_info_ca_def *)lp_gfnwi_if->dst_unq_info;

    lp_gfnws_st       = (db_gfnws_def *)p_cn_info_st;
    lp_nws_cardnet_st = (nws_unq_info_ca_def *)lp_gfnws_st->dst_unq_info;

    lp_gfnws_nw       = (db_gfnws_def *)p_cn_info_nw;
    lp_nws_cardnet_nw = (nws_unq_info_ca_def *)lp_gfnws_nw->dst_unq_info;

    /* カット対象日付取得処理 */
    ls_result = NWM_CTU_GET( p_gccut_info
                           , (NWM_CTU_INI_arg_3_def *)p_con_lid
                           , p_cg010in->emsinf.emsgkinf.prgid
                           , p_cg010in
                           , (NWM_CTU_INI_arg_6_def *)p_ems_info_add
                           , (char *)&l_gccut );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

/* ------------------------------------------------------------------------ */
/* 共通制御ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    memcpy( lp_sreqmsg->header.ctrl_hdr_type  , DEF_CA_CMHD_TYPE_F1
                                              , strlen(DEF_CA_CMHD_TYPE_F1));
    memset( lp_sreqmsg->header.ctrl_msg_len   , DEF_BUF_NULL
                                              , DEF_CA_CTRL_MSG_LEN);
    memcpy( lp_sreqmsg->header.ctrl_src_id    , lp_nws_cardnet_st->src_center_id
                                              , DEF_CA_CNTR_ID_LEN);
    memcpy( lp_sreqmsg->header.ctrl_dst_id    , lp_nws_cardnet_nw->dst_center_id
                                              , DEF_CA_CNTR_ID_LEN);
    memset( lp_sreqmsg->header.ctrl_merch_code, DEF_BUF_CZERO
                                              , DEF_CA_CNTR_ID_LEN);
    /* 送信日時設定 BCD変換必要 */
    COM_SDT ( DEF_COM_SDT_arg1_jpn, &com_sdt_arg_2, &com_sdt_arg_3, &ll_date_time);
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_STE_char2hex( (char *)&com_sdt_arg_2, lc_wbuf, sizeof(COM_SDT_arg_2_def));
    memcpy( lp_sreqmsg->header.ctrl_snd_time  , lc_wbuf
                                              , DEF_CA_SND_TIME_LEN);

    if ( lp_nwi_cardnet_if->mode_flg == DEF_MODE_FLG_OFF  ) {
        lp_sreqmsg->header.ctrl_mode_flg = DEF_CA_APHD_MODE_00_HONBAN;
    } else {
        lp_sreqmsg->header.ctrl_mode_flg = DEF_CA_APHD_MODE_10_TEST;
    }

    memset( lp_sreqmsg->header.ctrl_filler    , DEF_BUF_SPACE
                                              , sizeof(lp_sreqmsg->header.ctrl_filler));
/* ------------------------------------------------------------------------ */
/* 業務共通ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    memcpy( lp_sreqmsg->header.bh_hdr_type
          , DEF_CA_APHD_TYPE_A1 , strlen(DEF_CA_APHD_TYPE_A1));
    memcpy( lp_sreqmsg->header.bh_msg_type
          , DEF_CA_APHD_MSGCODE_C804_REQ, sizeof(DEF_CA_APHD_MSGCODE_C804_REQ));
    memset( lp_sreqmsg->header.bh_auth_val
          , DEF_BUF_NULL     , DEF_CA_AUTH_VAL_LEN);
    memset( lp_sreqmsg->header.bh_chk_digit.bh_chk_digit_kc
          , DEF_BUF_NULL     , DEF_CA_DIGIT_KC_LEN);
    memset( lp_sreqmsg->header.bh_chk_digit.bh_chk_digit_kmac
          , DEF_BUF_NULL     , DEF_CA_DIGIT_KMAC_LEN);
    lp_sreqmsg->header.bh_dst_type = DEF_BUF_SPACE;
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_STE_char2hex( l_gccut.cut_date_info.cut_date, lc_wbuf, sizeof(l_gccut.cut_date_info.cut_date));
    memcpy( lp_sreqmsg->header.bh_cut_date    , lc_wbuf      , DEF_CA_CUT_DATE_LEN);
    memset( lp_sreqmsg->header.bh_body_len    , DEF_BUF_NULL , DEF_CA_BODY_LEN);
    memset( lp_sreqmsg->header.bh_cardnet_id  , DEF_BUF_NULL , DEF_CA_CARDNET_ID_LEN);
    memset( lp_sreqmsg->header.bh_cardnet_seq , DEF_BUF_NULL , DEF_CA_CARDNET_SEQ_LEN);
    memset( lp_sreqmsg->header.bh_cardnet_area, DEF_BUF_NULL , DEF_CA_CARDNET_AREA_LEN);
    memset( lp_sreqmsg->header.bh_filler      , DEF_BUF_SPACE, sizeof(lp_sreqmsg->header.bh_filler));
/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_sreqmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_cardnet_1804_def));

    memcpy( lp_sreqmsg->mti , DEF_CA_MTI_1804_REQ, DEF_CA_MTI_LEN);
    memcpy( p_rslt_info->mti, DEF_CA_MTI_1804_REQ, DEF_CA_MTI_LEN);

    lp_mtidata = (fixedform_cardnet_1804_def *)&lp_sreqmsg->ffd;

    lp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length = DEF_CA_BIT11_DATA_LEN;
    memcpy( lp_mtidata->b11_system_audit_number.ffd_data
          , p_rslt_info->sys_no, DEF_CA_BIT11_DATA_LEN);

    lp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist           = true;
    lp_mtidata->b12_local_tran_time.ffd_header.m_fixvalue_length     = DEF_CA_BIT12_DATA_LEN;

    memcpy( lp_mtidata->b12_local_tran_time.ffd_data
          ,&com_sdt_arg_2.yyyy[2]    , DEF_CA_BIT12_DATA_LEN);

    lp_mtidata->b24_function_code.ffd_header.m_flg_exist             = true;
    lp_mtidata->b24_function_code.ffd_header.m_fixvalue_length       = DEF_CA_BIT24_DATA_LEN;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN ) {  /* 開局 */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_CA_F24_FUNCTION_801_OPN  , strlen(DEF_CA_F24_FUNCTION_801_OPN));
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {  /* 閉局 */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_CA_F24_FUNCTION_802_CLS  , strlen(DEF_CA_F24_FUNCTION_802_CLS));
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND ) {  /* エコー */
        memcpy( lp_mtidata->b24_function_code.ffd_data
              , DEF_CA_F24_FUNCTION_831_ECH  , strlen(DEF_CA_F24_FUNCTION_831_ECH));
    }

    lp_mtidata->b93_src_center_id.ffd_header.m_flg_exist             = true;
    lp_mtidata->b93_src_center_id.ffd_header.m_fixvalue_length       = DEF_CA_CNTR_ID_LEN;
    memcpy( lp_mtidata->b93_src_center_id.ffd_data
          , lp_nws_cardnet_nw->dst_center_id, DEF_CA_CNTR_ID_LEN);

    lp_mtidata->b94_dst_center_id.ffd_header.m_flg_exist             = true;
    lp_mtidata->b94_dst_center_id.ffd_header.m_fixvalue_length       = DEF_CA_CNTR_ID_LEN;
    memcpy( lp_mtidata->b94_dst_center_id.ffd_data
          , lp_nws_cardnet_st->src_center_id, DEF_CA_CNTR_ID_LEN);

    /* データ長算出 */
    p_rslt_info->denbun_len = sizeof(MSG_HEADER_CARDNET_def)
                            + DEF_CA_MTI_LEN
                            + sizeof(fixedform_cardnet_1804_def);

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
    short                    ls_result;
    long long                ll_date_time;
    char                     lc_wbuf[100];
    db_gccut_def             l_gccut;        /* カット対象日付管理F         */
    NWM_CTU_INI_arg_3_def    l_conlid;

    msg_cardnet_def         *lp_rspmsg;      /* 送信電文用P                 */
    msg_cardnet_def         *lp_rcvmsg;      /* 受信電文用P                 */
    fixedform_cardnet_1814_def               /* 応答電文データ部p           */
                            *lp_rsp_mtidata;
    fixedform_cardnet_1804_def               /* 受信電文データ部p           */
                            *lp_rcv_mtidata;

    COM_SDT_arg_2_def        com_sdt_arg_2;     /* 日時取得用データ            */
    COM_SDT_arg_3_def        com_sdt_arg_3;     /* 日時取得用データ            */

    lp_rspmsg         = (msg_cardnet_def *)p_snd_data;
    lp_rcvmsg         = (msg_cardnet_def *)p_rcv_data;

    memset(&l_conlid, DEF_BUF_NULL, sizeof(l_conlid));
    memcpy(&l_conlid, p_con_lid   , sizeof(l_conlid));
    memset( l_conlid.connect_id   , DEF_BUF_SPACE
                                  , sizeof(l_conlid.connect_id));
    /* カット対象日付取得処理 */
    ls_result = NWM_CTU_GET( p_gccut_info
                           ,&l_conlid
                           , p_cg010in->emsinf.emsgkinf.prgid
                           , p_cg010in
                           , (NWM_CTU_INI_arg_6_def *)p_ems_info_add
                           , (char *)&l_gccut );

    if ( ls_result != DEF_RET_OK ) {
        return DEF_RET_NG;
    }

/* ------------------------------------------------------------------------ */
/* 共通制御ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    memcpy( lp_rspmsg->header.ctrl_hdr_type
          , DEF_CA_CMHD_TYPE_F1              , sizeof(lp_rspmsg->header.ctrl_hdr_type));
    memset( lp_rspmsg->header.ctrl_msg_len
          , DEF_BUF_NULL                     , sizeof(lp_rspmsg->header.ctrl_msg_len));
    memcpy( lp_rspmsg->header.ctrl_src_id
          , lp_rcvmsg->header.ctrl_dst_id    , sizeof(lp_rspmsg->header.ctrl_src_id));
    memcpy( lp_rspmsg->header.ctrl_dst_id
          , lp_rcvmsg->header.ctrl_src_id    , sizeof(lp_rspmsg->header.ctrl_dst_id));
    memset( lp_rspmsg->header.ctrl_merch_code
          , DEF_BUF_CZERO                    , sizeof(lp_rspmsg->header.ctrl_merch_code));
    /* 送信日時設定 */
    memset ((char *)&com_sdt_arg_2, DEF_BUF_NULL   , sizeof(com_sdt_arg_2));
    memset ((char *)&com_sdt_arg_3, DEF_BUF_NULL   , sizeof(com_sdt_arg_3));
    COM_SDT( DEF_COM_SDT_arg1_jpn, &com_sdt_arg_2, &com_sdt_arg_3, &ll_date_time);

    lp_rspmsg->header.ctrl_mode_flg = lp_rcvmsg->header.ctrl_mode_flg;

    memset( lp_rspmsg->header.ctrl_filler
          , DEF_BUF_SPACE                    , sizeof(lp_rspmsg->header.ctrl_filler));
/* ------------------------------------------------------------------------ */
/* 業務共通ヘッダ編集                                                       */
/* ------------------------------------------------------------------------ */
    memcpy( lp_rspmsg->header.bh_hdr_type    , DEF_CA_APHD_TYPE_A1 , strlen(DEF_CA_APHD_TYPE_A1));
    memcpy( lp_rspmsg->header.bh_msg_type    , DEF_CA_APHD_MSGCODE_C814_RSP, strlen(DEF_CA_APHD_MSGCODE_C814_RSP));
    memset( lp_rspmsg->header.bh_auth_val    , DEF_BUF_NULL     , DEF_CA_AUTH_VAL_LEN);
    memset( lp_rspmsg->header.bh_chk_digit.bh_chk_digit_kc
                                             , DEF_BUF_NULL     , DEF_CA_DIGIT_KC_LEN);
    memset( lp_rspmsg->header.bh_chk_digit.bh_chk_digit_kmac
                                             , DEF_BUF_NULL     , DEF_CA_DIGIT_KMAC_LEN);
    lp_rspmsg->header.bh_dst_type = lp_rcvmsg->header.bh_dst_type;
    memset( lc_wbuf, DEF_BUF_NULL, sizeof(lc_wbuf));
    NWM_STE_char2hex( l_gccut.cut_date_info.cut_date
                    , lc_wbuf
                    , sizeof(l_gccut.cut_date_info.cut_date));
    memcpy( lp_rspmsg->header.bh_cut_date    , lc_wbuf          , DEF_CA_CUT_DATE_LEN);
    memset( lp_rspmsg->header.bh_body_len    , DEF_BUF_NULL     , DEF_CA_BODY_LEN);
    memcpy( lp_rspmsg->header.bh_cardnet_id
          , lp_rcvmsg->header.bh_cardnet_id  , DEF_CA_CARDNET_ID_LEN);
    memcpy( lp_rspmsg->header.bh_cardnet_seq
          , lp_rcvmsg->header.bh_cardnet_seq , DEF_CA_CARDNET_SEQ_LEN);
    memcpy( lp_rspmsg->header.bh_cardnet_area
          , lp_rcvmsg->header.bh_cardnet_area, DEF_CA_CARDNET_AREA_LEN);
    memset( lp_rcvmsg->header.bh_filler
          , DEF_BUF_SPACE                    , sizeof(lp_rcvmsg->header.bh_filler));

/* ------------------------------------------------------------------------ */
/* 制御電文データ部編集                                                     */
/* ------------------------------------------------------------------------ */
    memset( (char *)&lp_rspmsg->ffd, DEF_BUF_NULL, sizeof(fixedform_cardnet_1814_def));

    memcpy( lp_rspmsg->mti  , DEF_CA_MTI_1814_RSP, DEF_CA_MTI_LEN);
    memcpy( p_rslt_info->mti, DEF_CA_MTI_1814_RSP, DEF_CA_MTI_LEN);

    lp_rsp_mtidata = (fixedform_cardnet_1814_def *)&lp_rspmsg->ffd;
    lp_rcv_mtidata = (fixedform_cardnet_1804_def *)&lp_rcvmsg->ffd;

    lp_rsp_mtidata->b11_system_audit_number.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b11_system_audit_number.ffd_header.m_fixvalue_length = DEF_CA_BIT11_DATA_LEN;
    memcpy( lp_rsp_mtidata->b11_system_audit_number.ffd_data
          , lp_rcv_mtidata->b11_system_audit_number.ffd_data, DEF_CA_BIT11_DATA_LEN);

    lp_rsp_mtidata->b12_local_tran_time.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b12_local_tran_time.ffd_header.m_fixvalue_length = DEF_CA_BIT12_DATA_LEN;
    memcpy( lp_rsp_mtidata->b12_local_tran_time.ffd_data
          , lp_rcv_mtidata->b12_local_tran_time.ffd_data   , DEF_CA_BIT12_DATA_LEN);

    lp_rsp_mtidata->b24_function_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b24_function_code.ffd_header.m_fixvalue_length = DEF_CA_BIT24_DATA_LEN;
    memcpy( lp_rsp_mtidata->b24_function_code.ffd_data
          , lp_rcv_mtidata->b24_function_code.ffd_data     , DEF_CA_BIT24_DATA_LEN);

    lp_rsp_mtidata->b39_action_code.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b39_action_code.ffd_header.m_fixvalue_length = DEF_CA_BIT39_DATA_LEN;

    if ( p_rslt_info->rsp_result == NWM_STE_SEISA_NORMAL ) {
        memcpy( lp_rsp_mtidata->b39_action_code.ffd_data
              , DEF_ACT_NORMAL , DEF_CA_BIT39_DATA_LEN);
    } else {
        memcpy( lp_rsp_mtidata->b39_action_code.ffd_data
              , DEF_ACT_STS_ERR, DEF_CA_BIT39_DATA_LEN);
    }

    lp_rsp_mtidata->b93_src_center_id.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b93_src_center_id.ffd_header.m_fixvalue_length = DEF_CA_CNTR_ID_LEN;
    memcpy( lp_rsp_mtidata->b93_src_center_id.ffd_data
          , lp_rcv_mtidata->b93_src_center_id.ffd_data, DEF_CA_CNTR_ID_LEN);

    lp_rsp_mtidata->b94_dst_center_id.ffd_header.m_flg_exist       = true;
    lp_rsp_mtidata->b94_dst_center_id.ffd_header.m_fixvalue_length = DEF_CA_CNTR_ID_LEN;
    memcpy( lp_rsp_mtidata->b94_dst_center_id.ffd_data
          , lp_rcv_mtidata->b94_dst_center_id.ffd_data, DEF_CA_CNTR_ID_LEN);

    /* データ長算出 */
    p_rslt_info->denbun_len = sizeof(MSG_HEADER_CARDNET_def)
                            + DEF_CA_MTI_LEN
                            + sizeof(fixedform_cardnet_1814_def);

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
/*  DESCRIPTION     : 開閉局・エコー局状態チェック(要求)                     */
/*****************************************************************************/
short  NWM_STE_cst_check_req_rcv ( char           *p_stn_sts     /* 局状態       */
                                 , t_rcv_info_def *p_rslt_info ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_SST_KYOHI ;

    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if (( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)   ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING)) == 0 )) {
            memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_OPN , strlen(DEF_STTE_STS_OPN));
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_OPN
                                            , strlen(DEF_NERR_HSMK_SST_OPN));
            ls_retcode = NWM_STE_SST_KYOHI;
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            memcpy( p_rslt_info->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
            ls_retcode = NWM_STE_SST_KYOKA;
        }
    } else
    if ( p_rslt_info->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {
        if ( memcmp(p_stn_sts, DEF_STTE_STS_OPN, strlen(DEF_STTE_STS_OPN)) == 0 ) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_KYOKA;
        } else
        if (( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            memset( p_rslt_info->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            memcpy( p_rslt_info->naibu_errcd, DEF_NERR_HSMK_SST_ECHO
                                            , strlen(DEF_NERR_HSMK_SST_ECHO));
            ls_retcode = NWM_STE_SST_KYOHI;
        }
    }

    return ls_retcode;  /* 許可応答 */

} /* end of NWM_STE_cst_check_req_rcv */

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
/*  DESCRIPTION     : 開閉局・エコー局状態チェック(仕向応答)                */
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
        if ( naibu_rslt == DEF_CST_CHK_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPN , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        } else {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        }
    } else
    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_CLS ) {
        if ( naibu_rslt == DEF_CST_CHK_OK ) {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } 
        } else {
            if (( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_CLS    , strlen(DEF_STTE_STS_CLS)    ) == 0 )||
                ( memcmp(p_stn_sts, DEF_STTE_STS_OPNING , strlen(DEF_STTE_STS_OPNING) ) == 0 )) {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE    , DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            } else
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_CLOSING, strlen(DEF_STTE_STS_CLOSING)) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , DEF_SSTS_LEN);
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
/*                  : 2. p_rsp_rslt     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局・エコー局状態チェック(応答送信不可)            */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_err ( char           *p_stn_sts    /* 局状態       */
                                 , t_rcv_info_def *p_rsp_rslt ) /* 処理結果情報 */
{
    short  ls_retcode;

    ls_retcode = NWM_STE_SST_NOUPDATE ;

    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_OPN )  {
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
            ls_retcode = NWM_STE_SST_NOUPDATE;         /* 管理ファイル更新なし */
        }
    } else
    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_CNT_CLS )  {
        if ( p_rsp_rslt->ctrl_type[3] == DEF_CTLINT_ALLOW )  {
            if  ( memcmp(p_stn_sts, DEF_STTE_STS_OPN    , strlen(DEF_STTE_STS_OPN)   ) == 0 ) {
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS , strlen(DEF_STTE_STS_CLS));
                ls_retcode = NWM_STE_SST_UPDATE;     /* 管理ファイル更新あり */
            } else {
                memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
                ls_retcode = NWM_STE_SST_NOUPDATE;     /* 管理ファイル更新なし */
            }
        } else {
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE, DEF_SSTS_LEN);
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
/*                  : 3. p_rsp_rslt     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: コマンド受付可（電文送信あり）                     */
/*                  : 1: コマンド受付可（電文送信なし）                     */
/*                  : 2: コマンド受付不可                                   */
/*  DESCRIPTION     : 開閉局・エコー局状態チェック(コマンド)                */
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
            } else
            if ( memcmp( p_stn_sts, DEF_STTE_STS_CLS                        /*     閉局状態                       */
                       , strlen(DEF_STTE_STS_CLS)) == 0 ) {
                ls_retcode = NWM_STE_CMD_OK_SEND;                        /*       ->0:コマンド受付可(送信あり) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPNING        /*       ->11:開局処理                */
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
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
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
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
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
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_OPN           /*       ->10:開局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                     sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向処理区分エラー)*/
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
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
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
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_OPNING,
                                         sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向開局処理中)*/
                }
                else{
                    memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CLSING,
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
                ls_retcode = NWM_STE_CMD_OK_NOSEND;                       /*       ->1:コマンド受付可(送信なし) */
                memcpy( p_rsp_rslt->new_stn_sts, DEF_STTE_STS_CLS           /*       ->90:閉局                    */
                                               , DEF_SSTS_LEN);
            }
        } else {
            ls_retcode = NWM_STE_CMD_NG;                              /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE              /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
//          memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_CTLINT_ERR,
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SYSIF_LGC_ERR,
                                     sizeof(p_rsp_rslt->naibu_errcd));  /*       ->局状態エラー(仕向処理区分エラー)*/
        }
    } else
    if ( p_rsp_rslt->ctrl_type[2] == DEF_CTLTXT_ECH_SND )  {                     /* エコー                             */
        if ( memcmp( p_stn_sts, DEF_STTE_STS_OPN                            /*     開局状態                       */
                              , strlen(DEF_STTE_STS_OPN)) == 0 ) {
            ls_retcode = NWM_STE_CMD_OK_SEND;                            /*       ->0:コマンド受付可(送信あり) */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
        } else
        if (( memcmp( p_stn_sts, DEF_STTE_STS_CLS                           /*     閉局状態                       */
                               , strlen(DEF_STTE_STS_CLS    )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_OPNING                        /*     開局処理中                     */
                               , strlen(DEF_STTE_STS_OPNING )) == 0 )||
            ( memcmp( p_stn_sts, DEF_STTE_STS_CLOSING                       /*     閉局処理中                     */
                               , strlen(DEF_STTE_STS_CLOSING)) == 0 )) {
            ls_retcode = NWM_STE_CMD_NG;                                  /*       ->2:コマンド受付不可         */
            memset( p_rsp_rslt->new_stn_sts, DEF_BUF_SPACE                  /*       ->SPACE                      */
                                           , DEF_SSTS_LEN);
            memcpy( p_rsp_rslt->naibu_errcd, DEF_NERR_SMK_SST_ECHO,
                                 sizeof(p_rsp_rslt->naibu_errcd));          /*       ->局状態エラー(仕向エコー)   */
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
/*  DESCRIPTION     : BCDデータの数値チェックを行う                         */
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

