/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXG0                                    */
/*        FUNCTION          ････ GFP内部LCN採番                              */
/*                                                                           */
/*                               電文振分(inbound)、制御電文機能サーバから   */
/*                               GFP内部LCN採番要求を受信し、                */
/*                               採番したGFP内部LCNを                        */
/*                               要求元にREPLAYする。                        */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-09-17                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/09/17 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdio.h>    nolist
#include  <stdlib.h>   nolist
#include  <string.h>   nolist
#include  <ctype.h>    nolist
#include  <math.h>     nolist
#include  <errno.h>    nolist
#include  <unistd.h>   nolist
#include  <tal.h>      nolist
#include  <cextdecs.h> nolist
#include  <zspic>      nolist
#include  <zfilc>      nolist
/* USER HEADER */
#include "file.h"      nolist      // file
#include "ems.h"       nolist      // ems
#include "GFPOGGZ4_traceout.h" nolist   // trace
#include "errcd.h"     nolist      // error code
#include "ipc.h"       nolist      // IPC関連
#include "GFPCVXG0.h"  nolist      // GFP内部LCN採番
#include "GFPCGX50.h"  nolist      // システム日時取得
#include "GFPCGXB0.h"  nolist      // IOモジュール
#include "GFPCGXD0.h"  nolist      // ASSIGN情報取得
#include "vproc.h"     nolist      // vproc

// GLOBAL
static myinfo_def g_myinfo;
static _lowmem oggz1in_def g_oggz1in;
// トレース出力モジュール用
char  EXMYSRVCLSNAME[15];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;

