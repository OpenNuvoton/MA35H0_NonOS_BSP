/**************************************************************************//**
 * @file     crypto_reg.h
 * @version  V1.00
 * @brief    CRYPTO register definition header file
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __CRYPTO_REG_H__
#define __CRYPTO_REG_H__

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/**  @addtogroup REGISTER Control Register
   @{
*/

/**  @addtogroup CRYPTO Cryptographic Accelerator (CRYPTO)
    Memory Mapped Structure for CRYPTO Controller
@{ */

typedef struct
{
	__IO uint32_t INTEN;                 /*!< [0x0000] Crypto Interrupt Enable Control Register                         */
	__IO uint32_t INTSTS;                /*!< [0x0004] Crypto Interrupt Flag                                            */
	__IO uint32_t PRNG_CTL;              /*!< [0x0008] PRNG Control Register                                            */
	__O  uint32_t PRNG_SEED;             /*!< [0x000c] Seed for PRNG                                                    */
	__I  uint32_t PRNG_KEY[8];           /*!< [0x0010] ~ [0x002c] PRNG Generated Key0 ~ Key7                            */
	__I  uint32_t RESERVE0[8];
	__I  uint32_t AES_FDBCK[4];          /*!< [0x0050] ~ [0x005c] AES Engine Output Feedback Data after Cryptographic Operation     */
	__I  uint32_t RESERVE1[8];
	__IO uint32_t AES_GCM_IVCNT[2];      /*!< [0x0080] ~ [0x0084] AES GCM IV Byte Count Register                        */
	__IO uint32_t AES_GCM_ACNT[2];       /*!< [0x0088] ~ [0x008C] AES GCM A Byte Count Register                         */
	__IO uint32_t AES_GCM_PCNT[2];       /*!< [0x0090] ~ [0x0094] AES GCM P Byte Count Register                         */
	__I  uint32_t RESERVE1_1[2];
	__IO uint32_t AES_FBADDR;            /*!< [0x00A0] AES DMA Feedback Address                                         */
	__I  uint32_t RESERVE1_2[23];
	__IO uint32_t AES_CTL;               /*!< [0x0100] AES Control Register                                             */
	__I  uint32_t AES_STS;               /*!< [0x0104] AES Engine Flag                                                  */
	__IO uint32_t AES_DATIN;             /*!< [0x0108] AES Engine Data Input Port Register                              */
	__I  uint32_t AES_DATOUT;            /*!< [0x010c] AES Engine Data Output Port Register                             */
	__IO uint32_t AES0_KEY[8];           /*!< [0x0110] ~ [0x012c] AES Key Word 0~7 Register for Channel 0               */
	__IO uint32_t AES0_IV[4];            /*!< [0x0130] ~ [0x013c] AES Initial Vector Word 0 ~ 3 Register for Channel 0  */
	__IO uint32_t AES0_SADDR;            /*!< [0x0140] AES DMA Source Address Register for Channel 0                    */
	__IO uint32_t AES0_DADDR;            /*!< [0x0144] AES DMA Destination Address Register for Channel 0               */
	__IO uint32_t AES0_CNT;              /*!< [0x0148] AES Byte Count Register for Channel 0                            */
	/// @cond HIDDEN_SYMBOLS
	__I  uint32_t RESERVE5[109];
	/// @endcond //HIDDEN_SYMBOLS
	__IO uint32_t HMAC_CTL;              /*!< [0x0300] SHA/HMAC Control Register                                        */
	__I  uint32_t HMAC_STS;              /*!< [0x0304] SHA/HMAC Status Flag                                             */
	__I  uint32_t HMAC_DGST[16];         /*!< [0x0308] ~ [0x0344] SHA/HMAC Digest Message 0~15                          */
	__IO uint32_t HMAC_KEYCNT;           /*!< [0x0348] SHA/HMAC Key Byte Count Register                                 */
	__IO uint32_t HMAC_SADDR;            /*!< [0x034c] SHA/HMAC DMA Source Address Register                             */
	__IO uint32_t HMAC_DMACNT;           /*!< [0x0350] SHA/HMAC Byte Count Register                                     */
	__IO uint32_t HMAC_DATIN;            /*!< [0x0354] SHA/HMAC Engine Non-DMA Mode Data Input Port Register            */
	__IO uint32_t HMAC_FDBCK[88];        /*!< [0x0358] ~ [0x04B4] SHA/HMAC Output Feedback Data Register                */
	/// @cond HIDDEN_SYMBOLS
	__I  uint32_t RESERVE6[17];
	__IO uint32_t HMAC_FBADDR;           /*!< [0x04FC] SHA/HMAC DMA Feedback Address Register                           */
	__IO uint32_t HMAC_SHAKEDGST[42];    /*!< [0x0500] ~ [0x5A4] SHA/HMAC SHAKE digest message Register                 */
	__I  uint32_t RESERVE7[150];

	/// @endcond //HIDDEN_SYMBOLS
	__IO uint32_t ECC_CTL;               /*!< [0x0800] ECC Control Register                                             */
	__I  uint32_t ECC_STS;               /*!< [0x0804] ECC Status Register                                              */
	__IO uint32_t ECC_X1[18];            /*!< [0x0808] ~ [0x084c] ECC The X-coordinate word 0~17 of the first point     */
	__IO uint32_t ECC_Y1[18];            /*!< [0x0850] ~ [0x0894] ECC The Y-coordinate word 0~17 of the first point     */
	__IO uint32_t ECC_X2[18];            /*!< [0x0898] ~ [0x08dc] ECC The X-coordinate word 0~17 of the second point    */
	__IO uint32_t ECC_Y2[18];            /*!< [0x08e0] ~ [0x0924] ECC The Y-coordinate word 0~17 of the second point    */
	__IO uint32_t ECC_A[18];             /*!< [0x0928] ~ [0x096c] ECC The parameter CURVEA word 0~17 of elliptic curve  */
	__IO uint32_t ECC_B[18];             /*!< [0x0970] ~ [0x09b4] ECC The parameter CURVEB word 0~17 of elliptic curve  */
	__IO uint32_t ECC_N[18];             /*!< [0x09b8] ~ [0x09fc] ECC The parameter CURVEN word 0~17 of elliptic curve  */
	__O  uint32_t ECC_K[18];             /*!< [0x0a00] ~ [0x0a44] ECC The scalar SCALARK word 0~17 of point multiplication */
	__IO uint32_t ECC_SADDR;             /*!< [0x0a48] ECC DMA Source Address Register                                  */
	__IO uint32_t ECC_DADDR;             /*!< [0x0a4c] ECC DMA Destination Address Register                             */
	__IO uint32_t ECC_STARTREG;          /*!< [0x0a50] ECC Starting Address of Updated Registers                        */
	__IO uint32_t ECC_WORDCNT;           /*!< [0x0a54] ECC DMA Word Count                                               */
	__I  uint32_t RESERVE8[42];

	__IO uint32_t RSA_CTL;				 //   [0x0B00] RSA Control Register
	__IO uint32_t RSA_STS;				 //   [0x0B04] RSA Status Register
	__IO uint32_t RSA_SADDR[5];	 		 //   [0x0B08] ~ [0x0B18] RSA DMA Source Address Register
	__IO uint32_t RSA_DADDR;		         //   [0x0B1C] RSA DMA Destination Address Register
	__IO uint32_t RSA_MADDR[7];	 		 //   [0x0B20] ~ [0x0B38] RSA DMA Middle Address Register
	__IO uint32_t RSA_PKADDR;		         //   [0x0B3C] RSA Public Key Address Register
	__I  uint32_t RESERVE9[240];

	__IO uint32_t PRNG_KSCTL;			 //   [0x0F00] PRNG Key Control Register
	__IO uint32_t PRNG_KSSTS;			 //   [0x0F04] PRNG Key Status Register
	__I  uint32_t RESERVE10[2];
	__IO uint32_t AES_KSCTL;			 //   [0x0F10] AES Key Control Register
	__I  uint32_t RESERVE11[7];
	__IO uint32_t HMAC_KSCTL;			 //   [0x0F30] HMAC Key Control Register
	__I  uint32_t RESERVE12[3];
	__IO uint32_t ECC_KSCTL;			 //   [0x0F40] ECC Key Control Register
	__IO uint32_t ECC_KSSTS;			 //   [0x0F44] ECC Key Status Register
	__IO uint32_t ECC_KSXY;			         //   [0x0F48] ECC XY Key Number Register
	__I  uint32_t RESERVE13;
	__IO uint32_t RSA_KSCTL;			 //   [0x0F50] RSA Key Control Register
	__IO uint32_t RSA_KSSTS[2];			 //   [0x0F54] ~ [0x0F58] RSA Key Status Register
	__I  uint32_t RESERVE15[9];
	__IO uint32_t PAP_CTL;			 	 //   [0x0F80] CRYPTO Power Analysis Protection Register
	__I  uint32_t RESERVE16[30];
	__I  uint32_t CRYPTO_VERSION;		 //   [0x0FFC]
} CRPT_T;

