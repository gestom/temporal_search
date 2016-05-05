#include "CFregement.h"

using namespace std;
static bool debug = true; 

bool fremenSort(SFregement i,SFregement j) 
{ 
	return (i.amplitude>j.amplitude); 
}

CFregement::CFregement()
{
	outlierSet = NULL;
	fregements = NULL;
	signalLength = gain = outliers = order = 0;
}

CFregement::~CFregement()
{
	free(fregements);
	free(outlierSet);
}

void CFregement::build(unsigned char* signal,int signalLengthi,CFFTPlan *plan)
{
	signalLength = 24*3600;
	int ratio = signalLengthi/signalLength;
	float *input = (float*)calloc(signalLength,sizeof(float));
	float *refined = (float*)calloc(signalLength,sizeof(float));
	float *output = (float*)calloc(signalLength,sizeof(float));
	float *smooth = (float*)calloc(signalLength,sizeof(float));
	int offset = 0;

	//summarize over all days
	for (int i=0;i<signalLengthi;i++) input[i%signalLength]+=signal[i];
	for (int i=0;i<signalLength;i++) input[i]/=ratio;

	//smooth it and find a global minimum
	float sum = 0;
	int smoothLength = signalLength/2;
	for (int i=signalLength-smoothLength;i<signalLength;i++) sum+=input[i]; 
	for (int i=signalLength;i<signalLength*2;i++){
		 sum = sum - input[(i-smoothLength)%signalLength] + input[i%signalLength];
		 smooth[(i-smoothLength/2)%signalLength] = sum; 
	}

	float minimum = 10.0;
	for (int i=0;i<signalLength;i++){
		 smooth[i]/=smoothLength;
		 if (minimum > smooth[i])
		 {
			minimum = smooth[i];
			offset = i;
		 }
	}
	for (int i=0;i<signalLength;i++) refined[i]=input[(i+offset)%signalLength];
	sum = 0;
	for (int i=0;i<signalLength;i++) sum+=input[i];
	
	//initialize Gaussianu
	order = 3;
	int meanDistance = signalLength/(order+1);
	fregements = (SFregement*) calloc(order,sizeof(SFregement));
	for (int i = 0;i<order;i++)
	{
	 	fregements[i].mean = meanDistance*(i+1);
	 	fregements[i].amplitude = 1.0/order;
	 	fregements[i].sigma = signalLength/order/2;
	 	fregements[i].estimate = (float*)calloc(signalLength,sizeof(float));
	}

	//expectation step - calculate Gaussians
	float *y;
	float w,m,s;
	for (int steps = 0;steps<100;steps++){
		memset(output,0,signalLength*sizeof(float));
		for (int i = 0;i<order;i++)
		{
			y = fregements[i].estimate;
			w = fregements[i].amplitude;
			m = fregements[i].mean;
			s = fregements[i].sigma;
			for (int t=0;t<signalLength;t++){
				y[t] = w*exp(-(t-m)*(t-m)/(2*s*s))/(sqrt(2*M_PI)*s);
				output[t]+=y[t];
			}
		}	
		//maximization step
		for (int i = 0;i<order;i++)
		{
			w = 0;
			m = 0;
			s = 0;
			y = fregements[i].estimate;
			for (int t=0;t<signalLength;t++)
			{
				y[t]=y[t]/output[t]*refined[t]/sum;
				m+=t*y[t];	
				w+=y[t];	
			}
			m = m/w;
			for (int t=0;t<signalLength;t++) s+= (t-m)*(t-m)*y[t];
			fregements[i].amplitude = w;
			fregements[i].mean = m;
			fregements[i].sigma = sqrt(s/w);
		}

	}
	memset(output,0,signalLength*sizeof(float));
	for (int i = 0;i<order;i++)
	{
		y = fregements[i].estimate;
		w = fregements[i].amplitude;
		m = fregements[i].mean;
		s = fregements[i].sigma;
		for (int t=0;t<signalLength;t++){
			y[t] = w*exp(-(t-m)*(t-m)/(2*s*s))/(sqrt(2*M_PI)*s)*sum;
			output[t]+=y[t];
		}
	}

	//print the summary
	for (int i=0;i<signalLength;i++){
		for (int j = 0;j<order;j++)printf("%f ", fregements[j].estimate[i]);
		printf("%f %f %f\n",output[i],refined[i],smooth[i]);
	}
		
	for (int i=0;i<signalLength;i++) refined[i]=input[(i+offset)%signalLength];
	free(input);
	free(smooth);
	return;
}

void CFregement::update(int modelOrder,CFFTPlan *plan)
{
	if (order == 0 && outliers == 0){
		printf("nothing to do!\n");
	}else{
		unsigned char *reconstructed = (unsigned char*)malloc(signalLength*sizeof(unsigned char));
		reconstruct(reconstructed,plan);
		order = modelOrder;
		outliers = 0;
		free(fregements);
		fregements = (SFregement*) malloc(order*sizeof(SFregement));
		if (fregements == NULL) fprintf(stderr,"Failed to reallocate spectral components!\n");
		build(reconstructed,signalLength,plan);
		free(reconstructed);
	}
}

