#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_show  = 5;

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==3) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename (int)board_id" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test.root 2" << std::endl
	      << std::endl, abort();

  Char_t* infilename = app.Argv(1);
  Int_t   board_id   = atoi( app.Argv(2) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain = new TChain("slit128A");
  chain->Add(infilename);
  set_readbranch(chain);
  
  printf( "[input] %s : %d entries\n", infilename, (Int_t)chain->GetEntries() );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TH2C* hist     = new TH2C("hist",    "hist;             Time;Channel",n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  TH2C* hist_int = new TH2C("hist_int","hist(integration);Time;Channel",n_time, 0, n_time, n_chip*n_unit*n_bit, 0, n_chip*n_unit*n_bit );
  TCanvas* can = new TCanvas("can","can", 1600, 800 );
  can->Divide(1,2);
  can->Draw();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t cnt_show = 0;
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){
      if( board_id != t_board_v->at(ivec) ) continue; // select specified board
      
      //std::cout << "time = " << t_time_v->at(ivec) << ", board = " << t_board_v->at(ivec) << ", chip = " << t_chip_v->at(ivec) << ", unit = " << t_unit_v->at(ivec) << ", bit = " << t_bit_v->at(ivec) << " -> " << multi_ch_map(t_chip_v->at(ivec),t_unit_v->at(ivec),t_bit_v->at(ivec)) << std::endl;
      hist    ->Fill( t_time_v->at(ivec), multi_ch_map(t_chip_v->at(ivec),t_unit_v->at(ivec),t_bit_v->at(ivec)) );
      hist_int->Fill( t_time_v->at(ivec), multi_ch_map(t_chip_v->at(ivec),t_unit_v->at(ivec),t_bit_v->at(ivec)) );
    }


    if( fl_show>cnt_show ){
      printf( "  [Event:%d,Board%d] %d entries\n", t_event, board_id, (Int_t)hist->Integral() );
      hist    ->SetTitle( Form("Event : %d, Board : %d, %d entries", t_event,board_id, (Int_t)hist->Integral()) );
      hist_int->SetTitle( Form("Board : %d, Integration of %d events", board_id, cnt_show) );
      can->cd(1);
      hist->Draw("COLZ");
      can->cd(2);
      hist_int->Draw("COLZ");
      can->Update();
      can->WaitPrimitive();
      hist->Reset();
    }
    cnt_show++;
  }
  can->cd(2);
  hist_int->SetTitle( Form("Board : %d, Integration of %d events", board_id, cnt_show) );
  hist_int->Draw("COLZ");
  can->Update();
  can->WaitPrimitive();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  return 0;

}
