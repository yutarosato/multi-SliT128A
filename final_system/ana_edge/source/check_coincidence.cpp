#include "setting.h"
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TStyle.h>


const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 0;


const int min_width = 15; // estimated minimum signal width ( for signal estimation )

const int max_width = 200; // for histogram
const int max_hit   = 200; // for histogram
const int max_cluster = 30;
const int count_threshold = 10;
const double b2t = 5e-3; // bit to time (5 ns)
const int before_prompt_time = 2000; // bit time before prompt
const int muon_time = 2280; // bit time muon coming
const int noise_start_time = 1000;


using namespace std;

Int_t main( Int_t argc, Char_t** argv ){

  
  TStyle* sty = Style();
  gRandom->SetSeed(time(NULL));
  //  TApplication app( "app", &argc, argv );    
 

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( argc < 2 ){
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << argv[ 0 ]
	      << " (char*)infilename" 
	      <<  std::endl
	      << std::endl;

    return 0;
  }

  //-- input kanda detectors data--
  Char_t* name = argv[ 1 ];
  TFile *file = new TFile(name);
  TTree *tree = (TTree*)file->Get("tree");

  int t_good_evt_flag;
  int silicon_evt_flag;

  tree->SetBranchAddress( "good_evt_flag", &t_good_evt_flag );


  TChain* chain = new TChain("slit128A");
  
  for( int ii = 2; ii < argc; ii++ ){
    Char_t* infilename = argv[ ii ];
    chain->Add( infilename );
  }

  set_readbranch(chain);

  int n_pulse;  

  int nevt = tree ->GetEntries();
  if( nevt > chain->GetEntries() )
    nevt   = chain->GetEntries();
  for( int ievt = 0; ievt < nevt; ievt++ ){

    tree ->GetEntry( ievt );
    chain->GetEntry( ievt );

    int n_hit = t_ledge_v->size();


    //-- Initialize --
    int width = 0;
    int n_sig = 0;
    int n_pos = 0;
    //----------------

    for( Int_t ivec=0; ivec < n_hit; ivec++ ){
      
      width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );

      
      if( width > min_width ){
	n_sig++;
	if( t_ledge_v->at( ivec ) > muon_time ) n_pos++;
      }
    }
    
    if( n_sig > count_threshold ){
      n_pulse++;
    }


    cout << "event # " << ievt 
	 << " sci " << t_good_evt_flag
	 << " silicon " << n_sig
	 << endl;

  } // End of ievt Loop

}

