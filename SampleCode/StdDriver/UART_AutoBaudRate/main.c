/**************************************************************************//**
 * @file     main.c
 *
 *           Show how to use auto baud rate detection function.
 *
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void AutoBaudRate_Test(void);
void AutoBaudRate_TxTest(void);
void AutoBaudRate_RxTest(void);


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HXT */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* Waiting clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Enable IP clock */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(2));
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART6_MODULE, CLK_CLKSEL2_UART6SEL_HXT, CLK_CLKDIV2_UART6(1));
    CLK_EnableModuleClock(UART6_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set multi-function pins for Debug UART RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);
    /* Set multi-function pins for UART6 */
    SYS->GPN_MFPH &= ~(SYS_GPN_MFPH_PN12MFP_Msk | SYS_GPN_MFPH_PN13MFP_Msk | SYS_GPN_MFPH_PN14MFP_Msk | SYS_GPN_MFPH_PN15MFP_Msk);
    SYS->GPN_MFPH |= SYS_GPN_MFPH_PN12MFP_UART6_nCTS | SYS_GPN_MFPH_PN13MFP_UART6_nRTS |
    		SYS_GPN_MFPH_PN14MFP_UART6_RXD | SYS_GPN_MFPH_PN15MFP_UART6_TXD;

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init()
{
    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

void UART6_Init()
{
    /* Configure UART6 and set UART6 Baudrate */
    UART_Open(UART6, 115200);
}

int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART for sysprintf */
    UART_Init();

    /* Init UART6 */
    UART6_Init();


    sysprintf("\n\nCPU @ %dHz\n", SystemCoreClock);

    sysprintf("\nUART Sample Program\n");

    /* UART auto baud rate sample function */
    AutoBaudRate_Test();

    sysprintf("\nUART Sample Program End\n");

    while(1);

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Auto Baud Rate Function Test                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void AutoBaudRate_Test()
{

    uint32_t u32Item;

    sysprintf("\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|     Pin Configure                                         |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|  ______                                            _____  |\n");
    sysprintf("| |      |                                          |     | |\n");
    sysprintf("| |Master|--UART6_TXD    <=========>     UART6_RXD--|Slave| |\n");
    sysprintf("| |      |                                          |     | |\n");
    sysprintf("| |______|                                          |_____| |\n");
    sysprintf("|                                                           |\n");
    sysprintf("+-----------------------------------------------------------+\n");

    sysprintf("\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|     Auto Baud Rate Function Test                          |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|  Description :                                            |\n");
    sysprintf("|    The sample code needs two boards. One is Master and    |\n");
    sysprintf("|    the other is slave.  Master will send input pattern    |\n");
    sysprintf("|    0x1 with different baud rate. It can check if Slave    |\n");
    sysprintf("|    calculates correct baud rate.                          |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|  Please select Master or Slave test                       |\n");
    sysprintf("|  [0] Master    [1] Slave                                  |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    u32Item = sysgetchar();

    if(u32Item == '0')
        AutoBaudRate_TxTest();
    else
        AutoBaudRate_RxTest();

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Auto Baud Rate Function Tx Test                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void AutoBaudRate_TxTest()
{
    uint32_t u32Item;

    do
    {

        sysprintf("\n");
        sysprintf("+-----------------------------------------------------------+\n");
        sysprintf("|     Auto Baud Rate Function Test (Master)                 |\n");
        sysprintf("+-----------------------------------------------------------+\n");
        sysprintf("| [1] baud rate 38400 bps                                   |\n");
        sysprintf("| [2] baud rate 57600 bps                                   |\n");
        sysprintf("| [3] baud rate 115200 bps                                  |\n");
        sysprintf("|                                                           |\n");
        sysprintf("| Select baud rate and master will send 0x1 to slave ...    |\n");
        sysprintf("+-----------------------------------------------------------+\n");
        sysprintf("| Quit                                              - [ESC] |\n");
        sysprintf("+-----------------------------------------------------------+\n\n");
        u32Item = sysgetchar();
        sysprintf("%c\n", u32Item);

        /* Set different baud rate */
        switch(u32Item)
        {
        case '1':
            UART6->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 38400);
            break;
        case '2':
            UART6->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 57600);
            break;
        default:
            UART6->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 115200);
            break;
        }

        /* Send input pattern 0x1 for auto baud rate detection bit length is 1-bit */
        UART_WRITE(UART6, 0x1);

    }
    while(u32Item != 27);

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Get UART Baud Rate Function                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t GetUartBaudrate(UART_T* uart)
{
    uint8_t u8UartClkSrcSel = 0, u8UartClkDivNum = 0;
    uint32_t u32ClkTbl[4] = {__HXT, 0ul, __LXT, __HIRC};
    uint32_t u32Baud_Div;

    /* Get UART clock source selection and UART clock divider number */
    u8UartClkSrcSel = (CLK->CLKSEL2 & CLK_CLKSEL2_UART6SEL_Msk) >> CLK_CLKSEL2_UART6SEL_Pos;
    u8UartClkDivNum = (CLK->CLKDIV2 & CLK_CLKDIV2_UART6DIV_Msk) >> CLK_CLKDIV2_UART6DIV_Pos;

    /* Get PLL clock frequency if UART clock source selection is PLL */
    if(u8UartClkSrcSel == 1)
        u32ClkTbl[u8UartClkSrcSel] = CLK_GetSYSCLK1Freq();

    /* Get UART baud rate divider */
    u32Baud_Div = (uart->BAUD & UART_BAUD_BRD_Msk) >> UART_BAUD_BRD_Pos;

    /* Calculate UART baud rate if baud rate is set in MODE 0 */
    if((uart->BAUD & (UART_BAUD_BAUDM1_Msk | UART_BAUD_BAUDM0_Msk)) == UART_BAUD_MODE0)
        return ((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1) / (u32Baud_Div + 2)) >> 4;

    /* Calculate UART baud rate if baud rate is set in MODE 2 */
    else if((uart->BAUD & (UART_BAUD_BAUDM1_Msk | UART_BAUD_BAUDM0_Msk)) == UART_BAUD_MODE2)
        return ((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1) / (u32Baud_Div + 2));

    /* Calculate UART baud rate if baud rate is set in MODE 1 */
    else if((uart->BAUD & (UART_BAUD_BAUDM1_Msk | UART_BAUD_BAUDM0_Msk)) == UART_BAUD_BAUDM1_Msk)
        return ((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1) / (u32Baud_Div + 2)) / (((uart->BAUD & UART_BAUD_EDIVM1_Msk) >> UART_BAUD_EDIVM1_Pos) + 1);

    /* Unsupported baud rate setting */
    else
        return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Auto Baud Rate Function Rx Test                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
void AutoBaudRate_RxTest()
{
    /* Enable auto baud rate detect function */
    UART6->ALTCTL |= UART_ALTCTL_ABRDEN_Msk;

    sysprintf("\nreceiving input pattern... ");

    /* Wait until auto baud rate detect finished or time-out */
    while((UART6->ALTCTL & UART_ALTCTL_ABRIF_Msk) == 0);

    if(UART6->FIFOSTS & UART_FIFOSTS_ABRDIF_Msk)
    {
        /* Clear auto baud rate detect finished flag */
        UART6->FIFOSTS = UART_FIFOSTS_ABRDIF_Msk;
        sysprintf("Baud rate is %dbps.\n", GetUartBaudrate(UART6));
    }
    else if(UART6->FIFOSTS & UART_FIFOSTS_ABRDTOIF_Msk)
    {
        /* Clear auto baud rate detect time-out flag */
        UART6->FIFOSTS = UART_FIFOSTS_ABRDTOIF_Msk;
        sysprintf("Time-out!\n");
    }

}
