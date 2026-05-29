/******************************************************************************
 *                *          ***                    ***
 *              ***          ***                    ***
 * ***  ****  **********     ***        *****       ***  ****          *****
 * *********  **********     ***      *********     ************     *********
 * ****         ***          ***              ***   ***       ****   ***
 * ***          ***  ******  ***      ***********   ***        ****   *****
 * ***          ***  ******  ***    *************   ***        ****      *****
 * ***          ****         ****   ***       ***   ***       ****          ***
 * ***           *******      ***** **************  *************    *********
 * ***             *****        ***   *******   **  **  ******         *****
 *                           t h e  r e a l t i m e  t a r g e t  e x p e r t s
 *
 * http://www.rt-labs.com
 * Copyright (C) 2006. rt-labs AB, Sweden. All rights reserved.
 *------------------------------------------------------------------------------
 * $Id: lw_emac.c 348 2012-10-18 16:41:14Z rtlfrm $
 *
 *
 *------------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stddef.h>

//#include <bsp.h>
//#include <kern.h>
//#include <config.h>
//#include <bfin_dma.h>
#include <string.h>
#include "lw_emac.h"
#include "fsl_debug_console.h"
#include "ether_cat_list.h"
#include "kalyke_tool.h"
#include "ether_cat_task.h"



int bfin_EMAC_init (uint8_t *ethAddr)
{
    LOGV("lw_emac", "Enter %s()", __func__);
    return 0;
}

int bfin_EMAC_send (void *packet, int length)
{
    //LOGV("lw_emac", "Enter %s()", __func__);
    //hexdump(packet, length);
    
    soem_LAN_send((uint8_t *)packet + 14, length - 14);
    return 0;
}

int bfin_EMAC_recv (uint8_t *packet, size_t size)
{
#ifdef SOEM_SEND_RECV_HEXDUMP
    LOGV("lw_emac", "Enter %s(), size = %d", __func__, size);
#endif
    list_element_handle_ec_t pElement = EC_LIST_RemoveHead();
    if (pElement != NULL)
    {
        memcpy(packet, pElement->data, pElement->length);
        vPortFree(pElement);
        #ifdef SOEM_SEND_RECV_HEXDUMP
        hexdump(pElement->data + 14, pElement->length - 14);
        #endif
        return pElement->length;
    }

    return 0;
}

