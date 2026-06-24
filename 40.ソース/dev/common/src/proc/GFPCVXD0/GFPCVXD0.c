/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXD0                                    */
/*        FUNCTION          ････ コマンドサーバ                              */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-12-06                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/12/06 (J0680)新規作成                               */
/*  1.1  M.Matumoto 2026/05/20 (J0680)DR運用対応                             */
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
#include  <limits.h>   nolist
#include  <tal.h>      nolist
#include  <cextdecs.h> nolist
#include  <zspic>      nolist
#include  <zfilc>      nolist
/* USER HEADER */
#include "ems.h"       nolist      // ems
#include "GFPOGGZ4_traceout.h" nolist   // trace
#include "errcd.h"     nolist      // error code
#include "GFPCGX40.h"  nolist      // PATHSENDモジュール 
#include "GFPCGXD0.h"  nolist      // ASSIGN情報取得
#include "GFPCVXD0.h"  nolist      // コマンドサーバ
#include "vproc.h"     nolist      // vproc

// GLOBAL
static myinfo_def g_myinfo;
static fileinfo_def g_fileinfo[EFILEKINDNUM];
static destinfo_def g_dest_connctrl[ESITENUM][DEF_CONNCTRL_NUM_MAX];
static destinfo_def g_dest_cmdst[ESITENUM];
static destinfo_def g_dest_process[ESITENUM][EPROCNUM][DEF_PROC_SC_NUM_MAX];
static gflindata_def g_gflindata[ESITENUM][DEF_CONN_GRP_NUM_MAX];
static gfnwidata_def g_gfnwidata[ESITENUM][DEF_STA_GRP_NUM_MAX];
static short g_mnglyr[ESITENUM][EMNGLYRNUM];

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
    CMDS_init();

    /*------------------------------------------------*/
    /*    主処理                                      */
    /*------------------------------------------------*/
    CMDS_main();

    /*------------------------------------------------*/
    /*    終了処理                                    */
    /*------------------------------------------------*/
    CMDS_final();

    return 0;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  CMDS_init                                      */
/*  CALLING SEQ.    : void  CMDS_init(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void CMDS_init(void)
{
    short ret;
    char buffer[64];
    char sub_prog_sts[2];
    COM_IOM_arg_5_def iom_arg5;
    COM_IOM_arg_6_def iom_arg6;
    lk_zac2001r_arg_1_def trace_if;

    // 変数初期化
    memset(&g_myinfo, NULL, sizeof(myinfo_def));
    PROCESSHANDLE_NULLIT_(g_myinfo.procinfo.my_phandle);
    PROCESSHANDLE_NULLIT_(g_myinfo.procinfo.ans_phandle);
    g_myinfo.recv_fno = -1;
    g_myinfo.end_flg = DEF_FLG_OFF;
    g_myinfo.iom_gfphi.file_no = -1;

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
    TRACEOUT((char *)&trace_if);

    // 汎用COM_IOMトレース情報設定
    memset(&g_myinfo.iom_trace, NULL, sizeof(COM_IOM_arg_3_def));
    memcpy(g_myinfo.iom_trace.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    memcpy(g_myinfo.iom_trace.file_id, DEF_ASN_TRACE,  sizeof(DEF_ASN_TRACE)-1);
    memcpy(g_myinfo.iom_trace.file_name, EXTRACEFILENAME, strlen(EXTRACEFILENAME));

    // パラメータ取得処理
    CMDS_get_params();
    // 物理名情報ファイル読込
    CMDS_load_GFPHI();
    // 回線管理ファイル読込
    CMDS_load_GFLIN();
    // NW情報ファイル読込
    CMDS_load_GFNWI();

    // $RECEIVEオープン
    ret = FILE_OPEN_(DEF_RECVFILE_NAME, (short)sizeof(DEF_RECVFILE_NAME)-1,
                     &g_myinfo.recv_fno,,,1,DEF_RECV_DEPTH_MAX,); 
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：ファイルI/Oエラー           */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_FILE_IO_ERR);
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
        CMDS_abend();
    }

    // サーバ停止判定初期化
    COM_STP_INIT(&g_myinfo.stp_arg);
    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス起動                */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    CMDS_setmsgid(DEF_EVT_PROC_START);
    // 内部エラーコード
    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1);
    // ③プロセス名 (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&g_oggz1in);
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0  CMDS_main                                      */
/*  CALLING SEQ.    : void  CMDS_main(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void CMDS_main(void)
{
    while (g_myinfo.end_flg != DEF_FLG_ON) {
        // RECEIVE処理
        CMDS_read_recv();

        // イベント解析
        switch (g_myinfo.iocomp.errno) {
        // システムメッセージ処理
        case ZFIL_ERR_SYSMESS:
            CMDS_handle_sys_msg();
            break;
        // 要求受信処理
        case ZFIL_ERR_OK:
            CMDS_handle_req_msg();
            break;
        default:
            // nullリプライ
            memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
            break;
        }

        // リプライ処理
        CMDS_reply();
    }
}

/****************************************************************************/
/*  FUNCTION        : 3.0.0  CMDS_final                                     */
/*  CALLING SEQ.    : void  CMDS_final(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void CMDS_final(void)
{
    char sub_prog_sts[2];
    COM_IOM_arg_5_def iom_arg5;
    COM_IOM_arg_6_def iom_arg6;
    lk_zac2001r_arg_1_def trace_if;

    // $RECEIVEクローズ
    FILE_CLOSE_(g_myinfo.recv_fno);

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス正常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    CMDS_setmsgid(DEF_EVT_PROC_NORMAL_END);
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
/*  FUNCTION        : 8.0.0  CMDS_abend                                     */
/*  CALLING SEQ.    : void  CMDS_abend(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void CMDS_abend(void)
{
    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス異常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    CMDS_setmsgid(DEF_EVT_PROC_ABNORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&g_oggz1in);

    // プロセス終了
    PROCESS_STOP_(, , DEF_PROC_ABNORMAL_END);
}

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CMDS_get_params                                */
/*  CALLING SEQ.    : void  CMDS_get_params(void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void CMDS_get_params(void)
{
    short ret, fnm_len;
    char asn_name[32];
    char buffer[64];
    char cnvstr[64];
    char fname[ZSYS_VAL_LEN_FILENAME+1];
    // EMSリンケージ：NW識別変換
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
    CMDS_getparam(DEF_MSG_SRV_NAME, buffer, sizeof(buffer));
    memcpy (g_oggz1in.uytrminf.uytrmsrv, buffer, strlen(buffer));
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(buffer));
    memcpy (g_oggz1in.uytrminf.uytrmsrvlen, cnvstr, strlen(cnvstr));

    // 運用監視端末出力サーバ・PATHMON名取得
    CMDS_getparam(DEF_MSG_MON_NAME, buffer, sizeof(buffer));
    memcpy (g_oggz1in.uytrminf.uytrmmon, buffer, strlen(buffer));
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(buffer));
    memcpy (g_oggz1in.uytrminf.uytrmmonlen, cnvstr, strlen(cnvstr));

    // PATHSENDタイマー取得
    CMDS_getparam(DEF_PSEND_TIMER_10MSECOND, buffer, sizeof(buffer));
    g_myinfo.psd_timer = atol(buffer);
    // EMSのタイマー値：秒単位(10msec → sec切り上げ)
    snprintf(cnvstr, sizeof(cnvstr), "%04d", (int)ceil((double)g_myinfo.psd_timer / 100));
    memcpy (g_oggz1in.uytrminf.proctimer, cnvstr, strlen(cnvstr));

    // PATHSENDリトライ回数取得
    CMDS_getparam(DEF_PSEND_RETRY_CNT, buffer, sizeof(buffer));
    g_myinfo.psd_retrycnt = (short)atoi(buffer);

    // 自・親プロセス情報取得
    ret = COM_PRC(&g_myinfo.procinfo);
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：プロシージャコールエラー    */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_PROCEDURE_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
        // ③プロシージャ名(40バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.err_pname, strlen(g_myinfo.procinfo.err_pname));
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }
    memcpy(g_oggz1in.emsinf.emsgkinf.trmnm, g_myinfo.procinfo.my_pname, strlen(g_myinfo.procinfo.my_pname));

    // 自サーバクラス論理ID取得
    CMDS_getparam(DEF_SRV_LOGICAL_ID, buffer, sizeof(buffer));
    memcpy(g_myinfo.site_id, &buffer[DEF_SC_OFFSET_SITE], sizeof(g_myinfo.site_id));
    memcpy(g_myinfo.network_id, &buffer[DEF_SC_OFFSET_NW], sizeof(g_myinfo.network_id));
    memcpy(g_myinfo.group_id, &buffer[DEF_SC_OFFSET_GRP], sizeof(g_myinfo.group_id));
    memcpy(g_myinfo.serverclass_id, &buffer[DEF_SC_OFFSET_SCKND], DEF_SC_KIND_LEN);
    memcpy(&g_myinfo.serverclass_id[DEF_SC_KIND_LEN], &buffer[DEF_SC_OFFSET_SCNUM],
           sizeof(g_myinfo.serverclass_id) - DEF_SC_KIND_LEN);
    // NW識別を変換してEMSリンケージへ設定
    for (int i = 0; i < sizeof(cnvnw)/sizeof(struct __cnvnw); ++i) {
        if (cnvnw[i].nwid == g_myinfo.network_id[0]) {
            memcpy(g_oggz1in.emsinf.emsgkinf.h_nw_kbn, cnvnw[i].nwdiv, strlen(cnvnw[i].nwdiv));
            break;
        }
    }

    // ファイルIOタイマー取得
    CMDS_getparam(DEF_FILE_IO_TIMER_10MSECOND, buffer, sizeof(buffer));
    g_myinfo.fio_timer = atol(buffer);

    // プロセスW/Rタイマー取得
    CMDS_getparam(DEF_PROC_IO_TIMER_10MSECOND, buffer, sizeof(buffer));
    g_myinfo.pwr_timer = atol(buffer);

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
        CMDS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_ASN_GFPHI,
               sizeof(DEF_ASN_GFPHI)-1);
        // ④エラーコード(4バイト)
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }

    // 物理名情報ファイル用COM_IOMファイル情報設定
    memset(&g_myinfo.iom_gfphi, NULL, sizeof(COM_IOM_arg_4_def));
    memcpy(g_myinfo.iom_gfphi.file_id, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
    memcpy(g_myinfo.iom_gfphi.file_name, fname, strlen(fname));
}

