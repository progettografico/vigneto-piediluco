// motors.h — controllo differential drive dei due motori di propulsione.
#pragma once

void motorsBegin();

// Comando di guida.
//   v: velocità avanti/indietro [-1..+1]  (+ = avanti)
//   w: rotazione                [-1..+1]  (+ = gira a destra)
void motorsSetDrive(float v, float w);

void motorsStop();
