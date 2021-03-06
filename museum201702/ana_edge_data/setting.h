#ifndef SETTING_H
#define SETTING_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TColor.h>
#include <TChain.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TApplication.h>
#include <TRint.h>
#include <TRandom.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TPaveStats.h>
#include <TText.h>
#include <TLatex.h>
#include <THStack.h>
#include <TH1C.h>
#include <TH2C.h>
#include <TFile.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>

Int_t t_event;


const Int_t n_dac = 64;

const int n_chip = 4;
const int n_unit = 4;
const int n_bit  = 32;
const int n_time =  8192; // pow(2,13)
Int_t t_data[n_chip][n_unit][n_bit][n_time];
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_channel_v;
std::vector<int>* t_ledge_v;
std::vector<int>* t_tedge_v;
std::vector<int>* t_prev_ledge_v;
std::vector<int>* t_prev_tedge_v;

Float_t t_vref0;
Float_t t_vref1;
Float_t t_vref23;
Float_t t_hv;
Int_t   t_rf;

TStyle* Style(){
  TStyle* myStyle = new TStyle("sty", "sty");
  //set the background color to white
  myStyle->SetFillColor(10);
  myStyle->SetFrameFillColor(10);
  myStyle->SetCanvasColor(10);
  myStyle->SetPadColor(10);
  myStyle->SetTitleFillColor(10);
  myStyle->SetStatColor(10);
  
  //don't put a colored frame around the plots
  myStyle->SetFrameBorderMode(0);
  myStyle->SetCanvasBorderMode(0);
  myStyle->SetPadBorderMode(0);
  myStyle->SetLegendBorderSize(1);
  
  //use the primary color palette
  myStyle->SetPalette(1,0);
  
  //set the default line color for a histogram to be black
  myStyle->SetHistLineColor(kBlack);
  
  //set the default line color for a fit function to be red
  myStyle->SetFuncColor(kRed);

  //make the axis labels black
  myStyle->SetLabelColor(kBlack,"xyz");

  // set the length of error bar
  myStyle->SetEndErrorSize(0);

  //set the default title color to be black
  myStyle->SetTitleColor(kBlack);
  
  myStyle->SetPadBottomMargin(0.13);
  myStyle->SetPadTopMargin   (0.10);
  myStyle->SetPadRightMargin (0.10);
  myStyle->SetPadLeftMargin  (0.07);
  
  //set axis label and title text sizes
  //myStyle->SetLabelFont(42,"xyz");
  myStyle->SetLabelSize(0.05,"xyz");
  myStyle->SetLabelOffset(0.01,"xyz");
  //myStyle->SetTitleFont(42,"xyz");
  myStyle->SetTitleSize(0.06,"xyz");
  myStyle->SetTitleOffset(1.0,"x");
  myStyle->SetTitleOffset(0.5,"y");
  myStyle->SetTitleOffset(0.4,"z");
  //myStyle->SetStatFont(42);
  //myStyle->SetStatFontSize(0.07);
  myStyle->SetTitleBorderSize(0);
  myStyle->SetStatBorderSize(0);
  //myStyle->SetTextFont(42);

  // set label-style
  myStyle->SetStripDecimals(false);
  //TGaxis::SetMaxDigits(3);

  //set line widths
  myStyle->SetFrameLineWidth(1);
  myStyle->SetFuncWidth(1);
  myStyle->SetHistLineWidth(1);
  myStyle->SetFuncStyle(2);
  
  //set the number of divisions to show
  myStyle->SetNdivisions(506, "xy");

  //turn on xy grids
  myStyle->SetPadGridX(1);
  myStyle->SetPadGridY(1);
  
  //set the tick mark style
  myStyle->SetPadTickX(1);
  myStyle->SetPadTickY(1);
  myStyle->SetTickLength( 0.02, "XY" );

  //turn off stats
  myStyle->SetOptStat(0);
  //myStyle->SetOptFit(1111);
  
  //marker settings
  myStyle->SetMarkerStyle(20);
  myStyle->SetMarkerSize(0.7);
  myStyle->SetLineWidth(1);

  //done
  myStyle->cd();

  gROOT->GetColor( 3)->SetRGB(0.0, 0.0, 1.0); // blue
  gROOT->GetColor( 4)->SetRGB(0.0, 0.5, 0.0); // green
  gROOT->GetColor( 5)->SetRGB(0.8, 0.0, 0.8); // purple
  gROOT->GetColor( 6)->SetRGB(1.0, 0.4, 0.1); // orange
  gROOT->GetColor( 7)->SetRGB(0.2, 0.6, 0.6); // light blue
  gROOT->GetColor( 8)->SetRGB(0.6, 0.3, 0.3); // brown
  gROOT->GetColor( 9)->SetRGB(1.0, 1.0, 0.0); // yellow
  gROOT->GetColor(10)->SetRGB(1.0, 1.0, 1.0); // white (canvas color)
  gROOT->GetColor(11)->SetRGB(0.4, 0.4, 0.4); // gray
  gROOT->GetColor(12)->SetRGB(1.0, 0.0, 1.0); // light purple
  gROOT->GetColor(13)->SetRGB(1.0, 0.5, 1.0); // light red
  gROOT->GetColor(14)->SetRGB(1.0, 0.6, 0.0); // light orange
  gROOT->GetColor(15)->SetRGB(0.4, 1.0, 1.0); // very light blue
  gROOT->GetColor(16)->SetRGB(0.0, 1.0, 0.3); // light green
  gROOT->GetColor(17)->SetRGB(0.7, 0.7, 0.7); // light gray

  gROOT->ForceStyle();

  return myStyle;
}

