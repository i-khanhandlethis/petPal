#include "MovementCycle.h"
#include "scheduler/scheduler.h"
#include "filesystems/filesystem.h"
#include "scheduler/rtcMemory.h"
#include "health/health.h"
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM6DS3.h>
#include <LSM6DSL.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <SPIFFS.h>

int stepsTaken, bobCount;
extern int BatteryVoltage;

extern bool deviceConnected;
bool pollDebug = true;
bool layingDownState = false;

bool pollState = true;
float XLX[250], XLY[250], XLZ[250], GYROX[250], GYROY[250], GYROZ[250];
bool fullRun = true;

uint8_t TotalSteps = 0;

LSM6DS3 myIMU(SPI_MODE, 5);
LSM6DSLCore imu(LSM6DSL_MODE_SPI, 5);

void setupXLGYRO()
{
    //Over-ride default settings if desired
    myIMU.settings.gyroEnabled = 1;        //Can be 0 or 1
    myIMU.settings.gyroRange = 500;        //Max deg/s.  Can be: 125, 245, 500, 1000, 2000
    myIMU.settings.gyroSampleRate = 26;    //Hz.  Can be: 13, 26, 52, 104, 208, 416, 833, 1666
    myIMU.settings.gyroBandWidth = 50;     //Hz.  Can be: 50, 100, 200, 400;
    myIMU.settings.gyroFifoEnabled = 1;    //Set to include gyro in FIFO
    myIMU.settings.gyroFifoDecimation = 1; //set 1 for on /1

    myIMU.settings.accelEnabled = 1;
    myIMU.settings.accelRange = 8;          //Max G force readable.  Can be: 2, 4, 8, 16
    myIMU.settings.accelSampleRate = 26;    //Hz.  Can be: 13, 26, 52, 104, 208, 416, 833, 1666, 3332, 6664, 13330
    myIMU.settings.accelBandWidth = 50;     //Hz.  Can be: 50, 100, 200, 400;
    myIMU.settings.accelFifoEnabled = 1;    //Set to include accelerometer in the FIFO
    myIMU.settings.accelFifoDecimation = 1; //set 1 for on /1
    myIMU.settings.tempEnabled = 1;

    //Non-basic mode settings
    myIMU.settings.commMode = 1;

    //FIFO control settings
    myIMU.settings.fifoThreshold = 900;  //Can be 0 to 4096 (16 bit bytes)
    myIMU.settings.fifoSampleRate = 100; //Hz.  Can be: 10, 25, 50, 100, 200, 400, 800, 1600, 3300, 6600
    myIMU.settings.fifoModeWord = 6;     //FIFO mode.
    //FIFO mode.  Can be:
    //  0 (Bypass mode, FIFO off)
    //  1 (Stop when full)
    //  3 (Continuous during trigger)
    //  4 (Bypass until trigger)
    //  6 (Continous mode)

    //Call .beginCore() to configure the IMU
    //delay(10);
    if (myIMU.begin() != 0)
        Serial.print("BeginCore() | ");
    else
        Serial.print("beginCore() passed | ");

    myIMU.writeRegister(LSM6DSL_ACC_GYRO_CTRL10_C, 0x16);

    Serial.print("Configuring FIFO with no error checking | ");
    myIMU.fifoBegin();
    Serial.println("Done!");

    /*
    //Variables init
    uint8_t errorAccumulator = 0; //Error accumulation variable. If there is an error, it will become bigger than 0
    uint8_t dataToWrite = 0;      //Variable used to pass information into the SPI registers

    //Setup the ODR
    errorAccumulator += myIMU.readRegister(&dataToWrite, LSM6DS3_ACC_GYRO_CTRL4_C); //Set the ODR bit
    dataToWrite &= ~((uint8_t)LSM6DS3_ACC_GYRO_BW_SCAL_ODR_ENABLED);

    //Configure Tilt and Tap settings
    errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0b111100); //

    //Enable tap detection on X, Y, Z axis, but do not latch output

    //errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_TAP_CFG1, 0b00101110);
    errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0b01100000);
    // Set tap and tilt threshold
    // Write 0Ch into TAP_THS_6D
    errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_THS_6D, 0b01100000);

    // Single tap interrupt driven to INT1 pin -- enable latch
    errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_INT1_CTRL, 0x10);

    errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_MD2_CFG, 0b10);
    //Set

    if (errorAccumulator)
    {
        Serial.println("Problem configuring the device.");
    }
    else
    {
        Serial.println("Device O.K.");
    }
    */
}

