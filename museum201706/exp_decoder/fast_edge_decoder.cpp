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
const int n_chip =     4;
const int n_unit =     4;
const int n_bit  =    32;
const int n_time =  8192; // pow(2,13)

int t_now_ledge  [n_chip*n_unit*n_bit] = {0};
int t_now_tedge  [n_chip*n_unit*n_bit] = {0};
int t_prev_ledge [n_chip*n_unit*n_bit] = {0};
int t_prev_tedge [n_chip*n_unit*n_bit] = {0};

int t_event;
int t_nevent_overflow;

// for read
std::vector<int>* t_board_v;
std::vector<int>* t_chip_v;
std::vector<int>* t_unit_v;
std::vector<int>* t_bit_v;
std::vector<int>* t_time_v;
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
std::vector<int> t2_fl_fall_v;

float t_vref   = -999;
float t_hv     = -999;

//int   t_rf     = -999;

int set_tree( TTree* tree ){
  tree->Branch( "event",       &t_event,           "event/I"       );
  tree->Branch( "overflow",    &t_nevent_overflow, "overflow/I"    );
  tree->Branch( "board",       &t2_board_v      );
  tree->Branch( "chip",        &t2_chip_v       );
  tree->Branch( "unit",        &t2_unit_v       );
  tree->Branch( "bit",         &t2_bit_v        );
  tree->Branch( "channel",     &t2_channel_v    );
  tree->Branch( "ledge",       &t2_ledge_v      );
  tree->Branch( "tedge",       &t2_tedge_v      );
  tree->Branch( "prev_ledge",  &t2_prev_ledge_v );
  tree->Branch( "prev_tedge",  &t2_prev_tedge_v );
  tree->Branch( "fl_fall",     &t2_fl_fall_v    );
  tree->Branch( "vref",        &t_vref,  "vref/F" );
  tree->Branch( "hv",          &t_hv,    "hv/F"   );


  return 0;
}

int init_tree(){

  for( int i=0; i<n_chip*n_unit*n_bit; i++ ){
    t_now_ledge [i] = -999;
    t_now_tedge [i] = -999;
    t_prev_ledge[i] = -999;
    t_prev_tedge[i] = -999;
  }
  
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
  
  return 0;
}

