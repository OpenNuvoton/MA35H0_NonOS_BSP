/*************************************************************************//**
 * @file     ma35h0_mac.c
 * @brief    EMAC hardware init helper APIs.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright(C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include "netif/ma35h0_mac.h"

EMACdevice EMACdev[EMAC_CNT];
static DmaDesc tx_desc[EMAC_CNT][TRANSMIT_DESC_SIZE] __attribute__ ((aligned (64)));
static DmaDesc rx_desc[EMAC_CNT][RECEIVE_DESC_SIZE] __attribute__ ((aligned (64)));

static struct sk_buff rx_buf[EMAC_CNT][RECEIVE_DESC_SIZE] __attribute__ ((aligned (64)));

// These 2 are accessable from application
struct sk_buff txbuf[EMAC_CNT] __attribute__ ((aligned (64))); // set align to separate cacheable and non-cacheable data to different cache line.
struct sk_buff rxbuf[EMAC_CNT] __attribute__ ((aligned (64)));

u8 mac_addr0[6] = DEFAULT_MAC0_ADDRESS;
u8 mac_addr1[6] = DEFAULT_MAC1_ADDRESS;

static u32 EMAC_Power_down; // This global variable is used to indicate the ISR whether the interrupts occured in the process of powering down the mac or not

/**
 * @brief This sets up the transmit Descriptor queue in ring or chain mode.
 * This function is tightly coupled to the platform and operating system
 * Device is interested only after the descriptors are setup. Therefore this function
 * is not included in the device driver API. This function should be treated as an
 * example code to design the descriptor structures for ring mode or chain mode.
 * This function depends on the pcidev structure for allocation consistent dma-able memory in case of linux.
 * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
 *	- Allocates the memory for the descriptors.
 *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
 *	- Initialize the Busy and Next descriptors to first descriptor address.
 *  - Initialize the last descriptor with the endof ring in case of ring mode.
 *	- Initialize the descriptors in chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] no_of_desc number of descriptor expected in tx descriptor queue.
 * @param[in] desc_mode whether descriptors to be created in RING mode or CHAIN mode.
 * @return 0 upon success. Error code upon failure.
 * @note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
 *  function returns -EMACNOMEM in the process of descriptor chain creation. once returned from this function
 *  user should for emacdev->TxDescCount to see how many descriptors are there in the chain. Should continue further
 *  only if the number of descriptors in the chain meets the requirements
 */
s32 EMAC_setup_tx_desc_queue(EMACdevice *emacdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;
    DmaDesc *first_desc = &tx_desc[emacdev->Intf][0];
    emacdev->TxDescCount = 0;

    TR("Total size of memory required for Tx Descriptors in Ring Mode = 0x%08x\n",((sizeof(DmaDesc) * no_of_desc)));

    emacdev->TxDescCount = no_of_desc;
    emacdev->TxDesc      = first_desc;
#ifdef CACHE_ON
    emacdev->TxDescDma   = (DmaDesc *)((u64)first_desc | NON_CACHE);
#else
    emacdev->TxDescDma   = (DmaDesc *)((u64)first_desc);
#endif
    for(i = 0; i < emacdev->TxDescCount; i++) {
        EMAC_tx_desc_init_ring(emacdev->TxDescDma + i, i == (emacdev->TxDescCount - 1));
        TR("%02d %08x \n",i, (unsigned int)(emacdev->TxDesc + i) );
    }

    emacdev->TxNext = 0;
    emacdev->TxBusy = 0;
    emacdev->TxNextDesc = emacdev->TxDesc;
    emacdev->TxBusyDesc = emacdev->TxDesc;
    emacdev->BusyTxDesc  = 0;

    return 0;
}

/**
 * @brief This sets up the receive Descriptor queue in ring or chain mode.
 * This function is tightly coupled to the platform and operating system
 * Device is interested only after the descriptors are setup. Therefore this function
 * is not included in the device driver API. This function should be treated as an
 * example code to design the descriptor structures in ring mode or chain mode.
 * This function depends on the pcidev structure for allocation of consistent dma-able memory in case of linux.
 * This limitation is due to the fact that linux uses pci structure to allocate a dmable memory
 *	- Allocates the memory for the descriptors.
 *	- Initialize the Busy and Next descriptors indices to 0(Indicating first descriptor).
 *	- Initialize the Busy and Next descriptors to first descriptor address.
 *  - Initialize the last descriptor with the endof ring in case of ring mode.
 *	- Initialize the descriptors in chain mode.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] no_of_desc number of descriptor expected in rx descriptor queue.
 * @param[in] desc_mode whether descriptors to be created in RING mode or CHAIN mode.
 * @return 0 upon success. Error code upon failure.
 * @note This function fails if allocation fails for required number of descriptors in Ring mode, but in chain mode
 *  function returns -EMACNOMEM in the process of descriptor chain creation. once returned from this function
 *  user should for emacdev->RxDescCount to see how many descriptors are there in the chain. Should continue further
 *  only if the number of descriptors in the chain meets the requirements
 */
