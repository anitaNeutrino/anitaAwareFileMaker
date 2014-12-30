////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  addRunToMapFile
////      This is a simple program that converts ANITA Slow Rate ROOT files
////      into entries in the ANITA JSON map file
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>
#include <fstream>
#include <map>

//ANITA EventReaderRoot Includes
#include "SlowRate.h"
#include "AnitaGeomTool.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


SlowRate *slowPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/slowFile13.root\n";  
}


typedef struct {
  UInt_t unixTime;
  UInt_t run;
  UInt_t eventNumber;
  Float_t latitude;
  Float_t longitude;
  Float_t altitude;
  Float_t eventRate;
} MapPosStruct_t;
  


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
  TTree *slowTree = (TTree*) fp->Get("slowTree");
  if(!slowTree) {
    std::cerr << "Can't find slowTree\n";
    return -1;
  }

  if(slowTree->GetEntries()<1) {
    std::cerr << "No entries in slowTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  slowTree->SetBranchAddress("mon",&slowPtr);
  
  slowTree->GetEntry(0);




  //Now we set up out run list
  Long64_t numEntries=slowTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;



  char outputDir[FILENAME_MAX];
  char *outputDirEnv=getenv("AWARE_OUTPUT_DIR");
  if(outputDirEnv==NULL) {
    sprintf(outputDir,"/unix/anita1/data/aware/output");
  }
  else {
    strncpy(outputDir,outputDirEnv,FILENAME_MAX);
  }
    


  //Need to read in the map
  char mapJsonFile[FILENAME_MAX];
  sprintf(mapJsonFile,"%s/ANITA3/map",outputDir);
  gSystem->mkdir(mapJsonFile);
  sprintf(mapJsonFile,"%s/ANITA3/map/posSum.json",outputDir);

  MapPosStruct_t thisEntry;
  std::map<UInt_t,MapPosStruct_t> mapPosMap;


  
  #define MAP_DELTA_T 300

  std::ifstream MapJsonIn (mapJsonFile);
  if(MapJsonIn) {
    char temp[180];
    MapJsonIn.getline(temp,179); /// {
    MapJsonIn.getline(temp,179); /// poslist : [
    while(MapJsonIn.getline(temp,179)) {
      if(temp[0]==']') break;
      sscanf(temp,"{%u,%u,%u,%f,%f,%f,%f},",&thisEntry.unixTime,&thisEntry.run,&thisEntry.eventNumber,&thisEntry.latitude,&thisEntry.longitude,&thisEntry.altitude,&thisEntry.eventRate);
      mapPosMap[thisEntry.unixTime/MAP_DELTA_T]=thisEntry;
    }
    MapJsonIn.close();
  }


  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }
    
    //This line gets the Hk Entry
    slowTree->GetEntry(event);
    
    UInt_t mapTime=slowPtr->realTime/MAP_DELTA_T;
    std::map<UInt_t,MapPosStruct_t>::iterator mapIt=mapPosMap.find(mapTime);
    if(mapIt==mapPosMap.end()) {
      //Need to add a new entry
      thisEntry.unixTime=slowPtr->realTime;
      thisEntry.eventNumber=slowPtr->eventNumber;
      thisEntry.latitude=slowPtr->getLatitude();
      thisEntry.longitude=slowPtr->getLongitude();
      thisEntry.altitude=slowPtr->getAltitude();
      thisEntry.eventRate=slowPtr->getEventRate10Min();
      mapPosMap[mapTime]=thisEntry;
    } 
  }
  std::cerr << "\n";
 std::ofstream MapJsonOut (mapJsonFile);
 if(MapJsonOut) {
   MapJsonOut << "{\n";
   MapJsonOut << "\"poslist\" : [";
   std::map<UInt_t,MapPosStruct_t>::iterator mapIt=mapPosMap.begin();
   char temp[180];
   int firstTime=1;
   for(;mapIt!=mapPosMap.end();mapIt++) {  
     sprintf(temp,"\n{%u,%u,%u,%f,%f,%f,%f}",mapIt->second.unixTime,mapIt->second.run,mapIt->second.eventNumber,mapIt->second.latitude,mapIt->second.longitude,mapIt->second.altitude,mapIt->second.eventRate);
     if(!firstTime) MapJsonOut << ",";
     firstTime=0;
     MapJsonOut << temp;
   }
   MapJsonOut << "\n]\n}\n";
   MapJsonOut.close();
 }
}
