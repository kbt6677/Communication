/**
 * @brief カットオーバー電文精査・電文編集・・ヘッダファイル
 *
 * @date 2025/09/08 新規作成
*/
#ifndef _GFPCSN90_H
#define _GFPCSN90_H

// zsysc内ZSYS_VAL_LEN_PROCESSNAME未定義時のみINCLUDE
#include "zsysc" nolist

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"
#include "ipc.h"

#include "msg_NY.h"
#include "NWM_CTO.h"
#include "GFPCGX90.h"

/* MTI */
//#define DEF_NWM_CTO_NYCE_CTLMSG_REQ          "0800"
//#define DEF_NWM_CTO_NYCE_CTLMSG_RES          "0810"

/* アクションコード */

#define DEF_NWM_CTO_ACT_INSP_OK         "00"
#define DEF_NWM_CTO_CEN_STS_NG          "ECCK201"
#define DEF_NWM_CTO_MSG_FMT_NG          "ECCK200"

#define DEF_NWM_CTO_MTI_LEN             4
#define DEF_NWM_CTO_BIT_007_LENG        10              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_011_LENG         6              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_039_LENG         2              /* BITごとデータ長      */
#define DEF_NWM_CTO_BIT_070_LENG         3              /* BITごとデータ長      */

#define DEF_NWM_CTO_OK                  0
#define DEF_NWM_CTO_NULL                0
#define DEF_NWM_CTO_ZERO                '0'

#define DEF_CHK_ERR_MTI                 "MTI"
#define DEF_CHK_ERR_BIT_007             "BIT007"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_011             "BIT011"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_070             "BIT070"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_096             "BIT096"     /* エラー発生BIT        */
#define DEF_CHK_ERR_BIT_125             "BIT125"     /* エラー発生BIT        */

/* 要求電文から応答電文へのデータコピー処理用マクロ */
#define NWM_CTO_MCR_CPY_0810(dst,src) \
{\
    if(src.ffd_header.m_flg_exist != 0){\
        memcpy(&dst, &src, sizeof(dst));\
    }\
}

/*----------------------------------------------------------------------------*/
/* 非公開モジュール                                                           */
/*----------------------------------------------------------------------------*/

#endif