s32 EMAC_setup_rx_desc_queue(EMACdevice *emacdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;
    DmaDesc *first_desc = &rx_desc[emacdev->Intf][0];
    emacdev->RxDescCount = 0;

    TR("total size of memory required for Rx Descriptors in Ring Mode = 0x%08x\n",((sizeof(DmaDesc) * no_of_desc)));

    emacdev->RxDescCount = no_of_desc;
    emacdev->RxDesc      = first_desc;
#ifdef CACHE_ON
    emacdev->RxDescDma   = (DmaDesc *)((u64)first_desc | NON_CACHE);
#else
    emacdev->RxDescDma   = (DmaDesc *)((u64)first_desc);
#endif
    for(i = 0; i < emacdev->RxDescCount; i++) {
        EMAC_rx_desc_init_ring(emacdev->RxDescDma + i, i == (emacdev->RxDescCount - 1));
        TR("%02d %08x \n",i, (unsigned int)(emacdev->RxDesc + i));
    }

    emacdev->RxNext = 0;
    emacdev->RxBusy = 0;
    emacdev->RxNextDesc = emacdev->RxDesc;
    emacdev->RxBusyDesc = emacdev->RxDesc;
    emacdev->BusyRxDesc   = 0;

    return 0;
}

/**
 * @brief Set RGMII/RMII mode 1000/100/10M.
 * This function is called after partner determined.
 * @param[in] emacdev pointer to EMACdevice.
 * @return Always returns 0.
 */
s32 EMAC_set_mode(EMACdevice *emacdev)
{
    // Must stop Tx/Rx before change speed/mode
    EMAC_TX_DISABLE(emacdev);
    EMAC_RX_DISABLE(emacdev);

    EMAC_set_mii_speed(emacdev);

    EMAC_TX_ENABLE(emacdev);
    EMAC_RX_ENABLE(emacdev);

    return 0;
}

/**
 * @brief Function used when the interface is opened for use.
 * We register EMAC_linux_open function to linux open(). Basically this
 * function prepares the the device for operation . This function is called whenever ifconfig (in Linux)
 * activates the device (for example "ifconfig eth0 up"). This function registers
 * system resources needed
 * - Attaches device to device specific structure
 * - Programs the MDC clock for PHY configuration
 * - Check and initialize the PHY interface
 * - ISR registration
 * - Setup and initialize Tx and Rx descriptors
 * - Initialize MAC and DMA
 * - Allocate Memory for RX descriptors (The should be DMAable)
 * - Initialize one second timer to detect cable plug/unplug
 * - Configure and Enable Interrupts
 * - Enable Tx and Rx
 * - start the Linux network queue interface
 * @param[in] intf EMAC interface
 *          - \ref EMACINTF0
 *          - \ref EMACINTF1
 * @param[in] mode MII mode
 *          - \ref RGMII_1G
 *          - \ref RMII_100M
 *          - \ref RMII_10M
 * @return Returns 0 on success and error status upon failure.
 */
