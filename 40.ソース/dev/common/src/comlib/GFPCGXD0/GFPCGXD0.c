/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXD0                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               ASSIGN情報取得モジュール                    */
/*                                                                           */
/*                               ASSIGN論理フィル名よりASSIGN物理ファイル名  */
/*                               を取得する。                                */
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
#include "GFPCGXD0.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define   DEF_COM_ASN_ID_LEN      32                   /* 論理ファイルID長 */
#define   DEF_COM_ASN_NAME_LEN    48                   /* 物理ファイル名長 */
#define   DEF_COM_ASN_PORTION     "TANDEMNAME"         /* ASSIGN識別子     */

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_ASN                                        */
/*  CALLING SEQ.    : void  COM_ASN(char *, char *,short *)                 */
/*  ARGUMENT        : 1.file_id         (I)    ASSIGN論理ファイルID         */
/*                  : 2.file_name       (O)    ASSIGN物理ファイル名         */
/*                  : 3.file_name_len   (O)    ASSIGN物理ファイ名長         */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : ASSIGN情報取得のコントロールを行う                    */
/****************************************************************************/

void  COM_ASN(char *file_id, char *file_name, short *file_name_len)
{
    char   wk_file_id[DEF_COM_ASN_ID_LEN+1];
    char   wk_file_name[DEF_COM_ASN_NAME_LEN+1];
    char  *ptr;
    short  msg_num;
    short  ret;

    /* 内部変数 初期化理     */
    memset(wk_file_id,' ',sizeof(wk_file_id));
    wk_file_id[DEF_COM_ASN_ID_LEN] = 0x00;
    memset(wk_file_name,' ',sizeof(wk_file_name));

    /* 引数 初期化理     */
    memset(file_name,' ',DEF_COM_ASN_NAME_LEN);
    *file_name_len = 0;

    /* 第1アーギュメントの精査 */
    if(memcmp(wk_file_id,file_id,DEF_COM_ASN_ID_LEN) == 0){
        return; /* ALL SPACE      */
    }
    memcpy(wk_file_id,file_id,DEF_COM_ASN_ID_LEN);
    ptr = strchr(wk_file_id,' ');
    if (ptr == (char *)&wk_file_id[0]) {
        return; /* 左スペースあり */
    }

    /* 論理ファイル名のチェック */
    msg_num = SMU_ASSIGN_CHECKNAME_(wk_file_id
                                   ,DEF_COM_ASN_ID_LEN);
    if (msg_num <= 0) {
        return; /* ASSIGN物理名取得失敗 */
    }
    /* 物理ファイル名の取得 */
    ret = SMU_ASSIGN_GETTEXT_(msg_num
                             ,DEF_COM_ASN_PORTION
                             ,(short)strlen(DEF_COM_ASN_PORTION)
                             ,wk_file_name
                             ,DEF_COM_ASN_NAME_LEN);
    if (ret <= 0) {
        return; /* ASSIGN物理名取得失敗 */
    }

    /* 第2アーギュメントの設定*/
    memcpy(file_name,wk_file_name,ret);

    /* 第3アーギュメントの設定*/
    *file_name_len = ret;

    return;
} /* end of COM_ASN */
