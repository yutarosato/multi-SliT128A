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

const int fl_message = 1; // 0(simple message), 1(normal message), 2(detailed message)
const int n_board = 2;
const int board_list[n_board] = {2,5};
//const int n_board = 4;
//const int board_list[n_board] = {2,5,3,6};

// for read
std::vector<int>* t_board_v;
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_time_v;

// for write
std::vector<int> t2_board_v;
std::vector<int> t2_chip_v;
std::vector<int> t2_unit_v;
std::vector<int> t2_bit_v;
std::vector<int> t2_time_v;

int   t_event_number;
int   t_nevent_overflow;
float t_vref0  = -999;
float t_vref1  = -999;
float t_vref23 = -999;
float t_hv     = -999;

int set_tree( TTree* tree ){
  tree->Branch( "event",       &t_event_number,    "event/I"    );
  tree->Branch( "overflow",    &t_nevent_overflow, "overflow/I" );
  tree->Branch( "board",       &t2_board_v           );
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

int init_tree(){
  t2_board_v.clear();
  t2_chip_v.clear();
  t2_unit_v.clear();
  t2_bit_v.clear();
  t2_time_v.clear();
  
  return 0;
}

int set_readbranch( TChain* tree ){
  tree->SetBranchAddress("event", &t_event_number);
  tree->SetBranchAddress("board", &t_board_v     );
  tree->SetBranchAddress("chip",  &t_chip_v      );
  tree->SetBranchAddress("unit",  &t_unit_v      );
  tree->SetBranchAddress("bit",   &t_bit_v       );
  tree->SetBranchAddress("time",  &t_time_v      );
  tree->SetBranchAddress("vref0", &t_vref0       );
  tree->SetBranchAddress("vref1", &t_vref1       );
  tree->SetBranchAddress("vref23",&t_vref23      );
  tree->SetBranchAddress("hv",    &t_hv          );

  return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main( int argc, Char_t** argv ){
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
  printf( "[input] %s : %d entries\n", infilename, (int)tree->GetEntries() );
  std::cout << tree->GetEntries() << " events(" << infilename << ") -> " << outfilename << std::endl;

  TTree* newtree = new TTree( "slit128A", "slit128A" );
  set_tree( newtree );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::multimap<int, int> map; // <key, value>, sorted by key, All data is sorted by event number;
  for( int ievt=0; ievt<tree->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    tree->GetEntry(ievt);
    map.insert( std::pair<int, int>(t_event_number, ievt) );
  } // END EVENT-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  int cnt_board = 0;
  int cnt_evt = 0;
  int prev_event    = -999;
  std::multimap<int, int>::iterator it_last = map.end();
  it_last--;

  for( std::multimap<int, int>::iterator ievt = map.begin(); ievt != map.end(); ievt++ ){ // START EVENT LOOP sorted by event number
    tree->GetEntry( ievt->second, 0 );


    if( ievt!=map.begin() && (ievt->first != prev_event)  ){ // detect "next event"
      if( cnt_board!= n_board ) printf( "[Warning] evtNo=%d : Wrong #board : %d\n", prev_event,cnt_board );
      newtree->Fill();
      std::cout << "   Nhit = " << t2_chip_v.size() << std::endl;
      init_tree();
      std::cout << "   Nhitrst = " << t2_chip_v.size() << std::endl;
      cnt_evt++;
      cnt_board = 0;
      prev_event = ievt->first;
      if( fl_message ) std::cout << "evtNo = " << prev_event << std::endl;
    }else if( ievt==map.begin() ){
      prev_event = ievt->first;
      if( fl_message ) std::cout << "evtNo = " << prev_event << std::endl;      
    }
    std::cout << "   nhit = " << t_chip_v->size() << std::endl;
    for( int ivec=0; ivec<t_chip_v->size(); ivec++ ){
      t2_board_v.push_back( t_board_v->at(ivec) );
      t2_chip_v.push_back ( t_chip_v ->at(ivec) );
      t2_unit_v.push_back ( t_unit_v ->at(ivec) );
      t2_bit_v.push_back  ( t_bit_v  ->at(ivec) );
      t2_time_v.push_back ( t_time_v ->at(ivec) );
    }

    cnt_board++;

    if( ievt==it_last) { // for last event
      newtree->Fill();
      init_tree();
      cnt_evt++;
      if( cnt_board!= n_board ) printf( "[Warning] evtNo=%d : Wrong #board : %d\n", prev_event,cnt_board );
    }
  } // END EVENT-LOOP sorted by event number
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::cout << "AAAA" << std::endl;

  TFile* rootf = new TFile( outfilename, "RECREATE" );
  newtree->Write();
  rootf->Close();

  delete tree;
  delete newtree;
  delete rootf;

  return 0;

}
