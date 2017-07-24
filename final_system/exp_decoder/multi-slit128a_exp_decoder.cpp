#include <cstdio>
#include <fstream>
#include <iostream>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <set>

#include <arpa/inet.h>

#include <TROOT.h>
#include <TApplication.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TRandom.h>
#include <TGraph.h>


const Int_t  fl_message      = 0; // 0(only #event), 1(only global header), 2(global header + unit header), 3(detailed message)
Bool_t fl_bitfall_info       = true;
Bool_t fl_edge_decoder       = true;

Bool_t fl_scurve             = false;
const Int_t  n_chip          =     4;
const Int_t  n_unit          =     4;
const Int_t  n_bit           =    32;
const Int_t  n_time          =  8191; // pow(2,13)
//const Int_t chip_id[n_chip] = {0,1,2,4};

const Int_t byte_global_header = 8;
const Int_t byte_unit_header   = 6;
const Int_t byte_unit_data     = 6;

Int_t cnt_warning = 0;
Int_t cnt_event   = 0;

Char_t* data_filename;
Char_t* out_filename;
TTree*  tree;

Int_t t_event_number;
Int_t t_nevent_overflow;
Int_t t_board;
Int_t t_chip;
Int_t t_unit;
Int_t t_time;
Int_t t_data[n_bit];

Int_t t_now_ledge  [n_chip*n_unit*n_bit] = {0};
Int_t t_now_tedge  [n_chip*n_unit*n_bit] = {0};
Int_t t_prev_ledge [n_chip*n_unit*n_bit] = {0};
Int_t t_prev_tedge [n_chip*n_unit*n_bit] = {0};

// normal-decoder
std::vector<Int_t> t_board_v;
std::vector<Int_t> t_chip_v;
std::vector<Int_t> t_unit_v;
std::vector<Int_t> t_bit_v;
std::vector<Int_t> t_time_v;
std::vector<Int_t> t_channel_v;
// edge-decoder
std::vector<Int_t> t2_board_v;
std::vector<Int_t> t2_chip_v;
std::vector<Int_t> t2_unit_v;
std::vector<Int_t> t2_bit_v;
std::vector<Int_t> t2_time_v;
std::vector<Int_t> t2_channel_v;
std::vector<Int_t> t2_ledge_v;
std::vector<Int_t> t2_tedge_v;
std::vector<Int_t> t2_prev_ledge_v;
std::vector<Int_t> t2_prev_tedge_v;
// bit-fall information
std::vector<Int_t> t_board_bitfall_v;
std::vector<Int_t> t_chip_bitfall_v;
std::vector<Int_t> t_unit_bitfall_v;
std::vector<Int_t> t_time_bitfall_v;

// s-curve information
Float_t t_tpchg           = -999;
Int_t   t_dac             = -999;
Int_t   t_tpchip          = -999;
Int_t   t_tpchip_cycle    = -999;
Int_t   t_tpchannel       = -999;
Int_t   t_tpchannel_cycle = -999;
std::vector<Int_t> t_tpboard_v;
std::vector<Int_t> t_vref_v;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline Int_t lchannel_map (             Int_t unit, Int_t bit ){ return                     n_bit*unit + bit; } // return Local  Channel-No. (0-127)
inline Int_t gchannel_map ( Int_t chip, Int_t unit, Int_t bit ){ return chip*n_unit*n_bit + n_bit*unit + bit; } // return Global Channel-No. (0-511 for 4 ASIC)
inline Int_t gchannel2chip    ( Int_t gch ){ return (gch/(n_bit*n_unit));   } // return chip-No    from global Channel-No.
inline Int_t gchannel2unit    ( Int_t gch ){ return ((gch/n_bit)%n_unit);   } // return unit-No    from global Channel-No.
inline Int_t gchannel2bit     ( Int_t gch ){ return gch%n_bit;              } // return bit-No     from global Channel-No.
inline Int_t gchannel2lchannel( Int_t gch ){ return (gch%(n_bit*n_unit));   } // return channel-No from global Channel-No.

