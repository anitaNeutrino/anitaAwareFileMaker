/* 
 * makeTuffStatusJsonFiles
 * Cosmin Deaconu <cozzyd@kicp.uchicago.edu>
 */

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "TuffNotchStatus.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"

TuffNotchStatus *tuffPtr=0;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
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

  TTree *tuffTree = (TTree*) fp->Get("tuffTree");
  if(!tuffTree) {
      std::cerr << "Couldn't get tuffTree from " << argv[1] << "\n";
      return -1;
  }      

  tuffTree->SetBranchAddress("tuff",&tuffPtr);   


  if(tuffTree->GetEntries()<1) {
    std::cerr << "No entries in tuffTree\n";
    return -1;
  }
   
  
  tuffTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)tuffPtr->getUnixTime(),(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=tuffPtr->getRun();
  std::cout << tuffPtr->getRun() << "\t" << tuffPtr->getUnixTime() << "\n";

  //Now we set up out run list
  Long64_t numEntries=tuffTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");


  for(Long64_t event=0;event<numEntries;event++) {

    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    tuffTree->GetEntry(event);


    
    TTimeStamp timeStamp((time_t)tuffPtr->getUnixTime(),(Int_t)0);
    if(lastTime<tuffPtr->getUnixTime())    lastTime=tuffPtr->getUnixTime();

    char elementName[512]; 
    char elementLabel[512]; 

    for( int i=0; i<NUM_TUFF_NOTCHES; ++i ) {
      sprintf(elementName,"notch%dstart",i);
      sprintf(elementLabel,"Notch %d start phi sector (1-16, or 0 if disabled)", i);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,tuffPtr->getStartSector(i) + 1);
      sprintf(elementName,"notch%dstop",i);
      sprintf(elementLabel,"Notch %d stop phi sector (1-16, or 0 if disabled)",i);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,tuffPtr->getEndSector(i) + 1);
    }

    for( int i=0; i<NUM_RFCM; ++i ) 
    {

      sprintf(elementName,"irfcm%dtemp",i);
      sprintf(elementLabel,"IRFCM %d temp", i);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,tuffPtr->getIRFCMTemperature(i));
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
  summaryFile.writeSingleFullJSONFile(dirName,"tuff");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/tuffSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/tuffTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lasttuff",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
