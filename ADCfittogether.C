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
//+[3]*([4]-x)*exp([5]*([4]-x))

void ADCfittogether()
{
    // initialize arrays for results
    double MPVlist[32];                       // fit most probable values
    double MPVerrorlist[32];                  // fit most probable values error
    double chi2list[32];                      // fit chi2
    double ndflist[32];                       // fit degrees of freedom
    std::string fileName = "histos_and.root"; // filename of the rootfile

    // create the fit function
    TF1 *landau = new TF1("landau", "landau(0)", 110, 250); // landau fit function between 110 en 250

    // formatting
    landau->SetLineWidth(1);
    landau->SetLineColor(1);

    //set start parameters
    landau->SetParameters(100, 150, 1);
    landau->SetParLimits(1, 0.0, 500.0);

    // opening the file and initializing histograms
    std::string baseStart = "ADV_values_Channel_"; // name of the histogram before the number
    std::string names[32];                         // array of the names
    std::unique_ptr<TFile>
        file(TFile::Open(fileName.c_str()));
    file->ls();

    // create the canvas for the plots
    TCanvas *channelPlots = new TCanvas(
        "channelPlots", "Channel Plots");

    channelPlots->SetCanvasSize(4000, 2000);    // make the canvas large
    channelPlots->SetWindowSize(1000, 1000);    // make the window smaller so scrollbars appear
    channelPlots->Divide(8, 4, 0.001f, 0.001f); // divide the canvas into an 8 by 4 grid to fit the 32 channels

    for (int i = 0; i < 32; i++)
    {
        channelPlots->cd(i + 1);

        names[i] = baseStart + std::to_string(i);
        std::unique_ptr<TH1F> hist(file->Get<TH1F>(names[i].c_str()));
        landau->SetParameters(100, 150, 1);

        //rebin and fit
        hist->Rebin(4);
        TFitResultPtr fitR = hist->Fit("landau", "RSE");

        //formatting
        hist->SetLineColor(i + 50);
        hist->GetXaxis()->SetRangeUser(0, 500);
        hist->GetXaxis()->SetTitle("U [mV]");
        hist->GetYaxis()->SetTitle("counts");
        hist->GetYaxis()->SetTitleOffset(1.25);
        hist->SetMarkerStyle(33);
        hist->SetMarkerColor(i + 50);
        hist->SetTitle(("Channel " + std::to_string(i + 1)).c_str());
        hist->SetStats(false);

        hist->DrawCopy("E X0");

        //get fit parameters and errors
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

    // formatting
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

    // formatting
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