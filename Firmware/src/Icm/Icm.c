/**
 * @file Icm.c
 * @author Seb Madgwick
 * @brief ICM-45686 driver.
 */

//------------------------------------------------------------------------------
// Includes

#include "definitions.h"
#include "Fifo.h"
#include "Icm.h"
#include "IcmRegisters.h"
#include "IcmResetValuesTest.h"
#include "Spi/Spi2.h"
#include <stdbool.h>
#include <stddef.h>
#include "Timer/Timer.h"

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Uncomment this line to test reset values.
 */
//#define TEST_RESET_VALUES

/**
 * @brief SPI packet.
 */
typedef struct {
    unsigned address : 7;
    unsigned rw : 1;

    union {
        uint8_t data[15];
        uint8_t value;

        struct {
            uint16_t indirectAddress;
            uint8_t indirectValue;
        } __attribute__((__packed__));
    };
} __attribute__((__packed__)) SpiPacket;

/**
 * @brief FIFO packet.
 */
typedef struct {
    uint64_t ticks;
    IcmSensorRegisters registers;
} __attribute__((__packed__)) FifoPacket;

//------------------------------------------------------------------------------
// Function declarations

static int LowPassFilterToLpfbwSel(const IcmLowPassFilter lowPassFilter);
static int SampleRateToOdr(const IcmSampleRate sampleRate);
static uint8_t ReadRegister(const uint8_t address);
__attribute__((unused)) static uint8_t ReadIndirectRegister(const uint16_t address);
static void WriteRegister(const uint8_t address, const uint8_t value);
static void WriteIndirectRegister(const uint16_t address, const uint8_t value);
static void TransferComplete(void);

//------------------------------------------------------------------------------
// Variables

static volatile SpiPacket spiPacket;
static volatile uint64_t ticks;
static uint8_t fifoData[(16 * sizeof (FifoPacket)) + 1]; // FIFO capacity is 1 less than size
static Fifo fifo = {.data = fifoData, .dataSize = sizeof (fifoData)};
static volatile uint32_t bufferOverflow;

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises the module.
 * @param settings Settings.
 */
