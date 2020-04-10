
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

void plot_10s20s(int run_num, int run_num2 = 0){

    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);

    if (run_num2 < run_num){
        run_num2 = run_num;
    }

    //Variables
    const int num_det = 4;
    const int num_win = 10;
    const int rebin = 1;
    
    //Histograms
    TH1F *hCycle = new TH1F("hCycle", "hCycle", 5000, -10, 40);
    TH1F *hIrr   = new TH1F("239Pu_BeamOn_BothDets", "hIrr", 30000, 0, 3000);
    TH1F *hCount = new TH1F("239Pu_BothDets", "hCount", 30000, 0, 3000);
    TH1F *hEnCount[num_det];
    TH1F *hEnWin[num_win][num_det]; //[window 1-3][clover 1, 2, summed]

    for (int i=0; i<num_win; i++){ //window
        for (int j=0; j<num_det; j++){ //clover
            //hEnWin[i][j] = new TH1F(Form("Runs321-323_Time%i_Det%i", i+1, j+1), Form("hEnWin%iDet%i", i, j), 40000, 0, 4000);
            hEnWin[i][j] = new TH1F(Form("Runs%i-%i_Time%i_Det%i", run_num, run_num2, i+1, j+1), Form("hEnWin%iDet%i", i, j), 40000, 0, 4000);
        }
    }
    for (int i=0; i<num_det; i++){
        hEnCount[i] = new TH1F(Form("hEnCount%i", i), Form("hEnCount%i", i), 40000, 0, 4000);
    }

    //get histos
    for (int k=run_num; k<=run_num2; k++){
        cout << "Run number:  " << k << endl;
        TFile *fHist = new TFile(Form("data_hist/RABBIT_%i.root",k));

        hCycle->Add((TH1F*) fHist->Get("hCycle"));
        hIrr->Add((TH1F*) fHist->Get("hIrr"));
        hCount->Add((TH1F*) fHist->Get("hCount"));
        for (int i=0; i<num_win; i++){
            for (int j=0; j<num_det; j++){
                hEnWin[i][j]->Add((TH1F*) fHist->Get(Form("hEnWin%i%i", i, j)));
            }
        }
        for (int i=0; i<num_det; i++){
            hEnCount[i]->Add((TH1F*) fHist->Get(Form("hEnCount%i", i)));
        }

    //get histos

        fHist->Close();
        delete fHist;
    }

    TCanvas *cEn = new TCanvas("cEn","Summed segments",1000, 400);
    hCount->Rebin(rebin);
    hIrr->Rebin(rebin);
    hCount->SetLineColor(2);
    hIrr->Draw();
    hCount->Draw("same");

    TCanvas *cCycle = new TCanvas("cCycle","Cycle",1000, 400);
    hCycle->Draw();

    TCanvas *cClover1 = new TCanvas("cDet1","Det 1",1000, 400);
    for (int i=0; i<num_win; i++){
        hEnWin[i][0]->Rebin(rebin);
        hEnWin[i][0]->SetLineColor(i+2);
        hEnWin[i][0]->Draw("same");
    }

    TCanvas *cClover2 = new TCanvas("cDet2","Det 2",1000, 400);
    for (int i=0; i<num_win; i++){
        hEnWin[i][1]->Rebin(rebin);
        hEnWin[i][1]->SetLineColor(i+2);
        hEnWin[i][1]->Draw("same");
    }

    //hist2TKA(hCount);
    //hist2TKA(hIrr);
    for (int i=0; i<num_win; i++){
        for (int j=0; j<2; j++){
            hist2TKA(hEnWin[i][j]);
        }
    }

}
