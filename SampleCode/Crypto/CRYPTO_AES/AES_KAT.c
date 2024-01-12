/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define TEXT_MAX_LEN        256

extern uint8_t *au8InputData;
extern uint8_t *au8OutputData;
extern uint8_t *au8CascadeOut;
extern uint8_t *au8FDBCK;

extern void  dump_buff_hex(uint8_t *pucBuff, int nBytes);

typedef struct kat_t
{
	int			is_sm4;
	uint32_t    opmod;
	uint32_t	keysz;
	int         keylen;
	char        key[128];
	char        iv[128];
	char        plainT[TEXT_MAX_LEN];
	char        cipherT[TEXT_MAX_LEN];
} KAT_T;

#include "test_vectors.c"

uint8_t  *pPlainText, *pCipherText, *pOutput;

static uint8_t  g_hex[256];

uint8_t  char_to_hex(uint8_t c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	return 0;
}

int  str2hex(char *str, uint8_t *hex)
{
	int  val8, count = 0;

	while (*str)
	{
		val8 = char_to_hex(*str);
		str++;
		val8 = (val8 << 4) | char_to_hex(*str);
		str++;

		hex[count] = val8;
		count++;
	}
	return count;
}

void  word_swap_hex(uint8_t *hex, int len)
{
	int      i;
	uint8_t  val8;

	len = (len + 3) & 0xfffffffc;

	for (i = 0; i < len; i+=4)
	{
		val8 = hex[i];
		hex[i] = hex[i+3];
		hex[i+3] = val8;

		val8 = hex[i+1];
		hex[i+1] = hex[i+2];
		hex[i+2] = val8;
	}
}

void  swap_registers(uint32_t * reg_base, int reg_cnt)
{
	int       i;
	uint32_t  *reg = reg_base;
	uint32_t  data32;

	for (i = 0; i < reg_cnt; i++, reg++)
	{
		data32 = ((*reg >> 24) & 0xff) | ((*reg >> 8) & 0xff00) |
				 ((*reg << 8) & 0xff0000) | ((*reg << 24) & 0xff000000);
		*reg = data32;
	}
}

int  read_test_vector(KAT_T *t)
{
	int i, count;
	uint32_t val32, keys[8];

	/*
	 *  read AES key
	 */
	if (str2hex(t->key, g_hex) != t->keylen)
		return -1;

	for (i = 0; i < t->keylen; i+=4)
		keys[i / 4] = (g_hex[i]<<24) | (g_hex[i+1]<<16) | (g_hex[i+2]<<8) | g_hex[i+3];

	AES_SetKey(CRPT, keys, AES_KEY_SIZE_256);

	/*
	 *  read AES initial vector
	 */
	if (str2hex(t->iv, g_hex) != 16)
		return -1;

	for (i = 0; i < 16; i+=4)
		keys[i / 4] = (g_hex[i]<<24) | (g_hex[i+1]<<16) | (g_hex[i+2]<<8) | g_hex[i+3];

	AES_SetInitVect(CRPT, keys);

	/*
	 *  read plain text
	 */
	count = str2hex(t->plainT, pPlainText);

	/*
	 *  read cipher text
	 */
	str2hex(t->cipherT, pCipherText);

	return count;
}

/*----------------------------------------------------------------------------------------------*/
/*  KAT test                                                                                    */
/*----------------------------------------------------------------------------------------------*/
int  AES_KAT_test()
{
	int       i, j, len;

	pPlainText = au8InputData;
	pCipherText = au8OutputData;
	pOutput = au8CascadeOut;

	for (i = 0; i < sizeof(g_test_vector)/sizeof(KAT_T); i++)
	{
		sysprintf("KAT vecotr %d...\n", i);

		len = read_test_vector((KAT_T *)&g_test_vector[i]);
		if (len < 0)
		{
			sysprintf("Failed to read test vector!\n");
			return -1;
		}

		/*-------------------------------------------------
		 *   Encode test
		 *-------------------------------------------------*/
		memset(pOutput, 0, len);

		AES_Open(CRPT, AES_MODE_ENCRYPT, g_test_vector[i].opmod, g_test_vector[i].keysz, AES_IN_OUT_SWAP);

		AES_SetDMATransfer(CRPT, 0, ptr_to_u32(au8FDBCK), ptr_to_u32(pPlainText),
						   ptr_to_u32(pOutput), len);

		if (AES_Start(CRPT, g_test_vector[i].is_sm4, CRYPTO_DMA_ONE_SHOT) != 0)
		{
			sysprintf("AES encrypt/decrypt time-out!!\n");
			break;
		}
		
		if (memcmp(pCipherText, pOutput, len) != 0)
		{
			sysprintf("Dump pOutput ==>\n");
			dump_buff_hex((uint8_t *)pOutput, 16);
			sysprintf("Dump pCipherText ==>\n");
			dump_buff_hex((uint8_t *)pCipherText, 16);
			sysprintf("AES test vector encode failed at =>\n");
			sysprintf("KEY        = %s\n", g_test_vector[i].key);
			sysprintf("IV         = %s\n", g_test_vector[i].iv);
			sysprintf("PLAINTEXT  = %s\n", g_test_vector[i].plainT);
			sysprintf("CIPHERTEXT = %s\n", g_test_vector[i].cipherT);

			sysprintf("Encode output:\n");
			dump_buff_hex(pOutput, len);
			return -1;
		}
		sysprintf("[OK]\n");


		/*-------------------------------------------------
		 *   Decode test
		 *-------------------------------------------------*/
		read_test_vector((KAT_T *)&g_test_vector[i]);

		AES_Open(CRPT, AES_MODE_DECRYPT, g_test_vector[i].opmod, g_test_vector[i].keysz, AES_IN_OUT_SWAP);

		AES_SetDMATransfer(CRPT, 0, ptr_to_u32(au8FDBCK), ptr_to_u32(pCipherText),
						   ptr_to_u32(pOutput), len);

		memset(pOutput, 0,  len);

		if (AES_Start(CRPT, g_test_vector[i].is_sm4, CRYPTO_DMA_ONE_SHOT) != 0)
		{
			sysprintf("AES encrypt/decrypt time-out!!\n");
			break;
		}

		if (memcmp(pPlainText, pOutput, len) != 0)
		{
			sysprintf("AES test vector decode failed at =>\n");
			sysprintf("KEY        = %s\n", g_test_vector[i].key);
			sysprintf("IV         = %s\n", g_test_vector[i].iv);
			sysprintf("PLAINTEXT  = %s\n", g_test_vector[i].plainT);
			sysprintf("CIPHERTEXT = %s\n", g_test_vector[i].cipherT);

			sysprintf("Encode output:\n");
			dump_buff_hex(pOutput, len);
			return -1;
		}

		if (memcmp(au8FDBCK, (void *)CRPT->AES_FDBCK, 16) != 0)
		{
			dump_buff_hex(au8FDBCK, 16);
			sysprintf("0x%x 0x%x 0x%x 0x%x\n", CRPT->AES_FDBCK[0], CRPT->AES_FDBCK[1], CRPT->AES_FDBCK[2], CRPT->AES_FDBCK[3]);
			sysprintf("FDBCK check 1 failed!\n");
			return -1;
		}
		sysprintf("[OK]\n");
	}

	sysprintf("All test vector passed.\n");
	return 0;
}
