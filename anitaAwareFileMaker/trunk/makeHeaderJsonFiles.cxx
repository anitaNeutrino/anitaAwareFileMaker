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

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
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


  TTimeStamp timeStamp((time_t)hdPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=hdPtr->run;

  //Now we set up out run list
  Long64_t numEntries=headTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA3",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");


  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Header Entry
    headTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)hdPtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< realEvPtr->

    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];

    sprintf(elementName,"ppsNum");
    sprintf(elementLabel,"PPS Num");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->ppsNum);

    sprintf(elementName,"c3poNum");
    sprintf(elementLabel,"C3P0 Num");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->c3poNum);

    sprintf(elementName,"eventNumber");
    sprintf(elementLabel,"Event Number");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->eventNumber);


    for(int bit=0;bit<16;bit++) {
      sprintf(elementName,"l3TrigBit%d",bit);
      sprintf(elementLabel,"L3 Phi %d",bit+1);
      int value=hdPtr->isInL3Pattern(bit);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,value);
      
    }
    
  }
  std::cerr << "\n";


  char outputDir[FILENAME_MAX];
  char *outputDirEnv=getenv("AWARE_OUTPUT_DIR");
  if(outputDirEnv==NULL) {
    sprintf(outputDir,"/unix/anita1/data/aware/output");
  }
  else {
    strncpy(outputDir,outputDirEnv,FILENAME_MAX);
  }
    


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
  

  char fullDir[FILENAME_MAX];
  sprintf(fullDir,"%s/full",dirName);
  gSystem->mkdir(fullDir,kTRUE);
  summaryFile.writeFullJSONFiles(fullDir,"header");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/headerSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/headerTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastHeader",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
