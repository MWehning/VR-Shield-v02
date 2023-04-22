#include <Arduino.h>
#include <Wire.h>

// Setup

void iniPacket(int16_t ini_devices[]);
bool getDeviceData();
void updateDeviceData(bool debugflag);
bool enableHardware(bool debugflag);

// Debug Tools

void printOutMask(bool debugflag);
void printOutTasks(bool debugflag);

// Data

void getData(int16_t values[],byte type, byte count);
void getFilteredData(int16_t values[], byte type, byte count, int timeout);

// Task System

void kickTask(byte type, byte count,int timeout);
void updateTasks();
void executeTasks();