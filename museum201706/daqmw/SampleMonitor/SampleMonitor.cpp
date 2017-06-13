// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "SampleMonitor.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

const int SampleMonitor::board_map[] = {-999,-999,0,-999,-999,1}; // Board#2 -> [0], Board#5 -> [1]
const int SampleMonitor::rev_board_map[n_board] = {2,5}; // reverse map

// Module specification
// Change following items to suit your component's spec.
static const char* samplemonitor_spec[] =
{
    "implementation_id", "SampleMonitor",
    "type_name",         "SampleMonitor",
    "description",       "SampleMonitor component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

SampleMonitor::SampleMonitor(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_InPort("samplemonitor_in",   m_in_data),
      m_in_status(BUF_SUCCESS),

      m_tree(0),
      m_canvas(0),
      m_hist_bit_allch_1evt(0),
      m_hist_hit_allch_1evt(0),
      m_hist_bit_allch_int (0),
      m_hist_hit_allch_int (0),

      m_hist_nbit          (0),
      m_hist_nhit          (0),
      m_hist_width         (0),
      m_hist_time          (0),
      m_graph_nbit         (0),
      m_graph_nhit         (0),
      m_tex_error          (0),
      m_tex_warning        (0),

      m_monitor_rate(100),
      th_width(10),
      th_span(50),
      m_event_byte_size(0),
      m_debug(!true),
      m_sequence_number()
{

  // Registration: InPort/OutPort/Service
  
  // Set InPort buffers
  registerInPort ("samplemonitor_in",  m_InPort);
  
  init_command_port();
  init_state_table();
  set_comp_name("SAMPLEMONITOR");
  TStyle* sty = new TStyle();
  sty->SetFrameFillColor(10);
  sty->SetCanvasColor(10);
  sty->SetPadColor(10);
  sty->SetStatColor(10);
  sty->SetTitleFillColor(10);
  sty->SetFrameBorderMode(0);
  sty->SetCanvasBorderMode(0);
  sty->SetPadBorderMode(0);
  sty->SetLegendBorderSize(0);
  sty->SetMarkerStyle(20);
  sty->SetMarkerSize(0.8);
  sty->SetMarkerColor(2);
  sty->SetHistLineColor(2);
  sty->SetTitleSize(0.05,"xyz");  
  sty->SetTitleOffset(1.1,"x");  
  sty->SetTitleOffset(1.3,"y");  
  sty->SetLabelSize(0.04,"xyz");  
  sty->SetPadBottomMargin(0.13);
  sty->SetPadLeftMargin(0.13);
  sty->SetPadRightMargin(0.13);
  sty->SetPalette(1,0);

  sty->cd();

  m_hist_bit_allch_1evt = new TH2I*   [n_board];
  m_hist_hit_allch_1evt = new TH2I*   [n_board];
  m_hist_bit_allch_int  = new TH2I*   [n_board];
  m_hist_hit_allch_int  = new TH2I*   [n_board];
  m_hist_nbit           = new TH1I*   [n_board];
  m_hist_nhit           = new TH1I*   [n_board];
  m_hist_width          = new TH1I*   [n_board];
  m_hist_time           = new TH1I*   [n_board];
  m_graph_nbit          = new TGraph* [n_board];
  m_graph_nhit          = new TGraph* [n_board];
  for( int iboard=0; iboard<n_board; iboard++ ){
    m_hist_bit_allch_1evt[iboard] = 0;
    m_hist_hit_allch_1evt[iboard] = 0;
    m_hist_bit_allch_int [iboard] = 0;
    m_hist_hit_allch_int [iboard] = 0;
    m_hist_nbit          [iboard] = 0;
    m_hist_nhit          [iboard] = 0;
    m_hist_width         [iboard] = 0;
    m_hist_time          [iboard] = 0;
    m_graph_nbit         [iboard] = 0;
    m_graph_nhit         [iboard] = 0;
  }

}

SampleMonitor::~SampleMonitor()
{
}

RTC::ReturnCode_t SampleMonitor::onInitialize()
{
  if (m_debug) {
    std::cerr << "SampleMonitor::onInitialize()" << std::endl;
  }
  
  return RTC::RTC_OK;
}

RTC::ReturnCode_t SampleMonitor::onExecute(RTC::UniqueId ec_id)
{
  daq_do();
  
  return RTC::RTC_OK;
}

int SampleMonitor::daq_dummy()
{

  if( m_canvas ){
    m_canvas->Update();
  }
  // daq_dummy() will be invoked again after 10 msec.
  // This sleep reduces X servers' load.
  sleep(1);

  
  return 0;
}

int SampleMonitor::daq_configure()
{
  std::cerr << "*** SampleMonitor::configure" << std::endl;
  
  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);
  
  return 0;
}

int SampleMonitor::parse_params(::NVList* list)
{
  
  std::cerr << "param list length:" << (*list).length() << std::endl;
  
  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;
    
    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;
    
    if( sname == "monitorRate" ){
      if( m_debug ){
	std::cerr << "monitor rate: " << svalue << std::endl;
      }
      char* offset;
      m_monitor_rate = (int)strtol(svalue.c_str(), &offset, 10);
    }

    if( sname == "signal_width" ){
      if( m_debug ){
	std::cerr << "minimum signal width: " << svalue << " [bin]" << std::endl;
      }
      char *offset;
      th_width = (int)strtol(svalue.c_str(), &offset, 10);
    }
    if( sname == "signal_span" ){
      if( m_debug ){
	std::cerr << "minimum signal span: " << svalue << " [bin]" << std::endl;
      }
      char *offset;
      th_span = (int)strtol(svalue.c_str(), &offset, 10);
    }
  }
  
  return 0;
}

