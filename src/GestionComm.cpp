/*
 * GestionComm.cpp
 *
 *  Created on: 5 déc. 2023
 *      Author: isib
 */
#include <wiringPi.h>
#include "unistd.h"
#include <iostream>
#include <cstdint>
//#include <time.h>

struct ThreadCommArgs{//pr thread de gestComm
	const int newClock ;
	const int RxPin;
};


void timespecDiffMe(const struct timespec* start, const struct timespec* end, struct timespec* duration) {
	// TODO gérer diffs > 1s
	if (start->tv_sec < end->tv_sec)
		duration->tv_nsec = end->tv_nsec + (1000000000L - start->tv_nsec);
	else
		duration->tv_nsec = end->tv_nsec - start->tv_nsec;
}


void* GestionComm(void* args){//const int newClock, const int RxPin

	//Convert pointers to struct args
	ThreadCommArgs* threadCommArgs= static_cast<ThreadCommArgs*>(args);

	const int newClock=threadCommArgs->newClock;
	const int RxPin = threadCommArgs->RxPin;


	//gestion attente active
	struct timespec sleepStartTime, currentTime, duration;
	//long int timeToRW=0;

	//Nv code pour gérer comm
	digitalWrite(newClock,HIGH);
	//sleep(0.5);//a supprimer
	digitalWrite(newClock,LOW);

	while(digitalRead(RxPin)==HIGH){
		std::cout << "STM sleep "<< std::endl;
		digitalWrite(newClock,LOW);
	}
	std::cout << "STM reveille" << std::endl;
	//sleep(1);
	//Debut lecture data
	bool pinHigh = true;
	int iteration =0;
	int nbBits = 17;//nb bits à recevoir, 17 à la place de 16 car premier bit recu tjs =0
	int receptionData[nbBits]; //!!!Attention au sens de lecture des bits!!!
	for(int i=0;i<nbBits;i++){//init tab à 0
		receptionData[i]=0;
	}

	while(iteration != nbBits){// On lit 16 bit par exemple

		clock_gettime(CLOCK_MONOTONIC_RAW, &sleepStartTime);

		digitalWrite(newClock, (int)pinHigh);
		if(digitalRead(newClock)==LOW){ //int falling =1
			//digitalRead(RxPin);
			std::cout << "lecture du bit: " << iteration << std::endl;
			iteration++;
			/*if(iteration == 14){  //pr verifier qu'on lit bonne valeur d'angle
				receptionData[iteration]= 1;
			}else{
				receptionData[iteration]= digitalRead(RxPin); //attention que premier bit qu'on lit est pas un bit de comm
			}
			*/
			receptionData[iteration]= digitalRead(RxPin); //attention que premier bit qu'on lit est pas un bit de comm
		}else if (digitalRead(newClock)==HIGH){ //int falling = 2  INT_EDGE_RISING
			std::cout << "STM write data : "<< iteration << std::endl;
		}

		//sleep(0.5);// changer avec clockgettime

		//attente active clockgettime
		if (iteration != nbBits){
			clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
			//timeToRW = currentTime.tv_nsec - sleepStartTime.tv_nsec;
			timespecDiffMe(&sleepStartTime,&currentTime,&duration);
			std::cout << "time to read or write data in ns : "<< duration.tv_nsec << std::endl;
		}

		do{//attente active
			clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
			//timeToRW = currentTime.tv_nsec - sleepStartTime.tv_nsec;
			timespecDiffMe(&sleepStartTime,&currentTime,&duration);
			if(duration.tv_nsec<500000){
				std::cout << "wait for activeTiming "<< std::endl;
			}
		}while(duration.tv_nsec<500000);//voir quel temps à mettre

		pinHigh = !pinHigh;
	}

	//valeur tab
	for(int i=nbBits-1; i>=0; i--){
		std::cout <<receptionData[i] << " index: "<<i<<std::endl;
	}
	//Convertir la valeur du tab en int
	int16_t angle =0;
	for(int i=nbBits; i>=1; --i){// lire ds ce sens la car la première valeur du tableau correspond au bit le moins significatif de la valeur int16_t, vous devez également inverser la direction des décalages de bits.
			angle = (angle << 1) | receptionData[i];
	}
	std::cout<< "valeur angle: "<<angle<<std::endl;

	digitalWrite(newClock,HIGH);
	//sleep(0.5);// a supprimer
	return nullptr;
}