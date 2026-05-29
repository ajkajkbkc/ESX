/**
  ******************************************************************************
  * @file    mb_ctrl.c
  * @author  lixianyu
  * @version V0.0.1
  * @date    2019-03-19
  * @brief   Modbus slave 控制子功能
  ******************************************************************************
  */
#ifndef __PROTOCOL_MB_CTRL_H
#define __PROTOCOL_MB_CTRL_H
void mb_slave_ctrl_run_cmd(md_slave_msg_pack *pMsg);
void mb_slave_ctrl_stop_cmd(md_slave_msg_pack *pMsg);
void mb_slave_ctrl_manage(md_slave_msg_pack *pMsg);
#endif /*__PROTOCOL_MB_CTRL_H*/
