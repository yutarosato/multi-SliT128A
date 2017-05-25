#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>

#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TChain.h>
#include <TTree.h>
#include <TH2C.h>
#include <TFile.h>

const int fl_message = 0; // 0(simple message), 1(normal message), 2(detailed message)



// for read
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_time_v;

// for write
std::vector<int> t2_chip_v;
std::vector<int> t2_unit_v;
std::vector<int> t2_bit_v;
std::vector<int> t2_time_v;

Int_t   t_event;
Int_t   t_rf     = -999;
Float_t t_vref0  = -999;
Float_t t_vref1  = -999;
Float_t t_vref23 = -999;
Float_t t_hv     = -999;

int set_tree( TTree* tree ){
  tree->Branch( "event",       &t_event,  "event/I"  );
  tree->Branch( "rf",          &t_rf,     "rf/I"     );
  tree->Branch( "chip",        &t2_chip_v            );
  tree->Branch( "unit",        &t2_unit_v            );
  tree->Branch( "bit",         &t2_bit_v             );
  tree->Branch( "time",        &t2_time_v            );
  tree->Branch( "vref0",       &t_vref0,  "vref0/F"  );
  tree->Branch( "vref1",       &t_vref1,  "vref1/F"  );
  tree->Branch( "vref23",      &t_vref23, "vref23/F" );
  tree->Branch( "hv",          &t_hv,     "hv/F"     );

  return 0;
}

Int_t init_tree(){

  t2_chip_v.clear();
  t2_unit_v.clear();
  t2_bit_v.clear();
  t2_time_v.clear();
  
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
  std::cout << tree->GetEntries() << " events(" << infilename << ") -> " << outfilename << std::endl;

  TTree* newtree = new TTree( "slit128A", "slit128A" );
  set_tree( newtree );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::multimap<Int_t, Int_t> map; // <key, value>, sorted by key, All data is sorted by event number;
  for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    tree->GetEntry(ievt);
    map.insert( std::pair<Int_t, Int_t>(t_event, ievt) );
  } // END EVENT-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt           = 0;
  Int_t prev_event    = -999;
  std::multimap<Int_t, Int_t>::iterator it_last = map.end();
  it_last--;

  for( std::multimap<Int_t, Int_t>::iterator ievt = map.begin(); ievt != map.end(); ievt++ ){ // START EVENT LOOP sorted by event number
    tree->GetEntry( ievt->second, 0 );
    
    if( ievt!=map.begin() && (ievt->first != prev_event)  ){ // detect "next event" 
      newtree->Fill();
      init_tree();
      cnt++;
      prev_event = ievt->first;
    }
    for( Int_t ivec=0; ivec<t_chip_v->size(); ivec++ ){
      t2_chip_v.push_back( t_chip_v->at(ivec) );
      t2_unit_v.push_back( t_unit_v->at(ivec) );
      t2_bit_v.push_back ( t_bit_v->at (ivec) );
      t2_time_v.push_back( t_time_v->at(ivec) );
    }

    if( ievt==it_last) { // for last event
      newtree->Fill();
      init_tree();
      cnt++;
    }
    
  } // END EVENT-LOOP sorted by event number
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  TFile* rootf = new TFile( outfilename, "RECREATE" );
  newtree->Write();
  rootf->Close();

  delete tree;
  delete newtree;
  delete rootf;

  return 0;

}
