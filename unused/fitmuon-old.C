#include "TCanvas.h"
#include "TFrame.h"
#include "TBenchmark.h"
#include "TString.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TROOT.h"
#include "TError.h"
#include "TInterpreter.h"
#include "TSystem.h"
#include "TPaveText.h"
#include "TGraphErrors.h"

void fitmuon()
{
    int n = 40;

    const double err = 1 / TMath::Sqrt(12);
    std::cout << err << std::endl;

    TF1 *f1 = new TF1("f1", "[0]*x + [1]");

    std::string baseStart = "Pad_Plane_Event_";
    std::unique_ptr<TFile>
        file(TFile::Open("histos_final2.root"));
    file->ls();

    double alist[n];
    double blist[n];

    for (int i = 0; i < n; i++)
    {
        std::cout << "uhhm" << std::endl;
        string name = baseStart + std::to_string(i + 1);
        std::unique_ptr<TH2S> hist(file->Get<TH2S>(name.c_str()));

        short vals[6][6];
        double wvals[6];
        double xs[6];
        double xerr[6];
        double yerr[6];

        for (int f = 0; f < 6; f++)
        {
            xs[f] = f + 0.5;
            xerr[f] = err;
            yerr[f] = err; // change to weighted??
        }

        for (int k = 0; k < 6; k++)
        {
            f1->SetParameter(0, 0);
            f1->SetParameter(1, 1);

            long tot = 0;
            int totw = 0;
            short ct = 0;

            for (int l = 0; l < 6; l++)
            {
                vals[k][l] = hist->GetBinContent(k + 1, l + 1);
                tot += (l + 0.5) * vals[k][l];
                totw += vals[k][l];
                if (vals[k][l] > 0)
                {
                    ct++;
                    std::cout << i + 1 << "    " << k << " " << l << "  " << vals[k][l] << std::endl;
                }
            }
            if (ct > 0)
            {
                yerr[k] = ct*err;
                wvals[k] = (double)tot / (double)totw;
            }
            else
            {
                yerr[k] = NAN;
                wvals[k] = -1;
            }
            std::cout << "totval  " << i + 1 << "   " << k << "   " << wvals[k] << std::endl;
        }

        auto egr = new TGraphErrors(6, xs, wvals, xerr, yerr);

        auto c = new TCanvas(name.c_str());

        hist->DrawCopy();
        TFitResultPtr fitR = egr->Fit("f1", "S");
        egr->Draw("same P");

        alist[i] = fitR->Parameter(0);
        blist[i] = fitR->Parameter(1);
    }

    TCanvas *adist = new TCanvas("adist", "angle distribution");
    TH1 *hangle = new TH1I("hangle", "angles", 40, -45,45);

    for (int i = 0; i < n; i++)
    {
        std::cout << i+1 << " is at " << TMath::RadToDeg()*TMath::ATan(alist[i]) << " degrees" << std::endl;
        hangle->Fill(TMath::RadToDeg()*TMath::ATan(alist[i]));
    }

    hangle->DrawCopy();
}