/** @addtogroup CRPT_CONST CRPT Bit Field Definition
	Constant Definitions for CRPT Controller
@{ */

#define CRPT_INTEN_AESIEN_Pos            (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define CRPT_INTEN_AESIEN_Msk            (0x1ul << CRPT_INTEN_AESIEN_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define CRPT_INTEN_AESEIEN_Pos           (1)                                               /*!< CRPT_T::INTEN: AESEIEN Position        */
#define CRPT_INTEN_AESEIEN_Msk           (0x1ul << CRPT_INTEN_AESEIEN_Pos)                 /*!< CRPT_T::INTEN: AESEIEN Mask            */

#define CRPT_INTEN_PRNGIEN_Pos           (16)                                              /*!< CRPT_T::INTEN: PRNGIEN Position        */
#define CRPT_INTEN_PRNGIEN_Msk           (0x1ul << CRPT_INTEN_PRNGIEN_Pos)                 /*!< CRPT_T::INTEN: PRNGIEN Mask            */

#define CRPT_INTEN_ECCIEN_Pos            (22)                                              /*!< CRPT_T::INTEN: ECCIEN Position         */
#define CRPT_INTEN_ECCIEN_Msk            (0x1ul << CRPT_INTEN_ECCIEN_Pos)                  /*!< CRPT_T::INTEN: ECCIEN Mask             */

#define CRPT_INTEN_ECCEIEN_Pos           (23)                                              /*!< CRPT_T::INTEN: ECCEIEN Position        */
#define CRPT_INTEN_ECCEIEN_Msk           (0x1ul << CRPT_INTEN_ECCEIEN_Pos)                 /*!< CRPT_T::INTEN: ECCEIEN Mask            */

#define CRPT_INTEN_HMACIEN_Pos           (24)                                              /*!< CRPT_T::INTEN: HMACIEN Position        */
#define CRPT_INTEN_HMACIEN_Msk           (0x1ul << CRPT_INTEN_HMACIEN_Pos)                 /*!< CRPT_T::INTEN: HMACIEN Mask            */

#define CRPT_INTEN_HMACEIEN_Pos          (25)                                              /*!< CRPT_T::INTEN: HMACEIEN Position       */
#define CRPT_INTEN_HMACEIEN_Msk          (0x1ul << CRPT_INTEN_HMACEIEN_Pos)                /*!< CRPT_T::INTEN: HMACEIEN Mask           */

#define CRPT_INTEN_RSAIEN_Pos            (30)                                              /*!< CRPT_T::INTEN: RSAIEN Position         */
#define CRPT_INTEN_RSAIEN_Msk            (0x1ul << CRPT_INTEN_RSAIEN_Pos)                  /*!< CRPT_T::INTEN: RSAIEN Mask             */

#define CRPT_INTEN_RSAEIEN_Pos           (31)                                              /*!< CRPT_T::INTEN: RSAEIEN Position        */
#define CRPT_INTEN_RSAEIEN_Msk           (0x1ul << CRPT_INTEN_RSAEIEN_Pos)                 /*!< CRPT_T::INTEN: RSAEIEN Mask            */

#define CRPT_INTSTS_AESIF_Pos            (0)                                               /*!< CRPT_T::INTSTS: AESIF Position         */
#define CRPT_INTSTS_AESIF_Msk            (0x1ul << CRPT_INTSTS_AESIF_Pos)                  /*!< CRPT_T::INTSTS: AESIF Mask             */

#define CRPT_INTSTS_AESEIF_Pos           (1)                                               /*!< CRPT_T::INTSTS: AESEIF Position        */
#define CRPT_INTSTS_AESEIF_Msk           (0x1ul << CRPT_INTSTS_AESEIF_Pos)                 /*!< CRPT_T::INTSTS: AESEIF Mask            */

#define CRPT_INTSTS_PRNGIF_Pos           (16)                                              /*!< CRPT_T::INTSTS: PRNGIF Position        */
#define CRPT_INTSTS_PRNGIF_Msk           (0x1ul << CRPT_INTSTS_PRNGIF_Pos)                 /*!< CRPT_T::INTSTS: PRNGIF Mask            */

#define CRPT_INTSTS_ECCIF_Pos            (22)                                              /*!< CRPT_T::INTSTS: ECCIF Position         */
#define CRPT_INTSTS_ECCIF_Msk            (0x1ul << CRPT_INTSTS_ECCIF_Pos)                  /*!< CRPT_T::INTSTS: ECCIF Mask             */

#define CRPT_INTSTS_ECCEIF_Pos           (23)                                              /*!< CRPT_T::INTSTS: ECCEIF Position        */
#define CRPT_INTSTS_ECCEIF_Msk           (0x1ul << CRPT_INTSTS_ECCEIF_Pos)                 /*!< CRPT_T::INTSTS: ECCEIF Mask            */

#define CRPT_INTSTS_HMACIF_Pos           (24)                                              /*!< CRPT_T::INTSTS: HMACIF Position        */
#define CRPT_INTSTS_HMACIF_Msk           (0x1ul << CRPT_INTSTS_HMACIF_Pos)                 /*!< CRPT_T::INTSTS: HMACIF Mask            */

#define CRPT_INTSTS_HMACEIF_Pos          (25)                                              /*!< CRPT_T::INTSTS: HMACEIF Position       */
#define CRPT_INTSTS_HMACEIF_Msk          (0x1ul << CRPT_INTSTS_HMACEIF_Pos)                /*!< CRPT_T::INTSTS: HMACEIF Mask           */

#define CRPT_INTSTS_RSAIF_Pos            (30)                                              /*!< CRPT_T::INTSTS: RSAIF Position         */
#define CRPT_INTSTS_RSAIF_Msk            (0x1ul << CRPT_INTSTS_RSAIF_Pos)                  /*!< CRPT_T::INTSTS: RSAIF Mask             */

