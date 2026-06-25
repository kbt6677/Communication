/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXA0                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               トランザクション管理モジュール              */
/*                                                                           */
/*                               Nonstop TMFの機能を使用し、呼び出し元から   */
/*                               指定された機能名識別に従い、                */
/*                               トランザクションの管理を行う。              */
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
#include <stdio.h> nolist
#include <string.h> nolist
#include <cextdecs.h> nolist
#include <tal.h> nolist
#include <zsysc> nolist

/* USER HEADER     */
#include "GFPOGGZ4_traceout.h"
#include "GFPCGXA0.h"
#include "GFPCGX50.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_COM_TMF_JPN_TIME    2
/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/
short COM_TMF_Begin(long *,char *);
short COM_TMF_End(char *);
short COM_TMF_Abort(char *);
short COM_TMF_Resume(long,char *);
short COM_TMF_GetReceiveInfo(long *,char *);
short COM_TMF_ChangeMsgTag(long,char *);
void  COM_TMF_GetTimestamp(char *);
void  COM_TMF_OutTrace(char *,short,char *,char *);
//void  TRACEOUT(char *);


/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_TMF                                        */
/*  CALLING SEQ.    : short COM_TMF(char *,long *,char*,short *)            */
/*  ARGUMENT        : 1.func        (I)   機能名識別                        */
/*                  : 2.tran_id     (I)   トランザクションID                */
/*                  : 3.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 -1:パラメータエラー その他：TMFエラー          */
/*  DESCRIPTION     : トランザクション管理のコントロールを行う              */
/****************************************************************************/
short COM_TMF(char *func
             ,long *tran_id
             ,char *prog_id)
{
    short result;
    
    if        (memcmp(func,DEF_COM_TMF_BEGIN ,4) == 0) {
      result = COM_TMF_Begin((long *)tran_id,prog_id);
    } else if (memcmp(func,DEF_COM_TMF_END   ,4) == 0) {
      result = COM_TMF_End(prog_id);
    } else if (memcmp(func,DEF_COM_TMF_ABORT ,4) == 0) {
      result = COM_TMF_Abort(prog_id);
    } else if (memcmp(func,DEF_COM_TMF_RESUME,4) == 0) {
      result = COM_TMF_Resume((long)*tran_id,prog_id);
    } else if (memcmp(func,DEF_COM_TMF_CLEAR ,4) == 0) {
      result = COM_TMF_Resume(0,prog_id);
    } else if (memcmp(func,DEF_COM_TMF_GET   ,4) == 0) {
      result = COM_TMF_GetReceiveInfo((long *)tran_id,prog_id);
    } else if (memcmp(func,DEF_COM_TMF_ACTIVE,4) == 0) {
      result = COM_TMF_ChangeMsgTag((long)*tran_id,prog_id);
    } else {
      result = -1;
    }
    return result;
} /* end of COM_TMF */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  COM_TMF_BEGIN                                  */
/*  CALLING SEQ.    : short COM_TMF_BEGIN(long *, char *)                   */
/*  ARGUMENT        : 1.tran_id     (O)   トランザクションID                */
/*                    2.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : トランザクションの開始を行う                          */
/****************************************************************************/
short COM_TMF_Begin(long *tran_id, char *prog_id)
{
    char start_time[12];
    short sStatus;

    COM_TMF_GetTimestamp((char *)start_time);
    sStatus = BEGINTRANSACTION((long *)tran_id);
    COM_TMF_OutTrace("B-T     ",sStatus,(char *)start_time,prog_id);
    return sStatus;
} /* end of COM_TMF_Begin */

/****************************************************************************/
/*  FUNCTION        : 1.2.0  COM_TMF_End                                    */
/*  CALLING SEQ.    : short COM_TMF_End(char *)                             */
/*  AGUMENT         : 1.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : トランザクションの終了を行う                          */
/****************************************************************************/
short COM_TMF_End(char *prog_id)
{
    char start_time[12];
    short sStatus;

    COM_TMF_GetTimestamp((char *)start_time);
    sStatus = ENDTRANSACTION();
    COM_TMF_OutTrace("E-T     ",sStatus,(char *)start_time,prog_id);
    return sStatus;
} /* end of COM_TMF_End */

/****************************************************************************/
/*  FUNCTION        : 1.3.0  COM_TMF_Abort                                  */
/*  CALLING SEQ.    : short COM_TMF_Abort(char *)                           */
/*  AGUMENT         : 1.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : トランザクションのABORTを行う                         */
/****************************************************************************/
short COM_TMF_Abort(char *prog_id)
{
    char start_time[12];
    short sStatus;

    COM_TMF_GetTimestamp((char *)start_time);
    sStatus = ABORTTRANSACTION();
    COM_TMF_OutTrace("A-T     ",sStatus,(char *)start_time,prog_id);
    return sStatus;
} /* end of COM_TMF_Abort */

