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
/*                               コンポーネントスレッドの生成・管理・操作    */
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
#include <string.h> nolist
/* USER HEADER     */
#include <errcd.h> nolist
#include "GFPCVX20_cmp_odst_proc.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_thread_factory.h" nolist
#include "GFPCVX20_thread_header.h" nolist

/****************************************************************************/
/*   グローバル変数定義                                                     */
/****************************************************************************/
// スレッド管理
cncl_thread_factory_t cncl_thread_factory;

/*****************************************************************************/
/*  FUNCTION        :cncl_initial_thread_factory                             */
/*  CALLING SEQ.    :void cncl_initial_thread_factory(void)                  */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :スレッドファクトリ構造体を初期化し使用可能にする        */
/*****************************************************************************/
void cncl_initial_thread_factory()
{
    memset(&cncl_thread_factory, 0, sizeof(cncl_thread_factory_t));
}
/*****************************************************************************/
/*  FUNCTION        :cncl_set_conf_ind_thread_factory                        */
/*  CALLING SEQ.    :void cncl_set_conf_ind_thread_factory(sel_conf_ind_t    */
/*                   ind)                                                    */
/*  ARGUMENT        :ind:選択中のコンフィグインデックス                      */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :スレッドファクトリに現在のコンフィグインデックスを      */
/*                  :設定する                                                */
/*                  :※仕様変更により設定されているが現在使用してはいない    */
/*****************************************************************************/
void cncl_set_conf_ind_thread_factory(sel_conf_ind_t ind)
{
    cncl_thread_factory.crt_cfg_ind = ind;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_search_trans_thread                                */
/*  CALLING SEQ.    :thread_object_t *cncl_search_trans_thread(              */
/*                   gflin_pkey_def *recv_con_id)                            */
/*  ARGUMENT        :recv_con_id:検索対象の回線キー                          */
/*  RETURN CODE     :該当するtransportスレッドのポインタ,NULL(未発見時)      */
/*  DESCRIPTION     :渡された回線キーに合致するtransportスレッドを探す       */
/*****************************************************************************/
thread_object_t *cncl_search_trans_thread(gflin_pkey_def *recv_con_id)
{
    thread_object_t *thread;
    cmp_trans_t     *cmp_trans;

    // 台帳に登録されたスレッドが一つも存在しなければNULLを返す。
    if (cncl_thread_factory.trans.count == 0) return NULL;

    //トランスポートスレッド管理台帳に記載されたスレッドを先頭から列挙し、
    //スレッドから取得したトランスポートコンポーネントの回線情報が指定されたキーと
    // 一致するときスレッドオブジェクトを返す。
    thread = cncl_thread_factory.trans.top;
    while (thread) {
        cmp_trans = cncl_get_component(thread);
        if (memcmp(&(cmp_trans->gflin.pri_key), recv_con_id, sizeof(gflin_pkey_def)) == 0) {
            return thread;
        }
        thread = thread->header.next;
    }
    return NULL;
}
/****************************************************************************/
/*  FUNCTION        : cncl_col_trans_thread                                 */
/*  CALLING SEQ.    : bool cncl_col_trans_thread                            */
/*                           (gflin_pkey_def *recv_con_id,                  */
/*                            size_t key_length,                            */
/*                            thread_object_t **collection,                 */
/*                            long *item)                                   */
/*                                                                          */
/*  ARGUMENT        : recv_con_id [in]                                      */
/*                      検索対象の接続識別子（gflinの主キー）               */
/*                  : key_length [in]                                       */
/*                      主キー比較に使用するバイト長                        */
/*                  : collection [out]                                      */
/*                      一致したスレッドオブジェクトの格納先ポインタ配列    */
/*                  : item [out]                                            */
/*                      collectionに格納されたスレッド数                    */
/*                                                                          */
/*  RETURN CODE     : true  - 正常終了                                      */
/*                  : false - 引数不正（collectionまたはitemがNULL）        */
/*                                                                          */
/*  DESCRIPTION     : スレッドファクトリに存在するトランスポートスレッドの  */
/*                    中から、指定された接続IDと構成情報に一致するスレッドを*/
/*                   収集し、collectionに格納する。                         */
/*                    検索結果の件数はitemに格納される。                    */
/*                    ファクトリ内のスレッドが0件の場合はtrueを返す。       */
/****************************************************************************/
bool cncl_col_trans_thread(const gflin_pkey_def *recv_con_id, size_t key_length,thread_object_t **collection, long *item)
{
    thread_object_t *thread;
    cmp_trans_t     *cmp_trans;
    if(!collection || !item) return false;
    *item = 0;
    // 台帳に登録されたスレッドが一つも存在しなければitem数0件でtrueを返す。
    if (cncl_thread_factory.trans.count == 0) {
        return true;
    }
    //トランスポートスレッド管理台帳に記載されたスレッドを先頭から列挙し、
    //スレッドから取得したトランスポートコンポーネントの回線情報が指定されたキーと
    // 一致するときスレッドオブジェクトをcollectionに格納する。
    thread = cncl_thread_factory.trans.top;
    while (thread) {
        cmp_trans = cncl_get_component(thread);
        if (memcmp(&(cmp_trans->gflin.pri_key), recv_con_id, key_length) == 0) {
            *collection = thread;
            collection++;
            (*item)++;
        }
        thread = thread->header.next;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :alloc_thread                                            */
/*  CALLING SEQ.    :thread_object_t *alloc_thread(                          */
/*                   component_id_t component_id, size_t index)              */
/*  ARGUMENT        :component_id:生成するコンポーネント種別                 */
/*                  :index       :thread_buffer内インデックス                */
/*  RETURN CODE     :初期化したスレッドオブジェクトのポインタ                */
/*  DESCRIPTION     :指定したページを初期化し、管理台帳に登録する            */
/*****************************************************************************/
thread_object_t *alloc_thread(component_id_t component_id, size_t index)
{
    thread_object_t  *alloc_object;
    thread_object_t  *pos;
    thread_container *container;

    alloc_object = &(cncl_thread_factory.thread_buffer[index]);
    memset(alloc_object, 0, sizeof(thread_object_t));
    alloc_object->header.allocated     = true;
    alloc_object->header.component_id  = component_id;
    alloc_object->header.thread        = (short)index;
    alloc_object->header.selected_conf = cncl_thread_factory.crt_cfg_ind;
    switch (component_id) {
        case transport:
            container = &(cncl_thread_factory.trans);
            break;
        case sender:
            container = &(cncl_thread_factory.snd);
            break;
        case receiver:
            container = &(cncl_thread_factory.rcv);
            break;
        case notif_out_dist_procs:
            container = &(cncl_thread_factory.ntf_obound);
            break;
        case ipc_interface:
            container = &(cncl_thread_factory.ipc_if);
            break;
        case out_cmd_srvcls:
            container = &(cncl_thread_factory.otg_cmd);
            break;
        case out_dist_proc:
            container = &(cncl_thread_factory.odst_proc);
            break;
    };
    if (container->top == NULL) {
        container->top            = alloc_object;
        alloc_object->header.next = NULL;
        alloc_object->header.prev = NULL;
    } else {
        pos = container->top;
        while (pos->header.next) {
            pos = pos->header.next;
        }
        pos->header.next          = alloc_object;
        alloc_object->header.next = NULL;
        alloc_object->header.prev = pos;
    }
    container->count++;
    cncl_thread_factory.activethread++;
    alloc_object->container = container;
    return alloc_object;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_create_thread                                      */
/*  CALLING SEQ.    :thread_object_t *cncl_create_thread(                    */
/*                   component_id_t component_id)                            */
/*  ARGUMENT        :component_id:生成するコンポーネント種別                 */
/*  RETURN CODE     :生成したスレッドオブジェクトポインタ,NULL(枯渇時)       */
/*  DESCRIPTION     :指定されたコンポーネント種別のスレッドを作成            */
/*****************************************************************************/
thread_object_t *cncl_create_thread(component_id_t component_id)
{
    size_t index = 0;
    while (index < DEF_MAX_THREAD) {
        if (cncl_thread_factory.thread_buffer[index].header.allocated == false)
            return alloc_thread(component_id, index);
        index++;
    }
    return NULL;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_delete_thread                                      */
/*  CALLING SEQ.    :void cncl_delete_thread(thread_object_t *thread_object) */
/*  ARGUMENT        :thread_object:削除対象のスレッド                        */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :スレッドを所属するコンテナリストから切り離し開放        */
/*****************************************************************************/
void cncl_delete_thread(thread_object_t *current)
{
    thread_container *container = current->container;
    thread_object_t  *prev, *next;

    cncl_thread_factory.activethread--;
    container->count--;
    next = current->header.next;
    prev = current->header.prev;
    //削除対象(current)の一つ前(prev)が有るならば
    if (prev){
        //prev->next = current->next;
        prev->header.next = next;
        //current->nextがNULLでなければnext->prev = current->prev
        if(next) next->header.prev = prev;
    }
    else{
        //prev==NULLならばtopなのでtop=current->next
        container->top = next;
        //nextがnullでなければnext->prevはNULL
        if(next) next->header.prev = NULL;
    }
    current->header.allocated = false;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_component                                      */
/*  CALLING SEQ.    :void *cncl_get_component(thread_object_t *thread_object)*/
/*  ARGUMENT        :thread_object:対象スレッド                              */
/*  RETURN CODE     :コンポーネント領域                                      */
/*  DESCRIPTION     :スレッドに紐づくcmp_trans_t等の領域を返す               */
/*****************************************************************************/
void *cncl_get_component(thread_object_t *thread_object)
{
    if (!thread_object) return NULL;
    return &(thread_object->comp);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_threadInfo                                     */
/*  CALLING SEQ.    :thread_header_t *cncl_get_threadInfo(                   */
/*                   thread_object_t *thread_object)                         */
/*  ARGUMENT        :thread_object:対象スレッド                              */
/*  RETURN CODE     :スレッドヘッダ情報へのポインタ                          */
/*  DESCRIPTION     :指定スレッドの制御ブロック(thread_header_t)を取得       */
/*****************************************************************************/
thread_header_t *cncl_get_threadInfo(thread_object_t *thread_object)
{
    if (!thread_object) return NULL;
    return &(thread_object->header);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_max_transport                                      */
/*  CALLING SEQ.    :size_t cncl_max_transport(size_t new_max)               */
/*  ARGUMENT        :new_max:新たに設定する最大transport数                   */
/*  RETURN CODE     :現在の最大transport数                                   */
/*  DESCRIPTION     :transportスレッド数の上限取得/設定を行う                */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    size_t
    cncl_max_transport(size_t new_max)
{
    if (_arg_present(new_max)) {
        // 今はなにもしない
    }
    return DEF_MAX_TRANSMISSION;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_current_transport                                  */
/*  CALLING SEQ.    :size_t cncl_current_transport(void)                     */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :現在生成されているtransportスレッドの数                 */
/*  DESCRIPTION     :transportスレッドの現アクティブ数を返す                 */
/*****************************************************************************/
size_t cncl_current_transport()
{
    return cncl_thread_factory.trans.count;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_get_ntf_odst                                       */
/*  CALLING SEQ.    :size_t cncl_current_transport(void)                     */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :cmp_ntf_odst_procs_tの唯一のインスタンスを返す          */
/*  DESCRIPTION     :cmp_ntf_odst_procs_tの唯一のインスタンスを返す          */
/*****************************************************************************/
thread_object_t *cncl_get_ntf_odst()
{
    if (cncl_thread_factory.ntf_obound.count == 0) {
        cncl_ems_procedure_error("cncl_get_ntf_odst", 0, DEF_NERR_SYSIF_LGC_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
    }
    return cncl_thread_factory.ntf_obound.top;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_search_odst_proc_thread                            */
/*  CALLING SEQ.    :thread_object_t *cncl_search_odst_proc_thread(          */
/*                   short f_num)                                            */
/*  ARGUMENT        :f_num:ODSTプロセスのファイル番号                        */
/*  RETURN CODE     :対応するスレッドオブジェクトのポインタ,                 */
/*                   見つからなければNULL                                    */
/*  DESCRIPTION     :ファイル番号に対応するODSTプロセス用スレッドを検索する  */
/*                   ・odst_proc台帳にスレッドが存在しなければNULLを返す     */
/*                   ・台帳リストを走査し、out_dist_procs_f_num一致時に返却  */
/*                   ・一致するものがなければNULLを返却                      */
/*****************************************************************************/
thread_object_t *cncl_search_odst_proc_thread(short f_num)
{
    thread_object_t *thread;
    cmp_odst_proc_t *odst_proc;

    // 台帳に登録されたスレッドが一つも存在しなければNULLを返す。
    if (cncl_thread_factory.odst_proc.count == 0) return NULL;

    // 所定のスレッドを返す。
    thread = cncl_thread_factory.odst_proc.top;
    while (thread) {
        odst_proc = cncl_get_component(thread);
        if (odst_proc->out_dist_procs_f_num == f_num) {
            return thread;
        }
        thread = thread->header.next;
    }
    return NULL;
}
