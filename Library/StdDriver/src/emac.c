/**************************************************************************//**
 * @file     emac.c
 * @brief    EMAC driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright(C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include "NuMicro.h"
#include <string.h>
#include <stdio.h>

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup EMAC_Driver EMAC Driver
  @{
*/

/** @addtogroup EMAC_EXPORTED_CONSTANTS EMAC Exported Constants
  @{
*/

/**
  * @brief Delay execution for given amount of ticks.
  *
  * @param[in] ticks target tick count to delay execution to.
  * @return None.
  */
void plat_delay(uint32_t ticks) {
    uint32_t tgtTicks = msTicks0 + ticks;
    while (msTicks0 < tgtTicks);
}

/******************************************************************************
 * PHY
 ******************************************************************************/
/**
 * @brief Function to set the MDC clock for mdio transaction.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] clk_div clk divider value.
 *             - \ref GmiiCsrClk0 : 60-100  MHz
 *             - \ref GmiiCsrClk1 : 100-150 MHz
 *             - \ref GmiiCsrClk2 : 20-35   MHz
 *             - \ref GmiiCsrClk3 : 35-60   MHz
 *             - \ref GmiiCsrClk4 : 150-250 MHz
 *             - \ref GmiiCsrClk5 : 250-300 MHz
 * @return Reuturns 0 on success else return the error value.
 */
s32 EMAC_set_mdc_clk_div(EMACdevice *emacdev,u32 clk_div)
{
    u32 data;

    data = EMAC_READ((u64)&emacdev->MacBase->GmiiAddr);
    data &= ~EMAC_GmiiAddr_CR_Msk;
    data |= clk_div;
    EMAC_WRITE((u64)&emacdev->MacBase->GmiiAddr, data);
    return 0;
}

/**
 * @brief Returns the current MDC divider value programmed in the ip.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return Returns the MDC divider value read.
 */
u32 EMAC_get_mdc_clk_div(EMACdevice *emacdev)
{
    u32 data;

    data = EMAC_READ((u64)&emacdev->MacBase->GmiiAddr);
    data &= EMAC_GmiiAddr_CR_Msk;
    return data;
}

/**
 * @brief Function to read the Phy register. The access to phy register
 * is a slow process as the data is moved accross MDI/MDO interface.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] PhyOffset is the index of one of the 32 phy register.
 * @param[out] data u16 data read from the respective phy register (only valid iff return value is 0).
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_read_phy_reg(EMACdevice *emacdev, u32 PhyOffset, u16 *data)
{
    u32 addr, i;

    addr = ((emacdev->PhyBase << EMAC_GmiiAddr_PA_Pos) & EMAC_GmiiAddr_PA_Msk) | ((PhyOffset << EMAC_GmiiAddr_GR_Pos) & EMAC_GmiiAddr_GR_Msk);
    addr |= EMAC_GmiiAddr_GB_Msk | GmiiCsrClk4; // Gmii busy bit
    EMAC_WRITE((u64)&emacdev->MacBase->GmiiAddr, addr);

    for(i = 0; i < DEFAULT_LOOP_VARIABLE; i++) { //Wait till the busy bit gets cleared with in a certain amount of time
        if(!(EMAC_READ((u64)&emacdev->MacBase->GmiiAddr) & EMAC_GmiiAddr_GB_Msk)) {
            break;
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }
    if(i < DEFAULT_LOOP_VARIABLE)
        *data = (u16)(EMAC_READ((u64)&emacdev->MacBase->GmiiData) & 0xFFFF);
    else {
        TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
        return -EMACPHYERR;
    }
    return 0;
}

/**
 * @brief Function to write to the Phy register. The access to phy register
 * is a slow process as the data is moved accross MDI/MDO interface.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] PhyOffset is the index of one of the 32 phy register.
 * @param[in] data u16 data to be written to the respective phy register.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_write_phy_reg(EMACdevice *emacdev, u32 PhyOffset, u16 data)
{
    u32 addr, i;

    EMAC_WRITE((u64)&emacdev->MacBase->GmiiData, data);

    addr = ((emacdev->PhyBase << EMAC_GmiiAddr_PA_Pos) & EMAC_GmiiAddr_PA_Msk) |
           ((PhyOffset << EMAC_GmiiAddr_GR_Pos) & EMAC_GmiiAddr_GR_Msk) |
           GmiiCsrClk4 | EMAC_GmiiAddr_GW_Msk;
    addr |= EMAC_GmiiAddr_GB_Msk | GmiiCsrClk4; // set Gmii clk to 150-250 Mhz and Gmii busy bit

    EMAC_WRITE((u64)&emacdev->MacBase->GmiiAddr, addr);
    for(i = 0; i < DEFAULT_LOOP_VARIABLE; i++) {
        if(!(EMAC_READ((u64)&emacdev->MacBase->GmiiAddr) & EMAC_GmiiAddr_GB_Msk)) {
            break;
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }

    if(i < DEFAULT_LOOP_VARIABLE) {
        return 0;
    } else {
        TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
        return -EMACPHYERR;
    }
}

/**
 * @brief Function to configure the phy in loopback mode.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] loopback enable or disable the loopback.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_phy_loopback(EMACdevice *emacdev, bool loopback)
{
    s32 ret = 0;
    u16 reg;

    EMAC_read_phy_reg(emacdev, PHY_CONTROL_REG, &reg);

    if(loopback)
        ret = EMAC_write_phy_reg(emacdev, PHY_CONTROL_REG, reg | Mii_Loopback);
    else
        ret = EMAC_write_phy_reg(emacdev, PHY_CONTROL_REG, reg | Mii_NoLoopback);

    return ret;
}

/**
 * @brief Function to perform phy reset.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] mode RGMII or RMII mode.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_perform_phy_reset(EMACdevice *emacdev, int mode)
{
    s32 ret = 0;
    s32 i = DEFAULT_LOOP_VARIABLE;
    u16 data;

    ret = EMAC_write_phy_reg(emacdev, PHY_CONTROL_REG, Mii_reset);
    if(ret)
        return ret;

    while(i-- > 0) {
        ret = EMAC_read_phy_reg(emacdev, PHY_CONTROL_REG, &data);
        if(ret)
            return ret;

        if((data & Mii_reset) == 0) {
            TR("Reset Complete\n");
            break;
        }
    }

    // Adv local ability
    if(mode == RGMII_1G) {
        ret = EMAC_write_phy_reg(emacdev, PHY_AN_ADV_REG,
                                 (Mii_Adv_100_full | Mii_Adv_100_half));
        if(ret)
            return ret;
        ret = EMAC_write_phy_reg(emacdev, PHY_1000BT_CTRL_REG,
                                 (Mii_Adv_1000_full));
        if(ret)
            return ret;
    }
    else { // for 10/100M
        ret = EMAC_write_phy_reg(emacdev, PHY_AN_ADV_REG,
                                 (Mii_Adv_100_full | Mii_Adv_100_half | Mii_Adv_10_full | Mii_Adv_10_half));
        if(ret)
            return ret;
    }

    // restart auto-negotiation
    ret = EMAC_read_phy_reg(emacdev, PHY_CONTROL_REG, &data);
    if(ret)
        return ret;
    ret = EMAC_write_phy_reg(emacdev, PHY_CONTROL_REG, data | Mii_Restart_AN);
    if(ret)
        return ret;

    i = 10000000;
    while(i-- > 0) {
        /* first, a dummy read, needed to latch some MII phys */
        //EMAC_read_phy_reg(emacdev, PHY_STATUS_REG, &data);
        ret = EMAC_read_phy_reg(emacdev, PHY_STATUS_REG, &data);
        if(ret)
            return ret;

        if(data & Mii_AutoNegCmplt) {
            TR("Autonegotiation Complete, 0x%x\n", data);
            break;
        }
    }

    return ret;
}

/******************************************************************************
 * Driver
 ******************************************************************************/
/**
 * @brief Function to read the EMAC IP Version and populates the same in device data structure.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return Always return 0.
 */
void EMAC_read_version(EMACdevice *emacdev)
{
    u32 data = 0;

    data = EMAC_READ((u64)&emacdev->MacBase->Version);
    emacdev->Version = data;
}

/**
 * @brief Function to reset the EMAC core.
 * This reests the DMA and EMAC core. After reset all the registers holds their respective reset value.
 * @param[in] emacdev pointer to EMACdevice.
 * @return 0 on success else return the error status.
 */
