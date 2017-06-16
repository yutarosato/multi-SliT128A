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


const int fl_message = 1; // 0(only #event), 1(only global header), 2(global header + unit header), 3(detailed message)
const int n_chip =     4;
const int n_unit =     4;
const int n_bit  =    32;
const int n_time =  8191; // pow(2,13)
//const int chip_id[n_chip] = {0,1,2,4};

const int byte_global_header = 8;
const int byte_unit_header   = 6;
const int byte_unit_data     = 6;

int cnt_warning = 0;
int cnt_event   = 0;
int cnt_hit     = 0; // used for judgement of the endpoint of s-curve

TTree* tree;
int t_event_number;
int t_nevent_overflow;
int t_board;
int t_chip;
int t_unit;
int t_time;
int t_data[n_bit];

std::vector<int> t_board_v;
std::vector<int> t_chip_v;
std::vector<int> t_unit_v;
std::vector<int> t_bit_v;
std::vector<int> t_time_v;
std::vector<int> t_fl_fall_v;   // added @20170610

std::vector<int> t_fall_board_v;
std::vector<int> t_fall_chip_v;
std::vector<int> t_fall_unit_v;
std::vector<int> t_fall_time_v;

// for beam test
float t_vref2    = -999;
float t_vref5    = -999;
float t_hv       = -999;
int   t_rf       = -999;
float t_rf_freq  = -999;
float t_rf_power = -999;
//

int numofbits( int bits ){
  bits = (bits & 0x55555555) + (bits >>  1 & 0x55555555);
  bits = (bits & 0x33333333) + (bits >>  2 & 0x33333333);
  bits = (bits & 0x0f0f0f0f) + (bits >>  4 & 0x0f0f0f0f);
  bits = (bits & 0x00ff00ff) + (bits >>  8 & 0x00ff00ff);
  return bits = (bits & 0x0000ffff) + (bits >> 16 & 0x0000ffff);
}

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

  // added @20170606
  if( n < nbytes ){
    return -1;
  }

  return n;
}

int fill_event_buf( FILE *fp, unsigned char *event_buf ){ // 0(correctly read one event), -1(end event (or fread err))
  int n;
  int event_data_len = 0;
  const int max_buf = n_chip*n_unit*(byte_unit_header + byte_unit_data*8191)+byte_global_header; // = 786440
  static unsigned char tmpbuf[max_buf];

  // read global header
  n = read_n_bytes( fp, tmpbuf, byte_global_header );
  if( n <= 0 ) return -1;
  memcpy( &event_buf[event_data_len], tmpbuf, byte_global_header );
  event_data_len += byte_global_header;
  /*
  // mark "e"
  unsigned char global_mark = event_buf[0];
  global_mark = ( global_mark >> 4 );
  printf("************************************************Mark : %x\n",(int)global_mark);
  // flag for bit fall
  unsigned char bit_fall = event_buf[1];
  bit_fall = ( bit_fall >> 7 );
  printf("bit-fall : %x\n",(int)bit_fall);

  // over-flow event counter
  unsigned char nevent_overflow = event_buf[1];
  nevent_overflow = ( nevent_overflow & 0x7F);
  nevent_overflow = ( nevent_overflow >> 2 );
  printf("over-flow : %x\n",(int)nevent_overflow);
  */
  // ndata 
  unsigned long tmp_total_ndata = *(unsigned long*)&event_buf[1];
  tmp_total_ndata = ( tmp_total_ndata & 0xffffff01 );
  unsigned long total_ndata = ntohl( tmp_total_ndata );
  total_ndata = ( total_ndata >> 8 );
  //printf("# of total unit data (only unit data) : %x (%d)\n",total_ndata,total_ndata);
  /*
  // event number
  unsigned short* tmp_event_number = (unsigned short*)&event_buf[4];
  int event_number = ntohs(*tmp_event_number);
  t_event_number = event_number;
  printf("event number : %d\n", event_number);

  // unit enable
  unsigned short* tmp_unit_enable = (unsigned short*)&event_buf[6];
  int unit_enable = ntohs(*tmp_unit_enable);
  printf("unit enable : %x\n", unit_enable);
  */

  // read unit header+data
  int nread = byte_unit_header*n_chip*n_unit + byte_unit_data*total_ndata;
  //std::cout << "total data length (global header + unit header + unit data) = " << byte_unit_header*n_chip*n_unit + byte_unit_data*total_ndata << std::endl;
  n = read_n_bytes( fp, &event_buf[event_data_len], nread );
  if( n <= 0 ) return -1;
  event_data_len += nread;

  return 0;
}