/****************************************************************************/
/*  FUNCTION        : 1.2.0  CMDS_load_GFPHI                                */
/*  CALLING SEQ.    : void  CMDS_load_GFPHI()                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 物理名情報ファイル読込  エラー時はABEND               */
/****************************************************************************/
void CMDS_load_GFPHI()
{
    short               site, mysite, kind, scmn, rec_cnt, ret;
    char                sub_prog_sts[2];
    COM_IOM_arg_4_def   iom_arg4;
    COM_IOM_arg_5_def   iom_arg5;
    COM_IOM_arg_6_def   iom_arg6;
    db_gfphi_def        gfphi_pk;                 // 物理名情報ファイルPK
    db_gfphi_def*       pgfphi_rec;               // 物理名情報ファイルレコード
    destinfo_def*       destinfo;
    static const char site_id[ESITENUM] = {DEF_SITE_ID_TKY, DEF_SITE_ID_OSK};
    static const char *logical_fname[] = {DEF_FL_LIN_STS, DEF_FL_CEN_STS, DEF_FL_ECH_STS, DEF_FL_LIN_MG, DEF_FL_NW_INFO};
    static const char *conctrl[ECONSCNUM] = {DEF_SC_LISTEN, DEF_SC_CON_SVR, DEF_SC_CON_CLT};
    static const char *procsc[EPROCNUM] = {DEF_SC_FURI_I, DEF_SC_FURI_O, DEF_SC_LOG_OUT};
    static const char *procpr[EPROCNUM] = {DEF_PRC_FURI_I, DEF_PRC_FURI_O, DEF_PRC_LOG_OUT};
        
    // 物理名情報ファイルオープン
    // トレース情報.ファイルI/O種別設定
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_OPEN, sizeof(DEF_COM_IOM_FIO_OPEN)-1);
    // 入力情報.I/Oタイマ設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.io_timer = g_myinfo.fio_timer;
    memset(&iom_arg6, NULL, sizeof(iom_arg6));
    ret = COM_IOM(DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }

    // 物理名情報ファイルから対象ファイル物理名を取得する
    // Key設定
    memcpy(&gfphi_pk.pri_key.nw_id, g_myinfo.network_id, sizeof(gfphi_pk.pri_key.nw_id));
    memcpy(gfphi_pk.pri_key.grp_id, g_myinfo.group_id, sizeof(gfphi_pk.pri_key.grp_id));
    memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, '}',
           sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num, '}',
           sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, '}',
           sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num));
    memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, '0',
           sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
    memset(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num, '0',
           sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));
    // COM_IOM入力情報設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = sizeof(gfphi_pk.pri_key);
    iom_arg5.compare_len = sizeof(gfphi_pk.pri_key);
    iom_arg5.positioning_mode = DEF_COM_IOM_EXACT;
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.io_timer = g_myinfo.fio_timer;
    iom_arg5.rec_len = db_gfphi_def_Size;

    if (g_myinfo.site_id[0] == DEF_SITE_ID_TKY) {
        mysite = ESITEEAST;
    } else {
        mysite = ESITEWEST;
    }
    memset(g_fileinfo, NULL, sizeof(g_fileinfo));

    // サイト
    for (site = ESITEEAST; site < ESITENUM; ++site) {
        gfphi_pk.pri_key.site_id = site_id[site];
        for (kind = EFILEGCLST; kind < EFILEKINDNUM; ++kind) {
            if (site == ESITEEAST) {
                // 回線ステータスファイル
                if (kind == EFILEGCLST) {
                    g_fileinfo[kind].logical_fname = logical_fname[kind];
                    g_fileinfo[kind].rec_len = db_gclst_def_Size;
                    g_fileinfo[kind].rec_max =  (short)(DEF_CMD_DATA_SIZE / sizeof(gclst_data_def));
                // 局状態管理ファイル
                } else if (kind == EFILEGCSST) {
                    g_fileinfo[kind].logical_fname = logical_fname[kind];
                    g_fileinfo[kind].rec_len = db_gcsst_def_Size;
                    g_fileinfo[kind].rec_max =  DEF_CMD_DATA_SIZE / sizeof(gcsst_data_def);
                // エコー状態管理ファイル
                } else if (kind == EFILEGCEST) {
                    g_fileinfo[kind].logical_fname = logical_fname[kind];
                    g_fileinfo[kind].rec_len = db_gcest_def_Size;
                    g_fileinfo[kind].rec_max =  DEF_CMD_DATA_SIZE / sizeof(gcest_data_def);
                // 回線管理ファイル
                } else if (kind == EFILEGFLIN) {
                    g_fileinfo[kind].logical_fname = logical_fname[kind];
                    g_fileinfo[kind].rec_len = db_gflin_def_Size;
                // NW情報ファイル
                } else if (kind == EFILEGFNWI) {
                    g_fileinfo[kind].logical_fname = logical_fname[kind];
                    g_fileinfo[kind].rec_len = db_gfnwi_def_Size;
                }
            }
            // 回線管理ファイル、NW情報ファイルは自サイトのみ
            if ((kind == EFILEGFLIN || kind == EFILEGFNWI) && site != mysite) {
                continue;
            }
            // ファイル論理名
            memcpy(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, logical_fname[kind],
                    sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));
            memcpy(iom_arg5.key_value, &gfphi_pk.pri_key, sizeof(gfphi_pk.pri_key));
    
            // READ開始
            memset(&iom_arg6, NULL, sizeof(iom_arg6));
            memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
            ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
            // EOF判定
            if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                // 内部エラーコード
                memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                // ③ファイル論理名 (8バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
                // ④キー (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, iom_arg5.key_value, iom_arg5.key_len);
                // ⑤エラー項目/エラー理由 (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
                GFPOGGZ1(&g_oggz1in);
                // 異常終了処理
                CMDS_abend();
            // エラー判定
            } else if (ret != 0) {
                /*------------------------------------------------*/
                /*    メッセージ編集：共通モジュールエラー        */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
                // 内部エラーコード
                memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
                // ③モジュールID(8バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
                // ④エラーコード(4バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
                GFPOGGZ1(&g_oggz1in);
                // 異常終了処理
                CMDS_abend();
            }
            // 対象ファイル名取得
            pgfphi_rec = (db_gfphi_def*)iom_arg6.rec_area;
            memcpy(g_fileinfo[kind].name[site], pgfphi_rec->prc_file_info.prc_file_name,
                    sizeof(g_fileinfo[kind].name[site])-1);
            CMDS_strtrim(g_fileinfo[kind].name[site]);
        }
    }

    // 物理名情報ファイルから転送先のサーバクラス情報、プロセス情報を取得する
    // Key設定
    memcpy(&gfphi_pk.pri_key.nw_id, g_myinfo.network_id, sizeof(gfphi_pk.pri_key.nw_id));
    memcpy(gfphi_pk.pri_key.grp_id, g_myinfo.group_id, sizeof(gfphi_pk.pri_key.grp_id));
    memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num, '0', sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, '0', sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num));
    // COM_IOM入力情報設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = sizeof(gfphi_pk.pri_key);
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.io_timer = g_myinfo.fio_timer;
    iom_arg5.rec_len = db_gfphi_def_Size;

    memset(g_dest_connctrl, NULL, sizeof(g_dest_connctrl));

    // サイト
    for (site = ESITEEAST; site < ESITENUM; ++site) {
        gfphi_pk.pri_key.site_id = site_id[site];
        /*------------------------------------------------*/
        /*    回線系                                      */
        /*------------------------------------------------*/
        rec_cnt = 0;
        memset(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, NULL, sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, NULL, sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, NULL, sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num, NULL, sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));
        iom_arg5.compare_len = sizeof(gfphi_pk.pri_key.site_id) + sizeof(gfphi_pk.pri_key.nw_id) +
            sizeof(gfphi_pk.pri_key.grp_id) + sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind);
        iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
        destinfo = g_dest_connctrl[site];
        for (kind = 0; kind < ECONSCNUM; ++kind) {
            // サーバクラス論理名
            memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, conctrl[kind], sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
            memcpy(iom_arg5.key_value, &gfphi_pk.pri_key, sizeof(gfphi_pk.pri_key));
            // READ開始
            memset(&iom_arg6, NULL, sizeof(iom_arg6));
            memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
            ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
            while (ret == 0) {
                // EOF判定
                if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                    break;
                }
                // 最大数判定
                if (rec_cnt >= DEF_CONNCTRL_NUM_MAX) {
                    /*------------------------------------------------*/
                    /*    メッセージ編集：設定情報エラー              */
                    /*------------------------------------------------*/
                    // 任意メッセージ部初期化
                    CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                    // 内部エラーコード
                    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                    // ③ファイル論理名 (8バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
                    // ④キー (40バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfphi_pk.pri_key, iom_arg5.compare_len);
                    // ⑤エラー項目/エラー理由 (40バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_EXC_MAX_REC_CNT, sizeof(DEF_EMS_EXC_MAX_REC_CNT)-1);
                    GFPOGGZ1(&g_oggz1in);
                    // 異常終了処理
                    CMDS_abend();
                }
                // 対象サーバクラス情報取得
                pgfphi_rec = (db_gfphi_def*)iom_arg6.rec_area;
                memcpy(destinfo[rec_cnt].srv_cls_kind, conctrl[kind], sizeof(destinfo[rec_cnt].srv_cls_kind));
                memcpy(destinfo[rec_cnt].srv_cls_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,
                        sizeof(destinfo[rec_cnt].srv_cls_num));
                memcpy(destinfo[rec_cnt].srv_cls_mlt_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num,
                        sizeof(destinfo[rec_cnt].srv_cls_mlt_num));
                memcpy(destinfo[rec_cnt].pathmon_name, pgfphi_rec->srv_cls_info.pathmon_name, sizeof(destinfo[rec_cnt].pathmon_name));
                memcpy(destinfo[rec_cnt].srv_cls_name, pgfphi_rec->srv_cls_info.srv_cls_name, sizeof(destinfo[rec_cnt].srv_cls_name));
                ++rec_cnt;
                // NEXTREAD
                memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_READ, sizeof(DEF_COM_IOM_FIO_READ)-1);
                ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
            }
            // エラー判定
            if (ret != 0) {
                /*------------------------------------------------*/
                /*    メッセージ編集：共通モジュールエラー        */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
                // 内部エラーコード
                memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
                // ③モジュールID(8バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                    DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
                // ④エラーコード(4バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
                    sizeof(sub_prog_sts));
                GFPOGGZ1(&g_oggz1in);
                // 異常終了処理
                CMDS_abend();
            }
        }
        // リスナー、コネクション制御(サーバ・クライアント)で、対象レコードなし
        if (rec_cnt == 0) {
            /*------------------------------------------------*/
            /*    メッセージ編集：設定情報エラー              */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
            // ③ファイル論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
            // ④キー (40バイト) キーは、コネクション制御(クライアント)検索時のもの
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfphi_pk.pri_key, iom_arg5.compare_len);
            // ⑤エラー項目/エラー理由 (40バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }

        /*------------------------------------------------*/
        /*    制御電文振分                                */
        /*------------------------------------------------*/
        memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof("0000")-1);
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, '}', sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, '}', sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num, '}', sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));
        memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CTRL_FURI, sizeof(DEF_SC_CTRL_FURI)-1);
        memcpy(iom_arg5.key_value, &gfphi_pk.pri_key, sizeof(gfphi_pk.pri_key));
        iom_arg5.compare_len = sizeof(gfphi_pk.pri_key);
        iom_arg5.positioning_mode = DEF_COM_IOM_EXACT;
        // READ開始
        memset(&iom_arg6, NULL, sizeof(iom_arg6));
        memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
        ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
        // エラー判定
        if (ret != 0) {
            /*------------------------------------------------*/
            /*    メッセージ編集：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
            // ④エラーコード(4バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        // EOF判定
        } else if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
            /*------------------------------------------------*/
            /*    メッセージ編集：設定情報エラー              */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
            // ③ファイル論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
            // ④キー (40バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfphi_pk.pri_key, sizeof(gfphi_pk.pri_key));
            // ⑤エラー項目/エラー理由 (40バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }
        // 対象サーバクラス情報取得
        pgfphi_rec = (db_gfphi_def*)iom_arg6.rec_area;
        memcpy(g_dest_cmdst[site].srv_cls_kind, DEF_SC_CTRL_FURI, sizeof(DEF_SC_CTRL_FURI)-1);
        memcpy(g_dest_cmdst[site].srv_cls_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, sizeof(g_dest_cmdst[site].srv_cls_num));
        memcpy(g_dest_cmdst[site].srv_cls_mlt_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, sizeof(g_dest_cmdst[site].srv_cls_mlt_num));
        memcpy(g_dest_cmdst[site].domain_name, pgfphi_rec->srv_cls_info.domain_name, sizeof(g_dest_cmdst[site].domain_name));
        memcpy(g_dest_cmdst[site].srv_cls_name, pgfphi_rec->srv_cls_info.srv_cls_name, sizeof(g_dest_cmdst[site].srv_cls_name));
        /*------------------------------------------------*/
        /*    プロセス：電文振分(I/O)、ログ出力           */
        /*------------------------------------------------*/
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, '0', sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
        memset(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num, '0', sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));
        iom_arg5.compare_len = sizeof(gfphi_pk.pri_key) - sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num);
        iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
        for (kind = 0; kind < EPROCNUM; ++kind) {
            rec_cnt = 0;
            destinfo = g_dest_process[site][kind];
            // サーバクラス論理名
            memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, procsc[kind], sizeof(gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
            // プロセス論理名
            memcpy(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, procpr[kind], sizeof(gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));
            // サーバクラス冗長化番号が”0000”、”0001”のレコードを検索
            for (scmn = 0; scmn < 2; ++scmn) {
                if (scmn == 0) {
                    memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof("0000")-1);
                } else {
                    memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, "0001", sizeof("0001")-1);
                }
                // READ開始
                memcpy(iom_arg5.key_value, &gfphi_pk.pri_key, sizeof(gfphi_pk.pri_key));
                memset(&iom_arg6, NULL, sizeof(iom_arg6));
                memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
                ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
                while (ret == 0) {
                    // EOF判定
                    if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                        break;
                    }
                    // 最大数判定
                    if (rec_cnt >= DEF_PROC_SC_NUM_MAX) {
                        /*------------------------------------------------*/
                        /*    メッセージ編集：設定情報エラー              */
                        /*------------------------------------------------*/
                        // 任意メッセージ部初期化
                        CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                        // 内部エラーコード
                        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                        // ③ファイル論理名 (8バイト)
                        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
                        // ④キー (40バイト)
                        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfphi_pk.pri_key, iom_arg5.compare_len);
                        // ⑤エラー項目/エラー理由 (40バイト)
                        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_EXC_MAX_REC_CNT, sizeof(DEF_EMS_EXC_MAX_REC_CNT)-1);
                        GFPOGGZ1(&g_oggz1in);
                        // 異常終了処理
                        CMDS_abend();
                    }
                    // 対象サーバクラス情報取得
                    pgfphi_rec = (db_gfphi_def*)iom_arg6.rec_area;
                    memcpy(destinfo[rec_cnt].srv_cls_kind, procsc[kind], sizeof(destinfo[rec_cnt].srv_cls_kind));
                    memcpy(destinfo[rec_cnt].srv_cls_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, sizeof(destinfo[rec_cnt].srv_cls_num));
                    memcpy(destinfo[rec_cnt].srv_cls_mlt_num, pgfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, sizeof(destinfo[rec_cnt].srv_cls_mlt_num));
                    memcpy(destinfo[rec_cnt].prc_file_name, pgfphi_rec->prc_file_info.prc_file_name, sizeof(destinfo[rec_cnt].prc_file_name));
                    ++rec_cnt;
                    // NEXTREAD
                    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_READ, sizeof(DEF_COM_IOM_FIO_READ)-1);
                    ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
                }
                // エラー判定
                if (ret != 0) {
                    /*------------------------------------------------*/
                    /*    メッセージ編集：共通モジュールエラー        */
                    /*------------------------------------------------*/
                    // 任意メッセージ部初期化
                    CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
                    // 内部エラーコード
                    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
                    // ③モジュールID(8バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
                    // ④エラーコード(4バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
                    GFPOGGZ1(&g_oggz1in);
                    // 異常終了処理
                    CMDS_abend();
                }
            }
            // 各プロセス単位で、対象レコードなし
            if (rec_cnt == 0) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                // 内部エラーコード
                memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                // ③ファイル論理名 (8バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
                // ④キー (40バイト) キーは、サーバクラス冗長化番号”0000”を設定
                memcpy(gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof("0000")-1);
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfphi_pk.pri_key, iom_arg5.compare_len);
                // ⑤エラー項目/エラー理由 (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
                GFPOGGZ1(&g_oggz1in);
                // 異常終了処理
                CMDS_abend();
            }
        }
    }
    // 物理名情報ファイルクローズ
    // トレース情報.ファイルI/O種別設定
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_CLOSE, sizeof(DEF_COM_IOM_FIO_CLOSE)-1);
    COM_IOM(DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &g_myinfo.iom_trace, &g_myinfo.iom_gfphi, &iom_arg5, &iom_arg6);
}