/****************************************************************************/
/*  FUNCTION        : 0.0.0  main                                           */
/*  CALLING SEQ.    : int  main(void)                                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0                                                     */
/*  DESCRIPTION     : エントリポイント                                      */
/****************************************************************************/
int main(void)
{
    /*------------------------------------------------*/
    /*    初期処理                                    */
    /*------------------------------------------------*/
    LCNN_init();

    /*------------------------------------------------*/
    /*    主処理                                      */
    /*------------------------------------------------*/
    LCNN_main();

    /*------------------------------------------------*/
    /*    終了処理                                    */
    /*------------------------------------------------*/
    LCNN_final();

    return 0;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  LCNN_init                                      */
/*  CALLING SEQ.    : void  LCNN_init(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void LCNN_init(void)
{
    short ret;
    lk_zac2001r_arg_1_def trace_if;
    char buffer[64];

    // 変数初期化
    memset(&g_myinfo, NULL, sizeof(myinfo_def));
    PROCESSHANDLE_NULLIT_(g_myinfo.procinfo.my_phandle);
    PROCESSHANDLE_NULLIT_(g_myinfo.procinfo.ans_phandle);
    g_myinfo.recv_fno = -1;
    g_myinfo.end_flg = DEF_FLG_OFF;
    // g_myinfo.trace_flg = DEF_FLG_OFF;
    g_myinfo.seqnum.seqnum = -1;
    g_myinfo.seqnum.seqnum_min = -1;
    g_myinfo.seqnum.seqnum_max = -1;

    // EMS出力モジュールI/F用変数初期化
    memset(&g_oggz1in, ' ', sizeof(g_oggz1in));
    // リターンコード
    memcpy(&g_oggz1in.emsinf.rcd, DEF_EMS_DEF_RET_CODE, sizeof(g_oggz1in.emsinf.rcd));
    // システム名(GFP)
    memcpy(g_oggz1in.emsinf.emsgkinf.sysnm, DEF_EMS_SYSNM_GFP, sizeof(DEF_EMS_SYSNM_GFP)-1);
    // 
    memcpy(g_oggz1in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM, sizeof(DEF_EMS_SRV_KBN_COM)-1);
    // メッセージ出力元プログラム名
    memcpy(g_oggz1in.emsinf.emsgkinf.prgid, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);

    // トレース出力モジュール初期化
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    trace_if.func_flg = DEF_TRACE_FUNC_INI;
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    ret = TRACEOUT((char *)&trace_if);

    // パラメータ取得処理
    LCNN_get_params();

    // $RECEIVEオープン
    ret = FILE_OPEN_(DEF_RECVFILE_NAME,
                     (short)sizeof(DEF_RECVFILE_NAME)-1,
                     &g_myinfo.recv_fno,
                     ,// access default:0(r/w)
                     ,// exclusion
                     1,// nowait depth $RECEIVEの最大値:1
                     DEF_RECV_DEPTH_MAX, // sync or receive depth
                     // options
          ); 
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：ファイルI/Oエラー           */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_FILE_IO_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ⑤ファイル論理的名 (8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
               DEF_RECVFILE_NAME, sizeof(DEF_RECVFILE_NAME)-1);
        // ⑥アクション (19バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "OPEN", sizeof("OPEN")-1);
        // ⑧エラーコード (5バイト)
        snprintf(buffer, sizeof(buffer), "%05d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }

    // サーバ停止判定初期化
    COM_STP_INIT(&g_myinfo.stp_arg);

    // GFP内部LCNのユニーク性担保のため、1/100秒DELAYする。
    // GFP内部LCNの採番仕様に1/100秒の値が含まれているため、プロセスダウン、
    // オートリスタートでプロセス毎の連番が重複したとしても、
    // 起動時に1/100秒DELAYすることで、GFP内部LCN全体のユニーク性を担保。
    PROCESS_DELAY_(DEF_START_DELAY_CS);

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス起動                */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LCNN_setmsgid(DEF_EVT_PROC_START);
    // 内部エラーコード
    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1);
    // ③プロセス名 (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&g_oggz1in);
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0  LCNN_main                                      */
/*  CALLING SEQ.    : void  LCNN_main(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void LCNN_main(void)
{
    while (g_myinfo.end_flg != DEF_FLG_ON) {

        // RECEIVE処理
        LCNN_read_recv();

        // イベント解析
        switch (g_myinfo.iocomp.errno) {
        // システムメッセージ処理
        case ZFIL_ERR_SYSMESS:
            LCNN_handle_sys_msg();
            break;
        // 要求受信処理
        case ZFIL_ERR_OK:
            LCNN_handle_req_msg();
            break;
        default:
            // nullリプライ
            memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
            break;
        }

        // リプライ処理
        LCNN_reply();
    }
}

/****************************************************************************/
/*  FUNCTION        : 3.0.0  LCNN_final                                     */
/*  CALLING SEQ.    : void  LCNN_final(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void LCNN_final(void)
{
    char buffer[64];
    lk_zac2001r_arg_1_def trace_if;

    // $RECEIVEクローズ
    FILE_CLOSE_(g_myinfo.recv_fno);

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス正常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LCNN_setmsgid(DEF_EVT_PROC_NORMAL_END);
    // 内部エラーコード
    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1);
    // ③プロセス名 (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&g_oggz1in);

    // トレースモジュール終了
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    trace_if.func_flg = DEF_TRACE_FUNC_END;
    TRACEOUT((char *)&trace_if);

    // プロセス終了
    PROCESS_STOP_(, , DEF_PROC_NORMAL_END);
}

/****************************************************************************/
/*  FUNCTION        : 8.0.0  LCNN_abend                                     */
/*  CALLING SEQ.    : void  LCNN_abend(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void LCNN_abend(void)
{
    char buffer[64];
    lk_zac2001r_arg_1_def trace_if;

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス異常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LCNN_setmsgid(DEF_EVT_PROC_ABNORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&g_oggz1in);

    // トレースモジュール終了
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    trace_if.func_flg = DEF_TRACE_FUNC_END;
    TRACEOUT((char *)&trace_if);

    // プロセス終了
    PROCESS_STOP_(, , DEF_PROC_ABNORMAL_END);
}

/****************************************************************************/
/*  FUNCTION        : 1.1.0  LCNN_get_params                                */
/*  CALLING SEQ.    : void  LCNN_get_params(void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void LCNN_get_params(void)
{
    int i_ret;
    short ret, fnm_len;
    short found;
    long  fio_timer;
    double ptimer;
    char asn_name[32];
    char buffer[64];
    char cnvstr[64];
    char fname[ZSYS_VAL_LEN_FILENAME+1];
    char trc_fname[ZSYS_VAL_LEN_FILENAME+1];
    char sub_prog_sts[2];
    db_gfphi_def       t_gfphi_pk;              // 物理名情報ファイルPK
    db_gfphi_def*      pt_gfphi_rec;            // 物理名情報ファイルレコード
    COM_IOM_arg_3_def iom_arg3;
    COM_IOM_arg_4_def iom_arg4;
    COM_IOM_arg_5_def iom_arg5;
    COM_IOM_arg_6_def iom_arg6;
    // EMSリンケージ：N/W識別変換
    struct __cnvnw {
        char nwid;
        char *nwdiv;
    } cnvnw[] = {
        {DEF_NW_ID_JCN,      DEF_NW_KUBUN_CARDNET},         // JCN(CUP)
        {DEF_NW_ID_VISA,     DEF_NW_KUBUN_VISANET},         // Visanet
        {DEF_NW_ID_MASTER,   DEF_NW_KUBUN_BANKNET},         // Banknet
        {DEF_NW_ID_AMEX,     DEF_NW_KUBUN_AEGN},            // AEGEN
        {DEF_NW_ID_DISCOVER, DEF_NW_KUBUN_DISCOVER},        // Discover
        {DEF_NW_ID_NYCE,     DEF_NW_KUBUN_NYCE},            // NYCE
        {DEF_NW_ID_JLink,    DEF_NW_KUBUN_JLINK},           // J-Link(通信受信時)
        {DEF_NW_ID_UnionPay, DEF_NW_KUBUN_UNIONPAY}         // UnionPay
    };

    // 運用監視端末出力サーバ・サーバクラス名取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_MSG_SRV_NAME, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_MSG_SRV_NAME,
               sizeof(DEF_MSG_SRV_NAME)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    memcpy(g_oggz1in.uytrminf.uytrmsrv, buffer, strlen(buffer));
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(buffer));
    memcpy(g_oggz1in.uytrminf.uytrmsrvlen, cnvstr, strlen(cnvstr));

    // 運用監視端末出力サーバ・PATHMON名取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_MSG_MON_NAME, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_MSG_MON_NAME,
               sizeof(DEF_MSG_MON_NAME)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    memcpy(g_oggz1in.uytrminf.uytrmmon, buffer, strlen(buffer));
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(buffer));
    memcpy(g_oggz1in.uytrminf.uytrmmonlen, cnvstr, strlen(cnvstr));

    // PATHSENDタイマー取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_PSEND_TIMER_10MSECOND, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_PSEND_TIMER_10MSECOND,
               sizeof(DEF_PSEND_TIMER_10MSECOND)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    // EMSのタイマー値：秒単位(10msec → sec切り上げ)
    snprintf(cnvstr, sizeof(cnvstr), "%04d", (int)ceil((double)atoi(buffer) / 100));
    memcpy (g_oggz1in.uytrminf.proctimer, cnvstr, strlen(cnvstr));

    // 自・親プロセス情報取得
    ret = COM_PRC(&g_myinfo.procinfo);
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：プロシージャコールエラー    */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PROCEDURE_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
        // ③プロシージャ名(40バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               g_myinfo.procinfo.err_pname, strlen(g_myinfo.procinfo.err_pname));
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    memcpy(g_oggz1in.emsinf.emsgkinf.trmnm, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));

    // 自サーバクラス論理ID取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_SRV_LOGICAL_ID, buffer, (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_SRV_LOGICAL_ID,
               sizeof(DEF_SRV_LOGICAL_ID)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    memcpy(g_myinfo.site_id, &buffer[DEF_SC_OFFSET_SITE],
           sizeof(g_myinfo.site_id));
    memcpy(g_myinfo.network_id, &buffer[DEF_SC_OFFSET_NW],
           sizeof(g_myinfo.network_id));
    memcpy(g_myinfo.group_id, &buffer[DEF_SC_OFFSET_GRP],
           sizeof(g_myinfo.group_id));
    memcpy(g_myinfo.serverclass_id, &buffer[DEF_SC_OFFSET_SCKND],
           DEF_SC_KIND_LEN);
    memcpy(&g_myinfo.serverclass_id[DEF_SC_KIND_LEN],
           &buffer[DEF_SC_OFFSET_SCNUM],
           sizeof(g_myinfo.serverclass_id) - DEF_SC_KIND_LEN);

    // N/W識別を変換してEMSリンケージへ設定
    for (int i = 0; i < sizeof(cnvnw)/sizeof(struct __cnvnw); ++i) {
        if (cnvnw[i].nwid == g_myinfo.network_id[0]) {
            memcpy(g_oggz1in.emsinf.emsgkinf.h_nw_kbn, cnvnw[i].nwdiv, strlen(cnvnw[i].nwdiv));
            break;
        }
    }

    // ファイルIOタイマー取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_FILE_IO_TIMER_10MSECOND, buffer, (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FILE_IO_TIMER_10MSECOND,
               sizeof(DEF_FILE_IO_TIMER_10MSECOND)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
    fio_timer = atol(buffer);

    // ASSIGN情報取得モジュールにて物理名情報ファイル名を取得
    memset(asn_name, ' ', sizeof(asn_name));
    memcpy(asn_name, DEF_ASN_GFPHI, sizeof(DEF_ASN_GFPHI)-1);
    memset(fname, NULL, sizeof(fname));
    COM_ASN(asn_name, fname, &fnm_len);
    if (fnm_len == 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR, sizeof(DEF_NERR_PRM_RD_ERR)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_ASN_GFPHI, sizeof(DEF_ASN_GFPHI)-1);
        // ④エラーコード(4バイト)
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        LCNN_abend();
    }

    // COM_IOMトレース情報設定
    memset(&iom_arg3, NULL, sizeof(iom_arg3));
    memcpy(iom_arg3.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    memcpy(iom_arg3.file_id, DEF_FL_TRACE,  sizeof(DEF_FL_TRACE)-1);
    memcpy(iom_arg3.file_name, EXTRACEFILENAME, strlen(EXTRACEFILENAME));
    
    memcpy(iom_arg3.file_io_type, DEF_COM_IOM_FIO_OPEN, sizeof(DEF_COM_IOM_FIO_OPEN)-1);
    // COM_IOMファイル情報設定
    memset(&iom_arg4, NULL, sizeof(iom_arg4));
    memcpy(iom_arg4.file_id, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
    memcpy(iom_arg4.file_name, fname, strlen(fname));

    // 物理名情報ファイルオープン
    ret = COM_IOM(DEF_COM_IOM_FUNC_OPEN, sub_prog_sts,
                  &iom_arg3, &iom_arg4, &iom_arg5, &iom_arg6);
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }

    // Key設定
    memcpy(&t_gfphi_pk.pri_key.site_id, g_myinfo.site_id,
           sizeof(t_gfphi_pk.pri_key.site_id));
    memcpy(&t_gfphi_pk.pri_key.nw_id, g_myinfo.network_id,
           sizeof(t_gfphi_pk.pri_key.nw_id));
    memcpy(t_gfphi_pk.pri_key.grp_id, g_myinfo.group_id,
           sizeof(t_gfphi_pk.pri_key.grp_id));
    memcpy(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,
           g_myinfo.serverclass_id,
           sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num,
           &g_myinfo.serverclass_id[8],
           sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memset(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num,        '0',
           sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num));
    memcpy(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind,
           DEF_PRC_GFP_LCN,
           sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memset(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num,  '0',
           sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset(t_gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num,          '0',
           sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));

    // COM_IOM入力情報設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.part_key_type = 0;
    memcpy(iom_arg5.key_value, &t_gfphi_pk.pri_key, sizeof(t_gfphi_pk.pri_key));
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI,
           sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = sizeof(t_gfphi_pk.pri_key);
    iom_arg5.compare_len = sizeof(t_gfphi_pk.pri_key.site_id) + sizeof(t_gfphi_pk.pri_key.nw_id) + 
        sizeof(t_gfphi_pk.pri_key.grp_id) + sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id);
    iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.io_timer = fio_timer;
    iom_arg5.rec_len = db_gfphi_def_Size;

    // 自プロセス名と一致するまで繰り返しレコードを取得
    found = 0;
    // READ開始
    memcpy(iom_arg3.file_io_type, DEF_COM_IOM_FIO_START,
           sizeof(DEF_COM_IOM_FIO_START)-1);
    ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts,
                  &iom_arg3, &iom_arg4, &iom_arg5, &iom_arg6);
    while (ret == 0) {
        // EOF判定
        if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR,
            sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                LCNN_setmsgid(DEF_EVT_CONFIG_ERR);
                // ⑤エラー項目/エラー理由 (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
                       "target record not found", sizeof("target record not found")-1);
            break;
        } else if (memcmp(sub_prog_sts, DEF_COM_IOM_NO_ERR,
            sizeof(DEF_COM_IOM_NO_ERR)-1)) {
            ret = 1;
            break;
        }
        pt_gfphi_rec = (db_gfphi_def*)iom_arg6.rec_area;
        // プロセス名比較
        i_ret = LCNN_procnamecmp(&g_myinfo.procinfo, pt_gfphi_rec->prc_file_info.prc_file_name,
            sizeof(pt_gfphi_rec->prc_file_info.prc_file_name));
        if (i_ret == 0 &&
            (memcmp(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, DEF_NUM4DIGIT0, sizeof(DEF_NUM4DIGIT0)-1) == 0 ||
            memcmp(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, DEF_NUM4DIGIT1, sizeof(DEF_NUM4DIGIT1)-1) == 0) &&
            memcmp(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_PRC_GFP_LCN, sizeof(DEF_PRC_GFP_LCN)-1) == 0 &&
            memcmp(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_NUM4DIGIT0, sizeof(DEF_NUM4DIGIT0)-1) == 0) {
            // GFP内部LCN採番範囲取得
            memset(buffer, NULL, sizeof(buffer));
            memcpy(buffer, pt_gfphi_rec->lcn_num_scope.lcn_num_min,
            sizeof(pt_gfphi_rec->lcn_num_scope.lcn_num_min));
            g_myinfo.seqnum.seqnum_min = (short)atoi(buffer);
            memset(buffer, NULL, sizeof(buffer));
            memcpy(buffer, pt_gfphi_rec->lcn_num_scope.lcn_num_max,
            sizeof(pt_gfphi_rec->lcn_num_scope.lcn_num_max));
            g_myinfo.seqnum.seqnum_max = (short)atoi(buffer);
            g_myinfo.seqnum.seqnum = g_myinfo.seqnum.seqnum_min;
            // 範囲チェック
            if (g_myinfo.seqnum.seqnum_min < DEF_PROC_SEQNUM_MIN ||
                g_myinfo.seqnum.seqnum_min > DEF_PROC_SEQNUM_MAX) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                LCNN_setmsgid(DEF_EVT_CONFIG_ERR);
                // ⑤エラー項目/エラー理由 (40バイト)
                snprintf(buffer, sizeof(buffer), "lcn_num_scope.lcn_num_min(%04d) invalid",
                        g_myinfo.seqnum.seqnum_min);
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
                       buffer, strlen(buffer));
                break;
            } else if (g_myinfo.seqnum.seqnum_max < DEF_PROC_SEQNUM_MIN ||
                       g_myinfo.seqnum.seqnum_max > DEF_PROC_SEQNUM_MAX) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                LCNN_setmsgid(DEF_EVT_CONFIG_ERR);
                // ⑤エラー項目/エラー理由 (40バイト)
                snprintf(buffer, sizeof(buffer), "lcn_num_scope.lcn_num_max(%04d) invalid",
                        g_myinfo.seqnum.seqnum_max);
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
                       buffer, strlen(buffer));
                break;
            }
            // 大小チェック
            if (g_myinfo.seqnum.seqnum_min > g_myinfo.seqnum.seqnum_max) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                LCNN_setmsgid(DEF_EVT_CONFIG_ERR);
                // ⑤エラー項目/エラー理由 (40バイト)
                snprintf(buffer, sizeof(buffer), "min(%04d) is greater than max(%04d)",
                        g_myinfo.seqnum.seqnum_min, g_myinfo.seqnum.seqnum_max);
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
                       buffer, strlen(buffer));
                break;
            }
            found = 1;
            break;
        }
        // NEXTREAD
        memcpy(iom_arg3.file_io_type, DEF_COM_IOM_FIO_READ,
               sizeof(DEF_COM_IOM_FIO_READ)-1);
        ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts,
        &iom_arg3, &iom_arg4, &iom_arg5, &iom_arg6);
    }
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
    }

    // 物理名情報ファイルクローズ
    memcpy(iom_arg3.file_io_type, DEF_COM_IOM_FIO_CLOSE,
        sizeof(DEF_COM_IOM_FIO_CLOSE)-1);
    COM_IOM(DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts,
            &iom_arg3, &iom_arg4, &iom_arg5, &iom_arg6);

    if (!found) {
        if (ret == 0) {
            /*------------------------------------------------*/
            /*    メッセージ編集：設定情報エラー              */
            /*------------------------------------------------*/
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
            // ③ファイル論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
            // ④キー (40バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,
                   &t_gfphi_pk.pri_key, sizeof(t_gfphi_pk.pri_key));
        }
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.1.0  LCNN_read_recv                                 */
/*  CALLING SEQ.    : void  LCNN_read_recv(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : $RECEIVE処理                                          */
/****************************************************************************/
void LCNN_read_recv(void)
{
    iocomp_def* piocomp = &g_myinfo.iocomp;

    // $RECEIVEのREADUPDATE
    READUPDATEX(g_myinfo.recv_fno,
                g_myinfo.recvbuf,
                DEF_RCV_BUF_SIZE,
                &piocomp->len);
    piocomp->fno = g_myinfo.recv_fno;
    AWAITIOX(&piocomp->fno,
                  &piocomp->addr,
                  &piocomp->len,
                  &piocomp->tag);
    FILE_GETINFO_(piocomp->fno, &piocomp->errno);

    // $RECEIVE情報取得
    FILE_GETRECEIVEINFO_((short _far *)&piocomp->recv_info);
}

