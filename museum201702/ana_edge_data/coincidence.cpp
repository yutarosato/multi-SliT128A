#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
// maximum point by prompt

const Int_t peak_silicon  = 11145;
const Int_t peak_counter1 = 11467;
const Int_t peak_counter2 = 11470;

Int_t t_tdc_s1[1024];
Int_t t_tdc_s2[1024];
Int_t set_readbranch_counter( TTree* tree ){
  tree->SetBranchAddress("tdc_s1", t_tdc_s1 );
  tree->SetBranchAddress("tdc_s2", t_tdc_s2 );
}

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()==3) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0)
	      << " (char*)infilename_silicon (char*)infilename_counter" << std::endl
	      << "[e.g]" << std::endl
	      << app.Argv(0) << " test_silicon.root test_counter.root" << std::endl
	      << std::endl, abort();

  Char_t* infilename_silicon = app.Argv(1);
  Char_t* infilename_counter = app.Argv(2);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TChain* chain_silicon = new TChain("slit128A");
  chain_silicon->Add(infilename_silicon);
  set_readbranch(chain_silicon);

  TChain* chain_counter = new TChain("tree");
  chain_counter->Add(infilename_counter);
  set_readbranch_counter(chain_counter);
  
  printf( "[input] %s : %d entries\n", infilename_silicon, (Int_t)chain_silicon->GetEntries() );
  printf( "[input] %s : %d entries\n", infilename_counter, (Int_t)chain_counter->GetEntries() );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  const Int_t range_low  =  200;
  const Int_t range_high = 1000;
  TH1I* hist_silicon  = new TH1I( "hist_silicon", "Time(silicon);Time [ns]",  (range_low+range_high)/5, peak_silicon -range_low, peak_silicon +range_high );
  TH1I* hist_counter1 = new TH1I( "hist_counter1", "Time(counter);Time [ns]",  range_low+range_high,    peak_counter1-range_low, peak_counter1+range_high );
  TH1I* hist_counter2 = new TH1I( "hist_counter2", "Time(counter);Time [ns]",  range_low+range_high,    peak_counter1-range_low, peak_counter1+range_high );

  hist_counter2->SetLineColor(2);
  TCanvas* can = new TCanvas("can","can", 1400, 600 );
  can->Divide(1,2);
  can->Draw();
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Int_t nevt_silicon = chain_silicon->GetEntries();
  Int_t nevt_counter = chain_counter->GetEntries();
  Int_t nevt = (nevt_silicon < nevt_counter ? nevt_silicon : nevt_counter );
  for( Int_t ievt=0; ievt<nevt; ievt++ ){ // BEGIN EVENT-LOOP
    chain_silicon->GetEntry(ievt);
    chain_counter->GetEntry(ievt);
    for( Int_t ivec=0; ivec<t_unit_v->size(); ivec++ ){ // BEGIN SILICON-LOOP
      Int_t time  = t_ledge_v->at(ivec);
      hist_silicon->Fill( 5*time );
    } // BEGIN SILICON-LOOP
    
    for( Int_t i=0; i<1024; i++ ){ // BEGIN COUNTER-LOOP
      //std::cout << t_tdc_s1[i] << " : " << t_tdc_s2[i] << std::endl;
      hist_counter1->Fill(t_tdc_s1[i]);
      hist_counter2->Fill(t_tdc_s2[i]);
    } // END COUNTER-LOOP
    ///*
      can->cd(1);
      hist_silicon->Draw();
      can->cd(2);
      hist_counter1->Draw();
      hist_counter2->Draw("same");
      can->Update();
      std::cout << "ievt = " << ievt << std::endl;
      can->WaitPrimitive();
      hist_silicon ->Reset();
      hist_counter1->Reset();
      hist_counter2->Reset();
      //*/
  } // END EVENT-LOOP
  
  
  can->cd(1);
  can->cd(1)->SetLogy();
  hist_silicon->Draw();
  can->cd(2);
  can->cd(2)->SetLogy();
  hist_counter1->Draw();
  hist_counter2->Draw("same");
  can->Update();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  return 0;

}
