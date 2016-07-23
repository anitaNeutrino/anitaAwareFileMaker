////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeSlowRateJsonFiles
////      This is a simple program that converts ANITA Monitor hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "SlowRate.h"
#include "AnitaGeomTool.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


SlowRate *slowPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/slowFile13.root\n";  
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
  TTree *slowTree = (TTree*) fp->Get("slowTree");
  if(!slowTree) {
    std::cerr << "Can't find slowTree\n";
    return -1;
  }

  if(slowTree->GetEntries()<1) {
    std::cerr << "No entries in slowTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  slowTree->SetBranchAddress("mon",&slowPtr);
  
  slowTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)slowPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=slowPtr->run;


  //Now we set up out run list
  Long64_t numEntries=slowTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");
  int ant;
  int phi;
  AnitaRing::AnitaRing_t ring;
  AnitaPol::AnitaPol_t pol;

  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Hk Entry
    slowTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)slowPtr->realTime,(Int_t)0);

    if(lastTime<slowPtr->realTime) lastTime=slowPtr->realTime;

    //    std::cout << "Run: "<< realEvPtr->

    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    
    sprintf(elementName,"altitude");
    sprintf(elementLabel,"Altitude");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getAltitude());


    sprintf(elementName,"latitude");
    sprintf(elementLabel,"Latitude");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getLatitude());


    sprintf(elementName,"longitude");
    sprintf(elementLabel,"Longitude");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getLongitude());


    sprintf(elementName,"eventRate1Min");
    sprintf(elementLabel,"Event Rate (1 min.)");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getEventRate1Min());

    sprintf(elementName,"eventRate10Min");
    sprintf(elementLabel,"Event Rate (10 min.)");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getEventRate10Min());



    for( int i=0; i<4; ++i ) {
      sprintf(elementName,"temp%d",i);
      sprintf(elementLabel,"%s",slowPtr->getTempName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getTemp(i));
    }        
    for( int i=0; i<4; ++i ) {
      sprintf(elementName,"power%d",i);
      sprintf(elementLabel,"%s",slowPtr->getPowerName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getPower(i));
    }        
     
    for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
      for( int chan=0; chan<RFCHAN_PER_SURF; ++chan ) {
	AnitaGeomTool::getRingAntPolPhiFromSurfChan(surf,chan,ring,ant,pol,phi);
	sprintf(elementName,"rfPower%d_%d",surf,chan);
	sprintf(elementLabel,"%d-%d %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getRawRFPower(surf,chan)); //need to mask the top bit (brotter)
      }      
    }


    for( int surf=0; surf<TRIGGER_SURFS; ++surf ) {
      for( int chan=0; chan<SCALERS_PER_SURF; ++chan ) {
	AnitaGeomTool::getPhiRingPolFromSurfChanTrigger(surf+2,chan,phi,ring,pol);
	sprintf(elementName,"scaler%d_%d",surf+2,chan);
	sprintf(elementLabel,"%d-%d  %d%c%c",surf+3,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,slowPtr->getAvgScaler(surf,chan));
      }      
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
  summaryFile.writeFullJSONFiles(fullDir,"slow");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/slowSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/slowTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastSlow",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
