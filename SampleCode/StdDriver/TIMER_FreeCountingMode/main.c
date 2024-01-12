/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Use the timer pin PL.14 to demonstrate timer free counting mode
 *           function. And displays the measured input frequency to
 *           UART console
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


void TMR2_IRQHandler(void)
{
    static int cnt = 0;
    static uint32_t t0, t1;

    TIMER_ClearCaptureIntFlag(TIMER2);

    if(cnt == 0)
    {
        t0 = TIMER_GetCaptureData(TIMER2);
        cnt++;
    }
    else if(cnt == 1)
    {
        t1 = TIMER_GetCaptureData(TIMER2);
        cnt++;
        if(t0 >= t1)
        {
            // over run, drop this data and do nothing
        }
        else
        {
            sysprintf("Input frequency is %dHz\n", 180000000 / (t1 - t0));
        }
    }
    else
    {
        cnt = 0;
    }
}


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR2_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Set Timer2 capture pin */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL15MFP_TM2_EXT;

    /* Lock protected registers */
    SYS_LockReg();
}


int main(void)
{
    int volatile i;

    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    sysprintf("\nThis sample code demonstrate timer free counting mode.\n");
    sysprintf("Please connect input source with Timer2 capture pin PL.15, press any key to continue\n");
    sysgetchar();

    // Give a dummy target frequency here. Will over write capture resolution with macro
    TIMER_Open(TIMER2, TIMER_PERIODIC_MODE, 1000000);

    // Update prescale to set proper resolution.
    TIMER_SET_PRESCALE_VALUE(TIMER2, 0);

    // Set compare value as large as possible, so don't need to worry about counter overrun too frequently.
    TIMER_SET_CMP_VALUE(TIMER2, 0xFFFFFF);

    // Configure Timer2 free counting mode, capture TDR value on rising edge
    TIMER_EnableCapture(TIMER2, TIMER_CAPTURE_FREE_COUNTING_MODE, TIMER_CAPTURE_EVENT_RISING);

    // Start Timer2
    TIMER_Start(TIMER2);

    // Enable timer interrupt
    TIMER_EnableCaptureInt(TIMER2);
    IRQ_SetHandler((IRQn_ID_t)TMR2_IRQn, TMR2_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)TMR2_IRQn);

    while(1);

}
