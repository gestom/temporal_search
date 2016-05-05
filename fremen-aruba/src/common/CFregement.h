#ifndef CFREGEMENT_H
#define CFREGEMENT_H

#include <iostream>
#include <vector>
#include <complex>	
#include <algorithm> 
#include <iterator> 
#include <complex>	// goes along with fftw
#include <fftw3.h>
#include <string.h>
#include "CTimer.h"
#include "CFFTPlan.h"
	
/**
@author Tom Krajnik
*/

using namespace std;

typedef struct
{
	float amplitude;
	float phase;
	float mean;
	float sigma;
	float *estimate;
	unsigned int frequency;
}SFregement;

class CFregement
{
public:
  CFregement();
  ~CFregement();

  float estimate(float* signal,CFFTPlan *plan,float anomalyThreshold = 1.0,int signalLengthi = -1);
  float reconstruct(unsigned char* signal,CFFTPlan *plan,bool evaluate = false);

  /*state estimation: retrieves the state*/
  float estimate(int timeStamp);
  float fineEstimate(float timeStamp);

  /*state estimation: retrieves the state*/
  unsigned char retrieve(int timeStamp);

  /*adds a single measurement*/
  void add(unsigned char value);

  /*gets length of the stored signal*/
  unsigned int getLength();

  void build(unsigned char* signal,int signalLength,CFFTPlan *plan);

  void print(bool verbose=true);

  int save(FILE* file,bool lossy = false);
  int load(FILE* file);
  int save(char* name,bool lossy = false);
  int load(char* name);
  int loadTxt(FILE* file);

  /*changes the model order*/
  void update(int modelOrder,CFFTPlan *plan);

  double *signal;

//private:
	SFregement *fregements;
	unsigned int *outlierSet;
	unsigned int outliers;
	unsigned char order;
	float gain;
	int signalLength;
	int phaseShift;
};

#endif
