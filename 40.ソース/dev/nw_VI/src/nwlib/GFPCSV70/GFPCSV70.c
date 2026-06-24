/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSV70                                    */
/*        FUNCTION          ････ 局状態・エコー制御                          */
/*                               開局・閉局・エコー制御のNW個別処理(VISANET) */
/*                               処理を行う                                  */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-21                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS K.F    2025/04/21 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                            */
/*****************************************************************************/
#pragma ENV COMMON

/*****************************************************************************/
/*   INCLUDE定義                                                             */
/*****************************************************************************/
/* SYSTEM HEADER   */
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <stdbool.h> nolist

/* COMMON HEADER   */
#include "common.h"
#include "ems.h"
#include "file.h"

/* USER HEADER     */
#include "GFPCGX50.h"
#include "GFPCVXZ0.h"                        /* 制御電文共通メイン処理ヘッダ */
#include "GFPCVX80.h"                        /* 局状態・エコー制御サーバ     */

#include "msg_VI.h"
#include "GFPCSV70.h"
#include "vproc.h"

/*****************************************************************************/
/* ローカル処理                                                              */
/*****************************************************************************/
static short chk_numeric(int,char *);
static void  ASCIITOBCD(int,unsigned char *,unsigned char *);

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
short  NWM_STE_check_reqmsg ( char           *p_rcv_data        // 受信電文(要求電文)
                            , char           *p_rcv_len         // 受信電文長
                            , char           *p_nw_info_gp      // NW情報rec(Group)
                            , char           *p_nw_info_if      // NW情報rec(InterFace)
                            , char           *p_cn_info_nw      // 接続先固有情報rec(NetWork)
                            , char           *p_cn_info_if      // 接続先固有情報rec(InterFace)
                            , char           *p_cn_info_st      // 接続先固有情報rec(Station)
                            , char           *p_cn_info_cn      // 接続先固有情報rec(connection)
                            , gflin_pkey_def *p_con_lid         // コネクション論理ID
                            , t_rcv_info_def *p_rslt_info       // 処理結果情報
                            , char           *p_gcsst )         // 局状態管理Fileレコード
{
    msg_visanet_def               *head_p;
    fixedform_visanet_0800_def    *fixv_p;


    char              date[14+1];            // 日付情報格納域
    char              wkbuf[4+1];            //
    short             rtnvl;                 // 返却値取得域
    short             *wk_p     = NULL;      //
    short             *msglen_p = NULL;      //
    int               prmlen;                //

    /* 初期化                   */
    memset(date,0x00,sizeof(date));
    memset(wkbuf,0x00,sizeof(wkbuf));

    head_p = (msg_visanet_def *) p_rcv_data;
    fixv_p = (fixedform_visanet_0800_def *) &head_p->ffd;
    memcpy(wkbuf,p_rcv_len,sizeof(wkbuf)-1);
    prmlen = atoi(wkbuf);

    /**************************************************************************
    ** ヘッダ部チェック
    **************************************************************************/
    /* ヘッダ長チェック    */
    if (head_p->header.mh_hdr_len != DEF_MSG_HEADER_LEN) {
        // 処理結果情報設定追加 "Header_Length"
        memcpy(p_rslt_info->err_area,DEF_ERREMS_HEADLEN,\
                                                   strlen(DEF_ERREMS_HEADLEN));
        return(NWM_STE_SEISA_HAKI);
    }

    /* メッセージ長チェック*/
    msglen_p = (short *) head_p->header.mh_tot_len;
    if (*msglen_p != prmlen) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_MSGLEN,\
                                                    strlen(DEF_ERREMS_MSGLEN));
        return(NWM_STE_SEISA_HAKI);
    }

/* (a) MTI精査 */
    /* MTIチェック         */
    wk_p = (short *) head_p->mti;
    if (*wk_p != (short)DEF_VI_MTI_BCD_0800) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_MTICODE,\
                                                   strlen(DEF_ERREMS_MTICODE));
        return(NWM_STE_SEISA_HAKI);
    }

    /**************************************************************************
    ** データ部チェック
    **************************************************************************/