s32 EMAC_reset(EMACdevice *emacdev)
{
    u32 data = 0;

    EMAC_SETBITS((u64)&emacdev->MacBase->DmaBusMode, EMAC_DmaBusMode_SWR_Msk); // sw reset IP
    plat_delay(DEFAULT_DELAY_VARIABLE);

    do {
        data = EMAC_READ((u64)&emacdev->MacBase->DmaBusMode);
    } while(data & EMAC_DmaBusMode_SWR_Msk);

    TR("DATA after Reset = %08x\n",data);

    return 0;
}

/**
 * @brief EMAC programmed with the back off limit value.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note This function is tightly coupled with EMAC_RETRY_ENABLE(emacdev).
 */
void EMAC_back_off_limit(EMACdevice *emacdev, u32 val)
{
    u32 data;
    data = EMAC_READ((u64)&emacdev->MacBase->Config);
    data &= ~EMAC_Config_BL_Msk;
    data |= val;
    EMAC_WRITE((u64)&emacdev->MacBase->Config, data);
}

/**
 * @brief Enables forwarding of control frames.
 * When set forwards all the control frames (incl. unicast and multicast PAUSE frames).
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note Depends on RFE of FlowControlRegister[2]
 */
void EMAC_set_pass_control(EMACdevice * emacdev, u32 passcontrol)
{
    u32 data;
    data = EMAC_READ((u64)&emacdev->MacBase->FrameFilter);
    data &= ~EMAC_FrameFilter_PCF_Msk;
    data |= passcontrol;
    EMAC_WRITE((u64)&emacdev->MacBase->FrameFilter, data);
}

/**
 * @brief Initiate Flowcontrol operation.
 * When Set
 * - In full duplex EMAC initiates pause control frame.
 * - In Half duplex EMAC initiates back pressure function.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_tx_activate_flow_control(EMACdevice *emacdev)
{
    //In case of full duplex check for this bit to b'0. if it is read as b'1 indicates that
    //control frame transmission is in progress.
    if(emacdev->DuplexMode == FULLDUPLEX) {
        if(!(EMAC_READ((u64)&emacdev->MacBase->FrameFilter) & EMAC_FlowControl_FCA_BPA_Msk))
            EMAC_SETBITS((u64)&emacdev->MacBase->FlowControl, EMAC_FlowControl_FCA_BPA_Msk);
    } else { //if half duplex mode
        EMAC_SETBITS((u64)&emacdev->MacBase->FlowControl, EMAC_FlowControl_FCA_BPA_Msk);
    }
}

/**
 * @brief Stops Flowcontrol operation.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_tx_deactivate_flow_control(EMACdevice *emacdev)
{
    //In full duplex this bit is automatically cleared after transmitting a pause control frame.
    if(emacdev->DuplexMode == HALFDUPLEX) {
        EMAC_SETBITS((u64)&emacdev->MacBase->FlowControl, EMAC_FlowControl_FCA_BPA_Msk);
    }
}

/**
 * @brief This enables the pause frame generation after programming the appropriate registers.
 * presently activation is set at 3k and deactivation set at 4k. These may have to tweaked
 * if found any issues
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_pause_control(EMACdevice *emacdev)
{
    u32 data;

    /* DMA op mode */
    data = EMAC_READ((u64)&emacdev->MacBase->DmaOpMode);
    data |= DmaThresdholdAct3K | DmaThresdholdDeact4K | EMAC_DmaOpMode_EFC_Msk;
    EMAC_WRITE((u64)&emacdev->MacBase->DmaOpMode, data);

    /* Flow control */
    data = EMAC_READ((u64)&emacdev->MacBase->FlowControl);
    data |= EMAC_FlowControl_RFE_Msk | EMAC_FlowControl_TFE_Msk | EMAC_FlowControl_PT_Msk;
    EMAC_WRITE((u64)&emacdev->MacBase->FlowControl, data);
}

/**
 * @brief Example mac initialization sequence.
 * This function calls the initialization routines to initialize the EMAC register.
 * One can change the functions invoked here to have different configuration as per the requirement
 * @param[in] emacdev pointer to EMACdevice.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_init(EMACdevice *emacdev)
{
    EMAC_WD_ENABLE(emacdev);
    EMAC_JAB_ENABLE(emacdev);
    EMAC_FRAME_BURST_ENABLE(emacdev);
    EMAC_JUMBO_FRAME_DISABLE(emacdev);
    EMAC_RX_OWN_ENABLE(emacdev);
    EMAC_LOOPBACK_DISABLE(emacdev);

    EMAC_RETRY_ENABLE(emacdev);
    EMAC_STRIP_PAD_CRC_DISABLE(emacdev);
    EMAC_back_off_limit(emacdev, EmacBackoffLimit0);
    EMAC_DEFERRAL_CHK_DISABLE(emacdev);

    /*Frame Filter Configuration*/
    EMAC_FR_FILTER_ENABLE(emacdev);
    EMAC_set_pass_control(emacdev, EmacPassControl0);
    EMAC_BROADCAST_ENABLE(emacdev);
    EMAC_SA_FILTER_DISABLE(emacdev);
    EMAC_MULTICAST_DISABLE(emacdev);
    EMAC_DA_FILTER_NORMAL(emacdev);
    EMAC_PROMISC_ENABLE(emacdev);
    EMAC_UPF_DETECT_DISABLE(emacdev);

    /*Flow Control Configuration*/
    if(emacdev->DuplexMode == FULLDUPLEX) {
        EMAC_FULL_DUPLEX(emacdev);
        EMAC_RX_FLOW_CTRL_ENABLE(emacdev);
        EMAC_TX_FLOW_CTRL_ENABLE(emacdev);
    } else { // HALFDUPLEX
        EMAC_HALF_DUPLEX(emacdev);
        EMAC_RX_FLOW_CTRL_DISABLE(emacdev);
        EMAC_TX_FLOW_CTRL_DISABLE(emacdev);
    }

    return 0;
}

/**
 * @brief Checks and initialze phy (compatible with RTL8211/RTL8201).
 * This function checks whether the phy initialization is complete.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] mode RGMII or RMII mode.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_check_phy_init(EMACdevice *emacdev, int mode)
{
    u16 data, status, gstatus;
    s32 ret = 0;
    s32 i;

    EMAC_read_phy_reg(emacdev, PHY_STATUS_REG, &status);
    ret = EMAC_read_phy_reg(emacdev, PHY_STATUS_REG, &status);
    if(ret)
        return ret;

    if((status & Mii_Link) == 0) {
        TR("No Link\n");
        emacdev->LinkState = LINKDOWN;

        /* Link is down, set default value which may be overwrite after negotiation */
        if(mode == RMII_100M)
        {
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = FULLDUPLEX;
        }
        else
        {
            emacdev->Speed = SPEED1000;
            emacdev->DuplexMode = FULLDUPLEX;
        }

        return EMACPHYERR;
    } else {
        emacdev->LinkState = LINKUP;
        sysprintf("Link UP, checking PHY status...\n");

        if(mode == RMII_100M)
        {
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = FULLDUPLEX;
            sysprintf("MII : 100M FULLDUPLEX (Default)\n");
            return 0;
        }
        // go on if RGMII
    }

    /* PHY reset */
    if(EMAC_perform_phy_reset(emacdev, mode)) {
        TR("PHY reset fail\n");
        return -1;
    }

    /* check partner ablility */
    EMAC_read_phy_reg(emacdev, PHY_AN_LNK_PART_ABL_REG, &status);

    if(mode == RGMII_1G) {
        EMAC_read_phy_reg(emacdev, PHY_1000BT_STATUS_REG, &gstatus);

        if(gstatus & Mii_partner_1000_full) {
            sysprintf("GMII : 1G FULLDUPLEX\n");
            emacdev->Speed = SPEED1000;
            emacdev->DuplexMode = FULLDUPLEX;
        }
        else if(status & Mii_Adv_100_full) {
            sysprintf("MII : 100M FULLDUPLEX\n");
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = FULLDUPLEX;
        }
        else if(status & Mii_Adv_100_half) {
            sysprintf("MII : 100M HALFDUPLEX\n");
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = HALFDUPLEX;
        }
        else {
            sysprintf("Linked partner not support\n");
            return -1;
        }
    }
    else {
        if(status & Mii_Adv_100_full) {
            sysprintf("MII : 100M FULLDUPLEX\n");
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = FULLDUPLEX;
        }
        else if(status & Mii_Adv_100_half) {
            sysprintf("MII : 100M HALFDUPLEX\n");
            emacdev->Speed = SPEED100;
            emacdev->DuplexMode = HALFDUPLEX;
        }
        else if(status & Mii_Adv_10_full) {
            sysprintf("MII : 10M FULLDUPLEX\n");
            emacdev->Speed = SPEED10;
            emacdev->DuplexMode = FULLDUPLEX;
        }
        else if(status & Mii_Adv_10_half) {
            sysprintf("MII : 10M HALFDUPLEX\n");
            emacdev->Speed = SPEED10;
            emacdev->DuplexMode = HALFDUPLEX;
        }
        else {
            sysprintf("Not support by linked partner, partner ability : %x\n", status);
            return -1;
        }
    }

    if(emacdev->DuplexMode == HALFDUPLEX) {
        EMAC_write_phy_reg(emacdev, PHY_PAGE_SELECT_REG, 0xa43); // change to page 0xa43
        /*Enable CRS on Transmit*/
        EMAC_read_phy_reg(emacdev, PHY_SPECIFIC_CTRL1_REG, &data);
        EMAC_write_phy_reg(emacdev, PHY_SPECIFIC_CTRL1_REG, data | 0x80); // crs : bit 7
    }

    return 0;
}

