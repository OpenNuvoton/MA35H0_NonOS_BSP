/**************************************************************************//**
 * @file     ks.c
 * @brief    Key store driver source file
 *
 * @note
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "NuMicro.h"

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup KS_Driver Key Store Driver
  @{
*/

/** @addtogroup KS_EXPORTED_FUNCTIONS Key Store Exported Functions
  @{
*/

/// @cond HIDDEN_SYMBOLS

static uint16_t au8SRAMCntTbl[21] = {4,6,6,7,8,8,8,9,12,13,16,17,18,0,0,0,32,48,64,96,128};
static uint16_t au8OTPCntTbl[7] = {4,6,6,7,8,8,8};

#define KS_CLT_FUNC_MASK	(KS_CTL_IEN_Msk | KS_CTL_TCLR_Msk | KS_CTL_SCMB_Msk | KS_CTL_SILENT_Msk)

#define KS_OP_READ      (0 << KS_CTL_OPMODE_Pos)
#define KS_OP_WRITE     (1 << KS_CTL_OPMODE_Pos)
#define KS_OP_ERASE     (2 << KS_CTL_OPMODE_Pos)
#define KS_OP_ERASE_ALL (3 << KS_CTL_OPMODE_Pos)
#define KS_OP_REVOKE    (4 << KS_CTL_OPMODE_Pos)
#define KS_OP_REMAN     (5 << KS_CTL_OPMODE_Pos)

/// @endcond HIDDEN_SYMBOLS

/**
  * @brief      Initial key store
  * @return     None
  * @details    This function is used to initial the key store.
  *             It is necessary to be called before using other APIs of Key Store.
  */
void KS_Init(void)
{
	/* Start Key Store Initial */
	KS->CTL = KS_CTL_INIT_Msk | KS_CTL_START_Msk;

	/* Waiting for initilization */
	while((KS->STS & KS_STS_INITDONE_Msk) == 0);

	/* Waiting for processing */
	while(KS->STS & KS_STS_BUSY_Msk);
}


/**
  * @brief      Read key from key store
  * @param[in]  eType       The memory type. It could be:
  *                           \ref KS_SRAM
  *                           \ref KS_OTP
  * @param[in]  i32KeyIdx   The key index to read
  * @param[out] au32Key     The buffer to store the key
  * @param[in]  u32WordCnt  The word (32-bit) count of the key buffer size
  * @retval     0           Successful
  * @retval     -1          Fail
  * @retval     -3          read error
  * @details    This function is used to read the key.
  */
int32_t KS_Read(KS_MEM_Type eType, int32_t i32KeyIdx, uint32_t au32Key[], uint32_t u32WordCnt)
{
	int32_t i32Cnt;
	uint32_t u32Cont;
	int32_t offset, i, cnt;

	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
	{
		sysprintf("Error KS is in busy!\n");
		return -1;
	}

	/* Specify the key address */
	KS->METADATA = ((uint32_t)eType << KS_METADATA_DST_Pos) | KS_TOMETAKEY(i32KeyIdx);

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;
	offset = 0;
	u32Cont = 0;
	i32Cnt = u32WordCnt;
	do
	{
		/* Clear Status */
		KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

		// sysprintf("KS_Read - META=0x%x, CTL=0x%x\n", KS->METADATA, u32Cont | KS_OP_READ | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK));
		/* Trigger to read the key */
		KS->CTL = u32Cont | KS_OP_READ | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK);
		/* Waiting for key store processing */
		while(KS->STS & KS_STS_BUSY_Msk);

		/* Read the key to key buffer */
		cnt = i32Cnt;
		if(cnt > 8)
			cnt = 8;
		for(i=0;i<cnt;i++)
		{
			au32Key[offset+i] = KS->KEY[i];
			// sysprintf("KS_Read R[%d]:0x%08x\n", i, au32Key[offset+i]);
		}

		u32Cont = KS_CTL_CONT_Msk;
		i32Cnt -= 8;
		offset += 8;
	} while (i32Cnt > 0);

	// sysprintf("KS_STS (after KS_Read): 0x%x\n",KS->STS);

	/* Check error flag */
	if (KS->STS & KS_STS_EIF_Msk)
		return -3;

	return 0;
}

