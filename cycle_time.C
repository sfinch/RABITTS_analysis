
#include <iostream>
#include <fstream>
#include <vector>

#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>

#include "include/MDPP16_SCP.h"

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::vector;

int main(int argc, char *argv[]){

    if (argc<2)
    {
        cerr << "Invalid number of arguments" << endl;
        cerr << "Usage: " << argv[0] << " <run number>" << endl;
        return 1;
    }
    int run_num = atoi(argv[1]);

    //Variables
    const int rabbit_chn = 12;

    bool opt_verbose = 0;

    //cycle time variables
    bool source_run = 1;
    double min_time = 0.2;  //filter for duplicate in sec
    double max_var = 1.3;   //filter for missed signals
    double min_var = 0.7;   //filter for extra signals
    int pos = 0;            //-1 = counting, 0 not def, 1 = irradiation

    double last_move = 0;
    double last_irr_length = 0;
    double last_count_length = 0;
    double new_move = 0;
    vector<double> irr_start;
    vector<double> count_start;

    double start_offset = 0;
    double end_offset = 0;
    double start_event= 0;
    double end_event= 0;

    int num_irr = 0;
    int num_count = 0;
    int num_missed_irr = 0;
    int num_missed_count = 0;

    //in file
    MDPP16_SCP rabbit(run_num);

    //output tree variables
    double cycle_time = 0;
    TRandom3 r;

    //Histrograms
    TH1F *hCycleTime = new TH1F("hCycleTime", "hCycleTime", 10000, 14, 15);

    //check if Rabbit used during run
    TH1F *hRabbit = (TH1F*)rabbit.file->Get(Form("histos_SCP/hADC%i", rabbit_chn));
    int counts = hRabbit->Integral(10,65000);
    if (counts>1){
        source_run = 0;
    }

    //loop over data
    Long64_t nentries = rabbit.fChain->GetEntriesFast();
    Long64_t nbytes = 0, nb = 0;
    
    //find start offset
    if (!(source_run)){
        for (Long64_t jentry=0; jentry<nentries;jentry++) {
            rabbit.GetEntry(jentry);
            if (rabbit.ADC[rabbit_chn]>10){
                start_offset = rabbit.seconds; 
                start_event = jentry-1; 
                jentry = nentries;
            }
        }
    }
    cout << start_offset << " seconds data before RABITT turned on." << endl;

    //Find ending offset
    end_event = nentries;
    if (!(source_run)){
        for (Long64_t jentry=nentries-1; jentry>0; jentry--) {
            rabbit.GetEntry(jentry);
            if (rabbit.ADC[rabbit_chn]>10){
                end_offset = rabbit.seconds; 
                end_event = jentry+1; 
                jentry = 0;
            }
        }
    }
    rabbit.GetEntry(nentries-1);
    cout << rabbit.seconds-end_offset << " seconds data after RABITT turned off." << endl;

    //Find all movements
    if (!(source_run)){
        for (Long64_t jentry=start_event; jentry<end_event; jentry++) {
            nb = rabbit.GetEntry(jentry);   nbytes += nb;
            if (jentry%100000==0){
                if (!opt_verbose){
                    cout << '\r' << "Processing event " << jentry;
                }
            }

            //calculate cycle time
            if (pos == 0){//Rabbit has yet to be moved
                if ((rabbit.ADC[rabbit_chn]>10)||(rabbit.TDC[rabbit_chn]>10)){

                    pos = 1;
                    irr_start.push_back(rabbit.seconds);
                    //pos = -1;
                    //count_start.push_back(rabbit.seconds);
                    
                    last_move = rabbit.seconds;
                }
            }
            else{//rabbit has moved
                if ((rabbit.ADC[rabbit_chn]>10)||(rabbit.TDC[rabbit_chn]>10)){
                    //rabbit currently moving
                    if ((rabbit.seconds-last_move)>min_time){ //min time filter

                        if (pos == 1){          //irradiation
                            if ((rabbit.seconds-irr_start.back())>(last_irr_length*min_var)){//min var filter
                                pos = -1;
                                last_irr_length = rabbit.seconds - irr_start.back();
                                count_start.push_back(rabbit.seconds);
                                last_move = rabbit.seconds;

                                num_irr++;
                            }
                        }
                        else if (pos == -1){    //counting
                            if ((rabbit.seconds-count_start.back())>(last_count_length*min_var)){//min var filter
                                pos = 1;
                                last_count_length = rabbit.seconds - count_start.back();
                                irr_start.push_back(rabbit.seconds);
                                last_move = rabbit.seconds;

                                num_count++;
                            }
                        }

                        if (opt_verbose){
                            cout << pos << "\t" << rabbit.seconds<< "\t"
                                 << rabbit.seconds-irr_start.back() << "\t" 
                                 << rabbit.seconds-count_start.back() << endl;
                            cout << "\t" << cycle_time << "\t"
                                 << last_irr_length << "\t" 
                                 << last_count_length << endl;
                        }
                    }

                }
                
                //calculate cycle time
                cycle_time = rabbit.seconds - irr_start.back();

                //Check for missed transition
                if ((last_irr_length>min_time) && (last_count_length>min_time)){
                    if (rabbit.seconds<end_offset){
                        //not at the end of the run, after rabbit turned off
                        if ((pos==1) && 
                           ((rabbit.seconds-irr_start.back())>(last_irr_length*max_var))){
                            //missed irradiation pulse
                            if (opt_verbose){
                                cout << "== MISSED COUNTING PULSE ========" << endl;
                                cout << jentry << endl;
                                cout << "== BEFORE =======================" << endl;
                                cout << pos << "\t" << rabbit.seconds<< "\t"
                                     << rabbit.seconds-irr_start.back() << "\t" 
                                     << rabbit.seconds-count_start.back() << endl;
                                cout << "\t" << cycle_time << "\t"
                                     << last_irr_length << "\t" 
                                     << last_count_length << endl;
                            }

                            //rewind
                            do{
                                jentry--;
                                rabbit.GetEntry(jentry);
                            }while(rabbit.seconds>(irr_start.back()+last_irr_length));
                            //insert a movement equal to the last cycle length
                            pos = -1;
                            new_move = (count_start.back() + last_count_length + last_irr_length);
                            count_start.push_back(new_move);
                            last_move = count_start.back();
                            num_missed_irr++;

                            if (opt_verbose){
                                cout << "== AFTER ========================" << endl;
                                cout << pos << "\t" << rabbit.seconds<< "\t"
                                     << rabbit.seconds-irr_start.back() << "\t" 
                                     << rabbit.seconds-count_start.back() << endl;
                                cout << "\t" << cycle_time << "\t"
                                     << last_irr_length << "\t" 
                                     << last_count_length << endl;
                                cout << "---------------------------------" << endl;
                            }
                        }
                        else if ((pos==-1) && 
                                ((rabbit.seconds-count_start.back())>(last_count_length*max_var))){
                            //missed counting pulse
                            if (opt_verbose){
                                cout << "== MISSED COUNTING PULSE ========" << endl;
                                cout << jentry << endl;
                                cout << "== BEFORE =======================" << endl;
                                cout << pos << "\t" << rabbit.seconds<< "\t"
                                     << rabbit.seconds-irr_start.back() << "\t" 
                                     << rabbit.seconds-count_start.back() << endl;
                                cout << "\t" << cycle_time << "\t"
                                     << last_irr_length << "\t" 
                                     << last_count_length << endl;
                            }
                                 
                            //rewind
                            do{
                                jentry--;
                                rabbit.GetEntry(jentry);
                            }while(rabbit.seconds>(count_start.back()+last_count_length));
                            //insert a movement equal to the last cycle length
                            pos = 1;
                            new_move = (irr_start.back() + last_irr_length + last_count_length);
                            irr_start.push_back(new_move);
                            last_move = irr_start.back();
                            num_missed_count++;

                            if (opt_verbose){
                                cout << "== AFTER ========================" << endl;
                                cout << pos << "\t" << rabbit.seconds<< "\t"
                                     << rabbit.seconds-irr_start.back() << "\t" 
                                     << rabbit.seconds-count_start.back() << endl;
                                cout << "\t" << cycle_time << "\t"
                                     << last_irr_length << "\t" 
                                     << last_count_length << endl;
                                cout << "=================================" << endl;
                            }
                        }
                    }
                }

            }//end else

        }//end loop over events
    }//end !source run
    cout << endl;

    cout << endl;
    cout << nentries << " events processed" << endl;
    cout << "Counting cycles:           " << num_count << endl;
    cout << "Irradiation cycles:        " << num_irr << endl;
    cout << "Counting cycles missed:    " << num_missed_count << endl;
    cout << "Irradiation cycles missed: " << num_missed_irr  << endl;

    for (int i=1; i<irr_start.size(); i++){
        double diff = irr_start.at(i) - irr_start.at(i-1);
        //cout << irr_start.at(i) << endl;
        //cout << diff << endl;
        hCycleTime->Fill(diff);
    }

    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 500);
    hCycleTime->SetTitle("");
    hCycleTime->GetYaxis()->SetTitle("Number of cycles");
    hCycleTime->GetXaxis()->SetTitle("Cycle length [s]");
    hCycleTime->GetXaxis()->SetRangeUser(14.235, 14.238);
    hCycleTime->GetXaxis()->SetTitleSize(0.06);
    hCycleTime->GetYaxis()->SetTitleSize(0.06);
    hCycleTime->GetXaxis()->SetLabelSize(0.05);
    hCycleTime->GetYaxis()->SetLabelSize(0.05);
    hCycleTime->SetLineWidth(2);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.15);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.05);
    hCycleTime->Draw();
    c1->SaveAs("c1.C");
    c1->SaveAs("figures/cycle_time.png");


    
}