float CFregement::estimate(float* signal,CFFTPlan *plan,float anomalyThreshold,int signalLengthi)
{
	CTimer timer;
	timer.start();

	float *probability = (float*)malloc(signalLength*sizeof(float));	
	int repeats = signalLengthi/signalLength;
	memset(signal,0,signalLength*sizeof(float));
	if (debug) printf("GMM calculation %i %i\n",timer.getTime(),repeats);
	for (int i=0;i<order;i++){
		float scale = fregements[i].amplitude/(sqrt(2*M_PI)*fregements[i].sigma);
		float exponent = 0; 
		for (int x=0;x<signalLength;x++)
		{
			exponent = (x-fregements[i].mean)/fregements[i].sigma;
			signal[x]+= scale*exp(-exponent*exponent/2);
		} 
	}
	for (int i = 0;i<phaseShift;i++) probability[i] = signal[signalLength-phaseShift+i]; 
	for (int i = 0;i<signalLength-phaseShift;i++) probability[i+phaseShift] = signal[i]; 
	for (int i=0;i<repeats;i++) memcpy(&signal[signalLength*i],probability,signalLength*sizeof(float));

	for (int i = 0;i<signalLengthi;i++){
		if (signal[i]  > 1.0) signal[i] = 1.0;
		if (signal[i]  < 0.0) signal[i] = 0.0;
	}
//	for (int i = 0;i<signalLength;i++)printf("%f\n",signal[i]);
	return 0;
}

float CFregement::reconstruct(unsigned char* signal,CFFTPlan *plan,bool evaluate)
{
	float evaluation = -1; 
	CTimer timer;
	timer.start();

	int fftLength = signalLength/2+1;

	fftw_complex *coeffs;
	double *probability;

	probability = plan->probability; 
	coeffs = plan->coeffs;
	
	/*reconstructing the frequency spectrum*/
	if (order > 0){	
		memset(coeffs,0,fftLength*sizeof(fftw_complex));
		coeffs[0][0] = gain;
		for (int i=0;i<order;i++){
			coeffs[fregements[i].frequency][0] = fregements[i].amplitude*cos(fregements[i].phase);
			coeffs[fregements[i].frequency][1] = fregements[i].amplitude*sin(fregements[i].phase);
		}
		//cout << "IFFT preparation " << timer.getTime() << endl;
		fftw_execute_dft_c2r(plan->inverse,coeffs,probability);
		//for (int i = 0;i<signalLength;i++) cout << "Pro " << probability[i] << " " << estimate(i) << endl;
	}else{
		for (int i = 0;i<signalLength;i++) probability[i] = gain;
	}
	if (debug) cout << "IFFT calculation " << timer.getTime() << endl;

	/*application of the outlier set*/
	int j=0;
	unsigned char flip = 0;
	timer.reset();
	if (outliers > 0){
		int flipPos = outlierSet[j];
		for (int i = 0;i<signalLength;i++)
		{
			if (flipPos == i){
				flip = 1-flip;
				j++;
				if (j >= outliers) j = outliers-1; 
				flipPos = outlierSet[j];
			}
			signal[i] = ((probability[i]>=0.5)^flip);
		}
	}else{
		for (int i = 0;i<signalLength;i++) signal[i] = probability[i]>0.5;
	}
	if (evaluate){
		 evaluation = 0;
		 for (int i = 0;i<signalLength;i++) evaluation+=fabs(signal[i]-probability[i]);
		 evaluation/=signalLength;
	}
	if (debug) cout << "Signal reconstruction time " << timer.getTime() << endl;

	return evaluation;
}

/*gets length in terms of values measured*/
unsigned int CFregement::getLength()
{
	return signalLength;
}

void CFregement::add(unsigned char value)
{
	if (((estimate(signalLength) > 0.5)^((outliers%2)==1))!=value)
	{
		unsigned int* outlierSetTmp = (unsigned int*)realloc(outlierSet,(outliers+1)*(sizeof(unsigned int)));
		if (outlierSetTmp==NULL) fprintf(stderr,"Failed to reallocate the outlier set!\n"); 
		outlierSet = outlierSetTmp;
		outlierSet[outliers++] = signalLength;
	}
	signalLength++;
	return; 
}

/*text representation of the fremen model*/
void CFregement::print(bool verbose)
{
	int errs = 0;
	for (int i=0;i<outliers/2;i++) errs+=(outlierSet[2*i+1]-outlierSet[2*i]);
	if (outliers%2 == 1) errs+=signalLength-outlierSet[outliers-1];
	std::cout << "model order " << (int)order << " : " << gain << " error: " << ((float)errs/signalLength) << " size: " << signalLength << " ";
	if (order > 0) std::cout  << endl;
	if (verbose||true){
		for (int i = 0;i<order;i++){
			std::cout << "fregement " << i << " " << fregements[i].amplitude << " " << fregements[i].mean << " " << fregements[i].sigma << " " << fregements[i].frequency << " " << endl;
		}
	}
	if (verbose){
	std::cout << "outlier set size " << outliers << ":";
		for (int i = 0;i<outliers;i++) std::cout << " " << outlierSet[i];
	}
	std::cout << endl; 
}