/* (b) BITMAP部精査 */
    /* B07 エレメントチェック */
    if (fixv_p->b07.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B07_FALSE,\
                                                 strlen(DEF_ERREMS_B07_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B11 エレメントチェック */
    if (fixv_p->b11.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B11_FALSE,\
                                                 strlen(DEF_ERREMS_B11_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B70 エレメントチェック */
    if (fixv_p->b70.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B70_FALSE,\
                                                 strlen(DEF_ERREMS_B70_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }

/* (c) データ部精査 */
    /* B07 エレメントチェック */
    /* 固定フォーマット情報の日付形式チェック       */
    memcpy(&date[0],"0000",4);
    memcpy(&date[4],fixv_p->b07.ffd_data,sizeof(fixv_p->b07.ffd_data));
    rtnvl = CMIN_check_datetime(date);
    if (rtnvl != DEF_RTN_CHKOK) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B07_DATEFRM,\
                                           strlen(DEF_ERREMS_B07_DATEFRM));
        return(NWM_STE_SEISA_HAKI);
    }

    /* B11 エレメントチェック */
    /* 固定フォーマット情報のニューメリックチェック */
    rtnvl = chk_numeric(sizeof(fixv_p->b11.ffd_data), fixv_p->b11.ffd_data);
    if (rtnvl != DEF_RTN_CHKOK) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B11_NUMERIC,\
                                           strlen(DEF_ERREMS_B11_NUMERIC));
        return(NWM_STE_SEISA_HAKI);
    }

    return(NWM_STE_SEISA_NORMAL);
}

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
short  NWM_STE_check_rspmsg ( char           *p_rcv_data        // 受信電文(応答電文)
                            , char           *p_rcv_len         // 受信電文長
                            , char           *p_snd_data        // 仕向要求電文
                            , char           *p_nw_info_gp      // NW情報rec(Group)
                            , char           *p_nw_info_if      // NW情報rec(InterFace)
                            , char           *p_cn_info_nw      // 接続先固有情報rec(NetWork)
                            , char           *p_cn_info_if      // 接続先固有情報rec(InterFace)
                            , char           *p_cn_info_st      // 接続先固有情報rec(Station)
                            , char           *p_cn_info_cn      // 接続先固有情報rec(connection)
                            , gflin_pkey_def *p_con_lid         // コネクション論理ID
                            , t_rcv_info_def *p_rslt_info )     // 処理結果情報
{
    msg_visanet_def               *rhead_p;
    fixedform_visanet_0810_def    *rfixv_p;
    msg_visanet_def               *shead_p;
    fixedform_visanet_0800_def    *sfixv_p;


    char              date[14+1];            // 日付情報格納域
    char              wkbuf[4+1];            //
    short             rtnvl;                 // 返却値取得域
    short             *wk_p     = NULL;      // 作業用short型ポインタ
    short             *msglen_p = NULL;      //
    int               prmlen;                //

    /* 初期化                   */
    memset(date,0x00,sizeof(date));
    memset(wkbuf,0x00,sizeof(wkbuf));

    rhead_p = (msg_visanet_def *) p_rcv_data;
    rfixv_p = (fixedform_visanet_0810_def *) &rhead_p->ffd;
    shead_p = (msg_visanet_def *) p_snd_data;
    sfixv_p = (fixedform_visanet_0800_def *) &shead_p->ffd;
    memcpy(wkbuf,p_rcv_len,sizeof(wkbuf)-1);
    prmlen = atoi(wkbuf);

    /**************************************************************************
    ** ヘッダ部チェック
    **************************************************************************/
    /* ヘッダ長チェック    */
    if (rhead_p->header.mh_hdr_len != DEF_MSG_HEADER_LEN) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_HEADLEN,\
                                                   strlen(DEF_ERREMS_HEADLEN));
        return(NWM_STE_SEISA_HAKI);
    }

    /* メッセージ長チェック*/
    msglen_p = (short *) rhead_p->header.mh_tot_len;
    if (*msglen_p != prmlen) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_MSGLEN,\
                                                    strlen(DEF_ERREMS_MSGLEN));
        return(NWM_STE_SEISA_HAKI);
    }

/* (a) MTI精査 */
    /* MTIチェック         */
    wk_p = (short *) rhead_p->mti;
    if (*wk_p != (short)DEF_VI_MTI_BCD_0810) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_MTICODE,\
                                                   strlen(DEF_ERREMS_MTICODE));
        return(NWM_STE_SEISA_HAKI);
    }

    /**************************************************************************
    ** データ部チェック
    **************************************************************************/
