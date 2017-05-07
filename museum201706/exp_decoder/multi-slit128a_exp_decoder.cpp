#include <cstdio>
#include <fstream>
#include <iostream>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <arpa/inet.h>

#include <TROOT.h>
#include <TApplication.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TRandom.h>
#include <TGraph.h>


const int fl_message = 1; // 0(simple message), 1(normal message), 2(detailed message)
const int n_chip =     4;
const int n_unit =     4;
const int n_bit  =    32;
const int n_time =  8192; // pow(2,13)
//const int chip_id[n_chip] = {0,1,2,4};

const int byte_global_header = 8;
const int byte_unit_header   = 6;
const int byte_unit_data     = 6;

int nevt_success = 0;

int t_chip;
int t_unit;
int t_time;
int t_data[n_bit];

int cnt_data = 0; // used for judgement of the endpoint of s-curve

TTree* tree;
int t_event_number;
int t_nevent_overflow;

std::vector<int> t_chip_v;
std::vector<int> t_unit_v;
std::vector<int> t_bit_v;
std::vector<int> t_time_v;

float t_vref0  = -999;
float t_vref1  = -999;
float t_vref23 = -999;
float t_hv     = -999;
int   t_rf     = -999;

int prev_event = -999;
int prev_ndata = 0;

unsigned long long total_data_length;

int read_n_bytes( FILE *fp, unsigned char *buf, int nbytes ){ // -1(fread err), 0(end file), +n(correctly read n-byte)
  int n;
  n = fread( buf, 1, nbytes, fp );
  if( n==0 ){
    if( ferror(fp) ){
      warn("fread error");
      return -1;
    }
    if( feof(fp) ) return 0;
  }
  return n;
}

int fill_event_buf( FILE *fp, unsigned char *event_buf ){ // 0(correctly read one event), -1(end event (or fread err))
  int n;
  int event_data_len = 0;

  n = read_n_bytes( fp, &event_buf[event_data_len], byte_global_header ); // read global header
  if( n <= 0 ) return -1;
  event_data_len += byte_global_header;

  // mark "ee"
  unsigned char global_mark = event_buf[0];
  
  // over-flow event counter
  //unsigned char nevent_overflow = event_buf[1];
  //nevent_overflow = ( nevent_overflow & 0xfc );
  
  // data length
  unsigned long* tmp_total_data_length1 = (unsigned long*)&event_buf[0];
  unsigned long* tmp_total_data_length2 = (unsigned long*)&event_buf[2];
  *tmp_total_data_length1 = (*tmp_total_data_length1 & 0x3000 );
  total_data_length =  ntohs(*tmp_total_data_length1)*pow(2,16);
  total_data_length += ntohs(*tmp_total_data_length2);

  // event number
  //unsigned short* tmp_event_number = (unsigned short*)&event_buf[4];
  //int event_number = ntohs(*tmp_event_number);
  //t_event_number = event_number;

  // unit enable
  //unsigned short* unit_enable = (unsigned short*)&event_buf[6];

  int nread = byte_unit_header*n_chip*n_unit + byte_unit_data*total_data_length; // to be checked !!!!
  n = read_n_bytes( fp, &event_buf[event_data_len], nread ); // read unit header+data
  if( n <= 0 ) return -1;
  event_data_len += nread;

  return 0;
}

int set_tree(){
  tree = new TTree("slit128A","slit128A");
  tree->Branch( "event",  &t_event_number, "event/I"  );
  tree->Branch( "rf",     &t_rf,           "rf/I"     );
  tree->Branch( "chip",   &t_chip_v );
  tree->Branch( "unit",   &t_unit_v );
  tree->Branch( "bit",    &t_bit_v  );
  tree->Branch( "time",   &t_time_v );
  tree->Branch( "vref0",  &t_vref0,        "vref0/F"  );
  tree->Branch( "vref1",  &t_vref1,        "vref1/F"  );
  tree->Branch( "vref23", &t_vref23,       "vref23/F" );
  tree->Branch( "hv",     &t_hv,           "hv/F"     );

  return 0;
}

int init_tree(){
  for( int ibit=0; ibit<n_bit; ibit++ ) t_data[ibit] = 0;
  t_chip_v.clear();
  t_unit_v.clear();
  t_bit_v.clear();
  t_time_v.clear();
  
  return 0;
}

int delete_tree(){
  tree;
  return 0;
}

int bit_flip( bool bit ){
  if( bit ) return false;
  else      return true;
}



