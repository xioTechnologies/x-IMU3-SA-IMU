/**
 * @file AsciiMessage.c
 * @author Seb Madgwick
 * @brief ASCII messages.
 */

//------------------------------------------------------------------------------
// Includes

#include "AsciiMessage.h"
#include <stdint.h>

//------------------------------------------------------------------------------
// Function declarations

static inline __attribute__((always_inline)) void WriteFloat(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const float value);
static inline __attribute__((always_inline)) void WriteString(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const char* string);
static inline __attribute__((always_inline)) void WriteComma(void* const destination, const size_t destinationSize, size_t * const destinationIndex);
static inline __attribute__((always_inline)) void WriteTermination(void* const destination, const size_t destinationSize, size_t * const destinationIndex);
static inline __attribute__((always_inline)) void WriteChar(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const char character);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Writes an ASCII message of floats.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param floats Floats.
 * @param numberOfFloats Number of floats.
 * @return Message size.
 */
size_t AsciiMessageFloats(void* const destination, const size_t destinationSize, const float* const floats, const int numberOfFloats) {
    size_t destinationIndex = 0;
    for (int index = 0; index < numberOfFloats; index++) {
        if (index > 0) {
            WriteComma(destination, destinationSize, &destinationIndex);
        }
        WriteFloat(destination, destinationSize, &destinationIndex, floats[index]);
    }
    WriteTermination(destination, destinationSize, &destinationIndex);
    return destinationIndex;
}

/**
 * @brief Writes a float.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param destinationIndex Destination index.
 * @param value Value.
 */
static inline __attribute__((always_inline)) void WriteFloat(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const float value) {

    // Limits
    if (value >= 999999.9999f) {
        WriteString(destination, destinationSize, destinationIndex, "999999.9999");
        return;
    }
    if (value <= -999999.9999f) {
        WriteString(destination, destinationSize, destinationIndex, "-999999.9999");
        return;
    }

    // Sign
    float absolute = value;
    if (value < 0.0f) {
        WriteChar(destination, destinationSize, destinationIndex, '-');
        absolute = -value;
    }

    // Integer part
    const uint32_t integer = (uint32_t) absolute;
    if (integer == 0) {
        WriteChar(destination, destinationSize, destinationIndex, '0');
    } else {
        char reversed[6];
        int length = 0;
        uint32_t quotient = integer;
        while (quotient > 0) {
            reversed[length++] = '0' + (char) (quotient % 10); // index will never exceed 5 because integer is limited to 999999
            quotient /= 10;
        }
        while (--length >= 0) {
            WriteChar(destination, destinationSize, destinationIndex, reversed[length]);
        }
    }

    // Fractional part
    const uint32_t fraction = (uint32_t) (((absolute - (float) integer) * 10000.0f) + 0.5f);
    WriteChar(destination, destinationSize, destinationIndex, '.');
    WriteChar(destination, destinationSize, destinationIndex, '0' + (char) (fraction / 1000));
    WriteChar(destination, destinationSize, destinationIndex, '0' + (char) ((fraction / 100) % 10));
    WriteChar(destination, destinationSize, destinationIndex, '0' + (char) ((fraction / 10) % 10));
    WriteChar(destination, destinationSize, destinationIndex, '0' + (char) (fraction % 10));
}

/**
 * @brief Writes a string.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param destinationIndex Destination index.
 * @param string String.
 */
static inline __attribute__((always_inline)) void WriteString(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const char* string) {
    while (*string != '\0') {
        WriteChar(destination, destinationSize, destinationIndex, *string++);
    }
}

/**
 * @brief Writes a comma.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param destinationIndex Destination index.
 */
static inline __attribute__((always_inline)) void WriteComma(void* const destination, const size_t destinationSize, size_t * const destinationIndex) {
    WriteChar(destination, destinationSize, destinationIndex, ',');
}

/**
 * @brief Writes the termination.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param destinationIndex Destination index.
 */
static inline __attribute__((always_inline)) void WriteTermination(void* const destination, const size_t destinationSize, size_t * const destinationIndex) {
    if (*destinationIndex >= destinationSize) {
        if (destinationSize > 0) {
            ((char*) destination)[destinationSize - 1] = '\n';
        }
        return;
    }
    ((char*) destination)[(*destinationIndex)++] = '\n';
}

/**
 * @brief Writes a character.
 * @param destination Destination.
 * @param destinationSize Destination size.
 * @param destinationIndex Destination index.
 * @param character Character.
 */
static inline __attribute__((always_inline)) void WriteChar(void* const destination, const size_t destinationSize, size_t * const destinationIndex, const char character) {
    if (*destinationIndex >= destinationSize) {
        return;
    }
    if (((unsigned char) character < 0x20) || ((unsigned char) character > 0x7E)) {
        ((char*) destination)[(*destinationIndex)++] = '?';
        return;
    }
    ((char*) destination)[(*destinationIndex)++] = character;
}

//------------------------------------------------------------------------------
// End of file
