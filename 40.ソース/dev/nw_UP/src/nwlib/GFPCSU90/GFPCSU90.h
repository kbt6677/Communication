/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSU90                                    */
/*        FUNCTION          ････ NW個別(カットオーバー個別処理[UnionPay])    */
/*                               カットオーバー個別処理[UnionPay]を行う。    */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-09-18                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*                                                                           */
/*****************************************************************************/
#ifndef _GFPCSU90_H
#define _GFPCSU90_H

/* MTI */
//#define DEF_NWM_CTO_UNIONPAY_CTLMSG_REQ "0820"
//#define DEF_NWM_CTO_UNIONPAY_CTLMSG_RES "0830"

/* アクションコード */
#define DEF_NWM_CTO_ACT_INSP_OK         "00"

#define DEF_NWM_CTO_MTI_LEN             4
#define DEF_NWM_CTO_BIT_007_LENG        10              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_011_LENG         6              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_039_LENG         2              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_070_LENG         3              /* BITごとデータ長      */

#define DEF_NWM_CTO_OK                  0
#define DEF_NWM_CTO_NULL                0
#define DEF_NWM_CTO_ZERO                '0'

#define DEF_CHK_ERR_HDR_LEN            "Header Length"
#define DEF_CHK_ERR_TOT_LEN            "Total-Legnth"
#define DEF_CHK_ERR_MODE_FLG           "Header Flag"
#define DEF_CHK_ERR_MTI                "MTI"
#define DEF_CHK_ERR_BIT_007            "BIT007"        /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_011            "BIT011"        /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_033            "BIT033"        /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_053            "BIT053"        /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_070            "BIT070"        /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_100            "BIT100"        /* エラー発生BIT        */

/* 要求電文から応答電文へのデータコピー処理用マクロ */
#define NWM_CTO_MCR_CPY_0830(dst,src) \
{\
    if(src.ffd_header.m_flg_exist != 0){\
        memcpy(&dst, &src, sizeof(dst));\
    }\
}

/*----------------------------------------------------------------------------*/
/* 非公開モジュール                                                           */
/*----------------------------------------------------------------------------*/

#endif
