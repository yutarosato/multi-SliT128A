#include "setting.h"

const Bool_t fl_batch = !true;
const Bool_t fl_plot  = !true;
// true (one plot per parameter point) for reproducibility check or one-channel scan
// false(one plot per channel) for all-channel scan

Double_t threshold_mip = 0.3; // [MIP]

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetLabelSize(0.04,"y");
  sty->SetTitleOffset(0.9,"y");
  sty->SetPadLeftMargin(0.11);

  if( app.Argc()<3 )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (int) board-id (char*)infilename1 ...." << std::endl
	      << std::endl, abort();
  Int_t   board_id = atoi(app.Argv(1));
  Int_t   dac      = 0;
  const Char_t* treename = "meta";
  Int_t   nfile    = app.Argc()-2;
  std::cout << "nfile = " << nfile << std::endl;
  
  TFile**   rootfile = new TFile*  [nfile];
  TTree**   tree     = new TTree*  [nfile];
  Double_t* par      = new Double_t[nfile];
  TH2I**    h_width  = new TH2I*   [nfile];
  TH2I**    h_evt    = new TH2I*   [nfile];

  TGraphErrors** g_sigeff = new TGraphErrors*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ) g_sigeff[ich] = new TGraphErrors();

  Int_t   t_dac;
  Float_t t_tpchg;

  std::vector<Double_t>* t_sigeff_v;
  std::vector<Double_t>* t_sigeffE_v;

  for( Int_t ifile=0; ifile<nfile; ifile++ ){
    Char_t* infilename = app.Argv(ifile+2);
    std::cout << ifile << " : " << infilename << std::endl;
    rootfile[ifile] = new TFile(infilename);
    tree[ifile] = (TTree*)rootfile[ifile]->Get( treename );
    if( rootfile[ifile]->IsZombie() ) std::cerr << "[ABORT] can not find rootfile for " << infilename << std::endl, abort();
    tree[ifile]->SetBranchAddress("dac",     &t_dac       );
    tree[ifile]->SetBranchAddress("tpchg",   &t_tpchg     );
    //tree[ifile]->SetBranchAddress("sigeff",  &t_sigeff_v  );
    //tree[ifile]->SetBranchAddress("sigeffE", &t_sigeffE_v );
    tree[ifile]->GetEntry(0);
    par[ifile] = t_tpchg; // or t_dac
    //if( t_sigeff_v ->size() != n_chip*n_unit*n_bit ) std::cerr << "[ABORT] Wrong #Channel : " << t_sigeff_v ->size() << std::endl, abort();
    //if( t_sigeffE_v->size() != n_chip*n_unit*n_bit ) std::cerr << "[ABORT] Wrong #Channel : " << t_sigeffE_v->size() << std::endl, abort();

    //for( Int_t ich=0; ich<t_sigeff_v->size(); ich++ ){
      //g_sigeff[ich]->SetPoint     ( g_sigeff[ich]->GetN(),   par[ifile], t_sigeff_v ->at(ich) );
      //g_sigeff[ich]->SetPointError( g_sigeff[ich]->GetN()-1,        0.0, t_sigeffE_v->at(ich) );
    //}
    h_width[ifile] = (TH2I*)rootfile[ifile]->Get( Form("hist_width_board%d_dac%d",board_id,dac) );
    if( h_width[ifile]==NULL ) std::cerr << "[ABORT] can not find histogram for width : " << infilename << std::endl, abort();

    h_evt[ifile] = (TH2I*)rootfile[ifile]->Get( Form("hist_evt_int_board%d_dac%d",board_id,dac) );
    if( h_evt[ifile]==NULL ) std::cerr << "[ABORT] can not find histogram for evt : " << infilename << std::endl, abort();
  }

  THStack** mh_width = new THStack*[n_chip*n_unit*n_bit];
  TH1D*** h_width_1d = new TH1D**[nfile];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ) mh_width[ich] = new THStack();
  for( Int_t ifile=0; ifile<nfile; ifile++ ){
    h_width_1d[ifile] = new TH1D*[n_chip*n_unit*n_bit];
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      h_width_1d[ifile][ich] = new TH1D(*(h_width[ifile]->ProjectionX(Form("_%d",ich),ich+1,ich+1)));
      h_width_1d[ifile][ich]->SetLineColor(ifile+1);
      mh_width[ich]->Add( h_width_1d[ifile][ich] );
    }
  }


  TGraphErrors** g_width  = new TGraphErrors*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    g_width[ich] = new TGraphErrors();
    g_width[ich]->SetTitle(";Injected charge [fC];TOT [#mus]");
    for( Int_t ifile=0; ifile<nfile; ifile++ ){
      g_width[ich]->SetPoint(g_width[ich]->GetN(), par[ifile], h_width_1d[ifile][ich]->GetMean()*bit2time); // to be checked!!!
    }
  }

  //TGraphErrors** g_tot    = new TGraphErrors*[n_chip*n_unit*n_bit];
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Canvas>
  TLegend* leg  = new TLegend( 0.20,0.80,0.85,0.98 );

  TCanvas** can_width = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_width[ichip] = new TCanvas( Form("can_width_%d",ichip),Form("can_width_%d",ichip), 1500, 1050 );
    can_width[ichip]->Divide(8,16);
    can_width[ichip]->Draw();
  }

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    Int_t chip_id = ich/(n_unit*n_bit);
    Int_t lch_id  = ich%(n_unit*n_bit);
    Int_t sel_row    = lch_id%16;
    Int_t sel_column = lch_id/16;
    Int_t sel_pad    = 8*sel_row + sel_column+1;
    can_width[chip_id]->cd(sel_pad);
    mh_width[ich]->Draw("nostack");
  }

  TCanvas** can_linearity = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_linearity[ichip] = new TCanvas( Form("can_linearity_%d",ichip),Form("can_linearity_%d",ichip), 1500, 1050 );
    can_linearity[ichip]->Divide(8,16);
    can_linearity[ichip]->Draw();
  }

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
    Int_t chip_id = ich/(n_unit*n_bit);
    Int_t lch_id  = ich%(n_unit*n_bit);
    Int_t sel_row    = lch_id%16;
    Int_t sel_column = lch_id/16;
    Int_t sel_pad    = 8*sel_row + sel_column+1;
    can_linearity[chip_id]->cd(sel_pad);
    g_width[ich]->Sort();
    g_width[ich]->SetMarkerColor(2);
    g_width[ich]->Draw("AP");
  }
  can_linearity[0]->cd(0);
  g_width[0]->Draw("AP");
  //leg->Draw();

  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  return 0;
}
