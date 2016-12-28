#include "GpuPowerSpectra.h" 
#include <stdio.h>
#include "TString.h" 
#include <time.h>
#include "TSystem.h" 
#include "TAxis.h" 
#include "TTimeStamp.h" 
#include "TChain.h" 
#include "TText.h"
#include "TGraph.h" 
#include "TCanvas.h" 
#include "AwareRunDatabase.h" 

static GpuPowerSpectra* gpu = NULL;


TGraph * g[16] = {0}; 


int main(int nargs, char ** args)
{

  if (nargs < 2) 
  {
    fprintf(stderr, "Usage: %s <input files>\n", args[0]); 
    return 1; 
  }

  char * outputDir = getenv("AWARE_OUTPUT_DIR"); 
  if (!outputDir) 
  {
    fprintf(stderr,"Please define AWARE_OUTPUT_DIR\n"); 
    return 1 ;
  }



  TChain c("gpuTree"); 


  for (int i = 1; i < nargs; i++) 
  {
    c.Add(args[i]); 
  }

  c.SetBranchAddress("gpu",&gpu); 

  //figure out output directory 
  


  TString gpudir; 
  TString lastFileStr; 
  int run = -1; 
  unsigned lastTime = 0; 
  int lastRun = 0; 
  int last = -1; 
  int entry_within_run = 0; 
  for (int i = 0; i < c.GetEntries(); i++) 
  {
    c.GetEntry(i); 
    if (gpu->run != run) 
    {
      run = gpu->run; 
      entry_within_run = 0; 
      gpudir.Form("%s/ANITA4/gpu/run%d/", outputDir, run); 

      gSystem->mkdir(gpudir.Data(),true); 

      last = -1; 
      //check if last file exists, and if it does read last entry 
      lastFileStr = gpudir + "/last"; 
      FILE * f = fopen(lastFileStr.Data(),"r"); 
      if (f) 
      {
        fscanf(f,"%d",&last); 
        fclose(f); 
      }
    }

    if (lastRun < run) lastRun = run; 
    if (lastTime < gpu->unixTimeLastEvent) lastTime = gpu->unixTimeLastEvent; 

    if (entry_within_run > last) 
    {
      TCanvas c("gpu","gpu",2048,1600); 
      TPad ptop("ptop","top part", 0,0.85,1.0,1.0); 
      TPad pbot("pbot","bottom part",0,0.0,1.0,0.85); 
      ptop.Draw(); 
      ptop.cd(); 
      TTimeStamp ts(gpu->unixTimeFirstEvent); 
      int nsecs = gpu->unixTimeLastEvent - gpu->unixTimeFirstEvent; 
      TText text(0.5,0.7, TString::Format("RUN %d, ENTRY %d (%s, %s)", run, entry_within_run, gpu->pol == AnitaPol::kHorizontal ? "HPol" : "VPol", gpu->ring == AnitaRing::kTopRing ? "Top" : gpu->ring == AnitaRing::kMiddleRing ? "Middle" : "Bottom")); 
      TText text2(0.5,0.3, TString::Format("%d-second average starting at %s",  nsecs, ts.AsString("c")));    
      text.SetTextAlign(22); 
      text.SetTextSize(0.2); 
      text.Draw(); 
      text2.SetTextAlign(22); 
      text2.SetTextSize(0.2); 
      text2.Draw(); 
      c.cd(); 

      pbot.Draw(); 
      pbot.Divide(4,4); 

      for (int j = 0; j < 16; j++) 
      {
        pbot.cd(j+1); 
        if (g[j]) delete g[j]; 
        g[j] = gpu->getGraph(j); 
        g[j]->SetTitle(TString::Format("Phi Sector %d",j+1)); 
        g[j]->GetXaxis()->SetTitle("Frequency");
        g[j]->GetYaxis()->SetTitle("dB + random offset");
        g[j]->SetLineWidth(2); 
        g[j]->Draw("al"); 
      }
      c.SaveAs(TString::Format("%s/%04d.png",gpudir.Data(), entry_within_run)); 
      last = entry_within_run; 

      FILE * f = fopen(lastFileStr.Data(),"w"); 
      fprintf(f,"%d\n", last); 
      fclose(f); 
    }
    

    entry_within_run++; 

  }


  TString outName; 
  outName.Form("%s/ANITA4/lastGpuPowerSpectra", outputDir); 
  AwareRunDatabase::updateTouchFile(outName.Data(),lastRun,lastTime);

  return 0 ; 


}
