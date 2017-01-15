#include "setting.h"


Int_t main( Int_t argc, Char_t** argv ){

  if( argc != 2 ){
    std::cerr << "wrong input" << std::endl
	      << "Usage : (char*)infile" << std::endl
	      << std::endl;
    abort();
  }

  // --------------------------------------------------------------------  
  const Char_t* infile = argv[1];

  // --------------------------------------------------------------------
  TChain* chain = new TChain( "slit128A" );
  Int_t nfile = chain->Add( infile );
  //std::cout << nfile << std::endl;
  //std::cout << chain->GetEntries() << std::endl;
  set_readbranch     ( chain );
  set_readbranch_scan( chain );
  // --------------------------------------------------------------------
  TTree* newtree;
  if( chain->GetEntries()==0 ) std::cerr << "No entry : " << infile << std::endl, abort();
  newtree = chain->CloneTree(0,"newtree");
  set_tree( newtree );

  for( Int_t ievt=0; ievt<chain->GetEntries(); ievt++ ){
    chain->GetEntry(ievt);

    // BEGIN MODIFICATION
    t_tpchg = 5.0;
    // END MODIFICATION

    newtree->Fill();
  }

  const Char_t* outfile = "test.root";
  TFile* rootf = new TFile( outfile, "RECREATE" );
  newtree->Write();
  rootf->Close();

  delete chain;
  delete newtree;
  delete rootf;
  return 0;
}
