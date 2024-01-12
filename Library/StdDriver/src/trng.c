/**************************************************************************//**
 * @file     trng.c
 * @brief    TRNG driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright(C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include "NuMicro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup TRNG_Driver TRNG Driver
  @{
*/

/** @addtogroup TRNG_EXPORTED_CONSTANTS TRNG Exported Constants
  @{
*/

/// @cond HIDDEN_SYMBOLS

#define TRNG_DBGMSG         sysprintf
//#define TRNG_DBGMSG(...)

#define BUSY_TIMEOUT        12000000 /* 1 second */

static volatile uint32_t  irq_flag = 0;

void TRNG_IRQHandler(void)
{
	irq_flag = TRNG->ISTAT;

	if (irq_flag & TRNG_ISTAT_ZEROIZED_Msk)
	{
		TRNG_DBGMSG("[IRQ] ZEROIZED\n");
		TRNG->ISTAT = TRNG_ISTAT_ZEROIZED_Msk;
	}
	if (irq_flag & TRNG_ISTAT_KAT_COMPLETED_Msk)
	{
		TRNG_DBGMSG("[IRQ] COMPLETED\n");
		TRNG->ISTAT = TRNG_ISTAT_KAT_COMPLETED_Msk;
	}
	if (irq_flag & TRNG_ISTAT_NOISE_RDY_Msk)
	{
		TRNG_DBGMSG("[IRQ] NOISE_RDY\n");
		TRNG->ISTAT = TRNG_ISTAT_NOISE_RDY_Msk;
	}
	if (irq_flag & TRNG_ISTAT_ALARMS_Msk)
	{
		TRNG_DBGMSG("[IRQ] ALARMS. register ALARMS is 0x%x\n", TRNG->ALARMS);
		TRNG->ISTAT = TRNG_ISTAT_ALARMS_Msk;
	}
	if (irq_flag & TRNG_ISTAT_DONE_Msk)
	{
		// TRNG_DBGMSG("[IRQ] DONE\n");
		TRNG->ISTAT = TRNG_ISTAT_DONE_Msk;
	}
}

static int TRNG_Wait_Busy_Clear(void)
{
	uint64_t   t0 = EL0_GetCurrentPhysicalValue();

	// TRNG_DBGMSG("TRNG_Wait_Busy_Clear...");
	while (TRNG->STAT & TRNG_STAT_BUSY_Msk)
	{
		if (EL0_GetCurrentPhysicalValue() - t0 > BUSY_TIMEOUT)
			return -1;
	}
	// TRNG_DBGMSG("OK.\n");
	return 0;
}

static int TRNG_IssueCommand(int cmd)
{
	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	TRNG->CTRL = (TRNG->CTRL & ~TRNG_CTRL_CMD_Msk) | (cmd << TRNG_CTRL_CMD_Pos);
	return 0;
}

int TRNG_RenewState(void)
{
	irq_flag = 0;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	if (TRNG_IssueCommand(TCMD_RENEW_STATE) != 0)
		return -1;

	while (!(irq_flag & TRNG_ISTAT_DONE_Msk));
	return 0;
}

int TRNG_AdvanceState(void)
{
	irq_flag = 0;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	if (TRNG_IssueCommand(TCMD_ADVANCE_STATE) != 0)
		return -1;

	while (!(irq_flag & TRNG_ISTAT_DONE_Msk));
	return 0;
}

int TRNG_RefreshAddin(uint32_t *addin, int wcnt)
{
	if (!(TRNG->MODE & TRNG_MODE_ADDIN_PRESENT_Msk))
	{
		TRNG_DBGMSG("ADDIN not present!\n");
		return -1;
	}

	if (wcnt > 16)
		wcnt = 16;

	memcpy((void *)&TRNG->NPA_DATA[0], addin, wcnt*4);

	irq_flag = 0;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	if (TRNG_IssueCommand(TCMD_REFRESH_ADDIN) != 0)
		return -1;

	while (!(irq_flag & TRNG_ISTAT_DONE_Msk));
	return 0;
}

int  TRNG_KnownAnswerTest(void)
{
	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	irq_flag = 0;

	if (TRNG_IssueCommand(TCMD_RUN_KAT) != 0)
		return -1;

	while (irq_flag == 0);

	if (irq_flag & TRNG_ISTAT_KAT_COMPLETED_Msk)
	{
		TRNG_DBGMSG("KAT completed.\n");
		return 0;
	}
	TRNG_DBGMSG("No KAT complete interrupt! FAILED\n");
	return -1;
}

int TRNG_Zeroized(void)
{
	irq_flag = 0;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	if (TRNG_IssueCommand(TCMD_ZEROIZE) != 0)
		return -1;

	while (!(irq_flag & TRNG_ISTAT_ZEROIZED_Msk));
	return 0;
}
/// @endcond HIDDEN_SYMBOLS

/**
  * @brief    Initialize the TRNG engine, including reset TRNG engine, waiting
  *           for TRNG ready, and install TRNG interrupt handler.
  *
  * @retval   0     TRNG init success
  * @retval   < 0   Failed.
  */
