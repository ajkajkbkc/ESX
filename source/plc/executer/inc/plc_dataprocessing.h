/**
  ******************************************************************************
  * @file    plc_dataprocessing.h
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   数据处理相关指令,包括比较指令 数据运算指令等
  ******************************************************************************
  */

#ifndef __PLC_CONTACT_INS_H
#define __PLC_CONTACT_INS_H

#define PI					3.14159265358979f
/*判断是否位16进制ASCII*/
#define IS_HEX_NUM_ASCII(x) (((x)>=0x30 && (x)<=0x39) || ((x)>=0x41 && (x)<=0x46))


/*整数比较指令*/
unsigned char run_ld_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_and_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_or_compare_ins(plc_run_power_flow_st *ltp_RunEnv);

/*长整数比较指令*/
unsigned char run_ldd_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_andd_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ord_compare_ins(plc_run_power_flow_st *ltp_RunEnv);

/*浮点数比较指令*/
unsigned char run_ldr_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_andr_compare_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_orr_compare_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_cmp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_lcmp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rcmp_ins(plc_run_power_flow_st *ltp_RunEnv);
/*位处理指令*/
unsigned char run_ci_bld_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bldi_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_band_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bandi_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bor_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bori_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bset_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_brst_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bout_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_ld_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ldi_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_and_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ani_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_or_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ori_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_set_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rst_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_out_dx_y_ins(plc_run_power_flow_st *ltp_RunEnv);

/*增强行位处理指令*/
unsigned char run_ci_zrst_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_zset_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_deco_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_enco_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bits_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dbits_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bon_ins(plc_run_power_flow_st *ltp_RunEnv);

/*整数运算指令*/
unsigned char run_ci_add_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sub_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_mul_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_div_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sqt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_inc_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dec_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_vabs_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_neg_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_dadd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dsub_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dmul_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ddiv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dsqt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dinc_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ddec_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dvabs_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dneg_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sum_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dsum_ins(plc_run_power_flow_st *ltp_RunEnv);
/*浮点数运算指令*/
unsigned char run_ci_radd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rsub_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rmul_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rdiv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rsqt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rvabs_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rneg_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_cos_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_tan_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_asin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_acos_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_atan_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_power_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ln_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_exp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rsum_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_log_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rad_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_deg_ins(plc_run_power_flow_st *ltp_RunEnv);

/*数值转化指令*/
unsigned char run_ci_dti_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_itd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_flt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dflt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_int_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dint_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bcd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dbcd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dbin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_gry_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dgry_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_gbin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dgbin_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_seg_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_asc_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ita_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ati_ins(plc_run_power_flow_st *ltp_RunEnv);

/*字逻辑运算指令*/
unsigned char run_ci_wand_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wor_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wxor_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_winv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dwand_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dwor_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dwxor_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dwinv_ins(plc_run_power_flow_st *ltp_RunEnv);

/*位移动旋转指令*/
unsigned char run_ci_ror_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rol_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rcr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rcl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dror_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drol_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drcr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_drcl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_shr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_shl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dshr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dshl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sftr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_sftl_ins(plc_run_power_flow_st *ltp_RunEnv);

/*数据块处理指令*/
unsigned char run_ci_bkadd_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bksub_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bkcmp_ins(plc_run_power_flow_st *ltp_RunEnv);

/*LM 元件相关指令*/
unsigned char run_ci_ld_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ldi_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_and_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ani_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_or_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ori_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_set_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_out_lm_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rst_lm_ins(plc_run_power_flow_st *ltp_RunEnv);

/* 数据传输指令 */
unsigned char run_ci_mov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_bmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_fmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dfmov_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_swap_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_swapword_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_xch_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dxch_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_push_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_fifo_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_lifo_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wsfr_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wsfl_ins(plc_run_power_flow_st *ltp_RunEnv);

/* 数据处理指令 */
unsigned char run_ci_mean_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_wtob_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_btow_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_uni_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dis_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rnd_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_lcnv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_rlcnv_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_alt_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_dband_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_limit_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_zone_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_scl_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_ser_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_duty_ins(plc_run_power_flow_st *ltp_RunEnv);

unsigned char run_ci_ramp_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_hackle_ins(plc_run_power_flow_st *ltp_RunEnv);
unsigned char run_ci_triangle_ins(plc_run_power_flow_st *ltp_RunEnv);



#endif /*__PLC_CONTACT_INS_H*/