Int_t numofbits( Int_t bits ){
  bits = (bits & 0x55555555) + (bits >>  1 & 0x55555555);
  bits = (bits & 0x33333333) + (bits >>  2 & 0x33333333);
  bits = (bits & 0x0f0f0f0f) + (bits >>  4 & 0x0f0f0f0f);
  bits = (bits & 0x00ff00ff) + (bits >>  8 & 0x00ff00ff);
  return bits = (bits & 0x0000ffff) + (bits >> 16 & 0x0000ffff);
}

Int_t read_n_bytes( FILE* fp, UChar_t* buf, Int_t nbytes ){ // -1(fread err), 0(end file), +n(correctly read n-byte)
  Int_t n;
  n = fread( buf, 1, nbytes, fp );
  if( n==0 ){
    if( ferror(fp) ){
      warn("fread error");
      return -1;
    }
    if( feof(fp) ) return 0;
  }

  // added @20170606
  if( n < nbytes ){
    return -1;
  }

  return n;
}

Int_t fill_event_buf( FILE* fp, UChar_t* event_buf ){ // 0(correctly read one event), -1(end event (or fread err))
  Int_t n;
  Int_t event_data_len = 0;
  const Int_t max_buf = n_chip*n_unit*(byte_unit_header + byte_unit_data*8191)+byte_global_header; // = 786440
  static UChar_t tmpbuf[max_buf];

  // read global header
  n = read_n_bytes( fp, tmpbuf, byte_global_header );
  if( n <= 0 ) return -1;
  memcpy( &event_buf[event_data_len], tmpbuf, byte_global_header );
  event_data_len += byte_global_header;

  // mark "e"
  UChar_t global_mark = event_buf[0];
  global_mark = ( global_mark >> 4 );
  //printf("************************************************Mark : %x\n",(Int_t)global_mark);
  if( (Int_t)global_mark != 0xe ){
    fprintf( stderr,"      [Warning] Wrong Mark : cnt_event=%d : %s\n", cnt_event, data_filename );
    printf(         "      [Warning] Wrong Mark : cnt_event=%d : %s\n", cnt_event, data_filename );
    cnt_warning++;
    return -1;
  }

  /*
  // flag for bit fall
  UChar_t bit_fall = event_buf[1];
  bit_fall = ( bit_fall >> 7 );
  printf("bit-fall : %x\n",(Int_t)bit_fall);
  */
  /*
  // over-flow event counter
  UChar_t nevent_overflow = event_buf[1];
  nevent_overflow = ( nevent_overflow & 0x7F);
  nevent_overflow = ( nevent_overflow >> 2 );
  printf("over-flow : %x\n",(Int_t)nevent_overflow);
  */
  // ndata 
  ULong_t tmp_total_ndata = *(ULong_t*)&event_buf[1];
  tmp_total_ndata = ( tmp_total_ndata & 0xffffff01 );
  ULong_t total_ndata = ntohl( tmp_total_ndata );
  total_ndata = ( total_ndata >> 8 );
  //printf("# of total unit data (only unit data) : %x (%d)\n",total_ndata,total_ndata);
  /*
  // event number
  UShort_t* tmp_event_number = (UShort_t*)&event_buf[4];
  Int_t event_number = ntohs(*tmp_event_number);
  t_event_number = event_number;
  printf("event number : %d\n", event_number);

  // unit enable
  UShort_t* tmp_unit_enable = (UShort_t*)&event_buf[6];
  Int_t unit_enable = ntohs(*tmp_unit_enable);
  printf("unit enable : %x\n", unit_enable);
  */

  // read unit header+data
  Int_t nread = byte_unit_header*n_chip*n_unit + byte_unit_data*total_ndata;
  //std::cout << "total data length (global header + unit header + unit data) = " << byte_unit_header*n_chip*n_unit + byte_unit_data*total_ndata << std::endl;
  n = read_n_bytes( fp, &event_buf[event_data_len], nread );
  if( n <= 0 ) return -1;
  event_data_len += nread;

  return 0;
}

