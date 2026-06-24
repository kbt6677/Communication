/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ50                                    */
/*        FUNCTION          ････ NW個別(要求応答マッチングキー生成[VISANET]) */
/*                                                                           */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-04-18                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.F        2025/04/18 新規作成                                      */
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
#include "msg_VI.h"
#include "NWM_MKM.h"
#include "NWM_NTC.h"
#include "GFPCSV50.h"
#include "vproc.h"

/* LOCAL 関数      */
static void BCDTOASCII(int,unsigned char *,char *);

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  要求応答マッチングキー生成処理                  */
/*  CALLING SEQ.    : short NWM_MKM()                                        */
/*  ARGUMENT        : 1.mtiinfo        (I)   MTI                             */
/*                  : 2.queinfo        (I)   キューファイル電文情報          */
/*                  : 3.fixdata        (I)   固定フォーマットデータ          */
/*                  : 4.matchinfo      (I/O) マッチングキー情報              */
/*                    5.matkeysize     (I)   制御電文マッチングキーサイズ    */
/*                    6.matkeylen      (I/O) マッチングキー生成バイト長      */
/*  RETURN CODE     : 0:正常 1:エラー                                        */
/*  DESCRIPTION     : NW個別(要求応答マッチングキー生成[VISANET])            */
/*****************************************************************************/
short NWM_MKM(char           *mtiinfo,
              queue_data_def *queinfo,
              char           *fixdata,
              char           *matchinfo,
              short int      matkeysize,
              short int      *matkeylen)
{
    fixedform_visanet_0800_def  *ffd_0800;
    ffd_0800  = (fixedform_visanet_0800_def*)fixdata;
    fixedform_visanet_0810_def  *ffd_0810 = (fixedform_visanet_0810_def*)fixdata;
    mkm_matchkey_def            *matchkey = (mkm_matchkey_def*)matchinfo;
    MSG_HEADER_VISA_def         *msg_header;

    msg_header = (MSG_HEADER_VISA_def *)queinfo->denbun;
    memset(matchinfo,0x00,sizeof(mkm_matchkey_def));

    /* マッチングキー最大長チェック             */
    if (matkeysize < DEF_MAX_MATCHINGKEY) {
        *matkeylen = (short) 0;
        return DEF_MKM_ERROR;
    }
    
    /* 要求応答マッチングキー生成(要求電文受信) */
    if (memcmp(mtiinfo, DEF_VI_MTI_0800_REQ, strlen(DEF_VI_MTI_0800_REQ)) == 0) {
        if (ffd_0800->b37.ffd_header.m_flg_exist == true \
            && ffd_0800->b70.ffd_header.m_flg_exist == true) {
            BCDTOASCII(3,(unsigned char *)msg_header->mh_src_id,\
                                  (char *)matchkey->ctrl_src_id);
            memcpy(matchkey->mti,DEF_VI_MTI_0800_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->local_tran_time, \
                ffd_0800->b37.ffd_data, \
                ffd_0800->b37.ffd_header.m_fixvalue_length);
            memcpy(matchkey->function_code, \
                ffd_0800->b70.ffd_data, \
                ffd_0800->b70.ffd_header.m_fixvalue_length);
            *matkeylen = (short) DEF_MAX_MATCHINGKEY;
            return DEF_MKM_NORMAL;
        }
    }
    /* 要求応答マッチングキー生成(応答電文受信) */
    else if (memcmp(mtiinfo, DEF_VI_MTI_0810_RSP, strlen(DEF_VI_MTI_0810_RSP)) == 0) {
        if (ffd_0810->b37.ffd_header.m_flg_exist == true \
            && ffd_0810->b70.ffd_header.m_flg_exist == true) {
            BCDTOASCII(3,(unsigned char *)msg_header->mh_dst_id,\
                                  (char *)matchkey->ctrl_src_id);
            memcpy(matchkey->mti,DEF_VI_MTI_0800_REQ, sizeof(matchkey->mti));
            memcpy(matchkey->local_tran_time, \
                ffd_0810->b37.ffd_data, \
                ffd_0810->b37.ffd_header.m_fixvalue_length);
            memcpy(matchkey->function_code, \
                ffd_0810->b70.ffd_data, \
                ffd_0810->b70.ffd_header.m_fixvalue_length);
            *matkeylen = (short) DEF_MAX_MATCHINGKEY;
            return DEF_MKM_NORMAL;
        }
    }

    return DEF_MKM_ERROR;
}

/******************************************************************************
**
**  BCD型会員番号の文字型数値へ変換
**
**  FUNCTION         : BCDTOASCII()
**  CALLING SEQ.     : static void BCDTOASCII(int,char *,char *)
**
**  ARGUMENT         : 1. len       (I)    変換元BCDデータレングス
**                     2  *mbbcd_p  (I)    変換元BCD型格納領域ポインタ
**                     3. *mbstr_p  (I/O)  返還後文字型数値変換値格納域ポインタ
**
**  RETURN CODE      : NONE
**
**  DESCRIPTION      :
**  -----------------------------------------------------------------------
**  COMMENT          :
******************************************************************************/
static void BCDTOASCII(int len,unsigned char *mbbcd_p,char *mbstr_p)
{
    int        cnt;                                   // ループカウンタ
    int        set;                                   // データ設定位置バイト

    set = 0;
    for (cnt=0;cnt < len; cnt++) {
        *(mbstr_p + set) = (char)((*(mbbcd_p + cnt) >> 4) + 0x30);   // 上位4bit
        set++;
        *(mbstr_p + set) = (char)((*(mbbcd_p + cnt) & 0x0f) + 0x30); // 下位4bit
        set++;
    }
    return;
}
/* end of NWM_MKM */

