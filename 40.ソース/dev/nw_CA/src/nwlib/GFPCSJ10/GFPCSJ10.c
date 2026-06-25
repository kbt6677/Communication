/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJ10                                    */
/*        FUNCTION          ････ NW個別モジュール                            */
/*                               暗号化・復号処理(CARDNET)                   */
/*                                 指定された暗号化・復号区分をもとに電文の  */
/*                                 暗号化・復号を行う                        */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････ EMS出力モジュール                           */
/*                               IOモジュール                                */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*  1.1  T.Fukunaga 2025/04/22 認証値チェック不正対応（不具合No.57)          */
/*  1.2  S.Kimura   2025/06/22 初期化処理(NWM_ENI)分離                       */
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
#include "file.h"
#include "msg_CA.h"
#include "NWM_ENC.h"
#include "GFPCGX40.h"
#include "GFPCGXB0.h"
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
//#define  DEF_NWM_ENC_MTI_SEIGYO_REQ      "1804"          /* MTI                       */
//#define  DEF_NWM_ENC_MTI_SEIGYO_RSP      "1814"          /* MTI                       */
//#define  DEF_NWM_ENC_MTI_SYOGAI_NOTICE   "1644"          /* MTI                       */
#define  DEF_NWM_ENC_KEY_KC              "KC  "          /* GCKEY KC                  */
#define  DEF_NWM_ENC_KEY_KMAC            "KMAC"          /* GCKEY KMAC                */
#define  DEF_NWM_ENC_HEAD_LEN            80              /* MSG HEADER LEN            */
#define  DEF_NWM_ENC_MAC_CREATE_OK       "A8"            /* ATALLA RESPONSE           */
#define  DEF_NWM_ENC_ENCDEC_OK           "A7"            /* ATALLA RESPONSE           */
#define  DEF_NWM_ENC_MAC_CHECK_OK        "A9"            /* ATALLA RESPONNSE          */
#define  DEF_NWM_ENC_MAC_VERIFIED        'Y'             /* ATALLA RESPONSE           */
//#define  DEF_NWM_ENC_MSG_TYPE_TYPE_REQ   "C804"          /* MSG TYPE REQUEST          */
//#define  DEF_NWM_ENC_MSG_TYPE_TYPE_RSP   "C814"          /* MSG TYPE RESPONSE         */
#define  DEF_NWM_ENC_TBL_MAX_CNT         20              /* ATALLA RESPONSE TBL COUNT */
#define  DEF_NWM_ENC_RECV_MAX_LEN        5000            /* PATHSEND MAX RECV LEN     */
#define  DEF_NWM_ENC_FIELD_MAC_CREATE    4               /* ATALLA RESPONSE FIELD CNT */
#define  DEF_NWM_ENC_FIELD_ENCDEC        9               /* ATALLA RESPONSE FIELD CNT */
#define  DEF_NWM_ENC_FIELD_MAC_CHECK     4               /* ATALLA RESPONSE FIELD CNT */
typedef struct _NWM_ENC_atalla_rsp
{
    struct {
        char   *dt;
        short  len;
    } fields[DEF_NWM_ENC_TBL_MAX_CNT];
} NWM_ENC_atalla_rsp_def;

/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/
short NWM_ENC_enc(NWM_ENC_arg_2_def *
                , NWM_ENC_arg_3_def *
                , NWM_ENC_arg_4_def *
                , NWM_ENC_arg_5_def *
                , char *);
short NWM_ENC_dec(NWM_ENC_arg_2_def *
                , NWM_ENC_arg_3_def *
                , NWM_ENC_arg_4_def *
                , NWM_ENC_arg_5_def *
                , char *);
short NWM_ENC_gckey_read(char *
                       , NWM_ENC_arg_2_def *
                       , NWM_ENC_arg_4_def *
                       , COM_IOM_arg_5_def *
                       , COM_IOM_arg_6_def *
                       , char * );
short NWM_ENC_pathsend(COM_PSD_arg_1_def *
                      ,NWM_ENC_arg_3_def *
                      ,short
                      ,char *);
short NWM_ENC_atalla_rsp_get(char *
                            ,short
                            ,NWM_ENC_atalla_rsp_def *);

short NWM_ENC_CHAR2BCD(unsigned char *, short *);
short NWM_ENC_CHAR2HEX(const char    *, char  *, short);

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
/*  RETURN CODE     : 0  :正常(暗号化・復号実施）                           */
/*                  : 1  :正常(暗号化・復号実施対象外)                      */
/*                  : 2  :異常(認証値チェックエラー)                        */
/*                  : 3  :異常(チェックディジットエラー)                    */
/*                  : 4  :異常(ATALLAレスポンスエラー)                      */
/*                  : 5  :異常(PATHSENDエラー／ファイルIOエラー)            */
/*                  : 6  :異常(パラメターエラー)                            */
/*                  : 7  :異常(GCKEY KMAC エラー)                           */
/*                  : 8  :異常(GCKEY KC エラー)                             */
/*                  : 9  :異常(チェックディジットエラー(KMAC))              */
/*                  : 10 :異常(チェックディジットエラー(KC))                */
/*  DESCRIPTION     : 暗号化・復号処理を行う                                */
/****************************************************************************/
short NWM_ENC(short enc_dec_type
            , NWM_ENC_arg_2_def *key_file_info
            , NWM_ENC_arg_3_def *atalla_srv_info
            , NWM_ENC_arg_4_def *network_info
            , NWM_ENC_arg_5_def *msg_info
            , char *prog_id)
{
    short err = 0;
    
    switch(enc_dec_type){
    case DEF_NWM_ENC_ARG1_ENC:
        err = NWM_ENC_enc(key_file_info
                        , atalla_srv_info
                        , network_info
                        , msg_info
                        , prog_id );
        break;
    case DEF_NWM_ENC_ARG1_DEC:
        err = NWM_ENC_dec(key_file_info
                        , atalla_srv_info
                        , network_info
                        , msg_info
                        , prog_id );
        break;
    default:
        err = DEF_NWM_ENC_RTN_NG_PARAM;
    }
    
    return(err);
} /* end of NWM_ENC */