/****************************************************************************/
/*  FUNCTION        : 1.3.0  CMDS_load_GFLIN                                */
/*  CALLING SEQ.    : void  CMDS_load_GFLIN()                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線管理ファイル読込  エラー時はABEND                 */
/****************************************************************************/
void CMDS_load_GFLIN()
{
    short               ret;
    short               site;
    short               rec_cnt;
    char                sub_prog_sts[2];
    gflin_pkey_def      gflin_pk;
    db_gflin_def*       gflin_rec;                // 回線管理ファイル
    COM_IOM_arg_4_def   iom_arg4;
    COM_IOM_arg_5_def   iom_arg5;
    COM_IOM_arg_6_def   iom_arg6;
    gflindata_def*      lindt;
    static const char   site_id[ESITENUM] = {DEF_SITE_ID_TKY, DEF_SITE_ID_OSK};

    // 自N/W識別、グループ識別
    gflin_pk.nw_name = g_myinfo.network_id[0];
    memcpy(gflin_pk.group_name, g_myinfo.group_id, sizeof(gflin_pk.group_name));
    memset(g_gflindata, NULL, sizeof(g_gflindata));

    // COM_IOMファイル情報設定
    memset(&iom_arg4, NULL, sizeof(iom_arg4));
    memcpy(iom_arg4.file_id, DEF_FL_LIN_MG, sizeof(DEF_FL_LIN_MG)-1);

    // COM_IOM入力情報設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = sizeof(gflin_pkey_def);
    iom_arg5.compare_len = sizeof(gflin_pk.site_name) + sizeof(gflin_pk.nw_name) + sizeof(gflin_pk.group_name);
    iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.io_timer = g_myinfo.fio_timer;
    iom_arg5.rec_len = db_gflin_def_Size;

    // 回線管理ファイルオープン
    if (g_myinfo.site_id[0] == DEF_SITE_ID_TKY) {
        site = ESITEEAST;
    } else {
        site = ESITEWEST;
    }
    memset(iom_arg4.file_name, ' ', sizeof(iom_arg4.file_name));
    memcpy(iom_arg4.file_name, g_fileinfo[EFILEGFLIN].name[site], strlen(g_fileinfo[EFILEGFLIN].name[site]));
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_OPEN, sizeof(DEF_COM_IOM_FIO_OPEN)-1);
    ret = COM_IOM(DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }
    // サイト
    for (site = ESITEEAST; site < ESITENUM; ++site) {
        gflin_pk.site_name = site_id[site];
        memcpy(iom_arg5.key_value, &gflin_pk, iom_arg5.compare_len);
        memset(&iom_arg6, NULL, sizeof(iom_arg6));
        rec_cnt = 0;
        // READ開始
        memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
        ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
        while (ret == 0) {
            // EOF判定
            if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                break;
            }
            // 最大数判定
            if (rec_cnt >= DEF_CONN_GRP_NUM_MAX) {
                /*------------------------------------------------*/
                /*    メッセージ編集：設定情報エラー              */
                /*------------------------------------------------*/
                // 任意メッセージ部初期化
                CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                // 内部エラーコード
                memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                // ③ファイル論理名 (8バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_LIN_MG, sizeof(DEF_FL_LIN_MG)-1);
                // ④キー (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gflin_pk, iom_arg5.compare_len);
                // ⑤エラー項目/エラー理由 (40バイト)
                memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_EXC_MAX_REC_CNT, sizeof(DEF_EMS_EXC_MAX_REC_CNT)-1);
                GFPOGGZ1(&g_oggz1in);
                // 異常終了処理
                CMDS_abend();
            }
            lindt = &g_gflindata[site][rec_cnt];
            // 処理サーバクラス論理ID取得
            gflin_rec = (db_gflin_def*)iom_arg6.rec_area;
            // 有効レコード
            if (gflin_rec->invalid_flg == DEF_GFLIN_VALID_REC) {
                memcpy(&lindt->pri_key, &gflin_rec->pri_key, sizeof(gflin_pkey_def));
                memcpy(lindt->srv_cls_kind, gflin_rec->alt1_key_info.srv_cls_id.srv_cls_kind, sizeof(lindt->srv_cls_kind));
                memcpy(lindt->srv_cls_num, gflin_rec->alt1_key_info.srv_cls_id.srv_cls_num, sizeof(lindt->srv_cls_num));
                lindt->operation_id = gflin_rec->operation_id;
                ++rec_cnt;
            }
            // NEXTREAD
            memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_READ, sizeof(DEF_COM_IOM_FIO_READ)-1);
            ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
        }
        if (ret) {
            /*------------------------------------------------*/
            /*    メッセージ出力：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
            // ④エラーコード(4バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }
    }
    // 回線管理ファイルクローズ
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_CLOSE, sizeof(DEF_COM_IOM_FIO_CLOSE)-1);
    COM_IOM(DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
}

/****************************************************************************/
/*  FUNCTION        : 1.4.0  CMDS_load_GFNWI                                */
/*  CALLING SEQ.    : void  CMDS_load_GFNWI()                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : NW情報ファイル読込  エラー時はABEND                   */
/****************************************************************************/
void CMDS_load_GFNWI()
{
    short               site;
    short               kind;
    short               is_exists_mnglyr;
    short               rec_cnt;
    short               ret;
    char                mng_lyr;
    char                sub_prog_sts[2];
    gfnwidata_def       gfnwi_pk;
    gfnwidata_def*      nwidt;
    db_gfnwi_def*       gfnwi_rec;                // NW情報ファイル
    gflindata_def*      lindt;
    COM_IOM_arg_4_def   iom_arg4;
    COM_IOM_arg_5_def   iom_arg5;
    COM_IOM_arg_6_def   iom_arg6;
    static const char   site_id[ESITENUM] = {DEF_SITE_ID_TKY, DEF_SITE_ID_OSK};

    // 自N/W識別、グループ識別
    gfnwi_pk.pri_key.nw_id = g_myinfo.network_id[0];
    memcpy(gfnwi_pk.pri_key.grp_id, g_myinfo.group_id, sizeof(gfnwi_pk.pri_key.grp_id));
    memset(g_gfnwidata, NULL, sizeof(g_gfnwidata));
    memset(g_mnglyr, NULL, sizeof(g_mnglyr));

    // COM_IOMファイル情報設定
    memset(&iom_arg4, NULL, sizeof(iom_arg4));
    memcpy(iom_arg4.file_id, DEF_FL_NW_INFO, sizeof(DEF_FL_NW_INFO)-1);

    // COM_IOM入力情報設定
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = sizeof(gfnwi_pk.pri_key);
    iom_arg5.compare_len = sizeof(gfnwi_pk.pri_key.site_id) + sizeof(gfnwi_pk.pri_key.nw_id) + sizeof(gfnwi_pk.pri_key.grp_id);
    iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.io_timer = g_myinfo.fio_timer;
    iom_arg5.rec_len = db_gfnwi_def_Size;

    // NW情報ファイルオープン
    if (g_myinfo.site_id[0] == DEF_SITE_ID_TKY) {
        site = ESITEEAST;
    } else {
        site = ESITEWEST;
    }
    memset(iom_arg4.file_name, ' ', sizeof(iom_arg4.file_name));
    memcpy(iom_arg4.file_name, g_fileinfo[EFILEGFNWI].name[site], strlen(g_fileinfo[EFILEGFNWI].name[site]));
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_OPEN, sizeof(DEF_COM_IOM_FIO_OPEN)-1);
    ret = COM_IOM(DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }
    // サイト
    for (site = ESITEEAST; site < ESITENUM; ++site) {
        gfnwi_pk.pri_key.site_id = site_id[site];
        memcpy(iom_arg5.key_value, &gfnwi_pk.pri_key, iom_arg5.compare_len);
        memset(&iom_arg6, NULL, sizeof(iom_arg6));
        rec_cnt = 0;
        is_exists_mnglyr = 0;
        // READ開始
        memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
        ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
        while (ret == 0) {
            // EOF判定
            if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
                break;
            }
            gfnwi_rec = (db_gfnwi_def*)iom_arg6.rec_area;
            // 管理単位情報レコード
            if (!memcmp(gfnwi_rec->pri_key.if_id, DEF_IFID_DEFAULT, sizeof(DEF_IFID_DEFAULT)-1)) {
                for (kind = 0; kind < EMNGLYRNUM; ++kind) {
                    switch (kind) {
                    case EMNGLYROPCL:   // 開局/閉局管理単位
                        mng_lyr = gfnwi_rec->mng_lyr_info.open_close_mng_lyr;
                        break;
                    case EMNGLYRECHO:   // エコーテスト管理単位
                        mng_lyr = gfnwi_rec->mng_lyr_info.echo_test_mng_lyr;
                        break;
                    case EMNGLYRKCHG:   // 鍵交換管理単位
                        mng_lyr = gfnwi_rec->mng_lyr_info.key_cng_mng_lyr;
                        break;
                    case EMNGLYRCTOV:   // カットオーバー管理単位
                        mng_lyr = gfnwi_rec->mng_lyr_info.cut_over_mng_lyr;
                        break;
                    case EMNGLYRCONN:   // コネクション数管理単位
                        mng_lyr = gfnwi_rec->mng_lyr_info.connect_num_mng_lyr;
                        break;
                    }
                    if (mng_lyr == ' ') {
                        g_mnglyr[site][kind] = ELYRSITE;
                    } else if (mng_lyr == 'I') {
                        g_mnglyr[site][kind] = ELYRINTF;
                    } else if (mng_lyr == 'S') {
                        g_mnglyr[site][kind] = ELYRSTA;
                    } else if (mng_lyr == 'C') {
                        g_mnglyr[site][kind] = ELYRCONN;
                    }
                }
                is_exists_mnglyr = 1;
            } else {
                // 最大数判定
                if (rec_cnt >= DEF_STA_GRP_NUM_MAX) {
                    /*------------------------------------------------*/
                    /*    メッセージ編集：設定情報エラー              */
                    /*------------------------------------------------*/
                    // 任意メッセージ部初期化
                    CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
                    // 内部エラーコード
                    memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
                    // ③ファイル論理名 (8バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_NW_INFO, sizeof(DEF_FL_NW_INFO)-1);
                    // ④キー (40バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfnwi_pk.pri_key, iom_arg5.compare_len);
                    // ⑤エラー項目/エラー理由 (40バイト)
                    memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_EXC_MAX_REC_CNT, sizeof(DEF_EMS_EXC_MAX_REC_CNT)-1);
                    GFPOGGZ1(&g_oggz1in);
                    // 異常終了処理
                    CMDS_abend();
                }
                nwidt = &g_gfnwidata[site][rec_cnt];
                memcpy(&nwidt->pri_key, &gfnwi_rec->pri_key, sizeof(nwidt->pri_key));
                memcpy(&nwidt->nw_id_info, &gfnwi_rec->nw_id_info, sizeof(nwidt->nw_id_info));
                ++rec_cnt;
            }
            // NEXTREAD
            memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_READ, sizeof(DEF_COM_IOM_FIO_READ)-1);
            ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
        }
        // エラー判定
        if (ret != 0) {
            /*------------------------------------------------*/
            /*    メッセージ編集：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
            // ④エラーコード(4バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }
        // グループ単位(管理単位情報)レコードなし
        if (is_exists_mnglyr == 0) {
            /*------------------------------------------------*/
            /*    メッセージ編集：設定情報エラー              */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
            // ③ファイル論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_NW_INFO, sizeof(DEF_FL_NW_INFO)-1);
            // ④キー (40バイト) キーは、グループ単位(管理単位情報)レコードを設定
            memcpy(gfnwi_pk.pri_key.if_id, DEF_IFID_DEFAULT, sizeof(DEF_IFID_DEFAULT)-1);
            memcpy(gfnwi_pk.pri_key.station_id, DEF_STID_DEFAULT, sizeof(DEF_STID_DEFAULT)-1);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, &gfnwi_pk.pri_key, sizeof(gfnwi_pk.pri_key));
            // ⑤エラー項目/エラー理由 (40バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }
    }
    // NW情報ファイルクローズ
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_CLOSE, sizeof(DEF_COM_IOM_FIO_CLOSE)-1);
    COM_IOM(DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
}

