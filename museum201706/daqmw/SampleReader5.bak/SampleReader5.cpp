// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "SampleReader5.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
static const char* samplereader5_spec[] =
{
    "implementation_id", "SampleReader5",
    "type_name",         "SampleReader5",
    "description",       "SampleReader5 component",
    "version",           "1.0",
    "vendor",            "Yutaro Sato, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

SampleReader5::SampleReader5(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_OutPort("samplereader5_out", m_out_data),
      m_sock(0),
      m_recv_byte_size(0),
      m_out_status(BUF_SUCCESS),

      m_debug(!true)
{
    // Registration: InPort/OutPort/Service

    // Set OutPort buffers
    registerOutPort("samplereader5_out", m_OutPort);

    init_command_port();
    init_state_table();
    set_comp_name("SAMPLEREADER5");
}

SampleReader5::~SampleReader5()
{
}

RTC::ReturnCode_t SampleReader5::onInitialize()
{
    if (m_debug) {
        std::cerr << "SampleReader5::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t SampleReader5::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int SampleReader5::daq_dummy()
{
    return 0;
}

int SampleReader5::daq_configure()
{
    std::cerr << "*** SampleReader5::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);

    return 0;
}

int SampleReader5::parse_params(::NVList* list)
{
    bool srcAddrSpecified = false;
    bool srcPortSpecified = false;

    std::cerr << "param list length:" << (*list).length() << std::endl;

    int len = (*list).length();
    for (int i = 0; i < len; i+=2) {
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i+1].value;

        std::cerr << "sname: " << sname << "  ";
        std::cerr << "value: " << svalue << std::endl;

        if ( sname == "srcAddr" ) {
            srcAddrSpecified = true;
            if (m_debug) {
                std::cerr << "source addr: " << svalue << std::endl;
            }
            m_srcAddr = svalue;
        }
        if ( sname == "srcPort" ) {
            srcPortSpecified = true;
            if (m_debug) {
                std::cerr << "source port: " << svalue << std::endl;
            }
            char* offset;
            m_srcPort = (int)strtol(svalue.c_str(), &offset, 10);
        }

    }
    if (!srcAddrSpecified) {
        std::cerr << "### ERROR:data source address not specified\n";
        fatal_error_report(USER_DEFINED_ERROR1, "NO SRC ADDRESS");
    }
    if (!srcPortSpecified) {
        std::cerr << "### ERROR:data source port not specified\n";
        fatal_error_report(USER_DEFINED_ERROR2, "NO SRC PORT");
    }

    return 0;
}

int SampleReader5::daq_unconfigure()
{
    std::cerr << "*** SampleReader5::unconfigure" << std::endl;

    return 0;
}

int SampleReader5::daq_start()
{
    std::cerr << "*** SampleReader5::start" << std::endl;

    m_out_status = BUF_SUCCESS;

    try {
        // Create socket and connect to data server.
        m_sock = new DAQMW::Sock();
	m_sock->connect(m_srcAddr, m_srcPort);
	m_sock->setOptRecvTimeOut(10.0); // sec
    } catch (DAQMW::SockException& e) {
        std::cerr << "Sock Fatal Error : " << e.what() << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } catch (...) {
        std::cerr << "Sock Fatal Error : Unknown" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }

    // Check data port connections
    bool outport_conn = check_dataPort_connections( m_OutPort );
    if (!outport_conn) {
        std::cerr << "### NO Connection" << std::endl;
        fatal_error_report(DATAPATH_DISCONNECTED);
    }

    return 0;
}

int SampleReader5::daq_stop()
{
    std::cerr << "*** SampleReader5::stop" << std::endl;

    if (m_sock) {
        m_sock->disconnect();
        delete m_sock;
        m_sock = 0;
    }

    return 0;
}

int SampleReader5::daq_pause()
{
    std::cerr << "*** SampleReader5::pause" << std::endl;

    return 0;
}

int SampleReader5::daq_resume()
{
    std::cerr << "*** SampleReader5::resume" << std::endl;

    return 0;
}

int SampleReader5::read_data_from_detectors()
{
  /// write your logic here
  int event_data_len = 0;
  int status;

  status = m_sock->readAll(&m_data[0], GLOBAL_HEADER_SIZE);
  if( status == DAQMW::Sock::ERROR_FATAL ){
    std::cerr << "### ERROR: m_sock->readAll" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
  }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
    std::cerr << "### Timeout: m_sock->readAll" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
  }else{
    event_data_len += GLOBAL_HEADER_SIZE;
  }

  // ndata 
  unsigned long tmp_total_ndata = *(unsigned long*)&m_data[1];
  tmp_total_ndata = ((tmp_total_ndata & 0xffffff01)  );
  unsigned long total_ndata = ntohl(tmp_total_ndata);
  total_ndata = (total_ndata >> 8);

  // read unit header+data
  int nread = UNIT_HEADER_SIZE*N_CHIP*N_UNIT + UNIT_DATA_SIZE*total_ndata;
  /*
  status = m_sock->readAll(&m_data[event_data_len], nread);
  if( status == DAQMW::Sock::ERROR_FATAL ){
    std::cerr << "### ERROR: m_sock->readAll" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
  }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
    std::cerr << "### Timeout: m_sock->readAll" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
  }else{
    event_data_len += nread;
  }
  */

  int step = 1024;
  //std::cout << nread << " bytes" << std::endl;
  //std::cout << nread/step << " times" << std::endl;
  //std::cout << nread%step << " remain" << std::endl;
  for( int istep=0; istep <= nread/step; istep++ ){
    if     (   istep == nread/step && nread%step    ) status = m_sock->readAll(&m_data[event_data_len], nread%step);
    else if( istep == nread/step && nread%step==0   ) break;
    else                                              status = m_sock->readAll(&m_data[event_data_len], step);
    
    if( status == DAQMW::Sock::ERROR_FATAL ){
      std::cerr << "### ERROR: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
      std::cerr << "### Timeout: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }else{
      if( istep == nread/step ) event_data_len += nread%step;
      else                      event_data_len += step;
    }
  }
  
  //std::cout << "total data length (global header + unit header + unit data) = " << GLOBAL_HEADER_SIZE + nread
  //<< " : event_data_len = " << event_data_len << std::endl; // tmppppppp
  
  return event_data_len;	  
}

int SampleReader5::set_data(unsigned int data_byte_size)
{
  unsigned char header[8];
  unsigned char footer[8];
  
  set_header(&header[0], data_byte_size);
  set_footer(&footer[0]);
  
  ///set OutPort buffer length
  m_out_data.data.length(data_byte_size + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
  memcpy( &(m_out_data.data[0]                                ), &header[0], HEADER_BYTE_SIZE );
  memcpy( &(m_out_data.data[HEADER_BYTE_SIZE]                 ), &m_data[0], data_byte_size   );
  memcpy( &(m_out_data.data[HEADER_BYTE_SIZE + data_byte_size]), &footer[0], FOOTER_BYTE_SIZE );
  
  return 0;
}

int SampleReader5::write_OutPort()
{
    ////////////////// send data from OutPort  //////////////////
    bool ret = m_OutPort.write();

    //////////////////// check write status /////////////////////
    if( ret == false ){  // TIMEOUT or FATAL
        m_out_status  = check_outPort_status(m_OutPort);
        if     ( m_out_status == BUF_FATAL   ) fatal_error_report(OUTPORT_ERROR); // Fatal error
        else if( m_out_status == BUF_TIMEOUT ) return -1; // Timeout
    }else{ // successfully done
      m_out_status = BUF_SUCCESS;
    }
    
    return 0;
}

int SampleReader5::daq_run()
{
    if( m_debug ) std::cerr << "*** SampleReader5::run" << std::endl;

    if( check_trans_lock() ){ // check if stop command has come
      set_trans_unlock(); // transit to CONFIGURED state
      return 0;
    }
    
    if( m_out_status == BUF_SUCCESS ){ // previous OutPort.write() successfully done
      int ret = read_data_from_detectors();
      if( ret > 0 ){
	m_recv_byte_size = ret;
	set_data(m_recv_byte_size); // set data to OutPort Buffer
      }
    }
    
    if( write_OutPort() < 0 ){ // Timeout. do nothing.
      ;     
    }else{ // OutPort write successfully done
      inc_sequence_num();                     // increase sequence num.
      inc_total_data_size(m_recv_byte_size);  // increase total data byte size
    }
    
    return 0;
}

extern "C"
{
    void SampleReader5Init(RTC::Manager* manager)
    {
        RTC::Properties profile(samplereader5_spec);
        manager->registerFactory(profile,
                    RTC::Create<SampleReader5>,
                    RTC::Delete<SampleReader5>);
    }
};
