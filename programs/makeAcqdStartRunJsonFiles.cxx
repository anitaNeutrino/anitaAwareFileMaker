////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeAcqdStartRunJsonFiles
////      This is a simple program that converts ANITA Acqd Start Information
////      into JSON format.
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "AcqdStart.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


AcqdStart *acqdStartPtr;

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
  TTree *acqdStartTree = (TTree*) fp->Get("acqdStartTree");
  if(!acqdStartTree) {
    std::cerr << "Can't find acqdStartTree\n";
    return -1;
  }

  if(acqdStartTree->GetEntries()<1) {
    std::cerr << "No entries in acqdStartTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  acqdStartTree->SetBranchAddress("acqd",&acqdStartPtr);
  
  acqdStartTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)acqdStartPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=acqdStartPtr->run;


  //Now we set up out run list
  Long64_t numEntries=acqdStartTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");


  
  //    std::cout << "Run: "<< realEvPtr->
  
  //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
  //Summary file fun
  char elementName[FILENAME_MAX];
  char elementLabel[180];
  for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
    for( int chan=0; chan<CHANNELS_PER_SURF; ++chan ) {
      sprintf(elementName,"chanMean%d_%d",surf,chan);
      sprintf(elementLabel,"SURF %d Chan %d Mean",surf+1,chan+1);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,acqdStartPtr->chanMean[surf][chan]);
    }
  }
  for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
    for( int chan=0; chan<CHANNELS_PER_SURF; ++chan ) {
      sprintf(elementName,"chanRMS%d_%d",surf,chan);
      sprintf(elementLabel,"SURF %d Chan %d RMS",surf+1,chan+1);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,acqdStartPtr->chanRMS[surf][chan]);
    }
  }

  for( int thresh=0; thresh<10; ++thresh ) {
    sprintf(elementName,"threshVals_%d",thresh);
    sprintf(elementLabel,"Thresh %d",thresh);      
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,acqdStartPtr->threshVals[thresh]);
  }

  for(int thresh=0;thresh<10;thresh++) {
    for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
      for( int chan=0; chan<SCALERS_PER_SURF; ++chan ) {
	sprintf(elementName,"scalerVals_%d_%d_%d",surf,chan,thresh);
	sprintf(elementLabel,"SURF %d Trig %d Thresh %d",surf+1,chan+1,acqdStartPtr->threshVals[thresh]);      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,acqdStartPtr->scalerVals[surf][chan][thresh]);
      }
    }
  }


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

  summaryFile.writeSingleFullJSONFile(dirName,"acqdStart");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/acqdStartSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/acqdStartTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastAcqdStart",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