/****************************************************************************/
/*  FUNCTION        : 2.1.0  CMDS_read_recv                                 */
/*  CALLING SEQ.    : void  CMDS_read_recv(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : $RECEIVE処理                                          */
/****************************************************************************/
void CMDS_read_recv(void)
{
    iocomp_def* piocomp = &g_myinfo.iocomp;

    // $RECEIVEのREADUPDATE
    READUPDATEX(g_myinfo.recv_fno, g_myinfo.recvbuf, DEF_RCV_BUF_SIZE, &piocomp->len);
    piocomp->fno = g_myinfo.recv_fno;
    AWAITIOX(&piocomp->fno, &piocomp->addr, &piocomp->len, &piocomp->tag);
    FILE_GETINFO_(piocomp->fno, &piocomp->errno);

    // $RECEIVE情報取得
    FILE_GETRECEIVEINFO_((short _far *)&piocomp->recv_info);
}

/****************************************************************************/
/*  FUNCTION        : 2.2.0  CMDS_handle_symsg                              */
/*  CALLING SEQ.    : void  CMDS_handle_symsg(void)                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : システムメッセージ処理                                */
/****************************************************************************/
void CMDS_handle_sys_msg(void)
{
    short ret;
    char buffer[64];
    zsys_ddl_smsg_def *sysmsg = (zsys_ddl_smsg_def*)g_myinfo.recvbuf;

    // システムメッセージ判定
    switch (sysmsg->u_z_msg.z_msgnumber[0]) {
    // オープンメッセージ
    case ZSYS_VAL_SMSG_OPEN:
        CMDS_handle_sys_open();
        break;
    // クローズメッセージ
    case ZSYS_VAL_SMSG_CLOSE:
        CMDS_handle_sys_close();
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
            CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            snprintf(buffer, sizeof(buffer), "%04d", ret);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);
            // 異常終了処理
            CMDS_abend();
        }
        // fall through
    default:
        // nullリプライ
        memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
        break;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.0  CMDS_handle_req_msg                            */
/*  CALLING SEQ.    : void  CMDS_handle_req_msg(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 要求受信処理                                          */
/****************************************************************************/
void CMDS_handle_req_msg(void)
{
    short ret;
    char buffer[64];
    char rcv_ipc_hex[(sizeof(common_header_def)<<1)+1];
    c501_def* c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def* r501 = (r501_def*)g_myinfo.replyinfo.replybuf;

    // コマンド識別
    g_myinfo.command_code = (short)CMDS_atoii(c501->command_info.command_name, sizeof(c501->command_info.command_name));
 
    // インターフェースコードに応答コードを設定する
    memset(r501, ' ', sizeof(r501_def));
    memcpy(r501->common_header.interface_code, DEF_IPC_IFCD_CMD_RSP, sizeof(DEF_IPC_IFCD_CMD_RSP)-1);
    // コマンド情報を応答コードを設定する
    memcpy(&r501->command_info, &c501->command_info, sizeof(r501->command_info));

    // リクエストIPC内容精査処理
    ret = CMDS_validate_req_ipc();
    // 異常
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：リクエストエラー            */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_REQ_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // ③エラー内容 (20バイト)
        switch (ret) {
        case DEF_CHK_IPC_IFCD_ERR:  // インターフェースコード
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_IFCD, sizeof(DEF_EMS_IPC_CHK_IFCD)-1);
            break;
        case DEF_CHK_IPC_DLEN_ERR:  // データ長
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_LEN, sizeof(DEF_EMS_IPC_CHK_LEN)-1);
            break;
        case DEF_CHK_IPC_CMD_ERR:   // コマンド識別
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_CMDCD, sizeof(DEF_EMS_IPC_CHK_CMDCD)-1);
            break;
        case DEF_CHK_IPC_NW_ERR:    // N/W識別エラー
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_NW, sizeof(DEF_EMS_IPC_CHK_NW)-1);
            break;
        case DEF_CHK_IPC_GRP_ERR:   // グループ識別
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_GRP, sizeof(DEF_EMS_IPC_CHK_GRP)-1);
            break;
        case DEF_CHK_IPC_OPT_ERR:   // 必須項目不足
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_OPT, sizeof(DEF_EMS_IPC_CHK_OPT)-1);
            break;
        case DEF_CHK_IPC_SC_ERR:    // サーバークラス論理IDエ
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_SC, sizeof(DEF_EMS_IPC_CHK_SC)-1);
            break;
        }
        // ④受信IPC内容 (48バイト)
        CMDS_strtohex(g_myinfo.recvbuf, rcv_ipc_hex, sizeof(common_header_def));
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, rcv_ipc_hex, sizeof(rcv_ipc_hex)-1);
        GFPOGGZ1(&g_oggz1in);

        /*------------------------------------------------*/
        /*    応答バッファ共通ヘッダ編集                  */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // データ長
        r501->common_header.control_data_length = 0;
        g_myinfo.replyinfo.reply_len = sizeof(common_header_def);
    // 正常
    } else {
        /*------------------------------------------------*/
        /*    応答バッファ共通ヘッダ編集                  */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_OK;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1);
        // データ長
        r501->common_header.control_data_length = sizeof(c501_def) - sizeof(common_header_def);
        g_myinfo.replyinfo.reply_len = sizeof(r501_def);
        /*------------------------------------------------*/
        /*    コマンドサーバ処理                          */
        /*------------------------------------------------*/
        // ステータス照会処理
        if (g_myinfo.command_code == DEF_IPC_CMD_CD_STS_DSP ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_STS_DSP ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_ECH_STS_DSP) {
            CMDS_inquire_status();
        // 通信制御コマンド転送処理
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_OPN ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CLS ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_LSN_START ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_LSN_END) {
            CMDS_forward_connctrl();
        // 制御電文コマンド転送処理
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_ABS ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_UPD ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_AO1 ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_AO2 ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS_ABS ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS_UPD ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_ECH_SND ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_KEY_EXC_REQ ||
            g_myinfo.command_code == DEF_IPC_CMD_CD_KEY_EXC) {
            CMDS_forward_cmdst();
        // ログファイル切替コマンド転送処理
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_LOG_FL_EXC) {
            CMDS_forward_logswitch();
        // ファイル再読込（接続構成変更）コマンド転送処理
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN) {
            CMDS_forward_reload_lin();
        // ファイル再読込（東阪振分比率変更）コマンド転送処理
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_NSW) {
            CMDS_forward_reload_nsw();
        }
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.4.0  CMDS_reply                                     */
/*  CALLING SEQ.    : void  CMDS_reply(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void CMDS_reply(void)
{
    REPLYX(g_myinfo.replyinfo.replybuf, g_myinfo.replyinfo.reply_len,,,
           g_myinfo.replyinfo.reply_code);
}

/****************************************************************************/
/*  FUNCTION        : 2.2.1  CMDS_handle_syopen                             */
/*  CALLING SEQ.    : void  CMDS_handle_syopen(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープンメッセージ処理                                */
/****************************************************************************/
void CMDS_handle_sys_open(void)
{
    char        pname[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];
    char        buffer[64];
    short       pname_len;
    short       openid;
    short       reply_cd;
    short       ret;
    zsys_ddl_smsg_open_reply_def* popenrep_msg = (zsys_ddl_smsg_open_reply_def*)g_myinfo.replyinfo.replybuf;

    memset(pname, NULL, sizeof(pname));
    pname_len = 0;
    openid = 0;
    reply_cd = 0;

    // オープン発行元プロセス名取得
    PROCESSHANDLE_DECOMPOSE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender,,,,,,,pname, sizeof(pname), &pname_len);
    // オープン発行元プロセス名判定
    ret = PROCESSHANDLE_COMPARE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender, g_myinfo.procinfo.ans_phandle);
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
            CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
            // ③モジュールID(8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            snprintf(buffer, sizeof(buffer), "%04d", ret);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);

            // 異常終了処理
            CMDS_abend();
        }
    }

    // リプライメッセージ
    popenrep_msg->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
    popenrep_msg->z_openid = openid;
    g_myinfo.replyinfo.reply_len = zsys_ddl_smsg_open_reply_def_Size;
    g_myinfo.replyinfo.reply_code = reply_cd;
}

/****************************************************************************/
/*  FUNCTION        : 2.2.2  CMDS_handle_syclose                            */
/*  CALLING SEQ.    : void  CMDS_handle_syclose(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : クローズメッセージ処理                                */
/****************************************************************************/
void CMDS_handle_sys_close(void)
{
    short ret;
    char buffer[64];

    // サーバ停止判定
    ret = COM_STP_JUDGE(&g_myinfo.stp_arg, g_myinfo.recvbuf);
    // プロセス終了判定
    if (ret == 1) {
        g_myinfo.end_flg = DEF_FLG_ON;
    } else if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_EMPTY, sizeof(DEF_NERR_EMPTY)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }

    // nullリプライ
    memset(&g_myinfo.replyinfo, NULL, sizeof(replyinfo_def));
}

/****************************************************************************/
/*  FUNCTION        : 2.3.1  CMDS_validate_req_ipc                          */
/*  CALLING SEQ.    : void  CMDS_validate_req_ipc(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 正常: 0 ; 異常: other                                 */
/*  DESCRIPTION     : リクエストIPC内容精査処理                             */
/****************************************************************************/
short CMDS_validate_req_ipc(void)
{
    short is_openclose = 0, is_ctrlmsg = 0;
    c501_def* c501 = (c501_def*)g_myinfo.recvbuf;

    // 共通ヘッダ：インターフェースコードチェック
    if (memcmp(c501->common_header.interface_code,
        DEF_IPC_IFCD_CMD_REQ, sizeof(DEF_IPC_IFCD_CMD_REQ)-1)) {
        return DEF_CHK_IPC_IFCD_ERR;
    }
    // 共通ヘッダ：データ長チェック
    if (c501->common_header.control_data_length !=
        (sizeof(c501_def) - sizeof(common_header_def))) {
        return DEF_CHK_IPC_DLEN_ERR;
    }
    // コマンド識別チェック
    switch (g_myinfo.command_code) {
    case DEF_IPC_CMD_CD_OPN:                // オープン 
    case DEF_IPC_CMD_CD_CLS:                // クローズ 
    case DEF_IPC_CMD_CD_LSN_START:          // リスナー（開始）
    case DEF_IPC_CMD_CD_LSN_END:            // リスナー（終了）
    case DEF_IPC_CMD_CD_STS_DSP:            // コネクションステータス照会
        break;
    case DEF_IPC_CMD_CD_CNT_OPN:            // 開局
    case DEF_IPC_CMD_CD_CNT_OPN_ABS:        // 開局強制実行
    case DEF_IPC_CMD_CD_CNT_OPN_UPD:        // 開局状態更新のみ
    case DEF_IPC_CMD_CD_CNT_OPN_AO1:        // 自動開局（局状態制御）
    case DEF_IPC_CMD_CD_CNT_OPN_AO2:        // 自動開局（コネクション制御）
    case DEF_IPC_CMD_CD_CNT_CLS:            // 閉局
    case DEF_IPC_CMD_CD_CNT_CLS_ABS:        // 閉局強制実行
    case DEF_IPC_CMD_CD_CNT_CLS_UPD:        // 閉局状態更新のみ
        is_openclose = 1;
        break;
    case DEF_IPC_CMD_CD_CNT_STS_DSP:        // 局状態照会
        break;
    case DEF_IPC_CMD_CD_ECH_SND:            // エコー送信
    case DEF_IPC_CMD_CD_ECH_STS_DSP:        // エコーステータス照会
    case DEF_IPC_CMD_CD_KEY_EXC_REQ:        // 鍵交換依頼
    case DEF_IPC_CMD_CD_KEY_EXC:            // 鍵交換
        is_ctrlmsg = 1;
        break;
    case DEF_IPC_CMD_CD_LOG_FL_EXC:         // ログファイル切替
        break;
    case DEF_IPC_CMD_CD_FL_RE_LIN:          // ファイル再読込 接続構成変更
    case DEF_IPC_CMD_CD_FL_RE_NSW:          // ファイル再読込 東阪振分比率変更
        break;
    default:
        return DEF_CHK_IPC_CMD_ERR;
    }
    // 必須項目チェック
    // サイト識別、N/W識別、グループ識別
    if (c501->command_info.connection_logical_name.site_name != ' ' &&
        c501->command_info.connection_logical_name.nw_name != ' ' &&
        c501->command_info.connection_logical_name.group_name[0] != ' ') {
        // N/W識別チェック
        if (c501->command_info.connection_logical_name.nw_name != g_myinfo.network_id[0]) {
            return DEF_CHK_IPC_NW_ERR;
        }
        // グループ識別チェック
        if (memcmp(c501->command_info.connection_logical_name.group_name,
            g_myinfo.group_id, sizeof(g_myinfo.group_id))) {
            return DEF_CHK_IPC_GRP_ERR;
        }
    // 開閉局、インタフェース名 or ステーション名指定
    } else if (is_openclose == 1) {
        if (c501->command_info.interface_ext_name[0] == ' ' &&
            c501->command_info.station_ext_name[0] == ' ') {
            return DEF_CHK_IPC_OPT_ERR;
        }
    // ファイル再読込 東阪振分比率変更
    } else if (g_myinfo.command_code != DEF_IPC_CMD_CD_FL_RE_NSW) {
        return DEF_CHK_IPC_OPT_ERR;
    }
    // エコー送信、エコーステータス照会、鍵交換依頼、鍵交換
    // ファイル再読込 接続構成変更
    // インタフェース識別なし
    if ((is_ctrlmsg == 1 || g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN) &&
        c501->command_info.connection_logical_name.interface_name[0] == ' ') {
        return DEF_CHK_IPC_OPT_ERR;
    }
    // ファイル再読込 接続構成変更 サーバクラス論理ID指定時
    // サーバクラス論理IDがリスナー、コネクション制御(サーバ・クライアント)、電文振分(outbound)何れでもない
    if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN &&
        c501->command_info.srv_cls_id[0] != ' ' &&
        (memcmp(c501->command_info.srv_cls_id, DEF_SC_LISTEN, sizeof(DEF_SC_LISTEN)-1) &&
         memcmp(c501->command_info.srv_cls_id, DEF_SC_CON_SVR, sizeof(DEF_SC_CON_SVR)-1) &&
         memcmp(c501->command_info.srv_cls_id, DEF_SC_CON_CLT, sizeof(DEF_SC_CON_CLT)-1) &&
         memcmp(c501->command_info.srv_cls_id, DEF_SC_FURI_O, sizeof(DEF_SC_FURI_O)-1))) {
        return DEF_CHK_IPC_SC_ERR;
    }

    return DEF_CHK_IPC_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.3.2  CMDS_forward_connctrl                          */