s32 EMAC_open(int intf, int mode)
{
    s32 i;
    s32 status = 0;
    struct sk_buff *skb;
    EMACdevice *emacdev = &EMACdev[intf];

    /* Enable module clock and MFP */
    if(intf == EMACINTF0) {
        CLK->SYSCLK0 |= CLK_SYSCLK0_EMAC0EN_Msk; // Enable EMAC0 clock

        if(mode == RGMII_1G) {
            SYS->GPE_MFPL = 0x88888888;
            SYS->GPE_MFPH = (SYS->GPE_MFPH & ~0x00FFFFFF) | 0x00888888;
            SYS->EMAC0MISCR &= ~SYS_EMAC0MISCR_RMIIEN_Msk;
        }
        else {
            SYS->GPE_MFPL = 0x99999999;
            SYS->GPE_MFPH = (SYS->GPE_MFPH & ~0x000000FF) | 0x00000099;
            SYS->EMAC0MISCR |= SYS_EMAC0MISCR_RMIIEN_Msk;
        }
    } else {
        CLK->SYSCLK0 |= CLK_SYSCLK0_EMAC1EN_Msk; // Enable EMAC1 clock

        if(mode == RGMII_1G) {
            SYS->GPF_MFPL = 0x88888888;
            SYS->GPF_MFPH = (SYS->GPF_MFPH & ~0x00FFFFFF) | 0x00888888;
            SYS->EMAC1MISCR &= ~SYS_EMAC1MISCR_RMIIEN_Msk;
        }
        else {
            SYS->GPF_MFPL = 0x99999999;
            SYS->GPF_MFPH = (SYS->GPF_MFPH & ~0x000000FF) | 0x00000099;
            SYS->EMAC1MISCR |= SYS_EMAC1MISCR_RMIIEN_Msk;
        }
    }

    /*Attach the device to MAC struct This will configure all the required base addresses
      such as Mac base, configuration base, phy base address(out of 32 possible phys )*/
    EMAC_attach(emacdev, intf, DEFAULT_PHY_BASE);

    EMAC_reset(emacdev); // Reset to make RGMII/RMII setting take affect --ya

    /*Lets read the version of ip in to device structure*/
    EMAC_read_version(emacdev);

    /*Check for Phy initialization*/
    EMAC_set_mdc_clk_div(emacdev, GmiiCsrClk4);

    emacdev->ClockDivMdc = EMAC_get_mdc_clk_div(emacdev);

    status = EMAC_check_phy_init(emacdev, mode);
    if(status < 0)
        sysprintf("PHY init fail\n");

    /*Set up the tx and rx descriptor queue/ring*/
    EMAC_setup_tx_desc_queue(emacdev, TRANSMIT_DESC_SIZE, RINGMODE);
    EMAC_init_tx_desc_base(emacdev);	//Program the transmit descriptor base address in to DmaTxBase addr

    EMAC_setup_rx_desc_queue(emacdev, RECEIVE_DESC_SIZE, RINGMODE);
    EMAC_init_rx_desc_base(emacdev);	//Program the transmit descriptor base address in to DmaTxBase addr

    EMAC_DMA_BUSMODE_INIT(emacdev, DmaBurstLength32 | DmaDescriptorSkip0 | EMAC_DmaBusMode_ATDS_Msk); //pbl32 incr with rxthreshold 128 and Desc is 8 Words
    EMAC_DMA_OPMODE_INIT(emacdev, EMAC_DmaOpMode_TSF_Msk | EMAC_DmaOpMode_OSF_Msk | DmaRxThreshCtrl128);

    /*Initialize the mac interface*/
    status = EMAC_init(emacdev);

    EMAC_pause_control(emacdev); // This enables the pause control in Full duplex mode of operation

    /*IPC Checksum offloading is enabled for this driver. Should only be used if Full Ip checksumm offload engine is configured in the hardware*/
    EMAC_CHKSUM_OFFLOAD_ENABLE(emacdev);  	//Enable the offload engine in the receive path
    EMAC_TCPIP_DROP_ERR_ENABLE(emacdev); // This is default configuration, DMA drops the packets if error in encapsulated ethernet payload

    for(i = 0; i < RECEIVE_DESC_SIZE; i++) {
        skb = &rx_buf[intf][i];
        EMAC_set_rx_qptr(emacdev, (u32)((u64)(skb->data) & 0xFFFFFFFF), sizeof(skb->data));
    }

    EMAC_clear_interrupt(emacdev);
    EMAC_enable_interrupt(emacdev, DMA_INT_ENABLE);

    EMAC_DMA_RX_ENABLE(emacdev);
    EMAC_DMA_TX_ENABLE(emacdev);

    EMAC_set_mac_addr(emacdev, 0, intf == EMACINTF0 ? mac_addr0 : mac_addr1);

    EMAC_set_mode(emacdev);

    return 0;
}

/**
 * @brief This gives up the receive Descriptor queue in ring or chain mode.
 * This function is tightly coupled to the platform and operating system
 * Once device's Dma is stopped the memory descriptor memory and the buffer memory deallocation,
 * is completely handled by the operating system, this call is kept outside the device driver Api.
 * This function should be treated as an example code to de-allocate the descriptor structures in ring mode or chain mode
 * and network buffer deallocation.
 * This function depends on the pcidev structure for dma-able memory deallocation for both descriptor memory and the
 * network buffer memory under linux.
 * The responsibility of this function is to
 * - Free the network buffer memory if any.
 * - Fee the memory allocated for the descriptors.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] desc_mode whether descriptors to be created in RING mode or CHAIN mode.
 * @return None.
 */
void EMAC_giveup_rx_desc_queue(EMACdevice *emacdev, u32 desc_mode)
{
    emacdev->RxDesc    = NULL;
    emacdev->RxDescDma = 0;
}

/**
 * @brief This gives up the transmit Descriptor queue in ring or chain mode.
 * This function is tightly coupled to the platform and operating system
 * Once device's Dma is stopped the memory descriptor memory and the buffer memory deallocation,
 * is completely handled by the operating system, this call is kept outside the device driver Api.
 * This function should be treated as an example code to de-allocate the descriptor structures in ring mode or chain mode
 * and network buffer deallocation.
 * This function depends on the pcidev structure for dma-able memory deallocation for both descriptor memory and the
 * network buffer memory under linux.
 * The responsibility of this function is to
 * - Free the network buffer memory if any.
 * - Fee the memory allocated for the descriptors.
 * @param[in] emacdev pointer to EMACdevice.
 * @param[in] desc_mode whether descriptors to be created in RING mode or CHAIN mode.
 * @return None.
 */
void EMAC_giveup_tx_desc_queue(EMACdevice *emacdev, u32 desc_mode)
{
    emacdev->TxDesc    = NULL;
    emacdev->TxDescDma = 0;
}

/**
 * @brief Function used when the interface is closed.
 * This releases all the system resources allocated during open call.
 * - Disable the device interrupts
 * - Stop the receiver and get back all the rx descriptors from the DMA
 * - Stop the transmitter and get back all the tx descriptors from the DMA
 * - Stop the Linux network queue interface
 * - Free the irq (ISR registered is removed from the kernel)
 * - Release the TX and RX descripor memory
 * - De-initialize one second timer rgistered for cable plug/unplug tracking
 * @param[in] intf EMAC interface
 *          - \ref EMACINTF0
 *          - \ref EMACINTF1
 * @return Always return 0
 */
