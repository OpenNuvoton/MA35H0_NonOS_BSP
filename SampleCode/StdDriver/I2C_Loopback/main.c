/**************************************************************************//**
 * @file     main.c
 *
 * @brief
 *           Demonstrate how to set I2C Master mode and Slave mode.
 *           And show how a master access a slave on a chip.
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t slave_buff_addr;
volatile uint8_t g_au8SlvData[256];
volatile uint8_t g_au8SlvRxData[3];
volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8MstTxData[3];
volatile uint8_t g_u8MstRxData;
volatile uint8_t g_u8MstDataLen;
volatile uint8_t g_u8SlvDataLen;
volatile uint8_t g_u8MstEndFlag = 0;

typedef void (*I2C_FUNC)(uint32_t u32Status);

volatile static I2C_FUNC s_I2C5HandlerFn = NULL;
volatile static I2C_FUNC s_I2C4HandlerFn = NULL;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C5 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C5_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C5);

    if(I2C_GET_TIMEOUT_FLAG(I2C5))
    {
        /* Clear I2C5 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C5);
    }
    else
    {
        if(s_I2C5HandlerFn != NULL)
            s_I2C5HandlerFn(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C4 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C4_IRQHandler(void)
{
    uint32_t u32Status;

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
/*  I2C Master Rx Callback Function                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterRx(uint32_t u32Status)
{
    if(u32Status == 0x08)                       /* START has been transmitted and prepare SLA+W */
    {
        I2C_SET_DATA(I2C5, (g_u8DeviceAddr << 1));    /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C_SET_DATA(I2C5, g_au8MstTxData[g_u8MstDataLen++]);
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_STOP(I2C5);
        I2C_START(I2C5);
    }
    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
        if(g_u8MstDataLen != 2)
        {
            I2C_SET_DATA(I2C5, g_au8MstTxData[g_u8MstDataLen++]);
            I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
        }
        else
        {
            I2C_SET_CONTROL_REG(I2C5, I2C_CTL_STA_SI);
        }
    }
    else if(u32Status == 0x10)                  /* Repeat START has been transmitted and prepare SLA+R */
    {
        I2C_SET_DATA(I2C5, ((g_u8DeviceAddr << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x40)                  /* SLA+R has been transmitted and ACK has been received */
    {
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x58)                  /* DATA has been received and NACK has been returned */
    {
        g_u8MstRxData = (unsigned char) I2C_GET_DATA(I2C5);
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_STO_SI);
        g_u8MstEndFlag = 1;
    }
    else
    {
        /* TO DO */
        sysprintf("Status 0x%x is NOT processed\n", u32Status);
    }
}
/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Master Tx Callback Function                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterTx(uint32_t u32Status)
{
    if(u32Status == 0x08)                       /* START has been transmitted */
    {
        I2C_SET_DATA(I2C5, g_u8DeviceAddr << 1);    /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C_SET_DATA(I2C5, g_au8MstTxData[g_u8MstDataLen++]);
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
    }
    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_STOP(I2C5);
        I2C_START(I2C5);
    }
    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
        if(g_u8MstDataLen != 0x3)
        {
            I2C_SET_DATA(I2C5, g_au8MstTxData[g_u8MstDataLen++]);
            I2C_SET_CONTROL_REG(I2C5, I2C_CTL_SI);
        }
        else
        {
            I2C_SET_CONTROL_REG(I2C5, I2C_CTL_STO_SI);
            g_u8MstEndFlag = 1;
        }
    }
    else
    {
        /* TO DO */
        sysprintf("Status 0x%x is NOT processed\n", u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Slave TRx Callback Function                                                                       */
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
        g_au8SlvRxData[g_u8SlvDataLen] = I2C_GET_DATA(I2C4);
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
    CLK_EnableModuleClock(I2C5_MODULE);
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
    /* Set multi-function pins for I2C5 */
    SYS->GPM_MFPL &= ~(SYS_GPM_MFPL_PM4MFP_Msk | SYS_GPM_MFPL_PM5MFP_Msk);
    SYS->GPM_MFPL |= SYS_GPM_MFPL_PM4MFP_I2C5_SDA | SYS_GPM_MFPL_PM5MFP_I2C5_SCL;

    /* I2C pin enable schmitt trigger */
    PM->SMTEN |= GPIO_SMTEN_SMTEN0_Msk | GPIO_SMTEN_SMTEN1_Msk |
                 GPIO_SMTEN_SMTEN4_Msk | GPIO_SMTEN_SMTEN5_Msk;

    /* Lock protected registers */
    SYS_LockReg();
}

void I2C5_Init(void)
{
    /* Open I2C5 module and set bus clock */
    I2C_Open(I2C5, 100000);

    /* Get I2C5 Bus Clock */
    sysprintf("I2C5 clock %d Hz\n", I2C_GetBusClockFreq(I2C5));

    /* Enable I2C5 interrupt */
    I2C_EnableInt(I2C5);
    IRQ_SetHandler((IRQn_ID_t)I2C5_IRQn, I2C5_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)I2C5_IRQn);
}