/****************************************************************************/
/*  FUNCTION        : 1.4.0  COM_TMF_Resume                                 */
/*  CALLING SEQ.    : short COM_TMF_Resume(long ,char *)                    */
/*  AGUMENT         : 1.tran_id     (I)   トランザクションID                */
/*                  : 2.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : トランザクションの切り替えを行う                      */
/****************************************************************************/
short COM_TMF_Resume(long tran_id,char *prog_id)
{
    char start_time[12];
    short sStatus;

    COM_TMF_GetTimestamp((char *)start_time);
    sStatus = RESUMETRANSACTION(tran_id);
    COM_TMF_OutTrace("R-T     ",sStatus,(char *)start_time,prog_id);
    return sStatus;
} /* end of COM_TMF_Resume */

/****************************************************************************/
/*  FUNCTION        : 1.5.0  COM_TMF_GetReceiveInfo                         */
/*  CALLING SEQ.    : short COM_TMF_GetReceiveInfo(long ,char *)            */
/*  AGUMENT         : 1.tran_id     (O)   トランザクションID                */
/*                  : 2.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : リクエスタからの受信情報取得を行う                    */
/****************************************************************************/
short COM_TMF_GetReceiveInfo(long *tran_id,char *prog_id)
{
    char start_time[12];
    short sStatus;
    zsys_ddl_receiveinformation_def RecvInfo;

    COM_TMF_GetTimestamp((char *)start_time);
    sStatus = FILE_GETRECEIVEINFO_((short *)&RecvInfo);
    if (sStatus != 0) {
        return sStatus;
    }
    COM_TMF_OutTrace("G-T     ",sStatus,(char *)start_time,prog_id);
    *tran_id = ((long)RecvInfo.z_messagetag << 16 | (long)RecvInfo.z_messagetag);
    return sStatus;
} /* end of COM_TMF_GetReceiveInfo */

/****************************************************************************/
/*  FUNCTION        : 1.6.0  COM_TMF_ChangeMsgTag                           */
/*  CALLING SEQ.    : short COM_TMF_ChangeMsgTag(long ,char *)              */
/*  AGUMENT         : 1.tran_id     (I)   トランザクションID                */
/*                  : 2.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     : 0:正常 その他：TMFエラーコード                        */
/*  DESCRIPTION     : 指定されたメッセージタグにトランザクションの切り替え  */
/*                    を行う                                                */
/****************************************************************************/
short COM_TMF_ChangeMsgTag(long tran_id,char *prog_id)
{
    char start_time[12];
    short sMsgTag;
    short sStatus = 0;
    _cc_status iCC;

    COM_TMF_GetTimestamp((char *)start_time);
    sMsgTag = (short)(tran_id & 0x0000ffff);
    iCC = ACTIVATERECEIVETRANSID(sMsgTag);
    if (_status_ne(iCC)) {
        FILE_GETINFO_(-1,&sStatus);
    }
    COM_TMF_OutTrace("ACT     ",sStatus,(char *)start_time,prog_id);
    return sStatus;
} /* end of COM_TMF_ChangeMsgTag */

/****************************************************************************/
/*  FUNCTION        : 1.7.0  COM_TMF_GetTimestamp                           */
/*  CALLING SEQ.    : void  COM_TMF_GetTimestamp(char *)                    */
/*  AGUMENT         : 1.get_time    (O)   タイムスタンプ                    */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : トレース用時刻の取得を行う                            */
/****************************************************************************/
void COM_TMF_GetTimestamp(char *get_time)
{
long long julian_time;
char  timedata[20];
short timedata_b[8];

    COM_SDT(DEF_COM_TMF_JPN_TIME,(COM_SDT_arg_2_def *)timedata,(COM_SDT_arg_3_def *)timedata_b,&julian_time);
    memcpy((char *)get_time,(char *)&timedata[8],12);
} /* end of COM_TMF_GetTimestamp */

/****************************************************************************/
/*  FUNCTION        : 1.8.0  COM_TMF_OutTrace                               */
/*  CALLING SEQ.    : void  COM_TMF_OutTrace(char *,short ,char *)          */
/*  AGUMENT         : 1.file_id     (I)   ファイル識別子                    */
/*                  : 2.gurdian_err (I)   GuardianErrorコード               */
/*                  : 3.start_time  (I)   タイムスタンプ                    */
/*                  ; 4.prog_id     (I)   モジュールID                      */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : トレース取得を行う                                    */
/****************************************************************************/
void COM_TMF_OutTrace(char *file_id,short guardian_err,char *start_time,char *prog)
{
    lk_zac2001t_arg_1_def tmf_trace;

    memset((char *)&tmf_trace,' ',lk_zac2001t_arg_1_def_Size);
    tmf_trace.func_flg = '1';
    memcpy((char *)&tmf_trace.trace_info.prog_id,prog,8);
    memcpy((char *)&tmf_trace.trace_info.file_id,"TMF",3);
    /* file-name */
    memcpy(tmf_trace.trace_info.file_io_type,(char *)file_id
                                ,sizeof(tmf_trace.trace_info.file_io_type));
    sprintf(tmf_trace.trace_info.guardian_errcode,"%04d",guardian_err);
    memcpy(tmf_trace.trace_info.shori_start_time,(char *)start_time
                                ,sizeof(tmf_trace.trace_info.shori_start_time));
    COM_TMF_GetTimestamp((char *)tmf_trace.trace_info.shori_end_time);
    TRACEOUT((char *)&tmf_trace);
} /* end of COM_TMF_OutTrace */
