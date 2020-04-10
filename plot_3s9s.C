
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

void plot_3s9s(int run_num, int run_num2 = 0){

    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);

    if (run_num2 < run_num){
        run_num2 = run_num;
    }

    //Variables
    const int num_det = 2;
    const int rebin = 1;
    const int num_win = 3;
    //const int num_win = 9;
    
    //Histograms
    TH1F *hCycle = new TH1F("hCycle", "hCycle", 3000, -10, 20);
    TH1F *hIrr   = new TH1F("238U_BeamOn_BothDets", "hIrr", 30000, 0, 3000);
    TH1F *hCount = new TH1F("238U_BothDets", "hCount", 30000, 0, 3000);
    TH1F *hEnCount[num_det];
    TH1F *hEnWin[num_win][3]; //[window 1-3][clover 1, 2, summed]
    TH1F *hEnClover[2]; //[window 1-3][clover 1, 2, summed]

    for (int i=0; i<num_win; i++){ //window
        for (int j=0; j<2; j++){ //clover
            hEnWin[i][j] = new TH1F(Form("238U_Time%i_Det%i", i+1, j+1), Form("hEnWin%iDet%i", i, j), 30000, 0, 3000);
        }
        hEnWin[i][2] = new TH1F(Form("238U_Time%i_BothDets", i+1), Form("hEnWin%i", i), 30000, 0, 3000);
    }
    for (int i=0; i<2; i++){
        hEnClover[i] = new TH1F(Form("238U_Det%i", i+1), Form("hEnDet%i", i), 30000, 0, 3000);
    }
    for (int i=0; i<num_det; i++){
        hEnCount[i] = new TH1F(Form("hEnCount%i", i), Form("hEnCount%i", i), 30000, 0, 3000);
    }

    //get histos
    for (int k=run_num; k<=run_num2; k++){
        cout << "Run number:  " << k << endl;
        TFile *fHist = new TFile(Form("data_hist/RABBIT_%i.root",k));

        hCycle->Add((TH1F*) fHist->Get("hCycle"));
        hIrr->Add((TH1F*) fHist->Get("hIrr"));
        hCount->Add((TH1F*) fHist->Get("hCount"));
        for (int i=0; i<num_win; i++){
            for (int j=0; j<3; j++){
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
    for (int i=0; i<num_win; i++){
        hEnClover[0]->Add(hEnWin[i][0]);
        hEnClover[1]->Add(hEnWin[i][1]);
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
    }
    hEnClover[0]->Rebin(rebin);
    hEnClover[0]->Draw();
    hEnWin[0][0]->Draw("same");
    hEnWin[1][0]->Draw("same");
    hEnWin[2][0]->Draw("same");

    TCanvas *cClover2 = new TCanvas("cDet2","Det 2",1000, 400);
    for (int i=0; i<num_win; i++){
        hEnWin[i][1]->Rebin(rebin);
        hEnWin[i][1]->SetLineColor(i+2);
    }
    hEnClover[1]->Rebin(rebin);
    hEnClover[1]->Draw();
    hEnWin[0][1]->Draw("same");
    hEnWin[1][1]->Draw("same");
    hEnWin[2][1]->Draw("same");

    TCanvas *cClover = new TCanvas("cDet","Both dets",1000, 400);
    for (int i=0; i<num_win; i++){
        hEnWin[i][2]->Rebin(rebin);
        hEnWin[i][2]->SetLineColor(i+1);
    }
    hEnWin[0][2]->Draw();
    hEnWin[1][2]->Draw("same");
    hEnWin[2][2]->Draw("same");
    //hEnWin[3][2]->Draw("same");
    //hEnWin[4][2]->Draw("same");
    //hEnWin[5][2]->Draw("same");
    //hEnWin[6][2]->Draw("same");
    //hEnWin[7][2]->Draw("same");
    //hEnWin[8][2]->Draw("same");

    //hist2TKA(hCount);
    //hist2TKA(hIrr);
    for (int i=0; i<num_win; i++){
        for (int j=0; j<3; j++){
            hist2TKA(hEnWin[i][j]);
        }
    }
    //for (int i=0; i<2; i++){
    //    hist2TKA(hEnClover[i]);
    //}

}
