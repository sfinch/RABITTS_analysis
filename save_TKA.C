
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

#include "src/hist2TKA.C"


void save_TKA(int run_num){


    //Variables
    const int num_det = 2;
    const int rebin = 1;
    int dets[num_det] = {0, 1};
    
    //Histograms
    TH1F *hEn[num_det];

    //get histos
    TFile *fHist = new TFile(Form("data_processed/RABITTS10_%i.root", run_num));

    TCanvas *cEn = new TCanvas("cEn","Summed segments",1000, 400);
    cEn->Divide(num_det);

    for (int j=0; j<num_det; j++){
        hEn[j] = (TH1F*) (fHist->Get(Form("histos/hEn%i", dets[j])))->Clone();
        hEn[j]->SetName(Form("Run%i_Det%i", run_num, j+1));
        cEn->cd(j+1);
        hEn[j]->Draw();
        hist2TKA(hEn[j]);
    }




}
