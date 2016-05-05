#include "support.h"

using namespace std;

bool debug = false;
TEMPORAL element[NUM_ELEMENTS];

const char *locationStr[] = {
	"Master bedroom",
	"Master bathroom",
	"Living room",
	"Kitchen",
	"Center",
	"Corridor",
	"Second bedroom",
	"Office",
	"Second bathroom",
	"Outside"
};


int convertgmm()
{
	FILE *file;
	char name[1000];
	for (int f=1;f<6;f++){
		sprintf(name,"order_%i.gmm",f);
		file=fopen(name,"r");
		if (file == NULL) return -1;
//		for (int i =0;i<NUM_ELEMENTS;i++) element[i].loadTxt(file);
		for (int i =0;i<NUM_ELEMENTS;i++) element[i].print(false);
		fclose(file);
		sprintf(name,"order_%i.gmr",f);
		saveModel(name);
	}
	return 0;

}

int saveModel(const char* filename)
{
	FILE *file=fopen(filename,"w");
	fwrite(&element[0].signalLength,sizeof(int),1,file);
	for (int i =0;i<NUM_ELEMENTS;i++){
		 element[i].save(file,true);
		 if (debug) element[i].print(false);
	}
	fclose(file);
	return 0;
}

int loadModel(const char* filename)
{
	FILE *file=fopen(filename,"r");
	if (file == NULL) return -1;
	unsigned int signalLength;
	fread(&signalLength,sizeof(int),1,file);
	for (int i =0;i<NUM_ELEMENTS;i++){
		 element[i].load(file);
	  	 element[i].signalLength = signalLength;
	}
	fclose(file);
	if (debug) for (int i =0;i<NUM_ELEMENTS;i++) element[i].print(false);
	return 0;
}

void basicTest(char *modelFile)
{
	loadModel(modelFile);
	float *estimated = (float *)malloc(MAX_SIGNAL_LENGTH*sizeof(float));
	int learningLength = 4*7*24*3600;
	CFFTPlan learningPlan;
	learningPlan.prepare(learningLength);
	for (int j=0;j<NUM_ELEMENTS;j++)
	{
		element[j].estimate(estimated,&learningPlan,0,learningLength);
		char filetest[100];
		sprintf(filetest,"LOAD_%i.txt",j);
		FILE *file=fopen(filetest,"w");
		for (int i = 0;i<learningLength/4;i++) fprintf(file,"%.4f\n",estimated[i]);
		fclose(file);
	}
	free(estimated);
}

int construct(const char* timeFile,int order,int learningLength,char* savefile)
{
	unsigned char *timeline = (unsigned char*)malloc(MAX_SIGNAL_LENGTH);
	unsigned char *signal = (unsigned char*)malloc(MAX_SIGNAL_LENGTH);
	unsigned char *reconstructed = (unsigned char*)malloc(MAX_SIGNAL_LENGTH);
	float *estimated = (float *)malloc(MAX_SIGNAL_LENGTH*sizeof(float));
	int signalLength = 0;

	FILE *file=fopen(timeFile,"r");
	//read the input file
	int x;
	if (file != NULL){
		while (feof(file)==0){	
			fscanf(file,"%i\n",&x);
			timeline[signalLength++] = x;
		}
	}else{
		fprintf(stdout,"File -%s- not found\n",timeFile);
		return 1;	
	}
	fclose(file);
	if (debug) fprintf(stdout,"Data of length %i read from file %s \n",signalLength,timeFile);
	if (signalLength == 0) {
		fprintf(stdout,"Nothing to calculate - is the file correct ?\n");
		return 1;	
	}
	for (int j=0;j<NUM_ELEMENTS;j++)
	{
		//for (int i=0;i<signalLength;i++) printf("%i\n",signal[i]);
		CTimer timer;
		timer.reset();
		timer.start();

		CFFTPlan learningPlan;
		CFFTPlan predictingPlan;

		if (debug) cout << "Preparing learning plan " << timer.getTime() << " us." << endl; 
		learningPlan.prepare(learningLength);
		if (debug) cout << "Preparing reconstruction plan " << timer.getTime() << " us." << endl; 
		predictingPlan.prepare(signalLength);

		for (int i=0;i<signalLength;i++) signal[i] = (timeline[i] == j);

		if (debug) cout << "Plan preparation time " << timer.getTime() << " us." << endl; 
		timer.reset();
		element[j].build(signal,learningLength,&learningPlan);

		if (debug) cout << "Model build time " << timer.getTime()/1000 << " ms." << endl;
		timer.reset();
		element[j].update(order,&learningPlan);

		if (debug) cout << "Model update time " << timer.getTime()/1000 << " ms." << endl;
		if (debug) element[j].print(false);
		element[j].reconstruct(reconstructed,&learningPlan,false);
		element[j].estimate(estimated,&predictingPlan,0,signalLength);

		int err = 0;
		int pos = 0;
		for (int i=0;i<learningLength;i++) err+=abs((int)signal[i]-(int)reconstructed[i]);
		if (debug) cout << "Model lossless reconstruction error - should be 0: " << (float)err/(float)learningLength << " " << pos << endl;
		int eR = 0;
		int eP = 0;
		for (int i=0;i<learningLength;i++) eR+=abs((int)signal[i]-(int)(estimated[i]>0.5));
		for (int i=0;i<signalLength;i++)   eP+=abs((int)signal[i]-(int)(estimated[i]>0.5));
		printf("Room model %i reconstruction and prediction errors: %.3f %.3f.\n",j,(float)eR/(float)learningLength,(float)eP/(float)signalLength);

		if (debug){
			char filetest[100];
			sprintf(filetest,"SAVE_%02i.txt",j);
			FILE *file=fopen(filetest,"w");
			for (int i = 0;i<learningLength/4;i++) fprintf(file,"%.4f\n",estimated[i]);
			fclose(file);
		}
	}
	free(timeline);
	free(reconstructed);
	free(estimated);
	free(signal);
	saveModel(savefile);
	return 0;
}	

int predict(char *modelFile,int predictionTime)
{
	loadModel(modelFile);
	for (int j=0;j<NUM_ELEMENTS;j++) printf("%.4f ",element[j].estimate(predictionTime));
	printf("\n");

	//testing purposes - creates files with interpolated values
	if (debug){
		for (int j=0;j<NUM_ELEMENTS;j++)
		{
			char filetest[100];
			int learningLength = 4*7*24*3600;
			sprintf(filetest,"PRED_%02i.txt",j);
			FILE *file=fopen(filetest,"w");
			for (int i = 0;i<learningLength/4;i++) fprintf(file,"%.4f\n",element[j].estimate(i));
			fclose(file);
		}
	}
	return 0;
}
