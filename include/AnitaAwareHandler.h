////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  AnitaAwareHandler
////      This is a simple library for creating AWARE files for ANITA
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////


#ifndef ANITAAWAREHANDLER
#define ANITAAWAREHANDLER


#include "AwareRunSummaryFileMaker.h"

#include "RawAnitaHeader.h"


class AnitaAwareHandler 
{   
public:
   AnitaAwareHandler();
   void startHeaderFile(int runNumber,int dateInt);
   void finishHeaderFile();
   void addHeader(RawAnitaHeader *hdPtr);
    

private:
   AwareRunSummaryFileMaker *fHeadSumFile;
   Int_t fRunNumber;
   Int_t fDateInt;

};











#endif //ANITAAWAREHANDLER
