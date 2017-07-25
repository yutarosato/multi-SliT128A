#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 0;


const int min_width = 15; // estimated minimum signal width ( for signal estimation )

const int max_width = 200; // for histogram
const int max_hit   = 200; // for histogram
const int max_cluster = 30;
const int count_threshold = 10;
const double b2t = 5e-3; // bit to time (5 ns)
const int before_prompt_time = 2000; // bit time before prompt
const int muon_time = 2280; // bit time muon coming
const int noise_start_time = 1000;

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
  

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  
  //--------  histogram --------

  TH1D* h_time1       = new TH1D("h_time"   ,"h_time"  , n_time, 0, n_time*b2t);
  h_time1       -> Sumw2();

  TH1D* h_time       [ n_chip ][ n_channel ];
  TH1D* h_noise      [ n_chip ][ n_channel ];
  TH1D* h_count      [ n_chip ][ n_channel ];
  TH1D* h_width      [ n_chip ][ n_channel ];
  
  for( int ii = 0; ii < n_chip; ii++ ){
    for( int jj = 0; jj < n_channel; jj++ ){
      h_time    [ ii ][ jj ] = new TH1D( Form("h_time_c%dch%d"   ,ii,jj), Form("h_time_c%dch%d"   ,ii,jj), n_time, 0, n_time*b2t);
      h_count   [ ii ][ jj ] = new TH1D( Form("h_count_c%dch%d"  ,ii,jj), Form("h_count_c%dch%d"  ,ii,jj), n_time, 0, n_time    );
      h_width   [ ii ][ jj ] = new TH1D( Form("h_width_c%dch%d"  ,ii,jj), Form("h_width_c%dch%d"  ,ii,jj), n_time, 0, n_time*b2t);
      h_noise   [ ii ][ jj ] = new TH1D( Form("h_noise_c%dch%d"  ,ii,jj), Form("h_noise_c%dch%d"  ,ii,jj), n_time, 0, n_time*b2t);
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
      h_time1->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5);

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
	h_time [ t_chip_v->at( ivec ) ][ t_channel_v->at( ivec ) ]->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5);
	h_width[ t_chip_v->at( ivec ) ][ t_channel_v->at( ivec ) ]->Fill( width * b2t + 1e-5 );
      }
    }

    else{
      for( Int_t ivec=0; ivec < n_hit; ivec++ ){
	h_noise[ t_chip_v->at( ivec ) ][ t_channel_v->at( ivec ) ]->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5);
      }
    }

  } // End of event loop
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  cout << n_pulse << " "
       << n_event - n_pulse << " "
       << o_vref_b2 << " " 
       << o_vref_b5 << " "
       << noise_start_time << " "
       << before_prompt_time << " "
       << muon_time << " " 
       << n_time << " "
       << endl;

  for( int ii = 0; ii < n_chip-1; ii++ ){
    for( int jj = 0; jj < n_channel; jj++ ){

      cout << h_time [ ii ][ jj ]->Integral( muon_time , n_time ) << " " 
	   << h_time [ ii ][ jj ]->Integral( noise_start_time , before_prompt_time ) << " " 
	   << h_noise[ ii ][ jj ]->Integral( muon_time , n_time ) << " " 
	   << flush;
    }
  }
  cout << endl << endl;

  TFile* file = new TFile( Form("time.root"),"RECREATE" );
  h_time1      ->Write();


  for( int ii = 0; ii < n_chip; ii++ ){
    for( int jj = 0; jj < n_channel; jj++ ){
      
      h_time       [ ii ][ jj ]->Write();
      h_width      [ ii ][ jj ]->Write();
      h_noise      [ ii ][ jj ]->Write();
    }
  }
  file->Close();
  //  std::cout << "finish" << std::endl;

  //  if( !gROOT->IsBatch() ) app.Run();

  return 0;

}
