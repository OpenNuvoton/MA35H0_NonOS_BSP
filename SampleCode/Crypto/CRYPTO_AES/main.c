/**************************************************************************//**
 * @file     main.c
 * @brief    This sample program demonstrates the Crypto AES functions
 *           by encrypt/decrypt a set of KAT (Known Answer Test) pattern.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define BUFF_SIZE       0x10000

uint32_t au32MyAESKey[8] = {
	0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
	0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f
};

uint32_t au32MyAESIV[4] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

uint8_t au8InputData_Pool[BUFF_SIZE] __attribute__((aligned(32))) = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
	0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
} ;

uint8_t au8OutputData_Pool[BUFF_SIZE] __attribute__((aligned(32)));
uint8_t au8CascadeOut_Pool[BUFF_SIZE] __attribute__((aligned(32)));
uint8_t au8FDBCK_Pool[64] __attribute__((aligned(32)));

uint8_t  *au8InputData;
uint8_t  *au8OutputData;
uint8_t  *au8CascadeOut;
uint8_t  *au8FDBCK;

extern int  AES_KAT_test(void);

void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
	uint64_t  addr, end_addr;
	int       nIdx, i;

	addr = (uint64_t)pucBuff;
	end_addr = addr + nBytes - 1;

	if ((addr % 16) != 0)
	{
		sysprintf("0x%04x_%04x  ", (addr>>16)&0xffff, addr & 0xffff);
		for (i = 0; i < addr % 16; i++)
			sysprintf(".. ");

		for ( ; (addr % 16) != 0; addr++)
			sysprintf("%02x ", readb((uint8_t *)addr) & 0xff);
		sysprintf("\n");
	}

	for ( ; addr <= end_addr; )
	{
		sysprintf("0x%04x_%04x  ", (addr>>16)&0xffff, addr & 0xffff);
		for (i = 0; i < 16; i++, addr++)
		{
			if (addr > end_addr)
				break;
			sysprintf("%02x ", readb((uint8_t *)addr) & 0xff);
		}
		sysprintf("\n");
	}
	sysprintf("\n");
}

void SYS_Init(void)
{
	/* Waiting LXT ready */
	CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);

	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as SYSCLK1 and UART module clock divider as 15 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_SYSCLK1_DIV2, CLK_CLKDIV1_UART0(15));

	/* Enable Crypto engine clock */
	outpw(TSI_CLK_BASE + 0x4, inpw(TSI_CLK_BASE + 0x4) | (1 << 12));

	/* Set GPE multi-function pins for UART0 RXD and TXD */
	SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
	SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);
}

int32_t main(void)
{
	uint32_t rand_num[8];
	int i, j, item;

	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init();

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);

	au8InputData = nc_ptr(au8InputData_Pool);
	au8OutputData = nc_ptr(au8OutputData_Pool);
	au8CascadeOut = nc_ptr(au8CascadeOut_Pool);
	au8FDBCK = nc_ptr(au8FDBCK_Pool);

	Crypto_Init();

	AES_KAT_test();

	while (1);
}
