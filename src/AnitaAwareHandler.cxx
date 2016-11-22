////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  AnitaAwareHandler
////      This is a simple library for creating AWARE files for ANITA
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "TSystem.h"
#include "AnitaAwareHandler.h"


#define INST_NAME "ANITA4"


AnitaAwareHandler::AnitaAwareHandler() 
{
   fHeadSumFile=NULL;
   fRunNumber=-1;
}


void AnitaAwareHandler::startHeaderFile(int runNumber, int dateInt)
{
   fHeadSumFile=new AwareRunSummaryFileMaker(runNumber,"ANITA4",60);
   fRunNumber=runNumber;
   fDateInt=dateInt;
}


void AnitaAwareHandler::addHeader(RawAnitaHeader *hdPtr) {

   
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
   int numL1Mask=0;
   int numL3Trig=0;


   for(int bit=0;bit<16;bit++) {
      sprintf(elementName,"l3TrigBit%d",bit);
      sprintf(elementLabel,"L3 %d",bit+1);
      int value=(bit+1)*hdPtr->isInL3Pattern(bit);
      if(value) numL3Trig++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
      
      
      sprintf(elementName,"L1TrigMask%d",bit);
      sprintf(elementLabel,"L1 %d",bit+1);
      value=(bit+1)*hdPtr->isInL1Mask(bit);
      if(value) numL1Mask++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"PhiTrigMask%d",bit);
      sprintf(elementLabel,"Phi %d",bit+1);
      value=(bit+1)*hdPtr->isInPhiMask(bit);
      if(value) numPhiMask++;
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
   }

   sprintf(elementName,"numPhiMask");
   sprintf(elementLabel,"Num Phi");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numPhiMask);  

   sprintf(elementName,"numL1Mask");
   sprintf(elementLabel,"Num L1");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numL1Mask);  

   sprintf(elementName,"numL3Trig");
   sprintf(elementLabel,"Num L3");
   fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,numL3Trig);  

}


void AnitaAwareHandler::finishHeaderFile() 
{

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
  sprintf(dirName,"%s/%s/runs%d/runs%d/run%d/",outputDir,INST_NAME,fRunNumber-fRunNumber%10000,fRunNumber-fRunNumber%100,fRunNumber);
  sprintf(dateDirName,"%s/%s/%d/%04d/run%d/",outputDir,INST_NAME,fDateInt/10000,fDateInt%10000,fRunNumber);
  sprintf(subDateDirName,"%s/%s/%d/%04d/",outputDir,INST_NAME,fDateInt/10000,fDateInt%10000);
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

}
