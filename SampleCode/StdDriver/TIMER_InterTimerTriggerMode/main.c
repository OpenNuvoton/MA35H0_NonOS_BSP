/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Use the timer pin PL.14 to demonstrate inter timer trigger mode
 *           function. Also display the measured input frequency to UART console.
 *
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

int volatile complete = 0;

// Timer2 is working in event count mode, and Timer3 in capture mode, so we read the
// capture value from Timer3, _not_ timer2.
void TMR3_IRQHandler(void)
{

    // Timer clock is 180 MHz, counter value records the duration for 100 event counts.
    sysprintf("Event frequency is %d Hz\n", 180000000 / TIMER_GetCounter(TIMER3) * 100);
    TIMER_ClearCaptureIntFlag(TIMER3);
    complete = 1;

}

void SYS_Init(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR2_MODULE);
    CLK_EnableModuleClock(TMR3_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Set timer event counting pin */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL14MFP_TM2;

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

    /* This sample code demonstrate inter timer trigger mode using Timer2 and Timer3
     * In this mode, Timer2 is working as counter, and triggers Timer3. Using Timer3
     * to calculate the amount of time used by Timer2 to count specified amount of events.
     * By dividing the time period recorded in Timer3 by the event counts, we get
     * the event frequency.
     */
    sysprintf("Inter timer trigger mode demo code\n");
    sysprintf("Please connect input source with Timer2 counter pin PL.14, press any key to continue\n");
    sysgetchar();

    // Give a dummy target frequency here. Will over write prescale and compare value with macro
    TIMER_Open(TIMER2, TIMER_ONESHOT_MODE, 100);

    // Update prescale and compare value. Calculate average frequency every 100 events
    TIMER_SET_PRESCALE_VALUE(TIMER2, 0);
    TIMER_SET_CMP_VALUE(TIMER2, 100);

    // Update Timer3 prescale value. So Timer2 clock is 180MHz
    TIMER_SET_PRESCALE_VALUE(TIMER3, 0);

    // We need capture interrupt
    IRQ_SetHandler((IRQn_ID_t)TMR3_IRQn, TMR3_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)TMR3_IRQn);

    while(1)
    {
        complete = 0;
        // Count event by TIMER2, disable drop count (set to 0), disable timeout (set to 0). Enable interrupt after complete
        TIMER_EnableFreqCounter(TIMER2, 0, 0, TRUE);
        while(complete == 0);
    }

}