Int_t set_tree(){
  tree = new TTree("slit128A","slit128A");
  tree->Branch( "event",         &t_event_number,    "event/I"    );
  tree->Branch( "overflow",      &t_nevent_overflow, "overflow/I" );

  if( fl_edge_decoder ){ // edge decoder
    tree->Branch( "board",         &t2_board_v         );
    tree->Branch( "chip",          &t2_chip_v          );
    tree->Branch( "unit",          &t2_unit_v          );
    tree->Branch( "bit",           &t2_bit_v           );
    tree->Branch( "time",          &t2_time_v          );
    tree->Branch( "channel",       &t2_channel_v       );
    tree->Branch( "ledge",         &t2_ledge_v         );
    tree->Branch( "tedge",         &t2_tedge_v         );
    tree->Branch( "prev_ledge",    &t2_prev_ledge_v    );
    tree->Branch( "prev_tedge",    &t2_prev_tedge_v    );
  }else{ // normal decoder
    tree->Branch( "board",         &t_board_v         );
    tree->Branch( "chip",          &t_chip_v          );
    tree->Branch( "unit",          &t_unit_v          );
    tree->Branch( "bit",           &t_bit_v           );
    tree->Branch( "time",          &t_time_v          );
    tree->Branch( "channel",       &t_channel_v       );
  }
  
  if( fl_bitfall_info ){ // bit-fall information
    tree->Branch( "board_bitfall",   &t_board_bitfall_v   );
    tree->Branch( "chip_bitfall",    &t_chip_bitfall_v    );
    tree->Branch( "unit_bitfall",    &t_unit_bitfall_v    );
    tree->Branch( "time_bitfall",    &t_time_bitfall_v    );
  }

  if( fl_scurve ){// for s-curve
    tree->Branch( "tpchg",           &t_tpchg,           "tpchg/F"           );
    tree->Branch( "dac",             &t_dac,             "dac/I"             );
    tree->Branch( "tpchip",          &t_tpchip,          "tpchip/I"          );
    tree->Branch( "tpchip_cycle",    &t_tpchip_cycle,    "tpchip_cycle/I"    );
    tree->Branch( "tpchannel",       &t_tpchannel,       "tpchannel/I"       );
    tree->Branch( "tpchannel_cycle", &t_tpchannel_cycle, "tpchannel_cycle/I" );
    tree->Branch( "tpboard",         &t_tpboard_v );
    tree->Branch( "vref",            &t_vref_v    );
  }

  return 0;
}

Int_t init_tree(){
  for( Int_t ibit=0; ibit<n_bit; ibit++ ) t_data[ibit] = 0;
  // normal-decoder
  t_board_v.clear();
  t_chip_v.clear();
  t_unit_v.clear();
  t_bit_v.clear();
  t_time_v.clear();
  t_channel_v.clear();
 
  if( fl_edge_decoder ){ // edge-decoder
    t2_board_v.clear();
    t2_chip_v.clear();
    t2_unit_v.clear();
    t2_bit_v.clear();
    t2_time_v.clear();
    t2_channel_v.clear();
    t2_ledge_v.clear();
    t2_tedge_v.clear();
    t2_prev_ledge_v.clear();
    t2_prev_tedge_v.clear();
  }
  
  if( fl_bitfall_info ){ // bit-fall information
    t_board_bitfall_v.clear();
    t_chip_bitfall_v.clear();
    t_unit_bitfall_v.clear();
    t_time_bitfall_v.clear();
  }

  if( fl_scurve ){ // s-curve information
  t_tpboard_v.clear();
  t_vref_v.clear();
  }

  return 0;
}

Int_t delete_tree(){
  //tree;
  return 0;
}

Int_t bit_flip( bool bit ){
  if( bit ) return false;
  else      return true;
}