#define CRPT_INTSTS_RSAEIF_Pos           (31)                                              /*!< CRPT_T::INTSTS: RSAEIF Position        */
#define CRPT_INTSTS_RSAEIF_Msk           (0x1ul << CRPT_INTSTS_RSAEIF_Pos)                 /*!< CRPT_T::INTSTS: RSAEIF Mask            */

#define CRPT_PRNG_CTL_START_Pos          (0)                                               /*!< CRPT_T::PRNG_CTL: START Position       */
#define CRPT_PRNG_CTL_START_Msk          (0x1ul << CRPT_PRNG_CTL_START_Pos)                /*!< CRPT_T::PRNG_CTL: START Mask           */

#define CRPT_PRNG_CTL_SEEDRLD_Pos        (1)                                               /*!< CRPT_T::PRNG_CTL: SEEDRLD Position     */
#define CRPT_PRNG_CTL_SEEDRLD_Msk        (0x1ul << CRPT_PRNG_CTL_SEEDRLD_Pos)              /*!< CRPT_T::PRNG_CTL: SEEDRLD Mask         */

#define CRPT_PRNG_CTL_KEYSZ_Pos          (2)                                               /*!< CRPT_T::PRNG_CTL: KEYSZ Position       */
#define CRPT_PRNG_CTL_KEYSZ_Msk          (0xful << CRPT_PRNG_CTL_KEYSZ_Pos)                /*!< CRPT_T::PRNG_CTL: KEYSZ Mask           */

#define CRPT_PRNG_CTL_BUSY_Pos           (8)                                               /*!< CRPT_T::PRNG_CTL: BUSY Position        */
#define CRPT_PRNG_CTL_BUSY_Msk           (0x1ul << CRPT_PRNG_CTL_BUSY_Pos)                 /*!< CRPT_T::PRNG_CTL: BUSY Mask            */

#define CRPT_PRNG_CTL_SEEDSRC_Pos        (16)                                               /*!< CRPT_T::PRNG_CTL: BUSY Position        */
#define CRPT_PRNG_CTL_SEEDSRC_Msk        (0x1ul << CRPT_PRNG_CTL_SEEDSRC_Pos)                 /*!< CRPT_T::PRNG_CTL: BUSY Mask            */

#define CRPT_PRNG_SEED_SEED_Pos          (0)                                               /*!< CRPT_T::PRNG_SEED: SEED Position       */
#define CRPT_PRNG_SEED_SEED_Msk          (0xfffffffful << CRPT_PRNG_SEED_SEED_Pos)         /*!< CRPT_T::PRNG_SEED: SEED Mask           */

#define CRPT_PRNG_KEYx_KEY_Pos           (0)                                               /*!< CRPT_T::PRNG_KEY[8]: KEY Position      */
#define CRPT_PRNG_KEYx_KEY_Msk           (0xfffffffful << CRPT_PRNG_KEYx_KEY_Pos)          /*!< CRPT_T::PRNG_KEY[8]: KEY Mask          */

#define CRPT_AES_FDBCKx_FDBCK_Pos        (0)                                               /*!< CRPT_T::AES_FDBCK[4]: FDBCK Position   */
#define CRPT_AES_FDBCKx_FDBCK_Msk        (0xfffffffful << CRPT_AES_FDBCKx_FDBCK_Pos)       /*!< CRPT_T::AES_FDBCK[4]: FDBCK Mask       */

#define CRPT_AES_CTL_START_Pos           (0)                                               /*!< CRPT_T::AES_CTL: START Position        */
#define CRPT_AES_CTL_START_Msk           (0x1ul << CRPT_AES_CTL_START_Pos)                 /*!< CRPT_T::AES_CTL: START Mask            */

#define CRPT_AES_CTL_STOP_Pos            (1)                                               /*!< CRPT_T::AES_CTL: STOP Position         */
#define CRPT_AES_CTL_STOP_Msk            (0x1ul << CRPT_AES_CTL_STOP_Pos)                  /*!< CRPT_T::AES_CTL: STOP Mask             */

#define CRPT_AES_CTL_KEYSZ_Pos           (2)                                               /*!< CRPT_T::AES_CTL: KEYSZ Position        */
#define CRPT_AES_CTL_KEYSZ_Msk           (0x3ul << CRPT_AES_CTL_KEYSZ_Pos)                 /*!< CRPT_T::AES_CTL: KEYSZ Mask            */

#define CRPT_AES_CTL_DMALAST_Pos         (5)                                               /*!< CRPT_T::AES_CTL: DMALAST Position      */
#define CRPT_AES_CTL_DMALAST_Msk         (0x1ul << CRPT_AES_CTL_DMALAST_Pos)               /*!< CRPT_T::AES_CTL: DMALAST Mask          */

#define CRPT_AES_CTL_DMACSCAD_Pos        (6)                                               /*!< CRPT_T::AES_CTL: DMACSCAD Position     */
#define CRPT_AES_CTL_DMACSCAD_Msk        (0x1ul << CRPT_AES_CTL_DMACSCAD_Pos)              /*!< CRPT_T::AES_CTL: DMACSCAD Mask         */

#define CRPT_AES_CTL_DMAEN_Pos           (7)                                               /*!< CRPT_T::AES_CTL: DMAEN Position        */
#define CRPT_AES_CTL_DMAEN_Msk           (0x1ul << CRPT_AES_CTL_DMAEN_Pos)                 /*!< CRPT_T::AES_CTL: DMAEN Mask            */

#define CRPT_AES_CTL_OPMODE_Pos          (8)                                               /*!< CRPT_T::AES_CTL: OPMODE Position       */
#define CRPT_AES_CTL_OPMODE_Msk          (0xfful << CRPT_AES_CTL_OPMODE_Pos)               /*!< CRPT_T::AES_CTL: OPMODE Mask           */

#define CRPT_AES_CTL_ENCRPT_Pos          (16)                                              /*!< CRPT_T::AES_CTL: ENCRPT Position       */
#define CRPT_AES_CTL_ENCRPT_Msk          (0x1ul << CRPT_AES_CTL_ENCRPT_Pos)                /*!< CRPT_T::AES_CTL: ENCRPT Mask           */

#define CRPT_AES_CTL_SM4EN_Pos           (17)                                              /*!< CRPT_T::AES_CTL: ENCRPT Position       */
#define CRPT_AES_CTL_SM4EN_Msk           (0x1ul << CRPT_AES_CTL_SM4EN_Pos)                /*!< CRPT_T::AES_CTL: ENCRPT Mask           */

#define CRPT_AES_CTL_FBIN_Pos            (20)                                              /*!< CRPT_T::AES_CTL: OUTSWAP Position      */
#define CRPT_AES_CTL_FBIN_Msk            (0x1ul << CRPT_AES_CTL_FBIN_Pos)               /*!< CRPT_T::AES_CTL: OUTSWAP Mask          */

#define CRPT_AES_CTL_FBOUT_Pos           (21)                                              /*!< CRPT_T::AES_CTL: OUTSWAP Position      */
#define CRPT_AES_CTL_FBOUT_Msk           (0x1ul << CRPT_AES_CTL_FBOUT_Pos)               /*!< CRPT_T::AES_CTL: OUTSWAP Mask          */

#define CRPT_AES_CTL_OUTSWAP_Pos         (22)                                              /*!< CRPT_T::AES_CTL: OUTSWAP Position      */
#define CRPT_AES_CTL_OUTSWAP_Msk         (0x1ul << CRPT_AES_CTL_OUTSWAP_Pos)               /*!< CRPT_T::AES_CTL: OUTSWAP Mask          */

