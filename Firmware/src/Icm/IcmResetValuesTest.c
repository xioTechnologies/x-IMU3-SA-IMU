/**
 * @file IcmResetValuesTest.c
 * @author Seb Madgwick
 * @brief ICM-45686 reset values test.
 */

//------------------------------------------------------------------------------
// Includes

#include "IcmRegisters.h"
#include "IcmResetValuesTest.h"
#include <stdio.h>

//------------------------------------------------------------------------------
// Function declarations

static void TestNormal(const IcmResetValuesTestCallbacks * const callbacks, const char* const name, const uint8_t address, const uint8_t expected);
static void TestIndirect(const IcmResetValuesTestCallbacks * const callbacks, const char* const name, const uint16_t address, const uint8_t expected);
static void SoftReset(const IcmResetValuesTestCallbacks * const callbacks);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Tests register reset values.
 * @param callbacks Callbacks.
 */
void IcmResetValuesTest(const IcmResetValuesTestCallbacks * const callbacks) {

    // Normal registers
    TestNormal(callbacks, "ICM_PWR_MGMT0", ICM_PWR_MGMT0_ADDRESS, ICM_PWR_MGMT0_RESET_VALUE);
    TestNormal(callbacks, "ICM_INT1_CONFIG0", ICM_INT1_CONFIG0_ADDRESS, ICM_INT1_CONFIG0_RESET_VALUE);
    TestNormal(callbacks, "ICM_INT1_CONFIG2", ICM_INT1_CONFIG2_ADDRESS, ICM_INT1_CONFIG2_RESET_VALUE);
    TestNormal(callbacks, "ICM_ACCEL_CONFIG0", ICM_ACCEL_CONFIG0_ADDRESS, ICM_ACCEL_CONFIG0_RESET_VALUE);
    TestNormal(callbacks, "ICM_GYRO_CONFIG0", ICM_GYRO_CONFIG0_ADDRESS, ICM_GYRO_CONFIG0_RESET_VALUE);
    TestNormal(callbacks, "ICM_WHO_AM_I", ICM_WHO_AM_I_ADDRESS, ICM_WHO_AM_I_RESET_VALUE);
    TestNormal(callbacks, "ICM_REG_MISC2", ICM_REG_MISC2_ADDRESS, ICM_REG_MISC2_RESET_VALUE);

    // Indirect registers
    TestIndirect(callbacks, "ICM_IPREG_SYS1_REG_172", ICM_IPREG_SYS1_REG_172_ADDRESS, ICM_IPREG_SYS1_REG_172_RESET_VALUE);
    TestIndirect(callbacks, "ICM_IPREG_SYS2_REG_131", ICM_IPREG_SYS2_REG_131_ADDRESS, ICM_IPREG_SYS2_REG_131_RESET_VALUE);
}

/**
 * @brief Tests normal register reset value.
 * @param callbacks Callbacks.
 * @param name Register name.
 * @param address Register address.
 * @param expected Expected reset value.
 */
static void TestNormal(const IcmResetValuesTestCallbacks * const callbacks, const char* const name, const uint8_t address, const uint8_t expected) {
    SoftReset(callbacks);
    const uint8_t actual = callbacks->readRegister(address);
    printf("%-32s   %02X %02X %02X %s\n", name, address, expected, actual, actual == expected ? "Ok" : "Error");
}

/**
 * @brief Tests indirect register reset value.
 * @param callbacks Callbacks.
 * @param name Register name.
 * @param address Register address.
 * @param expected Expected reset value.
 */
static void TestIndirect(const IcmResetValuesTestCallbacks * const callbacks, const char* const name, const uint16_t address, const uint8_t expected) {
    SoftReset(callbacks);
    const uint8_t actual = callbacks->readIndirectRegister(address);
    printf("%-32s %04X %02X %02X %s\n", name, address, expected, actual, actual == expected ? "Ok" : "Error");
}

/**
 * @brief Performs a soft reset.
 * @param callbacks Callbacks.
 */
static void SoftReset(const IcmResetValuesTestCallbacks * const callbacks) {
    IcmRegMisc2Register icmRegMisc2Register = {.value = ICM_REG_MISC2_RESET_VALUE};
    icmRegMisc2Register.softRst = 1;
    callbacks->writeRegister(ICM_REG_MISC2_ADDRESS, icmRegMisc2Register.value);
    callbacks->delayMilliseconds(1);
}

//------------------------------------------------------------------------------
// End of file
