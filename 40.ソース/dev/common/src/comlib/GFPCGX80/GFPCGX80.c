/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX80                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               エラー出力ログ編集出力モジュール            */
/*                                                                           */
/*                               エラー出力ログファイルに対する              */
/*                               ファイル操作(OPEN/WRITE/CLOSE)を行う        */
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
/*  1.1  T.Fukunaga 2025/04/08 IT不具合No.36                                 */
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
#include <tal.h>

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"
#include "GFPCGXD0.h"
#include "GFPCGXB0.h"
#include "GFPCGX80.h"
#include "GFPCGX90.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/
short COM_ERL_param_check(short, short);
short COM_ERL_logfl_out(COM_ERL_arg_1_def *
                       ,COM_ERL_arg_2_def *
                       ,oggz1in_def       *
                       ,COM_ERL_arg_3_def *
                       ,char *);
void  COM_ERL_ems_out(short
                   , short
                   , COM_IOM_arg_4_def *
                   , COM_IOM_arg_5_def *
                   , COM_IOM_arg_6_def *
                   , oggz1in_def       *
                   , COM_ERL_arg_3_def *
                   , char  *);

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_ERL                                        */
/*  CALLING SEQ.    : short COM_ERL(struct *                                */
/*                                , struct *                                */
/*                                , struct *                                */
/*                                , char   *)                               */
/*  ARGUMENT        : 1.input_inf   (I)   入力情報                          */
/*                  : 2.errlog_inf  (I/O) エラー出力ログファイル情報        */
/*                  : 3.ems_cmn     (I)   EMS出力共通情報                   */
/*                  : 4.ems_add     (I)   EMS出力追加情報                   */
/*                  : 5.prog_id     (I)   プログラム論理ID                  */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : エラー出力ログ編集出力のコントロールを行う            */
/****************************************************************************/
short COM_ERL(COM_ERL_arg_1_def *input_inf
            , COM_ERL_arg_2_def *errlog_inf
            , oggz1in_def       *ems_cmn
            , COM_ERL_arg_3_def *ems_add
            , char *prog_id )
{
    short ret;

    /* パラメータチェック処理の呼び出し */
    ret = COM_ERL_param_check(input_inf->file_io_type, errlog_inf->file_no);
    if(ret != 0){
        return ret;
    }
    /* エラー出力ログ出力処理 */
    ret = COM_ERL_logfl_out(input_inf, errlog_inf, ems_cmn, ems_add, prog_id);

    return ret;

} /* end of COM_ERL */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  COM_ERL_param_check                            */
/*  CALLING SEQ.    : short COM_ERL_param_check(short                       */
/*                                             ,short )                     */
/*  ARGUMENT        : 1.file_io_type         (I)   ファイルIO種別           */
/*                  : 2.file_num             (I)   ファイル番号             */
/*  RETURN CODE     : 0:正常   -1:異常                                      */
/*  DESCRIPTION     : パラメータのチェックを行う                            */
/****************************************************************************/
short COM_ERL_param_check(short  file_io_type
                         ,short  file_num)
{
    short erFLG;

    switch(file_io_type){
    case DEF_COM_ERL_ARG1_OPEN:
        if(file_num == -1){
            erFLG =  0;             /* 正常 */
        }else {
            erFLG = -1;             /* 異常 */
        }
        break;
    case DEF_COM_ERL_ARG1_WRITE:
        if(file_num == -1){
            erFLG =  -1;            /* 異常 */
        }else {
            erFLG =   0;            /* 正常 */
        }
        break;
    case DEF_COM_ERL_ARG1_CLOSE:
        if(file_num == -1){
            erFLG =  -1;            /* 異常 */
        }else {
            erFLG =   0;            /* 正常 */
        }
        break;
    default:
        erFLG =  -1;                /* 異常 */
    }

    return erFLG;

} /* end of COM_ERL_param_check */