int set_tree(){
  tree = new TTree("slit128A","slit128A");
  tree->Branch( "event",    &t_event_number,    "event/I"    );
  tree->Branch( "overflow", &t_nevent_overflow, "overflow/I" );
  tree->Branch( "board",    &t_board_v    );
  tree->Branch( "chip",     &t_chip_v     );
  tree->Branch( "unit",     &t_unit_v     );
  tree->Branch( "bit",      &t_bit_v      );
  tree->Branch( "time",     &t_time_v     );
  tree->Branch( "fl_fall",  &t_fl_fall_v  );
  // for beam test
  tree->Branch( "rf",        &t_rf,      "rf/I"        );
  tree->Branch( "vref_b2",   &t_vref2,   "vref_b2/F"   );
  tree->Branch( "vref_b5",   &t_vref5,   "vref_b5/F"   );
  tree->Branch( "hv",        &t_hv,      "hv/F"        );
  tree->Branch( "rf_freq",   &t_rf_freq, "rf_freq/F"   );
  tree->Branch( "rf_power",  &t_rf_power, "rf_power/F" );
  // bit fall
  tree->Branch( "fall_board", &t_fall_board_v );
  tree->Branch( "fall_chip",  &t_fall_chip_v  );
  tree->Branch( "fall_unit",  &t_fall_unit_v  );
  tree->Branch( "fall_time",  &t_fall_time_v  );
  //
  return 0;
}

int init_tree(){
  for( int ibit=0; ibit<n_bit; ibit++ ) t_data[ibit] = 0;
  t_board_v.clear();
  t_chip_v.clear();
  t_unit_v.clear();
  t_bit_v.clear();
  t_time_v.clear();
  t_fl_fall_v.clear();
  t_fall_board_v.clear();
  t_fall_chip_v.clear();
  t_fall_unit_v.clear();
  t_fall_time_v.clear();

  return 0;
}

int delete_tree(){
  //tree;
  return 0;
}

int bit_flip( bool bit ){
  if( bit ) return false;
  else      return true;
}



