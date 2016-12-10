////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeSurfHkJsonFiles
////      This is a simple program that converts ANITA Surf hk root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "SurfHk.h"
#include "AnitaGeomTool.h"
#include "AnitaConventions.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Includes
#include "AwareRunSummaryFileMaker.h"
#include "AwareRunDatabase.h"


SurfHk *surfHkPtr;

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <input file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/surfHkFile13.root\n";  
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
  TTree *surfHkTree = (TTree*) fp->Get("surfHkTree");
  if(!surfHkTree) {
    std::cerr << "Can't find surfHkTree\n";
    return -1;
  }

  if(surfHkTree->GetEntries()<1) {
    std::cerr << "No entries in surfHkTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  surfHkTree->SetBranchAddress("surf",&surfHkPtr);
  
  surfHkTree->GetEntry(0);


  TTimeStamp timeStamp((time_t)surfHkPtr->realTime,(Int_t)0);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=surfHkPtr->run;


  //Now we set up out run list
  Long64_t numEntries=surfHkTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  AwareRunSummaryFileMaker summaryFile(runNumber,"ANITA4",60);

  

  char instrumentName[20];
  sprintf(instrumentName,"ANITA4");

  int ant;
  int phi;
  AnitaRing::AnitaRing_t ring;
  AnitaPol::AnitaPol_t pol;
  AnitaTrigPol::AnitaTrigPol_t trigPol;

  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Hk Entry
    surfHkTree->GetEntry(event);

    TTimeStamp timeStamp((time_t)surfHkPtr->realTime,(Int_t)0);
    //    std::cout << "Run: "<< realEvPtr->
    if(lastTime<surfHkPtr->realTime) lastTime=surfHkPtr->realTime;
    //  std::cout << event << "\t" << timeStamp.AsString("sl") << "\n";
    //Summary file fun
    char elementName[180];
    char elementLabel[180];
    for( int surf=2; surf<10; ++surf ) {
      for( int chan=0; chan<SCALERS_PER_SURF; ++chan ) {
	AnitaGeomTool::getPhiRingPolFromSurfChanTrigger(surf,chan,phi,ring,trigPol);
	sprintf(elementName,"scaler%d_%d",surf,chan);
	sprintf(elementLabel,"%d-%d  %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->scaler[surf][chan]);

	sprintf(elementName,"phiScaler%d_%c%c",phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));
	sprintf(elementLabel,"%d-%d  %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->scaler[surf][chan]);
      }   
      
    }
    for(int phi=0;phi<PHI_SECTORS;phi++) {
      for(int iring=0;iring<3;iring++) {
	int l1Chan=0,surf=0;
	ring=(AnitaRing::AnitaRing_t)iring;
	AnitaGeomTool::getSurfL1TriggerChanFromPhiRing(phi,ring,surf,l1Chan);
	sprintf(elementName,"l1Scaler%d_%d",surf,l1Chan);
	sprintf(elementLabel,"%d-%d  %d-%c",surf+1,l1Chan+1,phi+1,AnitaRing::ringAsChar(ring));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getL1Scaler(phi,ring));

	sprintf(elementName,"phiL1Scaler%d_%c",phi+1,AnitaRing::ringAsChar(ring));
	sprintf(elementLabel,"%d-%d  %d-%c",surf+1,l1Chan+1,phi+1,AnitaRing::ringAsChar(ring));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getL1Scaler(phi,ring));  
      }
           
      
      int l2Chan=0,surf=0;
      AnitaGeomTool::getSurfL2TriggerChanFromPhi(phi,surf,l2Chan);
      
      sprintf(elementName,"l2Scaler%d_%d",surf,l2Chan);
      sprintf(elementLabel,"%d-%d  %d",surf+1,l2Chan+1,phi+1);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getL2Scaler(phi));
      
      sprintf(elementName,"phiL2Scaler%d",phi+1);
      sprintf(elementLabel,"%d-%d  %d",surf+1,l2Chan+1,phi+1);      
      summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getL2Scaler(phi););  
    }

  
    

    // for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
    //   for( int chan=0; chan<4; ++chan ) {
    // 	sprintf(elementName,"l1Scaler%d_%d",surf,chan);
    // 	sprintf(elementLabel,"L1 Scaler %d-%d",surf+1,chan+1);      
    // 	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->l1Scaler[surf][chan]);
    //   }      
    // }

    for( int surf=2; surf<10; ++surf ) {
      for( int chan=0; chan<SCALERS_PER_SURF; ++chan ) {
	AnitaGeomTool::getPhiRingPolFromSurfChanTrigger(surf,chan,phi,ring,trigPol);
	sprintf(elementName,"threshold%d_%d",surf,chan);
	sprintf(elementLabel,"%d-%d %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->threshold[surf][chan]);

	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->threshold[surf][chan]);
	sprintf(elementName,"phiThreshold%d_%c%c",phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));
	sprintf(elementLabel,"%d-%d %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaTrigPol::polAsChar(trigPol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->threshold[surf][chan]);
      }      
    }

    for( int surf=0; surf<ACTIVE_SURFS; ++surf ) {
      for( int chan=0; chan<RFCHAN_PER_SURF; ++chan ) {
	AnitaGeomTool::getRingAntPolPhiFromSurfChan(surf,chan,ring,ant,pol,phi);
	sprintf(elementName,"rfPower%d_%d",surf,chan);
	sprintf(elementLabel,"%d-%d %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getRFPowerInK(surf,chan)); //need to mask the top bit (brotter)

	sprintf(elementName,"phiRfPower%d_%c%c",phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));
	sprintf(elementLabel,"%d-%d %d%c%c",surf+1,chan+1,phi+1,AnitaRing::ringAsChar(ring),AnitaPol::polAsChar(pol));      
	summaryFile.addVariablePoint(elementName,elementLabel,timeStamp,surfHkPtr->getRFPowerInK(surf,chan)); //need to mask the top bit (brotter)

	

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
  summaryFile.writeSingleFullJSONFile(dirName,"surfHk");

  char outName[FILENAME_MAX];

  sprintf(outName,"%s/surfHkSummary.json.gz",dirName);
  summaryFile.writeSummaryJSONFile(outName);


  sprintf(outName,"%s/surfHkTime.json.gz",dirName);
  summaryFile.writeTimeJSONFile(outName);


  sprintf(outName,"%s/%s/lastSurfHk",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);
  sprintf(outName,"%s/%s/lastRun",outputDir,instrumentName);
  AwareRunDatabase::updateTouchFile(outName,runNumber,lastTime);


  AwareRunDatabase::updateRunList(outputDir,instrumentName,runNumber,dateInt);
  AwareRunDatabase::updateDateList(outputDir,instrumentName,runNumber,dateInt);
}