/**
  * @brief      Write key to key store
  * @param[in]    eType     The memory type. It could be:
  *                           \ref KS_SRAM
  *                           \ref KS_OTP
  * @param[in]  u32Meta     The metadata of the key. It could be the combine of
  *                           \ref KS_META_AES
  *                           \ref KS_META_HMAC
  *                           \ref KS_META_RSA_EXP
  *                           \ref KS_META_RSA_MID
  *                           \ref KS_META_ECC
  *                           \ref KS_META_CPU
  *                           \ref KS_META_128
  *                           \ref KS_META_163
  *                           \ref KS_META_192
  *                           \ref KS_META_224
  *                           \ref KS_META_233
  *                           \ref KS_META_255
  *                           \ref KS_META_256
  *                           \ref KS_META_283
  *                           \ref KS_META_384
  *                           \ref KS_META_409
  *                           \ref KS_META_512
  *                           \ref KS_META_521
  *                           \ref KS_META_571
  *                           \ref KS_META_1024
  *                           \ref KS_META_2048
  *                           \ref KS_META_4096
  *                           \ref KS_META_BOOT
  *                           \ref KS_META_READABLE
  *                           \ref KS_META_PRIV
  *                           \ref KS_META_NONPRIV
  *                           \ref KS_META_SECURE
  *                           \ref KS_META_NONSECUR
  *
  * @param[out] au32Key     The buffer to store the key
  * @retval     0           Successful
  * @retval     -1          Fail
  * @retval     -2          Invalid meta data
  * @retval     -3          write error
  * @details    This function is used to write a key to key store.
  */
int32_t KS_Write(KS_MEM_Type eType, uint32_t u32Meta, uint32_t au32Key[])
{
	int32_t i32Cnt;
	uint32_t u32Cont;
	int32_t offset, i, cnt, sidx;


	/* Just return when key store is in busy */
	if (KS->STS & KS_STS_BUSY_Msk)
	{
		sysprintf("KS is busy!\n");
		return -1;
	}

	/* Specify the key address */
	KS->METADATA = (eType << KS_METADATA_DST_Pos) | u32Meta;
	// sysprintf("KS->METADATA = 0x%x\n", KS->METADATA);

	/* Get size index */
	sidx = ((u32Meta&KS_METADATA_SIZE_Msk) >> KS_METADATA_SIZE_Pos);
	i32Cnt = au8SRAMCntTbl[sidx];

	/* Invalid key length */
	if (i32Cnt == 0)
	{
		sysprintf("Invalid key length!\n");
		return -1;
	}

	/* OTP only support maximum 256 bits */
	if ((eType == KS_OTP) && (i32Cnt > 8))
		return -1;

	// sysprintf("[KS_Write] KS->METADATA = 0x%x\n", KS->METADATA);

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;
	offset = 0;
	u32Cont = 0;
	do
	{
		/* Prepare the key to write */
		cnt = i32Cnt;
		if(cnt > 8)
			cnt = 8;
		for(i=0;i<cnt;i++)
		{
			// sysprintf("[KS_Write] KEY %d = 0x%x\n", i, au32Key[offset+i]);
			KS->KEY[i] = au32Key[offset+i];
		}

		/* Clear Status */
		KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

		/* Write the key */
		KS->CTL = u32Cont | KS_OP_WRITE | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK);
		// sysprintf("           KS->CTL = 0x%x\n", u32Cont | KS_OP_WRITE | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK));

		u32Cont = KS_CTL_CONT_Msk;
		i32Cnt -= 8;
		offset += 8;

		/* Waiting for key store processing */
		while(KS->STS & KS_STS_BUSY_Msk);

	} while(i32Cnt > 0);

	/* Check error flag */
	if (KS->STS & KS_STS_EIF_Msk)
	{
		sysprintf("KS_Write. EIF!\n");
		return -3;
	}
	return KS_TOKEYIDX(KS->METADATA);
}

/**
  * @brief      Erase a key from key store
  * @param[in]    eType     The memory type. It could be:
  *                           \ref KS_SRAM
  *                           \ref KS_OTP
  * @param[in]  i32KeyIdx   The key index to erase
  * @retval     0           Successful
  * @retval     -1          Fail
  * @retval     -3          erase error
  * @details    This function is used to erase a key from SRAM of key store.
   */
