////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////  makeGpuSourceJsonFiles
////      This is a simple program that converts ANITA header root files 
////      into JSON files that can be read by the AWARE web plotter code
////
////    June 2014,  r.nichol@ucl.ac.uk 
////////////////////////////////////////////////////////////////////////////////

//Includes
#include <iostream>

//ANITA EventReaderRoot Includes
#include "RawAnitaHeader.h"
#include "AnitaGeomTool.h"
#include "Adu5Pat.h"

//ROOT Includes
#include "TTree.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTimeStamp.h"
#include "TSystem.h"

//AWARE Include
#include "AnitaAwareHandler.h"
#include "AwareRunDatabase.h"


int getSourceLonAndLatAtDesiredAlt(Double_t phiWave, Double_t thetaWave, Double_t latitude, Double_t longitude, Double_t altitude,Double_t heading, Double_t &sourceLon, Double_t &sourceLat, Double_t desiredAlt);



RawAnitaHeader *hdPtr;
Adu5Pat *patPtr;
AnitaGeomTool *fUPGeomTool;

//From UsefulAdu5Pat
  TVector3 fSourcePos; ///< Private variable to hold the source location in cartesian coordinates.
  Double_t fSourceLongitude; ///< The source longitude.
  Double_t fSourceLatitude; ///< The source latitude.
  Double_t fSourceAltitude; ///< The source altitude.
  Double_t fThetaWave; ///< The elevation angle of the plane wave in payload centric coordinates.
  Double_t fPhiWave; ///< The azimuthal angle of the plane wave in payload centric coordinates with phi equals zero lying along the direction of the ADU5 fore antenna.
  Double_t fBalloonCoords[3]; ///< The balloon position in cartesian coords
  TVector3 fBalloonPos; ///< The cartesian coords as a TVector3
  Double_t fBalloonTheta; ///< The balloon theta
  Double_t fBalloonPhi; ///< The balloon phi
  Double_t fBalloonHeight; ///< The balloon height

void usage(char **argv) 
{  
  std::cout << "Usage\n" << argv[0] << " <head file> <gps event file>\n";
  std::cout << "e.g.\n" << argv[0] << " http://www.hep.ucl.ac.uk/uhen/anita/private/anitaIIData/flight0809/root/run13/headFile13.root\n";  
}


