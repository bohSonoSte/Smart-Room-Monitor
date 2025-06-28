#ifndef __ACCELLEROMETER_H__
#define __ACCELLEROMETER_H__

void initAccelerometer(void);
void readAccelerometer(float *x, float *y, float *z);
bool isEarthquakeDetected();

#endif
