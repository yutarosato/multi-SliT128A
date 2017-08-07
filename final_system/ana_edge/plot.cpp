#include "setting.h"

const Bool_t fl_batch   = !true; // should be false for quick check.
const Int_t  fl_message = 1;
const Int_t  fl_show    = 1;

const Int_t  fl_mask = 1;

std::set<Int_t> noisy2;
std::set<Int_t> noisy5;
std::set<Int_t> noisy_channel[n_chip];

const Int_t min_width = 5; // estimated minimum signal width ( for signal estimation )
const Int_t max_width = 50; // [bit]

Int_t main( Int_t argc, Char_t** argv ){
  std::map<Int_t,Int_t> board_map;     // board_id -> index
  std::map<Int_t,Int_t> rev_board_map; // index    -> board_id

  for( Int_t iboard=0; iboard<n_board; iboard++ ){
    board_map.insert    ( std::make_pair(board_list[iboard],iboard            ) );
    rev_board_map.insert( std::make_pair(iboard,            board_list[iboard]) );
  }

  noisy_channel[board_map.at(2)].insert(416);
  noisy_channel[board_map.at(5)].insert(3);

  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->cd();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( app.Argc()< 2 ){
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename" 
	      <<  std::endl
	      << std::endl;

    return 0;
  }

  // READ INPUT FILE
  TFile*  file  = new TFile( "time.root","RECREATE" );
  TChain* chain = new TChain("slit128A");
  for( Int_t ii = 1; ii < app.Argc(); ii++ ){
    Char_t* infilename = app.Argv(ii);
    chain->Add( infilename );
    std::cout << "[INFILE] " << infilename << " : " << std::endl;
  }
  set_readbranch(chain);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MAKE OBJECTS
  TH2D* h_evt    [n_board];
  TH2D* h_evt_int[n_board];
  TH1D* h_nhit   [n_board];
  TH1D* h_time   [n_board][n_chip][n_channel]; 
  TH1D* h_width  [n_board][n_chip][n_channel];

  for( Int_t iboard=0; iboard<n_board; iboard++ ){ // BEGIN BOARD-LOOP
    h_evt    [iboard] = new TH2D( Form("h_evt_b%d",    rev_board_map.at(iboard)), Form("h_evt_%d;Time [#mus];Channel",    rev_board_map.at(iboard)), n_time, 0.0, n_time*bit2time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_evt_int[iboard] = new TH2D( Form("h_evt_int_b%d",rev_board_map.at(iboard)), Form("h_evt_int_%d;Time [#mus];Channel",rev_board_map.at(iboard)), n_time, 0.0, n_time*bit2time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_nhit   [iboard] = new TH1D( Form("h_nhit_b%d",   rev_board_map.at(iboard)), Form("h_nhit_b%d;Channel",  rev_board_map.at(iboard)), n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_nhit   [iboard]->Sumw2();
    for( Int_t ichip = 0; ichip < n_chip; ichip++ ){ // BEGIN CHIP-LOOP
      for( Int_t ichannel = 0; ichannel < n_channel; ichannel++ ){ // BEGIN CHANNEL-LOOP
	h_time [iboard][ichip][ichannel] = new TH1D( Form( "h_time_b%dc%dch%d",rev_board_map.at(iboard),ichip,ichannel), Form( "h_time_b%dc%dch%d;Channel;Time-over-Threshold [#mus]", rev_board_map.at(iboard),ichip,ichannel), n_time_bin, 0,    n_time*bit2time );
	h_width[iboard][ichip][ichannel] = new TH1D( Form("h_width_b%dc%dch%d",rev_board_map.at(iboard),ichip,ichannel), Form("h_width_b%dc%dch%d;Channel;Signal Width [#mus]",rev_board_map.at(iboard),ichip,ichannel),  max_width, 0, max_width*bit2time );
	h_time [iboard][ichip][ichannel]->Sumw2();
	h_width[iboard][ichip][ichannel]->Sumw2();
      } // END CHANNEL-LOOP
    } // END CHIP-LOOP
  } // END BOARD-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TCanvas* can_1evt = new TCanvas("can_1evt","can_1evt",400*n_board,600);
  can_1evt->Divide(n_board,2);
  can_1evt->Draw();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    chain->GetEntry(ievt);
    for( Int_t iboard=0; iboard<n_board; iboard++ ) h_evt[iboard]->Reset();
    if( fl_message && fl_show > cnt_show ) std::cout << "************************ [evtNo = " << t_event << "]********************************" << std::endl
						     << "   Nhit = " << t_ledge_v->size() << std::endl;
    for( Int_t ivec=0; ivec < t_ledge_v->size(); ivec++ ){ // BEGIN HIT-LOOP
      Int_t board    = t_board_v->at(ivec);
      Int_t chip     = t_chip_v ->at(ivec);
      Int_t unit     = t_unit_v ->at(ivec);
      Int_t bit      = t_bit_v  ->at(ivec);
      Int_t lchannel = lchannel_map(       unit, bit );
      Int_t gchannel = gchannel_map( chip, unit, bit );
      if( fl_message>1 ) std::cout << "        " << ivec     << " : " 
				   << "board = " << board    << ", "
				   << "chip = "  << chip     << ", "
				   << "unit = "  << unit     << ", "
				   << "ch = "    << lchannel << ", "
				   << "gch = "   << gchannel << std::endl;

      if( board_map.count(board)==0 ){ // chekc board-id
	std::cerr << "[WARNING] Wrong borad-id : " << board << std::endl;
	continue;
      }

      if( fl_mask && noisy_channel[board_map.at(board)].count(gchannel) ) continue; // remove noisy channel
      h_time [board_map[board]][chip][lchannel]->Fill( t_ledge_v->at(ivec)*bit2time + 1e-5);

      Int_t width = t_tedge_v->at(ivec) - t_ledge_v->at(ivec);
      h_width[board_map[board]][chip][lchannel]->Fill( width*bit2time + 1e-5 );
      Int_t tmp_iwidth = 0;
      while( tmp_iwidth < width ){
	h_evt    [board_map[board]]->Fill( (t_ledge_v->at(ivec)+tmp_iwidth) *bit2time + 1e-5, gchannel );
	h_evt_int[board_map[board]]->Fill( (t_ledge_v->at(ivec)+tmp_iwidth) *bit2time + 1e-5, gchannel );
	tmp_iwidth++;
      }
      if( width > min_width ) h_nhit[board_map[board]]->Fill( gchannel_map(chip,unit,bit) );

    } // End HIT-LOOP

    if( cnt_show++ < fl_show ){
      for( Int_t iboard = 0; iboard < n_board; iboard ++ ){
	can_1evt->cd(iboard+1);
	h_evt[iboard]->SetTitle( Form("Board#%d, evtNo.%d",rev_board_map.at(iboard),t_event) );
	h_evt[iboard]->Draw("COLZ");
	can_1evt->cd(iboard+1+n_board);
	h_evt_int[iboard]->SetTitle( Form("Board#%d, evtNo.%d (integral)",rev_board_map.at(iboard),t_event) );
	h_evt_int[iboard]->Draw("COLZ");
      }
      can_1evt->Update();
      can_1evt->WaitPrimitive();
    }
    cnt_show++;
  } // END EVENT-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH1D* h_time_total  = new TH1D(  "h_time_total",  "h_time_total", n_time_bin, 0,    n_time*bit2time );
  TH1D* h_width_total = new TH1D(  "h_width_total", "h_width_total", max_width, 0, max_width*bit2time );
  TH1D* h_time_board [n_board];
  TH1D* h_width_board[n_board];
  TH1D* h_time_chip  [n_board][n_chip];
  TH1D* h_width_chip [n_board][n_chip];
  TH1D* h_time_unit  [n_board][n_chip][n_unit];
  TH1D* h_width_unit [n_board][n_chip][n_unit];

  for( Int_t iboard=0; iboard<n_board; iboard++ ){  
    h_time_board [iboard] = new TH1D( Form( "h_time_board_b%d",rev_board_map.at(iboard)), Form( "h_time_board_b%d;Time [#mus]",               rev_board_map.at(iboard)), n_time_bin, 0,    n_time*bit2time );
    h_width_board[iboard] = new TH1D( Form("h_width_board_b%d",rev_board_map.at(iboard)), Form("h_width_board_b%d;Time-over-Threshold [#mus]",rev_board_map.at(iboard)),  max_width, 0, max_width*bit2time );
    h_time_board [iboard]->Sumw2();
    h_width_board[iboard]->Sumw2();
    for( Int_t ichip=0; ichip<n_chip; ichip++ ){
      h_time_chip [iboard][ichip] = new TH1D( Form( "h_time_chip_b%dc%d",rev_board_map.at(iboard),ichip), Form( "h_time_chip_b%dc%d;Time [#mus]",               rev_board_map.at(iboard),ichip), n_time_bin, 0,    n_time*bit2time );
      h_width_chip[iboard][ichip] = new TH1D( Form("h_width_chip_b%dc%d",rev_board_map.at(iboard),ichip), Form("h_width_chip_b%dc%d;Time-over-Threshold [#mus]",rev_board_map.at(iboard),ichip),  max_width, 0, max_width*bit2time );
      h_time_chip [iboard][ichip]->Sumw2();
      h_width_chip[iboard][ichip]->Sumw2();
      for( Int_t iunit=0; iunit<n_unit; iunit++ ){
	h_time_unit [iboard][ichip][iunit] = new TH1D( Form( "h_time_unit_b%dc%du%d",rev_board_map.at(iboard),ichip,iunit), Form( "h_time_unit_b%dc%du%d;Time [#mus]", rev_board_map.at(iboard),ichip,iunit), n_time_bin, 0,    n_time*bit2time );
	h_width_unit[iboard][ichip][iunit] = new TH1D( Form("h_width_unit_b%dc%du%d",rev_board_map.at(iboard),ichip,iunit), Form("h_width_unit_b%dc%du%d;Time-over-Threshold [#mus]", rev_board_map.at(iboard),ichip,iunit),  max_width, 0, max_width*bit2time );
	h_time_unit [iboard][ichip][iunit]->Sumw2();
	h_width_unit[iboard][ichip][iunit]->Sumw2();
      }
    }
  }
  
  for( Int_t iboard = 0; iboard < n_board; iboard ++ ){  
    for( Int_t ichip = 0; ichip < n_chip; ichip++ ){
      for( Int_t iunit = 0; iunit < n_channel; iunit++ ){
	h_time_total                             ->Add( h_time [iboard][ichip][iunit] );
	h_width_total                            ->Add( h_width[iboard][ichip][iunit] );
	h_time_board [iboard]                    ->Add( h_time [iboard][ichip][iunit] );
	h_width_board[iboard]                    ->Add( h_width[iboard][ichip][iunit] );
	h_time_chip  [iboard][ichip]             ->Add( h_time [iboard][ichip][iunit] );
	h_width_chip [iboard][ichip]             ->Add( h_width[iboard][ichip][iunit] );
	h_time_unit  [iboard][ichip][iunit/n_bit]->Add( h_time [iboard][ichip][iunit] );
	h_width_unit [iboard][ichip][iunit/n_bit]->Add( h_width[iboard][ichip][iunit] );
      }
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // DRAW
  TCanvas* can[n_board];
  for( Int_t iboard=0; iboard<n_board; iboard++ ){
    can[iboard] = new TCanvas( Form("can_b%d",rev_board_map.at(iboard)), Form("can_b%d",rev_board_map.at(iboard)), 1600, 400 );
    can[iboard]->Divide(4,1);
    can[iboard]->Draw();
  }
  
  for( Int_t iboard = 0; iboard < n_board; iboard++ ){
    can[iboard]->cd(1)->SetLogy();
    h_time_board[iboard]->SetMinimum(1);
    can[iboard]->cd(1); h_time_board [iboard]->Draw("hist");
    can[iboard]->cd(2); h_width_board[iboard]->Draw("hist");
    for( Int_t ichip = 0; ichip < n_chip; ichip++ ){
      ///*
      can[iboard]->cd(1);
      h_time_chip [iboard][ichip]->SetLineColor(ichip+2);
      h_time_chip [iboard][ichip]->Draw("histsame");
      can[iboard]->cd(2);
      h_width_chip[iboard][ichip]->SetLineColor(ichip+2);
      h_width_chip[iboard][ichip]->Draw("histsame");
      //*/
      /*
	for( Int_t iunit = 0; iunit < n_unit; iunit++ ){
	can[iboard]->cd(1);
	h_time_unit [iboard][ichip][iunit]->SetLineColor(ichip+2);
	h_time_unit [iboard][ichip][iunit]->Draw("histsame");
	can[iboard]->cd(2);
	h_width_unit[iboard][ichip][iunit]->SetLineColor(ichip+2);
	h_width_unit[iboard][ichip][iunit]->Draw("histsame");
	}
      */
      for( Int_t iunit = 0; iunit < n_channel; iunit++ ){
      }
    }
  }
  
  for( Int_t iboard = 0; iboard < n_board; iboard++ ){
    can[iboard]->cd(3); h_evt_int[iboard]->Draw("COLZ");      
    can[iboard]->cd(4); h_nhit   [iboard]->Draw("hist");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // SAVE
  for( Int_t iboard=0; iboard<n_board; iboard++ ){
    can[iboard]->Print( Form("time_b%d.eps",rev_board_map.at(iboard)) );
    can[iboard]->Print( Form("time_b%d.png",rev_board_map.at(iboard)) );
  }

  h_time_total ->Write();
  h_width_total->Write();
  for( Int_t iboard=0; iboard<n_board; iboard++ ){
    h_nhit[iboard]->Write();
    for( Int_t ichip=0; ichip<n_chip; ichip++ ){
      h_time_chip [iboard][ichip]->Write();
      h_width_chip[iboard][ichip]->Write();
      for( Int_t iunit=0; iunit<n_channel; iunit++ ){
	h_time [iboard][ichip][iunit]->Write();
	h_width[iboard][ichip][iunit]->Write();
      }
      for( Int_t iunit=0; iunit<n_unit; iunit++ ){
	h_time_unit [iboard][ichip][iunit]->Write();
	h_width_unit[iboard][ichip][iunit]->Write();
      }
    }
  }

  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();
  file->Close();

  delete can_1evt;
  /////
  
  return 0;
  
}
