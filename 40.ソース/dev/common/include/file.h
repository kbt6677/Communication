/* SCHEMA PRODUCED DATE - TIME : 6/19/2025 - 18:17:34 */
#pragma section db_gflin
/* Record DB-GFLIN created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gflin
typedef struct __db_gflin
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      struct
      {
         char                            srv_cls_kind[8];
         char                            srv_cls_num[4];
      } srv_cls_id;
   } alt1_key_info;
   struct
   {
      char                            lst_connect_id[6];
   } alt2_key_info;
   char                            tcpip_prc_name[6];
   char                            ip_adress_src[15];
   char                            port_num_src[5];
   char                            ip_adress_dst[15];
   char                            port_num_dst[5];
   char                            invalid_flg;
   char                            update_date[8];
   char                            operation_id;
   char                            future_use[62];
} db_gflin_def;
#define db_gflin_def_Size 160
#pragma section db_gclst
/* Record DB-GCLST created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gclst
typedef struct __db_gclst
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      char                            connect_sts[2];
      char                            connect_sts_update_time[20];
   } connect_sts_info;
   struct
   {
      char                            prc_sts[2];
      char                            prc_sts_update_time[14];
   } prc_sts_info;
   struct
   {
      char                            ip_adress_src[15];
      char                            port_num_src[5];
      char                            ip_adress_dst[15];
      char                            port_num_dst[5];
      char                            err_code[4];
      char                            disconnect_rsn[2];
   } connect_info;
   char                            future_use[56];
} db_gclst_def;
#define db_gclst_def_Size 164
#pragma section db_gcsst
/* Record DB-GCSST created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gcsst
typedef struct __db_gcsst
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      char                            nw_if[20];
      char                            nw_station[11];
      char                            future_use[39];
   } nw_id_info;
   struct
   {
      char                            state_sts[2];
      char                            state_sts_update_time[14];
      char                            open_state_retry_num[8];
      char                            future_use[24];
   } state_sts_info;
   char                            future_use[58];
} db_gcsst_def;
#define db_gcsst_def_Size 200
#pragma section db_gcest
/* Record DB-GCEST created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gcest
typedef struct __db_gcest
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      struct
      {
         char                            last_echo_start_time[14];
         char                            last_echo_end_time[14];
         char                            last_echo_result[2];
         char                            last_echo_ok_time[14];
      } cbs_echo_info;
      struct
      {
         char                            last_echo_req_recv_time[14];
         char                            last_echo_result[2];
         char                            last_echo_ok_time[14];
      } dst_echo_info;
   } echo_info;
   char                            future_use[42];
} db_gcest_def;
#define db_gcest_def_Size 140
#pragma section db_gccut
/* Record DB-GCCUT created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gccut
typedef struct __db_gccut
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      char                            bh_mch_key[20];
      char                            future_use[25];
   } nw_idv_info;
   struct
   {
      char                            cut_date[8];
      char                            cut_update_time[14];
      char                            last_cut_date[8];
      char                            last_cut_update_time[14];
      char                            future_use[20];
   } cut_date_info;
   char                            future_use[39];
} db_gccut_def;
#define db_gccut_def_Size 172
#pragma section db_glnlg
/* Record DB-GLNLG created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_glnlg
typedef struct __db_glnlg
{
   struct
   {
      char                            part_id[2];
      struct
      {
         char                            time_stamp[20];
         char                            time_stamp_branch[16];
      } tushin_denbun_id;
   } pri_key;
   char                            send_recv_id;
   char                            send_recv_denbun_len[5];
   char                            mti_id[4];
   char                            msg_shubetu[2];
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } connect_id;
   struct
   {
      char                            furiwake_dst;
      char                            furiwake_kubun[2];
      char                            future_use[7];
   } furiwake_info;
   char                            future_use[26];
   struct
   {
      char                            denbun_recv_time[20];
      char                            recv_kyoku_sts;
      char                            nw_kubun[2];
      char                            mti_id[4];
      char                            send_denbun_shubetu;
      struct
      {
         struct
         {
            char                            gfp_lcn_id[15];
            char                            denbun_keitai;
         } tran_id;
         char                            denbun_shubetu;
         char                            re_send_num[3];
      } denbun_log_key;
      char                            denbun_fmt_kubun;
      char                            tushin_log_save_filename[36];
      struct
      {
         char                            part_id[2];
         char                            time_stamp[20];
         char                            tushin_unique_info[16];
      } tushin_log_key;
      char                            future_use[37];
   } denbun_send_recv_info;
   struct
   {
      char                            if_id[20];
      char                            station_id[11];
      struct
      {
         char                            recv_connect_id[24];
         struct
         {
            char                            ip_adress_src[15];
            char                            port_num_src[5];
            char                            ip_adress_dst[15];
            char                            port_num_dst[5];
         } recv_connect_info;
         struct
         {
            char                            time_stamp[20];
            char                            tushin_unique_info[16];
         } denbun_recv_time_stamp;
      } line_info;
      char                            future_use[70];
   } tushin_cntrl_info;
   struct
   {
      char                            denbun_len[5];
      char                            mti_start_lct[5];
      char                            denbun[9999];
   } denbun_area;
} db_glnlg_def;
#define db_glnlg_def_Size 10480
#pragma section db_glelg
/* Record DB-GLELG created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_glelg
typedef struct __db_glelg
{
   struct
   {
      char                            part_id[2];
      char                            time_stamp[20];
      char                            time_stamp_branch[16];
   } pri_key;
   char                            err_denbun_id;
   char                            mti_id[4];
   char                            res_code[6];
   char                            naibu_err_code[7];
   struct
   {
      char                            srv_cls_id[12];
      char                            srv_cls_mlt_num[4];
   } srv_cls_info;
   char                            future_use[28];
   struct
   {
      char                            denbun_recv_time[20];
      char                            recv_kyoku_sts;
      char                            nw_kubun[2];
      char                            mti_id[4];
      char                            send_denbun_shubetu;
      struct
      {
         struct
         {
            char                            lcn_id[15];
            char                            denbun_keitai;
         } tran_id;
         char                            denbun_shubetu;
         char                            re_send_num[3];
      } denbun_log_key;
      char                            denbun_fmt_kubun;
      char                            tushin_log_save_filename[36];
      struct
      {
         char                            part_id[2];
         char                            time_stamp[20];
         char                            tushin_unique_info[16];
      } tushin_log_key;
      char                            future_use[37];
   } denbun_send_recv_info;
   struct
   {
      char                            if_id[20];
      char                            station_id[11];
      struct
      {
         char                            recv_connect_id[24];
         struct
         {
            char                            ip_adress_src[15];
            char                            port_num_src[5];
            char                            ip_adress_dst[15];
            char                            port_num_dst[5];
         } recv_connect_info;
         struct
         {
            char                            time_stamp[20];
            char                            tushin_unique_info[16];
         } denbun_recv_time_stamp;
      } line_info;
      char                            future_use[70];
   } tushin_cntrl_info;
   struct
   {
      char                            denbun_len[5];
      char                            mti_start_lct[5];
      char                            denbun[10400];
   } denbun_area;
} db_glelg_def;
#define db_glelg_def_Size 10871
#pragma section db_glmlg
/* Record DB-GLMLG created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_glmlg
typedef struct __db_glmlg
{
   struct
   {
      char                            part_id[2];
      char                            lcn_id[15];
      char                            s_h_kubun;
      char                            send_recv_id;
   } pri_key;
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } connect_id;
   char                            mti_id[4];
   char                            naibu_err_code[7];
   struct
   {
      char                            srv_cls_id[12];
      char                            srv_cls_mlt_num[4];
   } cntrl_denbun_srv_cls_info;
   char                            control_kind[4];
   char                            send_naibu_err_code[7];
   char                            entry_timestamp[20];
   struct
   {
      char                            key_type[4];
      char                            future_use[16];
   } proc_result_info;
   char                            future_use[78];
   char                            denbun_info_exist;
   struct
   {
      char                            denbun_len[5];
      char                            denbun[9999];
   } denbun_area;
} db_glmlg_def;
#define db_glmlg_def_Size 10204
#pragma section db_gfmtl
/* Record DB-GFMTL created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfmtl
typedef struct __db_gfmtl
{
   struct
   {
      char                            part_id[2];
      char                            lcn_id[15];
   } pri_key;
   char                            s_h_kubun;
   char                            torihiki_sts[2];
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } torihiki_info;
   char                            naibu_err_id[7];
   char                            res_code[4];
   char                            req_res_match_key[50];
   char                            timer_info_key[20];
   char                            control_kind[4];
   char                            entry_timestamp[20];
   char                            timer_entry_kbn;
   char                            future_use[56];
} db_gfmtl_def;
#define db_gfmtl_def_Size 206
#pragma section db_gckey
/* Record DB-GCKEY created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gckey
typedef struct __db_gckey
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            key_type[4];
   } pri_key;
   struct
   {
      struct
      {
         char                            bh_mch_name[20];
         char                            key_type[4];
      } bh_mch_key;
      char                            future_use[25];
   } nw_idv_info;
   struct
   {
      char                            key_use_alg[3];
      char                            key_format[2];
      char                            exportability;
      struct
      {
         char                            key_value[74];
         char                            check_digit[16];
         char                            key_update_time[14];
         char                            future_use[26];
      } disp_key_info;
      struct
      {
         char                            key_value[74];
         char                            check_digit[16];
         char                            key_update_time[14];
         char                            future_use[26];
      } recv_key_info;
      struct
      {
         char                            key_value[74];
         char                            check_digit[16];
         char                            key_update_time[14];
         char                            future_use[26];
      } send_key_info;
      char                            new_key_index[2];
      struct
      {
         char                            key_value[74];
         char                            check_digit[16];
         char                            key_update_time[14];
         char                            future_use[26];
      } key_info_01;
      struct
      {
         char                            key_value[74];
         char                            check_digit[16];
         char                            key_update_time[14];
         char                            future_use[26];
      } key_info_02;
   } key_info;
   char                            future_use[71];
} db_gckey_def;
#define db_gckey_def_Size 800
#pragma section db_gfnsw
/* Record DB-GFNSW created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfnsw
typedef struct __db_gfnsw
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
   } pri_key;
   char                            furiwake_rate_tky[3];
   char                            furiwake_rate_osk[3];
   char                            future_use[37];
} db_gfnsw_def;
#define db_gfnsw_def_Size 50
#pragma section db_gfqsw
/* Record DB-GFQSW created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfqsw
typedef struct __db_gfqsw
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            mti_id[4];
   } pri_key;
   struct
   {
      char                            tky_osk_kubun;
      char                            denbun_shubetu;
      char                            future_use[8];
   } furiwake_cntrl_info;
   struct
   {
      struct
      {
         struct
         {
            char                            srv_cls_kind[8];
            char                            srv_cls_num[4];
         } srv_cls_id;
         struct
         {
            char                            file_kind[8];
            char                            file_num[4];
         } file_id;
         char                            future_use[16];
      } tky_site_info;
      struct
      {
         struct
         {
            char                            srv_cls_kind[8];
            char                            srv_cls_num[4];
         } srv_cls_id;
         struct
         {
            char                            file_kind[8];
            char                            file_num[4];
         } file_id;
         char                            future_use[16];
      } osk_site_info;
      char                            future_use[20];
   } recv_qfile_info;
   char                            future_use[79];
} db_gfqsw_def;
#define db_gfqsw_def_Size 200
#pragma section db_gfnwi
/* Record DB-GFNWI created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfnwi
typedef struct __db_gfnwi
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
   } pri_key;
   struct
   {
      char                            nw_kubun[2];
      char                            nw_if[20];
      char                            nw_station[11];
      char                            future_use[29];
   } nw_id_info;
   struct
   {
      char                            open_close_mng_lyr;
      char                            echo_test_mng_lyr;
      char                            key_cng_mng_lyr;
      char                            cut_over_mng_lyr;
      char                            saf_send_mng_lyr;
      char                            filler1;
      char                            connect_num_mng_lyr;
      char                            future_use[13];
   } mng_lyr_info;
   struct
   {
      char                            data_len_start_lct[5];
      char                            data_len_size[2];
      char                            data_len_attribute[3];
      char                            data_len_include_id;
      char                            denbun_start_lct[5];
      char                            mti_start_lct[5];
      char                            mti_item_len[2];
      char                            mti_item_attribute[3];
      char                            text_format_version;
      char                            future_use[23];
   } denbun_item_lct_info;
   struct
   {
      char                            connect_nxt_prc_kind[2];
      char                            spc_data[100];
      char                            future_use[48];
   } connect_nxt_prc_info;
   struct
   {
      char                            connect_num_mng_kind;
      char                            future_use[9];
   } connect_num_mng_info;
   struct
   {
      char                            send_re_select_need;
      char                            syogai_tuuchi_need;
      char                            admin_denbun_res_need;
      char                            trigger_open_need;
      char                            future_use[6];
   } shori_kbn_info;
   struct
   {
      char                            act_stb_id;
      char                            future_use[9];
   } env_set_info;
   struct
   {
      char                            connect_wait_tmr[8];
      char                            send_wait_tmr[8];
      char                            nxt_data_recv_wait[8];
      char                            non_comm_monitor[8];
      char                            line_fail_rtr_num_srt[8];
      char                            line_fail_rtr_num_lng[8];
      char                            line_fail_rtr_num_lst[8];
      char                            tmr_08[8];
      char                            tmr_09[8];
      char                            tmr_10[8];
   } trans_cntrl_tmr_info;
   struct
   {
      char                            line_fail_rtr_num_srt[8];
      char                            line_fail_rtr_num_lng[8];
      char                            line_fail_rtr_num_lst[8];
      char                            cnt_04[8];
      char                            cnt_05[8];
   } trans_cntrl_cnt_info;
   struct
   {
      char                            open_res_wait_tmr[8];
      char                            close_res_wait_tmr[8];
      char                            echo_test_res_wait_tmr[8];
      char                            key_cng_req_wait_tmr[8];
      char                            key_cng_res_wait_tmr[8];
      char                            cut_over_res_wait_tmr[8];
      char                            saf_start_end_res_tmr[8];
      char                            tmr_08[8];
      char                            tmr_09[8];
      char                            tmr_10[8];
   } cntrl_denbun_tmr_info;
   struct
   {
      char                            open_retry_num[8];
      char                            cnt_02[8];
      char                            cnt_03[8];
      char                            cnt_04[8];
      char                            cnt_05[8];
   } cntrl_denbun_cnt_info;
   struct
   {
      char                            rec_unit[4];
      char                            future_use[6];
   } gfnws_info;
   char                            dst_unq_info[100];
   char                            future_use[120];
} db_gfnwi_def;
#define db_gfnwi_def_Size 800
#pragma section db_gfnws
/* Record DB-GFNWS created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfnws
typedef struct __db_gfnws
{
   struct
   {
      char                            nw_id;
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   char                            dst_unq_info[200];
   char                            future_use[82];
} db_gfnws_def;
#define db_gfnws_def_Size 300
#pragma section db_gfphi
/* Record DB-GFPHI created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfphi
typedef struct __db_gfphi
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      struct
      {
         struct
         {
            char                            srv_cls_kind[8];
            char                            srv_cls_num[4];
         } srv_cls_id;
         char                            srv_cls_mlt_num[4];
      } srv_cls_key;
      struct
      {
         struct
         {
            char                            prc_file_kind[8];
            char                            prc_file_num[4];
         } prc_file_id;
         char                            prc_file_mlt_num[4];
      } prc_file_key;
   } pri_key;
   struct
   {
      char                            domain_name[8];
      char                            pathmon_name[16];
      char                            srv_cls_name[16];
   } srv_cls_info;
   struct
   {
      char                            prc_file_name[48];
   } prc_file_info;
   char                            invalid_flg;
   char                            update_date[8];
   struct
   {
      char                            lcn_num_min[4];
      char                            lcn_num_max[4];
   } lcn_num_scope;
   char                            future_use[96];
} db_gfphi_def;
#define db_gfphi_def_Size 240
#pragma section db_gcsaf
/* Record DB-GCSAF created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gcsaf
typedef struct __db_gcsaf
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
      char                            connect_id[6];
   } pri_key;
   struct
   {
      char                            center_id[11];
      char                            future_use[29];
   } nw_ind_info;
   struct
   {
      char                            saf_send_sts[2];
      char                            saf_send_update_time[14];
      char                            future_use[18];
   } saf_send_sts_info;
   char                            future_use[52];
} db_gcsaf_def;
#define db_gcsaf_def_Size 150
#pragma section db_gcscn
/* Record DB-GCSCN created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gcscn
typedef struct __db_gcscn
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            if_id[5];
      char                            station_id[6];
   } pri_key;
   struct
   {
      char                            max_connect_num[4];
      char                            crt_connect_num[4];
      char                            re_connect_sts[2];
      char                            re_connect_start_time[20];
      char                            re_connect_svr_cls_id[12];
      char                            future_use[10];
   } connect_num_ctrl_info;
   char                            future_use[58];
} db_gcscn_def;
#define db_gcscn_def_Size 128
#pragma section db_gfeli
/* Record DB-GFELI created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfeli
typedef struct __db_gfeli
{
   struct
   {
      char                            nw_id;
      char                            ipc_id;
      char                            mti_id[4];
      char                            bit_id[3];
      char                            filler_0;
   } pri_key;
   struct
   {
      char                            data_len_type;
      char                            data_len_size;
      char                            data_len_attribute;
      char                            data_area_attribute;
      char                            data_max_len[5];
      char                            future_use[31];
   } msg_fmt_info;
   struct
   {
      char                            tbl_start_offset[5];
      char                            data_area_size[5];
      char                            code_change_need;
      char                            future_use[29];
   } fix_fmt_info;
} db_gfeli_def;
#define db_gfeli_def_Size 90
#pragma section db_gfqbk
/* Record DB-GFQBK created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gfqbk
typedef struct __db_gfqbk
{
   struct
   {
      char                            site_id;
      char                            nw_id;
      char                            grp_id[5];
      char                            mti_id[4];
   } pri_key;
   struct
   {
      struct
      {
         char                            srv_cls_name[8];
         char                            srv_cls_num[4];
      } srv_cls_id;
   } recv_qfile_info;
   char                            future_use[27];
} db_gfqbk_def;
#define db_gfqbk_def_Size 50
#pragma section queue_data
/* Definition QUEUE-DATA created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __queue_data
typedef struct __queue_data
{
   char                            denbun_len[4];
   char                            mti_start_lct[4];
   char                            denbun[9999];
} queue_data_def;
#define queue_data_def_Size 10007
#pragma section db_gqnwq
/* Record DB-GQNWQ created on 06/19/2025 at 18:17 */
#pragma fieldalign shared2 __db_gqnwq
typedef struct __db_gqnwq
{
   struct
   {
      char                            key_time_stamp[8];
   } pri_key;
   struct
   {
      char                            shori_kubun[2];
      char                            err_code[7];
      char                            acc_node_kubun;
      char                            iss_node_kubun;
      char                            re_rgst_num[2];
      char                            future_use[27];
   } cntrl_info;
   struct
   {
      char                            denbun_recv_time[20];
      char                            recv_kyoku_sts;
      char                            nw_kubun[2];
      char                            mti_id[4];
      char                            send_denbun_shubetu;
      struct
      {
         struct
         {
            struct
            {
               char                            gfp_lcn_site;
               char                            gfp_lcn_nw;
               char                            gfp_lcn_future_use;
               char                            gfp_lcn_y;
               char                            gfp_lcn_mdh[3];
               char                            gfp_lcn_mmss[4];
               char                            gfp_lcn_seri_num[4];
            } gfp_lcn_id;
            char                            denbun_keitai;
         } tran_id;
         char                            denbun_shubetu;
         char                            re_send_num[3];
      } denbun_log_key;
      char                            denbun_fmt_kubun;
      char                            tushin_log_save_filename[36];
      struct
      {
         char                            part_id[2];
         char                            time_stamp[20];
         char                            tushin_unique_info[16];
      } tushin_log_key;
      char                            future_use[37];
   } denbun_send_recv_info;
   struct
   {
      char                            if_id[20];
      char                            station_id[11];
      struct
      {
         struct
         {
            char                            site_id;
            char                            nw_id;
            char                            grp_id[5];
            char                            if_id[5];
            char                            station_id[6];
            char                            connect_id[6];
         } recv_connect_id;
         struct
         {
            char                            ip_adress_src[15];
            char                            port_num_src[5];
            char                            ip_adress_dst[15];
            char                            port_num_dst[5];
         } recv_connect_info;
         struct
         {
            char                            time_stamp[20];
            char                            tushin_unique_info[16];
         } denbun_recv_time_stamp;
      } line_info;
      char                            future_use[54];
   } tushin_cntrl_info;
   struct
   {
      char                            denbun_len[4];
      char                            mti_start_lct[4];
      char                            denbun[9999];
   } denbun_area;
} db_gqnwq_def;
#define db_gqnwq_def_Size 10400
