/**************************************************************************//**
 * @file     trng_reg.h
 * @version  V1.00
 * @brief    TRNG register definition header file
 *
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __TRNG_REG_H__
#define __TRNG_REG_H__

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/** @addtogroup REGISTER Control Register
   @{
*/

/** @addtogroup True Random Number Generator (TRNG)
    Memory Mapped Structure for Cryptographic Accelerator
@{ */

typedef struct
{
    __O  uint32_t CTRL;                  /*!< [0x0000] cause DWC TRNG NIST to execute one of a number of actions.                                       */
    __IO uint32_t MODE;                  /*!< [0x0004] used to enable or disable certain run-time features within the DWC TRNG NIST.                                      */
    __IO uint32_t SMODE;                 /*!< [0x0008] used to enable or disable certain MISSION mode run-time features.                                       */
    __IO uint32_t STAT;                  /*!< [0x000C] allow the user to monitor the internal status of DWC TRNG NIST.                                       */
    __IO uint32_t IE;                    /*!< [0x0010] used to enable or disable interrupts within the DWC TRNG NIST.                                      */
    __IO uint32_t ISTAT;                 /*!< [0x0014] allow the user to monitor the interrupt contributions of DWC TRNG NIST                                        */
    __IO uint32_t ALARMS;                /*!< [0x0018] allow the user to monitor the source of critical alarms                                    */
    __IO uint32_t COREKIT_REL;           /*!< [0x001C] contain the static coreKit release information                                        */
    __IO uint32_t FEATURES;              /*!< [0x0020] return the state of carious build-time parameter values                                       */
    __IO uint32_t RAND[4];               /*!< [0x0024] ~ [0x0030] used by host to read the newly generated 128-bits random number                                       */
    __IO uint32_t NPA_DATA[16];          /*!< [0x0034] ~ [0x0070] holds Noise/Nounce/Personalization String/Additional Input                                       */
    __IO uint32_t SEED[12];              /*!< [0x0074] ! [0x00a0] Key Store Control Register                                       */
    __IO uint32_t TIME_TO_SEED;          /*!< [0x0000] Key Store Control Register                                       */
} TRNG_T;

/** @addtogroup TRNG_CONST TRNG Bit Field Definition
    Constant Definitions for TRNG Controller
@{ */

