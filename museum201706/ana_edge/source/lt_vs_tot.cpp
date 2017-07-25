#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 0;


const int min_width = 15; // estimated minimum signal width ( for signal estimation )
const int read_chips= 3;
const int max_width = 200; // for histogram
const int max_hit   = 200; // for histogram
const int max_cluster = 30;
const int count_threshold = 10;
const double b2t = 5e-3; // bit to time (5 ns)
const int before_prompt_time = 2000; // bit time before prompt
const int muon_time = 2280; // bit time muon coming
const int noise_start_time = 1000;

const double slope[ 4 ] = { 25.6 / 34.77 * 38.157, 35 / 29.41*27.098, 30, 30}; // tot (us) to charge (fC) factor
//const double slope[ 4 ] = { 25.6, 35 , 30, 30}; // tot (us) to charge (fC) factor
const double intercept[ 4 ] = {1.2, 1.5, 0, 0};
using namespace std;

Int_t main( Int_t argc, Char_t** argv ){

  TStyle* sty = Style();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( argc < 2 ){
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << argv[ 0 ]
	      << " (char*)infilename" 
	      <<  std::endl
	      << std::endl;

    return 0;
  }

  TChain* chain = new TChain("slit128A");

  for( int ii = 1; ii < argc; ii++ ){
    Char_t* infilename = argv[ ii ];
    chain->Add( infilename );
    //    printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );
  }

  set_readbranch(chain);
  

  vector< bool > bad_strip_flag( n_channel*read_chips );

  fstream fbadstrip;

  fbadstrip.open("/home/nishimura/doctor/MuSEUM/201702/ana/bad_strip_list.txt", ios::in);

  if( !fbadstrip.is_open() ){ 
    cout << "fail to open file...." << endl;
    return 1;
  }

  while( 1 ){
    double tmp_chip,tmp_channel;
    fbadstrip >> tmp_chip >> tmp_channel;
    if( fbadstrip.eof() ) break;
    bad_strip_flag[ tmp_channel + tmp_chip * n_channel ] = true;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  
  //--------  histogram --------
  TH2D* h_charge     [ n_chip ][ n_unit ];
  TH2D* h_lttot      [ n_chip ][ n_unit ];
  TH2D* h_nlttot      [ n_chip ][ n_unit ];
  TH2D* h_tot = new TH2D( "h_tot", "h_tot", n_channel*read_chips, 0, n_channel*read_chips, n_time, 0, n_time*b2t );  
  for( int ii = 0; ii < read_chips; ii++ ){
    for( int jj = 0; jj < n_unit; jj++ ){
      h_charge [ ii ][ jj ] = new TH2D( Form("h_charge_c%du%d"   ,ii,jj), Form("h_charge_c%du%d"   ,ii,jj), n_time, 0, n_time*b2t, 600, 0, 600*b2t*slope[ ii ]);
      h_lttot  [ ii ][ jj ] = new TH2D( Form("h_lttot_c%dch%d"   ,ii,jj), Form("h_lttot_c%dch%d"   ,ii,jj), n_time, 0, n_time*b2t, 1000, 0, 1000*b2t);
      h_nlttot [ ii ][ jj ] = new TH2D( Form("h_nlttot_c%dch%d"  ,ii,jj), Form("h_nlttot_c%dch%d"  ,ii,jj), n_time, 0, n_time*b2t, 1000, 0, 1000*b2t);
    }
  }


  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  int n_pulse = 0;
  int total_n_sig = 0;
  int total_n_pos = 0;
  Float_t o_vref_b2;
  Float_t o_vref_b5;
  int n_event = chain->GetEntries();



  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    
    int n_hit = t_ledge_v->size();

    o_vref_b2 = t_vref_b2;
    o_vref_b5 = t_vref_b5;
    
    //-- Initialize --
    int width = 0;
    int n_sig = 0;
    int n_pos = 0;
    //----------------
    
    for( Int_t ivec=0; ivec < n_hit; ivec++ ){
      
      width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );

      if( width > min_width ){
	n_sig++;
	if( t_ledge_v->at( ivec ) > muon_time ) n_pos++;
      }
    }
    
    if( n_sig > count_threshold ){
      n_pulse++ ;
      total_n_sig += n_sig;
      total_n_pos += n_pos;

      for( Int_t ivec=0; ivec < n_hit; ivec++ ){
	width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );
	if( !bad_strip_flag[ t_channel_v->at( ivec ) + t_chip_v->at( ivec )*n_channel ] && 
	    t_ledge_v->at( ivec ) > muon_time
	    ) {
	  h_lttot[ t_chip_v->at( ivec ) ][ t_unit_v->at( ivec ) ]
	    ->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5, width * b2t + 1e-5 );

	  h_charge[ t_chip_v->at( ivec ) ][ t_unit_v->at( ivec ) ]
	    ->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5, width * b2t * slope[ t_chip_v->at( ivec ) ] + intercept[ t_chip_v->at( ivec ) ] + 1e-5 );

	  h_tot->Fill( t_channel_v->at( ivec ) + t_chip_v->at( ivec )*n_channel, width * b2t + 1e-5 );
	}
      }
    } else{
      for( Int_t ivec=0; ivec < n_hit; ivec++ ){
	width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );
	if( !bad_strip_flag[ t_channel_v->at( ivec ) + t_chip_v->at( ivec )*n_channel ]) 
	  h_nlttot[ t_chip_v->at( ivec ) ][ t_unit_v->at( ivec ) ]
	    ->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5, width * b2t + 1e-5 );
      }

    }

  } // End of event loop
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  cout << n_event << " " << n_pulse << " "
       << o_vref_b2 << " " << o_vref_b5 << endl;

  TFile* file = new TFile( Form("time.root"),"RECREATE" );

  h_tot->Write();
  for( int ii = 0; ii < 2; ii++ ){
    for( int jj = 0; jj < n_unit; jj++ ){
      h_charge    [ ii ][ jj ]->Write();
      h_lttot    [ ii ][ jj ]->Write();
      h_nlttot   [ ii ][ jj ]->Write();
    }
  }
  file->Close();
  //  std::cout << "finish" << std::endl;

  //  if( !gROOT->IsBatch() ) app.Run();

  return 0;

}