/**
 * @brief Detect cable plug/unplug
 * This function should be used to check phy link status perioldically
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] mode RGMII or RMII mode.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_link_monitor(EMACdevice *emacdev, int mode)
{
    u16 data, status;
    s32 ret = 0;

    ret = EMAC_read_phy_reg(emacdev, PHY_STATUS_REG, &status);
    if(ret)
        return ret;

    if(status & Mii_Link) // LINKUP
    {
        if(emacdev->LinkState == LINKDOWN)
        {
            ret = EMAC_check_phy_init(emacdev, mode);
            if(ret < 0)
                return -1;
            EMAC_init(emacdev);
        }
    }
    else
    {
        emacdev->LinkState = LINKDOWN;
        sysprintf("Link is down.\r");
        ret = -1;
    }
    return ret;
}

/**
 * @brief Sets the Mac address in to EMAC register.
 * This function sets the MAC address to the MAC register in question.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] AddrID MAC address 0~8.
 * @param[in] MacAddr buffer containing mac address to be programmed.
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_set_mac_addr(EMACdevice *emacdev, u32 AddrID, u8 *MacAddr)
{
    u32 data;

    if(AddrID > 8)
        return -1;

    data = (MacAddr[5] << 8) | MacAddr[4];
    EMAC_WRITE(((u64)&emacdev->MacBase->Addr0High + AddrID * 8), data);

    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0] ;
    EMAC_WRITE(((u64)&emacdev->MacBase->Addr0Low + AddrID * 8), data);
    return 0;
}

/**
 * @brief Attaches the EMACdevice structure to the hardware.
 * Device structure is populated with MAC/DMA and PHY base addresses.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] intf EMAC IP mac base address. EMAC0 or EMAC1
 * @param[in] phyBase EMAC IP phy base address. 0-31
 * @return Returns 0 on success else return the error status.
 */
s32 EMAC_attach(EMACdevice *emacdev, u32 intf, u32 phyBase)
{
    /*Make sure the Device data strucure is cleared before we proceed further*/
    memset((void *)emacdev, 0, sizeof(EMACdevice));

    if(intf == EMACINTF0) {
        emacdev->Intf = EMACINTF0;
        emacdev->MacBase = EMAC0;
    }
    else if(intf == EMACINTF1) {
        emacdev->Intf = EMACINTF1;
        emacdev->MacBase = EMAC1;
    }
    else
        return -1;

    emacdev->PhyBase = phyBase;

    return 0;
}

/**
 * @brief Set RGMII/RMII speed 1000/100/10M.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return Returns 0 on success else return the error status.
 */
void EMAC_set_mii_speed(EMACdevice *emacdev)
{
    if(emacdev->Speed == SPEED1000) {
        EMAC_PORT_SEL_GMII(emacdev);
    }
    else if(emacdev->Speed == SPEED100) {
        EMAC_PORT_SEL_MII(emacdev);
        EMAC_SPD_SEL_100M(emacdev);
    }
    else {
        EMAC_PORT_SEL_MII(emacdev);
        EMAC_SPD_SEL_10M(emacdev);
    }
}

/******************************************************************************
 * Descriptor
 ******************************************************************************/
/**
 * @brief Initialize the rx descriptors for ring or chain mode operation.
 * - Status field is initialized to 0.
 * - EndOfRing set for the last descriptor.
 * - buffer1 and buffer2 set to 0 for ring mode of operation.
 * @param[in] desc pointer to DmaDesc structure.
 * @param[in] last_ring_desc whether end of ring
 * @return None.
 * @note Initialization of the buffer1, buffer2, status are not done here. This only initializes whether one wants to use this descriptor
 * in chain mode or ring mode. For chain mode of operation the buffer2 and data2 are programmed before calling this function.
 */
void EMAC_rx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
    desc->status = 0;
    desc->length = last_ring_desc ? eDescRxEndOfRing : 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
}

/**
 * @brief Initialize the tx descriptors for ring or chain mode operation.
 * - Status field is initialized to 0.
 * - EndOfRing set for the last descriptor.
 * - buffer1 and buffer2 set to 0 for ring mode of operation.
 * @param[in] desc pointer to DmaDesc structure.
 * @param[in] last_ring_desc whether end of ring
 * @return void.
 * @note Initialization of the buffer1, buffer2, status are not done here. This only initializes whether one wants to use this descriptor
 * in chain mode or ring mode. For chain mode of operation the buffer2 and data2 are programmed before calling this function.
 */
void EMAC_tx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
    desc->status = last_ring_desc ? eDescTxEndOfRing : 0;
    desc->length = 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
}

/**
 * @brief Initialize the rx descriptors queue.
 * This function should be called after emacdev->TxDesc & emacdev->RxDesc is allocated
 * @param[in] emacdev pointer to EMACdevice.
 * @return Always return 0.
 */
s32 EMAC_init_tx_rx_desc_queue(EMACdevice *emacdev)
{
    s32 i;

    for(i = 0; i < emacdev->TxDescCount; i++) {
        EMAC_tx_desc_init_ring(emacdev->TxDesc + i, i == emacdev->TxDescCount - 1);
    }

    for(i = 0; i < emacdev->RxDescCount; i++) {
        EMAC_rx_desc_init_ring(emacdev->RxDesc + i, i == emacdev->RxDescCount - 1);
    }

    emacdev->TxNext = 0;
    emacdev->TxBusy = 0;
    emacdev->RxNext = 0;
    emacdev->RxBusy = 0;

    return 0;
}

/**
 * @brief Programs the DmaRxBaseAddress with the Rx descriptor base address.
 * Rx Descriptor's base address is available in the emacdev structure. This function progrms the
 * Dma Rx Base address with the starting address of the descriptor ring or chain.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_init_rx_desc_base(EMACdevice *emacdev)
{
    EMAC_WRITE((u64)&emacdev->MacBase->DmaRxDesc, (u32)((u64)emacdev->RxDescDma & 0xFFFFFFFF));
}

/**
 * @brief Programs the DmaTxBaseAddress with the Tx descriptor base address.
 * Tx Descriptor's base address is available in the emacdev structure. This function progrms the
 * Dma Tx Base address with the starting address of the descriptor ring or chain.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_init_tx_desc_base(EMACdevice *emacdev)
{
    EMAC_WRITE((u64)&emacdev->MacBase->DmaTxDesc, (u32)((u64)emacdev->TxDescDma & 0xFFFFFFFF));
}

/**
 * @brief Check whether descriptor is in enhanced/alternative mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @return whether enhanced mode or not.
 */
bool EMAC_is_desc_enhanced_mode(EMACdevice *emacdev)
{
    return (EMAC_READ((u64)&emacdev->MacBase->DmaBusMode) & EMAC_DmaBusMode_ATDS_Msk);
}

/**
 * @brief Checks whether the descriptor is owned by DMA.
 * If descriptor is owned by DMA then the OWN bit is set to 1. This API is same for both ring and chain mode.
 * @param[in] desc pointer to DmaDesc structure.
 * @return true if Dma owns descriptor and false if not.
 */
bool EMAC_is_desc_owned_by_dma(DmaDesc *desc)
{
    return ((desc->status & DescOwnByDma) == DescOwnByDma);
}

