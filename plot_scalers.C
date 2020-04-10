
#include <iostream>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>

#include "include/processed.h"

using std::cout;
using std::cerr;
using std::endl;

void plot_scalers(int run_num){

    TApplication* theApp = new TApplication("App", 0, 0);
     
    //Variables
    TH1F *hNmonT;
    TH1F *hHPGe[2];

    double bin_size = 0.5; //time in s

    //cuts
    int min_nmon_E = 100;
    int min_FC_E[2] = {1000, 1000};
    int min_HPGe_E[2] = {100, 100};

    //in file
    processed rabbit(run_num);

    //loop over data
    Long64_t nentries = rabbit.fChain->GetEntriesFast();
    Long64_t nbytes = 0, nb = 0;
    cout << nentries << endl;


    rabbit.GetEntry(nentries-1);
    //Double_t total_time = rabbit.seconds;
    //int num_bins = total_time/bin_size;
    double total_time = 65;
    int num_bins = 65/bin_size;
    cout << total_time  << " " << num_bins << endl;

    hNmonT  = new TH1F("hNmonT", "hNmonT", num_bins, 0, total_time);
    hNmonT->GetXaxis()->SetTitle("Time [s]");
    hNmonT->GetYaxis()->SetTitle("Neutron flux [arb]");
    hNmonT->GetXaxis()->SetTitleSize(0.06);
    hNmonT->GetYaxis()->SetTitleSize(0.06);
    hNmonT->SetTitle("");
    for (int i=0; i<2; i++){
        hHPGe[i] = new TH1F(Form("hHPGe%i", i), Form("hHPGe%i", i), num_bins, 0, total_time);
        hHPGe[i]->GetXaxis()->SetTitle("Time [s]");
        hHPGe[i]->GetYaxis()->SetTitle("HPGe Counts/0.5 s");
        hHPGe[i]->GetXaxis()->SetTitleSize(0.06);
        hHPGe[i]->GetYaxis()->SetTitleSize(0.06);
        hHPGe[i]->SetTitle("");
    }
    
    //for (Long64_t jentry=0; jentry<nentries; jentry++) {
    for (Long64_t jentry=0; jentry<1000000; jentry++) {
        nb = rabbit.GetEntry(jentry);   nbytes += nb;
        if (jentry%100000==0){
            cout << '\r' << "Processing event " << jentry;
        }
        if (rabbit.ADC[8]>min_nmon_E){
            hNmonT->Fill(rabbit.seconds);
        }
        if (rabbit.ADC[0]>min_HPGe_E[0]){
            hHPGe[0]->Fill(rabbit.seconds);
        }
        if (rabbit.ADC[1]>min_HPGe_E[1]){ hHPGe[1]->Fill(rabbit.seconds);
        }

    }//end loop over events
    cout << endl;

    //plot
    TCanvas *cScalers = new TCanvas("cScalers", "cScalers", 500, 1000);
    cScalers->Divide(1,3);
    cScalers->cd(1);
    hNmonT->Draw();
    cScalers->cd(2);
    hHPGe[0]->Draw();
    cScalers->cd(3);
    hHPGe[1]->Draw();
    cScalers->Modified();
    cScalers->Update();

    TCanvas *cPulse = new TCanvas("cPulse", "cPulse", 500, 700);

    cPulse->Divide(1,2);
    cPulse->cd(1);
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.05);
    hNmonT->GetXaxis()->SetRangeUser(4, 64);
    hNmonT->SetLineWidth(2);
    hNmonT->Draw();

    cPulse->cd(2);
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.05);
    hHPGe[1]->Add(hHPGe[0]);
    hHPGe[1]->GetXaxis()->SetRangeUser(4, 64);
    hHPGe[1]->SetLineWidth(2);
    hHPGe[1]->Draw();

    //TFile *out = new TFile("file.root", "RECREATE");
    //cScalers->Write();
    //hNmonT->Write();
    //hHPGe[0]->Write();
    //hHPGe[1]->Write();


    theApp->Run();
    gSystem->ProcessEvents();

    //cCycle->SaveAs(Form("time_spec/%i.png", run_num));
    //cCycle->SaveAs(Form("time_spec/%i.C", run_num));

    
}
