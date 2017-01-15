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
  unsigned int seed = time(NULL);
  //TRandom rnd(seed);
  //TRandom rnd(2000);
  TRandom rnd(3000);

  if( !(app.Argc()==4) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (char*)infilename (int)ch" << std::endl
	      << std::endl, abort();
  Char_t* infilename = app.Argv(1);
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".root") );
  Int_t  ch      = atoi( app.Argv(2) );
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
  std::vector<TGraphErrors>* g_scurve = new std::vector<TGraphErrors>[ntab];
  std::vector<TF1*>*         func     = new std::vector<TF1*>        [ntab];
  std::vector<Int_t>*        fl_fit   = new std::vector<Int_t>       [ntab];

  std::vector<TGraphErrors>* g_width = new std::vector<TGraphErrors>[ntab];
  std::vector<TGraphErrors>* g_nring = new std::vector<TGraphErrors>[ntab];
  std::vector<TGraphErrors>* g_span  = new std::vector<TGraphErrors>[ntab];


  /*  
  std::vector<TGraphErrors> g_scurve[ntab];
  std::vector<TF1*>         func    [ntab];
  std::vector<Int_t>        fl_fit  [ntab];

  std::vector<TGraphErrors> g_width[ntab];
  std::vector<TGraphErrors> g_nring[ntab];
  std::vector<TGraphErrors> g_span [ntab];
  */

  TGraphErrors** g_gain_vref = new TGraphErrors*[ntab_vref];
  for( Int_t itab=0; itab<ntab_vref; itab++ ){
    g_gain_vref[itab] = new TGraphErrors();
    g_gain_vref[itab]->SetName ( Form("g_gain_vref_%d",itab) );
    g_gain_vref[itab]->SetTitle( Form("g_gain_vref_%d;TP charge [fC];Threshold DAC",itab) );
    g_gain_vref[itab]->SetLineColor  (3);
    g_gain_vref[itab]->SetMarkerColor(3);
  }
  
  TGraphErrors** g_gain_tpchg = new TGraphErrors*[ntab_tpchg];
  for( Int_t itab=0; itab<ntab_tpchg; itab++ ){
    g_gain_tpchg[itab] = new TGraphErrors();
    g_gain_tpchg[itab]->SetName ( Form("g_gain_tpchg_%d",itab) );
    g_gain_tpchg[itab]->SetTitle( Form("g_gain_tpchg_%d;VREF [mV];Threshold DAC",itab) );
    g_gain_tpchg[itab]->SetLineColor  (3);
    g_gain_tpchg[itab]->SetMarkerColor(3);
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Input Data to Objects>
  Int_t    dac_before   =  999;
  Double_t tpchg_before = -999;
  Double_t vref_before  = -999;
  Int_t*  cnt_g = new Int_t[ntab];
  for( Int_t itab=0; itab<ntab; itab++ ) cnt_g[itab] = 0;
  
  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);
    Float_t tpchg     =        chain->GetLeaf( "tpchg"     )->GetValue();
    Float_t vref      =        chain->GetLeaf( "vref"      )->GetValue();
    Int_t   dac       = (Int_t)chain->GetLeaf( "dac"       )->GetValue();
    Float_t sigeff    =        chain->GetLeaf( "sigeff"    )->GetValue();
    Float_t sigeffE   =        chain->GetLeaf( "sigeffE"   )->GetValue();
    Float_t ringprob  =        chain->GetLeaf( "ringprob"  )->GetValue();
    Float_t ringprobE =        chain->GetLeaf( "ringprobE" )->GetValue();
    Float_t sigwidth  =        chain->GetLeaf( "sigwidth"  )->GetValue();
    Float_t sigwidthE =        chain->GetLeaf( "sigwidthE" )->GetValue();
    Float_t span      =        chain->GetLeaf( "span"      )->GetValue();
    Float_t spanE     =        chain->GetLeaf( "spanE"     )->GetValue();

    for( Int_t itab=0; itab<ntab; itab++ ){
      if( fabs(vref_tab[itab] - vref)  < 0.001 && fabs(tpchg_tab[itab] - tpchg) < 0.001 ){ // to be checked !!!
	if( dac_before > dac ){ // Make New Objects
	  TGraphErrors* tmp_g_scurve = new TGraphErrors();
	  if( fl_plot ){
	    tmp_g_scurve->SetLineColor  ( g_scurve[itab].size()+1 );
	    tmp_g_scurve->SetMarkerColor( g_scurve[itab].size()+1 );
	    //tmp_g_scurve->SetLineColor  ( ((Int_t)(g_scurve[itab].size()/10))+1 ); // tmpppppp
	    //tmp_g_scurve->SetMarkerColor( ((Int_t)(g_scurve[itab].size()/10))+1 ); // tmpppppp
	  }else{
	    tmp_g_scurve->SetLineColor  ( itab+1 );
	    tmp_g_scurve->SetMarkerColor( itab+1 );
	    tmp_g_scurve->SetMarkerStyle( 20+g_scurve[itab].size()+1 );
	  }
	  g_scurve[itab].push_back( *tmp_g_scurve );
	  delete tmp_g_scurve;
	  TF1* tmp_func = new TF1( Form("func%d_%d",itab,cnt_g[itab]), "0.5*TMath::Erf(([0]-x)/sqrt(2)/[1])+0.5", -31,31 ); // 0(mean), 1(sigma)
	  tmp_func->SetParameter(0, 0.0);
	  //tmp_func->SetParameter(1, 2.5);
	  tmp_func->SetParameter(1, 1.9); // tmpppppp
	  tmp_func->SetParNames( "mean", "sigma");
	  tmp_func->SetParLimits( 0, -31, 31 );
	  tmp_func->SetParLimits( 1, 0.0, 20 );
	  func[itab].push_back( tmp_func );
	  //delete tmp_func; // "graph->Fit"can not find object in std::vector by object name;
	  TGraphErrors* tmp_g_width = new TGraphErrors();
	  if( fl_plot ){
	    tmp_g_width->SetLineColor  ( g_width[itab].size()+1 );
	    tmp_g_width->SetMarkerColor( g_width[itab].size()+1 );
	    //tmp_g_width->SetLineColor  ( ((Int_t)(g_width[itab].size()/10))+1 ); // tmpppppp
	    //tmp_g_width->SetMarkerColor( ((Int_t)(g_width[itab].size()/10))+1 ); // tmpppppp
	  }else{
	    tmp_g_width->SetLineColor  ( itab+1 );
	    tmp_g_width->SetMarkerColor( itab+1 );
	    tmp_g_width->SetMarkerStyle( 20+g_width[itab].size()+1 );
	  }
	  tmp_g_width->SetTitle("Signal Width;DAC;Width;");
	  g_width[itab].push_back( *tmp_g_width );
	  delete tmp_g_width;
	  TGraphErrors* tmp_g_nring = new TGraphErrors();
	  if( fl_plot ){
	    tmp_g_nring->SetLineColor  ( g_nring[itab].size()+1 );
	    tmp_g_nring->SetMarkerColor( g_nring[itab].size()+1 );
	    //tmp_g_nring->SetLineColor  ( ((Int_t)(g_nring[itab].size()/10))+1 ); // tmpppppppppp
	    //tmp_g_nring->SetMarkerColor( ((Int_t)(g_nring[itab].size()/10))+1 ); // tmpppppppppp
	  }else{
	    tmp_g_nring->SetLineColor  ( itab+1 );
	    tmp_g_nring->SetMarkerColor( itab+1 );
	    tmp_g_nring->SetMarkerStyle( 20+g_nring[itab].size()+1 );
	  }
	  tmp_g_nring->SetTitle("Ringing Probability;DAC;P_{ringing};");
	  g_nring[itab].push_back( *tmp_g_nring );
	  delete tmp_g_nring;
	  TGraphErrors* tmp_g_span = new TGraphErrors();
	  if( fl_plot ){
	    tmp_g_span->SetLineColor  ( g_span[itab].size()+1 );
	    tmp_g_span->SetMarkerColor( g_span[itab].size()+1 );
	    //tmp_g_span->SetLineColor  ( ((Int_t)(g_span[itab].size()/10))+1 ); // tmpppp
	    //tmp_g_span->SetMarkerColor( ((Int_t)(g_span[itab].size()/10))+1 ); // tmpppp
	  }else{
	    tmp_g_span->SetLineColor  ( itab+1  );
	    tmp_g_span->SetMarkerColor( itab+1  );
	    tmp_g_span->SetMarkerStyle( 20+g_span[itab].size()+1 );
	  }
	  tmp_g_span->SetTitle("Span;DAC;Span;");
	  g_span[itab].push_back( *tmp_g_span );
	  delete tmp_g_span;

	  cnt_g[itab]++;
	} // End of <Make New Objects>

	// Input Data
	g_scurve[itab][cnt_g[itab]-1].SetPoint     ( g_scurve[itab][cnt_g[itab]-1].GetN(),   dac, sigeff    );
	g_scurve[itab][cnt_g[itab]-1].SetPointError( g_scurve[itab][cnt_g[itab]-1].GetN()-1,   0, sigeffE   );
	g_width [itab][cnt_g[itab]-1].SetPoint     ( g_width [itab][cnt_g[itab]-1].GetN(),   dac, sigwidth  );
	g_width [itab][cnt_g[itab]-1].SetPointError( g_width [itab][cnt_g[itab]-1].GetN()-1,   0, sigwidthE );
	g_nring [itab][cnt_g[itab]-1].SetPoint     ( g_nring [itab][cnt_g[itab]-1].GetN(),   dac, ringprob  );
	g_nring [itab][cnt_g[itab]-1].SetPointError( g_nring [itab][cnt_g[itab]-1].GetN()-1,   0, ringprobE );
	if( span ){
	  g_span[itab][cnt_g[itab]-1].SetPoint     ( g_span[itab][cnt_g[itab]-1].GetN(),   dac, span  );
	  g_span[itab][cnt_g[itab]-1].SetPointError( g_span[itab][cnt_g[itab]-1].GetN()-1,   0, spanE );
	}
	vref_before  = vref_tab [itab];
	tpchg_before = tpchg_tab[itab];
	break;
      }
    }
    dac_before = dac;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Draw Objects and Fit S-curve>
  TLegend* leg  = new TLegend( 0.20,0.80,0.85,0.98 );
  const Int_t npad = 12;
  TCanvas* can1;
  if( fl_plot ){ // one plot per parameter point
    can1 = new TCanvas("can1","can1", 1500, 1000 );
    can1->Divide(4,3);
  }else{ // one plot per channel
    can1 = new TCanvas("can1","can1", 1500, 400 );
    can1->Divide(4,1);
  }
  can1->Draw();
  Int_t cnt_pad  = 1;
  Int_t cnt_can1 = 0;
  Int_t cnt_tex  = 0;
  for( Int_t itab=0; itab<ntab; itab++ ){
    can1->cd( (fl_plot ? cnt_pad++ : 1) );

    if     (  fl_plot            ) gPad->DrawFrame( -32, 0.0, 32, 1.2, Form("VREF=%.2f, TPCHG=%.2f;DAC;Count Efficiency", vref_tab[itab], tpchg_tab[itab]) );
    else if( !fl_plot && itab==0 ) gPad->DrawFrame( -32, 0.0, 32, 1.2, Form(                   "%s;DAC;Count Efficiency", basename.c_str()               ) );
    for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
      if( g_scurve[itab][ig].GetN() ){
	g_scurve[itab][ig].Sort();
	g_width [itab][ig].Sort();
	g_nring [itab][ig].Sort();

	g_scurve[itab][ig].Draw("Psame");
	leg->AddEntry( &g_scurve[itab][ig], Form("VREF=%.2f, TPCHG=%.2f", vref_tab[itab], tpchg_tab[itab]), "PL" );
	// +++++++++++ trimming data(begin) +++++++++++
	Bool_t fl_th_high = false;
	for( Int_t ip1=g_scurve[itab][ig].GetN()-1; ip1>=0; ip1-- ){
	  if( g_scurve[itab][ig].GetY()[ip1] > 0.999 ) fl_th_high = true;
	  if( fl_th_high && g_scurve[itab][ig].GetY()[ip1] < 0.96 ){
	    for( Int_t ip2=ip1; ip2>=0; ip2-- ){
	      g_scurve[itab][ig].RemovePoint(ip2);
	      g_width [itab][ig].RemovePoint(ip2);
	      g_nring [itab][ig].RemovePoint(ip2);
	    }
	    for( Int_t ip2=g_span[itab][ig].GetN()-1; ip2>=0; ip2-- ){
	      if( g_span[itab][ig].GetX()[ip2] <= g_scurve[itab][ig].GetX()[ip1] ) g_span[itab][ig].RemovePoint(ip2);
	    }
	    break;
	  }
	}
	// +++++++++++ trimming data(end) +++++++++++
	// +++++++++++ iterative fit(begin) +++++++++++
	for( Int_t ipar1=0; ipar1<func[itab][ig]->GetNpar(); ipar1++ ){
	  func[itab][ig]->ReleaseParameter(ipar1);
	  for( Int_t ipar2=0; ipar2<func[itab][ig]->GetNpar(); ipar2++ ){
	    if( ipar1==ipar2 ) continue;
	    func[itab][ig]->FixParameter(ipar2, func[itab][ig]->GetParameter(ipar2) );
	  }
	  if( ipar1==0 ){
	    Int_t subfit_status = 1;
	    while( subfit_status ){
	      func[itab][ig]->SetParLimits(0,-31,31);
	      func[itab][ig]->SetParameter( 0, rnd.Uniform(-31.0,31.0) );
	      func[itab][ig]->SetParameter(1,2.5);
	      TFitResultPtr subfit_result = g_scurve[itab][ig].Fit( func[itab][ig],"SQ0" );
	      subfit_status = subfit_result->Status();
	    }
	  }else{
	    func[itab][ig]->SetParLimits( 1, 0, 20 );
	    g_scurve[itab][ig].Fit( func[itab][ig],"S0Q" );
	  }
	}
	for( Int_t ipar=0; ipar<func[itab][ig]->GetNpar(); ipar++ ) func[itab][ig]->ReleaseParameter(ipar);
	// +++++++++++ iterative fit(end) +++++++++++
	func[itab][ig]->SetParLimits( 0, -31, 31 );
	func[itab][ig]->SetParLimits( 1,   0, 20 );
	TFitResultPtr fit_result = g_scurve[itab][ig].Fit( func[itab][ig], "SQ0" );

	// +++++++++++ trimming data(begin) +++++++++++
	///*
	for( Int_t ip1=g_scurve[itab][ig].GetN()-1; ip1>=0; ip1-- ){
	  if( func[itab][ig]->GetParameter(0) < g_scurve[itab][ig].GetX()[ip1] ) continue;
	  if( g_scurve[itab][ig].GetY()[ip1]==0 ){ // data point with eff = 0 at invarid region should be removed.
	    for( Int_t ip2=ip1; ip2>=0; ip2-- ){
	      g_scurve[itab][ig].RemovePoint(ip2);
	      g_width [itab][ig].RemovePoint(ip2);
	      g_nring [itab][ig].RemovePoint(ip2);
	    }
	    for( Int_t ip2=g_span[itab][ig].GetN()-1; ip2>=0; ip2-- ){
	      if( g_span[itab][ig].GetX()[ip2] <= g_scurve[itab][ig].GetX()[ip1] ) g_span[itab][ig].RemovePoint(ip2);
	    }
	    break;
	  }
	}
	if( !fl_plot ) func[itab][ig]->SetLineColor(itab+1);
	fit_result = g_scurve[itab][ig].Fit( func[itab][ig], "S", "same" ); // final fit
	//*/
	// +++++++++++ trimming data(end) +++++++++++
	fl_fit[itab].push_back( fit_result->Status() ); // 0(success), other(false), 4(call limit)
	if( fl_plot ){
	  tex1->DrawLatexNDC( 0.65, 0.86-0.15*ig, Form("#mu = %.2f #pm %.2f",        func[itab][ig]->GetParameter(0), func[itab][ig]->GetParError(0)) );
	  tex1->DrawLatexNDC( 0.65, 0.82-0.15*ig, Form("#sigma = %.2f #pm %.2f",     func[itab][ig]->GetParameter(1), func[itab][ig]->GetParError(1)) );
	  tex1->DrawLatexNDC( 0.65, 0.78-0.15*ig, Form("#chi^{2}/NDF = %.2f / %d",   func[itab][ig]->GetChisquare(),  func[itab][ig]->GetNDF()      ) );
	}else{
	  TLatex* tex_col = new TLatex();
	  tex_col->SetTextColor(itab+1);
	  tex_col->SetTextSize(0.03);
	  tex_col->DrawLatexNDC( 0.65, 0.86-0.15*cnt_tex, Form("#mu = %.2f #pm %.2f",        func[itab][ig]->GetParameter(0), func[itab][ig]->GetParError(0)) );
	  tex_col->DrawLatexNDC( 0.65, 0.82-0.15*cnt_tex, Form("#sigma = %.2f #pm %.2f",     func[itab][ig]->GetParameter(1), func[itab][ig]->GetParError(1)) );
	  tex_col->DrawLatexNDC( 0.65, 0.78-0.15*cnt_tex, Form("#chi^{2}/NDF = %.2f / %d",   func[itab][ig]->GetChisquare(),  func[itab][ig]->GetNDF()      ) );
	  cnt_tex++;
	}
      }else{
	fl_fit[itab].push_back(-999); // 0(success), other(false), 4(call limit)
      }
    }
    
    
    
    can1->cd( (fl_plot ? cnt_pad++ : 2) );
    {
      Int_t fl_first = 1;
      for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
	if( g_width[itab][ig].GetN() ){
	  g_width[itab][ig].GetXaxis()->SetLimits(-32,32);
	  g_width[itab][ig].Draw( fl_first&&( fl_plot || (!fl_plot&&itab==0&&ig==0)) ? "AP" : "Psame" );
	  fl_first = 0;
	}
      }
    }

    can1->cd( (fl_plot ? cnt_pad++ : 3) );
    {
      Int_t fl_first = 1;
      for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
	if( g_nring[itab][ig].GetN() ){
	  g_nring[itab][ig].GetXaxis()->SetLimits(-32,32);
	  g_nring[itab][ig].Draw( fl_first&&( fl_plot || (!fl_plot&&itab==0&&ig==0)) ? "AP" : "Psame" );
	  fl_first = 0;
	}
      }
    }

    can1->cd( (fl_plot ? cnt_pad++ : 4) );
    {
      Int_t fl_first = 1;
      for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
	if( g_span[itab][ig].GetN() ){
	  g_span[itab][ig].GetXaxis()->SetLimits(-32,32);
	  g_span[itab][ig].Draw( fl_first&&( fl_plot || (!fl_plot&&itab==0&&ig==0)) ? "AP" : "Psame" );
	  fl_first = 0;
	}
      }
    }
    if( !fl_plot ) leg->Draw();

    if( cnt_pad==npad+1 ){
      can1->Update();
      can1->WaitPrimitive();
      cnt_pad = 1;
      can1->Print( Form("pic/%s_can1_%d.ps",basename.c_str(),cnt_can1) );
      if( itab!=ntab-1 ) cnt_can1++;
    }
  }
  
  can1->Update();
  //can1->WaitPrimitive();
  can1->Print( Form("pic/%s_can1_%d.ps",basename.c_str(),cnt_can1) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Make Plots of Threshold value v.s. VREF (or TP charge)>
  for( Int_t itab=0; itab<ntab; itab++ ){
    for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
      if( fl_fit[itab][ig] ) continue;
      if( func[itab][ig]->GetParameter(0) < -30.99 || func[itab][ig]->GetParameter(0) > 30.99 ){
	std::cout << Form("[Warining] under/over-flow in %d-ch, vref=%.2f, tpchg=%.2f", ch,vref_tab[itab],tpchg_tab[itab]) << std::endl;       
	continue;
      }
      for( Int_t itab_vref=0; itab_vref<ntab_vref; itab_vref++ ){
	if( fabs(vref_tab[itab] - vref_tab_single[itab_vref]) < 0.001 ){
	  g_gain_vref[itab_vref]->SetPoint     ( g_gain_vref[itab_vref]->GetN(),   tpchg_tab[itab], func[itab][ig]->GetParameter(0) );
	  g_gain_vref[itab_vref]->SetPointError( g_gain_vref[itab_vref]->GetN()-1,               0, func[itab][ig]->GetParError (0) );
	  break;
	}
      }
      
      for( Int_t itab_tpchg=0; itab_tpchg<ntab_tpchg; itab_tpchg++ ){
	if( fabs(tpchg_tab[itab] - tpchg_tab_single[itab_tpchg]) < 0.001 ){
	  g_gain_tpchg[itab_tpchg]->SetPoint     ( g_gain_tpchg[itab_tpchg]->GetN(),   vref_tab[itab], func[itab][ig]->GetParameter(0) );
	  g_gain_tpchg[itab_tpchg]->SetPointError( g_gain_tpchg[itab_tpchg]->GetN()-1,              0, func[itab][ig]->GetParError (0) );
	  break;
	}
      }
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // <Draw>
  TCanvas* can2 = new TCanvas("can2","can2", 1000, 400 );
  can2->Divide(2,1);
  can2->Draw();

  TMultiGraph* mg_vref = new TMultiGraph();
  mg_vref->SetTitle( Form("%s;TP charge [fC];Threshold DAC",basename.c_str()) );
  //mg_vref->SetMinimum(-20.0);
  //mg_vref->SetMaximum( 20.0);
  TLegend* leg_vref = new TLegend( 0.30,0.70,0.50,0.95 );
  leg_vref->SetHeader("VREF");
  for( Int_t itab=0; itab<ntab_vref; itab++ ){
    if( g_gain_vref[itab]->GetN()==0 ) continue;
    g_gain_vref[itab]->SetLineColor  (itab+1);
    g_gain_vref[itab]->SetMarkerColor(itab+1);
    if( g_gain_vref[itab]->GetN()>1 ){
      g_gain_vref[itab]->Fit("pol1");
    }
    g_gain_vref[itab]->Sort();
    mg_vref->Add( g_gain_vref[itab] );
    leg_vref->AddEntry( g_gain_vref[itab],Form("%.2f mV",vref_tab_single[itab]) ,"PL" );
  }

  TMultiGraph* mg_tpchg = new TMultiGraph();
  mg_tpchg->SetTitle(";VREF [mV];Threshold DAC");
  mg_tpchg->SetMinimum(-20.0);
  mg_tpchg->SetMaximum( 20.0);
  TLegend* leg_tpchg = new TLegend( 0.65,0.70,0.85,0.95 );
  leg_tpchg->SetHeader("TP charge");
  for( Int_t itab=0; itab<ntab_tpchg; itab++ ){
    if( g_gain_tpchg[itab]->GetN()==0 ) continue;
    g_gain_tpchg[itab]->SetLineColor  (itab+1);
    g_gain_tpchg[itab]->SetMarkerColor(itab+1);
    if( g_gain_tpchg[itab]->GetN()>1 ) g_gain_tpchg[itab]->Fit("pol1");
    g_gain_tpchg[itab]->Sort();
    mg_tpchg->Add( g_gain_tpchg[itab] );
    leg_tpchg->AddEntry( g_gain_tpchg[itab],Form("%.2f mV",tpchg_tab_single[itab]) ,"PL" );
  }

  can2->cd(1);
  if( mg_vref->GetListOfGraphs()->GetSize() ){
    mg_vref->Draw("APL");
    for( Int_t itab=0; itab<ntab_vref; itab++ ){
      if( g_gain_vref[itab]->GetFunction("pol1")==NULL ) continue;
      tex1->DrawLatexNDC( 0.65, 0.50-0.07*itab, Form("slope = %.2f #pm %.2f",  g_gain_vref[itab]->GetFunction("pol1")->GetParameter(1), g_gain_vref[itab]->GetFunction("pol1")->GetParError(1)) );
      tex1->DrawLatexNDC( 0.65, 0.47-0.07*itab, Form("offset = %.2f #pm %.2f", g_gain_vref[itab]->GetFunction("pol1")->GetParameter(0), g_gain_vref[itab]->GetFunction("pol1")->GetParError(0)) );
    }
    //mg_vref->GetXaxis()->SetLimits( 0.0, mg_vref->GetXaxis()->GetXmax() );
    //mg_vref->SetMaximum(XXX);
    //mg_vref->SetMinimum(XXX);
    //mg_vref->Draw("APL");
    leg_vref->Draw();
  }
  
  can2->cd(2);
  if( mg_tpchg->GetListOfGraphs()->GetSize() ){
    mg_tpchg->Draw("APL");
    for( Int_t itab=0; itab<ntab_tpchg; itab++ ){
      if( g_gain_tpchg[itab]->GetFunction("pol1")==NULL ) continue;
      tex1->DrawLatexNDC( 0.65, 0.60-0.07*itab, Form("slope = %.2f #pm %.2f",  g_gain_tpchg[itab]->GetFunction("pol1")->GetParameter(1), g_gain_tpchg[itab]->GetFunction("pol1")->GetParError(1)) );
      tex1->DrawLatexNDC( 0.65, 0.57-0.07*itab, Form("offset = %.2f #pm %.2f", g_gain_tpchg[itab]->GetFunction("pol1")->GetParameter(0), g_gain_tpchg[itab]->GetFunction("pol1")->GetParError(0)) );
    }
    leg_tpchg->Draw();
  }

  // for TDR
  /*
  can2->cd(2);
  for( Int_t ig=0; ig<4; ig++ ){
    TLatex* tex_prd = new TLatex();
    tex_prd->SetTextColor(ig+1);
    tex_prd->SetTextSize(0.03);
    func[ig][0]->SetLineColor(ig);
    g_scurve[ig][0].SetLineColor(ig+1); g_scurve[ig][0].SetMarkerColor(ig+1); g_scurve[ig][0].Draw( ig ? "Psame" : "AP" );
    tex_prd->DrawLatexNDC( 0.70, 0.86-0.15*ig, Form("#mu = %.2f #pm %.2f",        func[ig][0]->GetParameter(0), func[ig][0]->GetParError(0)) );
    tex_prd->DrawLatexNDC( 0.70, 0.82-0.15*ig, Form("#sigma = %.2f #pm %.2f",     func[ig][0]->GetParameter(1), func[ig][0]->GetParError(1)) );
    tex_prd->DrawLatexNDC( 0.70, 0.78-0.15*ig, Form("#chi^{2}/NDF = %.2f / %d",   func[ig][0]->GetChisquare(),  func[ig][0]->GetNDF()      ) );
  }
  */

  can2->Update();
  can2->Print( Form("pic/%s_can2.ps",basename.c_str()) );

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  TTree* tree = new TTree( "ch", "ch" );
  Int_t t_ch = ch;
  tree->Branch( "ch",  &t_ch,  "ch/I" );
  tree->Fill();  
    
  TFile outfile( Form("pic/%s.root",basename.c_str()), "RECREATE" );
  tree->Write();
  Int_t tmp_cnt = 1;
  for( Int_t itab=0; itab<ntab; itab++ ){
    for( Int_t ig=0; ig<cnt_g[itab]; ig++ ){
      g_scurve[itab][ig].SetName ( Form("scurve_%d_%d", itab,ig) );
      g_scurve[itab][ig].SetTitle( Form("vref=%.2f, tpchg=%.2f, %d", vref_tab[itab], tpchg_tab[itab],ig) );
      g_scurve[itab][ig].GetXaxis()->SetTitle("DAC [bit]");
      g_scurve[itab][ig].GetYaxis()->SetTitle("Count Efficiency");
      g_scurve[itab][ig].Write();
      std::cout << tmp_cnt++ << " "
		<< g_scurve[itab][ig].GetFunction( Form("func%d_%d",itab,ig) )->GetParameter(0) << " 0 " // tmppppppp
		<< g_scurve[itab][ig].GetFunction( Form("func%d_%d",itab,ig) )->GetParError (0) << std::endl;
    }
  }
  for( Int_t itab=0; itab<ntab_vref;  itab++ ) g_gain_vref [itab]->Write();
  for( Int_t itab=0; itab<ntab_tpchg; itab++ ) g_gain_tpchg[itab]->Write();
  outfile.Close();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  std::cout << "finish" << std::endl;
  if( !gROOT->IsBatch() ) app.Run();
  return 0;
}
