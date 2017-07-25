#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 0;


const int min_width = 15; // estimated minimum signal width ( for signal estimation )

const int max_width = 200; // for histogram
const int max_hit   = 200; // for histogram
const int max_cluster = 128;
const int timediff_window = 10; // cluster time diff window (bit)
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

  TH1D* h_cluster_diff   [ n_chip ];
  TH1D* h_ncluster   [ n_chip ];
  
  for( int ii = 0; ii < n_chip; ii++ ){
    h_ncluster [ ii ] = new TH1D( Form("h_ncluster_c%d" ,ii), Form("h_ncluster_c%d" ,ii ), max_cluster, 0, max_cluster);
    h_cluster_diff [ ii ] = new TH1D( Form("h_cluster_c%d" ,ii), Form("h_cluster_c%d" ,ii ), 2*n_time, -n_time, n_time);
  }


  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  int n_pulse = 0;
  int total_n_sig = 0;
  int total_n_pos = 0;
  Float_t o_vref_b2;
  Float_t o_vref_b5;


  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    if( ievt %1000 == 0 )
      cout << ievt << endl;    
    int n_hit = t_ledge_v->size();

    o_vref_b2 = t_vref_b2;
    o_vref_b5 = t_vref_b5;

    vector<bool> flag( n_hit );
    
    //-- Initialize --
    int width = 0;
    int n_sig = 0;
    int n_pos = 0;
    //----------------

    for( Int_t ivec=0; ivec < n_hit; ivec++ ){
      int i_chip = t_chip_v->at( ivec );
      int i_channel = t_channel_v->at( ivec ); 
      int i_ledge = t_ledge_v->at( ivec );

      if( i_ledge < muon_time || ( i_channel==0 && i_chip==0 ) ||
	  ( i_channel > 64 && i_chip==1 )
	  ) continue;
      
      for( Int_t jvec = ivec +1; jvec < n_hit; jvec++ ){
	int j_chip = t_chip_v->at( jvec );
	int j_channel = t_channel_v->at( jvec ); 
	int j_ledge = t_ledge_v->at( jvec );
	if( j_ledge < muon_time ) continue;
	
	if( i_chip == j_chip && i_channel +1 == j_channel ){
	  h_cluster_diff[ i_chip ]->Fill( j_ledge - i_ledge );
	}
	
      } 
      
    }


    //------------- cluster searching ----------------
    int search_chip;
    int search_channel;
    int n_cluster = 0;
    int lt1;
    int lt2;
    for( int ii = 0; ii < n_chip; ii++ ){
      for( int jj = 0; jj < n_channel; jj++ ){
	bool hit_flag = false;
	n_cluster = 0;

	for( int ivec = 0; ivec < n_hit; ivec++ ){
	  search_chip    = t_chip_v   ->at( ivec );
	  search_channel = t_channel_v->at( ivec ); 
	  lt1            = t_ledge_v->at( ivec );
	  if( lt1 < muon_time ) continue;
	  if( ii == search_chip && jj == search_channel && !flag[ ivec ] ){
	    flag[ ivec ]=true;
	    hit_flag = true;
	    n_cluster++;
	    break;
	  }
	}

	int next_channel = jj + 1;
	bool cl_flag = true;

	while( cl_flag && hit_flag ){
	  cl_flag = false;

	  for( int ivec = 0; ivec < n_hit; ivec++ ){
	    search_chip    = t_chip_v   ->at( ivec );
	    search_channel = t_channel_v->at( ivec ); 
	    lt2            = t_ledge_v->at( ivec );
	    if( lt2 < muon_time ) continue;
	    if( ii == search_chip && next_channel == search_channel && !flag[ ivec ] && 
		( lt2 - lt1 > -timediff_window && lt2 - lt1 < timediff_window ) ){	    
	      flag[ ivec ]=true;
	      cl_flag=true;
	      n_cluster++;
	      next_channel++;
	      lt1  = t_ledge_v->at( ivec );
	      break;
	    }
	  }
	}
	if( hit_flag )
	  h_ncluster[ ii ]->Fill( n_cluster );
      }
    }  // End of cluster searching
    
  } // End of event loop
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


  TFile* file = new TFile( Form("cluster.root"),"RECREATE" );
  
  
  for( int ii = 0; ii < n_chip; ii++ ){
    h_ncluster    [ ii ]->Write();
    h_cluster_diff[ ii ]->Write();
  }
  file->Close();
  //  std::cout << "finish" << std::endl;

  //  if( !gROOT->IsBatch() ) app.Run();

  return 0;

}
