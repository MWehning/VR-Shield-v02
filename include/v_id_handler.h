#include <Arduino.h>
#include <Wire.h>

// Setup

void iniPacket(int16_t ini_devices[]);
bool getDeviceData();
void updateDeviceData(bool debugflag);
bool enableHardware(bool debugflag);

// Debug Tools

void printOutMask(bool debugflag);

// Data

void getData(int16_t buffer[],byte type, byte count);