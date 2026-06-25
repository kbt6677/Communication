/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/02/28＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCVX20                                    */
/*        FUNCTION          ････ コネクション制御(クライアント)              */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/02/28                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/02/28 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <ctype.h>    nolist
#include <stdbool.h>  nolist
#include <stdlib.h>   nolist
/* USER HEADER     */
#include <errcd.h> nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_util.h" nolist

/*
 * grp_id : "G"+連番(4桁) G0001～G9999
 * if_id  : "I"+連番(4桁) I0001～I9999
 * station_id : "S"+任意1文字 + 連番(4桁) => 先頭"S",続く1文字,続く4桁
 * connect_id : "CC"+連番(4桁)
 * tcpip_prc_name : '$'で開始、以降は英数字
 * ip_adress_src/dst : IPv4アドレス
 * port_num_src : スペース or [0..65535]
 * port_num_dst : [0..65535] (スペース不可の場合)
 * invalid_flg  : ' ' or '1'
 * update_date  : YYYYMMDD
 *
 * 違反したら:
 *   cncl_ems_param_err("パラメータ名",0,0)して-1リターン
 */
/*****************************************************************************/
/*  FUNCTION        :checkGrpId                                              */
/*  CALLING SEQ.    :static bool checkGrpId(const char *grp_id)              */
/*  ARGUMENT        :grp_id:チェック対象文字列(先頭'G'+4桁数字)              */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :'G'+4桁が1~9999の範囲内かを判定する                    */
/*****************************************************************************/
static bool checkGrpId(const char *grp_id)
{
    /* grp_id[0]=='G' && grp_id[1..4] が数字 "0001"～"9999" */
    if (grp_id[0] != 'G') return false;
    /* 4桁が "0001"～"9999" */
    /* 全て数字か? */
    for (int i = 1; i < 5; i++) {
        if (!isdigit((unsigned char)grp_id[i])) {
            return false;
        }
    }
    size_t r_len = 0;
    short  s_err = 0;
    int    val   = str2ul_c(&grp_id[1], 4, &r_len, &s_err);
    if (val < 1 || val > 9999 || s_err) return false;
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkIfId                                               */
/*  CALLING SEQ.    :static bool checkIfId(const char *if_id)                */
/*  ARGUMENT        :if_id:チェック対象文字列(先頭'I'+4桁数字)               */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :'I'+4桁が1~9999の範囲内かを判定する                    */
/*****************************************************************************/
static bool checkIfId(const char *if_id)
{
    if (if_id[0] != 'I') return false;
    for (int i = 1; i < 5; i++) {
        if (!isdigit((unsigned char)if_id[i])) {
            return false;
        }
    }
    size_t r_len = 0;
    short  s_err = 0;
    int    val   = str2ul_c(&if_id[1], 4, &r_len, &s_err);
    if (val < 1 || val > 9999 || s_err) return false;
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkStationId                                          */
/*  CALLING SEQ.    :static bool checkStationId(const char *station_id)      */
/*  ARGUMENT        :station_id:先頭'S',続く1文字,続く4桁連番の文字列        */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :ステーションID('S'+任意1+4桁数字)の形式チェック         */
/*****************************************************************************/
static bool checkStationId(const char *station_id)
{
    /* "S" + 任意1文字 + 4桁連番 */
    if (station_id[0] != 'S') return false;
    /* station_id[1] は任意文字(ASCII想定なら何でもOKとする) */
    /* station_id[2..5] が数字か? */
    for (int i = 2; i < 6; i++) {
        if (!isdigit((unsigned char)station_id[i])) {
            return false;
        }
    }
    size_t r_len = 0;
    short  s_err = 0;
    int    val   = str2ul_c(&station_id[2], 4, &r_len, &s_err);
    if (val < 1 || val > 9999 || s_err) return false;
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkConnectId                                          */
/*  CALLING SEQ.    :static bool checkConnectId(const char *connect_id)      */
/*  ARGUMENT        :connect_id:"CC"+4桁数字の文字列                         */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :"CC"+4桁が1~9999の範囲内かを判定する                   */
/*****************************************************************************/
static bool checkConnectId(const char *connect_id)
{
    /* "CC" + 4桁 */
    if (connect_id[0] != 'C' || connect_id[1] != 'C') return false;
    for (int i = 2; i < 6; i++) {
        if (!isdigit((unsigned char)connect_id[i])) {
            return false;
        }
    }
    size_t r_len = 0;
    short  s_err = 0;
    int    val   = str2ul_c(&connect_id[2], 4, &r_len, &s_err);
    if (val < 1 || val > 9999 || s_err) return false;
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :check_db_gflin_def                                      */
/*  CALLING SEQ.    :int check_db_gflin_def(const db_gflin_def *p)           */
/*  ARGUMENT        :p:db_gflin_def構造体へのポインタ(チェック対象)          */
/*  RETURN CODE     :0(正常),-1(エラー時)                                    */
/*  DESCRIPTION     :複数フィールドを検証し、エラーならログ出力後-1を返す    */
/*                  :                                                        */
/*****************************************************************************/
int cncl_chk_gflin_rec(const db_gflin_def *p)
{
    int retval = 0;
    /* grp_id */
    if (!checkGrpId(p->pri_key.grp_id)) {
        cncl_ems_param_err("grp_id", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* if_id */
    if (!checkIfId(p->pri_key.if_id)) {
        cncl_ems_param_err("if_id", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* station_id */
    if (!checkStationId(p->pri_key.station_id)) {
        cncl_ems_param_err("station_id", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* connect_id */
    if (!checkConnectId(p->pri_key.connect_id)) {
        cncl_ems_param_err("connect_id", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* tcpip_prc_name : '$'で開始して英数字 */
    if (!checkStartCharAndAlnum(p->tcpip_prc_name, 6, '$')) {
        cncl_ems_param_err("tcpip_prc_name", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* ip_adress_src : IPv4 */
    if (!checkIPv4Address(p->ip_adress_src, 15, true)) {
        cncl_ems_param_err("ip_adress_src", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* port_num_src : スペース or 0～65535 */
    if (!checkPortSpec(p->port_num_src, 5, true)) {
        cncl_ems_param_err("port_num_src", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* ip_adress_dst : IPv4 */
    if (!checkIPv4Address(p->ip_adress_dst, 15, false)) {
        cncl_ems_param_err("ip_adress_dst", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* port_num_dst : 0～65535 */
    if (!checkPortSpec(p->port_num_dst, 5, false)) {
        cncl_ems_param_err("port_num_dst", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* invalid_flg : ' ' or '1' */
    if (p->invalid_flg != ' ' && p->invalid_flg != '1') {
        cncl_ems_param_err("invalid_flg", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* update_date : YYYYMMDD */
    if (!checkDateYYYYMMDD(p->update_date)) {
        cncl_ems_param_err("update_date", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    return retval; /* result */
}
