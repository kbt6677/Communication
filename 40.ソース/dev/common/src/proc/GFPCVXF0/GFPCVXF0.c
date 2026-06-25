/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCVXF0                                    */
/*        FUNCTION          ････ ログ出力                                    */
/*                                                                           */
/*                               電文振分(inbound)、電文振分(outbound)から   */
/*                               電文ログ出力要求を受信し                    */
/*                               NW通信ログにログ出力を行う。                */
/*                                                                           */
/*        AUTHER            ････ ISYS K.Tanigawa                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-09-26                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.Tanigawa 2024/09/26 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdio.h>     nolist
#include  <stdlib.h>    nolist
#include  <string.h>    nolist
#include  <errno.h>     nolist
#include  <math.h>      nolist
#include  <tal.h>       nolist
#include  <cextdecs.h>  nolist
#include  <zspic>       nolist
#include  <zfilc>       nolist
/* USER HEADER */
#include "file.h"       nolist      // file
#include "ems.h"        nolist      // ems
#include "GFPOGGZ4_traceout.h" nolist   // trace
#include "errcd.h"      nolist      // error code
#include "ipc.h"        nolist      // ipc
#include "GFPCVXF0.h"   nolist      // ログ出力
#include "GFPCGX50.h"   nolist      // システム日時取得
#include "GFPCGXA0.h"   nolist      // TMF管理
#include "GFPCGXB0.h"   nolist      // IOモジュール
#include "GFPCGXD0.h"   nolist      // ASSIGN情報取得
#include "vproc.h"      nolist      // vproc

/* GLOBAL */
static myinfo_def     g_myinfo;
static fileinfo_def   g_fileinfo;
static name_table_def g_name_tbl[31];     // 物理名情報ファイルから取得
                                          // NW通信ログ物理名メモリテーブル
/* IOモジュール用 */
static COM_IOM_arg_3_def iom_trc;         // トレース情報
static COM_IOM_arg_4_def iom_fil;         // ファイル情報
static COM_IOM_arg_5_def iom_inp;         // 入力情報
static COM_IOM_arg_6_def iom_out;         // 出力情報

static _lowmem oggz1in_def oggz1in;
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
    LOGS_init();

    /*------------------------------------------------*/
    /*    主処理                                      */
    /*------------------------------------------------*/
    LOGS_control();

    /*------------------------------------------------*/
    /*    終了処理                                    */
    /*------------------------------------------------*/
    LOGS_final();

    return 0;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  LOGS_init                                      */
/*  CALLING SEQ.    : void  LOGS_init(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void LOGS_init(void)
{
    short ret, len, i;
    short wkdate[8];
    short yday[3];  /* 前日 yyyy mm dd */
    short oday[3];  /* 当日 yyyy mm dd */
    short tday[3];  /* 翌日 yyyy mm dd */
    long long ll_timestamp;
    char buffer[64], yyyymmdd[8];
    char cnvstr[64];

    openers_mgr_def* op_mgr = &g_myinfo.op_mgr;
    lk_zac2001r_arg_1_def trace_if;

    // 変数初期化
    memset( &g_myinfo, NULL, sizeof( g_myinfo ));
    memset( &g_fileinfo, NULL, sizeof( g_fileinfo ));
    memset( g_name_tbl, NULL, sizeof( g_name_tbl ));

    PROCESSHANDLE_NULLIT_( g_myinfo.procinfo.my_phandle  );
    PROCESSHANDLE_NULLIT_( g_myinfo.procinfo.ans_phandle );

    // EMS出力モジュールI/F用変数初期化
    memset(&oggz1in, ' ', sizeof(oggz1in));
    // リターンコード
    memcpy(&oggz1in.emsinf.rcd, DEF_EMS_DEF_RET_CODE, sizeof(oggz1in.emsinf.rcd));
    // メッセージ通知区分 システムエラーを設定しておく
    oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    // システム名(GFP)
    memcpy(oggz1in.emsinf.emsgkinf.sysnm, DEF_EMS_SYSNM_GFP, sizeof(DEF_EMS_SYSNM_GFP)-1);
    // 
    memcpy(oggz1in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM, sizeof(DEF_EMS_SRV_KBN_COM)-1);
    // メッセージ出力元プログラム名
    memcpy(oggz1in.emsinf.emsgkinf.prgid, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);

    // トレース出力モジュール初期化
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    trace_if.func_flg = DEF_TRACE_FUNC_INI;
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    ret = TRACEOUT((char *)&trace_if);

    // 運用監視端末出力サーバ・サーバクラス名取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_MSG_SRV_NAME, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // ③パラメータ名(20バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_MSG_SRV_NAME,
               sizeof(DEF_MSG_SRV_NAME)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }
    memcpy (oggz1in.uytrminf.uytrmsrv, buffer, strlen(buffer));
    sprintf(cnvstr, "%02d", strlen(buffer));
    memcpy (oggz1in.uytrminf.uytrmsrvlen, cnvstr, strlen(cnvstr));

    // 運用監視端末出力サーバ・PATHMON名取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_MSG_MON_NAME, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // ③パラメータ名(20バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_MSG_MON_NAME,
               sizeof(DEF_MSG_MON_NAME)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }
    memcpy (oggz1in.uytrminf.uytrmmon, buffer, strlen(buffer));
    sprintf(cnvstr, "%02d", strlen(buffer));
    memcpy (oggz1in.uytrminf.uytrmmonlen, cnvstr, strlen(cnvstr));

    // PATHSENDタイマー取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_PSEND_TIMER_10MSECOND, buffer,
                            (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // ③パラメータ名(20バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_PSEND_TIMER_10MSECOND,
               sizeof(DEF_PSEND_TIMER_10MSECOND)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }
    // EMSのタイマー値：秒単位(10msec → sec切り上げ)
    sprintf(cnvstr, "%04d", (int)ceil((double)atoi(buffer) / 100));
    memcpy (oggz1in.uytrminf.proctimer, cnvstr, strlen(cnvstr));


    /*プロセス情報取得 */
    ret = COM_PRC( &g_myinfo.procinfo );
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXG0, sizeof(DEF_GFPCGXG0)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }
    memcpy(oggz1in.emsinf.emsgkinf.trmnm, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));

    /* 自サーバクラス論理ID取得処理 */
    memset( buffer, NULL, sizeof( buffer ));
    ret = get_param_by_name( DEF_SRV_LOGICAL_ID, buffer, (short)sizeof(buffer) );
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // ③パラメータ名(20バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_SRV_LOGICAL_ID,
               sizeof(DEF_SRV_LOGICAL_ID)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }

    /* 取得した自サーバクラス論理IDを分割する X-X-XXXXX-XXXXXXXX-XXXX*/
    memcpy( g_myinfo.site_id,    &buffer[0], sizeof( g_myinfo.site_id ) );
    memcpy( g_myinfo.network_id, &buffer[2], sizeof( g_myinfo.network_id ) );
    memcpy( g_myinfo.group_id,   &buffer[4], sizeof( g_myinfo.group_id ) );
    memcpy( g_myinfo.serverclass_id, &buffer[DEF_SC_OFFSET_SCKND], DEF_SC_KIND_LEN);
    memcpy( &g_myinfo.serverclass_id[DEF_SC_KIND_LEN], &buffer[DEF_SC_OFFSET_SCNUM],
            sizeof(g_myinfo.serverclass_id) - DEF_SC_KIND_LEN);

    // network_idを変換し、EMSモジュールのNW識別へ設定
    switch (g_myinfo.network_id[0]) {
    // JCN(CUP)
    case DEF_NW_ID_JCN:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_CARDNET, sizeof(DEF_NW_KUBUN_CARDNET)-1);
        break;
    // Visanet
    case DEF_NW_ID_VISA:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_VISANET, sizeof(DEF_NW_KUBUN_VISANET)-1);
        break;
    // Banknet
    case DEF_NW_ID_MASTER:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_BANKNET, sizeof(DEF_NW_KUBUN_BANKNET)-1);
        break;
    // AEGEN
    case DEF_NW_ID_AMEX:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_AEGN, sizeof(DEF_NW_KUBUN_AEGN)-1);
        break;
    // Discover
    case DEF_NW_ID_DISCOVER:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_DISCOVER, sizeof(DEF_NW_KUBUN_DISCOVER)-1);
        break;
    // NYCE
    case DEF_NW_ID_NYCE:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_NYCE, sizeof(DEF_NW_KUBUN_NYCE)-1);
        break;
    // J-Link(通信受信時)
    case DEF_NW_ID_JLink:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_JLINK, sizeof(DEF_NW_KUBUN_JLINK)-1);
        break;
    // UnionPay
    case DEF_NW_ID_UnionPay:
        memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, DEF_NW_KUBUN_UNIONPAY, sizeof(DEF_NW_KUBUN_UNIONPAY)-1);
        break;
    default:
        break;
    }

    // ファイルIOタイマー取得
    memset(buffer, NULL, sizeof(buffer));
    ret = get_param_by_name(DEF_PSEND_TIMER_10MSECOND, buffer, (short)sizeof(buffer));
    if (ret) {
        /*------------------------------------------------*/
        /*    メッセージ出力：パラメータ取得エラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PARAM_GET_ERR);
        // ③パラメータ名(20バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_PSEND_TIMER_10MSECOND,
               sizeof(DEF_PSEND_TIMER_10MSECOND)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }
    g_myinfo.fio_timer = atol(buffer);


    /* NW通信ログ物理名メモリ展開 */
    LOGS_getname_log();

    /*ローカル時間(カレント日付)取得 */
    memset( wkdate, NULL, sizeof(wkdate));
    ll_timestamp = JULIANTIMESTAMP();
    ll_timestamp = CONVERTTIMESTAMP( ll_timestamp );
    INTERPRETTIMESTAMP( ll_timestamp, (short *)&wkdate[0] );
    sprintf( g_fileinfo.open_log_date,"%4d%02d%02d",  wkdate[0], wkdate[1], wkdate[2] );

    /* メモリ展開した内部テーブルのINDEX(日付)を取得 */
    /* 前日日付INDEX取得 */
    memset( yday, NULL, sizeof(yday));
    LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_YES, (short *)&yday[0] );

    /*当日日付INDEX取得 */
    memset( oday, NULL, sizeof(oday));
    oday[2] = wkdate[2];

    /*翌日日付INDEX取得 */
    memset( tday, NULL, sizeof(tday));
    LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_TOM, (short *)&tday[0] );

    /*ログテーブルにログファイル名を設定 */
    memcpy( g_fileinfo.log_table[DEF_IDX_YES].log_fname,
            g_name_tbl[(yday[2] - 1)].log_fname, DEF_FILENAME_LEN);
    memcpy( g_fileinfo.log_table[DEF_IDX_TOD].log_fname,
            g_name_tbl[(oday[2] - 1)].log_fname, DEF_FILENAME_LEN);
    memcpy( g_fileinfo.log_table[DEF_IDX_TOM].log_fname,
            g_name_tbl[(tday[2] - 1)].log_fname, DEF_FILENAME_LEN);

    /* 前日・当日・翌日ログオープン */
    for( i = 0; i < 3; i++ ) {
        LOGS_open_log( g_fileinfo.log_table[i].log_fname, &g_fileinfo.log_table[i].file_no );
    }

    /* $RECEIVEオープン */
    ret = FILE_OPEN_( DEF_RECEIVE_NAME
                     ,(short)sizeof(DEF_RECEIVE_NAME)-1
                     ,&g_myinfo.recv_fno
                     , 
                     , 
                     ,DEF_NOWAIT_DEPTH
                     ,DEF_RECV_DEPTH_MAX 
                     , ); 
    if ( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：ファイルI/Oエラー           */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_FILE_IO_ERR);
        // ⑤ファイル論理的名 (8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
               DEF_RECEIVE_NAME, sizeof(DEF_RECEIVE_NAME)-1);
        // ⑥アクション (19バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "OPEN", sizeof("OPEN")-1);
        // ⑧エラーコード (5バイト)
        sprintf(buffer, "%05d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        ///cccems
        /* 異常終了処理  */
        LOGS_abend();
    }

    // オープナープロセス管理初期化
    COM_STP_INIT(&op_mgr->opp_arg1);

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス起動                */
    /*------------------------------------------------*/
    // メッセージ通知区分 正常時は*
    oggz1in.emsinf.emsgkinf.msgttkb = '*';
    // 任意メッセージ部初期化
    LOGS_setmsgid(DEF_EVT_PROC_START);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&oggz1in);

    // メッセージ通知区分 システムエラーを設定しておく
    oggz1in.emsinf.emsgkinf.msgttkb = 'S';

}

