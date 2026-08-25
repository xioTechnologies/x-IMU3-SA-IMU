/**
 * @file IcmRegisters.h
 * @author Seb Madgwick
 * @brief ICM-45686 registers.
 */

#ifndef ICM_REGISTERS_H
#define ICM_REGISTERS_H

//------------------------------------------------------------------------------
// Includes

#include <stdint.h>

//------------------------------------------------------------------------------
// Definitions - Register bank 0

#define ICM_ACCEL_DATA_X1_UI_ADDRESS         (0x00)
#define ICM_PWR_MGMT0_ADDRESS                (0x10)
#define ICM_INT1_CONFIG0_ADDRESS             (0x16)
#define ICM_INT1_CONFIG2_ADDRESS             (0x18)
#define ICM_ACCEL_CONFIG0_ADDRESS            (0x1B)
#define ICM_GYRO_CONFIG0_ADDRESS             (0x1C)
#define ICM_WHO_AM_I_ADDRESS                 (0x72)
#define ICM_IREG_ADDR_15_8_ADDRESS           (0x7C)
#define ICM_IREG_DATA_ADDRESS                (0x7E)
#define ICM_REG_MISC2_ADDRESS                (0x7F)

#define ICM_PWR_MGMT0_RESET_VALUE            (0x00)
#define ICM_INT1_CONFIG0_RESET_VALUE         (0x80)
#define ICM_INT1_CONFIG2_RESET_VALUE         (0x04)
#define ICM_ACCEL_CONFIG0_RESET_VALUE        (0x06)
#define ICM_GYRO_CONFIG0_RESET_VALUE         (0x06)
#define ICM_WHO_AM_I_RESET_VALUE             (0xE9)
#define ICM_REG_MISC2_RESET_VALUE            (0x01)

typedef struct {
    int16_t accelDataX;
    int16_t accelDataY;
    int16_t accelDataZ;
    int16_t gyroDataX;
    int16_t gyroDataY;
    int16_t gyroDataZ;
    int16_t tempData;
} __attribute__((__packed__)) IcmSensorRegisters;

typedef union {

    struct {
        unsigned accelMode : 2;
        unsigned gyroMode : 2;
        unsigned : 4;
    } __attribute__((__packed__));
    uint8_t value;
} IcmPwrMgmt0Register;

typedef union {

    struct {
        unsigned int1StatusEnFifoFull : 1;
        unsigned int1StatusEnFifoThs : 1;
        unsigned int1StatusEnDrdy : 1;
        unsigned int1StatusEnAux1Drdy : 1;
        unsigned int1StatusEnApFsync : 1;
        unsigned int1StatusEnApAgcRdy : 1;
        unsigned int1StatusEnAux1AgcRdy : 1;
        unsigned int1StatusEnResetDone : 1;
    } __attribute__((__packed__));
    uint8_t value;
} IcmInt1Config0Register;

typedef union {

    struct {
        unsigned int1Polarity : 1;
        unsigned int1Mode : 1;
        unsigned int1Drive : 1;
        unsigned : 5;
    } __attribute__((__packed__));
    uint8_t value;
} IcmInt1Config2Register;

typedef union {

    struct {
        unsigned accelOdr : 4;
        unsigned accelUiFsSel : 3;
        unsigned : 1;
    } __attribute__((__packed__));
    uint8_t value;
} IcmAccelConfig0Register;

typedef union {

    struct {
        unsigned gyroOdr : 4;
        unsigned gyroUiFsSel : 4;
    } __attribute__((__packed__));
    uint8_t value;
} IcmGyroConfig0Register;

typedef union {

    struct {
        unsigned iregDone : 1;
        unsigned softRst : 1;
        unsigned : 6;
    } __attribute__((__packed__));
    uint8_t value;
} IcmRegMisc2Register;

//------------------------------------------------------------------------------
// Definitions - Register bank IPREG_SYS1

#define ICM_IPREG_SYS1_ADDRESS               (0x00A4)
#define ICM_IPREG_SYS1_REG_172_ADDRESS       (0xAC00 | ICM_IPREG_SYS1_ADDRESS)

#define ICM_IPREG_SYS1_REG_172_RESET_VALUE   (0x80)

typedef union {

    struct {
        unsigned gyroUiLpfbwSel : 3;
        unsigned : 4;
        unsigned gyroOisHpf1Byp : 1;
    } __attribute__((__packed__));
    uint8_t value;
} IcmIpregSys1Reg172Register;

//------------------------------------------------------------------------------
// Definitions - Register bank IPREG_SYS2

#define ICM_IPREG_SYS2_ADDRESS               (0x00A5)
#define ICM_IPREG_SYS2_REG_131_ADDRESS       (0x8300 | ICM_IPREG_SYS2_ADDRESS)

#define ICM_IPREG_SYS2_REG_131_RESET_VALUE   (0x00)

typedef union {

    struct {
        unsigned accelUiLpfbwSel : 3;
        unsigned : 5;
    } __attribute__((__packed__));
    uint8_t value;
} IcmIpregSys2Reg131Register;

#endif

//------------------------------------------------------------------------------
// End of file
