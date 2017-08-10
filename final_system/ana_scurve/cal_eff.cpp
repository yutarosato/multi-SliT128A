#include "setting.h"

// message and plot
const Int_t  fl_message = 0;
const Bool_t fl_batch   = true; // should be true for s-curve scan
const Int_t  fl_show    = 3;
const Bool_t fl_save_file = true;
const Bool_t fl_save_plot = true;

// axis ragnge
const Int_t nsig_max = 15;
const Int_t nring_max= 20;
const Int_t width_max= 300;

// setup
const Int_t nsig_exp     =     8; // # of signals per event (probably 8 @200kHz)
const Int_t span_exp     =  1000; // 200kHz -> 5us -> 1000 bit
const Int_t origin_time  =     0; // not useful
const Int_t origin_range =  1000; // not useful

// signal definition
const Int_t    th_span       =   450;
const Int_t    th_width      =     5;
const Int_t    mask_prompt   =    50; // mask prompt noise

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TH2I* hist_nsig;
TH2I* hist_nring;
TH2I* hist_width;
TH2I* hist_span;
TH2I* hist_evt_int;
TH2I* hist_evt;

Int_t detect_signal( TH2I* hist ){
  if( fl_message > 1 ) std::cout << "[DETECT_SIGNAL]" << std::endl;
  for( Int_t iybin=0; iybin<hist->GetNbinsY(); iybin++ ){
    if( iybin%t_tpchannel_cycle!=t_tpchannel ) continue;
    if( hist->Integral(1,hist->GetNbinsX()+1,iybin+1,iybin+1)==0 ){
      hist_nsig->Fill( 0.0001, iybin );
      continue;
    }
    Int_t  bin_start        = 0;
    Int_t  bin_end          = 0;
    Int_t  bin_start_before = 0;
    Bool_t fl_bin           = false;
    Bool_t fl_start         = true;
    Int_t  cnt_nsig         = 0;
    Int_t  cnt_nring        = 0;
    for( Int_t ixbin=0; ixbin<hist->GetNbinsX(); ixbin++ ){
      if( hist->GetBinContent(ixbin+1,iybin+1) && !fl_bin ){ // edge(up)
	bin_start = ixbin;
	fl_bin = true;
      }else if( !hist->GetBinContent(ixbin+1,iybin+1) && fl_bin){ // edge(down)
	bin_end = ixbin;
	fl_bin = false;
	
	Int_t width = bin_end - bin_start;
	Int_t span  = bin_start - bin_start_before;
	
	if( fl_message>1 ) std::cout << bin_start << " -> " << bin_end
				     << " : width = "       << width
				     << ", span = "         << span
				     << std::endl;
	
	if( mask_prompt > ixbin ){ // remove prompt noise
	  ;
	}else{
	  if( ( span > th_span || bin_start_before == 0) && width > th_width ){ // true signal
	    hist_width  ->Fill( width,     iybin );
	    hist_evt_int->Fill( bin_start, iybin );
	    if( !fl_start ){
	      hist_nring->Fill( cnt_nring, iybin );
	      hist_span ->Fill( span,      iybin );
	    }
	    bin_start_before = bin_start;
	    if( fl_message > 1 ) std::cout << " ------> nsig" << std::endl;
	    cnt_nsig++;
	    cnt_nring = 0;
	    fl_start = false;
	  }else{ // ringing
	    cnt_nring++;
	  }
	}
      }
    }

    if( fl_message ) std::cout << " ------> nsig = " << cnt_nsig  << ", "
			       << "nring = "         << cnt_nring << std::endl;
    hist_nring->Fill( (cnt_nring >= nring_max ? nring_max-1 : cnt_nring), iybin );
    hist_nsig ->Fill( (cnt_nsig  >= nsig_max  ? nsig_max -1 : cnt_nsig ), iybin );
  }

  return 0;
}

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadGridX(0);
  sty->SetPadLeftMargin(0.12);
  sty->SetTitleOffset(1.05,"y");
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( app.Argc()!=4 )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename (int)dac (int)board_id" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root 30 2" << std::endl
	      << std::endl, abort();

  Char_t* infilename   = app.Argv(1);
  Int_t   dac          = atoi( app.Argv(2) );
  TString dac_name     = app.Argv(2);
  dac_name.ReplaceAll("-","n");
  Int_t   board_id     = atoi( app.Argv(3) );
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".root") );
  std::string headername = gSystem->BaseName( infilename );
  headername.erase( headername.rfind(".root") );
  headername.erase( headername.rfind(Form("_%d",dac)));

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TText* tex1 = new TText();
  tex1->SetTextColor(2);
  tex1->SetTextSize(0.1);
  TText* tex2 = new TText();
  tex2->SetTextColor(2);
  tex2->SetTextSize(0.05);

  hist_nsig    = new TH2I( "hist_nsig",    "hist_nsig;#signal/events;Channel",              nsig_max,           0,    nsig_max, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  hist_nring   = new TH2I( "hist_nring",   "hist_nring;#ringing/signals;Channel",          nring_max,           0,   nring_max, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  hist_width   = new TH2I( "hist_width",   "hist_width;signal width [bit];Channel",        width_max,           0,   width_max, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  hist_span    = new TH2I( "hist_span",    "hist_span;span between signals [bit];Channel",        40, span_exp-20, span_exp+20, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  hist_evt_int = new TH2I( "hist_evt_int", "hist_evt_int;Time [bit];Channel",                 n_time,           0,      n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );

  hist_nsig   ->SetLineColor(2);
  hist_nring  ->SetLineColor(2);
  hist_width  ->SetLineColor(2);
  hist_span   ->SetLineColor(2);
  hist_evt_int->SetLineColor(2);

  hist_nsig   ->SetName( Form("hist_%s_board%d_dac%s","nsig",   board_id,dac_name.Data()) );
  hist_nring  ->SetName( Form("hist_%s_board%d_dac%s","nring",  board_id,dac_name.Data()) );
  hist_width  ->SetName( Form("hist_%s_board%d_dac%s","width",  board_id,dac_name.Data()) );
  hist_span   ->SetName( Form("hist_%s_board%d_dac%s","span",   board_id,dac_name.Data()) );
  hist_evt_int->SetName( Form("hist_%s_board%d_dac%s","evt_int",board_id,dac_name.Data()) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  set_readbranch_scan(chain);
  set_readbranch(chain);

  if( fl_message ) printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  hist_evt = new TH2I( "hist_evt", "hist_evt;Time [bit];Channel", n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  hist_evt->SetLineColor(2);
  hist_evt->SetLabelSize(0.17,"XY");

  TCanvas* can1 = new TCanvas( "can1", "can1", 1800, 300 );
  can1->Draw();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  Int_t vref     = -999;
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    // read data
    chain->GetEntry(ievt);
    hist_evt->Reset();
    Bool_t fl_board = false;
    for( Int_t iboard=0; iboard<t_tpboard_v->size(); iboard++ ){
      if( board_id == t_tpboard_v->at(iboard) || t_tpboard_v->at(iboard)<0 ) fl_board = true;
      if( board_id == t_tpboard_v->at(iboard)                              ) vref     = t_vref_v->at(iboard);
    }
    if( !fl_board    ) continue; // select specified board
    if( dac != t_dac ) continue; // dac value
    for( Int_t ihit=0; ihit<t_unit_v->size(); ihit++ ){ // BEGIN HIT-LOOP
      if( t_board_v  ->at(ihit)                    != board_id                      ) continue; // board
      if( t_chip_v   ->at(ihit)%t_tpchip_cycle     != t_tpchip    && t_tpchip   >=0 ) continue; // chip
      if( t_channel_v->at(ihit)%t_tpchannel_cycle  != t_tpchannel && t_tpchannel>=0 ) continue; // channel
      hist_evt->Fill( t_time_v->at(ihit), gchannel_map(t_chip_v->at(ihit), t_unit_v->at(ihit), t_bit_v->at(ihit)) );
    } // END HIT-LOOP
    // calcualte
    detect_signal( hist_evt );

    // draw
    if( cnt_show < fl_show ){
      hist_evt->Draw("COLZ");
      can1->Update();
      can1->WaitPrimitive();
      if( cnt_show==1 && fl_save_plot ) can1->Print( Form("pic/%s_can1.ps",basename.c_str()) );
    }
    cnt_show++;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TCanvas* can2 = new TCanvas( "can2", "can2", 1600, 800 );
  if( fl_save_plot || !fl_batch ){
    can2->Clear();
    
    sty->SetOptStat(1111111);
    can2->Draw();
    can2->Divide(3,2);
    can2->cd(1); hist_nsig   ->Draw("COLZ");
    can2->cd(2); hist_nring  ->Draw("COLZ");
    can2->cd(3); hist_width  ->Draw("COLZ");
    can2->cd(4); hist_span   ->Draw("COLZ");
    can2->cd(5); hist_evt_int->Draw("COLZ");
    can2->cd(6);
    tex2->DrawTextNDC( 0.20, 0.65, Form("Board = %d",        board_id ) );
    tex2->DrawTextNDC( 0.20, 0.60, Form("DAC = %d",          dac      ) );
    tex2->DrawTextNDC( 0.20, 0.55, Form("span = %d",         th_span  ) );
    tex2->DrawTextNDC( 0.20, 0.50, Form("width = %d",        th_width ) );
    can2->Update();
    if( fl_save_plot ) can2->Print( Form("pic/%s_can2.ps",basename.c_str()) );
    can2->WaitPrimitive();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // triming noise data // tmppppp
  //Int_t peak_bin = hist_width[ich]->GetMaximumBin();
  //for( Int_t ixbin=0; ixbin<peak_bin-50; ixbin++ ) hist_width[ich]->SetBinContent(ixbin+1, 0.0);
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::vector<Double_t> t_sig_eff_v;
  std::vector<Double_t> t_sig_effE_v;
  for( Int_t iybin=0; iybin<hist_nsig->GetNbinsY(); iybin++ ){
    TH1D*    hist_nsig_proj = hist_nsig->ProjectionX("_px",iybin+1,iybin+1);
    Double_t nsig_integral  = hist_nsig_proj->Integral(1,hist_nsig->GetNbinsX());
    Double_t nsig_mean      = hist_nsig_proj->GetMean() - hist_nsig_proj->GetBinWidth(1)/2.0;

    TH1D*    hist_nring_proj = hist_nring->ProjectionX("_px",iybin+1,iybin+1);
    Double_t nring_integral  = hist_nring_proj->Integral(1,hist_nring->GetNbinsX());
    Double_t nring_mean      = hist_nring_proj->GetMean() - hist_nring_proj->GetBinWidth(1)/2.0;
;

    TH1D*    hist_width_proj = hist_width->ProjectionX("_px",iybin+1,iybin+1);
    TH1D*    hist_span_proj  = hist_span ->ProjectionX("_px",iybin+1,iybin+1);

    Double_t sig_eff    = ( nsig_integral ? nsig_mean/nsig_exp : 0.0 );
    Double_t sig_effE   = ( (nsig_mean*nsig_integral== 0) ? 0.0 : sig_eff * sqrt( (1-sig_eff)/(nsig_mean*nsig_integral)) );
    Double_t ring_prob  = ( (nring_integral==0) ? 0.0 : (Double_t)(nring_integral - hist_nring_proj->GetBinContent(1))/nring_integral );
    Double_t ring_probE = ( ((Double_t)(nring_integral - hist_nring_proj->GetBinContent(1))==0) ? 0.0 : ring_prob * sqrt( (1-ring_prob)/((Double_t)(nring_integral - hist_nring_proj->GetBinContent(1))) ) );
    Double_t sig_width  = hist_width_proj->GetMean() - hist_width_proj->GetBinWidth(1)/2.0;
    Double_t sig_widthE = hist_width_proj->GetMeanError(); // hist_width[ich]->GetRMS();
    Double_t span       = hist_span_proj ->GetMean() - hist_span_proj->GetBinWidth(1)/2.0;
    Double_t spanE      = hist_span_proj ->GetMeanError(); // hist_span [ich]->GetRMS();
    system( Form("mkdir -p dat_scurve/%s", headername.c_str()) );
    std::ofstream outlog(Form("dat_scurve/%s/%s_%03d.dat",headername.c_str(),headername.c_str(),iybin), std::ios::app );
    outlog << std::setw(15) << std::right << dac         << " "
	   << std::setw(15) << std::right << vref        << " "
	   << std::setw(15) << std::right << t_tpchg     << " "
	   << std::setw(15) << std::right << sig_eff     << " "
	   << std::setw(15) << std::right << sig_effE    << " "
	   << std::setw(15) << std::right << ring_prob   << " "
	   << std::setw(15) << std::right << ring_probE  << " "
	   << std::setw(15) << std::right << sig_width   << " "
	   << std::setw(15) << std::right << sig_widthE  << " "
	   << std::setw(15) << std::right << span        << " "
	   << std::setw(15) << std::right << spanE       << " "
	   << std::setw(15) << std::right << board_id    << " "
	   << std::setw(15) << std::right << gchannel2chip    (iybin) << " "
	   << std::setw(15) << std::right << gchannel2lchannel(iybin) << " "
	   << " BOARD"   << board_id                 //  board-No
	   << " CHIP"    << gchannel2chip    (iybin) //  chip-No
	   << " CHANNEL" << gchannel2lchannel(iybin) //  channel-No
	   << "HOGEEEE"
	   << std::endl;
    t_sig_eff_v.push_back ( sig_eff  );
    t_sig_effE_v.push_back( sig_effE );
  }
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( fl_save_file ){
    TFile outfile( Form("pic/%s.root",basename.c_str()), "RECREATE" );

    Int_t t_nsig = nsig_exp;
    Int_t t_span = span_exp;
    TTree* newtree = new TTree( "meta", "meta" );
    if( chain->GetEntries() ) chain->GetEntry(0);
    newtree->Branch( "dac",      &dac,      "dac/I"   );
    newtree->Branch( "tpchg",    &t_tpchg,  "tpchg/F" );
    newtree->Branch( "span",     &t_span,   "span/I"  );
    newtree->Branch( "nsig",     &t_nsig,   "nsig/I"  );
    newtree->Branch( "sigeff",   &t_sig_eff_v         );
    newtree->Branch( "sigeffE",  &t_sig_effE_v        );
    newtree->Fill();
    
    // To save data size, some histogram might be omitted.
    hist_nsig   ->Write();
    hist_nring  ->Write();
    hist_width  ->Write();
    hist_span   ->Write();
    hist_evt_int->Write();
    newtree->Write();
    //can2->Write();
    outfile.Close();
    
    //delete newtree;
  }
      
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if( fl_message ) std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  delete tex1;
  delete tex2;
  delete chain;
  delete hist_evt;
  delete can1;
  delete can2;

  return 0;

}