/****************************************************************************/
/*  FUNCTION        : 2.0.0  LOGS_control                                   */
/*  CALLING SEQ.    : void  LOGS_control(void)                              */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void LOGS_control( void )
{
    g_myinfo.end_flg = DEF_FLG_OFF;

    /* プロセス終了フラグ=ONになるまでループする */
    while ( g_myinfo.end_flg != DEF_FLG_ON ){

        /* $RECEIVE読み込み */
        LOGS_recv();

        // イベント解析
        switch (g_myinfo.iocomp.errno) {
        // システムメッセージ処理
        case ZFIL_ERR_SYSMESS:
            LOGS_sys_msg();
            break;
        // 要求受信処理
        case ZFIL_ERR_OK:
            LOGS_req_msg();
            break;
        default:
            // nullリプライ
            memset(&g_fileinfo.reply_buf, NULL, sizeof(g_fileinfo.reply_buf));
            break;
        }

        /* リプライ処理 */
        LOGS_recv_reply();
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.1.0  LOGS_recv                                      */
/*  CALLING SEQ.    : short  LOGS_recv(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : $RECEIVE 受信時処理                                   */
/****************************************************************************/
void LOGS_recv(void)
{

    /* $RECEIVEのREADUPDATE */
    READUPDATEX( g_myinfo.recv_fno,
                 g_myinfo.recvbuf,
                 DEF_RCV_BUF_SIZE,
                 (unsigned short *)&g_myinfo.iocomp.len );

    /* AWAITIOX */
    AWAITIOX( &g_myinfo.iocomp.fno,
              &g_myinfo.iocomp.l_addr,
              (unsigned short *)&g_myinfo.iocomp.len,
              &g_myinfo.iocomp.l_tag );

    /* $RECEIVEの完了情報取得 */
    FILE_GETINFO_(g_myinfo.iocomp.fno, &g_myinfo.iocomp.errno);
    FILE_GETRECEIVEINFO_((short _far *)&g_myinfo.iocomp.recv_info);
}

/****************************************************************************/
/*  FUNCTION        : 2.2.0  LOGS_sys_msg                                   */
/*  CALLING SEQ.    : void  LOGS_sys_msg(void)                              */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : システムメッセージ処理                                */
/****************************************************************************/
void LOGS_sys_msg(void)
{
    short ret;
    char buffer[64];
    zsys_ddl_smsg_def *sysmsg = (zsys_ddl_smsg_def*)g_myinfo.recvbuf;
    openers_mgr_def* op_mgr = &g_myinfo.op_mgr;

    // システムメッセージ判定
    switch (sysmsg->u_z_msg.z_msgnumber[0]) {
    /* オープンメッセージ */
    case ZSYS_VAL_SMSG_OPEN:
        LOGS_open_sys();
        break;
   /* クローズメッセージ */
    case ZSYS_VAL_SMSG_CLOSE:
        LOGS_close_sys();
        break;
    // CPU、NODEダウンメッセージ
    case ZSYS_VAL_SMSG_CPUDOWN:
    case ZSYS_VAL_SMSG_REMOTECPUDOWN:
    case ZSYS_VAL_SMSG_NODEDOWN:
        // オープナープロセス管理
        ret = COM_STP_JUDGE( &op_mgr->opp_arg1, g_myinfo.recvbuf );
        if (ret != 0 && ret != 1) {
            /*------------------------------------------------*/
            /*    メッセージ出力：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // ③モジュールID(8バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            sprintf(buffer, "%04d", ret);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&oggz1in);

            // 異常終了処理
            LOGS_abend();
        }
    default :
        /* nullリプライ */
        memset(&g_fileinfo.reply_buf, NULL, sizeof(DEF_RPL_BUF_SIZE));
        break;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.2.1  LOGS_open_sys                                  */
/*  CALLING SEQ.    : void  LOGS_open_sys(void)                             */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープンメッセージ処理                                */
/****************************************************************************/
void LOGS_open_sys(void)
{
    char        pname[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];
    char        buffer[64];
    short       pname_len = 0;
    short       openid = 0;
    short       reply_cd = 0;
    short       ret;
    openers_mgr_def* op_mgr = &g_myinfo.op_mgr;
    zsys_ddl_smsg_open_reply_def* g_openrep_msg = (zsys_ddl_smsg_open_reply_def*)g_fileinfo.reply_buf;
    zsys_ddl_smsg_open_def *sys_msg_p;


    sys_msg_p = (zsys_ddl_smsg_open_def *)g_myinfo.recvbuf;
    memset(pname, NULL, sizeof(pname));

    /* オープン発行元プロセス名取得 */
    PROCESSHANDLE_DECOMPOSE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender,,,,,,,
        pname,
        sizeof(pname),
        &pname_len);

    /* オープン発行元プロセス名判定 */
    if (PROCESSHANDLE_COMPARE_(
        (short *)&g_myinfo.iocomp.recv_info.z_sender, 
         g_myinfo.procinfo.ans_phandle) != 0) {
        openid = DEF_OPENID_ANCESTOR;
        reply_cd = 0;
    } else if (memcmp(pname, "$ZL", 3) == 0) {
        openid = DEF_OPENID_ROUT;
        reply_cd = 0;
    } else if (memcmp(sys_msg_p->u_z_data.z_qualifier,"#GFPCI",6) == 0) {
        /* コマンドサーバからのオープン */
        openid = DEF_OPENID_ROUT;
        reply_cd = 0;
    } else {
        reply_cd = 48;
    }

    /* オープナープロセス管理 */
    if ( reply_cd == 0 ){
        ret = COM_STP_JUDGE( &op_mgr->opp_arg1, g_myinfo.recvbuf );
        if (ret != 0 && ret != 1) {
            /*------------------------------------------------*/
            /*    メッセージ出力：共通モジュールエラー        */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // ③モジュールID(8バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
            // ④エラーコード(4バイト)
            sprintf(buffer, "%04d", ret);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&oggz1in);

            /* 異常終了処理  */
            LOGS_abend();
        }
    }

    /* リプライメッセージ */
    g_openrep_msg->z_msgnumber = ZSYS_VAL_SMSG_OPEN;
    g_openrep_msg->z_openid = openid;
    g_fileinfo.reply_len = zsys_ddl_smsg_open_reply_def_Size;
    g_fileinfo.reply_code = reply_cd;
}

/****************************************************************************/
/*  FUNCTION        : 2.2.2  LOGS_close_sys                                 */
/*  CALLING SEQ.    : void  LOGS_close_sys(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : クローズメッセージ処理                                */
/****************************************************************************/
void LOGS_close_sys(void)
{
    short ret;
    openers_mgr_def* op_mgr = &g_myinfo.op_mgr;
    char buffer[64];

    /* オープナープロセス管理 */
    ret = COM_STP_JUDGE( &op_mgr->opp_arg1, g_myinfo.recvbuf );
    if (ret != 0 && ret != 1) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXC0, sizeof(DEF_GFPCGXC0)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        /* 異常終了処理  */
        LOGS_abend();
    }

    /* プロセス終了判定 */
    if ( ret == 1 ) {
        g_myinfo.end_flg = DEF_FLG_ON;
    }

    /* nullリプライ */
    memset( &g_fileinfo.reply_buf, NULL, sizeof( DEF_RPL_BUF_SIZE ));
}

/****************************************************************************/
/*  FUNCTION        : 2.3.0  LOGS_req_msg                                   */
/*  CALLING SEQ.    : void  LOGS_req_msg(void)                              */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リクエストメッセージ処理                              */
/****************************************************************************/
void LOGS_req_msg(void)
{
    c601_def *request  = (c601_def *)g_myinfo.recvbuf;
    r601_def *response = (r601_def *)g_fileinfo.reply_buf;
    c502_def *request_com;
    short len;
    char rcv_ipc_str[25];
    char rcv_ipc_hex[49];

    /* REPLYバッファ初期化 */
    memset ( response, ' ', sizeof( r601_def ));
    /* ローカル変数初期化 */
    memset(rcv_ipc_str,NULL,sizeof(rcv_ipc_str));
    memset(rcv_ipc_hex,NULL,sizeof(rcv_ipc_hex));
    /* IF_CODEをチェックする */
    /* 電文ログ出力要求 */
    if ( memcmp( request->common_header.interface_code, DEF_IPC_IFCD_LG_OUT_REQ_DEN_REQ,
                 sizeof( DEF_IPC_IFCD_LG_OUT_REQ_DEN_REQ ) -1 ) == 0) {

        /* 受信データ長チェック */
        if ( request->common_header.control_data_length != ( g_myinfo.iocomp.len - sizeof(common_header_def))) {
            /*------------------------------------------------*/
            /*    メッセージ出力：リクエストエラー            */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_REQ_ERR);
            // ③エラー内容 (20バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_RCV_LEN_ERR
                  ,sizeof(DEF_EMS_RCV_LEN_ERR)-1);
            // ④受信IPC内容 (48バイト)
            memcpy(rcv_ipc_str,g_myinfo.recvbuf,sizeof(rcv_ipc_str)-1);
            LOGS_strtohex(rcv_ipc_str,rcv_ipc_hex,sizeof(rcv_ipc_str)-1);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,rcv_ipc_hex
                  ,sizeof(rcv_ipc_hex)-1);
            GFPOGGZ1(&oggz1in);

            /*IPC内容精査エラー */
            memcpy(response->common_header.interface_code, DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
                   sizeof( DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ ) -1 );
            response->common_header.error_code = DEF_ERCD_OTHER;
            memcpy( response->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof( DEF_NERR_IPC_SEISA_ERR ) -1 );
            response->common_header.control_data_length = 0;
            g_fileinfo.reply_len = sizeof( common_header_def );
            return;
        }

        /* 登録更新区分チェック */
        switch ( request->entry_update_cate ) {
        case  DEF_ENTRY_CATE:
        case  DEF_UPDATE_CATE:
            break;
        default:
            /*------------------------------------------------*/
            /*    メッセージ出力：リクエストエラー            */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_REQ_ERR);
            // ③エラー内容 (20バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_CATEGORY_ERR
                  ,sizeof(DEF_EMS_CATEGORY_ERR)-1);
            // ④受信IPC内容 (48バイト)
            memcpy(rcv_ipc_str,g_myinfo.recvbuf,sizeof(rcv_ipc_str)-1);
            LOGS_strtohex(rcv_ipc_str,rcv_ipc_hex,sizeof(rcv_ipc_str)-1);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,rcv_ipc_hex
                  ,sizeof(rcv_ipc_hex)-1);
            GFPOGGZ1(&oggz1in);

            /*IPC内容精査エラー */
            memcpy(response->common_header.interface_code, DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
                   sizeof(DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ) -1 );
            response->common_header.error_code = DEF_ERCD_OTHER;
            memcpy( response->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof( DEF_NERR_IPC_SEISA_ERR ) -1 );
            response->common_header.control_data_length = 0;
            g_fileinfo.reply_len = sizeof( common_header_def );
            return;
        }

        /*電文ログ出力要求 */
        LOGS_req_log();

    /*コマンド要求 */
    } else if ( memcmp( request->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_REQ,
                        sizeof( DEF_IPC_IFCD_CMD_PRC_REQ ) -1 ) == 0) {

        /* 受信データ長チェック */
        if ( request->common_header.control_data_length != g_myinfo.iocomp.len - sizeof(common_header_def)) {/* */
            /*------------------------------------------------*/
            /*    メッセージ出力：リクエストエラー            */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_REQ_ERR);
            // ③エラー内容 (20バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_RCV_LEN_ERR
                  ,sizeof(DEF_EMS_RCV_LEN_ERR)-1);
            // ④受信IPC内容 (48バイト)
            memcpy(rcv_ipc_str,g_myinfo.recvbuf,sizeof(rcv_ipc_str)-1);
            LOGS_strtohex(rcv_ipc_str,rcv_ipc_hex,sizeof(rcv_ipc_str)-1);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,rcv_ipc_hex
                  ,sizeof(rcv_ipc_hex)-1);
            GFPOGGZ1(&oggz1in);


            /*IPC内容精査エラー */
            memcpy( response->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP,
                    sizeof( DEF_IPC_IFCD_CMD_PRC_RSP ) -1 );
            response->common_header.error_code = DEF_ERCD_OTHER;
            memcpy( response->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof( DEF_NERR_IPC_SEISA_ERR ) -1 );
            response->common_header.control_data_length = 0;
            g_fileinfo.reply_len = sizeof( common_header_def );
            return;
        }

        /* コマンド名チェック */
        request_com  = (c502_def *)g_myinfo.recvbuf;
        if ( memcmp( request_com->command_info.command_name, DEF_IF_COM_NAME, sizeof(DEF_IF_COM_NAME) -1 ) != 0 ) {
            /*------------------------------------------------*/
            /*    メッセージ出力：リクエストエラー            */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_REQ_ERR);
            // ③エラー内容 (20バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_COMMAND_NAME_ERR
                  ,sizeof(DEF_EMS_COMMAND_NAME_ERR)-1);
            // ④受信IPC内容 (48バイト)
            memcpy(rcv_ipc_str,g_myinfo.recvbuf,sizeof(rcv_ipc_str)-1);
            LOGS_strtohex(rcv_ipc_str,rcv_ipc_hex,sizeof(rcv_ipc_str)-1);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,rcv_ipc_hex
                  ,sizeof(rcv_ipc_hex)-1);
            GFPOGGZ1(&oggz1in);


            /*IPC内容精査エラー */
            memcpy( response->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP, sizeof( DEF_IPC_IFCD_CMD_PRC_RSP ) -1 );
            response->common_header.error_code = DEF_ERCD_OTHER;
            memcpy( response->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof( DEF_NERR_IPC_SEISA_ERR ) -1 );
            response->common_header.control_data_length = 0;
            g_fileinfo.reply_len = sizeof( common_header_def );
            return;
        }

        /*コマンド要求 */
        LOGS_req_cmd();

    } else {
        ///cccEMS
        /* エラーリプライ編集 */
        memcpy( response->common_header.interface_code, request->common_header.interface_code,
                sizeof( response->common_header.interface_code ));
        response->common_header.error_code = DEF_ERCD_OTHER;
        memcpy( response->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof( DEF_NERR_IPC_SEISA_ERR ) -1 );
        response->common_header.control_data_length = 0;
        g_fileinfo.reply_len = sizeof( common_header_def );
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.1  LOGS_req_log                                   */
/*  CALLING SEQ.    : void  LOGS_req_log(void)                              */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 電文ログ出力要求処理                                  */
/****************************************************************************/
void LOGS_req_log(void)
{
    short ret;
    c601_def *request = (c601_def *)g_myinfo.recvbuf;
    char yyyymmdd[8+1];
    char buffer[64];
    long l_tranid;
    char rcv_ipc_str[25];
    char rcv_ipc_hex[49];

    /*前日 */
    short s_yday[3];
    char yday[8+1];
    /*翌日 */
    short s_tday[3];
    char tday[8+1];

    /* ローカル変数初期化 */
    memset(rcv_ipc_str,NULL,sizeof(rcv_ipc_str));
    memset(rcv_ipc_hex,NULL,sizeof(rcv_ipc_hex));

    // タイムスタンプをチェックする為、カレント日付の前日年月日を取得
    LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_YES, (short *)&s_yday[0] );
    sprintf( yday,"%4d%02d%02d", s_yday[0], s_yday[1], s_yday[2] );

    // タイムスタンプをチェックする為、カレント日付の翌日年月日を取得
    LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_TOM, (short *)&s_tday[0] );
    sprintf( tday,"%4d%02d%02d", s_tday[0], s_tday[1], s_tday[2] );

    /************************/
    /* 出力先ファイルの判定 */
    /************************/
    // 電文ログ出力要求にログファイル指定がない場合
    if ( request->logfile_id[0] == ' ' ) {

        // 電文ログ出力要求からタイムスタンプを取得する
        memset( yyyymmdd, NULL, sizeof(yyyymmdd));
        memcpy( yyyymmdd, request->t_glnlg.pri_key.tushin_denbun_id.time_stamp, sizeof(yyyymmdd)-1 );

        /* 電文のタイムスタンプが、前日／当日／翌日かチェックする */
        if( memcmp( g_fileinfo.open_log_date, yyyymmdd, 8 ) == 0 ) {
            /* 当日日付 */
            g_fileinfo.file_idx = DEF_IDX_TOD;
        } else if ( memcmp( yday, yyyymmdd, 8 ) == 0 ){
            /* 前日日付 */
            g_fileinfo.file_idx = DEF_IDX_YES;
        } else if ( memcmp( tday, yyyymmdd, 8 ) == 0 ){
            /* 翌日日付 */
            g_fileinfo.file_idx = DEF_IDX_TOM;
        } else {
            /*それ以外 */
            /*------------------------------------------------*/
            /*    メッセージ出力：リクエストエラー            */
            /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_REQ_ERR);
            // ③エラー内容 (20バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,DEF_EMS_DATE_ERR
                  ,sizeof(DEF_EMS_DATE_ERR)-1);
            // ④受信IPC内容 (48バイト)
            memcpy(rcv_ipc_str,g_myinfo.recvbuf,sizeof(rcv_ipc_str)-1);
            LOGS_strtohex(rcv_ipc_str,rcv_ipc_hex,sizeof(rcv_ipc_str)-1);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,rcv_ipc_hex
                  ,sizeof(rcv_ipc_hex)-1);
            GFPOGGZ1(&oggz1in);

            LOGS_abend();
        }

    // 電文ログ出力要求にログファイル指定がある場合
    } else {
        /* ログ指定あり */
        /* 電文のログファイルが、OPEN済の前日／当日／翌日かチェックする */
        if( memcmp( g_fileinfo.log_table[DEF_IDX_TOD].log_fname, request->logfile_id,
                    strcspn(g_fileinfo.log_table[DEF_IDX_TOD].log_fname, " ") ) == 0 ) {
            /* 当日日付 */
            g_fileinfo.file_idx = DEF_IDX_TOD;
        } else if ( memcmp( g_fileinfo.log_table[DEF_IDX_YES].log_fname, request->logfile_id,
                    strcspn(g_fileinfo.log_table[DEF_IDX_YES].log_fname, " ") ) == 0 ) {
            /* 前日日付 */
            g_fileinfo.file_idx = DEF_IDX_YES;
        } else if ( memcmp( g_fileinfo.log_table[DEF_IDX_TOM].log_fname, request->logfile_id,
                    strcspn(g_fileinfo.log_table[DEF_IDX_TOM].log_fname, " ") ) == 0 ) {
            /* 翌日日付 */
            g_fileinfo.file_idx = DEF_IDX_TOM;
        } else {
            /*それ以外 */
            g_fileinfo.file_idx = DEF_IDX_SET;
            memcpy( g_fileinfo.log_table[DEF_IDX_SET].log_fname,
                    request->logfile_id, sizeof( request->logfile_id ));
            /* 指定されたログファイルをオープンする */
            LOGS_open_log( g_fileinfo.log_table[DEF_IDX_SET].log_fname,
                                 &g_fileinfo.log_table[DEF_IDX_SET].file_no );
        }
    }

    /*トランザクション開始 */
    ret = COM_TMF( DEF_COM_TMF_BEGIN, &l_tranid, DEF_MY_PROGID );
    if ( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXA0, sizeof(DEF_GFPCGXA0)-1);
        // ④エラーコード(4バイト)
        sprintf(buffer, "%04d", ret);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
        GFPOGGZ1(&oggz1in);

        LOGS_abend();
    }

    /* 電文ログ出力要求の登録更新区分に従い処理を呼び出す */
    if ( request->entry_update_cate == DEF_ENTRY_CATE ) {
        /* 登録 */
        ret = LOGS_write_log();
    } else {
        /* 更新 */
        ret = LOGS_update_log();
    }

    if ( ret == DEF_RET_OK ) {
        /* トランザクション終了 */
        ret = COM_TMF( DEF_COM_TMF_END, &l_tranid, DEF_MY_PROGID );
        if ( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // ③モジュールID(8バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_GFPCGXA0, sizeof(DEF_GFPCGXA0)-1);
            // ④エラーコード(4バイト)
            sprintf(buffer, "%04d", ret);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&oggz1in);

            LOGS_abend();
        }
    } else {
        /* レコード登録・更新エラー発生時 */
        ret = COM_TMF( DEF_COM_TMF_ABORT, &l_tranid, DEF_MY_PROGID );
        if (ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
            // 任意メッセージ部初期化
            LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
            // ③モジュールID(8バイト)
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
                   DEF_GFPCGXA0, sizeof(DEF_GFPCGXA0)-1);
            // ④エラーコード(4バイト)
            sprintf(buffer, "%04d", ret);
            memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, strlen(buffer));
            GFPOGGZ1(&oggz1in);

            LOGS_abend();
        }
    }

    /********************************************/
    /* 出力先ファイルによって以下の後処理を行う */
    /********************************************/
    /* 指定ログファイル */
    if ( g_fileinfo.file_idx == DEF_IDX_SET ) {
        /* 指定ログファイルをクローズする */
        LOGS_close_log( g_fileinfo.log_table[DEF_IDX_SET].log_fname,
                        g_fileinfo.log_table[DEF_IDX_SET].file_no );
    /* 翌日ログファイルでログファイル指定でない時 */
    } else if (( g_fileinfo.file_idx == DEF_IDX_TOM ) && ( request->logfile_id[0] == ' ' )) {
        /* 日替わり処理 */
        /* カレント日付を翌日に設定 */
        memcpy(g_fileinfo.open_log_date,yyyymmdd,8);
        /* 前日用ファイルをクローズする */
        LOGS_close_log( g_fileinfo.log_table[DEF_IDX_YES].log_fname,
                        g_fileinfo.log_table[DEF_IDX_YES].file_no );

        // 前日ファイル情報 <- 当日ファイル情報
        memcpy( (char *)&g_fileinfo.log_table[DEF_IDX_YES].log_fname,
                (char *)&g_fileinfo.log_table[DEF_IDX_TOD].log_fname,
                sizeof( g_fileinfo.log_table[DEF_IDX_YES].log_fname ));
        g_fileinfo.log_table[DEF_IDX_YES].file_no = g_fileinfo.log_table[DEF_IDX_TOD].file_no;
        // 当日ファイル情報 <- 翌日ファイル情報
        memcpy( (char *)&g_fileinfo.log_table[DEF_IDX_TOD].log_fname,
                (char *)&g_fileinfo.log_table[DEF_IDX_TOM].log_fname,
                sizeof( g_fileinfo.log_table[DEF_IDX_TOD].log_fname ));
        g_fileinfo.log_table[DEF_IDX_TOD].file_no = g_fileinfo.log_table[DEF_IDX_TOM].file_no;
        // 翌日ファイル情報 <- 内部テーブル
        memcpy( g_fileinfo.log_table[DEF_IDX_TOM].log_fname,
                g_name_tbl[s_tday[2]].log_fname, DEF_FILENAME_LEN );

        /* 翌日用ファイルをオープンする */
        LOGS_open_log( g_fileinfo.log_table[DEF_IDX_TOM].log_fname,
                             &g_fileinfo.log_table[DEF_IDX_TOM].file_no   );

        /*------------------------------------------------*/
        /*    メッセージ出力：ログファイル切替            */
        /*------------------------------------------------*/
        // メッセージ通知区分 正常時は*
        oggz1in.emsinf.emsgkinf.msgttkb = '*';
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_LOG_FILE_CHANGE);
        // ③切替元ファイル名(当日)(48バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               g_fileinfo.log_table[DEF_IDX_YES].log_fname, sizeof(g_fileinfo.log_table[DEF_IDX_YES].log_fname));
        // ④切替先ファイル名(当日)(48バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,
               g_fileinfo.log_table[DEF_IDX_TOD].log_fname, sizeof(g_fileinfo.log_table[DEF_IDX_TOD].log_fname));
        GFPOGGZ1(&oggz1in);

        // メッセージ通知区分 システムエラーを設定しておく
        oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.3.2  LOGS_req_cmd                                   */
