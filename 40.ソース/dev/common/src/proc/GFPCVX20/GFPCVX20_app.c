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
#include <cextdecs.h> nolist
#include <string.h> nolist
#include <tal.h> nolist
/* USER HEADER     */
#include <common.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_cmp_ipc_interface.h" nolist
#include "GFPCVX20_cmp_ntf_odst_procs.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_config_info.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_gclst_info.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_util.h" nolist
#include "GFPCVX20_thread_factory.h" nolist
#include "GFPCVX20_thread_header.h" nolist


/****************************************************************************/
/*   グローバル変数定義                                                     */
/****************************************************************************/
/* アプリケーションオブジェクト */
Application App;
/*****************************************************************************/
/*  FUNCTION        :cncl_get_App                                           */
/*  CALLING SEQ.    :Application* cncl_get_App(void)                        */
/*  ARGUMENT        :なし                                                   */
/*  RETURN CODE     :アプリケーション情報へのポインタ                       */
/*  DESCRIPTION     :グローバルアプリケーション構造体を取得する             */
/****************************************************************************/
Application *cncl_get_App()
{
    return &App;
}
/****************************************************************************/
/*  FUNCTION        :cncl_get_nwcnf                                         */
/*  CALLING SEQ.    :nw_conf_t* cncl_get_nwcnf(void)                        */
/*  ARGUMENT        :なし                                                   */
/*  RETURN CODE     :ネットワーク設定情報へのポインタ                       */
/*  DESCRIPTION     :アプリケーション内のnw_confを取得する                  */
/****************************************************************************/
nw_conf_t *cncl_get_nwcnf()
{
    return App.nw_conf;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_eventlist                                      */
/*  CALLING SEQ.    :Event_list_t* cncl_get_eventlist(void)                  */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :イベントリストへのポインタ                              */
/*  DESCRIPTION     :アプリケーション内のイベントリストを取得                */
/*****************************************************************************/
Event_list_t *cncl_get_eventlist()
{
    return &App.event_list;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_initialize_application                             */
/*  CALLING SEQ.    :bool cncl_initialize_application(void)                  */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :true(正常),false(エラー時)                              */
/*  DESCRIPTION     :アプリケーション構造体を初期化しイベントリストを        */
/*                  :用意する                                                */
/*****************************************************************************/
bool cncl_initialize_application()
{
    Application  *app        = cncl_get_App();
    Event_list_t *event_list = &(cncl_get_App()->event_list);

    memset(app, 0, sizeof(Application));
    app->activethread = &cncl_thread_factory.activethread;
    initial_event_list(event_list);

    return true;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_initial                                         */
/*  CALLING SEQ.    :short cncl_initial(void)                             */
/*  ARGUMENT        :なし                                                 */
/*  RETURN CODE     :0,エラーコード                                       */
/*  DESCRIPTION     :初期化処理を行い、コンフィグ情報を読み込んで必要な    */
/*                  :スレッドを生成する                                   */
/*****************************************************************************/
short cncl_initial()
{
    io_trace_buf_t   trace_if;
    Application     *app        = cncl_get_App();
    myinfo_def      *my_info    = &(app->my_info);
    gclst_info_t    *gclst_info = &(app->gclst_info);
    db_gflin_def    *gflin;
    db_gfnwi_def    *gfnwi;
    thread_object_t *thread_trans;

    cncl_initialize_application();

    // メモリ管理をゼロクリアする。
    memset(&io_mem, 0, sizeof(io_mem_t));

    // トレース出力モジュール初期化
    memset((char *)&trace_if, ' ', sizeof(io_trace_buf_t));
    trace_if.func_flg = DEF_TRACE_FUNC_INI;
    memcpy(trace_if.trace_info.prog_id, DEF_MY_PROGID, sizeof(DEF_MY_PROGID) - 1);
    TRACEOUT((char *)&trace_if);

    /* サーバコンフィグ・プロセス情報などリロード不可情報の取得 */
    cncl_initial_myinfo(my_info);

    initial_ems(DEF_MY_PROGID, my_info->srv_clsId.network, my_info->uytrmmon, my_info->uytrmmonlen, my_info->uytrmsrv,
                my_info->uytrmsrvlen, my_info->uytrmtimer);

    /* マスタから取得するリロード可能な情報の取得 */
    app->nw_conf = cncl_load_config(nw_conf_factory);
    if (app->nw_conf == NULL) {
        cncl_ems_abnormal_end(DEF_PROC_NORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }

    if (!cncl_chk_nw_conf(app->nw_conf)) {
        cncl_ems_abnormal_end(DEF_PROC_NORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }

    if(!initial_io_mem(&io_mem,app->nw_conf->gflin_count)){
        cncl_ems_procedure_error("initial_io_mem", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }


    cncl_set_conf_ind_thread_factory(app->nw_conf->conf_ind);
    cncl_gclst_info_open(gclst_info);

    // TODO: unitは仮置き
    cncl_create_ntf_odst_procs(app->nw_conf->out_dist_procs, app->nw_conf->out_dist_procs_count);

    for (int gflin_index = 0; gflin_index < app->nw_conf->gflin_count; gflin_index++) {
        gflin        = &(app->nw_conf->gflin[gflin_index]);
        thread_trans = NULL;
        for (int gfnwi_index = 0; gfnwi_index < app->nw_conf->gfnwi_count; gfnwi_index++) {
            gfnwi = &(app->nw_conf->gfnwi[gfnwi_index]);
            if (memcmp(gfnwi, gflin, sizeof(gfnwi->pri_key)) == 0) {
                thread_trans = cncl_create_trans(gflin, gfnwi, &(app->nw_conf->in_dist_srvcls),
                                                 &(app->nw_conf->cmd_if_srvcls), &(gclst_info->gclst_io));
                break;
            }
        }
        if (!thread_trans) {
            // ems
            cncl_ems_config_error(gflin, sizeof(gflin->pri_key), DEF_FL_LIN_MG, DEF_NERR_PRM_RD_ERR);
        }
    }

    thread_object_t *thread_income = cncl_create_ipc();
    cncl_ipc_start(thread_income);
    cncl_ems_process_start();
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_business                                           */
/*  CALLING SEQ.    :short cncl_business(void)                               */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :stopwaitになるまでイベントを処理し続ける                */
/*****************************************************************************/
short cncl_business()
{
    short        ret = 0;
    Application *app = cncl_get_App();
    while (!(app->stopwait) && app->activethread != 0) {
        ret = cncl_event_proc();
    }
    return ret;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_finish                                             */
/*  CALLING SEQ.    :short cncl_finish(void)                                 */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :終了処理を行い、正常終了を通知する                      */
/*****************************************************************************/
short cncl_finish()
{
    cncl_ems_normal_end();
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_load_config_request                                */
/*  CALLING SEQ.    :short cncl_load_config_request(                         */
/*                   gflin_pkey_def *gflin_pkey,                             */
/*                   thread_object_t *thread_trans_old)                      */
/*  ARGUMENT        :gflin_pkey       :回線情報の主キー                      */
/*                  :thread_trans_old :既存回線スレッドのポインタ            */
/*  RETURN CODE     :0(成功),-1(失敗)                                        */
/*  DESCRIPTION     :マスタからコンフィグを再取得し、該当回線を更新または    */
/*                  :追加する                                                */
/*  UPDATE          :2025/07/02 廃止                                         */
/*****************************************************************************/
short cncl_load_config_request(gflin_pkey_def *gflin_pkey, thread_object_t *thread_trans_old)
{
    Application  *app        = cncl_get_App();
    gclst_info_t *gclst_info = &(app->gclst_info);
    db_gflin_def *gflin;
    db_gfnwi_def *gfnwi;
    nw_conf_t *new;
    thread_object_t *thread_trans = NULL;

    // 回線管理ファイルは変更しない。

    /* マスタから取得するリロード可能な情報の取得 */
    new                           = cncl_load_config(nw_conf_factory);
    if (new == NULL) {
        // TODO:要EMS
        return -1;
    }
    app->nw_conf = new;
    cncl_set_conf_ind_thread_factory(app->nw_conf->conf_ind);

    // 指定された回線情報を探す
    for (int gflin_index = 0; gflin_index < app->nw_conf->gflin_count; gflin_index++) {
        gflin = &(app->nw_conf->gflin[gflin_index]);
        if (memcmp(&(gflin->pri_key), gflin_pkey, sizeof(gflin_pkey_def)) == 0) {
            if (!cncl_chk_gflin_rec(gflin)) {
                // レコード内容正常
                break;
            }
        }
        gflin = NULL;
    }
    if (!gflin) {
        // 回線情報なし
        if (thread_trans_old) {
            // 回線スレッド削除
            cncl_delete_trans(thread_trans_old);
            return 0;
        }
        // 追加対象回線情報なし
        cncl_ems_config_error(gflin_pkey, sizeof(gflin_pkey_def), DEF_FL_LIN_MG, DEF_NERR_PRM_RD_ERR);
        return -1;
    }
    // gflinに対応するNW情報を検索し新スレッドを開始する。
    for (int gfnwi_index = 0; gfnwi_index < app->nw_conf->gfnwi_count; gfnwi_index++) {
        gfnwi = &(app->nw_conf->gfnwi[gfnwi_index]);
        if (memcmp(gfnwi, gflin, sizeof(gfnwi->pri_key)) == 0) {
            if (!cncl_chk_gfnwi_rec(gfnwi)) {
                // 旧回線スレッド削除
                cncl_delete_trans(thread_trans_old);
                thread_trans = cncl_create_trans(gflin, gfnwi, &(app->nw_conf->in_dist_srvcls),
                                                 &(app->nw_conf->cmd_if_srvcls), &(gclst_info->gclst_io));
                break;
            }
        }
    }
    if (!thread_trans) {
        // NW情報なし
        cncl_ems_config_error(gflin, sizeof(gflin->pri_key), DEF_FL_NW_INFO, DEF_NERR_PRM_RD_ERR);
        return -1;
    }
    return 0;
}
/****************************************************************************/
/*  FUNCTION        : cncl_get_gflin_part_key_length                        */
/*  CALLING SEQ.    : size_t cncl_get_gflin_part_key_length                 */
/*                           (gflin_pkey_def *gflin_pkey)                   */
/*                                                                          */
/*  ARGUMENT        : gflin_pkey [in]                                       */
/*                      キー情報構造体（gflin_pkey_def型）へのポインタ      */
/*                                                                          */
/*  RETURN CODE     : キー長（station_nameまでまたはconnection_nameまで）   */
/*                                                                          */
/*  DESCRIPTION     : gflinの主キー構造体から部分キー長を取得する。         */
/*                    station_nameが空白以外で指定されているかを確認し、    */
/*                    指定がある場合はconnection_nameまでを部分キーとする。 */
/*                    指定がなければstation_name直前までを部分キーとする。  */
/****************************************************************************/
size_t cncl_get_gflin_part_key_length(gflin_pkey_def *gflin_pkey)
{
    gflin_pkey_def gflin_part_key;
    size_t         gflin_part_key_length = gflin_part_key.station_name - (char *)&gflin_part_key;
    // station_nameが指定されているか確認し、指定されている場合は、キー長をステーション識別まで拡張する。
    for (int index = 0; index < (int)sizeof(gflin_pkey->station_name); index++) {
        if (gflin_pkey->station_name[index] != ' ') {
            gflin_part_key_length = gflin_part_key.connection_name - (char *)&gflin_part_key;
            break;
        }
    }
    return gflin_part_key_length;
}
/****************************************************************************/
/*  FUNCTION        : cncl_load_config_collection_request                   */
/*  CALLING SEQ.    : short cncl_load_config_collection_request             */
/*                           (gflin_pkey_def *gflin_pkey)                   */
/*                                                                          */
/*  ARGUMENT        : gflin_pkey [in]                                       */
/*                      回線識別キー構造体（部分一致検索用）                */
/*                                                                          */
/*  RETURN CODE     :  0   - 構成更新成功                                   */
/*                  : -1   - エラー（構成不整合、リソース不足等）           */
/*                  : -2   - エラー（指定した回線が存在しない）             */
/*                                                                          */
/*  DESCRIPTION     : 指定されたgflin主キーに一致するトランスポート情報を   */
/*                    最新の構成マスタより再構築する。                      */
/*                                                                          */
/*                    1. 対象となるgflinを取得し、該当する既存の            */
/*                       トランスポートが全て切断状態であることを確認       */
/*                    2. 最新の構成情報を読み込み、指定gflinに一致する      */
/*                       情報のみを抽出・検証（gfnwi含む）                  */
/*                    3. 構成整合性と最大トランスポート数を検証             */
/*                    4. 有効な新トランスポートを生成、既存の該当する       */
/*                       トランスポートは削除                               */
/*                                                                          */
/*                    ※一連の処理中でいずれかのエラーが発生した場合は、    */
/*                      処理を中断してエラーコードを返却                    */
/****************************************************************************/
short cncl_load_config_collection_request(gflin_pkey_def *gflin_pkey)
{
    Application        *app        = cncl_get_App();
    gclst_info_t       *gclst_info = &(app->gclst_info);
    db_gflin_def       *gflin;
    const db_gfnwi_def *gfnwi;
    nw_conf_t *new;
    // gflin_part_key_length をインターフェース識別までの長さで初期化する。
    size_t           gflin_part_key_length = cncl_get_gflin_part_key_length(gflin_pkey);

    thread_object_t *old_specified_trans_collection
        [cncl_current_transport()];                               // 構成変更対象の既存のtransportを格納するコレクション
    size_t old_total_trans_count     = cncl_current_transport();  // 現在の総接続数(transport数)
    long   old_specified_trans_count = 0;                         //  構成変更対象の既存のtransportの数

    // 既存のトランスポートからコマンドで選択された回線を列挙し、old_specified_trans_collectionに格納する。
    cncl_col_trans_thread(gflin_pkey, gflin_part_key_length, old_specified_trans_collection,
                          &old_specified_trans_count);
    // old_specified_trans_collectionに格納した構成変更対象のtransportの回線ステータスが全て切断状態であることをチェックする。
    for (long index = 0; index < old_specified_trans_count; index++) {
        cmp_trans_t *cmp_trans = cncl_get_component(old_specified_trans_collection[index]);
        if (cmp_trans->line_status != e_line_disconnected) return -1;
    }

    /* マスタからリロード可能な情報の再取得を行う */
    new = cncl_load_config(nw_conf_factory);
    if (new == NULL) {
        // TODO:要EMS
        return -1;
    }
    app->nw_conf = new;
    cncl_set_conf_ind_thread_factory(app->nw_conf->conf_ind);

    // 更新されたgflinから変更対象となるレコードのみを取得し、new_gflin_if_collectionに格納する。
    // 各レコードの精査及び、回線に対応するgfnwiの検索と該当するgfnwiの精査を行う。
    // レコードのデータ不良または、対応するgfnwiのレコードに不備が有る場合はエラーとする。
    db_gflin_def *new_gflin_if_collection[app->nw_conf->gflin_count];
    long          new_gflin_if_count = 0;
    for (int gflin_index = 0; gflin_index < app->nw_conf->gflin_count; gflin_index++) {
        gflin = &(app->nw_conf->gflin[gflin_index]);
        if (memcmp(&(gflin->pri_key), gflin_pkey, gflin_part_key_length) == 0) {
            if (!cncl_chk_gflin_rec(gflin)) {
                gfnwi = cncl_search_gfnwi(gflin);
                // 対応するgfnwiがない=回線を起動出来ない。
                if (!gfnwi) {
                    cncl_ems_config_error(gflin, sizeof(gflin->pri_key), DEF_FL_LIN_MG, DEF_NERR_PRM_RD_ERR);
                    return -1;
                }
                // 更新された(かもしれない)gfnwiをチェックする。
                if (cncl_chk_gfnwi_rec(gfnwi)) {
                    // gfnwiのエラーメッセージは関数内で出力されているので問題なし。
                    return -1;
                }
                new_gflin_if_collection[gflin_index] = gflin;
                new_gflin_if_count++;
            } else {
                // cncl_chk_gflin_rec内でパラメータエラーは出力されるので、ここではエラーだけ返せば良い。
                return -1;
            }
        }
    }
    // 指定した回線は既存になく(削除対象でもない)かつ新規回線にもない
    if (new_gflin_if_count == 0 && old_specified_trans_count == 0) {
        return -2;
    }

    // 変更対象の更新された回線数から既存の指定された回線を引いた差分を現在の回線数に加算(減算)した値が
    // 更新された回線数(スレッド数)
    long new_transport_count = old_total_trans_count + new_gflin_if_count - old_specified_trans_count;
    // 更新された回線数が最大回線数を超過していない場合は処理を継続する
    if (new_transport_count > (long)cncl_max_transport(no_param)) return -1;

    // 精査済のレコードを使用してtransportを構築していく
    thread_object_t *new_trans_collection[app->nw_conf->gflin_count];
    memset(new_trans_collection, 0, sizeof(new_trans_collection));
    for (int gflin_index = 0; gflin_index < new_gflin_if_count; gflin_index++) {
        gflin = new_gflin_if_collection[gflin_index];
        gfnwi = cncl_search_gfnwi(gflin);
        new_trans_collection[gflin_index] =
            cncl_create_trans(gflin, (db_gfnwi_def *)gfnwi, &(app->nw_conf->in_dist_srvcls),
                              &(app->nw_conf->cmd_if_srvcls), &(gclst_info->gclst_io));
        if (!new_trans_collection[gflin_index]) {
            int index = 0;
            while (new_trans_collection[index]) {
                cncl_delete_trans(new_trans_collection[index]);
                index++;
            }
            cncl_ems_config_error(gflin, sizeof(gflin->pri_key), DEF_FL_LIN_MG, DEF_NERR_PRM_RD_ERR);
            return -1;
        }
    }
    // 旧回線のスレッドを削除する。
    for (long index = 0; index < old_specified_trans_count; index++) {
        cncl_delete_trans(old_specified_trans_collection[index]);
    }
    return 0;
}
/****************************************************************************/
/*  FUNCTION        : cncl_search_gfnwi                                     */
/*  CALLING SEQ.    : const db_gfnwi_def* cncl_search_gfnwi                 */
/*                           (const db_gflin_def *gflin)                    */
/*                                                                          */
/*  ARGUMENT        : gflin [in]                                            */
/*                      回線情報レコード（gflin構造体ポインタ）             */
/*                                                                          */
/*  RETURN CODE     : gfnwiへのポインタ - 一致するNWインターフェース定義有  */
/*                  : NULL             - 一致なし                           */
/*                                                                          */
/*  DESCRIPTION     : gflinに対応するネットワークインターフェース情報       */
/*                    (gfnwi) を構成情報(app->nw_conf)から検索し、          */
/*                    対応するgfnwiエントリを返す。                         */
/*                                                                          */
/*                    比較はgfnwi->pri_key相当のサイズ分をメモリ比較し、    */
/*                    一致した最初のエントリを返却する。                    */
/****************************************************************************/
const db_gfnwi_def* cncl_search_gfnwi(const db_gflin_def * gflin)
{
    Application  *app   = cncl_get_App();
    db_gfnwi_def *gfnwi = NULL;
    for (int gfnwi_index = 0; gfnwi_index < app->nw_conf->gfnwi_count; gfnwi_index++) {
        gfnwi = &(app->nw_conf->gfnwi[gfnwi_index]);
        if (memcmp(gfnwi, gflin, sizeof(gfnwi->pri_key)) == 0) {
            return gfnwi;
        }
    }
    return NULL;
}
