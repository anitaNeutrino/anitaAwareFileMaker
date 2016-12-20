////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeHkJsonFiles
////      This is a simple program that converts ANITA hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
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
  UInt_t lastTime=timeStamp.GetSec();
  Long64_t lastEntry=0;
  UInt_t runNumber=hkPtr->run;
  std::cout << hkPtr->run << "\t" << hkPtr->realTime << "\n";

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


   //   std::cout << hkPtr->realTime << "\t" << prettyPtr->realTime << "\n";

    
    TTimeStamp timeStamp((time_t)hkPtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< hkPtr->run << "\n";
    if(lastTime<hkPtr->realTime)    {
      lastTime=hkPtr->realTime;
      lastEntry=event;
    }

    //    std::cout  << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    for( int i=0; i<NUM_INT_TEMPS+NUM_SBS_TEMPS; ++i ) {
      sprintf(elementName,"intTemps%d",i);
      if(i<NUM_INT_TEMPS) {
	strcpy(elementLabel,CalibratedHk::getInternalTempName(i));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getInternalTemp(i));
	//	std::cout << "Int temp: " << i << "\t" << hkPtr->getInternalTemp(i) << "\n";
      }
      else {
	strcpy(elementLabel,CalibratedHk::getSBSTempName(i-NUM_INT_TEMPS));            summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getSBSTemp(i-NUM_INT_TEMPS));
      }

    }
    for(int i=0;i<NUM_NTU_TEMPS;i++) {
      sprintf(elementName,"ntuTemps%d",i);
      strcpy(elementLabel,CalibratedHk::getNTUTempName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getNTUTemp(i));
    }

    for( int i=0; i<NUM_EXT_TEMPS; ++i ) {
      sprintf(elementName,"extTemps%d",i);
      strcpy(elementLabel,CalibratedHk::getExternalTempName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getExternalTemp(i));
    }
    for( int i=0; i<NUM_VOLTAGES; ++i ) {
      sprintf(elementName,"voltages%d",i);
      strcpy(elementLabel,CalibratedHk::getVoltageName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getVoltage(i));
    }
    for( int i=0; i<NUM_CURRENTS; ++i ) {
      sprintf(elementName,"currents%d",i);
      strcpy(elementLabel,CalibratedHk::getCurrentName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getCurrent(i));
    }
    for( int i=0; i<NUM_POWERS; ++i ) {
      sprintf(elementName,"powers%d",i);
      strcpy(elementLabel,CalibratedHk::getPowerName(i));
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getPower(i));
    }
    
    sprintf(elementName,"magx");
    strcpy(elementLabel,"Mag-X");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->magX);
    sprintf(elementName,"magy");
    strcpy(elementLabel,"Mag-Y");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->magY);
    sprintf(elementName,"magz");
    strcpy(elementLabel,"Mag-Z");
    summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->magZ);

    const char *pressureNames[2] = {"Low-Pressure","High-Pressure"};
    for( int i=0; i<2; ++i ) {
      sprintf(elementName,"pressures%d",i);
      strcpy(elementLabel,pressureNames[i]);
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getPressure(i));
    }
    const char *accelNames[2][4]={{"Ac1-X","Ac1-Y","Ac1-Z","Ac1-T"},{"Ac2-X","Ac2-Y","Ac2-Z","Ac2-T"}};    
    for( int i=0; i<2; ++i ) {
      for( int j=0; j<4; ++j ) {
	sprintf(elementName,"accelerometer%d_%d",i,j);
	strcpy(elementLabel,accelNames[i][j]);
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getAccelerometer(i,j));
      }
    }


    //    const char *ssNames[4]={"SS-1","SS-2","SS-3","SS-4"};
    const char *rawSSNames[5]={"x1","x2","y1","y2","T"};
    for( int i=0; i<4; ++i ) {
      for(int j=0;j<5;j++) {       
	sprintf(elementName,"rawSS_%d_%d",i,j);
	sprintf(elementLabel,"%s - %s",hkPtr->getSSName(i),rawSSNames[j]);
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getRawSunsensor(i,j));
      }
    

      Float_t mag=0;
      Float_t magX=0;
      Float_t magY=0;
      hkPtr->getSSMagnitude(i,&mag,&magX,&magY);
      sprintf(elementName,"ssMag_%d",i);
      sprintf(elementLabel,"ssMag %s",hkPtr->getSSName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,mag);
      sprintf(elementName,"ssMagX_%d",i);
      sprintf(elementLabel,"ssMagX %s",hkPtr->getSSName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,magX);
      sprintf(elementName,"ssMagY_%d",i);
      sprintf(elementLabel,"ssMagY %s",hkPtr->getSSName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,magY);

      Float_t temp=hkPtr->getSSTemp(i);
      sprintf(elementName,"ssTemp_%d",i);
      sprintf(elementLabel,"ssTemp %s",hkPtr->getSSName(i));      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,temp);

      Float_t ssPos[3]={0};
      Float_t ssAzimuth=0;
      Float_t ssElevation=0;
      Float_t ssRelElevation=0;
      Int_t goodFlag=hkPtr->getFancySS(i,ssPos,&ssAzimuth,&ssElevation,&ssRelElevation);

      sprintf(elementName,"ssElevation%d",i);
      strcpy(elementLabel,hkPtr->getSSName(i));       
      if(goodFlag)
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssElevation,AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
              
       sprintf(elementName,"ssAzimuthRaw%d",i);
       strcpy(elementLabel,hkPtr->getSSName(i));
       if(goodFlag)
	 summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssAzimuth,AwareAverageType::kDefault,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
       
       sprintf(elementName,"ssAzimuthAdu5%d",i);
       strcpy(elementLabel,hkPtr->getSSName(i));
       if(goodFlag)
	 summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,ssRelElevation,AwareAverageType::kAngleDegree,kTRUE,-999);
       else
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kAngleDegree,kTRUE,-999);
       
       sprintf(elementName,"ssGoodFlag%d",i);
       strcpy(elementLabel,hkPtr->getSSName(i));
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
  summaryFile.writeSingleFullJSONFile(dirName,"hk");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/hkSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/hkTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastHk",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);


  char statusPage[FILENAME_MAX];
  sprintf(statusPage,"%s/%s/statusPage",outputDir,instrumentName);
  gSystem->mkdir(statusPage,kTRUE);
  sprintf(statusPage,"%s/%s/statusPage/hkStatus.json.gz",outputDir,instrumentName);
  
  
  char elementName[180];
  char elementLabel[180];

  //Now update status page
  hkTree->GetEntry(lastEntry);
  
  //get most recent run number
  runNumber=hkPtr->run;
 
  //create status summary object
  AwareRunSummaryFileMaker statusSummaryFile(runNumber,"ANITA4",60);

  
  
  // add vital currents PV (currents1) , batt (currents6)
  const int arr_size = 2;
  int vitalCurrent[arr_size] = {1,6}; 
  for(auto& i : vitalCurrent) {
      sprintf(elementName,"currents%d",i);
      strcpy(elementLabel,CalibratedHk::getCurrentName(i));
      statusSummaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getCurrent(i)); 
  }
  
  // add vital Voltages PV (volatage.) and +24V 
  int vitalVoltage[arr_size] = {4,3};
  for(auto& i : vitalVoltage) {
      sprintf(elementName,"voltages%d",i);
      strcpy(elementLabel,CalibratedHk::getVoltageName(i));
      statusSummaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getVoltage(i));
  }

  // add vital internal temps CPU, Core 1, Core 2 
  for( int i=0; i<NUM_SBS_TEMPS; ++i ) {
      sprintf(elementName,"intTemps%d",i);
      strcpy(elementLabel,CalibratedHk::getSBSTempName(i));            
      statusSummaryFile.addVariablePoint(elementName,elementLabel,timeStamp,hkPtr->getSBSTemp(i));
  }  

  statusSummaryFile.writeTimeJSONFile(statusPage);  
}


