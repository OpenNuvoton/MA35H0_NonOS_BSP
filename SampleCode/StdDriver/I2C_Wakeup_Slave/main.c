/*************************************************************************//**
 * @file     main.c
 *
 * @brief
 *           Show how to wake up system from Power-down mode through I2C interface.
 *           This sample code needs to work with I2C_Master.
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

#define DEBUG_PORT   UART0

uint32_t slave_buff_addr;
uint8_t g_au8SlvData[256];
uint8_t g_au8SlvRxData[3];
uint8_t g_u8SlvI2CWK;
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t g_u8DeviceAddr;
uint8_t g_u8SlvDataLen;

typedef void (*I2C_FUNC)(uint32_t u32Status);

static I2C_FUNC s_I2C4HandlerFn = NULL;


void I2C4_IRQHandler(void)
{
    uint32_t u32Status;

    /* Check I2C Wake-up interrupt flag set or not */
    if(I2C_GET_WAKEUP_FLAG(I2C4))
    {
        /* Waitinn for I2C response ACK finish */
        while(!(I2C4->WKSTS & I2C_WKSTS_WKAKDONE_Msk));

        /* Clear Wakeup done flag, I2C will release bus */
        I2C4->WKSTS = I2C_WKSTS_WKAKDONE_Msk;

        /* Clear I2C Wake-up interrupt flag */
        I2C_CLEAR_WAKEUP_FLAG(I2C4);
        g_u8SlvI2CWK = 1;

        return;
    }

    u32Status = I2C_GET_STATUS(I2C4);

    if(I2C_GET_TIMEOUT_FLAG(I2C4))
    {
        /* Clear I2C4 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C4);
    }
    else
    {
        if(s_I2C4HandlerFn != NULL)
            s_I2C4HandlerFn(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void PowerDownFunction(void)
{
    /* Check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(DEBUG_PORT);

    /* Enter to Power-down mode */
    SYS_NPD();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Slave Transmit/Receive Callback Function                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_SlaveTRx(uint32_t u32Status)
{
    if(u32Status == 0x60)                       /* Own SLA+W has been receive; ACK has been return */
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0x80)                 /* Previously address with own SLA address
                                                   Data has been received; ACK has been returned*/
    {
        g_au8SlvRxData[g_u8SlvDataLen] = (unsigned char)I2C_GET_DATA(I2C4);
        g_u8SlvDataLen++;

        if(g_u8SlvDataLen == 2)
        {
            slave_buff_addr = (g_au8SlvRxData[0] << 8) + g_au8SlvRxData[1];
        }
        if(g_u8SlvDataLen == 3)
        {
            g_au8SlvData[slave_buff_addr] = g_au8SlvRxData[2];
            g_u8SlvDataLen = 0;
        }
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0xA8)                  /* Own SLA+R has been receive; ACK has been return */
    {
        I2C_SET_DATA(I2C4, g_au8SlvData[slave_buff_addr]);
        slave_buff_addr++;
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0xC0)                 /* Data byte or last data in I2CDAT has been transmitted
                                                   Not ACK has been received */
    {
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0x88)                 /* Previously addressed with own SLA address; NOT ACK has
                                                   been returned */
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else if(u32Status == 0xA0)                 /* A STOP or repeated START has been received while still
                                                   addressed as Slave/Receiver*/
    {
        g_u8SlvDataLen = 0;
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);
    }
    else
    {
        /* TO DO */
        sysprintf("Status 0x%x is NOT processed\n", u32Status);
    }
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HXT */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* Waiting clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Enable IP clock */
    CLK_EnableModuleClock(I2C4_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HXT, CLK_CLKDIV1_UART0(2));
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(GPM_MODULE);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPE multi-function pins for UART0 RXD and TXD */
    SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
    SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);
    /* Set multi-function pins for I2C4 */
    SYS->GPM_MFPL &= ~(SYS_GPM_MFPL_PM0MFP_Msk | SYS_GPM_MFPL_PM1MFP_Msk);
    SYS->GPM_MFPL |= SYS_GPM_MFPL_PM0MFP_I2C4_SDA | SYS_GPM_MFPL_PM1MFP_I2C4_SCL;

    /* I2C pin enable schmitt trigger */
    PM->SMTEN |= GPIO_SMTEN_SMTEN0_Msk | GPIO_SMTEN_SMTEN1_Msk;

    /* Lock protected registers */
    SYS_LockReg();
}