/*  CALLING SEQ.    : void  CMDS_forward_connctrl(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 通信制御コマンド転送処理                              */
/****************************************************************************/
void CMDS_forward_connctrl(void)
{
    short           i;
    short           site;
    short           compare_len;
    short           ret;
    short           fwd_cnt = 0, err_cnt = 0;
    c501_def*       c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def*       r501 = (r501_def*)g_myinfo.replyinfo.replybuf;
    gflin_pkey_def* gflin_pk = (gflin_pkey_def*)&c501->command_info.connection_logical_name;
    gflindata_def*  lindt;
    destinfo_def*   destinfo;

    // IPCから比較レングス設定
    compare_len = sizeof(gflin_pk->site_name) + sizeof(gflin_pk->nw_name) + sizeof(gflin_pk->group_name);
    if (gflin_pk->interface_name[0] != ' ') {
        compare_len += sizeof(gflin_pk->interface_name);
    }
    if (gflin_pk->station_name[0] != ' ') {
        compare_len += sizeof(gflin_pk->station_name);
    }
    if (gflin_pk->connection_name[0] != ' ') {
        compare_len += sizeof(gflin_pk->connection_name);
    }
    // サイト判定
    if (gflin_pk->site_name == DEF_SITE_ID_TKY) {
        lindt = g_gflindata[ESITEEAST];
    } else {
        lindt = g_gflindata[ESITEWEST];
    }
    for (i = 0; i < DEF_CONN_GRP_NUM_MAX; ++i) {
        // end of data
        if (lindt[i].srv_cls_kind[0] == NULL) {
            break;
        }
        // 処理対象判定
        // IPC指定識別子が一致
        // リスナー（開始）、リスナー（終了）は、リスナーまたは、
        // オープン、クローズは、コネクション制御(サーバ)、コネクション制御(クライアント)
        if (!memcmp(gflin_pk, &lindt[i].pri_key, compare_len) &&
            (((g_myinfo.command_code == DEF_IPC_CMD_CD_OPN || g_myinfo.command_code == DEF_IPC_CMD_CD_CLS) &&
            (!memcmp(lindt[i].srv_cls_kind, DEF_SC_CON_SVR, sizeof(DEF_SC_CON_SVR)-1) ||
             !memcmp(lindt[i].srv_cls_kind, DEF_SC_CON_CLT, sizeof(DEF_SC_CON_CLT)-1))) ||
            ((g_myinfo.command_code == DEF_IPC_CMD_CD_LSN_START || g_myinfo.command_code == DEF_IPC_CMD_CD_LSN_END) &&
             !memcmp(lindt[i].srv_cls_kind, DEF_SC_LISTEN, sizeof(DEF_SC_LISTEN)-1)))) {

            // コネクション指定以外は回線運用識別が通常回線であること(ADD 2026/05/20 DR運用対応)
            if ((gflin_pk->connection_name[0] == ' ') && (lindt[i].operation_id != ' ')) {
                continue;
            }

            // 送信先取得
            if (CMDS_get_destinfo(&lindt[i], &destinfo) == DEF_FLG_OK) {
                // IPC編集、PATHSEND処理
                ret = CMDS_pathsend(destinfo, &lindt[i].pri_key, sizeof(gflin_pkey_def));
                if (ret != 0) {
                    err_cnt++;
                } else {
                    fwd_cnt++;
                }
            } else {
                err_cnt++;
            }
        }
    }
    // エラーあり
    if (err_cnt != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE,
               sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    // 転送先なし
    } else if (fwd_cnt == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_TARGET_ERR,
            sizeof(DEF_NERR_CMD_TARGET_ERR)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.3  CMDS_forward_cmdst                             */
/*  CALLING SEQ.    : void  CMDS_forward_cmdst(void)                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 制御電文コマンド転送処理                              */
/****************************************************************************/
void CMDS_forward_cmdst(void)
{
    short               i, j;
    short               ret;
    short               specify_type = 0;
    short               fwd_cnt = 0, err_cnt = 0;
    short               site;
    short               mnglyrkind;
    short               ipc_lyr_no;
    short               mng_lyr_no;
    short               comp_len, lin_comp_len;
    short               connid_len;
    char                site_id;
    char                rcv_ipc_hex[(sizeof(common_header_def)<<1)+1];
    c501_def*           c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def*           r501 = (r501_def*)g_myinfo.replyinfo.replybuf;
    gflin_pkey_def*     gflin_pk = (gflin_pkey_def*)&c501->command_info.connection_logical_name;
    gfnwidata_def*      nwidt;
    gflindata_def*      lindt;
    void*               p;

    // サイト
    if (gflin_pk->site_name != ' ') {
        specify_type = 0;
        site_id = gflin_pk->site_name;
    } else {
        // ステーション名指定時（開局、閉局）
        if(c501->command_info.station_ext_name[0] != ' ') {
            specify_type = 1;
        // インタフェース名指定（開局、閉局）
        } else {
            specify_type = 2;
        }
        site_id = g_myinfo.site_id[0];
    }
    if (site_id == DEF_SITE_ID_TKY) {
        site = ESITEEAST;
    } else {
        site = ESITEWEST;
    }
    // インタフェース名指定（開局、閉局）
    if (specify_type == 2) {
        ipc_lyr_no = ELYRINTF;
        comp_len = sizeof(gflin_pk->site_name) + sizeof(gflin_pk->nw_name) + sizeof(gflin_pk->group_name) + sizeof(gflin_pk->interface_name);
    // ステーション名指定時（開局、閉局）
    } else if(specify_type == 1) {
        ipc_lyr_no = ELYRSTA;
        comp_len = sizeof(gflin_pk->site_name) + sizeof(gflin_pk->nw_name) + sizeof(gflin_pk->group_name) + sizeof(gflin_pk->interface_name) + sizeof(gflin_pk->station_name);
    // 識別子指定
    } else {
        ipc_lyr_no = ELYRGRP;
        comp_len = sizeof(gflin_pk->site_name) + sizeof(gflin_pk->nw_name) + sizeof(gflin_pk->group_name);
        if (gflin_pk->interface_name[0] != ' ') {
            comp_len += sizeof(gflin_pk->interface_name);
            ++ipc_lyr_no;
        }
        if (gflin_pk->station_name[0] != ' ') {
            comp_len += sizeof(gflin_pk->station_name);
            ++ipc_lyr_no;
        }
        if (gflin_pk->connection_name[0] != ' ') {
            ++ipc_lyr_no;
        }
    }
    // 開局・閉局
    if (g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN || g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_ABS ||
        g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_UPD || g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_AO1 ||
        g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_OPN_AO2 || g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS ||
        g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS_ABS || g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_CLS_UPD) {
        mnglyrkind = EMNGLYROPCL;
    // エコー送信
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_ECH_SND) {
        mnglyrkind = EMNGLYRECHO;
    // 鍵交換依頼・鍵交換
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_KEY_EXC_REQ || g_myinfo.command_code == DEF_IPC_CMD_CD_KEY_EXC) {
        mnglyrkind = EMNGLYRKCHG;
    }
    // 対象機能管理単位
    mng_lyr_no = g_mnglyr[site][mnglyrkind];
    // 該当機能なしまたは、
    // 対応するコマンドの管理単位とコマンドIPCの指定階層を比較して、
    // コマンドIPCの方が下位の階層指定だった場合エラー応答
    if (mng_lyr_no < ipc_lyr_no) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE, sizeof(DEF_NERR_CMD_ERR_DONE)-1);
        /*------------------------------------------------*/
        /*    メッセージ出力：リクエストエラー            */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_REQ_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // ③エラー内容 (20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_EMS_IPC_CHK_MNGLYR, sizeof(DEF_EMS_IPC_CHK_MNGLYR)-1);
        // ④受信IPC内容 (48バイト)
        CMDS_strtohex(g_myinfo.recvbuf, rcv_ipc_hex, sizeof(common_header_def));
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, rcv_ipc_hex, sizeof(rcv_ipc_hex)-1);
        GFPOGGZ1(&g_oggz1in);
        return;
    }
    // 管理単位：インターフェース
    if (mng_lyr_no == ELYRINTF) {
        connid_len = sizeof(g_gfnwidata[site][0].pri_key) - sizeof(g_gfnwidata[site][0].pri_key.station_id);
    // 管理単位：ステーション
    } else if (mng_lyr_no == ELYRSTA) {
        connid_len = sizeof(g_gfnwidata[site][0].pri_key);
    // 管理単位：コネクション
    } else if (mng_lyr_no == ELYRCONN) {
        connid_len = sizeof(gflin_pkey_def);
    }

    // NW情報ファイルデータ
    nwidt = g_gfnwidata[site];
    lindt = g_gflindata[site];
    for (i = 0; i < DEF_STA_GRP_NUM_MAX; ++i) {
        // end of data
        if (nwidt[i].pri_key.site_id == NULL) {
            break;
        }
        // 管理単位インターフェースの場合、ステーション識別が”}}}}}}”のレコードが対象
        if (mng_lyr_no == ELYRINTF && memcmp(nwidt[i].pri_key.station_id, DEF_STID_DEFAULT, sizeof(DEF_STID_DEFAULT)-1)) {
            continue;
        // 管理単位ステーション or コネクションの場合、ステーション識別が”}}}}}}”以外のレコードが対象
        } else if ((mng_lyr_no == ELYRSTA || mng_lyr_no == ELYRCONN) &&
                    !memcmp(nwidt[i].pri_key.station_id, DEF_STID_DEFAULT, sizeof(DEF_STID_DEFAULT)-1)) {
            continue;
        }

        // 識別子指定
        if ((specify_type == 0 && !memcmp(&nwidt[i].pri_key, gflin_pk, comp_len)) ||
            // ステーション名指定時
            (specify_type == 1 && nwidt[i].pri_key.site_id == g_myinfo.site_id[0] &&
             !memcmp(nwidt[i].nw_id_info.nw_station, c501->command_info.station_ext_name, sizeof(nwidt[i].nw_id_info.nw_station))) ||
            // インタフェース名指定
            (specify_type == 2 && nwidt[i].pri_key.site_id == g_myinfo.site_id[0] &&
             !memcmp(nwidt[i].nw_id_info.nw_if, c501->command_info.interface_ext_name, sizeof(nwidt[i].nw_id_info.nw_if)))) {
            // 管理単位：コネクション
            if (mng_lyr_no == ELYRCONN) {
                for (j = 0; j < DEF_CONN_GRP_NUM_MAX; ++j) {
                    if (lindt[j].srv_cls_kind[0] == NULL) {
                        break;
                    }
                    // 処理対象判定
                    // 識別子が一致かつ、コネクション識別が、コネクション制御(サーバ・クライアント)
                    if (ipc_lyr_no == ELYRCONN) {
                        p = gflin_pk;
                        lin_comp_len = sizeof(gflin_pkey_def);
                    } else {
                        p = &nwidt[i].pri_key;
                        lin_comp_len = sizeof(nwidt[i].pri_key);
                    }
                    if (!memcmp(p, &lindt[j].pri_key, lin_comp_len) &&
                        (!memcmp(lindt[j].srv_cls_kind, DEF_SC_CON_SVR, sizeof(DEF_SC_CON_SVR)-1) ||
                         !memcmp(lindt[j].srv_cls_kind, DEF_SC_CON_CLT, sizeof(DEF_SC_CON_CLT)-1))) {

                        // コネクション指定以外は回線運用識別が通常回線であること(ADD 2026/05/20 DR運用対応)
                        if ((c501->command_info.connection_logical_name.connection_name[0] == ' ') &&
                            (lindt[j].operation_id != ' ')) {
                            continue;
                        }

                        // IPC編集、PATHSEND処理
                        ret = CMDS_pathsend(&g_dest_cmdst[site], &lindt[j].pri_key, sizeof(gflin_pkey_def));
                        if (ret != 0) {
                            err_cnt++;
                        } else {
                            fwd_cnt++;
                        }
                    }
                }
            // 管理単位：コネクション以外
            } else {
                // IPC編集、PATHSEND処理
                ret = CMDS_pathsend(&g_dest_cmdst[site], &nwidt[i].pri_key, connid_len);
                if (ret != 0) {
                    err_cnt++;
                } else {
                    fwd_cnt++;
                }
            }
        }
    }
    // エラーあり
    if (err_cnt != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE, sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    // 転送先なし
    } else if (fwd_cnt == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_TARGET_ERR, sizeof(DEF_NERR_CMD_TARGET_ERR)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.4  CMDS_forward_logswitch                         */
/*  CALLING SEQ.    : void  CMDS_forward_logswitch(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ログファイル切替コマンド転送処理                      */
/****************************************************************************/
void CMDS_forward_logswitch(void)
{
    short             fwd_cnt = 0, err_cnt = 0;
    r501_def*         r501 = (r501_def*)g_myinfo.replyinfo.replybuf;

    CMDS_forward_process(&fwd_cnt, &err_cnt);
    // エラーあり
    if (err_cnt != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE,
               sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    // 転送先なし
    } else if (fwd_cnt == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_TARGET_ERR,
            sizeof(DEF_NERR_CMD_TARGET_ERR)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.5  CMDS_forward_reload_lin                        */
/*  CALLING SEQ.    : void  CMDS_forward_reload_lin(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ファイル再読込（接続構成変更）コマンド転送処理        */
/****************************************************************************/
void CMDS_forward_reload_lin(void)
{
    short           i;
    short           comp_len;
    short           ret;
    short           fwd_cnt = 0, err_cnt = 0;
    c501_def*       c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def*       r501 = (r501_def*)g_myinfo.replyinfo.replybuf;
    gflin_pkey_def* gflin_pk = (gflin_pkey_def*)&c501->command_info.connection_logical_name;
    destinfo_def*   destconn;
    gflindata_def*  lindt;
    destinfo_def*   destinfo;

    // 回線管理ファイル読込
    CMDS_load_GFLIN();
    // NW情報ファイル読込
    CMDS_load_GFNWI();

    // サーバクラス論理ID指定で、電文振分(outbound)の場合、PATHSENDなし
    if (memcmp(c501->command_info.srv_cls_id, DEF_SC_FURI_O, sizeof(DEF_SC_FURI_O)-1)) {
        // IPCから比較レングス設定
        comp_len = sizeof(gflin_pk->site_name) + sizeof(gflin_pk->nw_name) + sizeof(gflin_pk->group_name);
        if (gflin_pk->interface_name[0] != ' ') {
            comp_len += sizeof(gflin_pk->interface_name);
        }
        if (gflin_pk->station_name[0] != ' ') {
            comp_len += sizeof(gflin_pk->station_name);
        }
        // サイト判定
        if (gflin_pk->site_name == DEF_SITE_ID_TKY) {
            lindt = g_gflindata[ESITEEAST];
            destconn = g_dest_connctrl[ESITEEAST];
        } else {
            lindt = g_gflindata[ESITEWEST];
            destconn = g_dest_connctrl[ESITEWEST];
        }
        // 転送済フラグ初期化
        for (i = 0; i < DEF_CONNCTRL_NUM_MAX; ++i) {
            if (destconn[i].srv_cls_kind[0] == NULL) {
                break;
            }
            destconn[i].is_forwarded = DEF_FLG_OFF;
        }
        for (i = 0; i < DEF_CONN_GRP_NUM_MAX; ++i) {
            // end of data
            if (lindt[i].srv_cls_kind[0] == NULL) {
                break;
            }
            // 処理対象判定
            // IPC指定識別子が一致
            // サーバクラス論理ID指定時は、対象サーバクラスのみ
            if (!memcmp(gflin_pk, &lindt[i].pri_key, comp_len) && (c501->command_info.srv_cls_id[0] == ' ' ||
                !memcmp(lindt[i].srv_cls_kind, c501->command_info.srv_cls_id, sizeof(lindt[i].srv_cls_kind)))) {
                // 送信先取得
                if (CMDS_get_destinfo(&lindt[i], &destinfo) == DEF_FLG_OK) {
                    // 転送済サーバクラスには再送しない
                    if (destinfo->is_forwarded != DEF_FLG_ON) {
                        // IPC編集、PATHSEND処理
                        ret = CMDS_pathsend(destinfo, &lindt[i].pri_key, comp_len);
                        if (ret != 0) {
                            err_cnt++;
                        } else {
                            fwd_cnt++;
                        }
                        // 転送済に設定
                        destinfo->is_forwarded = DEF_FLG_ON;
                    }
                } else {
                    err_cnt++;
                }
            }
        }
    }

    // 電文振分(outbound)
    CMDS_forward_process(&fwd_cnt, &err_cnt);

    // エラーあり
    if (err_cnt != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE, sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    // 転送先なし
    } else if (fwd_cnt == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_TARGET_ERR,
            sizeof(DEF_NERR_CMD_TARGET_ERR)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.6  CMDS_forward_reload_nsw                        */
/*  CALLING SEQ.    : void  CMDS_forward_reload_nsw(void)                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ファイル再読込（東阪振分比率変更）コマンド転送処理    */
/****************************************************************************/
void CMDS_forward_reload_nsw(void)
{
    short             fwd_cnt = 0, err_cnt = 0;
    r501_def*         r501 = (r501_def*)g_myinfo.replyinfo.replybuf;

    CMDS_forward_process(&fwd_cnt, &err_cnt);
    // エラーあり
    if (err_cnt != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE,
               sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    // 転送先なし
    } else if (fwd_cnt == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_TARGET_ERR,
               sizeof(DEF_NERR_CMD_TARGET_ERR)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.7  CMDS_inquire_status                            */
/*  CALLING SEQ.    : void  CMDS_inquire_status(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ステータス照会処理                                    */
/****************************************************************************/
void CMDS_inquire_status(void)
{
    short             ret, rec_len, ipcdrec_len;
    short             key_len, cmp_len;
    short             rec_cnt, rec_max;
    short             site, file_kind;
    char              logical_fname[8];
    char              target_fname[ZSYS_VAL_LEN_FILENAME+1];
    char              sub_prog_sts[2];
    char*             p;
    c501_def*         c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def*         r501 = (r501_def*)g_myinfo.replyinfo.replybuf;
    gflin_pkey_def*   gflin_pk = (gflin_pkey_def*)&c501->command_info.connection_logical_name;
    db_gclst_def      gclst_pk;                 // 回線ステータスファイル
    db_gcsst_def      gcsst_pk;                 // 局状態管理ファイル
    db_gcest_def      gcest_pk;                 // エコー状態管理ファイル
    void*             pkey;
    COM_IOM_arg_4_def iom_arg4;
    COM_IOM_arg_5_def iom_arg5;
    COM_IOM_arg_6_def iom_arg6;

    // コネクションステータス照会
    if (g_myinfo.command_code == DEF_IPC_CMD_CD_STS_DSP) {
        memcpy(logical_fname, DEF_FL_LIN_STS, sizeof(logical_fname));
        // Key設定
        memcpy(&gclst_pk.pri_key.site_id, &gflin_pk->site_name, sizeof(gclst_pk.pri_key.site_id));
        memcpy(&gclst_pk.pri_key.nw_id, &gflin_pk->nw_name, sizeof(gclst_pk.pri_key.nw_id));
        memcpy(gclst_pk.pri_key.grp_id, gflin_pk->group_name, sizeof(gclst_pk.pri_key.grp_id));
        memcpy(gclst_pk.pri_key.if_id, gflin_pk->interface_name, sizeof(gclst_pk.pri_key.if_id));
        memcpy(gclst_pk.pri_key.station_id, gflin_pk->station_name, sizeof(gclst_pk.pri_key.station_id));
        memcpy(gclst_pk.pri_key.connect_id, gflin_pk->connection_name, sizeof(gclst_pk.pri_key.connect_id));
        pkey = &gclst_pk;
        key_len = sizeof(gclst_pk.pri_key);
        cmp_len = sizeof(gclst_pk.pri_key.site_id) + sizeof(gclst_pk.pri_key.nw_id) +
                  sizeof(gclst_pk.pri_key.grp_id);
        if (gflin_pk->interface_name[0] != ' ') {
            cmp_len += sizeof(gclst_pk.pri_key.if_id);
        }
        if (gflin_pk->station_name[0] != ' ') {
            cmp_len += sizeof(gclst_pk.pri_key.station_id);
        }
        if (gflin_pk->connection_name[0] != ' ') {
            cmp_len += sizeof(gclst_pk.pri_key.connect_id);
        }
        rec_len = db_gclst_def_Size;
        ipcdrec_len = sizeof(gclst_data_def);
        rec_max =  DEF_CMD_DATA_SIZE / ipcdrec_len;
        p = (char*)&((r501_cn_st_def*)g_myinfo.replyinfo.replybuf)->gclst_data;
        file_kind = EFILEGCLST;
    // 局状態照会
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_STS_DSP) {
        memcpy(logical_fname, DEF_FL_CEN_STS, sizeof(logical_fname));
        // Key設定
        memcpy(&gcsst_pk.pri_key.site_id, &gflin_pk->site_name, sizeof(gcsst_pk.pri_key.site_id));
        memcpy(&gcsst_pk.pri_key.nw_id, &gflin_pk->nw_name, sizeof(gcsst_pk.pri_key.nw_id));
        memcpy(gcsst_pk.pri_key.grp_id, gflin_pk->group_name, sizeof(gcsst_pk.pri_key.grp_id));
        memcpy(gcsst_pk.pri_key.if_id,  gflin_pk->interface_name, sizeof(gcsst_pk.pri_key.if_id));
        memcpy(gcsst_pk.pri_key.station_id, gflin_pk->station_name, sizeof(gcsst_pk.pri_key.station_id));
        memcpy(gcsst_pk.pri_key.connect_id, gflin_pk->connection_name, sizeof(gcsst_pk.pri_key.connect_id));
        pkey = &gcsst_pk;
        key_len = sizeof(gcsst_pk.pri_key);
        cmp_len = sizeof(gcsst_pk.pri_key.site_id) + sizeof(gcsst_pk.pri_key.nw_id) +
                  sizeof(gcsst_pk.pri_key.grp_id);
        if (gflin_pk->interface_name[0] != ' ') {
            cmp_len += sizeof(gcsst_pk.pri_key.if_id);
        }
        if (gflin_pk->station_name[0] != ' ') {
            cmp_len += sizeof(gcsst_pk.pri_key.station_id);
        }
        if (gflin_pk->connection_name[0] != ' ') {
            cmp_len += sizeof(gcsst_pk.pri_key.connect_id);
        }
        rec_len = db_gcsst_def_Size;
        ipcdrec_len = sizeof(gcsst_data_def);
        rec_max =  DEF_CMD_DATA_SIZE / ipcdrec_len;
        p = (char*)&((r501_sta_st_def*)g_myinfo.replyinfo.replybuf)->gcsst_data;
        file_kind = EFILEGCSST;
    // エコーステータス照会
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_ECH_STS_DSP) {
        memcpy(logical_fname, DEF_FL_ECH_STS, sizeof(logical_fname));
        // Key設定
        memcpy(&gcest_pk.pri_key.site_id, &gflin_pk->site_name, sizeof(gcest_pk.pri_key.site_id));
        memcpy(&gcest_pk.pri_key.nw_id, &gflin_pk->nw_name, sizeof(gcest_pk.pri_key.nw_id));
        memcpy(gcest_pk.pri_key.grp_id, gflin_pk->group_name, sizeof(gcest_pk.pri_key.grp_id));
        memcpy(gcest_pk.pri_key.if_id,  gflin_pk->interface_name, sizeof(gcest_pk.pri_key.if_id));
        memcpy(gcest_pk.pri_key.station_id, gflin_pk->station_name, sizeof(gcest_pk.pri_key.station_id));
        memcpy(gcest_pk.pri_key.connect_id, gflin_pk->connection_name, sizeof(gcest_pk.pri_key.connect_id));
        pkey = &gcest_pk;
        key_len = sizeof(gcest_pk.pri_key);
        cmp_len = sizeof(gcest_pk.pri_key.site_id) + sizeof(gcest_pk.pri_key.nw_id) +
                  sizeof(gcest_pk.pri_key.grp_id)  + sizeof(gcest_pk.pri_key.if_id);
        if (gflin_pk->station_name[0] != ' ') {
            cmp_len += sizeof(gcest_pk.pri_key.station_id);
        }
        if (gflin_pk->connection_name[0] != ' ') {
            cmp_len += sizeof(gcest_pk.pri_key.connect_id);
        }
        rec_len = db_gcest_def_Size;
        ipcdrec_len = sizeof(gcest_data_def);
        rec_max =  DEF_CMD_DATA_SIZE / ipcdrec_len;
        p = (char*)&((r501_echo_st_def*)g_myinfo.replyinfo.replybuf)->gcest_data;
        file_kind = EFILEGCEST;
    }

    // 対象ファイル名取得
    if (gflin_pk->site_name == DEF_SITE_ID_TKY) {
        site = ESITEEAST;
    } else {
        site = ESITEWEST;
    }
    memset(target_fname, NULL, sizeof(target_fname));
    memcpy(target_fname, g_fileinfo[file_kind].name[site], strlen(g_fileinfo[file_kind].name[site]));

    // 対象ファイル読込
    // COM_IOMファイル情報設定
    memset(&iom_arg4, NULL, sizeof(iom_arg4));
    memcpy(iom_arg4.file_id, logical_fname, strlen(logical_fname));
    memset(iom_arg4.file_name, ' ', sizeof(iom_arg4.file_name));
    memcpy(iom_arg4.file_name, target_fname, strlen(target_fname));
    memset(&iom_arg5, NULL, sizeof(iom_arg5));
    iom_arg5.io_timer = g_myinfo.fio_timer;
    memset(&iom_arg6, NULL, sizeof(iom_arg6));
    // 対象ファイルオープン
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_OPEN, sizeof(DEF_COM_IOM_FIO_OPEN)-1);
    ret = COM_IOM(DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE, sizeof(DEF_NERR_CMD_ERR_DONE)-1);
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_OPN_ERR, sizeof(DEF_NERR_FILE_OPN_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);
        return;
    }

    // COM_IOM入力情報設定
    iom_arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    memcpy(iom_arg5.key_value, pkey, key_len);
    memcpy(iom_arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    iom_arg5.key_len = key_len;
    iom_arg5.compare_len = cmp_len;
    iom_arg5.positioning_mode = DEF_COM_IOM_GENERIC;
    iom_arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_arg5.rec_len = rec_len;

    // READ開始
    rec_cnt = 0;
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_START, sizeof(DEF_COM_IOM_FIO_START)-1);
    ret = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
    while (ret == 0) {
        // EOF判定
        if (!memcmp(sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(DEF_COM_IOM_EOF_ERR)-1)) {
            break;
        }
        // 最大数判定
        if (rec_cnt >= rec_max) {
            /*------------------------------------------------*/
            /*    IPC編集                                     */
            /*------------------------------------------------*/
            // 内部エラーコード
            memcpy(r501->common_header.internal_error_code, DEF_NERR_EXCEEDING_UPPER_LIMIT,
                sizeof(DEF_NERR_EXCEEDING_UPPER_LIMIT)-1);
            break;
        }
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // コネクションステータス照会
        if (g_myinfo.command_code == DEF_IPC_CMD_CD_STS_DSP) {
            gclst_data_def *r501_cn = &((gclst_data_def*)p)[rec_cnt];
            db_gclst_def *pgclst = (db_gclst_def*)iom_arg6.rec_area;
            memcpy(r501_cn, pgclst, sizeof(gclst_data_def));
        // 局状態照会
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_CNT_STS_DSP) {
            gcsst_data_def *r501_sta = &((gcsst_data_def*)p)[rec_cnt];
            db_gcsst_def *pgcsst = (db_gcsst_def*)iom_arg6.rec_area;
            memcpy(&r501_sta->connection_logical_name, &pgcsst->pri_key, sizeof(r501_sta->connection_logical_name));
            memcpy(&r501_sta->nw_id_info, &pgcsst->nw_id_info, sizeof(r501_sta->nw_id_info));
            memcpy(&r501_sta->state_sts_info, &pgcsst->state_sts_info, sizeof(r501_sta->state_sts_info));
        // エコーステータス照会
        } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_ECH_STS_DSP) {
            gcest_data_def *r501_echo = &((gcest_data_def*)p)[rec_cnt];
            db_gcest_def *pgcest = (db_gcest_def*)iom_arg6.rec_area;
            memcpy(&r501_echo->connection_logical_name, &pgcest->pri_key, sizeof(r501_echo->connection_logical_name));
            memcpy(&r501_echo->echo_info, &pgcest->echo_info, sizeof(r501_echo->echo_info));
        }
        rec_cnt++;
        // NEXTREAD
        memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_READ, sizeof(DEF_COM_IOM_FIO_READ)-1);
        ret = COM_IOM(DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
    }
    if (ret == 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        r501->common_header.control_data_length += (ipcdrec_len * rec_cnt);
        r501->record_count = rec_cnt;
        g_myinfo.replyinfo.reply_len += (ipcdrec_len * rec_cnt);
    } else {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE,
               sizeof(DEF_NERR_CMD_ERR_DONE)-1);
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts));
        GFPOGGZ1(&g_oggz1in);
    }
    // 対象ファイルクローズ
    memcpy(g_myinfo.iom_trace.file_io_type, DEF_COM_IOM_FIO_CLOSE, sizeof(DEF_COM_IOM_FIO_CLOSE)-1);
    COM_IOM(DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &g_myinfo.iom_trace, &iom_arg4, &iom_arg5, &iom_arg6);
}

/****************************************************************************/
/*  FUNCTION        : 8.0.0  CMDS_setmsgid                                  */
/*  CALLING SEQ.    : void  CMDS_setmsgid(short msgid)                      */
/*  ARGUMENT        : 1. msgid         (I) メッセージID                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : EMS出力モジュールI/F 任意メッセージ部初期化           */
/****************************************************************************/
void CMDS_setmsgid(short msgid)
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
/*  FUNCTION        : 8.0.1  CMDS_getparam                                  */
/*  CALLING SEQ.    : void  CMDS_getparam(char *prmid, char *val,           */
/*                                        size_t len, short is_abend)       */
/*  ARGUMENT        : 1. prmid         (I) パラメータID                     */
/*                  : 2. val           (O) 取得値                           */
/*                  : 3. len           (I) 取得値バッファレングス           */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得＆EMS出力　エラー時はABEND              */
/****************************************************************************/
void CMDS_getparam(char *prmid, char *val, size_t len)
{
    short ret;
    char buffer[1024];

    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(prmid, buffer, (short)len);
    if (ret != 0) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
        // ③パラメータ名(20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, prmid, strlen(prmid));
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%04d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);
        // 異常終了処理
        CMDS_abend();
    }
    memset(val, NULL, len);
    memcpy(val, buffer, strlen(buffer));
}