s32 EMAC_close(int intf)
{
    EMACdevice *emacdev = &EMACdev[intf];

    /*Disable all the interrupts*/
    EMAC_disable_interrupt_all(emacdev);

    TR("the EMAC interrupt has been disabled\n");

    /*Disable the reception*/
    EMAC_DMA_RX_DISABLE(emacdev);
    EMAC_take_desc_ownership_rx(emacdev);
    TR("the EMAC Reception has been disabled\n");

    /*Disable the transmission*/
    EMAC_DMA_TX_DISABLE(emacdev);
    EMAC_take_desc_ownership_tx(emacdev);

    if(intf == 0) {
        IRQ_Disable(EMAC0_IRQn);
    } else {
        IRQ_Disable(EMAC1_IRQn);
    }
    TR("the EMAC interrupt handler has been removed\n");

    /*Free the Rx Descriptor contents*/
    TR("Now calling EMAC_giveup_rx_desc_queue \n");
    EMAC_giveup_rx_desc_queue(emacdev, RINGMODE);

    TR("Now calling EMAC_giveup_tx_desc_queue \n");
    EMAC_giveup_tx_desc_queue(emacdev, RINGMODE);

    return 0;
}

/**
 * @brief Function to transmit a given packet on the wire.
 * Whenever Linux Kernel has a packet ready to be transmitted, this function is called.
 * The function prepares a packet and prepares the descriptor and
 * enables/resumes the transmission.
 * @param[in] skb pointer to sk_buff structure.
 * @param[in] intf EMAC interface
 *          - \ref EMACINTF0
 *          - \ref EMACINTF1
 * @param[in] offload_needed whether enable hardware offload engine
 * @param[in] ts whether enable timestamp
 * @return Returns 0 on success and Error code on failure.
 */
s32 EMAC_xmit_frames(struct sk_buff *skb, int intf, u32 offload_needed, u32 ts)
{
    s32 status = 0;
    u32 dma_addr = (u32)((u64)skb->data & 0xFFFFFFFF);
    EMACdevice *emacdev = &EMACdev[intf];

    /*Now we have skb ready and OS invoked this function. Lets make our DMA know about this*/
    status = EMAC_set_tx_qptr(emacdev, skb->len, dma_addr, offload_needed, ts);
    if(status < 0) {
        TR("%s No More Free Tx Descriptors\n",__FUNCTION__);
        return -1;
    }

    /*Now force the DMA to start transmission*/
    EMAC_DMA_TX_PD_RESUME(emacdev);

    return 0;
}

/**
 * @brief Function to handle housekeeping after a packet is transmitted over the wire.
 * After the transmission of a packet DMA generates corresponding interrupt
 * (if it is enabled). It takes care of returning the sk_buff to the linux
 * kernel, updating the networking statistics and tracking the descriptors.
 * @param[in] intf EMAC interface
 *          - \ref EMACINTF0
 *          - \ref EMACINTF1
 * @return None.
 * @note This function runs in interrupt context.
 */
void EMAC_handle_transmit_over(int intf)
{
    EMACdevice *emacdev;
    s32 desc_index;
    u32 status;
    u32 length;
    u32 buffer1, buffer2;
    u32 ext_status;
    u32 time_stamp_high;
    u32 time_stamp_low;

    emacdev = &EMACdev[intf];

    /*Handle the transmit Descriptors*/
    do {
        desc_index = EMAC_get_tx_qptr(emacdev, &status, &length, &buffer1, &buffer2, &ext_status, &time_stamp_high, &time_stamp_low);
        //EMAC_TS_read_timestamp_higher_val(emacdev, &time_stamp_higher);

        if(desc_index >= 0 /*&& data1 != 0*/) {
            TR("Finished Transmit at Tx Descriptor %d for skb and buffer = %08x whose status is %08x \n", desc_index,buffer1,status);

            if(EMAC_is_tx_ipv4header_checksum_error(status)) {
                TR("Hardware Failed to Insert IPV4 Header Checksum\n");
                emacdev->NetStats.tx_ip_header_errors++;
            }
            if(EMAC_is_tx_payload_checksum_error(status)) {
                TR("Hardware Failed to Insert Payload Checksum\n");
                emacdev->NetStats.tx_ip_payload_errors++;
            }

            if(EMAC_is_desc_valid(status)) {
                emacdev->NetStats.tx_bytes += length;
                emacdev->NetStats.tx_packets++;
                if(status & DescTxTSStatus) {
                    emacdev->tx_sec = time_stamp_high;
                    emacdev->tx_subsec = time_stamp_low;
                } else {
                    emacdev->tx_sec = 0;
                    emacdev->tx_subsec = 0;
                }
            } else {
                TR("Error in Status %08x\n",status);
                emacdev->NetStats.tx_errors++;
                emacdev->NetStats.tx_aborted_errors += EMAC_is_tx_aborted(status);
                emacdev->NetStats.tx_carrier_errors += EMAC_is_tx_carrier_error(status);
            }
        }
        emacdev->NetStats.collisions += EMAC_get_tx_collision_count(status);
    } while(desc_index >= 0);
}