/**
 * @brief Returns the byte length of received frame including CRC.
 * This returns the no of bytes received in the received ethernet frame including CRC(FCS).
 * @param[in] status status of descriptor.
 * @return returns the length of received frame lengths in bytes.
 */
u32 EMAC_get_rx_desc_frame_length(u32 status)
{
    return ((status & DescRxFrameLengthMask) >> DescRxFrameLengthShift);
}

/**
 * @brief Checks whether the descriptor is valid
 * if no errors such as CRC/Receive Error/Watchdog Timeout/Late collision/Giant Frame/Overflow/Descriptor
 * error the descritpor is said to be a valid descriptor.
 * @param[in] status status of descriptor.
 * @return True if desc valid. false if error.
 */
bool EMAC_is_desc_valid(u32 status)
{
    return ((status & DescError) == 0);
}

/**
 * @brief Checks whether the descriptor is empty.
 * If the buffer1 and buffer2 lengths are zero in ring mode descriptor is empty.
 * In chain mode buffer2 length is 0 but buffer2 itself contains the next descriptor address.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] desc pointer to DmaDesc structure.
 * @return returns true if descriptor is empty, false if not empty.
 */
bool EMAC_is_desc_empty(EMACdevice *emacdev, DmaDesc *desc)
{
    //if both the buffer1 length and buffer2 length are zero desc is empty
    if(EMAC_is_desc_enhanced_mode(emacdev))
        return(((desc->length  & eDescSize1Mask) == 0) && ((desc->length  & eDescSize2Mask) == 0));
    else
        return(((desc->length  & nDescSize1Mask) == 0) && ((desc->length  & nDescSize2Mask) == 0));
}

/**
 * @brief Checks whether the rx descriptor is valid.
 * if rx descripor is not in error and complete frame is available in the same descriptor
 * @param[in] status status of descriptor.
 * @return returns true if no error and first and last desc bits are set, otherwise it returns false.
 */
bool EMAC_is_rx_desc_valid(u32 status)
{
    return ((status & DescError) == 0) && ((status & DescRxFirst) == DescRxFirst) && ((status & DescRxLast) == DescRxLast);
}

/**
 * @brief Checks whether the tx is aborted due to collisions.
 * @param[in] status status of descriptor.
 * @return returns true if collisions, else returns false.
 */
bool EMAC_is_tx_aborted(u32 status)
{
    return (((status & DescTxLateCollision) == DescTxLateCollision) | ((status & DescTxExcCollisions) == DescTxExcCollisions));
}

/**
 * @brief Checks whether the tx carrier error.
 * @param[in] status status of descriptor.
 * @return returns true if carrier error occured, else returns falser.
 */
bool EMAC_is_tx_carrier_error(u32 status)
{
    return (((status & DescTxLostCarrier) == DescTxLostCarrier) | ((status & DescTxNoCarrier) == DescTxNoCarrier));
}

/**
 * @brief Gives the transmission collision count.
 * returns the transmission collision count indicating number of collisions occured before the frame was transmitted.
 * Make sure to check excessive collision didnot happen to ensure the count is valid.
 * @param[in] status status of descriptor.
 * @return returns the count value of collision.
 */
u32 EMAC_get_tx_collision_count(u32 status)
{
    return ((status & DescTxCollMask) >> DescTxCollShift);
}

/**
 * @brief Checks whether transmission collision count excceed 16.
 * @param[in] status status of descriptor.
 * @return returns true if collision count excceed 16, else returns falser.
 */
u32 EMAC_is_exc_tx_collisions(u32 status)
{
    return ((status & DescTxExcCollisions) == DescTxExcCollisions);
}

/**
 * @brief Check for damaged frame due to overflow or collision.
 * Retruns true if rx frame was damaged due to buffer overflow in MTL or late collision in half duplex mode.
 * @param[in] status status of descriptor.
 * @return returns true if error else returns false.
 */
bool EMAC_is_rx_frame_damaged(u32 status)
{
    return (((status & DescRxDamaged) == DescRxDamaged) | ((status & DescRxCollision) == DescRxCollision));
}

/**
 * @brief Check whether timestamp is available or not.
 *
 * @param[in] status status of descriptor.
 * @return returns true if timestamp available else returns false.
 */
bool EMAC_is_timestamp_available(u32 status)
{
    return ((status & DescRxTSAvailable) == DescRxTSAvailable);
}

/**
 * @brief Check for damaged rx frame due to collision.
 * Retruns true if rx frame was damaged due to late collision in half duplex mode.
 * @param[in] status status of descriptor.
 * @return returns true if rx frame collosion else returns false.
 */
bool EMAC_is_rx_frame_collision(u32 status)
{
    return ((status & DescRxCollision) == DescRxCollision);
}

/**
 * @brief Check for receive CRC error.
 * Retruns true if rx frame CRC error occured.
 * @param[in] status status of descriptor.
 * @return returns true if CRC error else returns false.
 */
bool EMAC_is_rx_crc(u32 status)
{
    return ((status & DescRxCrc) == DescRxCrc);
}

/**
 * @brief Indicates rx frame has non integer multiple of bytes. (odd nibbles).
 * Retruns true if dribbling error in rx frame.
 * @param[in] status status of descriptor.
 * @return returns true if error else returns false.
 */
bool EMAC_is_frame_dribbling_errors(u32 status)
{
    return ((status & DescRxDribbling) == DescRxDribbling);
}

/**
 * @brief Indicates error in rx frame length.
 * Retruns true if received frame length doesnot match with the length field
 * @param[in] status status of descriptor.
 * @return returns true if error else returns false.
 */
bool EMAC_is_rx_frame_length_errors(u32 status)
{
    return((status & DescRxLengthError) == DescRxLengthError);
}

/**
 * @brief Checks whether this rx descriptor is last rx descriptor.
 * This returns true if it is last descriptor either in ring mode or in chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] desc pointer to DmaDesc structure.
 * @return returns true if it is last descriptor, false if not.
 */
bool EMAC_is_last_rx_desc(EMACdevice *emacdev, DmaDesc *desc)
{
    if(EMAC_is_desc_enhanced_mode(emacdev))
        return ((desc->length & eDescRxEndOfRing) == eDescRxEndOfRing);
    else
        return ((desc->length & nDescRxEndOfRing) == nDescRxEndOfRing);
}

/**
 * @brief Checks whether this tx descriptor is last tx descriptor.
 * This returns true if it is last descriptor either in ring mode or in chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] desc pointer to DmaDesc structure.
 * @return returns true if it is last descriptor, false if not.
 */
bool EMAC_is_last_tx_desc(EMACdevice *emacdev, DmaDesc *desc)
{
    if(EMAC_is_desc_enhanced_mode(emacdev))
        return ((desc->status & eDescTxEndOfRing) == eDescTxEndOfRing);
    else
        return ((desc->status & nDescTxEndOfRing) == nDescTxEndOfRing);
}

/**
 * @brief Decodes the Rx Descriptor status to various checksum error conditions.
 *
 * @param[in] status status of descriptor.
 * @return returns decoded enum RxDescCOEEncode indicating the status.
 */
u32 EMAC_is_rx_checksum_error(u32 status)
{
    if(((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) == 0) && ((status & nDescRxChkBit0) == 0))
        return RxLenLT600;
    else if(((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) == 0) && ((status & nDescRxChkBit0) != 0))
        return RxIpHdrPayLoadChkBypass;
    else if(((status & DescRxChkBit5) == 0) && ((status & DescRxChkBit7) != 0) && ((status & nDescRxChkBit0) != 0))
        return RxChkBypass;
    else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) == 0) && ((status & nDescRxChkBit0) == 0))
        return RxNoChkError;
    else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) == 0) && ((status & nDescRxChkBit0) != 0))
        return RxPayLoadChkError;
    else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) != 0) && ((status & nDescRxChkBit0) == 0))
        return RxIpHdrChkError;
    else if(((status & DescRxChkBit5) != 0) && ((status & DescRxChkBit7) != 0) && ((status & nDescRxChkBit0) != 0))
        return RxIpHdrPayLoadChkError;
    else
        return RxIpHdrPayLoadRes;
}

/**
 * @brief Checks if any Ipv4 header checksum error in the frame just transmitted.
 * This serves as indication that error occureed in the IPv4 header checksum insertion.
 * The sent out frame doesnot carry any ipv4 header checksum inserted by the hardware.
 * @param[in] status status of descriptor.
 * @return returns true if error in ipv4 header checksum, else returns false.
 */
