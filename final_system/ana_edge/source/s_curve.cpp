#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 0;

const int min_width = 15; // estimated minimum signal width ( for signal estimation )

const int max_width = 200; // for histogram
const int max_hit   = 5000; // for histogram
const int n_event_threshold = 3;
const double b2t = 5e-3; // bit to time (5 ns)

const int rebin = 1;


Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==2) ){
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root 0" << std::endl
	      << std::endl;//, abort();
    return 0;
  }
  Char_t* infilename = app.Argv(1);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  set_readbranch(chain);
  
  //  printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  int n_pulse = 0;

  
  Float_t o_vref_b2;
  Float_t o_vref_b5;


  //--------  histogram --------


  TH1D* h_hit_all       = new TH1D("h_hit_all"   ,"h_hit_all"   , max_hit, 1, max_hit);
  TH1D* h_hit_all_wcut  = new TH1D("h_hit_all_wcut"   ,"h_hit_all_wcut"   , max_hit, 1, max_hit);
  TH1D* h_hit_all_tcut[ n_chip ];
    for( int ii = 0; ii < n_chip; ii++ )
      h_hit_all_tcut[ ii ]  = new TH1D( Form("h_hit_all_tcut%d", ii)   , Form("h_hit_all_tcut%d", ii)   , max_hit, 1, max_hit);

  TH2D* hitmap          = new TH2D("hitmap", "hitmap", n_channel*2/rebin, 0, n_channel*2, 2, 0, 2);
  TH2D* h_width_vs_time = new TH2D("h_width_vs_time","h_width_vs_time", n_time, 0, n_time, max_width, 0, max_width);

  TH1D* h_time_cut     = new TH1D("h_time_cut", "h_time_cut", n_time, 0, (double)n_time *b2t);
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);

    int n_hit = t_ledge_v->size();

    if( n_hit < n_event_threshold ) continue;

    o_vref_b2 = t_vref_b2;
    o_vref_b5 = t_vref_b5;
    //-- Initialize --
    int width = 0;
    int pre_ledge = 0;
    int pre_channle = 0;
    int n_hit_wcut = 0;
    int n_hit_tcut[ n_chip ];
    for( int ii = 0; ii < n_chip; ii++ )
      n_hit_tcut[ ii ] = 0; 
    bool beam_on = false;
    //----------------
    
    for( Int_t ivec=0; ivec < n_hit; ivec++ ){
      
      
      width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );
      
      h_width_vs_time->Fill( t_ledge_v->at( ivec ) , width );
      
      
      if( t_ledge_v->at( ivec ) > 2300 )
	n_hit_tcut[ t_chip_v->at( ivec ) ]++;
      
      if( width > min_width ){
	beam_on = true;
	hitmap->Fill( t_channel_v->at( ivec ) + (n_channel * (t_chip_v->at( ivec ) %2) ) , t_chip_v->at( ivec ) /2 );
	n_hit_wcut++;
	h_time_cut->Fill( (double)t_ledge_v->at( ivec )*b2t + 1e-4 );
      }
      
    
      
      //
      h_hit_all_wcut->Fill( n_hit_wcut );
      for( int ii = 0; ii < n_chip; ii++ )
	h_hit_all_tcut[ ii ]->Fill( n_hit_tcut[ ii ] );
      h_hit_all->Fill( n_hit );
      if( beam_on ) n_pulse ++;
      //    std::cout << ievt << std::endl;
    }
  } // End of event loop
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  h_hit_all->SetTitle( Form("total # of hit (%d entries, %d pulses)", 
			    chain->GetEntries(), n_pulse) 
		       );
  

  
   std::cout << o_vref_b2 << " "
	     << o_vref_b5 << " "
     /*
	     << h_hit_all ->GetMean() << " " 
	     << h_hit_all ->GetMeanError() << " "
     */
	     << h_hit_all_tcut[ 0 ] ->GetMean() << " " 
	     << h_hit_all_tcut[ 0 ] ->GetMeanError() << " "
	     << h_hit_all_tcut[ 1 ] ->GetMean() << " " 
	     << h_hit_all_tcut[ 1 ] ->GetMeanError() << " "

     /*
     	     << h_hit_all_wcut ->GetMean() << " " 
	     << h_hit_all_wcut ->GetMeanError() << " "
     */
	     << hitmap->GetBinContent( 123, 1 ) << " "
	     << hitmap->GetBinContent( 124, 1 ) << " "
	     << hitmap->GetBinContent( 125, 1 ) << " "
	     << std::endl; 

   //   std::cout << "finish" << std::endl;

   TFile *file = new TFile("tmp.root","RECREATE");


//   //  if( !gROOT->IsBatch() ) app.Run();

//   return 0;

}
