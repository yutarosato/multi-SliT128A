#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_message = 0;
const Int_t  fl_mask = 1;


std::set<Int_t> noisy2;
std::set<Int_t> noisy5;

const Int_t board_map[] = {-999,-999,0,-999,-999,1}; // Board#2 -> [0], Board#5 -> [1] 
const Int_t rev_board_map[n_board] = {2,5};
const Int_t min_width = 5; // estimated minimum signal width ( for signal estimation )

const Int_t max_hit   = 200; // for histogram
const Int_t max_cluster = 30;
const Int_t count_threshold = 15;
const Double_t b2t = 5e-3; // bit to time (5 ns)
const Int_t muon_time = 1840; // muon injected time (bit)
const Int_t prompt_time   = 1750; // = 8.75/0.005;
const Int_t prompt_window =   30; // = 0.15/0.005;

using namespace std;

Int_t main( Int_t argc, Char_t** argv ){
  
  noisy2.insert(416);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(425);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(457);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(466);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(467);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(468);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(504);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(508);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(509);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy2.insert(511);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!


  noisy5.insert(3);    /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(163);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(177);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(183);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(190);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(200);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(206);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(219);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(235);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(237);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(246);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(253);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(256);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(284);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(301);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(311);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(359);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(385);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(393);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(448);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(502);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  noisy5.insert(511);  /////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadLeftMargin(0.12);
  sty->SetLabelSize(0.04,"xyz");
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

  TFile*  file  = new TFile( "time.root","RECREATE" );
  TChain* chain = new TChain("slit128A");
  for( Int_t ii = 1; ii < app.Argc(); ii++ ){
    Char_t* infilename = app.Argv(ii);
    chain->Add( infilename );
    //    printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );
    std::cout << "[INFILE] " << infilename << " : " << std::endl;
  }

  set_readbranch(chain);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  TH1D* h_pulse_IO = new TH1D( "h_pulse_IO","h_pulse_IO", chain->GetEntries(), 0, chain->GetEntries() );
  TH2D* h_evt  [n_board];
  TH1D* h_nhit [n_board]; // number of hit
  TH1D* h_npos [n_board]; // number of positoron hit (after prompt timing)
  TH1D* h_time [n_board][n_chip][n_channel]; 
  TH1D* h_width[n_board][n_chip][n_channel];

  for( Int_t hh = 0; hh < n_board; hh ++ ){  
    h_evt [hh] = new TH2D( Form("h_evt_b%d", rev_board_map[hh]), Form("h_evt_%d",   rev_board_map[hh]), n_time, 0.0, n_time*b2t, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_nhit[hh] = new TH1D( Form("h_nhit_b%d",rev_board_map[hh]), Form("h_nhit_b%d", rev_board_map[hh]), n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_npos[hh] = new TH1D( Form("h_npos_b%d",rev_board_map[hh]), Form("h_npos_b%d", rev_board_map[hh]), n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
    h_nhit[hh]->Sumw2();
    h_npos[hh]->Sumw2();
    for( Int_t ii = 0; ii < n_chip; ii++ ){
      for( Int_t jj = 0; jj < n_channel; jj++ ){
	h_time [hh][ii][jj] = new TH1D( Form( "h_time_b%dc%dch%d",rev_board_map[hh],ii,jj), Form( "h_time_b%dc%dch%d", rev_board_map[hh],ii,jj), n_time_bin, 0, n_time*b2t );
	h_width[hh][ii][jj] = new TH1D( Form("h_width_b%dc%dch%d",rev_board_map[hh],ii,jj), Form("h_width_b%dc%dch%d", rev_board_map[hh],ii,jj),        150, 0,    150*b2t );
	h_time [hh][ii][jj]->Sumw2();
	h_width[hh][ii][jj]->Sumw2();
      }
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  /*
  TCanvas* can_tmp = new TCanvas("can_tmp","can_tmp",1800,600); // tmpppp
  can_tmp->Divide(2,1); // tmpppp
  can_tmp->Draw(); // tmpppp
  */
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    //for( Int_t hh = 0; hh < n_board; hh ++ ) h_evt[hh]->Reset(); // tmpppppppp
    if( fl_message ) std::cout << "************************ [evtNo = " << t_event << "]********************************" << std::endl;
    
    //-- Initialize --
    Int_t width    = 0;
    Int_t n_hit    = 0;
    Int_t n_pos    = 0;
    Int_t n_prompt = 0;
    //----------------
    if( fl_message ) std::cout << "   Nhit = " << t_ledge_v->size() << std::endl;
    for( Int_t ivec=0; ivec < t_ledge_v->size(); ivec++ ){
      Int_t board   = t_board_v->at(ivec);
      Int_t chip    = t_chip_v ->at(ivec);
      Int_t unit    = t_unit_v ->at(ivec);
      Int_t bit     = t_bit_v  ->at(ivec);
      Int_t ch      = ch_map(unit,bit);
      Int_t gch     = multi_ch_map( chip, unit, bit );
      if( fl_message>1 ) std::cout << "        " << ivec    << " : " 
				   << "board = " << board   << ", "
				   << "chip = "  << chip    << ", "
				   << "unit = "  << unit    << ", "
				   << "ch = "    << ch     << ", "
				   << "gch = "   << gch     << std::endl;
      
      if( board!=2 && board!=5 ){
	std::cerr << "[WARNING] Wrong borad-id : " << board << std::endl;
	continue;
      }

      if(  fl_mask &&
	   (
	    (board==2 && noisy2.count(gch)) ||
	    (board==5 && noisy5.count(gch))
	    )
	   ) continue;
      width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );
      h_time[board_map[board]][chip][ch]->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5);
      int tmp_cnt = 0;
      while( tmp_cnt < width ){
	h_evt [board_map[board]]->Fill( (t_ledge_v->at(ivec)+tmp_cnt) *b2t + 1e-5, gch );
	tmp_cnt++;
      }
      if( width > min_width ){
	n_hit++;
	if( t_ledge_v->at(ivec) > muon_time ) n_pos++;
	if( t_ledge_v->at(ivec) > prompt_time && t_ledge_v->at(ivec) < prompt_time + prompt_window ){
	  n_prompt++;
	}
      }
    } // End Hit-Loop
    if( fl_message ) std::cout << "n_hit(width-cut) = " << n_hit << ", n_prompt = " << n_prompt << std::endl;
    if( n_prompt <= count_threshold ) continue;
    h_pulse_IO->Fill( ievt );

    for( Int_t ivec=0; ivec < t_ledge_v->size(); ivec++ ){
      Int_t board = t_board_v->at(ivec);
      Int_t chip  = t_chip_v ->at(ivec);
      Int_t unit  = t_unit_v ->at(ivec);
      Int_t bit   = t_bit_v  ->at(ivec);
      Int_t ch    = ch_map( unit, bit );
      Int_t gch   = multi_ch_map( chip, unit, bit );
      if( board!=2 && board!=5 ){
	std::cerr << "[WARNING] Wrong borad-id : " << board << std::endl;
	continue;
      }

      if(  fl_mask &&
	   (
	    (board==2 && noisy2.count(gch)) ||
	    (board==5 && noisy5.count(gch))
	    )
	   ) continue;

      width = t_tedge_v->at( ivec ) - t_ledge_v->at( ivec );
      h_time [board_map[board]][chip][ch]->Fill( t_ledge_v->at( ivec ) *b2t + 1e-5);
      h_width[board_map[board]][chip][ch]->Fill( width * b2t + 1e-5 );
      if( width > min_width ){
	h_nhit[board_map[board]]->Fill( multi_ch_map(chip,unit,bit) );
	if( t_ledge_v->at( ivec ) > muon_time ) h_npos[board_map[board]]->Fill( multi_ch_map(chip,unit,bit) );
      }
    }

    /* // tmpppp
    for( Int_t hh = 0; hh < n_board; hh ++ ){
      h_evt[hh]->SetTitle( Form("Board#%d, evtNo.%d",rev_board_map[hh],t_event) );
      can_tmp->cd(hh+1);
      h_evt[hh]->Draw("COLZ");
    }
    can_tmp->Update();
    can_tmp->WaitPrimitive();
    */

  } // End of event loop
  //delete can_tmp; // tmpppp
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH1D* h_time_total  = new TH1D(  "h_time_total",  "h_time_total", n_time_bin, 0, n_time*b2t );
  TH1D* h_width_total = new TH1D( "h_width_total", "h_width_total",        150, 0,    150*b2t );
  TH1D* h_time_board [n_board];
  TH1D* h_width_board[n_board];
  TH1D* h_time_chip  [n_board][n_chip];
  TH1D* h_width_chip [n_board][n_chip];
  TH1D* h_time_unit  [n_board][n_chip][n_unit];
  TH1D* h_width_unit [n_board][n_chip][n_unit];

  for( Int_t hh = 0; hh < n_board; hh ++ ){  
    h_time_board [hh] = new TH1D( Form( "h_time_board_b%d",rev_board_map[hh]), Form( "h_time_board_b%d", rev_board_map[hh]), n_time_bin, 0, n_time*b2t );
    h_width_board[hh] = new TH1D( Form("h_width_board_b%d",rev_board_map[hh]), Form("h_width_board_b%d", rev_board_map[hh]),        150, 0,    150*b2t );
    h_time_board [hh]->Sumw2();
    h_width_board[hh]->Sumw2();
    for( Int_t ii = 0; ii < n_chip; ii++ ){
      h_time_chip [hh][ii] = new TH1D( Form( "h_time_chip_b%dc%d",rev_board_map[hh],ii), Form( "h_time_chip_b%dc%d", rev_board_map[hh],ii), n_time_bin, 0, n_time*b2t );
      h_width_chip[hh][ii] = new TH1D( Form("h_width_chip_b%dc%d",rev_board_map[hh],ii), Form("h_width_chip_b%dc%d", rev_board_map[hh],ii),        150, 0,    150*b2t );
      h_time_chip [hh][ii]->Sumw2();
      h_width_chip[hh][ii]->Sumw2();
      for( Int_t jj = 0; jj < n_unit; jj++ ){
	h_time_unit [hh][ii][jj] = new TH1D( Form( "h_time_unit_b%dc%du%d",rev_board_map[hh],ii,jj), Form( "h_time_unit_b%dc%du%d", rev_board_map[hh],ii,jj), n_time_bin, 0, n_time*b2t );
	h_width_unit[hh][ii][jj] = new TH1D( Form("h_width_unit_b%dc%du%d",rev_board_map[hh],ii,jj), Form("h_width_unit_b%dc%du%d", rev_board_map[hh],ii,jj),        150, 0,    150*b2t );
	h_time_unit [hh][ii][jj]->Sumw2();
	h_width_unit[hh][ii][jj]->Sumw2();
      }
    }
  }
  
  for( Int_t hh = 0; hh < n_board; hh ++ ){  
    for( Int_t ii = 0; ii < n_chip; ii++ ){
      for( Int_t jj = 0; jj < n_channel; jj++ ){
	h_time_total                   ->Add( h_time [hh][ii][jj] );
	h_width_total                  ->Add( h_width[hh][ii][jj] );
	h_time_board [hh]              ->Add( h_time [hh][ii][jj] );
	h_width_board[hh]              ->Add( h_width[hh][ii][jj] );
	h_time_chip  [hh][ii]          ->Add( h_time [hh][ii][jj] );
	h_width_chip [hh][ii]          ->Add( h_width[hh][ii][jj] );
	h_time_unit  [hh][ii][jj/n_bit]->Add( h_time [hh][ii][jj] );
	h_width_unit [hh][ii][jj/n_bit]->Add( h_width[hh][ii][jj] );
      }
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // LOG
  std::ofstream outlog("time.log");
  outlog << h_pulse_IO->Integral() << " "
	 << t_vref_b2 << " " 
	 << t_vref_b5 << " "
	 << endl;
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  TCanvas* can1[n_board];
  for( Int_t hh = 0; hh < n_board; hh++ ){
    can1[hh] = new TCanvas( Form("can%d",hh), Form("can%d",hh), 1600, 800 );
    can1[hh]->Divide(4,2);
    can1[hh]->Draw();
  }
  
  for( Int_t hh = 0; hh < n_board; hh++ ){
    can1[hh]->cd(1)->SetLogy();
    h_time_board[hh]->SetMinimum(1);
    can1[hh]->cd(1); h_time_board [hh]->Draw("hist");
    can1[hh]->cd(2); h_width_board[hh]->Draw("hist");
    for( Int_t ii = 0; ii < n_chip; ii++ ){
      ///*
      can1[hh]->cd(1);
      h_time_chip [hh][ii]->SetLineColor(ii+2);
      h_time_chip [hh][ii]->Draw("histsame");
      can1[hh]->cd(2);
      h_width_chip[hh][ii]->SetLineColor(ii+2);
      h_width_chip[hh][ii]->Draw("histsame");
      //*/
      /*
	for( Int_t jj = 0; jj < n_unit; jj++ ){
	can1[hh]->cd(1);
	h_time_unit [hh][ii][jj]->SetLineColor(ii+2);
	h_time_unit [hh][ii][jj]->Draw("histsame");
	can1[hh]->cd(2);
	h_width_unit[hh][ii][jj]->SetLineColor(ii+2);
	h_width_unit[hh][ii][jj]->Draw("histsame");
	}
      */
      for( Int_t jj = 0; jj < n_channel; jj++ ){
      }
    }
  }
  
  
  for( Int_t hh = 0; hh < n_board; hh++ ){
    can1[hh]->cd(3); h_evt [hh]->Draw("COLZ");      
      can1[hh]->cd(5); h_nhit[hh]->Draw("hist");
      can1[hh]->cd(6); h_npos[hh]->Draw("hist");
      can1[hh]->cd(8); h_pulse_IO->Draw();
  }


  // SAVE
  for( Int_t hh = 0; hh < n_board; hh++ ){
    can1[hh]->Print( Form("time_b%d.eps",rev_board_map[hh]) );
    can1[hh]->Print( Form("time_b%d.png",rev_board_map[hh]) );
  }

  
  h_pulse_IO   ->Write();
  h_time_total ->Write();
  h_width_total->Write();
  for( Int_t hh = 0; hh < n_board; hh++ ){
    h_nhit[hh]->Write();
    h_npos[hh]->Write();
    for( Int_t ii = 0; ii < n_chip; ii++ ){
      h_time_chip [hh][ii]->Write();
      h_width_chip[hh][ii]->Write();
      for( Int_t jj = 0; jj < n_channel; jj++ ){
	h_time [hh][ii][jj]->Write();
	h_width[hh][ii][jj]->Write();
      }
      for( Int_t jj = 0; jj < n_unit; jj++ ){
	h_time_unit [hh][ii][jj]->Write();
	h_width_unit[hh][ii][jj]->Write();
      }
    }
  }

  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();
  file->Close();

  
  return 0;
  
}
