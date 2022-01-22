

void muongen()
{
    int n = 5000;
    double espread = 0.025;
    double avgspread = 0.75;
    int resx = 6;
    int resy = resx;
    double dimx = 6;
    double dimy = 6;

    
    double minangle = -20;
    double maxangle = 20;




    TFile f("histos_generated_muons.root", "recreate");
    double stepsize = (maxangle - minangle) / (n - 1);

    std::cout << "min " << minangle << " max " << maxangle << " step " << stepsize << std::endl;
    TRandom *rand = new TRandom(48968834);

    for (int i = 0; i <= n; i++)
    {
        string name = "Muon_" + std::to_string(i);
        TH2S *hist = new TH2S(name.c_str(), name.c_str(), resx, 0, dimx, resy, 0, dimy);

        double m = TMath::Tan((minangle + i * stepsize) * TMath::DegToRad());
        double o = rand->Gaus(3, avgspread);

        for (int j = 0; j < 20000; j++)
        {
            double r = rand->Uniform(6) - 3;
            double y = r * m + o + rand->Gaus(0, espread);
            double x = r + 3 + rand->Gaus(0, espread);
            hist->Fill(x, y);
        }
        hist->SetMarkerColor(8);
        hist->SetMarkerStyle(6);
        hist->Write();
    }
}
