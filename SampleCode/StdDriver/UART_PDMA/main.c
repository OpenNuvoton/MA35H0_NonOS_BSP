/**************************************************************************//**
 * @file     main.c
 *
 * @brief    Demonstrate UART transmit and receive function with PDMA
 *
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

#define PDMA                   ((PDMA_T *)  PDMA1_BASE)

#define ENABLE_PDMA_INTERRUPT 1
#define PDMA_TEST_LENGTH 128
#define PDMA_TIME 0x5555

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
static uint8_t g_u8Tx_Buffer[PDMA_TEST_LENGTH] __attribute__((aligned(512)));
static uint8_t g_u8Rx_Buffer[PDMA_TEST_LENGTH] __attribute__((aligned(512)));

volatile uint32_t u32IsTestOver = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA1_IRQHandler(void);
void UART_PDMATest(void);


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
    CLK_EnableModuleClock(PDMA1_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPE multi-function pins for UART0 RXD and TXD */
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
    UART_Open(UART0, 115200);
}

void UART6_Init()
{
    UART_Open(UART6, 115200);
}

void PDMA_Init(void)
{
    /* Open PDMA Channel */
    PDMA_Open(PDMA,1 << 0); // Channel 0 for UART6 TX
    PDMA_Open(PDMA,1 << 1); // Channel 1 for UART6 RX
    // Set data width and transfer count
    PDMA_SetTransferCnt(PDMA,0, PDMA_WIDTH_8, PDMA_TEST_LENGTH);
    PDMA_SetTransferCnt(PDMA,1, PDMA_WIDTH_8, PDMA_TEST_LENGTH);
    //Set PDMA Transfer Address
    PDMA_SetTransferAddr(PDMA,0, ((uint32_t)((uint64_t)(&g_u8Tx_Buffer[0]))), PDMA_SAR_INC, UART6_BASE, PDMA_DAR_FIX);
    PDMA_SetTransferAddr(PDMA,1, UART6_BASE, PDMA_SAR_FIX, ((uint32_t)((uint64_t)(&g_u8Rx_Buffer[0]))), PDMA_DAR_INC);
    //Select Single Request
    PDMA_SetBurstType(PDMA,0, PDMA_REQ_SINGLE, 0);
    PDMA_SetBurstType(PDMA,1, PDMA_REQ_SINGLE, 0);
    // Select basic mode
    PDMA_SetTransferMode(PDMA,0, PDMA_UART6_TX, 0, 0);
    PDMA_SetTransferMode(PDMA,1, PDMA_UART6_RX, 0, 0);
    //Set timeout
    //PDMA_SetTimeOut(0, 0, 0x5555);
    //PDMA_SetTimeOut(1, 0, 0x5555);

#ifdef ENABLE_PDMA_INTERRUPT
    PDMA_EnableInt(PDMA,0, 0);
    PDMA_EnableInt(PDMA,1, 0);
    IRQ_SetHandler((IRQn_ID_t)PDMA1_IRQn, PDMA1_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)PDMA1_IRQn);
    u32IsTestOver = 0;
#endif
}

int main(void)
{
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    /* Init UART0 for sysprintf */
    UART_Init();

    /* Init UART6 */
    UART6_Init();

    UART_PDMATest();

    while(1);
}

/**
 * @brief       DMA IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The DMA default IRQ.
 */
void PDMA1_IRQHandler(void)
{
    uint32_t status = PDMA_GET_INT_STATUS(PDMA);

    if (status & 0x1)   /* abort */
    {
        sysprintf("target abort interrupt !!\n");
        if (PDMA_GET_ABORT_STS(PDMA) & 0x4)
            u32IsTestOver = 2;
        PDMA_CLR_ABORT_FLAG(PDMA,PDMA_GET_ABORT_STS(PDMA));
    }
    else if (status & 0x2)     /* done */
    {
        if ( (PDMA_GET_TD_STS(PDMA) & (1 << 0)) && (PDMA_GET_TD_STS(PDMA) & (1 << 1)) )
        {
            u32IsTestOver = 1;
            PDMA_CLR_TD_FLAG(PDMA,PDMA_GET_TD_STS(PDMA));
        }
    }
    else if (status & 0x300)     /* channel 2 timeout */
    {
        sysprintf("timeout interrupt !!\n");
        u32IsTestOver = 3;

        PDMA_SetTimeOut(PDMA,0, 0, 0);
        PDMA_CLR_TMOUT_FLAG(PDMA,0);
        PDMA_SetTimeOut(PDMA,0, 1, PDMA_TIME);

        PDMA_SetTimeOut(PDMA,1, 0, 0);
        PDMA_CLR_TMOUT_FLAG(PDMA,1);
        PDMA_SetTimeOut(PDMA,1, 1, PDMA_TIME);
    }
    else
        sysprintf("unknown interrupt !!\n");
}


