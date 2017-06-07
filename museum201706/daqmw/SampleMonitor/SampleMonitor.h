// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef SAMPLEMONITOR_H
#define SAMPLEMONITOR_H

#include "DaqComponentBase.h"

#include <arpa/inet.h> // for ntohl()

////////// ROOT Include files //////////
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TApplication.h"
#include "TString.h"
#include "TStyle.h"

#include "../MTree/MTree.h"

#include <vector>

using namespace RTC;

class SampleMonitor
  : public DAQMW::DaqComponentBase
{
public:
  SampleMonitor(RTC::Manager* manager);
  ~SampleMonitor();
  
  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();
  
  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);
  
private:
  TimedOctetSeq          m_in_data;
  InPort<TimedOctetSeq>  m_InPort;
  
private:
  int daq_dummy();
  int daq_configure();
  int daq_unconfigure();
  int daq_start();
  int daq_run();
  int daq_stop();
  int daq_pause();
  int daq_resume();
  
  int parse_params(::NVList* list);
  int reset_InPort();
  int delete_obj();
  int draw_obj();
  int reset_obj();
  
  unsigned int read_InPort();
  //int online_analyze();

  int fill_data    (const unsigned char* event_buf, const int size);
  int detect_signal( int iboard );

  BufferStatus m_in_status;
  
  ////////// ROOT Histogram //////////

  TCanvas** m_canvas;

  TH2I** m_hist_bit_allch_1evt;
  TH2I** m_hist_hit_allch_1evt;
  TH2I** m_hist_bit_allch_int;
  TH2I** m_hist_hit_allch_int;

  TH1I** m_hist_nbit;
  TH1I** m_hist_nhit;
  TH1I** m_hist_width;
  TH1I** m_hist_time;

  TGraph**  m_graph_nbit;
  TGraph**  m_graph_nhit;
  
  // external parameters
  int      m_monitor_update_rate;
  int      m_monitor_sampling_rate;
  int      th_width; // bin
  int      th_span;  // bin

  static const int n_board =    2;
  static const int n_chip  =    4;
  static const int n_unit  =    4;
  static const int n_bit   =   32;
  static const int n_time  = 8191; // pow(2,13)
  static const int board_map[];
  static const int rev_board_map[];

  static const int byte_global_header = 8;
  static const int byte_unit_header   = 6;
  static const int byte_unit_data     = 6;

  static const int SEND_BUFFER_SIZE =
    byte_global_header
    + byte_unit_header*n_chip*n_unit + (n_time*byte_unit_data)*n_chip*n_unit+1024; // 786440 + 1024; 1024 is spare // for 4 ASIC (exp firmware)
  unsigned char m_recv_data[SEND_BUFFER_SIZE];
  unsigned int  m_event_byte_size;

  static const int fl_message = 0; // 0(simple message), 1(normal message), 2(detailed message)

  MTree* m_tree;
  bool   m_debug;
  int    m_sequence_number[n_board];
  
  int t_event;
  std::vector<int> t_chip_v; // for write
  std::vector<int> t_unit_v;
  std::vector<int> t_bit_v;
  std::vector<int> t_time_v;

  std::vector<int>* t2_chip_v; // for read
  std::vector<int>* t2_unit_v;
  std::vector<int>* t2_bit_v;
  std::vector<int>* t2_time_v;
};


extern "C"
{
  void SampleMonitorInit(RTC::Manager* manager);
};

#endif // SAMPLEMONITOR_H