/****************************************************************************/
/*  FUNCTION        : 2.2.0  LCNN_handle_symsg                              */
/*  CALLING SEQ.    : void  LCNN_handle_symsg(void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : システムメッセージ処理                                */
/****************************************************************************/
void LCNN_handle_sys_msg(void)
{
    short ret;
    char buffer[64];
    zsys_ddl_smsg_def *sysmsg = (zsys_ddl_smsg_def*)g_myinfo.recvbuf;

    // システムメッセージ判定
    switch (sysmsg->u_z_msg.z_msgnumber[0]) {
    // オープンメッセージ
    case ZSYS_VAL_SMSG_OPEN:
        LCNN_handle_sys_open();
        break;
    // クローズメッセージ
    case ZSYS_VAL_SMSG_CLOSE:
        LCNN_handle_sys_close();
        break;
    // CPU、NODEダウンメッセージ
    case ZSYS_VAL_SMSG_CPUDOWN:
    case ZSYS_VAL_SMSG_REMOTECPUDOWN:
    case ZSYS_VAL_SMSG_NODEDOWN:
        // サーバ停止判定
        ret = COM_STP_JUDGE(&g_myinfo.stp_arg, g_myinfo.recvbuf);
        if (ret != 0 && ret != 1) {
            /*------------------------------------------------*/
            /*    メッセージ出力：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LCNN_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            snprintf(buffer, sizeof(buffer), "%04d", ret);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);

            // 異常終了処理
            LCNN_abend();
        }
        // fall through
    default:
        // nullリプライ
        memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
        break;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.0  LCNN_handle_req_msg                            */
/*  CALLING SEQ.    : void  LCNN_handle_req_msg(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求受信処理                                          */
/****************************************************************************/
void LCNN_handle_req_msg(void)
{
    short ret;
    char buffer[64];
    r701_def* r701 = (r701_def*)g_myinfo.replyinfo.replybuf;
    char rcv_ipc_str[25];
    char rcv_ipc_hex[49];

    /* ローカル変数初期化 */
    memset(rcv_ipc_str,NULL,sizeof(rcv_ipc_str));
    memset(rcv_ipc_hex,NULL,sizeof(rcv_ipc_hex));

    // インターフェースコードに応答コードを設定する
    memcpy(r701->common_header.interface_code, DEF_IPC_IFCD_LCN_NUM_RSP,
           sizeof(DEF_IPC_IFCD_LCN_NUM_RSP)-1);

    // IPC内容精査処理
    ret = LCNN_validate_ipc(g_myinfo.recvbuf);
    // 異常
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：リクエストエラー            */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_REQ_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // ③エラー内容 (20バイト)
        //memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_IPC_ERR, sizeof(DEF_EMS_IPC_ERR)-1);
        switch (ret) {
        case DEF_CHK_IPC_IFCD_ERR:  // インターフェースコード
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_IFCD, sizeof(DEF_EMS_IPC_CHK_IFCD)-1);
            break;
        case DEF_CHK_IPC_DLEN_ERR:  // データ長
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_LEN, sizeof(DEF_EMS_IPC_CHK_LEN)-1);
            break;
        case DEF_CHK_IPC_SITE_ERR:  // 採番システム
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_SITE, sizeof(DEF_EMS_IPC_CHK_SITE)-1);
            break;
        case DEF_CHK_IPC_NW_ERR:    // 場所
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_NW, sizeof(DEF_EMS_IPC_CHK_NW)-1);
            break;
        }
        // ④受信IPC内容 (48バイト)
        memcpy(rcv_ipc_str, g_myinfo.recvbuf, sizeof(rcv_ipc_str)-1);
        LCNN_strtohex(rcv_ipc_str, rcv_ipc_hex, sizeof(rcv_ipc_str)-1);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, rcv_ipc_hex, sizeof(rcv_ipc_hex)-1);
        GFPOGGZ1(&g_oggz1in);

        /*------------------------------------------------*/
        /*    応答バッファ共通ヘッダ編集                  */
        /*------------------------------------------------*/
        // エラーコード
        r701->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r701->common_header.internal_error_code,
               DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // データ長
        r701->common_header.control_data_length = 0;
        g_myinfo.replyinfo.reply_len = sizeof(common_header_def);
    // 正常
    } else {
        /*------------------------------------------------*/
        /*    応答バッファ共通ヘッダ編集                  */
        /*------------------------------------------------*/
        // エラーコード
        r701->common_header.error_code = DEF_IPC_ERRCD_OK;
        // 内部エラーコード
        memcpy(r701->common_header.internal_error_code, DEF_NERR_NOMAL,
               sizeof(DEF_NERR_NOMAL)-1);
        // データ長
        r701->common_header.control_data_length = sizeof(gfplcn_def);
        g_myinfo.replyinfo.reply_len = sizeof(r701_def);
        // GFP内部LCN採番処理
        LCNN_number_gfplcn();
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.4.0  LCNN_reply                                     */
/*  CALLING SEQ.    : void  LCNN_reply(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void LCNN_reply(void)
{
    REPLYX(g_myinfo.replyinfo.replybuf,
           g_myinfo.replyinfo.reply_len,,,
           g_myinfo.replyinfo.reply_code);
}

/****************************************************************************/
/*  FUNCTION        : 2.2.1  LCNN_handle_syopen                             */
/*  CALLING SEQ.    : void  LCNN_handle_syopen(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープンメッセージ処理                                */
/****************************************************************************/
void LCNN_handle_sys_open(void)
{
    char        pname[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];
    char        buffer[64];
    short       pname_len;
    short       openid;
    short       reply_cd;
    short       ret;
    zsys_ddl_smsg_open_reply_def* popenrep_msg =
             (zsys_ddl_smsg_open_reply_def*)g_myinfo.replyinfo.replybuf;

    memset(pname, NULL, sizeof(pname));
    pname_len = 0;
    openid = 0;
    reply_cd = 0;

    // オープン発行元プロセス名取得
    PROCESSHANDLE_DECOMPOSE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender,,,,,,,
        pname, sizeof(pname), &pname_len);
    // オープン発行元プロセス名判定
    ret = PROCESSHANDLE_COMPARE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender,
        g_myinfo.procinfo.ans_phandle);
    if (ret != 0) {
        openid = DEF_OPENID_ANCESTOR;
        reply_cd = 0;
    } else if (memcmp(pname, "$ZL", 3) == 0) {
        openid = DEF_OPENID_ROUT;
        reply_cd = 0;
    } else {
        reply_cd = ZFIL_ERR_SECVIOL;
    }

    if (reply_cd == 0) {
        // サーバ停止判定
        ret = COM_STP_JUDGE(&g_myinfo.stp_arg, g_myinfo.recvbuf);
        if (ret != 0 && ret != 1) {
            /*------------------------------------------------*/
            /*    メッセージ出力：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LCNN_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            snprintf(buffer, sizeof(buffer), "%04d", ret);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);

            // 異常終了処理
            LCNN_abend();
        }
    }

    // リプライメッセージ
    popenrep_msg->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
    popenrep_msg->z_openid = openid;
    g_myinfo.replyinfo.reply_len = zsys_ddl_smsg_open_reply_def_Size;
    g_myinfo.replyinfo.reply_code = reply_cd;
}

/****************************************************************************/
/*  FUNCTION        : 2.2.2  LCNN_handle_syclose                            */
/*  CALLING SEQ.    : void  LCNN_handle_syclose(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : クローズメッセージ処理                                */
/****************************************************************************/
void LCNN_handle_sys_close(void)
{
    short ret;
    char buffer[64];

    // サーバ停止判定
    ret = COM_STP_JUDGE(&g_myinfo.stp_arg, g_myinfo.recvbuf);
    if (ret == 1) {
        g_myinfo.end_flg = DEF_FLG_ON;
    } else if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LCNN_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);

        // 異常終了処理
        LCNN_abend();
    }

    // nullリプライ
    memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
}