#define CRPT_AES_CTL_INSWAP_Pos          (23)                                              /*!< CRPT_T::AES_CTL: INSWAP Position       */
#define CRPT_AES_CTL_INSWAP_Msk          (0x1ul << CRPT_AES_CTL_INSWAP_Pos)                /*!< CRPT_T::AES_CTL: INSWAP Mask           */

#define CRPT_AES_CTL_KOUTSWAP_Pos        (24)                                              /*!< CRPT_T::AES_CTL: CHANNEL Position      */
#define CRPT_AES_CTL_KOUTSWAP_Msk        (0x1ul << CRPT_AES_CTL_KOUTSWAP_Pos)               /*!< CRPT_T::AES_CTL: CHANNEL Mask          */

#define CRPT_AES_CTL_KINSWAP_Pos         (25)                                              /*!< CRPT_T::AES_CTL: CHANNEL Position      */
#define CRPT_AES_CTL_KINSWAP_Msk         (0x1ul << CRPT_AES_CTL_KINSWAP_Pos)               /*!< CRPT_T::AES_CTL: CHANNEL Mask          */

#define CRPT_AES_CTL_KEYUNPRT_Pos        (26)                                              /*!< CRPT_T::AES_CTL: KEYUNPRT Position     */
#define CRPT_AES_CTL_KEYUNPRT_Msk        (0x1ful << CRPT_AES_CTL_KEYUNPRT_Pos)             /*!< CRPT_T::AES_CTL: KEYUNPRT Mask         */

#define CRPT_AES_CTL_KEYPRT_Pos          (31)                                              /*!< CRPT_T::AES_CTL: KEYPRT Position       */
#define CRPT_AES_CTL_KEYPRT_Msk          (0x1ul << CRPT_AES_CTL_KEYPRT_Pos)                /*!< CRPT_T::AES_CTL: KEYPRT Mask           */

#define CRPT_AES_STS_BUSY_Pos            (0)                                               /*!< CRPT_T::AES_STS: BUSY Position         */
#define CRPT_AES_STS_BUSY_Msk            (0x1ul << CRPT_AES_STS_BUSY_Pos)                  /*!< CRPT_T::AES_STS: BUSY Mask             */

#define CRPT_AES_STS_INBUFEMPTY_Pos      (8)                                               /*!< CRPT_T::AES_STS: INBUFEMPTY Position   */
#define CRPT_AES_STS_INBUFEMPTY_Msk      (0x1ul << CRPT_AES_STS_INBUFEMPTY_Pos)            /*!< CRPT_T::AES_STS: INBUFEMPTY Mask       */

#define CRPT_AES_STS_INBUFFULL_Pos       (9)                                               /*!< CRPT_T::AES_STS: INBUFFULL Position    */
#define CRPT_AES_STS_INBUFFULL_Msk       (0x1ul << CRPT_AES_STS_INBUFFULL_Pos)             /*!< CRPT_T::AES_STS: INBUFFULL Mask        */

#define CRPT_AES_STS_INBUFERR_Pos        (10)                                              /*!< CRPT_T::AES_STS: INBUFERR Position     */
#define CRPT_AES_STS_INBUFERR_Msk        (0x1ul << CRPT_AES_STS_INBUFERR_Pos)              /*!< CRPT_T::AES_STS: INBUFERR Mask         */

#define CRPT_AES_STS_CNTERR_Pos          (12)                                              /*!< CRPT_T::AES_STS: CNTERR Position       */
#define CRPT_AES_STS_CNTERR_Msk          (0x1ul << CRPT_AES_STS_CNTERR_Pos)                /*!< CRPT_T::AES_STS: CNTERR Mask           */

#define CRPT_AES_STS_OUTBUFEMPTY_Pos     (16)                                              /*!< CRPT_T::AES_STS: OUTBUFEMPTY Position  */
#define CRPT_AES_STS_OUTBUFEMPTY_Msk     (0x1ul << CRPT_AES_STS_OUTBUFEMPTY_Pos)           /*!< CRPT_T::AES_STS: OUTBUFEMPTY Mask      */

#define CRPT_AES_STS_OUTBUFFULL_Pos      (17)                                              /*!< CRPT_T::AES_STS: OUTBUFFULL Position   */
#define CRPT_AES_STS_OUTBUFFULL_Msk      (0x1ul << CRPT_AES_STS_OUTBUFFULL_Pos)            /*!< CRPT_T::AES_STS: OUTBUFFULL Mask       */

#define CRPT_AES_STS_OUTBUFERR_Pos       (18)                                              /*!< CRPT_T::AES_STS: OUTBUFERR Position    */
#define CRPT_AES_STS_OUTBUFERR_Msk       (0x1ul << CRPT_AES_STS_OUTBUFERR_Pos)             /*!< CRPT_T::AES_STS: OUTBUFERR Mask        */

#define CRPT_AES_STS_BUSERR_Pos          (20)                                              /*!< CRPT_T::AES_STS: BUSERR Position       */
#define CRPT_AES_STS_BUSERR_Msk          (0x1ul << CRPT_AES_STS_BUSERR_Pos)                /*!< CRPT_T::AES_STS: BUSERR Mask           */

#define CRPT_AES_DATIN_DATIN_Pos         (0)                                               /*!< CRPT_T::AES_DATIN: DATIN Position      */
#define CRPT_AES_DATIN_DATIN_Msk         (0xfffffffful << CRPT_AES_DATIN_DATIN_Pos)        /*!< CRPT_T::AES_DATIN: DATIN Mask          */

#define CRPT_AES_DATOUT_DATOUT_Pos       (0)                                               /*!< CRPT_T::AES_DATOUT: DATOUT Position    */
#define CRPT_AES_DATOUT_DATOUT_Msk       (0xfffffffful << CRPT_AES_DATOUT_DATOUT_Pos)      /*!< CRPT_T::AES_DATOUT: DATOUT Mask        */

#define CRPT_AES0_KEYx_KEY_Pos           (0)                                               /*!< CRPT_T::AES0_KEY[8]: KEY Position      */
#define CRPT_AES0_KEYx_KEY_Msk           (0xfffffffful << CRPT_AES0_KEYx_KEY_Pos)          /*!< CRPT_T::AES0_KEY[8]: KEY Mask          */

#define CRPT_AES0_IVx_IV_Pos             (0)                                               /*!< CRPT_T::AES0_IV[4]: IV Position        */
#define CRPT_AES0_IVx_IV_Msk             (0xfffffffful << CRPT_AES0_IVx_IV_Pos)            /*!< CRPT_T::AES0_IV[4]: IV Mask            */

#define CRPT_AES0_SADDR_SADDR_Pos        (0)                                               /*!< CRPT_T::AES0_SADDR: SADDR Position     */
#define CRPT_AES0_SADDR_SADDR_Msk        (0xfffffffful << CRPT_AES0_SADDR_SADDR_Pos)       /*!< CRPT_T::AES0_SADDR: SADDR Mask         */

#define CRPT_AES0_DADDR_DADDR_Pos        (0)                                               /*!< CRPT_T::AES0_DADDR: DADDR Position     */
#define CRPT_AES0_DADDR_DADDR_Msk        (0xfffffffful << CRPT_AES0_DADDR_DADDR_Pos)       /*!< CRPT_T::AES0_DADDR: DADDR Mask         */