/* (b) BITMAP部精査 */
    /* B07 エレメントチェック */
    if (rfixv_p->b07.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B07_FALSE,\
                                                 strlen(DEF_ERREMS_B07_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B11 エレメントチェック */
    if (rfixv_p->b11.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B11_FALSE,\
                                                 strlen(DEF_ERREMS_B11_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B39 エレメントチェック */
    if (rfixv_p->b39.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B39_FALSE,\
                                                 strlen(DEF_ERREMS_B39_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B70 エレメントチェック */
    if (rfixv_p->b70.ffd_header.m_flg_exist != true) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B70_FALSE,\
                                                 strlen(DEF_ERREMS_B70_FALSE));
        return(NWM_STE_SEISA_HAKI);
    }

/* (c) データ部精査 */
    /* B07 エレメントチェック */
    /* 固定フォーマット情報の日付形式チェック       */
    memcpy(&date[0],"0000",4);
    memcpy(&date[4],rfixv_p->b07.ffd_data,sizeof(rfixv_p->b07.ffd_data));
    rtnvl = CMIN_check_datetime(date);
    if (rtnvl != DEF_RTN_CHKOK) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B07_DATEFRM,\
                                           strlen(DEF_ERREMS_B07_DATEFRM));
        return(NWM_STE_SEISA_HAKI);
    }
    /* B11 エレメントチェック */
    /* 固定フォーマット情報のニューメリックチェック */
    rtnvl = chk_numeric(sizeof(rfixv_p->b11.ffd_data),rfixv_p->b11.ffd_data);
    if (rtnvl != DEF_RTN_CHKOK) {
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B11_NUMERIC,\
                                           strlen(DEF_ERREMS_B11_NUMERIC));
        return(NWM_STE_SEISA_HAKI);
    }
    /* 要求電文の固定フォーマットとの一致確認       */
    if (memcmp(rfixv_p->b11.ffd_data,sfixv_p->b11.ffd_data,\
                                       sizeof(rfixv_p->b11.ffd_data)) != 0){
        memcpy(p_rslt_info->err_area,DEF_ERREMS_B11_REQMISSMATCH,\
                                       strlen(DEF_ERREMS_B11_REQMISSMATCH));
        return(NWM_STE_SEISA_HAKI);
    }

    /* 精査OKの場合           */
    if (memcmp(rfixv_p->b39.ffd_data,DEF_INSIDE_RSPCD,\
                                         sizeof(rfixv_p->b39.ffd_data)) == 0) {
       /* 制御電文種別4桁目に"A"を設定 */
       p_rslt_info->ctrl_type[3] = DEF_CTLTYP_STRA;
    } else{
       /* 制御電文種別4桁目に"B"を設定 */
       p_rslt_info->ctrl_type[3] = DEF_CTLTYP_STRB;
       return(NWM_STE_SEISA_KYOHI);
    }

    return(NWM_STE_SEISA_NORMAL);
}

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
short  NWM_STE_edit_reqmsg ( char                  *p_req_data       // 送信電文(要求電文)
                           , char                  *p_nw_info_gp     // NW情報rec(Group)
                           , char                  *p_nw_info_if     // NW情報rec(InterFace)
                           , char                  *p_cn_info_nw     // 接続先固有情報rec(NetWork)
                           , char                  *p_cn_info_if     // 接続先固有情報rec(InterFace)
                           , char                  *p_cn_info_st     // 接続先固有情報rec(Station)
                           , char                  *p_cn_info_cn     // 接続先固有情報rec(connection)
                           , gflin_pkey_def        *p_con_lid        // コネクション論理ID
                           , NWM_CTU_INI_arg_2_def *p_gccut_info     // カット対象日付管理情報
                           , t_rcv_info_def        *p_rslt_info      // 処理結果情報
                           , oggz1in_def           *p_ems_info_cmn   // EMS出力共通情報
                           , ems_info_add          *p_ems_info_add ) // EMS出力付加情報
{
    COM_SDT_arg_2_def             date_str;             // 日付取得領域（文字列形式）
    COM_SDT_arg_3_def             date_bin;             // 日付取得領域（バイナリ形式）
    db_gfnws_def                  *strec_p;             // ステーション情報レコード
    nws_unq_info_vi_def           *cnfix_p;             // 接続先固有情報ファイル
    long long                     ltimstmp;             // タイムスタンプ
    short                         rtnval;               // 返却値取得域
    char                          ymd[8+1];             // 年月日取得・設定域
    char                          day[3+1];             // 通算日取得域
    char                          sno[6+1];             //
    char                          wrk[18];              //
    char                          sid[6+1];             //
    char                          buf[20];              // 作業用バッファ

    char                          edt[2];               //

    msg_visanet_def               *head_p;
    fixedform_visanet_0800_def    *fixv_p;

    /* 初期化                   */
    ltimstmp = 0;
    memset(&date_str,0x00,sizeof(date_str));
    memset(wrk,0x00,sizeof(wrk));
    memset(buf,0x00,sizeof(buf));
    memset(ymd,0x00,sizeof(ymd));
    memset(day,0x00,sizeof(day));
    memset(edt,0x00,sizeof(edt));
    memset(sno,0x00,sizeof(sno));
    memset(sid,0x00,sizeof(sid));

    head_p  = (msg_visanet_def *) p_req_data;
    fixv_p  = (fixedform_visanet_0800_def *) &head_p->ffd;
    strec_p = (db_gfnws_def *)p_cn_info_st;
    cnfix_p = (nws_unq_info_vi_def *) strec_p->dst_unq_info;

    /* ステーションID 取得      */
    memcpy(sid,cnfix_p->visa_station_id,sizeof(cnfix_p->visa_station_id));

    /* BCD変換                  */
    ASCIITOBCD(6,sid,head_p->header.mh_src_id);


    /* ヘッダー設定             */
    memset((char *)&head_p->header.mh_hdr_len,0x00,sizeof(MSG_HEADER_VISA_def));
    head_p->header.mh_hdr_len     = 0x16;               // Header Length
    head_p->header.mh_hdr_flg_fmt = 0x01;               // Header Flag and Format
    head_p->header.mh_txt_fmt     = 0x02;               // Text Format
                                                        // ステーションID設定
    ASCIITOBCD(6,(unsigned char *)sid,(unsigned char *)head_p->header.mh_src_id);

                                                        // MTI
    head_p->mti[0] = (char)0x08;
    head_p->mti[1] = (char)0x00;
    memcpy(p_rslt_info->mti,DEF_VI_MTI_0800_REQ,sizeof(p_rslt_info->mti));

    /* システム日時取得(UTC)    */
    COM_SDT(DEF_UTC_DATE,&date_str,&date_bin,&ltimstmp);
    //COM_SDT(2,&date_str,&date_bin,&ltimstmp);
    sprintf(buf,"%02d%02d%02d%02d%02d",\
                 date_bin.mm,date_bin.dd,date_bin.hh,date_bin.md,date_bin.ss);

    /* 通算日取得               */
    sprintf(ymd,"%4d%2d%2d",date_bin.yyyy,date_bin.mm,date_bin.dd);
    rtnval = CMIN_get_day_of_year(ymd,day);
    if (rtnval != 0) {
        return(DEF_RTN_ERRCD);
    }

    /* データ部設定             */
    /*   B07 エレメント情報設定 */
    fixv_p->b07.ffd_header.m_flg_exist = true;
    fixv_p->b07.ffd_header.m_dmy1 = 0x20;
    fixv_p->b07.ffd_header.m_fixvalue_length = 10;
    memcpy(fixv_p->b07.ffd_data,buf,sizeof(fixv_p->b07.ffd_data));

    /*   B11 エレメント情報設定 */
    fixv_p->b11.ffd_header.m_flg_exist = true;
    fixv_p->b11.ffd_header.m_dmy1 = 0x20;
    fixv_p->b11.ffd_header.m_fixvalue_length = 6;
    memcpy(fixv_p->b11.ffd_data,p_rslt_info->sys_no,sizeof(fixv_p->b11.ffd_data));

    /*   B37 エレメント情報設定 */
    fixv_p->b37.ffd_header.m_flg_exist = true;
    fixv_p->b37.ffd_header.m_dmy1 = 0x20;
    fixv_p->b37.ffd_header.m_fixvalue_length = 12;
    edt[0] = date_str.yyyy[3];
    memcpy(sno,p_rslt_info->sys_no,sizeof(p_rslt_info->sys_no));
    sprintf(wrk,"%1s%3s%02d%6s",edt,day,date_bin.hh,sno);
    memcpy(fixv_p->b37.ffd_data,wrk,sizeof(fixv_p->b37.ffd_data));

    /*   B63 エレメント情報設定 */
    fixv_p->b63.ffd_header.m_flg_exist = false;
    fixv_p->b63.ffd_header.m_dmy1 = 0x20;
    fixv_p->b63.ffd_header.m_fixvalue_length = 0;
    fixv_p->b63.m_fiiller_1 = 0x20;
    memset(fixv_p->b63.ffd_data,0x20,sizeof(fixv_p->b63.ffd_data));

    /*     制御電文種別3桁目:"3"エコーの場合は以下のデータを上書き */
    if (*(p_rslt_info->ctrl_type + 2) == 0x33) {
        fixv_p->b63.ffd_header.m_flg_exist = true;
        fixv_p->b63.ffd_header.m_fixvalue_length = 5;
        memset(fixv_p->b63.ffd_data,0x00,sizeof(fixv_p->b63.ffd_data));
        fixv_p->b63.ffd_data[0] = 0x80;
        fixv_p->b63.ffd_data[1] = 0x00;
        fixv_p->b63.ffd_data[2] = 0x00;
        fixv_p->b63.ffd_data[3] = 0x00;
        fixv_p->b63.ffd_data[4] = 0x02;
    }

    /*   B70 エレメント情報設定 */
    fixv_p->b70.ffd_header.m_flg_exist = true;
    fixv_p->b70.ffd_header.m_dmy1 = 0x20;
    fixv_p->b70.ffd_header.m_fixvalue_length = 4;

    /* データ長算出 */
    p_rslt_info->denbun_len = (short) sizeof(MSG_HEADER_VISA_def) + \
                                      sizeof(head_p->mti) + \
                                      sizeof(fixedform_visanet_0800_def);

    switch (p_rslt_info->ctrl_type[2]) {
            case 0x31:
                {
                    memcpy(fixv_p->b70.ffd_data,DEF_VI_F70_INFOCODE_071_SON,sizeof(fixv_p->b70.ffd_data));
                }
                break;
            case 0x32:
                {
                    memcpy(fixv_p->b70.ffd_data,DEF_VI_F70_INFOCODE_072_SOF,sizeof(fixv_p->b70.ffd_data));
                }
                break;
            case 0x33:
                {
                    memcpy(fixv_p->b70.ffd_data,DEF_VI_F70_INFOCODE_301_ECH,sizeof(fixv_p->b70.ffd_data));
                }
                break;
            default:
                {
                    /* データ長クリア */
                    p_rslt_info->denbun_len = (short) 0;
                }
                break;
    }

    return(NWM_STE_EDIT_NORMAL);
}

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
short  NWM_STE_edit_rspmsg ( char                  *p_rsp_data       // 送信電文(応答電文)
                           , char                  *p_rcv_data       // 受信した被仕向け要求電文
                           , char                  *p_nw_info_gp     // NW情報rec(Group)
                           , char                  *p_nw_info_if     // NW情報rec(InterFace)
                           , char                  *p_cn_info_nw     // 接続先固有情報rec(NetWork)
                           , char                  *p_cn_info_if     // 接続先固有情報rec(InterFace)
                           , char                  *p_cn_info_st     // 接続先固有情報rec(Station)
                           , char                  *p_cn_info_cn     // 接続先固有情報rec(connection)
                           , gflin_pkey_def        *p_con_lid        // コネクション論理ID
                           , NWM_CTU_INI_arg_2_def *p_gccut_info     // カット対象日付管理情報
                           , t_rcv_info_def        *p_rslt_info      // 処理結果情報
                           , oggz1in_def           *p_ems_info_cmn   // EMS出力共通情報
                           , ems_info_add          *p_ems_info_add ) // EMS出力付加情報
{
    msg_visanet_def               *rhead_p;
    fixedform_visanet_0800_def    *rfixv_p;

    msg_visanet_def               *shead_p;
    fixedform_visanet_0810_def    *sfixv_p;

    rhead_p = (msg_visanet_def *) p_rcv_data;
    rfixv_p = (fixedform_visanet_0800_def *) &rhead_p->ffd;
    shead_p = (msg_visanet_def *) p_rsp_data;
    sfixv_p = (fixedform_visanet_0810_def *) &shead_p->ffd;

    /* ヘッダー設定             */
    memset((char *)&shead_p->header.mh_hdr_len,0x00,sizeof(MSG_HEADER_VISA_def));
    shead_p->header.mh_hdr_len     = 0x16;               // Header Length
    shead_p->header.mh_hdr_flg_fmt = 0x01;               // Header Flag and Format
    shead_p->header.mh_txt_fmt     = 0x02;               // Text Format

    /***************************************************************************
    2025-05-18 修正
    memcpy((void *)shead_p->header.mh_dst_id,\
                    (void *)rhead_p->header.mh_dst_id,\
                             sizeof(shead_p->header.mh_dst_id));
    memcpy((void *)shead_p->header.mh_src_id,\
                    (void *)rhead_p->header.mh_src_id,\
                             sizeof(shead_p->header.mh_src_id));
    ***************************************************************************/
    memcpy((void *)shead_p->header.mh_dst_id,\
                    (void *)rhead_p->header.mh_src_id,\
                             sizeof(shead_p->header.mh_dst_id));
    memcpy((void *)shead_p->header.mh_src_id,\
                    (void *)rhead_p->header.mh_dst_id,\
                             sizeof(shead_p->header.mh_src_id));
    shead_p->header.mh_rnd_trip = rhead_p->header.mh_rnd_trip;
    memcpy(shead_p->header.mh_vip_flg,\
                    (void *)rhead_p->header.mh_vip_flg,\
                            sizeof(shead_p->header.mh_vip_flg));
    memcpy((void *)shead_p->header.mh_msg_sts_flg,
                    (void *)rhead_p->header.mh_msg_sts_flg,\
                        sizeof(shead_p->header.mh_msg_sts_flg));
    shead_p->header.mh_bat_num = (char)rhead_p->header.mh_bat_num;
    memcpy(shead_p->header.mh_visa_use,\
                    (void *)rhead_p->header.mh_visa_use,\
                           sizeof(shead_p->header.mh_visa_use));
    shead_p->header.mh_usr_info = (char)rhead_p->header.mh_usr_info;

                                                         // MTI
    shead_p->mti[0] = (char)0x08;
    shead_p->mti[1] = (char)0x10;
    memcpy(p_rslt_info->mti,DEF_VI_MTI_0810_RSP,sizeof(p_rslt_info->mti));

    /* データ長算出 */
    p_rslt_info->denbun_len = (short) sizeof(MSG_HEADER_VISA_def) + \
                                      sizeof(shead_p->mti) + \
                                      sizeof(fixedform_visanet_0810_def);

    /* データ部設定             */
    switch (p_rslt_info->ctrl_type[2]) {
        case 0x31:
        case 0x32:
        case 0x33:
            {
                /*   B07 エレメント情報設定 */
                sfixv_p->b07.ffd_header.m_flg_exist = true;
                sfixv_p->b07.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b07.ffd_header.m_fixvalue_length = 10;
                memcpy(sfixv_p->b07.ffd_data,rfixv_p->b07.ffd_data,sizeof(sfixv_p->b07.ffd_data));

                /*   B11 エレメント情報設定 */
                sfixv_p->b11.ffd_header.m_flg_exist = true;
                sfixv_p->b11.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b11.ffd_header.m_fixvalue_length = 6;
                memcpy(sfixv_p->b11.ffd_data,rfixv_p->b11.ffd_data,sizeof(sfixv_p->b11.ffd_data));

                /*   B37 エレメント情報設定 */
                sfixv_p->b37.ffd_header.m_flg_exist = rfixv_p->b37.ffd_header.m_flg_exist;
                sfixv_p->b37.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b37.ffd_header.m_fixvalue_length = rfixv_p->b37.ffd_header.m_fixvalue_length;
                memcpy(sfixv_p->b37.ffd_data,rfixv_p->b37.ffd_data,sizeof(sfixv_p->b37.ffd_data));

                /*   B39 返却値コード       */
                sfixv_p->b39.ffd_header.m_flg_exist = true;
                sfixv_p->b39.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b39.ffd_header.m_fixvalue_length = 2;
                memcpy(sfixv_p->b39.ffd_data,DEF_INSIDE_RSPCD,sizeof(sfixv_p->b39.ffd_data));

                /*   B63 V.I.P. Private-Use Fields */
                sfixv_p->b63.ffd_header.m_flg_exist = rfixv_p->b63.ffd_header.m_flg_exist;
                sfixv_p->b63.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b63.ffd_header.m_fixvalue_length = rfixv_p->b63.ffd_header.m_fixvalue_length;
                memcpy(sfixv_p->b63.ffd_data,rfixv_p->b63.ffd_data,sizeof(sfixv_p->b63.ffd_data));

                /*   B70 Network Management Information Code */
                sfixv_p->b70.ffd_header.m_flg_exist = true;
                sfixv_p->b70.ffd_header.m_dmy1 = 0x20;
                sfixv_p->b70.ffd_header.m_fixvalue_length = 4;
                memcpy(sfixv_p->b70.ffd_data,rfixv_p->b70.ffd_data, sizeof(sfixv_p->b70.ffd_data));

                /* 精査OKの場合           */
                if (memcmp(sfixv_p->b39.ffd_data,DEF_INSIDE_RSPCD,\
                                         sizeof(sfixv_p->b39.ffd_data)) == 0) {
                    /* 制御電文種別4桁目に"A"を設定 */
                    p_rslt_info->ctrl_type[3] = DEF_CTLTYP_STRA;
                } else{
                    /* 制御電文種別4桁目に"B"を設定 */
                    p_rslt_info->ctrl_type[3] = DEF_CTLTYP_STRB;
                }
            }
            break;
        default:
            {
                /* データ長クリア */
                p_rslt_info->denbun_len = (short) 0;

                /* 制御電文種別4桁目に"B"を設定 */
                p_rslt_info->ctrl_type[3] = DEF_CTLTYP_STRB;
            }
            break;
    }

    return(NWM_STE_EDIT_NORMAL);
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_req_rcv                       */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_req_rcv(                      */
/*                              char*, t_rcv_info_def*)                      */
/*  ARGUMENT        : 1. p_dept_st_befor (I)   局状態                        */
/*                  : 2. p_rslt_info     (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 許可応答                                            */
/*                  : 1: 拒否応答                                            */
/*                  : 2: 電文破棄                                            */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(要求)                      */
/*****************************************************************************/
short  NWM_STE_cst_check_req_rcv(char           *p_dept_st_befor        // 局状態
                               , t_rcv_info_def *p_rslt_info )          // 処理結果情報
{
    short       chkval;                                        // 判定結果格納域
    char        ctltyp;                                        // 制御種別
    int         dptsts;                                        // 局状態 INT型
    char        buf[2+1];                                      // 作業領域

    memset(buf,0x00,sizeof(buf));                              // 初期化

    chkval = NWM_STE_SST_KYOHI;                               // 返却値初期化(拒否応答）

    ctltyp = *(p_rslt_info->ctrl_type + 2);                    // 制御電文種別取得
    memcpy(buf,p_dept_st_befor,2);                             // 局状態値取得
    dptsts = atoi(buf);                                        // 局状態を数値化

    /* 制御電文種別で分岐 */
    switch (ctltyp) {
        case DEF_DEPT_OPEN:    // 開局
            {
                /* 局状態で分岐    */
                switch (dptsts) {
                     case  DEF_STATE_IOPEN:      //  開局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOS:      //  閉局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                           sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_IOPENPROC:  //  開局処理中
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                           sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOSPROC:  //  閉局処理中
                         {
                             chkval = (short) NWM_STE_SST_HAKI;      // 電文破棄
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                             memcpy(p_rslt_info->naibu_errcd,DEF_NERR_HSMK_SST_OPN,\
                                                  sizeof(p_rslt_info->naibu_errcd));
                         }
                         break;
                     default:
                         {
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                             memcpy(p_rslt_info->naibu_errcd,DEF_NERR_HSMK_SST_OPN,\
                                                     strlen(DEF_NERR_HSMK_SST_OPN));
                         }
                         break;
                 }
            }
            break;
        case DEF_DEPT_CLOS:    // 閉局
            {
                /* 局状態で分岐    */
                switch (dptsts) {
                     case  DEF_STATE_IOPEN:     //   開局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                           sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOS:     //   閉局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_IOPENPROC: //   開局処理中
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                           sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOSPROC: //   閉局処理中
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                           sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     default:
                         {
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                             memcpy(p_rslt_info->naibu_errcd,DEF_NERR_HSMK_SST_CLS,\
                                                     strlen(DEF_NERR_HSMK_SST_CLS));
                         }
                         break;
                 }
            }
            break;
        case DEF_DEPT_ECHO:    // エコーテスト
            {
                /* 局状態で分岐    */
                switch (dptsts) {
                     case  DEF_STATE_IOPEN:      //  開局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOS:      //  閉局
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_IOPENPROC:  //  開局処理中
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOSPROC:  //  閉局処理中
                         {
                             chkval = (short) NWM_STE_SST_KYOKA;    // 許可応答
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     default:
                         {
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                             memcpy(p_rslt_info->naibu_errcd,DEF_NERR_HSMK_SST_ECHO,\
                                              strlen(DEF_NERR_HSMK_SST_ECHO));
                         }
                         break;
                }
            }
            break;
        default:
            {
                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                               sizeof(p_rslt_info->new_stn_sts));
            }
            break;
    }
    return(chkval);
}


/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_err                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_err(                     */
/*                                      char*, t_rcv_info_def*)             */
/*  ARGUMENT        : 1. p_dept_st_befor(I)   局状態                        */
/*                  : 2. p_rslt_info    (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(応答送信不可)             */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_err(char           *p_dept_st_befor        // 局状態
                                ,t_rcv_info_def *p_rslt_info )          // 処理結果情報
{
    short       chkval;                                    // 判定結果格納域
    char        ctltyp;                                    // 制御種別
    char        anstyp;                                    // 制御応答種別
    int         dptsts;                                    // 局状態 INT型
    char        buf[2+1];                                  // 作業領域

    memset(buf,0x00,sizeof(buf));                          // 初期化

    chkval = NWM_STE_SST_NOUPDATE;                         // 返却値初期化（更新無)
    ctltyp = *(p_rslt_info->ctrl_type + 2);                // 制御電文種別
    anstyp = *(p_rslt_info->ctrl_type + 3);                // 応答種別取得
    memcpy(buf,p_dept_st_befor,2);                         // 局状態値取得
    dptsts = atoi(buf);                                    // 局状態を数値化

    /* 制御電文種別で分岐 */
    switch (ctltyp) {
        case DEF_DEPT_OPEN:    // 開局
            {
                if (anstyp == (char) DEF_ANSTYPE_AUTHOR) {    // 許可応答
                    /* 局状態で分岐    */
                    switch (dptsts) {
                        case  DEF_STATE_IOPEN:      //   開局
                            {
                                chkval = (short) NWM_STE_SST_UPDATE;    // 管理ファイル更新有
                                memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                              sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOS:      //   閉局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_IOPENPROC:  //   開局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        default:
                            {
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                    }
                } else {    // 拒否応答
                    chkval = (short) NWM_STE_SST_NOUPDATE;    //   管理ファイル更新無
                    memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                   sizeof(p_rslt_info->new_stn_sts));
                }
            }
            break;
        case DEF_DEPT_CLOS:    // 閉局
            {
                if (anstyp == (char) DEF_ANSTYPE_AUTHOR) {    // 許可応答
                    /* 局状態で分岐    */
                    switch (dptsts) {
                        case  DEF_STATE_IOPEN:      //   開局
                            {
                                chkval = (short) NWM_STE_SST_UPDATE;    // 管理ファイル更新有
                                memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                              sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOS:      //   閉局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_IOPENPROC:  //   開局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        default:
                            {
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                    }
                } else {    // 拒否応答
                    chkval = (short) NWM_STE_SST_NOUPDATE;    //   管理ファイル更新無
                    memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                   sizeof(p_rslt_info->new_stn_sts));
                }
            }
            break;
        default :    // その他
            {
                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                               sizeof(p_rslt_info->new_stn_sts));
            }
    }
    return(chkval);
}

/****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_STE_cst_check_rsp_rcv                      */
/*  CALLING SEQ.    : short  NWM_STE_cst_check_rsp_rcv(                     */
/*                              char*, char*, char*, t_rcv_info_def*)       */
/*  ARGUMENT        : 1. p_dept_st_befor(I)   局状態                        */
/*                  : 2. p_req_type     (I)   要求種別                      */
/*                  : 3. p_rcv_data     (I)   受信電文                      */
/*                  : 4. p_rslt_info    (I/O) 処理結果情報                  */
/*  RETURN CODE     : 0: 管理ファイル更新あり                               */
/*                  : 1: 管理ファイル更新なし                               */
/*  DESCRIPTION     : 開閉局･エコー局状態チェック(仕向応答)                 */
/****************************************************************************/
short  NWM_STE_cst_check_rsp_rcv(char  *p_dept_st_befor        // 局状態
                               , char  *p_req_type             // 要求種別
                               , char  *p_rcv_data             // 仕向け要求電文データ
                               , t_rcv_info_def *p_rslt_info)  // 処理結果情報
{
    short       chkval;                                        // 判定結果格納域
        short       rcvsts;                                        // 仕向け応答受信結果
    char        ctltyp;                                        // 制御種別
    int         dptsts;                                        // 局状態 INT型
    char        buf[2+1];                                      // 作業領域

    memset(buf,0x00,sizeof(buf));                              // 初期化

    msg_visanet_def               *shead_p;
    fixedform_visanet_0810_def    *sfixv_p;

    chkval = NWM_STE_SST_NOUPDATE;                                 // 返却値初期化（更新無）

    ctltyp = *(p_rslt_info->ctrl_type + 2);                    // 制御電文種別を取得
    memcpy(buf,p_dept_st_befor,2);                             // 局状態値取得
    dptsts = atoi(buf);                                        // 局状態を数値化

    if (memcmp(p_req_type,DEF_REQTYP_DESTANS,2) == 0) {        // 要求種別:20
        shead_p = (msg_visanet_def *) p_rcv_data;
        sfixv_p = (fixedform_visanet_0810_def *) &shead_p->ffd;
        rcvsts = (short) DEF_DESTANS_NG;                       // 仕向け応答NG設定
        if (p_rslt_info->ctrl_type[3] == DEF_CTLTYP_STRA) {
            rcvsts = (short) DEF_DESTANS_OK;                   // 応答OK
        }
    } else if (memcmp(p_req_type,DEF_REQTYP_ANSTIMO,2) == 0) { // 要求種別:30
        rcvsts = (short) DEF_DESTANS_NG;      // 仕向け応答NG設定
    } else if (memcmp(p_req_type, DEF_REQTYP_SENDNG,2) == 0) { // 要求種別:20or30以外
        rcvsts = (short) DEF_DESTANS_NG;      // 仕向け応答NG設定
    }

    /* 制御電文種別で分岐 */
    switch (ctltyp) {
        case DEF_DEPT_OPEN:    // 開局
            {
                if (rcvsts == (short) DEF_DESTANS_OK) {    // 仕向け応答OK
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_SST_UPDATE;    // 管理ファイル更新有
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                               sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));

                             }
                             break;
                    }
                } else {                                                    // 仕向け応答OK
                    /* 局状態で分岐    */
                    switch (dptsts) {
                        case  DEF_STATE_IOPEN:      //   開局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                             break;
                        case  DEF_STATE_ICLOS:      //   閉局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_IOPENPROC:  //   開局処理中
                            {
                                chkval = (short) NWM_STE_SST_UPDATE;     // 管理ファイル更新有
                                memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        default:
                            {
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                    }
                }
            }
            break;
        case DEF_DEPT_CLOS:              // 閉局
            {
                if (rcvsts == (short) DEF_DESTANS_OK) {
                    /* 局状態で分岐    */
                    switch (dptsts) {
                        case  DEF_STATE_IOPEN:      //   開局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOS:      //   閉局
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_IOPENPROC:  //   開局処理中
                            {
                                chkval = (short) NWM_STE_SST_NOUPDATE;     // 管理ファイル更新無
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                            {
                                chkval = (short) NWM_STE_SST_UPDATE;     // 管理ファイル更新有
                                memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                               sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                        default:
                            {
                                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                            }
                            break;
                    }
                } else {
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_SST_NOUPDATE;    // 管理ファイル更新無
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_SST_UPDATE;    // 管理ファイル更新有
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                    }
                }
            }
            break;
        default:
            {
                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                               sizeof(p_rslt_info->new_stn_sts));
            }
            break;
    }
    return(chkval);
}

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
short   NWM_STE_cst_check_command(char  *p_dept_st_befor          // 局状態
                                 ,char  *p_nwi_if                 // NW情報レコード(インタフェース単位)
                                 ,t_rcv_info_def *p_rslt_info )   // 処理結果情報
{
    short       chkval;                                        // 判定結果格納域
    char        ctltyp;                                        // 制御種別
    char        comtyp;                                        // 制御応答種別
    int         dptsts;                                        // 局状態 INT型
    char        buf[2+1];                                      // 作業領域

    memset(buf,0x00,sizeof(buf));                              // 初期化

    chkval = NWM_STE_CMD_NG;                                 // 返却値初期設定（受付不可）

    ctltyp = *(p_rslt_info->ctrl_type + 2);                    // 制御種別取得
    comtyp = *(p_rslt_info->ctrl_type + 3);                    // 制御応答種別取得
    memcpy(buf,p_dept_st_befor,2);                             // 局状態値取得
    dptsts = atoi(buf);                                        // 局状態を数値化

    /* 制御電文種別で分岐 */
    switch (ctltyp) {
        case DEF_DEPT_OPEN:    // 開局
            {
                if (comtyp == DEF_CTLINT_NORMAL) {  // オプション無
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:     //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:     //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPNING,\
                                                   sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC: //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_OPNING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC: //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_CLSING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                    }
                } else if (comtyp == DEF_CTLINT_FORCE) {  // 強制実行
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:           //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPNING,\
                                                   sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:           //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPNING,\
                                                   sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:       //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_OPNING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_CLSING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));

                             }
                             break;
                    }
                } else if (comtyp == DEF_CTLINT_UPDATEONLY) {     // 状態更新のみ
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 //memcpy(p_rslt_info->naibu_errcd,\
                                 //    DEF_NERR_HSMK_SST_OPNING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_OPN,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 //memcpy(p_rslt_info->naibu_errcd,\
                                 //    DEF_NERR_HSMK_SST_CLSING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));

                             }
                             break;
                    }
                } else {
                    chkval = (short) NWM_STE_CMD_NG;
                    memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,
                                   sizeof(p_rslt_info->new_stn_sts));
                    memcpy(p_rslt_info->naibu_errcd,
//                      DEF_NERR_SMK_SST_CTLINT_ERR,sizeof(p_rslt_info->naibu_errcd));
                        DEF_NERR_SYSIF_LGC_ERR,sizeof(p_rslt_info->naibu_errcd));
                }
            }
            break;
        case DEF_DEPT_CLOS:              // 閉局
            {
                if (comtyp == DEF_CTLINT_NORMAL) {            // オプション無
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLOSING,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_OPNING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_CLSING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                    }
               } else if (comtyp == DEF_CTLINT_FORCE) {     // 強制実行
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLOSING,\
                                                   sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_SEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLOSING,\
                                                   sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_OPNING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_NG;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                                 memcpy(p_rslt_info->naibu_errcd,\
                                     DEF_NERR_HSMK_SST_CLSING,sizeof(p_rslt_info->naibu_errcd));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                    }
                } else if (comtyp == DEF_CTLINT_UPDATEONLY) {     // 状態更新のみ
                    /* 局状態で分岐    */
                    switch (dptsts) {
                         case  DEF_STATE_IOPEN:      //   開局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                               sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOS:      //   閉局
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_IOPENPROC:  //   開局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                               sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                             {
                                 chkval = (short) NWM_STE_CMD_OK_NOSEND;
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STTE_STS_CLS,\
                                               sizeof(p_rslt_info->new_stn_sts));
                             }
                             break;
                         default:
                             {
                                 memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                                sizeof(p_rslt_info->new_stn_sts));

                             }
                             break;
                    }
                } else {
                    chkval = (short) NWM_STE_CMD_NG;
                    memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,
                                   sizeof(p_rslt_info->new_stn_sts));
                    memcpy(p_rslt_info->naibu_errcd,
//                      DEF_NERR_SMK_SST_CTLINT_ERR,sizeof(p_rslt_info->naibu_errcd));
                        DEF_NERR_SYSIF_LGC_ERR,sizeof(p_rslt_info->naibu_errcd));
                }
            }
            break;
        case DEF_DEPT_ECHO:              // エコーテスト
            {
                /* 局状態で分岐    */
                switch (dptsts) {
                     case  DEF_STATE_IOPEN:      //   開局
                         {
                             chkval = (short) NWM_STE_CMD_OK_SEND;
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOS:      //   閉局
                         {
                             chkval = (short) NWM_STE_CMD_OK_SEND;
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_IOPENPROC:  //   開局処理中
                         {
                             chkval = (short) NWM_STE_CMD_OK_SEND;
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     case  DEF_STATE_ICLOSPROC:  //   閉局処理中
                         {
                             chkval = (short) NWM_STE_CMD_OK_SEND;
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                     default:
                         {
                             memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                                            sizeof(p_rslt_info->new_stn_sts));
                         }
                         break;
                }
            }
            break;
        default:
            {
                memcpy(p_rslt_info->new_stn_sts,DEF_STATE_SPACE,\
                               sizeof(p_rslt_info->new_stn_sts));
            }
            break;
    }
    return(chkval);
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  chk_numeric                                     */
/*  DESCRIPTION     : ニューメリックチェック                                 */
/*                                                                           */
/*  CALLING SEQ.    : short chk_numeric()                                    */
/*  ARGUMENT        : size            : データサイズ（バイト長）             */
/*                    *ptr            : チェック対象文字列格納域アドレス     */
/*                                                                           */
/*  RETURN CODE     : 0:正常終了  1:チェック異常                             */
/*****************************************************************************/
static short chk_numeric(int size,char *ptr)
{
    int      cnt;                                     // ループカウンタ
    short    rtncd;                                   // 返却値格納域

    rtncd = DEF_RTN_NORMAL;
    for (cnt=0;cnt < size;cnt++) {
       if (*(ptr + cnt) < 0x30 || *(ptr + cnt) > 0x39) {
           rtncd = DEF_RTN_ERRCD;
           break;
       }
    }
    return(rtncd);
}

/****************************************************************************
**
**  文字型数値のBCDデータへの変換
**
**  FUNCTION         : ASCIITOBCD()
**  CALLING SEQ.     : void ASCIITOBCD(int,unsigned char *,char *)
**
**  ARGUMENT         : 1. len       (I)    変換元データレングス
**                     2  *str_p    (I)    変換元データ（ASCII）
**                     3. *bcd_p    (I/O)  変換先データ(BCD)格納域ポインタ
**
**  RETURN CODE      : NONE
**
**  DESCRIPTION      :
**  -----------------------------------------------------------------------
**  COMMENT          :
****************************************************************************/
void ASCIITOBCD(int len,unsigned char *str_p,unsigned char *bcd_p)
{
    int        cnt;                                   // ループカウンタ
    int        set;                                   // データ設定位置バイト

    set = 0;
    for (cnt=0;cnt < len; cnt++) {
       if (cnt == 0 || (cnt % 2) == 0) {
           *(bcd_p + set)  = (unsigned char) (*(str_p + cnt) << 4);
       } else {
           *(bcd_p + set) |= (unsigned char) (*(str_p + cnt) & 0x0f);
           set++;
       }
    }
    return;
}