int main(int argc, char **argv) {
  if(argc<3) {
    usage(argv);
    return -1;
  }
  fUPGeomTool=AnitaGeomTool::Instance();

  TFile *fpHead = TFile::Open(argv[1]);
  if(!fpHead) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *headTree = (TTree*) fpHead->Get("headTree");
  if(!headTree) {
    std::cerr << "Can't find headTree\n";
    return -1;
  }

  if(headTree->GetEntries()<1) {
    std::cerr << "No entries in headTree\n";
    return -1;
  }
   
  //Check an event in the run Tree and see if it is station1 or TestBed (stationId<2)
  headTree->SetBranchAddress("header",&hdPtr);
  headTree->GetEntry(0);


  TFile *fpGps = TFile::Open(argv[2]);
  if(!fpGps) {
    std::cerr << "Can't open file\n";
    return -1;
  }
  TTree *adu5PatTree=(TTree*) fpGps->Get("adu5PatTree");
  if(!adu5PatTree) {
    std::cerr << "Can't find adu5PatTree\n";
    return -1;
  }

  if(adu5PatTree->GetEntries()<1) {
    std::cerr << "No entries in adu5PatTree\n";
    return -1;
  }
   
  adu5PatTree->SetBranchAddress("pat",&patPtr);


  TTimeStamp timeStamp((time_t)hdPtr->triggerTime,(Int_t)hdPtr->triggerTimeNs);
  UInt_t dateInt=timeStamp.GetDate();
  UInt_t firstTime=timeStamp.GetSec();  
  UInt_t lastTime=timeStamp.GetSec();
  UInt_t runNumber=hdPtr->run;

  //Now we set up out run list
  Long64_t numEntries=headTree->GetEntries();
  Long64_t starEvery=numEntries/80;
  if(starEvery==0) starEvery++;

  char instrumentName[20];
  sprintf(instrumentName,"ANITA3");

  
  //  numEntries=1;
  for(Long64_t event=0;event<numEntries;event++) {
    if(event%starEvery==0) {
      std::cerr << "*";       
    }

    //This line gets the Header Entry
    headTree->GetEntry(event);
    adu5PatTree->GetEntry(event);
    
    Double_t phiWave=hdPtr->getPeakPhiRad();    
    Double_t thetaWave=hdPtr->getPeakThetaRad();
    Double_t latitude=patPtr->latitude;
    Double_t longitude=patPtr->longitude;
    Double_t altitude=patPtr->altitude;
    Double_t heading=patPtr->heading;
    Double_t desiredAlt=2000; //Could change this to be ground level ish.
    Double_t sourceLat=0;Double_t sourceLon=0;


   fUPGeomTool->getCartesianCoords(latitude,longitude,altitude,
				   fBalloonCoords);
   fBalloonPos.SetXYZ(fBalloonCoords[0],fBalloonCoords[1],fBalloonCoords[2]);
   fBalloonTheta=fBalloonPos.Theta();
   fBalloonPhi=fBalloonPos.Phi();
   if(fBalloonPhi<0) fBalloonPhi+=TMath::TwoPi();
   fBalloonHeight=fBalloonPos.Mag();

    Int_t retVal=getSourceLonAndLatAtDesiredAlt(phiWave,thetaWave,latitude,longitude,altitude,heading,sourceLon,sourceLat,desiredAlt);
    std::cout << retVal << "\t" << latitude << "\t" << longitude << "\t" << phiWave*TMath::RadToDeg() << "\t" << thetaWave*TMath::RadToDeg() << "\t" << heading << "\t" << sourceLon << "\t" << sourceLat << "\n";
    

    
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
    
}


int getSourceLonAndLatAtDesiredAlt(Double_t phiWave, Double_t thetaWave, Double_t latitude, Double_t longitude, Double_t altitude, Double_t heading, Double_t &sourceLon, Double_t &sourceLat, Double_t desiredAlt) {

  std::cout << "getSourceLonAndLatAtDesiredAlt " << phiWave << "\t" << thetaWave << "\n";
   
   Double_t tempPhiWave=phiWave;
   Double_t tempThetaWave=TMath::PiOver2()-thetaWave;
   //Now need to take account of balloon heading


   if(heading>=0 && heading<=360) {
     TVector3 arbDir;
     arbDir.SetMagThetaPhi(1,tempThetaWave,-1*tempPhiWave);

     //std::cout << "tempPhiWave3: " << arbDir.Phi() << "\t" << tempPhiWave << "\n";

     arbDir.Rotate((heading-45)*TMath::DegToRad(),fUPGeomTool->fHeadingRotationAxis);

     tempPhiWave=arbDir.Phi();
     //std::cout << "tempPhiWave2: " << tempPhiWave << "\n";
     if(tempPhiWave>TMath::TwoPi()) {
       tempPhiWave-=TMath::TwoPi();
     }
     if(tempPhiWave<0) {
       tempPhiWave+=TMath::TwoPi();
     }
     tempThetaWave=arbDir.Theta();
   }
   else std::cout << "heading bad" << std::endl;
   
   std::cout << "Get source angles: " <<  tempThetaWave << "\t" << tempPhiWave << "\n";

 
   
   Double_t reBalloon=fUPGeomTool->getDistanceToCentreOfEarth(latitude)+desiredAlt; // ACG mod
   //   std::cout << "Radius difference: " << re-re2 << "\t" << re << "\t" << re2 << "\n";
   Double_t re=reBalloon;
   Double_t nextRe=re;
   Double_t reh=reBalloon+altitude;
   do {
     //Okay so effectively what we do here is switch to cartesian coords with the balloon at RE_balloon + altitude and then try to fins where the line at thetaWave cuts the Earth's surface.
     //This is iterative because the Earth is cruel and isn't flat, and I couldn't be bothered to work out how to do it more elegantly.
     re=nextRe;

     Double_t sintw=TMath::Sin(tempThetaWave);
     Double_t costw=TMath::Cos(tempThetaWave);
     //     Double_t sqrtArg=(reh*reh*costw*costw - (reh*reh-re*re));
     Double_t sqrtArg(re*re-reh*reh*sintw*sintw);
     if(sqrtArg<0) {
       // No solution possible
       //     std::cout << "No solution possible\n";
       return 0;
     }
     Double_t L=reh*costw - TMath::Sqrt(sqrtArg);
     Double_t sinThetaL=L*sintw/re;
     Double_t sourceTheta=TMath::ASin(sinThetaL);
     
     //Start at ground below balloon
     fSourcePos.SetX(0);
     fSourcePos.SetY(0);
     fSourcePos.SetZ(re);
     
     //std::cout << "00ReLoc: " << fSourcePos.X() << "\t" << fSourcePos.Y() << "\t" << fSourcePos.Z() << "\n";
     
     //Rotate to latitude relative to balloon
     fSourcePos.RotateY(sourceTheta);   
     //Rotate to longitude relative to balloon
     fSourcePos.RotateZ(-1*tempPhiWave);
     
     //std::cout << "RelBalloonLoc: " << fSourcePos.X() << "\t" << fSourcePos.Y() << "\t" << fSourcePos.Z() << "\n";
     
     //Rotate to correct absolute values
     //std::cout << "Balloon angles: " << fBalloonTheta << "\t" << fBalloonPhi << "\n";
     fSourcePos.RotateY(fBalloonTheta);
     fSourcePos.RotateZ(fBalloonPhi);
     //Goofy sign thing
     //   fSourcePos.SetZ(-1*fSourcePos.Z());
     
     Double_t sourceVec[3];
     fSourcePos.GetXYZ(sourceVec);
     //std::cout << "BalloonPos: " << fBalloonPos.X() << "\t" << fBalloonPos.Y() << "\t" << fBalloonPos.Z() << "\n";
     //std::cout << "ZeroAltLoc: " << sourceVec[0] << "\t" << sourceVec[1] << "\t" << sourceVec[2] << "\n";
     Double_t sourceAlt;
     fUPGeomTool->getLatLonAltFromCartesian(sourceVec,sourceLat,sourceLon,sourceAlt);
     //std::cout << "SourceLatLonAlt: " << sourceLat << "\t" << sourceLon << "\t" 
     //	       << sourceAlt << "\n";
     nextRe=fUPGeomTool->getDistanceToCentreOfEarth(sourceLat);
     //std::cout << "Earth radius: " << nextRe << "\t" << re << "\n";
     //     break;
   } while(TMath::Abs(nextRe-re)>1);
     //   fUPGeomTool->getLonLat(fSourcePos,sourceLon,sourceLat);
   //   sourceLat*=-1;
   std::cout << "source lat " << sourceLat << " sourceLon " << sourceLon << std::endl;
   return 1;
}
