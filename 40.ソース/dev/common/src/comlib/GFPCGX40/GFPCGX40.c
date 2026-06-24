/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX40                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               PATHSEND処理モジュール                      */
/*                                                                           */
/*                               本プログラムは呼び出し元から受け取った情報を*/
/*                               元に指定されたPATHWAY、サーバクラスへの     */
/*                               PATHSENDを行う。                            */
/*                               PATHSENDエラー発生時は、指定された          */
/*                               リトライ回数までのリトライを行う。          */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････ EMS出力モジュール                           */
/*                               トレースモジュール                          */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
#pragma ENV COMMON

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist

/* USER HEADER     */
#include "ems.h" 
#include "errcd.h"
#include "GFPCGX40.h"
#include "GFPCGX50.h"

/* vproc関数の宣言 */
#include "vproc.h"

/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/
short COM_PSD_Pathsend_Exec(char *,short,char *,short,char *,
                            short,short,short *,long,short *,short *,char *
                            , oggz1in_def *, COM_PSD_arg_4_def *);

void COM_PSD_EMS(char *,char *,short,char *,short,short *,short *,
                 oggz1in_def *,COM_PSD_arg_4_def *);

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_PSD                                        */
/*  CALLING SEQ.    : short COM_PSD(COM_PSD_arg_1_def *,COM_PSD_arg_2_def * */
/*                                 ,COM_PSD_arg_3_def *,oggz1in_def *       */
/*                                 ,COM_PSD_arg_4_def *)                    */
/*  ARGUMENT        : 1.req         (I/O) PATHSEND要求情報                  */
/*                  : 2.trc         (I)   トレース情報                      */
/*                  : 3.res         (O)   PATHSEND結果情報                  */
/*                  : 4.ems_cmn     (I)   EMS出力共通情報                   */
/*                  : 5.ems_add     (I)   EMS出力出力追加情報               */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : PATHSEND処理のコントロールを行う                      */
/****************************************************************************/
short COM_PSD(COM_PSD_arg_1_def *req
            , COM_PSD_arg_2_def *trc
            , COM_PSD_arg_3_def *res
            , oggz1in_def       *ems_cmn
            , COM_PSD_arg_4_def *ems_add)
{
    short loop_flag;
    short retry_count;
    short err = 0;
    char *ptr;
    char pathmon_name[16+1];
    char serverclass_name[32+1];
    short pathmon_name_len = 0;
    short serverclass_name_len = 0;
    char save_buff[15000];

    /* 初期化処理 */
    res->pathsend_errcode = 0;
    res->guardian_errcode = 0;
    
    /* PATHMONプロセス名長の取得 */
    memset(pathmon_name,' ',sizeof(pathmon_name));
    memcpy(pathmon_name,req->pathmon_name,sizeof(req->pathmon_name));
    pathmon_name[sizeof(pathmon_name)-1] = 0;
    ptr = strchr(pathmon_name,' ');
    if( ptr != NULL ){
        pathmon_name_len = (short)(ptr - pathmon_name);
            pathmon_name[pathmon_name_len] = 0;
    } else {
        res->guardian_errcode = DEF_COM_PSD_ERR_INVALID_FILE;
        return -1;   /* ファイル名異常 */
    }
    /* サーバクラス名長の取得 */
    memset(serverclass_name,' ',sizeof(serverclass_name));
    memcpy(serverclass_name,req->serverclass_name,sizeof(req->serverclass_name));
    serverclass_name[sizeof(serverclass_name)-1] = 0;
    ptr = strchr(serverclass_name,' ');
    if( ptr != NULL ){
        serverclass_name_len = (short)(ptr-serverclass_name);
        serverclass_name[serverclass_name_len] = 0;
    } else {
        res->guardian_errcode = DEF_COM_PSD_ERR_INVALID_FILE;
        return -1;   /* ファイル名異常 */
    }
    /* PATHSENDデータの退避 */
    memset(save_buff,0x00,sizeof(save_buff));
    memcpy(save_buff,(char *)req->msg_buf,sizeof(save_buff));
    loop_flag = 0;
    retry_count = 0;
    for (loop_flag = 0,retry_count = 0;loop_flag == 0;) {
        /* COM_PSD_Pathsend_Execの呼び出し */
        err = COM_PSD_Pathsend_Exec((char *)pathmon_name
                                   ,pathmon_name_len
                                   ,(char *)serverclass_name
                                   ,serverclass_name_len
                                   ,(char *)req->msg_buf
                                   ,req->req_send_len
                                   ,req->receive_max_len
                                   ,(short *)&req->receive_len
                                   ,req->send_timer_msec
                                   ,&res->pathsend_errcode
                                   ,&res->guardian_errcode
                                   ,trc->prog_id
                                   ,ems_cmn
                                   ,ems_add);
        if ((err == 0)) {
            loop_flag = 1;
        } else {
            if (retry_count < req->retry_cnt) {
                /* EMS出力 リトライ可 */
                COM_PSD_EMS(DEF_NERR_PSEND_ERR_RE_OK
                           ,pathmon_name
                           ,pathmon_name_len
                           ,(char *)serverclass_name
                           ,serverclass_name_len
                           ,&res->pathsend_errcode
                           ,&res->guardian_errcode
                           ,ems_cmn
                           ,ems_add);
                retry_count++;
                memcpy((char *)&req->msg_buf,save_buff,sizeof(save_buff));
            } else {
                loop_flag = 1;
                /* EMS出力 リトライアウト */
                COM_PSD_EMS(DEF_NERR_PSEND_ERR_RE_OUT
                           ,pathmon_name
                           ,pathmon_name_len
                           ,(char *)serverclass_name
                           ,serverclass_name_len
                           ,&res->pathsend_errcode
                           ,&res->guardian_errcode
                           ,ems_cmn
                           ,ems_add);
            }
        }
    }
    return err;
} /* end of COM_PSD */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  COM_PSD_Pathsend_Exec                          */
/*  CALLING SEQ.    : short COM_PSD_Pathsend_Exec(char *,short,char *,short */
/*                                               ,char *,short,short,short* */
/*                                               ,long,short *,short *      */
/*                                               ,char *, oggz1in_def *     */
/*                                               ,COM_PSD_arg_4_def)        */
/*  ARGUMENT        : 1.pathmon_name         (I)   PATHMON名                */
/*                  : 2.pahtmon_len          (I)   PATHMON名長              */
/*                  : 3.serverclasss_name    (I)   サーバクラス名           */
/*                  : 4.serverclss_len       (I)   サーバクラス名長         */
/*                  : 5.buff                 (I/O) メッセージバッファ       */
/*                  : 6.send_len             (I)   送信長                   */
/*                  : 7.max_recv_len         (I)   最大受信長               */
/*                  : 8.recv_len             (O)   応答長                   */
/*                  : 9.timer_val            (I)   タイムアウト値           */
/*                  : 10.serverclass_err     (O)   PATHSENDエラーコード     */
/*                  : 11.filesys_err         (O)   Guardianエラーコード     */
/*                  : 12.prog_id             (I)   モジュールID             */
/*                  : 13.ems_cmn             (I)   EMS出力共通情報          */
/*                  : 14.ems_add             (I)   EMS出力追加情報          */
/*  RETURN CODE     : 0:正常   -1:異常                                      */
/*  DESCRIPTION     : PATHSEND処理のコントロールを行う                      */
/****************************************************************************/
short COM_PSD_Pathsend_Exec(char  *pathmon_name
                           ,short  pathmon_len
                           ,char  *serverclass_name
                           ,short  serverclass_len
                           ,char  *buff
                           ,short  send_len
                           ,short  max_recv_len
                           ,short *recv_len
                           ,long   timer_val
                           ,short *serverclass_err
                           ,short *filesys_err
                           ,char  *prog_id
                           ,oggz1in_def       *ems_cmn
                           ,COM_PSD_arg_4_def *ems_add)
{
    short err;
    long long julian_time;
    COM_SDT_arg_3_def guard_err;
    char text[256];
    COM_PSD_zac2001p_arg_1_def     scs_trace;        /* トレース出力構造体           */
    struct {
        char    start[20];
        char    end[20];
    } trace_time;

    /* トレース出力初期処理 */
    memset((char *)&scs_trace, 0x20, sizeof(scs_trace));
    COM_SDT(2, (COM_SDT_arg_2_def *)trace_time.start, &guard_err, &julian_time);
    scs_trace.func_flg = '1';
    memcpy((char *)&scs_trace.trace_info.prog_id,           prog_id,8);
    memcpy((char *)&scs_trace.trace_info.file_id,           DEF_COM_PSD_TRC_FILE_ID,8);
    memset(text,0x00,sizeof(text));
    sprintf(text,"%s/%s",pathmon_name,                      serverclass_name);
    memcpy((char *)&scs_trace.trace_info.file_name,         text,strlen(text));
    memcpy((char *)&scs_trace.trace_info.file_io_type,      DEF_COM_PSD_TRC_FILE_IO_WRITE, 5);
    memcpy((char *)&scs_trace.trace_info.guardian_errcode,  "0000", 4);
    memcpy((char *) scs_trace.trace_info.shori_start_time,  (char *)&trace_time.start[8],
                                                            sizeof(scs_trace.trace_info.shori_start_time));
    memcpy((char *)&scs_trace.trace_info.shori_end_time,    scs_trace.trace_info.shori_start_time,
                                                            sizeof(scs_trace.trace_info.shori_end_time));
    memset(text,0x00,sizeof(text));
    snprintf(text,sizeof(text), "%05d",send_len);
    memcpy((char *)&scs_trace.data_info.rec_len,    text,strlen(text));
    memcpy((char *)&scs_trace.data_info.rec_area,    buff,(int)send_len);
    /* トレース出力モジュールの呼び出し */
    TRACEOUT((char *)&scs_trace);

    /* 指定サーバクラスへのPATHSEND */
    *serverclass_err = 0;
    *filesys_err = 0;
    err = SERVERCLASS_SEND_(pathmon_name
                           ,pathmon_len
                           ,serverclass_name
                           ,serverclass_len
                           ,(char *)buff
                           ,send_len
                           ,max_recv_len
                           ,(short *)recv_len
                           ,timer_val);
    SERVERCLASS_SEND_INFO_(serverclass_err, filesys_err);

    /* トレース出力情報設定 */
    memset((char *)&scs_trace, 0x20, sizeof(scs_trace));
    COM_SDT(2, (COM_SDT_arg_2_def *)trace_time.end, &guard_err, &julian_time);

    scs_trace.func_flg = '1';
    memcpy((char *)&scs_trace.trace_info.prog_id,           prog_id,8);
    memcpy((char *)&scs_trace.trace_info.file_id,           DEF_COM_PSD_TRC_FILE_ID,8);
    memset(text,0x00,sizeof(text));
    snprintf(text,sizeof(text), "%s/%s", pathmon_name, serverclass_name);
    memcpy((char *)&scs_trace.trace_info.file_name,         text,strlen(text));
    memcpy((char *)&scs_trace.trace_info.file_io_type,      DEF_COM_PSD_TRC_FILE_IO_READ, 4);
    memset(text,0x00,sizeof(text));
    snprintf(text, sizeof(text),"%04d", *filesys_err);
    memcpy((char *)&scs_trace.trace_info.guardian_errcode,  text, strlen(text));
    memcpy((char *) scs_trace.trace_info.shori_start_time,  (char *)&trace_time.start[8],
                                                            sizeof(scs_trace.trace_info.shori_start_time));
    memcpy((char *)&scs_trace.trace_info.shori_end_time,    (char *)&trace_time.end[8],
                                                            sizeof(scs_trace.trace_info.shori_end_time));
    memset(text,0x00,sizeof(text));
    snprintf(text, sizeof(text),"%05d", *recv_len);
    memcpy((char *)&scs_trace.data_info.rec_len,            text, sizeof(scs_trace.data_info.rec_len));
    memcpy((char *)&scs_trace.data_info.rec_area,           buff, (int)*recv_len);
    /* トレース出力モジュールの呼び出し */
    TRACEOUT((char *)&scs_trace);

    if (err != 0) {
        return (-1);
    } else {
        return (0);
    }
} /* end of COM_PSD_Pathsend_Exec */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  COM_PSD_EMS                                    */
/*  CALLING SEQ.    : void COM_PSD_EMS(char *,char *,short ,char *,short ,  */
/*                                     short *,short *,oggz1in_def *,       */
/*                                     COM_PSD_arg_4_def *)                 */
/*  ARGUMENT        : 1.gfp_err_cd           (I)   GFP内部エラーコード      */
/*                  : 2.pathmon_name         (I)   PATHMON名                */
/*                  : 3.pahtmon_len          (I)   PATHMON名長              */
/*                  : 4.serverclasss_name    (I)   サーバクラス名           */
/*                  : 5.serverclss_len       (I)   サーバクラス名長         */
/*                  : 6.serverclass_err      (O)   PATHSENDエラーコード     */
/*                  : 7.filesys_err          (O)   Guardianエラーコード     */
/*                  : 8.ems_cmn              (I)   EMS出力共通情報          */
/*                  : 9.ems_add              (I)   EMS出力追加情報          */
/*  RETURN CODE     : 無し                                                  */
/*  DESCRIPTION     : EMS出力を行う                                         */
/****************************************************************************/
void COM_PSD_EMS( char  *gfp_err_cd
                 ,char  *pathmon_name
                 ,short  pathmon_len
                 ,char  *serverclass_name
                 ,short  serverclass_len
                 ,short *serverclass_err
                 ,short *filesys_err
                 ,oggz1in_def       *ems_cmn
                 ,COM_PSD_arg_4_def *ems_add)
{
    oggz1in_def   ems;            /* EMS出力構造体                */
    char          text[256];

    // EMS情報設定
    memset(&ems, 0x20, sizeof(ems));     //領域初期化
    // 運用監視端末出力サーバI/Oエラー
    ems.subrcd      = '0';

    // EMS出力情報・メッセージID
    snprintf(text, sizeof(text), "%05.05d", DEF_EVT_PSEND_ERR_DETECT);
    memcpy(ems.emsinf.msgid, text, strlen(text));

    /* 運用監視端末情報 */
    memcpy(&ems.uytrminf,  &ems_cmn->uytrminf, sizeof(ems.uytrminf));

    // 業務共通メッセージ
    ems.emsinf.rcd   = '0';
    memcpy(&ems.emsinf.emsgkinf.msgttkb,     DEF_COM_PSD_EMS_MSGTTKB,           strlen(DEF_COM_PSD_EMS_MSGTTKB));
    memcpy(&ems.emsinf.emsgkinf.sysnm,       ems_cmn->emsinf.emsgkinf.sysnm,    sizeof(ems.emsinf.emsgkinf.sysnm));
    memcpy(&ems.emsinf.emsgkinf.srv_kbn,     ems_cmn->emsinf.emsgkinf.srv_kbn,  sizeof(ems.emsinf.emsgkinf.srv_kbn));
    memcpy(&ems.emsinf.emsgkinf.h_nw_kbn,    ems_cmn->emsinf.emsgkinf.h_nw_kbn, sizeof(ems.emsinf.emsgkinf.h_nw_kbn));
    memcpy(&ems.emsinf.emsgkinf.s_nw_kbn,    ems_cmn->emsinf.emsgkinf.s_nw_kbn, sizeof(ems.emsinf.emsgkinf.s_nw_kbn));
    memcpy(&ems.emsinf.emsgkinf.prgid,       ems_cmn->emsinf.emsgkinf.prgid,    sizeof(ems.emsinf.emsgkinf.prgid));
    memcpy(&ems.emsinf.emsgkinf.trmnm,       ems_cmn->emsinf.emsgkinf.trmnm,    sizeof(ems.emsinf.emsgkinf.trmnm));
    memcpy(&ems.emsinf.emsgkinf.inter_errcd, gfp_err_cd,                        strlen(gfp_err_cd));

    // 任意メッセージ・メッセージ要求元の任意パラメータI  ・サーバークラス論理ID
    memcpy(ems.emsinf.emsnninf.msgtbl[0].msgtbl_vl, ems_add->srv_logical_id, sizeof(ems_add->srv_logical_id));

    // 任意メッセージ・メッセージ要求元の任意パラメータII ・LCN (15バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[1].msgtbl_vl, ems_add->lcn, sizeof(ems_add->lcn));

    // 任意メッセージ・メッセージ要求元の任意パラメータIII・PATHMON名 (16バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[2].msgtbl_vl, pathmon_name, pathmon_len);

    // 任意メッセージ・メッセージ要求元の任意パラメータIV・サーバークラス (15バイト)
    memcpy(ems.emsinf.emsnninf.msgtbl[3].msgtbl_vl, serverclass_name, serverclass_len);

    // 任意メッセージ・メッセージ要求元の任意パラメータV ・エラーコード (5バイト)
    snprintf(text, sizeof(text), "%05.05d", *filesys_err);
    memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, text, 5);

    /* EMS出力モジュールの呼び出し */
    GFPOGGZ1(&ems);

    return;
} /* end of COM_PSD_EMS */
