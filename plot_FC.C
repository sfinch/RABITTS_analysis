
using namespace std;
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "TRint.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TH1.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TCut.h"

void plot_FC(int run_num){


    //Variables
    const int num_det = 2;
    const int rebin = 1;
    int dets[num_det] = {6, 7};
    
    //Histograms
    TH1F *hFC[num_det];

    //get histos
    TFile *fHist = new TFile(Form("data_root/RABITTS10_%i.root", run_num));

    TCanvas *cFC = new TCanvas("cFC","Summed segments",1000, 400);
    cFC->Divide(num_det);

    for (int j=0; j<num_det; j++){
        hFC[j] = (TH1F*) (fHist->Get(Form("histos_SCP/hADC%i", dets[j])))->Clone();
        hFC[j]->SetName(Form("run%i_Det%i", run_num, j+1));
        cout << hFC[j]->Integral(2000, 65000) << endl;
        hFC[j]->Rebin(16);
        hFC[j]->GetXaxis()->SetRangeUser(2000, 65000);
        cFC->cd(j+1);
        hFC[j]->Draw();
    }




}
