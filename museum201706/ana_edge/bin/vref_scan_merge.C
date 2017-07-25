#include<set>

void vref_scan_merge( string filename = "./scan_results/scan_txt/scan_result_merge.txt"){

  cout << "//////////// Start code ////////////" << endl;

  // double pulse;
  // double Vref2, Vref5;
  // double Run;
  double pulse[99]={};
  double Vref2[99]={};
  double Vref5[99]={};
  double Run[99]={};
  const int nrun = 2;
  double unit2[99]={};
  double unit5[99]={};
  // int count2[512];
  // int count5[512];
  // int count[1024];
  int count[99][1024]={};

  // const int board =   2;
  // const int cp    =   4;
  // const int ch    = 128;
  // const int div   =  128/32;
  // const int part  = 32;

  std::set<int> noisy2{255};
  std::set<int> noisy5{3,200,256,311,385,448};

  // noisy2.count(440); To count/classify noisy ch

  ifstream ifs(filename.c_str(), std::ios::in);

  TCanvas* c1 = new TCanvas("c1","c1");
  TCanvas* c2 = new TCanvas("c2","c2");
  TCanvas* c3 = new TCanvas("c3","c3");
  TCanvas* c4 = new TCanvas("c4","c4");

  TCanvas* c5 = new TCanvas("c5","c5");
  TCanvas* c6 = new TCanvas("c6","c6");

  ////////// Only ONE RUN data reading /////////

  // ifs >> Run >> pulse >> Vref2 >> Vref5;

  // cout << "Pulse = " << pulse << endl;
  // cout << "Vref2 = " << Vref2 << endl;
  // cout << "Vref5 = " << Vref5 << endl;

  // for(int i = 0 ; i < 1024 ; i++){
  //   ifs >> count[i];
  //   // cout << "Board2::" << i << "::" << count[i] << endl;
  //   // cout << "Board5::" << i << "::" << count[i+512] << endl;
  //   cout << count[i] << " " ;
  // }

  // cout << endl;

  ////////// Reading merged run data /////////

  cout << "//////////// BEFORE stargint LOOP ////////////" << endl;

  int j = 0;
  while( j != nrun ){

    cout << "//////////// Starting LOOP ////////////" << endl;
   

    ifs >> Run[j] >> pulse[j] >> Vref2[j] >> Vref5[j];

    cout << "Run# = "  << Run[j]   << endl;
    cout << "Pulse = " << pulse[j] << endl;
    cout << "Vref2 = " << Vref2[j] << endl;
    cout << "Vref5 = " << Vref5[j] << endl;

    for(int i = 0 ; i < 1024 ; i++){

      ifs >> count[j][i];
      // cout << "Board2::" << i << "::" << count[i] << endl;
      // cout << "Board5::" << i << "::" << count[i+512] << endl;
      // cout << count[j][i] << " " << "missing?";
    }

    cout << endl;

    j++;

  }
  for(int j = 0 ; j < nrun ; j++){
	for(int i = 0; i < 1024 ; i++){
	  cout << count[j][i] << " " ;
	}
	cout << endl;
  }

  // int j = 0;

    cout << "//////////// Ending LOOP ////////////" << endl;
  // for(int i = 0 ; i < 512 ; i++){
  //     ifs >> count2[i];
  //     gref2->SetPoint(i,i,count2[i]);
  //     cout << "Board2::" << i << "::" << count2[i] << endl;
  // }

  // for(int i = 0 ; i < 512 ; i++){
  //     ifs >> count5[i];
  //     gref5->SetPoint(i,i,count2[i]);
  //     cout << "Board5::" << i << "::" << count2[i] << endl;
  // }


  TGraphErrors* gref2 = new TGraphErrors();
  TGraphErrors* gref5 = new TGraphErrors();
  TGraphErrors* gref2_2 = new TGraphErrors();
  TGraphErrors* gref5_2 = new TGraphErrors();

  TGraphErrors* gv2 = new TGraphErrors();
  TGraphErrors* gv5 = new TGraphErrors();

  gv2->SetMarkerStyle(8);
  gv2->SetMarkerSize(1);
  gv5->SetMarkerStyle(8);
  gv5->SetMarkerSize(1);
  // TGraphErrors** gref2 = new TGraphErrors*[j];
  // TGraphErrors** gref5 = new TGraphErrors*[j];

  // for( int jj=0; jj<j; jj+ ){
  //   gref2[jj] = new TGraphErrors();
  //   gref5[jj] = new TGraphErrors();
    
  //   for(int i = 0 ; i < 512 ; i++){
  //     gref2[jj]->SetPoint(i,i,count[jj][i]/pulse[jj]);
  //     gref5[jj]->SetPoint(i,i,count[jj][i+512]/pulse[jj]);
  //   }
  // }

  // for(int i = 0 ; i < 512 ; i++){
  //   gref2[0]->SetPoint(i,i,count[0][i]/pulse[0]);
  //   gref5[0]->SetPoint(i,i,count[0][i+512]/pulse[0]);
  // }

  for(int i = 0 ; i < 512 ; i++){
      gref2->SetPoint(i,i,count[0][i]/pulse[0]);
      gref5->SetPoint(i,i,count[0][i+512]/pulse[0]);
  }

  for(int i = 0 ; i < 512 ; i++){
      gref2_2->SetPoint(i,i,count[1][i]/pulse[1]);
      gref5_2->SetPoint(i,i,count[1][i+512]/pulse[1]);
  }

  for(int k = 0 ; k < nrun ; k++){
    cout << pulse[k] << endl;
    for(int i = 0 ; i < 1024 ; i++){
      cout << count[k][i]/pulse[k] << " " ;
    }
    cout << endl;
  }

  // for(int i = 0 ; i < 32 ; i++){
  //   for(int k = 0 ; k < 32 ; k++){
  //     unit[i]+=count[k+i*32]

  for(int k = 0 ; k < nrun ; k++){
    for(int i = 0 ; i < 512 ; i++){
      unit2[k] += count[k][i]/pulse[k];
      unit5[k] += count[k][i+512]/pulse[k];
    }
  }

  cout << "Vref scan data analize"  << endl;

  for(int i = 0 ; i < nrun ; i++){
    gv2->SetPoint(i,Vref2[i],unit2[i]);
    gv5->SetPoint(i,Vref5[i],unit5[i]);
    cout << i << " " << Vref2[i] << " " << unit2[i] << endl;
    cout << i << " " << Vref5[i] << " " << unit5[i] << endl;
  }


  // cout << endl;

  c1->cd();
  gref2->Draw("al");

  c2->cd();
  gref5->Draw("al");

  c3->cd();
  gref2_2->Draw("al");

  c4->cd();
  gref5_2->Draw("al");

  // c1->cd();
  // gref2_2->Draw("l");
  // gref2_2->SetLineColors(2);

  // c2->cd();
  // gref5_2->Draw("l");
  // gref5_2->SetLineColors(2);

  c5->cd();
  // gv2->SetLogy();
  c5->SetLogy();
  gv2->Draw("ap");

  c6->cd();
  // gv5->SetLogY();
  c6->SetLogy();
  gv5->Draw("ap");

}
