#include <iomanip>
#include <iostream>
#include <fstream>
#include <TROOT.h>
#include <TApplication.h>

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(true);
  TApplication app( "app", &argc, argv );

  if( !(app.Argc()==6 || app.Argc()==7) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (int)board (char*)chip (int)channel (int)cycle (char*)command [(char*)command for others]" << std::endl
	      << "      command : 16 bit"
	      << std::endl, abort();
  Int_t   BOARD         = atoi(app.Argv(1));
  Char_t* CHIP          =      app.Argv(2);
  Int_t   CHANNEL       = atoi(app.Argv(3));
  Int_t   CYCLE         = atoi(app.Argv(4));
  Char_t* COMMAND       =      app.Argv(5);
  const Char_t* COMMAND_OTHER = ( app.Argc()==7 ? app.Argv(6) : "LLLLLLLLLLLLLLLL" );

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
    if( CH_ID%CYCLE==CHANNEL ) fout << COMMAND       << std::endl;
    else                       fout << COMMAND_OTHER << std::endl;
    if( CNT_CH==127) break;
    
    CNT_CH++;
    CH_ID--;
  }
  
  fout.close();
  return 0;
}
