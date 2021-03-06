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
  sprintf(mapJsonFile,"%s/ANITA4/map",outputDir);
  gSystem->mkdir(mapJsonFile);
  sprintf(mapJsonFile,"%s/ANITA4/map/posSum.json",outputDir);

  MapPosStruct_t thisEntry;
  std::map<UInt_t,MapPosStruct_t> mapPosMap;


  
  #define MAP_DELTA_T 300

  std::ifstream MapJsonIn (mapJsonFile);
  if(MapJsonIn) {
    char temp[180];
    MapJsonIn.getline(temp,179); /// {
    MapJsonIn.getline(temp,179); /// poslist : [
    while(MapJsonIn.getline(temp,179)) { // {unixTime:xxxxx,
      if(temp[0]==']') break;
      sscanf(temp,"{\"unixTime\":%u",&thisEntry.unixTime);
      //      std::cout << temp << "\t" << thisEntry.unixTime << "\n";
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"run\":%u",&thisEntry.run);
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"eventNumber\":%u",&thisEntry.eventNumber);
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"latitude\":%f",&thisEntry.latitude);
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"longitude\":%f",&thisEntry.longitude);
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"altitude\":%f",&thisEntry.altitude);
      MapJsonIn.getline(temp,179);
      sscanf(temp,"\"eventRate\":%f}",&thisEntry.eventRate);
      mapPosMap[thisEntry.unixTime/MAP_DELTA_T]=thisEntry;
      //      std::cout << thisEntry.unixTime << "\t" << thisEntry.run << "\t" << thisEntry.eventNumber << "\t" << thisEntry.unixTime/MAP_DELTA_T << "\n";

      MapJsonIn.getline(temp,179); // ","
      if(temp[0]==']') break;

    }
    MapJsonIn.close();
  }

  std::cout << "Currently mapPosMap.size() " << mapPosMap.size() << "\n";
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
      thisEntry.run=slowPtr->run;
      thisEntry.eventNumber=slowPtr->eventNumber;
      thisEntry.latitude=slowPtr->getLatitude();
      thisEntry.longitude=slowPtr->getLongitude();
      thisEntry.altitude=slowPtr->getAltitude();
      thisEntry.eventRate=slowPtr->getEventRate10Min();
      mapPosMap[mapTime]=thisEntry;
    } 
  }
  std::cerr << "\n";
  std::cout << "Currently mapPosMap.size() " << mapPosMap.size() << "\n";
 std::ofstream MapJsonOut (mapJsonFile);
 if(MapJsonOut) {
   MapJsonOut << "{\n";
   MapJsonOut << "\"poslist\" : [\n";
   std::map<UInt_t,MapPosStruct_t>::iterator mapIt=mapPosMap.begin();
   int firstTime=1;
   for(;mapIt!=mapPosMap.end();mapIt++) {  
     if(!firstTime) MapJsonOut << ",\n";
     MapJsonOut << "{\"unixTime\":" << mapIt->second.unixTime << ",\n";
     MapJsonOut << "\"run\":" << mapIt->second.run << ",\n";
     MapJsonOut << "\"eventNumber\":" << mapIt->second.eventNumber << ",\n";
     MapJsonOut << "\"latitude\":" << mapIt->second.latitude << ",\n";
     MapJsonOut << "\"longitude\":" << mapIt->second.longitude << ",\n";
     MapJsonOut << "\"altitude\":" << mapIt->second.altitude << ",\n";
     MapJsonOut << "\"eventRate\":" << mapIt->second.eventRate << "}\n";
     firstTime=0;
   }
   MapJsonOut << "\n]\n}\n";
   MapJsonOut.close();
 }
}