#define CRPT_AES0_CNT_CNT_Pos            (0)                                               /*!< CRPT_T::AES0_CNT: CNT Position         */
#define CRPT_AES0_CNT_CNT_Msk            (0xfffffffful << CRPT_AES0_CNT_CNT_Pos)           /*!< CRPT_T::AES0_CNT: CNT Mask             */

#define CRPT_HMAC_CTL_START_Pos          (0)                                               /*!< CRPT_T::HMAC_CTL: START Position       */
#define CRPT_HMAC_CTL_START_Msk          (0x1ul << CRPT_HMAC_CTL_START_Pos)                /*!< CRPT_T::HMAC_CTL: START Mask           */

#define CRPT_HMAC_CTL_STOP_Pos           (1)                                               /*!< CRPT_T::HMAC_CTL: STOP Position        */
#define CRPT_HMAC_CTL_STOP_Msk           (0x1ul << CRPT_HMAC_CTL_STOP_Pos)                 /*!< CRPT_T::HMAC_CTL: STOP Mask            */

#define CRPT_HMAC_CTL_DMAFIRST_Pos       (4)                                               /*!< CRPT_T::HMAC_CTL: DMALAST Position     */
#define CRPT_HMAC_CTL_DMAFIRST_Msk       (0x1ul << CRPT_HMAC_CTL_DMAFIRST_Pos)              /*!< CRPT_T::HMAC_CTL: DMALAST Mask         */

#define CRPT_HMAC_CTL_DMALAST_Pos        (5)                                               /*!< CRPT_T::HMAC_CTL: DMALAST Position     */
#define CRPT_HMAC_CTL_DMALAST_Msk        (0x1ul << CRPT_HMAC_CTL_DMALAST_Pos)              /*!< CRPT_T::HMAC_CTL: DMALAST Mask         */

#define CRPT_HMAC_CTL_DMACSCAD_Pos       (6)                                               /*!< CRPT_T::HMAC_CTL: DMALAST Position     */
#define CRPT_HMAC_CTL_DMACSCAD_Msk       (0x1ul << CRPT_HMAC_CTL_DMACSCAD_Pos)              /*!< CRPT_T::HMAC_CTL: DMALAST Mask         */

#define CRPT_HMAC_CTL_DMAEN_Pos          (7)                                               /*!< CRPT_T::HMAC_CTL: DMAEN Position       */
#define CRPT_HMAC_CTL_DMAEN_Msk          (0x1ul << CRPT_HMAC_CTL_DMAEN_Pos)                /*!< CRPT_T::HMAC_CTL: DMAEN Mask           */

#define CRPT_HMAC_CTL_OPMODE_Pos         (8)                                               /*!< CRPT_T::HMAC_CTL: OPMODE Position      */
#define CRPT_HMAC_CTL_OPMODE_Msk         (0x7ul << CRPT_HMAC_CTL_OPMODE_Pos)               /*!< CRPT_T::HMAC_CTL: OPMODE Mask          */

#define CRPT_HMAC_CTL_HMACEN_Pos         (11)                                               /*!< CRPT_T::HMAC_CTL: HMACEN Position      */
#define CRPT_HMAC_CTL_HMACEN_Msk         (0x1ul << CRPT_HMAC_CTL_HMACEN_Pos)               /*!< CRPT_T::HMAC_CTL: HMACEN Mask          */

#define CRPT_HMAC_CTL_SHA3EN_Pos         (12)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_SHA3EN_Msk         (0x1ul << CRPT_HMAC_CTL_SHA3EN_Pos)               /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_SM3EN_Pos          (13)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_SM3EN_Msk          (0x1ul << CRPT_HMAC_CTL_SM3EN_Pos)                 /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_MD5EN_Pos          (14)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_MD5EN_Msk          (0x1ul << CRPT_HMAC_CTL_MD5EN_Pos)                 /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_FBIN_Pos           (20)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_FBIN_Msk           (0x1ul << CRPT_HMAC_CTL_FBIN_Pos)                 /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_FBOUT_Pos          (21)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_FBOUT_Msk          (0x1ul << CRPT_HMAC_CTL_FBOUT_Pos)                /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_OUTSWAP_Pos        (22)                                              /*!< CRPT_T::HMAC_CTL: OUTSWAP Position     */
#define CRPT_HMAC_CTL_OUTSWAP_Msk        (0x1ul << CRPT_HMAC_CTL_OUTSWAP_Pos)              /*!< CRPT_T::HMAC_CTL: OUTSWAP Mask         */

#define CRPT_HMAC_CTL_INSWAP_Pos         (23)                                              /*!< CRPT_T::HMAC_CTL: INSWAP Position      */
#define CRPT_HMAC_CTL_INSWAP_Msk         (0x1ul << CRPT_HMAC_CTL_INSWAP_Pos)               /*!< CRPT_T::HMAC_CTL: INSWAP Mask          */

#define CRPT_HMAC_CTL_NEXTDGST_Pos       (24)                                              /*!< CRPT_T::HMAC_CTL: INSWAP Position      */
#define CRPT_HMAC_CTL_NEXTDGST_Msk       (0x1ul << CRPT_HMAC_CTL_NEXTDGST_Pos)               /*!< CRPT_T::HMAC_CTL: INSWAP Mask          */

#define CRPT_HMAC_CTL_FINISHDGST_Pos     (25)                                              /*!< CRPT_T::HMAC_CTL: INSWAP Position      */
#define CRPT_HMAC_CTL_FINISHDGST_Msk     (0x1ul << CRPT_HMAC_CTL_FINISHDGST_Pos)               /*!< CRPT_T::HMAC_CTL: INSWAP Mask          */

#define CRPT_HMAC_STS_BUSY_Pos           (0)                                               /*!< CRPT_T::HMAC_STS: BUSY Position        */
#define CRPT_HMAC_STS_BUSY_Msk           (0x1ul << CRPT_HMAC_STS_BUSY_Pos)                 /*!< CRPT_T::HMAC_STS: BUSY Mask            */

#define CRPT_HMAC_STS_DMABUSY_Pos        (1)                                               /*!< CRPT_T::HMAC_STS: DMABUSY Position     */
#define CRPT_HMAC_STS_DMABUSY_Msk        (0x1ul << CRPT_HMAC_STS_DMABUSY_Pos)              /*!< CRPT_T::HMAC_STS: DMABUSY Mask         */

#define CRPT_HMAC_STS_SHAKEBUSY_Pos      (2)                                               /*!< CRPT_T::HMAC_STS: DMABUSY Position     */
#define CRPT_HMAC_STS_SHAKEBUSY_Msk      (0x1ul << CRPT_HMAC_STS_SHAKEBUSY_Pos)              /*!< CRPT_T::HMAC_STS: DMABUSY Mask         */

#define CRPT_HMAC_STS_DMAERR_Pos         (8)                                               /*!< CRPT_T::HMAC_STS: DMAERR Position      */
#define CRPT_HMAC_STS_DMAERR_Msk         (0x1ul << CRPT_HMAC_STS_DMAERR_Pos)               /*!< CRPT_T::HMAC_STS: DMAERR Mask          */

#define CRPT_HMAC_STS_KSERR_Pos          (9)                                               /*!< CRPT_T::HMAC_STS: DMAERR Position      */
#define CRPT_HMAC_STS_KSERR_Msk          (0x1ul << CRPT_HMAC_STS_KSERR_Pos)               /*!< CRPT_T::HMAC_STS: DMAERR Mask          */

