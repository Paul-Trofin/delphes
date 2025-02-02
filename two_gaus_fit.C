// RUN LIKE THIS:
// root -l two_gaus_fit.C'("ff_z_ee_m_ee.root")'
// Include the necessary ROOT headers
#include <TCanvas.h>
#include <TH1F.h>
#include <TF1.h>
#include <TFile.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>

// Function to perform the fit and plot
void two_gaus_fit(const char *inputFile) {
    // Open the root file containing the tree
    TFile *File = TFile::Open(inputFile);

    // Retrieve the tree
    TTree *tree_m_ee = (TTree*)File->Get("tree_m_ee");

    // Create histogram to store the m_ee values
    TH1F *hist_m_ee = new TH1F("hist_m_ee", "Invariant Mass of e- e+ pairs", 100, 70, 110);
    tree_m_ee->Draw("m_ee>>hist_m_ee");

    // PLOT on CANVAS
    TCanvas *c1 = new TCanvas("c1","", 1920, 1080);
    gStyle->SetOptStat(0);
    hist_m_ee->GetXaxis()->SetTitle("m_ee (GeV)");
    hist_m_ee->GetYaxis()->SetTitle("(Counts / GeV)");
    hist_m_ee->Draw();

    // FIT the Histogram with FIRST GAUSSIAN
    double x_low = 80, x_high = 100;
    TF1 *gaus_fit_1 = new TF1("gaus_fit_1", "gaus", x_low, x_high);
    gaus_fit_1->SetNpx(1000);
    gaus_fit_1->SetLineWidth(4);
    gaus_fit_1->SetLineColor(kOrange+2);
    hist_m_ee->Fit(gaus_fit_1);

    // Extract parameters from first Gaussian
    double constant_1 = gaus_fit_1->GetParameter(0); // Constant
    double mean_1 = gaus_fit_1->GetParameter(1);     // Mean
    double sigma_1 = gaus_fit_1->GetParameter(2);    // Sigma
    double chi2_1 = gaus_fit_1->GetChisquare();      // Chi^2
    int ndf_1 = gaus_fit_1->GetNDF();                // Number of Degrees of Freedom

    // Calculate range for second Gaussian
    double range_low = mean_1 - 2 * sigma_1;
    double range_high = mean_1 + 2 * sigma_1;

    // Fit the Histogram with SECOND GAUSSIAN
    TF1 *gaus_fit_2 = new TF1("gaus_fit_2", "gaus", range_low, range_high);
    gaus_fit_2->SetNpx(1000);
    gaus_fit_2->SetLineWidth(5);
    gaus_fit_2->SetLineColor(kGreen+3);
    hist_m_ee->Fit(gaus_fit_2, "R+");

    // Extract parameters from second Gaussian
    double constant_2 = gaus_fit_2->GetParameter(0); // Constant
    double mean_2 = gaus_fit_2->GetParameter(1);     // Mean
    double sigma_2 = gaus_fit_2->GetParameter(2);    // Sigma
    double chi2_2 = gaus_fit_2->GetChisquare();      // Chi^2
    int ndf_2 = gaus_fit_2->GetNDF();                // Number of Degrees of Freedom

    // Add legend for Histogram
    TLegend *legend_hist = new TLegend(0.75, 0.85, 0.9, 0.9); // Position: x1, y1, x2, y2
    legend_hist->SetTextSize(0.03);
    legend_hist->SetFillStyle(0);  // Transparent background
    legend_hist->AddEntry(hist_m_ee, "Histogram", "l");
    legend_hist->Draw();

    // Add Entries, Mean, and Std Dev for the histogram
    TPaveText *hist_info = new TPaveText(0.75, 0.75, 0.9, 0.85, "NDC");
    hist_info->SetTextFont(42);
    hist_info->SetFillColor(0);
    hist_info->SetTextSize(0.02);
    hist_info->SetBorderSize(1);
    hist_info->AddText(Form("Entries: %.0f", hist_m_ee->GetEntries()));
    hist_info->AddText(Form("Mean: %.2f", hist_m_ee->GetMean()));
    hist_info->AddText(Form("Std Dev: %.2f", hist_m_ee->GetStdDev()));
    hist_info->Draw();

    // Add legend for Gaussian 1
    TLegend *legend_gaus1 = new TLegend(0.75, 0.70, 0.9, 0.75); // Position: x1, y1, x2, y2
    legend_gaus1->SetTextSize(0.03);
    legend_gaus1->SetFillStyle(0);  // Transparent background
    legend_gaus1->AddEntry(gaus_fit_1, "Gaussian Fit 1", "l");
    legend_gaus1->Draw();

    // Add fit parameters for Gaussian 1
    TPaveText *fit_info_1 = new TPaveText(0.75, 0.60, 0.9, 0.70, "NDC");
    fit_info_1->SetBorderSize(1);
    fit_info_1->SetTextFont(42);
    fit_info_1->SetFillColor(0);
    fit_info_1->SetTextSize(0.02);
    fit_info_1->AddText(Form("Constant: %.2f", constant_1));
    fit_info_1->AddText(Form("Mean: %.2f", mean_1));
    fit_info_1->AddText(Form("Sigma: %.2f", sigma_1));
    fit_info_1->AddText(Form("#chi^{2}/NDF: %.2f/%d", chi2_1, ndf_1));
    fit_info_1->Draw();

    // Add legend for Gaussian 2
    TLegend *legend_gaus2 = new TLegend(0.75, 0.55, 0.9, 0.60); // Position: x1, y1, x2, y2
    legend_gaus2->SetTextSize(0.03);
    legend_gaus2->SetFillStyle(0);  // Transparent background
    legend_gaus2->AddEntry(gaus_fit_2, "Gaussian Fit 2", "l");
    legend_gaus2->Draw();

    // Add fit parameters for Gaussian 2
    TPaveText *fit_info_2 = new TPaveText(0.75, 0.45, 0.9, 0.55, "NDC");
    fit_info_2->SetBorderSize(1);
    fit_info_2->SetTextFont(42);
    fit_info_2->SetFillColor(0);
    fit_info_2->SetTextSize(0.02);
    fit_info_2->AddText(Form("Constant: %.2f", constant_2));
    fit_info_2->AddText(Form("Mean: %.2f", mean_2));
    fit_info_2->AddText(Form("Sigma: %.2f", sigma_2));
    fit_info_2->AddText(Form("#chi^{2}/NDF: %.2f/%d", chi2_2, ndf_2));
    fit_info_2->Draw();

    // SAVE CANVAS
    c1->SaveAs("m_ee_plot.png");
}
