#include <iomanip>
#include <iostream>
#include <fstream>
#include <TROOT.h>
#include <TApplication.h>

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(true);
  TApplication app( "app", &argc, argv );

  if( !(app.Argc()==5 || app.Argc()==6) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (int)board (char*)chip (int)channel (char*)command [(char*)command for others]" << std::endl
	      << std::endl, abort();
  Int_t   BOARD         = atoi(app.Argv(1));
  Char_t* CHIP          = app.Argv(2);
  Int_t   CHANNEL       = atoi(app.Argv(3));
  Char_t* COMMAND       = app.Argv(4);
  const Char_t* COMMAND_OTHER = ( app.Argc()==6 ? app.Argv(5) : "LLLLLLLLLLLLLLLL" );

  system("mkdir -p files");
  ofstream fout;
  fout.open( Form("files/control_%d_%s.dat",BOARD,CHIP) );
  
  fout << "#"            << std::endl;
  fout << "# dip switch" << std::endl;
  fout << CHIP           << std::endl;
  fout << "#"            << std::endl;

  Int_t CNT_CH = 0;
  Int_t CH_ID  = 127;

  while( 1 ){
    fout << std::setw(5) << std::left << CNT_CH;
    //if( CH_ID == CHANNEL ) fout << COMMAND       << std::endl;
    if( CH_ID%32 == CHANNEL ) fout << COMMAND       << std::endl;
    else                      fout << COMMAND_OTHER << std::endl;
    if( CNT_CH==127) break;
    
    CNT_CH++;
    CH_ID--;
  }
  
  fout.close();
  return 0;
}
