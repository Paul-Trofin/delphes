/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A FILE THAT TAKES AS INPUT ROOT FILES (inv mass m_ee)
// THE FIRST FILE IS THE SIGNAL
// THE FOLLOWING FILES ARE BACKGROUNG
// RUN LIKE THIS:
// root -l signal_bkg_plot.C'("ff_z_ee/ff_z_ee_m_ee.root", "ff_zz_4e/ff_zz_4e_m_ee.root", "qg_qz/qg_qz_m_ee.root", "qq_gz/qq_gz_m_ee.root")'
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <TCanvas.h>
#include <TH1F.h>
#include <TF1.h>
#include <TFile.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>

void signal_bkg_plot(const char* inputFile0, const char* inputFile1, const char* inputFile2, const char* inputFile3) {
    // Open signal file
    TFile* file0 = TFile::Open(inputFile0);
    if (!file0) {
        std::cerr << "** ERROR: Could not open file " << inputFile0 << std::endl;
        return;
    }

    // Get signal tree
    TTree* tree0 = (TTree*)file0->Get("tree_m_ee");
    if (!tree0) {
        std::cerr << "** ERROR: Could not find tree 'tree_m_ee' in file " << inputFile0 << std::endl;
        file0->Close();
        return;
    }

    // Open 1st bkg file
    TFile* file1 = TFile::Open(inputFile1);
    if (!file1) {
        std::cerr << "** ERROR: Could not open file " << inputFile1 << std::endl;
        return;
    }

    // Get 1st bkg tree
    TTree* tree1 = (TTree*)file1->Get("tree_m_ee");
    if (!tree1) {
        std::cerr << "** ERROR: Could not find tree 'tree_m_ee' in file " << inputFile1 << std::endl;
        file1->Close();
        return;
    }

    // Open 2nd bkg file
    TFile* file2 = TFile::Open(inputFile2);
    if (!file2) {
        std::cerr << "** ERROR: Could not open file " << inputFile2 << std::endl;
        return;
    }

    // Get 2nd bkg tree
    TTree* tree2 = (TTree*)file2->Get("tree_m_ee");
    if (!tree2) {
        std::cerr << "** ERROR: Could not find tree 'tree_m_ee' in file " << inputFile2 << std::endl;
        file2->Close();
        return;
    }

    // Open 3rd bkg file
    TFile* file3 = TFile::Open(inputFile3);
    if (!file3) {
        std::cerr << "** ERROR: Could not open file " << inputFile3 << std::endl;
        return;
    }

    // Get 3rd bkg tree
    TTree* tree3 = (TTree*)file3->Get("tree_m_ee");
    if (!tree3) {
        std::cerr << "** ERROR: Could not find tree 'tree_m_ee' in file " << inputFile3 << std::endl;
        file3->Close();
        return;
    }

    // Create histograms to store the m_ee values for the signal and backgrounds
    TH1F* hist0 = new TH1F("hist0", "Invariant Mass of e- e+ pairs (Signal + Background)", 100, 70, 110);
    // Set hist0 to be black and filled black
    hist0->SetLineColor(kBlack);  // Set the line color to black
    hist0->SetFillColor(kBlack);  // Set the fill color to black
    hist0->SetFillStyle(1001);    // Solid fill style
    hist0->SetLineColor(kBlack);
    tree0->Draw("m_ee>>hist0");

    TH1F* hist1 = new TH1F("hist1", "Invariant Mass of e- e+ pairs (Bkg 1)", 100, 70, 110);

    TH1F* hist2 = new TH1F("hist2", "Invariant Mass of e- e+ pairs (Bkg 2)", 100, 70, 110);

    TH1F* hist3 = new TH1F("hist3", "Invariant Mass of e- e+ pairs (Bkg 3)", 100, 70, 110);

    // Fill background histograms
    tree1->Draw("m_ee>>hist1");
    tree2->Draw("m_ee>>hist2");
    tree3->Draw("m_ee>>hist3");

    // Create a new histogram for the sum of the backgrounds
    TH1F* hist_bkg_1 = (TH1F*)hist1->Clone("hist1");

    TH1F* hist_bkg_12 = (TH1F*)hist1->Clone("hist1");
    hist_bkg_12->Add(hist2);

    TH1F* hist_bkg_123 = (TH1F*)hist1->Clone("hist1");
    hist_bkg_123->Add(hist2);
    hist_bkg_123->Add(hist3);

    
    // PLOT on CANVAS
    TCanvas* c1 = new TCanvas("c1", "", 1920, 1080);
    gStyle->SetOptStat(0);
    hist0->GetXaxis()->SetTitle("m_ee (GeV)");
    hist0->GetYaxis()->SetTitle("(Counts / GeV)");

    // Update the y-axis range to accommodate both signal and background histograms
    float maxHist0 = hist0->GetMaximum();
    float maxHistBkg = hist_bkg_123->GetMaximum();
    hist0->SetMaximum(1.2 * std::max(maxHist0, maxHistBkg));  // Increase range for clarity

    hist0->Draw();

    hist_bkg_1->SetLineColor(kRed);  // Red for the combined background
    hist_bkg_1->Draw("SAME");
    
    hist_bkg_12->SetLineColor(kGreen);  // Red for the combined background
    hist_bkg_12->Draw("SAME");
    
    hist_bkg_123->SetLineColor(kViolet);  // Red for the combined background
    hist_bkg_123->Draw("SAME");

    // Save the canvas as an image
    c1->SaveAs("signal_bkg_plot.png");
}


