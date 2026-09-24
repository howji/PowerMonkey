/*******************************************************************************
*  ______                            ______                 _
* (_____ \                          |  ___ \               | |
*  _____) )___   _ _ _   ____   ___ | | _ | |  ___   ____  | |  _  ____  _   _
* |  ____// _ \ | | | | / _  ) / __)| || || | / _ \ |  _ \ | | / )/ _  )| | | |
* | |    | |_| || | | |( (/ / | |   | || || || |_| || | | || |< (( (/ / | |_| |
* |_|     \___/  \____| \____)|_|   |_||_||_| \___/ |_| |_||_| \_)\____) \__  |
*                                                                       (____/
* Copyright (C) 2021-2022 Ivan Dimkovic. All rights reserved.
*
* All trademarks, logos and brand names are the property of their respective
* owners. All company, product and service names used are for identification
* purposes only. Use of these names, trademarks and brands does not imply
* endorsement.
*
* SPDX-License-Identifier: Apache-2.0
* Full text of the license is available in project root directory (LICENSE)
*
* WARNING: This code is a proof of concept for educative purposes. It can
* modify internal computer configuration parameters and cause malfunctions or
* even permanent damage. It has been tested on a limited range of target CPUs
* and has minimal built-in failsafe mechanisms, thus making it unsuitable for
* recommended use by users not skilled in the art. Use it at your own risk.
*
*******************************************************************************/

#pragma once

#include "Platform.h"

/*******************************************************************************
 * MSRs
 ******************************************************************************/

#define MSR_OC_MAILBOX                  0x150
#define MSR_FLEX_RATIO                  0x194
#define MSR_TURBO_RATIO_LIMIT           0x1AD

/*******************************************************************************
 * OC Mailbox - Thermal Velocity Boost (TVB)
 *
 * !!! WARNING - VERIFY BEFORE USE !!!
 * The OC Mailbox command ID and data-bit layout used to configure TVB are NOT
 * part of Intel's public documentation - they are taken from FSP UPDs
 * (TvbRatioClipping / TvbVoltageOptimization) and community reverse-engineering
 * and MUST be confirmed against your CPU's BWG / FSP before enabling TVB
 * programming. The OC Mailbox rejects unsupported commands (status is checked),
 * so a wrong command ID fails gracefully rather than mis-programming the CPU.
 ******************************************************************************/

#define OC_MBOX_CMD_TVB                 0x14   // <-- VERIFY for your CPU / FSP
#define OC_TVB_RATIO_CLIPPING_DISABLE   bit0u32  // 1 = disable ratio clipping
#define OC_TVB_VOLTAGE_OPT_DISABLE      bit1u32  // 1 = disable voltage optim.
#define MSR_TURBO_RATIO_LIMIT_ECORE     0x650
#define MSR_POWER_CONTROL               0x1FC
#define MSR_VR_CURRENT_CONFIG           0x601
#define MSR_PACKAGE_POWER_SKU_UNIT      0x606
#define MSR_PACKAGE_POWER_LIMIT         0x610
#define MSR_PKG_POWER_INFO              0x614
#define MSR_PL3_CONTROL                 0x615
#define MSR_PP0_POWER_LIMIT             0x638
#define MSR_PLATFORM_POWER_LIMIT        0x65C
#define MSR_CONFIG_TDP_CONTROL          0x64B

 /*******************************************************************************
  * MCHBAR Registers
  ******************************************************************************/

#define MMIO_PACKAGE_POWER_LIMIT        0x59A0
#define MMIO_PACKAGE_POWER_LIMIT_HI     0x59A4

/*******************************************************************************
 *
 ******************************************************************************/

EFI_STATUS EFIAPI IAPERF_ProbeDomainVF(
  IN const UINT8 domain, OUT DOMAIN *desc );

/*******************************************************************************
 *
 ******************************************************************************/

EFI_STATUS EFIAPI IAPERF_ProgramDomainVF(IN const UINT8 domIdx,
  IN OUT DOMAIN* dom, 
  IN const UINT8 programVfPoints,
  IN const UINT8 programIccMax);

/*******************************************************************************
 *
 ******************************************************************************/

VOID IaCore_OcLock(VOID);

/*******************************************************************************
 * IaCore_ProgramTvb
 ******************************************************************************/

EFI_STATUS EFIAPI IaCore_ProgramTvb(IN const UINT8 enableTvb);