/*---------------------------------------------------------------------------------------------------------*/
/*  UART PDMA Test                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void UART6_IRQHandler(void);
void UART_PDMATest()
{
    uint32_t i;

    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|  UART PDMA Test                                           |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("|  Description :                                            |\n");
    sysprintf("|    The sample code will demo UART6 PDMA function.         |\n");
    sysprintf("|    Please connect UART6_TX and UART6_RX pin.              |\n");
    sysprintf("+-----------------------------------------------------------+\n");
    sysprintf("Please press any key to start test. \n\n");

    sysgetchar();

    /*
        Using UAR1 external loop back.
        This code will send data from UART6_TX and receive data from UART6_RX.
    */

    for (i=0; i<PDMA_TEST_LENGTH; i++)
    {
        *(unsigned char volatile *)((u64)&g_u8Tx_Buffer[i] | 0x100000000) = i;
        *(unsigned char volatile *)((u64)&g_u8Rx_Buffer[i] | 0x100000000) = 0xff;
    }

    while(1)
    {
        PDMA_Init();

        UART6->INTEN = UART_INTEN_RLSIEN_Msk; // Enable Receive Line interrupt
        IRQ_SetHandler((IRQn_ID_t)UART6_IRQn, UART6_IRQHandler);
        IRQ_Enable ((IRQn_ID_t)UART6_IRQn);

        UART6->INTEN |= UART_INTEN_TXPDMAEN_Msk | UART_INTEN_RXPDMAEN_Msk;

#ifdef ENABLE_PDMA_INTERRUPT
        while(u32IsTestOver == 0);

        if (u32IsTestOver == 1)
            sysprintf("test done...\n");
        else if (u32IsTestOver == 2)
            sysprintf("target abort...\n");
        else if (u32IsTestOver == 3)
            sysprintf("timeout...\n");
#else
        while( (!(PDMA_GET_TD_STS(PDMA) & (1 << 0))) || (!(PDMA_GET_TD_STS(PDMA) & (1 << 1))) );

        PDMA_CLR_TD_FLAG(PDMA, PDMA_TDSTS_TDIF0_Msk|PDMA_TDSTS_TDIF1_Msk);
#endif

        UART6->INTEN &= ~UART_INTEN_TXPDMAEN_Msk;
        UART6->INTEN &= ~UART_INTEN_RXPDMAEN_Msk;

        for (i=0; i<PDMA_TEST_LENGTH; i++)
        {
            if(*(unsigned char volatile *)((u64)&g_u8Rx_Buffer[i] | 0x100000000) != i)
            {
                sysprintf("\n Receive Data Compare Error !!");
                while(1);
            }

            *(unsigned char volatile *)((u64)&g_u8Rx_Buffer[i] | 0x100000000) = 0xff;
        }

        sysprintf("\nUART PDMA test Pass.\n");
    }

}

void UART6_IRQHandler(void)
{
    uint32_t u32DAT;
    uint32_t u32IntSts = UART6->INTSTS;

    if(u32IntSts & UART_INTSTS_PRLSIF_Msk)
    {
        if(UART6->FIFOSTS & UART_FIFOSTS_BIF_Msk)
            sysprintf("\n BIF \n");
        if(UART6->FIFOSTS & UART_FIFOSTS_FEF_Msk)
            sysprintf("\n FEF \n");
        if(UART6->FIFOSTS & UART_FIFOSTS_PEF_Msk)
            sysprintf("\n PEF \n");

        u32DAT = UART6->DAT; // read out data
        sysprintf("\n Error Data is '0x%x' \n", u32DAT);
        UART6->FIFOSTS = (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk);
    }
}





