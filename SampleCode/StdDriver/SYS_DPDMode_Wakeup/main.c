/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Let CA35 enter DPD mode only, DDR keep working. Use Timer 
 *           to warm boot system from DPD mode.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

void TMR0_IRQHandler(void)
{
    // Clear wake up flag
    TIMER_ClearWakeupFlag(TIMER0);
    // Clear interrupt flag
    TIMER_ClearIntFlag(TIMER0);
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);
    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));

    /* Enable Timer clock */
    CLK_EnableModuleClock(TMR0_MODULE);
    /* Select Timer clock source from LIRC */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_LIRC, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Lock protected registers */
    SYS_LockReg();
}

int main1(void)
{
    __WFI();
}

int main(void)
{
    uint32_t reg;

    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
    sysprintf("MA35H0 Warm boot sample code\n");
    while(!UART_IS_TX_EMPTY(UART0));

    if(SYS->DDRCQCSR & 0x0002FF00) {
        sysprintf("Q ch deny interrupt flag set %08x\n", SYS->DDRCQCSR);
        reg = SYS->DDRCQCSR;
        SYS->DDRCQCSR = reg;
    }

    /* Initial Timer0 to one-shot mode with 1Hz, since system is fast (180MHz)
       and timer is slow (32kHz), and following function calls all modified timer's
       CTL register, so add extra delay between each function call and make sure the
       setting take effect */
    TIMER_Open(TIMER0, TIMER_ONESHOT_MODE, 1);

    /* Enable timer wake up system */
    TIMER_EnableWakeup(TIMER0);

    /* Enable Timer0 interrupt */
    TIMER_EnableInt(TIMER0);

    IRQ_SetHandler((IRQn_ID_t)TMR0_IRQn, TMR0_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)TMR0_IRQn);

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Start Timer0 counting */
    TIMER_Start(TIMER0);

    SYS->CA35WRBPAR0= 0x7761726D;
    SYS->CA35WRBADR0= 0x28018000;

    SYS_DPD();

    while(1);
}