/****************************************************************************/
/*  FUNCTION        : 8.0.2  CMDS_validate_res_ipc                          */
/*  CALLING SEQ.    : void  CMDS_validate_res_ipc(r502_def* r502,           */
/*                                                destinfo_def* destinfo)   */
/*  ARGUMENT        : 1. r502          (I) レスポンスIPC                    */
/*                  : 2. destinfo      (I) 送信先情報                       */
/*  RETURN CODE     : 正常: 0 ; 異常: other                                 */
/*  DESCRIPTION     : レスポンスIPC内容精査処理 異常の場合、メッセージ出力  */
/****************************************************************************/
short CMDS_validate_res_ipc(r502_def* r502, destinfo_def* destinfo)
{
    short ret = DEF_CHK_IPC_OK;
    char  rcv_ipc_hex[(sizeof(common_header_def)<<1)+1];
    char  err_detail[DEF_EVT_RSP_ERR_DETAIL_LEN+1];
    char  internal_error_code[sizeof(r502->common_header.internal_error_code)+1];

    memset(err_detail, NULL, sizeof(err_detail));
    memset(internal_error_code, NULL, sizeof(internal_error_code));
    // インターフェースコードチェック
    if (memcmp(r502->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP, sizeof(DEF_IPC_IFCD_CMD_PRC_RSP)-1)) {
        memcpy(err_detail, DEF_EMS_IPC_CHK_IFCD, sizeof(DEF_EMS_IPC_CHK_IFCD)-1);
    // データ長チェック
    } else if (r502->common_header.control_data_length != 0) {
        memcpy(err_detail, DEF_EMS_IPC_CHK_LEN, sizeof(DEF_EMS_IPC_CHK_LEN)-1);
    // エラーコード・内部エラーコードチェック
    } else if (r502->common_header.error_code != DEF_IPC_ERRCD_OK || memcmp(r502->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1)) {
        memcpy(internal_error_code, r502->common_header.internal_error_code, sizeof(r502->common_header.internal_error_code));
        snprintf(err_detail, sizeof(err_detail), DEF_EMS_IPC_CHK_ERRCD_FMT, r502->common_header.error_code, internal_error_code);
    }
    if (err_detail[0] != NULL) {
        /*------------------------------------------------*/
        /*    メッセージ出力：応答エラー                  */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_RSP_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_IPC_SEISA_ERR, sizeof(DEF_NERR_IPC_SEISA_ERR)-1);
        // ③LCN (15バイト)
        // ④送信先サーバークラス (16バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, destinfo->srv_cls_kind,
            sizeof(destinfo->srv_cls_kind) + sizeof(destinfo->srv_cls_num) + sizeof(destinfo->srv_cls_mlt_num));
        // ⑤エラー内容 (20バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, err_detail, strlen(err_detail));
        // ⑥受信IPC内容(48バイト)
        CMDS_strtohex((char*)r502, rcv_ipc_hex, sizeof(common_header_def));
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, rcv_ipc_hex, sizeof(rcv_ipc_hex)-1);
        GFPOGGZ1(&g_oggz1in);
        ret = DEF_CHK_IPC_NG;
    }

    return ret;
}

