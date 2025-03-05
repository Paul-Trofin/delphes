/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A FILE THAT TAKES AS INPUT ROOT VARIABLES FILES
// RUN LIKE THIS:
// root -l stack_hist.C'("ff_z_ee/variables.root", "qq_gz/variables.root", "qg_qz/variables.root")'
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

void stack_hist(const char* signalFile, const char* bkgFile1, const char* bkgFile2) {
    std::vector<std::string> inputFiles = {signalFile, bkgFile1, bkgFile2};
    int nFiles = inputFiles.size();
    std::vector<TFile*> files(nFiles);
    std::vector<TTree*> trees(nFiles);
    std::vector<std::string> branches = {"m_ee", "pT_ee", "eta_ee", "phi_ee"};
    std::vector<int> colors = {kGray+2, kBlue-2, kOrange+2};
    std::vector<double> scales = {1.5e-6, 1.0e-6, 9e-7}; // Scale by crossx

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

        trees[i] = (TTree*)files[i]->Get("ElectronPairs");
        if (!trees[i]) {
            std::cerr << "** ERROR: Could not find tree 'ElectronPairs' in file " << inputFiles[i] << std::endl;
            files[i]->Close();
            return;
        }
    }

    // Ranges for each branch
    std::map<std::string, std::pair<double, double>> ranges;
    ranges["m_ee"] = {66, 116};
    ranges["pT_ee"] = {0, 200};
    ranges["eta_ee"] = {-10, 10};
    ranges["phi_ee"] = {0, 2 * TMath::Pi()};

    for (const auto& branch : branches) {
        std::vector<TH1F*> hist(nFiles);
        for (int i = 0; i < nFiles; i++) {
            hist[i] = new TH1F(Form("hist_%s_%d", branch.c_str(), i), branch.c_str(), 100, ranges[branch].first, ranges[branch].second);
            hist[i]->SetFillColor(colors[i]);
            hist[i]->SetLineColor(colors[i]);
            hist[i]->SetLineWidth(2);
        }

        for (int i = 0; i < nFiles; i++) {
            trees[i]->Draw(Form("%s>>hist_%s_%d", branch.c_str(), branch.c_str(), i), "", "", 50000);
            hist[i]->Scale(scales[i]);
        }

        THStack* stack = new THStack(Form("stack_%s", branch.c_str()), "");
        for (int i = 0; i < nFiles; i++) {
            stack->Add(hist[i]);
        }

        TCanvas* c1 = new TCanvas(Form("c1_%s", branch.c_str()), "", 1920, 1080);
        gStyle->SetOptStat(0);
        stack->Draw("NOSTACK HIST");

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