/**
 * @brief Function to Receive a packet from the interface.
 * After Receiving a packet, DMA transfers the received packet to the system memory
 * and generates corresponding interrupt (if it is enabled). This function prepares
 * the sk_buff for received packet after removing the ethernet CRC, and hands it over
 * to linux networking stack.
 * - Updataes the networking interface statistics
 * - Keeps track of the rx descriptors
 * @param[in] intf EMAC interface
 *          - \ref EMACINTF0
 *          - \ref EMACINTF1
 * @return None.
 * @note This function runs in interrupt context.
 */
uint32_t EMAC_handle_received_data(int intf, struct sk_buff *prskb)
{
    EMACdevice *emacdev;
    s32 desc_index;
    u32 len;

    u32 status;
    u32 dma_addr1;
    u32 ext_status;
    u32 time_stamp_high;
    u32 time_stamp_low;
    u32 ret = 0;
    struct sk_buff *rb = prskb;

    //struct sk_buff *skb; //This is the pointer to hold the received data

    TR("%s\n",__FUNCTION__);

    emacdev = &EMACdev[intf];

    /*Handle the Receive Descriptors*/
    do {
        desc_index = EMAC_get_rx_qptr(emacdev, &status, NULL, &dma_addr1, NULL, &ext_status, &time_stamp_high, &time_stamp_low);
        if(desc_index > 0) {
            TR("S:%08x ES:%08x DA1:%08x TSH:%08x TSL:%08x\n",status,ext_status,dma_addr1,time_stamp_high,time_stamp_low);
        }

        if(desc_index >= 0) {
            TR("Received Data at Rx Descriptor %d for skb whose status is %08x\n",desc_index,status);

            //skb = (struct sk_buff *)((u64)data1);
            if(1) {
                // Always enter this loop. EMAC_is_rx_desc_valid() also report invalid descriptor
                // if there's packet error generated by test code and drop it. But we need to execute ext_status
                // check code to tell what's going on.                                          --ya

                len = EMAC_get_rx_desc_frame_length(status) - 4; //Not interested in Ethernet CRC bytes

                // Now lets check for the IPC offloading
                /*  Since we have enabled the checksum offloading in hardware, lets inform the kernel
                    not to perform the checksum computation on the incoming packet. Note that ip header
                    checksum will be computed by the kernel immaterial of what we inform. Similary TCP/UDP/ICMP
                    pseudo header checksum will be computed by the stack. What we can inform is not to perform
                    payload checksum.
                    When CHECKSUM_UNNECESSARY is set kernel bypasses the checksum computation.
                */

                TR("Checksum Offloading will be done now\n");

                if(EMAC_is_ext_status(emacdev, status)) { // extended status present indicates that the RDES4 need to be probed
                    TR("Extended Status present\n");
                    if(EMAC_ES_is_IP_header_error(ext_status)) {      // IP header (IPV4) checksum error
                        //Linux Kernel doesnot care for ipv4 header checksum. So we will simply proceed by printing a warning ....
                        TR("(EXTSTS)Error in IP header error\n");
                        emacdev->NetStats.rx_ip_header_errors++;
                    }
                    if(EMAC_ES_is_rx_checksum_bypassed(ext_status)) {  // Hardware engine bypassed the checksum computation/checking
                        TR("(EXTSTS)Hardware bypassed checksum computation\n");
                    }
                    if(EMAC_ES_is_IP_payload_error(ext_status)) {      // IP payload checksum is in error (UDP/TCP/ICMP checksum error)
                        TR("(EXTSTS) Error in EP payload\n");
                        emacdev->NetStats.rx_ip_payload_errors++;
                    }
                } else { // No extended status. So relevant information is available in the status itself
                    if(EMAC_is_rx_checksum_error(status) == RxNoChkError) {
                        TR("Ip header and TCP/UDP payload checksum Bypassed <Chk Status = 4>  \n");
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxIpHdrChkError) {
                        //Linux Kernel doesnot care for ipv4 header checksum. So we will simply proceed by printing a warning ....
                        TR(" Error in 16bit IPV4 Header Checksum <Chk Status = 6>  \n");
                        emacdev->NetStats.rx_ip_header_errors++;
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxLenLT600) {
                        TR("IEEE 802.3 type frame with Length field Lesss than 0x0600 <Chk Status = 0> \n");
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxIpHdrPayLoadChkBypass) {
                        TR("Ip header and TCP/UDP payload checksum Bypassed <Chk Status = 1>\n");
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxChkBypass) {
                        TR("Ip header and TCP/UDP payload checksum Bypassed <Chk Status = 3>  \n");
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxPayLoadChkError) {
                        TR(" TCP/UDP payload checksum Error <Chk Status = 5>  \n");
                        emacdev->NetStats.rx_ip_payload_errors++;
                    }
                    if(EMAC_is_rx_checksum_error(status) == RxIpHdrPayLoadChkError) {
                        //Linux Kernel doesnot care for ipv4 header checksum. So we will simply proceed by printing a warning ....
                        TR(" Both IP header and Payload Checksum Error <Chk Status = 7>  \n");
                        emacdev->NetStats.rx_ip_header_errors++;
                        emacdev->NetStats.rx_ip_payload_errors++;
                    }
                }

                rb->rdy = 1;
                rb->len = len;
                rb->pData = (void *)((u64)dma_addr1 | NON_CACHE);
                ret++;
                rb = (struct sk_buff *)rb + 1;

                emacdev->NetStats.rx_packets++;
                emacdev->NetStats.rx_bytes += len;
                if(EMAC_is_timestamp_available(status)) {
                    emacdev->rx_sec = time_stamp_high;
                    emacdev->rx_subsec = time_stamp_low;
                } else {
                    emacdev->rx_sec = 0;
                    emacdev->rx_subsec = 0;
                }
            } else {
                /*Now the present skb should be set free*/
                TR("s: %08x\n",status);
                emacdev->NetStats.rx_errors++;
                emacdev->NetStats.collisions       += EMAC_is_rx_frame_collision(status);
                emacdev->NetStats.rx_crc_errors    += EMAC_is_rx_crc(status);
                emacdev->NetStats.rx_frame_errors  += EMAC_is_frame_dribbling_errors(status);
                emacdev->NetStats.rx_length_errors += EMAC_is_rx_frame_length_errors(status);
            }

        }
    } while(desc_index >= 0); // do until desc is empty

    return ret;
}

