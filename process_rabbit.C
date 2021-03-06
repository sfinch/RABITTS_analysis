
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

struct calibration{     //struct storing calibration data for both dets
    Float_t m[2];
    Float_t b[2];
};
    
calibration read_in_cal(int run_num); // function to read in calibration from file

int main(int argc, char *argv[]){

    if (argc<2)
    {
        cerr << "Invalid number of arguments" << endl;
        cerr << "Usage: " << argv[0] << " <run number>" << endl;
        return 1;
    }
    int run_num = atoi(argv[1]);

    //Variables
    const int num_det = 4;
    int det_chn[num_det] = {0, 1, 2, 3};
    const int rabbit_chn = 12;

    bool opt_verbose = 0;

    //cycle time variables
    bool source_run = 1;
    double min_time = 0.2;  //filter for duplicate in sec
    double max_var = 1.1;   //filter for missed signals
    double min_var = 0.9;   //filter for extra signals
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
    //check for any additional calibration corrections
    calibration cal = read_in_cal(run_num);

    //output tree variables
    Float_t cycle_time = 0;
    Float_t En[num_det];
    TRandom3 r;

    //out file
    TFile *fOut = new TFile(Form("data_processed/RABITTS10_%i.root", run_num), "RECREATE");
    TTree *tProcessed = new TTree("processed", "Processed rabbit data");

    tProcessed->Branch(Form("En[%i]", num_det), &En, Form("En[%i]/F", num_det));
    tProcessed->Branch("cycle_time", &cycle_time, "cycle_time/F");

    //Histrograms
    TH1F *hIrrTime = new TH1F("hIrrTime", "hIrrTime", 100*100, 0, 100);
    TH1F *hCountTime = new TH1F("hCountTime", "hCountTime", 100*100, 0, 100);
    TH1F *hCycleTime = new TH1F("hCycleTime", "hCycleTime", 10*110, -10, 100);

    TH1F *hEn[num_det];

    for (int det=0; det<num_det; det++){
        hEn[det] = new TH1F(Form("hEn%i", det), Form("hEn%i", det), 10*3000, 0, 3000);
    }

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

                                hIrrTime->Fill(last_irr_length);
                                num_irr++;
                            }
                        }
                        else if (pos == -1){    //counting
                            if ((rabbit.seconds-count_start.back())>(last_count_length*min_var)){//min var filter
                                pos = 1;
                                last_count_length = rabbit.seconds - count_start.back();
                                irr_start.push_back(rabbit.seconds);
                                last_move = rabbit.seconds;

                                hCountTime->Fill(last_count_length);
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

    //loop over all events and write output tree
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        nb = rabbit.GetEntry(jentry);   nbytes += nb;
        if (jentry%100000==0){
            if (!opt_verbose){
                cout << '\r' << "Processing event " << jentry;
            }
        }

        cycle_time = 0;

        if (!source_run){
            if (jentry<start_event){
                //cycle time should be negative
                cycle_time = rabbit.seconds - start_offset;
            }
            else{
                //for (int i=0; i<irr_start.size(); i++){
                for (auto & cycle_start : irr_start){
                    if (rabbit.seconds > cycle_start){
                        cycle_time = rabbit.seconds - cycle_start;
                    }
                }
            }
        }

        //energy calibrate detectors
        for (int det=0; det<num_det; det++){ //loop over detectors
            En[det] = 0;
            if ((rabbit.ADC[det_chn[det]]>10)&&(!(rabbit.overflow[det_chn[det]]))){
                //Calibrate
                En[det] = (rabbit.ADC[det_chn[det]]+r.Rndm()-0.5)*( (*rabbit.m) )[det_chn[det]] + ( (*rabbit.b) )[det_chn[det]];
                En[det_chn[det]] = En[det_chn[det]]*cal.m[det_chn[det]] + cal.b[det_chn[det]];
                //fill histos
                hEn[det]->Fill(En[det]);
                hCycleTime->Fill(cycle_time);
            }
        }

        //fill tree
        tProcessed->Fill();
    }
    cout << endl;

    cout << nentries << " events processed" << endl;
    cout << "Counting cycles:           " << num_count << endl;
    cout << "Irradiation cycles:        " << num_irr << endl;
    cout << "Counting cycles missed:    " << num_missed_count << endl;
    cout << "Irradiation cycles missed: " << num_missed_irr  << endl;
    
    //write data to file
    fOut->cd();
    tProcessed->Write();

    fOut->mkdir("histos");
    fOut->cd("histos");
    hIrrTime->Write();
    hCountTime->Write();
    hCycleTime->Write();
    for (int det=0; det<num_det; det++){
        hEn[det]->Write();
    }

    fOut->Close();
    
}

// Reads in the calibration file and finds the calibration for the given run
calibration read_in_cal(int run_num){

    //variables to read in from file
    Float_t m_file[2], b_file[2];
    int run, run2;
    
    //calibration data
    ifstream infile;
    infile.open("datafiles/det_cal.dat");

    //return values
    calibration cal;
    for (int i = 0; i<2; i++){
        cal.m[i] = 1.;
        cal.b[i] = 0;
    }

    //read in data from file
    do{
        infile>>run;
        if (run<0 || run>10000){
            break;
        }
        for (int j=0; j<2; j++){
            infile>>m_file[j];
        }
        infile>>run2;
        if ((run != run2) && (run2 != 0)){
            cout << "ERROR IN DATA FILE! Entry:  " << run << endl;
        }
        for (int j=0; j<2; j++){
            infile>>b_file[j];
        }
        if (run == run_num){
            break;
        }
    }while(!infile.eof());
    infile.close();
    
    //There exists calibration points for the given run
    if (run == run_num){
        //print calibration values
        cout << "Found run " << run << ", using calibration:" << endl;
        cout << "m:  ";
        for (int i = 0; i<2; i++){
            cout << m_file[i] << "  ";
        }
        cout << endl << "b:  ";
        for (int i = 0; i<2; i++){
            cout << b_file[i] << "  ";
        }
        cout << endl;
        //check with user
        //cout << "Is this OK? (y or n)  ";
        char ans = 'y';
        //cin >> ans;
        if  (ans == 'y'){
            for (int i = 0; i<2; i++){
                cal.m[i] = m_file[i];
                cal.b[i] = b_file[i];
            }
        }
    }
    else{
        cout << "No additional calibration found in datafiles/det_cal.dat. Using only MVME calibration." 
             << endl;
    }
    return cal; //return calibration
}

