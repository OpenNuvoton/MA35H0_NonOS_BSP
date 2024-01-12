/**************************************************************************//**
 * @file     main.c
 * @brief    This sample program demonstrates Key Store SRAM operations.
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define KS_KEY_MAX      (KS_MAX_KEY_SIZE / 32)
#define KEY_SEED        0x1A39175D

struct ks_keys {
	uint32_t   key[KS_KEY_MAX];
	int        size;
	int        number;
	uint32_t   meta;
};

struct ks_keys  _my_keys[KS_SRAM_KEY_CNT];

int   key_size_list[] = { 128, 128, 128, 256, 256, 128, 163, 192, 224, 233, 255,
						  256, 283, 384, 409, 512, 521, 571, 1024, 2048, 4096 };
int   key_size_meta[] = { KS_META_128, KS_META_128, KS_META_128, KS_META_256, KS_META_256,
						  KS_META_128, KS_META_163, KS_META_192, KS_META_224, KS_META_233,
						  KS_META_255, KS_META_256, KS_META_283, KS_META_384, KS_META_409,
						  KS_META_512, KS_META_521, KS_META_571, KS_META_1024,KS_META_2048,
						  KS_META_4096 };

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

static void ClearStatus(void)
{
	KS->STS = KS_STS_IF_Msk | KS_STS_EIF_Msk;
}

static void ShowStatus(void)
{
	uint32_t bcnt, kcnt;
	uint32_t u32Status;

	u32Status = KS->STS;

	sysprintf("Key Store Status:");

	if (u32Status & KS_STS_IF_Msk)
		sysprintf(" STS_IF!");

	if (u32Status & KS_STS_EIF_Msk)
		sysprintf(" STS_EIF!");

	if (u32Status & KS_STS_BUSY_Msk)
		sysprintf(" STS_BUSY!");

	if (u32Status & KS_STS_SRAMFULL_Msk)
		sysprintf(" STS_SRAMFULL!");

	if ((u32Status & KS_STS_INITDONE_Msk) != KS_STS_INITDONE_Msk)
		sysprintf(" STS_NotInit!");

	sysprintf("\n");

	if (KS_GetSRAMRemain(&bcnt, &kcnt) == 0)
	{
		sysprintf("Key Store SRAM remaining space byte count: %d\n", bcnt);
		sysprintf("Key Store SRAM remaining key count: %d\n", kcnt);
	}
	sysprintf("\n");
}

/*
 *  Initialize test keys test database
 */
static void test_vector_init(void)
{
	int         i, j, k, klist_size;
	uint32_t    k32 = KEY_SEED;

	sysprintf("Initialize test vectors...");
	klist_size = sizeof(key_size_list)/4;
	k = 0;
	for (i = 0; i < KS_SRAM_KEY_CNT; i++)
	{
		for (j = 0; j < 8; j++)
		{
			_my_keys[i].key[j] = k32;
			k32 += KEY_SEED;
		}
		_my_keys[i].number = -1;    /* not used */
		_my_keys[i].size = key_size_list[k];
		_my_keys[i].meta = key_size_meta[k];
		k = (k + 1) % klist_size;
	}
	sysprintf("Done.\n");
}

static int ks_key_compare(uint32_t au32KeyExpect[], uint32_t au32KeyData[], int key_size)
{
	int  i, j, bits;

	for (i = 0; i < key_size/32; i++)
	{
		if (au32KeyExpect[i] != au32KeyData[i])
		{
			sysprintf("\nKey compare mismatch at word %d\n", i);
			goto mismatch;
		}
	}
	if (key_size % 32)
	{
		i = (key_size / 32);
		bits = key_size % 32;
		for (j = 0; j < bits; j++)
		{
			if ((au32KeyExpect[i] & (1<<j)) != (au32KeyData[i] & (1<<j)))
			{
				sysprintf("\nKey compare mismatch at bit %d\n", i * 32 + j);
				goto mismatch;
			}
		}
	}
	return 0;

mismatch:
	sysprintf("Key Expect: ");
	for (i = 0; i < (key_size + 31) / 32; i++)
		sysprintf("%08x ", au32KeyExpect[i]);
	sysprintf("\n");
	sysprintf("Key Data: ");
	for (i = 0; i < (key_size + 31) / 32; i++)
		sysprintf("%08x ", au32KeyData[i]);
	return -1;
}

