#include "setting.h"

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(true);
  TApplication app( "app", &argc, argv );
  TStyle* sty = Style();

  if( (app.Argc()<2) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (char*)datfilename (char*)datfilename_tab (char*)datfilename_tab_vref (char*)datfilename_tab_tpchg" << std::endl
	      << std::endl, abort();
  Char_t* infilename           = app.Argv(1);
  std::string basename = gSystem->BaseName( infilename );
  basename.erase( basename.rfind(".dat") );
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  TTree* tree           = new TTree( "scurve",     "scurve"     );
  TTree* tree_tab       = new TTree( "vref_tpchg", "vref_tpchg" );
  TTree* tree_tab_vref  = new TTree( "vref",       "vref"       );
  TTree* tree_tab_tpchg = new TTree( "tpchg",      "tpchg"      );
  tree->ReadFile( infilename,           "dac/I:vref/F:tpchg/F:sigeff/F:sigeffE/F:ringprob/F:ringprobE/F:sigwidth/F:sigwidthE/F:span/F:spanE/F:board/I:chip/I:channel/I" );

  system( Form("mkdir -p %s", "root_data") );
  TFile outfile( Form("root_data/%s.root",basename.c_str()), "RECREATE" );
  tree->Write();

  if( app.Argc()>2 ){
    Char_t* infilename_tab = app.Argv(2);
    tree_tab->ReadFile( infilename_tab, "no/I:vref/F:tpchg/F" );
    tree_tab->Write();
  }
  if( app.Argc()>3 ){
    Char_t* infilename_tab_vref  = app.Argv(3);
    tree_tab_vref->ReadFile( infilename_tab_vref, "no/I:vref/F" );
    tree_tab_vref->Write();
  }
  if( app.Argc()>4 ){
    Char_t* infilename_tab_tpchg = app.Argv(4);
    tree_tab_tpchg->ReadFile( infilename_tab_tpchg, "no/I:tpchg/F" );
    tree_tab_tpchg->Write();
  }

  outfile.Close();
  delete tree;
  delete tree_tab;
  delete tree_tab_vref;
  delete tree_tab_tpchg;
  
  return 0;
}
