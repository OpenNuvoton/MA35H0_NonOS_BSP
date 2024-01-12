/**************************************************************************//**
 * @file     main.c
 *
 * @brief    This Ethernet sample tends to get a DHCP lease from DHCP server.
 *           Modify configuration according to target MAC in ma35h0_mac.h
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include "NuMicro.h"
#include "net.h"

void UART0_Init()
{
    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));

    /* Set multi-function pins */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
}

/**
 * @brief Set EMAC clock source EPLL, init DDR and SSMCC for EMAC
 */
void SYS_Init()
{
    // Configure EPLL = 500MHz
    CLK->PLL[EPLL].CTL0 = (6 << CLK_PLLnCTL0_INDIV_Pos) | (250 << CLK_PLLnCTL0_FBDIV_Pos); // M=6, N=250
    CLK->PLL[EPLL].CTL1 = 2 << CLK_PLLnCTL1_OUTDIV_Pos; // EPLL divide by 2 and enable
    CLK_WaitClockReady(CLK_STATUS_STABLE_EPLL);

    // DDR Init
    outp32(UMCTL2_BASE + 0x6a0, 0x01);
}

void IRQ_Init(int intf)
{
    if(intf == EMACINTF0) {
        IRQ_SetHandler(EMAC0_IRQn, EMAC0_IRQHandler);
        IRQ_Enable(EMAC0_IRQn);
    }
    else {
        IRQ_SetHandler(EMAC1_IRQn, EMAC1_IRQHandler);
        IRQ_Enable(EMAC1_IRQn);
    }
}

/* main function */
int main(void)
{
    SYS_UnlockReg();

    SYS_Init();

    UART0_Init();

    global_timer_init();

    sysprintf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    sysprintf("+-----------------------------------+\n");
    sysprintf("|   MA35H0 EMAC Tx/Rx Sample Code   |\n");
    sysprintf("|       - Get IP from DHCP server   |\n");
    sysprintf("+-----------------------------------+\n\n");

    /* Initial MA35H0 EMAC module */
    if(EMAC_open(EMAC_INTF, EMAC_MODE))
        sysprintf("Link not found\n");

    IRQ_Init(EMAC_INTF);

    if(dhcp_start(EMAC_INTF) < 0)
    {
        // Cannot get a DHCP lease
        sysprintf("\nDHCP failed......\n");
    }

    while(1) {}
}
