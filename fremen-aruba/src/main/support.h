#ifndef SUPPORT_H
#define SUPPORT_H

#include <iostream>
#include <fstream>	
#include <cstdlib>	
#include "CFrelement.h"
#include "CFregement.h"
#include "CTimer.h"
#define MAX_SIGNAL_LENGTH 10000000
#define NUM_ELEMENTS 10 

#ifndef TEMPORAL
#define TEMPORAL CFrelement
#endif 


using namespace std;

int convertgmm();
int saveModel(const char* filename);
int loadModel(const char* filename);
void basicTest(char *modelFile);
int construct(const char* timefile,int order,int learningLength,char* savefile);
int predict(char *modelFile,int predictionTime);
#endif //SUPPORT_H