/*  CALLING SEQ.    : void  LOGS_req_cmd(void)                              */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンド要求処理                                      */
/****************************************************************************/
void LOGS_req_cmd(void)
{
    r502_def *response = (r502_def *)g_fileinfo.reply_buf;
    long long ll_timestamp;
    char yyyymmdd[8+1];
    short wkdate[8], i, ret;
    /*前日 */
    short yday[3];
    /*当日 */
    short oday[3];
    /*翌日 */
    short tday[3];

    /*システム日付取得 */
    ll_timestamp = JULIANTIMESTAMP();
    ll_timestamp = CONVERTTIMESTAMP( ll_timestamp );
    INTERPRETTIMESTAMP( ll_timestamp, wkdate );
    sprintf( yyyymmdd,"%4d%02d%02d", wkdate[0], wkdate[1], wkdate[2] );

    /* 取得したシステム日付がカレント日付と同じ場合 */
    if ( memcmp( yyyymmdd, g_fileinfo.open_log_date, 8 ) == 0 ) {
        // nop
    } else {
    /* 取得したシステム日付がカレント日付と異なる場合 */
        /*前日、当日、翌日ログクローズ */
        for ( i = 0; i < 3; i++ ) {
            LOGS_close_log( g_fileinfo.log_table[i].log_fname,
                            g_fileinfo.log_table[i].file_no );
        }
        /* システム日付をカレント日付に設定 */
        memcpy(g_fileinfo.open_log_date, yyyymmdd, 8 );

        /* 前日日付INDEX取得 */
        LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_YES, (short *)&yday[0] );

        /*当日日付INDEX取得 */
        oday[2] = wkdate[2];

        /*翌日日付INDEX取得 */
        LOGS_get_day( g_fileinfo.open_log_date, DEF_OFFSET_TOM, (short *)&tday[0] );

        memcpy( g_fileinfo.log_table[DEF_IDX_YES].log_fname,
                g_name_tbl[yday[2]-1].log_fname, DEF_FILENAME_LEN );
        memcpy( g_fileinfo.log_table[DEF_IDX_TOD].log_fname,
                g_name_tbl[oday[2]-1].log_fname, DEF_FILENAME_LEN );
        memcpy( g_fileinfo.log_table[DEF_IDX_TOM].log_fname,
                g_name_tbl[tday[2]-1].log_fname, DEF_FILENAME_LEN );
    
        /* 前日、当日、翌日ログオープン */
        for (i = 0; i < 3; i++ ) {
            LOGS_open_log( g_fileinfo.log_table[i].log_fname,
                                 &g_fileinfo.log_table[i].file_no   );
        }
        /*------------------------------------------------*/
        /*    メッセージ出力：ログファイル切替            */
        /*------------------------------------------------*/
        // メッセージ通知区分 正常時は*
        oggz1in.emsinf.emsgkinf.msgttkb = '*';
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_LOG_FILE_CHANGE);
        // ③切替元ファイル名(当日)(48バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               g_fileinfo.log_table[DEF_IDX_YES].log_fname, sizeof(g_fileinfo.log_table[DEF_IDX_YES].log_fname));
        // ④切替先ファイル名(当日)(48バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl,
               g_fileinfo.log_table[DEF_IDX_TOD].log_fname, sizeof(g_fileinfo.log_table[DEF_IDX_TOD].log_fname));
        GFPOGGZ1(&oggz1in);

        // メッセージ通知区分 システムエラーを設定しておく
        oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    }

    /*リプライバッファ設定 */
    memset( response, ' ', sizeof( common_header_def ));
    memcpy( response->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP,
            sizeof( DEF_IPC_IFCD_CMD_PRC_RSP ) -1 );
    response->common_header.error_code = DEF_ERCD_NOM;
    memcpy( response->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof( DEF_NERR_NOMAL ) -1 );
    response->common_header.control_data_length = 0;
    g_fileinfo.reply_len = sizeof( common_header_def );
}

