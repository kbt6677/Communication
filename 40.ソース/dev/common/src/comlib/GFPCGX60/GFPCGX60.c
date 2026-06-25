/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX60                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               IOタグ生成・解析モジュール                  */
/*                                                                           */
/*                               IOタグの生成・解析を行う                    */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
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
#include <clurdec.h>  nolist

/* USER HEADER     */
#include "GFPCGX60.h"

/* vproc関数の宣言 */
#include "vproc.h"

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/
typedef union
{
    struct {
        unsigned short compo   :  4;
        unsigned short thread  : 12;
        unsigned short event   : 16;
    } x;
    unsigned long val;
} tags_u;
tags_u tags;
#define  DEF_COM_TGM_MAX_COMPO       15
#define  DEF_COM_TGM_MAX_THREAD      4095
#define  DEF_COM_TGM_MAX_EVENT       65535
/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_TGM                                        */
/*  CALLING SEQ.    : short  COM_TGM(unsigned short                         */
/*                                  ,unsigned short                         */
/*                                  ,unsigned short                         */
/*                                  ,unsigned *long )                       */
/*  ARGUMENT        : 1.compo           (I)    コンポーネント               */
/*                  : 2.thread          (I)    スレッド                     */
/*                  : 3.event           (I)    イベント                     */
/*                  : 4.io_tag          (O)    IOタグ                       */
/*  RETURN CODE     : 生成タグ(long)                                        */
/*  DESCRIPTION     : IOタグの生成を行う                                    */
/****************************************************************************/
short COM_TGM(unsigned short compo
             ,unsigned short thread
             ,unsigned short event
             ,unsigned long  *io_tag)
{
    if(compo  > DEF_COM_TGM_MAX_COMPO){
        return(-1);
    }
    if(thread > DEF_COM_TGM_MAX_THREAD){
        return(-1);
    }
    if(event  > DEF_COM_TGM_MAX_EVENT){
        return(-1);
    }
    tags.x.compo   = compo;
    tags.x.thread  = thread;
    tags.x.event   = event;
    *io_tag        = tags.val;
    return(0);
} /* end of COM_TGM */

/****************************************************************************/
/*  FUNCTION        : 2.0.0  COM_TGA                                        */
/*  CALLING SEQ.    : void   COM_TGA(unsigned long                          */
/*                                  ,unsigned short *                       */
/*                                  ,unsigned short *                       */
/*                                  ,unsigned short *)                      */
/*  ARGUMENT        : 1.io_tag          (I)    IOタグ                       */
/*                  : 2.compo           (O)    コンポーネント               */
/*                  : 3.thread          (O)    スレッド                     */
/*                  : 4.event           (O)    イベント                     */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : IOタグの生成を行う                                    */
/****************************************************************************/

void  COM_TGA(unsigned long  io_tag
             ,unsigned short *compo
             ,unsigned short *thread
             ,unsigned short *event)
{
    tags.val = io_tag;
    *compo   = tags.x.compo;
    *thread  = tags.x.thread;
    *event   = tags.x.event;
    return;
} /* end of COM_TGM */
