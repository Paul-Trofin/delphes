/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A FILE THAT TAKES AS INPUT ROOT VARIABLES FILES
// SCALING FACTORS:
// ff_z_ee : 1.524e-06 (signal)
// ff_tata : 1.514e-06 (bkg -?)
// ff_gm_ee: 3.633e-05 (bkg - red)
// ttbar   : 6.574e-09
// ff_zz   : 1.091e-11 (bkg - irr?)
// ff_zw   : 9.020e-11 (bkg - irr?)
// ff_ww   : 7.978e-10 (bkg - irr?)
// ttbar   : 6.625e-09 (bkg - ?)
// RUN LIKE THIS:
// SIGNAL FIRST, REST BACKGROUND
// root -l stack_hist.C'({"processes/ff_z_ee/variables.root", "processes/ff_tata/variables.root", "processes/ff_gm_ee/variables.root", "processes/ttbar/variables.root"}, {1.524e-06, 1.514e-06, 3.633e-05, 6.574e-09})'
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <cstring>
#include <TCanvas.h>
#include <TH1F.h>
#include <TFile.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <THStack.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <map>

void stack_hist(std::vector<std::string> inputFiles, std::vector<double> scales) {
    if (inputFiles.size() < 2 || inputFiles.size() != scales.size()) {
        std::cerr << "** ERROR: Input file and scale vectors must have the same size and at least one background file." << std::endl;
        return;
    }

    int nFiles = inputFiles.size();
    std::vector<TFile*> files(nFiles);
    std::vector<TTree*> trees(nFiles);
    std::vector<std::string> branches = {"DielectronMass", "Z_p4.Pt", "Z_angles.eta", "Z_angles.phi", "Z_angles.theta", "deltaR_ee"};
    std::vector<int> colors = {kGray+2, kBlue+2, kOrange+3, kGreen+2, kRed+2, kCyan+2, kYellow+2}; 

    struct stat info;
    if (stat("PLOTS", &info) != 0) {
        system("mkdir -p PLOTS");
    }

    for (int i = 0; i < nFiles; i++) {
        files[i] = TFile::Open(inputFiles[i].c_str());
        if (!files[i]) {
            std::cerr << "** ERROR: Could not open file " << inputFiles[i] << std::endl;
            return;
        }

        trees[i] = (TTree*)files[i]->Get("Dielectrons");
        if (!trees[i]) {
            std::cerr << "** ERROR: Could not find tree 'Dielectrons' in file " << inputFiles[i] << std::endl;
            files[i]->Close();
            return;
        }
    }

    std::map<std::string, std::pair<double, double>> ranges = {
        {"DielectronMass", {0, 200}},
        {"deltaR_ee", {2, 5}},  // Fixed key mismatch
        {"Z_p4.Pt", {0, 10}},
        {"Z_angles.eta", {-10, 10}},
        {"Z_angles.phi", {0, 2 * TMath::Pi()}},
        {"Z_angles.theta", {0, TMath::Pi()}}
    };

    for (const auto& branch : branches) {
        if (ranges.find(branch) == ranges.end()) {
            std::cerr << "** ERROR: No defined range for branch " << branch << std::endl;
            continue;
        }

        std::vector<TH1F*> hist(nFiles);
        for (int i = 0; i < nFiles; i++) {
            hist[i] = new TH1F(Form("hist_%s_%d", branch.c_str(), i), branch.c_str(), 200, ranges[branch].first, ranges[branch].second);
            hist[i]->SetFillColor(colors[i % colors.size()]);
            hist[i]->SetLineColor(kBlack);
            hist[i]->SetLineWidth(2);
        }

        for (int i = 0; i < nFiles; i++) {
            trees[i]->Draw(Form("%s>>hist_%s_%d", branch.c_str(), branch.c_str(), i), "", "", 10000);
            hist[i]->Scale(scales[i]);
        }

        THStack* stack = new THStack(Form("stack_%s", branch.c_str()), "");
		
        for (int i = 1; i < nFiles; i++) {
            stack->Add(hist[i]); // Background first
        }
        stack->Add(hist[0]); // Signal on top

        TCanvas* c1 = new TCanvas(Form("c1_%s", branch.c_str()), "", 1920, 1080);
        gStyle->SetOptStat(0);

        stack->Draw("HIST");
        gPad->Update();  // Ensure ROOT registers the drawn histograms

        double maxY = stack->GetMaximum();
        stack->SetMaximum(1.2 * maxY);  // Scale Y-axis
        c1->Modified();  // Force redraw with updated limits
        c1->Update();

        TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
        legend->SetTextSize(0.02);
        legend->SetFillStyle(0);
        for (int i = 0; i < nFiles; i++) {
            legend->AddEntry(hist[i], inputFiles[i].c_str(), "l");
        }
        legend->Draw();

        gPad->RedrawAxis();
        c1->Update();
        c1->SaveAs(Form("PLOTS/%s.png", branch.c_str()));
    }
}

