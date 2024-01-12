/**************************************************************************//**
 * @file     main.c
 * @brief    This sample program demonstrates the Crypto SHA functions
 *           by perform SHA operation on a set of known answer test patterns
 *           and compare to verify if the result is correct or not.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define BUFF_SIZE      0x10000

uint8_t    g_buff_pool[BUFF_SIZE] __attribute__((aligned(32)));
uint32_t   g_fdbk_buff[88] __attribute__((aligned(32)));
uint8_t    *g_sha_msg;
uint8_t    g_sha_digest[1024];
int        g_msg_bit_len;
int        g_digest_len = 32;
int        g_sha_mode;

#include "SHA_vector_parser_image.c"

void do_swap(uint8_t *buff, int len)
{
	int       i;
	uint8_t   val8;

	len = (len+3) & 0xfffffffc;
	for (i = 0; i < len; i+=4)
	{
		val8 = buff[i];
		buff[i] = buff[i+3];
		buff[i+3] = val8;
		val8 = buff[i+1];
		buff[i+1] = buff[i+2];
		buff[i+2] = val8;
	}
}

int  do_compare(uint32_t *output, uint32_t *expect, int cmp_len)
{
	int   i;

	for (i = 0; i < cmp_len; i++)
	{
		if (output[i] != expect[i])
		{
			sysprintf("\nMismatch!! - %d\n", cmp_len);
			for (i = 0; i < cmp_len; i++)
				sysprintf("expect: 0x%0x    SHA digest: 0x%0x\n", expect[i], output[i]);

			return -1;
		}
	}
	return 0;
}

int  SHA_test()
{
	uint32_t answer_digest[16];

	if ((g_msg_bit_len < 8) || (g_msg_bit_len % 8) != 0)
	{
		sysprintf("Key length is %d, not multiple of 8!\n", g_msg_bit_len);
		return 0;
	}

	sysprintf("data len = %d\n", g_msg_bit_len / 8);

	if (g_digest_len == 28)
	{
		g_sha_mode = SHA_MODE_SHA224;
	}
	else if (g_digest_len == 32)
	{
		g_sha_mode = SHA_MODE_SHA256;
	}
	else if (g_digest_len == 48)
	{
		g_sha_mode = SHA_MODE_SHA384;
	}
	else if (g_digest_len == 64)
	{
		g_sha_mode = SHA_MODE_SHA512;
	}
	else if (g_digest_len == 20)
	{
		g_sha_mode = SHA_MODE_SHA1;
	}
	else
	{
		sysprintf("g_digest_len = %d is wrong!!\n", g_digest_len);
		return -1;
	}

	sysprintf("SHA DMA mode test with INSWAP & OUTSWAP...\n");

	SHA_Open(CRPT, g_sha_mode, SHA_IN_OUT_SWAP, 0);

	SHA_SetDMATransfer(CRPT, ptr_to_u32(g_sha_msg), g_msg_bit_len / 8);

	sysprintf("SHA byte count = %d\n", g_msg_bit_len / 8);

	if (SHA_Start(CRPT, CRYPTO_DMA_ONE_SHOT) != 0)
	{
		sysprintf("SHA time-out error!\n");
		return -1;
	}

	SHA_Read(CRPT, answer_digest);

	do_swap((uint8_t *)&g_sha_digest[0], g_digest_len);

	if (do_compare(answer_digest, (uint32_t *)g_sha_digest, g_digest_len / 4) < 0)
		while (1);

	do_swap((uint8_t *)&g_sha_digest[0], g_digest_len);

	/*--------------------------------------------*/
	/*  INSWAP test                               */
	/*--------------------------------------------*/
	sysprintf("SHA DMA mode test with INSWAP...\n");

	SHA_Open(CRPT, g_sha_mode, SHA_IN_SWAP, 0);

	SHA_SetDMATransfer(CRPT, ptr_to_u32(g_sha_msg), g_msg_bit_len / 8);

	sysprintf("SHA byte count = %d\n", g_msg_bit_len/8);

	if (SHA_Start(CRPT, CRYPTO_DMA_ONE_SHOT) != 0)
	{
		sysprintf("SHA time-out error!\n");
		return -1;
	}

	SHA_Read(CRPT, answer_digest);

	if (do_compare(answer_digest, (uint32_t *)g_sha_digest, g_digest_len/4) < 0)
	{
		while (1);
	}

	/*--------------------------------------------*/
	/*  No INSWAP & OUTSWAP test                  */
	/*--------------------------------------------*/
	sysprintf("SHA DMA mode test without SWAP...\n");

	do_swap(g_sha_msg, g_msg_bit_len/8);

	SHA_Open(CRPT, g_sha_mode, SHA_NO_SWAP, 0);

	SHA_SetDMATransfer(CRPT, ptr_to_u32(g_sha_msg), g_msg_bit_len / 8);

	sysprintf("SHA byte count = %d\n", g_msg_bit_len/8);

	if (SHA_Start(CRPT, CRYPTO_DMA_ONE_SHOT) != 0)
	{
		sysprintf("SHA time-out error!\n");
		return -1;
	}

	SHA_Read(CRPT, answer_digest);

	if (do_compare(answer_digest, (uint32_t *)g_sha_digest, g_digest_len/4) < 0)
	{
		while (1);
	}
	do_swap(g_sha_msg, g_msg_bit_len/8);

	return 0;
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
	int loop;

	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init();

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);

	Crypto_Init();

	g_sha_msg = nc_ptr(g_buff_pool);

	sysprintf("\n\n");
	sysprintf("+----------------------------------------+\n");
	sysprintf("|  SHA Known Answer Test                 |\n");
	sysprintf("+----------------------------------------+\n");

	open_test_file(NULL);

	for (loop = 1; ; loop++)
	{
		sysprintf("Run test vector %d...\n", loop);

		// memset(g_sha_msg, 0, sizeof(g_buff_pool));

		if (get_next_pattern() < 0)
		{
			sysprintf("No more test pattern.\n");
			break;
		}

		SHA_test();
	}

	close_test_file();

	sysprintf("\n\nTEST OK!\n");
	while (1);
}
