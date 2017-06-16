#include "setting.h"

const Bool_t   fl_batch = true;
const Double_t ref_chg  = 1.92; // [fC] // reference points for estimation of noise and s-curve
const Int_t    nch      = 128;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch); // tmpppppp
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadLeftMargin (0.15);
  sty->SetPadRightMargin(0.15);
  sty->SetTitleOffset(0.9,"y");
  sty->SetMarkerColor(2);
  sty->SetLineColor(2);
  sty->SetOptFit(1);
  sty->SetMarkerSize(0.4);

  // set palette
  const Int_t ncol = 90;
  const Int_t ncol_step = 9; //                        **
  Double_t r   [ncol_step] = {0.00, 0.05, 0.10, 0.15, 1.00, 1.00, 1.00, 1.00, 1.00};
  Double_t g   [ncol_step] = {0.00, 0.05, 0.10, 0.15, 1.00, 0.15, 0.10, 0.05, 0.00};
  Double_t b   [ncol_step] = {1.00, 0.95, 0.90, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00};
  Double_t stop[ncol_step] = {0.00, 0.02, 0.04, 0.06, 0.50, 0.94, 0.96, 0.98, 1.00};
  //Double_t stop[ncol_step] = {0.00, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 1.0};
  Int_t    base    = TColor::CreateGradientColorTable( ncol_step, stop, r, g, b, ncol );

  Int_t palette[ncol];
  for( Int_t icol=0; icol<ncol; icol++) palette[icol] = base + icol;
  sty->SetPalette( ncol, palette );


  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if( !(app.Argc()>1) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (char*)infilenames ..." << std::endl
	      << std::endl, abort();
  const Int_t nfile     = app.Argc() - 1;

  //< Definition of Objects >

  Int_t    fl_alive [nch] = {0};
  Double_t v_noise  [nch] = {0.0};
  Double_t v_noiseE [nch] = {0.0};
  Double_t v_offset [nch] = {0.0};
  Double_t v_offsetE[nch] = {0.0};
  Double_t v_gain   [nch] = {0.0};
  Double_t v_gainE  [nch] = {0.0};
  Double_t v_ch     [nch]; for( Int_t ich=0; ich<nch; ich++ ) v_ch[ich] = ich;
  Double_t v_chE    [nch] = {0.0};
  
  const Int_t xbin_dac = 248;
  TH2D* hist_scurve = new TH2D("hist_scurve","hist_scurve;Channel;DAC", 128, 0, 128, xbin_dac, -31, 31 );
  
  TGraph* g_chi2 = new TGraph();
  g_chi2->SetTitle("#chi^{2} of linearity;Channel;#chi^{2}");
  
  // < Make a Canvas >
  TCanvas* can0 = new TCanvas("can0", "can0", 1200, 800);
  can0->Divide(2,2);
  can0->cd(1);
  gPad->DrawFrame(0.0, -45, 2.7, 45, ";Injected Charge [fC];Threshold DAC");
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Read Data and Input to TGraphErrors>
  for( Int_t ifile=0; ifile<nfile; ifile++ ){ // BEGIN file-loop
    TFile* infile = new TFile( app.Argv(ifile+1) );
    if( infile->IsZombie() ) std::cerr << "[ABORT] can not find file for " << app.Argv(ifile+1) << std::endl, abort();
    
    TTree* tree = (TTree*)infile->Get( "ch" );
    if( tree==NULL ) std::cerr << "[ABORT] can not find tree in " << app.Argv(ifile+1) << std::endl, abort();
    tree->GetEntry(0);
    Int_t ch = (Int_t)tree->GetLeaf("ch")->GetValue();

    Double_t tmp_div  = 100000;
    Int_t   ref_point = 0;

    for( Int_t ievt=0; ievt<tree->GetEntries(); ievt++ ){
      tree->GetEntry(ievt);
      Double_t tmp_chg = (Double_t)tree->GetLeaf("tpchg")->GetValue();
      if( fabs(tmp_chg-ref_chg) < tmp_div ){
	ref_point = ievt;
	tmp_div = fabs(tmp_chg-ref_chg);
      }
    }

    if( tmp_div > 0.1 ) std::cerr << "[WARNING] reference point is not accurate : ch " << ch << std::endl;
    TGraphErrors* g_gain_vref = (TGraphErrors*)infile->Get( "g_gain_vref_0" );
    if( g_gain_vref==NULL ) std::cerr << "[ABORT] can not find g_gain_vref_0 in " << app.Argv(ifile+1) << std::endl, abort();
    fl_alive [ch] = 1;
    TF1* fit_func = g_gain_vref->GetFunction("pol1");
    if( fit_func==NULL){ std::cerr<< "[WARNING] can not find g_gain_vref_0->GetFunction(pol1) : " << app.Argv(ifile+1) << std::endl; continue; }
    v_gain   [ch] = fit_func->GetParameter(1);
    v_gainE  [ch] = fit_func->GetParError (1);
    //v_offset [ch] = fit_func->GetParameter(0);
    v_offset [ch] = fit_func->GetParameter(0) + fit_func->GetParameter(1)*1.92; // tmpppp
    v_offsetE[ch] = fit_func->GetParError (0);

    //if( fit_func->GetNDF()<=1 || v_gain[ch] < 0 || v_gain[ch] > 10000 || v_gainE[ch] > 10000 ){ // tmpppp
    if( v_gain[ch] < 0 || v_gain[ch] > 10000 || v_gainE[ch] > 50 || v_offsetE[ch] > 50 ){ // tmpppp
      v_gain   [ch] = 0;
      v_gainE  [ch] = 0;
      v_offset [ch] = 0;
      v_offsetE[ch] = 0;
    }

    g_gain_vref->Draw("Psame");
    g_chi2->SetPoint( g_chi2->GetN(), ch, g_gain_vref->GetRMS(2) ); // tmppppppp

    TGraphErrors* g_scurve = (TGraphErrors*)infile->Get( Form("scurve_%d_0", ref_point) );

    if( g_scurve==NULL ) std::cerr << "[ABORT] can not find scurve in " << app.Argv(ifile+1) << std::endl, abort();
    Double_t tmp_gain   = g_gain_vref->GetFunction("pol1")->GetParameter(1);
    Double_t tmp_gainE  = g_gain_vref->GetFunction("pol1")->GetParError (1);
    Double_t tmp_width  = g_scurve->GetFunction( Form("func%d_0",ref_point) )->GetParameter(1);
    Double_t tmp_widthE = g_scurve->GetFunction( Form("func%d_0",ref_point) )->GetParError (1);
    Double_t tmp_noise  = tmp_width/tmp_gain;
    Double_t tmp_noiseE = tmp_noise*sqrt(
					 pow( tmp_widthE/tmp_width, 2 ) + 
					 pow( tmp_gainE /tmp_gain,  2 )
					 );
    if( tmp_noise<0 || tmp_gain==0 || tmp_noise > 1.5 || tmp_noiseE > 1.5){ // tmppppp
      tmp_noise  = 0;
      tmp_noiseE = 0;
    }
    v_noise [ch] = tmp_noise;
    v_noiseE[ch] = tmp_noiseE;

    for( Int_t ip=0; ip<xbin_dac; ip++ ){
      Double_t tmp_x = -31.0+ip*(62.0/xbin_dac);
      Double_t tmp_y = g_scurve->GetFunction( Form("func%d_0",ref_point) )->Eval(tmp_x);
      hist_scurve->Fill( ch, tmp_x, tmp_y );
      if( ip==0 ) hist_scurve->SetTitle( Form("S-curve (%s)",g_scurve->GetTitle()) );
    }
  } // END file-loop

  can0->cd(2);
  hist_scurve->Draw("COLZ");
  can0->cd(3);
  g_chi2->Sort();
  g_chi2->Draw("AP");

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TGraphErrors* g_noise  = new TGraphErrors( nch, v_ch, v_noise,  v_chE, v_noiseE  );
  TGraphErrors* g_offset = new TGraphErrors( nch, v_ch, v_offset, v_chE, v_offsetE );
  TGraphErrors* g_gain   = new TGraphErrors( nch, v_ch, v_gain,   v_chE, v_gainE   );
  g_noise ->SetTitle( Form(     "Noise (Ref. : %.2f fC);Channel;Noise [fC];",   ref_chg) );
  g_offset->SetTitle( Form("Offset DAC (Ref. : %.2f fC);Channel;Offset DAC;",   ref_chg) );
  g_gain  ->SetTitle( Form(      "Gain (Ref. : %.2f fC);Channel;Gain [DAC/fC];",ref_chg) );
  g_noise ->Sort();
  g_offset->Sort();
  g_gain  ->Sort();
  g_noise->SetMinimum(0.0);
  g_gain ->SetMinimum(0.0);

  TH1D* h_noise  = conv_graphtohist( g_noise,  "hist_noise",  "Noise; Noise [fC]"   );
  //TH1D* h_noise  = conv_graphtohist( g_noise,  "hist_noise",  "Noise; Noise [fC]", 40, 0.0, 0.20 ); // tmppppp
  TH1D* h_offset = conv_graphtohist( g_offset, "hist_offset", "Offset; Offset DAC" );
  //TH1D* h_offset = conv_graphtohist( g_offset, "hist_offset", "Offset; Offset DAC", 40, -40, 40 ); // tmpppp
  TH1D* h_gain   = conv_graphtohist( g_gain,   "hist_gain",   "Gain; Gain [DAC/fC]" );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TCanvas* can1 = new TCanvas( "can1","can1", 1600, 800 );
  can1->Divide( 3,2 );
  can1->Draw();

  can1->cd(1);
  g_noise->Draw("AP");
  can1->cd(2);
  g_offset->Draw("AP");
  can1->cd(3);
  g_gain->Draw("AP");


  can1->cd(4);
  h_noise->Draw();
  //h_noise->Fit("gaus","L","same");
  h_noise->Fit("gaus","LR","same",0.050,1.00); // tmpppp
  can1->cd(5);
  h_offset->Print();
  h_offset->Draw();
  h_offset->Fit("gaus","L","same");
  //h_offset->Fit("gaus","LR","same",-300, 0); // tmpppp
  can1->cd(6);
  h_gain->Draw();
  //h_gain->Fit("gaus","L", "same"); 
  h_gain->Fit("gaus","LR","same",5,100); // tmpppp

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::ofstream fout("threshold_calib.dat");
  std::cout << "======================================================================================================" << std::endl
	    << "  ch. alive   noise             offset            gain"                                       << std::endl
	    << "------------------------------------------------------------------------------------------------------" << std::endl;
  for( Int_t ich=0; ich<nch; ich++ ){
    std::cout << std::setw( 5) << std::right << v_ch   [ich] << "   "
	      << std::setw( 3) << std::right << fl_alive[ich] << "   "
	      << std::setw( 8) << std::left  << Form(" %.4f",v_noise  [ich])
	      << std::setw(10) << std::left  << Form(" %.4f",v_noiseE [ich])
	      << std::setw( 8) << std::left  << Form(" %.3f",v_offset [ich])
	      << std::setw(10) << std::left  << Form(" %.3f",v_offsetE[ich])
	      << std::setw( 8) << std::left  << Form(" %.3f",v_gain   [ich])
	      << std::setw(10) << std::left  << Form(" %.3f",v_gainE  [ich])
	      << std::endl;

    fout << std::setw( 5) << std::right << v_ch    [ich] << "   "
	 << std::setw( 3) << std::right << fl_alive[ich] << "   "
	 << std::setw( 8) << std::left  << Form(" %.4f",v_noise  [ich])
	 << std::setw(10) << std::left  << Form(" %.4f",v_noiseE [ich])
	 << std::setw( 8) << std::left  << Form(" %.3f",v_offset [ich])
	 << std::setw(10) << std::left  << Form(" %.3f",v_offsetE[ich])
	 << std::setw( 8) << std::left  << Form(" %.3f",v_gain   [ich])
	 << std::setw(10) << std::left  << Form(" %.3f",v_gainE  [ich])
	 << std::endl;

  }
  can0->Update();
  can0->Print("pic/threshold_calib_can0.eps");
  can0->Print("pic/threshold_calib_can0.root");
  can1->Update();
  can1->Print("pic/threshold_calib_can1.eps");
  can1->Print("pic/threshold_calib_can1.root");

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();
  return 0;
}
