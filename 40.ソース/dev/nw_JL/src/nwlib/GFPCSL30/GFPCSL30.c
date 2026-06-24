/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSL30                                    */
/*        FUNCTION          ････ 電文ヘッダ編集[J-link]                      */
/*                                                                           */
/*                               編集前電文にJ-Linkヘッダーを                */
/*                               付加したデータを編集後電文に設定する        */
/*                               電文データ開始位置の値によって              */
/*                               REDSYSフォーマットとREDSYS以外              */
/*                               フォーマットを区別し各ヘッダー部の編集を行う*/
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-10                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/03/10 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <stdio.h>    nolist
#include  <string.h>   nolist
/* USER HEADER     */
#include "vproc.h"     nolist 
#include "NWM_HDE.h"

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define  DEF_REDSYS_START_POS       3       // 電文開始位置判定値
#define  DEF_DATA_OFSET_REDSYS      2       // 編集後電文データ部オフセット(REDSYS)
#define  DEF_DATA_OFSET_OTHER       4       // 編集後電文データ部オフセット(REDSYS以外)
#define  DEF_YOBI_OFSET_OTHER       2       // 編集後電文予備オフセット(REDSYS以外)
#define  DEF_YOBI_DATA              0x00    // 予備データ(1byte)
#define  DEF_YOBI_SIZE              2       // 予備サイズ

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_HDE                                        */
/*  CALLING SEQ.    : void   NWM_HDE(strcut *, struct *, short, short)      */
/*  ARGUMENT        : 1.msg_in         (I)   編集前電文情報                 */
/*                  : 2.msg_out        (O)   編集後電文情報                 */
/*                  : 3.start_position (I)   電文開始位置                   */
/*                  : 4.send_msg_type  (I)   送信電文種別                   */
/*  RETURN CODE     : viod                                                  */
/*  DESCRIPTION     : 電文ヘッダの編集を行う                                */
/****************************************************************************/
void NWM_HDE(NWM_HDE_arg_1_def *msg_in
          ,  NWM_HDE_arg_2_def *msg_out
          ,  short start_position
          ,  short send_msg_type)
{
    // 電文開始位置が3の場合
    if (start_position == DEF_REDSYS_START_POS){
        // 編集後電文先頭アドレスにヘッダ部追加してデータ部のLengthを設定
        memcpy(msg_out->msg_out_addr
            ,  &msg_in->msg_in_len
            ,  DEF_DATA_OFSET_REDSYS);
        // データ部を設定
        memcpy(msg_out->msg_out_addr + DEF_DATA_OFSET_REDSYS 
            ,  msg_in->msg_in_addr
            ,  msg_in->msg_in_len);
        // 編集後電文レングス設定
        msg_out->msg_out_len = msg_in->msg_in_len + DEF_DATA_OFSET_REDSYS;
    // 電文開始位置が3以外の場合
    } else {
        // 編集後電文先頭アドレスにヘッダ部追加してデータ部のLengthを設定
        memcpy(msg_out->msg_out_addr
            ,  &msg_in->msg_in_len
            ,  DEF_YOBI_OFSET_OTHER);
        // 編集後電文先頭アドレスの予備設定
        memset(msg_out->msg_out_addr + DEF_YOBI_OFSET_OTHER
            ,  DEF_YOBI_DATA
            ,  DEF_YOBI_SIZE);
        // データ部を設定
        memcpy(msg_out->msg_out_addr + DEF_DATA_OFSET_OTHER
            ,  msg_in->msg_in_addr
            ,  msg_in->msg_in_len);
        // 編集後電文レングス設定
        msg_out->msg_out_len = msg_in->msg_in_len + DEF_DATA_OFSET_OTHER;
    }
}
