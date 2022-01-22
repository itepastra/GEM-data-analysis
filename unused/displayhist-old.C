#include "TCanvas.h"
#include "TFrame.h"
#include "TBenchmark.h"
#include "TString.h"
#include "TF1.h"
#include "TH1.h"
#include "TFile.h"
#include "TROOT.h"
#include "TError.h"
#include "TInterpreter.h"
#include "TSystem.h"
#include "TPaveText.h"

void displayhist()
{
    TFile *file = TFile::Open("grootfill.root");
    file->ls();

    TF1 *groot = 0;
    TH1 *hist = 0;

    TCanvas *c1 = new TCanvas("c1_fit1", "The Fit Canvas", 200, 10, 700, 500);
    c1->SetGridx();
    c1->SetGridy();
    c1->GetFrame()->SetFillColor(21);
    c1->GetFrame()->SetBorderMode(-1);
    c1->GetFrame()->SetBorderSize(5);

    file->GetObject("groot", groot);
    file->GetObject("groothist", hist);

    groot->Print();

    hist->SetFillColor(9);
    hist->Fit("groot");

    c1->Update();
}