#define CRPT_HMAC_STS_DATINREQ_Pos       (16)                                              /*!< CRPT_T::HMAC_STS: DATINREQ Position    */
#define CRPT_HMAC_STS_DATINREQ_Msk       (0x1ul << CRPT_HMAC_STS_DATINREQ_Pos)             /*!< CRPT_T::HMAC_STS: DATINREQ Mask        */

#define CRPT_HMAC_DGSTx_DGST_Pos         (0)                                               /*!< CRPT_T::HMAC_DGST[16]: DGST Position   */
#define CRPT_HMAC_DGSTx_DGST_Msk         (0xfffffffful << CRPT_HMAC_DGSTx_DGST_Pos)        /*!< CRPT_T::HMAC_DGST[16]: DGST Mask       */

#define CRPT_HMAC_KEYCNT_KEYCNT_Pos      (0)                                               /*!< CRPT_T::HMAC_KEYCNT: KEYCNT Position   */
#define CRPT_HMAC_KEYCNT_KEYCNT_Msk      (0xfffffffful << CRPT_HMAC_KEYCNT_KEYCNT_Pos)     /*!< CRPT_T::HMAC_KEYCNT: KEYCNT Mask       */

#define CRPT_HMAC_SADDR_SADDR_Pos        (0)                                               /*!< CRPT_T::HMAC_SADDR: SADDR Position     */
#define CRPT_HMAC_SADDR_SADDR_Msk        (0xfffffffful << CRPT_HMAC_SADDR_SADDR_Pos)       /*!< CRPT_T::HMAC_SADDR: SADDR Mask         */

#define CRPT_HMAC_DMACNT_DMACNT_Pos      (0)                                               /*!< CRPT_T::HMAC_DMACNT: DMACNT Position   */
#define CRPT_HMAC_DMACNT_DMACNT_Msk      (0xfffffffful << CRPT_HMAC_DMACNT_DMACNT_Pos)     /*!< CRPT_T::HMAC_DMACNT: DMACNT Mask       */

#define CRPT_HMAC_DATIN_DATIN_Pos        (0)                                               /*!< CRPT_T::HMAC_DATIN: DATIN Position     */
#define CRPT_HMAC_DATIN_DATIN_Msk        (0xfffffffful << CRPT_HMAC_DATIN_DATIN_Pos)       /*!< CRPT_T::HMAC_DATIN: DATIN Mask         */

#define CRPT_ECC_CTL_START_Pos           (0)                                               /*!< CRPT_T::ECC_CTL: START Position        */
#define CRPT_ECC_CTL_START_Msk           (0x1ul << CRPT_ECC_CTL_START_Pos)                 /*!< CRPT_T::ECC_CTL: START Mask            */

#define CRPT_ECC_CTL_STOP_Pos            (1)                                               /*!< CRPT_T::ECC_CTL: STOP Position         */
#define CRPT_ECC_CTL_STOP_Msk            (0x1ul << CRPT_ECC_CTL_STOP_Pos)                  /*!< CRPT_T::ECC_CTL: STOP Mask             */

#define CRPT_ECC_CTL_PFA2C_Pos           (3)                                               /*!< CRPT_T::ECC_CTL: STOP Position         */
#define CRPT_ECC_CTL_PFA2C_Msk           (0x1ul << CRPT_ECC_CTL_PFA2C_Pos)                  /*!< CRPT_T::ECC_CTL: STOP Mask             */

#define CRPT_ECC_CTL_ECDSAS_Pos          (4)                                               /*!< CRPT_T::ECC_CTL: STOP Position         */
#define CRPT_ECC_CTL_ECDSAS_Msk          (0x1ul << CRPT_ECC_CTL_ECDSAS_Pos)                  /*!< CRPT_T::ECC_CTL: STOP Mask             */

#define CRPT_ECC_CTL_ECDSAR_Pos          (5)                                               /*!< CRPT_T::ECC_CTL: STOP Position         */
#define CRPT_ECC_CTL_ECDSAR_Msk          (0x1ul << CRPT_ECC_CTL_ECDSAR_Pos)                  /*!< CRPT_T::ECC_CTL: STOP Mask             */

#define CRPT_ECC_CTL_DMAEN_Pos           (7)                                               /*!< CRPT_T::ECC_CTL: DMAEN Position        */
#define CRPT_ECC_CTL_DMAEN_Msk           (0x1ul << CRPT_ECC_CTL_DMAEN_Pos)                 /*!< CRPT_T::ECC_CTL: DMAEN Mask            */

#define CRPT_ECC_CTL_FSEL_Pos            (8)                                               /*!< CRPT_T::ECC_CTL: FSEL Position         */
#define CRPT_ECC_CTL_FSEL_Msk            (0x1ul << CRPT_ECC_CTL_FSEL_Pos)                  /*!< CRPT_T::ECC_CTL: FSEL Mask             */

#define CRPT_ECC_CTL_ECCOP_Pos           (9)                                               /*!< CRPT_T::ECC_CTL: ECCOP Position        */
#define CRPT_ECC_CTL_ECCOP_Msk           (0x3ul << CRPT_ECC_CTL_ECCOP_Pos)                 /*!< CRPT_T::ECC_CTL: ECCOP Mask            */

#define CRPT_ECC_CTL_MODOP_Pos           (11)                                              /*!< CRPT_T::ECC_CTL: MODOP Position        */
#define CRPT_ECC_CTL_MODOP_Msk           (0x3ul << CRPT_ECC_CTL_MODOP_Pos)                 /*!< CRPT_T::ECC_CTL: MODOP Mask            */

#define CRPT_ECC_CTL_CSEL_Pos            (13)                                              /*!< CRPT_T::ECC_CTL: MODOP Position        */
#define CRPT_ECC_CTL_CSEL_Msk            (0x1ul << CRPT_ECC_CTL_CSEL_Pos)                 /*!< CRPT_T::ECC_CTL: MODOP Mask            */

#define CRPT_ECC_CTL_SCAP_Pos            (14)                                              /*!< CRPT_T::ECC_CTL: MODOP Position        */
#define CRPT_ECC_CTL_SCAP_Msk            (0x1ul << CRPT_ECC_CTL_SCAP_Pos)                 /*!< CRPT_T::ECC_CTL: MODOP Mask            */

#define CRPT_ECC_CTL_ASCAP_Pos           (15)                                              /*!< CRPT_T::ECC_CTL: MODOP Position        */
#define CRPT_ECC_CTL_ASCAP_Msk           (0x1ul << CRPT_ECC_CTL_ASCAP_Pos)                 /*!< CRPT_T::ECC_CTL: MODOP Mask            */

#define CRPT_ECC_CTL_LDP1_Pos            (16)                                              /*!< CRPT_T::ECC_CTL: LDP1 Position         */
#define CRPT_ECC_CTL_LDP1_Msk            (0x1ul << CRPT_ECC_CTL_LDP1_Pos)                  /*!< CRPT_T::ECC_CTL: LDP1 Mask             */

#define CRPT_ECC_CTL_LDP2_Pos            (17)                                              /*!< CRPT_T::ECC_CTL: LDP2 Position         */
#define CRPT_ECC_CTL_LDP2_Msk            (0x1ul << CRPT_ECC_CTL_LDP2_Pos)                  /*!< CRPT_T::ECC_CTL: LDP2 Mask             */

