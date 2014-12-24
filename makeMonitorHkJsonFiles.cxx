////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeMonitorHkJsonFiles
////      This is a simple program that converts ANITA Monitor hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "MonitorHk.h"
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


MonitorHk *monitorPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/monitorFile13.root\n";  
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
  TTree *monitorTree = (TTree*) fp->Get("monitorTree");
  if(!monitorTree) {
    std::cerr << "Can't find monitorTree\n";
    return -1;
  }

  if(monitorTree->GetEntries()<1) {
    std::cerr << "No entries in monitorTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  monitorTree->SetBranchAddress("mon",&monitorPtr);
  
  monitorTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)monitorPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=monitorPtr->run;


  //Now we set up out run list
  Long64_t numEntries=monitorTree->GetEntries();
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
    monitorTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)monitorPtr->realTime,(Int_t)0);

    if(lastTime<monitorPtr->realTime) lastTime=monitorPtr->realTime;

    //    std::cout << "Run: "<< realEvPtr->

    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    for( int i=0; i<NUM_DISK_SPACES; ++i ) {
      sprintf(elementName,"diskSpace%d",i);
      strcpy(elementLabel,monitorPtr->getDriveName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->getDiskSpaceAvailable(i));
      sprintf(elementName,"diskPercent%d",i);
      strcpy(elementLabel,monitorPtr->getDriveName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->getDiskSpacePercentage(i));
    }      
    for( int i=0; i<NUM_HK_TELEM_QUEUES; ++i ) {
      sprintf(elementName,"hkLinks%d",i);
      strcpy(elementLabel,monitorPtr->getHkQueueName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->hkLinks[i]);
    }      
    for( int i=0; i<NUM_PRIORITIES; ++i ) {
      sprintf(elementName,"eventLinks%d",i);
      sprintf(elementLabel,"Priority %d",i);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->eventLinks[i]);
    }        
 
    for( int i=0; i<NUM_PROCESSES; ++i ) {
      sprintf(elementName,"utime%d",i);
      strcpy(elementLabel,monitorPtr->getProcName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->utime[i]);
    }       
    for( int i=0; i<NUM_PROCESSES; ++i ) {
      sprintf(elementName,"stime%d",i);
      strcpy(elementLabel,monitorPtr->getProcName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->stime[i]);
    }       
    for( int i=0; i<NUM_PROCESSES; ++i ) {
      sprintf(elementName,"vsize%d",i);
      strcpy(elementLabel,monitorPtr->getProcName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,monitorPtr->vsize[i]);
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
  summaryFile.writeFullJSONFiles(fullDir,"monitor");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/monitorSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/monitorTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastMonitor",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
