/**
 * @file main.c
 * @author Seb Madgwick
 * @brief Main file.
 *
 * Device:
 * PIC32MM0064GPL020
 *
 * Compiler:
 * XC32 v6.00, MPLAB Harmony 3
 */

//------------------------------------------------------------------------------
// Includes

#include "AsciiMessage/AsciiMessage.h"
#include "definitions.h"
#include "Icm/Icm.h"
#include "Periodic.h"
#include "ResetCause/ResetCause.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Timer/Timer.h"
#include "Uart/Uart1.h"
#include "Uart/Uart2.h"

//------------------------------------------------------------------------------
// Functions

int main(void) {
    SYS_Initialize(NULL);

    // Initialise modules
    TimerInitialise();
    Uart1Initialise(&uartSettingsDefault);
    Uart2Initialise(&uartSettingsDefault);

    // Print start up message
    TimerDelayMilliseconds(500);
    ResetCausePrint(ResetCauseGet());
    const char startupMessage[] = "x-IMU3-SA-IMU v1.0.0\n";
    printf(startupMessage);
    Uart1Write(startupMessage, strlen(startupMessage));

    // Initialise ICM
    const IcmSettings icmSettings = {
        .sampleRate = IcmSampleRate100Hz,
        .gyroscopeLowPassFilter = IcmLowPassFilterBypass,
        .accelerometerLowPassFilter = IcmLowPassFilterBypass,
    };
    IcmInitialise(&icmSettings);

    // Main program loop
    while (true) {
        SYS_Tasks();

        // Send ICM data
        char asciiMessage[128];
        IcmData icmData;
        while ((Uart1AvailableWrite() > sizeof (asciiMessage)) && (IcmGetData(&icmData) == IcmResultOk)) {
            const size_t messageLength = AsciiMessageFloats(asciiMessage, sizeof (asciiMessage), icmData.floatArray, 6);
            Uart1Write(asciiMessage, messageLength);
        }

        // ICM buffer overflow
        if (PERIODIC_POLL(1.0f)) {
            const uint32_t overflow = IcmBufferOverflow();
            if (overflow > 0) {
                printf("Buffer overflow: %u samples lost\n", overflow);
            }
        }

        // Enable idle mode (will wake up on next interrupt)
        POWER_LowPowerModeEnter(LOW_POWER_IDLE_MODE);
    }
    return (EXIT_FAILURE);
}

//------------------------------------------------------------------------------
// End of file