//Raw Polling
void rawXLGPoll(time_t timestamp)
{
    if (myIMU.begin() != 0)
        Serial.print("Error at beginCore().\n");
    else
        Serial.print("\nbeginCore() passed.\n");

    //int startpolling = millis();
    float temp; //This is to hold read data
    uint16_t tempUnsigned;
    int count = 0;
    Serial.println("done initializing polling shits");

    while ((myIMU.fifoGetStatus() & 0x8000) == 0)
    {
    }; //Wait for watermark
    Serial.println("watermark is full");

    float timestampvar = now() - 2;

    File file = SPIFFS.open("/data.txt", FILE_APPEND);
    if (!file)
    {
        Serial.println("There was an error opening the file for writing");
        return;
    }

    //Now loop until FIFO is empty.  NOTE:  As the FIFO is only 8 bits wide,
    //the channels must be synchronized to a known position for the data to align
    //properly.  Emptying the fifo is one way of doing this (this example)
    while ((myIMU.fifoGetStatus() & 0x1000) == 0)
    {
        String vals = "";
        temp = myIMU.calcGyro(myIMU.fifoRead());
        if (count < 160)
            vals = "\"XG\":\"" + String(temp) + "\",";

        temp = myIMU.calcGyro(myIMU.fifoRead());
        if (count < 160)
            vals = vals + "\"YG\":\"" + String(temp) + "\",";

        temp = myIMU.calcGyro(myIMU.fifoRead());
        if (count < 160)
            vals = vals + "\"ZG\":\"" + String(temp) + "\",";

        temp = myIMU.calcAccel(myIMU.fifoRead());
        if (count < 160)
            vals = vals + "\"XA\":\"" + String(temp) + "\",";

        temp = myIMU.calcAccel(myIMU.fifoRead());
        if (count < 160)
            vals = vals + "\"YA\":\"" + String(temp) + "\",";

        temp = myIMU.calcAccel(myIMU.fifoRead());
        if (count < 160)
            vals = vals + "\"ZA\":\"" + String(temp) + "\"";

        if (count < 160)
        {
            String dataSave = "\"TIMESTAMP\":\"" + String(timestampvar) + "\",\"COUNT\":\"" + count + "\"," + vals + "}_";
            if (!file.print(dataSave))
                Serial.println("File write failed");
        }

        count++;
    }

    Serial.println("done polling & formatting");

    file.close();

    /*

    int endpolling = millis();
    int timetaken = endpolling - startpolling;
    float timeinterval = timetaken / count;
    Serial.print("Total counts of ");
    Serial.print(count);
    Serial.print(" with time of");
    Serial.print(timetaken);
    Serial.print("ms with interval of ");
    Serial.print(timeinterval);
    Serial.println("ms");
    */
    tempUnsigned = myIMU.fifoGetStatus();
    Serial.print("\nFifo Status 1 and 2 (16 bits): 0x");
    Serial.println(tempUnsigned, HEX);
    Serial.print("\n");
}

//Check if animal is laying down
bool checkLayingState()
{

    if (imu.beginCore() != IMU_SUCC)
    {
        Serial.println("Failed initializing IMU sensor");
    }
    else
    {
        Serial.print("IMU SUCC LIKE THE MEAT CHEF");
    }

    imu.writeRegister(0b10000, 0x15); //set low power mode

    imu.writeRegister(0b1011000000, 0x10); //turn on accelerometer

    imu.writeRegister(0b00010000, 0x11); //turn on gyro

    uint8_t errorAccumulator = 0;
    uint8_t dataToWrite = 0;

    // Setup accelerometer
    dataToWrite = 0;
    dataToWrite |= LSM6DSL_ACC_GYRO_FS_XL_2g;
    dataToWrite |= LSM6DSL_ACC_GYRO_ODR_G_416Hz;
    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_CTRL1_XL_REG, dataToWrite);

    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, 0x00);

    // set FF threshold (FF_THS[2:0] = 011b)
    // set six samples event duration (FF_DUR[5:0] = 00110b)
    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_FREE_FALL, 0x33);

    // FF interrupt driven to INT1 pin
    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_MD1_CFG, 0x10);

    // route to INT2 as well
    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_MD2_CFG, 0x10);

    // Latch interrupt & enable interrupt
    errorAccumulator += imu.writeRegister(LSM6DSL_ACC_GYRO_TAP_CFG, 0x81);

    int16_t triggerReasonTwo;
    imu.readRegisterInt16(&triggerReasonTwo, 0x1D);
    //masking
    triggerReasonTwo = triggerReasonTwo & 0b111111;

    imu.writeRegister(0b00000000, 0x10); //turn off accelerometer

    imu.writeRegister(0b00000000, 0x11); //turn off gyro

    Serial.println(triggerReasonTwo);
    switch (triggerReasonTwo)
    {
    case 1:
        Serial.println("XL");
        break;

    case 2:
        Serial.println("XH");
        break;

    case 4:
        Serial.println("YL");
        break;

    case 8:
        Serial.println("YH");
        break;

    case 16:
        Serial.println("ZL");
        break;

    case 32:
        Serial.println("ZH");
        return true;
        break;

    default:
        Serial.println("Unknown");
    }
    return false;
}

int checkPedometer()
{
    uint8_t readDataByte = 0;
    stepsTaken = 0;
    if (myIMU.beginCore() != 0)
    {
        Serial.print("Error at beginCore().\n");
    }
    else
    {
        Serial.print("\nbeginCore() passed.\n");
    }
    //Read the 16bit value by two 8bit operations
    myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);

    stepsTaken = ((uint16_t)readDataByte) << 8;
    myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);

    int stepsminutes = minute(now());
    stepsminutes = stepsminutes % 10;
    stepsminutes = - stepsminutes;

    Serial.print("Round up minutes: ");
    Serial.println(stepsminutes);

    stepsTaken |= readDataByte;
    myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0b111110);

    Serial.print("Steps taken: ");
    Serial.println(stepsTaken);


            File file = SPIFFS.open("/data.txt", FILE_APPEND);
    if (!file)
    {
        Serial.println("There was an error opening the file for writing");
    }
    

        String dataSave;

        if (devState == 0)
            dataSave = "\"TIMESTAMP\":\"" + String(now() + stepsminutes * 60) + "\",\"STEPS\":\"" + stepsTaken + "\",\"BATTERY\":\"" + battVoltage() + "\"}_";

        if (devState == 1)
            dataSave = "\"TIMESTAMP\":\"" + String(now()) + "\",\"WALKER\":\"" + stepsTaken + "\",\"BATTERY\":\"" + battVoltage() + "\"}_";

        if (!file.print(dataSave))
            Serial.println("File write failed");
        else
            Serial.println("File write success!");
        file.close();



    if (stepsTaken > 0)
    {

        return stepsTaken;
    }
    else
        return 0;
}
