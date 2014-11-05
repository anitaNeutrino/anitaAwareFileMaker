////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeAdu5VtgJsonFiles
////      This is a simple program that converts ANITA ADU5 VTG root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "Adu5Vtg.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


Adu5Vtg *adu5VtgPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file> <0or1>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/adu5VtgFile13.root\n";  
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
  TTree *adu5VtgTree;
  if(whichAdu5==0) adu5VtgTree= (TTree*) fp->Get("adu5VtgTree");
  else if(whichAdu5==1) adu5VtgTree = (TTree*) fp->Get("adu5bVtgTree");
  else {
    std::cerr << "Invalid option must be 0 or 1\n";
    return -1;
  }
  if(!adu5VtgTree) {
    std::cerr << "Can't find adu5VtgTree\n";
    return -1;
  }

  if(adu5VtgTree->GetEntries()<1) {
    std::cerr << "No entries in adu5VtgTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  adu5VtgTree->SetBranchAddress("vtg",&adu5VtgPtr);
  
  adu5VtgTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)adu5VtgPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=adu5VtgPtr->run;


  //Now we set up out run list
  Long64_t numEntries=adu5VtgTree->GetEntries();
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
	adu5VtgTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)adu5VtgPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      
      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];
      sprintf(elementName,"trueCourse_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"True Course %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5VtgPtr->trueCourse,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"magneticCourse_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Magnetic Course %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5VtgPtr->magneticCourse,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"speedInKnots_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Speed In Knots %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5VtgPtr->speedInKnots);
      
      sprintf(elementName,"speedInKPH_%c",adu5Letter[whichAdu5]);
      sprintf(elementLabel,"Speed In KPH %c",adu5Letter[whichAdu5]);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5VtgPtr->speedInKPH);
       
      
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

  char tagName[180];
  sprintf(tagName,"adu5%cVtg",adu5Letter[whichAdu5]); 
  summaryFile.writeFullJSONFiles(fullDir,tagName);


  char outName[FILENAME_MAX];

  sprintf(outName,"%s/adu5%cVtgSummary.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/adu5%cVtgTime.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastAdu5%cVtg",outputDir,instrumentName,adu5Letter[whichAdu5]);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
