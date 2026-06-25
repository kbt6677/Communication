/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/29＞         *
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
/*        WRITTEN-DATE      ････ 2025/01/29                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/01/29 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <string.h> nolist
#include <tal.h> nolist
/* USER HEADER     */
#include <GFPCGXB0.h> nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_util.h" nolist
#include "common.h" nolist

/****************************************************************************/
/*  FUNCTION        :cncl_initial_ioparams                                  */
/*  CALLING SEQ.    :void cncl_initial_ioparams(iom_params_def *iom_params) */
/*  ARGUMENT        :iom_params:入出力パラメータ構造体へのポインタ          */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :iom_params内を初期化して、ファイル番号等を設定         */
/****************************************************************************/
void cncl_initial_ioparams(iom_params_def *iom_params)
{
    memset(iom_params, ' ', sizeof(iom_params_def));
    iom_params->arg4.file_no = DEF_FILENO_CLOSED;
    memset(&(iom_params->arg5), 0, sizeof(COM_IOM_arg_5_def));
    memset(iom_params->arg5.key_value, ' ', sizeof(iom_params->arg5.key_value));
    memset(iom_params->arg5.key_type, ' ', sizeof(iom_params->arg5.key_type));
    memset(iom_params->arg5.rec_area, ' ', sizeof(iom_params->arg5.rec_area));
    iom_params->arg6.guardian_errcode = 0;
    iom_params->arg6.rec_len          = 0;
}
/****************************************************************************/
/*  FUNCTION        :cncl_preset_ioparams                                   */
/*  CALLING SEQ.    :void cncl_preset_ioparams(                             */
/*                   iom_params_def *iom_params,                            */
/*                   filename_p_t filename,                                 */
/*                   filename_l_t fileId,                                   */
/*                   filename_l_t fileType)                                 */
/*  ARGUMENT        :iom_params:入出力パラメータ構造体                      */
/*                  :filename  :ファイル名                                  */
/*                  :fileId    :ファイルID                                  */
/*                  :fileType  :ファイル種別                                */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :iom_paramsにプログラムIDやファイル名などを設定         */
/****************************************************************************/
void cncl_preset_ioparams(iom_params_def *iom_params, filename_p_t filename, filename_l_t fileId, filename_l_t fileType)
{
    memmove(iom_params->arg3.prog_id, DEF_GFPCVX20, strlen(DEF_GFPCVX20));
    memmove(iom_params->arg3.file_name, filename, strnlen_isys(filename, sizeof(filename_p_t) - 1));
    memmove(&iom_params->arg3.file_id, fileType, strnlen_isys(fileType, sizeof(filename_l_t) - 1));
    memmove(iom_params->arg4.file_id, fileId, strnlen_isys(fileId, sizeof(filename_l_t) - 1));
    memmove(iom_params->arg4.file_name, filename, strnlen_isys(filename, sizeof(filename_p_t) - 1));
}
/****************************************************************************/
/*  FUNCTION        :cncl_prepare_ioparams                                  */
/*  CALLING SEQ.    :void cncl_prepare_ioparams(                            */
/*                   iom_params_def *iom_params,                            */
/*                   func_type_t func_type,                                 */
/*                   short part_key_type, short part_key_position,          */
/*                   short part_key_len, char key_value[70],                */
/*                   char key_type[2], short key_len, short compare_len,    */
/*                   short positioning_mode, short lock_flg,                */
/*                   short asc_desc_type, long io_timer, void *rec_area,    */
/*                   short rec_len)                                         */
/*  ARGUMENT        :iom_params       :入出力パラメータ構造体               */
/*                  :func_type        :実行するI/O関数種別                  */
/*                  :part_key_type    :パーティションキー種別               */
/*                  :part_key_position:パーティションキー開始位置           */
/*                  :part_key_len     :パーティションキー長                 */
/*                  :key_value        :キー値                               */
/*                  :key_type         :キー種別                             */
/*                  :key_len          :キー長                               */
/*                  :compare_len      :比較時の桁数                         */
/*                  :positioning_mode :レコード検索モード                   */
/*                  :lock_flg         :ロック指定                           */
/*                  :asc_desc_type    :昇順/降順指定                        */
/*                  :io_timer         :I/Oタイムアウト値                    */
/*                  :rec_area         :I/O用レコードバッファ                */
/*                  :rec_len          :レコード長                           */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :iom_paramsの各種フィールドを指定値でセット             */
/****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_prepare_ioparams(iom_params_def *iom_params, func_type_t func_type, short part_key_type,
                          short part_key_position, short part_key_len, char key_value[70], char key_type[2],
                          short key_len, short compare_len, short positioning_mode, short lock_flg, short asc_desc_type,
                          long io_timer, void *rec_area, short rec_len)
{
    COM_IOM_arg_5_def *arg5 = &(iom_params->arg5);
    if (_arg_present(func_type)) {
        cobolization(iom_params->func_type, func_type, sizeof(func_type_t) - 1);
        memset(iom_params->arg3.file_io_type, ' ', sizeof(iom_params->arg3.file_io_type));
        memmove(iom_params->arg3.file_io_type, func_type, strlen(func_type));
    }
    if (_arg_present(part_key_type)) {
        arg5->part_key_type = part_key_type;
    }
    if (_arg_present(part_key_position)) {
        arg5->part_key_position = part_key_position;
    }
    if (_arg_present(part_key_len)) {
        arg5->part_key_len = part_key_len;
    }
    if (_arg_present(key_value)) {
        memset(arg5->key_value, ' ', 70);
        memmove(arg5->key_value, key_value, key_len > 70 ? 70 : key_len);
    }
    if (_arg_present(key_type)) {
        memcpy(arg5->key_type, key_type, 2);
    }
    if (_arg_present(key_len)) {
        arg5->key_len = key_len;
    }
    if (_arg_present(compare_len)) {
        arg5->compare_len = compare_len;
    }
    if (_arg_present(positioning_mode)) {
        arg5->positioning_mode = positioning_mode;
    }
    if (_arg_present(lock_flg)) {
        arg5->lock_flg = lock_flg;
    }
    if (_arg_present(asc_desc_type)) {
        arg5->asc_desc_type = asc_desc_type;
    }
    if (_arg_present(io_timer)) {
        arg5->io_timer = io_timer;
    }
    if (_arg_present(rec_area) && _arg_present(rec_len)) {
        memset(arg5->rec_area, ' ', sizeof(iom_params->arg6.rec_area));
        memmove(arg5->rec_area, rec_area, rec_len);
    }
    if (_arg_present(rec_len)) {
        arg5->rec_len = rec_len;
    } else {
        arg5->rec_len = sizeof(iom_params->arg6.rec_area);
        memset(arg5->rec_area, ' ', sizeof(iom_params->arg6.rec_area));
    }
    iom_params->arg6.rec_len = 0;
    memset(iom_params->sub_prog_sts, ' ', sizeof(sub_prog_sts_t));
    memset(iom_params->arg6.rec_area, ' ', sizeof(iom_params->arg5.rec_area));
}
/****************************************************************************/
/*  FUNCTION        :cncl_gfphi_key                                         */
/*  CALLING SEQ.    :void cncl_gfphi_key(db_gfphi_def *db_gfphi,            */
/*                   char site_id, char nw_id, char grp_id[5],              */
/*                   char srv_cls_kind[8], char srv_cls_num[4],             */
/*                   char srv_cls_mlt_num[4], char prc_file_kind[8],        */
/*                   char prc_file_num[4], char prc_file_mlt_num[4])        */
/*  ARGUMENT        :db_gfphi    :GFPHIレコード構造体のポインタ             */
/*                  :site_id    :サイトID                                   */
/*                  :nw_id      :NW ID                                      */
/*                  :grp_id     :グループID                                 */
/*                  :srv_cls_kind   :サーバクラス種別                       */
/*                  :srv_cls_num    :サーバクラス番号                       */
/*                  :srv_cls_mlt_num:サーバクラス拡張番号                   */
/*                  :prc_file_kind  :物理ファイル種別                       */
/*                  :prc_file_num   :物理ファイル番号                       */
/*                  :prc_file_mlt_num:物理ファイル拡張番号                  */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :引数をCOBOL形式に変換し、db_gfphiの主キーに設定        */
/****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_gfphi_key(db_gfphi_def *db_gfphi, char site_id, char nw_id, char grp_id[5], char srv_cls_kind[8],
                   char srv_cls_num[4], char srv_cls_mlt_num[4], char prc_file_kind[8], char prc_file_num[4],
                   char prc_file_mlt_num[4])
{
    if (_arg_present(site_id)) {
        db_gfphi->pri_key.site_id = site_id;
    }
    if (_arg_present(nw_id)) {
        db_gfphi->pri_key.nw_id = nw_id;
    }
    if (_arg_present(grp_id)) {
        cobolization(db_gfphi->pri_key.grp_id, grp_id, sizeof(db_gfphi->pri_key.grp_id));
    }
    if (_arg_present(srv_cls_kind)) {
        cobolization(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, srv_cls_kind,
                     sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    }
    if (_arg_present(srv_cls_num)) {
        cobolization(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num, srv_cls_num,
                     sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    }
    if (_arg_present(srv_cls_mlt_num)) {
        cobolization(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num, srv_cls_mlt_num,
                     sizeof(db_gfphi->pri_key.srv_cls_key.srv_cls_mlt_num));
    }
    if (_arg_present(prc_file_kind)) {
        cobolization(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_kind, prc_file_kind,
                     sizeof(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_kind));
    }
    if (_arg_present(prc_file_num)) {
        cobolization(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_num, prc_file_num,
                     sizeof(db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_num));
    }
    if (_arg_present(prc_file_mlt_num)) {
        cobolization(db_gfphi->pri_key.prc_file_key.prc_file_mlt_num, prc_file_mlt_num,
                     sizeof(db_gfphi->pri_key.prc_file_key.prc_file_mlt_num));
    }
}
/****************************************************************************/
/*  FUNCTION        :cncl_db_gfif_key                                       */
/*  CALLING SEQ.    :void cncl_db_gfif_key(void *db_gfif_key,               */
/*                   char site_id, char nw_id, char grp_id[5],              */
/*                   char if_id[5], char station_id[6])                     */
/*  ARGUMENT        :db_gfif_key:GFNWIレコード等の主キー保持構造体          */
/*                  :site_id   :サイトID                                    */
/*                  :nw_id     :NW ID                                       */
/*                  :grp_id    :グループID                                  */
/*                  :if_id     :インタフェースID                            */
/*                  :station_id:ステーションID                              */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :引数をCOBOL形式に変換し、db_gfif_keyの主キーに設定     */
/****************************************************************************/

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_db_gfif_key(void *db_gfif_key, char site_id, char nw_id, char grp_id[5], char if_id[5], char station_id[6])
{
    db_gfnwi_def *db_gfnwi = db_gfif_key;
    if (!_arg_present(db_gfif_key)) return;
    if (_arg_present(site_id)) {
        db_gfnwi->pri_key.site_id = site_id;
    }
    if (_arg_present(nw_id)) {
        db_gfnwi->pri_key.nw_id = nw_id;
    }
    if (_arg_present(grp_id)) {
        cobolization(db_gfnwi->pri_key.grp_id, grp_id, sizeof(db_gfnwi->pri_key.grp_id));
    }
    if (_arg_present(if_id)) {
        cobolization(db_gfnwi->pri_key.if_id, if_id, sizeof(db_gfnwi->pri_key.if_id));
    }
    if (_arg_present(station_id)) {
        cobolization(db_gfnwi->pri_key.station_id, station_id, sizeof(db_gfnwi->pri_key.station_id));
    }
}
/****************************************************************************/
/*  FUNCTION        :cncl_db_gflin_a1_key                                   */
/*  CALLING SEQ.    :void cncl_db_gflin_a1_key(gflin_a1_key_t *gflin_a1_key,*/
/*                      char server_class_name[8],char server_class_num[4]) */
/*  ARGUMENT        :gflin_a1_key:gflinレコードのalt1キー保持構造体         */
/*                  :server_class_name                                      */
/*                  :server_class_num                                       */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :引数をCOBOL形式に変換し、db_gfif_keyのA1キーに設定     */
/****************************************************************************/
void cncl_db_gflin_a1_key(gflin_a1_key_t *gflin_a1_key, char server_class_name[8], char server_class_num[4])
{
    if (!gflin_a1_key) return;
    cobolization(gflin_a1_key->srv_cls_kind, server_class_name, sizeof(gflin_a1_key->srv_cls_kind));
    cobolization(gflin_a1_key->srv_cls_num, server_class_num, sizeof(gflin_a1_key->srv_cls_num));
}
