#include <iomanip>
#include <iostream>
#include <fstream>
#include <TROOT.h>
#include <TApplication.h>
#include <TGraph.h>

const Int_t n_channel     = 128;
const Int_t ONEMIP        = 3.84; // [fC]
const Bool_t fl_meanshift = true;
const Bool_t fl_message   = !true;

Int_t Rounding( Double_t num ){
  if( num>0 ) return Int_t(num+0.5);
  else        return Int_t(num-0.5);
}
TString Dec2Bin( Int_t num ){
  TString str;
  if( num>=31 ){
    str += "HHHHHH";
    return str;
  }else if( num<=-31 ){
    str += "LLLLLL";
    return str;
  }
  
  if( num >=0 ) str += "H";
  else          str += "L";
  num = abs(num);
  for( Int_t i=4; i>=0; i-- ){
    if( pow(2,i) <= num ){
      str += "H";
      num -= pow(2,i);
    }else{
      str += "L";
    }
  }

  return str;
}

Int_t main( Int_t argc, Char_t** argv ){
  gROOT->SetBatch(true);
  TApplication app( "app", &argc, argv );

  if( !(app.Argc()==8 || app.Argc()==9) )
    std::cerr << "Wrong input" << std::endl
	      << "Usage : " << app.Argv(0) << " (int)board (char*)chip (char*)datfile (double)threshold_mip (int)channel (int)cycle (char*)command [(char*)command for others]" << std::endl
	      << "      command : 3 bit [digital-output/analog-monitor/test-pulse-in]"
	      << std::endl, abort();
  Int_t   BOARD         = atoi(app.Argv(1));
  Char_t* CHIP          =      app.Argv(2);
  Char_t* INFILE        =      app.Argv(3);
  Float_t THRESHOLD_MIP = atof(app.Argv(4));
  Int_t   CHANNEL       = atoi(app.Argv(5));
  Int_t   CYCLE         = atoi(app.Argv(6));
  Char_t* COMMAND       =      app.Argv(7);
  const Char_t* COMMAND_OTHER = ( app.Argc()==9 ? app.Argv(8) : "LLL" );

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  system("mkdir -p files_calib");
  ifstream fin;
  fin.open( INFILE, std::ios::in );
  std::string sTmp;

  Int_t   tmp_board_id;
  Int_t   tmp_chip_id;
  Int_t   tmp_unit_id;
  Int_t   tmp_lchannel;
  Int_t   tmp_gchannel;
  Int_t   tmp_fl_alive;
  Int_t   fl_alive[n_channel];
  Float_t gain    [n_channel];
  Float_t gainE   [n_channel];
  Float_t offset  [n_channel];
  Float_t offsetE [n_channel];
  Float_t noise   [n_channel];
  Float_t noiseE  [n_channel];

  Float_t rawDAC  [n_channel];
  TGraph* g = new TGraph();

  Int_t tmp_cnt = 0;
  while( getline(fin,sTmp) ){
    if( tmp_cnt>=n_channel ) std::cerr << "[ABORT] Wrong calibration dat-file : " << INFILE << std::endl, abort();
    sscanf(sTmp.data(), "%d %d %d %d %d %d %f %f %f %f %f %f",
	   &tmp_board_id,&tmp_chip_id,&tmp_unit_id,&tmp_lchannel,&tmp_gchannel,&fl_alive[tmp_cnt],&gain[tmp_cnt],&gainE[tmp_cnt],&offset[tmp_cnt],&offsetE[tmp_cnt],&noise[tmp_cnt],&noiseE[tmp_cnt] );
    if( fl_alive[tmp_cnt] ){
      rawDAC[tmp_cnt] = gain[tmp_cnt]*THRESHOLD_MIP*ONEMIP + offset[tmp_cnt];
      g->SetPoint( g->GetN(), tmp_cnt, rawDAC[tmp_cnt] );
    }else{
      rawDAC[tmp_cnt] = 999; // dead-channel
    }
    
    tmp_cnt++;
  }
  /* // for debug
  for( Int_t ichannel=0; ichannel<n_channel; ichannel++ ){
    std::cout << ichannel << " : " << fl_alive[ichannel] << " : "
	      << "gain = "   << gain  [ichannel] << " +- " << gainE  [ichannel] << ", "
	      << "offset = " << offset[ichannel] << " +- " << offsetE[ichannel] << ", "
	      << "noise = "  << noise [ichannel] << " +- " << noiseE [ichannel]
	      << std::endl;
  }
  */

  Float_t DAC[n_channel];
  Double_t mean = g->GetMean(2);
  if( fl_meanshift ){
    for( Int_t ichannel=0; ichannel<n_channel; ichannel++ ){
      if( fl_alive[ichannel] ) DAC[ichannel] = rawDAC[ichannel] - mean;
      else                     DAC[ichannel] = rawDAC[ichannel];
    }
  }

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ofstream fout;
  fout.open( Form("files_calib/control_%d_%s.dat",BOARD,CHIP) );
  
  fout << "#"            << std::endl;
  fout << "# dip switch" << std::endl;
  fout << CHIP           << std::endl;
  fout << "#"            << std::endl;

  Int_t CNT_CH = 0;
  Int_t CH_ID  = 127;

  while( 1 ){
    fout << std::setw(5) << std::left << CNT_CH;
    fout << "LLLLL";


    Int_t CTRL_DAC = Rounding(DAC[CH_ID]);
    TString str = Dec2Bin(CTRL_DAC);
    if( fl_message ) std::cout << CH_ID << " : " << fl_alive[CH_ID] << " : DAC = " << rawDAC[CH_ID] << " -> " << DAC[CH_ID] << " -> " << CTRL_DAC << " -> " << str.Data() << std::endl;

    fout << str.Data();

    fout << "LL";
    if( CH_ID%CYCLE==CHANNEL ) fout << COMMAND       << std::endl;
    else                       fout << COMMAND_OTHER << std::endl;

    if( CNT_CH==127) break;
    
    CNT_CH++;
    CH_ID--;
  }
  
  fout.close();
  return 0;
}
