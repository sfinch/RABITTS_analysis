//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Feb  6 13:16:29 2019 by ROOT version 5.34/38
// from TTree processed/Processed rabbit data
// found on file: data_processed/RABBIT_DEC18_12.root
//////////////////////////////////////////////////////////

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class cycle_process {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   TFile          *file;

   // Declaration of leaf types
   Float_t         En[4];
   Float_t         cycle_time;

   // List of branches
   TBranch        *b_En;   //!
   TBranch        *b_cycle_time;   //!


   cycle_process(int run_num);
   virtual ~cycle_process();
   virtual Int_t    GetEntry(Long64_t entry);
   virtual void     Init();
   virtual void     Show(Long64_t entry = -1);
};

cycle_process::cycle_process(int run_num)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   file = new TFile(Form("data_processed/RABITTS10_%i.root", run_num), "RECREATE");
   file->GetObject("processed", fChain);

   Init();
}

cycle_process::~cycle_process()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t cycle_process::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}

void cycle_process::Init()
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.

   // Set branch addresses and branch pointers
   if (!fChain) return;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("En[4]", En, &b_En);
   fChain->SetBranchAddress("cycle_time", &cycle_time, &b_cycle_time);


}

void cycle_process::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