int SampleMonitor::daq_unconfigure()
{
  std::cerr << "*** SampleMonitor::unconfigure" << std::endl;

  delete_obj();


  return 0;
}

int SampleMonitor::daq_start()
{
  std::cerr << "*** SampleMonitor::start" << std::endl;
  
  m_in_status  = BUF_SUCCESS;
  delete_obj();

  if( !m_tree ) m_tree = new MTree();
  else          m_tree->Reset(); // tmppppp

  
  m_canvas = new TCanvas( "c1", "c1", 0, 0, 1800, 1050);
  m_canvas->Divide(4,4);
  m_canvas->Draw();


  int    m_hist_xbin = 8192;
  double m_hist_xmin = 0.0;
  double m_hist_xmax = 8192.0;
  
  int    m_hist_ybin = n_chip*n_unit*n_bit;
  double m_hist_ymin = 0.0;
  double m_hist_ymax = n_chip*n_unit*n_bit;


  for( int iboard=0; iboard<n_board; iboard++ ){
    m_hist_bit_allch_1evt[iboard] = new TH2I( Form("hist_bit_allch_1evt_board%d",rev_board_map[iboard]), Form("hist_bit_allch_1evt_board%d",rev_board_map[iboard]), m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
    m_hist_hit_allch_1evt[iboard] = new TH2I( Form("hist_hit_allch_1evt_board%d",rev_board_map[iboard]), Form("hist_hit_allch_1evt_board%d",rev_board_map[iboard]), m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
    m_hist_bit_allch_int [iboard] = new TH2I( Form("hist_bit_allch_int_board%d", rev_board_map[iboard]), Form("hist_bit_allch_int_board%d", rev_board_map[iboard]), m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
    m_hist_hit_allch_int [iboard] = new TH2I( Form("hist_hit_allch_int_board%d", rev_board_map[iboard]), Form("hist_hit_allch_int_board%d", rev_board_map[iboard]), m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );

    m_hist_nbit [iboard] = new TH1I( Form("hist_nbit_board%d", rev_board_map[iboard]),  Form("N_{bit} (board#%d);Nbit;",     rev_board_map[iboard]),  50,         0.0,     10000.0 );
    m_hist_nhit [iboard] = new TH1I( Form("hist_nhit_board%d", rev_board_map[iboard]),  Form("N_{hit} (board#%d);Nhit;",     rev_board_map[iboard]),  40,         0.0,        80.0 );
    m_hist_width[iboard] = new TH1I( Form("hist_width_board%d",rev_board_map[iboard]),  Form("Width (board#%d);Width [bit];",rev_board_map[iboard]),  60,         0.0,       120.0 );
    m_hist_time [iboard] = new TH1I( Form("hist_time_board%d", rev_board_map[iboard]),  Form("Time (board#%d);Time [bit];",  rev_board_map[iboard]),  50, m_hist_xmin, m_hist_xmax );
    //m_hist_time [iboard] = new TH1I( Form("hist_time_board%d", rev_board_map[iboard]),  Form("Time (board#%d);Time [bit];",  rev_board_map[iboard]), 100,        2000,        6000 ); // tmppp
    
    m_graph_nbit[iboard] = new TGraph();
    m_graph_nhit[iboard] = new TGraph();
    m_graph_nbit[iboard]->SetTitle( Form("N_{bit} (board#%d);events;Bits/events",rev_board_map[iboard])     );
    m_graph_nhit[iboard]->SetTitle( Form("N_{hit} (board#%d);events;Hits/events",rev_board_map[iboard])     );
    
    m_graph_nbit[iboard]->SetMinimum(0.0);
    m_graph_nhit[iboard]->SetMinimum(0.0);
    m_graph_nhit[iboard]->SetLineColor(2);
  }

  m_tree->set_writebranch();
  m_tree->init_tree();

  if( !m_tex_error ){
    m_tex_error   = new TText();
    m_tex_error  ->SetTextColor(2);
    m_tex_error  ->SetTextSize(0.08);
  }
  if( !m_tex_warning ){
    m_tex_warning = new TText();
    m_tex_warning->SetTextColor(4);
    m_tex_warning->SetTextSize(0.08);
  }
  
  for( int iboard=0; iboard<n_board; iboard++ ){
    m_cksum   [iboard] = 1;
    m_fl_fall [iboard] = 0;
    m_overflow[iboard] = 0;
  }
  return 0;
}

int SampleMonitor::daq_stop()
{
  std::cerr << "*** SampleMonitor::stop" << std::endl;

  draw_obj();

  unsigned int runNumber = m_daq_service0.getRunNo();
  
  //m_canvas->Print( Form("monitor_pic/run%d.eps", runNumber) );
  m_canvas->Print( Form("monitor_pic/run%d.png", runNumber) );

  reset_InPort();
  
  return 0;
}

int SampleMonitor::daq_pause()
{
  std::cerr << "*** SampleMonitor::pause" << std::endl;
  
  return 0;
}

int SampleMonitor::daq_resume()
{
  std::cerr << "*** SampleMonitor::resume" << std::endl;
  
  return 0;
}

int SampleMonitor::reset_InPort()
{
  int ret = true;
  while(ret == true) {
    ret = m_InPort.read();
  }
  
  return 0;
}


int SampleMonitor::fill_data(const unsigned char* event_buf, const int size)
{

  if( m_tree->decode_data(event_buf, size) ){
    std::cerr << "[ERROR] NOT succeeded in decoding" << std::endl;
    return 1;
  }

  double prev_nbit[n_board];
  double prev_nhit[n_board];
  for( int iboard=0; iboard<n_board; iboard++ ){
    prev_nbit[iboard] = m_hist_bit_allch_int[iboard]->GetEntries();
    prev_nhit[iboard] = m_hist_hit_allch_int[iboard]->GetEntries();
  }

  int board_id = 0;

  for( Int_t ivec=0; ivec<m_tree->getnhit(); ivec++ ){ // read 1 event data
        board_id       = m_tree->get_board().at(ivec);
    int chip_id        = m_tree->get_chip ().at(ivec);
    int unit_id        = m_tree->get_unit ().at(ivec);
    int time           = m_tree->get_time ().at(ivec);
    int global_channel = m_tree->global_channel_map( chip_id,unit_id,m_tree->get_bit().at(ivec) );
    //std::cout << "gl = " << global_channel << ", t = " << time << ", chip = " << chip_id << ", unit = " << unit_id << ", board = " << board_id << std::endl;
    if( board_id!=2 && board_id!=5 ){ // tmppppp
      std::cerr << "[WARNING] Wrong board_id : " << board_id << std::endl;
      continue;
    }

    m_hist_bit_allch_1evt[board_map[board_id]]->Fill( time, global_channel );
    m_hist_bit_allch_int [board_map[board_id]]->Fill( time, global_channel );
  }

  //detect_signal(board_map[board_id]); // tmppppp

  // input to graph
  m_graph_nbit[board_map[board_id]]->SetPoint     ( m_graph_nbit[board_map[board_id]]->GetN(),  m_sequence_number[board_map[board_id]], m_hist_bit_allch_int[board_map[board_id]]->GetEntries()-prev_nbit[board_map[board_id]] );
  m_graph_nhit[board_map[board_id]]->SetPoint     ( m_graph_nhit[board_map[board_id]]->GetN(),  m_sequence_number[board_map[board_id]], m_hist_hit_allch_int[board_map[board_id]]->GetEntries()-prev_nhit[board_map[board_id]] );
  m_hist_nbit [board_map[board_id]]->Fill( m_hist_bit_allch_int[board_map[board_id]]->GetEntries()-prev_nbit[board_map[board_id]] );
  m_hist_nhit [board_map[board_id]]->Fill( m_hist_hit_allch_int[board_map[board_id]]->GetEntries()-prev_nhit[board_map[board_id]] );

  return 0;
}


int SampleMonitor::detect_signal( int iboard ){
  for( int ich=0; ich<m_hist_bit_allch_1evt[iboard]->GetNbinsY(); ich++ ){

    int  bin_start        = 0;
    int  bin_end          = 0;
    int  prev_bin_start   = 0;
    //int  prev_bin_end     = 0;
    bool fl_bin_state     = false;

    for( int itime=0; itime<m_hist_bit_allch_1evt[iboard]->GetNbinsX(); itime++ ){
      if( m_hist_bit_allch_1evt[iboard]->GetBinContent(itime+1,ich+1) && !fl_bin_state ){ // detect rising-edge
	bin_start = itime;
	fl_bin_state = true;
      }else if( !m_hist_bit_allch_1evt[iboard]->GetBinContent(itime+1,ich+1) && fl_bin_state ){ // detect trailing-edge
	bin_end = itime;
	fl_bin_state = false;
	
	int width = bin_end   - bin_start;
	int span  = bin_start - prev_bin_start;
	//int span  = bin_start - prev_bin_end; // which is better ??
	
	if( ( span >= th_span || prev_bin_start == 0) && width >= th_width ){ // identified as true signal
	  m_hist_width[iboard]->Fill( width     );
	  m_hist_time [iboard]->Fill( bin_start );

	  m_hist_hit_allch_1evt[iboard]->Fill( itime, ich );
	  m_hist_hit_allch_int [iboard]->Fill( itime, ich );
	  
	  // for next hits
	  prev_bin_start = bin_start;
	  //prev_bin_end   = bin_end;
	}else{ // identified as ringing
	  ;
	}
      }
    }
  }

  return 0;
}

unsigned int SampleMonitor::read_InPort()
{
    /////////////// read data from InPort Buffer ///////////////
    unsigned int recv_byte_size = 0;
    bool ret = m_InPort.read();

    //////////////////// check read status /////////////////////
    if (ret == false) { // false: TIMEOUT or FATAL
        m_in_status = check_inPort_status(m_InPort);
        if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
            if (check_trans_lock()) {     // Check if stop command has come.
                set_trans_unlock();       // Transit to CONFIGURE state.
            }
        }
        else if (m_in_status == BUF_FATAL) { // Fatal error
            fatal_error_report(INPORT_ERROR);
        }
    }
    else {
        recv_byte_size = m_in_data.data.length();
    }

    if (m_debug) {
        std::cerr << "m_in_data.data.length():" << recv_byte_size
                  << std::endl;
    }

    return recv_byte_size;
}

int SampleMonitor::daq_run()
{
  if( m_debug ) std::cerr << "*** SampleMonitor::run" << std::endl;
  reset_obj();
  unsigned int recv_byte_size = read_InPort();
  if( recv_byte_size == 0 ) return 0;// Timeout

  check_header_footer(m_in_data, recv_byte_size); // check header and footer
  m_event_byte_size = get_event_size(recv_byte_size);
  if( m_event_byte_size > SEND_BUFFER_SIZE ) fatal_error_report(USER_DEFINED_ERROR1, "DATA BUF TOO SHORT");


  /////////////  Write component main logic here. /////////////
  //unsigned long sequence_num = get_sequence_num();
  if( m_monitor_rate == 0 ) m_monitor_rate =  100;

  unsigned char header_board_id = m_in_data.data[HEADER_BYTE_SIZE];
  header_board_id = ( header_board_id & 0x0f );
  int board_id    = (int)header_board_id;

  // mark "ee"
  unsigned char global_mark = m_in_data.data[HEADER_BYTE_SIZE];
  global_mark = ( global_mark >> 4 );
  //printf("Mark : %x\n",(int)global_mark);

  if( board_id!=2 && board_id!=5 ){ // tmppppp
    std::cerr << "[WARNING] Wrong board_id : " << board_id << std::endl;
    return 0;
  }

  m_sequence_number[board_map[board_id]]++;
  /*
  std::cout << "m_sequence_number = "
	    << m_sequence_number[0] << ", "
	    << m_sequence_number[1] << std::endl;
  */
  if( ( m_sequence_number[board_map[board_id]]%m_monitor_rate != 0 )
      || 
      (
       ( (m_sequence_number[board_map[board_id]]/m_monitor_rate)%(n_board+1) != board_map[board_id] ) // sampling
       &&
       ( board_id!=rev_board_map[0] || (m_sequence_number[0]/m_monitor_rate)%(n_board+1) != n_board ) // draw update       
       )
      ){
    inc_sequence_num();                      // increase sequence num.
    inc_total_data_size(m_event_byte_size);  // increase total data byte size
    return 0;
  }
  /*
  std::cout << "sampling" << std::endl;
  std::cout << (m_sequence_number[board_map[board_id]]/m_monitor_rate)%(n_board+1) << " : " <<  board_map[board_id] << std::endl;
  std::cout << (m_sequence_number[0]/m_monitor_rate)%(n_board+1) << " : " << n_board << std::endl;
  */
  memcpy(&m_recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], m_event_byte_size);
  reset_obj();

  fill_data( &m_recv_data[0], m_event_byte_size );

  m_cksum   [board_map[board_id]] = m_tree->getcksum();
  m_fl_fall [board_map[board_id]] = m_tree->getfl_fall();
  m_overflow[board_map[board_id]] = m_tree->getoverflow();
  

  // Draw
  if( board_id==rev_board_map[0] &&  (m_sequence_number[0]/m_monitor_rate)%(n_board+1) == n_board ){
    std::cout << "draw" << std::endl;
    for( int iboard=0; iboard<n_board; iboard++ ){
      m_hist_bit_allch_1evt[iboard]->SetTitle( Form("Bit (%d events, board#%d);Time [bit];Channel", m_sequence_number[iboard],rev_board_map[iboard] ) );
      m_hist_hit_allch_1evt[iboard]->SetTitle( Form("Hit (%d events, board#%d);Time [bit];Channel", m_sequence_number[iboard],rev_board_map[iboard] ) );
      m_hist_bit_allch_int [iboard]->SetTitle( Form("Bit (Integral of %d events, board#%d);Time [bit];Channel", (int)(m_sequence_number[iboard]/m_monitor_rate),rev_board_map[iboard]) );
      m_hist_hit_allch_int [iboard]->SetTitle( Form("Hit (Integral of %d events, board#%d);Time [bit];Channel", (int)(m_sequence_number[iboard]/m_monitor_rate),rev_board_map[iboard]) );
    }
    draw_obj();
    m_canvas->cd(1);
    for( int iboard=0; iboard<n_board; iboard++ ){
      if( m_cksum   [iboard]==0 ) m_tex_error->DrawTextNDC( 0.15, 0.92-iboard*0.05, Form("[ ERROR :event number shift] board#%d",board_id) );
      if( m_fl_fall [iboard]==1 ) m_tex_error->DrawTextNDC( 0.15, 0.82-iboard*0.05, Form("[ ERROR :    fall-bit      ] board#%d",board_id) );
      if( m_overflow[iboard]    ) m_tex_error->DrawTextNDC( 0.15, 0.72-iboard*0.05, Form("[ ERROR :    over-flow     ] board#%d",board_id) );
    }
  }
  
  std::cout << std::endl;
  
  
  /////////////////////////////////////////////////////////////
  inc_sequence_num();                      // increase sequence num.
  inc_total_data_size(m_event_byte_size);  // increase total data byte size
  
  return 0;
}

int SampleMonitor::delete_obj(){
  if( m_canvas ){ delete m_canvas; m_canvas = 0; }
  for( int iboard=0; iboard<n_board; iboard++ ){
    m_sequence_number[iboard] = 0;

    if( m_hist_bit_allch_1evt[iboard] ){ delete m_hist_bit_allch_1evt[iboard]; m_hist_bit_allch_1evt[iboard] = 0; }
    if( m_hist_hit_allch_1evt[iboard] ){ delete m_hist_hit_allch_1evt[iboard]; m_hist_hit_allch_1evt[iboard] = 0; }
    if( m_hist_bit_allch_int [iboard] ){ delete m_hist_bit_allch_int [iboard]; m_hist_bit_allch_int [iboard] = 0; }
    if( m_hist_hit_allch_int [iboard] ){ delete m_hist_hit_allch_int [iboard]; m_hist_hit_allch_int [iboard] = 0; }
    if( m_hist_nbit          [iboard] ){ delete m_hist_nbit          [iboard]; m_hist_nbit          [iboard] = 0; }
    if( m_hist_nhit          [iboard] ){ delete m_hist_nhit          [iboard]; m_hist_nhit          [iboard] = 0; }
    if( m_hist_width         [iboard] ){ delete m_hist_width         [iboard]; m_hist_width         [iboard] = 0; }
    if( m_hist_time          [iboard] ){ delete m_hist_time          [iboard]; m_hist_time          [iboard] = 0; }
    if( m_graph_nbit         [iboard] ){ delete m_graph_nbit         [iboard]; m_graph_nbit         [iboard] = 0; }
    if( m_graph_nhit         [iboard] ){ delete m_graph_nhit         [iboard]; m_graph_nhit         [iboard] = 0; }
  }

  delete m_tex_error;
  delete m_tex_warning;

  return 0;
}

int SampleMonitor::draw_obj(){
  const int n_plot = 8;
  for( int iboard=0; iboard<n_board; iboard++ ){
    m_canvas->cd(1+n_plot*iboard); m_hist_bit_allch_1evt[iboard]->Draw("COLZ");
    m_canvas->cd(2+n_plot*iboard); m_hist_bit_allch_int [iboard]->Draw("COLZ");
    m_canvas->cd(3+n_plot*iboard); if( m_graph_nbit[iboard]->GetN() ){ m_graph_nbit[iboard]->Draw("AP"); }
    m_canvas->cd(4+n_plot*iboard); if( m_graph_nhit[iboard]->GetN() ){ m_graph_nhit[iboard]->Draw("AP"); }
    m_canvas->cd(5+n_plot*iboard)->SetLogy();
    m_canvas->cd(5+n_plot*iboard); m_hist_time [iboard] ->Draw();
    m_canvas->cd(6+n_plot*iboard); m_hist_width[iboard] ->Draw();
    m_canvas->cd(7+n_plot*iboard); m_hist_nbit [iboard] ->Draw();
    m_canvas->cd(8+n_plot*iboard); m_hist_nhit [iboard] ->Draw();
  }
    m_canvas->Update();


  return 0;
}


int SampleMonitor::reset_obj(){
  for( int iboard=0; iboard<n_board; iboard++ ){
    m_hist_bit_allch_1evt[iboard]->Reset();
    m_hist_hit_allch_1evt[iboard]->Reset();
  }
  
  return 0;
}

extern "C"
{
  void SampleMonitorInit(RTC::Manager* manager)
  {
    RTC::Properties profile(samplemonitor_spec);
    manager->registerFactory(profile,
			     RTC::Create<SampleMonitor>,
			     RTC::Delete<SampleMonitor>);
  }
};