int  TRNG_Init(void)
{
	int i;

	irq_flag = 0;

	/*
	 * Reset TRNG
	 */
	outpw(TSI_SYS_BASE + 0xC, inpw(TSI_SYS_BASE + 0xC) | 0x80000000);
	for (i = 0; i < 1000; i++)
		__NOP();
	outpw(TSI_SYS_BASE + 0xC, inpw(TSI_SYS_BASE + 0xC) & ~0x80000000);

	TRNG_DBGMSG("Wait startup busy state (under startup test) cleared....\n");
	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	TRNG_DBGMSG("Check START_UP_XXX...\n");
	while (TRNG->STAT & (TRNG_STAT_STARTUP_TEST_STUCK_Msk | TRNG_STAT_STARTUP_TEST_IN_PROG_Msk));

	TRNG_DBGMSG("COREKIT_REL = 0x%x\n", TRNG->COREKIT_REL);
	TRNG_DBGMSG("FEATURE = 0x%x\n", TRNG->FEATURES);

	SELECT_ALG_AES_256;

	/* enable all interrupts */
	TRNG->IE = TRNG_IE_ZEROIZED_Msk | TRNG_IE_KAT_COMPLETED_Msk | TRNG_IE_NOISE_RDY_Msk | TRNG_IE_ALARMS_Msk | TRNG_IE_DONE_Msk | TRNG_IE_GLBL_Msk;

	IRQ_SetHandler((IRQn_ID_t)TRNG_IRQn, TRNG_IRQHandler);
	IRQ_Enable((IRQn_ID_t)TRNG_IRQn);
	IRQ_SetTarget(TRNG_IRQn, IRQ_CPU_0);

	return 0;
}

/**
  * @brief    TRNG generate seeds by H/W collected noise.
  *
  * @retval   0     Success.
  * @retval   < 0   Failed.
  */
int TRNG_GenNoise(void)
{
	uint64_t   t0;

	TRNG->SMODE &= ~TRNG_SMODE_NONCE_Msk;

	irq_flag = 0;
	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	TRNG_DBGMSG("send command TCMD_GEN_NOISE...\n");

	if (TRNG_IssueCommand(TCMD_GEN_NOISE) != 0)
		return -1;

	t0 = EL0_GetCurrentPhysicalValue();

	TRNG_DBGMSG("Wait ISTAT done...\n");
	while (!(irq_flag & TRNG_ISTAT_DONE_Msk))
	{
		if (EL0_GetCurrentPhysicalValue() - t0 > 12000000) /* 1 second timeout */
		{
			TRNG_DBGMSG("SMODE = 0x%x\n", TRNG->SMODE);
			return -1;
		}
	}
	TRNG_DBGMSG("ISTAT Done.\n");
	return 0;
}

/**
  * @brief    TRNG generate seed by user provided nonce data.
  * @param[in]  nonce  48 words nonce data.
  *
  * @retval   0     Success.
  * @retval   < 0   Failed.
  */
int TRNG_GenNonce(uint32_t *nonce)
{
	int   i, j, loop;

	TRNG->SMODE |= TRNG_SMODE_NONCE_Msk;

	TRNG_DBGMSG("TRNG_GenNonce, SMODE = 0x%x\n", TRNG->SMODE);

	if (((TRNG->MODE & TRNG_MODE_SEC_ALG_Msk)>>TRNG_MODE_SEC_ALG_Pos) == 0)
		loop = 2;
	else
		loop = 3;

	for (i = 0; i < loop; i++)
	{
		TRNG_DBGMSG("Write loop %d\n", i);
		irq_flag = 0;

		if (TRNG_Wait_Busy_Clear() != 0)
			return -1;

		for (j = 0; j < 16; j++)
			TRNG->NPA_DATA[j] = nonce[i * 16 + j];

		if (TRNG_IssueCommand(TCMD_GEN_NONCE) != 0)
			return -1;

		while (!(irq_flag & TRNG_ISTAT_DONE_Msk));
	}
	return 0;
}

/**
  * @brief    TRNG use the seed assigned by user.
  * @param[in]  seed  12 words seed data.
  *
  * @retval   0     Success.
  * @retval   < 0   Failed.
  */
int TRNG_SetSeed(uint32_t *seed)
{
	int   i;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	TRNG->SMODE |= TRNG_SMODE_NONCE_Msk;

	for (i = 0; i < 12; i++)
		TRNG->SEED[i] = seed[i];
	return 0;
}

/**
  * @brief    This should be called after TRNG seed generated.
  *           It will make TRNG engine start to work state.
  *
  * @retval   0     Success.
  * @retval   < 0   Failed.
  */
int TRNG_CreateState(void)
{
	irq_flag = 0;

	if (TRNG_Wait_Busy_Clear() != 0)
		return -1;

	if (TRNG_IssueCommand(TCMD_CREATE_STATE) != 0)
		return -1;

	while (!(irq_flag & TRNG_ISTAT_DONE_Msk));
	return 0;
}

/**
  * @brief    Get random data generated by TRNG engine.
  * @param[out]  buff  The data buffer for random data.
  *                    It should be a word array.
  * @param[in]   wcnt  The word count of data to be read.
  *
  * @retval   0     Success.
  * @retval   < 0   Failed.
  */
int  TRNG_GenerateRandomNumber(uint32_t *buff, int wcnt)
{
	int  i, out_cnt;

	for (out_cnt = 0; out_cnt < wcnt; )
	{
		irq_flag = 0;

		if (TRNG_Wait_Busy_Clear() != 0)
			return -1;

		if (TRNG_IssueCommand(TCMD_GEN_RANDOM) != 0)
			return -1;

		while (!(irq_flag & TRNG_ISTAT_DONE_Msk));

		for (i = 0; i < 4; i++)
		{
			buff[out_cnt] = TRNG->RAND[i];
			out_cnt++;
			if (out_cnt >= wcnt)
				break;
		}
	}
	return 0;
}

/*! @}*/ /* end of group TRNG_EXPORTED_CONSTANTS */

/*! @}*/ /* end of group TRNG_Driver */

/*! @}*/ /* end of group Standard_Driver */