bool EMAC_is_tx_ipv4header_checksum_error(u32 status)
{
    return ((status & DescTxIpv4ChkError) == DescTxIpv4ChkError);
}

/**
 * @brief Checks if any payload checksum error in the frame just transmitted.
 * This serves as indication that error occureed in the payload checksum insertion.
 * The sent out frame doesnot carry any payload checksum inserted by the hardware.
 * @param[in] status status of descriptor.
 * @return returns true if payload checksum error, else returns false.
 */
bool EMAC_is_tx_payload_checksum_error(u32 status)
{
    return ((status & DescTxPayChkError) == DescTxPayChkError);
}

/**
 * @brief The checksum offload engine is bypassed in the tx path.
 * Checksum is not computed in the Hardware.
 * @param[in] desc pointer to DmaDesc structure.
 * @return None.
 */
void EMAC_tx_checksum_offload_bypass(DmaDesc *desc)
{
    desc->status = (desc->status & (~eDescTxCisMask));//ENH_DESC
}

/**
 * @brief The checksum offload engine is enabled to do only IPV4 header checksum.
 * IPV4 header Checksum is computed in the Hardware.
 * @param[in] desc pointer to DmaDesc structure.
 * @return None.
 */
void EMAC_tx_checksum_offload_ipv4hdr(DmaDesc *desc)
{
    desc->status = ((desc->status & (~eDescTxCisMask)) | eDescTxCisIpv4HdrCs);//ENH_DESC
}

/**
 * @brief The checksum offload engine is enabled to do TCPIP checsum assuming Pseudo header is available.
 * Hardware computes the tcp ip checksum assuming pseudo header checksum is computed in software.
 * Ipv4 header checksum is also inserted.
 * @param[in] desc pointer to DmaDesc structure.
 * @return None.
 */
void EMAC_tx_checksum_offload_tcponly(DmaDesc *desc)
{
    desc->status = ((desc->status & (~eDescTxCisMask)) | eDescTxCisTcpOnlyCs);//ENH_DESC
}

/**
 * @brief The checksum offload engine is enabled to do complete checksum computation.
 * Hardware computes the tcp ip checksum including the pseudo header checksum.
 * Here the tcp payload checksum field should be set to 0000.
 * Ipv4 header checksum is also inserted.
 * @param[in] desc pointer to DmaDesc structure.
 * @return None.
 */
void EMAC_tx_checksum_offload_tcp_pseudo(DmaDesc *desc)
{
    desc->status = ((desc->status & (~eDescTxCisMask)) | eDescTxCisTcpPseudoCs);//ENH_DESC
}

/**
 * @brief This function indicates whether extended status is available in the RDES0.
 * Any function which accesses the fields of extended status register must ensure a check on this has been made
 * This is valid only for Enhanced Descriptor.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] status u32 status field of the corresponding descriptor.
 * @return returns TRUE or FALSE.
 * @note extended status present indicates that the RDES4 need to be probed
 */
bool EMAC_is_ext_status(EMACdevice *emacdev, u32 status)
{
    if(EMAC_is_desc_enhanced_mode(emacdev))
        return ((status & eDescRxEXTsts) != 0); // if extstatus set then it returns 1
    else
        return 0;
}

/**
 * @brief This function returns true if the IP header checksum bit is set in the extended status.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] ext_status extended status of descriptor.
 * @return returns true if error in ipv4 header checksum, else returns false.
 */
bool EMAC_ES_is_IP_header_error(u32 ext_status)          // IP header (IPV4) checksum error
{
    return ((ext_status & eDescRxIpHeaderError) != 0); // if IPV4 header error return 1
}

/**
 * @brief This function returns true if the Checksum is bypassed in the hardware.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] ext_status extended status of descriptor.
 * @return returns true if hardware engine bypassed the checksum computation/checking, else returns false
 */
bool EMAC_ES_is_rx_checksum_bypassed(u32 ext_status)
{
    return ((ext_status & eDescRxChkSumBypass ) != 0); // if checksum offloading bypassed return 1
}

/**
 * @brief This function returns true if payload checksum error is set in the extended status.
 * Valid only when enhaced status available is set in RDES0 bit 0.
 * This is valid only for Enhanced Descriptor.
 * @param[in] ext_status extended status of descriptor.
 * @return returns true if error in IP payload checksum, else returns false.
 */
bool EMAC_ES_is_IP_payload_error(u32 ext_status)         // IP payload checksum is in error (UDP/TCP/ICMP checksum error)
{
    return ((ext_status & eDescRxIpPayloadError) != 0); // if IP payload error return 1
}

/**
 * @brief This function is defined two times. Once when the code is compiled for ENHANCED DESCRIPTOR SUPPORT and Once for Normal descriptor
 * Get the index and address of Tx desc.
 * This api is same for both ring mode and chain mode.
 * This function tracks the tx descriptor the DMA just closed after the transmission of data from this descriptor is
 * over. This returns the descriptor fields to the caller.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[out] status status of descriptor.
 * @param[out] Length length of buffer1 (Max is 2048).
 * @param[out] Buffer1 Dma-able buffer1 pointer.
 * @param[out] Buffer2 Dma-able buffer2 pointer.
 * @param[out] ExtStatus extended status of descriptor.
 * @param[out] TSLow timestamp lower DWORD
 * @param[out] TSHigh timestamp higher DWORD
 * @return returns present tx descriptor index on success. Negative value if error.
 */
s32 EMAC_get_tx_qptr(EMACdevice *emacdev, u32 *Status, u32 *Length, u32 *Buffer1, u32 *Buffer2, u32 *ExtStatus, u32 *TSLow, u32 *TSHigh)
{
    u32 txover = emacdev->TxBusy;
#ifdef CACHE_ON
    DmaDesc *txdesc = (DmaDesc *)((uint64_t)(emacdev->TxBusyDesc) | NON_CACHE);
#else
    DmaDesc *txdesc = emacdev->TxBusyDesc;
#endif
    if(EMAC_is_desc_owned_by_dma(txdesc))
        return -1;
    if(EMAC_is_desc_empty(emacdev, txdesc))
        return -1;

    (emacdev->BusyTxDesc)--; //busy tx descriptor is reduced by one as it will be handed over to Processor now

    if(Status)
        *Status = txdesc->status;
    if(Buffer1)
        *Buffer1 = txdesc->buffer1;
    if(Buffer2)
        *Buffer2 = txdesc->buffer2;

    if(EMAC_is_desc_enhanced_mode(emacdev)) {
        if(Length)
            *Length = (txdesc->length & eDescSize1Mask) >> eDescSize1Shift;
        if(ExtStatus)
            *ExtStatus = txdesc->extstatus;
        if(TSHigh)
            *TSHigh = txdesc->timestamphigh;
        if(TSLow)
            *TSLow = txdesc->timestamplow;
    }
    else {
        if(Length)
            *Length = (txdesc->length & nDescSize1Mask) >> nDescSize1Shift;
    }

    emacdev->TxBusy = EMAC_is_last_tx_desc(emacdev, txdesc) ? 0 : txover + 1;

    if(1 /* ring mode */) {
        emacdev->TxBusyDesc = EMAC_is_last_tx_desc(emacdev, txdesc) ? emacdev->TxDesc : (txdesc + 1);
        EMAC_tx_desc_init_ring(txdesc, EMAC_is_last_tx_desc(emacdev, txdesc));
    }
    TR("(get)%02d %08x %08x %08x %08x %08x\n",txover,(u32)((u64)txdesc & 0xFFFFFFFF),txdesc->status,txdesc->length,txdesc->buffer1,txdesc->buffer2);

    return txover;
}

/**
 * @brief This function enable hardware CRC replacement.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_set_crc_replacement(EMACdevice *emacdev)
{
    /* This is for desc enhanced mode */
#ifdef CACHE_ON
    DmaDesc *txdesc = (DmaDesc *)((uint64_t)(emacdev->TxNextDesc) | NON_CACHE);
#else
    DmaDesc *txdesc = emacdev->TxNextDesc;
#endif
    txdesc->status |= eDescTxDisableCrc | eDescTxCrcReplacement;

    emacdev->TxPrevDesc = txdesc;
}

/**
 * @brief This function disable hardware CRC replacement.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note This function should be called after EMAC_set_crc_replacement
 */
void EMAC_clr_crc_replacement(EMACdevice *emacdev)
{
    if(emacdev->TxPrevDesc) {
        emacdev->TxPrevDesc->status &= ~(eDescTxDisableCrc | eDescTxCrcReplacement);
        emacdev->TxPrevDesc = NULL;
    }
}