Int_t set_readbranch( TChain* tree ){
  tree->SetBranchAddress("event",    &t_event           );
  tree->SetBranchAddress("overflow", &t_nevent_overflow );
  tree->SetBranchAddress("board",    &t_board_v         );
  tree->SetBranchAddress("chip",     &t_chip_v          );
  tree->SetBranchAddress("unit",     &t_unit_v          );
  tree->SetBranchAddress("bit",      &t_bit_v           );
  tree->SetBranchAddress("time",     &t_time_v          );
  tree->SetBranchAddress("fl_fall",  &t_fl_fall_v       );
  tree->SetBranchAddress("vref",     &t_vref            );
  tree->SetBranchAddress("hv",       &t_hv              );

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
  std::cout << tree->GetEntries() << " events(" << infilename << ") -> " << outfilename << std::endl;

  TFile* rootf = new TFile( outfilename, "RECREATE" );
  TTree* newtree = new TTree( "slit128A", "slit128A" );
  set_tree( newtree );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    if( ievt % 100== 0 ) std::cout << " " << ievt << " event..."<< std::endl;
	//    if( ievt % (tree->GetEntries()/100)==0 ) std::cout << ievt/(tree->GetEntries()/100) << "%" << std::endl;
    tree->GetEntry(ievt);

    //std::cout << std::endl << "============= ievt = " << ievt << " ==================" << std::endl;
    // ++++++++++++++++++++++++++++
    // NEW EDGE DECODE LOGIC
    init_tree();
    std::set<Int_t> fl_bin_prev_state;
    std::set<Int_t> fl_bin_now_state;
    int now_time  = -999;
    std::set<Int_t>::iterator it_now;
    std::set<Int_t>::iterator it_prev;
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){ // BEGIN BIT-LOOP
      int gch = multi_ch_map( t_chip_v->at(ivec), t_unit_v->at(ivec), t_bit_v ->at(ivec) );
      if( now_time == -999 ){ // exception for first time-bin
	now_time  = t_time_v->at(ivec);
	if( fl_message ) std::cout << "    [first time-bin]" << std::endl
				   << "      prev_time = now_time = " << t_time_v->at(ivec) << std::endl
				   << "        gch = " <<  gch << ", chip = " << t_chip_v->at(ivec) << ", uint = " << t_unit_v->at(ivec) << ", bit = " << t_bit_v->at(ivec) << std::endl;
	fl_bin_now_state.insert(gch);
      }else if( t_time_v->at(ivec) == now_time ){ // keep same time-bin
	if( fl_message ) std::cout << "    [keep same time-bin]" << std::endl
				   << "        gch = " <<  gch << ", chip = " << t_chip_v->at(ivec) << ", uint = " << t_unit_v->at(ivec) << ", bit = " << t_bit_v->at(ivec) << std::endl;
	fl_bin_now_state.insert(gch);
      }else{ // next time bin
	if( t_time_v->at(ivec)-now_time!=1 ){ // all hits at previous time bin are indentified as trailing-edge when time bin is skipped. 
	  if( fl_message ) std::cout << "    [move next time-bin (far)] " << now_time << " -> " << t_time_v->at(ivec) << std::endl
				     << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
	  
	  it_now = fl_bin_now_state.begin();
	  while( it_now !=fl_bin_now_state.end() ){
	    if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
	      if( fl_message ) std::cout << "         find leading-edge : " << *it_now << std::endl;
	      t_now_ledge[*it_now] = now_time;
	    }
	    ++it_now;
	  }

	  
	  it_prev = fl_bin_prev_state.begin();
	  while( it_prev !=fl_bin_prev_state.end() ){
	    if( fl_message ) std::cout << "         identify trailing-edge : " << *it_prev << std::endl;

	    if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ) t_now_tedge[*it_prev] = now_time;
	    else                                                          t_now_tedge[*it_prev] = now_time+1;

	    if( t_now_ledge[*it_prev] > t_now_tedge[*it_prev] ) t_now_tedge[*it_prev] = t_now_ledge[*it_prev] + 1;
	    t2_ledge_v.push_back     ( t_now_ledge [*it_prev]       );
	    t2_tedge_v.push_back     ( t_now_tedge [*it_prev]       );
	    t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]       );
	    t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]       );
	    t2_chip_v.push_back      ( rev_ch_map_chip   (*it_prev) );
	    t2_unit_v.push_back      ( rev_ch_map_unit   (*it_prev) );
	    t2_bit_v.push_back       ( rev_ch_map_bit    (*it_prev) );
	    t2_channel_v.push_back   ( rev_ch_map_channel(*it_prev) );
	    
	    t_prev_ledge[*it_prev] = t_now_ledge[*it_prev];
	    t_prev_tedge[*it_prev] = t_now_tedge[*it_prev];
	    
	    ++it_prev;
	  }

	  it_now = fl_bin_now_state.begin();
	  while( it_now !=fl_bin_now_state.end() ){
	    if( fl_message ) std::cout << "         identify trailing-edge : " << *it_now << std::endl;

	    if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ) t_now_tedge[*it_now] = now_time+1;
	    else{
	      ++it_now;
	      continue;
	    }

	    if( t_now_ledge[*it_now] > t_now_tedge[*it_now] ) t_now_tedge[*it_now] = t_now_ledge[*it_now] + 1;
	    t2_ledge_v.push_back     ( t_now_ledge [*it_now]       );
	    t2_tedge_v.push_back     ( t_now_tedge [*it_now]       );
	    t2_prev_ledge_v.push_back( t_prev_ledge[*it_now]       );
	    t2_prev_tedge_v.push_back( t_prev_tedge[*it_now]       );
	    t2_chip_v.push_back      ( rev_ch_map_chip   (*it_now) );
	    t2_unit_v.push_back      ( rev_ch_map_unit   (*it_now) );
	    t2_bit_v.push_back       ( rev_ch_map_bit    (*it_now) );
	    t2_channel_v.push_back   ( rev_ch_map_channel(*it_now) );
	    
	    t_prev_ledge[*it_now] = t_now_ledge[*it_now];
	    t_prev_tedge[*it_now] = t_now_tedge[*it_now];
	    
	    ++it_now;
	  }


	  // clear bin_state
	  fl_bin_now_state.clear();
	  fl_bin_prev_state.clear();
	  now_time = t_time_v->at(ivec);
	  fl_bin_now_state.insert(gch);
	  if( fl_message ) std::cout << "        gch = " <<  gch << ", chip = " << t_chip_v->at(ivec) << ", uint = " << t_unit_v->at(ivec) << ", bit = " << t_bit_v->at(ivec) << std::endl;

	}else{	
	  if( fl_message ) std::cout << "    [move next time-bin] " << now_time << " -> " << t_time_v->at(ivec) << std::endl
				     << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
	  it_now = fl_bin_now_state.begin();
	  while( it_now !=fl_bin_now_state.end() ){
	    if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
	      if( fl_message ) std::cout << "         find leading-edge : " << *it_now << std::endl;
	      t_now_ledge[*it_now] = now_time;
	    }
	    ++it_now;
	  }
	  
	  it_prev = fl_bin_prev_state.begin();
	  while( it_prev !=fl_bin_prev_state.end() ){
	    if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ){ // trailing-edge
	      if( fl_message ) std::cout << "         find trailing-edge : " << *it_prev << std::endl;
	      t_now_tedge[*it_prev] = now_time;
	      
	      t2_ledge_v.push_back     ( t_now_ledge [*it_prev]       );
	      t2_tedge_v.push_back     ( t_now_tedge [*it_prev]       );
	      t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]       );
	      t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]       );
	      t2_chip_v.push_back      ( rev_ch_map_chip   (*it_prev) );
	      t2_unit_v.push_back      ( rev_ch_map_unit   (*it_prev) );
	      t2_bit_v.push_back       ( rev_ch_map_bit    (*it_prev) );
	      t2_channel_v.push_back   ( rev_ch_map_channel(*it_prev) );
	      
	      t_prev_ledge[*it_prev] = t_now_ledge[*it_prev];
	      t_prev_tedge[*it_prev] = t_now_tedge[*it_prev];
	    }
	    ++it_prev;
	  }
	  
	  // copy fl_bin_state from now to prev
	  fl_bin_prev_state.clear();
	  it_now = fl_bin_now_state.begin();
	  while( it_now !=fl_bin_now_state.end() ){
	    fl_bin_prev_state.insert(*it_now);
	    ++it_now;
	  }
	  fl_bin_now_state.clear();
	  now_time = t_time_v->at(ivec);
	  fl_bin_now_state.insert(gch);
	  if( fl_message ) std::cout << "        gch = " <<  gch << ", chip = " << t_chip_v->at(ivec) << ", uint = " << t_unit_v->at(ivec) << ", bit = " << t_bit_v->at(ivec) << std::endl;
	}
      }
    } // END BIT-LOOP
    
    // exception for last time-bin
    if( fl_message ) std::cout << "    [move last+1 time-bin] " << now_time << " -> ->" << std::endl
			       << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
    it_now = fl_bin_now_state.begin();
    while( it_now !=fl_bin_now_state.end() ){
      if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
	if( fl_message ) std::cout << "         find leading-edge : " << *it_now << std::endl;
	t_now_ledge[*it_now] = now_time;
	    }
      ++it_now;
    }

    it_prev = fl_bin_prev_state.begin();
    while( it_prev !=fl_bin_prev_state.end() ){
      if( fl_message ) std::cout << "         identify trailing-edge : " << *it_prev << std::endl;

      if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ) t_now_tedge[*it_prev] = now_time;
      else                                                          t_now_tedge[*it_prev] = now_time+1;
      
      if( t_now_ledge[*it_prev] > t_now_tedge[*it_prev] ) t_now_tedge[*it_prev] = t_now_ledge[*it_prev] + 1;
      t2_ledge_v.push_back     ( t_now_ledge [*it_prev]       );
      t2_tedge_v.push_back     ( t_now_tedge [*it_prev]       );
      t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]       );
      t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]       );
      t2_chip_v.push_back      ( rev_ch_map_chip   (*it_prev) );
      t2_unit_v.push_back      ( rev_ch_map_unit   (*it_prev) );
      t2_bit_v.push_back       ( rev_ch_map_bit    (*it_prev) );
      t2_channel_v.push_back   ( rev_ch_map_channel(*it_prev) );
      
      t_prev_ledge[*it_prev] = t_now_ledge[*it_prev];
      t_prev_tedge[*it_prev] = t_now_tedge[*it_prev];
      
      ++it_prev;
    }
    
    it_now = fl_bin_now_state.begin();
    while( it_now !=fl_bin_now_state.end() ){
      if( fl_message ) std::cout << "         identify trailing-edge : " << *it_now << std::endl;
      
      if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ) t_now_tedge[*it_now] = now_time+1;
      else{
	++it_now;
	continue;
      }
      
      if( t_now_ledge[*it_now] > t_now_tedge[*it_now] ) t_now_tedge[*it_now] = t_now_ledge[*it_now] + 1;
      t2_ledge_v.push_back     ( t_now_ledge [*it_now]       );
      t2_tedge_v.push_back     ( t_now_tedge [*it_now]       );
      t2_prev_ledge_v.push_back( t_prev_ledge[*it_now]       );
      t2_prev_tedge_v.push_back( t_prev_tedge[*it_now]       );
      t2_chip_v.push_back      ( rev_ch_map_chip   (*it_now) );
      t2_unit_v.push_back      ( rev_ch_map_unit   (*it_now) );
      t2_bit_v.push_back       ( rev_ch_map_bit    (*it_now) );
      t2_channel_v.push_back   ( rev_ch_map_channel(*it_now) );
      
      t_prev_ledge[*it_now] = t_now_ledge[*it_now];
      t_prev_tedge[*it_now] = t_now_tedge[*it_now];
      
      ++it_now;
    }

    for( int ii = 0; ii < t2_chip_v.size(); ii++ ){
      t2_board_v.push_back ( t_board_v->at(ii) );
    }
    for( int ii = 0; ii < n_chip*n_unit; ii++ ){ // added @20170610
      t2_fl_fall_v.push_back( t_fl_fall_v->at(ii) );
    }
      newtree->Fill();
    //    if( ievt == 38 ) break;
    //std::cout << "1evt end" << std::endl;
  } // END EVENT-LOOP
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


  newtree->Write();
  rootf->Close();

  delete tree;
  //  delete newtree;
  //  delete rootf;

  return 0;

}