/*retrieves a boolean*/
unsigned char CFregement::retrieve(int timeStamp)
{
	int i = 0;
	for (i= 0;i<outliers;i++){
		if (timeStamp < outlierSet[i]) break;
	}
	return (estimate(timeStamp) >= 0.5)^(i%2);
}
 
int CFregement::save(char* name,bool lossy)
{
	FILE* file = fopen(name,"w");
	fwrite(&signalLength,sizeof(unsigned int),1,file);
	save(file,lossy);
	fclose(file);
	return 0;
}

int CFregement::load(char* name)
{
	FILE* file = fopen(name,"r");
	if (fread(&signalLength,sizeof(unsigned int),1,file)!=1) return -1;
	load(file);
	fclose(file);
	return 0;
}


int CFregement::save(FILE* file,bool lossy)
{
	unsigned int outlierNum = outliers;
	unsigned char code = 255;
	if (order == 0 && outliers == 0)
	{
		if (gain == 1) code=254;
		fwrite(&code,sizeof(unsigned char),1,file);
	}else{ 
		if (lossy) outlierNum = 0; 
		fwrite(&order,sizeof(unsigned char),1,file);
		fwrite(&outlierNum,sizeof(unsigned int),1,file);
		fwrite(&gain,sizeof(float),1,file);
		fwrite(&phaseShift,sizeof(int),1,file);
		fwrite(fregements,sizeof(SFregement),order,file);
		fwrite(outlierSet,sizeof(unsigned int),outlierNum,file);
	}
	return 0;
}

int CFregement::loadTxt(FILE* file)
{
	float a;
	fscanf(file,"%f\n",&a);
	signalLength = a;
	fscanf(file,"%f\n",&a);
	phaseShift = a;
	fscanf(file,"%f\n",&a);
	order = a;
//	printf("%i %i %i\n",signalLength,phaseShift,order);
	fregements = (SFregement*) malloc(order*sizeof(SFregement));
	for (int i = 0;i<order;i++)fregements[i].frequency = 1;	
	for (int i = 0;i<order;i++)fscanf(file,"%f\n",&fregements[i].amplitude);	
	for (int i = 0;i<order;i++)fscanf(file,"%f\n",&fregements[i].mean);	
	for (int i = 0;i<order;i++)fscanf(file,"%f\n",&fregements[i].sigma);
	return 0;	
}

int CFregement::load(FILE* file)
{
	int ret =0;
/*	order = 3;
	outliers = 0;
	signalLength = 86400;
	phaseShift=64432; 
	fregements = (SFregement*) malloc(order*sizeof(SFregement));
	fregements[0].frequency=1; 
	fregements[0].amplitude=0.3660*31958; 
	fregements[0].mean=32595; 
	fregements[0].sigma=9221;
 
	fregements[1].frequency=1; 
	fregements[1].amplitude=0.4207*31958; 
	fregements[1].mean=37904; 
	fregements[1].sigma=10375;
 
	fregements[2].frequency=1; 
	fregements[2].amplitude=0.2133*31958; 
	fregements[2].mean=45879; 
	fregements[2].sigma=13902; */
	ret+=fread(&order,sizeof(unsigned char),1,file);
	if (order>250){
		gain = 0;
		if (order == 254) gain = 1;
		outliers = 0;
		order = 0;
		if (ret != 1) ret = -1; else ret = 0;
	}else{
		ret+=fread(&outliers,sizeof(unsigned int),1,file);
		ret+=fread(&gain,sizeof(float),1,file);
		ret+=fread(&phaseShift,sizeof(int),1,file);
		free(outlierSet);
		free(fregements);
		fregements = (SFregement*) malloc(order*sizeof(SFregement));
		outlierSet = (unsigned int*)malloc(outliers*(sizeof(unsigned int)));
		ret+=fread(fregements,sizeof(SFregement),order,file);
		ret+=fread(outlierSet,sizeof(unsigned int),outliers,file);
		if (ret != 4+outliers+order) ret = -1; else ret = 0;
	}
	return ret;
}

float CFregement::estimate(int timeStamp)
{
	float time = (float)timeStamp/signalLength;
	float estimate = gain;
	for (int i = 0;i<order;i++){
		estimate+=2*fregements[i].amplitude*cos(time*fregements[i].frequency*2*M_PI+fregements[i].phase);
	}
	if (estimate > 1.0) return 1.0;
	if (estimate < 0.0) return 0.0;
	return estimate;
}

float CFregement::fineEstimate(float timeStamp)
{
	float time = (float)timeStamp/signalLength;
	float estimate = gain;
	for (int i = 0;i<order;i++){
		estimate+=2*fregements[i].amplitude*cos(time*fregements[i].frequency*2*M_PI+fregements[i].phase);
	}
	if (estimate > 1.0) return 1.0;
	if (estimate < 0.0) return 0.0;
	return estimate;
}
