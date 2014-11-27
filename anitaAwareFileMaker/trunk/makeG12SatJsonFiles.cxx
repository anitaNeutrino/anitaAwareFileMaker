////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeG12SatJsonFiles
////      This is a simple program that converts ANITA G12 SAT root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    July 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "G12Sat.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


G12Sat *g12SatPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/g12SatFile13.root\n";  
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
  TTree *g12SatTree = (TTree*) fp->Get("g12SatTree");
  if(!g12SatTree) {
    std::cerr << "Can't find g12SatTree\n";
    return -1;
  }

  if(g12SatTree->GetEntries()<1) {
    std::cerr << "No entries in g12SatTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  g12SatTree->SetBranchAddress("sat",&g12SatPtr);
  
  g12SatTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)g12SatPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=g12SatPtr->run;


  //Now we set up out run list
  Long64_t numEntries=g12SatTree->GetEntries();
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
	g12SatTree->GetEntry(event);

      TTimeStamp timeStamp((time_t)g12SatPtr->realTime,(Int_t)0);
      //    std::cout << "Run: "<< realEvPtr->
      if(lastTime<g12SatPtr->realTime) lastTime=g12SatPtr->realTime;
      
      //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
      //Summary file fun
      char elementName[180];
      char elementLabel[180];

      sprintf(elementName,"numSats_");
      sprintf(elementLabel,"Num Sats");      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->numSats);
	
      for(int whichSat=0;whichSat<MAX_SATS;whichSat++) {
	int goodSat=0;
	if(whichSat<(int)g12SatPtr->numSats) goodSat=1;

	sprintf(elementName,"prn_%d",whichSat+1);
	sprintf(elementLabel,"PRN %d",whichSat+1);      
	if(goodSat) summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->prn[whichSat],AwareAverageType::kDefault,kTRUE,-999);
	else summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
	
	sprintf(elementName,"elevation_%d",whichSat+1);
	sprintf(elementLabel,"Elevation %d",whichSat+1);      
	if(goodSat) summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->elevation[whichSat],AwareAverageType::kDefault,kTRUE,-999);
	else summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
	
	sprintf(elementName,"snr_%d",whichSat+1);
	sprintf(elementLabel,"SNR %d",whichSat+1);      
	if(goodSat) summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->snr[whichSat],AwareAverageType::kDefault,kTRUE,-999);
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
		
	sprintf(elementName,"flag_%d",whichSat+1);
	sprintf(elementLabel,"Flag %d",whichSat+1);      
	if(goodSat) summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->flag[whichSat],AwareAverageType::kDefault,kTRUE,-999);
	else summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kDefault,kTRUE,-999);
	
	sprintf(elementName,"azimuth_%d",whichSat+1);
	sprintf(elementLabel,"Azimuth %d",whichSat+1);      
	if(goodSat) summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,g12SatPtr->azimuth[whichSat],AwareAverageType::kAngleDegree,kTRUE,-999);
	else summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,-999,AwareAverageType::kAngleDegree,kTRUE,-999);
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
  summaryFile.writeFullJSONFiles(fullDir,"g12Sat");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/g12SatSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/g12SatTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastG12Sat",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