#define TRNG_CTRL_CMD_Pos                (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_CTRL_CMD_Msk                (0xful << TRNG_CTRL_CMD_Pos)                      /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_MODE_SEC_ALG_Pos            (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_MODE_SEC_ALG_Msk            (0x1ul << TRNG_MODE_SEC_ALG_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_MODE_PRED_RESET_Pos         (3)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_MODE_PRED_RESET_Msk         (0x1ul << TRNG_MODE_PRED_RESET_Pos)               /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_MODE_ADDIN_PRESENT_Pos      (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_MODE_ADDIN_PRESENT_Msk      (0x1ul << TRNG_MODE_ADDIN_PRESENT_Pos)            /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_MODE_KAT_VEC_Pos            (5)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_MODE_KAT_VEC_Msk            (0x3ul << TRNG_MODE_KAT_VEC_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_MODE_KAT_SEL_Pos            (7)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_MODE_KAT_SEL_Msk            (0x3ul << TRNG_MODE_KAT_SEL_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_SMODE_NONCE_Pos             (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_SMODE_NONCE_Msk             (0x1ul << TRNG_SMODE_NONCE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_SMODE_MISSION_MODE_Pos      (1)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_SMODE_MISSION_MODE_Msk      (0x1ul << TRNG_SMODE_MISSION_MODE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_SMODE_MAX_REJECTS_Pos       (2)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_SMODE_MAX_REJECTS_Msk       (0xfful << TRNG_SMODE_MAX_REJECTS_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_SMODE_INDIV_HT_DISABLE_Pos  (16)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_SMODE_INDIV_HT_DISABLE_Msk  (0xfful << TRNG_SMODE_INDIV_HT_DISABLE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_SMODE_NOISE_COLLECT_Pos     (31)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_SMODE_NOISE_COLLECT_Msk     (0x1ul << TRNG_SMODE_NOISE_COLLECT_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_LAST_CMD_Pos           (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_LAST_CMD_Msk           (0xful << TRNG_STAT_LAST_CMD_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_SEC_ALG_Pos            (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_SEC_ALG_Msk            (0x1ul << TRNG_STAT_SEC_ALG_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_NONCE_MODE_Pos         (5)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_NONCE_MODE_Msk         (0x1ul << TRNG_STAT_NONCE_MODE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_MISSION_MODE_Pos       (6)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_MISSION_MODE_Msk       (0x1ul << TRNG_STAT_MISSION_MODE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_DRBG_STATE_Pos         (7)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_DRBG_STATE_Msk         (0x3ul << TRNG_STAT_DRBG_STATE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_STARTUP_TEST_STUCK_Pos (9)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_STARTUP_TEST_STUCK_Msk (0x1ul << TRNG_STAT_STARTUP_TEST_STUCK_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_STARTUP_TEST_IN_PROG_Pos (10)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_STARTUP_TEST_IN_PROG_Msk (0x1ul << TRNG_STAT_STARTUP_TEST_IN_PROG_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_STAT_BUSY_Pos               (31)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_STAT_BUSY_Msk               (0x1ul << TRNG_STAT_BUSY_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_ZEROIZED_Pos             (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_ZEROIZED_Msk             (0x1ul << TRNG_IE_ZEROIZED_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_KAT_COMPLETED_Pos        (1)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_KAT_COMPLETED_Msk        (0x1ul << TRNG_IE_KAT_COMPLETED_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_NOISE_RDY_Pos            (2)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_NOISE_RDY_Msk            (0x1ul << TRNG_IE_NOISE_RDY_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_ALARMS_Pos               (3)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_ALARMS_Msk               (0x1ul << TRNG_IE_ALARMS_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_DONE_Pos                 (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_DONE_Msk                 (0x1ul << TRNG_IE_DONE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_IE_GLBL_Pos                 (31)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_IE_GLBL_Msk                 (0x1ul << TRNG_IE_GLBL_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ISTAT_ZEROIZED_Pos          (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ISTAT_ZEROIZED_Msk          (0x1ul << TRNG_ISTAT_ZEROIZED_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ISTAT_KAT_COMPLETED_Pos     (1)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ISTAT_KAT_COMPLETED_Msk     (0x1ul << TRNG_ISTAT_KAT_COMPLETED_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ISTAT_NOISE_RDY_Pos         (2)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ISTAT_NOISE_RDY_Msk         (0x1ul << TRNG_ISTAT_NOISE_RDY_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ISTAT_ALARMS_Pos            (3)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ISTAT_ALARMS_Msk            (0x1ul << TRNG_ISTAT_ALARMS_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ISTAT_DONE_Pos              (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ISTAT_DONE_Msk              (0x1ul << TRNG_ISTAT_DONE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ALARMS_FAILED_TEST_ID_Pos   (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ALARMS_FAILED_TEST_ID_Msk   (0xful << TRNG_ALARMS_FAILED_TEST_ID_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ALARMS_ILLEGAL_CMD_SEQ_Pos  (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ALARMS_ILLEGAL_CMD_SEQ_Msk   (0xful << TRNG_ALARMS_ILLEGAL_CMD_SEQ_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_ALARMS_FAILED_SEED_ST_HT_Pos    (5)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_ALARMS_FAILED_SEED_ST_HT_Msk    (0xful << TRNG_ALARMS_FAILED_SEED_ST_HT_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_COREKIT_REL_REL_NUM_Pos     (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_COREKIT_REL_REL_NUM_Msk     (0xfffful << TRNG_COREKIT_REL_REL_NUM_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_COREKIT_REL_EXT_VER_Pos     (16)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_COREKIT_REL_EXT_VER_Msk     (0xfful << TRNG_COREKIT_REL_EXT_VER_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_COREKIT_REL_EXT_ENUM_Pos    (28)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_COREKIT_REL_EXT_ENUM_Msk    (0xful << TRNG_COREKIT_REL_EXT_ENUM_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_SECURE_RST_STATE_Pos   (0)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_SECURE_RST_STATE_Msk   (0x1ul << TRNG_FEATURES_SECURE_RST_STATE_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_DIAG_LEVEL_ST_HLT_Pos  (1)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_DIAG_LEVEL_ST_HLT_Msk  (0x7ul << TRNG_FEATURES_DIAG_LEVEL_ST_HLT_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_DIAG_LEVEL_CLP800_Pos  (4)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_DIAG_LEVEL_CLP800_Msk  (0x7ul << TRNG_FEATURES_DIAG_LEVEL_CLP800_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_DIAG_LEVEL_NS_Pos  (7)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_DIAG_LEVEL_NS_Msk  (0x1ul << TRNG_FEATURES_DIAG_LEVEL_NS_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_PS_PRESENT_Pos     (8)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_PS_PRESENT_Msk     (0x1ul << TRNG_FEATURES_PS_PRESENT_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

#define TRNG_FEATURES_AES_256_Pos        (9)                                               /*!< CRPT_T::INTEN: AESIEN Position         */
#define TRNG_FEATURES_AES_256_Msk        (0x1ul << TRNG_FEATURES_AES_256_Pos)                  /*!< CRPT_T::INTEN: AESIEN Mask             */

/**@}*/ /* TRNG_CONST */
/**@}*/ /* end of TRNG register group */
/**@}*/ /* end of REGISTER group */

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

#endif
