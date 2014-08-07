////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makePrettyHkJsonFiles
////      This is a simple program that converts ANITA Pretty hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "PrettyAnitaHk.h"
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


PrettyAnitaHk *prettyPtr=0;
CalibratedHk *hkPtr=0;

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
  UInt_t firstTime=timeStamp.GetSec();
  UInt_t runNumber=hkPtr->run;


  //Now we set up out run list
  Long64_t numEntries=hkTree->GetEntries();
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
    hkTree->GetEntry(event);


   if(prettyPtr) delete prettyPtr;
   prettyPtr= new PrettyAnitaHk(hkPtr,0);

   //   std::cout << hkPtr->realTime << "\t" << prettyPtr->realTime << "\n";

    
    TTimeStamp timeStamp((time_t)prettyPtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< hkPtr->run << "\n";

    //    std::cout  << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    for( int i=0; i<NUM_INT_TEMPS+NUM_SBS_TEMPS; ++i ) {
      sprintf(elementName,"intTemps%d",i);
      if(i<NUM_INT_TEMPS)
	strcpy(elementLabel,CalibratedHk::getInternalTempName(i));      
      else 
	strcpy(elementLabel,CalibratedHk::getSBSTempName(i-NUM_INT_TEMPS));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->intTemps[i]);
    }
    for( int i=0; i<NUM_EXT_TEMPS; ++i ) {
      sprintf(elementName,"extTemps%d",i);
      strcpy(elementLabel,CalibratedHk::getExternalTempName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->extTemps[i]);
    }
    for( int i=0; i<NUM_VOLTAGES; ++i ) {
      sprintf(elementName,"voltages%d",i);
      strcpy(elementLabel,CalibratedHk::getVoltageName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->voltages[i]);
    }
    for( int i=0; i<NUM_CURRENTS; ++i ) {
      sprintf(elementName,"currents%d",i);
      strcpy(elementLabel,CalibratedHk::getCurrentName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->currents[i]);
    }
    const char *magNames[3] = {"Mag-X","Mag-Y","Mag-Z"};
    for( int i=0; i<3; ++i ) {
      sprintf(elementName,"magentometer%d",i);
      strcpy(elementLabel,magNames[i]);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->magnetometer[i]);
    }
    const char *pressureNames[2] = {"Low-Pressure","High-Pressure"};
    for( int i=0; i<2; ++i ) {
      sprintf(elementName,"pressures%d",i);
      strcpy(elementLabel,pressureNames[i]);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->pressures[i]);
    }
    const char *accelNames[2][4]={{"Ac1-X","Ac1-Y","Ac1-Z","Ac1-T"},{"Ac2-X","Ac2-Y","Ac2-Z","Ac2-T"}};    
    for( int i=0; i<2; ++i ) {
      for( int j=0; j<4; ++j ) {
	sprintf(elementName,"accelerometer%d_%d",i,j);
	strcpy(elementLabel,accelNames[i][j]);
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->accelerometer[i][j]);
      }
    }
    const char *ssMagNames[4][2]={{"SS1-X","SS1-Y"},{"SS2-X","SS2-Y"},{"SS3-X","SS3-Y"},{"SS4-X","SS4-Y"}};
    const char *ssNames[4]={"SS1","SS2","SS3","SS4"};
    for( int i=0; i<4; ++i ) {
      for( int j=0; j<2; ++j ) {
	sprintf(elementName,"ssMag%d_%d",i,j);
	strcpy(elementLabel,ssMagNames[i][j]);
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->ssMag[i][j]);

      }
    }
    for( int i=0; i<4; ++i ) {
       sprintf(elementName,"ssElevation%d",i);
       strcpy(elementLabel,ssNames[i]);       
       if(prettyPtr->ssGoodFlag[i])
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->ssElevation[i],AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
       

       
       sprintf(elementName,"ssAzimuthRaw%d",i);
       strcpy(elementLabel,ssNames[i]);
       if(prettyPtr->ssGoodFlag[i])
	 summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->ssAzimuth[i],AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
       sprintf(elementName,"ssAzimuthAdu5%d",i);
       strcpy(elementLabel,ssNames[i]);
       if(prettyPtr->ssGoodFlag[i])
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->ssAzimuthAdu5[i],AwareAverageType::kAngleDegree,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kAngleDegree,kTRUE,-999);
       
       sprintf(elementName,"ssGoodFlag%d",i);
       strcpy(elementLabel,ssNames[i]);
       summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,prettyPtr->ssGoodFlag[i]);
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
  summaryFile.writeFullJSONFiles(fullDir,"hk");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/hkSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/hkTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastHk",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
