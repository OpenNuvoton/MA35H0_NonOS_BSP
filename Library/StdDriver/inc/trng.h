/**************************************************************************//**
 * @file     trng.h
 * @brief    TRNG driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __TRNG_H__
#define __TRNG_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup TRNG_Driver TRNG Driver
  @{
*/

/** @addtogroup TRNG_EXPORTED_CONSTANTS TRNG Exported Constants
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Temperature Sensor Calibration Constant Definitions                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define TCMD_NOP               0x0       /*!< Execute a NOP */
#define TCMD_GEN_NOISE         0x1       /*!< Generate ful-entropy seed from noise  */
#define TCMD_GEN_NONCE         0x2       /*!< Generate seed from host written nonce */
#define TCMD_CREATE_STATE      0x3       /*!< Move DRBG to create state  */
#define TCMD_RENEW_STATE       0x4       /*!< Move DRBG to renew state   */
#define TCMD_REFRESH_ADDIN     0x5       /*!< Move DRBG to refresh addin */
#define TCMD_GEN_RANDOM        0x6       /*!< Generate a random number   */
#define TCMD_ADVANCE_STATE     0x7       /*!< Advance DRBG state         */
#define TCMD_RUN_KAT           0x8       /*!< Run KAT on DRBG or entropy source */
#define TCMD_ZEROIZE           0xf       /*!< Zeroize                    */

#define SELECT_ALG_AES_128     (TRNG->MODE &= ~TRNG_MODE_SEC_ALG_Msk) /*!< TRNG mode select algorithm AES-128 */
#define SELECT_ALG_AES_256     (TRNG->MODE |= TRNG_MODE_SEC_ALG_Msk)  /*!< TRNG mode select algorithm AES-256 */

/*! @}*/ /* end of group TRNG_EXPORTED_CONSTANTS */

/** @addtogroup TRNG_EXPORTED_FUNCTIONS TRNG Exported Functions
  @{
*/

int  TRNG_Init(void);
int  TRNG_GenNoise(void);
int  TRNG_GenNonce(uint32_t *nonce);
int  TRNG_SetSeed(uint32_t *seed);
int  TRNG_CreateState(void);
int  TRNG_GenerateRandomNumber(uint32_t *buff, int wcnt);

/*! @}*/ /* end of group TRNG_EXPORTED_FUNCTIONS */
/*! @}*/ /* end of group TRNG_Driver */
/*! @}*/ /* end of group Standard_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __TRNG_H__ */

