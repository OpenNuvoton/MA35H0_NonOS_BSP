/*************************************************************************//**
 * @file     ma35h0_mac.h
 * @brief    Packet processor header file for MA35H0
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright(C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __MA35H0_MAC_H__
#define __MA35H0_MAC_H__

#include "NuMicro.h"
#include <string.h>
#include <stdio.h>

/******************************************************************************
 * User configuration
 * NuMaker HMI MA35H0 :
 *  - EMACINTF0 + RMII_100M
 ******************************************************************************/
#define EMAC_INTF    EMACINTF0

#define EMAC_MODE    RMII_100M // mii mode supported by local PHY

#define DEFAULT_MAC0_ADDRESS {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}
#define DEFAULT_MAC1_ADDRESS {0x00, 0x11, 0x22, 0x33, 0x44, 0x66}

/******************************************************************************
 * Functions
 ******************************************************************************/
s32 EMAC_setup_tx_desc_queue(EMACdevice *emacdev, u32 no_of_desc, u32 desc_mode);
s32 EMAC_setup_rx_desc_queue(EMACdevice *emacdev, u32 no_of_desc, u32 desc_mode);
s32 EMAC_set_mode(EMACdevice *emacdev);
s32 EMAC_open(int intf, int mode);
s32 EMAC_register_interrupt(int intf);
void EMAC_giveup_rx_desc_queue(EMACdevice *emacdev, u32 desc_mode);
void EMAC_giveup_tx_desc_queue(EMACdevice *emacdev, u32 desc_mode);
s32 EMAC_close(int intf);
s32 EMAC_xmit_frames(struct sk_buff *skb, int intf, u32 offload_needed, u32 ts);
void EMAC_handle_transmit_over(int intf);
uint32_t EMAC_handle_received_data(int intf, struct sk_buff *prskb);
static void EMAC_powerup_mac(EMACdevice *emacdev);
static void EMAC_powerdown_mac(EMACdevice *emacdev);
uint32_t EMAC_int_handler0(struct sk_buff *prskb);
uint32_t EMAC_int_handler1(struct sk_buff *prskb);

extern EMACdevice EMACdev[];
extern u8 mac_addr0[];
extern u8 mac_addr1[];
extern struct sk_buff txbuf[];
extern struct sk_buff rxbuf[];

#endif /* __MA35H0_MAC_H__ */