void edge_decode(){
  std::set<Int_t> fl_bin_prev_state;
  std::set<Int_t> fl_bin_now_state;
  Int_t now_time  = -999;
  std::set<Int_t>::iterator it_now;
  std::set<Int_t>::iterator it_prev;
  for( Int_t ivec=0; ivec<t_unit_v.size(); ivec++ ){ // BEGIN BIT-LOOP
    Int_t gch = gchannel_map( t_chip_v.at(ivec), t_unit_v.at(ivec), t_bit_v.at(ivec) );
    if( now_time == -999 ){ // exception for first time-bin
      now_time  = t_time_v.at(ivec);
      if( fl_message>2 ) std::cout << "    [first time-bin]" << std::endl
				 << "      prev_time = now_time = " << t_time_v.at(ivec) << std::endl
				 << "        gch = " <<  gch << ", chip = " << t_chip_v.at(ivec) << ", uint = " << t_unit_v.at(ivec) << ", bit = " << t_bit_v.at(ivec) << std::endl;
      fl_bin_now_state.insert(gch);
    }else if( t_time_v.at(ivec) == now_time ){ // keep same time-bin
      if( fl_message>2 ) std::cout << "    [keep same time-bin]" << std::endl
				 << "        gch = " <<  gch << ", chip = " << t_chip_v.at(ivec) << ", uint = " << t_unit_v.at(ivec) << ", bit = " << t_bit_v.at(ivec) << std::endl;
      fl_bin_now_state.insert(gch);
    }else{ // next time bin
      if( t_time_v.at(ivec)-now_time!=1 ){ // all hits at previous time bin are indentified as trailing-edge when time bin is skipped. 
	if( fl_message>2 ) std::cout << "    [move next time-bin (far)] " << now_time << " -> " << t_time_v.at(ivec) << std::endl
				   << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
	
	it_now = fl_bin_now_state.begin();
	while( it_now !=fl_bin_now_state.end() ){
	  if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
	    if( fl_message>2 ) std::cout << "         find leading-edge : " << *it_now << std::endl;
	    t_now_ledge[*it_now] = now_time;
	  }
	  ++it_now;
	}
	
	
	it_prev = fl_bin_prev_state.begin();
	while( it_prev !=fl_bin_prev_state.end() ){
	  if( fl_message>2 ) std::cout << "         identify trailing-edge : " << *it_prev << std::endl;
	  
	  if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ) t_now_tedge[*it_prev] = now_time;
	  else                                                          t_now_tedge[*it_prev] = now_time+1;
	  
	  if( t_now_ledge[*it_prev] > t_now_tedge[*it_prev] ) t_now_tedge[*it_prev] = t_now_ledge[*it_prev] + 1;
	  t2_chip_v.push_back      ( gchannel2chip    (*it_prev) );
	  t2_unit_v.push_back      ( gchannel2unit    (*it_prev) );
	  t2_bit_v.push_back       ( gchannel2bit     (*it_prev) );
	  t2_channel_v.push_back   ( gchannel2lchannel(*it_prev) );
	  t2_ledge_v.push_back     ( t_now_ledge [*it_prev]      );
	  t2_tedge_v.push_back     ( t_now_tedge [*it_prev]      );
	  t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]      );
	  t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]      );

	  t_prev_ledge[*it_prev] = t_now_ledge[*it_prev];
	  t_prev_tedge[*it_prev] = t_now_tedge[*it_prev];
	  
	  ++it_prev;
	}
	
	it_now = fl_bin_now_state.begin();
	while( it_now !=fl_bin_now_state.end() ){
	  if( fl_message>2 ) std::cout << "         identify trailing-edge : " << *it_now << std::endl;
	  
	  if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ) t_now_tedge[*it_now] = now_time+1;
	  else{
	    ++it_now;
	    continue;
	  }
	  
	  if( t_now_ledge[*it_now] > t_now_tedge[*it_now] ) t_now_tedge[*it_now] = t_now_ledge[*it_now] + 1;
	  t2_chip_v.push_back      ( gchannel2chip    (*it_now) );
	  t2_unit_v.push_back      ( gchannel2unit    (*it_now) );
	  t2_bit_v.push_back       ( gchannel2bit     (*it_now) );
	  t2_channel_v.push_back   ( gchannel2lchannel(*it_now) );
	  t2_ledge_v.push_back     ( t_now_ledge [*it_now]       );
	  t2_tedge_v.push_back     ( t_now_tedge [*it_now]       );
	  t2_prev_ledge_v.push_back( t_prev_ledge[*it_now]       );
	  t2_prev_tedge_v.push_back( t_prev_tedge[*it_now]       );
	  
	  t_prev_ledge[*it_now] = t_now_ledge[*it_now];
	  t_prev_tedge[*it_now] = t_now_tedge[*it_now];
	  
	  ++it_now;
	}
	
	
	// clear bin_state
	fl_bin_now_state.clear();
	fl_bin_prev_state.clear();
	now_time = t_time_v.at(ivec);
	fl_bin_now_state.insert(gch);
	if( fl_message>2 ) std::cout << "        gch = " <<  gch << ", chip = " << t_chip_v.at(ivec) << ", uint = " << t_unit_v.at(ivec) << ", bit = " << t_bit_v.at(ivec) << std::endl;
	
      }else{	
	if( fl_message>2 ) std::cout << "    [move next time-bin] " << now_time << " . " << t_time_v.at(ivec) << std::endl
				     << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
	it_now = fl_bin_now_state.begin();
	while( it_now !=fl_bin_now_state.end() ){
	  if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
	    if( fl_message>2 ) std::cout << "         find leading-edge : " << *it_now << std::endl;
	    t_now_ledge[*it_now] = now_time;
	  }
	  ++it_now;
	}
	
	it_prev = fl_bin_prev_state.begin();
	while( it_prev !=fl_bin_prev_state.end() ){
	  if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ){ // trailing-edge
	    if( fl_message>2 ) std::cout << "         find trailing-edge : " << *it_prev << std::endl;
	    t_now_tedge[*it_prev] = now_time;
	    
	    t2_chip_v.push_back      ( gchannel2chip    (*it_prev) );
	    t2_unit_v.push_back      ( gchannel2unit    (*it_prev) );
	    t2_bit_v.push_back       ( gchannel2bit     (*it_prev) );
	    t2_channel_v.push_back   ( gchannel2lchannel(*it_prev) );
	    t2_ledge_v.push_back     ( t_now_ledge [*it_prev]       );
	    t2_tedge_v.push_back     ( t_now_tedge [*it_prev]       );
	    t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]       );
	    t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]       );
	    
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
	now_time = t_time_v.at(ivec);
	fl_bin_now_state.insert(gch);
	if( fl_message>2 ) std::cout << "        gch = " <<  gch << ", chip = " << t_chip_v.at(ivec) << ", uint = " << t_unit_v.at(ivec) << ", bit = " << t_bit_v.at(ivec) << std::endl;
      }
    }
  } // END BIT-LOOP
  
    // exception for last time-bin
  if( fl_message>2 ) std::cout << "    [move last+1 time-bin] " << now_time << " -> ->" << std::endl
			     << "      prev_state : " << fl_bin_prev_state.size() << ", now_state  : " << fl_bin_now_state.size()  << std::endl;
  it_now = fl_bin_now_state.begin();
  while( it_now !=fl_bin_now_state.end() ){
    if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ){ // leading-edge
      if( fl_message>2 ) std::cout << "         find leading-edge : " << *it_now << std::endl;
      t_now_ledge[*it_now] = now_time;
    }
    ++it_now;
  }
  
  it_prev = fl_bin_prev_state.begin();
  while( it_prev !=fl_bin_prev_state.end() ){
    if( fl_message>2 ) std::cout << "         identify trailing-edge : " << *it_prev << std::endl;
    
    if( fl_bin_now_state.find(*it_prev)==fl_bin_now_state.end() ) t_now_tedge[*it_prev] = now_time;
    else                                                          t_now_tedge[*it_prev] = now_time+1;
    
    if( t_now_ledge[*it_prev] > t_now_tedge[*it_prev] ) t_now_tedge[*it_prev] = t_now_ledge[*it_prev] + 1;
    t2_chip_v.push_back      ( gchannel2chip    (*it_prev) );
    t2_unit_v.push_back      ( gchannel2unit    (*it_prev) );
    t2_bit_v.push_back       ( gchannel2bit     (*it_prev) );
    t2_channel_v.push_back   ( gchannel2lchannel(*it_prev) );
    t2_ledge_v.push_back     ( t_now_ledge [*it_prev]      );
    t2_tedge_v.push_back     ( t_now_tedge [*it_prev]      );
    t2_prev_ledge_v.push_back( t_prev_ledge[*it_prev]      );
    t2_prev_tedge_v.push_back( t_prev_tedge[*it_prev]      );
    
    t_prev_ledge[*it_prev] = t_now_ledge[*it_prev];
    t_prev_tedge[*it_prev] = t_now_tedge[*it_prev];
    
    ++it_prev;
  }
  
  it_now = fl_bin_now_state.begin();
  while( it_now !=fl_bin_now_state.end() ){
    if( fl_message>2 ) std::cout << "         identify trailing-edge : " << *it_now << std::endl;
    
    if( fl_bin_prev_state.find(*it_now)==fl_bin_prev_state.end() ) t_now_tedge[*it_now] = now_time+1;
    else{
      ++it_now;
      continue;
    }
    
    if( t_now_ledge[*it_now] > t_now_tedge[*it_now] ) t_now_tedge[*it_now] = t_now_ledge[*it_now] + 1;
    t2_chip_v.push_back      ( gchannel2chip    (*it_now) );
    t2_unit_v.push_back      ( gchannel2unit    (*it_now) );
    t2_bit_v.push_back       ( gchannel2bit     (*it_now) );
    t2_channel_v.push_back   ( gchannel2lchannel(*it_now) );
    t2_ledge_v.push_back     ( t_now_ledge [*it_now]      );
    t2_tedge_v.push_back     ( t_now_tedge [*it_now]      );
    t2_prev_ledge_v.push_back( t_prev_ledge[*it_now]      );
    t2_prev_tedge_v.push_back( t_prev_tedge[*it_now]      );
    
    t_prev_ledge[*it_now] = t_now_ledge[*it_now];
    t_prev_tedge[*it_now] = t_now_tedge[*it_now];
    
    ++it_now;
  }

  for( int ii = 0; ii < t2_chip_v.size(); ii++ ){
    t2_board_v.push_back ( t_board_v.at(0) );
  }
  
  return;
}

