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

  //+++++++++++++++
  Int_t* cnt_vref  = new Int_t[ntab_vref ];
  Int_t* cnt_tpchg = new Int_t[ntab_tpchg];
  for( Int_t itab_vref =0; itab_vref <ntab_vref;  itab_vref++  ) cnt_vref [itab_vref ] = 0;
  for( Int_t itab_tpchg=0; itab_tpchg<ntab_tpchg; itab_tpchg++ ) cnt_tpchg[itab_tpchg] = 0;

  for( Int_t itab=0; itab<ntab; itab++ ){
    vref_tab [itab];
    tpchg_tab[itab];
    for( Int_t itab_vref=0; itab_vref<ntab_vref; itab_vref++ ){
      if( fabs(vref_tab_single[itab_vref] - vref_tab[itab])  < 0.001 ){
	cnt_vref[itab_vref]++;
	break;
      }
    }
    for( Int_t itab_tpchg=0; itab_tpchg<ntab_tpchg; itab_tpchg++ ){
      if( fabs(tpchg_tab_single[itab_tpchg] - tpchg_tab[itab])  < 0.001 ){
	cnt_tpchg[itab_tpchg]++;
	break;
      }
    }
  }

  // Determine reference VREF
  Int_t sel_tab_vref  = 0;
  Int_t tmp_max_vref = -999;
  for( Int_t itab_vref=0; itab_vref<ntab_vref; itab_vref++ ){
    if( cnt_vref[itab_vref] > tmp_max_vref ){
      tmp_max_vref = cnt_vref[itab_vref];
      sel_tab_vref = itab_vref;
    }
  }

  // Determine reference TPCHG
  Int_t sel_tab_tpchg = 0;
  Int_t tmp_max_tpchg = -999;
  for( Int_t itab_tpchg=0; itab_tpchg<ntab_tpchg; itab_tpchg++ ){
    if( cnt_tpchg[itab_tpchg] > tmp_max_tpchg ){
      tmp_max_tpchg = cnt_tpchg[itab_tpchg];
      sel_tab_tpchg = itab_tpchg;
    }
  }
  std::cout << "reference vref  = " <<  vref_tab_single[sel_tab_vref ] << " [mV]" << std::endl
	    << "reference tpchg = " << tpchg_tab_single[sel_tab_tpchg] << " [fC]" << std::endl;

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
	  tmp_func->SetParLimits( 1, 2.0, 20 );
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
  TLegend*  leg        = new TLegend( 0.20,0.80,0.85,0.98 );
  TCanvas** can_scurve = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_scurve[ichip] = new TCanvas( Form("can_scurve_%d",ichip),Form("can_scurve_%d",ichip), 1500, 1050 );
    can_scurve[ichip]->Divide(8,16);
    can_scurve[ichip]->Draw();
  }
  /*
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
  */

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
	  g_scurve[itab][ich][ig]->SetLineColor  ( itab+2 );
	  g_scurve[itab][ich][ig]->SetMarkerColor( itab+2 );
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
	  func[itab][ich][ig]->SetParameter( 1,     5.0 );
	  for( Int_t ipar1=0; ipar1<func[itab][ich][ig]->GetNpar(); ipar1++ ){
	    func[itab][ich][ig]->ReleaseParameter(ipar1);
	    for( Int_t ipar2=0; ipar2<func[itab][ich][ig]->GetNpar(); ipar2++ ){
	      if( ipar1==ipar2 ) continue;
	      func[itab][ich][ig]->FixParameter(ipar2, func[itab][ich][ig]->GetParameter(ipar2) );
	    }
	    func[itab][ich][ig]->SetParLimits( 0, -40, 40 );
	    func[itab][ich][ig]->SetParLimits( 1, 2.0, 20 );
	    g_scurve[itab][ich][ig]->Fit( func[itab][ich][ig],"SQ0" );
	  }
	  for( Int_t ipar=0; ipar<func[itab][ich][ig]->GetNpar(); ipar++ ) func[itab][ich][ig]->ReleaseParameter(ipar);
	  // +++++++++++ iterative fit(end) +++++++++++
	  func[itab][ich][ig]->SetParLimits( 0, -40, 40 );
	  func[itab][ich][ig]->SetParLimits( 1, 2.0, 20 );
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
	  func[itab][ich][ig]->SetLineColor(itab+2);
	  if( g_scurve[itab][ich][ig]->GetN() ){
	    fit_result = g_scurve[itab][ich][ig]->Fit( func[itab][ich][ig], "SQ", "same" ); // final fit
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
	  g_width[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_width[itab][ich][ig]->SetLineColor  ( itab+2 );
	  g_width[itab][ich][ig]->SetMarkerColor( itab+2 );
	  g_width[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_width[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  //can_width[chip_id]->cd(sel_pad);
	  //g_width[itab][ich][ig]->Draw( fl_first_width ? "AP" : "Psame" );
	  fl_first_width = false;
	}
      }

      
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_nring[itab][ich][ig]->GetN() ){

	  g_nring[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_nring[itab][ich][ig]->SetLineColor  ( itab+2 );
	  g_nring[itab][ich][ig]->SetMarkerColor( itab+2 );
	  g_nring[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_nring[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  //can_nring[chip_id]->cd(sel_pad);
	  //g_nring[itab][ich][ig]->Draw( fl_first_nring ? "AP" : "Psame" );
	  fl_first_nring = false;
	}
      }
      
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( g_span[itab][ich][ig]->GetN() ){
	  g_span[itab][ich][ig]->GetXaxis()->SetLimits(-32,32);
	  g_span[itab][ich][ig]->SetLineColor  ( itab+2 );
	  g_span[itab][ich][ig]->SetMarkerColor( itab+2 );
	  g_span[itab][ich][ig]->SetMarkerStyle( 21+ig  );
	  g_span[itab][ich][ig]->SetMarkerSize ( 0.2    );
	  //can_span[chip_id]->cd(sel_pad);
	  //g_span[itab][ich][ig]->Draw( fl_first_span ? "AP" : "Psame" );
	  fl_first_span = false;
	}
      }
    } // END TABLE-LOOP
  } // END CHANNEL-LOOP
  
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_scurve[ichip]->cd(n_unit*n_bit);
    leg->Draw();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Plots of Threshold value v.s. VREF (or TP charge)>
  for( Int_t itab=0; itab<ntab; itab++ ){ // BEGIN TABLE-LOOP
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	if( fl_fit[itab][ich][ig] ) continue;
	if( func[itab][ich][ig]->GetParameter(0) < -30 || func[itab][ich][ig]->GetParameter(0) > 30 ){
	  //std::cout << Form("[WARNING] under/over-flow in chip#%d, channel%d, vref=%.2f, tpchg=%.2f", gchannel2chip(ich),gchannel2lchannel(ich),vref_tab[itab],tpchg_tab[itab]) << std::endl;       
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
  TCanvas** can_vref = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_vref[ichip] = new TCanvas( Form("can_vref_%d",ichip),Form("can_vref_%d",ichip), 1500, 1050 );
    can_vref[ichip]->Divide(8,16);
    can_vref[ichip]->Draw();
  }

  TCanvas** can_tpchg = new TCanvas*[n_chip];
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_tpchg[ichip] = new TCanvas( Form("can_tpchg_%d",ichip),Form("can_tpchg_%d",ichip), 1500, 1050 );
    can_tpchg[ichip]->Divide(8,16);
    can_tpchg[ichip]->Draw();
  }

  TLegend* leg_vref = new TLegend( 0.30,0.70,0.50,0.95 );
  leg_vref->SetHeader("VREF");
  TMultiGraph** mg_vref = new TMultiGraph*[n_chip*n_unit*n_bit];
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    mg_vref[ich] = new TMultiGraph();
    mg_vref[ich]->SetTitle( ";TP charge [fC];Threshold DAC" );
    //mg_vref[ich]->SetMinimum(-20.0);
    //mg_vref[ich]->SetMaximum( 20.0);
    for( Int_t itab=0; itab<ntab_vref; itab++ ){ // BEGIN TABLE-LOOP
      g_gain_vref[itab][ich]->SetLineColor  (itab+2);
      g_gain_vref[itab][ich]->SetMarkerColor(itab+2);
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
      g_gain_tpchg[itab][ich]->SetLineColor  (itab+2);
      g_gain_tpchg[itab][ich]->SetMarkerColor(itab+2);
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
    can_vref[chip_id]->cd(sel_pad);
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
    can_tpchg[chip_id]->cd(sel_pad);
    //if( mg_tpchg[ich]->GetListOfGraphs()->GetSize() ){
    if( mg_tpchg[ich]->GetListOfGraphs() ){
      mg_tpchg[ich]->Draw("APL");
    }
    if( ich==n_chip*n_unit*n_bit-1 ) leg_tpchg->Draw();
  } // END CHANNEL-LOOP

  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MAKE SUMMARY GRAPH & TABLE
  TGraphErrors* sg_gain      = new TGraphErrors(); sg_gain     ->SetTitle( ";Channel;Gain [bit/fC]" );
  TGraphErrors* sg_offset    = new TGraphErrors(); sg_offset   ->SetTitle( ";Channel;Offset [bit]"  );
  TGraphErrors* sg_offset2   = new TGraphErrors(); sg_offset2  ->SetTitle( ";Channel;Offset [bit]"  ); // dead-channel is omitted
  TGraphErrors* sg_noise     = new TGraphErrors(); sg_noise    ->SetTitle( ";Channel;Noise [fC]"    );
  TGraphErrors* sg_threshold = new TGraphErrors(); sg_threshold->SetTitle( Form("%.2f MIP threshold;Channel;Offset [bit]",threshold_mip)  );

  std::multimap<Double_t,Int_t> noise_map;
  Int_t fl_alive[n_chip*n_unit*n_bit] = {0};

  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    if( g_gain_vref[sel_tab_vref][ich]->GetN() < 2 ){
      noise_map.insert( std::make_pair(999, ich) );
      sg_gain     ->SetPoint     ( sg_gain     ->GetN(),   ich,    -1 );
      sg_gain     ->SetPointError( sg_gain     ->GetN()-1,   0,     0 );
      sg_offset   ->SetPoint     ( sg_offset   ->GetN(),   ich,  -999 );
      sg_offset   ->SetPointError( sg_offset   ->GetN()-1,   0,     0 );
      sg_noise    ->SetPoint     ( sg_noise    ->GetN(),   ich,    -1 );
      sg_noise    ->SetPointError( sg_noise    ->GetN()-1,   0,     0 );
      sg_threshold->SetPoint     ( sg_threshold->GetN(),   ich,    -1 );
      sg_threshold->SetPointError( sg_threshold->GetN()-1,   0,     0 );
      continue;
    }

    Double_t gain    = g_gain_vref[sel_tab_vref][ich]->GetFunction("pol1")->GetParameter(1); // [bit/fC]
    Double_t gainE   = g_gain_vref[sel_tab_vref][ich]->GetFunction("pol1")->GetParError (1);
    Double_t offset  = g_gain_vref[sel_tab_vref][ich]->GetFunction("pol1")->GetParameter(0); // [bit/fC]
    Double_t offsetE = g_gain_vref[sel_tab_vref][ich]->GetFunction("pol1")->GetParError (0);
    Double_t noise  = -999; 
    Double_t noiseE = -999;
    for( Int_t itab=0; itab<ntab; itab++ ){ // temporal treatment // tmppppp
      if( fabs(vref_tab_single[sel_tab_vref] - vref_tab[itab]) > 0.001 ) continue;
      if( g_scurve[itab][ich][0]->GetFunction(Form("func%d_%d_%d",itab,ich,0))->GetParameter(0)>-30 ){
	noise  = g_scurve[itab][ich][0]->GetFunction(Form("func%d_%d_%d",itab,ich,0))->GetParameter(1); // [bit]
	noiseE = g_scurve[itab][ich][0]->GetFunction(Form("func%d_%d_%d",itab,ich,0))->GetParError (1);
	break;
      }
    }

    if( noise < 0 || gain==0 ){
      std::cout << ich << ", gain = " << gain << ", noise = " << noise << std::endl;
      sg_gain     ->SetPoint     ( sg_gain     ->GetN(),   ich,      -1 );
      sg_gain     ->SetPointError( sg_gain     ->GetN()-1,   0,       0 );
      sg_offset   ->SetPoint     ( sg_offset   ->GetN(),   ich,    -999 );
      sg_offset   ->SetPointError( sg_offset   ->GetN()-1,   0,       0 );
      sg_offset2  ->SetPoint     ( sg_offset2  ->GetN(),   ich,    -999 );
      sg_offset2  ->SetPointError( sg_offset2  ->GetN()-1,   0,       0 );
      sg_noise    ->SetPoint     ( sg_noise    ->GetN(),   ich,      -1 ); // [fC]
      sg_noise    ->SetPointError( sg_noise    ->GetN()-1,   0,       0 );
      sg_threshold->SetPoint     ( sg_threshold->GetN(),   ich,      -1 );
      sg_threshold->SetPointError( sg_threshold->GetN()-1,   0,       0 );
      noise_map.insert( std::make_pair(999, ich) );
    }else{
      sg_gain     ->SetPoint     ( sg_gain     ->GetN(),   ich, gain    );
      sg_gain     ->SetPointError( sg_gain     ->GetN()-1,   0, gainE   );
      sg_offset   ->SetPoint     ( sg_offset   ->GetN(),   ich, offset  );
      sg_offset   ->SetPointError( sg_offset   ->GetN()-1,   0, offsetE );
      sg_offset2  ->SetPoint     ( sg_offset2  ->GetN(),   ich, offset  );
      sg_offset2  ->SetPointError( sg_offset2  ->GetN()-1,   0, offsetE );
      sg_noise    ->SetPoint     ( sg_noise    ->GetN(),   ich, noise/gain ); // [fC]
      sg_noise    ->SetPointError( sg_noise    ->GetN()-1,   0, noise/gain*sqrt( pow(noiseE/noise,2) + pow(gainE/gain,2)) );
      sg_threshold->SetPoint     ( sg_threshold->GetN(),   ich, gain*onemip*threshold_mip+offset );
      sg_threshold->SetPointError( sg_threshold->GetN()-1,   0, sqrt(pow(gainE*onemip*threshold_mip,2) + pow(offsetE,2)) ); // correlation is ignored. to be fix.
      noise_map.insert( std::make_pair(noise/gain, ich) );
      fl_alive[ich] = 1;
    }
  } // END CHANNEL LOOP
  
  std::multimap<Double_t,Int_t>::reverse_iterator it_noise = noise_map.rbegin();
  Int_t cnt_dead  = 0;
  Int_t cnt_noisy = 0;
  
  while( it_noise != noise_map.rend() ){
    if     ( (*it_noise).first > 900 ) cnt_dead++;
    else if( (*it_noise).first > 0.2 ) cnt_noisy++;
    else                               break;
    std::cout << "Chip#" << gchannel2chip((*it_noise).second) << ", Local Channel#" << gchannel2lchannel((*it_noise).second) << ",Global Channel#" << (*it_noise).second << ", noise = " << (*it_noise).first << std::endl;
    it_noise++;
  }
  std::cout << "N( dead-ch) = " << cnt_dead  << std::endl
	    << "N(noisy-ch) = " << cnt_noisy << std::endl;


  //++++++++++++++++++++++++++
  // VREF DEPENDENCE
  TGraphErrors* sg_vrefdep_bit  = new TGraphErrors(); sg_vrefdep_bit ->SetTitle( ";Channel;VREF dep. [bit/mV]" );
  TGraphErrors* sg_vrefdep_chg  = new TGraphErrors(); sg_vrefdep_chg ->SetTitle( ";Channel;VREF dep. [fC/mV]"  );
  TGraphErrors* sg_vrefdep_bit2 = new TGraphErrors(); sg_vrefdep_bit2->SetTitle( ";Channel;VREF dep. [bit/mV]" ); // dead-channel is omitted
  TGraphErrors* sg_vrefdep_chg2 = new TGraphErrors(); sg_vrefdep_chg2->SetTitle( ";Channel;VREF dep. [fC/mV]"  ); // dead-channel is omitted
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    if( g_gain_tpchg[sel_tab_tpchg][ich]->GetN() < 2 ){
      sg_vrefdep_bit->SetPoint     ( sg_vrefdep_bit->GetN(),   ich, -1 );
      sg_vrefdep_bit->SetPointError( sg_vrefdep_bit->GetN()-1,   0,  0 );
      sg_vrefdep_chg->SetPoint     ( sg_vrefdep_chg->GetN(),   ich, -1 );
      sg_vrefdep_chg->SetPointError( sg_vrefdep_chg->GetN()-1,   0,  0 );
      continue;
    }
    Double_t vrefdep    = g_gain_tpchg[sel_tab_tpchg][ich]->GetFunction("pol1")->GetParameter(1); // [bit/mV]
    Double_t vrefdepE   = g_gain_tpchg[sel_tab_tpchg][ich]->GetFunction("pol1")->GetParError (1);
    sg_vrefdep_bit ->SetPoint     ( sg_vrefdep_bit ->GetN(),   ich,  vrefdep  );
    sg_vrefdep_bit ->SetPointError( sg_vrefdep_bit ->GetN()-1,   0,  vrefdepE );
    sg_vrefdep_chg ->SetPoint     ( sg_vrefdep_chg ->GetN(),   ich,  vrefdep/(sg_gain->GetY()[ich]) );
    sg_vrefdep_chg ->SetPointError( sg_vrefdep_chg ->GetN()-1,   0,  vrefdep/(sg_gain->GetY()[ich])*sqrt(pow(vrefdepE/vrefdep,2)+pow(sg_gain->GetEY()[ich]/sg_gain->GetY()[ich],2)) );
    sg_vrefdep_bit2->SetPoint     ( sg_vrefdep_bit2->GetN(),   ich,  vrefdep  );
    sg_vrefdep_bit2->SetPointError( sg_vrefdep_bit2->GetN()-1,   0,  vrefdepE );
    sg_vrefdep_chg2->SetPoint     ( sg_vrefdep_chg2->GetN(),   ich,  vrefdep/(sg_gain->GetY()[ich]) );
    sg_vrefdep_chg2->SetPointError( sg_vrefdep_chg2->GetN()-1,   0,  vrefdep/(sg_gain->GetY()[ich])*sqrt(pow(vrefdepE/vrefdep,2)+pow(sg_gain->GetEY()[ich]/sg_gain->GetY()[ich],2)) );
  } // END CHANNEL LOOP
  //++++++++++++++++++++++++++
  // PRECISE CALIBRATION
  const Double_t vref_nstep = 40;
  Double_t vref_max = vref_tab_single[sel_tab_vref];
  Double_t vref_min = vref_tab_single[sel_tab_vref]-200;
  Int_t cnt_vref_max   = 0;
  Double_t calib_vref = 0;
  for( Int_t istep=0; istep<=vref_nstep; istep++ ){
    Int_t tmp_cnt = 0;
    Double_t tmp_vref = vref_min + (vref_max-vref_min)/vref_nstep*istep;
    for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){
      Double_t dac = sg_gain->GetY()[ich] * threshold_mip * onemip + sg_offset->GetY()[ich] + sg_vrefdep_bit->GetY()[ich]*(tmp_vref - vref_tab_single[sel_tab_vref]);
      if( dac >= -31 && dac <= 31 ) tmp_cnt++;
    }
    if( tmp_cnt > cnt_vref_max ){
      cnt_vref_max = tmp_cnt;
      calib_vref   = tmp_vref;
    }
  }
  std::cout << "calib_vref = " << calib_vref << " [mV] , cnt_vref_max = " << cnt_vref_max << std::endl;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MAKE CANVAS FOR SUMMARY
  TCanvas* can = new TCanvas( "can_summary", "can_summary", 1800, 1050 );
  can->Divide(3,2);
  can->Draw();
  can->cd(1);
  sg_gain->SetMinimum(0.0);
  sg_gain->Draw("AP");
  can->cd(2);
  sg_noise->SetMinimum(0.0);
  sg_noise->Draw("AP");
  can->cd(3);
  sg_offset2->Draw("AP");
  can->cd(4);
  sg_threshold->Draw("AP");
  can->cd(5);
  //sg_vrefdep_bit2->Draw("AP");
  sg_vrefdep_chg2->Draw("AP");
  can->cd(6);

  can->Update();
  can->Print( Form("pic/%s_summary.ps",  basename.c_str()) );
  can->Print( Form("pic/%s_summary.png", basename.c_str()) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // SAVE
  TFile outfile( Form("pic/%s.root",basename.c_str()), "RECREATE" );
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    for( Int_t itab=0; itab<ntab; itab++ ){ // BEGIN TABLE-LOOP
      for( Int_t ig=0; ig<cnt_g[itab][ich]; ig++ ){
	g_scurve[itab][ich][ig]->SetName ( Form("scurve_%d_%d_%d", itab,ich,ig) );
	g_scurve[itab][ich][ig]->SetTitle( Form("vref=%.2f, tpchg=%.2f, %d", vref_tab[itab], tpchg_tab[itab],ig) );
	g_scurve[itab][ich][ig]->GetXaxis()->SetTitle("DAC [bit]");
	g_scurve[itab][ich][ig]->GetYaxis()->SetTitle("Count Efficiency");
	g_scurve[itab][ich][ig]->Write();

	g_width[itab][ich][ig]->SetName( Form("width_%d_%d_%d", itab,ich,ig) );
	g_nring[itab][ich][ig]->SetName( Form("nring_%d_%d_%d", itab,ich,ig) );
	g_span [itab][ich][ig]->SetName( Form("span_%d_%d_%d",  itab,ich,ig) );
	g_width[itab][ich][ig]->Write();
	g_nring[itab][ich][ig]->Write();
	g_span [itab][ich][ig]->Write();
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
  // LOG
  std::ofstream* fout = new std::ofstream[n_chip];
  system( "mkdir -p dat_calib;");
  for( Int_t ichip=0; ichip<n_chip; ichip++ ) fout[ichip].open(Form("dat_calib/threshold_calib_board%d_chip%d.dat",board_id,ichip) );
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    Int_t chip_id  = ich/(n_unit*n_bit);
    Int_t unit_id  = chip_id/n_bit;
    Int_t lchannel = ich%(n_unit*n_bit);
    Int_t gchannel = ich;
    fout[chip_id] << std::setw(2) << std::right << board_id      << "   "
		  << std::setw(2) << std::right << chip_id       << "   "
		  << std::setw(2) << std::right << unit_id       << "   "
		  << std::setw(4) << std::right << lchannel      << "   "
		  << std::setw(4) << std::right << gchannel      << "   "
		  << std::setw(2) << std::right << fl_alive[ich] << "   "
		  << std::setw( 8) << std::left  << Form(" %.4f",sg_gain       ->GetY ()[ich])
		  << std::setw(10) << std::left  << Form(" %.4f",sg_gain       ->GetEY()[ich])
		  << std::setw( 8) << std::left  << Form(" %.3f",sg_offset     ->GetY ()[ich])
		  << std::setw(10) << std::left  << Form(" %.3f",sg_offset     ->GetEY()[ich])
		  << std::setw( 8) << std::left  << Form(" %.3f",sg_noise      ->GetY ()[ich])
		  << std::setw(10) << std::left  << Form(" %.3f",sg_noise      ->GetEY()[ich])
		  << std::setw( 8) << std::left  << Form(" %.4f",sg_vrefdep_chg->GetY ()[ich])
		  << std::setw(10) << std::left  << Form(" %.4f",sg_vrefdep_chg->GetEY()[ich])
		  << std::endl;
  }
  for( Int_t ichip=0; ichip<n_chip; ichip++ ) fout[ichip].close();

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  for( Int_t ich=0; ich<n_chip*n_unit*n_bit; ich++ ){ // BEGIN CHANNEL-LOOP
    Int_t chip_id = ich/(n_unit*n_bit);
    Int_t lch_id  = ich%(n_unit*n_bit);
    Int_t sel_row    = lch_id%16;
    Int_t sel_column = lch_id/16;
    Int_t sel_pad    = 8*sel_row + sel_column+1;
    
    if( fl_alive[ich]==0 ){
      can_scurve[chip_id]->GetPad(sel_pad)->SetFillColor(kYellow);
      can_vref  [chip_id]->GetPad(sel_pad)->SetFillColor(kYellow);
      can_tpchg [chip_id]->GetPad(sel_pad)->SetFillColor(kYellow);
    }
  } // END CHANNEL-LOOP
  for( Int_t ichip=0; ichip<n_chip; ichip++ ){
    can_scurve[ichip]->Update();
    can_scurve[ichip]->Print( Form("pic/%s_scurve_%d.ps" ,basename.c_str(),ichip) );
    can_scurve[ichip]->Print( Form("pic/%s_scurve_%d.png",basename.c_str(),ichip) );
    can_vref  [ichip]->Update();
    can_vref  [ichip]->Print( Form("pic/%s_vref_%d.ps",   basename.c_str(),ichip) );
    can_vref  [ichip]->Print( Form("pic/%s_vref_%d.png",  basename.c_str(),ichip) );
    can_tpchg [ichip]->Update();
    can_tpchg [ichip]->Print( Form("pic/%s_tpchg_%d.ps",  basename.c_str(),ichip) );
  }
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  

  //for( Int_t ichip=0; ichip<n_chip; ichip++ ) delete can_tpchg[ichip]; // tmppppp
  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();

  return 0;
}
