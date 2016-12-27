////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeHeaderJsonFiles
////      This is a simple program that converts ANITA header root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "RawAnitaHeader.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Include
#include "AnitaAwareHandler.h"
#include "AwareRunDatabase.h"


RawAnitaHeader *hdPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/headFile13.root\n";  
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
  TTree *headTree = (TTree*) fp->Get("headTree");
  if(!headTree) {
    std::cerr << "Can't find headTree\n";
    return -1;
  }

  if(headTree->GetEntries()<1) {
    std::cerr << "No entries in headTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  headTree->SetBranchAddress("header",&hdPtr);
  headTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();  
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=hdPtr->run;

  //Now we set up out run list
  Long64_t numEntries=headTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");

  //  AnitaAwareHandler awareHandler;
  //  awareHandler.startHeaderFile(runNumber,dateInt);


  AwareRunSummaryFileMaker *fHeadSumFile=new AwareRunSummaryFileMaker(runNumber,"ANITA4",60);
  Int_t fRunNumber=runNumber;
  Int_t fDateInt=dateInt;


  UInt_t lastTrigTime[16]={0};
  
  
  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Header Entry
    headTree->GetEntry(event);

   //   TTimeStamp timeStamp((time_t)hdPtr->realTime,(Int_t)0);

   TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
   char elementName[180];
   char elementLabel[180];
   
   sprintf(elementName,"ppsNum");
   sprintf(elementLabel,"PPS Num");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->ppsNum);
   
   sprintf(elementName,"c3poNum");
   sprintf(elementLabel,"C3P0 Num");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->c3poNum);
   
   sprintf(elementName,"eventNumber");
   sprintf(elementLabel,"Event Number");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->eventNumber);

   sprintf(elementName,"triggerTimeNs");
   sprintf(elementLabel,"Trigger Time");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->triggerTimeNs);

   sprintf(elementName,"priority");
   sprintf(elementLabel,"Priority");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->priority&0xf);

   sprintf(elementName,"bufferDepth");
   sprintf(elementLabel,"Buffer Depth");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->bufferDepth);

   sprintf(elementName,"imagePeak");
   sprintf(elementLabel,"Image Peak");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getImagePeak());

   sprintf(elementName,"coherentSumPeak");
   sprintf(elementLabel,"Coherent Sum Peak");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getCoherentSumPeak());

   sprintf(elementName,"peakTheta");
   sprintf(elementLabel,"Peak Theta");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getPeakThetaDeg());

   sprintf(elementName,"peakPhi");
   sprintf(elementLabel,"Peak Phi");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getPeakPhiDeg());

   sprintf(elementName,"peakPol");
   sprintf(elementLabel,"Peak Pol");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getPeakPol());

   sprintf(elementName,"aboveThresholdFlag");
   sprintf(elementLabel,"abolveThresholdFlag");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getAboveThresholdFlag());

   sprintf(elementName,"binToBinIncreaseFlag");
   sprintf(elementLabel,"binToBinIncreaseFlag");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getBinToBinIncreaseFlag());

   sprintf(elementName,"saturationFlag");
   sprintf(elementLabel,"saturationFlag");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,hdPtr->getSaturationFlag());

   for(int bit=0;bit<4;bit++) {
     const char *trigLabels[4]={"RF","ADU5","G12","Soft"};
      sprintf(elementName,"trigtype%d",bit);
      sprintf(elementLabel,"%s",trigLabels[bit]);
      int value=(hdPtr->trigType&(1<<bit)?1:0);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
   }
   

   int numPhiMask=0;
   int numL2Mask=0;
   int numL3Trig=0;


   for(int bit=0;bit<16;bit++) {
      sprintf(elementName,"l3TrigBit%02d",bit);
      sprintf(elementLabel,"L3 %d",bit+1);
      int value=(bit+1)*hdPtr->isInL3Pattern(bit);
      if(value) numL3Trig++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value,AwareAverageType::kBitMask,false,bit+1);

      sprintf(elementName,"timeSinceLastL3TrigBit%02d",bit);
      sprintf(elementLabel,"Time since L3 %d",bit+1);
      if(value) lastTrigTime[bit]=hdPtr->triggerTime;
      if(lastTrigTime[bit]>0) value=hdPtr->triggerTime-lastTrigTime[bit];
      else value=0;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
      
      
      
      sprintf(elementName,"l2TrigMask%02d",bit);
      sprintf(elementLabel,"L2 %d",bit+1);
      value=(bit+1)*hdPtr->isInL2Mask(bit);
      if(value) numL2Mask++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value,AwareAverageType::kBitMask,false,bit+1);

      sprintf(elementName,"PhiTrigMask%02d",bit);
      sprintf(elementLabel,"Phi %d",bit+1);
      value=(bit+1)*hdPtr->isInPhiMask(bit);
      if(value) numPhiMask++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value,AwareAverageType::kBitMask,false,bit+1);
   }

   sprintf(elementName,"numPhiMask");
   sprintf(elementLabel,"Num Phi");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numPhiMask);  

   sprintf(elementName,"numL2Mask");
   sprintf(elementLabel,"Num L2");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numL2Mask);  

   sprintf(elementName,"numL3Trig");
   sprintf(elementLabel,"Num L3");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numL3Trig);  
    
   //    awareHandler.addHeader(hdPtr);


    
    if(hdPtr->triggerTime>lastTime) lastTime=hdPtr->triggerTime;
    
  }
  std::cerr << "\n";
  //  awareHandler.finishHeaderFile();


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
  sprintf(dirName,"%s/%s/runs%d/runs%d/run%d/",outputDir,"ANITA4",fRunNumber-fRunNumber%10000,fRunNumber-fRunNumber%100,fRunNumber);
  sprintf(dateDirName,"%s/%s/%d/%04d/run%d/",outputDir,"ANITA4",fDateInt/10000,fDateInt%10000,fRunNumber);
  sprintf(subDateDirName,"%s/%s/%d/%04d/",outputDir,"ANITA4",fDateInt/10000,fDateInt%10000);
  gSystem->mkdir(dirName,kTRUE);
  gSystem->mkdir(subDateDirName,kTRUE);
  gSystem->Symlink(dirName,dateDirName);

  std::cout << "Making: " << dirName << "\n";
  

  char fullDir[FILENAME_MAX];
  sprintf(fullDir,"%s/full",dirName);
  gSystem->mkdir(fullDir,kTRUE);
  fHeadSumFile->writeSingleFullJSONFile(dirName,"header");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/headerSummary.json.gz",dirName);
  fHeadSumFile->writeSummaryJSONFile(outName);


  sprintf(outName,"%s/headerTime.json.gz",dirName);
  fHeadSumFile->writeTimeJSONFile(outName);


  delete fHeadSumFile;
  fHeadSumFile=NULL;
  
    
  sprintf(outName,"%s/%s/lastHeader",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,firstTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
