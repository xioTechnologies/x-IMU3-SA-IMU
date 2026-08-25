/**
 * @file IcmResetValuesTest.h
 * @author Seb Madgwick
 * @brief ICM-45686 reset values test.
 */

#ifndef ICM_RESET_VALUES_TEST_H
#define ICM_RESET_VALUES_TEST_H

//------------------------------------------------------------------------------
// Includes

#include <stdint.h>

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Callbacks.
 */
typedef struct {
    uint8_t(*readRegister)(const uint8_t address);
    uint8_t(*readIndirectRegister)(const uint16_t address);
    void (*writeRegister)(const uint8_t address, const uint8_t value);
    void (*delayMilliseconds)(const uint32_t milliseconds);
} IcmResetValuesTestCallbacks;

//------------------------------------------------------------------------------
// Function declarations

void IcmResetValuesTest(const IcmResetValuesTestCallbacks * const callbacks);

#endif

//------------------------------------------------------------------------------
// End of file
