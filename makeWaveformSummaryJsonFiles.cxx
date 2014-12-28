////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeEventJsonFiles 
////      This is a simple program that converts event root files into JSON
////      files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////



//Includes
#include <iostream>
#include <cstdio>

//Event Reader Root Includes
#include "RawAnitaEvent.h"
#include "UsefulAnitaEvent.h"
#include "RawAnitaHeader.h"
#include "AnitaGeomTool.h"
#include "AnitaEventCalibrator.h"
#include "FFTtools.h"

//AWARE includes
#include "AwareWaveformChannel.h"
#include "AwareWaveformEventFileMaker.h"
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"
#include "AwareEventDatabase.h"

//ROOT Includes
#include "TTree.h"
#include "TTreeIndex.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

#include <map>

RawAnitaEvent *rawEvPtr=0;
RawAnitaHeader *hdPtr =0;


void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <header file> <event file>\n";
  std::cout << "e.g.\n" << argv[0] << " head10.root event10.root  0\n";
}




int main(int argc, char **argv) {
  if(argc<3) {
    usage(argv);
    return -1;
  }

  TFile *fpHead = TFile::Open(argv[1]);
  if(!fpHead) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *headTree = (TTree*) fpHead->Get("headTree");
  if(!headTree) {
    std::cerr << "Can't find headTree\n";
    return -1;
  }
  headTree->SetBranchAddress("header",&hdPtr);

  TFile *fp = TFile::Open(argv[2]);
  TTree *eventTree = (TTree*) fp->Get("eventTree");
  if(!eventTree) {
    std::cerr << "Can't find eventTree\n";
    return -1;
  }
  eventTree->SetBranchAddress("event",&rawEvPtr);
   


  Long64_t numEntries=eventTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;


  //  AnitaGeomTool *fGeomTool = AnitaGeomTool::Instance();

  char waveformLabel[NUM_PHI][3][2][20];
  int phi=0;
  AnitaRing::AnitaRing_t ring;
  AnitaPol::AnitaPol_t pol;
  for(phi=0;phi<NUM_PHI;phi++) {
    for(int ringInd=0;ringInd<3;ringInd++) {
      ring=AnitaRing::AnitaRing_t(ringInd);
      for(int polInd=0;polInd<2;polInd++) {
	pol=AnitaPol::AnitaPol_t(polInd);
	
	sprintf(waveformLabel[phi][ring][pol],"%d%c%c",phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));      
      }
    }
  }
    
    

  headTree->GetEntry(0);
  TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
  UInt_t dateInt=timeStamp.GetDate();
  //  UInt_t firstTime=timeStamp.GetSec();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=hdPtr->run;

  char outputDir[FILENAME_MAX];
  char *outputDirEnv=getenv("AWARE_OUTPUT_DIR");
  if(outputDirEnv==NULL) {
    sprintf(outputDir,"/unix/anita/data/aware/output");
  }
  else {
    strncpy(outputDir,outputDirEnv,FILENAME_MAX);
  }
    

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");

  char dirName[FILENAME_MAX];
  char dateDirName[FILENAME_MAX];
  char subDateDirName[FILENAME_MAX];
  sprintf(dirName,"%s/%s/runs%d/runs%d/run%d/",outputDir,instrumentName,runNumber-runNumber%10000,runNumber-runNumber%100,runNumber);
  sprintf(dateDirName,"%s/%s/%d/%04d/run%d/",outputDir,instrumentName,dateInt/10000,dateInt%10000,runNumber);
  sprintf(subDateDirName,"%s/%s/%d/%04d/",outputDir,instrumentName,dateInt/10000,dateInt%10000);
  gSystem->mkdir(dirName,kTRUE);
  gSystem->mkdir(subDateDirName,kTRUE);
  gSystem->Symlink(dirName,dateDirName);


  std::cout << "Making: " << dirName << "\n";

  AwareRunSummaryFileMaker summaryFile(runNumber,instrumentName);   


  //  numEntries=200;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }
    
    headTree->GetEntry(event);
    eventTree->GetEntry(event);
    UsefulAnitaEvent realEvent(rawEvPtr,WaveCalType::kVTFast);
    

    Int_t eventNumber=hdPtr->eventNumber;
    TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
    if(hdPtr->triggerTime>lastTime) lastTime=hdPtr->triggerTime;

    if(hdPtr->triggerTime<100000) continue;
    //    std::cout << hdPtr->triggerTime << "\n";
    
    char elementName[180];

    int chan=8;
    int countGood=0;
    for(int surf=0;surf<ACTIVE_SURFS;surf++) {
      TGraph *grClock= realEvent.getGraphFromSurfAndChan(surf,chan);
      if(grClock) {
	//	std::cout << surf << "\t" << grClock->GetN() << "\t" << grClock->GetRMS(2) << "\n";	
	if(grClock->GetRMS()>10) countGood++;
	delete grClock;
      }      
    }
    if(countGood!=ACTIVE_SURFS) {
      //Lets skip the event
      continue;
    }


    
    for(phi=0;phi<NUM_PHI;phi++) {
      for(int ringInd=0;ringInd<3;ringInd++) {
	ring=AnitaRing::AnitaRing_t(ringInd);
	for(int polInd=0;polInd<2;polInd++) {
	  pol=AnitaPol::AnitaPol_t(polInd);
	  TGraph *gr = realEvent.getGraph(ring,phi,pol);
	  

	  //The bit below is just to get an FFT
	  Double_t newX[248],newY[248];
	  TGraph *grInt = FFTtools::getInterpolatedGraph(gr,1./2.6);
	  Int_t numSamps=grInt->GetN();
	  Double_t *xVals=grInt->GetX();
	  Double_t *yVals=grInt->GetY();
	  for(int i=0;i<248;i++) {
	    if(i<numSamps) {
	      newX[i]=xVals[i];
	      newY[i]=yVals[i];
	    }
	    else {
	      newX[i]=newX[i-1]+(1./2.6);
	      newY[i]=0;
	    }      
	  }
	  TGraph *grNew = new TGraph(248,newX,newY);
	  TGraph *grFFT = FFTtools::makePowerSpectrumMilliVoltsNanoSecondsdB(grNew);




	  //Add the waveform RMS
	  Double_t rms=gr->GetRMS(2);      
	  sprintf(elementName,"rms%d_%c%c",phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));     
	  summaryFile.addVariablePoint(elementName,waveformLabel[phi][ring][pol],timeStamp,rms);

	  
	  for(int bin=0;bin<grFFT->GetN();bin++) {
	    Double_t value=grFFT->GetY()[bin];
	    Double_t freq=grFFT->GetX()[bin];
	    sprintf(elementName,"db%04f_%d_%c%c",freq,phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));     
	    summaryFile.addVariablePoint(elementName,waveformLabel[phi][ring][pol],timeStamp,value);
	  }	    	    



	  //Delete the graphs
	  delete gr;
	  delete grInt;
	  delete grNew;
	  delete grFFT;
	  
	}
      }
    }        
  }
  std::cerr << "\n";

  char outName[FILENAME_MAX];
  
  char outName2[FILENAME_MAX];
  sprintf(outName2,"%s/full",dirName);
  gSystem->mkdir(outName2,kTRUE);

  summaryFile.writeFullJSONFiles(outName2,"waveform");
  sprintf(outName,"%s/waveformSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);

  sprintf(outName,"%s/waveformTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);
  
  sprintf(outName,"%s/%s/lastWaveform",outputDir,instrumentName);  
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);

}
