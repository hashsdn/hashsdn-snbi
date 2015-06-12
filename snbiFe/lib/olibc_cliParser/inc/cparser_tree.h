/*----------------------------------------------------------------------
 * This file is generated by mk_parser.py.
 *----------------------------------------------------------------------*/
#ifndef __CPARSER_TREE_H__
#define __CPARSER_TREE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern cparser_node_t cparser_root;

cparser_result_t cparser_cmd_clear_screen(cparser_context_t *context);
cparser_result_t cparser_cmd_show_autonomic_interface(cparser_context_t *context);
cparser_result_t cparser_cmd_show_ip_interfaces(cparser_context_t *context);
cparser_result_t cparser_cmd_show_process(cparser_context_t *context);
cparser_result_t cparser_cmd_show_snbi_device(cparser_context_t *context);
cparser_result_t cparser_cmd_show_snbi_neighbors(cparser_context_t *context);
cparser_result_t cparser_cmd_show_snbi_intf_db(cparser_context_t *context);
cparser_result_t cparser_cmd_enable_privileged_mode(cparser_context_t *context);
cparser_result_t cparser_cmd_disable_privileged_mode(cparser_context_t *context);
cparser_result_t cparser_cmd_quit(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_quit(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_discovery(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_no_discovery(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_no_debug_neighbor_discovery_type_level(cparser_context_t *context,
    char **type_ptr,
    char **level_ptr);
cparser_result_t cparser_cmd_snbi_no_debug_bootstrap_type_level(cparser_context_t *context,
    char **type_ptr,
    char **level_ptr);
cparser_result_t cparser_cmd_snbi_interface_start(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_interface_stop(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_start(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_stop(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_debug_log_console(cparser_context_t *context);
cparser_result_t cparser_cmd_snbi_debug_log_file_logfile(cparser_context_t *context,
    char **logfile_ptr);
cparser_result_t cparser_cmd_snbi_debug_neighbor_discovery_type_level(cparser_context_t *context,
    char **type_ptr,
    char **level_ptr);
cparser_result_t cparser_cmd_snbi_debug_bootstrap_type_level(cparser_context_t *context,
    char **type_ptr,
    char **level_ptr);
cparser_result_t cparser_cmd_test(cparser_context_t *context);
cparser_result_t cparser_cmd_test_avl_get_firstnode(cparser_context_t *context);
cparser_result_t cparser_cmd_test_avl_init(cparser_context_t *context);
cparser_result_t cparser_cmd_test_avl_insert_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_avl_remove_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_avl_search_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_avl_uninit(cparser_context_t *context);
cparser_result_t cparser_cmd_test_avl_walk(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_create(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_destroy(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_is_valid(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_is_empty(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_lookup_node_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_list_enqueue_node_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_list_dequeue_node(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_remove_node_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_list_get_head_data(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_walk(cparser_context_t *context);
cparser_result_t cparser_cmd_test_list_iterator_walk(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_logging_stdout(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_pthread_create(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_stop_evnt_loop(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_timer_create(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_timer_destroy(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_timer_start_value(cparser_context_t *context,
    uint32_t *value_ptr);
cparser_result_t cparser_cmd_test_event_timer_stop(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_timer_running(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_msg_q_create(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_msg_q_destroy(cparser_context_t *context);
cparser_result_t cparser_cmd_test_event_msg_q_enqueue_msgtype_valarg_ptrarg(cparser_context_t *context,
    uint32_t *msgtype_ptr,
    uint32_t *valarg_ptr,
    char **ptrarg_ptr);
cparser_result_t cparser_cmd_test_event_msg_q_scale(cparser_context_t *context);
cparser_result_t cparser_cmd_test_hash_create(cparser_context_t *context);
cparser_result_t cparser_cmd_test_hash_destory(cparser_context_t *context);
cparser_result_t cparser_cmd_test_hash_insert_key_value(cparser_context_t *context,
    char **key_ptr,
    char **value_ptr);
cparser_result_t cparser_cmd_test_hash_lookup_key(cparser_context_t *context,
    char **key_ptr);
cparser_result_t cparser_cmd_test_hash_remove_key(cparser_context_t *context,
    char **key_ptr);
cparser_result_t cparser_cmd_test_hash_walk(cparser_context_t *context);
cparser_result_t cparser_cmd_test_if_list(cparser_context_t *context);
cparser_result_t cparser_cmd_test_quit(cparser_context_t *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CPARSER_TREE_H__ */
