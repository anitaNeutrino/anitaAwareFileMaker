////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeGpsGgaJsonFiles
////      This is a simple program that converts ANITA GPS GGA root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "GpsGga.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


GpsGga *gpsGgaPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file> <0 or 1 or 2>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/gpsGgaFile13.root\t0\n";  
}


int main(int argc, char **argv) {
  if(argc<3) {
    usage(argv);
    return -1;
  }

  int whichGps=atoi(argv[2]);
  TFile *fp = TFile::Open(argv[1]);
  if(!fp) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *gpsGgaTree;
  if(whichGps==0) gpsGgaTree=(TTree*) fp->Get("adu5GgaTree");
  else if(whichGps==1) gpsGgaTree = (TTree*) fp->Get("aud5bGgaTree");
  else if(whichGps==2) gpsGgaTree = (TTree*) fp->Get("g12GgaTree");
  else {
    std::cerr << "Invalid option must be 0 or 1 or 2\n";
    return -1;
  }

  if(!gpsGgaTree) {
    std::cerr << "Can't find gpsGgaTree\n";
    return -1;
  }

  if(gpsGgaTree->GetEntries()<1) {
    std::cerr << "No entries in gpsGgaTree\n";
    return -1;
  }
   
  gpsGgaTree->SetBranchAddress("gga",&gpsGgaPtr);
  
  gpsGgaTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)gpsGgaPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=gpsGgaPtr->run;


  //Now we set up out run list
  Long64_t numEntries=gpsGgaTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA3",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");


  const char *gpsName[3]={"Adu5A","Adu5B","G12"};
  

  for(Long64_t event=0;event<numEntries;event++) {
      if(event%starEvery==0) {
	std::cerr << "*";       
      }
      
      //This line gets the Hk Entry
      gpsGgaTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)gpsGgaPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      
      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];
      sprintf(elementName,"numSats_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Num Sats %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->numSats);
      
      sprintf(elementName,"latitude_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Latitude %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->latitude,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"longitude_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Longitude %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->longitude,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"altitude_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Altitude %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->altitude);
      
      sprintf(elementName,"hdop_%s",gpsName[whichGps]);
      sprintf(elementLabel,"HDOP %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->hdop);
      
      sprintf(elementName,"geoidSeparation_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Geoid Separation %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->geoidSeparation);
      
      sprintf(elementName,"ageOfCalc_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Age of Calc. %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->ageOfCalc);
      
      sprintf(elementName,"posFixType_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Pos. Fix Type %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->posFixType);
      
      sprintf(elementName,"baseStationId_%s",gpsName[whichGps]);
      sprintf(elementLabel,"Base Station Id %s",gpsName[whichGps]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gpsGgaPtr->baseStationId);      
      
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
  summaryFile.writeFullJSONFiles(fullDir,"gpsGga");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/%sGgaSummary.json.gz",dirName,gpsName[whichGps]);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/%sGgaTime.json.gz",dirName,gpsName[whichGps]);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/last%sGga",outputDir,instrumentName,gpsName[whichGps]);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
