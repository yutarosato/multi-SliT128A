#include "setting.h"

const Bool_t fl_batch = !true;
const Bool_t fl_plot  = !true;
// true (one plot per parameter point) for reproducibility check or one-channel scan
// false(one plot per channel) for all-channel scan
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(fl_batch);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();
  sty->SetPadLeftMargin(0.16);
  sty->SetLabelSize(0.04,"y");
  sty->SetTitleOffset(1.2,"y");

  if( !(app.Argc()==3) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (char*)infilename (int)board" << std::endl
	      << std::endl, abort();
  Char_t* infilename = app.Argv(1);
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".root") );
  Int_t  board_id = atoi( app.Argv(2) );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Read ROOT file>
  TChain* chain           = new TChain( "scurve"     );
  TChain* chain_tab       = new TChain( "vref_tpchg" );
  TChain* chain_tab_vref  = new TChain( "vref"       );
  TChain* chain_tab_tpchg = new TChain( "tpchg"      );
  chain          ->Add( infilename );
  chain_tab      ->Add( infilename );
  chain_tab_vref ->Add( infilename );
  chain_tab_tpchg->Add( infilename );
  std::cout << chain          ->GetEntries() << " entries : "
	    << chain_tab_vref ->GetEntries() << " vref-points * "
	    << chain_tab_tpchg->GetEntries() << " tpchg-points -> "
	    << chain_tab      ->GetEntries() << " points"
	    << std::endl;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TLatex* tex1 = new TLatex();
  tex1->SetTextColor(2);
  tex1->SetTextSize(0.03);
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make List Tables>
  Int_t ntab = (Int_t)chain_tab->GetEntries();
  Double_t* vref_tab  = new Double_t[ntab];
  Double_t* tpchg_tab = new Double_t[ntab];

  for( Int_t itab=0; itab<ntab; itab++ ){
    chain_tab->GetEntry(itab);
    vref_tab [itab] = chain_tab->GetLeaf( "vref"  )->GetValue();
    tpchg_tab[itab] = chain_tab->GetLeaf( "tpchg" )->GetValue();
  }

  Int_t ntab_vref = (Int_t)chain_tab_vref->GetEntries();
  Double_t* vref_tab_single  = new Double_t[ntab_vref];

  for( Int_t itab=0; itab<ntab_vref; itab++ ){
    chain_tab_vref->GetEntry(itab);
    vref_tab_single[itab] = chain_tab_vref->GetLeaf( "vref" )->GetValue();
  }

  Int_t ntab_tpchg = (Int_t)chain_tab_tpchg->GetEntries();
  Double_t* tpchg_tab_single  = new Double_t[ntab_tpchg];

  for( Int_t itab=0; itab<ntab_tpchg; itab++ ){
    chain_tab_tpchg->GetEntry(itab);
    tpchg_tab_single[itab] = chain_tab_tpchg->GetLeaf( "tpchg" )->GetValue();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Objects>
  std::vector<std::vector<std::vector<TGraphErrors*> > > g_scurve;
  std::vector<std::vector<std::vector<TF1*         > > > func;
  std::vector<std::vector<std::vector<Int_t        > > > fl_fit;

  std::vector<std::vector<std::vector<TGraphErrors*> > > g_width;
  std::vector<std::vector<std::vector<TGraphErrors*> > > g_nring;
  std::vector<std::vector<std::vector<TGraphErrors*> > > g_span;
  for( Int_t itab=0; itab<ntab; itab++ ){
    std::vector<std::vector<TGraphErrors*> > tmp_g_scurve1;
    std::vector<std::vector<TF1*         > > tmp_func1;
    std::vector<std::vector<Int_t        > > tmp_fl_fit1;
    
    std::vector<std::vector<TGraphErrors*> > tmp_g_width1;
    std::vector<std::vector<TGraphErrors*> > tmp_g_nring1;
    std::vector<std::vector<TGraphErrors*> > tmp_g_span1;
    
    g_scurve.push_back( tmp_g_scurve1 );
    func.push_back    ( tmp_func1     );
    fl_fit.push_back  ( tmp_fl_fit1   );
    g_width.push_back ( tmp_g_width1  );
    g_nring.push_back ( tmp_g_nring1  );
    g_span.push_back  ( tmp_g_span1   );
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      std::vector<TGraphErrors*> tmp_g_scurve2;
      std::vector<TF1*         > tmp_func2;
      std::vector<Int_t        > tmp_fl_fit2;
      std::vector<TGraphErrors*> tmp_g_width2;
      std::vector<TGraphErrors*> tmp_g_nring2;
      std::vector<TGraphErrors*> tmp_g_span2;
      g_scurve.at(itab).push_back( tmp_g_scurve2 );
      func.at    (itab).push_back( tmp_func2     );
      fl_fit.at  (itab).push_back( tmp_fl_fit2   );
      g_width.at (itab).push_back( tmp_g_width2  );
      g_nring.at (itab).push_back( tmp_g_nring2  );
      g_span.at  (itab).push_back( tmp_g_span2   );
    }
  }

  std::vector<std::vector<TGraphErrors*> > g_gain_vref;
  std::vector<std::vector<TGraphErrors*> > g_gain_tpchg;
  for( Int_t itab=0; itab<ntab_vref; itab++ ){
    std::vector<TGraphErrors*> tmp_g_gain_vref1;
    g_gain_vref.push_back ( tmp_g_gain_vref1  );

    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      TGraphErrors* tmp_g_gain_vref2 = new TGraphErrors();
      tmp_g_gain_vref2->SetName ( Form("g_gain_vref_%d",itab) );
      tmp_g_gain_vref2->SetTitle( Form("g_gain_vref_%d;TP charge [fC];Threshold DAC",itab) );
      g_gain_vref.at(itab).push_back( tmp_g_gain_vref2 );
    }
  }

  for( Int_t itab=0; itab<ntab_tpchg; itab++ ){
    std::vector<TGraphErrors*> tmp_g_gain_tpchg1;
    g_gain_tpchg.push_back( tmp_g_gain_tpchg1 );
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      TGraphErrors* tmp_g_gain_tpchg2 = new TGraphErrors();
      tmp_g_gain_tpchg2->SetName ( Form("g_gain_tpchg_%d",itab) );
      tmp_g_gain_tpchg2->SetTitle( Form("g_gain_tpchg_%d;VREF [mV];Threshold DAC",itab) );
      g_gain_tpchg.at(itab).push_back( tmp_g_gain_tpchg2 );
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Input Data to Objects>
  Int_t    dac_before   =  999;
  Double_t tpchg_before = -999;
  Double_t vref_before  = -999;
  Int_t**   cnt_g = new Int_t*[ntab];
  for( Int_t itab=0; itab<ntab; itab++ ){
    cnt_g[itab] = new Int_t[n_chip*n_unit*n_bit];
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      cnt_g[itab][ich] = 0;
    }
  }

  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){ // BEGIN EVENT-LOOP
    chain->GetEntry(ievt);
    Float_t tpchg     = chain->GetLeaf( "tpchg"     )->GetValue();
    Float_t vref      = chain->GetLeaf( "vref"      )->GetValue();
    Int_t   dac       = chain->GetLeaf( "dac"       )->GetValue();
    Float_t sigeff    = chain->GetLeaf( "sigeff"    )->GetValue();
    Float_t sigeffE   = chain->GetLeaf( "sigeffE"   )->GetValue();
    Float_t ringprob  = chain->GetLeaf( "ringprob"  )->GetValue();
    Float_t ringprobE = chain->GetLeaf( "ringprobE" )->GetValue();
    Float_t sigwidth  = chain->GetLeaf( "sigwidth"  )->GetValue();
    Float_t sigwidthE = chain->GetLeaf( "sigwidthE" )->GetValue();
    Float_t span      = chain->GetLeaf( "span"      )->GetValue();
    Float_t spanE     = chain->GetLeaf( "spanE"     )->GetValue();
    Int_t   board     = chain->GetLeaf( "board"     )->GetValue();
    Int_t   chip      = chain->GetLeaf( "chip"      )->GetValue();
    Int_t   lchannel  = chain->GetLeaf( "channel"   )->GetValue();
    Int_t   gchannel  = gchannel_map( chip,lchannel );
    
    for( Int_t itab=0; itab<ntab; itab++ ){
      if( fabs(vref_tab[itab] - vref)  < 0.001 && fabs(tpchg_tab[itab] - tpchg) < 0.001 ){ // to be checked !!!
	if( dac_before > dac ){ // Make New Objects
	  TGraphErrors* tmp_g_scurve = new TGraphErrors();
	  g_scurve[itab][gchannel].push_back( tmp_g_scurve );

	  TF1* tmp_func = new TF1( Form("func%d_%d_%d",itab,gchannel,cnt_g[itab][gchannel]), "0.5*TMath::Erf(([0]-x)/sqrt(2)/[1])+0.5", -31,31 ); // 0(mean), 1(sigma)
	  tmp_func->SetParameter(0, 0.0);
	  tmp_func->SetParameter(1, 1.9);
	  tmp_func->SetParNames( "mean", "sigma");
	  tmp_func->SetParLimits( 0, -40, 40 );
	  tmp_func->SetParLimits( 1, 0.0, 20 );
	  func[itab][gchannel].push_back( tmp_func );

	  TGraphErrors* tmp_g_width = new TGraphErrors();
	  tmp_g_width->SetTitle("Signal Width;DAC;Width;");
	  g_width[itab][gchannel].push_back( tmp_g_width );

	  TGraphErrors* tmp_g_nring = new TGraphErrors();
	  tmp_g_nring->SetTitle("Ringing Probability;DAC;P_{ringing};");
	  g_nring[itab][gchannel].push_back( tmp_g_nring );

	  TGraphErrors* tmp_g_span = new TGraphErrors();
	  tmp_g_span->SetTitle("Span;DAC;Span;");
	  g_span[itab][gchannel].push_back( tmp_g_span );

	  cnt_g[itab][gchannel]++;
	} // End of <Make New Objects>

	// Input Data
	g_scurve[itab][gchannel][cnt_g[itab][gchannel]-1]->SetPoint     ( g_scurve[itab][gchannel][cnt_g[itab][gchannel]-1]->GetN(),   dac, sigeff    );
	g_scurve[itab][gchannel][cnt_g[itab][gchannel]-1]->SetPointError( g_scurve[itab][gchannel][cnt_g[itab][gchannel]-1]->GetN()-1,   0, sigeffE   );
	g_width [itab][gchannel][cnt_g[itab][gchannel]-1]->SetPoint     ( g_width [itab][gchannel][cnt_g[itab][gchannel]-1]->GetN(),   dac, sigwidth  );
	g_width [itab][gchannel][cnt_g[itab][gchannel]-1]->SetPointError( g_width [itab][gchannel][cnt_g[itab][gchannel]-1]->GetN()-1,   0, sigwidthE );
	g_nring [itab][gchannel][cnt_g[itab][gchannel]-1]->SetPoint     ( g_nring [itab][gchannel][cnt_g[itab][gchannel]-1]->GetN(),   dac, ringprob  );
	g_nring [itab][gchannel][cnt_g[itab][gchannel]-1]->SetPointError( g_nring [itab][gchannel][cnt_g[itab][gchannel]-1]->GetN()-1,   0, ringprobE );
	if( span ){
	  g_span[itab][gchannel][cnt_g[itab][gchannel]-1]->SetPoint     ( g_span[itab][gchannel][cnt_g[itab][gchannel]-1]->GetN(),   dac, span  );
	  g_span[itab][gchannel][cnt_g[itab][gchannel]-1]->SetPointError( g_span[itab][gchannel][cnt_g[itab][gchannel]-1]->GetN()-1,   0, spanE );
	}
	vref_before  = vref_tab [itab];
	tpchg_before = tpchg_tab[itab];
	break;
      }
    }
    dac_before = dac;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Canvas>
  TLegend* leg  = new TLegend( 0.20,0.80,0.85,0.98 );
  const Int_t npad = 12;

  TCanvas** can_scurve = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_scurve[ichip] = new TCanvas( Form("can_scurve_%d",ichip),Form("can_scurve_%d",ichip), 1500, 1050 );
    can_scurve[ichip]->Divide(8,16);
    can_scurve[ichip]->Draw();
  }

  TCanvas** can_width = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_width[ichip] = new TCanvas( Form("can_width_%d",ichip),Form("can_width_%d",ichip), 1500, 1050 );
    can_width[ichip]->Divide(8,16);
    can_width[ichip]->Draw();
  }

  TCanvas** can_nring = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_nring[ichip] = new TCanvas( Form("can_nring_%d",ichip),Form("can_nring_%d",ichip), 1500, 1050 );
    can_nring[ichip]->Divide(8,16);
    can_nring[ichip]->Draw();
  }

  TCanvas** can_span = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_span[ichip] = new TCanvas( Form("can_span_%d",ichip),Form("can_span_%d",ichip), 1500, 1050 );
    can_span[ichip]->Divide(8,16);
    can_span[ichip]->Draw();
  }
  
  // <Draw Objects and Fit S-curve>

  Bool_t fl_first_width = true;
  Bool_t fl_first_nring = true;
  Bool_t fl_first_span  = true;
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    for( Int_t itab=0; itab<ntab; itab++ ){ // BEGIN TABLE-LOOP
      Int_t chip_id = ich/(n_unit*n_bit);
      Int_t lch_id  = ich%(n_unit*n_bit);
      Int_t sel_row    = lch_id%16;
      Int_t sel_column = lch_id/16;
      Int_t sel_pad    = 8*sel_row + sel_column+1;
      can_scurve[chip_id]->cd(sel_pad);
      if( itab==0 ) gPad->DrawFrame( -32, 0.0, 32, 1.2, ";DAC;Count Efficiency" );
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_scurve[itab][ich][ig]->GetN() ){
	  g_scurve[itab][ich][ig]->Sort();
	  g_width [itab][ich][ig]->Sort();
	  g_nring [itab][ich][ig]->Sort();
	  can_scurve[chip_id]->cd(sel_pad);
	  g_scurve[itab][ich][ig]->SetLineColor  ( itab+1 );
	  g_scurve[itab][ich][ig]->SetMarkerColor( itab+1 );
	  g_scurve[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_scurve[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  g_scurve[itab][ich][ig]->Draw("Psame");
	  if( ich==0 ) leg->AddEntry( g_scurve[itab][ich][ig], Form("VREF=%.2f, TPCHG=%.2f", vref_tab[itab], tpchg_tab[itab]), "PL" );

	  // +++++++++++ trimming data(begin) +++++++++++
	  Bool_t fl_th_high = false;
	  Int_t  init_mu = -31;
	  for( Int_t ip1=g_scurve[itab][ich][ig]->GetN()-1; ip1>=0; ip1-- ){
	    if( g_scurve[itab][ich][ig]->GetY()[ip1] > 0.999 ) fl_th_high = true;
	    if( g_scurve[itab][ich][ig]->GetY()[ip1] > 0.50 && init_mu <= -31 ) init_mu = g_scurve[itab][ich][ig]->GetX()[ip1]; // initial value of mu
	    if( fl_th_high && g_scurve[itab][ich][ig]->GetY()[ip1] < 0.96 ){
	      for( Int_t ip2=ip1; ip2>=0; ip2-- ){
		g_scurve[itab][ich][ig]->RemovePoint(ip2);
		g_width [itab][ich][ig]->RemovePoint(ip2);
		g_nring [itab][ich][ig]->RemovePoint(ip2);
	      }
	      for( Int_t ip2=g_span[itab][ich][ig]->GetN()-1; ip2>=0; ip2-- ){
		if( g_span[itab][ich][ig]->GetX()[ip2] <= g_scurve[itab][ich][ig]->GetX()[ip1] ) g_span[itab][ich][ig]->RemovePoint(ip2);
	      }
	      break;
	    }
	  }
	  // +++++++++++ trimming data(end) +++++++++++
	  // +++++++++++ iterative fit(begin) +++++++++++
	  func[itab][ich][ig]->SetParameter( 0, init_mu );
	  func[itab][ich][ig]->SetParameter( 1,     2.5 );
	  for( Int_t ipar1=0; ipar1<func[itab][ich][ig]->GetNpar(); ipar1++ ){
	    func[itab][ich][ig]->ReleaseParameter(ipar1);
	    for( Int_t ipar2=0; ipar2<func[itab][ich][ig]->GetNpar(); ipar2++ ){
	      if( ipar1==ipar2 ) continue;
	      func[itab][ich][ig]->FixParameter(ipar2, func[itab][ich][ig]->GetParameter(ipar2) );
	    }
	    func[itab][ich][ig]->SetParLimits( 0, -40, 40 );
	    func[itab][ich][ig]->SetParLimits( 1,   0, 20 );
	    g_scurve[itab][ich][ig]->Fit( func[itab][ich][ig],"SQ0" );
	  }
	  for( Int_t ipar=0; ipar<func[itab][ich][ig]->GetNpar(); ipar++ ) func[itab][ich][ig]->ReleaseParameter(ipar);
	  // +++++++++++ iterative fit(end) +++++++++++
	  func[itab][ich][ig]->SetParLimits( 0, -40, 40 );
	  func[itab][ich][ig]->SetParLimits( 1,   0, 20 );
	  TFitResultPtr fit_result = g_scurve[itab][ich][ig]->Fit( func[itab][ich][ig], "SQ0" );
	  // +++++++++++ trimming data(begin) +++++++++++
	  for( Int_t ip1=g_scurve[itab][ich][ig]->GetN()-1; ip1>=0; ip1-- ){
	    if( func[itab][ich][ig]->GetParameter(0) < g_scurve[itab][ich][ig]->GetX()[ip1] ) continue;
	    if( g_scurve[itab][ich][ig]->GetY()[ip1]==0 ){ // data point with eff = 0 at invarid region should be removed.
	      for( Int_t ip2=g_span[itab][ich][ig]->GetN()-1; ip2>=0; ip2-- ){
		if( g_span[itab][ich][ig]->GetX()[ip2] <= g_scurve[itab][ich][ig]->GetX()[ip1] ) g_span[itab][ich][ig]->RemovePoint(ip2);
	      }
	      for( Int_t ip2=ip1; ip2>=0; ip2-- ){
		g_scurve[itab][ich][ig]->RemovePoint(ip2);
		g_width [itab][ich][ig]->RemovePoint(ip2);
		g_nring [itab][ich][ig]->RemovePoint(ip2);
	      }
	      break;
	    }
	  }
	  func[itab][ich][ig]->SetLineColor(itab+1);
	  if( g_scurve[itab][ich][ig]->GetN() ){
	    fit_result = g_scurve[itab][ich][ig]->Fit( func[itab][ich][ig], "SQ0", "same" ); // final fit
	  // +++++++++++ trimming data(end) +++++++++++
	  fl_fit[itab][ich].push_back( fit_result->Status() ); // 0(success), other(false), 4(call limit)
	  }else{
	    fl_fit[itab][ich].push_back(-999);
	  }
	}else{
	  fl_fit[itab][ich].push_back(-999); // 0(success), other(false), 4(call limit)
	}
      }

      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_width[itab][ich][ig]->GetN() ){
	  can_width[chip_id]->cd(sel_pad);
	  g_width[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_width[itab][ich][ig]->SetLineColor  ( itab+1 );
	  g_width[itab][ich][ig]->SetMarkerColor( itab+1 );
	  g_width[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_width[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  g_width[itab][ich][ig]->Draw( fl_first_width ? "AP" : "Psame" );
	  fl_first_width = false;
	}
      }

      
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_nring[itab][ich][ig]->GetN() ){
	  can_nring[chip_id]->cd(sel_pad);
	  g_nring[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_nring[itab][ich][ig]->SetLineColor  ( itab+1 );
	  g_nring[itab][ich][ig]->SetMarkerColor( itab+1 );
	  g_nring[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_nring[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  g_nring[itab][ich][ig]->Draw( fl_first_nring ? "AP" : "Psame" );
	  fl_first_nring = false;
	}
      }
      
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_span[itab][ich][ig]->GetN() ){
	  can_span[chip_id]->cd(sel_pad);
	  g_span[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_span[itab][ich][ig]->SetLineColor  ( itab+1 );
	  g_span[itab][ich][ig]->SetMarkerColor( itab+1 );
	  g_span[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_span[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  g_span[itab][ich][ig]->Draw( fl_first_span ? "AP" : "Psame" );
	  fl_first_span = false;
	}
      }

    } // END TABLE-LOOP
  } // END CHANNEL-LOOP
  
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_scurve[ichip]->cd(n_unit*n_bit);
    leg->Draw();
    can_scurve[ichip]->Update();
    can_scurve[ichip]->Print( Form("pic/%s_scurve_%d.ps",basename.c_str(),ichip) );
  }
  can_scurve[3]->WaitPrimitive();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Plots of Threshold value v.s. VREF (or TP charge)>
  for( Int_t itab=0; itab<ntab; itab++ ){ // BEGIN TABLE-LOOP
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( fl_fit[itab][ich][ig] ) continue;
	if( func[itab][ich][ig]->GetParameter(0) < -37 || func[itab][ich][ig]->GetParameter(0) > 37 ){
	  std::cout << Form("[WARNING] under/over-flow in chip#%d, channel%d, vref=%.2f, tpchg=%.2f", gchannel2chip(ich),gchannel2lchannel(ich),vref_tab[itab],tpchg_tab[itab]) << std::endl;       
	  continue;
	}

	for( Int_t itab_vref=0; itab_vref<ntab_vref; itab_vref++ ){
	  if( fabs(vref_tab[itab] - vref_tab_single[itab_vref]) < 0.001 ){
	    g_gain_vref[itab_vref][ich]->SetPoint     ( g_gain_vref[itab_vref][ich]->GetN(),   tpchg_tab[itab], func[itab][ich][ig]->GetParameter(0) );
	    g_gain_vref[itab_vref][ich]->SetPointError( g_gain_vref[itab_vref][ich]->GetN()-1,               0, func[itab][ich][ig]->GetParError (0) );
	    break;
	  }
	}

	for( Int_t itab_tpchg=0; itab_tpchg<ntab_tpchg; itab_tpchg++ ){
	  if( fabs(tpchg_tab[itab] - tpchg_tab_single[itab_tpchg]) < 0.001 ){
	    g_gain_tpchg[itab_tpchg][ich]->SetPoint     ( g_gain_tpchg[itab_tpchg][ich]->GetN(),   vref_tab[itab], func[itab][ich][ig]->GetParameter(0) );
	    g_gain_tpchg[itab_tpchg][ich]->SetPointError( g_gain_tpchg[itab_tpchg][ich]->GetN()-1,              0, func[itab][ich][ig]->GetParError (0) );
	    break;
	  }
	}
      }
    } // END CHANNEL-LOOP
  } // END TABLE-LOOP

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Draw>
  TCanvas* can_vref = new TCanvas("can_vref","can_vref", 1500,1050 );
  can_vref->Divide(8,16);
  can_vref->Draw();

  TCanvas* can_tpchg = new TCanvas("can_tpchg","can_tpchg", 1500,1050 );
  can_tpchg->Divide(8,16);
  can_tpchg->Draw();

  TLegend* leg_vref = new TLegend( 0.30,0.70,0.50,0.95 );
  leg_vref->SetHeader("VREF");
  TMultiGraph** mg_vref = new TMultiGraph*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    mg_vref[ich] = new TMultiGraph();
    mg_vref[ich]->SetTitle( ";TP charge [fC];Threshold DAC" );
    //mg_vref[ich]->SetMinimum(-20.0);
    //mg_vref[ich]->SetMaximum( 20.0);
    for( Int_t itab=0; itab<ntab_vref; itab++ ){ // BEGIN TABLE-LOOP
      g_gain_vref[itab][ich]->SetLineColor  (itab+1);
      g_gain_vref[itab][ich]->SetMarkerColor(itab+1);
      if( ich==0 ) leg_vref->AddEntry( g_gain_vref[itab][ich],Form("%.2f mV",vref_tab_single[itab]) ,"PL" );
      if( g_gain_vref[itab][ich]->GetN()==0 ) continue;
      g_gain_vref[itab][ich]->Sort();
      if( g_gain_vref[itab][ich]->GetN()>1 ) g_gain_vref[itab][ich]->Fit("pol1","Q");
      mg_vref[ich]->Add( g_gain_vref[itab][ich] );
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP

  TLegend* leg_tpchg = new TLegend( 0.65,0.70,0.85,0.95 );
  leg_tpchg->SetHeader("TP charge");
  TMultiGraph** mg_tpchg = new TMultiGraph*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    mg_tpchg[ich] = new TMultiGraph();
    mg_tpchg[ich]->SetTitle(";VREF [mV];Threshold DAC");
    mg_tpchg[ich]->SetMinimum(-20.0);
    mg_tpchg[ich]->SetMaximum( 20.0);
    for( Int_t itab=0; itab<ntab_tpchg; itab++ ){ // BEGIN TABLE-LOOP
      g_gain_tpchg[itab][ich]->SetLineColor  (itab+1);
      g_gain_tpchg[itab][ich]->SetMarkerColor(itab+1);
      if( ich==0 ) leg_tpchg->AddEntry( g_gain_tpchg[itab][ich],Form("%.2f mV",tpchg_tab_single[itab]) ,"PL" );
      if( g_gain_tpchg[itab][ich]->GetN()==0 ) continue;
      g_gain_tpchg[itab][ich]->Sort();
      if( g_gain_tpchg[itab][ich]->GetN()>1 ) g_gain_tpchg[itab][ich]->Fit("pol1","Q");
      mg_tpchg[ich]->Add( g_gain_tpchg[itab][ich] );
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP  
    Int_t chip_id = ich/(n_unit*n_bit);
    Int_t lch_id  = ich%(n_unit*n_bit);
    Int_t sel_row    = lch_id%16;
    Int_t sel_column = lch_id/16;
    Int_t sel_pad    = 8*sel_row + sel_column+1;


    // VREF
    can_vref->cd(sel_pad);
    //if( mg_vref[ich]->GetListOfGraphs()->GetSize() ){
    if( mg_vref[ich]->GetListOfGraphs()!=0 ){
      mg_vref[ich]->Draw("APL");
      //mg_vref[ich]->GetXaxis()->SetLimits( 0.0, mg_vref->GetXaxis()->GetXmax() );
      //mg_vref[ich]->SetMaximum(XXX);
      //mg_vref[ich]->SetMinimum(XXX);
      //mg_vref[ich]->Draw("APL");
    }
    if( ich==n_chip*n_unit*n_bit-1 ) leg_vref->Draw();
    
    // TPCHG
    can_tpchg->cd(sel_pad);
    //if( mg_tpchg[ich]->GetListOfGraphs()->GetSize() ){
    if( mg_tpchg[ich]->GetListOfGraphs() ){
      mg_tpchg[ich]->Draw("APL");
    }
    if( ich==n_chip*n_unit*n_bit-1 ) leg_tpchg->Draw();
  } // END CHANNEL-LOOP

  can_vref ->Update();
  can_tpchg->Update();
  can_vref ->Print( Form("pic/%s_vref.ps", basename.c_str()) );
  can_tpchg->Print( Form("pic/%s_tpchg.ps",basename.c_str()) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TFile outfile( Form("pic/%s.root",basename.c_str()), "RECREATE" );
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    for( Int_t itab=0; itab<ntab; itab++ ){ // BEGIN TABLE-LOOP
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	g_scurve[itab][ich][ig]->SetName ( Form("scurve_%d_%d_%d", itab,ich,ig) );
	g_scurve[itab][ich][ig]->SetTitle( Form("vref=%.2f, tpchg=%.2f, %d", vref_tab[itab], tpchg_tab[itab],ig) );
	g_scurve[itab][ich][ig]->GetXaxis()->SetTitle("DAC [bit]");
	g_scurve[itab][ich][ig]->GetYaxis()->SetTitle("Count Efficiency");
	g_scurve[itab][ich][ig]->Write();
	//std::cout << g_scurve[itab][ich][ig]->GetFunction( Form("func%d_%d_%d",itab,ich,ig) )->GetParameter(0) << " 0 "
	//<< g_scurve[itab][ich][ig]->GetFunction( Form("func%d_%d_%d",itab,ich,ig) )->GetParError (0) << std::endl;
      }
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    for( Int_t itab=0; itab<ntab_vref;  itab++ ){ // BEGIN TABLE-LOOP
      g_gain_vref[itab][ich]->Write();
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    for( Int_t itab=0; itab<ntab_tpchg; itab++ ){ // BEGIN TABLE-LOOP
      g_gain_tpchg[itab][ich]->Write();
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP
  outfile.Close();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  return 0;
}
