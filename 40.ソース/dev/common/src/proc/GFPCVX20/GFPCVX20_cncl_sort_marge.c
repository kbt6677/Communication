/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/04/08＞         *
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
/*        WRITTEN-DATE      ････ 2025/04/08                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/04/08 新規作成                                      */
/*  1.1  ISYS 工藤  2025/04/28 GFNWI 2.01版対応                              */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stddef.h> nolist
#include <stdint.h> nolist
#include <string.h> nolist
/* USER HEADER     */
#ifdef _TANDEM_SOURCE
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define IF_ID_SENTINEL      "}}}}}"  /* 5 byte */
#define STATION_ID_SENTINEL "}}}}}}" /* 6 byte */

/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : if_id省略判定                                         */
/****************************************************************************/
static int is_if_id_sentinel(const char if_id[5])
{
    return memcmp(if_id, IF_ID_SENTINEL, 5) == 0;
}

/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : station_id省略判定                                    */
/****************************************************************************/
static int is_station_sentinel(const char st_id[6])
{
    return memcmp(st_id, STATION_ID_SENTINEL, 6) == 0;
}

/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : ステーションレベル判定                                */
/****************************************************************************/
/* レコード種別 */
int is_station_rec(const db_gfnwi_def *r)
{
    return !is_station_sentinel(r->pri_key.station_id);
}

/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : インターフェースレベル判定                            */
/****************************************************************************/
int is_if_id_rec(const db_gfnwi_def *r)
{
    return is_station_sentinel(r->pri_key.station_id) && !is_if_id_sentinel(r->pri_key.if_id);
}
/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : グループレベル判定                                    */
/****************************************************************************/
static int is_grp_id_rec(const db_gfnwi_def *r)
{
    return is_station_sentinel(r->pri_key.station_id) && is_if_id_sentinel(r->pri_key.if_id);
}
/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     :station <-> grp_id マッチ判定                           */
/****************************************************************************/
static int match_station_if_id(const db_gfnwi_def *st, const db_gfnwi_def *c)
{
    return st->pri_key.site_id == c->pri_key.site_id && st->pri_key.nw_id == c->pri_key.nw_id
           && memcmp(st->pri_key.grp_id, c->pri_key.grp_id, 5) == 0
           && memcmp(st->pri_key.if_id, c->pri_key.if_id, 5) == 0;
}
/****************************************************************************/
/*  FUNCTION        :                                                       */
/*  CALLING SEQ.    :                                                       */
/*  ARGUMENT        :                                                       */
/*                  :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     :station <-> grp_id マッチ判定                           */
/****************************************************************************/
static int match_station_grp(const db_gfnwi_def *st, const db_gfnwi_def *c)
{
    return st->pri_key.site_id == c->pri_key.site_id && st->pri_key.nw_id == c->pri_key.nw_id
           && memcmp(st->pri_key.grp_id, c->pri_key.grp_id, 5) == 0;
}
/****************************************************************************/
/*  FUNCTION        :cncl_sort_marge                                        */
/*  CALLING SEQ.    :short cncl_sort_marge(db_gfnwi_def *dst,               */
/*                     size_t *dst_cnt, const db_gfnwi_def *src,            */
/*                     size_t src_cnt);                                     */
/*  ARGUMENT        :db_gfnwi_def *dst  出力先                              */
/*                   size_t *dst_cnt    出力可能件数/出力結果件数           */
/*                   const db_gfnwi_def *src 入力                           */
/*                   size_t src_cnt  入力件数                               */
/*  RETURN CODE     :0:正常 0> 欠落有 0<件数超過                            */
/*  DESCRIPTION     :ステーション/インターフェース/グループレベルの定義を   */
/*                     統合する。                                           */
/****************************************************************************/
short cncl_sort_marge(db_gfnwi_def *dst, size_t *dst_cnt, const db_gfnwi_def *src, size_t src_cnt)
{
    if (!src || src_cnt == 0) return 0; /* 入力なし */

    size_t   dst_idx    = 0;            /* 実際に書き込んだ件数 */
    size_t   total_sets = 0;            /* station レコード総数 */
    uint16_t missing_ct = 0;            /* if_id / grp_id が不足した数 */
    size_t   dst_max    = *dst_cnt;

    for (size_t i = 0; i < src_cnt; ++i) {
        const db_gfnwi_def *station = &src[i];
        if (!is_station_rec(station))   /* station レコード以外は無視 */
            continue;

        ++total_sets;

        const db_gfnwi_def *if_id = NULL;
        const db_gfnwi_def *grp   = NULL;

        /* 対応する if_id / grp_id を探索 */
        for (size_t j = 0; j < src_cnt; ++j) {
            const db_gfnwi_def *c = &src[j];
            if (c == station) continue;

            if (!if_id && is_if_id_rec(c) && match_station_if_id(station, c))
                if_id = c;
            else if (!grp && is_grp_id_rec(c) && match_station_grp(station, c))
                grp = c;

            if (if_id && grp) break;
        }

        if (!if_id) ++missing_ct;
        if (!grp) ++missing_ct;

        /* dst に空きがある場合のみコピーを実施 */
        if (dst_idx < dst_max && dst) {
            dst[dst_idx] = *station; /* station ベース */

            if (grp) {
                memcpy(dst[dst_idx].nw_id_info.nw_kubun, grp->nw_id_info.nw_kubun,
                       sizeof(dst[dst_idx].nw_id_info.nw_kubun));

                dst[dst_idx].mng_lyr_info         = grp->mng_lyr_info;
                dst[dst_idx].connect_num_mng_info = grp->connect_num_mng_info;
                dst[dst_idx].env_set_info         = grp->env_set_info;
                //dst[dst_idx].gfnws_info           = grp->gfnws_info;  // コネクション制御クライアントが使用しないの&削除されたようなので削除 変更頻度に耐えられない。
            }
            if (if_id) {
                dst[dst_idx].denbun_item_lct_info     = if_id->denbun_item_lct_info;
                dst[dst_idx].connect_nxt_prc_info     = if_id->connect_nxt_prc_info;
                //dst[dst_idx].shori_kbn_info           = if_id->shori_kbn_info;    // コネクション制御クライアントが使用しないので削除 変更頻度に耐えられない。
                dst[dst_idx].trans_cntrl_tmr_info     = if_id->trans_cntrl_tmr_info;
                dst[dst_idx].trans_cntrl_cnt_info     = if_id->trans_cntrl_cnt_info;
                dst[dst_idx].cntrl_denbun_tmr_info    = if_id->cntrl_denbun_tmr_info;
                dst[dst_idx].cntrl_denbun_cnt_info    = if_id->cntrl_denbun_cnt_info;
                //memmove(dst[dst_idx].dst_unq_info, if_id->dst_unq_info, sizeof(if_id->dst_unq_info));  // コネクション制御クライアントが使用しないので削除 変更頻度に耐えられない。
            }
            ++dst_idx;
        }
    }

    *dst_cnt = total_sets;
    /* ───── 返却値判定 ───── */
    if (total_sets > dst_max) /* 容量不足 → 負値で不足件数 */
        return -(short)(total_sets - dst_max);

    /* 容量は十分 → 正値で if_id / grp_id 不足件数 */
    return (short)missing_ct;
}
