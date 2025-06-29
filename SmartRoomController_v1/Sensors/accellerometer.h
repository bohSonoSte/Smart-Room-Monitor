#ifndef ACCElLEROMETER_H
#define ACCElLEROMETER_H

#include <stdbool.h>

// Inizializza l'accelerometro
void Accelerometer_init(void);

// Rileva movimento (TRUE = movimento rilevato)
bool isMovementDetected(void);

void updateCalibration(void);

#endif // ACCELEROMETER_H