/****************************************************************************/
/*  FUNCTION        : 0.1.0  LOGS_get_day                                   */
/*  CALLING SEQ.    : void  LOGS_get_day(void)                              */
/*  ARGUMENT        : 1. baseday     (I) 基準日                             */
/*  ARGUMENT        : 2. オフセット  (I)                                    */
/*  ARGUMENT        : 3. 出力日      (O)                                    */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 日付取得処理                                          */
/*                  : 基準年月日(char)を渡してOFFSETの年月日(short)を返す   */
/****************************************************************************/
void LOGS_get_day( char *baseday, short offset, short day[3] )
{
    short s_year, s_month, s_day;
    long  l_day_no;
    char  wkyear[5], wkmonth[3], wkday[3];

    memset( wkyear,  NULL, sizeof( wkyear  ));
    memset( wkmonth, NULL, sizeof( wkmonth ));
    memset( wkday,   NULL, sizeof( wkday   ));

    /* 基準日を分割 */
    memcpy( wkyear,  baseday,   4 );
    memcpy( wkmonth, baseday+4, 2 );
    memcpy( wkday,   baseday+6, 2 );

    s_year  = ( short )atoi( wkyear  );
    s_month = ( short )atoi( wkmonth );
    s_day   = ( short )atoi( wkday   );

    /*通算日に変換 */
    l_day_no = COMPUTEJULIANDAYNO( s_year, s_month, s_day );
    if( l_day_no == -1 ) {
        /*引数が不正だった場合、"-1"が返る */
        /*------------------------------------------------*/
        /*    メッセージ出力：プロシジャーコールエラー    */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PROCEDURE_ERR);
        // ③プロシジャー名(40バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               "COMPUTEJULIANDAYNO", sizeof("COMPUTEJULIANDAYNO")-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, "-1", sizeof("-1")-1);
        GFPOGGZ1(&oggz1in);
        /* 異常終了処理  */
        LOGS_abend();
    }
    l_day_no += offset;

    /* 通算日+OFFSETの年月日に変換 */
    INTERPRETJULIANDAYNO( l_day_no, (short *)&day[0],
                                    (short *)&day[1],
                                    (short *)&day[2] );

    if( day[0] == -1 ) {
        /*引数が不正だった場合、年に"-1"が返る */
        /*------------------------------------------------*/
        /*    メッセージ出力：プロシジャーコールエラー    */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_PROCEDURE_ERR);
        // ③プロシジャー名(40バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               "INTERPRETJULIANDAYNO", sizeof("INTERPRETJULIANDAYNO")-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, "-1", sizeof("-1")-1);
        GFPOGGZ1(&oggz1in);
        /* 異常終了処理  */
        LOGS_abend();
    }
}

