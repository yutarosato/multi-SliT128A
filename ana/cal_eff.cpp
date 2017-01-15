#include "setting.h"

// message and plot
const Int_t  fl_message = 0;    // should be zero
const Bool_t fl_batch   = true; // should be true
const Int_t  fl_show    = 2;

// axis ragnge
const Int_t nsig_max = 15;
const Int_t nring_max= 20;
const Int_t width_max= 300;

// setup
const Int_t nsig_exp =    8; // # of signals per event (probably 8 @200kHz)
//const Int_t nsig_exp =    2; // # of signals per event (8@200kHz)
const Int_t span_exp = 1000; // 200kHz -> 5us -> 1000 bit

// signal definition
const Int_t    th_span       =   450;
const Int_t    th_width      =     2;
const Int_t    th_window     =    20;
const Double_t th_eff_before =   2.0; // >=1.0 is no-cut
const Double_t th_eff_after  =  -1.0; // <=0.0 is no-cut

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


TH1I* hist_nsig    = new TH1I( "hist_nsig",    "hist_nsig;#signal/events",          nsig_max,           0,    nsig_max );
TH1I* hist_nring   = new TH1I( "hist_nring",   "hist_nring;#ringing/signals",      nring_max,           0,   nring_max );
TH1I* hist_width   = new TH1I( "hist_width",   "hist_width;signal width",          width_max,           0,   width_max );
TH1I* hist_span    = new TH1I( "hist_span",    "hist_span;span between signals",          40, span_exp-20, span_exp+20 );
TH1I* hist_1ch_int = new TH1I( "hist_1ch_int", "hist_1ch_int;Time [bit]",             n_time,           0,      n_time ); 
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

      if( entry_eff_before <= th_eff_before && entry_eff_after >= th_eff_after && ( span > th_span || bin_start_before == 0) && width > th_width ){ // true signal
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
    }
  }

  if( fl_message ) std::cout << " ------> nsig = " << cnt_nsig << ", "
			     << "nring = " << cnt_nring << std::endl;
  hist_nring->Fill( cnt_nring );
  hist_nsig ->Fill( cnt_nsig );

  return 0;
}

