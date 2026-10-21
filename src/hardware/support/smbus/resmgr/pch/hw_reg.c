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

#include <hw/inout.h>

#include "smbus_internal.h"
#include "log.h"
#include "hw.h"


/**
 * @brief Read 8-bit SMBus register
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @return uint8_t  SMBus controller register value
 */
uint8_t hw_reg_read8(smbus_ctrl_t *const ctrl, const unsigned int reg)
{
    assert(ctrl != NULL);
    assert(ctrl->base.addr != NULL);
    if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
        return in8((uint16_t)((uintptr_t)ctrl->base.addr + reg));
    } else {
        return *(ctrl->base.addr + reg);
    }
}

/**
 * @brief Read 16-bit SMBus register
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @return uint16_t SMBus controller register value
 */
uint16_t hw_reg_read16(smbus_ctrl_t *const ctrl, const unsigned int reg)
{
    assert(ctrl != NULL);
    assert(ctrl->base.addr != NULL);
    if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
        return in16((uint16_t)((uintptr_t)ctrl->base.addr + reg));
    } else {
        return *(volatile uint16_t*)(ctrl->base.addr + reg);
    }
}

/**
 * @brief Write 8-bit SMBus register
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @param data      Value to write to SMBus controller register
 */
void hw_reg_write8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t data)
{
    assert(ctrl != NULL);
    assert(ctrl->base.addr != NULL);
    if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
        out8((uint16_t)((uintptr_t)ctrl->base.addr + reg), data);
    } else {
        *(ctrl->base.addr + reg) = data;
    }
}

/**
 * @brief Write 16-bit SMBus register
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @param data      Value to write to SMBus controller register
 */
void hw_reg_write16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t data)
{
    assert(ctrl != NULL);
    assert(ctrl->base.addr != NULL);
    if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
        out16((uint16_t)((uintptr_t)ctrl->base.addr + reg), data);
    } else {
        *(volatile uint16_t*)(ctrl->base.addr + reg) = data;
    }
}

/**
 * @brief Write 8-bit SMBus register with mask and data
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @param mask      Mask for the value to write
 * @param data      Value to write to SMBus controller register
 */
void hw_reg_write_mask8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t mask, const uint8_t data)
{
    hw_reg_write8(ctrl, reg, (uint8_t)((hw_reg_read8(ctrl, reg) & ~mask) | (data & mask)));
}

/**
 * @brief Write 16-bit SMBus register with mask and data
 *
 * @param ctrl      SMBus controller context
 * @param reg       SMBus controller register offset
 * @param mask      Mask for the value to write
 * @param data      Value to write to SMBus controller register
 */
void hw_reg_write_mask16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t mask, const uint16_t data)
{
    hw_reg_write16(ctrl, reg, (uint16_t)((hw_reg_read16(ctrl, reg) & ~mask) | (data & mask)));
}