/****************************************************************************/
/*  FUNCTION        : 0.2.0  LOGS_open_log                                  */
/*  CALLING SEQ.    : void  LOGS_open_log(void)                             */
/*  ARGUMENT        : 1. ファイル名     (I)                                 */
/*  ARGUMENT        : 2. ファイル番号   (O)                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : NW通信ログオープン処理                                */
/****************************************************************************/
void LOGS_open_log( char *filename, short *filenum )
{
    short ret;
    char  sub_prog_sts[2];

    /* IOモジュールパラメータ設定 */
    memset( &iom_trc, NULL, sizeof( iom_trc ));
    memset( &iom_fil, NULL, sizeof( iom_fil ));
    memset( &iom_inp, NULL, sizeof( iom_inp ));
    memset( &iom_out, NULL, sizeof( iom_out ));

    /* ファイル情報設定 */
    memcpy( iom_fil.file_id, DEF_FL_NW_LOG, sizeof(DEF_FL_NW_LOG) -1 );
    memcpy( iom_fil.file_name, filename, DEF_FILENAME_LEN );
    iom_fil.file_no = 0;

    /* NW通信ログファイルオープン */
    ret = COM_IOM( DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
    if( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&oggz1in);

        /* 異常終了処理  */
        LOGS_abend();
    }
    *filenum = iom_fil.file_no;
}

