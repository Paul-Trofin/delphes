/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A FILE THAT TAKES AS INPUT ROOT VARIABLES FILES
// SCALING FACTORS:
// ff_z    : 1.521e-06 (signal)
// ff_gm   : 3.633e-05 (bkg - 1)
// ff_w    : 5.877e-05 (bkg - 2)
// ff_tata : 1.514e-06 (bkg - 3)
// RUN LIKE THIS:
// SIGNAL FIRST, REST BACKGROUND
// root -l stack_hist.C'({"processes/ff_z/variables.root", "processes/ff_gm/variables.root", "processes/ff_w/variables.root", "processes/ff_tata/variables.root"}, {1.524e-06, 3.633e-05, 5.877e-05, 1.514e-06})'
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

void stack_hist(std::vector<std::string> inputFiles, std::vector<double> crossSection) {

    ///////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////// OPTIONS //////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // Number of Events per file (process)
    Int_t N_events = 70000;
    // Normalization procedure
    // 1 - Normalization to 1
    // 2 - Normalization to Integrated Luminosity
    Int_t NORM = 2;
    

    if (inputFiles.size() < 2 || inputFiles.size() != crossSection.size()) {
        std::cerr << "** ERROR: Input file and scale vectors must have the same size and at least one background file." << std::endl;
        return;
    }

    int nFiles = inputFiles.size();
    std::vector<TFile*> files(nFiles);
    std::vector<TTree*> trees(nFiles);
    std::vector<std::string> branches = {"DielectronMass", "Dielectron_p4.Px", "Dielectron_p4.Py", "Dielectron_p4.Pz", "Dielectron_p4.Pt", "Dielectron_angle.eta", "Dielectron_angle.phi", "Dielectron_angle.theta", "DielectronDeltaR"};
    std::vector<int> colors = {kGray+2, kGreen+3, kBlue+2, kRed+2, kCyan+2, kYellow+2};

    for (int i = 0; i < nFiles; i++) {
        crossSection[i] = crossSection[i] * 1e12; // Convert to fb
    }

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
        {"DielectronMass", {50, 150}},
        {"DielectronDeltaR", {0, 7}},  // Fixed key mismatch
        {"Dielectron_p4.Pt", {0, 50}},
        {"Dielectron_p4.Px", {-50, 50}},
        {"Dielectron_p4.Py", {-50, 50}},
        {"Dielectron_p4.Pz", {-50, 50}},
        {"Dielectron_angle.eta", {-10, 10}},
        {"Dielectron_angle.phi", {0, 2 * TMath::Pi()}},
        {"Dielectron_angle.theta", {0, TMath::Pi()}}
    };


    for (const auto& branch : branches) {
        if (ranges.find(branch) == ranges.end()) {
            std::cerr << "** ERROR: No defined range for branch " << branch << std::endl;
            continue;
        }

        std::vector<TH1F*> hist(nFiles);
        for (int i = 0; i < nFiles; i++) {
            hist[i] = new TH1F(Form("hist_%s_%d", branch.c_str(), i), branch.c_str(), 150, ranges[branch].first, ranges[branch].second);
            hist[i]->SetFillColor(colors[i % colors.size()]);
            hist[i]->SetLineColor(kBlack);
            hist[i]->SetLineWidth(2);
        }
        
        double Lint_total = 0;
        double Hint_total = 0;
        double Hint_final = 0;
        double Lint_final = 0;
        for (int i = 0; i < nFiles; i++) {
            trees[i]->Draw(Form("%s>>hist_%s_%d", branch.c_str(), branch.c_str(), i), "", "", N_events);
            hist[i]->Scale(crossSection[i]);

            // Histogram Integral Calculation
            Hint_total += hist[i]->Integral();
            // Total Integrated Luminosity calculation
            Lint_total += N_events / crossSection[i];
        }

        if (NORM == 1) {
            for (int i = 0; i < nFiles; i++) {
                hist[i]->Scale(1.0 / Hint_total);
            }
            for (int i = 0; i < nFiles; i++) {
                Hint_final += hist[i]->Integral();
            }
            std::cout << std::endl << "Hint_final = " << Hint_final << std::endl;
                
        }

        if (NORM == 2) {
            for (int i = 0; i < nFiles; i++) {
                hist[i]->Scale(Lint_total / Hint_total);
            }
            for (int i = 0; i < nFiles; i++) {
                Lint_final += hist[i]->Integral();
            }
            std::cout << std::endl << "Lint_final = " << Lint_final << " fb-1" << std::endl;
            std::cout << std::endl << "Lint_total = " << Lint_total << " fb-1" << std::endl;
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
        stack->SetMaximum(2 * maxY);  // Scale Y-axis
        c1->Modified();  // Force redraw with updated limits
        c1->Update();

        TLegend* legend = new TLegend(0.67, 0.6, 0.9, 0.9);
        legend->SetTextSize(0.02);
        legend->SetFillStyle(0);
        
        for (int i = 0; i < nFiles; i++) {
            // Extract the actual process name from the file path
            std::string fileName = inputFiles[i];
        
            // Find the substring between "processes/" and "/variables.root"
            size_t startPos = fileName.find("processes/") + std::string("processes/").length();
            size_t endPos = fileName.find("/variables.root");
            fileName = fileName.substr(startPos, endPos - startPos); // Extract the process name
        
            // Replace ff with f fbar
            size_t pos = fileName.find("ff");
            if (pos != std::string::npos) {
                fileName.replace(pos, 2, "f fbar");
            }
        
            // Replace gm with \gamma
            pos = fileName.find("gm");
            if (pos != std::string::npos) {
                fileName.replace(pos, 2, "\\gamma");
            }
        
            // Replace z with Z
            pos = fileName.find("z");
            if (pos != std::string::npos) {
                fileName.replace(pos, 1, "Z");
            }
        
            // Replace w with W
            pos = fileName.find("w");
            if (pos != std::string::npos) {
                fileName.replace(pos, 1, "W");
            }
        
            // Replace underscores with '->'
            size_t posArrow = fileName.find('_');
            while (posArrow != std::string::npos) {
                fileName.replace(posArrow, 1, "->");
                posArrow = fileName.find('_', posArrow + 1);
            }
        
            // Create the legend entry with cross-section
            std::string legendEntry = Form("%s \\quad \\quad (\\sigma = %.2e \\quad fb)", fileName.c_str(), crossSection[i]);
        
            // Add the entry to the legend
            legend->AddEntry(hist[i], legendEntry.c_str(), "f");
        }                 
        
        legend->Draw();
        
        gPad->RedrawAxis();
        c1->Update();
        c1->SaveAs(Form("PLOTS/%s.png", branch.c_str()));
    }
}
