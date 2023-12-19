/*
 * GestionComm.h
 *
 *  Created on: 5 déc. 2023
 *      Author: isib
 */


#ifndef GESTIONCOMM_H_
#define GESTIONCOMM_H_
#include "wiringPi.h"
#include "unistd.h"
#include <iostream>
#include <cstdint>
#include <time.h>

extern int16_t angle;



//void GestionComm(const int clockPin, const int RxPin);
void* GestionComm(void* args);
void timespecDiffMe(const struct timespec* start, const struct timespec* end, struct timespec* duration);


#endif /* GESTIONCOMM_H_ */
