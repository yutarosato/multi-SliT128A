#include "setting.h"

// message and plot
const Int_t  fl_message = 0;
const Bool_t fl_batch   = !true; // should be false for quick check.
const Int_t  fl_show    = 100;
const Int_t  fl_save    = !true;

const Int_t nboard = 2;
const Int_t rev_board_map[nboard] = {2,5}; // reverse map
const Int_t board_map[] = {-999,-999,0,-999,-999,1}; // Board#2 -> [0], Board#5 -> [1]
// axis ragnge
const Int_t nsig_max   =  15;
const Int_t nring_max  =  20;
const Int_t width_max  = 300;
const Int_t nnoise_max =  50;

// setup
const Int_t nsig_exp     =    8; // # of signals per event (probably 8 @200kHz)
const Int_t span_exp     = 1000; // 200kHz -> 5us -> 1000 bit
const Int_t origin_time  =     0; // not useful
const Int_t origin_range =  1000; // not useful

// signal definition
const Int_t    th_span       =   450;
const Int_t    th_width      =     5;
const Int_t    th_window     =    20;
const Int_t    mask_prompt   =    50;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TH1I* hist_nsig    = new TH1I( "hist_nsig",    "hist_nsig;#signal/events",          nsig_max,           0,    nsig_max  );
TH1I* hist_nring   = new TH1I( "hist_nring",   "hist_nring;#ringing/signals",      nring_max,           0,   nring_max  );
TH1I* hist_nnoise  = new TH1I( "hist_nnoise",  "hist_nnoise;#noise/events",        nnoise_max,          0,   nnoise_max );
TH1I* hist_width   = new TH1I( "hist_width",   "hist_width;signal width",          width_max,           0,   width_max  );
TH1I* hist_span    = new TH1I( "hist_span",    "hist_span;span between signals",          40, span_exp-20, span_exp+20  );
TH1I* hist_1ch_int = new TH1I( "hist_1ch_int", "hist_1ch_int;Time [bit]",             n_time,           0,      n_time  ); 
TH2I* hist_wid_tim = new TH2I( "hist_wid_tim", "hist_wid_tim;Time [bit];Width [bit]", n_time,           0,      n_time, width_max, 0,   width_max );

