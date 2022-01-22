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

void grootfill()
{
    auto formula1 = new TFormula("formula1", "abs(cos(x)*x-sin(x))");
    auto groot = new TF1("groot", "x*gaus(0) + [3]*formula1", 0, 10);
    groot->SetLineColor(6);
    groot->SetLineWidth(4);
    groot->SetParameters(10, 4, 1, 20);

    auto groothist = new TH1D("groothist", "Testing with numbers", 200, 0, 10);
    groothist->FillRandom("groot", 40000);

    TFile myfile("grootfill.root", "RECREATE");
    formula1->Write();
    groot->Write();
    groothist->Write();
}