/**
 * @brief Function to power up and resume EMAC IP if magic packet is determined.
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 * @note This function runs in interrupt context.
 */
void EMAC_powerup_mac(EMACdevice *emacdev)
{
    EMAC_Power_down = 0;	// Let ISR know that MAC is out of power down now
    if(EMAC_is_magic_packet_received(emacdev))
        sysprintf("EMAC wokeup due to Magic Pkt Received\n");
    //Disable the assertion of PMT interrupt
    EMAC_PMT_INT_DISABLE(emacdev);
    //Enable the mac and Dma rx and tx paths
    EMAC_RX_ENABLE(emacdev);
    EMAC_DMA_RX_ENABLE(emacdev);

    EMAC_TX_ENABLE(emacdev);
    EMAC_DMA_TX_ENABLE(emacdev);
}

/**
 * @brief Function to power down EMAC IP.
 *
 * @param[in] emacdev pointer to EMACdevice.
 * @return None.
 */
void EMAC_powerdown_mac(EMACdevice *emacdev)
{
    TR("Put the EMAC to power down mode..\n");
    // Disable the Dma engines in tx path
    EMAC_Power_down = 1;	// Let ISR know that Mac is going to be in the power down mode
    EMAC_DMA_TX_DISABLE(emacdev);
    plat_delay(DEFAULT_LOOP_VARIABLE);		// Allow any pending transmission to complete
    // Disable the Mac for both tx and rx
    EMAC_TX_DISABLE(emacdev);
    EMAC_RX_DISABLE(emacdev);
    plat_delay(DEFAULT_LOOP_VARIABLE); 		// Allow any pending buffer to be read by host
    //Disable the Dma in rx path
    EMAC_DMA_RX_DISABLE(emacdev);

    //enable the power down mode
    //EMAC_pmt_unicast_enable(emacdev);

    //prepare the emac for magic packet reception and wake up frame reception
    EMAC_PMT_MAGIC_PKT_ENABLE(emacdev);

    //gate the application and transmit clock inputs to the code. This is not done in this driver :).

    //enable the Mac for reception
    EMAC_RX_ENABLE(emacdev);

    //Enable the assertion of PMT interrupt
    EMAC_PMT_INT_ENABLE(emacdev);
    //enter the power down mode
    EMAC_PMT_PD_ENABLE(emacdev);
}

/**
 * @brief Interrupt service routing for EMAC0.
 * This is the function registered as ISR for device interrupts.
 * @param[in] None
 * @return None
 * @note This function runs in interrupt context
 */
