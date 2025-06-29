#ifndef ACCElLEROMETER_H
#define ACCElLEROMETER_H

#include <stdbool.h>

// Starts the accelerometer
void Accelerometer_init(void);

// Detect movement (TRUE = movement detected)
bool isMovementDetected(void);

void updateCalibration(void);

#endif // ACCELEROMETER_H
