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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <devctl.h>
#include <getopt.h>

#include <hw/dcmd_smbus.h>


/**
 * @brief SMBus resource manager pathname
 */
static const char *smbus_resmgr_path = "/dev/smb0";

/**
 * @brief Transaction configuration set by command line options
 */
static unsigned int slave_addr = 0;     /* SMBus slave address */
static size_t recv_len = 0;             /* Number of bytes to read */
static size_t send_len = 0;             /* Number of bytes to write */
static int pec_en = 0;                  /* Packet Error Code (PEC) flag */
static int i2c_en = 0;                  /* I2C mode flag */
static int i2c_read = 0;                /* I2C read flag */
static int raw_output = 0;              /* Enable unformatted raw output */

/**
 * @brief Verbosity level
 */
static unsigned int verbosity = 0;

/**
 * @brief SMBus devctl message container
 */
typedef struct _smbus_msg_t {
    smbus_devctl_cmd_t  hdr;
    uint8_t             data[0];
} smbus_msg_t;

/**
 * @brief Process the command line options
 *
 * @param argc      [in] Number of command line arguments
 * @param argv      [in] Array of command line arguments
 * @return int      0 on success, -1 otherwise
 */
static int parse_options(const int argc, char *argv[])
{
    static const struct option longopts[] = {
        { .name = "addr",     .has_arg = required_argument, .flag = NULL,        .val = 'a' },
        { .name = "len",      .has_arg = required_argument, .flag = NULL,        .val = 'l' },
        { .name = "path",     .has_arg = required_argument, .flag = NULL,        .val = 'n' },
        { .name = "verbose",  .has_arg = required_argument, .flag = NULL,        .val = 'v' },
        { .name = "i2c-en",   .has_arg = no_argument,       .flag = &i2c_en,     .val = 1 },
        { .name = "i2c-read", .has_arg = no_argument,       .flag = &i2c_read,   .val = 1 },
        { .name = "pec",      .has_arg = no_argument,       .flag = &pec_en,     .val = 1 },
        { .name = "raw",      .has_arg = no_argument,       .flag = &raw_output, .val = 1 },
        { .name = NULL,       .has_arg = 0,                 .flag = NULL,        .val = 0 }
    };
    int op;
    int long_idx;

    while ((op = getopt_long(argc, argv, "a:l:n:v", longopts, &long_idx)) != -1) {

        switch (op) {
            case 'a':
                if(optarg != NULL) {
                    errno = EOK;
                    slave_addr = (unsigned int)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'l':
                if(optarg != NULL) {
                    errno = EOK;
                    recv_len = (size_t)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'n':
                if(optarg != NULL) {
                    smbus_resmgr_path = strdup(optarg);
                }
                break;
            case 'v':
                if (optarg != NULL) {
                    errno = EOK;
                    verbosity = (unsigned int)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                } else {
                    verbosity++;
                }
                break;
            case 0:
                /* Catch long options */
                break;
            default:
                return -1;
        }
    }

    /* Set the number of bytes to send */
    send_len = (size_t)(argc-optind);

    return 0;
}

/**
 * @brief Verify the options and determine the transaction command
 *
 * @param cmd       [out] Pointer to the transaction command
 * @param rw        [out] Pointer to the transaction access (read/write)
 * @return int      0 on success, -1 otherwise
 */
static int verify_options(smbus_cmd_t *cmd, smbus_rw_t *rw)
{
    /* Sanity check */
    if ((cmd == NULL) || (rw == NULL)) {
        return -1;
    }

    /* Ensure that the address was specified */
    if (slave_addr == 0) {
        (void)fprintf(stderr, "Error: the slave address is missing!\n");
        return -1;
    }

    /* Ensure that PEC and I2C are not enabled simultaneously */
    if ((pec_en == 1)
        && ((i2c_en == 1) || (i2c_read == 1))) {
        (void)fprintf(stderr, "Error: PEC and I2C are mutually exclusive!\n");
        return -1;
    }

    /*
     * Determine the transaction command and access
     * by checking the transaction lengths and PEC option
     */

    /*
     * Name: I2C Read (SMBUS_CMD_I2C_READ)
     * Rules:
     *  - Write
     *      Unsupported
     *  - Read
     *      send_len = 1
     *      recv_len >= 1
     */
    if (i2c_read == 1)
    {
        if ((send_len == 1) && (recv_len >= 1)) {
            *cmd = SMBUS_CMD_I2C_READ;
            *rw  = SMBUS_RW_READ;
            return 0;
        }
    }

    /*
     * Name: Block Read/Write (SMBUS_CMD_BLOCK)
     * Rules:
     *  - Write
     *      if PEC_EN:  send_len >= 4
     *      if I2C_EN:  send_len >= 2
     *      else:       send_len >= 3
     *      recv_len = 0
     *  - Read
     *      send_len = 1
     *      if PEC_EN:  recv_len >= 3
     *      if I2C_EN:  recv_len >= 1
     *      else:       recv_len >= 2
     */
    else
    {
        if (recv_len == 0) {
            if (((send_len >= 4) && (pec_en == 1))
                || ((send_len >= 3) && (pec_en == 0))
                || ((send_len >= 2) && (pec_en == 0) && (i2c_en == 1))) {
                *cmd = SMBUS_CMD_BLOCK;
                *rw  = SMBUS_RW_WRITE;
                return 0;
            }
        }
        if (send_len == 1) {
            if (((recv_len >= 3) && (pec_en == 1))
                || ((recv_len >= 2) && (pec_en == 0))
                || ((recv_len >= 1) && (pec_en == 0) && (i2c_en == 1))) {
                *cmd = SMBUS_CMD_BLOCK;
                *rw  = SMBUS_RW_READ;
                return 0;
            }
        }
    }

    /* Unsupported transaction format */
    (void)fprintf(stderr, "Error: Unsupported transaction format!\n");
    return -1;
}

/**
 * @brief Main routine of the SMBus block utility
 *
 * @param argc      [in] Number of command line arguments
 * @param argv      [in] Array of command line arguments
 * @return int      EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(const int argc, char *argv[])
{
    int err = EOK;
    int fd = -1;
    unsigned int idx;

    smbus_msg_t *msg = NULL;
    size_t msg_len = 0;
    smbus_cmd_t command;
    smbus_rw_t rw;

    /* Get the command line options */
    if (parse_options(argc, argv) < 0) {
        return EXIT_FAILURE;
    }

    /* Verify the options and determine the transaction command */
    if (verify_options(&command, &rw) < 0) {
        return EXIT_FAILURE;
    }

    /* Open the SMBus device for read and write transactions */
    fd = open(smbus_resmgr_path, O_RDWR);
    if (fd < 0) {
        (void)fprintf(stderr, "Error: Unable to open %s: %s\n", smbus_resmgr_path, strerror(errno));
        return EXIT_FAILURE;
    }

    /* Determine the message length and allocate resources for the message */
    msg_len = max(send_len, recv_len);
    msg = calloc(1, sizeof(*msg) + msg_len);
    if (msg == NULL) {
        (void)fprintf(stderr, "Error: Message allocation failed with %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* Configure the SMBus devctl header */
    msg->hdr.cmd = command;
    msg->hdr.addr = slave_addr;
    msg->hdr.rw = rw;
    msg->hdr.send_len = send_len;
    msg->hdr.recv_len = recv_len;
    msg->hdr.opt |= (pec_en == 1) ? (unsigned int)SMBUS_OPT_PEC_EN : 0;
    msg->hdr.opt |= (i2c_en == 1) ? (unsigned int)SMBUS_OPT_I2C_EN : 0;

    /* Fill the message buffer with the SMBus data to send */
    for (idx = (unsigned int)optind; idx < (unsigned int)argc; idx++) {
        msg->data[idx - (unsigned int)optind] = (uint8_t)strtoul(argv[idx], NULL, 0);
    }

    /* Display the data to send */
    if ((verbosity > 1) && (send_len > 0)) {
        printf("Data Sent: ");
        for (idx = 0; idx < send_len; idx++) {
            printf("%02x ", msg->data[idx]);
        }
        printf("\n");
    }

    /* Execute the SMBus devctl command */
    err = devctl(fd, DCMD_SMBUS_CMD, msg, sizeof(*msg) + msg_len, NULL);
    if (err != EOK) {
        (void)fprintf(stderr, "Error: The command %d failed with %s\n", command, strerror(err));
        return EXIT_FAILURE;
    }

    /* Display the received data */
    if (msg->hdr.recv_len > 0) {
        if (verbosity > 0) {
            printf("Data Received: ");
            if (msg->hdr.recv_len > 16) {
                printf("\n");
            }
        }
        for (idx = 0; idx < msg->hdr.recv_len; idx++) {
            if (raw_output) {
                printf("%02x", msg->data[idx]);
            } else {
                printf("%02x ", msg->data[idx]);
                if ((idx + 1)%16 == 0) {
                    printf("\n");
                }
            }
        }
        if ((!raw_output) && (idx%16 != 0)) {
            printf("\n");
        }
    }

    /* Check for message corruption */
    if ((msg->hdr.cmd != command)
        || (msg->hdr.addr != slave_addr)
        || (msg->hdr.rw != rw)
        || (msg->hdr.send_len != send_len)
        || (msg->hdr.recv_len != recv_len)) {
        fprintf(stderr, "Error: The message header is corrupted!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

