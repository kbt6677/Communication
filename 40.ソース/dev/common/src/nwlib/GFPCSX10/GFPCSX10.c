/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSX10                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               暗号化・復号処理用初期処理(ダミー)          */
/*                                 ATALLA振分サーバ情報と鍵管理ファイル情報に*/
/*                                 ダミー値を設定                            */
/*                               暗号化・復号処理(ダミー)                    */
/*                                 暗号化/復号前電文をそのまま暗号化/復号後  */
/*                                 電文として返却する                        */
/*        AUTHER            ････ HAS S.Kimura                                */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-01-29                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  S.Kimura   2025/01/29 (J0680)新規作成                               */
/*  1.1  S.Kimura   2025/06/22 NWM_INIのパラメータ変更                       */
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
#include "common.h"
#include "ems.h"
#include "NWM_ENI.h"
#include "NWM_ENC.h"
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_ENI                                        */
/*  CALLING SEQ.    : short NWM_ENI(strcut *, struct *, struct *, struct *  */
/*                                , char *)                                 */
/*  ARGUMENT        : 1.physical_info  (I)   物理名情報ファイル情報         */
/*                  : 2.key_file_info  (I/O) 鍵管理ファイル情報             */
/*                  : 3.atalla_srv_info(O)   PATHSEND結果情報               */
/*                  : 4.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 5.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*  DESCRIPTION     : 暗号化・復号処理の初期化を行う                        */
/****************************************************************************/
short NWM_ENI(char node_type
            , NWM_ENI_arg_1_def *physical_info
            , NWM_ENI_arg_2_def *key_file_info
            , NWM_ENI_arg_2_def *key_file_info_other
            , NWM_ENI_arg_3_def *atalla_srv_info
            , NWM_ENI_arg_4_def *network_info
            , char *prog_id
            , oggz1in_def *ems_info
            , ems_info_add *ems_add)
{
    /* 引数． 鍵管理ファイル情報の設定 */
    memset(key_file_info->file_id, DEF_NWM_ENC_SPACE, sizeof(key_file_info->file_id));
    memset(key_file_info->file_name, DEF_NWM_ENC_SPACE, sizeof(key_file_info->file_name));
    key_file_info->file_no = DEF_NWM_ENI_FILENO_INIT;

    /* 引数． ATALLA振分サーバ情報の設定 */
    memset(atalla_srv_info->domain_name ,DEF_NWM_ENC_SPACE,
                         sizeof(atalla_srv_info->domain_name));
    memset(atalla_srv_info->server_name ,DEF_NWM_ENC_SPACE,
                         sizeof(atalla_srv_info->server_name));

    return(DEF_NWM_ENI_RTN_OK);
}
/* end of NWM_ENI */

/****************************************************************************/
/*  FUNCTION        : 2.0.0  NWM_ENC                                        */
/*  CALLING SEQ.    : short NWM_ENC(short, strcut *, struct *, struct *     */
/*                                , struct * ,char *                        */
/*  ARGUMENT        : 1.enc_dec_type   (I)   暗号化・復号区分               */
/*                  : 2.key_file_info  (I)   鍵管理ファイル情報             */
/*                  : 3.atalla_srv_info(I)   ATALLA振分サーバ情報           */
/*                  : 4.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 5.msg_info       (I/O) 電文情報                       */
/*                  : 6.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 1 :正常(暗号化・復号実施対象外)                       */
/*                  : 6 :異常(パラメターエラー)                             */
/*  DESCRIPTION     : 暗号化・復号処理を行う                                */
/****************************************************************************/
short NWM_ENC(short enc_dec_type
            , NWM_ENC_arg_2_def *key_file_info
            , NWM_ENC_arg_3_def *atalla_srv_info
            , NWM_ENC_arg_4_def *network_info
            , NWM_ENC_arg_5_def *msg_info
            , char *prog_id)
{
    if(enc_dec_type != DEF_NWM_ENC_ARG1_ENC &&
       enc_dec_type != DEF_NWM_ENC_ARG1_DEC ) {
        /* 6 :異常(パラメターエラー) 返却 */
        return(DEF_NWM_ENC_RTN_NG_PARAM);
    }

    /* 暗号化/復号後電文に暗号化/復号前電文を設定 */
    memcpy( msg_info->after_msg, msg_info->before_msg, msg_info->before_len);

    /* 暗号化/復号後電文長に暗号化/復号前電文長を設定 */
    msg_info->after_len = msg_info->before_len;

    return(DEF_NWM_ENC_RTN_OK_NONE);

} /* end of NWM_ENC */

