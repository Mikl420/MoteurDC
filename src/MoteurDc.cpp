//============================================================================
// Name        : RTSoftTimer.cpp
// Author      : Gwendal
// Version     :
// Copyright   : -
// Description : Signal de timer logiciel haute précision
//============================================================================

#include <iostream>
#include "unistd.h"
#include <pthread.h>
#include <time.h>

//Nvelle
#include <wiringPi.h>
#include "GestionComm.h"

//connecter rasp au wifi puis voir son ip et ajouter dans lauch config
//ajouter ds properties ,C++BUILD,Settings,Cross G++ linker, lib   les librairies pthread wiringPI et libWiringPI(/usr/lib)?
//ajouter dossier MoteurDC dans Rasp, on envoie dessus l executable, ajouter wiringPi depuis rap et lib libwiringPi.so.2.50 c'est elle qui contient tout les fichiers wiringpi!.h ..,
//reg Clock pin, branché pin interieur du rasp qu'on connecte après a l'oscillo, on run le prog et on reg
//Il faut lancer le program en sudo?


// Ce programme peut encore être amélioré, par exemple :
//   - en gérant les erreurs de temps-réel (à un certain % près)
//   - en vérifiant les valeurs d'erreur de nanosleep (voir 'man nanosleep')
//   - en gérant les très longs délais (actuellement: pas prévu pour délais ~ 1 seconde)


// Variables globales / defines, pas propre... à éviter au maximum.
// Mais plus facile ici, pour ne pas complexifier ce code d'exemple

const int clockPin = 0;  // numérotation wiringPi. On aura un signal d'horloge:
						 // chaque front montant ou descendant correspond à un tick du timer
const int dbgPin = 1;  // numérotation wiringPi

#define TIMER_RUN_TIME_S (5*60)  // temps approx d'exécution du programme, en secondes
#define TIMER_PERIOD_US 500  // période du timer, en microsecondes
#define OS_MAX_LATENCY_US 130  // latence max de l'OS temps-réel (cyclictest), en microsecondes

// En moyenne 2 microsecondes, mais pointe mesurée à 130microsecondes...
// probablement arrivée dans un cas où le kernel lui-même était temporairement sur-chargé
#define CLOCK_GETTIME_AVG_DURATION_NS (2100L)

#define USE_COMPENSATED_SLEEP 1  // compensation de latence, ou sleep naïf
/* Comparaison des 2 pour un rpi 3 A+ kernel 4.19.71-rt24-v7+; compilation release -03, prio FIFO 99
 * mesures à l'oscilloscope pendant 5 minutes, pour un timer à 2kHz (500us), mesures en us
 *
 *                          avg       min      max
 * sans compensation:       526       481      708
 * avec compensation:       500       479      638
 */

//Nv code, Ajout pin LogicRelay,PowerRelay,PWM,Sens1,Sens2 (WiringPi)  Pr moteur
const int logicRelay= 15;
const int powerRelay= 16;
const int pwmPin= 23;
const int sens1= 4;
const int sens2= 5;


//Nv code, Ajout de IRQ et RX
//const int IrqPin = 6; // pas necessaire c'est la clock qui va faire les irq
//const int newClock = 2;
//const int RxPin = 6;

struct ThreadCommArgs{//pr thread de gestComm
	const int newClock = 2;
	const int RxPin = 6;
};



// Fonctions du timer lui-même
void* rtSoftTimerThread(void* arg);
void naiveSleepUntil(struct timespec* sleepEndTime);
void compensatedSleepUntil(struct timespec* sleepEndTime);

//Nvelles Fonctions ajoutées Pr moteur
void initialisationRelais();
void gestionSens();
void initialisationPWM(int pwm);
void stopRelais();

//Nvelles méthodes pour comm
//void initialisationRx();


