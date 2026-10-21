/*
 * Copyright (c) 2018,2021,2025 BlackBerry Limited.
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

#ifndef _DCMD_SMBUS_H_INCLUDED
#define _DCMD_SMBUS_H_INCLUDED


/**
 * @brief SMBus Command IDs
 */
typedef enum _smbus_cmd_t {
    SMBUS_CMD_QUICK = 0,        /* Quick */
    SMBUS_CMD_BYTE = 1,         /* Send/Receive Byte */
    SMBUS_CMD_BYTE_DATA = 2,    /* Write/Read Byte */
    SMBUS_CMD_WORD_DATA = 3,    /* Write/Read Word */
    SMBUS_CMD_PROC_CALL = 4,    /* Process Call */
    SMBUS_CMD_BLOCK = 5,        /* Block Write/Read */
    SMBUS_CMD_I2C_READ = 6,     /* I2C Combined Read */
    SMBUS_CMD_BLOCK_PROC = 7,   /* Block Write - Block Read Process Call */

    SMBUS_CMD_FIRST_ENTRY = SMBUS_CMD_QUICK,
    SMBUS_CMD_LAST_ENTRY = SMBUS_CMD_BLOCK_PROC
} smbus_cmd_t;

/**
 * @brief SMBus Read/Write Access
 */
typedef enum _smbus_rw_t {
    SMBUS_RW_WRITE = 0,
    SMBUS_RW_READ = 1
} smbus_rw_t;

/**
 * @brief SMBus Command Options
 *
 *  Enable additional features:
 *      - Packet Error Check (PEC)
 *      - I2C Transaction Mode
 */
#define SMBUS_OPT_PEC_EN    (1 << 0)
#define SMBUS_OPT_I2C_EN    (1 << 1)

/**
 * @brief SMBus Transaction Command
 *
 * This is the command header for the devctl DCMD_SMBUS_CMD.
 * Note that the data to send or receive is expected to follow this header
 * such as:
 *      struct {
 *          smbus_devctl_cmd_t hdr;     // SMBus transcation command header
 *          uint8_t            data[0]; // SMBus data
 *      } smbus_msg_t;
 *
 *
 * Command rules
 *
 *  The fields cmd, rw, addr, and opt are mandatory for all the commands.
 *  The data buffer size must be equal or greater than `max(send_len, recv_len)`.
 *
 *  Here are the details for each command:
 *
 *  - Quick (SMBUS_CMD_QUICK)
 *      Set send_len = recv_len = 0.
 *
 *  - Send/Receive Byte (SMBUS_CMD_BYTE)
 *      On SMBUS_RW_WRITE, set send_len > 0, and set recv_len = 0.
 *      On SMBUS_RW_READ, set send_len = 0, and set recv_len > 0.
 *
 *  - Read/Write Byte (SMBUS_CMD_BYTE_DATA)
 *      On SMBUS_RW_WRITE, set send_len > 1, and set recv_len = 0.
 *      On SMBUS_RW_READ, set send_len = 1, and recv_len > 0.
 *
 *  - Read/Write Word (SMBUS_CMD_WORD_DATA)
 *      On SMBUS_RW_WRITE, set send_len > 2, and set recv_len = 0.
 *      On SMBUS_RW_READ, set send_len = 1, and recv_len > 1.
 *
 *  - Process Call (SMBUS_CMD_PROC_CALL)
 *      Command unsupported.
 *
 *  - Block Read/Write (SMBUS_CMD_BLOCK)
 *      On SMBUS_RW_WRITE, set send_len > 2, and set recv_len = 0.
 *      On SMBUS_RW_READ, set send_len = 1, and recv_len > 1.
 *
 *  - I2C Read (SMBUS_CMD_I2C_READ)
 *      SMBUS_RW_WRITE in unsupported.
 *      On SMBUS_RW_READ, set send_len = 1, and recv_len > 0.
 *
 *  - Block Write/Read Process Call (SMBUS_CMD_BLOCK_PROC)
 *      Command unsupported.
 */
typedef struct _smbus_devctl_cmd_t {
    smbus_cmd_t     cmd;        /* [in] SMBus Command ID */
    smbus_rw_t      rw;         /* [in] SMBus Read/Write Flag */
    unsigned int    addr;       /* [in] SMBus 7-bit Slave Address */
    unsigned int    opt;        /* [in] SMBus Command Options */
    size_t          send_len;   /* [in/out] Length of data to send */
    size_t          recv_len;   /* [in/out] Length of data to receive */
} smbus_devctl_cmd_t;

/**
 * @brief SMBus Devctl IDs
 */
#define DCMD_SMBUS_CMD         (__DIOTF(_DCMD_MISC, 0, smbus_devctl_cmd_t))


#endif /*_DCMD_SMBUS_H_INCLUDED*/