Int_t decode( UChar_t *event_buf, Int_t length ){
  // mark "e"
  //UChar_t global_mark = event_buf[0];
  //global_mark = ( global_mark >> 4 );
  //printf("Mark : %x\n",(int)global_mark);

  // board-ID
  UChar_t header_board_id = event_buf[0];
  header_board_id = ( header_board_id & 0x0f );
  //printf("Board-ID : %d\n",(int)header_board_id);

  // flag for bit fall
  UChar_t bit_fall = event_buf[1];
  bit_fall = ( bit_fall >> 7 );
  //printf("bit-fall : %x\n",(int)bit_fall);

  // over-flow event counter
  UChar_t nevent_overflow = event_buf[1];
  nevent_overflow = ( nevent_overflow & 0x7F);
  nevent_overflow = ( nevent_overflow >> 2 );
  t_nevent_overflow = nevent_overflow;
  //printf("over-flow : %x\n",(int)nevent_overflow);

  // ndata 
  ULong_t tmp_total_ndata = *(ULong_t*)&event_buf[1];
  tmp_total_ndata = ((tmp_total_ndata & 0xffffff01)  );
  ULong_t total_ndata = ntohl(tmp_total_ndata);
  total_ndata = (total_ndata >> 8);
  //printf("# of total unit data (only unit data) : %x (%d)\n",total_ndata,total_ndata);  

  // event number
  UShort_t* tmp_event_number = (UShort_t*)&event_buf[4];
  Int_t     event_number     = ntohs(*tmp_event_number);
  t_event_number = event_number;
  //printf("event number : %d\n", event_number);

  // unit enable
  UShort_t* tmp_unit_enable = (UShort_t*)&event_buf[6];
  Int_t     unit_enable     = ntohs(*tmp_unit_enable);
  //printf("unit enable : %x\n", unit_enable);
  Int_t n_active_unit = numofbits( unit_enable);
  
  if( fl_message ) printf("[Global Header] evtNo#%d,Board#%d,Ndata(%d),N_OF(%d),Bit_Fall(%d),Unit_Enb(%x=>N=%d)\n",event_number,(int)header_board_id,(int)total_ndata,(int)nevent_overflow,bit_fall,unit_enable,n_active_unit);
  /*
  if( total_ndata!=n_time*n_active_unit ){
    //fprintf( stderr, "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event_number, t_board, total_ndata,n_time*n_chip*n_unit );
    printf( "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event_number, t_board, total_ndata,n_time*n_chip*n_unit );
    cnt_warning++;
  }
  */

  Int_t index = byte_global_header;
  for( Int_t iunit=0; iunit<n_chip*n_unit; iunit++ ){ // iterate for unit head/data
    // unit header
    UChar_t board_id = event_buf[index+0]; board_id = ( (board_id & 0x78)>>3 );
    UChar_t chip_id  = event_buf[index+0]; chip_id  = (  chip_id  & 0x07     );
    UChar_t unit_id  = event_buf[index+1];
    t_board = (int)board_id;
    t_chip  = (int)chip_id;
    t_unit  = (int)unit_id;
    //printf("board#%d, chip#%d, unit#%d\n",(int)board_id,(int)chip_id,(int)unit_id);
    
    // ndata 
    UShort_t tmp_unit_ndata = *(UShort_t*)&event_buf[index+2];
    tmp_unit_ndata = ((tmp_unit_ndata & 0xff3f)  );
    UShort_t unit_ndata = ntohs(tmp_unit_ndata);
    //printf("# of unit unit data (only unit data) : %x (%d)\n",unit_ndata,unit_ndata);

    // bit-fall
    UChar_t unit_bit_fall = event_buf[index+2];
    unit_bit_fall = ( unit_bit_fall & 0x80 );
    unit_bit_fall = ( unit_bit_fall >> 7 );
    
    // event number
    UShort_t* tmp_event_number_unit = (UShort_t*)&event_buf[index+4];
    Int_t     event_number_unit     = ntohs(*tmp_event_number_unit);

    // check-sum for event number
    UChar_t cksum = event_buf[index+2];
    cksum = ( cksum & 0x40 );
    cksum = ( cksum >> 6 );

    //printf("event number(cksum) : %d(%d)\n", event_number_unit,cksum);
    /*
    UChar_t mark1 = event_buf[index+0];
    mark1 = ( mark1 & 0x80 );
    mark1 = ( mark1 >> 7 );

    UChar_t mark2 = event_buf[index+2];
    mark2 = ( mark2 & 0x80 );
    mark2 = ( mark2 >> 7 );
    */

    bool fl_active = ( (unit_enable & (0x0001 << iunit))>> iunit );
    if( fl_message>1 ) printf("   [Unit Header%d] Board#%d,Chip#%d,Unit#%d,Ndata(%d),Bit_Fall(%d),evtNo#%d(cksum=%d), active(%d)\n",iunit,board_id,chip_id,unit_id,unit_ndata,unit_bit_fall,event_number_unit,cksum,(int)fl_active); // default

    if( (int)cksum==0 && fl_active ){
      fprintf( stderr,"      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header) : %s\n", event_number_unit, t_board, t_chip, t_unit, event_number, data_filename );
      printf(         "      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header) : %s\n", event_number_unit, t_board, t_chip, t_unit, event_number, data_filename );
      cnt_warning++;
      return -1;
    }
    /*
    if( unit_ndata!=n_time && fl_active ){
      //fprintf( stderr, "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
      printf( "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
      cnt_warning++;
    }
    */

    Int_t prev_time =-1;
    // unit data for each unit
    for( Int_t idata=0; idata<unit_ndata; idata++ ){
      UShort_t* tmp_time = (UShort_t*)&event_buf[index+byte_unit_header+idata*byte_unit_data+0];
      ULong_t*  tmp_data = (ULong_t* )&event_buf[index+byte_unit_header+idata*byte_unit_data+2];
      UShort_t  time     = ntohs(*tmp_time);
      UShort_t  data     = ntohl(*tmp_data);
      t_time = time;

      if( fl_bitfall_info && prev_time != t_time-1 ){
	for( Int_t itime=prev_time+1; itime<t_time; itime++ ){
	  t_board_bitfall_v.push_back   (t_board);
	  t_chip_bitfall_v.push_back    (t_chip );
	  t_unit_bitfall_v.push_back    (t_unit );
	  t_time_bitfall_v.push_back    (itime  );
	}
      }
      prev_time = t_time;
      
      if( fl_message > 2 ) printf( "%4d(t=%4d) : ",idata,time );

      for( Int_t ibyte=0; ibyte<4; ibyte++ ){ // for-loop from large ch number to small ch number
	UChar_t byte_data = event_buf[index+byte_unit_header+idata*byte_unit_data+2+ibyte];
	if( fl_message > 2 ) printf( " %2x(%d%d%d%d %d%d%d%d)",
				     (int)((unsigned char)(byte_data)),
				     (bool)((unsigned char)(byte_data & 0x80)),
				     (bool)((unsigned char)(byte_data & 0x40)),
				     (bool)((unsigned char)(byte_data & 0x20)),
				     (bool)((unsigned char)(byte_data & 0x10)),
				     (bool)((unsigned char)(byte_data & 0x08)),
				     (bool)((unsigned char)(byte_data & 0x04)),
				     (bool)((unsigned char)(byte_data & 0x02)),
				     (bool)((unsigned char)(byte_data & 0x01))
				     );
	if( fl_message > 2 && ibyte==3 ) std::cout << std::endl;
	
	t_data[7+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x80)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[6+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x40)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[5+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x20)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[4+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x10)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[3+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x08)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[2+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x04)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[1+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x02)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[0+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x01)) ); // bit-flip correction // modified for ch-map correction @20161004
	
	for( Int_t i=0; i<8; i++ ){
	  if( t_data[i+(3-ibyte)*8] ){ // modified for ch-map correction @20161004
	    t_board_v.push_back(t_board);
	    t_chip_v.push_back(t_chip);
	    t_unit_v.push_back(t_unit);
	    t_bit_v.push_back (i+(3-ibyte)*8); // modified for ch-map correction @20161004
	    t_time_v.push_back(t_time);
	    t_channel_v.push_back(lchannel_map(t_unit_v.at(t_unit_v.size()-1),t_bit_v.at(t_bit_v.size()-1)));
	  }
	}
      }

    }
    
    index += byte_unit_header+byte_unit_data*unit_ndata;
  }

  if( fl_edge_decoder ) edge_decode();

  tree->Fill();
  cnt_event ++;
  init_tree();

  return 0;
}

