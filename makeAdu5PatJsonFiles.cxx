////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeAdu5PatJsonFiles
////      This is a simple program that converts ANITA ADU5 PAT root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "Adu5Pat.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


Adu5Pat *adu5PatPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file> <AorB>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/adu5PatFile13.root\t0\n";  
}


int main(int argc, char **argv) {
  if(argc<3) {
    usage(argv);
    return -1;
  }

  int whichAdu5=atoi(argv[2]);
  TFile *fp = TFile::Open(argv[1]);
  if(!fp) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *adu5PatTree;
  if(whichAdu5==0) adu5PatTree=(TTree*) fp->Get("adu5PatTree");
  else if(whichAdu5==1) adu5PatTree = (TTree*) fp->Get("adu5bPatTree");
  else {
    std::cerr << "Invalid option must be 0 or 1\n";
    return -1;
  }

  if(!adu5PatTree) {
    std::cerr << "Can't find adu5PatTree\n";
    return -1;
  }

  if(adu5PatTree->GetEntries()<1) {
    std::cerr << "No entries in adu5PatTree\n";
    return -1;
  }
   
  adu5PatTree->SetBranchAddress("pat",&adu5PatPtr);
  
  adu5PatTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)adu5PatPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=adu5PatPtr->run;


  //Now we set up out run list
  Long64_t numEntries=adu5PatTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA3",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");


  char adu5Letter[2]={'A','B'};
  

  for(Long64_t event=0;event<numEntries;event++) {
      if(event%starEvery==0) {
	std::cerr << "*";       
      }
      
      //This line gets the Hk Entry
      adu5PatTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)adu5PatPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      
      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];
      sprintf(elementName,"heading_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Heading %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->heading,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"latitude_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Latitude %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->latitude,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"longitude_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Longitude %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->longitude,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"altitude_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Altitude %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->altitude);
      
      sprintf(elementName,"pitch_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Pitch %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->pitch);
      
      sprintf(elementName,"roll_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Roll %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->roll);
      
      sprintf(elementName,"mrms_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"MRMS %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->mrms);
      
      sprintf(elementName,"brms_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"BRMS %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->brms);
      
      sprintf(elementName,"attFlag_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"AttFlag %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5PatPtr->attFlag);      
      
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
  summaryFile.writeFullJSONFiles(fullDir,"adu5Pat");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/adu5%cPatSummary.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/adu5%cPatTime.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastAdu5%cPat",outputDir,instrumentName,adu5Letter[whichAdu5]);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
