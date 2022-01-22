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

void ADCfit1()
{
    // initialize arrays for fit results
    double MPVlist[32];
    double MPVerrorlist[32];
    double constantlist[32];
    double constanterrorlist[32];
    double chi2list[32];
    double ndflist[32];

    // create the fit function and set some starting parameters
    TF1 *g1 = new TF1("landaufunc", "landau", 110,250);
    TF1 *g2 = new TF1("expfunc", "[a] * ([c]-x) * exp([b]*([c]-x))", 110,500);
    // TF1 *g2 = new TF1("expfunc", "expo", 180,500);

    TF1 *gaussexp = new TF1("gaussexp", "landaufunc(0) + expfunc(3)", 110, 500);
    gaussexp->SetLineWidth(1);
    gaussexp->SetLineColor(1);

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
        // gaussexp->SetParameters(100, 150, 1, -1, 0);
        

        auto c = new TCanvas();
        //rebin and fit
        hist->Rebin(4);
        TFitResultPtr fitR = hist->Fit(g1, "RSE");
        TFitResultPtr fitR2 = hist->Fit(g2, "RSE+");

        g1->GetParameters(&gepar[0]);
        g2->GetParameters(&gepar[3]);

        gaussexp->SetParameters(gepar);
        hist->Fit(gaussexp, "R+");

        //formatting
        hist->SetLineColor(i + 50);
        // hist->GetXaxis()->SetRangeUser(0, 500);
        hist->GetXaxis()->SetTitle("U [mV]");
        hist->GetYaxis()->SetTitle("counts");
        hist->GetYaxis()->SetTitleOffset(1.25);
        hist->SetMarkerStyle(33);
        hist->SetMarkerColor(i + 50);
        hist->SetTitle(("Channel " + std::to_string(i + 1)).c_str());

        hist->DrawCopy("E X0");
        // gaussexp->Draw("SAME");

        //get fit parameters and errors
        constantlist[i] = fitR->Parameter(0);
        constanterrorlist[i] = fitR->ParError(0);
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

    // //scaling modifier plot
    // TCanvas *cconst = new TCanvas("cconst", "multiplication constants");
    // TGraphErrors *congr = new TGraphErrors(n, channels, constantlist, channelerrors, constanterrorlist);
    // congr->SetMarkerStyle(5);
    // congr->SetMarkerColor(9);
    // congr->Draw("AP");

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