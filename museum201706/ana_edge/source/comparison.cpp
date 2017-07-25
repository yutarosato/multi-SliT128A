#include "setting.h"

const Bool_t fl_batch = !true; // should be false for quick check.
const Int_t  fl_message = 0;
const Int_t  fl_mask = 1;


const Int_t board_map[] = {-999,-999,0,-999,-999,1}; // Board#2 -> [0], Board#5 -> [1] 
const Int_t rev_board_map[n_board] = {2,5};

Int_t main( Int_t argc, Char_t** argv ){
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

  const Char_t* infilename = "scan_results/%d.root";
  const Int_t board = 2;
  const Int_t chip = 0;
  //const Int_t channel = 1;
  Int_t nfile = app.Argc()-1;
  TFile** infile = new TFile*[nfile];
  TH1D** hist_width = new TH1D*[nfile];
  for( Int_t ifile=0; ifile<nfile; ifile++ ){
    std::cout << Form(infilename,atoi(app.Argv(ifile+1))) << std::endl;
    infile[ifile] = new TFile( Form(infilename,atoi(app.Argv(ifile+1))) );
    if( infile[ifile]->IsZombie() ) std::cerr << "[ABORT] can not find infile : " << app.Argv(ifile+1) << std::endl;
    std::cout << Form("h_width_b%dc%d",board,chip) << std::endl;
    hist_width[ifile] = (TH1D*)infile[ifile]->Get( Form("h_width_chip_b%dc%d",board,chip) );
    if( hist_width[ifile]->IsZombie() ) std::cerr << "[ABORT] can not find histogram : " << app.Argv(ifile+1) << std::endl;
    hist_width[ifile]->SetLineColor  (ifile+1);
    hist_width[ifile]->SetMarkerColor(ifile+1);
    
    //for( Int_t ibin=0; ibin<10; ibin++ ) hist_width[ifile]->SetBinContent( ibin+1, 0 ); ///* // tmpppppp
    //hist_width[ifile]->Scale( 1.0/hist_width[ifile]->GetMaximum() );
  }

  TCanvas* can = new TCanvas( "can", "can", 500, 400 );
  can->Draw();
  THStack* mh = new THStack();
  for( Int_t ifile=0; ifile<nfile; ifile++ ){
    mh->Add( hist_width[ifile] );
  }
  mh->Draw("nostack");
  
  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();
  
  return 0;
  
}
