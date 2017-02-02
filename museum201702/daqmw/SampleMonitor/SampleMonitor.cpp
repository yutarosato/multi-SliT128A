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
      m_canvas(0),
      //m_hist_bit_1ch_1evt  (0),
      //m_hist_hit_1ch_1evt  (0),
      //m_hist_bit_1ch_int   (0),
      //m_hist_hit_1ch_int   (0),
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
      m_monitor_update_rate(100),
      m_monitor_sampling_rate(25),
      m_obs_chip(0),
      m_obs_ch(0),
      th_width(10),
      th_span(50),
      m_event_byte_size(0),
      m_nevt_success(0),
      m_nevt_fail(0),
      m_debug(!true)
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
  if (m_canvas) {
    m_canvas->Update();
    // daq_dummy() will be invoked again after 10 msec.
    // This sleep reduces X servers' load.
    sleep(1);
  }
  
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
    
    if( sname == "monitorUpdateRate" ){
      if( m_debug ){
	std::cerr << "monitor update rate: " << svalue << std::endl;
      }
      char* offset;
      m_monitor_update_rate = (int)strtol(svalue.c_str(), &offset, 10);
    }
    // If you have more param in config.xml, write here
    if( sname == "monitorSamplingRate" ){
      if( m_debug ){
	std::cerr << "monitor sampling rate: " << svalue << std::endl;
      }
      char* offset;
      m_monitor_sampling_rate = (int)strtol(svalue.c_str(), &offset, 10);
    }
    if( sname == "sel1ch" ){
      if( m_debug ){
	std::cerr << "selected channel: " << svalue << std::endl;
      }
      char *offset;
      m_obs_ch = (int)strtol(svalue.c_str(), &offset, 10);
    }
    if( sname == "sel1chip" ){
      if( m_debug ){
	std::cerr << "selected chip: " << svalue << std::endl;
      }
      char *offset;
      m_obs_chip = (int)strtol(svalue.c_str(), &offset, 10);
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
  if( m_tree ){ delete m_tree; m_tree = 0; }

  m_tree   = new MTree();
  m_canvas = new TCanvas("c1", "histos", 0, 0, 1800, 900);
  //m_canvas->Divide(4,4);
  m_canvas->Divide(4,2);
  
  int    m_hist_xbin = 8192;
  double m_hist_xmin = 0.0;
  double m_hist_xmax = 8192.0;
  
  int    m_hist_ybin = n_chip*n_unit*n_bit;
  //int    m_hist_ybin = n_chip*n_unit*n_bit/2; // tmpppp
  double m_hist_ymin = 0.0;
  double m_hist_ymax = n_chip*n_unit*n_bit;
  //double m_hist_ymax = n_chip*n_unit*n_bit/2; // tmpppp
  /*
  m_hist_bit_1ch_1evt   = new TH1I( "hist_bit_1ch_1evt",   "hist_bit_1ch_1evt",   m_hist_xbin, m_hist_xmin, m_hist_xmax );
  m_hist_hit_1ch_1evt   = new TH1I( "hist_hit_1ch_1evt",   "hist_hit_1ch_1evt",   m_hist_xbin, m_hist_xmin, m_hist_xmax );
  m_hist_bit_1ch_int    = new TH1I( "hist_bit_1ch_int",    "hist_bit_1ch_int",    m_hist_xbin, m_hist_xmin, m_hist_xmax );
  m_hist_hit_1ch_int    = new TH1I( "hist_hit_1ch_int",    "hist_hit_1ch_int",    m_hist_xbin, m_hist_xmin, m_hist_xmax );
  */
  m_hist_bit_allch_1evt = new TH2I( "hist_bit_allch_1evt", "hist_bit_allch_1evt", m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
  m_hist_hit_allch_1evt = new TH2I( "hist_hit_allch_1evt", "hist_hit_allch_1evt", m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
  m_hist_bit_allch_int  = new TH2I( "hist_bit_allch_int",  "hist_bit_allch_int",  m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );
  m_hist_hit_allch_int  = new TH2I( "hist_hit_allch_int",  "hist_hit_allch_int",  m_hist_xbin, m_hist_xmin, m_hist_xmax, m_hist_ybin, m_hist_ymin, m_hist_ymax );

  m_hist_nbit  = new TH1I( "hist_nbit",  "hist_nbit;Nbit;",                  50,         0.0,     10000.0 );
  m_hist_nhit  = new TH1I( "hist_nhit",  "hist_nhit;Nhit;",                  50,         0.0,       200.0 );
  m_hist_width = new TH1I( "hist_width", "hist_width;Width [bit];",          40,         0.0,        80.0 );
  m_hist_time  = new TH1I( "hist_time",  "hist_time;Time [bit];",            50, m_hist_xmin, m_hist_xmax );

  m_graph_nbit  = new TGraph();
  m_graph_nhit  = new TGraphErrors();
  m_graph_nbit ->SetTitle( "N_{bit};events;Bits/events"     );
  m_graph_nhit ->SetTitle( "N_{hit};events;Hits/events"     );

  m_graph_nbit ->SetMinimum(0.0);
  m_graph_nhit ->SetMinimum(0.0);
  m_graph_nhit->SetLineColor(2);
  
  m_tree->set_writebranch();
  m_tree->init_tree();

  std::cerr << "*** SampleMonitor::start2" << std::endl;
  
  return 0;
}

int SampleMonitor::daq_stop()
{
  std::cerr << "*** SampleMonitor::stop" << std::endl;
  draw_obj();
  m_canvas->Print("pic.eps");
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


int SampleMonitor::fill_data(const unsigned char* mydata, const int size)
{
  if( m_tree->decode_data(mydata, size) ){ // false
    m_nevt_fail++;
    return 1;
  }else{ // success
    m_nevt_success++;
  }

  unsigned long sequence_num = get_sequence_num();
  if( (sequence_num % m_monitor_update_rate)!=0 && (sequence_num % m_monitor_sampling_rate)!=0 ) return 0;

  double prev_nbit = m_hist_bit_allch_int->GetEntries();
  double prev_nhit = m_hist_hit_allch_int->GetEntries();

  for( Int_t ivec=0; ivec<m_tree->getnhit(); ivec++ ){ // read 1 event data
    int obs_chip = m_tree->get_chip().at(ivec);
    int time     = m_tree->get_time().at(ivec);
    int obs_ch   = m_tree->ch_map      (         m_tree->get_unit().at(ivec),m_tree->get_bit().at(ivec));
    int obs_gch  = m_tree->multi_ch_map(obs_chip,m_tree->get_unit().at(ivec),m_tree->get_bit().at(ivec));

    m_hist_bit_allch_1evt->Fill( time, obs_gch );
    m_hist_bit_allch_int ->Fill( time, obs_gch );
    /*
    if( m_obs_chip==obs_chip && m_obs_ch==obs_ch ){
      m_hist_bit_1ch_1evt->Fill( time );
      m_hist_bit_1ch_int ->Fill( time );
    }
    */
  }

  detect_signal();

  // input to graph
  m_graph_nbit ->SetPoint     ( m_graph_nbit->GetN(),    sequence_num, m_hist_bit_allch_int->GetEntries()-prev_nbit );
  m_graph_nhit ->SetPoint     ( m_graph_nhit->GetN(),    sequence_num, m_hist_hit_allch_int->GetEntries()-prev_nhit );
  m_graph_nhit ->SetPointError( m_graph_nhit->GetN()-1,           0.0, (double)(sqrt(m_hist_hit_allch_int->GetEntries()-prev_nhit)) );
  m_hist_nbit->Fill( m_hist_bit_allch_int->GetEntries()-prev_nbit );
  m_hist_nhit->Fill( m_hist_hit_allch_int->GetEntries()-prev_nhit );

  return 0;
}

int SampleMonitor::detect_signal(){
  for( int ich=0; ich<m_hist_bit_allch_1evt->GetNbinsY(); ich++ ){
    int  bin_start        = 0;
    int  bin_end          = 0;
    int  prev_bin_start   = 0;
    //int  prev_bin_end     = 0;
    bool fl_bin_state     = false;

    for( int itime=0; itime<m_hist_bit_allch_1evt->GetNbinsX(); itime++ ){
      if( m_hist_bit_allch_1evt->GetBinContent(itime+1,ich+1) && !fl_bin_state ){ // detect rising-edge
	bin_start = itime;
	fl_bin_state = true;
      }else if( !m_hist_bit_allch_1evt->GetBinContent(itime+1,ich+1) && fl_bin_state ){ // detect trailing-edge
	bin_end = itime;
	fl_bin_state = false;
	
	int width = bin_end   - bin_start;
	int span  = bin_start - prev_bin_start;
	//int span  = bin_start - prev_bin_end; // which is better ??
	
	if( ( span >= th_span || prev_bin_start == 0) && width >= th_width ){ // identified as true signal
	  m_hist_width->Fill( width     );
	  m_hist_time ->Fill( bin_start );
	  

	  m_hist_hit_allch_1evt->Fill( itime, ich );
	  m_hist_hit_allch_int ->Fill( itime, ich );
	  /*
	  if( m_obs_chip==m_tree->rev_ch_map_chip(ich) && m_obs_ch==m_tree->rev_ch_map_ch(ich) ){
	    m_hist_hit_1ch_1evt->Fill( itime );
	    m_hist_hit_1ch_int ->Fill( itime );
	  }
	  */
	  
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
  unsigned int recv_byte_size = read_InPort();
  if( recv_byte_size == 0 ) return 0;// Timeout
  
  check_header_footer(m_in_data, recv_byte_size); // check header and footer
  m_event_byte_size = get_event_size(recv_byte_size);
  if( m_event_byte_size > DATA_BUF_SIZE ) fatal_error_report(USER_DEFINED_ERROR1, "DATA BUF TOO SHORT");
  
  /////////////  Write component main logic here. /////////////
  memcpy(&m_recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], m_event_byte_size);
  if( m_monitor_update_rate   == 0 ) m_monitor_update_rate   = 100;
  if( m_monitor_sampling_rate == 0 ) m_monitor_sampling_rate =  25;
  unsigned long sequence_num = get_sequence_num();
  if( (sequence_num % m_monitor_update_rate  )==0 ) reset_obj();
  if( (sequence_num % m_monitor_sampling_rate)==0 ) reset_obj(); // tmpppppp
  fill_data(&m_recv_data[0], m_event_byte_size);
  // Draw

  if( (sequence_num % m_monitor_update_rate)==0 ){
    //m_hist_bit_1ch_int   ->SetTitle( Form("Chip%d,Channel%d, Integral of %d events;Time [bit];Bits",m_obs_chip, m_obs_ch, (int)(sequence_num/m_monitor_update_rate)+1) );
    //m_hist_hit_1ch_int   ->SetTitle( Form("Chip%d,Channel%d, Integral of %d events;Time [bit];Hits",m_obs_chip, m_obs_ch, (int)(sequence_num/m_monitor_update_rate)+1) );
    m_hist_bit_allch_1evt->SetTitle( Form("Bit (%d);Time [bit];Channel",                                                  (int)(sequence_num)                        ) );
    m_hist_hit_allch_1evt->SetTitle( Form("Hit (%d);Time [bit];Channel",                                                  (int)(sequence_num)                        ) );
    m_hist_bit_allch_int ->SetTitle( Form("Bit (Integral of %d events);Time [bit];Channel",                               (int)(sequence_num/m_monitor_update_rate)+1) );
    m_hist_hit_allch_int ->SetTitle( Form("Hit (Integral of %d events);Time [bit];Channel",                               (int)(sequence_num/m_monitor_update_rate)+1) );
    draw_obj();
  }

  /////////////////////////////////////////////////////////////
  inc_sequence_num();                      // increase sequence num.
  inc_total_data_size(m_event_byte_size);  // increase total data byte size
  
  return 0;
}

int SampleMonitor::delete_obj(){

  if( m_canvas ){ delete m_canvas; m_canvas = 0; }
  /*
  if( m_hist_bit_1ch_1evt ){ delete m_hist_bit_1ch_1evt; m_hist_bit_1ch_1evt = 0; }
  if( m_hist_hit_1ch_1evt ){ delete m_hist_hit_1ch_1evt; m_hist_hit_1ch_1evt = 0; }
  if( m_hist_bit_1ch_int  ){ delete m_hist_bit_1ch_int;  m_hist_bit_1ch_int  = 0; }
  if( m_hist_hit_1ch_int  ){ delete m_hist_hit_1ch_int;  m_hist_hit_1ch_int  = 0; }
  */
  if( m_hist_bit_allch_1evt ){ delete m_hist_bit_allch_1evt; m_hist_bit_allch_1evt = 0; }
  if( m_hist_hit_allch_1evt ){ delete m_hist_hit_allch_1evt; m_hist_hit_allch_1evt = 0; }
  if( m_hist_bit_allch_int  ){ delete m_hist_bit_allch_int;  m_hist_bit_allch_int  = 0; }
  if( m_hist_hit_allch_int  ){ delete m_hist_hit_allch_int;  m_hist_hit_allch_int  = 0; }

  if( m_hist_nbit   ){ delete m_hist_nbit;   m_hist_nbit   = 0; }
  if( m_hist_nhit   ){ delete m_hist_nhit;   m_hist_nhit   = 0; }
  if( m_hist_width  ){ delete m_hist_width;  m_hist_width  = 0; }
  if( m_hist_time   ){ delete m_hist_time;   m_hist_time   = 0; }

  if( m_graph_nbit  ){ delete m_graph_nbit;  m_graph_nbit  = 0; }
  if( m_graph_nhit  ){ delete m_graph_nhit;  m_graph_nhit  = 0; }

  return 0;
}

int SampleMonitor::draw_obj(){
  //m_canvas->cd( 1); m_hist_bit_1ch_1evt  ->Draw();
  //m_canvas->cd( 2); m_hist_hit_1ch_1evt  ->Draw();
  //m_canvas->cd( 3); m_hist_bit_1ch_int   ->Draw();
  //m_canvas->cd( 4); m_hist_hit_1ch_int   ->Draw();
  //m_canvas->cd( 5); m_hist_bit_allch_1evt->Draw("COLZ");
  //m_canvas->cd( 6); m_hist_hit_allch_1evt->Draw("COLZ");
  //m_canvas->cd( 7); m_hist_bit_allch_int ->Draw("COLZ");
  //m_canvas->cd( 8); m_hist_hit_allch_int ->Draw("COLZ");


  m_canvas->cd(1); m_hist_bit_allch_1evt->Draw("COLZ");
  m_canvas->cd(2); m_hist_bit_allch_int ->Draw("COLZ");
  m_canvas->cd(3); m_graph_nbit         ->Draw("AP");
  m_canvas->cd(4); m_graph_nhit         ->Draw("AP");
  m_canvas->cd(5)->SetLogy();
  m_canvas->cd(5); m_hist_time          ->Draw();
  m_canvas->cd(6); m_hist_width         ->Draw();
  m_canvas->cd(7); m_hist_nbit          ->Draw();
  m_canvas->cd(8); m_hist_nhit          ->Draw();


  m_canvas->Update();

  return 0;
}

int SampleMonitor::reset_obj(){
  //m_hist_bit_1ch_1evt  ->Reset();
  //m_hist_hit_1ch_1evt  ->Reset();
  m_hist_bit_allch_1evt->Reset();
  m_hist_hit_allch_1evt->Reset();
  
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