/*
Int_t detect_signal( TH1I* hist ){
  if( fl_message > 1 ) std::cout << "[DETECT_SIGNAL]" << std::endl;
  Int_t  bin_start        = 0;
  Int_t  bin_end          = 0;
  Int_t  bin_start_before = 0;
  Bool_t fl_bin           = false;
  Bool_t fl_start         = true;
  Int_t  cnt_nsig         = 0;
  Int_t  cnt_nring        = 0;

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

      if( entry_eff_before <= 0.15 && entry_eff_after >= 0.30 && ( span > th_span || bin_start_before == 0) && width > th_width ){ // true signal
	hist_width->Fill( width );
	if( !fl_start ) hist_nring->Fill( cnt_nring );
	if( !fl_start ) hist_span ->Fill( span );
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

  if( fl_message ) std::cout << " ------> nsig = " << cnt_nsig << ", "
			     << "nring = " << cnt_nring << std::endl;
  hist_nring->Fill( cnt_nring );
  hist_nsig ->Fill( cnt_nsig );

  return 0;
}
*/
/*
Int_t detect_signal( TH1I* hist ){
  if( fl_message > 1 ) std::cout << "[DETECT_SIGNAL]" << std::endl;
  Int_t  bin_start        = 0;
  Int_t  bin_end          = 0;
  Int_t  bin_start_before = 0;
  Bool_t fl_bin           = false;
  Bool_t fl_start         = true;
  Int_t  cnt_nsig         = 0;
  Int_t  cnt_nring        = 0;

  for( Int_t ibin=0; ibin<hist->GetNbinsX(); ibin++ ){
    if( hist->GetBinContent(ibin+1) && !fl_bin ){ // edge(up)
      bin_start = ibin;
      fl_bin = true;
    }else if( !hist->GetBinContent(ibin+1) && fl_bin){ // edge(down)
      bin_end = ibin;
      fl_bin = false;

      Int_t width = bin_end - bin_start;
      Int_t span  = bin_start - bin_start_before;
      if( fl_message>1 ) std::cout << bin_start << " -> " << bin_end
				   << " : width = " << width
				   << ", span = "   << span << std::endl;

      if( span > th_span && width > th_width ){ // true signal
	hist_width->Fill( width );
	if( !fl_start ) hist_nring->Fill( cnt_nring );
	if( !fl_start ) hist_span ->Fill( span );
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

  if( fl_message > 1 ) std::cout << " ------> nsig = " << cnt_nsig << ", "
				 << "nring = " << cnt_nring << std::endl;
  hist_nring->Fill( cnt_nring );
  hist_nsig ->Fill( cnt_nsig );

  return 0;
}
*/

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadGridX(0);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==5) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename (int)chip_id (int)obs_ch (int)tp_ch (int)dac" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root 0 0" << std::endl
	      << std::endl, abort();

  Char_t* infilename = app.Argv(1);
  Int_t   chip_id    = atoi( app.Argv(2) );
  Int_t   obs_ch     = atoi( app.Argv(3) );
  Int_t   tp_ch      = atoi( app.Argv(4) );
  Int_t   dac        = atoi( app.Argv(5) );
  
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".root") );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TText* tex1 = new TText();
  tex1->SetTextColor(2);
  tex1->SetTextSize(0.12);
  TText* tex2 = new TText();
  tex2->SetTextColor(2);
  tex2->SetTextSize(0.05);

  hist_nsig   ->SetLineColor(2);
  hist_nring  ->SetLineColor(2);
  hist_width  ->SetLineColor(2);
  hist_span   ->SetLineColor(2);
  hist_1ch_int->SetLineColor(2);

  hist_nsig   ->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","nsig",   obs_ch,tp_ch,dac) );
  hist_nring  ->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","nring",  obs_ch,tp_ch,dac) );
  hist_width  ->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","width",  obs_ch,tp_ch,dac) );
  hist_span   ->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","span",   obs_ch,tp_ch,dac) );
  hist_1ch_int->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","1ch_int",obs_ch,tp_ch,dac) );
  hist_wid_tim->SetName( Form("hist_%s_obsch%03d_tpch%03d_dac%d","wid_tim",obs_ch,tp_ch,dac) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  if( chain->GetEntries( Form("selch==%d && dac==%d",tp_ch,dac) )==0 ) return 0;
  TTree* tree = chain->CopyTree( Form("selch==%d && dac==%d",tp_ch,dac) );
  set_readbranch_scan(tree);
  //set_readbranch(tree);

  if( fl_message ) printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH1I* hist_1ch   = new TH1I("hist_1ch",  "hist;Time [bit];Channel",n_time,   0,        n_time   );
  hist_1ch->SetLineColor(2);
  hist_1ch->SetLabelSize(0.10,"XY");
  
  const Int_t ndiv = 4;
  TH1I** hist_1ch_div = new TH1I*[ndiv];
  for( Int_t idiv=0; idiv<ndiv; idiv++ ){
    hist_1ch_div[idiv] = new TH1I( Form("hist_1ch_div%d",idiv), "hist;Time [bit];Channel", n_time/ndiv, idiv*n_time/ndiv, (idiv+1)*n_time/ndiv );
    hist_1ch_div[idiv]->SetLineColor(2);    
    hist_1ch_div[idiv]->SetLabelSize(0.10,"XY");
  }

  TCanvas* can1 = new TCanvas("can1","can1", 1600, 600 );
  can1->Divide(1,ndiv+1);
  can1->Draw();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){
    tree->GetEntry(ievt);
    hist_1ch  ->Reset();
    for( Int_t idiv=0; idiv<ndiv; idiv++ ) hist_1ch_div[idiv]->Reset();
    if( dac    != t_dac   ) continue; // dac value
    if( tp_ch  != t_selch ) continue; // test puse channel
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){
      if( chip_id!=t_chip_v->at(ivec) ) continue;
      if( obs_ch != ch_map(t_unit_v->at(ivec),t_bit_v->at(ivec)) ) continue; // observed channel
      hist_1ch    ->Fill( t_time_v->at(ivec) );
      for( Int_t idiv=0; idiv<ndiv; idiv++ ) hist_1ch_div[idiv]->Fill( t_time_v->at(ivec) );
    }
    
    if( hist_1ch->GetEntries()==0 ){
      hist_nsig->Fill(0);
      continue;
    }

    detect_signal( hist_1ch );
    
    if( fl_message ) printf( "  [Event:%d,Chip:%d,Channel:(obs)%d,(tp)%d] %d entries\n",    t_event,chip_id,obs_ch,tp_ch, (Int_t)hist_1ch->Integral() );
    hist_1ch->SetTitle( Form("Event : %d, Chip : %d, Channel : (obs)%d,(tp)%d, %d entries", t_event,chip_id,obs_ch,tp_ch, (Int_t)hist_1ch->Integral()) );
    
    if( cnt_show++ < fl_show ){
      can1->cd(1); hist_1ch->Draw();
      for( Int_t idiv=0; idiv<ndiv; idiv++ ){
	can1->cd(idiv+2);
	hist_1ch_div[idiv]->Draw();
      }
      can1->cd(1);
      tex1->DrawTextNDC( 0.15, 0.92, Form("Event : %d, Chip : %d, Channel : (obs)%d,(tp)%d, DAC : %d (%d entries)", t_event,chip_id,obs_ch,tp_ch, dac, (Int_t)hist_1ch->Integral()) );
      can1->Update();
      can1->WaitPrimitive();
      if( cnt_show==1 ) can1->Print( Form("pic/%s_obsch%03d_tpch%03d_dac%d_can1.ps",basename.c_str(),obs_ch,tp_ch,dac) );
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TCanvas* can2 = new TCanvas( Form("can_obsch%03d_tpch%03d_dac%d",obs_ch,tp_ch,dac), Form("can2"), 1600, 800 );
  sty->SetOptStat(1111111);
  can2->Draw();
  can2->Divide(3,2);
  can2->cd(1); hist_nsig   ->Draw();
  can2->cd(2); hist_nring  ->Draw();
  can2->cd(3); hist_width  ->Draw();
  can2->cd(4); hist_span   ->Draw();
  can2->cd(5); hist_1ch_int->Draw();
  can2->cd(6); hist_wid_tim->Draw("COLZ");
  can2->cd(4);
  tex2->DrawTextNDC( 0.20, 0.75, Form("Chip = %d",         chip_id       ) );
  tex2->DrawTextNDC( 0.20, 0.70, Form("Ch(obs) = %d",      obs_ch        ) );
  tex2->DrawTextNDC( 0.20, 0.65, Form("Ch(tp) = %d",       tp_ch         ) );
  tex2->DrawTextNDC( 0.20, 0.60, Form("DAC = %d",          dac           ) );
  tex2->DrawTextNDC( 0.20, 0.55, Form("span = %d",         th_span       ) );
  tex2->DrawTextNDC( 0.20, 0.50, Form("width = %d",        th_width      ) );
  tex2->DrawTextNDC( 0.20, 0.45, Form("window = %d",       th_window     ) );
  tex2->DrawTextNDC( 0.20, 0.40, Form("eff(before) = %.2f",th_eff_before ) );
  tex2->DrawTextNDC( 0.20, 0.35, Form("eff(after) = %.2f", th_eff_after  ) );
  can2->Update();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // triming noise data // tmppppp
  Int_t peak_bin = hist_width->GetMaximumBin();
  for( Int_t ibin=0; ibin<peak_bin-50; ibin++ ) hist_width->SetBinContent(ibin+1, 0.0);
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Double_t sig_eff    = ( hist_nsig->Integral() ? (hist_nsig->GetMean()*hist_nsig->Integral())/(hist_nsig->Integral()*nsig_exp) : 0.0 );
  Double_t sig_effE   = ( (hist_nsig->GetMean()*hist_nsig->Integral()== 0) ? 0.0 : sig_eff * sqrt( (1-sig_eff)/(hist_nsig->GetMean()*hist_nsig->Integral()) ) );
  Double_t ring_prob  = ( (hist_nring->Integral()==0) ? 0.0 : (Double_t)(hist_nring->Integral() - hist_nring->GetBinContent(1))/hist_nring->Integral() );
  Double_t ring_probE = ( ((Double_t)(hist_nring->Integral() - hist_nring->GetBinContent(1))==0) ? 0.0 : ring_prob * sqrt( (1-ring_prob)/((Double_t)(hist_nring->Integral() - hist_nring->GetBinContent(1))) ) );
  Double_t sig_width  = hist_width->GetMean();
  Double_t sig_widthE = hist_width->GetMeanError(); // hist_width->GetRMS();
  Double_t span       = hist_span ->GetMean();
  Double_t spanE      = hist_span ->GetMeanError(); // hist_span ->GetRMS();
  std::cout << std::setw(15) << std::right << dac        << " "
	    << std::setw(15) << std::right << t_vref     << " "
	    << std::setw(15) << std::right << t_tpchg    << " "
	    << std::setw(15) << std::right << sig_eff    << " "
	    << std::setw(15) << std::right << sig_effE   << " "
	    << std::setw(15) << std::right << ring_prob  << " "
	    << std::setw(15) << std::right << ring_probE << " "
	    << std::setw(15) << std::right << sig_width  << " "
	    << std::setw(15) << std::right << sig_widthE << " "
	    << std::setw(15) << std::right << span       << " "
	    << std::setw(15) << std::right << spanE      << " "
	    << std::setw(15) << std::right << obs_ch     << " "
	    << std::setw(15) << std::right << tp_ch      << " "
	    << std::setw(15) << std::right << chip_id    << " "
	    << " OBSCH" << Form("%03d",obs_ch) //  observed  channel-No
	    << "TPCH"   << Form("%03d",tp_ch ) // test pulse channel-No
	    << "HOGE"   << std::endl;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TTree* newtree = new TTree( Form("meta_obsch%03d_tpch%03d_dac%d",obs_ch,tp_ch,dac), "meta for timing" );
  if( tree->GetEntries() ) tree->GetEntry(0); // need for t_tpchg
  newtree->Branch( "dac",    &dac,     "dac/I"   );
  newtree->Branch( "tpchg",  &t_tpchg, "tpchg/F" );
  newtree->Fill();
  
  TFile outfile( Form("pic/%s_obsch%03d_tpch%03d_dac%d.root",basename.c_str(),obs_ch,tp_ch,dac), "RECREATE" );
  // To save data size, some histogram might be omitted.
  hist_nsig   ->Write();
  hist_nring  ->Write();
  hist_width  ->Write();
  hist_span   ->Write();
  hist_1ch_int->Write();
  hist_wid_tim->Write();
  newtree->Write();
  //can2->Write();
  outfile.Close();
  can2->Print( Form("pic/%s_obsch%03d_tpch%03d_dac%d_can2.ps",basename.c_str(),obs_ch,tp_ch,dac) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if( fl_message ) std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  delete tex1;
  delete tex2;
  delete chain;
  delete tree;
  delete hist_1ch;
  for( Int_t idiv=0; idiv<ndiv; idiv++ ) delete hist_1ch_div[idiv];
  delete hist_1ch_div;
  delete can1;
  delete can2;
  delete newtree;

  return 0;

}