uint32_t EMAC_int_handler0(struct sk_buff *prskb)
{
    EMACdevice *emacdev = &EMACdev[EMACINTF0];
    u32 interrupt, dma_status_reg, mac_status_reg;
    u32 dma_addr;
    u32 volatile reg;
	uint32_t ret = 0;

    // Check EMAC interrupt
    mac_status_reg = EMAC_GET_INT_SUMMARY(emacdev);

    /* handle ts status */
    if(mac_status_reg & EMAC_IntStatus_TSIS_Msk) {
        emacdev->NetStats.ts_int = 1;
        reg = EMAC_GET_TS_STATUS(emacdev); // read to clear
        if(!(reg & EMAC_TSStatus_TSTARGT_Msk))
            TR("TS alarm flag not set??\n");
        else
            TR("TS alarm!!!!!!!!!!!!!!!!\n");
    }

    /* handle lpi status */
    if(mac_status_reg & EMAC_IntStatus_LPIIS_Msk) {
        reg = EMAC_GET_LPI_STATUS(emacdev); // read to clear
    }

    /* handle rgmii status */
    if(mac_status_reg & EMAC_IntStatus_RGSMIIIS_Msk) {
        reg = EMAC_GET_RGMII_STATUS(emacdev); // read to clear
    }

    EMAC_CLR_INT_SUMMARY(emacdev, mac_status_reg);

    /*Read the Dma interrupt status to know whether the interrupt got generated by our device or not*/
    dma_status_reg = EMAC_GET_DMA_STATUS(emacdev);

    if(dma_status_reg == 0)
        return ret;

    EMAC_disable_interrupt_all(emacdev);

    TR("%s:Dma Status Reg: 0x%08x\n",__FUNCTION__,dma_status_reg);

    if(dma_status_reg & EMAC_DmaStatus_GPI_Msk) {
        TR("%s:: Interrupt due to PMT module\n",__FUNCTION__);
        EMAC_powerup_mac(emacdev);
    }

    if(dma_status_reg & EMAC_DmaStatus_GLI_Msk) {
        TR("%s:: Interrupt due to EMAC LINE module\n",__FUNCTION__);
    }

    /*Now lets handle the DMA interrupts*/
    interrupt = EMAC_get_interrupt_type(emacdev);
    TR("%s:Interrupts to be handled: 0x%08x\n",__FUNCTION__,interrupt);

    if(interrupt & EMACDmaError) {
        TR("%s::Fatal Bus Error Inetrrupt Seen\n",__FUNCTION__);
        EMAC_DMA_TX_DISABLE(emacdev);
        EMAC_DMA_RX_DISABLE(emacdev);

        EMAC_take_desc_ownership_tx(emacdev);
        EMAC_take_desc_ownership_rx(emacdev);

        EMAC_init_tx_rx_desc_queue(emacdev);

        EMAC_reset(emacdev); //reset the DMA engine and the EMAC ip

        EMAC_set_mac_addr(emacdev, 0, emacdev->Intf == 0 ? mac_addr0 : mac_addr1);
        EMAC_DMA_BUSMODE_INIT(emacdev, EMAC_DmaBusMode_FB_Msk | DmaBurstLength8 | DmaDescriptorSkip0);
        EMAC_DMA_OPMODE_INIT(emacdev, EMAC_DmaOpMode_TSF_Msk);

        EMAC_init_rx_desc_base(emacdev);
        EMAC_init_tx_desc_base(emacdev);
        EMAC_init(emacdev);
        EMAC_DMA_RX_ENABLE(emacdev);
        EMAC_DMA_TX_ENABLE(emacdev);
    }

    if(interrupt & EMACDmaRxNormal) {
        TR("%s:: Rx Normal \n", __FUNCTION__);
        notify_rx_task(EMACINTF0);
    }

    if(interrupt & EMACDmaRxAbnormal) {
        TR("%s::Abnormal Rx Interrupt Seen\n",__FUNCTION__);
        emacdev->NetStats.rx_over_errors++;

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_DMA_RX_PD_RESUME(emacdev);//To handle GBPS with 12 descriptors
        }
    }

    if(interrupt & EMACDmaRxStopped) {
        TR("%s::Receiver stopped seeing Rx interrupts\n",__FUNCTION__); //Receiver gone in to stopped state

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            emacdev->NetStats.rx_over_errors++;
            EMAC_DMA_RX_ENABLE(emacdev);
        }
    }

    if(interrupt & EMACDmaTxNormal) {
        //xmit function has done its job
        TR("%s::Finished Normal Transmission \n",__FUNCTION__);
        EMAC_handle_transmit_over(EMACINTF0);//Do whatever you want after the transmission is over
    }

    if(interrupt & EMACDmaTxAbnormal) {
        TR("%s::Abnormal Tx Interrupt Seen\n",__FUNCTION__);

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_handle_transmit_over(EMACINTF0);
        }
    }

    if(interrupt & EMACDmaTxStopped) {
        TR("%s::Transmitter stopped sending the packets\n",__FUNCTION__);

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_DMA_TX_DISABLE(emacdev);
            EMAC_take_desc_ownership_tx(emacdev);

            EMAC_DMA_TX_ENABLE(emacdev);
            TR("%s::Transmission Resumed\n",__FUNCTION__);
        }
    }

    /* Enable the interrupt before returning from ISR*/
    EMAC_enable_interrupt(emacdev, DMA_INT_ENABLE);

	return ret;
}

/**
 * @brief Interrupt service routing for EMAC1.
 * This is the function registered as ISR for device interrupts.
 * @param[in] None
 * @return None
 * @note This function runs in interrupt context
 */