/**
 * @brief Populate the tx desc structure with the buffer address.
 * Once the driver has a packet ready to be transmitted, this function is called with the
 * valid dma-able buffer addresses and their lengths. This function populates the descriptor
 * and make the DMA the owner for the descriptor. This function also controls whetther Checksum
 * offloading to be done in hardware or not.
 * This api is same for both ring mode and chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] Length length of buffer1 (Max is 2048).
 * @param[in] Buffer1 Dma-able buffer1 pointer.
 * @param[in] offload_needed indicating whether the checksum offloading in HW/SW.
 * @param[in] ts indicating whether timestamp
 * @return returns present tx descriptor index on success. Negative value if error.
 */
s32 EMAC_set_tx_qptr(EMACdevice *emacdev, u32 Length, u32 Buffer1, u32 offload_needed, u32 ts)
{
    u32 txnext = emacdev->TxNext;
#ifdef CACHE_ON
    DmaDesc *txdesc = (DmaDesc *)((uint64_t)(emacdev->TxNextDesc) | NON_CACHE);
#else
    DmaDesc *txdesc = emacdev->TxNextDesc;
#endif
    if(!EMAC_is_desc_empty(emacdev, txdesc))
        return -1;

    (emacdev->BusyTxDesc)++; //busy tx descriptor is incremented by one as it will be handed over to DMA

    if(1 /* ring mode */) {
        if(EMAC_is_desc_enhanced_mode(emacdev)) {
            txdesc->length |= ((Length << eDescSize1Shift) & eDescSize1Mask);
            txdesc->status |= (eDescTxFirstSeg | eDescTxLastSeg | eDescTxIntOnCompl | (ts == 1 ? eDescTxTSEnable : 0));
        } else {
            txdesc->length |= ((Length << nDescSize1Shift) & nDescSize1Mask) |
                              nDescTxFirstSeg | nDescTxLastSeg | nDescTxIntOnCompl | (ts == 1 ? nDescTxTSEnable : 0);
            offload_needed = 0;
        }

        //memcpy((void *)((u64)(tx_buf[][txnext]->Data) | NON_CACHE), (void *)((u64)Buffer1), Length1);
        txdesc->buffer1 = Buffer1;

        if(offload_needed) {
            /*
             Make sure that the OS you are running supports the IP and TCP checkusm offloaidng,
             before calling any of the functions given below.
             */
            txdesc->status = ((txdesc->status & (~eDescTxCisMask)) | eDescTxCisTcpPseudoCs);
        } else {
            txdesc->status = txdesc->status & (~eDescTxCisMask);
        }
        __DSB();
        txdesc->status |= DescOwnByDma;

        emacdev->TxNext = EMAC_is_last_tx_desc(emacdev, txdesc) ? 0 : txnext + 1;
        emacdev->TxNextDesc = EMAC_is_last_tx_desc(emacdev, txdesc) ? emacdev->TxDesc : (txdesc + 1);
    }

    TR("(set)%02d %08x %08x %08x %08x %08x\n",txnext,(u32)((u64)txdesc & 0xFFFFFFFF),txdesc->status,txdesc->length,txdesc->buffer1,txdesc->buffer2);

    return txnext;
}

/**
 * @brief Prepares the descriptor to receive packets.
 * The descriptor is allocated with the valid buffer addresses (sk_buff address) and the length fields
 * and handed over to DMA by setting the ownership. After successful return from this function the
 * descriptor is added to the receive descriptor pool/queue.
 * This api is same for both ring mode and chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] Buffer1 Dma-able buffer1 pointer.
 * @param[in] Length1 length of buffer1 (Max is 2048).
 * @return returns present rx descriptor index on success. Negative value if error.
 */
s32 EMAC_set_rx_qptr(EMACdevice *emacdev, u32 Buffer1, u32 Length1)
{
    u32 rxnext = emacdev->RxNext;
#ifdef CACHE_ON
    DmaDesc *rxdesc = (DmaDesc *)((uint64_t)(emacdev->RxNextDesc) | NON_CACHE);
#else
    DmaDesc *rxdesc = emacdev->RxNextDesc;
#endif
    if(!EMAC_is_desc_empty(emacdev, rxdesc))
        return -1;

    if(EMAC_is_desc_enhanced_mode(emacdev))
        rxdesc->length |= ((Length1 << eDescSize1Shift) & eDescSize1Mask);
    else
        rxdesc->length |= ((Length1 << nDescSize1Shift) & nDescSize1Mask);
    rxdesc->buffer1 = Buffer1;
    rxdesc->buffer2 = 0;
    rxdesc->extstatus = 0;
    rxdesc->reserved1 = 0;
    rxdesc->timestamplow = 0;
    rxdesc->timestamphigh = 0;

    if((rxnext % MODULO_INTERRUPT) != 0)
        rxdesc->length |= DescRxDisIntCompl;

    rxdesc->status = DescOwnByDma;

    emacdev->RxNext     = EMAC_is_last_rx_desc(emacdev, rxdesc) ? 0 : rxnext + 1;
    emacdev->RxNextDesc = EMAC_is_last_rx_desc(emacdev, rxdesc) ? emacdev->RxDesc : (rxdesc + 1);

    TR("%02d %08x %08x %08x %08x %08x\n",rxnext,(u32)((u64)rxdesc & 0xFFFFFFFF),rxdesc->status,rxdesc->length,rxdesc->buffer1,rxdesc->buffer2);

    (emacdev->BusyRxDesc)++; //One descriptor will be given to Hardware. So busy count incremented by one

    return rxnext;
}

/**
 * @brief This function is defined two times. Once when the code is compiled for ENHANCED DESCRIPTOR SUPPORT and Once for Normal descriptor
 * Get back the descriptor from DMA after data has been received.
 * When the DMA indicates that the data is received (interrupt is generated), this function should be
 * called to get the descriptor and hence the data buffers received. With successful return from this
 * function caller gets the descriptor fields for processing. check the parameters to understand the
 * fields returned.`
 * @param[in] emacdev pointer to EMACdevice.
 * @param[out] status status of descriptor.
 * @param[out] Length length of buffer1 (Max is 2048).
 * @param[out] Buffer1 Dma-able buffer1 pointer.
 * @param[out] Buffer2 Dma-able buffer2 pointer.
 * @param[out] ExtStatus extended status of descriptor.
 * @param[out] TSLow timestamp lower DWORD
 * @param[out] TSHigh timestamp higher DWORD
 * @return returns present rx descriptor index on success. Negative value if error.
 */
s32 EMAC_get_rx_qptr(EMACdevice *emacdev, u32 *Status, u32 *Length, u32 *Buffer1, u32 *Buffer2, u32 *ExtStatus, u32 *TSHigh, u32 *TSLow)
{
    u32 rxnext = emacdev->RxBusy; // index of descriptor the DMA just completed. May be useful when data
    //is spread over multiple buffers/descriptors
#ifdef CACHE_ON
    DmaDesc *rxdesc = (DmaDesc *)((uint64_t)(emacdev->RxBusyDesc) | NON_CACHE);
#else
    DmaDesc *rxdesc = emacdev->RxBusyDesc;
#endif
    if(EMAC_is_desc_owned_by_dma(rxdesc))
        return -1;
    if(EMAC_is_desc_empty(emacdev, rxdesc))
        return -1;

    if(Status != 0)
        *Status = rxdesc->status;// send the status of this descriptor
    if(Buffer1 != 0)
        *Buffer1 = rxdesc->buffer1;
    if(Buffer2 != 0)
        *Buffer2 = rxdesc->buffer2;

    if(EMAC_is_desc_enhanced_mode(emacdev)) {
        if(Length != 0)
            *Length = (rxdesc->length & eDescSize1Mask) >> eDescSize1Shift;
        if(ExtStatus != 0)
            *ExtStatus = rxdesc->extstatus;
        if(TSHigh != 0)
            *TSHigh = rxdesc->timestamphigh;
        if(TSLow != 0)
            *TSLow = rxdesc->timestamplow;
    }
    else {
        if(Length != 0)
            *Length = (rxdesc->length & nDescSize1Mask) >> nDescSize1Shift;
    }

    emacdev->RxBusy     = EMAC_is_last_rx_desc(emacdev, rxdesc) ? 0 : rxnext + 1;

    emacdev->RxBusyDesc = EMAC_is_last_rx_desc(emacdev, rxdesc) ? emacdev->RxDesc : (rxdesc + 1);
    rxdesc->status = DescOwnByDma;
    rxdesc->extstatus = 0;
    rxdesc->reserved1 = 0;
    rxdesc->timestamplow = 0;
    rxdesc->timestamphigh = 0;
    TR("%02d %08x %08x %08x %08x %08x\n",rxnext,(u32)((u64)rxdesc & 0xFFFFFFFF),rxdesc->status,rxdesc->length,rxdesc->buffer1,rxdesc->buffer2);
    (emacdev->BusyRxDesc)--; //busy tx descriptor is reduced by one as it will be handed over to Processor now

    return(rxnext);
}

