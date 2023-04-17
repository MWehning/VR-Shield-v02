#include <Arduino.h>
#include <Wire.h>

// Setup

bool getDeviceData();
void updateDeviceData(bool debugflag);
bool setupDevices(bool debugflag);

// Debug Tools

void printOutMask(bool debugflag);

// Data

void getData(float buffer[],byte type, byte count);