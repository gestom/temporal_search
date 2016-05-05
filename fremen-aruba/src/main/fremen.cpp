#include <iostream>
#include <fstream>	
#include <cstdlib>	
#include "CFrelement.h"
#include "CFregement.h"
#include "CTimer.h"
#include "support.h"
#include "graph.h"
#define MAX_SIGNAL_LENGTH 10000000
#define MAX_PLAN_LENGTH 100 

extern bool debug;

typedef struct
{
	int placeSequence[MAX_PLAN_LENGTH];
	float timeSequence[MAX_PLAN_LENGTH];
	int length;
}SPlan;

unsigned char *timeline; 
extern TEMPORAL element[NUM_ELEMENTS];
extern const char *locationStr[];
imr::Graph graph;


SPlan establishPlan(float probabilities[])
{
  SPlan plan;
  imr::Graph::Probabilities probs;
  for(unsigned int i=0;i<graph.nodes.size();i++) {
    probs.push_back(probabilities[i]);
  }    
  graph.setProbabilities(probs);
  imr::Graph::PathSolution solution = graph.getPath();
  std::cout << "Best path (exp. time = " << solution.expectedTime << "): " << std::endl;
  graph.displayPath(solution.path,solution.pathTimes,true);
  
  plan.length = solution.path.size();
  for(unsigned int i=0;i<plan.length;i++) {
    plan.placeSequence[i] = solution.path[i];
    plan.timeSequence[i] = solution.pathTimes[i];
  }
  
  return plan;
}

/*checks when the robot actually arrives at the correct location*/
float evaluatePlan(SPlan plan,int realPlace)
{
	float result = plan.timeSequence[plan.length-1];
	for (int i=0;i<plan.length;i++)
	{
		if (plan.placeSequence[i]==realPlace)
		{
			result = plan.timeSequence[i];	
		 	break;
		}
	}	
	printf("The object found in %i at %0.3f\n",realPlace,result);
	return result;
}

/*performs the actual benchmark - 
 * model file contains a model build in the  'construct' or 'convert' phases 
 * time file is the ground truth constructed from the Aruba dataset
 * startTime - starting time of testing (dataset time)
 * endTime - ending time of testing (dataset time)
 * interval - how often to perform the test
 * e.g. startTime = 86400, endTime=172800 interval=60 will perform 1440 searches (one search per minute) during the second dataset day*/
float establishTime(char *modelFile,char *timeFile,int startTime,int endTime,int interval)
{
	timeline = (unsigned char*)malloc(MAX_SIGNAL_LENGTH);

	//read the ground truth
	FILE *file=fopen(timeFile,"r");
	int x;
	int signalLength = 0; 
	if (file != NULL){
		while (feof(file)==0){	
			fscanf(file,"%i\n",&x);
			timeline[signalLength++] = x;
		}
	}else{
		fprintf(stdout,"File %s not found\n",timeFile);
		return 1;	
	}
	fclose(file);

	//load the models and recover the probabilities
	float *estimated = (float *)malloc(NUM_ELEMENTS*MAX_SIGNAL_LENGTH*sizeof(float));
	bool omniscient = false;
	if (strcmp(modelFile,"omniscient")==0) omniscient = true;
	if (omniscient == false){
		loadModel(modelFile);

		printf("Preparing plan .\n");
		CFFTPlan learningPlan;
		learningPlan.prepare(signalLength);


		printf("Calculating predictions .\n");
		for (int j=0;j<NUM_ELEMENTS;j++)
		{
			element[j].estimate(&estimated[j*signalLength],&learningPlan,0,signalLength);
			printf("Estimated timeline for the element %i.\n",j);
		}
	}
	//run evaluation
	float sumSearch = 0;
	int numSearch = 0;
	float sumFound = 0;
	float searchTime = 0;
	int numFound = 0;
	SPlan plan;
	int evaluationLength = (endTime-startTime)/interval;
	for (int i=startTime;i<endTime;i+=interval)
	{
		float probabilities[NUM_ELEMENTS];
		if (omniscient){
			for (int j=0;j<NUM_ELEMENTS;j++) probabilities[j] = 0;
			probabilities[timeline[i]] = 1;
		}else{
			for (int j=0;j<NUM_ELEMENTS;j++) probabilities[j] = estimated[j*signalLength+i];
		}
		if (debug){
			for (int j=0;j<NUM_ELEMENTS;j++) printf("%.4f ",element[j].estimate(i));
			printf("\n");
			for (int j=0;j<NUM_ELEMENTS;j++) printf("%.4f ",probabilities[j]);
			printf("\n");
		}
		plan=establishPlan(probabilities);
		searchTime = evaluatePlan(plan,timeline[i]);
	
		if (timeline[i]!=NUM_ELEMENTS-1)
		{
			sumFound += searchTime;
			numFound ++;
		}
		if (numSearch%1000 == 0) fprintf(stdout,"Evaluation step %09i of %09i \r",numSearch,evaluationLength);
		sumSearch += searchTime;
		numSearch++;
	}
	printf("Model used: %s - average found time %.3f \n",modelFile,sumFound/numFound);		//mean time it takes to find a person if a person is present 
	printf("Model used: %s - average rescue time %.3f \n",modelFile,sumSearch/numSearch);		//mean time it takes to conclude a search 
	return sumFound/numFound;
}

int constructTest(const char* name,int signalLengtha)
{
	timeline = (unsigned char*)malloc(MAX_SIGNAL_LENGTH);

	//read the relevant file 
	FILE *file=fopen(name,"r");
	int x;
	int signalLength = 0; 
	if (file != NULL){
		while (feof(file)==0){	
			fscanf(file,"%i\n",&x);
			timeline[signalLength++] = x;
		}
	}else{
		fprintf(stdout,"File %s not found\n",name);
		return 1;	
	}
	fclose(file);
	
	CFregement frege;
	CFFTPlan *bla;
	frege.build(timeline,signalLength,bla);
	//frege.print();
}

int main(int argc,char *argv[])
{
	//model construction - read MATLAB-build GMM models
	if (strcmp(argv[1],"convert")==0) convertgmm();

	//construct a spatiotemporal model from data
	if (strcmp(argv[1],"construct")==0) construct(argv[2],atoi(argv[3]),atoi(argv[4]),argv[5]);

	//predict person presence
	if (strcmp(argv[1],"predict")==0) predict(argv[2],atoi(argv[3]));

	//establish the time it takes to find a person
	if (strcmp(argv[1],"establish")==0) {
		graph.load(argv[7]);
		graph.display();
		establishTime(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
	}

	//auxiliary functions
	if (strcmp(argv[1],"test_reconstruction")==0) basicTest(argv[2]);
	if (strcmp(argv[1],"test_gmm")==0) constructTest(argv[2],atoi(argv[3]));
	return 0;
}
