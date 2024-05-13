/**
 * Copyright 2022 Charly Delay <charly@codesink.dev> (@0xcharly)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#undef FORCE_NKRO
#define FORCE_NKRO
#define ONESHOT_TAP_TOGGLE 4  /* Tapping this number of times holds the key until tapped once again. */
#define ONESHOT_TIMEOUT 5000
#define DIODE_DIRECTION COL2ROW
#define OS_DETECTION_KEYBOARD_RESET
#define MATRIX_ROWS 12 // Rows are doubled-up
#define MATRIX_COLS 6
#define BOOTMAGIC_ROW 0
#define BOOTMAGIC_COLUMN 1
/* Key matrix configuration. */
#define MATRIX_ROW_PINS \
    { GP13, GP8, GP7, GP6, GP3, GP2 }
#define MATRIX_COL_PINS \
    { GP29, GP28, GP27, GP26, GP15, GP14 }

/* Handedness. */
#define SPLIT_HAND_PIN GP25
#define SPLIT_HAND_PIN_LOW_IS_LEFT // High -> right, Low -> left.

/* USART */
#define SERIAL_USART_SPEED 921600
#define SERIAL_USART_FULL_DUPLEX   // Enable full duplex operation mode.
#define SERIAL_USART_TX_PIN GP0     // USART TX pin
#define SERIAL_USART_RX_PIN GP1
/* PMW3360 */
#define PMW33XX_CS_PIN                       GP9
#define PMW33XX_CPI                          7200
#define POINTING_DEVICE_INVERT_Y
#define POINTING_DEVICE_INVERT_Y_RIGHT
#define SPLIT_POINTING_ENABLE
#define POINTING_DEVICE_COMBINED
/* I2C */
#define I2C_DRIVER        I2CD0
#define I2C1_SCL_PIN      GP5
#define I2C1_SDA_PIN      GP4
#define I2C1_CLOCK_SPEED  400000
/* HAPTIC DRV */
#define SPLIT_HAPTIC_ENABLE
#define HAPTIC_OFF_IN_LOW_POWER 1
#define DRV2605L_GREETING DRV2605L_EFFECT_CLEAR_SEQUENCE
#define DRV2605L_DEFAULT_MODE DRV2605L_EFFECT_CLEAR_SEQUENCE
#define DRV2605L_FB_ERM_LRA         0
/* Please refer to your datasheet for the optimal setting for your specific motor. */
#define DRV2605L_FB_BRAKEFACTOR     6 /* For 1x:0, 2x:1, 3x:2, 4x:3, 6x:4, 8x:5, 16x:6, Disable Braking:7 */
#define DRV2605L_FB_LOOPGAIN        0 /* For  Low:0, Medium:1, High:2, Very High:3 */
#define DRV2605L_RATED_VOLTAGE      3
#define DRV2605L_V_PEAK             3.3

#define NO_HAPTIC_ALPHA
#define NO_HAPTIC_PUNCTUATION
#define NO_HAPTIC_LOCKKEYS
#define NO_HAPTIC_NUMERIC
#define HAPTIC_FEEDBACK_DEFAULT

#define SPI_DRIVER SPID1
#define SPI_SCK_PIN GP10
#define SPI_MOSI_PIN GP11
#define SPI_MISO_PIN GP12

/* Reset. */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 1000U
