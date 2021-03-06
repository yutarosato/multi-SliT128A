// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "SampleReader.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
static const char* samplereader_spec[] =
{
    "implementation_id", "SampleReader",
    "type_name",         "SampleReader",
    "description",       "SampleReader component",
    "version",           "1.0",
    "vendor",            "Yutaro Sato, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

SampleReader::SampleReader(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_OutPort("samplereader_out", m_out_data),
      m_sock(0),
      m_recv_byte_size(0),
      m_out_status(BUF_SUCCESS),

      m_debug(!true)
{
    // Registration: InPort/OutPort/Service

    // Set OutPort buffers
    registerOutPort("samplereader_out", m_OutPort);

    init_command_port();
    init_state_table();
    set_comp_name("SAMPLEREADER");
}

SampleReader::~SampleReader()
{
}

RTC::ReturnCode_t SampleReader::onInitialize()
{
    if (m_debug) {
        std::cerr << "SampleReader::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t SampleReader::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int SampleReader::daq_dummy()
{
    return 0;
}

int SampleReader::daq_configure()
{
    std::cerr << "*** SampleReader::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);

    return 0;
}

int SampleReader::parse_params(::NVList* list)
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

int SampleReader::daq_unconfigure()
{
    std::cerr << "*** SampleReader::unconfigure" << std::endl;

    return 0;
}

int SampleReader::daq_start()
{
    std::cerr << "*** SampleReader::start" << std::endl;

    m_out_status = BUF_SUCCESS;

    try {
        // Create socket and connect to data server.
        m_sock = new DAQMW::Sock();
        m_sock->connect(m_srcAddr, m_srcPort);
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

int SampleReader::daq_stop()
{
    std::cerr << "*** SampleReader::stop" << std::endl;

    if (m_sock) {
        m_sock->disconnect();
        delete m_sock;
        m_sock = 0;
    }

    return 0;
}

int SampleReader::daq_pause()
{
    std::cerr << "*** SampleReader::pause" << std::endl;

    return 0;
}

int SampleReader::daq_resume()
{
    std::cerr << "*** SampleReader::resume" << std::endl;

    return 0;
}

int SampleReader::read_data_from_detectors()
{
  /// write your logic here
  static bool first_read = true;
  static unsigned char tmpbuf[12];
  int event_data_len = 0;
  unsigned short* event_number_short;
  unsigned short* current_event_number_short;
  int event_number;
  static int current_event_number;
  //unsigned short* event_number = (unsigned short*)&mydata[2];
  
  if( first_read ){ // first time: read from file. after second time stored in tmpbuf
    int status = m_sock->readAll(&m_data[0], 3*DATA_STEP);
    if( status == DAQMW::Sock::ERROR_FATAL ){
      std::cerr << "### ERROR: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
      std::cerr << "### Timeout: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }else{
      event_data_len += 3*DATA_STEP;
      current_event_number_short = (unsigned short*)&m_data[2];
      current_event_number       = ntohs(*current_event_number_short);
      event_number               = current_event_number;
    }
    first_read = false;
    /*
    std::cerr << std::endl << "FIRST_READ" << std::endl; // tmppppp
    std::cout << (bool)((unsigned char)(m_data[0] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[0] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[0] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[0] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[1] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[1] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[1] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[1] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[2] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[2] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[2] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[2] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[3] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[3] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[3] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[3] & 0x01)) << " : " << std::endl;

    std::cout << (bool)((unsigned char)(m_data[4] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[4] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[4] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[4] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[5] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[5] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[5] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[5] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[6] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[6] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[6] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[6] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[7] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[7] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[7] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[7] & 0x01)) << " : " << std::endl;

    std::cout << (bool)((unsigned char)(m_data[8] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[8] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[8] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[8] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[9] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[9] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[9] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[9] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[10] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[10] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[10] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[10] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[11] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[11] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[11] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[11] & 0x01)) << " : " << std::endl;
    std::cout << "event_data_len = " << event_data_len << std::endl;
    std::cout << std::endl;
    */

  }else{
    memcpy( m_data, tmpbuf, 3*DATA_STEP );
    event_data_len += 3*DATA_STEP;
    //std::cerr << std::endl << "SECOND FIRST_RESD" << std::endl; // tmppppp
  }
  
  while(1){
    int status = m_sock->readAll(&m_data[event_data_len], 2*DATA_STEP);
    /*
    std::cout << "+++++++++++++ READ 8 byte " << std::endl;
    std::cout << (bool)((unsigned char)(m_data[event_data_len+0] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+0] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+0] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+0] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+1] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+1] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+1] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+1] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+2] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+2] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+2] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+2] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+3] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+3] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+3] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+3] & 0x01)) << " : " << std::endl;

    std::cout << (bool)((unsigned char)(m_data[event_data_len+4] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+4] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+4] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+4] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+5] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+5] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+5] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+5] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+6] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+6] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+6] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+6] & 0x01)) << " : "
	      << (bool)((unsigned char)(m_data[event_data_len+7] & 0x08)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+7] & 0x04)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+7] & 0x02)) << " "
	      << (bool)((unsigned char)(m_data[event_data_len+7] & 0x01)) << " : " << std::endl;
    std::cout << std::endl;
    */
    
    if( status == DAQMW::Sock::ERROR_FATAL ){
      std::cerr << "### ERROR: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
      std::cerr << "### Timeout: m_sock->readAll" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }
    /*
    unsigned char* tmp_unit0 = (unsigned char*)&m_data[event_data_len];
    unsigned char* tmp_unit1 = (unsigned char*)&m_data[event_data_len];
    int tmp_num0            = ntohs(*tmp_unit0);
    int tmp_num1            = ntohs(*tmp_unit1);
    std::cout << "tmp_num0 = " << tmp_num0 << std::endl;
    std::cout << "tmp_num1 = " << tmp_num1 << std::endl;
    */


    if( (m_data[event_data_len] & 0x80) == 0x00 ){ // read more 4 byte, because it is 12 byte header.
      event_number_short   = (unsigned short*)&m_data[event_data_len+2];
      event_number         = ntohs(*event_number_short);

      int status = m_sock->readAll(&m_data[event_data_len+2*DATA_STEP], DATA_STEP);
      if( status == DAQMW::Sock::ERROR_FATAL ){
	std::cerr << "### ERROR: m_sock->readAll" << std::endl;
	fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
      }else if( status == DAQMW::Sock::ERROR_TIMEOUT ){
	std::cerr << "### Timeout: m_sock->readAll" << std::endl;
	fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
      }
      if( event_number!=current_event_number ){ // end of the event
	memcpy( tmpbuf, &m_data[event_data_len], 3*DATA_STEP );
	current_event_number = event_number;
	return event_data_len;	  
      }else{ // continue the event
	event_data_len += 3*DATA_STEP;
	//std::cerr << "   ===> continue events(HEADER) : " << event_data_len << std::endl; // tmppp
      }
    }else{ // continue the eventn
      event_data_len += 2*DATA_STEP;
      //std::cerr << "   ===> continue events(NOT HEADER) : " << event_data_len << std::endl; // tmppp
    }
  }
}

int SampleReader::set_data(unsigned int data_byte_size)
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

int SampleReader::write_OutPort()
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

int SampleReader::daq_run()
{
    if( m_debug ) std::cerr << "*** SampleReader::run" << std::endl;

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
    void SampleReaderInit(RTC::Manager* manager)
    {
        RTC::Properties profile(samplereader_spec);
        manager->registerFactory(profile,
                    RTC::Create<SampleReader>,
                    RTC::Delete<SampleReader>);
    }
};
