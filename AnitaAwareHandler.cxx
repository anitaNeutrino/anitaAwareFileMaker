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


#define INST_NAME "ANITA3"


AnitaAwareHandler::AnitaAwareHandler() 
{
   fHeadSumFile=NULL;
   fRunNumber=-1;
}


void AnitaAwareHandler::startHeaderFile(int runNumber, int dateInt)
{
   fHeadSumFile=new AwareRunSummaryFileMaker(runNumber,"ANITA3",60);
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

   
   for(int bit=0;bit<16;bit++) {
      sprintf(elementName,"l3TrigBit%d",bit);
      sprintf(elementLabel,"L3 Phi %d",bit+1);
      int value=hdPtr->isInL3Pattern(bit);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
      
      
      sprintf(elementName,"upperL2TrigBit%d",bit);
      sprintf(elementLabel,"L2 Upper Phi %d",bit+1);
      value=hdPtr->isInL2Pattern(bit,AnitaRing::kUpperRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
      
      sprintf(elementName,"lowerL2TrigBit%d",bit);
      sprintf(elementLabel,"L2 Lower Phi %d",bit+1);
      value=hdPtr->isInL2Pattern(bit,AnitaRing::kLowerRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);
      
      sprintf(elementName,"nadirL2TrigBit%d",bit);
      sprintf(elementLabel,"L2 Nadir Phi %d",bit+1);
      value=hdPtr->isInL2Pattern(bit,AnitaRing::kNadirRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"upperL1TrigBit%d",bit);
      sprintf(elementLabel,"L1 Upper Phi %d",bit+1);
      value=hdPtr->isInL1Pattern(bit,AnitaRing::kUpperRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"lowerL1TrigBit%d",bit);
      sprintf(elementLabel,"L1 Lower Phi %d",bit+1);
      value=hdPtr->isInL1Pattern(bit,AnitaRing::kLowerRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);

      sprintf(elementName,"nadirL1TrigBit%d",bit);
      sprintf(elementLabel,"L1 Nadir Phi %d",bit+1);
      value=hdPtr->isInL1Pattern(bit,AnitaRing::kNadirRing);
      fHeadSumFile->addVariablePoint(elementName,elementLabel,timeStamp,value);      
   }
   
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
  fHeadSumFile->writeFullJSONFiles(fullDir,"header");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/headerSummary.json.gz",dirName);
  fHeadSumFile->writeSummaryJSONFile(outName);


  sprintf(outName,"%s/headerTime.json.gz",dirName);
  fHeadSumFile->writeTimeJSONFile(outName);


  delete fHeadSumFile;
  fHeadSumFile=NULL;

}