int main() {

	//ajout code
	wiringPiSetup();
	stopRelais();
	initialisationRelais();
	//sleep(1);
	gestionSens();
	/*
	while(1){
		initialisationPWM(1023);
		sleep(1);//en sec
		initialisationPWM(0);
		sleep(1);
	}
	*/
	//stopRelais();


	/*//Ajout méthode pour créer la clock du rasp, methode ds un autre fichier .cpp, tester recoit bien les bits apres, ajouter ds un thread qd tt est ok
	wiringPiSetup();
	initialisationRx();
	GestionComm(newClock,RxPin);
	*/
	//wiringPiSetup();
	ThreadCommArgs threadCommArgs;
	pinMode(threadCommArgs.RxPin,INPUT);
	pinMode(threadCommArgs.newClock,OUTPUT);
	digitalWrite(threadCommArgs.newClock,HIGH);

	pthread_t threadCommTid;//creation du thread tid
	pthread_attr_t threadAttributes2;// Configuration et lancement du thread
	pthread_attr_init(&threadAttributes2);
	pthread_attr_setdetachstate(&threadAttributes2, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threadCommTid, &threadAttributes2, &GestionComm, static_cast<void*>(&threadCommArgs));//
	pthread_attr_destroy(&threadAttributes2);


	std::cout << "Real-Time software Timer on Raspberry Pi 3" << std::endl;
	char hostname[100];
	gethostname(hostname, 100);
	std::cout << "Machine name: " << hostname << std::endl;

	// TID
	pthread_t rtThreadTid;
	// Configuration et lancement du thread
	pthread_attr_t threadAttributes;
	pthread_attr_init(&threadAttributes);
	pthread_attr_setdetachstate(&threadAttributes, PTHREAD_CREATE_JOINABLE);
	pthread_create(&rtThreadTid, &threadAttributes, &rtSoftTimerThread, 0);
	pthread_attr_destroy(&threadAttributes);



	//attente de la fin du thread de gestComm
	pthread_join(threadCommTid, 0);

	// Attente la fin du threads - join
	pthread_join(rtThreadTid, 0);

	return 0;
}




//Nv code pour initialisation des relais
void initialisationRelais(){
	pinMode(logicRelay,OUTPUT);
	pinMode(powerRelay,OUTPUT);
	digitalWrite(logicRelay,HIGH);
	digitalWrite(powerRelay,HIGH);
}

//Nv code, arreter relais
void stopRelais(){
	digitalWrite(logicRelay,LOW);
	digitalWrite(powerRelay,LOW);
}

//Nv code pour gerer PWM
void initialisationPWM(int pwm){
	pinMode(pwmPin,PWM_OUTPUT);
	//digitalWrite(pwmPin,HIGH);
	pwmWrite(pwmPin,pwm);

}

//Nv code pour gerer sens
void gestionSens(){
	pinMode(sens1,OUTPUT);
	pinMode(sens2,OUTPUT);
	digitalWrite(sens1,LOW);
	digitalWrite(sens2,HIGH);
}

//Initialisation Rx pr comm
/*
void initialisationRx(){

	pinMode(RxPin,INPUT);
	pinMode(newClock,OUTPUT);
	digitalWrite(newClock,HIGH);
}
*/


void periodicComputation() {
	// TODO Dans cette fonction, on peut mettre un calcul qui sera exécuté de manière
	// (quasi-)périodique par la RT software clock

	// Ici on va juste simuler un calcul aléatoire inutile
	int numIter = rand() % 100;
	int uselessInt = 0;
	for (int i = 0 ; i < numIter ; i++)
		uselessInt = (uselessInt + rand()) % 1000;
}


