#include "MTree.h"

MTree::MTree(){
  m_tree = new TTree();
}

MTree::MTree( const Char_t* name ){
  m_tree  = new TTree(name,name);
}

MTree::~MTree(){
  delete m_tree;
  init_tree();
  t2_board_v = 0;
  t2_chip_v  = 0;
  t2_unit_v  = 0;
  t2_bit_v   = 0;
  t2_time_v  = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int MTree::bit_flip( bool bit ){
  return (int)(!bit);
}


int MTree::numofbits( int bits ){
  bits = (bits & 0x55555555) + (bits >>  1 & 0x55555555);
  bits = (bits & 0x33333333) + (bits >>  2 & 0x33333333);
  bits = (bits & 0x0f0f0f0f) + (bits >>  4 & 0x0f0f0f0f);
  bits = (bits & 0x00ff00ff) + (bits >>  8 & 0x00ff00ff);
  return bits = (bits & 0x0000ffff) + (bits >> 16 & 0x0000ffff);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int MTree::set_readbranch(){
  m_tree->SetBranchAddress("event", &t_event    );
  m_tree->SetBranchAddress("board", &t2_board_v );
  m_tree->SetBranchAddress("chip",  &t2_chip_v  );
  m_tree->SetBranchAddress("unit",  &t2_unit_v  );
  m_tree->SetBranchAddress("bit",   &t2_bit_v   );
  m_tree->SetBranchAddress("time",  &t2_time_v  );

  t2_board_v->clear();
  t2_chip_v ->clear();
  t2_unit_v ->clear();
  t2_bit_v  ->clear();
  t2_time_v ->clear();

  return 0;
}

int MTree::set_writebranch(){
  m_tree->Branch( "event",    &t_event,           "event/I"    );
  m_tree->Branch( "overflow", &t_nevent_overflow, "overflow/I" );
  m_tree->Branch( "board",    &t_board_v );
  m_tree->Branch( "chip",     &t_chip_v  );
  m_tree->Branch( "unit",     &t_unit_v  );
  m_tree->Branch( "bit",      &t_bit_v   );
  m_tree->Branch( "time",     &t_time_v  );
  
  return 0;
}

int MTree::init_tree(){
  t_board_v.clear();
  t_chip_v.clear();
  t_unit_v.clear();
  t_bit_v.clear();
  t_time_v.clear();
  m_tree->Reset();
  
  return 0;
}


int MTree::decode_data(const unsigned char* event_buf, int length)
{
  init_tree();

  // mark "ee"
  //unsigned char global_mark = event_buf[0];
  //global_mark = ( global_mark >> 4 );
  //printf("Mark : %x\n",(int)global_mark);

  // board-ID
  unsigned char header_board_id = event_buf[0];
  header_board_id = ( header_board_id >> 4 );
  //printf("Board-ID : %d\n",(int)header_board_id);
  
  // over-flow event counter
  unsigned char nevent_overflow = event_buf[1];
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
  t_event = event_number;
  //printf("event number : %d\n", event_number);

  // unit enable
  unsigned short* tmp_unit_enable = (unsigned short*)&event_buf[6];
  int unit_enable = ntohs(*tmp_unit_enable);
  printf("unit enable : %x\n", unit_enable);
  int n_active_unit = numofbits( unit_enable);
  
  if( fl_message ) printf("[Global Header] evtNo#%d,Board#%d,Ndata(%d),N_OF(%d),Unit_Enb(%x=>N=%d)\n",event_number,(int)header_board_id,total_ndata,(int)nevent_overflow,unit_enable,n_active_unit);
  if( total_ndata!=n_time*n_active_unit ){
    //fprintf( stderr, "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event, t_board, total_ndata,n_time*n_chip*n_unit );
    printf( "      [Warning] evtNo=%d, board#%d : Wrong total number of events : %d (correct value is %d)\n", t_event, t_board, total_ndata,n_time*n_chip*n_unit );
  }
  
  int index = byte_global_header;
  for( int iunit=0; iunit<n_chip*n_unit; iunit++ ){ // iterate for unit head/data
    // unit header
    unsigned char board_id = event_buf[index+0]; board_id = ( (board_id & 0x78)>>3 );
    unsigned char chip_id  = event_buf[index+0]; chip_id = ( chip_id & 0x7f );
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
    
    // event number
    unsigned short* tmp_event_number_unit = (unsigned short*)&event_buf[index+4];
    int event_number_unit = ntohs(*tmp_event_number_unit);
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
    if( fl_message ) printf("   [Unit Header%d] Board#%d,Chip#%d,Unit#%d,Ndata(%d),evtNo#%d(cksum=%d), active(%d)\n",iunit,board_id,chip_id,unit_id,unit_ndata,event_number_unit,cksum,(int)fl_active);

    if( (int)cksum==0 && fl_active ){
      //fprintf( stderr,"      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header)\n", event_number_unit, t_board, t_chip, t_unit, event_number );
      printf( "      [Warning] Event number shift : evtNo=%d(Board#%d,Chip#%d,Unit#%d) & %d(global header)\n", event_number_unit, t_chip, t_unit, event_number );
    }
    if( unit_ndata!=n_time && fl_active ){
      //fprintf( stderr, "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
      printf( "      [Warning] evtNo=%d : Wrong number of events in board#%d, chip#%d, unit#%d : %d (correct value is %d)\n", event_number, t_board,t_chip,t_unit,unit_ndata,n_time );
    }

    // unit data for each unit
    for( int idata=0; idata<unit_ndata; idata++ ){
      unsigned short* tmp_time = (unsigned short*)&event_buf[index+byte_unit_header+idata*byte_unit_data+0];
      unsigned long*  tmp_data = (unsigned long* )&event_buf[index+byte_unit_header+idata*byte_unit_data+2];
      unsigned short  time     = ntohs(*tmp_time);
      unsigned short  data     = ntohl(*tmp_data);
      t_time = time;
      if( fl_message > 1 ) printf( "%4d(t=%4d) : ",idata,time );

      for( int ibyte=0; ibyte<4; ibyte++ ){ // for-loop from large ch number to small ch number
	unsigned char byte_data = event_buf[index+byte_unit_header+idata*byte_unit_data+2+ibyte];
	if( fl_message > 1 ) printf( " %2x(%d%d%d%d %d%d%d%d)",
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


  
  m_tree->Fill();
  m_tree->GetEntry(m_tree->GetEntries()-1);

  return 0;
}
