#include<set>

void vref_scan( string filename = "./scan_results/scan_txt/scan_result_merge.txt"){

  cout << "//////////// Start code ////////////" << endl;

  double pulse=0;
  double Vref2, Vref5=0;
  double Run=0;
  // double pulse[99];
  // double Vref2[99], Vref5[99];
  // double Run[99];
  // int count2[512];
  // int count5[512];
  int count[1024]={};
  // int count[99][1024];

  // const int board =   2;
  // const int cp    =   4;
  // const int ch    = 128;
  // const int div   =  128/32;
  // const int part  = 32;

  std::set<int> noisy2{};
  std::set<int> noisy5{};

  // noisy2.count(440); To count/classify noisy ch

  ifstream ifs(filename.c_str(), std::ios::in);

  TCanvas* c1 = new TCanvas("c1","c1");
  TCanvas* c2 = new TCanvas("c2","c2");

  ////////// Only ONE RUN data reading /////////

  ifs >> Run >> pulse >> Vref2 >> Vref5;

  cout << "Run# = "  << Run   << endl;
  cout << "Pulse = " << pulse << endl;
  cout << "Vref2 = " << Vref2 << endl;
  cout << "Vref5 = " << Vref5 << endl;

  for(int i = 0 ; i < 1024 ; i++){
    ifs >> count[i];
    // cout << "Board2::" << i << "::" << count[i] << endl;
    // cout << "Board5::" << i << "::" << count[i+512] << endl;
    cout << count[i] << " " ;
  }


  cout << endl;

  ////////// Reading merged run data /////////

  // cout << "//////////// BEFORE stargint LOOP ////////////" << endl;

  // int j = 0;
  // while( pulse[j] !=0 ){

  //   cout << "//////////// Starting LOOP ////////////" << endl;
    

  //   ifs >> Run[j] >> pulse[j] >> Vref2[j] >> Vref5[j];

  //   cout << "Pulse = " << pulse[j] << endl;
  //   cout << "Vref2 = " << Vref2[j] << endl;
  //   cout << "Vref5 = " << Vref5[j] << endl;

  //   for(int i = 0 ; i < 1024 ; i++){
  //     ifs >> count[j][i];
  //     // cout << "Board2::" << i << "::" << count[i] << endl;
  //     // cout << "Board5::" << i << "::" << count[i+512] << endl;
  //     cout << count[j][i] << " " ;
  //   }

  //   cout << endl;

  //   j++;

  // }

  //   cout << "//////////// Ending LOOP ////////////" << endl;


  //////////// Data READING ////////////

  TGraphErrors* gref2 = new TGraphErrors();
  TGraphErrors* gref5 = new TGraphErrors();
  // TGraphErrors** gref2 = new TGraphErrors*[j];
  // TGraphErrors** gref5 = new TGraphErrors*[j];
  // for( int jj=0; jj<j; jj+ ){
  //   gref2[jj] = new TGraphErrors();
  //   gref5[jj] = new TGraphErrors();
    
  // for(int i = 0 ; i < 512 ; i++){
  //     gref2[jj]->SetPoint(i,i,count[jj][i]/pulse[jj]);
  //     gref5[jj]->SetPoint(i,i,count[jj][i+512]/pulse[jj]);
  //   }
  // }

  for(int i = 0 ; i < 512 ; i++){
      gref2->SetPoint(i,i,count[i]/pulse);
      gref5->SetPoint(i,i,count[i+512]/pulse);
  }

  cout << pulse << endl;

  for(int i = 0 ; i < 1024 ; i++){
    cout << count[i]/pulse << " " ;
  }
  cout << endl;

  c1->cd();
  gref2->Draw("al");

  c2->cd();
  gref5->Draw("al");


  // c1->cd();
  // gref2[]->Draw("al");

  // c2->cd();
  // gref5[]->Draw("al");


}
