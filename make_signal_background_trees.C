///////////////////////////////////////////////////////////////
///////////// GENERATE SIGNAL AND BACKGROUND TREES ////////////
///////////////////////////////////////////////////////////////
// Run like this:
// root make_signal_background_trees.C
void make_signal_background_trees() {
    // Open ROOT files
    TFile* signalFile = TFile::Open("invariant_mass.root");
    TFile* backgroundFile = TFile::Open("background_m_ee.root");

    // Retrieve histograms from the ROOT files
    TH1D* signalHist = (TH1D*)signalFile->Get("hist_m_ee");
    TH1D* backgroundHist = (TH1D*)backgroundFile->Get("hist_m_ee_back");

    // Create output file for the new trees
    TFile* outputFile = TFile::Open("signal_background_trees.root", "RECREATE");

    // Create trees
    TTree* signalTree = new TTree("SignalTree", "Signal events");
    TTree* backgroundTree = new TTree("BackgroundTree", "Background events");

    // Add a branch for invariant mass
    Float_t invariant_mass;
    signalTree->Branch("invariant_mass", &invariant_mass, "invariant_mass/F");
    backgroundTree->Branch("invariant_mass", &invariant_mass, "invariant_mass/F");

    // Fill signal tree
    for (int bin = 1; bin <= signalHist->GetNbinsX(); bin++) {
        invariant_mass = signalHist->GetBinCenter(bin);
        int nEvents = signalHist->GetBinContent(bin);
        for (int i = 0; i < nEvents; i++) {
            signalTree->Fill();
        }
    }

    // Fill background tree
    for (int bin = 1; bin <= backgroundHist->GetNbinsX(); bin++) {
        invariant_mass = backgroundHist->GetBinCenter(bin);
        int nEvents = backgroundHist->GetBinContent(bin);
        for (int i = 0; i < nEvents; i++) {
            backgroundTree->Fill();
        }
    }

    // Write trees to the output file
    signalTree->Write();
    backgroundTree->Write();
    outputFile->Close();

    std::cout << "Histograms converted to trees and saved in 'signal_background_trees.root'!" << std::endl;
}

