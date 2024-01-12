/**************************************************************************//**
 * @file     main.c
 * @brief    This sample program demonstrates the Crypto PRNG functions.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define TEST_SEED   0x76223

void  dump_prng_key_buff(uint32_t *buff)
{
	int   i;

	sysprintf("DATA ==>\n");
	for (i = 0; i < 8; i++)
		sysprintf("  0x%08x", buff[i]);
	sysprintf("\n");
}

void  prng_gen_and_dump(int keysz)
{
	uint32_t buff[8];
	int  i;

	memset(buff, 0, sizeof(buff));

	PRNG_Config(CRPT, keysz, 0, 0);
	PRNG_Start(CRPT);
	PRNG_Read(CRPT, buff);
	dump_prng_key_buff(buff);
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
	uint32_t rand_num[8], buff[8];
	int i, j, item;

	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init();

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);

	Crypto_Init();

	PRNG_Config(CRPT, PRNG_KEY_SIZE_256, PRNG_SEED_RELOAD, TEST_SEED);

	while (1)
	{
		sysprintf("\n");
		sysprintf("+---------------------------------------------------------------+\n");
		sysprintf("|  CRYPTO PRNG demo                                             |\n");
		sysprintf("+---------------------------------------------------------------+\n");
		sysprintf("| [1] Generate 128/163/192/224/233/255/256 bits random runber   |\n");
		sysprintf("| [2] Inifinite loop redundancy check                           |\n");
		sysprintf("| [3] Seed differential test                                    |\n");
		sysprintf("+---------------------------------------------------------------+\n");

		item = sysgetchar();
		switch (item)
		{
			case '1':
				sysprintf("\nPRNG Key size 128 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_128);
				sysprintf("\nPRNG Key size 163 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_163);
				sysprintf("\nPRNG Key size 192 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_192);
				sysprintf("\nPRNG Key size 224 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_224);
				sysprintf("\nPRNG Key size 233 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_233);
				sysprintf("\nPRNG Key size 255 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_255);
				sysprintf("\nPRNG Key size 256 ==>\n");
				prng_gen_and_dump(PRNG_KEY_SIZE_256);
				break;

			case '2':
				PRNG_Config(CRPT, PRNG_KEY_SIZE_256, PRNG_SEED_RELOAD, TEST_SEED);
				PRNG_Start(CRPT);
				PRNG_Read(CRPT, rand_num);

				for (i = 1; i < 1000; i++)
				{
					sysprintf("Cycle: %d\n", i);

					PRNG_Start(CRPT);
					PRNG_Read(CRPT, buff);

					for (j = 0; j < 8; j++)
					{
						if (buff[j] == rand_num[j])
						{
							sysprintf("Random number not changed!! Test failed!!\n");
							while (1);
						}
						rand_num[j] = buff[j];
					}
				}
				sysprintf("\nTest passed. Press any key...\n");
				sysgetchar();
				break;

			case '3':
				sysprintf("\nPress any key to do seed differential test and press 'x' to stop test...\n");
				PRNG_Config(CRPT, PRNG_KEY_SIZE_256, PRNG_SEED_RELOAD, TEST_SEED);
				PRNG_Start(CRPT);
				PRNG_Read(CRPT, rand_num);

				for (i = 1; i < 50; i++)
				{
					sysprintf("Test cycle: %d/50\n", i);
					item += TEST_SEED;
					PRNG_Config(CRPT, PRNG_KEY_SIZE_256, PRNG_SEED_RELOAD, item);
					PRNG_Start(CRPT);
					PRNG_Read(CRPT, buff);
					for (j = 0; j < 8; j++)
					{
						if (buff[j] == rand_num[j])
						{
							sysprintf("\nSeed differential test failed!\n");
							dump_prng_key_buff(buff);
							dump_prng_key_buff(rand_num);
							while (1);
						}
					}
					for (j = 0; j < 8; j++)
						rand_num[j] = buff[j];

					PRNG_Start(CRPT);
				}
				sysprintf("\nTest passed.\n");
				break;

			default:
				break;
		}
		sysprintf("\n\nPress any key...\n");
	}
}