/****************************************************************************/
/*  FUNCTION        : 8.0.3  CMDS_forward_process                           */
/*  CALLING SEQ.    : void  CMDS_forward_process(short* fwd_cnt,            */
/*                                               short* err_cnt)            */
/*  ARGUMENT        : void                                                  */
/*  ARGUMENT        : 1. fwd_cnt       (O) 転送数                           */
/*                  : 2. err_cnt       (O) 転送エラー数                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンド転送処理（プロセス）                          */
/****************************************************************************/
void CMDS_forward_process(short* fwd_cnt, short* err_cnt)
{
    short             i;
    short             ret;
    short             fno;
    short             err;
    short             prockind;
    unsigned short    len;
    long              addr;
    char              site_id;
    char              process_name[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];
    char              buffer[64];
    _cc_status        cc;
    c501_def*         c501 = (c501_def*)g_myinfo.recvbuf;
    r501_def*         r501 = (r501_def*)g_myinfo.replyinfo.replybuf;
    c502_def          c502;
    r502_def*         r502;
    gflin_pkey_def*   gflin_pk = (gflin_pkey_def*)&c501->command_info.connection_logical_name;
    destinfo_def*     destinfo;
    static const char *procpr[EPROCNUM] = {DEF_PRC_FURI_I, DEF_PRC_FURI_O, DEF_PRC_LOG_OUT};

    /*------------------------------------------------*/
    /*    対象プロセス種別                            */
    /*------------------------------------------------*/
    // ログファイル切替
    if (g_myinfo.command_code == DEF_IPC_CMD_CD_LOG_FL_EXC) {
        prockind = EPROCLOGSV;
    // ファイル再読込 接続構成変更
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN) {
        // サーバクラス論理ID指定で電文振分(outbound)ではない場合、何もしない
        if (c501->command_info.srv_cls_id[0] != ' ' &&
            memcmp(c501->command_info.srv_cls_id, DEF_SC_FURI_O, sizeof(DEF_SC_FURI_O)-1)) {
            return;
        }
        prockind = EPROCMSDSO;
    // ファイル再読込 東阪振分比率変更
    } else if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_NSW) {
        prockind = EPROCMSDSI;
    }

    // サイト
    if (gflin_pk->site_name == ' ') {
        site_id = g_myinfo.site_id[0];
    } else {
        site_id = gflin_pk->site_name;
    }
    if (site_id == DEF_SITE_ID_TKY) {
        destinfo = g_dest_process[ESITEEAST][prockind];
    } else {
        destinfo = g_dest_process[ESITEWEST][prockind];
    }

    for (i = 0; i < DEF_PROC_SC_NUM_MAX; ++i) {
        // end of data
        if (destinfo[i].srv_cls_kind[0] == NULL) {
            break;
        }
        // コマンド処理要求IPC編集
        memcpy(&c502, c501, sizeof(c502_def));
        memcpy(c502.common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_REQ, sizeof(DEF_IPC_IFCD_CMD_PRC_REQ)-1);
        // ファイル再読込の場合、転送先にコマンド識別"4010"を設定
        if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN || g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_NSW) {
            memcpy(c502.command_info.command_name, DEF_IPC_CMD_FL_RE_READ, sizeof(DEF_IPC_CMD_FL_RE_READ)-1);
        }
        c502.common_header.control_data_length = sizeof(c502_def) - sizeof(common_header_def);
        // プロセスW/R
        // 送信先プロセスオープン
        memset(process_name, NULL, sizeof(process_name));
        memcpy(process_name, destinfo[i].prc_file_name, sizeof(process_name)-1);
        CMDS_strtrim(process_name);
        // クオリファイア付与
        strncat(process_name, DEF_CMDS_QUORIFIRE, ZSYS_VAL_LEN_UNIQUEPROCESSNAME-strlen(process_name)-1);
        ret = FILE_OPEN_(process_name, (short)strlen(process_name), &fno, , , 1); 
        if (ret) {
            /*------------------------------------------------*/
            /*    メッセージ出力：プロセスI/Oエラー           */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_PROC_IO_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PROC_OPN_ERR, sizeof(DEF_NERR_PROC_OPN_ERR)-1);
            // ⑤プロセス論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, procpr[prockind], 8);
            // ⑥アクション (19バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "OPEN", sizeof("OPEN")-1);
            // ⑧エラーコード (5バイト)
            snprintf(buffer, sizeof(buffer), "%05d", ret);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);
            (*err_cnt)++;
            continue;
        }
        cc = WRITEREADX(fno, (char *)&c502, sizeof(c502), sizeof(c502), &len);
        if (_status_eq(cc)) {
            cc = AWAITIOX(&fno, &addr, &len, , g_myinfo.pwr_timer);
        }
        if (_status_ne(cc)) {
            FILE_GETINFO_(fno, &err);
            /*------------------------------------------------*/
            /*    メッセージ出力：プロセスI/Oエラー           */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            CMDS_setmsgid(DEF_EVT_PROC_IO_ERR);
            // 内部エラーコード
            memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_FILE_IO_ERR, sizeof(DEF_NERR_FILE_IO_ERR)-1);
            // ⑤プロセス論理名 (8バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, procpr[prockind], 8);
            // ⑥アクション (19バイト)
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "WRITEREAD", sizeof("WRITEREAD")-1);
            // ⑧エラーコード (5バイト)
            snprintf(buffer, sizeof(buffer), "%05d", err);
            memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&g_oggz1in);
            (*err_cnt)++;
        } else {
            err = CMDS_validate_res_ipc((r502_def*)&c502, &destinfo[i]);
            if (err != 0) {
                (*err_cnt)++;
            } else {
                (*fwd_cnt)++;
            }
        }
        FILE_CLOSE_(fno);
    }
    if ((*err_cnt) != 0) {
        /*------------------------------------------------*/
        /*    IPC編集                                     */
        /*------------------------------------------------*/
        // エラーコード
        r501->common_header.error_code = DEF_IPC_ERRCD_NG;
        // 内部エラーコード
        memcpy(r501->common_header.internal_error_code, DEF_NERR_CMD_ERR_DONE,
               sizeof(DEF_NERR_CMD_ERR_DONE)-1);
    }
}

