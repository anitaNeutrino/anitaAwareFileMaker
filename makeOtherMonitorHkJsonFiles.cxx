////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeOtherMonitorHkJsonFiles
////      This is a simple program that converts ANITA Other Monitor hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "OtherMonitorHk.h"
#include "CalibratedHk.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


OtherMonitorHk *otherPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/prettyHkFile13.root\n";  
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
  TTree *otherTree = (TTree*) fp->Get("otherTree");
  if(!otherTree) {
    std::cerr << "Can't find otherTree\n";
    return -1;
  }

  if(otherTree->GetEntries()<1) {
    std::cerr << "No entries in otherTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  otherTree->SetBranchAddress("othermon",&otherPtr);
  
  otherTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)otherPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=otherPtr->run;


  //Now we set up out run list
  Long64_t numEntries=otherTree->GetEntries();
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

    //This line gets the Hk Entry
    otherTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)otherPtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< realEvPtr->

    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    
    sprintf(elementName,"ramDiskInodes");
    sprintf(elementLabel,"Ramdisk Inodes");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,otherPtr->ramDiskInodes);

    sprintf(elementName,"runStartTime");
    sprintf(elementLabel,"Run Start Time");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,otherPtr->runStartTime);

    sprintf(elementName,"runStartEventNumber");
    sprintf(elementLabel,"Run Start Event");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,otherPtr->runStartEventNumber);


    for( int i=0; i<3; ++i ) {
      sprintf(elementName,"dirFiles%d",i);
      strcpy(elementLabel,otherPtr->getDirName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,otherPtr->dirFiles[i]);
    }      

    for( int i=0; i<3; ++i ) {
      sprintf(elementName,"dirLinks%d",i);
      strcpy(elementLabel,otherPtr->getDirName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,otherPtr->dirLinks[i]);
    }      

    for(int bit=0;bit<NUM_PROCESSES;bit++) {
      sprintf(elementName,"procBit%d",bit);
      sprintf(elementLabel,"%s",otherPtr->getProcName(bit));
      int value=otherPtr->isInProcessMask(bit);
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
  summaryFile.writeFullJSONFiles(fullDir,"other");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/otherSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/otherTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastOther",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