int decode( unsigned char *event_buf, int length ){

  // mark "e"
  //unsigned char global_mark = event_buf[0];
  //global_mark = ( global_mark >> 4 );
  //printf("Mark : %x\n",(int)global_mark);

  // board-ID
  unsigned char header_board_id = event_buf[0];
  header_board_id = ( header_board_id & 0x0f );
  //printf("Board-ID : %d\n",(int)header_board_id);

  // flag for bit fall
  unsigned char bit_fall = event_buf[1];
  bit_fall = ( bit_fall >> 7 );
  //printf("bit-fall : %x\n",(int)bit_fall);

  // over-flow event counter
  unsigned char nevent_overflow = event_buf[1];
  nevent_overflow = ( nevent_overflow & 0x7F);
  nevent_overflow = ( nevent_overflow >> 2 );
  t_nevent_overflow = nevent_overflow;
  //printf("over-flow : %x\n",(int)nevent_overflow);

  // ndata 
  unsigned long tmp_total_ndata = *(unsigned long*)&event_buf[1];
  tmp_total_ndata = ((tmp_total_ndata & 0xffffff01)  );
  unsigned long total_ndata = ntohl(tmp_total_ndata);
  total_ndata = (total_ndata >> 8);
  //printf("# of total unit data (only unit data) : %x (%d)\n",total_ndata,total_ndata);  

  // event number
  unsigned short* tmp_event_number = (unsigned short*)&event_buf[4];
  int event_number = ntohs(*tmp_event_number);
  t_event_number = event_number;
  //printf("event number : %d\n", event_number);

  // unit enable
  unsigned short* tmp_unit_enable = (unsigned short*)&event_buf[6];
  int unit_enable = ntohs(*tmp_unit_enable);
  //printf("unit enable : %x\n", unit_enable);
  int n_active_unit = numofbits( unit_enable);
  
  if( fl_message ) printf("[Global Header] evtNo#%d,Board#%d,Ndata(%d),N_OF(%d),Bit_Fall(%d),Unit_Enb(%x=>N=%d)\n",event_number,(int)header_board_id,(int)total_ndata,(int)nevent_overflow,bit_fall,unit_enable,n_active_unit);
  /*
  if( total_ndata!=n_time*n_active_unit ){
    //fprintf( stderr, "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event_number, t_board, total_ndata,n_time*n_chip*n_unit );
    printf( "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event_number, t_board, total_ndata,n_time*n_chip*n_unit );
    cnt_warning++;
  }
  */

  int index = byte_global_header;
  for( int iunit=0; iunit<n_chip*n_unit; iunit++ ){ // iterate for unit head/data
    // unit header
    unsigned char board_id = event_buf[index+0]; board_id = ( (board_id & 0x78)>>3 );
    unsigned char chip_id  = event_buf[index+0]; chip_id  = (  chip_id  & 0x07     );
    unsigned char unit_id  = event_buf[index+1];
    t_board = (int)board_id;
    t_chip  = (int)chip_id;
    t_unit  = (int)unit_id;
    //printf("board#%d, chip#%d, unit#%d\n",(int)board_id,(int)chip_id,(int)unit_id);
    
    // ndata 
    unsigned short tmp_unit_ndata = *(unsigned short*)&event_buf[index+2];
    tmp_unit_ndata = ((tmp_unit_ndata & 0xff3f)  );
    unsigned short unit_ndata = ntohs(tmp_unit_ndata);
    //printf("# of unit unit data (only unit data) : %x (%d)\n",unit_ndata,unit_ndata);

    // bit-fall
    unsigned char unit_bit_fall = event_buf[index+2];
    unit_bit_fall = ( unit_bit_fall & 0x80 );
    unit_bit_fall = ( unit_bit_fall >> 7 );
    t_fl_fall_v.push_back( (int)unit_bit_fall );
    
    // event number
    unsigned short* tmp_event_number_unit = (unsigned short*)&event_buf[index+4];
    int event_number_unit = ntohs(*tmp_event_number_unit);

    // check-sum for event number
    unsigned char cksum = event_buf[index+2];
    cksum = ( cksum & 0x40 );
    cksum = ( cksum >> 6 );

    //printf("event number(cksum) : %d(%d)\n", event_number_unit,cksum);
    /*
    unsigned char mark1 = event_buf[index+0];
    mark1 = ( mark1 & 0x80 );
    mark1 = ( mark1 >> 7 );

    unsigned char mark2 = event_buf[index+2];
    mark2 = ( mark2 & 0x80 );
    mark2 = ( mark2 >> 7 );
    */

    bool fl_active = ( (unit_enable & (0x0001 << iunit))>> iunit );
    if( fl_message>1 ) printf("   [Unit Header%d] Board#%d,Chip#%d,Unit#%d,Ndata(%d),Bit_Fall(%d),evtNo#%d(cksum=%d), active(%d)\n",iunit,board_id,chip_id,unit_id,unit_ndata,unit_bit_fall,event_number_unit,cksum,(int)fl_active);

    if( (int)cksum==0 && fl_active ){
      //fprintf( stderr,"      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header)\n", event_number_unit, t_board, t_chip, t_unit, event_number );
      printf( "      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header)\n", event_number_unit, t_board, t_chip, t_unit, event_number );
      cnt_warning++;
    }
    /*
    if( unit_ndata!=n_time && fl_active ){
      //fprintf( stderr, "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
      printf( "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
      cnt_warning++;
    }
    */

    // unit data for each unit
    for( int idata=0; idata<unit_ndata; idata++ ){
      unsigned short* tmp_time = (unsigned short*)&event_buf[index+byte_unit_header+idata*byte_unit_data+0];
      unsigned long*  tmp_data = (unsigned long* )&event_buf[index+byte_unit_header+idata*byte_unit_data+2];
      unsigned short  time     = ntohs(*tmp_time);
      unsigned short  data     = ntohl(*tmp_data);
      t_time = time;
      if( fl_message > 2 ) printf( "%4d(t=%4d) : ",idata,time );

      for( int ibyte=0; ibyte<4; ibyte++ ){ // for-loop from large ch number to small ch number
	unsigned char byte_data = event_buf[index+byte_unit_header+idata*byte_unit_data+2+ibyte];
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
	
	for( int i=0; i<8; i++ ){
	  if( t_data[i+(3-ibyte)*8] ){ // modified for ch-map correction @20161004
	    t_board_v.push_back(t_board);
	    t_chip_v.push_back(t_chip);
	    t_unit_v.push_back(t_unit);
	    t_bit_v.push_back (i+(3-ibyte)*8); // modified for ch-map correction @20161004
	    t_time_v.push_back(t_time);
	  }
	}
      }

    }
    
    index += byte_unit_header+byte_unit_data*unit_ndata;
  }

  tree->Fill();
  cnt_hit += t_time_v.size();
  cnt_event ++;
  init_tree();

  return 0;
}