int32_t KS_EraseKey(KS_MEM_Type eType, int32_t i32KeyIdx)
{
	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
		return -1;

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;

	/* Specify the key address */
	KS->METADATA = (eType << KS_METADATA_DST_Pos) | KS_TOMETAKEY(i32KeyIdx);

	/* Clear Status */
	KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

	/* Erase the key */
	KS->CTL = KS_OP_ERASE | KS_CTL_START_Msk  | (KS->CTL & KS_CLT_FUNC_MASK);

	/* Waiting for processing */
	while(KS->STS & KS_STS_BUSY_Msk);

	/* Check error flag */
	if(KS->STS & KS_STS_EIF_Msk)
		return -3;

	return 0;
}

/**
  * @brief      Erase all keys from Key Store SRAM
  * @retval     0           Successful
  * @retval     -1          Fail
  * @retval     -3          erase error
  * @details    This function is used to erase all keys in SRAM or Flash of key store.
  */
int32_t KS_EraseAll(void)
{
	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
		return -1;

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;

	/* Specify the key address */
	KS->METADATA = (KS_SRAM << KS_METADATA_DST_Pos);

	/* Clear Status */
	KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

	/* Erase the key */
	KS->CTL = KS_OP_ERASE_ALL | KS_CTL_START_Msk  | (KS->CTL & KS_CLT_FUNC_MASK);

	/* Waiting for processing */
	while(KS->STS & KS_STS_BUSY_Msk);

	/* Check error flag */
	if(KS->STS & KS_STS_EIF_Msk)
		return -3;

	return 0;
}

/**
  * @brief      Revoke a key in key store
  * @param[in]  eType       The memory type. It could be:
  *                           \ref KS_SRAM
  *                           \ref KS_OTP
  * @param[in]  i32KeyIdx   The key index to read
  * @retval     0           Successful
  * @retval     -1          Fail
  * @retval     -3          revoke key error
  * @details    This function is used to revoke a key in key store.
  */
int32_t KS_RevokeKey(KS_MEM_Type eType, int32_t i32KeyIdx)
{
	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
		return -1;

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;

	/* Specify the key address */
	KS->METADATA = (eType << KS_METADATA_DST_Pos) | KS_TOMETAKEY(i32KeyIdx);

	/* Clear Status */
	KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

	/* Erase the key */
	KS->CTL = KS_OP_REVOKE | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK);

	/* Waiting for processing */
	while(KS->STS & KS_STS_BUSY_Msk);

	/* Check error flag */
	if(KS->STS & KS_STS_EIF_Msk)
		return -3;

	return 0;
}

/**
  * @brief      Get remaining available space and key count of Key Store SRAM
  * @param[out] bcnt   The remaining byte count space of Key Store SRAM.
  * @param[out] kcnt   The remaining key count of Key Store SRAM.
  * @retval     0   success
  *             -1  failed
  * @details    This function is used to get remain size of Key Store.
  */
int KS_GetSRAMRemain(uint32_t *bcnt, uint32_t *kcnt)
{
	uint32_t u32Reg;
	uint32_t u32SramRemain;

	/* Waiting for initilization */
	while((KS->STS & KS_STS_INITDONE_Msk) == 0);

	/* Waiting for processing */
	while(KS->STS & KS_STS_BUSY_Msk);

	if (!(KS->STS & KS_STS_INITDONE_Msk) || (KS->STS & KS_STS_BUSY_Msk))
		return -1;

	*bcnt = KS->REMAIN & 0x1FFF;
	*kcnt = KS->REMKCNT & 0x3F;

	return 0;
}

