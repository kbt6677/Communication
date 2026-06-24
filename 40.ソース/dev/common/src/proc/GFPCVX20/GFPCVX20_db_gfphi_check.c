/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/03/01＞         *
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
/*        WRITTEN-DATE      ････ 2025/03/01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/03/01 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <ctype.h>      nolist
#include <stdbool.h>    nolist
#include <stdlib.h>     nolist
#include <string.h>     nolist
#ifdef _TANDEM_SOURCE
#include <cextdecs.h(FILENAME_SCAN_)> nolist
#include <tal.h> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif
/* USER HEADER     */
#include <errcd.h>  nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_util.h" nolist
/*
 * domain_name    : '%'で始まる 2～8桁
 * pathmon_name   : '$'で始まる2～6桁, または ノード名付き(要件詳細不明。今回は例示のみ)
 * srv_cls_name   : 英字で始まる1～15桁
 * invalid_flg    : ' ' or '1'
 * lcn_num_min,max: "0000"～"8191"
 */
/*****************************************************************************/
/*  FUNCTION        :checkDomainName                                        */
/*  CALLING SEQ.    :static bool checkDomainName(const char *domain_name)   */
/*  ARGUMENT        :domain_name:先頭'%'で始まり2~8桁使用想定の文字列      */
/*  RETURN CODE     :true,false                                             */
/*  DESCRIPTION     :ドメイン名の先頭文字や長さが妥当かを検証               */
/***************************************************************************S*/
static bool checkDomainName(const char *domain_name)
{
    bool allspace = true;

    /* 全て空白は許可 */
    for (int i = 0; i < 8; i++) {
        if (domain_name[i] != ' ') {
            allspace = false;
            break;
        }
    }
    if (allspace) return true;

    /* '%' で始まり、実際の文字数が2～8という要件 */
    /* または‘$’で始まってもよい(work around) */
    if (domain_name[0] != '%' && domain_name[0] != '$') return false;

    /* domain_name は配列8バイト (終端なし) なので、
       実際に何文字使われているか判定するには要件次第 */
    /* 簡易的に最初のスペースor'\0'までを長さとみなすなど…(ここでは例示) */

    /* ここではとりあえず8文字フルに見てしまい、2～8桁全部が有効文字(スペース以外)であるとする */
    int usedLen = 1;
    for (int i = 1; i < 8; i++) {
        if (domain_name[i] == ' ') break; /* スペースが来たら終わりとみなす */
        usedLen++;
    }
    if (usedLen < 2 || usedLen > 8) return false;

    return true;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
/*****************************************************************************/
/*  FUNCTION        :checkPathmonName                                        */
/*  CALLING SEQ.    :static bool checkPathmonName(const char *pathmon_name)  */
/*  ARGUMENT        :pathmon_name:'$'開始2~6桁か、別の形式(NODE$xxxx等)      */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :pathmon名(先頭'$'またはノード名付き)の形式を検証        */
/*****************************************************************************/
static bool checkPathmonName(const char *pathmon_name)
{
    short length = 0;
    short result;
    //空白が現れるまでか16文字めまで
    for (int i = 0; i < 16; i++) {
        if (pathmon_name[i] == ' ') break;
        length++;
    }
    //全空白は通過
    if(length == 0) {
        return true;
    }
    //形式チェック
    result = FILENAME_SCAN_(pathmon_name, length, no_param, no_param, no_param, no_param);

    return result == 0 ? true : false;
}
#pragma clang diagnostic pop
/*****************************************************************************/
/*  FUNCTION        :checkSrvClsName                                         */
/*  CALLING SEQ.    :static bool checkSrvClsName(const char *srv_cls_name)   */
/*  ARGUMENT        :srv_cls_name:英字開始1~15桁の英数字文字列              */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :サーバクラス名(先頭英字,残り英数字,1~15桁)を検証       */
/*****************************************************************************/
static bool checkSrvClsName(const char *srv_cls_name)
{
    bool allspace = true;

    /* 全て空白は許可 */
    for (int i = 0; i < 16; i++) {
        if (srv_cls_name[i] != ' ') {
            allspace = false;
            break;
        }
    }
    if (allspace) return true;

    /* 英字で始まり、1～15桁(英数字) */
    /* srv_cls_name は16バイト */
    int usedLen = 0;
    for (int i = 0; i < 16; i++) {
        if (srv_cls_name[i] == ' ') break; /* スペースが来たら終端扱い(例) */
        usedLen++;
    }
    if (usedLen < 1 || usedLen > 15) {
        return false;
    }
    /* 先頭英字 */
    if (!isalpha((unsigned char)srv_cls_name[0])) {
        return false;
    }
    /* 残りは英数字のみ */
    for (int i = 1; i < usedLen; i++) {
        if (!isalnum((unsigned char)srv_cls_name[i]) && srv_cls_name[i] != '-') {
            return false;
        }
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkLcnNum                                             */
/*  CALLING SEQ.    :static bool checkLcnNum(const char *lcn_num)            */
/*  ARGUMENT        :lcn_num:4桁数字"0000"~"8191"の範囲文字列               */
/*  RETURN CODE     :true,false                                              */
/*  DESCRIPTION     :4桁数値が0~8191の範囲に収まるかを検証                  */
/*****************************************************************************/
/* 文字列4桁で "0000"～"8191" */
static bool checkLcnNum(const char *lcn_num)
{
    bool allspace = true;

    /* 全て空白は許可 */
    for (int i = 0; i < 4; i++) {
        if (lcn_num[i] != ' ') {
            allspace = false;
            break;
        }
    }
    if (allspace) return true;
    /* "0000"～"8191" */
    if (!isAllDigits(lcn_num, 4)) {
        return false;
    }
    /* 数値変換 */
    char buf[5];
    memcpy(buf, lcn_num, 4);
    buf[4]  = '\0';
    int val = atoi(buf);
    if (val < 0 || val > 8191) {
        return false;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_chk_gfphi_rec                                   */
/*  CALLING SEQ.    :int cncl_chk_db_gfphi_rec(const db_gfphi_def *p)        */
/*  ARGUMENT        :p:db_gfphi_def構造体へのポインタ(各フィールド検証)      */
/*  RETURN CODE     :0(正常),-1(エラー検出時)                                */
/*  DESCRIPTION     :複数のフィールドをチェックし異常時はエラー出力後-1返却  */
/*****************************************************************************/
int cncl_chk_gfphi_rec(const db_gfphi_def *p)
{
    int retval = 0;
    /* domain_name */
    if (!checkDomainName(p->srv_cls_info.domain_name)) {
        cncl_ems_param_err("domain_name", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* pathmon_name */
    if (!checkPathmonName(p->srv_cls_info.pathmon_name)) {
        cncl_ems_param_err("pathmon_name", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* srv_cls_name */
    if (!checkSrvClsName(p->srv_cls_info.srv_cls_name)) {
        cncl_ems_param_err("srv_cls_name", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* invalid_flg */
    if (p->invalid_flg != ' ' && p->invalid_flg != '1') {
        cncl_ems_param_err("invalid_flg", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* lcn_num_min */
    if (!checkLcnNum(p->lcn_num_scope.lcn_num_min)) {
        cncl_ems_param_err("lcn_num_min", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    /* lcn_num_max */
    if (!checkLcnNum(p->lcn_num_scope.lcn_num_max)) {
        cncl_ems_param_err("lcn_num_max", 0, DEF_NERR_PRM_RD_ERR_INV);
        retval = -1;
    }

    return retval; /* retval */
}
