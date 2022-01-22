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

/* 
finds the best straight line fit for a muon in a 2d histogram
by calculating the weighted average for every column and then fitting to the result using root fit
modifies the input histogram to contain the calculated angles for the particles
*/
void FitLines(int n, std::string fileName, std::string baseStart, int resx, int resy, double dimx, double dimy, int bins, TH1 *hangle)
{
    std::unique_ptr<TFile>
        file(TFile::Open(fileName.c_str()));

    TF1 *f1 = new TF1("f1", "[0]*x + [1]"); // function for a line, used for fitting

    double padsizex = dimx / resx;            // the size per pad in the x-direction
    double padsizey = dimy / resy;            // the size per pad in the y-direction
    double errx = padsizex / TMath::Sqrt(12); // the uncertainty for a uniform variable in the x-direction of the pad
    double erry = padsizey / TMath::Sqrt(12); // the uncertainty for a uniform variable in the y-direction of the pad

    double alist[n]; // array of angles found by the fit

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
        fitR = egr->Fit("f1", "SQN0");                            // fit the line on the graph and get the fit results

        alist[i] = fitR->Parameter(0); // add the fit angle to the angle array
    }

    // filling the histogram with the angles from the fit, converting from radians to degrees
    for (int i = 0; i < n; i++)
    {
        hangle->Fill(TMath::RadToDeg() * TMath::ATan(alist[i]));
    }
}

/* 
plots the fitted angular distributions 
of multiple root files to be able to compare them
and see which one fits best
*/
void distributioncompare()
{
    int n1 = 40;     // amount of histograms in the first file
    int n2 = 5000;   // amount of histograms in the second file
    int n3 = 5000;   // amount of histograms in the third file
    int resx = 6;    // amount of pads in the x-direction
    int resy = resx; // amount of pads in the y-direction
    double dimx = 6; // total detector size in the x-direction
    double dimy = 6; // total detector size in the y-direction
    int bins = 39;   // amount of bins to use for the results

    std::string fileName1 = "histos_final2.root";          // file name to use for histogram 1
    std::string fileName2 = "histos_generated_muons.root"; // file name to use for histogram 2
    std::string fileName3 = "histos_uniform_muons.root";   // file name to use for histogram 3

    std::string baseStart1 = "Pad_Plane_Event_"; // the name before the number of the histograms in file 1
    std::string baseStart2 = "Muon_";            // the name before the number of the histograms in file 2
    std::string baseStart3 = "Muon_";            // the name before the number of the histograms in file 3

    TH1 *hist1 = new TH1D("hist1", "angles", bins, -30, 30); // histogram 1
    TH1 *hist2 = new TH1D("hist2", "angles", bins, -30, 30); // histogram 2
    TH1 *hist3 = new TH1D("hist3", "angles", bins, -30, 30); // histogram 3

    // filling the histograms with data
    FitLines(n1, fileName1, baseStart1, resx, resy, dimx, dimy, bins, epsilon, base, hist1);
    FitLines(n2, fileName2, baseStart2, resx, resy, dimx, dimy, bins, epsilon, base, hist2);
    FitLines(n3, fileName3, baseStart3, resx, resy, dimx, dimy, bins, epsilon, base, hist3);

    // normalizing the histograms so n1 and n2 and n3 can be different
    hist1->Scale(1.0 / n1);
    hist2->Scale(1.0 / n2);
    hist3->Scale(1.0 / n3);

    // coloring the lines for readability
    hist1->SetLineColor(4);
    hist2->SetLineColor(2);
    hist3->SetLineColor(7);

    // show all 3 histograms on top of eachother for visual comparison
    TCanvas *adist = new TCanvas("adist", "angle distribution");

    hist1->Draw("hist c");
    hist2->DrawCopy("hist same c");
    hist3->DrawCopy("hist same c");

    // show the 1st histogram divided by the 2nd and 3rd to see the ratio difference
    TCanvas *divadist = new TCanvas("divadist", "divided angle distribution");

    hist2->Divide(hist1, hist2, 1, 1, "B");
    hist3->Divide(hist1, hist3, 1, 1, "B");
    hist2->Draw("hist c");
    hist3->Draw("Same hist c");
}
