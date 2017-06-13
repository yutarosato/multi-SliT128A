// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef SAMPLEREADER5_H
#define SAMPLEREADER5_H

#include "DaqComponentBase.h"

#include <daqmw/Sock.h>

using namespace RTC;

class SampleReader5
  : public DAQMW::DaqComponentBase
{
public:
  SampleReader5(RTC::Manager* manager);
  ~SampleReader5();
  
  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();
  
  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);
  
private:
  TimedOctetSeq          m_out_data;
  OutPort<TimedOctetSeq> m_OutPort;
  
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
  int read_data_from_detectors();
  int set_data(unsigned int data_byte_size);
  int write_OutPort();
  
  DAQMW::Sock* m_sock;               /// socket for data server
  
  //static const int SEND_BUFFER_SIZE = (8192*8+4)*4+1024; // 262160 + 1024; 1024 is spare // for 1 ASIC (eval firmware)
  //static const int SEND_BUFFER_SIZE = (8192*8+4)*16+1024; // 1048640 + 1024; 1024 is spare // for 4 ASIC (eval firmware)

  static const int GLOBAL_HEADER_SIZE = 8;
  static const int UNIT_HEADER_SIZE   = 6;
  static const int UNIT_DATA_SIZE     = 6;
  static const int N_CHIP             = 4;
  static const int N_UNIT             = 4;
  static const int N_TIME             = 8191; // pow(2,13)
  static const int SEND_BUFFER_SIZE =
    GLOBAL_HEADER_SIZE
    + UNIT_HEADER_SIZE*N_CHIP*N_UNIT + (N_TIME*UNIT_DATA_SIZE)*N_CHIP*N_UNIT+1024; // 786440 + 1024; 1024 is spare // for 4 ASIC (exp firmware)
  
  unsigned char m_data[SEND_BUFFER_SIZE];
  unsigned int  m_recv_byte_size;
  
  BufferStatus m_out_status;
  
  int m_srcPort;                        /// Port No. of data server
  std::string m_srcAddr;                /// IP addr. of data server
  
  bool m_debug;
};


extern "C"
{
    void SampleReader5Init(RTC::Manager* manager);
};

#endif // SAMPLEREADER5_H
