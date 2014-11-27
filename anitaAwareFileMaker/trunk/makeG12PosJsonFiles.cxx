////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeG12PosJsonFiles
////      This is a simple program that converts ANITA G12 POS root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "G12Pos.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


G12Pos *g12PosPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/g12PosFile13.root\n";  
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
  TTree *g12PosTree = (TTree*) fp->Get("g12PosTree");
  if(!g12PosTree) {
    std::cerr << "Can't find g12PosTree\n";
    return -1;
  }

  if(g12PosTree->GetEntries()<1) {
    std::cerr << "No entries in g12PosTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  g12PosTree->SetBranchAddress("pos",&g12PosPtr);
  
  g12PosTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)g12PosPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=g12PosPtr->run;


  //Now we set up out run list
  Long64_t numEntries=g12PosTree->GetEntries();
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
	g12PosTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)g12PosPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      if(lastTime<g12PosPtr->realTime) lastTime=g12PosPtr->realTime;

      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];
      sprintf(elementName,"trueCourse");
      sprintf(elementLabel,"True Course ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->trueCourse,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"latitude");
      sprintf(elementLabel,"Latitude ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->latitude,AwareAverageType::kDefault,kTRUE,-999);

      sprintf(elementName,"longitude");
      sprintf(elementLabel,"Longitude ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->longitude,AwareAverageType::kDefault,kTRUE,-999);
      
      sprintf(elementName,"numSats");
      sprintf(elementLabel,"Num Sats ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->numSats);
 
      sprintf(elementName,"altitude");
      sprintf(elementLabel,"Altitude ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->altitude);
 
     sprintf(elementName,"verticalVelocity");
      sprintf(elementLabel,"Vertical Velocity ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->verticalVelocity);

      sprintf(elementName,"speedInKnots");
      sprintf(elementLabel,"Speed In Knots ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->speedInKnots);

      sprintf(elementName,"pdop");
      sprintf(elementLabel,"PDOP ");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->pdop);


      sprintf(elementName,"hdop");
      sprintf(elementLabel,"HDOP");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->hdop);


      sprintf(elementName,"vdop");
      sprintf(elementLabel,"VDOP");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->vdop);

      sprintf(elementName,"tdop");
      sprintf(elementLabel,"TDOP");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12PosPtr->tdop);
       
      
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
  summaryFile.writeFullJSONFiles(fullDir,"g12Pos");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/g12PosSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/g12PosTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastG12Pos",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
