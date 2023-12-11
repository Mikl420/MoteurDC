/*
 * GestionComm.h
 *
 *  Created on: 5 déc. 2023
 *      Author: isib
 */


#ifndef GESTIONCOMM_H_
#define GESTIONCOMM_H_




//void GestionComm(const int clockPin, const int RxPin);
void* GestionComm(void* args);
void timespecDiffMe(const struct timespec* start, const struct timespec* end, struct timespec* duration);


#endif /* GESTIONCOMM_H_ */