static int test_vector_verify_sram_keys(void)
{
	uint32_t    au32RKey[KS_KEY_MAX];
	int         i, err;

	sysprintf("Verify SRAM Keys...\n");
	for (i = 0; i < KS_SRAM_KEY_CNT; i++)
	{
		sysprintf("<%d> Read SRAM key %d size %d....", _my_keys[i].number, _my_keys[i].size);

		memset(au32RKey, 0, sizeof(au32RKey));

		ClearStatus();
		err = KS_Read(KS_SRAM, _my_keys[i].number, au32RKey, (_my_keys[i].size+31)/32);
		if (err < 0)
		{
			sysprintf("[FAIL]\nRead SRAM key fail! %d\n", err);
			ShowStatus();
			return -1;
		}

		if (ks_key_compare(_my_keys[i].key, au32RKey, _my_keys[i].size) != 0)
		{
			sysprintf("SRAM Key %d read verify failed!\n", i);
			return -1;
		}
		sysprintf("0x%x 0x%x [PASS]\n", au32RKey[0], au32RKey[1]);
	}
	sysprintf("Verify PASS.\n");
	return 0;
}

int SRAM_Key_ReadWriteTest(void)
{
	uint32_t    au32RKey[KS_KEY_MAX];
	int         i32KeyIdx;
	int         i, j, err;

	sysprintf("\n\n\n");
	sysprintf("+---------------------------------------------------+\n");
	sysprintf("|  Key Store SRAM key write and read verify test    |\n");
	sysprintf("+---------------------------------------------------+\n");

	KS_EraseAll();

	sysprintf(">>> Write keys to Key Store SRAM...\n");

	for (i = 0; i < KS_SRAM_KEY_CNT; i++)
	{
		ClearStatus();
		_my_keys[i].meta |= KS_META_CPU | KS_META_READABLE;
		sysprintf("Write key size %d, meta=0x%x....", _my_keys[i].size, _my_keys[i].meta);

		i32KeyIdx = KS_Write(KS_SRAM, _my_keys[i].meta, _my_keys[i].key);
		if (i32KeyIdx < 0)
		{
			sysprintf("KS_Write failed!\n");
			return -1;
		}
		else
		{
			_my_keys[i].number = i32KeyIdx;
			sysprintf("[PASS] number=%d\n", _my_keys[i].number);
		}
	}
	sysprintf("OK.\n");

	return test_vector_verify_sram_keys();
}

int SRAM_Key_RevokeTest(void)
{
	uint32_t *pu32;
	uint32_t au32Key[8], au32RKey[8], au32ZeroKey[8];
	int i32KeyIdx;
	int err;
	int i, test_loop;

	pu32 = (uint32_t *)KS_BASE;

	au32Key[0] = 0x123456f0;
	au32Key[1] = 0x223456f1;
	au32Key[2] = 0x323456f2;
	au32Key[3] = 0x423456f3;
	au32Key[4] = 0x523456f4;
	au32Key[5] = 0x623456f5;
	au32Key[6] = 0x723456f6;
	au32Key[7] = 0x823456f7;

	memset(au32ZeroKey, 0, 4 * 8);

	err = KS_EraseAll();
	if (err)
	{
		sysprintf("Fail to erase whole KS SRAM\n");
		goto lexit;
	}

	for (test_loop = 0; test_loop < KS_SRAM_KEY_CNT; test_loop++)
	{
		//--------------------------------------------------------------------
		sysprintf("Write key to KS_SRAM %d ... ", test_loop);
		i32KeyIdx = KS_Write(KS_SRAM, KS_META_256 | KS_META_CPU | KS_META_READABLE, au32Key);
		if(i32KeyIdx < 0)
		{
			sysprintf("[FAIL]\nWrite key fail\n");
			err = -1;
			goto lexit;
		}
		else
		{
			sysprintf("Key index = 0x%08x\n", i32KeyIdx);
		}

		memset(au32RKey, 0, 4*8);
		err = KS_Read(KS_SRAM, i32KeyIdx, au32RKey, 4);
		if (err < 0)
		{
			sysprintf("[FAIL]\nRead key fail!\n");
			goto lexit;
		}
		sysprintf("[PASS]\n");

		sysprintf("Key=0x%08x %08x %08x %08x\n", au32RKey[0], au32RKey[1], au32RKey[2], au32RKey[3]);

		sysprintf("Revoke key test ...");
		// Revoke the key
		err = KS_RevokeKey(KS_SRAM, i32KeyIdx);
		if (err)
		{
			sysprintf("[FAIL]\nFail to revoke key\n");
			goto lexit;
		}

		memset(au32RKey, 0, 4 * 8);
		err = KS_Read(KS_SRAM, i32KeyIdx, au32RKey, 4);
		if (err == 0)
		{
			err = -1;
			sysprintf("[FAIL]\n The key seems not be revoked.\n");
			sysprintf("Key=0x%08x %08x %08x %08x\n", au32RKey[0], au32RKey[1], au32RKey[2], au32RKey[3]);
			goto lexit;
		}
		sysprintf("[PASS]\n");
	}
	err = 0;

lexit:
	sysprintf("\n");
	if (err)
	{
		sysprintf("Test FAILED\n");
		return -1;
	}
	else
	{
		sysprintf("Test PASSED!\n");
		return 0;
	}
	return 0;
}