void* rtSoftTimerThread(void* arg) {
	// Changement politique et priorité
	struct sched_param params;
	params.sched_priority = 99;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
	// Vérification du changement
	int policy;
	pthread_getschedparam(pthread_self(), &policy, &params);
	std::cout << "Thread RT:  SCHED_FIFO=" << std::boolalpha << (policy == SCHED_FIFO)
			<< "  prio=" << params.sched_priority << std::endl;

	// Configuration WiringPi
	int wiringPiSetupStatus = wiringPiSetup();
	if (wiringPiSetupStatus != 0) {
		std::cerr << "wiringPiSetupStatus=" << wiringPiSetupStatus << " ; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	pinMode(clockPin, OUTPUT);
	pinMode(dbgPin, OUTPUT);
	bool pinHigh = false;

	/*
	//Nv code pour faire tourner moteur
	wiringPiSetup();
	stopRelais();
	initialisationRelais();
	sleep(1);
	gestionSens();
	//while(1){
	initialisationPWM(1023);
	sleep(20);//en sec
	//initialisationPWM(0);
	//sleep(1);
	//}
	stopRelais();
	*/





	// type long: d'après la commande "man clock_gettime"
	long timerPeriod_ns = 1000L * TIMER_PERIOD_US;

	// On boucle sur un temps pré-défini (le temps de faire les tests)
	struct timespec startTime, iterationStartTime, sleepEndTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
	bool continueTimer = true;
	while (continueTimer) {
		// TODO expliquer pourquoi CLOCK_MONOTONIC_RAW est l'option de clock la + adaptée
		digitalWrite(dbgPin, HIGH);  // pour mesure le temps approx de ce call OS lui-même
		clock_gettime(CLOCK_MONOTONIC_RAW, &iterationStartTime);
		digitalWrite(dbgPin, LOW);

		// Pour visualiser la clock facilement sur un oscilloscope
		pinHigh = !pinHigh;
		digitalWrite(clockPin, (int)pinHigh);

		// TODO doc: something can be done here
		periodicComputation();
		// Condition de sortie: si le temps de run en secondes est dépassé
		if ((iterationStartTime.tv_sec - TIMER_RUN_TIME_S) > startTime.tv_sec)
			continueTimer = false;

		// Calcul de l'instant auquel on aimerait se réveiller, pour enchaîner directement
		// sur la boucle suivante. Attention à gérer correctement la structure timespec,
		// surtout les overflows de nanosecondes
		if (iterationStartTime.tv_nsec + timerPeriod_ns >= 1000000000L) {  // Si on passe à la seconde suivante
			sleepEndTime.tv_nsec = iterationStartTime.tv_nsec + timerPeriod_ns - 1000000000L;
			sleepEndTime.tv_sec = iterationStartTime.tv_sec + 1;
		}
		else {  // Sinon, cas le + simple: on ajoute juste les nanosecondes à attendre
			sleepEndTime.tv_nsec = iterationStartTime.tv_nsec + timerPeriod_ns;
			sleepEndTime.tv_sec = iterationStartTime.tv_sec;
		}

		// TODO sleep_until, 2 modes (compensated or not)
		if (USE_COMPENSATED_SLEEP)
			compensatedSleepUntil(&sleepEndTime);
		else
			naiveSleepUntil(&sleepEndTime);
	}


	// Fin du thread
	digitalWrite(clockPin, LOW);
	digitalWrite(dbgPin, LOW);
	std::cout << "Thread RT a terminé" << std::endl;
	return 0;  // pointeur sur rien du tout
}

void timespecDiff(const struct timespec* start, const struct timespec* end, struct timespec* duration) {
	// TODO gérer diffs > 1s
	if (start->tv_sec < end->tv_sec)
		duration->tv_nsec = end->tv_nsec + (1000000000L - start->tv_nsec);
	else
		duration->tv_nsec = end->tv_nsec - start->tv_nsec;
}

// Solution dite "naïve", qui ne prend pas en compte les latences de l'OS (même si RTOS)
void naiveSleepUntil(struct timespec* sleepEndTime) {
	struct timespec sleepStartTime, sleepDuration;
	sleepDuration.tv_sec = 0;
	clock_gettime(CLOCK_MONOTONIC_RAW, &sleepStartTime);
	timespecDiff(&sleepStartTime, sleepEndTime, &sleepDuration);
	nanosleep(&sleepDuration, 0);  // TODO gérer erreurs et remaining time
}

// Solution améliorée
void compensatedSleepUntil(struct timespec* sleepEndTime) {
	// TODO define du temps pris par clock_gettime
	struct timespec sleepStartTime, sleepCompensatedEndTime, sleepDuration;
	sleepDuration.tv_sec = 0;
	clock_gettime(CLOCK_MONOTONIC_RAW, &sleepStartTime);
	const long osMaxLatency_ns = OS_MAX_LATENCY_US * 1000L;
	if (sleepEndTime->tv_nsec < osMaxLatency_ns) {
		sleepCompensatedEndTime.tv_sec = sleepEndTime->tv_sec - 1;
		sleepCompensatedEndTime.tv_nsec =  1000000000L + sleepEndTime->tv_nsec - osMaxLatency_ns;
	}
	else {
		sleepCompensatedEndTime.tv_nsec = sleepEndTime->tv_nsec - osMaxLatency_ns;
		sleepCompensatedEndTime.tv_sec = sleepEndTime->tv_sec;
	}
	timespecDiff(&sleepStartTime, &sleepCompensatedEndTime, &sleepDuration);
	nanosleep(&sleepDuration, 0);  // TODO gérer erreurs et remaining time

	// à ce stade, on sait qu'on est resté endormi trop peu de temps
	// --> attente active (mais avec calls à l'OS, temps de calcul du kernel non-déterministe)
	struct timespec activePauseIterationStartTime, activePauseDuration;
	do {
		clock_gettime(CLOCK_MONOTONIC_RAW, &activePauseIterationStartTime);
		timespecDiff(&activePauseIterationStartTime, sleepEndTime, &activePauseDuration);
	}
	while (activePauseDuration.tv_nsec > CLOCK_GETTIME_AVG_DURATION_NS);
}
