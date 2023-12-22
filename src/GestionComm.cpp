/*
 * GestionComm.cpp
 *
 *  Created on: 5 déc. 2023
 *      Author: isib
 */

#include "GestionComm.h"

struct ThreadCommArgs{//pr thread de gestComm
	const int newClock ;
	const int RxPin;
};

//ThreadCommArgs* threadCommArgs= static_cast<ThreadCommArgs*>(args);
//const int newClock=threadCommArgs->newClock;
//const int RxPin = 6;

int iteration =0;
const int nbBits = 16;//nb bits à recevoir, 17 à la place de 16 car premier bit recu tjs =0
unsigned char receptionData[nbBits];


void timespecDiffMe(const struct timespec* start, const struct timespec* end, struct timespec* duration) {
	// TODO gérer diffs > 1s
	if (start->tv_sec < end->tv_sec)
		duration->tv_nsec = end->tv_nsec + (1000000000L - start->tv_nsec);
	else
		duration->tv_nsec = end->tv_nsec - start->tv_nsec;
}

/*
void handleFallingEdge(void) {
	std::cout << "lecture du bit: " << iteration << std::endl;
	iteration++;
	if(iteration == 14){  //pr verifier qu'on lit bonne valeur d'angle
		receptionData[iteration]= 1;
	}else{
		receptionData[iteration]= digitalRead(RxPin); //attention que premier bit qu'on lit est pas un bit de comm
	}

	receptionData[iteration]= digitalRead(RxPin);
}
*/

void* GestionComm(void* args){

	struct sched_param param;
	int policy= SCHED_FIFO;
	param.sched_priority = 98;
	pthread_setschedparam(pthread_self(), policy, &param);
	//std::cout<<pthread_getschedparam(pthread_self(), &policy, &param);
	pthread_getschedparam(pthread_self(), &policy, &param);
			std::cout << "Thread RT:  SCHED_FIFO=" << std::boolalpha << (policy == SCHED_FIFO)
					<< "  prio=" << param.sched_priority << std::endl;



	//Convert pointers to struct args
	ThreadCommArgs* threadCommArgs= static_cast<ThreadCommArgs*>(args);

	const int newClock=threadCommArgs->newClock;
	const int RxPin = threadCommArgs->RxPin;
	bool isClockHigh = false;
	//gestion attente active
	struct timespec sleepStartTime, currentTime, duration;
	//long int timeToRW=0;
	//Nv code pour gérer comm
	digitalWrite(newClock,HIGH);
	delay(100);
	digitalWrite(newClock,LOW);
	delay(100);
	while(1){

		isClockHigh = false;
		digitalWrite(newClock,LOW);
		for(int i=0;i<nbBits;i++){//init tab à 0
			receptionData[i]=0;
		}
		while(digitalRead(RxPin)==HIGH){
			std::cout << "STM sleep "<< std::endl;
		}
		std::cout << "STM reveille" << std::endl;//A NE SURTOUT PAS ENLEVER!!!!!!!!!!!!!!!!!!!!!!!!
		iteration =0;

		while(iteration < nbBits){// On lit 16 bit par exemple

			clock_gettime(CLOCK_MONOTONIC_RAW, &sleepStartTime);
			isClockHigh = !isClockHigh;
			digitalWrite(newClock, (int)isClockHigh);
			//if(iteration>=1){
			if(!isClockHigh){ //int falling =1
				/*
			//if(wiringPiISR(newClock, INT_EDGE_FALLING, &handleFallingEdge)){
				//digitalRead(RxPin);
				//std::cout << "lecture du bit: " << iteration << std::endl;
				//bit=;
				*/
				receptionData[iteration]= digitalRead(RxPin); //attention que premier bit qu'on lit est pas un bit de comm
				iteration++;
			}
			/*
			//else if (digitalRead(newClock)==HIGH){ //int falling = 2  INT_EDGE_RISING
				//std::cout << "STM write data : "<< iteration << std::endl;
			//}
			//}
			//if(iteration==0){
				//iteration++;
			//}
			//sleep(0.5);// changer avec clockgettime

			//attente active clockgettime

			if (iteration != nbBits){
				clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
				//timeToRW = currentTime.tv_nsec - sleepStartTime.tv_nsec;
				timespecDiffMe(&sleepStartTime,&currentTime,&duration);
				std::cout << "time to read or write data in ns : "<< duration.tv_nsec << std::endl;
			}
			*/

			do{//attente active
				clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
				//timeToRW = currentTime.tv_nsec - sleepStartTime.tv_nsec;
				timespecDiffMe(&sleepStartTime,&currentTime,&duration);
				/*
				//if(duration.tv_nsec<100000){
					//std::cout << "wait for activeTiming "<< std::endl;
				//}
				 * /
				 */
			}while(duration.tv_nsec<10000);//voir quel temps à mettre
		}

		/*
		//valeur tab
		//for(int i=nbBits-1; i>=0; i--){
			//std::cout <<receptionData[i] << " index: "<<i<<std::endl;
		//}
		//Convertir la valeur du tab en int
		//int16_t angle =0;
		//for(int i=nbBits-1; i>=0; --i){// lire ds ce sens la car la première valeur du tableau correspond au bit le moins significatif de la valeur int16_t, vous devez également inverser la direction des décalages de bits.
				//angle = (angle << 1) | receptionData[i];//on lit du 17 au 1

		//}


		//for(int i=0; i<nbBits; ++i){// lire ds ce sens la car la première valeur du tableau correspond au bit le moins significatif de la valeur int16_t, vous devez également inverser la direction des décalages de bits.
			//	angle |= (receptionData[i] & 0x01)<<i;
				//angle = (angle << 1) | receptionData[i];
		//}

		 */
		for (int i = 15; i >= 0; --i) {
			angle = (angle << 1) | receptionData[i];
		    }
		std::cout<< "valeur angle: "<<angle<<std::endl;
		std::cout<< ""<<std::endl;
		//sleep(0.01);//a supprimer
		digitalWrite(newClock,HIGH);
		usleep(1500);
	}
	return nullptr;
}
