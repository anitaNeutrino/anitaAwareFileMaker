////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeSSHkJsonFiles
////      This is a simple program that converts ANITA sunsensor hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "CalibratedSSHk.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


CalibratedSSHk *hkPtr=0;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/hkFile13.root\n";  
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
   TTree *hkTree = (TTree*) fp->Get("hkTree");
   if(!hkTree) {
      std::cerr << "Couldn't get hkTree from " << argv[1] << "\n";
      return -1;
   }      
   hkTree->SetBranchAddress("hk",&hkPtr);   

   

  if(hkTree->GetEntries()<1) {
    std::cerr << "No entries in hkTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  
  hkTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)hkPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=hkPtr->run;


  //Now we set up out run list
  Long64_t numEntries=hkTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");


  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Hk Entry
    hkTree->GetEntry(event);



    
    TTimeStamp timeStamp((time_t)hkPtr->realTime,(Int_t)0);
    if(lastTime<hkPtr->realTime) lastTime=hkPtr->realTime;
    //    std::cout << "Run: "<< hkPtr->run << "\n";

    //    std::cout  << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];


    const char *rawSSNames[5]={"x1","x2","y1","y2","T"};
    for( int i=0; i<4; ++i ) {
      for(int j=0;j<5;j++) {
        sprintf(elementName,"rawSS_%d_%d",i,j);
        sprintf(elementLabel,"%s - %s",hkPtr->getName(i),rawSSNames[j]);
        summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getRawSunsensor(i,j));
      } 

      Float_t mag=0;
      Float_t magX=0;
      Float_t magY=0;
      hkPtr->getSSMagnitude(i,&mag,&magX,&magY);
      sprintf(elementName,"ssMag_%d",i);
      sprintf(elementLabel,"ssMag %s",hkPtr->getName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,mag);
      sprintf(elementName,"ssMagX_%d",i);
      sprintf(elementLabel,"ssMagX %s",hkPtr->getName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,magX);
      sprintf(elementName,"ssMagY_%d",i);
      sprintf(elementLabel,"ssMagY %s",hkPtr->getName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,magY);

      Float_t temp=hkPtr->getSSTemp(i);
      sprintf(elementName,"ssTemp_%d",i);
      sprintf(elementLabel,"ssTemp %s",hkPtr->getName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,temp);

      Float_t ssPos[3]={0};
      Float_t ssAzimuth=0;
      Float_t ssElevation=0;
      Float_t ssRelElevation=0;
      Int_t goodFlag=hkPtr->getFancySS(i,ssPos,&ssAzimuth,&ssElevation,&ssRelElevation);

      sprintf(elementName,"ssElevation%d",i);
      strcpy(elementLabel,hkPtr->getName(i));       
      if(goodFlag)
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssElevation,AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
              
       sprintf(elementName,"ssAzimuthRaw%d",i);
       strcpy(elementLabel,hkPtr->getName(i));
       if(goodFlag)
	 summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssAzimuth,AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
       sprintf(elementName,"ssAzimuthAdu5%d",i);
       strcpy(elementLabel,hkPtr->getName(i));
       if(goodFlag)
	 summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssRelElevation,AwareAverageType::kAngleDegree,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kAngleDegree,kTRUE,-999);
       
       sprintf(elementName,"ssGoodFlag%d",i);
       strcpy(elementLabel,hkPtr->getName(i));
       summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,goodFlag);
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
  summaryFile.writeSingleFullJSONFile(dirName,"sshk");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/sshkSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/sshkTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastSshk",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