int main( int argc, char *argv[] ){
  const int max_buf = n_chip*n_unit*(byte_unit_header + byte_unit_data*8191)+byte_global_header; // = 786440
  unsigned char event_buf[max_buf];
  int n;
  
  FILE *fp;
  char *data_filename;
  char *out_filename;

  if( argc<3 ){
    std::cerr << "Usage : "
	      << argv[0] << "  "
	      << "output_rootfile  input_binary_file  (RF) (VREF2) (VREF5)  (HV) (RF_FREQ) (RF_POWER)" << std::endl; // for beam test
    abort();    
  }else{
    out_filename  = argv[1];
    data_filename = argv[2];
    fp = fopen( data_filename, "r" );
    if( fp == NULL ) err( EXIT_FAILURE, "fopen" );
    // for beam test
    t_rf       = ( argc>3 ? atoi(argv[3]) : -999 );
    t_vref2    = ( argc>4 ? atof(argv[4]) : -999 );
    t_vref5    = ( argc>5 ? atof(argv[5]) : -999 );
    t_hv       = ( argc>6 ? atof(argv[6]) : -999 );
    t_rf_freq  = ( argc>7 ? atof(argv[7]) : -999 );
    t_rf_power = ( argc>8 ? atof(argv[8]) : -999 );
  }

  // open output file and define tree's branches
  TFile outfile( out_filename, "RECREATE" );
  set_tree();
  init_tree();

  int tmp_cnt = 0;
  while( 1 ){
    if( tmp_cnt % 100 == 0 ) std::cout << " " << tmp_cnt << " event..." << std::endl; 
    n = fill_event_buf( fp, event_buf ); // read one event
    if( n < 0 ) break;
    else        decode( event_buf, n ); // save it in tree
    tmp_cnt++;
    //    if( tmp_cnt > 19 ) break;
  }

  std::cout << "#event = "   << cnt_event   << ", "
	    << "#hit = "     << cnt_hit     << ", "
	    << "#warning = " << cnt_warning
	    << std::endl;
  std::ofstream outlog("tmp_nhit.log");
  outlog << cnt_hit << std::endl;
  
  tree->Write();
  outfile.Close();

  delete_tree();

  if( cnt_hit ) return 0;
  else          return 1; // no hit !
  //return 0;
}