uint32_t EMAC_int_handler1(struct sk_buff *prskb)
{
    EMACdevice *emacdev = &EMACdev[EMACINTF1];
    u32 interrupt, dma_status_reg, mac_status_reg;
    u32 dma_addr;
    u32 volatile reg;
    uint32_t ret = 0;

    // Check EMAC interrupt
    mac_status_reg = EMAC_GET_INT_SUMMARY(emacdev);

    /* handle ts status */
    if(mac_status_reg & EMAC_IntStatus_TSIS_Msk) {
        emacdev->NetStats.ts_int = 1;
        reg = EMAC_GET_TS_STATUS(emacdev); // read to clear
        if(!(reg & EMAC_TSStatus_TSTARGT_Msk))
            TR("TS alarm flag not set??\n");
        else
            TR("TS alarm!!!!!!!!!!!!!!!!\n");
    }

    /* handle lpi status */
    if(mac_status_reg & EMAC_IntStatus_LPIIS_Msk) {
        reg = EMAC_GET_LPI_STATUS(emacdev); // read to clear
    }

    /* handle rgmii status */
    if(mac_status_reg & EMAC_IntStatus_RGSMIIIS_Msk) {
        reg = EMAC_GET_RGMII_STATUS(emacdev); // read to clear
    }

    EMAC_CLR_INT_SUMMARY(emacdev, mac_status_reg);

    /*Read the Dma interrupt status to know whether the interrupt got generated by our device or not*/
    dma_status_reg = EMAC_GET_DMA_STATUS(emacdev);

    if(dma_status_reg == 0)
        return ret;

    EMAC_disable_interrupt_all(emacdev);

    TR("%s:Dma Status Reg: 0x%08x\n",__FUNCTION__,dma_status_reg);

    if(dma_status_reg & EMAC_DmaStatus_GPI_Msk) {
        TR("%s:: Interrupt due to PMT module\n",__FUNCTION__);
        EMAC_powerup_mac(emacdev);
    }

    if(dma_status_reg & EMAC_DmaStatus_GLI_Msk) {
        TR("%s:: Interrupt due to EMAC LINE module\n",__FUNCTION__);
    }

    /*Now lets handle the DMA interrupts*/
    interrupt = EMAC_get_interrupt_type(emacdev);
    TR("%s:Interrupts to be handled: 0x%08x\n",__FUNCTION__,interrupt);

    if(interrupt & EMACDmaError) {
        TR("%s::Fatal Bus Error Inetrrupt Seen\n",__FUNCTION__);
        EMAC_DMA_TX_DISABLE(emacdev);
        EMAC_DMA_RX_DISABLE(emacdev);

        EMAC_take_desc_ownership_tx(emacdev);
        EMAC_take_desc_ownership_rx(emacdev);

        EMAC_init_tx_rx_desc_queue(emacdev);

        EMAC_reset(emacdev); //reset the DMA engine and the EMAC ip

        EMAC_set_mac_addr(emacdev, 0, emacdev->Intf == 0 ? mac_addr0 : mac_addr1);
        EMAC_DMA_BUSMODE_INIT(emacdev, EMAC_DmaBusMode_FB_Msk | DmaBurstLength8 | DmaDescriptorSkip0);
        EMAC_DMA_OPMODE_INIT(emacdev, EMAC_DmaOpMode_TSF_Msk);

        EMAC_init_rx_desc_base(emacdev);
        EMAC_init_tx_desc_base(emacdev);
        EMAC_init(emacdev);
        EMAC_DMA_RX_ENABLE(emacdev);
        EMAC_DMA_TX_ENABLE(emacdev);
    }

    if(interrupt & EMACDmaRxNormal) {
        TR("%s:: Rx Normal \n", __FUNCTION__);
        notify_rx_task(EMACINTF1);
    }

    if(interrupt & EMACDmaRxAbnormal) {
        TR("%s::Abnormal Rx Interrupt Seen\n",__FUNCTION__);
        emacdev->NetStats.rx_over_errors++;

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_DMA_RX_PD_RESUME(emacdev);//To handle GBPS with 12 descriptors
        }
    }

    if(interrupt & EMACDmaRxStopped) {
        TR("%s::Receiver stopped seeing Rx interrupts\n",__FUNCTION__); //Receiver gone in to stopped state

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            emacdev->NetStats.rx_over_errors++;
            EMAC_DMA_RX_ENABLE(emacdev);
        }
    }

    if(interrupt & EMACDmaTxNormal) {
        //xmit function has done its job
        TR("%s::Finished Normal Transmission \n",__FUNCTION__);
        EMAC_handle_transmit_over(EMACINTF1);//Do whatever you want after the transmission is over
    }

    if(interrupt & EMACDmaTxAbnormal) {
        TR("%s::Abnormal Tx Interrupt Seen\n",__FUNCTION__);

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_handle_transmit_over(EMACINTF1);
        }
    }

    if(interrupt & EMACDmaTxStopped) {
        TR("%s::Transmitter stopped sending the packets\n",__FUNCTION__);

        if(EMAC_Power_down == 0) {	// If Mac is not in powerdown
            EMAC_DMA_TX_DISABLE(emacdev);
            EMAC_take_desc_ownership_tx(emacdev);

            EMAC_DMA_TX_ENABLE(emacdev);
            TR("%s::Transmission Resumed\n",__FUNCTION__);
        }
    }

    /* Enable the interrupt before returning from ISR*/
    EMAC_enable_interrupt(emacdev, DMA_INT_ENABLE);

    return ret;
}