/**
 * @brief Take ownership of this Descriptor.
 * The function is same for both the ring mode and the chain mode DMA structures.
 * @param[in] desc pointer to DmaDesc structure.
 * @return None.
 */
void EMAC_take_desc_ownership(DmaDesc *desc)
{
    if(desc) {
        desc->status &= ~DescOwnByDma;  //Clear the DMA own bit
    }
}

/**
 * @brief Take ownership of all the rx Descriptors.
 * This function is called when there is fatal error in DMA transmission.
 * When called it takes the ownership of all the rx descriptor in rx descriptor pool/queue from DMA.
 * The function is same for both the ring mode and the chain mode DMA structures.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note Make sure to disable the transmission before calling this function, otherwise may result in racing situation.
 */
void EMAC_take_desc_ownership_rx(EMACdevice *emacdev)
{
    s32 i;
    DmaDesc *desc;
    desc = emacdev->RxDesc;
    for(i = 0; i < emacdev->RxDescCount; i++) {
        if(1 /* ring mode */) {
            EMAC_take_desc_ownership(desc + i);
        }
    }
}

/**
 * @brief Take ownership of all the rx Descriptors.
 * This function is called when there is fatal error in DMA transmission.
 * When called it takes the ownership of all the tx descriptor in tx descriptor pool/queue from DMA.
 * The function is same for both the ring mode and the chain mode DMA structures.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note Make sure to disable the transmission before calling this function, otherwise may result in racing situation.
 */
void EMAC_take_desc_ownership_tx(EMACdevice *emacdev)
{
    s32 i;
    DmaDesc *desc;
    desc = emacdev->TxDesc;
    for(i = 0; i < emacdev->TxDescCount; i++) {
        if(1 /* ring mode */) {
            EMAC_take_desc_ownership(desc + i);
        }
    }
}

/******************************************************************************
 * Timestamp
 ******************************************************************************/
/**
 * @brief Selet the type of clock mode for PTP.
 * Please note to use one of enum TSSnapType as the clk_type argument.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] clk_type u32 value representing one of the above clk value.
 * @return None.
 */
void EMAC_TS_set_clk_type(EMACdevice *emacdev, u32 clk_type)
{
    u32 clkval;
    clkval = EMAC_READ((u64)&emacdev->MacBase->TSControl); //set the mdc clock to the user defined value
    clkval = (clkval & ~EMAC_TSControl_SNAPTYPSEL_Msk) | clk_type;
    EMAC_WRITE((u64)&emacdev->MacBase->TSControl, clkval);
    return;
}

/**
 * @brief Addend register update
 * This function loads the contents of Time stamp addend register with the supplied 32 value.
 * This is reserved function when only coarse correction option is selected.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] addend_value 32 bit addend value.
 * @return returns 0 for Success or else Failure.
 */
s32 EMAC_TS_addend_update(EMACdevice *emacdev, u32 addend_value)
{
    u32 i;
    EMAC_WRITE((u64)&emacdev->MacBase->TSAddend, addend_value);
    for(i = 0; i < DEFAULT_LOOP_VARIABLE; i++) { //Wait till the busy bit gets cleared with in a certain amount of time
        if(!(EMAC_READ((u64)&emacdev->MacBase->TSControl) & EMAC_TSControl_TSADDREG_Msk)) { // if it is cleared then break
            break;
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }
    if(i < DEFAULT_LOOP_VARIABLE)
        EMAC_SETBITS((u64)&emacdev->MacBase->TSControl, EMAC_TSControl_TSADDREG_Msk);
    else {
        TR("Error::: The TSADDREG bit is not getting cleared !!!!!!\n");
        return -EMACPHYERR;
    }
    return 0;
}

/**
 * @brief Time stamp update
 * This function updates (adds/subtracts) with the value specified in the Timestamp High Update and
 * Timestamp Low Update register.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] sec Timestamp High Update value.
 * @param[in] nanosec Timestamp Low Update value.
 * @return returns 0 for Success or else Failure.
 */
s32 EMAC_TS_timestamp_update(EMACdevice *emacdev, u32 sec, u32 nanosec)
{
    u32 i;
    EMAC_WRITE((u64)&emacdev->MacBase->TSSecUpdate, sec);
    EMAC_WRITE((u64)&emacdev->MacBase->TSNanosecUpdate, nanosec);
    for(i = 0; i < DEFAULT_LOOP_VARIABLE; i++) { //Wait till the busy bit gets cleared with in a certain amount of time
        if(!(EMAC_READ((u64)&emacdev->MacBase->TSControl) & EMAC_TSControl_TSUPDT_Msk)) { // if it is cleared then break
            break;
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }
    if(i < DEFAULT_LOOP_VARIABLE)
        EMAC_SETBITS((u64)&emacdev->MacBase->TSControl, EMAC_TSControl_TSUPDT_Msk);
    else {
        TR("Error::: The TSADDREG bit is not getting cleared !!!!!!\n");
        return -EMACPHYERR;
    }
    return 0;
}

/**
 * @brief Time stamp initialize
 * This function loads/initializes the value specified in the timestamp high update and
 * timestamp low update register.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] sec Timestamp High Load value.
 * @param[in] nanosec Timestamp Low Load value.
 * @return returns 0 for Success or else Failure.
 */
s32 EMAC_TS_timestamp_init(EMACdevice *emacdev, u32 sec, u32 nanosec)
{
    u32 i;
    EMAC_WRITE((u64)&emacdev->MacBase->TSSecUpdate, sec);
    EMAC_WRITE((u64)&emacdev->MacBase->TSNanosecUpdate, nanosec);
    for(i = 0; i < DEFAULT_LOOP_VARIABLE; i++) { //Wait till the busy bit gets cleared with in a certain amount of time
        if(!(EMAC_READ((u64)&emacdev->MacBase->TSControl) & EMAC_TSControl_TSINIT_Msk)) { // if it is cleared then break
            break;
        }
        plat_delay(DEFAULT_DELAY_VARIABLE);
    }
    if(i < DEFAULT_LOOP_VARIABLE)
        EMAC_SETBITS((u64)&emacdev->MacBase->TSControl, EMAC_TSControl_TSINIT_Msk);
    else {
        TR("Error::: The TSADDREG bit is not getting cleared !!!!!!\n");
        return -EMACPHYERR;
    }
    return 0;
}

/**
 * @brief Load the sub second increment value in to sub second increment register
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] sub_sec_inc_value increment of subsecond.
 * @return None.
 */
void EMAC_TS_subsecond_incr_init(EMACdevice *emacdev, u32 sub_sec_inc_value)
{
    EMAC_WRITE((u64)&emacdev->MacBase->TSSubSecIncr, sub_sec_inc_value & EMAC_TSSubSecIncr_SSINC_Msk);
}

/**
 * @brief Reads the time stamp contents in to the respective pointers
 * These registers are readonly.
 * This function returns the 48 bit time stamp assuming Version 2 timestamp with higher word is selected.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[out] higher_sec_val pointer to hold 16 higher bit second register contents.
 * @param[out] sec_val pointer to hold 32 bit second register contents.
 * @param[out] sub_sec_val pointer to hold 32 bit subsecond register contents.
 * @return None.
 * @note Please note that since the atomic access to the timestamp registers is not possible,
 *  the contents read may be different from the actual time stamp.
 */
void EMAC_TS_read_timestamp(EMACdevice *emacdev, u16 *higher_sec_val, u32 *sec_val, u32 *sub_sec_val)
{
    *higher_sec_val = (u16)EMAC_READ((u64)&emacdev->MacBase->TSSecHigher);
    *sec_val        = EMAC_READ((u64)&emacdev->MacBase->TSSec);
    *sub_sec_val    = EMAC_READ((u64)&emacdev->MacBase->TSNanosec);
}

/**
 * @brief Loads the time stamp higher sec value from the value supplied
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] higher_sec_val 16 higher bit second register contents passed as 32 bit value.
 * @return None.
 */
void EMAC_TS_load_timestamp_higher_val(EMACdevice *emacdev, u32 higher_sec_val)
{
    EMAC_WRITE((u64)&emacdev->MacBase->TSSecHigher, higher_sec_val & EMAC_TSSecHigher_TSHWR_Msk);
}

/**
 * @brief Reads the time stamp higher sec value to respective pointers
 * @param[in] emacdev pointer to EMACdevice.
 * @param[out] higher_sec_val pointer to hold 16 higher bit second register contents.
 * @return None.
 */
void EMAC_TS_read_timestamp_higher_val(EMACdevice *emacdev, u16 *higher_sec_val)
{
    *higher_sec_val = (u16)EMAC_READ((u64)&emacdev->MacBase->TSSecHigher) & EMAC_TSSecHigher_TSHWR_Msk;
}

/**
 * @brief Load the target time stamp registers
 * This function Loads the target time stamp registers with the values proviced
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] sec_val target Timestamp High value.
 * @param[in] sub_sec_val target Timestamp Low value.
 * @return None.
 */
void EMAC_TS_load_target_timestamp(EMACdevice *emacdev, u32 sec_val, u32 sub_sec_val)
{
    EMAC_WRITE((u64)&emacdev->MacBase->TSTargetSec, sec_val);
    EMAC_WRITE((u64)&emacdev->MacBase->TSTargetNanosec, sub_sec_val);
}

/**
 * @brief Reads the target time stamp registers
 * This function Loads the target time stamp registers with the values proviced
 * @param[in] emacdev pointer to EMACdevice.
 * @param[out] sec_val pointer to hold target Timestamp High value.
 * @param[out] sub_sec_val pointer to hold target Timestamp Low value.
 * @return None.
 */
void EMAC_TS_read_target_timestamp(EMACdevice *emacdev, u32 *sec_val, u32 *sub_sec_val)
{
    *sec_val     = EMAC_READ((u64)&emacdev->MacBase->TSTargetSec);
    *sub_sec_val = EMAC_READ((u64)&emacdev->MacBase->TSTargetNanosec);
}

/******************************************************************************
 * VLAN
 ******************************************************************************/
/**
 * @brief Enable svlan and insert vlan tag.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] vlantag u16 vlan tag.
 * @return None.
 */
void EMAC_svlan_insertion_enable(EMACdevice *emacdev, u16 vlantag)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, EmacVLP | EmacSVLAN | EmacVLANIns| vlantag);
}