/****************************************************************************/
/*  FUNCTION        : 8.0.4  CMDS_get_destinfo                              */
/*  CALLING SEQ.    : void  CMDS_get_destinfo()                             */
/*  ARGUMENT        : void                                                  */
/*  ARGUMENT        : 1. lindt   (I)  回線ファイルデータ                    */
/*                  : 2. dest    (I)  送信先情報                            */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 送信先情報取得処理                                    */
/****************************************************************************/
short CMDS_get_destinfo(gflindata_def* lindt, destinfo_def** dest)
{
    short           i, ret = DEF_FLG_NG;
    destinfo_def*   destinfo;

    if (lindt->pri_key.site_name == DEF_FURI_DST_TKY) {
        destinfo = g_dest_connctrl[ESITEEAST];
    } else {
        destinfo = g_dest_connctrl[ESITEWEST];
    }
    for (i = 0; i < DEF_CONNCTRL_NUM_MAX; ++i) {
        if (destinfo[i].srv_cls_kind[0] == NULL) {
            break;
        }
        if (!memcmp(lindt->srv_cls_kind, destinfo[i].srv_cls_kind, sizeof(destinfo[i].srv_cls_kind)) &&
            !memcmp(lindt->srv_cls_num, destinfo[i].srv_cls_num, sizeof(destinfo[i].srv_cls_num))) {
            *dest = &destinfo[i];
            ret = DEF_FLG_OK;
            break;
        }
    }
    if (ret != DEF_FLG_OK) {
        /*------------------------------------------------*/
        /*    メッセージ編集：設定情報エラー              */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_CONFIG_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PRM_RD_ERR_INV, sizeof(DEF_NERR_PRM_RD_ERR_INV)-1);
        // ③ファイル論理名 (8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
        // ④キー (40バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, lindt, sizeof(gflindata_def));
        // ⑤エラー項目/エラー理由 (40バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, DEF_EMS_REC_NOT_EXISTS, sizeof(DEF_EMS_REC_NOT_EXISTS)-1);
        GFPOGGZ1(&g_oggz1in);
    }
    return ret;
}

/****************************************************************************/
/*  FUNCTION        : 8.0.4  CMDS_pathsend                                  */
/*  CALLING SEQ.    : void  CMDS_pathsend(destinfo_def* dest,               */
/*                                        void* conn_id, size_t len))       */
/*  ARGUMENT        : 1. dest    (I)  送信先情報                            */
/*                  : 2. conn_id (I)  コネクション論理ID                    */
/*                  : 3. len     (I)  コネクション論理ID長                  */
/*  RETURN CODE     : 正常: 0 ; 異常: other                                 */
/*  DESCRIPTION     : IPC編集、PATHSEND処理 異常の場合、メッセージ出力      */
/****************************************************************************/
short CMDS_pathsend(destinfo_def* dest, void* conn_id, size_t len)
{
    short               ret;
    char                buffer[64];
    c502_def            c502;
    r502_def*           r502;
    COM_PSD_arg_1_def   psd_arg1;
    COM_PSD_arg_2_def   psd_arg2;
    COM_PSD_arg_3_def   psd_arg3;
    COM_PSD_arg_4_def   psd_arg4;

    // PATHSENDエラーでも送信済にする
    dest->is_forwarded = DEF_FLG_ON;
    // コマンド処理要求IPC編集
    memset(&c502, ' ', sizeof(c502_def));
    memcpy(c502.common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_REQ, sizeof(c502.common_header.interface_code));
    c502.common_header.error_code = DEF_IPC_ERRCD_OK;
    memcpy(c502.common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL)-1);
    c502.common_header.control_data_length = sizeof(c502_def) - sizeof(common_header_def);
    // コマンド識別
    if (g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_LIN || g_myinfo.command_code == DEF_IPC_CMD_CD_FL_RE_NSW) {
        memcpy(c502.command_info.command_name, DEF_IPC_CMD_FL_RE_READ, sizeof(c502.command_info.command_name));
    } else {
        memcpy(c502.command_info.command_name, ((c501_def*)g_myinfo.recvbuf)->command_info.command_name, sizeof(c502.command_info.command_name));
    }
    // コネクション論理ID
    memcpy(&c502.command_info.connection_logical_name, conn_id, len);
    // PATHSEND
    memset(&psd_arg1, ' ', sizeof(psd_arg1));
    // 制御電文振分：ドメイン名、左記以外：PATHMON名
    if (!memcmp(dest->srv_cls_kind, DEF_SC_CTRL_FURI, sizeof(DEF_SC_CTRL_FURI)-1)) {
        memcpy(psd_arg1.pathmon_name, dest->domain_name, sizeof(dest->domain_name));
    } else {
        memcpy(psd_arg1.pathmon_name, dest->pathmon_name, sizeof(dest->pathmon_name));
    }
    memcpy(psd_arg1.serverclass_name, dest->srv_cls_name, sizeof(dest->srv_cls_name));
    memcpy(psd_arg1.msg_buf, &c502, sizeof(c502));
    psd_arg1.req_send_len = sizeof(c502);
    psd_arg1.receive_max_len = sizeof(c502);
    psd_arg1.send_timer_msec = g_myinfo.psd_timer;
    psd_arg1.retry_cnt = g_myinfo.psd_retrycnt;
    memset(&psd_arg2, ' ', sizeof(psd_arg2));
    memcpy(psd_arg2.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    memset(&psd_arg4, ' ', sizeof(psd_arg4));
    memcpy(psd_arg4.srv_logical_id, DEF_MY_SCID, sizeof(DEF_MY_SCID)-1);
    ret = COM_PSD(&psd_arg1, &psd_arg2, &psd_arg3, &g_oggz1in, &psd_arg4);
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        CMDS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // 内部エラーコード
        memcpy(g_oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_PSEND_ERR_RE_OUT, sizeof(DEF_NERR_PSEND_ERR_RE_OUT)-1);
        // ③モジュールID(8バイト)
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCGX40, sizeof(DEF_GFPCGX40)-1);
        // ④エラーコード(4バイト)
        snprintf(buffer, sizeof(buffer), "%05d", ret);
        memcpy(g_oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&g_oggz1in);
        return DEF_FLG_NG;
    }
    return CMDS_validate_res_ipc((r502_def*)psd_arg1.msg_buf, dest);
}

/****************************************************************************/
/*  FUNCTION        : 9.0.0  CMDS_strtohex                                  */
/*  CALLING SEQ.    : void CMDS_strtohex(const char* input, char* output,   */
/*                                       short len)                         */
/*  ARGUMENT        : 1. input  (I)  文字列                                 */
/*                  : 2. output (O)  hex                                    */
/*                  : 3. len    (I)  処理長                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 文字列のhex変換モジュール                             */
/****************************************************************************/
void CMDS_strtohex(const char* input, char* output, short len)
{
    int i;
    //char to hexを文字数分繰り返す
    for (i=0; i<len; i++) {
        sprintf(output + (i * 2), "%02X", input[i]);
    }
    output[i * 2] = NULL;
}

/****************************************************************************/
/*  FUNCTION        : 9.0.1  CMDS_atoii                                     */
/*  CALLING SEQ.    : void  CMDS_atoii(const char *, size_t)                */
/*  ARGUMENT        : 1. p             (I) 文字列                           */
/*                  : 2. n             (I) 文字列長                         */
/*  RETURN CODE     : 変換したint値                                         */
/*  DESCRIPTION     : 指定文字列長の文字列pをintに変換する                  */
/****************************************************************************/
int CMDS_atoii(const char *p, size_t len)
{
    long long n;
    int sign;
    int i;
    n = 0;
    for (i = 0; isspace(p[i]) && i < len; i++) ;
    if (i >= len) return (int)n;
    sign = (p[i] == '-') ? -1 : 1;
    if (p[i] == '+' || p[i] == '-') i++;
    for ( ; i < len; i++) {
        n = 10 * n + (p[i] - '0');
        if (sign == 1 && n >= INT_MAX) return INT_MAX;
        if (sign == -1 && (sign * n) <= INT_MIN) return INT_MIN;
    }

    return (int)(sign * n);
}

/****************************************************************************/
/*  FUNCTION        : 9.0.2  CMDS_strtrim                                   */
/*  CALLING SEQ.    : void  CMDS_strtrim(char *)                            */
/*  ARGUMENT        : 1. p             (I/O) 文字列                         */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 文字列pの後ろから、スペースを'\0'に変換               */
/****************************************************************************/
void CMDS_strtrim(char *p)
{
    char *pEnd;

    for (pEnd = strchr(p, '\0') - 1
       ; pEnd >= p && *pEnd == ' '
       ; pEnd--); // empty loop body
        *++pEnd = '\0';
}
