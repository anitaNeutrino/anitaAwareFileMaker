////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeAdu5SatJsonFiles
////      This is a simple program that converts ANITA ADU5 SAT root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "Adu5Sat.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


Adu5Sat *adu5SatPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>  <AorB>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/adu5SatFile13.root\t0";  
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
  TTree *adu5SatTree;
  if(whichAdu5==0) adu5SatTree= (TTree*) fp->Get("adu5SatTree");
  else adu5SatTree= (TTree*) fp->Get("adu5bSatTree");
  if(!adu5SatTree) {
    std::cerr << "Can't find adu5SatTree\n";
    return -1;
  }

  if(adu5SatTree->GetEntries()<1) {
    std::cerr << "No entries in adu5SatTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  adu5SatTree->SetBranchAddress("sat",&adu5SatPtr);
  
  adu5SatTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)adu5SatPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=adu5SatPtr->run;


  //Now we set up out run list
  Long64_t numEntries=adu5SatTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");


  char adu5Letter[2]={'A','B'};
  
  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }
    
      //This line gets the Hk Entry
    adu5SatTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)adu5SatPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      
      if(lastTime<adu5SatPtr->realTime) lastTime=adu5SatPtr->realTime;
      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];
      for(int antInd=0;antInd<4;antInd++) {
	sprintf(elementName,"numSats_%c_%d",adu5Letter[whichAdu5],antInd);
	sprintf(elementLabel,"Num Sats %c-%d",adu5Letter[whichAdu5],antInd);      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->numSats[antInd]);
	
	for(int whichSat=0;whichSat<(int)adu5SatPtr->numSats[whichSat];whichSat++) {
	  sprintf(elementName,"prn_%c_%d_%d",adu5Letter[whichAdu5],whichSat+1,antInd);
	  sprintf(elementLabel,"PRN %c %d %d",adu5Letter[whichAdu5],whichSat+1,antInd);      
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->prn[antInd][whichSat]);

	  sprintf(elementName,"elevation_%c_%d %d",adu5Letter[whichAdu5],whichSat+1,antInd);
	  sprintf(elementLabel,"Elevation %c %d %d",adu5Letter[whichAdu5],whichSat+1,antInd);      
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->elevation[antInd][whichSat]);

	  sprintf(elementName,"snr_%c_%d %d",adu5Letter[whichAdu5],whichSat+1,antInd);
	  sprintf(elementLabel,"SNR %c %d %d",adu5Letter[whichAdu5],whichSat+1,antInd);      
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->snr[antInd][whichSat]);


	  sprintf(elementName,"flag_%c_%d %d",adu5Letter[whichAdu5],whichSat+1,antInd);
	  sprintf(elementLabel,"Flag %c %d %d",adu5Letter[whichAdu5],whichSat+1,antInd);      
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->flag[antInd][whichSat]);

	  sprintf(elementName,"azimuth_%c_%d %d",adu5Letter[whichAdu5],whichSat+1,antInd);
	  sprintf(elementLabel,"Azimuth %c %d %d",adu5Letter[whichAdu5],whichSat+1,antInd);      
	  summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,adu5SatPtr->azimuth[antInd][whichSat],AwareAverageType::kDefault,kTRUE,-999);
	}	  
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

  char tagName[180];
  sprintf(tagName,"adu5%cSat",adu5Letter[whichAdu5]); 
  summaryFile.writeSingleFullJSONFile(dirName,tagName);

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/adu5%cSatSummary.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/adu5%cSatTime.json.gz",dirName,adu5Letter[whichAdu5]);
  summaryFile.writeTimeJSONFile(outName);


sprintf(outName,"%s/%s/lastAdu5%cSat",outputDir,instrumentName,adu5Letter[whichAdu5]);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