Int_t set_readbranch( TChain* tree ){
  tree->SetBranchAddress("event",      &t_event        );
  tree->SetBranchAddress("rf",         &t_rf           );
  tree->SetBranchAddress("chip",       &t_chip_v       );
  tree->SetBranchAddress("unit",       &t_unit_v       );
  tree->SetBranchAddress("bit",        &t_bit_v        );
  tree->SetBranchAddress("channel",    &t_channel_v    );
  tree->SetBranchAddress("ledge",      &t_ledge_v      );
  tree->SetBranchAddress("tedge",      &t_tedge_v      );
  tree->SetBranchAddress("prev_ledge", &t_prev_ledge_v );
  tree->SetBranchAddress("prev_tedge", &t_prev_tedge_v );
  tree->SetBranchAddress("vref0",      &t_vref0        );
  tree->SetBranchAddress("vref1",      &t_vref1        );
  tree->SetBranchAddress("vref23",     &t_vref23       );
  tree->SetBranchAddress("hv",         &t_hv           );
  

  return 0;
}

Int_t set_readbranch( TTree* tree ){
  tree->SetBranchAddress("event",      &t_event        );
  tree->SetBranchAddress("rf",         &t_rf           );
  tree->SetBranchAddress("chip",       &t_chip_v       );
  tree->SetBranchAddress("unit",       &t_unit_v       );
  tree->SetBranchAddress("bit",        &t_bit_v        );
  tree->SetBranchAddress("channel",    &t_channel_v    );
  tree->SetBranchAddress("ledge",      &t_ledge_v      );
  tree->SetBranchAddress("tedge",      &t_tedge_v      );
  tree->SetBranchAddress("prev_ledge", &t_prev_ledge_v );
  tree->SetBranchAddress("prev_tedge", &t_prev_tedge_v );
  tree->SetBranchAddress("vref0",      &t_vref0        );
  tree->SetBranchAddress("vref1",      &t_vref1        );
  tree->SetBranchAddress("vref23",     &t_vref23       );
  tree->SetBranchAddress("hv",         &t_hv           );

  return 0;
}


Int_t set_tree( TTree* tree ){// for modify tree @20160822
  tree = new TTree("slit128A","slit128A");
  tree->Branch( "event",      &t_event,  "event/I"   );
  tree->Branch( "rf",         &t_rf,     "rf/I"      );
  tree->Branch( "chip",       &t_chip_v              );
  tree->Branch( "unit",       &t_unit_v              );
  tree->Branch( "bit",        &t_bit_v               );
  tree->Branch( "channel",    &t_channel_v           );
  tree->Branch( "ledge",      &t_ledge_v             );
  tree->Branch( "tedge",      &t_tedge_v             );
  tree->Branch( "prev_ledge", &t_prev_ledge_v        );
  tree->Branch( "prev_tedge", &t_prev_tedge_v        );
  tree->Branch( "vref0",      &t_vref0,   "vref0/F"  );
  tree->Branch( "vref1",      &t_vref1,   "vref1/F"  );
  tree->Branch( "vref23",     &t_vref23,  "vref23/F" );
  tree->Branch( "hv",         &t_hv,      "t_hv/F"   );

  return 0;
}

Int_t ch_map( Int_t unit, Int_t bit ){
  return n_bit*unit + bit;
}

Int_t multi_ch_map   ( Int_t chip, Int_t unit, Int_t bit ){ return chip*n_unit*n_bit + n_bit*unit + bit; } // return Global Channel-No. (0-511 for 4ASIC)
Int_t rev_ch_map_chip( Int_t gch ){ return (gch/(n_bit*n_unit)); } // return chip-No    from global Channel-No.
Int_t rev_ch_map_ch  ( Int_t gch ){ return (gch%(n_bit*n_unit)); } // return channel-No from global Channel-No.

#endif
