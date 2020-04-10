
#include <iostream>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>

#include "include/processed.h"

using std::cout;
using std::cerr;
using std::endl;

void plot_nmon(int run_num){

    //Variables
    TH1F *hCycNmon  = new TH1F("hCycNmon", "hCycNmon", 20000, -1, 19);
    TH1F *hNmonT;
    TH1F *hCycFC[2];
    TH1F *hHPGe[2];
    for (int i=0; i<2; i++){
        hCycFC[i] = new TH1F(Form("hCycFC%i", i), Form("hCycFC%i", i), 2000, -1, 19);
        hHPGe[i] = new TH1F(Form("hHPGe%i", i), Form("hHPGe%i", i), 2000, -1, 19);
    }

    //cuts
    int min_nmon_E = 100;
    int min_FC_E[2] = {2000, 2000};
    int min_HPGe_E[2] = {100, 100};

    //in file
    processed rabbit(run_num);

    //loop over data
    Long64_t nentries = rabbit.fChain->GetEntriesFast();
    Long64_t nbytes = 0, nb = 0;


    rabbit.GetEntry(nentries-1);
    //Double_t total_time = rabbit.seconds;
    //hNmonT  = new TH1F("hNmonT", "hNmonT", 1*total_time, 0, total_time);
    
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        nb = rabbit.GetEntry(jentry);   nbytes += nb;
        if (jentry%100000==0){
            cout << '\r' << "Processing event " << jentry;
        }
        if (rabbit.ADC[8]>min_nmon_E){
            hCycNmon->Fill(rabbit.cycle_time);
            //hNmonT->Fill(rabbit.seconds);
        }
        if (rabbit.ADC[6]>min_FC_E[0]){
            hCycFC[0]->Fill(rabbit.cycle_time);
        }
        if (rabbit.ADC[7]>min_FC_E[1]){
            hCycFC[1]->Fill(rabbit.cycle_time);
        }
        if (rabbit.En[0]>min_HPGe_E[0]){
            hHPGe[0]->Fill(rabbit.cycle_time);
        }
        if (rabbit.En[1]>min_HPGe_E[1]){
            hHPGe[1]->Fill(rabbit.cycle_time);
        }

    }//end loop over events

    //plot
    TCanvas *cCycle = new TCanvas("cCycle", "cCycle", 500, 1000);
    cCycle->Divide(1,5);
    cCycle->cd(1);
    hCycNmon->Draw();
    gPad->SetLogy();
    cCycle->cd(2);
    hCycFC[0]->Draw();
    cCycle->cd(3);
    hCycFC[1]->Draw();
    cCycle->cd(4);
    hHPGe[0]->Draw();
    cCycle->cd(5);
    hHPGe[1]->Draw();

    //new TCanvas();
    //hHPGe[0]->Draw();
    //new TCanvas();
    //hHPGe[1]->Draw();
    //new TCanvas();
    //hNmonT->Draw();
    
    
    TCanvas *cNmon = new TCanvas("cNmon", "cNmon", 600, 400);
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.05);
    hCycNmon->GetXaxis()->SetTitleSize(0.06);
    hCycNmon->GetYaxis()->SetTitleSize(0.06);
    hCycNmon->SetTitle("");
    hCycNmon->GetXaxis()->SetTitle("Time [s]");
    hCycNmon->GetYaxis()->SetTitle("Neutron flux [arb]");
    hCycNmon->GetXaxis()->SetRangeUser(0,0.3);
    hCycNmon->SetLineWidth(2);
    hCycNmon->Draw();



    //save to file
    cCycle->SaveAs(Form("time_spec/run%i.png", run_num));
    cCycle->SaveAs(Form("time_spec/run%i.C", run_num));
    
}
