#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TChain.h>
#include <TTree.h>
#include <TH2C.h>
#include <TFile.h>

const int fl_message = 0; // 0(simple message), 1(normal message), 2(detailed message)
const int n_chip =     4;
const int n_unit =     4;
const int n_bit  =    32;
const int n_time =  8192; // pow(2,13)
//const int chip_id[n_chip] = {0,1,2,4};

int t_chip;
int t_unit;
int t_channel;
int t_time;
int t_ledge;
int t_tedge;
int t_prev_ledge;
int t_prev_tedge;
int t_data[n_bit];

int t_event;

// for read
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_time_v;

// for write
std::vector<int> t2_chip_v;
std::vector<int> t2_unit_v;
std::vector<int> t2_bit_v;
std::vector<int> t2_channel_v;
std::vector<int> t2_ledge_v;
std::vector<int> t2_tedge_v;
std::vector<int> t2_prev_ledge_v;
std::vector<int> t2_prev_tedge_v;

float t_vref0  = -999;
float t_vref1  = -999;
float t_vref23 = -999;
float t_hv     = -999;
int   t_rf     = -999;

int set_tree( TTree* tree ){
  tree->Branch( "event",  &t_event,  "event/I" );
  tree->Branch( "rf",     &t_rf,     "rf/I"    );
  tree->Branch( "chip",   &t2_chip_v    );
  tree->Branch( "unit",   &t2_unit_v    );
  tree->Branch( "bit",    &t2_bit_v     );
  tree->Branch( "channel",&t2_channel_v );
  tree->Branch( "ledge",  &t2_ledge_v   );
  tree->Branch( "tedge",  &t2_tedge_v   );
  tree->Branch( "prev_ledge",  &t2_prev_ledge_v   );
  tree->Branch( "prev_tedge",  &t2_prev_tedge_v   );
  tree->Branch( "vref0",  &t_vref0,  "vref0/F"  );
  tree->Branch( "vref1",  &t_vref1,  "vref1/F"  );
  tree->Branch( "vref23", &t_vref23, "vref23/F" );
  tree->Branch( "hv",     &t_hv,     "hv/F"     );

  return 0;
}

int init_tree(){
  for( int ibit=0; ibit<n_bit; ibit++ ) t_data[ibit] = 0;
  t2_chip_v.clear();
  t2_unit_v.clear();
  t2_bit_v.clear();
  t2_channel_v.clear();
  t2_ledge_v.clear();
  t2_tedge_v.clear();
  t2_prev_ledge_v.clear();
  t2_prev_tedge_v.clear();
  
  return 0;
}

Int_t set_readbranch( TChain* tree ){
  tree->SetBranchAddress("event", &t_event   );
  tree->SetBranchAddress("rf",    &t_rf      );
  tree->SetBranchAddress("chip",  &t_chip_v  );
  tree->SetBranchAddress("unit",  &t_unit_v  );
  tree->SetBranchAddress("bit",   &t_bit_v   );
  tree->SetBranchAddress("time",  &t_time_v  );
  tree->SetBranchAddress("vref0", &t_vref0   );
  tree->SetBranchAddress("vref1", &t_vref1   );
  tree->SetBranchAddress("vref23",&t_vref23  );
  tree->SetBranchAddress("hv",    &t_hv      );

  return 0;
}