/****************************************************************************/
/*  FUNCTION        : 0.3.0  LOGS_close_log                                 */
/*  CALLING SEQ.    : void  LOGS_close_log(void)                            */
/*  ARGUMENT        : 1. ファイル名     (I)                                 */
/*  ARGUMENT        : 2. ファイル番号   (I)                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : NW通信ログクローズ処理                                */
/****************************************************************************/
void LOGS_close_log( char *filename, short filenum )
{
    short ret;
    char  sub_prog_sts[2];

    /* IOモジュールパラメータ設定 */
    memcpy( iom_trc.file_io_type, DEF_FLIOTYPE_CLOSE, sizeof( DEF_FLIOTYPE_CLOSE ) -1 );
    memset( &iom_fil, NULL, sizeof( iom_fil ));
    memset( &iom_inp, NULL, sizeof( iom_inp ));
    memset( &iom_out, NULL, sizeof( iom_out ));

    /* ファイル情報設定 */
    memcpy( iom_fil.file_id, DEF_FL_NW_LOG, sizeof( DEF_FL_NW_LOG ) -1 );
    memcpy( iom_fil.file_name, &filename, DEF_FILENAME_LEN);
    iom_fil.file_no = filenum;

    /* NW通信ログファイルクローズ */
    COM_IOM( DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
}

/****************************************************************************/
/*  FUNCTION        : 0.4.0  LOGS_getname_log                               */
/*  CALLING SEQ.    : void  LOGS_getname_log(void)                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : NW通信ログ物理名メモリ展開処理                        */
/****************************************************************************/
void LOGS_getname_log()
{
    char  fname[ZSYS_VAL_LEN_FILENAME+1];
    char  trc_fname[ZSYS_VAL_LEN_FILENAME+1];
    char  sub_prog_sts[2];
    char  asn_name[32];
    short ret, wkerr, i, fnm_len;
    db_gfphi_def    t_gfphi_pk;            /* 物理名情報ファイルPK       */
    db_gfphi_def   *g_gfphi_rec;           /* 物理名情報ファイルレコード */

    /* ASSIGN情報取得モジュールにて物理名情報ファイル名を取得 */
    memset(asn_name, ' ', sizeof(asn_name));
    memcpy(asn_name, DEF_ASN_GFPHI, sizeof(DEF_ASN_GFPHI)-1);
    memset( fname, NULL, sizeof( fname ));
    COM_ASN( asn_name, fname, &fnm_len );

    /**************************** */
    /* IOモジュールパラメータ設定 */
    /**************************** */
    /* トレース情報の設定 */
    memset( &iom_trc, NULL, sizeof( iom_trc ));
    memcpy( iom_trc.prog_id, DEF_MY_PROGID, sizeof( DEF_MY_PROGID )-1 );
    memcpy( iom_trc.file_id,  DEF_FL_TRACE, sizeof( DEF_FL_TRACE ) -1 );
    memcpy( iom_trc.file_name, trc_fname, strlen( trc_fname ));
    memcpy( iom_trc.file_io_type, DEF_FLIOTYPE_OPEN, sizeof( DEF_FLIOTYPE_OPEN ) -1 );

    /* ファイル情報設定 */
    memset( &iom_fil, NULL, sizeof( iom_fil ));
    memcpy( iom_fil.file_id, DEF_FL_PHSIC_INFO, sizeof( DEF_FL_PHSIC_INFO ) -1 );
    memcpy( iom_fil.file_name, fname, strlen( fname ));

    /*物理名情報ファイルオープン */
    ret = COM_IOM( DEF_COM_IOM_FUNC_OPEN, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out);
    if( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ出力：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&oggz1in);

        // 異常終了処理
        LOGS_abend();
    }

    /* Key設定 */
    memset( &t_gfphi_pk, NULL, sizeof( t_gfphi_pk ));
    /* PKEY サイト識別からグループ識別設定 */
    memcpy( &t_gfphi_pk.pri_key.site_id, g_myinfo.site_id,  sizeof(t_gfphi_pk.pri_key.site_id));
    memcpy( &t_gfphi_pk.pri_key.nw_id, g_myinfo.network_id, sizeof(t_gfphi_pk.pri_key.nw_id));
    memcpy( t_gfphi_pk.pri_key.grp_id, g_myinfo.group_id,   sizeof(t_gfphi_pk.pri_key.grp_id));

    /* PKEY サーバクラス論理KEY設定 */
    memset( (char *)&t_gfphi_pk.pri_key.srv_cls_key, '}', sizeof( t_gfphi_pk.pri_key.srv_cls_key ));

    /* PKEY プロセス／ファイル論理KEY設定 */
    memcpy( t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_NW_LOG,
            sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_kind));

    memset( t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, '0',
            sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy( t_gfphi_pk.pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_GFPHI_DT,
            sizeof(DEF_GFPHI_DT)-1);

    memset( t_gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num, '0',
            sizeof(t_gfphi_pk.pri_key.prc_file_key.prc_file_mlt_num));

    /* トレース情報の設定 */
    memcpy( iom_trc.file_io_type, DEF_FLIOTYPE_READ, sizeof( DEF_FLIOTYPE_READ ) -1 );

    /* 入力情報設定 */
    memset( &iom_inp, NULL, sizeof(iom_inp));
    iom_inp.part_key_type = 0;
    memcpy( iom_inp.key_value, &t_gfphi_pk, sizeof( t_gfphi_pk ));
    memcpy( iom_inp.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof( DEF_COM_IOM_KEYTYPE_PRI ) -1 );
    iom_inp.key_len = sizeof(t_gfphi_pk.pri_key);
    iom_inp.compare_len = DEF_GFPHI_COMP_KEYLEN;  //33byte "DT"まで一致するレコード読み込み
    iom_inp.positioning_mode = DEF_COM_IOM_GENERIC;
    iom_inp.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_inp.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_inp.io_timer = g_myinfo.fio_timer;
    iom_inp.rec_len = db_gfphi_def_Size;

    i = 0;
    /* READ開始 */
    ret = COM_IOM( DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
    if(( memcmp( sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(DEF_COM_IOM_NO_ERR)-1 ) != 0 )){
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&oggz1in);
        LOGS_abend();
    }

    while(( memcmp( sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(DEF_COM_IOM_NO_ERR)-1 ) == 0 ) && ( i < 31 )) {
        g_gfphi_rec = (db_gfphi_def *)iom_out.rec_area;
        /* ファイル名をテーブルに設定 */
        memcpy( g_name_tbl[i].log_fname, g_gfphi_rec->prc_file_info.prc_file_name, DEF_FILENAME_LEN);
        /* NEXTREAD */
        ret = COM_IOM( DEF_COM_IOM_FUNC_NEXTREAD, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
        i++;
    }
    if( i != 31 ){
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&oggz1in);
        LOGS_abend();
    }

    /* トレース情報の設定 */
    memcpy( iom_trc.file_io_type, DEF_FLIOTYPE_CLOSE, sizeof( DEF_FLIOTYPE_CLOSE ) -1 );

    /* 物理名情報ファイルクローズ */
    COM_IOM( DEF_COM_IOM_FUNC_CLOSE, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );

}