int decode( unsigned char *buf, int length ){

  // mark "ee"
  //unsigned char global_mark = buf[0];
  
  // over-flow event counter
  unsigned char* nevent_overflow = (unsigned char*)&buf[1];
  //*nevent_overflow = ( nevent_overflow & 0xfc ); // tmppppp to be comment on !!!!
  
  // data length
  //unsigned long* tmp_total_data_length1 = (unsigned long*)&buf[0];
  //unsigned long* tmp_total_data_length2 = (unsigned long*)&buf[2];
  //*tmp_total_data_length1 = (*tmp_total_data_length1 & 0x3000 );
  //total_data_length =  ntohs(*tmp_total_data_length1)*pow(2,16);
  //total_data_length += ntohs(*tmp_total_data_length2);

  // event number
  unsigned short* tmp_event_number = (unsigned short*)&buf[4];
  int event_number = ntohs(*tmp_event_number);
  t_event_number = event_number;

  // unit enable
  unsigned short* unit_enable = (unsigned short*)&buf[6];

  if( fl_message ) printf( "=>[ Event#=%d : #Data=%llu, #Over-Flow-Events=%d]\n", (int)event_number, total_data_length, (int)nevent_overflow );

  int index =0;
  while(1){ // iterate for 16 units

    // header for each unit
    unsigned char   chip_id              = buf[index+0]; chip_id = ( chip_id & 0x7f );
    unsigned char   unit_id              = buf[index+1];
    unsigned short* tmp_unit_data_length = (unsigned short*)&buf[index+2]; *tmp_unit_data_length = ( *tmp_unit_data_length & 0xff3f );
    int             unit_data_length     = ntohs(*tmp_unit_data_length);
    unsigned short* tmp_event_number     = (unsigned short*)&buf[index+4];
    int             event_number         = ntohs(*tmp_event_number);
    
    unsigned char mark1 = buf[index+0];
    mark1 = ( mark1 & 0x80 );
    mark1 = ( mark1 >> 7 );

    unsigned char mark2 = buf[index+2];
    mark2 = ( mark2 & 0x80 );
    mark2 = ( mark2 >> 7 );

    unsigned char cksum = buf[index+2];
    cksum = ( cksum & 0x40 );
    cksum = ( cksum >> 6 );

    if( fl_message > 1 ) printf( "Chip-ID=%d, Unit-ID=%d, Length=%d, Event#=%d ] \n", (int)chip_id, (int)unit_id, unit_data_length, event_number );
    
    // data for each unit
    for( int idata=0; idata<unit_data_length; idata++ ){
      unsigned short* time_info = (unsigned short*)&buf[index+byte_unit_header+idata*byte_unit_data+0];
      unsigned long*  data      = (unsigned long* )&buf[index+byte_unit_header+idata*byte_unit_data+2];
      if( fl_message > 2 ) printf( "%3d : (time=%d, data=%x)\n", idata, ntohs(*time_info), ntohl(*data) );
      t_chip = chip_id;
      t_unit = unit_id;
      t_time = ntohs(*time_info);


      for( int ibyte=0; ibyte<4; ibyte++ ){ // for-loop from large ch number to small ch number
	unsigned char byte_data = buf[index+byte_unit_header+idata*byte_unit_data+2+ibyte];
	
	if( fl_message > 1 ) std::cout << "("
				       << (int )((unsigned char)(byte_data)) << " : "
				       << (bool)((unsigned char)(byte_data & 0x80)) << " "
				       << (bool)((unsigned char)(byte_data & 0x40)) << " "
				       << (bool)((unsigned char)(byte_data & 0x20)) << " "
				       << (bool)((unsigned char)(byte_data & 0x10)) << "  "
				       << (bool)((unsigned char)(byte_data & 0x08)) << " "
				       << (bool)((unsigned char)(byte_data & 0x04)) << " "
				       << (bool)((unsigned char)(byte_data & 0x02)) << " "
				       << (bool)((unsigned char)(byte_data & 0x01)) << ") ";
	if( fl_message > 1 && ibyte==3 ) std::cout << std::endl;
	
	t_data[7+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x80)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[6+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x40)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[5+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x20)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[4+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x10)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[3+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x08)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[2+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x04)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[1+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x02)) ); // bit-flip correction // modified for ch-map correction @20161004
	t_data[0+(3-ibyte)*8] = bit_flip( (bool)((unsigned char)(byte_data & 0x01)) ); // bit-flip correction // modified for ch-map correction @20161004
	
	for( int i=0; i<8; i++ ){
	  if( t_data[i+(3-ibyte)*8] ){ // modified for ch-map correction @20161004
	    t_chip_v.push_back(t_chip);
	    t_unit_v.push_back(t_unit);
	    t_bit_v.push_back (i+(3-ibyte)*8); // modified for ch-map correction @20161004
	    t_time_v.push_back(t_time);
	  }
	}
      }

      tree->Fill();
      init_tree();
    }
  }

  return 0;
}

int main( int argc, char *argv[] ){
  unsigned char event_buf[1024*1024];
  int n;
  
  FILE *fp;
  char *data_filename;
  char *out_filename;

  if( argc<7 ){
    std::cerr << "Usage : "
	      << argv[0] << "  "
	      << "output_rootfile  input_binary_file  VREF0 VREF1 VREF23 HV" << std::endl;
    abort();    
  }else{
    out_filename = argv[1];

    data_filename = argv[2];
    fp = fopen( data_filename, "r" );
    if( fp == NULL ) err( EXIT_FAILURE, "fopen" );
    
    t_vref0  = atof(argv[3]);
    t_vref1  = atof(argv[4]);
    t_vref23 = atof(argv[5]);
    t_hv     = atof(argv[6]);
  }

  // open output file and define tree's branches
  TFile outfile( out_filename, "RECREATE" );
  set_tree();
  init_tree();
  
  while(1){
    n = fill_event_buf( fp, event_buf ); // read one event

    if( n < 0 ) break;
    else decode( event_buf, n ); // save it in tree
  }

  std::cout << "#event = "  << nevt_success << ", "
	    << "#hit = "    << cnt_data
	    << std::endl;
  //std::ofstream outlog("tmp.log");
  //outlog << cnt_data << std::endl;
  
  tree->Write();
  outfile.Close();

  delete_tree();

  if( cnt_data ) return 0;
  else           return 1; // no hit !
  //return 0;
}