/**
 * @brief Enable cvlan and insert vlan tag.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] vlantag u16 vlan tag.
 * @return None.
 */
void EMAC_cvlan_insertion_enable(EMACdevice *emacdev, u16 vlantag)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, EmacVLP | EmacCVLAN | EmacVLANIns | vlantag);
}

/**
 * @brief Enable svlan and replace with vlan tag.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] vlantag u16 vlan tag.
 * @return None.
 */
void EMAC_svlan_replace_enable(EMACdevice *emacdev, u16 vlantag)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, EmacVLP | EmacSVLAN | EmacVLANRep | vlantag);
}

/**
 * @brief Enable cvlan and replace with vlan tag.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] vlantag u16 vlan tag.
 * @return None.
 */
void EMAC_cvlan_replace_enable(EMACdevice *emacdev, u16 vlantag)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, EmacVLP | EmacCVLAN | EmacVLANRep | vlantag);
}

/**
 * @brief Enable vlan deletion.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_vlan_deletion_enable(EMACdevice *emacdev)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, EmacVLP | EmacVLANDel);
}

/**
 * @brief No vlan tag deletion, insertion, or replacement.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_vlan_no_act_enable(EMACdevice *emacdev)
{
    EMAC_WRITE((u64)&emacdev->MacBase->VlanIncRep, 0UL);
}

/******************************************************************************
 * Interrupt
 ******************************************************************************/
/**
 * @brief Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_clear_interrupt(EMACdevice *emacdev)
{
    u32 data;
    data = EMAC_GET_DMA_STATUS(emacdev);
    EMAC_CLR_DMA_STATUS(emacdev, data);
}

/**
 * @brief Returns the all unmasked interrupt status after reading the DmaStatus register.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return u32 enum usrDMAIntHandle interrupt type.
 */
u32 EMAC_get_interrupt_type(EMACdevice *emacdev)
{
    u32 data;
    u32 interrupts = 0;
    data = EMAC_GET_DMA_STATUS(emacdev);
    EMAC_CLR_DMA_STATUS(emacdev, data); //This is the appropriate location to clear the interrupts
    TR("DMA status reg is  %08x\n",data);
    if(data & EMAC_DmaStatus_FBI_Msk) interrupts |= EMACDmaError;
    if(data & EMAC_DmaStatus_RI_Msk)  interrupts |= EMACDmaRxNormal;
    if(data & EMAC_DmaStatus_RU_Msk)  interrupts |= EMACDmaRxAbnormal;
    if(data & EMAC_DmaStatus_RPS_Msk) interrupts |= EMACDmaRxStopped;
    if(data & EMAC_DmaStatus_TI_Msk)  interrupts |= EMACDmaTxNormal;
    if(data & EMAC_DmaStatus_UNF_Msk) interrupts |= EMACDmaTxAbnormal;
    if(data & EMAC_DmaStatus_TPS_Msk) interrupts |= EMACDmaTxStopped;

    return interrupts;
}

/**
 * @brief Enable all the interrupts.
 * Enables the DMA interrupt as specified by the bit mask.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] interrupts bit mask of interrupts to be enabled.
 * @return None.
 */
void EMAC_enable_interrupt(EMACdevice *emacdev, u32 interrupts)
{
    EMAC_WRITE((u64)&emacdev->MacBase->DmaInt, interrupts);
}

/**
 * @brief Disable all the interrupts.
 * Disables all DMA interrupts.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_disable_interrupt_all(EMACdevice *emacdev)
{
    EMAC_WRITE((u64)&emacdev->MacBase->DmaInt, 0UL);
}

/**
 * @brief Disable interrupt according to the bitfield supplied.
 * Disables only those interrupts specified in the bit mask in second argument.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] interrupts bit mask for interrupts to be disabled.
 * @return None.
 */
void EMAC_disable_interrupt(EMACdevice *emacdev, u32 interrupts)
{
    EMAC_CLEARBITS((u64)&emacdev->MacBase->DmaInt, interrupts);
}

/******************************************************************************
 * Packet
 ******************************************************************************/
/**
 * @brief Enable source MAC address insertion
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_src_addr_insert_enable(EMACdevice *emacdev)
{
    EMAC_CLEARBITS((u64)&emacdev->MacBase->Config, EMAC_Config_SARC_Msk);
    EMAC_SETBITS((u64)&emacdev->MacBase->Config, EmacSAInstAddr0);
}

/**
 * @brief Disable source MAC address insertion
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_src_addr_insert_disable(EMACdevice *emacdev)
{
    EMAC_CLEARBITS((u64)&emacdev->MacBase->Config, EMAC_Config_SARC_Msk);
}

/**
 * @brief Enable source MAC address replacement
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_src_addr_replace_enable(EMACdevice *emacdev)
{
    EMAC_CLEARBITS((u64)&emacdev->MacBase->Config, EMAC_Config_SARC_Msk);
    EMAC_SETBITS((u64)&emacdev->MacBase->Config, EmacSAReplAddr0);
}

/**
 * @brief Disable source MAC address replacement
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_src_addr_replace_disable(EMACdevice *emacdev)
{
    EMAC_CLEARBITS((u64)&emacdev->MacBase->Config, EMAC_Config_SARC_Msk);
}

/**
 * @brief Checks whether the packet received is a magic packet?
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return returns True if magic packet received else returns false.
 */
bool EMAC_is_magic_packet_received(EMACdevice *emacdev)
{
    u32 data;
    data = EMAC_READ((u64)&emacdev->MacBase->PmtCtrlStatus);
    return ((data & EMAC_PmtCtrlStatus_MGKPRCVD_Msk) == EMAC_PmtCtrlStatus_MGKPRCVD_Msk);
}

/*! @}*/ /* EMAC of group EMAC_EXPORTED_FUNCTIONS */

/*! @}*/ /* end of group EMAC_Driver */

/*! @}*/ /* end of group Standard_Driver */
