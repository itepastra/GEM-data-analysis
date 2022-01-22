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

void fitmuon2()
{
    int n1 = 40;     // amount of histograms in the first file
    int n2 = 5000;   // amount of histograms in the second file
    int n3 = 5000;   // amount of histograms in the third file
    int resx = 6;    // amount of pads in the x-direction
    int resy = resx; // amount of pads in the y-direction
    double dimx = 6; // total detector size in the x-direction
    double dimy = 6; // total detector size in the y-direction
    int bins = 39;   // amount of bins to use for the results


    double padsizex = dimx / resx;            // the size per pad in the x-direction
    double padsizey = dimy / resy;            // the size per pad in the y-direction
    double errx = padsizex / TMath::Sqrt(12); // the uncertainty for a uniform variable in the x-direction of the pad
    double erry = padsizey / TMath::Sqrt(12); // the uncertainty for a uniform variable in the y-direction of the pad

    double alist[n]; // array of angles found by the fit

    TF1 *f1 = new TF1("f1", "[0]*x + [1]");

    std::string baseStart = "Muon_";
    std::unique_ptr<TFile>
        file(TFile::Open("histos_generated_muons.root"));


    for (int i = 0; i < n; i++)
    {
        std::string name = baseStart + std::to_string(i + 1);      //name of the n-th histogram in the file
        std::unique_ptr<TH2S> hist(file->Get<TH2S>(name.c_str())); // getting the pointer to the histogram with the name above

        // reset the parameters of the fit function
        f1->SetParameter(0, 0);
        f1->SetParameter(1, (dimy / 2.0));

        short vals[resx][resy]; // pad value array
        double wvals[resx];     // weighted values per column
        double xs[resx];        // x-coordinates of middle of each column
        double xerr[resx];      // errors in the x-coordinates
        double yerr[resx];      // errors in the y-coordinates

        // initializes the x-coordinates and the errors for the fit
        for (int f = 0; f < resx; f++)
        {
            xs[f] = (f + 0.5) * padsizex;
            xerr[f] = errx;
        }

        // getting the weighted average for every column of a specific histogram
        for (int k = 0; k < resx; k++)
        {
            double tot = 0;  // amount of points in the column multiplied by the y-coordinate of those points
            double totw = 0; // amount of points in the column
            short ct = 0;    // amount of nonzero pads in the column

            for (int l = 0; l < resy; l++)
            {
                // getting the contents of each bin and calculating the weights
                vals[k][l] = hist->GetBinContent(k + 1, l + 1);
                if (vals[k][l] > 0)
                {
                    tot += (l + 0.5) * padsizey * vals[k][l];
                    totw += vals[k][l];
                    ct++;
                }
            }

            if (ct > 0) // if there is data in the column set the value of the point and the errors
            {
                yerr[k] = TMath::Sqrt(ct) * erry;
                wvals[k] = tot / totw;
            }
            else // set the error to infinity and the value to -1 so the point is not used for the fit
            {
                yerr[k] = NAN;
                wvals[k] = -1;
            }
        }


        auto egr = new TGraphErrors(resx, xs, wvals, xerr, yerr); // TGraphErrors of the weighted averages
        TFitResultPtr fitR;                                       // pointer used to get the data from the fit


        // every "showevery" muons we plot the histogram with the fit, otherwise we only fit without graphics
        if (showevery > 1 && (i + 1) % showevery == 0)
        {
            auto c = new TCanvas(name.c_str());

            hist->DrawCopy();
            fitR = egr->Fit("f1", "SQ");
            egr->Draw("same P");
            std::cout << i << "     " << fitR->Parameter(0) << "     " << fitR->Parameter(0) * TMath::RadToDeg() << std::endl;
        }
        else
        {
            fitR = egr->Fit("f1", "SQN0");
        }

        alist[i] = fitR->Parameter(0);
    }

    //create output histogram
    TCanvas *adist = new TCanvas("adist", "angle distribution");
    TH1 *hangle = new TH1I("hangle", "angles", bins, -45, 45);

    // fill output histogram with data
    for (int i = 0; i < n; i++)
    {
        hangle->Fill(TMath::RadToDeg() * TMath::ATan(alist[i]));
    }

    hangle->DrawCopy();
}