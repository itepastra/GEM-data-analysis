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
#include "TGraphErrors.h"

/*
this function fits a landau function to each of the histograms of the 32 channels received from the ADC,
then it plots the location of the peak of the landau, and the reduced chi^2 of the fit
if all pads are working correctly the peaks should all be similar to eachother.
*/
void ADCfitseperate()
{
    // initialize arrays for fit results
    double MPVlist[32];      // fit results for most probable values
    double MPVerrorlist[32]; // fit errors for most probable values
    double chi2list[32];     // fit chi^2 results
    double ndflist[32];      // fit number of independent variables

    // create the fit function and set some starting parameters
    TF1 *g1 = new TF1("landaufunc", "landau", 110, 250);

    // opening the file and initializing histograms
    std::string baseStart = "ADV_values_Channel_";
    std::string names[32];
    std::unique_ptr<TH1F> histos[32];
    std::unique_ptr<TFile>
        file(TFile::Open("histos_and.root"));
    file->ls();

    for (int i = 0; i < 32; i++)
    {
        double_t gepar[5];
        names[i] = baseStart + std::to_string(i);
        std::unique_ptr<TH1F> hist(file->Get<TH1F>(names[i].c_str()));

        auto c = new TCanvas();
        // rebin and fit
        hist->Rebin(4);
        TFitResultPtr fitR = hist->Fit(g1, "RSE");

        // formatting
        hist->SetLineColor(i + 50);
        hist->GetXaxis()->SetRangeUser(0, 500);
        hist->GetXaxis()->SetTitle("U [mV]");
        hist->GetYaxis()->SetTitle("counts");
        hist->GetYaxis()->SetTitleOffset(1.25);
        hist->SetMarkerStyle(33);
        hist->SetMarkerColor(i + 50);
        hist->SetTitle(("Channel " + std::to_string(i + 1)).c_str());

        hist->DrawCopy("E X0");
        TImageDump *imgDump = new TImageDump(("../channel_" + std::to_string(i) + ".png").c_str());
        c->Paint();
        imgDump->Close();

        // get fit parameters and errors
        MPVlist[i] = fitR->Parameter(1);
        MPVerrorlist[i] = fitR->ParError(1);
        chi2list[i] = fitR->Chi2();
        ndflist[i] = fitR->Ndf();
    }

    // create an array of [0 ... 31] and and array of 32 0's
    int n = 32;
    double channels[n];
    double channelerrors[n];
    for (int i = 0; i < n; i++)
    {
        channels[i] = i + 1;
        channelerrors[i] = 0;
    }

    // most probable values plot
    TCanvas *cmpv = new TCanvas("cmpv", "most probable values");
    TGraphErrors *mpvgr = new TGraphErrors(n, channels, MPVlist, channelerrors, MPVerrorlist);
    mpvgr->SetMarkerStyle(5);
    mpvgr->SetMarkerColor(9);
    mpvgr->SetTitle("most probable values");
    mpvgr->GetYaxis()->SetRangeUser(0, 300);
    mpvgr->GetXaxis()->SetTitle("channel");
    mpvgr->GetYaxis()->SetTitle("most probable value [mV]");
    mpvgr->Draw("AP");

    // calculate reduced chi^2
    double chi2redlist[n];
    for (int i = 0; i < n; i++)
    {
        chi2redlist[i] = chi2list[i] / ndflist[i];
    }

    // chi2 plot
    TCanvas *cchi = new TCanvas("cchi", "chi2");
    TGraphErrors *chigr = new TGraphErrors(n, channels, chi2redlist, channelerrors, channelerrors);
    chigr->SetMarkerStyle(33);
    chigr->SetMarkerColor(9);
    chigr->SetTitle("Reduced Chi^2 of fit per channel");
    chigr->GetXaxis()->SetTitle("Channel");
    chigr->GetYaxis()->SetTitle("reduced Chi^2");
    chigr->GetYaxis()->SetRangeUser(0, 4);
    chigr->GetXaxis()->SetRangeUser(0, 33);
    chigr->Draw("AP");
    cchi->SetGrid();
}