Int_t main( Int_t argc, char *argv[] ){
  const Int_t max_buf = n_chip*n_unit*(byte_unit_header + byte_unit_data*8191)+byte_global_header; // = 786440
  UChar_t event_buf[max_buf];
  Int_t n;
  FILE* fp;

  if( argc<3 ){
    std::cerr << "Usage : "
	      << argv[0] << "  "
	      << "output_rootfile  input_binary_file  (tpchg tpchip tpchip_cycle tpchannel tpchannel_cycle dac tpboard vref (tpboard vref)) ..." << std::endl; // for s-curve
    abort();    
  }
  
  out_filename  = argv[1];
  data_filename = argv[2];
  fp = fopen( data_filename, "r" );
  if( fp == NULL ) err( EXIT_FAILURE, "fopen" );

  // for s-curve
  if( argc>2 ) fl_scurve = true;
  t_tpchg           = ( argc>3 ? atof(argv[3]) : -999 );
  t_dac             = ( argc>4 ? atoi(argv[4]) : -999 );
  t_tpchip          = ( argc>5 ? atoi(argv[5]) : -999 );
  t_tpchip_cycle    = ( argc>6 ? atoi(argv[6]) : -999 );
  t_tpchannel       = ( argc>7 ? atoi(argv[7]) : -999 );
  t_tpchannel_cycle = ( argc>8 ? atoi(argv[8]) : -999 );

  if( argc>9 && argc%2==0 ){
    std::cerr << "Wrong pair of board-id and vref-value : " << argc << std::endl;
    abort();
  }
  for( Int_t iargv=9; iargv<argc; ){
    t_tpboard_v.push_back( atoi(argv[iargv  ]) );
    t_vref_v.push_back   ( atoi(argv[iargv+1]) );
    iargv += 2;
  }

  // open output file and define tree's branches
  TFile outfile( out_filename, "RECREATE" );
  set_tree();
  init_tree();
  while( 1 ){
    n = fill_event_buf( fp, event_buf ); // read one event
    if( n < 0 ) break;
    n = decode( event_buf, n ); // save it in tree
    if( n < 0 ) break;
    //if( cnt_event > 100 ) break; // tmpppp
  }

  std::cout << "#event = "   << cnt_event   << ", "
	    << "#warning = " << cnt_warning << " : "
	    << data_filename
	    << std::endl;
  
  tree->Write();
  outfile.Close();

  delete_tree();

  return 0;
}
