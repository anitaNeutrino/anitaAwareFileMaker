/*
 * makeTuffStatusJsonFiles
 * Cosmin Deaconu <cozzyd@kicp.uchicago.edu>
 */

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "GpuPowerSpectra.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"

//GpuPowerNotchStatus *gpuPowerPtr=0;
GpuPowerSpectra* gpu = NULL;

void usage(char **argv)
{
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
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

  TTree *gpuTree = (TTree*) fp->Get("gpuTree");
  if(!gpuTree) {
    std::cerr << "Couldn't get gpu tree from " << argv[1] << "\n";
    return -1;
  }

  gpuTree->SetBranchAddress("gpu",&gpu);

  if(gpuTree->GetEntries()<1) {
    std::cerr << "No entries in gpuTree\n";
    return -1;
  }


  gpuTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)gpu->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=gpu->run;
  std::cout << gpu->run << "\t" << gpu->realTime << "\n";

  //Now we set up out run list
  Long64_t numEntries=gpuTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");

  // Let's assume GPU packets appear in time increasing order for now...
  // we need to be ready to add a default value for each time stamp.

  const int numPol = 2;
  const int numRing = 3;

  std::vector<UInt_t> treeTimes;
  std::vector<UInt_t> treeTimesThisPacket[numPol][numRing];
  std::vector<UInt_t> entriesThisPacket[numPol][numRing];

  for(Long64_t event=0;event<numEntries;event++) {

    gpuTree->GetEntry(event);

    UInt_t realTime = gpu->realTime;

    std::vector<UInt_t>::iterator x = std::find(treeTimes.begin(), treeTimes.end(), realTime);
    if(x!=treeTimes.end()){
      treeTimes.push_back(realTime);
    }

    int pol = gpu->pol;
    int ring = gpu->ring;
    treeTimesThisPacket[pol][ring].push_back(realTime);
    entriesThisPacket[pol][ring].push_back(entry);
  }

  // now we have a list of all the times in the tree...
  // we produce the json file by time rather than entry
  // and write fake data for packets we don't have

  const int numPoints = 128;

  for(UInt_t timeInd = 0; timeInd < treeTimes.size(); timeInd++){
    UInt_t realTime = treeTimes.at(timeInd);

    for(int pol=0; pol < numPol; pol++){
      for(int ring=0; ring < numRing; ring++){

	std::vector<UInt_t> & ts = treeTimesThisPacket[pol][ring];
	std::vector<UInt_t>::iterator x = std::find(ts.begin(), ts.end(), realTime);

	if(x!=ts.end()){
	  // add real data points
	  UInt_t entry = entriesThisPacket[pol][ring];
	  gpuTree->GetEntry(entry);


	  char elementName[512];
	  char elementLabel[512];
	  for(int phi=0; phi<NUM_PHI; phi++){

	    TGraph* gr = gpu->getGraph(phi);

	    TString polName = gpu->pol == 0 ? "H" : "V";
	    TString ringName = gpu->ring == 0 ? "T" : (gpu->ring == 1 ? "M" : "B");

	    for(int i=0; i < gr->GetN(); i++){
	      sprintf(elementName,"%d%s%s-freq%d", phi, ringName.Data(), polName.Data(), i);
	      sprintf(elementLabel,"%d%s%s %4.2lfMHz", phi+1, ringName.Data(), polName.Data(), gr->GetX()[i]);
	      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gr->GetY()[i] + 100*i);
	    }
	    delete gr;
	  }


	} // if x!=end()
	else{
	  // add fake data points

	  for(int phi=0; phi < NUM_PHI; phi++){
	    for(int i=0; i < numPoints; i++){
	      sprintf(elementName,"%d%s%s-freq%d", phi, ringName.Data(), polName.Data(), i);
	      sprintf(elementLabel,"%d%s%s %4.2lfMHz", phi+1, ringName.Data(), polName.Data(), gr->GetX()[i]);
	      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,gr->GetY()[i] + 100*i);
	    }
	  }
	}
      }
    }
  }

  for(Long64_t event=0;event<numEntries;event++) {

    if(event%starEvery==0) {
      std::cerr << "*";
    }
    gpuTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)gpu->realTime,(Int_t)0);
    if(lastTime < gpu->realTime){
      lastTime = gpu->realTime;
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
  summaryFile.writeSingleFullJSONFile(dirName,"gpuPowerSpectra");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/gpuPowerSpectraSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/gpuPowerSpectraTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastGpuPowerSpectra",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
