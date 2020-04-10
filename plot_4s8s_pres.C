
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

void plot_4s8s_pres(int run_num, int run_num2 = 0){

    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);

    if (run_num2 < run_num){
        run_num2 = run_num;
    }

    //Variables
    const int num_det = 2;
    const int rebin = 5;
    const int num_win = 8;
    
    //Histograms
    TH1F *hCycle = new TH1F("hCycle", "hCycle", 3000, -10, 20);
    TH1F *hIrr   = new TH1F("239Pu_BeamOn_BothClovers", "hIrr", 30000, 0, 3000);
    TH1F *hCount = new TH1F("239Pu_BothClovers", "hCount", 30000, 0, 3000);
    TH1F *hEnCount[num_det];
    TH1F *hEnWin[num_win][3]; //[window 1-3][clover 1, 2, summed]
    TH1F *hEnClover[2]; //[window 1-3][clover 1, 2, summed]

    for (int i=0; i<num_win; i++){ //window
        for (int j=0; j<2; j++){ //clover
            hEnWin[i][j] = new TH1F(Form("239Pu_Time%i_Clover%i", i+1, j+1), Form("hEnWin%iClover%i", i, j), 30000, 0, 3000);
        }
        hEnWin[i][2] = new TH1F(Form("239Pu_Time%i_BothClovers", i+1), Form("hEnWin%i", i), 30000, 0, 3000);
    }
    for (int i=0; i<2; i++){
        hEnClover[i] = new TH1F(Form("239Pu_Clover%i", i+1), Form("hEnClover%i", i), 30000, 0, 3000);
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

    TCanvas *cClover = new TCanvas("cClover","Clover Both",800, 500);
    THStack *hs = new THStack("hs", "hs");
    gStyle->SetPalette(kIsland);
    TColor::InvertPalette();

    for (int i=0; i<num_win; i++){
        hEnWin[i][2]->Rebin(rebin);
        hs->Add(hEnWin[i][2]);
    }

    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.05);

    hs->Draw();
    hs->SetTitle("");
    hs->GetXaxis()->SetTitle("E_{#gamma} [keV]");
    hs->GetYaxis()->SetTitle("Counts/(0.5 keV s)");
    hs->GetXaxis()->SetTitleSize(0.06);
    hs->GetYaxis()->SetTitleSize(0.06);

    hs->GetXaxis()->SetRangeUser(500, 600);
    hs->GetXaxis()->SetRangeUser(430, 510);
    hs->GetXaxis()->SetRangeUser(760, 860);
    hs->GetXaxis()->SetRangeUser(1062, 1140);
    hs->SetMaximum(800);

    
    hs->Draw("pfc nostack");
    //hs->Draw("pfc ");



}
