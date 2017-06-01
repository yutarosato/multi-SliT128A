#ifndef MTree_H
#define MTree_H

#include <TROOT.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <arpa/inet.h> // for ntohl()

class MTree{
 public:
  MTree();
  MTree( const Char_t* name );
  ~MTree();
   
 private:
  static const int fl_message = 0; // 0(simple message), 1(normal message), 2(detailed message)
  static const int n_chip =     4;
  static const int n_unit =     4;
  static const int n_bit  =    32;
  static const int n_time =  8191; // pow(2,13)

  static const int byte_global_header = 8;
  static const int byte_unit_header   = 6;
  static const int byte_unit_data     = 6;

  // tree object
  TTree* m_tree;

  // branches  
  int t_event;
  int t_nevent_overflow;
  std::vector<int> t_board_v; // for write
  std::vector<int> t_chip_v;
  std::vector<int> t_unit_v;
  std::vector<int> t_bit_v;
  std::vector<int> t_time_v;

  int t_board;
  int t_chip;
  int t_unit;
  int t_bit;
  int t_time;
  int t_data[n_bit];

  std::vector<int>* t2_board_v; // for read
  std::vector<int>* t2_chip_v;
  std::vector<int>* t2_unit_v;
  std::vector<int>* t2_bit_v;
  std::vector<int>* t2_time_v;

 public:
  //int unit_id_mapping( int unit );
  int bit_flip ( bool bit  );
  int numofbits( int  bits );
  int ch_map         (           int unit, int bit ){ return n_bit*unit + bit; } // return Channel-No. (0-127)
  int multi_ch_map   ( int chip, int unit, int bit ){ return chip*n_unit*n_bit + n_bit*unit + bit; } // return Global Channel-No. (0-511 for 4ASIC)
  int rev_ch_map_chip( int gch ){ return (gch/(n_bit*n_unit)); } // return chip-No    from global Channel-No.
  int rev_ch_map_ch  ( int gch ){ return (gch%(n_bit*n_unit)); } // return channel-No from global Channel-No.
  

 public:
  int set_readbranch();
  int set_writebranch();
  int init_tree();
  int delete_tree();
  int decode_data(const unsigned char* event_buf, int length);

  TTree* gettree   (          ){ return m_tree;               }
  int    GetEntries(          ){ return m_tree->GetEntries(); }
  void   readevt   (int ievt=0){ m_tree->GetEntry(ievt);      }

  int getnhit    (){ return t_chip_v.size();   }
  int getevtno   (){ return t_event;           }
  int getoverflow(){ return t_nevent_overflow; }
  std::vector<int> get_board(){ return t_board_v; }
  std::vector<int> get_chip (){ return t_chip_v;  }
  std::vector<int> get_unit (){ return t_unit_v;  }
  std::vector<int> get_bit  (){ return t_bit_v;   }
  std::vector<int> get_time (){ return t_time_v;  }

};

#endif
