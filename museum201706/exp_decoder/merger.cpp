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
const int n_board    = 2;
const int n_chip =     4;
const int n_unit =     4;
const int board_list[n_board] = {2,5};
//const int n_board = 4;
//const int board_list[n_board] = {2,5,3,6};

// for read
std::vector<int>* t_board_v;
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_channel_v;
std::vector<int>* t_ledge_v;
std::vector<int>* t_tedge_v;
std::vector<int>* t_prev_ledge_v;
std::vector<int>* t_prev_tedge_v;
std::vector<int>* t_fl_fall_v;


// for write
std::vector<int> t2_board_v;
std::vector<int> t2_chip_v;
std::vector<int> t2_unit_v;
std::vector<int> t2_bit_v;
std::vector<int> t2_channel_v;
std::vector<int> t2_ledge_v;
std::vector<int> t2_tedge_v;
std::vector<int> t2_prev_ledge_v;
std::vector<int> t2_prev_tedge_v;
std::vector<int> t2_fl_fall_v;       // added @20170610
std::vector<int> t2_fl_fall_board_v; // added @20170610


int   t_event_number;
int   t_nevent_overflow;
float t_vref   = -999;
float t_hv     = -999;

int set_tree( TTree* tree ){
  tree->Branch( "event",         &t_event_number,    "event/I"    );
  tree->Branch( "overflow",      &t_nevent_overflow, "overflow/I" );
  tree->Branch( "board",         &t2_board_v         );
  tree->Branch( "chip",          &t2_chip_v          );
  tree->Branch( "unit",          &t2_unit_v          );
  tree->Branch( "bit",           &t2_bit_v           );
  tree->Branch( "channel",       &t2_channel_v       );
  tree->Branch( "ledge",         &t2_ledge_v         );
  tree->Branch( "tedge",         &t2_tedge_v         );
  tree->Branch( "prev_ledge",    &t2_prev_ledge_v    );
  tree->Branch( "prev_tedge",    &t2_prev_tedge_v    );
  tree->Branch( "fl_fall",       &t2_fl_fall_v       );
  tree->Branch( "fl_fall_board", &t2_fl_fall_board_v );
  tree->Branch( "vref",          &t_vref,  "vref/F"  );
  tree->Branch( "hv",            &t_hv,    "hv/F"    );

  return 0;
}

int init_tree(){
  t2_board_v.clear();
  t2_chip_v.clear();
  t2_unit_v.clear();
  t2_bit_v.clear();
  t2_channel_v.clear();
  t2_ledge_v.clear();
  t2_tedge_v.clear();
  t2_prev_ledge_v.clear();
  t2_prev_tedge_v.clear();
  t2_fl_fall_v.clear();
  t2_fl_fall_board_v.clear();
  
  return 0;
}

int set_readbranch( TChain* tree ){
  tree->SetBranchAddress( "event",      &t_event_number    );
  tree->SetBranchAddress( "overflow",   &t_nevent_overflow );
  tree->SetBranchAddress( "board",      &t_board_v         );
  tree->SetBranchAddress( "chip",       &t_chip_v          );
  tree->SetBranchAddress( "unit",       &t_unit_v          );
  tree->SetBranchAddress( "bit",        &t_bit_v           );
  tree->SetBranchAddress( "channel",    &t_channel_v       );
  tree->SetBranchAddress( "ledge",      &t_ledge_v         );
  tree->SetBranchAddress( "tedge",      &t_tedge_v         );
  tree->SetBranchAddress( "prev_ledge", &t_prev_ledge_v    );
  tree->SetBranchAddress( "prev_tedge", &t_prev_tedge_v    );
  tree->SetBranchAddress( "fl_fall",    &t_fl_fall_v       );
  tree->SetBranchAddress( "vref",       &t_vref            );
  tree->SetBranchAddress( "hv",         &t_hv              );

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

  TFile* rootf = new TFile( outfilename, "RECREATE" );

  TTree* newtree = new TTree( "slit128A", "slit128A" );
  set_tree( newtree );

  std::cout << std::endl << "Now Loading..." << std::endl;
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::multimap<int, int> map; // <key, value>, sorted by key, All data is sorted by event number;
  for( int ievt=0; ievt<tree->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    if( ievt % 100== 0 ) std::cout << " " << ievt << " event..."<< std::endl;
    tree->GetEntry(ievt);
    map.insert( std::pair<int, int>(t_event_number, ievt) );
  } // END EVENT-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  int cnt_board  = 0;
  int cnt_evt    = 0;
  int prev_event = -999;
  std::multimap<int, int>::iterator it_last = map.end();
  it_last--;

  std::cout << std::endl << "Now Sorting..." << std::endl;
  //  for( std::multimap<int, int>::iterator ievt = map.begin(); ievt != map.end(); ievt++ ){ // START EVENT LOOP sorted by event number
  for( std::multimap<int, int>::iterator ievt = map.begin(); ievt != it_last; ievt++ ){ // START EVENT LOOP sorted by event number

    tree->GetEntry( ievt->second, 0 );

    if( ievt!=map.begin() && (ievt->first != prev_event)  ){ // detect "next event"

      if( cnt_evt % 100== 0 ) std::cout << " " << cnt_evt << " event..."<< std::endl;
      
      if( cnt_board!= n_board ) printf( "[Warning] evtNo=%d : Wrong #board : %d\n", prev_event,cnt_board );
      newtree->Fill();

      if( fl_message ) std::cout << "   Nhit = " << t2_chip_v.size() << std::endl;
      init_tree();
      cnt_evt++;
      cnt_board = 0;
      prev_event = ievt->first;
      if( fl_message ) std::cout << "evtNo  = " << prev_event << std::endl;


    }else if( ievt==map.begin() ){  // detect "first event"
      prev_event = ievt->first;
      if( fl_message ) std::cout << "evtNo = " << prev_event << std::endl;
    }
    /*
    std::cout << " " << t_chip_v->size()
      	      << " " << t_unit_v->size()
      	      << " " << t_channel_v->size()
      	      << " " << t_bit_v->size()
	      << " " << t_board_v->size()
	      << std::endl;
     */
    if( fl_message ) std::cout << "   nhit = " << t_chip_v->size() << std::endl;
    for( int ivec=0; ivec<t_chip_v->size(); ivec++ ){
      t2_board_v.push_back( t_board_v->at(ivec) );
      t2_chip_v.push_back ( t_chip_v ->at(ivec) );
      t2_unit_v.push_back ( t_unit_v ->at(ivec) );
      t2_bit_v.push_back  ( t_bit_v  ->at(ivec) );
      t2_channel_v.push_back( t_channel_v  ->at(ivec) );
      t2_ledge_v.push_back ( t_ledge_v ->at(ivec) );
      t2_tedge_v.push_back ( t_tedge_v ->at(ivec) );
      t2_prev_ledge_v.push_back ( t_prev_ledge_v ->at(ivec) );
      t2_prev_tedge_v.push_back ( t_prev_tedge_v ->at(ivec) );
    }

    if( t_chip_v->size() ){
      t2_fl_fall_board_v.push_back( t_board_v->at(0) );
      for( int ii = 0; ii < n_chip*n_unit; ii++ ){ // added @20170610
	t2_fl_fall_v.push_back( t_fl_fall_v->at(ii) );
      }
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

  newtree->Write();
  rootf->Close();

  delete tree;
  delete rootf;
  //    delete newtree;


  return 0;

}
