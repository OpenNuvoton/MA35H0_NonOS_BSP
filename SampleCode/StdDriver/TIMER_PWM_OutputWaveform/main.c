/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Enable 4 Timer PWM output channels with different frequency and duty ratio.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR2_MODULE);
    CLK_EnableModuleClock(TMR3_MODULE);
    CLK_EnableModuleClock(TMR4_MODULE);
    CLK_EnableModuleClock(TMR7_MODULE);

    /* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);
    CLK_SetModuleClock(TMR4_MODULE, CLK_CLKSEL1_TMR4SEL_PCLK2, 0);
    CLK_SetModuleClock(TMR7_MODULE, CLK_CLKSEL1_TMR7SEL_PCLK0, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Set Timer PWM output pins */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL14MFP_TM2;
    SYS->GPG_MFPL |= SYS_GPG_MFPL_PG2MFP_TM3 | SYS_GPG_MFPL_PG4MFP_TM4 | SYS_GPG_MFPL_PG6MFP_TM7;

    /* Lock protected registers */
    SYS_LockReg();
}

int main(void)
{
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    sysprintf("+-----------------------------------------------+\n");
    sysprintf("|    Timer  PWM Output Duty Sample Code         |\n");
    sysprintf("+-----------------------------------------------+\n\n");

    sysprintf("# Timer2 PWM_CH0 output frequency is 18000 Hz and duty is 50%%.\n");
    sysprintf("# Timer3 PWM_CH0 output frequency is 10000 Hz and duty is 10%%.\n");
    sysprintf("# Timer4 PWM_CH0 output frequency is  9000 Hz and duty is 75%%.\n");
    sysprintf("# Timer7 PWM_CH0 output frequency is  4000 Hz and duty is 20%%.\n");
    sysprintf("# I/O configuration:\n");
    sysprintf("    - Timer2 PWM_CH0 on PL.14\n");
    sysprintf("    - Timer3 PWM_CH0 on PG.2\n");
    sysprintf("    - Timer4 PWM_CH0 on PG.4\n");
    sysprintf("    - Timer7 PWM_CH0 on PG.6\n\n");

    /* Change Timer to PWM counter mode */
    TPWM_ENABLE_PWM_MODE(TIMER2);
    TPWM_ENABLE_PWM_MODE(TIMER3);
    TPWM_ENABLE_PWM_MODE(TIMER4);
    TPWM_ENABLE_PWM_MODE(TIMER7);

    /* Set PWM mode as independent mode*/
    TPWM_ENABLE_INDEPENDENT_MODE(TIMER2);
    TPWM_ENABLE_INDEPENDENT_MODE(TIMER3);
    TPWM_ENABLE_INDEPENDENT_MODE(TIMER4);
    TPWM_ENABLE_INDEPENDENT_MODE(TIMER7);

    /* Enable output of PWM_CH0 */
    TPWM_ENABLE_OUTPUT(TIMER2, TPWM_CH0);
    TPWM_ENABLE_OUTPUT(TIMER3, TPWM_CH0);
    TPWM_ENABLE_OUTPUT(TIMER4, TPWM_CH0);
    TPWM_ENABLE_OUTPUT(TIMER7, TPWM_CH0);

    /* Set Timer2 PWM output frequency is 18000 Hz, duty 50% in up count type */
    TPWM_ConfigOutputFreqAndDuty(TIMER2, 18000, 50);

    /* Set Timer3 PWM output frequency is 10000 Hz, duty 10% in up count type */
    TPWM_ConfigOutputFreqAndDuty(TIMER3, 10000, 10);

    /* Set Timer4 PWM output frequency is 9000 Hz, duty 75% in up count type */
    TPWM_ConfigOutputFreqAndDuty(TIMER4, 9000, 75);

    /* Set Timer7 PWM output frequency is 4000 Hz, duty 20% in up count type */
    TPWM_ConfigOutputFreqAndDuty(TIMER7, 4000, 20);

    /* Set PWM up count type */
    TPWM_SET_COUNTER_TYPE(TIMER2, TPWM_UP_COUNT);
    TPWM_SET_COUNTER_TYPE(TIMER3, TPWM_UP_COUNT);
    TPWM_SET_COUNTER_TYPE(TIMER4, TPWM_UP_COUNT);
    TPWM_SET_COUNTER_TYPE(TIMER7, TPWM_UP_COUNT);

    /* Start Timer PWM counter */
    TPWM_START_COUNTER(TIMER2);
    TPWM_START_COUNTER(TIMER3);
    TPWM_START_COUNTER(TIMER4);
    TPWM_START_COUNTER(TIMER7);

    sysprintf("*** Check Timer PWM_CH0 output waveform by oscilloscope ***\n");

    while(1);
}