#define CRPT_ECC_CTL_LDA_Pos             (18)                                              /*!< CRPT_T::ECC_CTL: LDA Position          */
#define CRPT_ECC_CTL_LDA_Msk             (0x1ul << CRPT_ECC_CTL_LDA_Pos)                   /*!< CRPT_T::ECC_CTL: LDA Mask              */

#define CRPT_ECC_CTL_LDB_Pos             (19)                                              /*!< CRPT_T::ECC_CTL: LDB Position          */
#define CRPT_ECC_CTL_LDB_Msk             (0x1ul << CRPT_ECC_CTL_LDB_Pos)                   /*!< CRPT_T::ECC_CTL: LDB Mask              */

#define CRPT_ECC_CTL_LDN_Pos             (20)                                              /*!< CRPT_T::ECC_CTL: LDN Position          */
#define CRPT_ECC_CTL_LDN_Msk             (0x1ul << CRPT_ECC_CTL_LDN_Pos)                   /*!< CRPT_T::ECC_CTL: LDN Mask              */

#define CRPT_ECC_CTL_LDK_Pos             (21)                                              /*!< CRPT_T::ECC_CTL: LDK Position          */
#define CRPT_ECC_CTL_LDK_Msk             (0x1ul << CRPT_ECC_CTL_LDK_Pos)                   /*!< CRPT_T::ECC_CTL: LDK Mask              */

#define CRPT_ECC_CTL_CURVEM_Pos          (22)                                              /*!< CRPT_T::ECC_CTL: CURVEM Position       */
#define CRPT_ECC_CTL_CURVEM_Msk          (0x3fful << CRPT_ECC_CTL_CURVEM_Pos)              /*!< CRPT_T::ECC_CTL: CURVEM Mask           */

#define CRPT_ECC_STS_BUSY_Pos            (0)                                               /*!< CRPT_T::ECC_STS: BUSY Position         */
#define CRPT_ECC_STS_BUSY_Msk            (0x1ul << CRPT_ECC_STS_BUSY_Pos)                  /*!< CRPT_T::ECC_STS: BUSY Mask             */

#define CRPT_ECC_STS_DMABUSY_Pos         (1)                                               /*!< CRPT_T::ECC_STS: DMABUSY Position      */
#define CRPT_ECC_STS_DMABUSY_Msk         (0x1ul << CRPT_ECC_STS_DMABUSY_Pos)               /*!< CRPT_T::ECC_STS: DMABUSY Mask          */

#define CRPT_ECC_STS_BUSERR_Pos          (16)                                              /*!< CRPT_T::ECC_STS: BUSERR Position       */
#define CRPT_ECC_STS_BUSERR_Msk          (0x1ul << CRPT_ECC_STS_BUSERR_Pos)                /*!< CRPT_T::ECC_STS: BUSERR Mask           */

#define CRPT_ECC_X1_POINTX1_Pos          (0)                                               /*!< CRPT_T::ECC_X1[18]:  POINTX1 Position  */
#define CRPT_ECC_X1_POINTX1_Msk          (0xfffffffful << CRPT_ECC_X1_POINTX1_Pos)         /*!< CRPT_T::ECC_X1[18]:  POINTX1 Mask      */

#define CRPT_ECC_Y1_POINTY1_Pos          (0)                                               /*!< CRPT_T::ECC_Y1[18]: POINTY1 Position   */
#define CRPT_ECC_Y1_POINTY1_Msk          (0xfffffffful << CRPT_ECC_Y1_POINTY1_Pos)         /*!< CRPT_T::ECC_Y1[18]: POINTY1 Mask       */

#define CRPT_ECC_X2_POINTX2_Pos          (0)                                               /*!< CRPT_T::ECC_X2[18]: POINTX2 Position   */
#define CRPT_ECC_X2_POINTX2_Msk          (0xfffffffful << CRPT_ECC_X2_POINTX2_Pos)         /*!< CRPT_T::ECC_X2[18]: POINTX2 Mask       */

#define CRPT_ECC_Y2_POINTY2_Pos          (0)                                               /*!< CRPT_T::ECC_Y2[18]: POINTY2 Position   */
#define CRPT_ECC_Y2_POINTY2_Msk          (0xfffffffful << CRPT_ECC_Y2_POINTY2_Pos)         /*!< CRPT_T::ECC_Y2[18]: POINTY2 Mask       */

#define CRPT_ECC_A_CURVEA_Pos            (0)                                               /*!< CRPT_T::ECC_A[18]: CURVEA Position     */
#define CRPT_ECC_A_CURVEA_Msk            (0xfffffffful << CRPT_ECC_A_CURVEA_Pos)           /*!< CRPT_T::ECC_A[18]: CURVEA Mask         */

#define CRPT_ECC_B_CURVEB_Pos            (0)                                               /*!< CRPT_T::ECC_B[18]: CURVEB Position     */
#define CRPT_ECC_B_CURVEB_Msk            (0xfffffffful << CRPT_ECC_B_CURVEB_Pos)           /*!< CRPT_T::ECC_B[18]: CURVEB Mask         */

#define CRPT_ECC_N_CURVEN_Pos            (0)                                               /*!< CRPT_T::ECC_N[18]: CURVEN Position     */
#define CRPT_ECC_N_CURVEN_Msk            (0xfffffffful << CRPT_ECC_N_CURVEN_Pos)           /*!< CRPT_T::ECC_N[18]: CURVEN Mask         */

#define CRPT_ECC_K_SCALARK_Pos           (0)                                               /*!< CRPT_T::ECC_K[18]: SCALARK Position    */
#define CRPT_ECC_K_SCALARK_Msk           (0xfffffffful << CRPT_ECC_K_SCALARK_Pos)          /*!< CRPT_T::ECC_K[18]: SCALARK Mask        */

#define CRPT_ECC_DADDR_DADDR_Pos         (0)                                               /*!< CRPT_T::ECC_DADDR: DADDR Position      */
#define CRPT_ECC_DADDR_DADDR_Msk         (0xfffffffful << CRPT_ECC_DADDR_DADDR_Pos)        /*!< CRPT_T::ECC_DADDR: DADDR Mask          */

#define CRPT_ECC_STARTREG_STARTREG_Pos   (0)                                               /*!< CRPT_T::ECC_STARTREG: STARTREG Position*/
#define CRPT_ECC_STARTREG_STARTREG_Msk   (0xfffffffful << CRPT_ECC_STARTREG_STARTREG_Pos)  /*!< CRPT_T::ECC_STARTREG: STARTREG Mask    */

#define CRPT_ECC_WORDCNT_WORDCNT_Pos     (0)                                               /*!< CRPT_T::ECC_WORDCNT: WORDCNT Position  */
#define CRPT_ECC_WORDCNT_WORDCNT_Msk     (0xfffffffful << CRPT_ECC_WORDCNT_WORDCNT_Pos)    /*!< CRPT_T::ECC_WORDCNT: WORDCNT Mask      */

#define CRPT_RSA_CTL_START_Pos            (0)                                              /*!< CRPT RSA_CTL: START Position            */
#define CRPT_RSA_CTL_START_Msk            (0x1ul << CRPT_RSA_CTL_START_Pos)                /*!< CRPT RSA_CTL: START Mask                */