Int_t multi_ch_map      ( Int_t chip, Int_t unit, Int_t bit ){ return chip*n_unit*n_bit + n_bit*unit + bit; } // return Global Channel-No. (0-511 for 4ASIC)
Int_t rev_ch_map_chip   ( Int_t gch ){ return (gch/(n_bit*n_unit));   } // return chip-No    from global Channel-No.
Int_t rev_ch_map_unit   ( Int_t gch ){ return ((gch/n_bit)%n_unit);   } // return unit-No    from global Channel-No.
Int_t rev_ch_map_bit    ( Int_t gch ){ return gch%n_bit;              } // return bit-No     from global Channel-No.
Int_t rev_ch_map_channel( Int_t gch ){ return (gch%(n_bit*n_unit));   } // return channel-No from global Channel-No.

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t main( Int_t argc, Char_t** argv ){
  TApplication app( "app", &argc, argv );
  gStyle->SetOptStat(1111111);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==3) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)outfilename (char*)infilename" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test_out.root test_int.root" << std::endl
	      << std::endl, abort();

  Char_t* outfilename = app.Argv(1);
  Char_t* infilename  = app.Argv(2);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* tree = new TChain("slit128A");
  FileStat_t info;
  if( gSystem->GetPathInfo(infilename, info) ){
    std::cout << "[ERROR] Can not find input file : " << infilename << std::endl;
    return 1;
  }
  tree->Add(infilename);
  set_readbranch(tree);
  printf( "[input] %s : %d entries\n", infilename, (Int_t)tree->GetEntries() );

  //++++++++++++++++++++++++++++++++++++++++++++++
  TTree* newtree = new TTree( "slit128A", "slit128A" );
  set_tree( newtree );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH2C* hist = new TH2C("hist", "hist; Time;Channel",n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::cout << tree->GetEntries() << " events(" << infilename << ") -> " << outfilename << std::endl;
  for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    if( ievt % (tree->GetEntries()/100)==0 ) std::cout << ievt/(tree->GetEntries()/100) << "%" << std::endl;
    tree->GetEntry(ievt);
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ) hist->Fill( t_time_v->at(ivec), multi_ch_map(t_chip_v->at(ivec),t_unit_v->at(ivec),t_bit_v->at(ivec)) );

    // ++++++++++++++++++++++++++++
    // EDGE DECODE LOGIC
    init_tree();

    for( int ich=0; ich<hist->GetNbinsY(); ich++ ){ // BEGIN Y-LOOP
      bool fl_bin_state = false;
      t_ledge      = 0;
      t_tedge      = 0;
      t_prev_ledge = -999;
      t_prev_tedge = -999;
      
      for( int itime=0; itime<hist->GetNbinsX(); itime++ ){ // BEGIN X-LOOP
	if( hist->GetBinContent(itime+1,ich+1) && !fl_bin_state ){ // detect rising-edge
	  t_ledge = itime;
	  fl_bin_state = true;
	}else if( !hist->GetBinContent(itime+1,ich+1) && fl_bin_state ){ // detect trailing-edge
	  t_tedge = itime;
	  fl_bin_state = false;
	  t2_chip_v.push_back      ( rev_ch_map_chip   (ich) );
	  t2_unit_v.push_back      ( rev_ch_map_unit   (ich) );
	  t2_bit_v.push_back       ( rev_ch_map_bit    (ich) );
	  t2_channel_v.push_back   ( rev_ch_map_channel(ich) );
	  t2_ledge_v.push_back     ( t_ledge                 );
	  t2_tedge_v.push_back     ( t_tedge                 );
	  t2_prev_ledge_v.push_back( t_prev_ledge            );
	  t2_prev_tedge_v.push_back( t_prev_tedge            );
	  t_prev_ledge = t_ledge;
	  t_prev_tedge = t_tedge;
	}
      } // END X-LOOP
      if( fl_bin_state == true ){ // exception for last time-bin
	t2_chip_v.push_back   ( rev_ch_map_chip   (ich) );
	t2_unit_v.push_back   ( rev_ch_map_unit   (ich) );
	t2_bit_v.push_back    ( rev_ch_map_bit    (ich) );
	t2_channel_v.push_back( rev_ch_map_channel(ich) );
	t2_ledge_v.push_back  ( t_ledge   );
	t2_tedge_v.push_back  ( t_ledge+1 );
	t2_prev_ledge_v.push_back( t_prev_ledge            );
	t2_prev_tedge_v.push_back( t_prev_tedge            );
      }
    } // END Y-LOOP
    // ++++++++++++++++++++++++++++
    newtree->Fill();
    hist->Reset();

  } // END EVENT-LOOP
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  TFile* rootf = new TFile( outfilename, "RECREATE" );
  newtree->Write();
  rootf->Close();

  delete tree;
  delete newtree;
  delete rootf;

  return 0;

}