/**
  * @brief      Write OTP key to key store
  * @param[in]  i32KeyIdx   The OTP key index to store the key. It could be 0~7.
  *                         OTP key index 0 is default for ROTPK.
  * @param[in]  u32Meta     The metadata of the key. It could be the combine of
  *                           \ref KS_META_AES
  *                           \ref KS_META_HMAC
  *                           \ref KS_META_RSA_EXP
  *                           \ref KS_META_RSA_MID
  *                           \ref KS_META_ECC
  *                           \ref KS_META_CPU
  *                           \ref KS_META_128
  *                           \ref KS_META_163
  *                           \ref KS_META_192
  *                           \ref KS_META_224
  *                           \ref KS_META_233
  *                           \ref KS_META_255
  *                           \ref KS_META_256
  *                           \ref KS_META_BOOT
  *                           \ref KS_META_READABLE
  *                           \ref KS_META_PRIV
  *                           \ref KS_META_NONPRIV
  *                           \ref KS_META_SECURE
  *                           \ref KS_META_NONSECUR
  *                           
  * @param[out] au32Key     The buffer to store the key
  * @param[in]  u32WordCnt  The word (32-bit) count of the key buffer size
  * @retval     0           Successful
  * @retval     -1          Fail
  * @details    This function is used to write a key to OTP key store.
  */
int32_t KS_WriteOTP(int32_t i32KeyIdx, uint32_t u32Meta, uint32_t au32Key[])
{
	int32_t i32Cnt;
	uint32_t u32Cont;
	int32_t offset, i, cnt, sidx;

	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
		return -1;

	/* Specify the key address */
	KS->METADATA = (KS_OTP << KS_METADATA_DST_Pos) | u32Meta | KS_TOMETAKEY(i32KeyIdx);

	/* Get size index */
	sidx = (u32Meta >> KS_METADATA_SIZE_Pos) & 0xful;

	/* OTP only support maximum 256 bits */
	if(sidx >= 7)
		return -1;

	i32Cnt = au8OTPCntTbl[sidx];

	// sysprintf("[KS_WriteOTP] KS->METADATA = 0x%x\n", KS->METADATA);

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk;
	offset = 0;
	u32Cont = 0;
	do
	{
		/* Prepare the key to write */
		cnt = i32Cnt;
		if(cnt > 8)
			cnt = 8;
		for(i=0;i<cnt;i++)
		{
			KS->KEY[i] = au32Key[offset+i];
			// sysprintf("[KS_WriteOTP] KEY %d = 0x%x\n", i, au32Key[offset+i]);
		}

		/* Clear Status */
		KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;

		/* Write the key */
		KS->CTL = u32Cont | KS_OP_WRITE | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK);
		// sysprintf("           KS->CTL = 0x%x\n", u32Cont | KS_OP_WRITE | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK));

		u32Cont = KS_CTL_CONT_Msk;
		i32Cnt -= 8;
		offset += 8;

		/* Waiting for key store processing */
		while(KS->STS & KS_STS_BUSY_Msk);

	} while(i32Cnt > 0);

	/* Check error flag */
	if (KS->STS & KS_STS_EIF_Msk)
	{
		sysprintf("KS_WriteOTP. EIF!\n");
		return -1;
	}
	return i32KeyIdx;
}

/**
  * @brief      Trigger to run anti-remanence procedure one time.
  * @retval     0           Successful
  * @retval     -1          Fail
  * @details    This function is used to trigger anti-remanence procedure to avoid physical attack.
  */
int32_t KS_TrigReman(void)
{
	int32_t i32Cnt;
	uint32_t u32Cont;
	int32_t offset, i, cnt;

	/* Just return when key store is in busy */
	if(KS->STS & KS_STS_BUSY_Msk)
		return -1;

	/* Specify the key address */
	KS->METADATA = ((uint32_t)KS_SRAM << KS_METADATA_DST_Pos);

	/* Clear error flag */
	KS->STS = KS_STS_EIF_Msk | KS_STS_IF_Msk;
	/* Trigger to read the key */
	KS->CTL = u32Cont | KS_OP_REMAN | KS_CTL_START_Msk | (KS->CTL & KS_CLT_FUNC_MASK);
	/* Waiting for key store processing */
	while(KS->STS & KS_STS_BUSY_Msk);

	/* Check error flag */
	if (KS->STS & KS_STS_EIF_Msk)
	{
		//sysprintf("Remanence option fail!\n");
		return -1;
	}
	return 0;
}

/*@}*/ /* end of group KS_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group KS_Driver */

/*@}*/ /* end of group Standard_Driver */

/*** (C) COPYRIGHT 2023 Nuvoton Technology Corp. ***/
