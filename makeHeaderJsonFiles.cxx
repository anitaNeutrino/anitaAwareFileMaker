////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeHeaderJsonFiles
////      This is a simple program that converts ANITA header root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "RawAnitaHeader.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Include
#include "AnitaAwareHandler.h"
#include "AwareRunDatabase.h"


RawAnitaHeader *hdPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/headFile13.root\n";  
}


int main(int argc, char **argv) {
  if(argc<2) {
    usage(argv);
    return -1;
  }


  TFile *fp = TFile::Open(argv[1]);
  if(!fp) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *headTree = (TTree*) fp->Get("headTree");
  if(!headTree) {
    std::cerr << "Can't find headTree\n";
    return -1;
  }

  if(headTree->GetEntries()<1) {
    std::cerr << "No entries in headTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  headTree->SetBranchAddress("header",&hdPtr);
  headTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();  
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=hdPtr->run;

  //Now we set up out run list
  Long64_t numEntries=headTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");

  AnitaAwareHandler awareHandler;
  awareHandler.startHeaderFile(runNumber,dateInt);
  
  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Header Entry
    headTree->GetEntry(event);
    awareHandler.addHeader(hdPtr);

    if(hdPtr->triggerTime>lastTime) lastTime=hdPtr->triggerTime;
    
  }
  std::cerr << "\n";
  awareHandler.finishHeaderFile();

  char outputDir[FILENAME_MAX];
  char *outputDirEnv=getenv("AWARE_OUTPUT_DIR");
  if(outputDirEnv==NULL) {
    sprintf(outputDir,"/unix/anita1/data/aware/output");
  }
  else {
    strncpy(outputDir,outputDirEnv,FILENAME_MAX);
  }
    

  char outName[FILENAME_MAX];


  sprintf(outName,"%s/%s/lastHeader",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