/****************************************************************************/
/*  FUNCTION        : 2.3.1  LCNN_validate_ipc                              */
/*  CALLING SEQ.    : void  LCNN_validate_ipc(char* precv)                  */
/*  ARGUMENT        : 1. precv      (I/O) 要求バッファ                      */
/*  RETURN CODE     : 正常: 0 ; 異常: other                                 */
/*  DESCRIPTION     : IPC内容精査処理                                       */
/****************************************************************************/
short LCNN_validate_ipc(char* precv)
{
    c701_def* c701 = (c701_def*)precv;
    const char nwcode[] = {
        DEF_NW_ID_GFP, DEF_NW_ID_VISA, DEF_NW_ID_MASTER, DEF_NW_ID_AMEX,
        DEF_NW_ID_DISCOVER, DEF_NW_ID_JCN, DEF_NW_ID_NYCE, DEF_NW_ID_JLink,
        DEF_NW_ID_UnionPay, DEF_NW_ID_CBSP, DEF_NW_ID_FEP, DEF_NW_ID_DTP
    };
    unsigned int i;

    // 共通ヘッダ：インターフェースコードチェック
    if (memcmp(c701->common_header.interface_code,
        DEF_IPC_IFCD_LCN_NUM_REQ, sizeof(DEF_IPC_IFCD_LCN_NUM_REQ)-1)) {
        return DEF_CHK_IPC_IFCD_ERR;
    }
    // 共通ヘッダ：データ長チェック
    if (c701->common_header.control_data_length !=
        (sizeof(c701_def) - sizeof(common_header_def))) {
        return DEF_CHK_IPC_DLEN_ERR;
    }
    // 採番システムチェック
    if (toupper(c701->site_code) != DEF_SITE_ID_TKY &&
        toupper(c701->site_code) != DEF_SITE_ID_OSK &&
        toupper(c701->site_code) != DEF_FEP_SITE_ID_TKY &&
        toupper(c701->site_code) != DEF_FEP_SITE_ID_OSK) {
        return DEF_CHK_IPC_SITE_ERR;
    }
    // 場所チェック
    for (i = 0; i < sizeof(nwcode); ++i) {
        if (toupper(c701->network_code) == nwcode[i]) {
            break;
        }
    }
    if (i == sizeof(nwcode)) {
        return DEF_CHK_IPC_NW_ERR;
    }

    return DEF_CHK_IPC_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.3.2  LCNN_number_gfplcn                             */
/*  CALLING SEQ.    : void  LCNN_number_gfplcn(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : GFP内部LCN採番処理                                    */
/****************************************************************************/
void LCNN_number_gfplcn(void)
{
    long long tm;
    COM_SDT_arg_2_def com_sdt_arg2;
    COM_SDT_arg_3_def com_sdt_arg3;
    c701_def* c701 = (c701_def*)&g_myinfo.recvbuf;
    r701_def* r701 = (r701_def*)&g_myinfo.replyinfo.replybuf;
    seqnum_def* seqnum = (seqnum_def*)&g_myinfo.seqnum;

    // システム時刻取得
    COM_SDT(DEF_COM_SDT_CHG_TYPE_JST, &com_sdt_arg2, &com_sdt_arg3, &tm);

    /*------------------------------------------------*/
    /*    応答バッファ編集                            */
    /*------------------------------------------------*/
    // 採番システム
    r701->gfplcn.site_code    = c701->site_code;
    // 場所
    r701->gfplcn.network_code = c701->network_code;
    // 予備
    memcpy(&r701->gfplcn.reserve, DEF_GFPLCN_RESERVE_VAL,
           sizeof(r701->gfplcn.reserve));
    // Y
    memcpy(&r701->gfplcn.year, &com_sdt_arg2.yyyy[3],
           sizeof(r701->gfplcn.year));
    // MDH
    LCNN_cnv32d(com_sdt_arg3.hh |
                com_sdt_arg3.dd <<  DEF_SEQNUM_BIT_BOUNDARY |
                com_sdt_arg3.mm << (DEF_SEQNUM_BIT_BOUNDARY << 1),
                r701->gfplcn.mdh,
                sizeof(r701->gfplcn.mdh));
    // MMSS
    memcpy(r701->gfplcn.mmss,     com_sdt_arg2.md, sizeof(com_sdt_arg2.md));
    memcpy(&r701->gfplcn.mmss[2], com_sdt_arg2.ss, sizeof(com_sdt_arg2.ss));
    // 連番
    // 1～13bit：プロセスごとの連番(0～8191)
    // 14～20bit：1/100秒(CC)   要求電文受信時刻の1/100秒
    // 32進数変換
    LCNN_cnv32d((com_sdt_arg3.ms/10) << 13 | seqnum->seqnum,
                r701->gfplcn.seqnum, sizeof(r701->gfplcn.seqnum));

    // 連番更新
    if (seqnum->seqnum == seqnum->seqnum_max) {
        seqnum->seqnum = seqnum->seqnum_min;
    } else {
        seqnum->seqnum++;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.3  LCNN_cnv32d                                    */
/*  CALLING SEQ.    : void  LCNN_cnv32d(unsigned long lval, char* pval      */
/*                                     ,short len)                          */
/*  ARGUMENT        : 1. lval          (I)   変換する値                     */
/*                  : 2. pval          (I/O) 変換後の文字列が格納される配列 */
/*                  : 3. len           (I)   pvalのレングス                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 指定値を下位から5bit区切りで32進数表記に変換する      */
/****************************************************************************/
void LCNN_cnv32d(unsigned long lval, char* pval, short len)
{
    const char* const cnv32str = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    short i;

    // 32進数変換
    for (i = len-1; i >= 0; --i, lval >>= DEF_SEQNUM_BIT_BOUNDARY) {
        pval[i] = cnv32str[(lval & DEF_SEQNUM_BIT_MSK_VALUE)];
    }
}

/****************************************************************************/
/*  FUNCTION        : 1.1.1  LCNN_procnamecmp                               */
/*  CALLING SEQ.    : void  LCNN_procnamecmp(const procinfo_def *,          */
/*                                           const char *, size_t)          */
/*  ARGUMENT        : 1. p1            (I) プロセス情報                     */
/*                  : 2. p2            (I) 文字列2                          */
/*                  : 3. n             (I) p2バッファ長                     */
/*  RETURN CODE     : 一致 0 ; 不一致: other                                */
/*  DESCRIPTION     : プロセス情報と文字列2をノード修飾の有無にかかわらず   */
/*                    プロセス名が一致するかを判定する                      */
/****************************************************************************/
int LCNN_procnamecmp(const procinfo_def *p1, const char *p2, size_t n)
{
    int i;
    char procname[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];

    for(i = 0; i < n; ++i) {
        // ドット判定
        if (*(p2 + i) == '.') {
            break;
        }
    }
    memset(procname, NULL, sizeof(procname));
    // ドットが含まれない場合
    if (i == n) {
        memcpy(procname, p1->my_pname, strlen(p1->my_pname));
    // ドットが含まれる場合
    } else {
        snprintf(procname, sizeof(procname), "%s.%s",
                 p1->my_nodename, p1->my_pname);
    }
    return LCNN_strncmpi(procname, p2, strlen(procname));
}

/****************************************************************************/
/*  FUNCTION        : 1.1.2  LCNN_strncmpi                                  */
/*  CALLING SEQ.    : void  LCNN_strncmpi(const char *, const char *,       */
/*                                        size_t)                           */
/*  ARGUMENT        : 1. p1            (I) 文字列1                          */
/*                  : 2. p2            (I) 文字列2                          */
/*                  : 3. n             (I) 比較文字列長                     */
/*  RETURN CODE     : 一致 0 ; 不一致: other                                */
/*  DESCRIPTION     : 大文字小文字区別しないことを除けばstrncmpと同じ       */
/****************************************************************************/
int LCNN_strncmpi(const char *p1, const char *p2, size_t n)
{
    for( ; n && tolower(*p1) == tolower(*p2); p1++, p2++, n--) {
        if (*p1 == '\0') {
            return 0;
        }
    }
    if (n == 0) {
        return 0;
    } else {
        return (tolower(*p1) - tolower(*p2));
    }
}

/****************************************************************************/
/*  FUNCTION        : 9.0.0  LCNN_setmsgid                                  */
/*  CALLING SEQ.    : void  LCNN_setmsgid(short msgid)                      */
/*  ARGUMENT        : 1. msgid         (I) メッセージID                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : EMS出力モジュールI/F 任意メッセージ部初期化           */
/****************************************************************************/
void LCNN_setmsgid(short msgid)
{
    int i;
    int item_size;
    int item_cnt;
    char buffer[64];

    // GFP通信制御メッセージ番号
    snprintf(buffer, sizeof(buffer), "%05d", msgid);
    memcpy(&g_oggz1in.emsinf.msgid, buffer, strlen(buffer));
    // メッセージ通知区分
    // プロセス起動、プロセス正常終了
    if (msgid == DEF_EVT_PROC_START || msgid == DEF_EVT_PROC_NORMAL_END) {
        g_oggz1in.emsinf.emsgkinf.msgttkb = '*';
    } else {
        g_oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    }
    // ②サーバークラス論理ID (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[0].msgtbl_vl,
           DEF_MY_SCID, sizeof(DEF_MY_SCID)-1);

    item_size = sizeof(g_oggz1in.emsinf.emsnninf.msgtbl[0]);
    item_cnt =  sizeof(g_oggz1in.emsinf.emsnninf.msgtbl) / item_size;
    for (i = 1; i < item_cnt; i++) {
        memset(&g_oggz1in.emsinf.emsnninf.msgtbl[i], ' ', item_size);
    }
}

/****************************************************************************/
/*  FUNCTION        : 9.0.0  LCNN_strtohex                                  */
/*  CALLING SEQ.    : void LCNN_strtohex(char* input,char* output,short len)*/
/*  ARGUMENT        : 1. input  (I)  文字列                                 */
/*                  : 2. output (O)  hex                                    */
/*                  : 3. len    (I)  処理長                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 文字列のhex変換モジュール                             */
/****************************************************************************/
void LCNN_strtohex(const char* input,char* output,short len)
{
    int i;
    //char to hexを文字数分繰り返す
    for (i=0;i<len;i++){
        sprintf(output + (i * 2),"%02X",input[i]);
    }
    output[i * 2 ] = NULL;
}