void I2C4_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C4, 100000);

    /* Get I2C4 Bus Clock */
    sysprintf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C4));

    /* Set I2C 4 Slave Addresses */
    I2C_SetSlaveAddr(I2C4, 0, 0x15, 0);   /* Slave Address : 0x15 */
    I2C_SetSlaveAddr(I2C4, 1, 0x35, 0);   /* Slave Address : 0x35 */
    I2C_SetSlaveAddr(I2C4, 2, 0x55, 0);   /* Slave Address : 0x55 */
    I2C_SetSlaveAddr(I2C4, 3, 0x75, 0);   /* Slave Address : 0x75 */

    /* Set I2C 4 Slave Addresses Mask */
    I2C_SetSlaveAddrMask(I2C4, 0, 0x01);
    I2C_SetSlaveAddrMask(I2C4, 1, 0x04);
    I2C_SetSlaveAddrMask(I2C4, 2, 0x01);
    I2C_SetSlaveAddrMask(I2C4, 3, 0x04);

    /* Enable I2C interrupt */
    I2C_EnableInt(I2C4);
    IRQ_SetHandler((IRQn_ID_t)I2C4_IRQn, I2C4_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)I2C4_IRQn);
}

void I2C4_Close(void)
{
    /* Disable I2C4 interrupt and clear corresponding GIC bit */
    I2C_DisableInt(I2C4);
    GIC_DisableIRQ(I2C4_IRQn);

    /* Disable I2C4 and close I2C4 clock */
    I2C_Close(I2C4);
    CLK_DisableModuleClock(I2C4_MODULE);
}

int main1(void)
{
    __WFI();
}

int32_t main(void)
{
    uint32_t i;

    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();

    /* Configure UART: 115200, 8-bit word, no parity bit, 1 stop bit. */
    UART_Open(DEBUG_PORT, 115200);

    /*
        This sample code is I2C SLAVE mode and it simulates EEPROM function
    */
    sysprintf("\n");
    sysprintf("+----------------------------------------------------------------------+\n");
    sysprintf("| I2C Driver Sample Code (Slave) for wake-up & access Slave test       |\n");
    sysprintf("|                                                                      |\n");
    sysprintf("| I2C Master (I2C4) <---> I2C Slave(I2C4)                              |\n");
    sysprintf("+----------------------------------------------------------------------+\n");

    sysprintf("Configure I2C4 as a slave.\n");
    sysprintf("The I/O connection for I2C4:\n");

    /* Init I2C4 */
    I2C4_Init();

    for(i = 0; i < 0x100; i++)
    {
        g_au8SlvData[i] = 0;
    }

    /* I2C function to Transmit/Receive data as slave */
    s_I2C4HandlerFn = I2C_SlaveTRx;

    /* Set I2C4 enter Not Address SLAVE mode */
    I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable I2C wake-up */
    I2C_EnableWakeup(I2C4);
    g_u8SlvI2CWK = 0;

    sysprintf("\n");
    sysprintf("\n");
    sysprintf("CHIP enter power down status.\n");

    if(((I2C4->CTL0)&I2C_CTL0_SI_Msk) != 0)
    {
        I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI);
    }
    /* Enter to Power-down mode */
    PowerDownFunction();

    /* Waiting for syteem wake-up and I2C wake-up finish*/
    while(g_u8SlvI2CWK == 0);

    /* Wake-up Interrupt Message */
    sysprintf("I2C4 WAKE INT 0x%x\n", I2C4->WKSTS);

    /* Lock protected registers */
    SYS_LockReg();

    sysprintf("\n");
    sysprintf("Slave wake-up from power down status.\n");

    sysprintf("\n");
    sysprintf("Slave Waiting for receiving data.\n");

    while(1);
}



