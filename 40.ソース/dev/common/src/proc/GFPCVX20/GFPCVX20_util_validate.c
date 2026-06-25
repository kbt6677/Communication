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
/*                   2025/03/15 末尾の空白を許容するように修正               */
/*                     checkIPv4Address                                      */
/*                     checkStartCharAndAlnum                                */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <ctype.h> nolist
#include <stdbool.h> nolist
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
/* USER HEADER     */
#include "GFPCVX20_util.h" nolist
/**
 * @brief
 *
 * @param str
 * @param buffer_length
 * @return char*
 *
 * @author isys kudo, daisuke
 * @date 2024-11-08 initial release.
 */
char *trim(char *str, size_t buffer_length)
{
    if (str == NULL || buffer_length == 0) {
        return NULL;
    }

    // 先頭の空白とNULLを取り除く
    char *start = str;
    while (*start != '\0' && (isspace((unsigned char)*start) || *start == '\0')) {
        start++;
    }

    // 末尾の空白とNULLを取り除く
    char *end = str + buffer_length - 1;  // buffer_length - 1 は NULL 終端の位置
    while (end > start && (isspace((unsigned char)*end) || *end == '\0')) {
        end--;
    }

    // 新しい終端を設定
    *(end + 1) = '\0';

    return start;
}
/*****************************************************************************/
/*  FUNCTION        :isAllDigits                                             */
/*  CALLING SEQ.    :bool isAllDigits(const char *str, int length)           */
/*  ARGUMENT        :str   :チェック対象文字列                               */
/*                  :length:文字列長                                         */
/*  RETURN CODE     :true(すべて数字),false(それ以外含む)                    */
/*  DESCRIPTION     :与えられた文字列が全て数字文字かどうかを確認            */
/*****************************************************************************/
bool isAllDigits(const char *str, int length)
{
    for (int i = 0; i < length; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :isAllHex                                                */
/*  CALLING SEQ.    :bool isAllHex(const char *str, int length)              */
/*  ARGUMENT        :str   :チェック対象文字列                               */
/*                  :length:文字列長                                         */
/*  RETURN CODE     :true(16進数文字のみ),false(それ以外含む)                */
/*  DESCRIPTION     :与えられた文字列がすべて16進文字かどうかを確認          */
/*****************************************************************************/
bool isAllHex(const char *str, int length)
{
    for (int i = 0; i < length; i++) {
        if (!isxdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkDecimalRange                                       */
/*  CALLING SEQ.    :bool checkDecimalRange(const char *str, int length,     */
/*                   int minVal, int maxVal, bool allowSpace)                */
/*  ARGUMENT        :str       :数値文字列                                   */
/*                  :length    :文字列長                                     */
/*                  :minVal    :最小値                                       */
/*                  :maxVal    :最大値                                       */
/*                  :allowSpace:全スペースを許容するか                       */
/*  RETURN CODE     :true(範囲内),false(範囲外)                              */
/*  DESCRIPTION     :文字列を整数に変換し、指定の範囲内にあるかを確認        */
/*****************************************************************************/
bool checkDecimalRange(const char *str, int length, int minVal, int maxVal, bool allowSpace)
{
    char *trimed;
    int   trimedLen;
    bool allSpace = true;
    for (int i = 0; i < length; i++) {
        if (str[i] != ' ') {
            allSpace = false;
            break;
        }
    }
    if (allSpace) {
        /* 全スペースの場合に、許容ならtrue,非許容ならばfalseを返す */
        return  allowSpace ? true : false;
    }

    /* 数値変換 */
    char buf[length + 1];
    memset(buf, 0, sizeof(buf));
    /* length超えないようコピー(ヌル終端化して atoi が使えるようにする) */
    int copyLen = (length < (int)sizeof(buf) - 1) ? length : (int)sizeof(buf) - 1;
    memcpy(buf, str, copyLen);
    buf[copyLen] = '\0';

    trimed = trim(buf, copyLen);
    trimedLen = (int)strlen(buf);

    //intの最大桁数は10桁
    if(trimedLen > 10) return false;

    /* 数字チェック */
    for (int i = 0; i < trimedLen; i++) {
        if (!isdigit((unsigned char)buf[i])) {
            return false;
        }
    }

    int val = atoi(trimed);
    if (val < minVal || val > maxVal) {
        return false;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkIPv4Address                                        */
/*  CALLING SEQ.    :bool checkIPv4Address(const char *str, int length,      */
/*                   bool allowEmpty)                                        */
/*  ARGUMENT        :str       :IPv4アドレス文字列                           */
/*                  :length    :文字列長                                     */
/*                  :allowEmpty:全スペースの場合もOKにするか                 */
/*  RETURN CODE     :true(妥当),false(形式不正)                              */
/*  DESCRIPTION     :IPv4アドレス(x.x.x.x)形式をチェックする                 */
/*  UPDATE          :2025/03/15 末尾の空白を許容するように修正               */
/*****************************************************************************/
bool checkIPv4Address(const char *str, int length, bool allowEmpty)
{
    /* 全部スペースを許容するなら */
    if (allowEmpty) {
        bool allSpace = true;
        for (int i = 0; i < length; i++) {
            if (str[i] != ' ') {
                allSpace = false;
                break;
            }
        }
        if (allSpace) return true;
    }

    /* 一旦バッファにコピーしてパース */
    char buf[64];
    memset(buf, 0, sizeof(buf));
    int copyLen = (length < (int)sizeof(buf) - 1) ? length : (int)sizeof(buf) - 1;
    memcpy(buf, str, copyLen);
    buf[copyLen] = '\0';
    cncl_rm_trspc_wn(buf, copyLen);
    copyLen      = (int)strlen(buf);

    /* 簡易チェック: x.x.x.x の形式で各xは0-255 */
    int dotCount = 0;
    for (int i = 0; i < copyLen; i++) {
        if (buf[i] == '.') {
            dotCount++;
        } else if (!isdigit((unsigned char)buf[i])) {
            return false;
        }
    }
    if (dotCount != 3) {
        return false;
    }
    /* トークンに切って各値を 0～255 かどうか */
    char *token        = strtok(buf, ".");
    int   segmentCount = 0;
    while (token) {
        segmentCount++;
        if (strlen(token) == 0) return false;
        int val = atoi(token);
        if (val < 0 || val > 255) {
            return false;
        }
        token = strtok(NULL, ".");
    }
    if (segmentCount != 4) {
        return false;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkDateYYYYMMDD                                       */
/*  CALLING SEQ.    :bool checkDateYYYYMMDD(const char *str)                 */
/*  ARGUMENT        :str:8桁のYYYYMMDD文字列                                 */
/*  RETURN CODE     :true(全て数字),false(それ以外含む)                      */
/*  DESCRIPTION     :日付(YYYYMMDD)の文字列形式チェックを行う                */
/*****************************************************************************/
bool checkDateYYYYMMDD(const char *str)
{
    /* 8桁すべて数字か */
    for (int i = 0; i < 8; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    /* 必要ならさらに年・月・日の妥当性チェックを実装 */
    return true;
}

/*****************************************************************************/
/*  FUNCTION        :checkStartCharAndAlnum                                  */
/*  CALLING SEQ.    :bool checkStartCharAndAlnum(const char *str, int length, */
/*                   char startChar)                                         */
/*  ARGUMENT        :str       :文字列                                       */
/*                  :length    :文字列長                                     */
/*                  :startChar :先頭で期待する文字                           */
/*  RETURN CODE     :true(妥当),false(不正)                                  */
/*  DESCRIPTION     :先頭文字が指定文字かつ残りが英数字かを確認              */
/*  UPDATE          :2025/03/15 末尾の空白を許容するように修正               */
/*                     checkStartCharAndAlnum                                */
/*****************************************************************************/
bool checkStartCharAndAlnum(const char *str, int length, char startChar)
{
    bool trail_space = false;
    if (length <= 0) return false;
    if (str[0] != startChar) {
        return false;
    }
    for (int i = 1; i < length; i++) {
        if (trail_space && (unsigned char)str[i] != ' ') {
            // ここではspace以外許容しない
            return false;
        } else {
            // 最初のSpaceを見つけたら遷移する。
            if ((unsigned char)str[i] == ' ') {
                trail_space = true;
            } else if (!isalnum((unsigned char)str[i])) {
                return false;
            }
        }
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkStringInList                                       */
/*  CALLING SEQ.    :bool checkStringInList(const char *str, int length,     */
/*                   const char *candidateList[], int candidateCount)        */
/*  ARGUMENT        :str    :確認対象文字列                                  */
/*                  :length :文字列長                                        */
/*                  :candidateList:許容する文字列リスト                      */
/*                  :candidateCount:リスト要素数                             */
/*  RETURN CODE     :true(一致あり),false(一致なし)                          */
/*  DESCRIPTION     :文字列が指定リスト内いずれかと一致するかを判定          */
/*****************************************************************************/
bool checkStringInList(const char *str, int length, const char *candidateList[], int candidateCount)
{
    char buf[32];
    memset(buf, 0, sizeof(buf));
    int copyLen = (length < (int)sizeof(buf) - 1) ? length : (int)sizeof(buf) - 1;
    memcpy(buf, str, copyLen);
    buf[copyLen] = '\0';

    for (int i = 0; i < candidateCount; i++) {
        if (strcmp(buf, candidateList[i]) == 0) {
            return true;
        }
    }
    return false;
}
/*****************************************************************************/
/*  FUNCTION        :checkAlphabetStartAndAlnumBody                          */
/*  CALLING SEQ.    :bool checkAlphabetStartAndAlnumBody(const char *str,    */
/*                   int length, int minLen, int maxLen)                     */
/*  ARGUMENT        :str    :文字列                                          */
/*                  :length :文字列長                                        */
/*                  :minLen :許容最小文字数                                  */
/*                  :maxLen :許容最大文字数                                  */
/*  RETURN CODE     :true(妥当),false(不正)                                  */
/*  DESCRIPTION     :先頭英字+残り英数字で構成されるかチェック               */
/*****************************************************************************/
bool checkAlphabetStartAndAlnumBody(const char *str, int length, int minLen, int maxLen)
{
    if (length < minLen || length > maxLen) {
        return false;
    }
    if (!isalpha((unsigned char)str[0])) {
        return false;
    }
    for (int i = 1; i < length; i++) {
        if (!isalnum((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkPortSpec                                           */
/*  CALLING SEQ.    :bool checkPortSpec(const char *str, int length,         */
/*                   bool allowSpace)                                        */
/*  ARGUMENT        :str       :ポート番号文字列                             */
/*                  :length    :文字列長                                     */
/*                  :allowSpace:全スペースを許容するか                       */
/*  RETURN CODE     :true(0~65535内),false(不正)                            */
/*  DESCRIPTION     :数値がポート範囲(0~65535)内かをチェック                */
/*****************************************************************************/
bool checkPortSpec(const char *str, int length, bool allowSpace)
{
    /* スペースだけもOKならチェック */
    if (allowSpace) {
        bool allSpace = true;
        for (int i = 0; i < length; i++) {
            if (str[i] != ' ') {
                allSpace = false;
                break;
            }
        }
        if (allSpace) return true; /* 全部スペースならOK */
    }
    /* 数値として 0～65535 チェック */
    return checkDecimalRange(str, length, 0, 65535, false);
}
/*****************************************************************************/
/*  FUNCTION        :checkNumericGreaterEqual1                               */
/*  CALLING SEQ.    :bool checkNumericGreaterEqual1(const char *str,         */
/*                   int length)                                             */
/*  ARGUMENT        :str   :数値文字列                                       */
/*                  :length:文字列長                                         */
/*  RETURN CODE     :true(1以上),false(それ未満or不正)                       */
/*  DESCRIPTION     :文字列が1以上の数値かを判定                             */
/*****************************************************************************/
bool checkNumericGreaterEqual1(const char *str, int length)
{
    /* 数値かつ1以上 */
    /* 例: "0"はNG, "1"以上OK */
    char  buf[32];
    char *trimed;
    memset(buf, 0, sizeof(buf));
    int copyLen = (length < (int)sizeof(buf) - 1) ? length : (int)sizeof(buf) - 1;
    memcpy(buf, str, copyLen);
    buf[copyLen] = '\0';

    trimed       = trim(buf, copyLen);

    for (int i = 0; i < (int)strlen(trimed); i++) {
        if (!isdigit((unsigned char)trimed[i])) {
            return false;
        }
    }
    if (atoi(trimed) < 1) {
        return false;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :checkSpcOrHex                                           */
/*  CALLING SEQ.    :bool checkSpcOrHex(const char *str, int length)         */
/*  ARGUMENT        :str   :文字列                                           */
/*                  :length:文字列長                                         */
/*  RETURN CODE     :true(スペースor16進文字),false(不正)                    */
/*  DESCRIPTION     :文字列がスペースまたは16進数表記かを確認                */
/*****************************************************************************/
// bool checkSpcOrHex(const char *str, int length)
// {
//     /* 左詰めかどうかの判定は場合に応じて別途 */
//     for (int i = 0; i < length; i++) {
//         if (str[i] == ' ') {
//             /* スペースは許容 */
//             continue;
//         } else if (!isxdigit((unsigned char)str[i])) {
//             return false;
//         }
//     }
//     return true;
// }
/*
 * checkSpcOrHex
 *   str    : 入力文字列（ASCII形式、ヌル終端の保証なし）
 *   length : str の先頭から見る最大長
 *
 * 戻り値:
 *   - 先頭から 0 個以上の 16進文字が続き、その後はすべて空白またはNULLであれば true
 *   - それ以外の不正文字が含まれる場合 false
 *   - 全部空白(またはNULL)でも true とする
 */
bool checkSpcOrHex(const char *str, int length)
{
    if (str == NULL || length < 0) {
        return false;
    }

    bool inTrailingSpace = false;  // 末尾の空白判定に入ったかどうか

    for (int i = 0; i < length; i++) {
        unsigned char c = (unsigned char)str[i];

        if (c == '\0') {
            // NULL に遭遇した時点で残りはすべて末尾空白扱い
            inTrailingSpace = true;
            continue;
        }

        if (inTrailingSpace) {
            // 末尾空白フェーズでは、空白か NULL 以外は不正
            if (c != ' ') {
                return false;
            }
        } else {
            // まだ本体を読み込み中
            if (c == ' ') {
                // 空白が出たら、それ以降は末尾空白フェーズに遷移
                inTrailingSpace = true;
            } else {
                // 16進文字かを判定 (isxdigit または自前で [0-9A-Fa-f] 判定)
                if (!isxdigit(c)) {
                    return false;
                }
            }
        }
    }

    // 最後まで不正文字が無ければ true
    return true;
}

/*****************************************************************************/
/*  FUNCTION        :checkCharInSet                                          */
/*  CALLING SEQ.    :bool checkCharInSet(char c, const char *set)            */
/*  ARGUMENT        :c  :判定対象文字                                        */
/*                  :set:許容文字集合                                        */
/*  RETURN CODE     :true(set内),false(set外)                                */
/*  DESCRIPTION     :文字cが文字セットsetに含まれているか判定                */
/*****************************************************************************/
bool checkCharInSet(char c, const char *set)
{
    if(!set) return false;
    /* set に含まれていればOK */
    for (const char *p = set; *p != '\0'; p++) {
        if (c == *p) return true;
    }
    return false;
}