/****************************************************************************/
/*  FUNCTION        : 2.3.1.1  LOGS_write_log                               */
/*  CALLING SEQ.    : void  LOGS_write_log(void)                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : NW通信ログ登録処理                                    */
/****************************************************************************/
short LOGS_write_log( void )
{
    c601_def *request = (c601_def *)g_myinfo.recvbuf;
    char sub_prog_sts[2];
    char buffer[64];
    long l_tranid;
    short ret,rec_len;

    /*ファイル情報設定 */
    memset( &iom_fil, NULL, sizeof( iom_fil ));
    memcpy( iom_fil.file_id, DEF_FL_NW_LOG, sizeof( DEF_FL_NW_LOG ) -1 );
    memcpy( iom_fil.file_name, g_fileinfo.log_table[g_fileinfo.file_idx].log_fname, DEF_FILENAME_LEN);
    iom_fil.file_no = g_fileinfo.log_table[g_fileinfo.file_idx].file_no;

    /*入力情報設定 */
    memset( &iom_inp, NULL, sizeof( iom_inp ));
    // NW通信ログファイルへ書き込む実データ長 = 共通ヘッダ.データ長 - ログファイル名 - 登録更新区分
    rec_len = request->common_header.control_data_length -
              sizeof(request->logfile_id)                -
              sizeof(request->entry_update_cate);
    iom_inp.part_key_type = 0;
    memcpy( iom_inp.rec_area, (char *)&request->t_glnlg, rec_len );
    iom_inp.io_timer = g_myinfo.fio_timer;
    iom_inp.rec_len  = rec_len;

    /* レコード登録 */
    ret = COM_IOM( DEF_COM_IOM_FUNC_ADD, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
    if ( ret == DEF_RET_OK ) {
        /* REPLY編集 */
        LOGS_edit_reply( DEF_ERCD_NOM, DEF_NERR_NOMAL, NULL, NULL ); 
        return DEF_RET_OK;
    }

    /*------------------------------------------------*/
    /*    メッセージ出力：ファイルI/Oエラー           */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LOGS_setmsgid(DEF_EVT_FILE_IO_ERR);
    // ⑤ファイル論理的名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
           DEF_FL_NW_LOG, sizeof(DEF_FL_NW_LOG)-1);
    // ⑥アクション (19バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "WRITE", sizeof("WRITE")-1);
    // ⑧エラーコード (5バイト)
    sprintf(buffer, "%05d", iom_out.guardian_errcode);
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
    GFPOGGZ1(&oggz1in);

    /* エラーに対応するREPLY編集 */
    switch ( iom_out.guardian_errcode ) {
    case  DEF_RET_PKEY:
        /*プライマリーキー重複 */
        LOGS_edit_reply( DEF_ERCD_LCN, DEF_NERR_LOGS_DUPLICATE_PKEY,
                         request->t_glnlg.denbun_send_recv_info.denbun_log_key.tran_id.gfp_lcn_id,
                         (char *)&request->t_glnlg.furiwake_info ); 
        break;
    case  DEF_RET_AKEY:
        /*ALTキー重複 */
        LOGS_edit_reply( DEF_ERCD_LCN, DEF_NERR_LOGS_DUPLICATE_A1KEY, NULL, NULL ); 
        break;
    case  DEF_RET_FFUL:
        /*ファイルFULL */
        LOGS_edit_reply( DEF_ERCD_OTHER, DEF_NERR_LOGS_LOG_FULL, NULL, NULL ); 
        break;
    default:
        /*その他エラー */
        LOGS_edit_reply( DEF_ERCD_OTHER, DEF_NERR_LOGS_OTHER_ERR, NULL, NULL );
        break;
    }
    return DEF_RET_NG;
}

