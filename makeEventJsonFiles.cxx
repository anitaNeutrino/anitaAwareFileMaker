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


  AnitaGeomTool *fGeomTool = AnitaGeomTool::Instance();

  char waveformLabel[NUM_DIGITZED_CHANNELS][20];
  for(int ci=0;ci<NUM_DIGITZED_CHANNELS;ci++) {
    int surf,chan;
    fGeomTool->getSurfChanFromChanIndex(ci,surf,chan);
    sprintf(waveformLabel[ci],"SURF %d -- Chan %d",surf,chan);
  }

  headTree->GetEntry(0);
  TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
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

  AwareEventDatabase eventDb(outputDir,instrumentName,dateInt,runNumber);
  AwareRunSummaryFileMaker summaryFile(runNumber,instrumentName);   


  //  numEntries=200;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }
    
    headTree->GetEntry(event);
    eventTree->GetEntry(event);
    UsefulAnitaEvent realEvent(rawEvPtr,WaveCalType::kVoltageTime);
    

    Int_t eventNumber=hdPtr->eventNumber;
    TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
    
    char outName[FILENAME_MAX];
    sprintf(outName,"%s/events%d",dirName,eventNumber-(eventNumber%1000));
    gSystem->mkdir(outName,kTRUE);
    sprintf(outName,"%s/events%d/event%d.json.gz",dirName,eventNumber-(eventNumber%1000),eventNumber);

    Int_t numChannels=NUM_DIGITZED_CHANNELS;
    Int_t numTrigChannels=0;

    //#ifdef MAKE_EVENT_FILES
    AwareWaveformEventFileMaker fileMaker(runNumber,eventNumber,instrumentName,outName);    
    fileMaker.addVariableToEvent("time",timeStamp.AsString("sl"));
    fileMaker.addVariableToEvent("triggerTime",(int)hdPtr->triggerTimeNs);
    fileMaker.addVariableToEvent("numTrigChannels",numTrigChannels);
    TGraph *gr[NUM_DIGITZED_CHANNELS]={0};
    //#endif

    Double_t rmsValues[NUM_DIGITZED_CHANNELS];        
    for( int i=0; i<numChannels; ++i ) {
      gr[i] = realEvent.getGraph(i);   
      rmsValues[i]=gr[i]->GetRMS(2);
      Double_t deltaT=gr[i]->GetX()[gr[i]->GetN()-1]-gr[i]->GetX()[0];
      deltaT/=(gr[i]->GetN()-1);
      //#ifdef MAKE_EVENT_FILES
      if(gr[i]) {
	AwareWaveformChannel awareChan(gr[i],i,waveformLabel[i]);
	//	awareChan.addVariableToChannel("inTrig",triggerBits[i]);
	//	std::cout << "make: " << eventNumber << "\t" << i << "\t" << deltaT << "\n";
	fileMaker.addChannelToFile(awareChan);
      }      
      //#endif
    }
    //#ifdef MAKE_EVENT_FILES
    fileMaker.writeFile();
    eventDb.addEventToList(eventNumber);
      
    for( int i=0; i<numChannels; ++i ) {
      delete gr[i];
    }
    //#endif
  }
  std::cerr << "\n";

  char outName[FILENAME_MAX];
  //#ifdef MAKE_EVENT_FILES
  //The event list
  sprintf(outName,"%s/eventList.json",dirName);
  eventDb.writeEventList(outName);
  //#endif
  //Now loop over thingies add them to the thingy and do the thingy
  
  
  // char outName2[FILENAME_MAX];
  // sprintf(outName2,"%s/full",dirName);
  // gSystem->mkdir(outName2,kTRUE);

  // summaryFile.writeFullJSONFiles(outName2,"header");
  // sprintf(outName,"%s/headerSummary.json.gz",dirName);
  // summaryFile.writeSummaryJSONFile(outName);

  // sprintf(outName,"%s/headerTime.json.gz",dirName);
  // summaryFile.writeTimeJSONFile(outName);
  

  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);

  sprintf(outName,"%s/%s/lastEvent",outputDir,instrumentName);  
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);

}
