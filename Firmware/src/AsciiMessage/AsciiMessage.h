/**
 * @file AsciiMessage.h
 * @author Seb Madgwick
 * @brief ASCII messages.
 */

#ifndef ASCII_MESSAGE_H
#define ASCII_MESSAGE_H

//------------------------------------------------------------------------------
// Includes

#include <stddef.h>

//------------------------------------------------------------------------------
// Function declarations

size_t AsciiMessageFloats(void* const destination, const size_t destinationSize, const float* const floats, const int numberOfFloats);

#endif

//------------------------------------------------------------------------------
// End of file
