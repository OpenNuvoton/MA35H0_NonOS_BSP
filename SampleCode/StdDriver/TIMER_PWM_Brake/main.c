/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Demonstrate how to use Timer2 PWM brake function.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

void TMR2_IRQHandler(void)
{
    sysprintf("\nFault brake!\n");
    sysprintf("Press any key to unlock brake state. (TIMER2 PWM output will toggle again)\n");
    sysgetchar();

    // Clear brake interrupt flag
    SYS_UnlockReg();
    TIMER2->PWMINTSTS1 = TIMER_PWMINTSTS1_BRKEIF0_Msk | TIMER_PWMINTSTS1_BRKEIF1_Msk | TIMER_PWMINTSTS1_BRKESTS0_Msk | TIMER_PWMINTSTS1_BRKESTS1_Msk;
    SYS_LockReg();
}


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR2_MODULE);

    /* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Set Timer2 PWM output pins and EPWM2 brake pin 0 (TPWM_TM_BRAKE0),
       Timers share the same brake pins with EPWM */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL14MFP_TM2;
    SYS->GPB_MFPH |= SYS_GPB_MFPH_PB8MFP_EPWM2_BRAKE0;

    /* Lock protected registers */
    SYS_LockReg();
}

int main(void)
{
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);

    sysprintf("\nL->H state change on PB.8 will generate brake interrupt,\n");
    sysprintf("and Timer2 PWM output will stop until brake state cleared.\n");

    /* Change Timer to PWM counter mode */
    TPWM_ENABLE_PWM_MODE(TIMER2);

    /* Set PWM mode as independent mode*/
    TPWM_ENABLE_INDEPENDENT_MODE(TIMER2);

    /* Set PWM up count type */
    TPWM_SET_COUNTER_TYPE(TIMER2, TPWM_UP_COUNT);

    /* Enable output of PWM_CH0 */
    TPWM_ENABLE_OUTPUT(TIMER2, TPWM_CH0);

    /* Set Timer2 PWM output frequency is 18000 Hz, duty 50% in up count type */
    TPWM_ConfigOutputFreqAndDuty(TIMER2, 18000, 50);

    /* Start Timer PWM counter */
    TPWM_START_COUNTER(TIMER2);

    // Enable brake and interrupt, PWM output stays at low after brake event
    SYS_UnlockReg();
    TPWM_SET_BRAKE_PIN_SOURCE(TIMER2, TPWM_TM_BRAKE0);
    TPWM_EnableFaultBrake(TIMER2, TPWM_OUTPUT_LOW, TPWM_OUTPUT_LOW, TPWM_BRAKE_SOURCE_EDGE_BKPIN);
    TPWM_EnableFaultBrakeInt(TIMER2, TPWM_BRAKE_EDGE);
    SYS_LockReg();

    IRQ_SetHandler((IRQn_ID_t)TMR2_IRQn, TMR2_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)TMR2_IRQn);

    while(1);
}
