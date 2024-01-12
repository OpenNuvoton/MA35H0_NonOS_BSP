/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Demonstrate Timer PWM Complementary mode and Dead-Time function.
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

    /* Peripheral clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(1));
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);

    /* Set Timer2 PWM CH0(TM2) and Timer3 PWM CH0(TM3) */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL14MFP_TM2;
    SYS->GPG_MFPL |= SYS_GPG_MFPL_PG2MFP_TM3;

    /* Set Timer2 PWM CH1(T2_EXT) and Timer3 PWM CH1(T3_EXT) */
    SYS->GPL_MFPH |= SYS_GPL_MFPH_PL15MFP_TM2_EXT;
    SYS->GPG_MFPL |= SYS_GPG_MFPL_PG3MFP_TM3_EXT;

    /* Lock protected registers */
    SYS_LockReg();
}


int main(void)
{
    uint32_t u32Period, u32CMP, u32Prescaler, u32DeadTime;

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);

    sysprintf("+--------------------------------------------------------------+\n");
    sysprintf("|    Timer PWM Complementary mode and Dead-Time Sample Code    |\n");
    sysprintf("+--------------------------------------------------------------+\n\n");

    /* Configure Timer2 PWM */
    /* Change Timer to PWM counter mode */
    TPWM_ENABLE_PWM_MODE(TIMER2);

    /* Set PWM mode as complementary mode*/
    TPWM_ENABLE_COMPLEMENTARY_MODE(TIMER2);

    /* Set Timer2 PWM output frequency is 6000 Hz, duty 40% */
    TPWM_ConfigOutputFreqAndDuty(TIMER2, 6000, 40);

    /* Enable output of PWM_CH0 and PWM_CH1 */
    TPWM_ENABLE_OUTPUT(TIMER2, (TPWM_CH1|TPWM_CH0));

    /* Get u32Prescaler, u32Period and u32CMP after called TPWM_ConfigOutputFreqAndDuty() API */
    u32Prescaler = (TIMER2->PWMCLKPSC + 1);
    u32Period = (TIMER2->PWMPERIOD + 1);
    u32CMP = TIMER2->PWMCMPDAT;
    u32DeadTime = u32CMP/2;

    sysprintf("# Timer2 PWM output frequency is 6000 Hz and duty 40%%.\n");
    sysprintf("    - Counter clock source:    PCLK1 \n");
    sysprintf("    - Counter clock prescaler: %d \n", u32Prescaler);
    sysprintf("    - Counter type:            Up count type \n");
    sysprintf("    - Operation mode:          Complementary in auto-reload mode \n");
    sysprintf("    - Period value:            %d \n", u32Period);
    sysprintf("    - Comparator value:        %d \n", u32CMP);
    sysprintf("# I/O configuration:\n");
    sysprintf("    - Timer2 PWM_CH0 on PL.14, PWM_CH1 on PL.15\n\n");


    /* Configure Timer3 PWM */
    sysprintf("# Timer3 PWM output frequency is 6000 Hz and duty 40%% with dead-time insertion.\n");
    sysprintf("    - Counter clock source:    PCLK1 \n");
    sysprintf("    - Counter clock prescaler: %d \n", u32Prescaler);
    sysprintf("    - Counter type:            Up count type \n");
    sysprintf("    - Operation mode:          Complementary in auto-reload mode \n");
    sysprintf("    - Period value:            %d \n", u32Period);
    sysprintf("    - Comparator value:        %d \n", u32CMP);
    sysprintf("    - Dead-Time interval:      %d \n", u32DeadTime);
    sysprintf("# I/O configuration:\n");
    sysprintf("    - Timer3 PWM_CH0 on PG.3, PWM_CH1 on PG.4\n\n");

    /* Change Timer to PWM counter mode */
    TPWM_ENABLE_PWM_MODE(TIMER3);

    /* Set PWM mode as complementary mode*/
    TPWM_ENABLE_COMPLEMENTARY_MODE(TIMER3);

    /* Set Timer3 PWM output frequency is 6000 Hz, duty 40% */
    TPWM_ConfigOutputFreqAndDuty(TIMER3, 6000, 40);

    /* Enable output of PWM_CH0 and PWM_CH1 */
    TPWM_ENABLE_OUTPUT(TIMER3, (TPWM_CH1|TPWM_CH0));

    /* Enable and configure dead-time interval is (u32DeadTime * TMR3_PWMCLK * prescaler) */
    SYS_UnlockReg(); // Unlock protected registers
    TPWM_EnableDeadTimeWithPrescale(TIMER3, (u32DeadTime-1));
    SYS_LockReg(); // Lock protected registers

    sysprintf("*** Check Timer2 and Timer3 PWM output waveform by oscilloscope ***\n");

    /* Start Timer2 and Timer3 PWM counter by trigger Timer2 sync. start */
    TPWM_SET_COUNTER_SYNC_MODE(TIMER2, TPWM_CNTR_SYNC_START_BY_TIMER2);
    TPWM_SET_COUNTER_SYNC_MODE(TIMER3, TPWM_CNTR_SYNC_START_BY_TIMER2);
    TPWM_TRIGGER_COUNTER_SYNC(TIMER2);

    while(1);
}