#define CRPT_RSA_CTL_STOP_Pos             (1)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_STOP_Msk             (0x1ul << CRPT_RSA_CTL_STOP_Pos)                 /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_CTL_CRT_Pos              (2)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_CRT_Msk              (0x1ul << CRPT_RSA_CTL_CRT_Pos)                 /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_CTL_CRTBYP_Pos           (3)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_CRTBYP_Msk           (0x1ul << CRPT_RSA_CTL_CRTBYP_Pos)                 /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_CTL_KEYLENG_Pos          (4)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_KEYLENG_Msk          (0x3ul << CRPT_RSA_CTL_KEYLENG_Pos)                 /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_CTL_SCAP_Pos             (8)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_SCAP_Msk             (0x1ul << CRPT_RSA_CTL_SCAP_Pos)                 /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_CTL_CFIAP_Pos            (9)                                              /*!< CRPT RSA_CTL: STOP Position             */
#define CRPT_RSA_CTL_CFIAP_Msk            (0x1ul << CRPT_RSA_CTL_CFIAP_Pos)                /*!< CRPT RSA_CTL: STOP Mask                 */

#define CRPT_RSA_STS_BUSY_Pos             (0)                                              /*!< CRPT RSA_STS: BUSY Position             */
#define CRPT_RSA_STS_BUSY_Msk             (0x1ul << CRPT_RSA_STS_BUSY_Pos)                 /*!< CRPT RSA_STS: BUSY Mask                 */

#define CRPT_RSA_STS_DMABUSY_Pos          (1)                                              /*!< CRPT RSA_STS: DMABUSY Position          */
#define CRPT_RSA_STS_DMABUSY_Msk          (0x1ul << CRPT_RSA_STS_DMABUSY_Pos)              /*!< CRPT RSA_STS: DMABUSY Mask              */

#define CRPT_RSA_STS_BUSERR_Pos           (16)                                             /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_STS_BUSERR_Msk           (0x1ul << CRPT_RSA_STS_BUSERR_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_RSA_STS_CTLERR_Pos           (17)                                             /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_STS_CTLERR_Msk           (0x1ul << CRPT_RSA_STS_CTLERR_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_RSA_STS_KSERR_Pos            (18)                                             /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_STS_KSERR_Msk            (0x1ul << CRPT_RSA_STS_KSERR_Pos)                /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_NUM_Pos           (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_NUM_Msk           (0x1ful << CRPT_PRNG_KSCTL_NUM_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_KEYSRC_Pos        (8)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_KEYSRC_Msk        (0x1ul << CRPT_PRNG_KSCTL_KEYSRC_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_TRUST_Pos         (16)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_TRUST_Msk         (0x1ul << CRPT_PRNG_KSCTL_TRUST_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_ECDH_Pos          (19)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_ECDH_Msk          (0x1ul << CRPT_PRNG_KSCTL_ECDH_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_ECDSA_Pos         (20)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_ECDSA_Msk         (0x1ul << CRPT_PRNG_KSCTL_ECDSA_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_WDST_Pos          (21)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_WDST_Msk          (0x1ul << CRPT_PRNG_KSCTL_WDST_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_WSDST_Pos         (22)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_WSDST_Msk         (0x3ul << CRPT_PRNG_KSCTL_WSDST_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSCTL_OWNER_Pos         (24)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSCTL_OWNER_Msk         (0x7ul << CRPT_PRNG_KSCTL_OWNER_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSSTS_NUM_Pos           (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSSTS_NUM_Msk           (0x1ful << CRPT_PRNG_KSSTS_NUM_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PRNG_KSSTS_KCTLERR_Pos       (16)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PRNG_KSSTS_KCTLERR_Msk       (0x1ul << CRPT_PRNG_KSSTS_KCTLERR_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_AES_KSCTL_NUM_Pos            (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_AES_KSCTL_NUM_Msk            (0x1ful << CRPT_AES_KSCTL_NUMC_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_AES_KSCTL_RSRC_Pos           (5)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_AES_KSCTL_RSRC_Msk           (0x1ul << CRPT_AES_KSCTL_RSRC_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_AES_KSCTL_RSSRC_Pos          (6)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_AES_KSCTL_RSSRC_Msk          (0x3ul << CRPT_AES_KSCTL_RSSRC_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_HMAC_KSCTL_NUM_Pos           (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_HMAC_KSCTL_NUM_Msk           (0x1ful << CRPT_HMAC_KSCTL_NUMC_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_HMAC_KSCTL_RSRC_Pos          (5)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_HMAC_KSCTL_RSRC_Msk          (0x1ul << CRPT_HMAC_KSCTL_RSRC_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_HMAC_KSCTL_RSSRC_Pos         (6)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_HMAC_KSCTL_RSSRC_Msk         (0x3ul << CRPT_HMAC_KSCTL_RSSRC_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_NUMK_Pos           (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_NUMK_Msk           (0x1ful << CRPT_ECC_KSCTL_NUMK_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_RSRCK_Pos          (5)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_RSRCK_Msk          (0x1ul << CRPT_ECC_KSCTL_RSRCK_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_RSSRCK_Pos         (6)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_RSSRCK_Msk         (0x3ul << CRPT_ECC_KSCTL_RSSRCK_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_ECDH_Pos           (14)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_ECDH_Msk           (0x1ul << CRPT_ECC_KSCTL_ECDH_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_TRUST_Pos          (16)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_TRUST_Msk          (0x1ul << CRPT_ECC_KSCTL_TRUST_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_XY_Pos             (20)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_XY_Msk             (0x1ul << CRPT_ECC_KSCTL_XY_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_WDST_Pos           (21)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_WDST_Msk           (0x1ul << CRPT_ECC_KSCTL_WDST_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSCTL_OWNER_Pos          (24)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSCTL_OWNER_Msk          (0x7ul << CRPT_ECC_KSCTL_OWNER_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSXY_NUMX_Pos            (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSXY_NUMX_Msk            (0x1ful << CRPT_ECC_KSXY_NUMX_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSXY_RSRCXY_Pos          (5)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSXY_RSRCXY_Msk          (0x1ul << CRPT_ECC_KSXY_RSRCXY_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSXY_RSSRCX_Pos          (6)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSXY_RSSRCX_Msk          (0x3ul << CRPT_ECC_KSXY_RSSRCX_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSXY_NUMY_Pos            (8)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSXY_NUMY_Msk            (0x1ful << CRPT_ECC_KSXY_NUMY_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_ECC_KSXY_RSSRCY_Pos          (14)                                             /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_ECC_KSXY_RSSRCY_Msk          (0x3ul << CRPT_ECC_KSXY_RSSRCY_Pos)              /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_RSA_KSCTL_NUM_Pos            (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_KSCTL_NUM_Msk            (0x1ful << CRPT_RSA_KSCTL_NUM_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_RSA_KSCTL_RSRC_Pos           (5)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_KSCTL_RSRC_Msk           (0x1ul << CRPT_RSA_KSCTL_RSRC_Pos)               /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_RSA_KSCTL_RSSRC_Pos          (6)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_RSA_KSCTL_RSSRC_Msk          (0x3ul << CRPT_RSA_KSCTLY_RSSRC_Pos)             /*!< CRPT ECC_RSA: BUSERR Mask               */

#define CRPT_PAP_CTL_PAPEN_Pos            (0)                                              /*!< CRPT ECC_RSA: BUSERR Position           */
#define CRPT_PAP_CTL_PAPEN_Msk            (0x1ul << CRPT_PAP_CTL_PAPEN_Pos)                /*!< CRPT ECC_RSA: BUSERR Mask               */


/**@}*/ /* CRPT_CONST CRYPTO */
/**@}*/ /* end of CRYPTO register group */
/**@}*/ /* end of REGISTER group */

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

#endif /* __CRYPTO_REG_H__ */