/****************************************************************************/
/*  FUNCTION        : 2.1.0  NWM_ENC_enc                                    */
/*  CALLING SEQ.    : short NWM_ENC_enc(short, strcut *, struct *, struct * */
/*                                , struct * ,char *                        */
/*  ARGUMEN         : 1.key_file_info  (I)   鍵管理ファイル情報             */
/*                  : 2.atalla_srv_info(I)   ATALLA振分サーバ情報           */
/*                  : 3.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 4.msg_info       (I/O) 電文情報                       */
/*                  : 5.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常(暗号化・復号実施）                            */
/*                  : 1 :正常(暗号化・復号実施対象外)                       */
/*                  : 4 :異常(ATALLAレスポンスエラー)                       */
/*                  : 5 :異常(PATHSENDエラー／ファイルIOエラー)             */
/*                  : 6  :異常(パラメターエラー)                            */
/*                  : 7  :異常(GCKEY KMAC エラー)                           */
/*                  : 8  :異常(GCKEY KC エラー)                             */
/*  DESCRIPTION     : 暗号化を行う                                          */
/****************************************************************************/
short NWM_ENC_enc(NWM_ENC_arg_2_def *key_file_info
                , NWM_ENC_arg_3_def *atalla_srv_info
                , NWM_ENC_arg_4_def *network_info
                , NWM_ENC_arg_5_def *msg_info
                , char *prog_id)
{
    short  err = 0;
    COM_IOM_arg_5_def in_fl_inf;             /* IOモジュール引数 */
    COM_IOM_arg_6_def out_fl_inf;            /* IOモジュール引数 */
    COM_PSD_arg_1_def snd_req;               /* PATHSENDモジュール引数 */
    MSG_HEADER_CARDNET_def *before_msg_ptr;  /* 編集用CARDNETヘッダーBEFORE */
    MSG_HEADER_CARDNET_def *after_msg_ptr;   /* 編集用CARDNETヘッダーAFTER  */
    db_gckey_def      *gckey_o;              /* 鍵管理ファイルREADバッファ */
    NWM_ENC_atalla_rsp_def atalla_rsp;       /* ATALLAレスポンス解析用     */
    
    char   key_value[74+1];                  /* ATALLAコマンド用キー値     */
    short  cmd_len;                          /* ATALLAコマンド長           */
    short  data_msg_len = 0;                 /* ATALLAコマンド用電文データ長 */
    char   wk_mac_bcd[8];                    /* 作業用BCD認証値            */
    long   wk_mac_bin;                       /* 作業用Binary認証値         */
    char   wk_check_digit[16];               /* 作業用チェックディジット   */
    char   check_digit_kmac[2];              /* チェックディジット KMAC    */
    char   check_digit_kc[2];                /* チェックディジット KC      */
    short  field_cnt;
    char   wk_char[4+1];
    short  wk_ctrl_msg_len;
    
    /* 初期化 */
    gckey_o = (db_gckey_def *)&out_fl_inf.rec_area[0];
    before_msg_ptr = (MSG_HEADER_CARDNET_def *)&msg_info->before_msg[0];
    after_msg_ptr  = (MSG_HEADER_CARDNET_def *)&msg_info->after_msg[0];
    /* 暗号化対象電文の判定 */
    if(memcmp(&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,DEF_CA_MTI_1804_REQ, strlen(DEF_CA_MTI_1804_REQ)) == 0 ||
       memcmp(&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,DEF_CA_MTI_1814_RSP, strlen(DEF_CA_MTI_1814_RSP)) == 0 ||
       memcmp(&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,DEF_CA_MTI_1644_NTC, strlen(DEF_CA_MTI_1644_NTC)) == 0 ) {
        memcpy(msg_info->after_msg ,msg_info->before_msg ,msg_info->before_len);
        msg_info->after_len = msg_info->before_len;

        /******************************************************/
        /* 文字列→BCD変換モジュールを使って 全体電文長を設定 */
        /******************************************************/
        wk_ctrl_msg_len = msg_info->after_len;
        sprintf(wk_char,"%04d",wk_ctrl_msg_len);
        NWM_ENC_CHAR2BCD(wk_char,(short*)&after_msg_ptr->ctrl_msg_len[0]);

        return(DEF_NWM_ENC_RTN_OK_NONE);
    }
    /* 鍵管理ファイルよりKMAC情報レコードの取得 */
    err = NWM_ENC_gckey_read(DEF_NWM_ENC_KEY_KMAC
                            ,key_file_info
                            ,network_info
                            ,&in_fl_inf
                            ,&out_fl_inf
                            ,prog_id);
    if(err != 0){
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
    /* ATALLAコマンド（認証値作成）の編集 */
       memset(key_value ,0x00 ,sizeof(key_value));
       memset(wk_check_digit ,0x00 ,sizeof(wk_check_digit));
    if(memcmp(gckey_o->key_info.new_key_index ,"01",
            sizeof(gckey_o->key_info.new_key_index)) == 0 ) {
        memcpy(key_value ,gckey_o->key_info.key_info_01.key_value 
            ,sizeof(gckey_o->key_info.key_info_01.key_value));
        memcpy(wk_check_digit ,gckey_o->key_info.key_info_01.check_digit 
            ,sizeof(gckey_o->key_info.key_info_01.check_digit));
    } else {
        memcpy(key_value ,gckey_o->key_info.key_info_02.key_value 
            ,sizeof(gckey_o->key_info.key_info_01.key_value));
        memcpy(wk_check_digit ,gckey_o->key_info.key_info_02.check_digit
            ,sizeof(gckey_o->key_info.key_info_02.check_digit));
    }
    /***********************************************************************/
    /* HEX文字->Binary変換モジュール使って check_digitをcheck_digit_kmacへ */
    /***********************************************************************/
    err = NWM_ENC_CHAR2HEX(wk_check_digit,check_digit_kmac,4);
    if(err != 0) {
        return(DEF_NWM_ENC_RTN_NG_KMAC);
    }
    if (msg_info->before_len < DEF_NWM_ENC_HEAD_LEN){
        /* 対象電文長異常のためパラメータエラーとする */
        return(DEF_NWM_ENC_RTN_NG_PARAM);
    }

    data_msg_len = msg_info->before_len - DEF_NWM_ENC_HEAD_LEN;
    cmd_len = (short)sprintf(snd_req.msg_buf ,"<98#%s#1#1##B#%d#"
                            ,key_value ,data_msg_len);
    memcpy(&snd_req.msg_buf[cmd_len] ,&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,data_msg_len);
    cmd_len = cmd_len + data_msg_len;
    memcpy(&snd_req.msg_buf[cmd_len],"#>",2);
    cmd_len = cmd_len + 2;
    /* ATALLAコマンド（認証値作成）の発行 */
    err = NWM_ENC_pathsend(&snd_req ,atalla_srv_info,cmd_len,prog_id);
    if( err != 0 ){
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
    /* ATALLAレスポンスの取得 */
    snd_req.msg_buf[snd_req.receive_len] = 0x00;
    field_cnt = NWM_ENC_atalla_rsp_get(snd_req.msg_buf ,snd_req.receive_len ,&atalla_rsp);
    if(field_cnt < DEF_NWM_ENC_FIELD_MAC_CREATE ){
        return(DEF_NWM_ENC_RTN_NG_ATALLA);
    }
    /* ATALLAレスポンスの判定 */
    if((memcmp(atalla_rsp.fields[0].dt ,DEF_NWM_ENC_MAC_CREATE_OK ,2) != 0) ||
        (atalla_rsp.fields[0].len) != 2 ){
        return(DEF_NWM_ENC_RTN_NG_ATALLA);
    }
    if(atalla_rsp.fields[2].len != 9){
        return(DEF_NWM_ENC_RTN_NG_ATALLA);
    }
    /* 認証値のセーブ (5桁目のスペースを除く)  */
    memcpy(wk_mac_bcd      ,atalla_rsp.fields[2].dt    ,4);
    memcpy(&wk_mac_bcd[4] ,&atalla_rsp.fields[2].dt[5] ,4);
    /************************************************************/
    /* HEX文字→Binary変換モジュールを使って 認証値を変換       */
    /************************************************************/
    err = NWM_ENC_CHAR2HEX(wk_mac_bcd,(char*)&wk_mac_bin,8);
    /* 鍵管理ファイルよりKC情報レコードの取得 */
    err = NWM_ENC_gckey_read(DEF_NWM_ENC_KEY_KC
                            ,key_file_info
                            ,network_info
                            ,&in_fl_inf
                            ,&out_fl_inf
                            ,prog_id);
    if(err != 0){
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
    /* ATALLAコマンド（暗号化）の編集 */
       memset(key_value ,0x00 ,sizeof(key_value));
       memset(wk_check_digit ,0x00 ,sizeof(wk_check_digit));
    if(memcmp(gckey_o->key_info.new_key_index ,"01",
            sizeof(gckey_o->key_info.new_key_index)) == 0 ) {
        memcpy(key_value ,gckey_o->key_info.key_info_01.key_value 
            ,sizeof(gckey_o->key_info.key_info_01.key_value));
        memcpy(wk_check_digit ,gckey_o->key_info.key_info_01.check_digit
            ,sizeof(gckey_o->key_info.key_info_01.check_digit));
    } else {
        memcpy(key_value ,gckey_o->key_info.key_info_02.key_value 
            ,sizeof(gckey_o->key_info.key_info_02.key_value));
        memcpy(wk_check_digit ,gckey_o->key_info.key_info_02.check_digit
            ,sizeof(gckey_o->key_info.key_info_02.check_digit));
    }
    /**********************************************************************/
    /* HEX文字->Binar変換モジュールを使って check_digitをcheck_digit_kcへ */
    /**********************************************************************/
    NWM_ENC_CHAR2HEX(wk_check_digit,check_digit_kc,4);
    if(err != 0) {
        return(DEF_NWM_ENC_RTN_NG_KC);
    }
    data_msg_len = msg_info->before_len - DEF_NWM_ENC_HEAD_LEN;
    cmd_len = (short)sprintf(snd_req.msg_buf ,"<97#E#1#%s#D#B#%d#"
                            ,key_value ,data_msg_len);
    memcpy(&snd_req.msg_buf[cmd_len] ,&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN],data_msg_len);
    cmd_len = cmd_len + data_msg_len;
    memcpy(&snd_req.msg_buf[cmd_len],"#>",2);
    cmd_len = cmd_len + 2;

    /* ATALLAコマンド（暗号化）の発行 */
    err = NWM_ENC_pathsend(&snd_req ,atalla_srv_info ,cmd_len ,prog_id);
    if( err != 0 ){
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
    /* ATALLAレスポンスの取得 */
    snd_req.msg_buf[snd_req.receive_len] = 0x00;
    field_cnt = NWM_ENC_atalla_rsp_get(snd_req.msg_buf, snd_req.receive_len, &atalla_rsp);
    if(field_cnt < DEF_NWM_ENC_FIELD_ENCDEC ){
        return(DEF_NWM_ENC_RTN_NG_ATALLA);
    }
    /* ATALLAレスポンスコードの判定 */
    if((memcmp(atalla_rsp.fields[0].dt ,DEF_NWM_ENC_ENCDEC_OK ,2) != 0) ||
        (atalla_rsp.fields[0].len != 2)){
        return(DEF_NWM_ENC_RTN_NG_ATALLA);
    }
    /* 引数．電文情報（前）の編集 */
    memcpy(before_msg_ptr->bh_auth_val ,(char *)&wk_mac_bin
        ,sizeof(before_msg_ptr->bh_auth_val));                 /* 電文認証値 */
    memcpy(before_msg_ptr->bh_chk_digit.bh_chk_digit_kc ,check_digit_kc
        ,sizeof(before_msg_ptr->bh_chk_digit.bh_chk_digit_kc));/* KCチェックディジット */
    memcpy(before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac ,check_digit_kmac
        ,sizeof(before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac));/* KMACチェックディジット */
    /* 引数．電文情報（後）の設定 */
    memcpy(msg_info->after_msg ,msg_info->before_msg ,DEF_NWM_ENC_HEAD_LEN); /* ヘッダー部 */
    /******************************************************/
    /* 文字列→BCD変換モジュールを使って 全体電文長を設定 */
    /******************************************************/
    wk_ctrl_msg_len = (short)atoi(atalla_rsp.fields[7].dt);
    wk_ctrl_msg_len = DEF_NWM_ENC_HEAD_LEN + wk_ctrl_msg_len;
    sprintf(wk_char,"%04d",wk_ctrl_msg_len);
    NWM_ENC_CHAR2BCD(wk_char,(short*)&after_msg_ptr->ctrl_msg_len[0]);
    memcpy(after_msg_ptr->bh_auth_val ,(char*)&wk_mac_bin
        ,sizeof(after_msg_ptr->bh_auth_val));                  /* 電文認証値 */
    memcpy(after_msg_ptr->bh_chk_digit.bh_chk_digit_kc ,check_digit_kc
        ,sizeof(after_msg_ptr->bh_chk_digit.bh_chk_digit_kc)); /* KCチェックディジット */
    memcpy(after_msg_ptr->bh_chk_digit.bh_chk_digit_kmac ,check_digit_kmac
        ,sizeof(after_msg_ptr->bh_chk_digit.bh_chk_digit_kmac));/* KMACチェックディジット */
    memcpy(&msg_info->after_msg[DEF_NWM_ENC_HEAD_LEN] ,atalla_rsp.fields[8].dt ,atalla_rsp.fields[8].len);
                                                               /* データ部 */
    msg_info->after_len = DEF_NWM_ENC_HEAD_LEN + atalla_rsp.fields[8].len;
    return(DEF_NWM_ENC_RTN_OK);

} /* end of NWM_ENC_enc */
/****************************************************************************/
/*  FUNCTION        : 2.2.0  NWM_ENC_dec                                    */
/*  CALLING SEQ.    : short NWM_ENC_dec(short, strcut *, struct *, struct * */
/*                                , struct * ,char *                        */
/*  ARGUMEN         : 1.key_file_info  (I)   鍵管理ファイル情報             */
/*                  : 2.atalla_srv_info(I)   PATHSEND結果情報               */
/*                  : 3.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 4.msg_info       (I/O) 電文情報                       */
/*                  : 5.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0  :正常(暗号化・復号実施）                           */
/*                  : 1  :正常(暗号化・復号実施対象外)                      */
/*                  : 2  :異常(認証値チェックエラー)                        */
/*                  : 3  :異常(チェックディジットエラー)                    */
/*                  : 4  :異常(ATALLAレスポンスエラー)                      */
/*                  : 5  :異常(PATHSENDエラー／ファイルIOエラー)            */
/*                  : 9  :異常(チェックディジットエラー(KMAC))              */
/*                  : 10 :異常(チェックディジットエラー(KC))                */
/*  DESCRIPTION     : 復号を行う                                            */
/****************************************************************************/
short NWM_ENC_dec(NWM_ENC_arg_2_def *key_file_info
                , NWM_ENC_arg_3_def *atalla_srv_info
                , NWM_ENC_arg_4_def *network_info
                , NWM_ENC_arg_5_def *msg_info
                , char *prog_id)
{
    short  err = 0;
    COM_IOM_arg_5_def in_fl_inf;             /* IOモジュール引数 */
    COM_IOM_arg_6_def out_fl_inf;            /* IOモジュール引数 */
    COM_PSD_arg_1_def snd_req;               /* PATHSENDモジュール引数 */
    MSG_HEADER_CARDNET_def *before_msg_ptr;  /* 編集用CARDNETヘッダーBEFORE */
    MSG_HEADER_CARDNET_def *after_msg_ptr;   /* 編集用CARDNETヘッダーAFTER  */
    db_gckey_def      *gckey_o;              /* 鍵管理ファイルREADバッファ */
    NWM_ENC_atalla_rsp_def atalla_rsp;       /* ATALLAレスポンス解析用     */

    char   key_value[74+1];                  /* ATALLAコマンド用キー値     */
    short  cmd_len;                          /* ATALLAコマンド長           */
    short  auth_len;                         /* ATALLAコマンド長           */
    short  data_msg_len = 0;                 /* ATALLAコマンド用電文データ長 */
    char   check_digit_kmac[4+1];              /* チェックディジット KMAC    */
    char   check_digit_kc[4+1];                /* チェックディジット KC      */
    short  field_cnt;
    char   dec_data[4096];
    short  dec_len;
    unsigned short  *wk_short;
    unsigned short  *wk_auth_val1;
    unsigned short  *wk_auth_val2;
    char   wk_char[4+1];
    short  wk_ctrl_msg_len;
    char   null_check_digit[2] = {0x00,0x00};
    
    /* 初期化 */
    gckey_o = (db_gckey_def *)&out_fl_inf.rec_area[0];
    before_msg_ptr = (MSG_HEADER_CARDNET_def *)&msg_info->before_msg[0];
    after_msg_ptr  = (MSG_HEADER_CARDNET_def *)&msg_info->after_msg[0];
    /* 復号・認証チェック対象電文の判定(チェックディジット) */
    if((memcmp(before_msg_ptr->bh_chk_digit.bh_chk_digit_kc  ,null_check_digit,2)  == 0) &&
       (memcmp(before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac,null_check_digit,2)  == 0)){
        memcpy(msg_info->after_msg ,msg_info->before_msg ,msg_info->before_len);
           msg_info->after_len = msg_info->before_len;
        return(DEF_NWM_ENC_RTN_OK_NONE);
    }
    /* 復号・認証チェック対象電文の判定(電文種別コード) */
    if(memcmp(before_msg_ptr->bh_msg_type ,DEF_CA_APHD_MSGCODE_C804_REQ, strlen(DEF_CA_APHD_MSGCODE_C804_REQ)) == 0 ||
       memcmp(before_msg_ptr->bh_msg_type ,DEF_CA_APHD_MSGCODE_C814_RSP, strlen(DEF_CA_APHD_MSGCODE_C814_RSP)) == 0 ) {
        memcpy(msg_info->after_msg ,msg_info->before_msg ,msg_info->before_len);
           msg_info->after_len = msg_info->before_len;
        return(DEF_NWM_ENC_RTN_OK_NONE);
    }
    /* 復号対象電文の判定 */
    /* チェックディジット（暗号化キー）が0の場合は復号は実施不要 */
    if((memcmp(before_msg_ptr->bh_chk_digit.bh_chk_digit_kc  ,null_check_digit,2)  != 0)){
        /* 復号処理を実施する */
        /* 鍵管理ファイルよりKC情報レコードの取得 */
        err = NWM_ENC_gckey_read(DEF_NWM_ENC_KEY_KC
                                ,key_file_info
                                ,network_info
                                ,&in_fl_inf
                                ,&out_fl_inf
                                ,prog_id);
        if(err != 0){
            return(DEF_NWM_ENC_RTN_NG_IO);
        }
        /* 変換前電文中のKCチェックディジットとKC情報レコードのチェックディジットのチェック */
        wk_short = (unsigned short *)&before_msg_ptr->bh_chk_digit.bh_chk_digit_kc[0];
        sprintf(check_digit_kc,"%04X",*wk_short);
        memset(key_value ,0x00 ,sizeof(key_value));
        if(memcmp(check_digit_kc,gckey_o->key_info.key_info_01.check_digit ,4) == 0) {
            memcpy(key_value ,gckey_o->key_info.key_info_01.key_value 
                ,sizeof(gckey_o->key_info.key_info_01.key_value));
        } else {
            if(memcmp(check_digit_kc,gckey_o->key_info.key_info_02.check_digit ,4) == 0) {
                memcpy(key_value ,gckey_o->key_info.key_info_02.key_value 
                    ,sizeof(gckey_o->key_info.key_info_02.key_value));
            } else {
                return(DEF_NWM_ENC_RTN_NG_DIGITS_KC);
            }
        }
        /* ATALLAコマンド（復号）の編集 */
        data_msg_len = msg_info->before_len - DEF_NWM_ENC_HEAD_LEN;
        cmd_len = (short)sprintf(snd_req.msg_buf ,"<97#D#1#%s#D#B#%d#"
                                ,key_value ,data_msg_len);
        memcpy(&snd_req.msg_buf[cmd_len] ,&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,data_msg_len);
        cmd_len = cmd_len + data_msg_len;
        memcpy(&snd_req.msg_buf[cmd_len],"#>",2);
        cmd_len = cmd_len + 2;
        /* ATALLAコマンド（復号）の発行 */
        err = NWM_ENC_pathsend(&snd_req ,atalla_srv_info ,cmd_len ,prog_id);
        if( err != 0 ){
            return(DEF_NWM_ENC_RTN_NG_IO);
        }
        /* ATALLAレスポンスの取得 */
        snd_req.msg_buf[snd_req.receive_len] = 0x00;
        field_cnt = NWM_ENC_atalla_rsp_get(snd_req.msg_buf, snd_req.receive_len, &atalla_rsp);
        if(field_cnt < DEF_NWM_ENC_FIELD_ENCDEC ){
            return(DEF_NWM_ENC_RTN_NG_ATALLA);
        }
        /* ATALLAレスポンスの判定 */
        if((memcmp(atalla_rsp.fields[0].dt ,DEF_NWM_ENC_ENCDEC_OK ,2) != 0) ||
            (atalla_rsp.fields[0].len != 2)){
            return(DEF_NWM_ENC_RTN_NG_ATALLA);
        }
        memset(dec_data,0x00,sizeof(dec_data));
        dec_len = atalla_rsp.fields[8].len;
        memcpy(dec_data,atalla_rsp.fields[8].dt,dec_len);
    }else{
        /* 復号処理は実施なし */
        memset(dec_data,0x00,sizeof(dec_data));
        dec_len = msg_info->before_len - DEF_NWM_ENC_HEAD_LEN;
        memcpy(dec_data,&msg_info->before_msg[DEF_NWM_ENC_HEAD_LEN] ,dec_len);
    }
    
    /* 認証チェック対象の判定 */
    /* チェックディジット（認証キー）が0の場合は認証チェック実施不要 */
    if((memcmp(before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac  ,null_check_digit,2)  != 0)){
        /* 認証チェック実施あり */
        /* 鍵管理ファイルよりKMAC情報レコードの取得 */
        err = NWM_ENC_gckey_read(DEF_NWM_ENC_KEY_KMAC
                                ,key_file_info
                                ,network_info
                                ,&in_fl_inf
                                ,&out_fl_inf
                                ,prog_id);
        if(err != 0){
            return(DEF_NWM_ENC_RTN_NG_IO);
        }
        /* KMACチェックディジットのチェック */
        /* 変換前電文中のKCチェックディジットとKMAC情報レコードのチェックディジットのチェック */
        wk_short = (unsigned short *)&before_msg_ptr->bh_chk_digit.bh_chk_digit_kmac[0];
        sprintf(check_digit_kmac,"%04X",*wk_short);
        memset(key_value ,0x00 ,sizeof(key_value));
        if(memcmp(check_digit_kmac,gckey_o->key_info.key_info_01.check_digit ,4) == 0) {
            memcpy(key_value ,gckey_o->key_info.key_info_01.key_value 
                ,sizeof(gckey_o->key_info.key_info_01.key_value));
        } else {
            if(memcmp(check_digit_kmac,gckey_o->key_info.key_info_02.check_digit ,4) == 0) {
                memcpy(key_value ,gckey_o->key_info.key_info_02.key_value 
                    ,sizeof(gckey_o->key_info.key_info_02.key_value));
            } else {
                return(DEF_NWM_ENC_RTN_NG_DIGITS_KMAC);
            }
        }
        wk_auth_val1 = (unsigned short *)&before_msg_ptr->bh_auth_val[0];
        wk_auth_val2 = (unsigned short *)&before_msg_ptr->bh_auth_val[2];
        /* ATALLAコマンド（認証値チェック）の編集 */
        cmd_len = (short)sprintf(snd_req.msg_buf ,"<99#%s#1##1#B#%d#"
                                ,key_value,dec_len);
        memcpy(&snd_req.msg_buf[cmd_len] ,dec_data,dec_len);
        cmd_len = cmd_len + dec_len;
        auth_len = (short)sprintf(&snd_req.msg_buf[cmd_len] ,"#%04X %04X#>"
                                ,*wk_auth_val1,*wk_auth_val2);
        cmd_len = cmd_len + auth_len;
        /* ATALLAコマンド（認証値チェック）の発行 */
        err = NWM_ENC_pathsend(&snd_req ,atalla_srv_info ,cmd_len ,prog_id);
        if( err != 0 ){
            return(DEF_NWM_ENC_RTN_NG_IO);
        }
        /* ATALLAレスポンスの取得 */
        snd_req.msg_buf[snd_req.receive_len] = 0x00;
        field_cnt = NWM_ENC_atalla_rsp_get(snd_req.msg_buf, snd_req.receive_len, &atalla_rsp);
        if(field_cnt < DEF_NWM_ENC_FIELD_MAC_CHECK ){
            return(DEF_NWM_ENC_RTN_NG_ATALLA);
        }
        /* ATALLAレスポンスの判定 */
        if((memcmp(atalla_rsp.fields[0].dt ,DEF_NWM_ENC_MAC_CHECK_OK ,2) != 0) ||
            (atalla_rsp.fields[0].len != 2)){
            return(DEF_NWM_ENC_RTN_NG_ATALLA);
        }
        /* 認証値チェック結果の判定 */
        if((atalla_rsp.fields[2].dt[0] != DEF_NWM_ENC_MAC_VERIFIED ) ||
            (atalla_rsp.fields[2].len != 1)){
            return(DEF_NWM_ENC_RTN_NG_AUTHORI);
        }
    }
    /* 引数．電文情報（後）のヘッダー部設定 */
    memcpy(msg_info->after_msg ,msg_info->before_msg ,DEF_NWM_ENC_HEAD_LEN);
    /* 文字列→BCD変換モジュールを使って 全体電文長を設定 */
    wk_ctrl_msg_len = DEF_NWM_ENC_HEAD_LEN + dec_len;
    sprintf(wk_char,"%04d",wk_ctrl_msg_len);
    NWM_ENC_CHAR2BCD(wk_char,(short*)&after_msg_ptr->ctrl_msg_len[0]);
    /* 引数．電文情報（後）のデータ部設定 */
    memcpy(&msg_info->after_msg[DEF_NWM_ENC_HEAD_LEN] ,dec_data ,dec_len);
    /* 引数．電文情報（後）の電文長設定 */
    msg_info->after_len = DEF_NWM_ENC_HEAD_LEN + dec_len;
    
    return(DEF_NWM_ENC_RTN_OK);
} /* end of NWM_ENC_dec */
/****************************************************************************/
/*  FUNCTION        : 2.3.1  NWM_ENC_gckey_read                             */
/*  CALLING SEQ.    : short NWM_ENC_gckey_read  (char *                     */
/*                                              ,strcut *                   */
/*                                              ,struct *                   */
/*                                              ,struct *                   */
/*                                              ,struct *                   */
/*                                              ,char * )                   */
/*  ARGUMENT        : 1.key_type       (I)   KMAC  KC                       */
/*                  : 2.key_file_info  (I)   鍵管理ファイル情報             */
/*                  : 3.netwrok_info   (I)   ネットワーク特定情報           */
/*                  : 4.in_fl_inf      (I)   入力情報                       */
/*                  : 5.out_fl_inf     (O)   出力情報                       */
/*                  : 6.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : 5 :異常(PATHSENDエラー／ファイルIOエラー)             */
/*  DESCRIPTION     : 鍵管理ファイルの読み込みを行う                        */
/****************************************************************************/
short NWM_ENC_gckey_read(char *key_type
                       , NWM_ENC_arg_2_def *key_file_info
                       , NWM_ENC_arg_4_def *network_info
                       , COM_IOM_arg_5_def *in_fl_inf
                       , COM_IOM_arg_6_def *out_fl_inf
                       , char *prog_id )
{
    char   subprog_sts[2];
    COM_IOM_arg_3_def trace_inf;             /* IOモジュール引数 */
    COM_IOM_arg_4_def file_inf;              /* IOモジュール引数 */
    db_gckey_def      *gckey_i;              /* 鍵管理ファイルPRI-KEY      */

    /* 初期化 */
    gckey_i = (db_gckey_def *)&in_fl_inf->key_value[0];

    /* IOモジュール引数設定　トレース情報 */
    memset(trace_inf.prog_id      ,' ',sizeof(trace_inf.prog_id));
    memset(trace_inf.file_id      ,' ',sizeof(trace_inf.file_id));
    memset(trace_inf.file_name    ,' ',sizeof(trace_inf.file_name));
    memset(trace_inf.file_io_type ,' ',sizeof(trace_inf.file_io_type));
    memcpy(trace_inf.prog_id       ,prog_id ,sizeof(trace_inf.prog_id));
    memcpy(trace_inf.file_id       ,DEF_GCKEY, sizeof(DEF_GCKEY)-1);
    memcpy(trace_inf.file_name     ,key_file_info->file_name ,sizeof(trace_inf.file_name));
    memcpy(trace_inf.file_io_type  ,"READ" ,4);
    /* IOモジュール引数設定　ファイル情報 */
    memset(file_inf.file_id   ,' ',sizeof(file_inf.file_id));
    memset(file_inf.file_name ,' ',sizeof(file_inf.file_name));
    memcpy(file_inf.file_id ,DEF_GCKEY, sizeof(DEF_GCKEY)-1);
    memcpy(file_inf.file_name ,key_file_info->file_name
        ,sizeof(file_inf.file_name));
    file_inf.file_no = key_file_info->file_no;
    /* IOモジュール引数設定　入力情報 */
    in_fl_inf->part_key_type      = 0x00;
    in_fl_inf->part_key_position  = 0x00;
    in_fl_inf->part_key_len       = 0x00;
    gckey_i->pri_key.site_id  = network_info->site_id;
    gckey_i->pri_key.nw_id    = network_info->nw_id;
    memcpy(gckey_i->pri_key.grp_id ,network_info->grp_id ,sizeof(gckey_i->pri_key.grp_id));
    memcpy(gckey_i->pri_key.if_id ,network_info->if_id ,sizeof(gckey_i->pri_key.if_id));
    memcpy(gckey_i->pri_key.station_id ,network_info->station_id ,sizeof(gckey_i->pri_key.station_id));
    if(memcmp(key_type,DEF_NWM_ENC_KEY_KMAC,4) == 0) {
        memcpy(gckey_i->pri_key.key_type ,DEF_NWM_ENC_KEY_KMAC ,sizeof(DEF_NWM_ENC_KEY_KMAC));
    } else {
        memcpy(gckey_i->pri_key.key_type ,DEF_NWM_ENC_KEY_KC   ,sizeof(DEF_NWM_ENC_KEY_KC));
    }
    memcpy(in_fl_inf->key_type ,DEF_COM_IOM_KEYTYPE_PRI ,sizeof(in_fl_inf->key_type));
    in_fl_inf->key_len            = (short)sizeof(gckey_i->pri_key);
    in_fl_inf->compare_len        = (short)sizeof(gckey_i->pri_key);
    in_fl_inf->positioning_mode   = DEF_COM_IOM_EXACT;
    in_fl_inf->lock_flg           = DEF_COM_IOM_NOLOCK;
    in_fl_inf->asc_desc_type      = DEF_COM_IOM_ASCEND;
    in_fl_inf->io_timer           = key_file_info->io_timer;
    in_fl_inf->rec_len            = db_gckey_def_Size;
    /* IOモジュール呼び出し */
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD
                , subprog_sts
                , &trace_inf
                , &file_inf
                , in_fl_inf
                , out_fl_inf );

    /* 処理結果判定 */
    if(memcmp(subprog_sts ,DEF_COM_IOM_NO_ERR ,sizeof(subprog_sts)) == 0 ){
        return(DEF_NWM_ENC_RTN_OK);
    } else {
        /* EMS出力 */
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
} /* end of NWM_ENC_gckey_read */
/****************************************************************************/
/*  FUNCTION        : 2.3.2  NWM_ENC_pathsend                               */
/*  CALLING SEQ.    : short NWM_ENC_pathsend(struct *, strcut *, short      */
/*                                          ,char * )                       */
/*  ARGUMENT        : 1.snd_req        (I/O) PATHSEND情報                   */
/*                  : 2.atalla_srv_info(I)   ATALLA振分サーバ情報           */
/*                  : 3.cmd_len        (I)   コマンド長                     */
/*                  : 4.prog_id        (I)   モジュールID                   */
/*  RETURN CODE     : 0 :正常                                               */
/*                  : 5 :異常(PATHSENDエラー／ファイルIOエラー)             */
/*  DESCRIPTION     : ATALLAへのPATHSENDを行う                              */
/****************************************************************************/
short NWM_ENC_pathsend(COM_PSD_arg_1_def *snd_req
                      ,NWM_ENC_arg_3_def *atalla_srv_info
                      ,short cmd_len
                      ,char *prog_id)
{
    short  err = 0;
    COM_PSD_arg_2_def snd_trc;               /* PATHSENDモジュール引数 */
    COM_PSD_arg_3_def snd_res;               /* PATHSENDモジュール引数 */
    oggz1in_def       ems_inf;               /* EMS共通情報            */
    COM_PSD_arg_4_def ems_add;               /* EMS付加情報            */

    memcpy(snd_req->pathmon_name ,atalla_srv_info->domain_name ,sizeof(snd_req->pathmon_name));
    memcpy(snd_req->serverclass_name ,atalla_srv_info->server_name ,sizeof(snd_req->serverclass_name));
    snd_req->req_send_len = cmd_len;
    snd_req->receive_max_len = DEF_NWM_ENC_RECV_MAX_LEN;
    snd_req->send_timer_msec = atalla_srv_info->pathsend_timer;
    snd_req->retry_cnt = atalla_srv_info->pathsend_retry_cnt;
    memcpy(snd_trc.prog_id,prog_id ,sizeof(snd_trc.prog_id));

    err = COM_PSD(snd_req ,&snd_trc ,&snd_res , &ems_inf, &ems_add);

    /* 処理結果判定 */
    if( err == 0 ){
        return(DEF_NWM_ENC_RTN_OK);
    } else {
        return(DEF_NWM_ENC_RTN_NG_IO);
    }
} /* end of NWM_ENC_pathsend */
/****************************************************************************/
/*  FUNCTION        : 2.3.3  NWM_ENC_atalla_rsp_get                         */
/*  CALLING SEQ.    : short NWM_ENC_atalla_rsp_get (char  *                 */
/*                                                 ,short                   */
/*                                                 ,struct *)               */
/*  ARGUMENT        : 1.msg_buf        (I)   ATALLAレスポンスデータ         */
/*                  : 2.msg_len        (I)   ATALLAレスポンスデータ長       */
/*                  : 3.atalla_rsp     (O)   ATALLAレスポンステーブル       */
/*  RETURN CODE     : field数                                               */
/*  DESCRIPTION     : ATALLAレスポンステーブルへの展開を行う                */
/****************************************************************************/
short NWM_ENC_atalla_rsp_get(char *msg_buf
                            ,short msg_len
                            ,NWM_ENC_atalla_rsp_def *atalla_rsp )
{
    short    i           = 0;
    char     *s_ptr;
    char     *e_ptr;
    short    wk_len;

    e_ptr = memchr(msg_buf,'<',msg_len);
    if(e_ptr == 0x00 ){
        return(i);
    }
    s_ptr = e_ptr;
    s_ptr++;
    for( i=0; i < DEF_NWM_ENC_TBL_MAX_CNT; i++){
        switch(i) {
        case 8:
            /* 暗号化・復号データの場合、データとデータ長をデータ長フィールドより取得 */
            atalla_rsp->fields[i].len = (short)atoi(atalla_rsp->fields[7].dt);
            atalla_rsp->fields[i].dt = s_ptr;
            e_ptr = s_ptr + atalla_rsp->fields[i].len;
            *e_ptr = 0x00;
            s_ptr = e_ptr;
            s_ptr++;
            break;
        default:
            /* その他フィールドの場合、"#"で区切られたデータを取得 */
            wk_len = msg_len - (short)(s_ptr - msg_buf);
            e_ptr = memchr(s_ptr,'#',wk_len);
            if( e_ptr != 0x00){
                atalla_rsp->fields[i].len = (short)(e_ptr - s_ptr);
                atalla_rsp->fields[i].dt = s_ptr;
                *e_ptr = 0x00;
                s_ptr = e_ptr;
                s_ptr++;
            } else {
                return(i);
            }
        }
    }
    return(i);
}
/****************************************************************************/
/*  FUNCTION        : 2.3.4  NWM_ENC_CHAR2BCD                               */
/*  CALLING SEQ.    : short NWM_ENC_CHAR2BCD       (unsigned char  *        */
/*                                                 ,short          *)       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.len            (O)   変換した値                     */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 10進文字->BCD変換を行う                               */
/****************************************************************************/
short NWM_ENC_CHAR2BCD(unsigned char *ascii_p, short *len)
{
    short sValue;

    for (sValue=0; sValue<4; sValue++) {
        if (ascii_p[sValue] < '0' || ascii_p[sValue] > '9') {
            *len = 0;
            return (1);
        }
    }
    sValue = (short)((ascii_p[0] & 0x0f) << 12 |
                     (ascii_p[1] & 0x0f) <<  8 |
                     (ascii_p[2] & 0x0f) <<  4 |
                     (ascii_p[3] & 0x0f));

   *len = sValue;
   return (0);
} /* end of NWM_ENC_CHAR2BCD */
/****************************************************************************/
/*  FUNCTION        : 2.3.5  NWM_ENC_CHAR2HEX                               */
/*  CALLING SEQ.    : short NWM_ENC_CHAR2HEX       (const char     *        */
/*                                                 ,char           *        */
/*                                                 ,short           )       */
/*  ARGUMENT        : 1.ascii_p        (I)   変換元バッファ                 */
/*                  : 2.hex_p          (O)   変換先バッファ                 */
/*                  : 3.s_len          (I)   変換長                         */
/*  RETURN CODE     : 0 :正常                                               */
/*                    1 :異常                                               */
/*  DESCRIPTION     : 16進文字->BINARY変換を行う                            */
/****************************************************************************/
short NWM_ENC_CHAR2HEX(const char *ascii_p, char *hex_p, short s_len)
{
    short var,base,half,iix,oix;

    base = (short)(s_len & 0x01);
    for (iix = 0,oix = 0,var = 0;iix < s_len;iix++) {

        if        (ascii_p[iix] >= '0' && ascii_p[iix] <= '9') {
           half = ascii_p[iix] - '0';
        } else if (ascii_p[iix] >= 'A' && ascii_p[iix] <= 'F') {
           half = ascii_p[iix] - 'A' + 10;
        } else if (ascii_p[iix] >= 'a' && ascii_p[iix] <= 'f') {
           half = ascii_p[iix] - 'a' + 10;
        } else {
          memset(hex_p,0x20,s_len/2);
          return (1);
        }
        if (base == 0) {
            var = half;
            base = 1;
        } else {
            var = (short)((var << 4) + half);
            hex_p[oix] = (char)var;
            oix++;
            base = 0;
        }
    }
    return (0);

} /* end of NWM_ENC_CHAR2HEX */