void IcmInitialise(const IcmSettings * const settings) {

    // Ensure default states
    IcmDeinitialise();

    // Initialise SPI
    Spi2Initialise(&spiSettingsDefault);

    // Test reset values
#ifdef TEST_RESET_VALUES
    const IcmResetValuesTestCallbacks callbacks = {
        .readRegister = ReadRegister,
        .readIndirectRegister = ReadIndirectRegister,
        .writeRegister = WriteRegister,
        .delayMilliseconds = TimerDelayMilliseconds,
    };
    IcmResetValuesTest(&callbacks);
#endif

    // Read device ID
    __attribute__((unused)) const uint8_t deviceId = ReadRegister(ICM_WHO_AM_I_ADDRESS);

    // Soft reset
    IcmRegMisc2Register icmRegMisc2Register = {.value = ICM_REG_MISC2_RESET_VALUE};
    icmRegMisc2Register.softRst = 1;
    WriteRegister(ICM_REG_MISC2_ADDRESS, icmRegMisc2Register.value);
    TimerDelayMilliseconds(1);

    // Configure interrupt source
    IcmInt1Config0Register icmInt1Config0Register = {.value = ICM_INT1_CONFIG0_RESET_VALUE};
    icmInt1Config0Register.int1StatusEnDrdy = 1; // enable interrupt status bit to flag the occurrence of UI Data Ready event on INT1
    WriteRegister(ICM_INT1_CONFIG0_ADDRESS, icmInt1Config0Register.value);

    // Configure interrupt pin
    IcmInt1Config2Register icmInt1Config2Register = {.value = ICM_INT1_CONFIG2_RESET_VALUE};
    icmInt1Config2Register.int1Polarity = 1; // active high
    icmInt1Config2Register.int1Drive = 0; // push-pull
    WriteRegister(ICM_INT1_CONFIG2_ADDRESS, icmInt1Config2Register.value);

    // Configure gyroscope low-pass filter
    IcmIpregSys1Reg172Register icmIpregSys1Reg172Register = {.value = ICM_IPREG_SYS1_REG_172_RESET_VALUE};
    icmIpregSys1Reg172Register.gyroUiLpfbwSel = LowPassFilterToLpfbwSel(settings->gyroscopeLowPassFilter);
    WriteIndirectRegister(ICM_IPREG_SYS1_REG_172_ADDRESS, icmIpregSys1Reg172Register.value);

    // Configure accelerometer low-pass filter
    IcmIpregSys2Reg131Register icmIpregSys2Reg131Register = {.value = ICM_IPREG_SYS2_REG_131_RESET_VALUE};
    icmIpregSys2Reg131Register.accelUiLpfbwSel = LowPassFilterToLpfbwSel(settings->accelerometerLowPassFilter);
    WriteIndirectRegister(ICM_IPREG_SYS2_REG_131_ADDRESS, icmIpregSys2Reg131Register.value);

    // Configure accelerometer ODR and FS
    IcmAccelConfig0Register icmAccelConfig0Register = {.value = ICM_ACCEL_CONFIG0_RESET_VALUE};
    icmAccelConfig0Register.accelOdr = SampleRateToOdr(settings->sampleRate);
    icmAccelConfig0Register.accelUiFsSel = 0b000; // 32g
    WriteRegister(ICM_ACCEL_CONFIG0_ADDRESS, icmAccelConfig0Register.value);

    // Configure gyroscope ODR and FS
    IcmGyroConfig0Register icmGyroConfig0Register = {.value = ICM_GYRO_CONFIG0_RESET_VALUE};
    icmGyroConfig0Register.gyroOdr = SampleRateToOdr(settings->sampleRate);
    icmGyroConfig0Register.gyroUiFsSel = 0b0000; // 4000dps
    WriteRegister(ICM_GYRO_CONFIG0_ADDRESS, icmGyroConfig0Register.value);

    // Turn on gyroscope and accelerometer
    IcmPwrMgmt0Register icmPwrMgmt0Register = {.value = ICM_PWR_MGMT0_RESET_VALUE};
    icmPwrMgmt0Register.accelMode = 0b11; // places accelerometer in Low Noise (LN) Mode
    icmPwrMgmt0Register.gyroMode = 0b11; // places gyroscope in Low Noise (LN) Mode
    WriteRegister(ICM_PWR_MGMT0_ADDRESS, icmPwrMgmt0Register.value);
    TimerDelayMilliseconds(35);

    // Configure interrupt
    INTCONbits.INT4EP = 1; // Rising edge
    EVIC_SourceStatusClear(INT_SOURCE_EXTERNAL_4);
    EVIC_SourceEnable(INT_SOURCE_EXTERNAL_4);
}

/**
 * @brief Returns the LPFBW_SEL value for a low-pass filter.
 * @param lowPassFilter Low-pass filter.
 * @return LPFBW_SEL value.
 */
static int LowPassFilterToLpfbwSel(const IcmLowPassFilter lowPassFilter) {
    switch (lowPassFilter) {
        case IcmLowPassFilterBypass:
            return 0b000;
        case IcmLowPassFilter4:
            return 0b001;
        case IcmLowPassFilter8:
            return 0b010;
        case IcmLowPassFilter16:
            return 0b011;
        case IcmLowPassFilter32:
            return 0b100;
        case IcmLowPassFilter64:
            return 0b101;
        case IcmLowPassFilter128:
            return 0b110;
    }
    return 0b000; // avoid compiler warning
}

/**
 * @brief Returns the ODR value for a sample rate.
 * @param sampleRate Sample rate.
 * @return ODR value.
 */
static int SampleRateToOdr(const IcmSampleRate sampleRate) {
    switch (sampleRate) {
        case IcmSampleRate12Hz:
            return 0b1100;
        case IcmSampleRate25Hz:
            return 0b1011;
        case IcmSampleRate50Hz:
            return 0b1010;
        case IcmSampleRate100Hz:
            return 0b1001;
        case IcmSampleRate200Hz:
            return 0b1000;
        case IcmSampleRate400Hz:
            return 0b0111;
        case IcmSampleRate800Hz:
            return 0b0110;
        case IcmSampleRate1600Hz:
            return 0b0101;
        case IcmSampleRate3200Hz:
            return 0b0100;
        case IcmSampleRate6400Hz:
            return 0b0011;
    }
    return 0b0110; // avoid compiler warning
}

/**
 * @brief Deinitialises the module.
 */
void IcmDeinitialise(void) {
    EVIC_SourceDisable(INT_SOURCE_EXTERNAL_4);
    while (Spi2TransferInProgress());
    Spi2Deinitialise();
    FifoClear(&fifo);
    bufferOverflow = 0;
}