Int_t detect_signal( TH1I* hist ){
  if( fl_message > 1 ) std::cout << "[DETECT_SIGNAL]" << std::endl;
  Int_t  bin_start        = 0;
  Int_t  bin_end          = 0;
  Int_t  bin_start_before = 0;
  Bool_t fl_bin           = false;
  Bool_t fl_start         = true;
  Int_t  cnt_nsig         = 0;
  Int_t  cnt_nring        = 0;
  Int_t  cnt_nnoise       = 0;

  for( Int_t ibin=0; ibin<hist->GetNbinsX(); ibin++ ){
    if( hist->GetBinContent(ibin+1) && !fl_bin ){ // edge(up)
      bin_start = ibin;
      fl_bin = true;
    }else if( !hist->GetBinContent(ibin+1) && fl_bin){ // edge(down)
      bin_end = ibin;
      fl_bin = false;

      Int_t width = bin_end - bin_start;
      Int_t span  = bin_start - bin_start_before;

      Double_t entry_eff_before = ( bin_start>th_window          ? hist->Integral( bin_start-th_window, bin_start-1           )/((Double_t)th_window) : hist->Integral(1,         bin_start-1 )/(Double_t)(bin_start-1       ) );
      Double_t entry_eff_after  = ( bin_start<n_time-th_window+2 ? hist->Integral( bin_start,           bin_start+th_window-1 )/((Double_t)th_window) : hist->Integral(bin_start, n_time      )/(Double_t)(n_time-bin_start+1) );
      //if( bin_start>th_window          ) std::cout << "[TEST(pre) ] " << bin_start-th_window << " -> " << bin_start-1           << " : " << hist->Integral( bin_start-th_window, bin_start-1           ) << std::endl;
      //if( bin_start<n_time-th_window+2 ) std::cout << "[TEST(post)] " << bin_start           << " -> " << bin_start+th_window-1 << " : " << hist->Integral( bin_start,           bin_start+th_window-1 ) << std::endl;
      
      if( fl_message>1 ) std::cout << bin_start << " -> " << bin_end
				   << " : width = "       << width
				   << ", span = "         << span
				   << ", eff(before) = "  << entry_eff_before
				   << ", eff(after) = "   << entry_eff_after  << std::endl;

      if( mask_prompt > ibin ){ // remove prompt noise
	;
      }else{
	Int_t origin = bin_start;
	while( origin> span_exp ){
	  origin -= span_exp;
	}

	if( origin > origin_time && origin < origin_time + origin_range ){ // signal window
	  if( ( span > th_span || bin_start_before == 0) && width > th_width ){ // true signal
	    hist_width  ->Fill( width     );
	    hist_1ch_int->Fill( bin_start );
	    hist_wid_tim->Fill( bin_start, width );
	    if( !fl_start ){
	      hist_nring->Fill( cnt_nring );
	      hist_span ->Fill( span );
	    }
	    bin_start_before = bin_start;
	    if( fl_message > 1 ) std::cout << " ------> nsig" << std::endl;
	    cnt_nsig++;
	    cnt_nring = 0;
	    fl_start = false;
	  }else{ // ringing
	    cnt_nring++;
	  }
	}else{ // noise
	  cnt_nnoise++;
	}
      }
    }
  }
  
  if( fl_message ) std::cout << " ------> "
			     << "nsig = "   << cnt_nsig   << ", "
			     << "nring = "  << cnt_nring  << ", "
			     << "nnoise = " << cnt_nnoise << std::endl;
  hist_nring ->Fill( cnt_nring  );
  hist_nsig  ->Fill( cnt_nsig   );
  hist_nnoise->Fill( cnt_nnoise );

  return 0;
}

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadGridX(0);
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==2) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root" << std::endl
	      << std::endl, abort();

  Char_t* infilename = app.Argv(1);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  set_readbranch(chain);

  if( fl_message ) printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH2I** hist_1evt     = new TH2I*[nboard];
  TH2I** hist_1evt_int = new TH2I*[nboard];
  for( Int_t iboard=0; iboard<nboard; iboard++ ){
    hist_1evt[iboard] = new TH2I( Form("hist_1evt_board%d",rev_board_map[iboard]), Form("hist_1evt_board%d",rev_board_map[iboard]),
				  n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    hist_1evt_int[iboard] = new TH2I( Form("hist_1evt_int_board%d",rev_board_map[iboard]), Form("hist_1evt_int_board%d",rev_board_map[iboard]),
				      n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //TH1I* hist_time  = new TH1I("hist_1ch",  "hist;Time  [bit];Channel",n_time, 0, n_time );
  //TH1I* hist_width = new TH1I("hist_1ch",  "hist;Width [bit];Channel",   100, 0,    100 );
  
  TCanvas* can1 = new TCanvas("can1","can1", 1800, 1000 );
  can1->Divide(2,2);
  can1->Draw();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    for( Int_t iboard=0; iboard<nboard; iboard++ ){
      hist_1evt[iboard]->Reset();
    }
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){
      if( t_board_v->at(ivec) !=2 && t_board_v->at(ivec) !=5 ){
	std::cerr << "[WARNING] Wrong board-id : " << t_board_v->at(ivec) << std::endl;
	continue;
      }
      hist_1evt[board_map[t_board_v->at(ivec)]]->Fill( t_time_v->at(ivec), multi_ch_map( t_chip_v->at(ivec), t_unit_v->at(ivec), t_bit_v->at(ivec) ) );
    }
    
    
    //detect_signal( hist_1evt );
    
    //if( fl_message ) printf( "  [Event:%d,Board:%d,Chip:%d,Channel:%d] %d entries\n", t_event, board_id, chip_id_v, channel_id, (Int_t)hist_1evt->Integral() );
    for( Int_t iboard=0; iboard<nboard; iboard++ ){
      hist_1evt    [iboard]->SetTitle( Form("Event:%d, Board:%d", t_event, rev_board_map[iboard]) );
      hist_1evt_int[iboard]->SetTitle( Form("Event:%d, Board:%d", t_event, rev_board_map[iboard]) );
    }
    
    if( cnt_show++ < fl_show ){
      for( Int_t iboard=0; iboard<nboard; iboard++ ){
	can1->cd(2*iboard+1); hist_1evt    [iboard]->Draw();
	can1->cd(2*iboard+2); hist_1evt_int[iboard]->Draw();

      }
      can1->Update();
      can1->WaitPrimitive();
    }
  } // END EVENT-LOOP
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if( fl_message ) std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  delete chain;
  for( Int_t iboard=0; iboard<nboard; iboard++ ){
    delete hist_1evt    [iboard];
    delete hist_1evt_int[iboard];
  }
  delete can1;

  return 0;

}