/****************************************************************************/
/*  FUNCTION        : 2.3.1.2  LOGS_update_log                              */
/*  CALLING SEQ.    : void  LOGS_update_log(void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : NW通信ログ更新処理                                    */
/****************************************************************************/
short LOGS_update_log(void)
{
    c601_def *request = (c601_def *)g_myinfo.recvbuf;
    char sub_prog_sts[2];
    char buffer[64];
    short ret,rec_len;

    /*ファイル情報設定 */
    memset( &iom_fil, NULL, sizeof( iom_fil ));
    memcpy( iom_fil.file_id, DEF_FL_NW_LOG, sizeof( DEF_FL_NW_LOG ) -1 );
    memcpy( iom_fil.file_name, g_fileinfo.log_table[g_fileinfo.file_idx].log_fname, DEF_FILENAME_LEN );
    iom_fil.file_no = g_fileinfo.log_table[g_fileinfo.file_idx].file_no;

    /*入力情報設定 */
    memset( &iom_inp, NULL, sizeof( iom_inp ));
    iom_inp.part_key_type = 0;
    iom_inp.part_key_position = 0;
    iom_inp.part_key_len = 0;
    memcpy( iom_inp.key_value, (char *)&request->t_glnlg.pri_key, sizeof(request->t_glnlg.pri_key));
    memcpy( iom_inp.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof( DEF_COM_IOM_KEYTYPE_PRI )-1);
    iom_inp.key_len = sizeof(request->t_glnlg.pri_key);
    iom_inp.compare_len = sizeof(request->t_glnlg.pri_key);
    iom_inp.positioning_mode = DEF_COM_IOM_EXACT;
    iom_inp.lock_flg = DEF_COM_IOM_LOCK;
    iom_inp.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_inp.rec_len = sizeof(db_glnlg_def);
    iom_inp.io_timer = g_myinfo.fio_timer;
  
    /*レコード読み込み(LOCK) */
    ret = COM_IOM( DEF_COM_IOM_FUNC_STARTREAD, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
    if ( ret != DEF_RET_OK ) {
        /*------------------------------------------------*/
        /*    メッセージ編集：共通モジュールエラー        */
        /*------------------------------------------------*/
        // 任意メッセージ部初期化
        LOGS_setmsgid(DEF_EVT_COMMON_MOD_ERR);
        // ③モジュールID(8バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl,
               DEF_GFPCGXB0, sizeof(DEF_GFPCGXB0)-1);
        // ④エラーコード(4バイト)
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts,
               sizeof(sub_prog_sts));
        GFPOGGZ1(&oggz1in);

        /*その他エラー */
        LOGS_edit_reply( DEF_ERCD_OTHER, DEF_NERR_LOGS_OTHER_ERR, NULL, NULL);
    }

    /*入力情報設定 */
    memset( &iom_inp, NULL, sizeof( iom_inp ));
    // NW通信ログファイルへ書き込む実データ長 = 共通ヘッダ.データ長 - ログファイル名 - 登録更新区分
    rec_len = request->common_header.control_data_length -
              sizeof(request->logfile_id)                -
              sizeof(request->entry_update_cate);
    iom_inp.part_key_type = 0;
    memcpy( iom_inp.rec_area, (char *)&request->t_glnlg, rec_len);
    iom_inp.rec_len = rec_len;
    iom_inp.lock_flg = DEF_COM_IOM_LOCK;
    iom_inp.io_timer = g_myinfo.fio_timer;
  
    /*レコード更新 */
    ret = COM_IOM( DEF_COM_IOM_FUNC_UPDATE, sub_prog_sts, &iom_trc, &iom_fil, &iom_inp, &iom_out );
    if ( ret == DEF_RET_OK ) {
        /* REPLY編集 */
        LOGS_edit_reply( DEF_ERCD_NOM, DEF_NERR_NOMAL, NULL, NULL); 
        return DEF_RET_OK;
    }

    /* レコード更新エラー発生時 */
    /*------------------------------------------------*/
    /*    メッセージ出力：ファイルI/Oエラー           */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LOGS_setmsgid(DEF_EVT_FILE_IO_ERR);
    // ⑤ファイル論理的名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl,
           DEF_FL_NW_LOG, sizeof(DEF_FL_NW_LOG)-1);
    // ⑥アクション (19バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "WRITE", sizeof("WRITE")-1);
    // ⑧エラーコード (5バイト)
    sprintf(buffer, "%05d", iom_out.guardian_errcode);
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, strlen(buffer));
    GFPOGGZ1(&oggz1in);

    /* エラーに対応するREPLY編集 */
    switch ( iom_out.guardian_errcode ) {
    case  DEF_RET_UPDATE:
        /*更新対象未存在 */
        LOGS_edit_reply( DEF_ERCD_OTHER, DEF_NERR_LOGS_NO_UPDATES, NULL, NULL); 
        break;
    default:
        /*その他エラー */
        LOGS_edit_reply( DEF_ERCD_OTHER, DEF_NERR_LOGS_OTHER_ERR, NULL, NULL);
        break;
    }
    return DEF_RET_NG;
}

/****************************************************************************/
/*  FUNCTION        : 3.0.0  LOGS_edit_reply                                 */
/*  CALLING SEQ.    : void  LOGS_edit_reply(void)                            */
/*  ARGUMENT        : 1. エラーコード     (I)                               */
/*  ARGUMENT        : 2. 内部エラーコード (I)                               */
/*  ARGUMENT        : 3. GFP内部LCN       (I)                               */
/*  ARGUMENT        : 4. 東阪振分情報     (I)                               */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : REPLY編集処理                                         */
/****************************************************************************/
void LOGS_edit_reply( short errcode, char* in_errcode, char* lcn, char* frwk_kbn)
{
    r601_def *response = (r601_def *)g_fileinfo.reply_buf;

    /*リプライバッファ設定 */
    memset ( response, ' ', sizeof( r601_def ));
    memcpy( response->common_header.interface_code, DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ,
            sizeof( DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ ) -1 );
    response->common_header.error_code = errcode;
    memcpy( response->common_header.internal_error_code, in_errcode,
            sizeof( response->common_header.internal_error_code ));
    response->common_header.control_data_length = sizeof( r601_def ) - sizeof( common_header_def );

    memcpy( response->logfile_id, g_fileinfo.log_table[g_fileinfo.file_idx].log_fname,
            DEF_FILENAME_LEN );
    // P-KEY重複時は、GFP内部LCNと東阪振分情報を返す
    if ( errcode == DEF_ERCD_LCN ) {
        memcpy( response->lcn, lcn, sizeof( response->lcn ));
        memcpy( response->thnkbn, frwk_kbn, sizeof( response->thnkbn ));
    }

    g_fileinfo.reply_len = sizeof( r601_def );
}

/****************************************************************************/
/*  FUNCTION        : 2.4.0  LOGS_recv_reply                                */
/*  CALLING SEQ.    : void  LOGS_recv_reply(void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リプライ処理                                          */
/****************************************************************************/
void LOGS_recv_reply(void)
{

    REPLYX( g_fileinfo.reply_buf
           ,g_fileinfo.reply_len
           ,
           ,
           ,g_fileinfo.reply_code );
    /* エラーハンドリングしない */
}

/****************************************************************************/
/*  FUNCTION        : 4.0.0  LOGS_final                                     */
/*  CALLING SEQ.    : void  LOGS_final(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void LOGS_final(void)
{
    short i;
    lk_zac2001r_arg_1_def trace_if;

    /* OPEN中のNW通信ログファイルをCLOSEする */
    for(i = 0; i < 3; i++ ) {
        LOGS_close_log( g_fileinfo.log_table[i].log_fname, g_fileinfo.log_table[i].file_no );
    }
    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス正常終了            */
    /*------------------------------------------------*/
    // メッセージ通知区分 正常時は*
    oggz1in.emsinf.emsgkinf.msgttkb = '*';
    // 任意メッセージ部初期化
    LOGS_setmsgid(DEF_EVT_PROC_NORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&oggz1in);

    // トレースモジュール終了
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    trace_if.func_flg = DEF_TRACE_FUNC_END;
    TRACEOUT((char *)&trace_if);

}

/****************************************************************************/
/*  FUNCTION        : 5.0.0  LOGS_abend                                     */
/*  CALLING SEQ.    : void  LOGS_abend(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void LOGS_abend(void)
{
    lk_zac2001r_arg_1_def trace_if;

    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス異常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    LOGS_setmsgid(DEF_EVT_PROC_ABNORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, g_myinfo.procinfo.my_pname,
           strlen(g_myinfo.procinfo.my_pname));
    GFPOGGZ1(&oggz1in);

    // トレースモジュール終了
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID)-1);
    trace_if.func_flg = DEF_TRACE_FUNC_END;
    TRACEOUT((char *)&trace_if);

    /* プロセス終了 */
    PROCESS_STOP_( , , DEF_PROC_ABNORMAL_END );
}
/****************************************************************************/
/*  FUNCTION        : 9.0.0  LOGS_setmsgid                                  */
/*  CALLING SEQ.    : void  LOGS_setmsgid(short msgid)                      */
/*  ARGUMENT        : 1. msgid         (I) メッセージID                     */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : EMS出力モジュールI/F 任意メッセージ部初期化           */
/****************************************************************************/
void LOGS_setmsgid(short msgid)
{
    int i;
    int item_size;
    int item_cnt;
    char buffer[64];

    // GFP通信制御メッセージ番号
    sprintf(buffer, "%05d", msgid);
    memcpy(&oggz1in.emsinf.msgid, buffer, strlen(buffer));
    // ②サーバークラス論理ID (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[0].msgtbl_vl,
           DEF_MY_SCID, sizeof(DEF_MY_SCID)-1);

    item_size = sizeof(oggz1in.emsinf.emsnninf.msgtbl[0]);
    item_cnt =  sizeof(oggz1in.emsinf.emsnninf.msgtbl) / item_size;
    for (i = 1; i < item_cnt; i++) {
        memset(&oggz1in.emsinf.emsnninf.msgtbl[i], ' ', item_size);
    }
}

/****************************************************************************/
/*  FUNCTION        : 9.0.0  LOGS_strtohex                                  */
/*  CALLING SEQ.    : void LOGS_strtohex(char* input,char* output,short len)*/
/*  ARGUMENT        : 1. input  (I)  文字列                                 */
/*                  : 2. output (O)  hex                                    */
/*                  : 3. len    (I)  処理長                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 文字列のhex変換モジュール                             */
/****************************************************************************/
void LOGS_strtohex(const char* input,char* output,short len)
{
    int i;
    //char to hexを文字数分繰り返す
    for (i=0;i<len;i++){
        sprintf(output + (i * 2),"%02X",input[i]);
    }
    output[i * 2 ] = NULL;
}
