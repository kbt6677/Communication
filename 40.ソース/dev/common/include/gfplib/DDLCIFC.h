/* SCHEMA PRODUCED DATE - TIME : 9/19/2025 - 12:23:20 */
#pragma section data_ctrl_info
/* Definition DATA-CTRL-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __data_ctrl_info
/**/
typedef struct __data_ctrl_info
{
   char                            total_len[5];
   char                            msg_id[2];
   char                            req_rec_node[8];
   struct
   {
      struct
      {
         char                            numbering_system_kbn;
         char                            location;
         char                            filler_1;
         char                            year_y;
         char                            mdh[3];
         char                            time_mmss[4];
         union
         {
            char                            gfp_lcn_serial_num[4];
            struct
            {
               char                            gfp_lcn_serial_num_1;
               char                            gfp_lcn_serial_num_2;
               char                            gfp_lcn_serial_num_3;
               char                            gfp_lcn_serial_num_4;
            } gfp_lcn_serial_num_div;
         } u_gfp_lcn_serial_num;
      } gfp_lcn;
      char                            msg_form;
   } transaction_id;
   char                            header_offset[5];
   char                            content_count[4];
   char                            data_offset[5];
   char                            data_len[5];
   char                            process_result_code[4];
   char                            process_start_time[20];
} data_ctrl_info_def;
#define data_ctrl_info_def_Size 74
#pragma section header_info_rq
/* Definition HEADER-INFO-RQ created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __header_info_rq
/**/
typedef struct __header_info_rq
{
   char                            receivequeue_content_id[4];
   char                            receivequeue_offset[5];
   char                            receivequeue_len[5];
   char                            sndqueue_content_id[4];
   char                            sndqueue_offset[5];
   char                            sndqueue_len[5];
   char                            cominfo_content_id[4];
   char                            cominfo_offset[5];
   char                            cominfo_len[5];
   char                            rcvmsg_comfield_content_id[4];
   char                            rcvmsg_comfield_offset[5];
   char                            rcvmsg_comfield_len[5];
   char                            rcvmsg_indvfield_content_id[4];
   char                            rcvmsg_indvfield_offset[5];
   char                            rcvmsg_indvfield_len[5];
   char                            masterinfo_content_id[4];
   char                            masterinfo_offset[5];
   char                            masterinfo_len[5];
   char                            loginfo_content_id[4];
   char                            loginfo_offset[5];
   char                            loginfo_len[5];
   char                            acqif_process_content_id[4];
   char                            acqif_process_offset[5];
   char                            acqif_process_len[5];
   char                            issuer_judgement_content_id[4];
   char                            issuer_judgement_offset[5];
   char                            issuer_judgement_len[5];
   char                            cancelinfo_content_id[4];
   char                            cancelinfo_offset[5];
   char                            cancelinfo_len[5];
   char                            adviceinfo_content_id[4];
   char                            adviceinfo_offset[5];
   char                            adviceinfo_len[5];
   char                            counter_info_content_id[4];
   char                            counter_info_offset[5];
   char                            counter_info_len[5];
   char                            neg_info_content_id[4];
   char                            neg_info_offset[5];
   char                            neg_info_len[5];
} header_info_rq_def;
#define header_info_rq_def_Size 182
#pragma section header_info_resp
/* Definition HEADER-INFO-RESP created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __header_info_resp
/**/
typedef struct __header_info_resp
{
   char                            receivequeue_content_id[4];
   char                            receivequeue_offset[5];
   char                            receivequeue_len[5];
   char                            sndqueue_content_id[4];
   char                            sndqueue_offset[5];
   char                            sndqueue_len[5];
   char                            cominfo_content_id[4];
   char                            cominfo_offset[5];
   char                            cominfo_len[5];
   char                            rcvmsg_comfield_content_id[4];
   char                            rcvmsg_comfield_offset[5];
   char                            rcvmsg_comfield_len[5];
   char                            rcvmsg_indvfield_content_id[4];
   char                            rcvmsg_indvfield_offset[5];
   char                            rcvmsg_indvfield_len[5];
   char                            delegate_info_content_id[4];
   char                            delegate_info_offset[5];
   char                            delegate_info_len[5];
   char                            adviceinfo_content_id[4];
   char                            adviceinfo_offset[5];
   char                            adviceinfo_len[5];
   char                            filler_1[84];
} header_info_resp_def;
#define header_info_resp_def_Size 182
#pragma section internal_rq
/* Definition INTERNAL-RQ created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __internal_rq
/**/
typedef struct __internal_rq
{
   struct
   {
      char                            total_len[5];
      char                            msg_id[2];
      char                            req_rec_node[8];
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            union
            {
               char                            gfp_lcn_serial_num[4];
               struct
               {
                  char                            gfp_lcn_serial_num_1;
                  char                            gfp_lcn_serial_num_2;
                  char                            gfp_lcn_serial_num_3;
                  char                            gfp_lcn_serial_num_4;
               } gfp_lcn_serial_num_div;
            } u_gfp_lcn_serial_num;
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      char                            header_offset[5];
      char                            content_count[4];
      char                            data_offset[5];
      char                            data_len[5];
      char                            process_result_code[4];
      char                            process_start_time[20];
   } data_ctrl_info;
   struct
   {
      char                            receivequeue_content_id[4];
      char                            receivequeue_offset[5];
      char                            receivequeue_len[5];
      char                            sndqueue_content_id[4];
      char                            sndqueue_offset[5];
      char                            sndqueue_len[5];
      char                            cominfo_content_id[4];
      char                            cominfo_offset[5];
      char                            cominfo_len[5];
      char                            rcvmsg_comfield_content_id[4];
      char                            rcvmsg_comfield_offset[5];
      char                            rcvmsg_comfield_len[5];
      char                            rcvmsg_indvfield_content_id[4];
      char                            rcvmsg_indvfield_offset[5];
      char                            rcvmsg_indvfield_len[5];
      char                            masterinfo_content_id[4];
      char                            masterinfo_offset[5];
      char                            masterinfo_len[5];
      char                            loginfo_content_id[4];
      char                            loginfo_offset[5];
      char                            loginfo_len[5];
      char                            acqif_process_content_id[4];
      char                            acqif_process_offset[5];
      char                            acqif_process_len[5];
      char                            issuer_judgement_content_id[4];
      char                            issuer_judgement_offset[5];
      char                            issuer_judgement_len[5];
      char                            cancelinfo_content_id[4];
      char                            cancelinfo_offset[5];
      char                            cancelinfo_len[5];
      char                            adviceinfo_content_id[4];
      char                            adviceinfo_offset[5];
      char                            adviceinfo_len[5];
      char                            counter_info_content_id[4];
      char                            counter_info_offset[5];
      char                            counter_info_len[5];
      char                            neg_info_content_id[4];
      char                            neg_info_offset[5];
      char                            neg_info_len[5];
   } header_info_rq;
   char                            data_info_rq[32511];
} internal_rq_def;
#define internal_rq_def_Size 32767
#pragma section internal_resp
/* Definition INTERNAL-RESP created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __internal_resp
/**/
/**/
typedef struct __internal_resp
{
   struct
   {
      char                            total_len[5];
      char                            msg_id[2];
      char                            req_rec_node[8];
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            union
            {
               char                            gfp_lcn_serial_num[4];
               struct
               {
                  char                            gfp_lcn_serial_num_1;
                  char                            gfp_lcn_serial_num_2;
                  char                            gfp_lcn_serial_num_3;
                  char                            gfp_lcn_serial_num_4;
               } gfp_lcn_serial_num_div;
            } u_gfp_lcn_serial_num;
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      char                            header_offset[5];
      char                            content_count[4];
      char                            data_offset[5];
      char                            data_len[5];
      char                            process_result_code[4];
      char                            process_start_time[20];
   } data_ctrl_info;
   struct
   {
      char                            receivequeue_content_id[4];
      char                            receivequeue_offset[5];
      char                            receivequeue_len[5];
      char                            sndqueue_content_id[4];
      char                            sndqueue_offset[5];
      char                            sndqueue_len[5];
      char                            cominfo_content_id[4];
      char                            cominfo_offset[5];
      char                            cominfo_len[5];
      char                            rcvmsg_comfield_content_id[4];
      char                            rcvmsg_comfield_offset[5];
      char                            rcvmsg_comfield_len[5];
      char                            rcvmsg_indvfield_content_id[4];
      char                            rcvmsg_indvfield_offset[5];
      char                            rcvmsg_indvfield_len[5];
      char                            delegate_info_content_id[4];
      char                            delegate_info_offset[5];
      char                            delegate_info_len[5];
      char                            adviceinfo_content_id[4];
      char                            adviceinfo_offset[5];
      char                            adviceinfo_len[5];
      char                            filler_1[84];
   } header_info_resp;
   char                            data_info_resp[32511];
} internal_resp_def;
#define internal_resp_def_Size 32767
#pragma section common_info
/* Definition COMMON-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __common_info
/**/
typedef struct __common_info
{
   char                            filler_1[2];
   struct
   {
      char                            guardian_error_code[5];
      char                            pathsend_error_code[5];
      char                            error_detect_info[36];
      char                            oth_error_info[5];
   } internal_process_result;
   struct
   {
      struct
      {
         char                            partition_id[2];
         struct
         {
            struct
            {
               char                            numbering_system_kbn;
               char                            location;
               char                            filler_1;
               char                            year_y;
               char                            mdh[3];
               char                            time_mmss[4];
               union
               {
                  char                            gfp_lcn_serial_num[4];
                  struct
                  {
                     char                            gfp_lcn_serial_num_1;
                     char                            gfp_lcn_serial_num_2;
                     char                            gfp_lcn_serial_num_3;
                     char                            gfp_lcn_serial_num_4;
                  } gfp_lcn_serial_num_div;
               } u_gfp_lcn_serial_num;
            } gfp_lcn;
            char                            msg_form;
         } transaction_id;
      } p_key;
      struct
      {
         char                            filler_2[12];
         struct
         {
            char                            process_status_kbn;
            struct
            {
               char                            arpc[16];
               char                            arc[2];
               char                            emv_key_ctrl[16];
            } ic_handover_info;
            char                            filler_3[14];
         } process_info;
         struct
         {
            char                            pathmon_name_primary[16];
            char                            serverclass_name_primary[15];
            char                            timer_ctrl_addr1[4];
            char                            timer_ctrl_cpu1[2];
            char                            timer_ctrl_pin1[2];
            char                            timer_ctrl_addr2[4];
            char                            timer_ctrl_cpu2[2];
            char                            timer_ctrl_pin2[2];
         } timer_info;
         char                            filler_4[146];
         char                            update_yyyymmddhhmmss[14];
         char                            gfp_update_yyyymmddhhmmss[14];
      } gltrn_data_info;
   } gltrn_data;
   struct
   {
      struct
      {
         char                            partition_id[2];
         struct
         {
            struct
            {
               char                            numbering_system_kbn;
               char                            location;
               char                            filler_1;
               char                            year_y;
               char                            mdh[3];
               char                            time_mmss[4];
               union
               {
                  char                            gfp_lcn_serial_num[4];
                  struct
                  {
                     char                            gfp_lcn_serial_num_1;
                     char                            gfp_lcn_serial_num_2;
                     char                            gfp_lcn_serial_num_3;
                     char                            gfp_lcn_serial_num_4;
                  } gfp_lcn_serial_num_div;
               } u_gfp_lcn_serial_num;
            } gfp_lcn;
            char                            msg_form;
         } transaction_id;
      } p_key;
      struct
      {
         struct
         {
            char                            internal_error_code[7];
            char                            req_msg_rec_node_kbn;
            char                            iss_connect_node_kbn;
            char                            resp_msg_receive_node_kbn;
            char                            resend_count[3];
            char                            to_kbn;
            char                            cancel_kbn;
            char                            resp_receive_flg;
            char                            bypass_implementation_flg;
            char                            agency_implement_flg;
            char                            agy_bps_internal_error_code[7];
            char                            adv_kbn[2];
            char                            adv_resend_receive_flg;
            char                            sales_adv_corres_log_key[18];
            char                            cancel_adv_biz_log_key[18];
            union
            {
               char                            transaction_type[4];
               struct
               {
                  char                            transaction_type_1;
                  char                            transaction_type_2;
                  char                            transaction_type_3;
                  char                            transaction_type_4;
               } transaction_type_div;
            } u_transaction_type;
            char                            jcb_card_kbn;
         } process_status;
         struct
         {
            struct
            {
               struct
               {
                  char                            msg_receive_timestamp[20];
                  char                            receive_kyoku_status_kbn;
                  char                            nw_kbn[2];
                  char                            mti[4];
                  char                            msg_format_kbn;
               } msg_receive_info;
               struct
               {
                  char                            interface[20];
                  char                            station[11];
                  struct
                  {
                     struct
                     {
                        char                            site_id;
                        char                            nw_ident;
                        char                            group_id[5];
                        char                            interface_id[5];
                        char                            station_id[6];
                        char                            connection_id[6];
                     } receive_conn_log_id;
                     struct
                     {
                        char                            self_ip_address[15];
                        char                            self_port_num[5];
                        char                            connect_dest_ip_address[15];
                        char                            connect_dest_port_num[5];
                     } receive_conn_info;
                     struct
                     {
                        char                            timestamp[20];
                        char                            comm_unique_info[16];
                     } msg_receive_timestamp;
                  } line_info;
               } comm_ctrl_process_info;
               char                            comm_log_save_file_name[36];
               char                            comm_log_key[36];
            } acq_comm_info;
            struct
            {
               struct
               {
                  char                            msg_receive_timestamp[20];
                  char                            receive_kyoku_status_kbn;
                  char                            nw_kbn[2];
                  char                            mti[4];
                  char                            msg_format_kbn;
               } msg_receive_info;
               struct
               {
                  char                            interface[20];
                  char                            station[11];
                  struct
                  {
                     struct
                     {
                        char                            site_id;
                        char                            nw_ident;
                        char                            group_id[5];
                        char                            interface_id[5];
                        char                            station_id[6];
                        char                            connection_id[6];
                     } receive_conn_log_id;
                     struct
                     {
                        char                            self_ip_address[15];
                        char                            self_port_num[5];
                        char                            connect_dest_ip_address[15];
                        char                            connect_dest_port_num[5];
                     } receive_conn_info;
                     struct
                     {
                        char                            timestamp[20];
                        char                            comm_unique_info[16];
                     } msg_receive_timestamp;
                  } line_info;
               } comm_ctrl_process_info;
               char                            comm_log_save_file_name[36];
               char                            comm_log_key[36];
            } iss_comm_info;
         } comm_info;
         struct
         {
            char                            msg_log_phys_file_name_acq[36];
            char                            msg_log_phys_file_name_iss[36];
         } msg_log_info;
         struct
         {
            char                            cancel_req_log_key[30];
            char                            cancel_orig_resp_log_key[18];
            char                            cancel_orig_req_log_key[18];
            char                            cancel_target_recv_time[20];
            struct
            {
               char                            amt_entry_flg;
               char                            atm_cumul_exec_kbn;
               char                            member_use_cumul_kbn;
               char                            issuer_cumul_kbn;
               char                            emergency_cs_limit_kbn;
               char                            emergency_sp_limit_kbn;
            } orig_tran_cumul_info;
            char                            cancel_target_resp_code[3];
            char                            orig_tran_status;
         } cancel_info;
         struct
         {
            char                            acq_rq_receive_timestamp[20];
            char                            acq_resp_send_timestamp[20];
            char                            iss_rq_send_timestamp[20];
            char                            iss_resp_receive_timestamp[20];
         } process_time_info;
         struct
         {
            struct
            {
               char                            acq_timer_no_delegate[4];
               char                            acq_timer_delegate[4];
               char                            iss_timer[4];
               char                            sender_center_id[11];
               char                            agent_id[3];
            } acq_center_info;
            struct
            {
               char                            interface[20];
               char                            nw_kbn[2];
               char                            issuer_id[6];
               char                            iss_timer[4];
            } iss_center_info;
            struct
            {
               char                            conv_exp_date[4];
               char                            acq_conv_process_code[6];
               char                            iss_conv_process_code[6];
               char                            conv_country_code[3];
               char                            conv_term_country_code[3];
               char                            conv_currency_code[3];
               struct
               {
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_f4_tran_amt[17];
                  } conv_amt_f4;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_f28_fee_usd[10];
                  } conv_f28_fee_amt;
                  struct
                  {
                     char                            decimal_point_digit;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_cust_fee_amt;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_intcomp_fee_amt;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_oth_fee_amt;
                  } conv_f46_fee_amt;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_amt_usd[17];
                  } conv_tax_other_f60;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_amt_usd[17];
                  } conv_replace_amt_f95_1;
               } acq_conv_amt_info;
               struct
               {
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_f4_tran_amt[17];
                  } conv_amt_f4;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_f28_fee_usd[10];
                  } conv_f28_fee_amt;
                  struct
                  {
                     char                            decimal_point_digit;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_cust_fee_amt;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_intcomp_fee_amt;
                     struct
                     {
                        char                            amt_sign;
                        char                            conv_amt_usd[17];
                        char                            settle_amt_sign;
                        char                            conv_settle_amt_usd[17];
                     } conv_f46_oth_fee_amt;
                  } conv_f46_fee_amt;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_amt_usd[17];
                  } conv_tax_other_f60;
                  struct
                  {
                     char                            decimal_point_digit;
                     char                            conv_amt_usd[17];
                  } conv_replace_amt_f95_1;
               } iss_conv_amt_info;
            } conv_info;
            struct
            {
               char                            f_pan_len[2];
               char                            f_pan[19];
               char                            f_pan_expiry_date[4];
               char                            ic_precheck_result1;
               char                            ic_precheck_result2;
               char                            exp_date_check_result;
               char                            csc_pre_check_result;
               char                            pos_data_code_check_result;
               char                            token_status_check_result;
               char                            rpc_verify_check_result;
               char                            rq_status_kbn;
               char                            pan_csc_start_position[2];
               char                            pan_card_security_code[4];
               char                            token_kbn[2];
               char                            token_status[2];
               char                            cof_tran_id[8];
               char                            token_rqer_id[11];
               char                            cof_token_flg;
               char                            error_reason_code[3];
               char                            token_result_send_flg;
            } token_info;
            struct
            {
               struct
               {
                  struct
                  {
                     char                            real_num_section[10];
                     char                            index_section[4];
                     char                            decimal_section_digit[2];
                  } tran_conv_rate;
               } acq_currency_info;
               struct
               {
                  struct
                  {
                     char                            real_num_section[10];
                     char                            index_section[4];
                     char                            decimal_section_digit[2];
                     char                            issuer_currency_code[3];
                  } tran_conv_rate;
               } iss_currency_info;
               char                            action_code_ctrl[16];
            } tbl_get_info;
            struct
            {
               char                            pin_flg;
               char                            ic_info_flg;
               char                            icc_app_kbn[2];
               char                            cav1_3_flg;
               char                            cav1_3_sec_code_len;
               char                            cav2_flg;
               char                            cav2_sec_code_len;
               char                            liquorring_flg;
               union
               {
                  char                            pan[19];
                  struct
                  {
                     char                            pan_1_18[18];
                     char                            pan_19;
                  } pan_div;
               } u_pan;
               char                            pan_len[2];
               char                            pan_get_bit[2];
               char                            prefix[12];
               char                            exp_date_yymm[4];
               char                            service_code[3];
            } security_info;
            struct
            {
               struct
               {
                  char                            check_digit_mod10_ctrl;
                  char                            check_digit_mod11_ctrl;
                  char                            visa_pvv_check_ctrl;
                  char                            exp_date_check_ctrl;
                  char                            neg_check_ctrl;
                  char                            cav1_verify_ctrl;
                  char                            cav1_pre_verify_ctrl_flg;
                  char                            cav2_verify_ctrl;
                  char                            cav3_verify_ctrl;
                  char                            ac_verify_ctrl;
                  char                            tvrcvr_verify_ctrl;
                  char                            small_amt_tran_action_ctrl;
                  char                            approv_limit_agency_ctrl;
                  char                            reject_advice_ctrl_flg;
                  char                            j_secure_info_send_ctrl_flg;
                  char                            bh_auth_ctrl_tbl_reserve[5];
               } bh_auth_ctrl_tbl_info;
               struct
               {
                  char                            check_digit_mod10_result;
                  char                            check_digit_mod11_result;
                  char                            visa_pvv_verify_result;
                  char                            exp_date_check_result;
                  char                            service_code_check_result;
                  char                            neg_check_verify_result;
                  char                            cav1_verify_result;
                  char                            cav1_pre_verify_result;
                  char                            cav2_verify_result;
                  char                            cav3_verify_result;
                  char                            ac_verify_result;
                  char                            tvrcvr_verify_result;
                  char                            small_tran_delegate_verify;
                  char                            approv_limit_agency_verify;
                  char                            bh_auth_verify_reserve[5];
               } bh_auth_ctrl_result_info;
            } bh_check_result;
            struct
            {
               char                            rejection_discard_kbn;
               char                            amt_entry_flg;
               char                            cut_date_yyyymmdd[8];
               char                            auth_judge_inst_kbn;
               char                            transaction_initiator_flg;
               char                            numbering_approv_num[6];
               char                            merchant_category_code[4];
               char                            merchant_category_group[5];
               struct
               {
                  char                            atm_cumulative_flg;
                  char                            member_use_cumul_kbn;
                  char                            issuer_cumul_kbn;
                  char                            emergency_cs_limit_kbn;
                  char                            emergency_sp_limit_kbn;
               } cumul_info;
               char                            dtp_tran_id[30];
            } biz_process_info;
            struct
            {
               struct
               {
                  union
                  {
                     char                            mti[4];
                     struct
                     {
                        char                            mti_1_3[3];
                        char                            mti_4;
                     } mti_div;
                  } u_mti;
                  struct
                  {
                     char                            len_2[2];
                     char                            member_num[19];
                  } member_num_info;
                  union
                  {
                     char                            process_code[6];
                     struct
                     {
                        char                            process_code_1_2[2];
                        char                            process_code_3_6[4];
                     } process_code_div;
                  } u_process_code;
                  char                            transaction_amt[17];
                  char                            send_datemmddhhmmss[10];
                  char                            system_trace_audit_num[6];
                  char                            transaction_yymmddhhmmss[12];
                  char                            transaction_mmdd[4];
                  char                            exp_date_yymm[4];
                  char                            merchant_type[4];
                  char                            country_code[3];
                  char                            pos_entry_mode[12];
                  char                            fee[9];
                  struct
                  {
                     char                            len_2[2];
                     char                            acq_inst_id[11];
                  } acq_inst_id_info;
                  char                            retrieval_reference_num[12];
                  char                            auth_num[6];
                  char                            resp_code[3];
                  char                            terminal_num[8];
                  char                            merchant_num[15];
                  char                            merchant_name_location[42];
                  char                            other_fee[105];
                  char                            domestic_resp[8];
                  char                            transaction_currency_code[3];
                  char                            ic_info_flg;
                  char                            domestic_reserve_tax_other[7];
                  char                            orig_data_element[42];
               } req_info;
               struct
               {
                  char                            mti[4];
                  char                            transaction_amt[17];
                  char                            fee[9];
                  char                            auth_num[6];
                  char                            resp_code[3];
                  char                            other_fee[105];
                  char                            domestic_resp[8];
                  char                            ic_info_flg;
                  char                            domestic_reserve_tax_other[7];
               } resp_info;
            } acq_msg_info;
            struct
            {
               struct
               {
                  char                            mti[4];
                  struct
                  {
                     char                            len_2[2];
                     char                            member_num[19];
                  } member_num_info;
                  char                            process_code[6];
                  char                            transaction_amt[17];
                  char                            send_datemmddhhmmss[10];
                  char                            system_trace_audit_num[6];
                  char                            transaction_yymmddhhmmss[12];
                  char                            transaction_mmdd[4];
                  char                            exp_date_yymm[4];
                  char                            merchant_type[4];
                  char                            country_code[3];
                  char                            pos_entry_mode[12];
                  char                            fee[9];
                  struct
                  {
                     char                            len_2[2];
                     char                            acq_inst_id[11];
                  } acq_inst_id_info;
                  char                            retrieval_reference_num[12];
                  char                            auth_num[6];
                  char                            resp_code[3];
                  char                            terminal_num[8];
                  char                            merchant_num[15];
                  char                            merchant_name_location[42];
                  char                            other_fee[105];
                  char                            domestic_resp[8];
                  char                            transaction_currency_code[3];
                  char                            ic_info_flg;
                  char                            domestic_reserve_tax_other[7];
                  char                            orig_data_element[42];
               } req_info;
               struct
               {
                  char                            mti[4];
                  char                            transaction_amt[17];
                  char                            fee[9];
                  char                            auth_num[6];
                  char                            resp_code[3];
                  char                            other_fee[105];
                  char                            domestic_resp[8];
                  char                            ic_info_flg;
                  char                            domestic_reserve_tax_other[7];
               } resp_info;
            } iss_msg_info;
         } biz_info;
      } glblg_data_info;
   } glblg_data;
} common_info_def;
#define common_info_def_Size 2925
#pragma section receive_msg_common_field
/* Definition RECEIVE-MSG-COMMON-FIELD created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __receive_msg_common_field
/**/
typedef struct __receive_msg_common_field
{
   char                            mti[4];
   struct
   {
      char                            bitmap_primary[64];
      char                            bitmap_secondary[64];
   } bitmap;
   struct
   {
      char                            f2_card_num_len[2];
      char                            f2_card_num[19];
   } card_number_info;
   char                            f3_process_code[6];
   char                            f4_tran_amt[12];
   char                            f6_bill_amt[12];
   char                            f7_send_dt[10];
   char                            f10_conv_rate[8];
   char                            f11_system_trace_audit_num[6];
   char                            f12_tran_time[6];
   char                            f13_tran_date[4];
   char                            f14_exp_date[4];
   char                            f15_settle_date[4];
   char                            f16_conv_date[4];
   char                            f17_collect_date[4];
   char                            f18_merchant_type[4];
   char                            f19_acq_country_code[3];
   char                            f22_pos_entry_mode[12];
   char                            f23_card_seq_num[4];
   char                            f25_pos_cond_code[2];
   char                            f28_tran_fee[9];
   struct
   {
      char                            f32_acq_id_len[2];
      char                            f32_acq_id[11];
   } acq_inst_id_info;
   struct
   {
      char                            f33_fwd_id_len[2];
      char                            f33_fwd_id[11];
   } forward_inst_id_info;
   struct
   {
      char                            f35_track2_len[2];
      char                            f35_track2_info[40];
   } track2_info;
   char                            f37_retrieval_ref_num[12];
   char                            f38_auth_num[6];
   char                            f39_resp_code[3];
   char                            f41_term_num[8];
   char                            f42_merchant_num[15];
   char                            f43_merchant_name_addr[42];
   struct
   {
      char                            f45_track1_len[2];
      char                            f45_track1_info[79];
   } track1_info;
   char                            f49_tran_currency_code[3];
   char                            f51_bill_currency_code[3];
   char                            f52_pinblock_info[8];
   struct
   {
      char                            f53_security_info_len[2];
      char                            f53_security_data[16];
   } security_info;
   struct
   {
      char                            f55_ic_info_len[3];
      char                            f55_ic_data[255];
   } ic_info;
   struct
   {
      char                            f90_orig_data_elem_len[2];
      char                            f90_orig_data_elem[42];
   } original_data;
   char                            f95_alt_amt[42];
} receive_msg_common_field_def;
#define receive_msg_common_field_def_Size 878
#pragma section master_info_section
/* Definition MASTER-INFO-SECTION created on 09/19/2025 at 12:23 */
/**/
typedef char                            master_info_section_def[650];
#pragma section queue_server_add_info
/* Definition QUEUE-SERVER-ADD-INFO created on 09/19/2025 at 12:23 */
/**/
typedef char                            queue_server_add_info_def[280];
#pragma section acqif_process_info
/* Definition ACQIF-PROCESS-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __acqif_process_info
/**/
typedef struct __acqif_process_info
{
   char                            acq_route_ctrl[16];
   char                            cardnet_cup_merchant_flg;
   char                            discover_member_ng_flg;
   char                            keyfile_read_kpe[22];
   char                            keyfile_read_index[2];
} acqif_process_info_def;
#define acqif_process_info_def_Size 42
#pragma section issuer_judge_result_info
/* Definition ISSUER-JUDGE-RESULT-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __issuer_judge_result_info
/**/
typedef struct __issuer_judge_result_info
{
   struct
   {
      char                            prefix_tbl_info[8];
      char                            dpan_txn_flg;
      struct
      {
         char                            issuer_route_ctrl[16];
         char                            issuer_transaction_ctrl[16];
         char                            route_code[16];
         char                            domestic_atm_use_flg;
         char                            auth_interface[20];
         char                            auth_nw_kbn[2];
         char                            advice_interface[20];
         char                            advice_nw_kbn[2];
      } dest_inst_info;
   } issuer_judge_result_unit;
} issuer_judge_result_info_def;
#define issuer_judge_result_info_def_Size 102
#pragma section cancel_info_section
/* Definition CANCEL-INFO-SECTION created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __cancel_info_section
/**/
typedef struct __cancel_info_section
{
   char                            orig_txn_status;
   char                            cancel_process_judge_result;
   char                            cancel_kbn;
   char                            allocation_node_kbn;
   char                            cancel_orig_aqc_log_key[18];
   char                            cancel_orig_iss_log_key[18];
   char                            cancel_retry_count;
} cancel_info_section_def;
#define cancel_info_section_def_Size 41
#pragma section advice_info_section
/* Definition ADVICE-INFO-SECTION created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __advice_info_section
/**/
typedef struct __advice_info_section
{
   char                            adv_resend_flg;
   char                            advice_receive_file_key[26];
   char                            resend_cnt[3];
   char                            permission_rejection_kbn;
} advice_info_section_def;
#define advice_info_section_def_Size 31
#pragma section delegate_info
/* Definition DELEGATE-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __delegate_info
/**/
typedef struct __delegate_info
{
   char                            delegate_result;
} delegate_info_def;
#define delegate_info_def_Size 1
#pragma section numbering_rq
/* Definition NUMBERING-RQ created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __numbering_rq
/**/
typedef struct __numbering_rq
{
   char                            unique_msg_id[4];
   char                            serial_num_kbn[2];
} numbering_rq_def;
#define numbering_rq_def_Size 6
#pragma section numbering_resp
/* Definition NUMBERING-RESP created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __numbering_resp
/**/
typedef struct __numbering_resp
{
   char                            unique_msg_id[4];
   char                            serial_num_kbn[2];
   char                            numbering_value[6];
   char                            internal_error_code[7];
   char                            guardian_error_code[5];
   char                            error_detect_info[36];
} numbering_resp_def;
#define numbering_resp_def_Size 60
#pragma section gqnwq_data
/* Definition GQNWQ-DATA created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __gqnwq_data
/**/
typedef struct __gqnwq_data
{
   struct
   {
      char                            queue_key[8];
   } p_key;
   struct
   {
      struct
      {
         char                            process_code_kbn[2];
         char                            internal_error_code[7];
         char                            acq_process_node_kbn;
         char                            iss_connect_node_kbn;
         char                            re_registration_count[2];
         char                            filler_1[27];
      } ctrl_info;
      struct
      {
         char                            msg_receive_timestamp[20];
         char                            receive_kyoku_status_kbn;
         char                            nw_kbn[2];
         char                            mti[4];
         char                            send_msg_type_kbn;
         struct
         {
            struct
            {
               struct
               {
                  char                            numbering_system_kbn;
                  char                            location;
                  char                            filler_1;
                  char                            year_y;
                  char                            mdh[3];
                  char                            time_mmss[4];
                  union
                  {
                     char                            gfp_lcn_serial_num[4];
                     struct
                     {
                        char                            gfp_lcn_serial_num_1;
                        char                            gfp_lcn_serial_num_2;
                        char                            gfp_lcn_serial_num_3;
                        char                            gfp_lcn_serial_num_4;
                     } gfp_lcn_serial_num_div;
                  } u_gfp_lcn_serial_num;
               } gfp_lcn;
               char                            msg_form;
            } transaction_id;
            char                            msg_type;
            char                            resend_count[3];
         } msg_log_key;
         char                            msg_format_kbn;
         char                            comm_log_save_file_name[36];
         struct
         {
            char                            partition_id[2];
            char                            timestamp[20];
            char                            comm_unique_info[16];
         } comm_log_key;
         char                            filler_3[37];
      } msg_snd_receive_info;
      struct
      {
         char                            interface[20];
         char                            station[11];
         struct
         {
            struct
            {
               char                            site_id;
               char                            nw_ident;
               char                            group_id[5];
               char                            interface_id[5];
               char                            station_id[6];
               char                            connection_id[6];
            } receive_conn_log_id;
            struct
            {
               char                            self_ip_address[15];
               char                            self_port_num[5];
               char                            connect_dest_ip_address[15];
               char                            connect_dest_port_num[5];
            } receive_conn_info;
            struct
            {
               char                            timestamp[20];
               char                            comm_unique_info[16];
            } msg_receive_timestamp;
         } line_info;
         char                            filler_4[54];
      } comm_ctrl_info;
      struct
      {
         char                            msg_len[4];
         char                            mti_offset[4];
         char                            send_receive_msg[9999];
      } snd_receive_data_info;
      char                            filler_5[5];
   } gqnwq_data_info;
} gqnwq_data_def;
#define gqnwq_data_def_Size 10405
#pragma section gqnwq_data_rtn
/* Definition GQNWQ-DATA-RTN created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __gqnwq_data_rtn
/**/
typedef struct __gqnwq_data_rtn
{
   struct
   {
      char                            queue_key[8];
   } p_key;
   struct
   {
      struct
      {
         char                            process_code_kbn[2];
         char                            internal_error_code[7];
         char                            acq_process_node_kbn;
         char                            iss_connect_node_kbn;
         char                            re_registration_count[2];
         char                            filler_1[27];
      } ctrl_info;
      struct
      {
         char                            error_detect_server_kbn;
         char                            rtn_reason_kbn;
         struct
         {
            struct
            {
               char                            numbering_system_kbn;
               char                            location;
               char                            filler_1;
               char                            year_y;
               char                            mdh[3];
               char                            time_mmss[4];
               union
               {
                  char                            gfp_lcn_serial_num[4];
                  struct
                  {
                     char                            gfp_lcn_serial_num_1;
                     char                            gfp_lcn_serial_num_2;
                     char                            gfp_lcn_serial_num_3;
                     char                            gfp_lcn_serial_num_4;
                  } gfp_lcn_serial_num_div;
               } u_gfp_lcn_serial_num;
            } gfp_lcn;
            char                            msg_form;
         } transaction_id;
         char                            filler_2[22];
      } biz_ctrl_info;
      struct
      {
         char                            master_info_jcb_prefix[300];
         char                            master_info_iss_info[250];
         char                            keyfile_read_kpe[22];
         char                            keyfile_read_index[2];
         struct
         {
            char                            prefix_tbl_info[8];
            char                            dpan_txn_flg;
            struct
            {
               char                            issuer_route_ctrl[16];
               char                            issuer_transaction_ctrl[16];
               char                            route_code[16];
               char                            domestic_atm_use_flg;
               char                            auth_interface[20];
               char                            auth_nw_kbn[2];
               char                            advice_interface[20];
               char                            advice_nw_kbn[2];
            } dest_inst_info;
         } issuer_judge_result_unit;
         char                            receive_msg_common_fi_area[878];
         char                            filler_3[163];
         char                            receive_msg_unique_fi_area[6000];
      } common_if_info;
      char                            filler_4[2600];
   } gqnwq_data_rtn_info;
} gqnwq_data_rtn_def;
#define gqnwq_data_rtn_def_Size 10405
#pragma section receivequeue_info
/* Definition RECEIVEQUEUE-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __receivequeue_info
/**/
typedef struct __receivequeue_info
{
   union
   {
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            acq_process_node_kbn;
                  char                            iss_connect_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_1[27];
               } ctrl_info;
               struct
               {
                  char                            msg_receive_timestamp[20];
                  char                            receive_kyoku_status_kbn;
                  char                            nw_kbn[2];
                  char                            mti[4];
                  char                            send_msg_type_kbn;
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            msg_type;
                     char                            resend_count[3];
                  } msg_log_key;
                  char                            msg_format_kbn;
                  char                            comm_log_save_file_name[36];
                  struct
                  {
                     char                            partition_id[2];
                     char                            timestamp[20];
                     char                            comm_unique_info[16];
                  } comm_log_key;
                  char                            filler_3[37];
               } msg_snd_receive_info;
               struct
               {
                  char                            interface[20];
                  char                            station[11];
                  struct
                  {
                     struct
                     {
                        char                            site_id;
                        char                            nw_ident;
                        char                            group_id[5];
                        char                            interface_id[5];
                        char                            station_id[6];
                        char                            connection_id[6];
                     } receive_conn_log_id;
                     struct
                     {
                        char                            self_ip_address[15];
                        char                            self_port_num[5];
                        char                            connect_dest_ip_address[15];
                        char                            connect_dest_port_num[5];
                     } receive_conn_info;
                     struct
                     {
                        char                            timestamp[20];
                        char                            comm_unique_info[16];
                     } msg_receive_timestamp;
                  } line_info;
                  char                            filler_4[54];
               } comm_ctrl_info;
               struct
               {
                  char                            msg_len[4];
                  char                            mti_offset[4];
                  char                            send_receive_msg[9999];
               } snd_receive_data_info;
               char                            filler_5[5];
            } gqnwq_data_info;
         } gqnwq_data;
      } rcvgqnwq_data_ext;
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            acq_process_node_kbn;
                  char                            iss_connect_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_1[27];
               } ctrl_info;
               struct
               {
                  char                            error_detect_server_kbn;
                  char                            rtn_reason_kbn;
                  struct
                  {
                     struct
                     {
                        char                            numbering_system_kbn;
                        char                            location;
                        char                            filler_1;
                        char                            year_y;
                        char                            mdh[3];
                        char                            time_mmss[4];
                        union
                        {
                           char                            gfp_lcn_serial_num[4];
                           struct
                           {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                           } gfp_lcn_serial_num_div;
                        } u_gfp_lcn_serial_num;
                     } gfp_lcn;
                     char                            msg_form;
                  } transaction_id;
                  char                            filler_2[22];
               } biz_ctrl_info;
               struct
               {
                  char                            master_info_jcb_prefix[300];
                  char                            master_info_iss_info[250];
                  char                            keyfile_read_kpe[22];
                  char                            keyfile_read_index[2];
                  struct
                  {
                     char                            prefix_tbl_info[8];
                     char                            dpan_txn_flg;
                     struct
                     {
                        char                            issuer_route_ctrl[16];
                        char                            issuer_transaction_ctrl[16];
                        char                            route_code[16];
                        char                            domestic_atm_use_flg;
                        char                            auth_interface[20];
                        char                            auth_nw_kbn[2];
                        char                            advice_interface[20];
                        char                            advice_nw_kbn[2];
                     } dest_inst_info;
                  } issuer_judge_result_unit;
                  char                            receive_msg_common_fi_area[878];
                  char                            filler_3[163];
                  char                            receive_msg_unique_fi_area[6000];
               } common_if_info;
               char                            filler_4[2600];
            } gqnwq_data_rtn_info;
         } gqnwq_data_rtn;
      } rcvgqnwq_data_rtn;
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            process_result;
                  char                            req_send_system_kbn;
                  char                            req_send_node_kbn;
                  char                            req_rec_system_kbn;
                  char                            req_rec_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_2[27];
               } ctrl_info;
               struct
               {
                  char                            msg_send_timestamp[20];
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            resp_receive_queue_file[30];
                     char                            filler_3[25];
                  } gfp_info;
                  struct
                  {
                     char                            fep_dtp_int_tran_id[30];
                     char                            log_save_date[2];
                     char                            resp_receive_queue_file[30];
                  } fepdtp_info;
                  char                            filler_4[28];
               } system_info;
               struct
               {
                  char                            send_receive_msg[9999];
               } send_receive_msg_info;
            } fepdtp_queue_data_info;
         } gqfdq_data;
         char                            filler_x[169];
      } rcvgqnwq_data_gqfdq;
   } u_rcvgqnwq_data_ext;
} receivequeue_info_def;
#define receivequeue_info_def_Size 10405
#pragma section sndqueue_info
/* Definition SNDQUEUE-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __sndqueue_info
/**/
typedef struct __sndqueue_info
{
   union
   {
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            acq_process_node_kbn;
                  char                            iss_connect_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_1[27];
               } ctrl_info;
               struct
               {
                  char                            msg_receive_timestamp[20];
                  char                            receive_kyoku_status_kbn;
                  char                            nw_kbn[2];
                  char                            mti[4];
                  char                            send_msg_type_kbn;
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            msg_type;
                     char                            resend_count[3];
                  } msg_log_key;
                  char                            msg_format_kbn;
                  char                            comm_log_save_file_name[36];
                  struct
                  {
                     char                            partition_id[2];
                     char                            timestamp[20];
                     char                            comm_unique_info[16];
                  } comm_log_key;
                  char                            filler_3[37];
               } msg_snd_receive_info;
               struct
               {
                  char                            interface[20];
                  char                            station[11];
                  struct
                  {
                     struct
                     {
                        char                            site_id;
                        char                            nw_ident;
                        char                            group_id[5];
                        char                            interface_id[5];
                        char                            station_id[6];
                        char                            connection_id[6];
                     } receive_conn_log_id;
                     struct
                     {
                        char                            self_ip_address[15];
                        char                            self_port_num[5];
                        char                            connect_dest_ip_address[15];
                        char                            connect_dest_port_num[5];
                     } receive_conn_info;
                     struct
                     {
                        char                            timestamp[20];
                        char                            comm_unique_info[16];
                     } msg_receive_timestamp;
                  } line_info;
                  char                            filler_4[54];
               } comm_ctrl_info;
               struct
               {
                  char                            msg_len[4];
                  char                            mti_offset[4];
                  char                            send_receive_msg[9999];
               } snd_receive_data_info;
               char                            filler_5[5];
            } gqnwq_data_info;
         } gqnwq_data;
      } sndgqnwq_data_ext;
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            acq_process_node_kbn;
                  char                            iss_connect_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_1[27];
               } ctrl_info;
               struct
               {
                  char                            error_detect_server_kbn;
                  char                            rtn_reason_kbn;
                  struct
                  {
                     struct
                     {
                        char                            numbering_system_kbn;
                        char                            location;
                        char                            filler_1;
                        char                            year_y;
                        char                            mdh[3];
                        char                            time_mmss[4];
                        union
                        {
                           char                            gfp_lcn_serial_num[4];
                           struct
                           {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                           } gfp_lcn_serial_num_div;
                        } u_gfp_lcn_serial_num;
                     } gfp_lcn;
                     char                            msg_form;
                  } transaction_id;
                  char                            filler_2[22];
               } biz_ctrl_info;
               struct
               {
                  char                            master_info_jcb_prefix[300];
                  char                            master_info_iss_info[250];
                  char                            keyfile_read_kpe[22];
                  char                            keyfile_read_index[2];
                  struct
                  {
                     char                            prefix_tbl_info[8];
                     char                            dpan_txn_flg;
                     struct
                     {
                        char                            issuer_route_ctrl[16];
                        char                            issuer_transaction_ctrl[16];
                        char                            route_code[16];
                        char                            domestic_atm_use_flg;
                        char                            auth_interface[20];
                        char                            auth_nw_kbn[2];
                        char                            advice_interface[20];
                        char                            advice_nw_kbn[2];
                     } dest_inst_info;
                  } issuer_judge_result_unit;
                  char                            receive_msg_common_fi_area[878];
                  char                            filler_3[163];
                  char                            receive_msg_unique_fi_area[6000];
               } common_if_info;
               char                            filler_4[2600];
            } gqnwq_data_rtn_info;
         } gqnwq_data_rtn;
      } sndgqnwq_data_rtn;
      struct
      {
         struct
         {
            struct
            {
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            process_result;
                  char                            req_send_system_kbn;
                  char                            req_send_node_kbn;
                  char                            req_rec_system_kbn;
                  char                            req_rec_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_2[27];
               } ctrl_info;
               struct
               {
                  char                            msg_send_timestamp[20];
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            resp_receive_queue_file[30];
                     char                            filler_3[25];
                  } gfp_info;
                  struct
                  {
                     char                            fep_dtp_int_tran_id[30];
                     char                            log_save_date[2];
                     char                            resp_receive_queue_file[30];
                  } fepdtp_info;
                  char                            filler_4[28];
               } system_info;
               struct
               {
                  char                            send_receive_msg[9999];
               } send_receive_msg_info;
            } fepdtp_queue_data_info;
         } gqfdq_data;
         char                            filler_x[169];
      } sndgqnwq_data_gqfdq;
      struct
      {
         struct
         {
            struct
            {
               struct
               {
                  struct
                  {
                     struct
                     {
                        char                            numbering_system_kbn;
                        char                            location;
                        char                            filler_1;
                        char                            year_y;
                        char                            mdh[3];
                        char                            time_mmss[4];
                        union
                        {
                           char                            gfp_lcn_serial_num[4];
                           struct
                           {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                           } gfp_lcn_serial_num_div;
                        } u_gfp_lcn_serial_num;
                     } gfp_lcn;
                     char                            msg_form;
                     char                            filler_1[14];
                  } fep_dtp_int_tran_id;
               } fep_queue_key;
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            process_result;
                  char                            req_send_system_kbn;
                  char                            req_send_node_kbn;
                  char                            req_rec_system_kbn;
                  char                            req_rec_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_2[27];
               } ctrl_info;
               struct
               {
                  char                            msg_send_timestamp[20];
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            resp_receive_queue_file[30];
                     char                            filler_3[25];
                  } gfp_info;
                  struct
                  {
                     char                            fep_dtp_int_tran_id[30];
                     char                            log_save_date[2];
                     char                            resp_receive_queue_file[30];
                  } fepdtp_info;
                  char                            filler_4[28];
               } system_info;
               struct
               {
                  char                            send_receive_msg[9999];
               } send_receive_msg_info;
            } fepdtp_queue_data_info;
         } gqfdk_data;
         char                            filler_x[139];
      } sndgqnwq_data_gqfdk;
      struct
      {
         struct
         {
            struct
            {
               char                            fep_queue_key[2];
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            process_result;
                  char                            req_send_system_kbn;
                  char                            req_send_node_kbn;
                  char                            req_rec_system_kbn;
                  char                            req_rec_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_2[27];
               } ctrl_info;
               struct
               {
                  char                            msg_send_timestamp[20];
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            resp_receive_queue_file[30];
                     char                            filler_3[25];
                  } gfp_info;
                  struct
                  {
                     char                            fep_dtp_int_tran_id[30];
                     char                            log_save_date[2];
                     char                            resp_receive_queue_file[30];
                  } fepdtp_info;
                  char                            filler_4[28];
               } system_info;
               struct
               {
                  char                            send_receive_msg[9999];
               } send_receive_msg_info;
            } fepdtp_queue_data_info;
         } gqfek_data;
         char                            filler_x[167];
      } sndgqnwq_data_gqfek;
      struct
      {
         struct
         {
            struct
            {
               struct
               {
                  struct
                  {
                     char                            numbering_system_kbn;
                     char                            location;
                     char                            filler_1;
                     char                            year_y;
                     char                            mdh[3];
                     char                            time_mmss[4];
                     union
                     {
                        char                            gfp_lcn_serial_num[4];
                        struct
                        {
                           char                            gfp_lcn_serial_num_1;
                           char                            gfp_lcn_serial_num_2;
                           char                            gfp_lcn_serial_num_3;
                           char                            gfp_lcn_serial_num_4;
                        } gfp_lcn_serial_num_div;
                     } u_gfp_lcn_serial_num;
                  } gfp_lcn;
               } pa_queue_key;
               char                            queue_key[8];
            } p_key;
            struct
            {
               struct
               {
                  char                            process_code_kbn[2];
                  char                            internal_error_code[7];
                  char                            acq_process_node_kbn;
                  char                            iss_connect_node_kbn;
                  char                            re_registration_count[2];
                  char                            filler_1[27];
               } ctrl_info;
               struct
               {
                  char                            msg_receive_timestamp[20];
                  char                            receive_kyoku_status_kbn;
                  char                            nw_kbn[2];
                  char                            mti[4];
                  char                            send_msg_type_kbn;
                  struct
                  {
                     struct
                     {
                        struct
                        {
                           char                            numbering_system_kbn;
                           char                            location;
                           char                            filler_1;
                           char                            year_y;
                           char                            mdh[3];
                           char                            time_mmss[4];
                           union
                           {
                              char                            gfp_lcn_serial_num[4];
                              struct
                              {
                              char                            gfp_lcn_serial_num_1;
                              char                            gfp_lcn_serial_num_2;
                              char                            gfp_lcn_serial_num_3;
                              char                            gfp_lcn_serial_num_4;
                              } gfp_lcn_serial_num_div;
                           } u_gfp_lcn_serial_num;
                        } gfp_lcn;
                        char                            msg_form;
                     } transaction_id;
                     char                            msg_type;
                     char                            resend_count[3];
                  } msg_log_key;
                  char                            msg_format_kbn;
                  char                            comm_log_save_file_name[36];
                  struct
                  {
                     char                            partition_id[2];
                     char                            timestamp[20];
                     char                            comm_unique_info[16];
                  } comm_log_key;
                  char                            filler_3[37];
               } msg_snd_receive_info;
               struct
               {
                  char                            interface[20];
                  char                            station[11];
                  struct
                  {
                     struct
                     {
                        char                            site_id;
                        char                            nw_ident;
                        char                            group_id[5];
                        char                            interface_id[5];
                        char                            station_id[6];
                        char                            connection_id[6];
                     } receive_conn_log_id;
                     struct
                     {
                        char                            self_ip_address[15];
                        char                            self_port_num[5];
                        char                            connect_dest_ip_address[15];
                        char                            connect_dest_port_num[5];
                     } receive_conn_info;
                     struct
                     {
                        char                            timestamp[20];
                        char                            comm_unique_info[16];
                     } msg_receive_timestamp;
                  } line_info;
                  char                            filler_4[54];
               } comm_ctrl_info;
               struct
               {
                  char                            msg_len[4];
                  char                            mti_offset[4];
                  char                            send_receive_msg_pa[9989];
               } snd_receive_data_info;
            } gqpak_data_info;
         } gqpak_data;
      } sndgqnwq_data_gqpak;
   } u_sndgqnwq_data_ext;
} sndqueue_info_def;
#define sndqueue_info_def_Size 10405
#pragma section receive_msg_common_fi_info
/* Definition RECEIVE-MSG-COMMON-FI-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __receive_msg_common_fi_info
/**/
typedef struct __receive_msg_common_fi_info
{
   char                            receive_msg_common_fi_area[878];
} receive_msg_common_fi_info_def;
#define receive_msg_common_fi_info_def_Size 878
#pragma section receive_msg_unique_fi_info
/* Definition RECEIVE-MSG-UNIQUE-FI-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __receive_msg_unique_fi_info
/**/
typedef struct __receive_msg_unique_fi_info
{
   char                            receive_msg_unique_fi_area[6000];
} receive_msg_unique_fi_info_def;
#define receive_msg_unique_fi_info_def_Size 6000
#pragma section f60_domestic_reserve
/* Definition F60-DOMESTIC-RESERVE created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __f60_domestic_reserve
/**/
typedef struct __f60_domestic_reserve
{
   char                            f60_len[3];
   char                            f60_term_shiki_num[13];
   char                            f60_term_process_serial_num[5];
   char                            f60_payment_kbn[2];
   char                            f60_tax[7];
   char                            f60_cancel_kbn;
   char                            f60_auth_num[6];
   char                            f60_voucher_num[5];
   char                            f60_payment_method;
} f60_domestic_reserve_def;
#define f60_domestic_reserve_def_Size 43
#pragma section dn01_cardnet_unique_info
/* Definition DN01-CARDNET-UNIQUE-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __dn01_cardnet_unique_info
/**/
typedef struct __dn01_cardnet_unique_info
{
   char                            h000_header[4];
   char                            h000_length[3];
   struct
   {
      char                            h000_value_len[4];
      char                            h000_sender_center_id[11];
      char                            h000_receiver_center_id[11];
      char                            h000_merchant_company_code[11];
      char                            h000_send_date_time[14];
      char                            h000_mode_flg[2];
      char                            h000_msg_type[4];
      char                            h000_corres_kbn[2];
      char                            h000_cut_date[8];
      char                            h000_body_len[4];
      char                            h000_cardnet_tran_id[4];
      char                            h000_cardnet_tran_num[12];
      char                            h000_cardnet_use_fld[2];
   } h000_value;
} dn01_cardnet_unique_info_def;
#define dn01_cardnet_unique_info_def_Size 96
#pragma section neg_info
/* Definition NEG-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __neg_info
/**/
typedef struct __neg_info
{
   char                            neg_tag[5];
   char                            neg_len[5];
   char                            neg_data[80];
} neg_info_def;
#define neg_info_def_Size 90
#pragma section counter_info
/* Definition COUNTER-INFO created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __counter_info
/**/
typedef struct __counter_info
{
   struct
   {
      char                            epsit_count[10];
      char                            wdral_error_cancel_count[10];
      char                            wdral_count[10];
      char                            epsit_err_cancel_count[10];
      char                            inquiry_count[10];
      char                            auth_count[10];
      char                            inquiry_error_cancel_count[10];
      char                            epsit_amt_if[16];
      char                            wdral_error_cancel_amt_if[16];
      char                            wdral_amt_if[16];
      char                            epsit_err_cancel_amt_if[16];
      char                            auth_fail_cancel_count[10];
      char                            verify_total_amt_sign;
      char                            verify_total_amt_if[16];
      struct
      {
         char                            cust_fee_type[2];
         char                            cust_fee_count[12];
         char                            cust_fee_amt[10];
         char                            intcomp_fee_type[2];
         char                            intcomp_fee_count[12];
         char                            intcomp_fee_amt[10];
         char                            oth_fee_type[2];
         char                            oth_fee_count[12];
         char                            oth_fee_amt[10];
      } reception_fee_amt_info;
      struct
      {
         char                            cust_fee_type[2];
         char                            cust_fee_count[12];
         char                            cust_fee_amt[10];
         char                            intcomp_fee_type[2];
         char                            intcomp_fee_count[12];
         char                            intcomp_fee_amt[10];
         char                            oth_fee_type[2];
         char                            oth_fee_count[12];
         char                            oth_fee_amt[10];
      } corres_fee_amt_info;
   } cs_counter_info;
   struct
   {
      char                            sales_rtn_cancel_count[10];
      char                            sales_err_cancel_count[10];
      char                            sales_count[10];
      char                            sales_rtn_err_cancel_count[10];
      char                            inquiry_count[10];
      char                            auth_count[10];
      char                            inquiry_error_cancel_count[10];
      char                            sales_rtn_cancel_amt_if[16];
      char                            sales_err_cancel_amt_if[16];
      char                            sales_amt_if[16];
      char                            sales_rtn_err_cancel_amt_if[16];
      char                            auth_fail_cancel_count[10];
      char                            verify_total_amt_sign;
      char                            verify_total_amt_if[16];
   } sp_counter_info;
} counter_info_def;
#define counter_info_def_Size 466
#pragma section header_info_dtp
/* Definition HEADER-INFO-DTP created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __header_info_dtp
/**/
typedef struct __header_info_dtp
{
   char                            dtpinfo_content_id[4];
   char                            dtpinfo_offset[5];
   char                            dtpinfo_len[5];
} header_info_dtp_def;
#define header_info_dtp_def_Size 14
#pragma section internal_dtp
/* Definition INTERNAL-DTP created on 09/19/2025 at 12:23 */
#pragma fieldalign shared2 __internal_dtp
/**/
typedef struct __internal_dtp
{
   struct
   {
      char                            total_len[5];
      char                            msg_id[2];
      char                            req_rec_node[8];
      struct
      {
         struct
         {
            char                            numbering_system_kbn;
            char                            location;
            char                            filler_1;
            char                            year_y;
            char                            mdh[3];
            char                            time_mmss[4];
            union
            {
               char                            gfp_lcn_serial_num[4];
               struct
               {
                  char                            gfp_lcn_serial_num_1;
                  char                            gfp_lcn_serial_num_2;
                  char                            gfp_lcn_serial_num_3;
                  char                            gfp_lcn_serial_num_4;
               } gfp_lcn_serial_num_div;
            } u_gfp_lcn_serial_num;
         } gfp_lcn;
         char                            msg_form;
      } transaction_id;
      char                            header_offset[5];
      char                            content_count[4];
      char                            data_offset[5];
      char                            data_len[5];
      char                            process_result_code[4];
      char                            process_start_time[20];
   } data_ctrl_info;
   struct
   {
      char                            dtpinfo_content_id[4];
      char                            dtpinfo_offset[5];
      char                            dtpinfo_len[5];
   } header_info_dtp;
   char                            data_info_dtp[1787];
} internal_dtp_def;
#define internal_dtp_def_Size 1875