void I2C4_Init(void)
{
    /* Open I2C4 module and set bus clock */
    I2C_Open(I2C4, 100000);

    /* Get I2C4 Bus Clock */
    sysprintf("I2C4 clock %d Hz\n", I2C_GetBusClockFreq(I2C4));

    /* Set I2C4 4 Slave Addresses */
    I2C_SetSlaveAddr(I2C4, 0, 0x16, 0);   /* Slave Address : 0x16 */
    I2C_SetSlaveAddr(I2C4, 1, 0x36, 0);   /* Slave Address : 0x36 */
    I2C_SetSlaveAddr(I2C4, 2, 0x56, 0);   /* Slave Address : 0x56 */
    I2C_SetSlaveAddr(I2C4, 3, 0x76, 0);   /* Slave Address : 0x76 */

    /* Set I2C4 4 Slave Addresses Mask */
    I2C_SetSlaveAddrMask(I2C4, 0, 0x04);
    I2C_SetSlaveAddrMask(I2C4, 1, 0x02);
    I2C_SetSlaveAddrMask(I2C4, 2, 0x04);
    I2C_SetSlaveAddrMask(I2C4, 3, 0x02);

    /* Enable I2C interrupt */
    I2C_EnableInt(I2C4);
    IRQ_SetHandler((IRQn_ID_t)I2C4_IRQn, I2C4_IRQHandler);
    IRQ_Enable ((IRQn_ID_t)I2C4_IRQn);
}

void I2C5_Close(void)
{
    /* Disable I2C5 interrupt and clear corresponding GIC bit */
    I2C_DisableInt(I2C5);
    GIC_DisableIRQ(I2C5_IRQn);

    /* Disable I2C5 and close I2C5 clock */
    I2C_Close(I2C5);
    CLK_DisableModuleClock(I2C5_MODULE);

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

int32_t I2C5_Read_Write_Slave(uint8_t slvaddr)
{
    uint32_t i;

    g_u8DeviceAddr = slvaddr;

    for(i = 0; i < 0x100; i++)
    {
        g_au8MstTxData[0] = (uint8_t)((i & 0xFF00) >> 8);
        g_au8MstTxData[1] = (uint8_t)(i & 0x00FF);
        g_au8MstTxData[2] = (uint8_t)(g_au8MstTxData[1] + 3);

        g_u8MstDataLen = 0;
        g_u8MstEndFlag = 0;

        /* I2C5 function to write data to slave */
        s_I2C5HandlerFn = (I2C_FUNC)I2C_MasterTx;

        /* I2C5 as master sends START signal */
        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_STA);

        /* Wait I2C5 Tx Finish */
        while(g_u8MstEndFlag == 0);
        g_u8MstEndFlag = 0;

        /* I2C5 function to read data from slave */
        s_I2C5HandlerFn = (I2C_FUNC)I2C_MasterRx;

        g_u8MstDataLen = 0;
        g_u8DeviceAddr = slvaddr;

        I2C_SET_CONTROL_REG(I2C5, I2C_CTL_STA);

        /* Wait I2C5 Rx Finish */
        while(g_u8MstEndFlag == 0);

        /* Compare data */
        if(g_u8MstRxData != g_au8MstTxData[2])
        {
            sysprintf("I2C5 Byte Write/Read Failed, Data 0x%x\n", g_u8MstRxData);
            return -1;
        }
    }
    sysprintf("Master Access Slave (0x%X) Test OK\n", slvaddr);
    return 0;
}


int32_t main(void)
{
    uint32_t i;

    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();

    /* Configure UART: 115200, 8-bit word, no parity bit, 1 stop bit. */
    UART_Open(UART0, 115200);

    /*
        This sample code sets I2C bus clock to 100kHz. Then, Master accesses Slave with Byte Write
        and Byte Read operations, and check if the read data is equal to the programmed data.
    */

    sysprintf("+-------------------------------------------------------+\n");
    sysprintf("| I2C Driver Sample Code for loopback test              |\n");
    sysprintf("|                                                       |\n");
    sysprintf("| I2C Master (I2C5) <---> I2C Slave(I2C4)               |\n");
    sysprintf("+-------------------------------------------------------+\n");

    sysprintf("\n");
    sysprintf("Configure I2C5 as Master, and I2C4 as a slave.\n");
    sysprintf("The I/O connection I2C5 to I2C4:\n");
    sysprintf("I2C5_SDA, I2C5_SCL\n");
    sysprintf("I2C4_SDA, I2C4_SCL\n\n");

    /* Init I2C5 */
    I2C5_Init();

    /* Init I2C4 */
    I2C4_Init();

    /* I2C4 enter non address SLV mode */
    I2C_SET_CONTROL_REG(I2C4, I2C_CTL_SI_AA);

    for(i = 0; i < 0x100; i++)
    {
        g_au8SlvData[i] = 0;
    }

    /* I2C4 function to Slave receive/transmit data */
    s_I2C4HandlerFn = I2C_SlaveTRx;

    sysprintf("\n");
    sysprintf("I2C4 Slave Mode is Running.\n");

    /* Access Slave with no address */
    sysprintf("\n");
    sysprintf(" == No Mask Address ==\n");
    I2C5_Read_Write_Slave(0x16);
    I2C5_Read_Write_Slave(0x36);
    I2C5_Read_Write_Slave(0x56);
    I2C5_Read_Write_Slave(0x76);
    sysprintf("Slave Address test OK.\n");

    /* Access Slave with address mask */
    sysprintf("\n");
    sysprintf(" == Mask Address ==\n");
    I2C5_Read_Write_Slave(0x16 & ~0x04);
    I2C5_Read_Write_Slave(0x36 & ~0x02);
    I2C5_Read_Write_Slave(0x56 & ~0x04);
    I2C5_Read_Write_Slave(0x76 & ~0x02);
    sysprintf("Slave Address Mask test OK.\n");

    s_I2C5HandlerFn = NULL;
    s_I2C4HandlerFn = NULL;

    /* Close I2C4,5 */
    I2C5_Close();
    I2C4_Close();

    while(1);

}