/****************************************************************************/
/*  FUNCTION        : 1.2.0  COM_ERL_logfl_out                              */
/*  CALLING SEQ.    : short COM_ERL_logfl_out(struct *                      */
/*                                          , struct *                      */
/*                                          , struct *                      */
/*                                          , struct *                      */
/*                                          , char   *)                     */
/*  ARGUMENT        : 1.input_inf   (I)   入力情報                          */
/*                  : 2.errlog_inf  (I/O) エラー出力ログファイル情報        */
/*                  : 3.ems_cmn     (I)   EMS出力共通情報                   */
/*                  : 4.ems_add     (I)   EMS出力追加情報                   */
/*                  : 5.prog_id     (I)   プログラム論理ID                  */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : エラー出力ログファイル操作を行う                      */
/****************************************************************************/
short COM_ERL_logfl_out(COM_ERL_arg_1_def *input_inf
                      , COM_ERL_arg_2_def *errlog_inf
                      , oggz1in_def       *ems_cmn
                      , COM_ERL_arg_3_def *ems_add
                      , char *prog_id)
{
    short err;
    short file_name_len;
    char  subprog_sts[2];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_fl_inf;
    COM_IOM_arg_6_def out_fl_inf;
    COM_UNQ_arg_1_def unq_time_1;
    char              unq_time_2[16];
    db_glelg_def      *glelg = (db_glelg_def *)&in_fl_inf.rec_area[0];

    switch(input_inf->file_io_type){
    case DEF_COM_ERL_ARG1_OPEN:
        /* エラー出力ログファイル物理名の取得 */
        memset(file_inf.file_id,' ',sizeof(file_inf.file_id));
        memcpy(file_inf.file_id,DEF_GLELG,sizeof(DEF_GLELG)-1);
        COM_ASN(file_inf.file_id,file_inf.file_name,&file_name_len);
        if(file_name_len == 0){
            /* ASSIGN取得失敗のEMS出力を行う              */
            COM_ERL_ems_out(DEF_EVT_ASN_FILE_GET_ERR
                         , input_inf->file_io_type
                         , &file_inf
                         , &in_fl_inf
                         , &out_fl_inf
                         , ems_cmn
                         , ems_add
                         , prog_id);
            return -1;
        }
        /* トレース情報の設定 */
        memset((char *)&trace_inf,0x20,sizeof(trace_inf));
        memcpy(trace_inf.prog_id,          prog_id  , 8);
        memcpy(trace_inf.file_id,          DEF_GLELG, sizeof(DEF_GLELG)-1);
        memcpy(trace_inf.file_name,        file_inf.file_name,file_name_len);
        memcpy(trace_inf.file_io_type,     "OPEN",4);
        /* エラー出力ログファイルのOPEN */
        in_fl_inf.io_timer           = input_inf->io_timer;
        err = COM_IOM(DEF_COM_IOM_FUNC_OPEN
                      , subprog_sts
                      , &trace_inf
                      , &file_inf
                      , &in_fl_inf
                      , &out_fl_inf
                        );
        if(err != 0){
            /* OPEN失敗のEMS出力を行う              */
            COM_ERL_ems_out(DEF_EVT_FILE_IO_ERR
                         , input_inf->file_io_type
                         , &file_inf
                         , &in_fl_inf
                         , &out_fl_inf
                         , ems_cmn
                         , ems_add
                         , prog_id);
            return -1;
        }
        memcpy(errlog_inf->file_name, file_inf.file_name, sizeof(errlog_inf->file_name));
        errlog_inf->file_no = file_inf.file_no;
        break;
    case DEF_COM_ERL_ARG1_WRITE:
        /* トレース情報の設定 */
        memset((char *)&trace_inf,0x20,sizeof(trace_inf));
        memcpy(trace_inf.prog_id,          prog_id  , 8);
        memcpy(trace_inf.file_id,          DEF_GLELG, sizeof(DEF_GLELG)-1);
        memcpy(trace_inf.file_name,        errlog_inf->file_name,sizeof(trace_inf.file_name));
        memcpy(trace_inf.file_io_type,     "WRITE",5);
        /* エラー出力ログファイルへのWRITE */
        memset(file_inf.file_id,' ',sizeof(file_inf.file_id));
        memcpy(file_inf.file_id,DEF_GLELG,sizeof(DEF_GLELG)-1);
        memcpy(file_inf.file_name, errlog_inf->file_name, sizeof(file_inf.file_name));
        file_inf.file_no = errlog_inf->file_no;
        in_fl_inf.part_key_type      = 0;
        in_fl_inf.part_key_position  = 0;
        in_fl_inf.part_key_len       = 0;
        in_fl_inf.io_timer           = input_inf->io_timer;
        memcpy(in_fl_inf.rec_area,input_inf->data_area,input_inf->data_len);
        in_fl_inf.rec_len            = input_inf->data_len;
        /* プライマリーキー設定 */
        err = COM_UNQ(&unq_time_1,unq_time_2);
        memcpy(&glelg->pri_key.part_id,&unq_time_1.cc[1],sizeof(glelg->pri_key.part_id));
        memcpy(glelg->pri_key.time_stamp,&unq_time_1,sizeof(glelg->pri_key.time_stamp));
        memcpy(glelg->pri_key.time_stamp_branch,unq_time_2,sizeof(glelg->pri_key.time_stamp_branch));

        err = COM_IOM(DEF_COM_IOM_FUNC_ADD
                      , subprog_sts
                      , &trace_inf
                      , &file_inf
                      , &in_fl_inf
                      , &out_fl_inf
                        );
        if(err != 0){
            /* WRITE失敗のEMS出力を行う              */
            COM_ERL_ems_out(DEF_EVT_FILE_IO_ERR
                         , input_inf->file_io_type
                         , &file_inf
                         , &in_fl_inf
                         , &out_fl_inf
                         , ems_cmn
                         , ems_add
                         , prog_id);
            return -1;
        }
        break;
    case DEF_COM_ERL_ARG1_CLOSE:
        /* トレース情報の設定 */
        memset((char *)&trace_inf,0x20,sizeof(trace_inf));
        memcpy(trace_inf.prog_id,          prog_id  , 8);
        memcpy(trace_inf.file_id,          DEF_GLELG, sizeof(DEF_GLELG)-1);
        memcpy(trace_inf.file_name,        errlog_inf->file_name,sizeof(trace_inf.file_name));
        memcpy(trace_inf.file_io_type,     "CLOSE",5);
        /* エラー出力ログファイルのCLOSE */
        memset(file_inf.file_id,' ',sizeof(file_inf.file_id));
        memcpy(file_inf.file_id,DEF_GLELG,sizeof(DEF_GLELG)-1);
        memcpy(file_inf.file_name, errlog_inf->file_name, sizeof(file_inf.file_name));
        file_inf.file_no = errlog_inf->file_no;
        in_fl_inf.io_timer           = input_inf->io_timer;
        err = COM_IOM(DEF_COM_IOM_FUNC_CLOSE
                      , subprog_sts
                      , &trace_inf
                      , &file_inf
                      , &in_fl_inf
                      , &out_fl_inf
                        );
        if(err != 0){
            /* CLOSE取得失敗のEMS出力を行う              */
            COM_ERL_ems_out(DEF_EVT_FILE_IO_ERR
                         , input_inf->file_io_type
                         , &file_inf
                         , &in_fl_inf
                         , &out_fl_inf
                         , ems_cmn
                         , ems_add
                         , prog_id);
            return -1;
        }
        break;
    }
    return 0;
} /* end of COM_ERL_logfl_out */
/****************************************************************************/
/*  FUNCTION        : 1.2.1  COM_ERL_ems_out                                */
/*  CALLING SEQ.    : void   COM_ERL_ems_out   (short                       */
/*                                          , short                         */
/*                                          , struct *                      */
/*                                          , struct *                      */
/*                                          , struct *                      */
/*                                          , char   * )                    */
/*  ARGUMENT        : 1.evnet_id    (I)   イベントID                        */
/*                  : 2.file_io_type(I)   機能識別                          */
/*                  : 3.file_inf    (I)   ファイル情報                      */
/*                  : 4.in_fl_inf   (I)   入力情報                          */
/*                  : 5.ems_cmn     (I)   EMS出力共通情報                   */
/*                  : 6.ems_add     (I)   EMS出力追加情報                   */
/*                  : 7.prog_id     (I)   プログラム論理ID                  */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : エラー出力ログファイル操作を行う                      */
/****************************************************************************/
void     COM_ERL_ems_out(short  event_id
                      , short  file_io_type
                      , COM_IOM_arg_4_def *file_inf
                      , COM_IOM_arg_5_def *in_fl_inf
                      , COM_IOM_arg_6_def *out_fl_inf
                      , oggz1in_def       *ems_cmn
                      , COM_ERL_arg_3_def *ems_add
                      , char  *prog_id )
{
    oggz1in_def   ems;              /* EMS出力構造体                */
    char  wk_msgid[5+1];            /* メッセージID  CHAR変換用WORK */
    char  wK_guardian_errcode[5+1]; /* GuardianError CHAR変換用WORK */
    db_glelg_def  *glelg;           /* GLELG KEY長取得用            */

    /* 初期設定 */
    memset(&ems, ' ', sizeof(ems));

    ems.emsinf.rcd  = '0';
    sprintf(wk_msgid,"%05d",event_id);
    memcpy(ems.emsinf.msgid ,wk_msgid,strlen(ems.emsinf.msgid));

    /* 運用監視端末情報 */
    memcpy(&ems.uytrminf,  &ems_cmn->uytrminf, sizeof(ems.uytrminf));

    // 業務共通メッセージ
    memcpy(&ems.emsinf.emsgkinf.msgttkb,  DEF_COM_ERL_EMS_MSGTTKB,           strlen(DEF_COM_ERL_EMS_MSGTTKB));
    memcpy(&ems.emsinf.emsgkinf.sysnm,    ems_cmn->emsinf.emsgkinf.sysnm,    sizeof(ems.emsinf.emsgkinf.sysnm));
    memcpy(&ems.emsinf.emsgkinf.srv_kbn,  ems_cmn->emsinf.emsgkinf.srv_kbn,  sizeof(ems.emsinf.emsgkinf.srv_kbn));
    memcpy(&ems.emsinf.emsgkinf.h_nw_kbn, ems_cmn->emsinf.emsgkinf.h_nw_kbn, sizeof(ems.emsinf.emsgkinf.h_nw_kbn));
    memcpy(&ems.emsinf.emsgkinf.s_nw_kbn, ems_cmn->emsinf.emsgkinf.s_nw_kbn, sizeof(ems.emsinf.emsgkinf.s_nw_kbn));
    memcpy(&ems.emsinf.emsgkinf.prgid,    ems_cmn->emsinf.emsgkinf.prgid,    sizeof(ems.emsinf.emsgkinf.prgid));
    memcpy(&ems.emsinf.emsgkinf.trmnm,    ems_cmn->emsinf.emsgkinf.trmnm,    sizeof(ems.emsinf.emsgkinf.trmnm));

    // 任意メッセージ(固定)

    // イベントID毎個別設定
    switch(event_id){
    case DEF_EVT_ASN_FILE_GET_ERR :
        // 業務共通メッセージ・GFP内部エラーコード
        memcpy(ems.emsinf.emsgkinf.inter_errcd, DEF_COM_ERL_EMS_INN_ECD2, strlen(DEF_COM_ERL_EMS_INN_ECD2));
        /* 任意メッセージ情報 */
        memcpy(ems.emsinf.emsnninf.msgtbl[0].msgtbl_vl, ems_add->srv_logical_id,sizeof(ems_add->srv_logical_id));
        memcpy(ems.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GLELG,strlen(DEF_GLELG));
        break;
    case DEF_EVT_FILE_IO_ERR :
        /* 任意メッセージ情報 */
        memcpy(ems.emsinf.emsnninf.msgtbl[0].msgtbl_vl, ems_add->srv_logical_id, sizeof(ems_add->srv_logical_id));
        memcpy(ems.emsinf.emsnninf.msgtbl[1].msgtbl_vl, ems_add->lcn,            sizeof(ems_add->lcn));
        memcpy(ems.emsinf.emsnninf.msgtbl[2].msgtbl_vl, ems_add->connect,        sizeof(ems_add->connect));
        memcpy(ems.emsinf.emsnninf.msgtbl[3].msgtbl_vl, file_inf->file_id,sizeof(file_inf->file_id));
        switch(file_io_type){
        case DEF_COM_ERL_ARG1_OPEN:
            // 業務共通メッセージ・GFP内部エラーコード
            memcpy(ems.emsinf.emsgkinf.inter_errcd, DEF_COM_ERL_EMS_INN_ECD2, strlen(DEF_COM_ERL_EMS_INN_ECD2));
            /* 任意メッセージ情報 */
            memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "OPEN", 4);
            sprintf(wK_guardian_errcode,"%05d", out_fl_inf->guardian_errcode);
            memcpy(ems.emsinf.emsnninf.msgtbl[6].msgtbl_vl, wK_guardian_errcode, strlen(wK_guardian_errcode));
            break;
        case DEF_COM_ERL_ARG1_WRITE:
            // 業務共通メッセージ・GFP内部エラーコード
            memcpy(ems.emsinf.emsgkinf.inter_errcd, DEF_COM_ERL_EMS_INN_ECD1, strlen(DEF_COM_ERL_EMS_INN_ECD1));
            /* 任意メッセージ情報 */
            memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "WRITE",5);
            memcpy(ems.emsinf.emsnninf.msgtbl[5].msgtbl_vl, in_fl_inf->rec_area,sizeof(glelg->pri_key));
            sprintf(wK_guardian_errcode,"%05d",out_fl_inf->guardian_errcode);
            memcpy(ems.emsinf.emsnninf.msgtbl[6].msgtbl_vl, wK_guardian_errcode, strlen(wK_guardian_errcode));
            break;
        case DEF_COM_ERL_ARG1_CLOSE:
            // 業務共通メッセージ・GFP内部エラーコード
            memcpy(ems.emsinf.emsgkinf.inter_errcd, DEF_COM_ERL_EMS_INN_ECD1, strlen(DEF_COM_ERL_EMS_INN_ECD1));
            /* 任意メッセージ情報 */
            memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, "CLOSE",4);
            sprintf(wK_guardian_errcode,"%05d",out_fl_inf->guardian_errcode);
            memcpy(ems.emsinf.emsnninf.msgtbl[6].msgtbl_vl, wK_guardian_errcode, strlen(wK_guardian_errcode));
            break;
        }
        break;
    }
    /* 呼び出し */
    GFPOGGZ1(&ems);
}
