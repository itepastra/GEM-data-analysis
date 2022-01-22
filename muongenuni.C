
/* 
    generates an amount of muons with directions 
    following a uniform distribution between 2 angles
    and creates the same type of histogram as the measured muons
    so the real muons can be compared to a idealized situation
*/
void muongenuni()
{
    int n = 5000;                                       // amount of muons to simulate
    double espread = 0.025;                             // gaussian spread size around the 'real' line
    double avgspread = 0.75;                            // gaussian spread for the middle point around the vertical middle
    int resx = 6;                                       // amount of pads in the x direction
    int resy = resx;                                    // amount of pads in the y direction
    double dimx = 6;                                    // total detector size in cm in the x direction
    double dimy = 6;                                    // total detector size in cm in the y direction
    double aspread = 9.43;                              // gaussian spread for the angle
    double minangle = -20;                              // minimum muon angle (degrees)
    double maxangle = 20;                               // maximum muon angle (degrees)
    std::string fileName = "histos_uniform_muons.root"; // filename to use for the generated muon histograms

    TFile f(fileName.c_str(), "recreate");

    //initializing the random number generator with a seed
    TRandom *rand = new TRandom(6606601);

    for (int i = 0; i <= n; i++)
    {
        // create histogram with correct name
        string name = "Muon_" + std::to_string(i);
        TH2S *hist = new TH2S(name.c_str(), name.c_str(), resx, 0, dimx, resy, 0, dimy);

        double h = rand->Uniform(maxangle - minangle) + minangle; //the random direction of the muon in degrees
        double m = TMath::Tan(h * TMath::DegToRad());             // slope of the muon
        double o = rand->Gaus(3, avgspread);                      // height of the middle point of the line

        // to get data for the histogram we generate points randomly around the line
        for (int j = 0; j < 20000; j++)
        {
            double r = rand->Uniform(dimx) - (dimx / 2);
            double y = r * m + o + rand->Gaus(0, espread);
            double x = r + (dimx / 2) + rand->Gaus(0, espread);
            hist->Fill(x, y);
        }

        //formatting the histogram
        hist->SetMarkerColor(8);
        hist->SetMarkerStyle(6);

        //saving the histogram
        hist->Write();
    }
}
