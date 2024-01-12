/**************************************************************************//**
 * @file     main.c
 * @brief    This sample program demonstrates how to make TRNG generate
 *           random numbers.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

uint32_t  user_nonce[48] = {
	0xc47b0294, 0xdbbbee0f, 0xec4757f2, 0x2ffeee35, 0x87ca4730, 0xc3d33b69, 0x1df38bab, 0x63ac0a6b,
	0xd38da3ab, 0x584a50ea, 0xb93f2603, 0x09a5c691, 0x09a5c691, 0x024f91ac, 0x6063ce20, 0x229160d9,
	0x49e00388, 0x1ab6b0cd, 0xe657cb40, 0x87c5aa81, 0xd611eab8, 0xa7ae6d1c, 0x3a181a28, 0x9391bbec,
	0x22186179, 0xb6476813, 0x67e64213, 0x47cc0c01, 0xf53bc314, 0x73545902, 0xd8a14864, 0xb31262d1,
	0x2bf77bc3, 0xd81c9e3a, 0xa0657c50, 0x51a2fe50, 0x91ff8818, 0x6de4dc00, 0xba468631, 0x7601971c,
	0xdec69b2f, 0x336e9662, 0xef73d94a, 0x618226a3, 0x3cdd3154, 0xf361b408, 0x55d394b4, 0xfc3d7775,
};

static volatile uint64_t  _start_time = 0;

void start_timer(void)
{
	_start_time = EL0_GetCurrentPhysicalValue();
}

void delay_us(int usec)
{
	uint64_t   t0;

	t0  = EL0_GetCurrentPhysicalValue();
	while ((EL0_GetCurrentPhysicalValue() - t0) < (usec * 12));
}

uint64_t get_time_us(void)
{
	uint64_t   t_off;

	t_off = EL0_GetCurrentPhysicalValue() - _start_time;
	t_off = t_off / 12;
	return t_off;
}

void SYS_Init(void)
{
	/* Enable LXT */
	CLK->PWRCTL |= CLK_PWRCTL_LXTEN_Msk; // 32K (LXT) Enabled

	/* Waiting LXT ready */
	CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);

	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as SYSCLK1 and UART module clock divider as 15 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_SYSCLK1_DIV2, CLK_CLKDIV1_UART0(15));

	/* Enable Crypto engine clock */
	outpw(TSI_CLK_BASE + 0x4, inpw(TSI_CLK_BASE + 0x4) | (1 << 12));

	/* Enable TRNG engine clock */
	outpw(TSI_CLK_BASE + 0xC, inpw(TSI_CLK_BASE + 0xC) | (1 << 25));

	/* Set GPE multi-function pins for UART0 RXD and TXD */
	SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
	SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);
}

void print_stat()
{
	sysprintf("\n[TRNG->STAT] ==>\n");
	sysprintf("    SEL_ALG is AES-%s\n", (TRNG->STAT & TRNG_STAT_SEC_ALG_Msk) ? "256" : "128");
	sysprintf("    Nonce mode: %s\n", (TRNG->STAT & TRNG_STAT_NONCE_MODE_Msk) ? "enabled" : "disabled");
	sysprintf("    Core is in: %s mode\n", (TRNG->STAT & TRNG_STAT_MISSION_MODE_Msk) ? "MISSION" : "TEST");
	if (((TRNG->STAT & TRNG_STAT_DRBG_STATE_Msk)>>TRNG_STAT_DRBG_STATE_Pos) == 0x0)
		sysprintf("    DRBG state in not instantiated\n");
	else if (((TRNG->STAT & TRNG_STAT_DRBG_STATE_Msk)>>TRNG_STAT_DRBG_STATE_Pos) == 0x1)
		sysprintf("    DRBG state in instantiated using the built-in noise source\n");
	else
		sysprintf("    DRBG state in instantiated using the host provided nonce\n");
	if (TRNG->STAT & TRNG_STAT_STARTUP_TEST_STUCK_Msk)
		sysprintf("    Startup test stucked\n");
	if (TRNG->STAT & TRNG_STAT_STARTUP_TEST_IN_PROG_Msk)
		sysprintf("    Startup test is in progress\n");
	if (TRNG->STAT & TRNG_STAT_BUSY_Msk)
		sysprintf("    Core state is BUSY.\n");
	else
		sysprintf("    Core state is IDLE.\n");
}

int32_t main(void)
{
	uint32_t  rnd_buff[64];
	int i, item, ret;

	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init();

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);

	start_timer();
	if (TRNG_Init() != 0)
	{
		sysprintf("TRNG init failed!!\n");
		while (1);
	}

	sysprintf("TRNG init done, time us = %d\n", get_time_us());

	sysprintf("\n\n");
	sysprintf("+---------------------------------------------------------------+\n");
	sysprintf("|  Select TRNG seeding method                                   |\n");
	sysprintf("+---------------------------------------------------------------+\n");
	sysprintf("| [1] Self-seeding (Seed is from TRNG noise)                    |\n");
	sysprintf("| [2] Nonce seeding.                                            |\n");
	sysprintf("| [3] User seed                                                 |\n");
	sysprintf("+---------------------------------------------------------------+\n");

	while (1)
	{
		item = sysgetchar();
		if ((item == '1') || (item == '2') || (item == '3'))
			break;
	}

	if (item == '1')
		ret = TRNG_GenNoise();
	else if (item == '2')
		ret = TRNG_GenNonce(user_nonce);
	else
		ret = TRNG_SetSeed(user_nonce);

	if (ret != 0)
	{
		sysprintf("TRNG seeding failed! (%d)\n", ret);
		while (1);
	}

	sysprintf("TRNG_CreateState...\n");
	if (TRNG_CreateState() != 0)
	{
		sysprintf("TRNG_CreateState failed!!\n");
		while (1);
	}

	sysprintf("TRNG sedding done. Ready for generating random number.\n");
	print_stat();

	while (1)
	{
		sysprintf("\nGenerate random numbers ==>\n");

		if (TRNG_GenerateRandomNumber(rnd_buff, 8) != 0)
		{
			sysprintf("TRNG_GenerateRandomNumber failed!\n\n");
		}
		else
		{
			for (i = 0; i < 8; i++)
				sysprintf("0x%x ", rnd_buff[i]);
			sysprintf("\n");
		}

		delay_us(1000000);
		if (sysIsKbHit())
		{
			sysgetchar();
			sysprintf("\nPress any key to continue...\n");
			sysgetchar();
		}
	}
}
