#include "setting.h"

// message and plot
const Int_t  fl_message = 0;
const Bool_t fl_batch   = !true; // should be false for quick check.
const Int_t  fl_show    = 1;
const Bool_t fl_save    = true;

// axis ragnge
const Int_t nsig_max = 15;
const Int_t nring_max= 20;
const Int_t width_max= 300;

// setup
const Int_t nsig_exp =    8; // # of signals per event (probably 8 @200kHz)
const Int_t span_exp = 1000; // 200kHz -> 5us -> 1000 bit

// signal definition
const Int_t    th_span       =   450;
//const Int_t    th_width      =     5;
const Int_t    th_width      =    15; // tmpppp
const Int_t    th_window     =    20;
const Int_t    mask_prompt   =    50;
const Double_t th_eff_before =   2.0; // >=1.0 is no-cut
const Double_t th_eff_after  =  -1.0; // <=0.0 is no-cut

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


TH1I** hist_nsig;
TH1I** hist_nring;
TH1I** hist_width;
TH1I** hist_span;
TH1I** hist_1ch_int;
TH2I** hist_wid_tim;

TH1I** hist_1ch;

Int_t detect_signal( TH1I* hist, Int_t ich ){
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

      if( mask_prompt > ibin ){ // remove prompt noise
	;
      }else if( entry_eff_before <= th_eff_before && entry_eff_after >= th_eff_after && ( span > th_span || bin_start_before == 0) && width > th_width ){ // true signal
	hist_width  [ich]->Fill( width     );
	hist_1ch_int[ich]->Fill( bin_start );
	hist_wid_tim[ich]->Fill( bin_start, width );
	if( !fl_start ){
	  hist_nring[ich]->Fill( cnt_nring );
	  hist_span [ich]->Fill( span );
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

  if( fl_message ) std::cout << " ------> nsig = " << cnt_nsig  << ", "
			     << "nring = "         << cnt_nring << std::endl;
  hist_nring[ich]->Fill( cnt_nring );
  hist_nsig [ich]->Fill( cnt_nsig );

  return 0;
}

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadGridX(0);
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==4) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename (int)board_id (int)dac" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root 2 30" << std::endl
	      << std::endl, abort();

  Char_t* infilename = app.Argv(1);
  Int_t   board_id   = atoi( app.Argv(2) );
  Int_t   dac        = atoi( app.Argv(3) );
  
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".root") );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TText* tex1 = new TText();
  tex1->SetTextColor(2);
  tex1->SetTextSize(0.1);
  TText* tex2 = new TText();
  tex2->SetTextColor(2);
  tex2->SetTextSize(0.05);

  hist_nsig    = new TH1I*[n_chip*n_unit*n_bit];
  hist_nring   = new TH1I*[n_chip*n_unit*n_bit];
  hist_width   = new TH1I*[n_chip*n_unit*n_bit];
  hist_span    = new TH1I*[n_chip*n_unit*n_bit];
  hist_1ch_int = new TH1I*[n_chip*n_unit*n_bit];
  hist_wid_tim = new TH2I*[n_chip*n_unit*n_bit];

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    hist_nsig    [ich] = new TH1I( Form("hist_nsig_%d", ich),   Form("hist_nsig_%d;#signal/events",           ich),  nsig_max,           0,    nsig_max );
    hist_nring   [ich] = new TH1I( Form("hist_nring_%d",ich),   Form("hist_nring_%d;#ringing/signals",        ich), nring_max,           0,   nring_max );
    hist_width   [ich] = new TH1I( Form("hist_width_%d",ich),   Form("hist_width_%d;signal width",            ich), width_max,           0,   width_max );
    hist_span    [ich] = new TH1I( Form("hist_span_%d", ich),   Form("hist_span_%d;span between signals",     ich),        40, span_exp-20, span_exp+20 );
    hist_1ch_int [ich] = new TH1I( Form("hist_1ch_int_%d",ich), Form("hist_1ch_int_%d;Time [bit]",            ich),    n_time,           0,      n_time ); 
    hist_wid_tim [ich] = new TH2I( Form("hist_wid_tim_%d",ich), Form("hist_wid_tim_%d;Time [bit];Width [bit]",ich),    n_time,           0,      n_time, width_max, 0,   width_max );
    hist_nsig    [ich]->SetLineColor(2);
    hist_nring   [ich]->SetLineColor(2);
    hist_width   [ich]->SetLineColor(2);
    hist_span    [ich]->SetLineColor(2);
    hist_1ch_int [ich]->SetLineColor(2);
    hist_nsig    [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","nsig",   ich,dac) );
    hist_nring   [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","nring",  ich,dac) );
    hist_width   [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","width",  ich,dac) );
    hist_span    [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","span",   ich,dac) );
    hist_1ch_int [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","1ch_int",ich,dac) );
    hist_wid_tim [ich]->SetName( Form("hist_%s_board%d_channel%03d_dac%d","wid_tim",ich,dac) );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  if( chain->GetEntries(         Form("tpboard==%d && dac==%d",board_id,dac) )==0 ) return 0;
  TTree* tree = chain->CopyTree( Form("tpboard==%d && dac==%d",board_id,dac) );
  set_readbranch_scan(tree);
  set_readbranch(tree);

  if( fl_message ) printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  hist_1ch = new TH1I*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    hist_1ch[ich]   = new TH1I( Form("hist_1ch_%d",ich),  Form("hist_%d;Time [bit];Channel",ich), n_time,   0,        n_time   );
    hist_1ch[ich]->SetLineColor(2);
    //hist_1ch[ich]->SetLabelSize(0.17,"XY");
  }

  TCanvas* can1 = new TCanvas("can1","can1", 1800, 300 );
  can1->Draw();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){
    // read data
    tree->GetEntry(ievt);
        for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ) hist_1ch[ich]->Reset();
    if( dac         != t_dac     ) continue; // dac value
    if( board_id    != t_tpboard ) continue; // board
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){
      if( board_id   != t_board_v->at(ivec) ) continue; // board
      hist_1ch[multi_ch_map(t_chip_v->at(ivec),t_unit_v->at(ivec),t_bit_v->at(ivec))]->Fill( t_time_v->at(ivec) );
    }

    // calcualte
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      if( hist_1ch[ich]->GetEntries()==0 ){
	hist_nsig[ich]->Fill(0);
	continue;
      }
      
      detect_signal( hist_1ch[ich], ich );

      if( fl_message ) printf( "  [Event:%d,Board:%d,Chip:%d,Channel:%d] %d entries\n",   t_event, board_id, rev_ch_map_chip(ich),rev_ch_map_channel(ich), (Int_t)hist_1ch[ich]->Integral() );
      hist_1ch[ich]->SetTitle( Form("Event:%d, Board:%d, Chip:%d, Channel:%d, %d entries",t_event, board_id, rev_ch_map_chip(ich),rev_ch_map_channel(ich), (Int_t)hist_1ch[ich]->Integral()) );
    }

    // draw
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      if( cnt_show < fl_show ){
	can1->cd(1); hist_1ch[ich]->Draw();
	can1->cd(1);
	tex1->DrawTextNDC( 0.30, 0.92, Form("Event:%d, Board:%d, Chip:%d, Channel:%d, DAC : %d (%d entries)",
					    t_event, board_id, rev_ch_map_chip(ich),rev_ch_map_channel(ich), dac, (Int_t)hist_1ch[ich]->Integral()) );
	can1->Update();
	can1->WaitPrimitive();
	if( cnt_show==1 && fl_save ) can1->Print( Form("pic/%s_board%d_chip%d_channel%03d_dac%d_can1.ps",basename.c_str(),board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac) );
      }
    }
    cnt_show++;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TCanvas* can2 = new TCanvas( "can2", "can2", 1600, 800 );
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    if( fl_save || !fl_batch ){
      can2->Clear();
      can2->SetName ( Form("can_board%d_chip%d_channel%03d_dac%d",board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac) );
      can2->SetTitle( Form("can_board%d_chip%d_channel%03d_dac%d",board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac) );
      
      sty->SetOptStat(1111111);
      can2->Draw();
      can2->Divide(3,2);
      can2->cd(1); hist_nsig   [ich]->Draw();
      can2->cd(2); hist_nring  [ich]->Draw();
      can2->cd(3); hist_width  [ich]->Draw();
      can2->cd(4); hist_span   [ich]->Draw();
      can2->cd(5); hist_1ch_int[ich]->Draw();
      can2->cd(6); hist_wid_tim[ich]->Draw("COLZ");
      can2->cd(4);
      tex2->DrawTextNDC( 0.20, 0.75, Form("Board = %d",        board_id      ) );
      tex2->DrawTextNDC( 0.20, 0.70, Form("Chip = %d",         rev_ch_map_chip   (ich)) );
      tex2->DrawTextNDC( 0.20, 0.65, Form("Channel = %d",      rev_ch_map_channel(ich)) );
      tex2->DrawTextNDC( 0.20, 0.60, Form("DAC = %d",          dac           ) );
      tex2->DrawTextNDC( 0.20, 0.55, Form("span = %d",         th_span       ) );
      tex2->DrawTextNDC( 0.20, 0.50, Form("width = %d",        th_width      ) );
      tex2->DrawTextNDC( 0.20, 0.45, Form("window = %d",       th_window     ) );
      tex2->DrawTextNDC( 0.20, 0.40, Form("eff(before) = %.2f",th_eff_before ) );
      tex2->DrawTextNDC( 0.20, 0.35, Form("eff(after) = %.2f", th_eff_after  ) );
      can2->Update();
      if( fl_save ) can2->Print( Form("pic/%s_board%d_chip%d_channel%03d_dac%d_can2.ps",basename.c_str(),board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac) );
      can2->WaitPrimitive();
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // triming noise data // tmppppp
    Int_t peak_bin = hist_width[ich]->GetMaximumBin();
    for( Int_t ibin=0; ibin<peak_bin-50; ibin++ ) hist_width[ich]->SetBinContent(ibin+1, 0.0);
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Double_t sig_eff    = ( hist_nsig[ich]->Integral() ? (hist_nsig[ich]->GetMean()*hist_nsig[ich]->Integral())/(hist_nsig[ich]->Integral()*nsig_exp) : 0.0 );
    Double_t sig_effE   = ( (hist_nsig[ich]->GetMean()*hist_nsig[ich]->Integral()== 0) ? 0.0 : sig_eff * sqrt( (1-sig_eff)/(hist_nsig[ich]->GetMean()*hist_nsig[ich]->Integral()) ) );
    Double_t ring_prob  = ( (hist_nring[ich]->Integral()==0) ? 0.0 : (Double_t)(hist_nring[ich]->Integral() - hist_nring[ich]->GetBinContent(1))/hist_nring[ich]->Integral() );
    Double_t ring_probE = ( ((Double_t)(hist_nring[ich]->Integral() - hist_nring[ich]->GetBinContent(1))==0) ? 0.0 : ring_prob * sqrt( (1-ring_prob)/((Double_t)(hist_nring[ich]->Integral() - hist_nring[ich]->GetBinContent(1))) ) );
    Double_t sig_width  = hist_width[ich]->GetMean();
    Double_t sig_widthE = hist_width[ich]->GetMeanError(); // hist_width[ich]->GetRMS();
    Double_t span       = hist_span [ich]->GetMean();
    Double_t spanE      = hist_span [ich]->GetMeanError(); // hist_span [ich]->GetRMS();
    std::ofstream outlog(Form("dat_scurve/%s_board%d_chip%d.dat",basename.c_str(),board_id,rev_ch_map_chip(ich)), std::ios::app );
    outlog << std::setw(15) << std::right << dac         << " "
	   << std::setw(15) << std::right << t_vref      << " "
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
	   << std::setw(15) << std::right << rev_ch_map_chip   (ich) << " "
	   << std::setw(15) << std::right << rev_ch_map_channel(ich) << " "
	   << " BOARD"   << board_id                //  board-No
	   << " CHIP"    << rev_ch_map_chip   (ich) //  chip-No
	   << " CHANNEL" << rev_ch_map_channel(ich) //  channel-No
	   << std::endl;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if( fl_save ){
      TFile outfile( Form("pic/%s_board%d_chip%d_channel%03d_dac%d.root",basename.c_str(),board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac), "RECREATE" );
      
      TTree* newtree = new TTree( Form("meta_board%d_chip%d_channel%03d_dac%d",board_id,rev_ch_map_chip(ich),rev_ch_map_channel(ich),dac), "meta for timing" );
      if( tree->GetEntries() ) tree->GetEntry(0);
      newtree->Branch( "dac",    &dac,     "dac/I"   );
      newtree->Branch( "tpchg",  &t_tpchg, "tpchg/F" );
      newtree->Fill();
      
      // To save data size, some histogram might be omitted.
      hist_nsig   [ich]->Write();
      hist_nring  [ich]->Write();
      hist_width  [ich]->Write();
      hist_span   [ich]->Write();
      hist_1ch_int[ich]->Write();
      hist_wid_tim[ich]->Write();
      newtree->Write();
      //can2->Write();
      outfile.Close();
      
      //delete newtree;
    }
  }
  delete can2;
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if( fl_message ) std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  delete tex1;
  delete tex2;
  delete chain;
  delete tree;
  delete hist_1ch;
  delete can1;


  return 0;

}