/**
 * @brief Reads the register value.
 * @param address Address.
 * @return Value.
 */
static uint8_t ReadRegister(const uint8_t address) {
    spiPacket = (SpiPacket){.rw = 1, .address = address};
    Spi2Transfer(CS_PIN, &spiPacket, 2, NULL);
    while (Spi2TransferInProgress());
    return spiPacket.data[0];
}

/**
 * @brief Reads the indirect register value.
 * @param address Address.
 * @return Value.
 */
__attribute__((unused)) static uint8_t ReadIndirectRegister(const uint16_t address) {
    spiPacket = (SpiPacket){.rw = 0, .address = ICM_IREG_ADDR_15_8_ADDRESS, .indirectAddress = address};
    Spi2Transfer(CS_PIN, &spiPacket, 3, NULL);
    while (Spi2TransferInProgress());
    TimerDelayMicroseconds(4);
    return ReadRegister(ICM_IREG_DATA_ADDRESS);
}

/**
 * @brief Writes the register value.
 * @param address Address.
 * @param value Value.
 */
static void WriteRegister(const uint8_t address, const uint8_t value) {
    spiPacket = (SpiPacket){.rw = 0, .address = address, .value = value};
    Spi2Transfer(CS_PIN, &spiPacket, 2, NULL);
    while (Spi2TransferInProgress());
}

/**
 * @brief Writes the indirect register value.
 * @param address Address.
 * @param value Value.
 */
static void WriteIndirectRegister(const uint16_t address, const uint8_t value) {
    spiPacket = (SpiPacket){.rw = 0, .address = ICM_IREG_ADDR_15_8_ADDRESS, .indirectAddress = address, .indirectValue = value};
    Spi2Transfer(CS_PIN, &spiPacket, 4, NULL);
    while (Spi2TransferInProgress());
    TimerDelayMicroseconds(4);
}

/**
 * @brief External interrupt handler. This function should be called by the ISR
 * implementation generated by MPLAB Harmony.
 */
void External4InterruptHandler(void) {
    if (Spi2TransferInProgress() == false) {
        ticks = TimerGetTicks64();
        spiPacket = (SpiPacket){.rw = 1, .address = ICM_ACCEL_DATA_X1_UI_ADDRESS};
        Spi2Transfer(CS_PIN, &spiPacket, sizeof (IcmSensorRegisters) + 1, TransferComplete);
    }
    EVIC_SourceStatusClear(INT_SOURCE_EXTERNAL_4);
}

/**
 * @brief Transfer complete callback.
 */
static void TransferComplete(void) {
    const FifoPacket fifoPacket = {
        .ticks = ticks,
        .registers = *((IcmSensorRegisters*) spiPacket.data),
    };
    if (FifoWrite(&fifo, &fifoPacket, sizeof (fifoPacket)) != FifoResultOk) {
        bufferOverflow++;
    }
}

/**
 * @brief Gets data.
 * @param data Data.
 * @return Result.
 */
IcmResult IcmGetData(IcmData * const data) {
    FifoPacket fifoPacket;
    if (FifoRead(&fifo, &fifoPacket, sizeof (fifoPacket)) == 0) {
        return IcmResultError;
    }
    data->ticks = fifoPacket.ticks;
    data->gyroscopeX = (float) fifoPacket.registers.gyroDataX * (1.0f / 8.2f);
    data->gyroscopeY = (float) fifoPacket.registers.gyroDataY * (-1.0f / 8.2f);
    data->gyroscopeZ = (float) fifoPacket.registers.gyroDataZ * (-1.0f / 8.2f);
    data->accelerometerX = (float) fifoPacket.registers.accelDataX * (1.0f / 1024.0f);
    data->accelerometerY = (float) fifoPacket.registers.accelDataY * (-1.0f / 1024.0f);
    data->accelerometerZ = (float) fifoPacket.registers.accelDataZ * (-1.0f / 1024.0f);
    data->temperature = (float) fifoPacket.registers.tempData * (1.0f / 128.0f) + 25.0f;
    return IcmResultOk;
}

/**
 * @brief Returns the number of samples lost due to buffer overflow. Calling
 * this function will reset the value.
 * @return Number of samples lost due to buffer overflow.
 */
uint32_t IcmBufferOverflow(void) {
    return __sync_lock_test_and_set(&bufferOverflow, 0);
}

//------------------------------------------------------------------------------
// End of file