int SRAM_Key_ScrabmleTest(void)
{
	uint32_t    au32RKey[KS_KEY_MAX];
	int         i, raminv_state, err = 0;

	KS_ENABLE_SCRAMBLE();

	err = SRAM_Key_ReadWriteTest();
	if (err)
		goto lexit;

	KS_DISABLE_SCRAMBLE();

	sysprintf("[Read all SRAM key with scramble disabled]\n");
	/*
	 *  Read all SRAM keys without scrable. They must be different from original keys.
	 */
	for (i = 0; i < KS_SRAM_KEY_CNT; i++)
	{
		sysprintf("Read SRAM key %d...\n", i);

		memset(au32RKey, 0, sizeof(au32RKey));
		ClearStatus();
		err = KS_Read(KS_SRAM, _my_keys[i].number, au32RKey, (_my_keys[i].size+31)/32);
		if (err < 0)
		{
			if (_my_keys[i].size % 128)
				continue;           /* It's OK. design issue */

			sysprintf("[FAIL]\nRead SRAM key fail!\n");
			ShowStatus();
			goto lexit;
		}

		if ((_my_keys[i].key[0] == au32RKey[0]) || (_my_keys[i].key[1] == au32RKey[1]) ||
			(_my_keys[i].key[2] == au32RKey[2]) || (_my_keys[i].key[3] == au32RKey[3]))
		{
			sysprintf("[FAIL] Key may not be scrambled!\n");
			err = -1;
			goto lexit;
		}
	}
	sysprintf("[PASS]\n");

	sysprintf("\n\n[Read test with scramble enable]\n");

	KS_ENABLE_SCRAMBLE();

	sysprintf("[Read SRAM keys with scramble enabled]\n");
	err = test_vector_verify_sram_keys();
	if (err != 0)
	{
		sysprintf("[FAIL] Verfiy SRAM keys failed!\n");
		goto lexit;
	}

lexit:
	sysprintf("\n");
	if (err)
	{
		sysprintf("Test FAILED\n");
	}
	else
	{
		sysprintf("Test PASSED\n");
	}
	return err;
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

	/* Enable Key Store engine clock */
	outpw(TSI_CLK_BASE + 0x4, inpw(TSI_CLK_BASE + 0x4) | (1 << 14));

	/* Set GPE multi-function pins for UART0 RXD and TXD */
	SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE14MFP_Msk | SYS_GPE_MFPH_PE15MFP_Msk);
	SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE14MFP_UART0_TXD | SYS_GPE_MFPH_PE15MFP_UART0_RXD);
}

int32_t main(void)
{
	int i, item, ret;

	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init();

	/* Init UART to 115200-8n1 for print message */
	UART_Open(UART0, 115200);

	test_vector_init();

	while (1)
	{
		sysprintf("\n");
		sysprintf("+---------------------------------------------+\n");
		sysprintf("|  Key Store Test                             |\n");
		sysprintf("+---------------------------------------------+\n");
		sysprintf("| [1] Key Store SRAM status                   |\n");
		sysprintf("| [2] SRAM key read/write test                |\n");
		sysprintf("| [3] SRAM Key revoke test                    |\n");
		sysprintf("| [4] SRAM Key Scramble test                  |\n");
		sysprintf("| [5] Erase all SRAM keys                     |\n");
		sysprintf("+---------------------------------------------+\n");

		item = sysgetchar();

		switch (item)
		{
			case '1':
				ShowStatus();
				ret = 0;
				break;

			case '2':
				ret = SRAM_Key_ReadWriteTest();
				break;

			case '3':
				ret = SRAM_Key_RevokeTest();
				break;

			case '4':
				ret = SRAM_Key_ScrabmleTest();
				break;

			case '5':
				ret = KS_EraseAll();
				break;

			default:
				ret = 0;
				break;
		}

		if (ret == 0)
			sysprintf("\n\nTest passed.\n");
		else
			sysprintf("\n\nTest failed!\n");

		sysprintf("Press any key to continue...\n");
		sysgetchar();
	}
}
