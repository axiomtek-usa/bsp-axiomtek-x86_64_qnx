/*
 * Copyright (c) 2021,2025 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _REG_H_INCLUDED
#define _REG_H_INCLUDED


/*
 * SMBus PCI Configuration Space Registers
 */

/* Device Status (DS) */
#define SMUBS_DS            (0x06)
  #define SMBUS_DS_INTS         (1 << 3)    /* Interrupt Status */

/* Host Configuration (HCFG) */
#define SMBUS_HCFG          (0x40)
  #define SMBUS_HCFG_HSTEN      (1 << 0)    /* SMB Host Controller Enable */
  #define SMBUS_HCFG_SSEN       (1 << 1)    /* SMB SMI Enable */
  #define SMBUS_HCFG_I2CEN      (1 << 2)    /* I2C Mode Enable */
  #define SMBUS_HCFG_SSRESET    (1 << 3)    /* Soft SMBus Reset */
  #define SMBUS_HCFG_SDPWD      (1 << 4)    /* SPD Write Disable */


/*
 * SMBus Registers
 */

/* Host Status Register Address (HSTS) */
#define SMBUS_HSTS          (0x00)
  #define SMBUS_HSTS_HBSY       (1 << 0)    /* Host Busy */
  #define SMBUS_HSTS_INTR       (1 << 1)    /* Interrupt Completion */
  #define SMBUS_HSTS_DERR       (1 << 2)    /* Device Error */
  #define SMBUS_HSTS_BERR       (1 << 3)    /* Bus Error */
  #define SMBUS_HSTS_FAIL       (1 << 4)    /* Failed */
  #define SMBUS_HSTS_SMSTS      (1 << 5)    /* SMB Alert Status */
  #define SMBUS_HSTS_IUS        (1 << 6)    /* In Use Status */
  #define SMBUS_HSTS_BDS        (1 << 7)    /* Byte Done Status */

/* Host Control Register (HCTL) */
#define SMBUS_HCTL          (0x02)
  #define SMBUS_HCTL_INTREN     (1 << 0)    /* Interrupt Enable */
  #define SMBUS_HCTL_KILL       (1 << 1)    /* Kill/Stop Transactions */
  #define SMBUS_HCTL_SMB_CMD_MASK (7 << 2)
  #define SMBUS_HCTL_SMB_CMD(_cmd_) \
                                (((_cmd_) & 0x07) << 2) /* SMBus Command */
  #define SMBUS_HCTL_LAST_BYTE  (1 << 5)    /* Last Byte */
  #define SMBUS_HCTL_START      (1 << 6)    /* Start/Execute Command */
  #define SMBUS_HCTL_PEC_EN     (1 << 7)    /* PEC Enable */

/* Host Command Register (HCMD) */
#define SMBUS_HCMD          (0x03)

/* Transmit Slave Address Register (TSA) */
#define SMBUS_TSA           (0x04)
  #define SMBUS_TSA_RW_MASK     (1 << 0)
  #define SMBUS_TSA_RW(_rw_)    (((_rw_) & 0x01) << 0)
  #define SMBUS_TSA_ADDR_MASK   (0x7F << 1)
  #define SMBUS_TSA_ADDR(_a_)   (((_a_) & 0x7F) << 1)

/* Data 0 Register (HD0) */
#define SMBUS_HD0           (0x05)

/* Data 1 Register (HD1) */
#define SMBUS_HD1           (0x06)

/* Host Block Data (HBD) */
#define SMBUS_HDB           (0x07)

/* Packet Error Check Data Register (PEC) */
#define SMBUS_PEC           (0x08)

/* Receive Slave Address Register (RSA) */
#define SMBUS_RSA           (0x09)
  #define SMBUS_RSA_ADDR(_a_)   ((_a_) & 0x7F)  /* Slave Address */

/* Slave Data Register (SD) */
#define SMBUS_SD            (0x0A)

/* Auxiliary Status (AUXS) */
#define SMBUS_AUXS          (0x0C)
  #define SMBUS_AUXS_CRCE       (1 << 0)    /* CRC Error */

/* Auxiliary Control (AUXC) */
#define SMBUS_AUXC          (0x0D)
  #define SMBUS_AUXC_AAC        (1 << 0)    /* Automatically Append CRC */
  #define SMBUS_AUXC_E32B       (1 << 1)    /* Enable 32-byte Buffer */

/* SMLINK_PIN_CTL Register (SMLC) */
#define SMBUS_SMLC          (0x0E)

/* SMBUS_PIN_CTL Register (SMBC) */
#define SMBUS_SMBC          (0x0F)

/* Slave Status Register (SSTS) */
#define SMBUS_SSTS          (0x10)

/* Slave Command Register (SCMD) */
#define SMBUS_SCMD          (0x11)

/* Notify Device Address Register (NDA) */
#define SMBUS_NDA           (0x14)

/* Notify Data Low Byte Register (NDLB) */
#define SMBUS_NDLB          (0x16)

/* Notify Data High Byte Register (NDHB) */
#define SMBUS_NDHB          (0x17)


#endif /*_REG_H_INCLUDED*/

