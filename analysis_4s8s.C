
#include <iostream>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>

#include "include/processed.h"

using std::cout;
using std::cerr;
using std::endl;

void analysis_4s8s(int run_num){

    //Variables
    const int num_det = 2;

    //const int num_win = 4;
    const int num_win = 8;

    TH1F *hCycle = new TH1F("hCycle", "hCycle", 3000, -10, 20);
    TH1F *hIrr   = new TH1F("hIrr", "hIrr", 30000, 0, 3000);
    TH1F *hCount = new TH1F("hCount", "hCount", 30000, 0, 3000);
    TH1F *hEnCount[num_det];
    TH1F *hEnWin[num_win][num_det+1]; //[window 1-3][det 1, 2, summed]

    for (int i=0; i<num_win; i++){
        for (int j=0; j<3; j++){
            hEnWin[i][j] = new TH1F(Form("hEnWin%i%i", i, j), Form("hEnWin%i%i", i, j), 30000, 0, 3000);
        }
    }

    for (int i=0; i<num_det; i++){
        hEnCount[i] = new TH1F(Form("hEnCount%i", i), Form("hEnCount%i", i), 30000, 0, 3000);
    }

    //cuts
    double time_irr[2] = {0.0, 4.0};
    double time_count[2] = {5., 14};

    double time_win[num_win][2];
    for (int i=0; i<num_win; i++){
        time_win[i][0] = i+5.15;
        time_win[i][1] = i+1+5.15;
        //time_win[i][0] = 2*i+5.15;
        //time_win[i][1] = (2*(i+1))+5.15;
    }

    //in file
    processed rabbit(run_num);

    //out file
    TFile *fHist = new TFile(Form("data_hist/RABBIT_%i.root", run_num), "RECREATE");

    //loop over data
    Long64_t nentries = rabbit.fChain->GetEntriesFast();
    Long64_t nbytes = 0, nb = 0;
    
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        nb = rabbit.GetEntry(jentry);   nbytes += nb;
        if (jentry%100000==0){
            cout << '\r' << "Processing event " << jentry;
        }
        if ((rabbit.cycle_time > 0.00001)||(rabbit.cycle_time<-0.00001)){
            hCycle->Fill(rabbit.cycle_time);
        }

        if ((rabbit.cycle_time>time_irr[0]) && (rabbit.cycle_time<time_irr[1])){
            for (int det=0; det<num_det; det++){
                if (rabbit.En[det]>1){
                    hIrr->Fill(rabbit.En[det]);
                }
            }
        }
        else if ((rabbit.cycle_time>time_count[0]) && (rabbit.cycle_time<time_count[1])){
            for (int det=0; det<num_det; det++){
                if (rabbit.En[det]>1){
                    hCount->Fill(rabbit.En[det]);
                    hEnCount[det]->Fill(rabbit.En[det]);
                }
            }
            for (int window=0; window<num_win; window++){
                if ((rabbit.cycle_time>time_win[window][0]) && (rabbit.cycle_time<time_win[window][1])){
                    //clover 1
                    if (rabbit.En[0]>1){
                        hEnWin[window][0]->Fill(rabbit.En[0]);
                        hEnWin[window][2]->Fill(rabbit.En[0]);
                    }
                    //clover 2
                    if (rabbit.En[1]>1){
                        hEnWin[window][1]->Fill(rabbit.En[1]);
                        hEnWin[window][2]->Fill(rabbit.En[1]);
                    }
                }
            }//end time windows

        }//end counting time
    }//end loop over events

    //write histos to file
    fHist->cd();

    hCycle->Write();
    hIrr->Write();
    hCount->Write();
    for (int det=0; det<num_det; det++){
        hEnCount[det]->Write();
    }
    for (int i=0; i<num_win; i++){
        for (int j=0; j<num_det+1; j++){
            hEnWin[i][j]->Write();
        }
    }

    fHist->Write();
    fHist->Close();
    
}