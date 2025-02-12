############################################################
####### THIS CODE IS USED TO GENERATE .C FILES #############
######### THAT WILL PLOT AND STACK HISTOGRAMS ##############
############################################################
#### RUN LIKE THIS:
#### python3 generate_plots.py
import os

##############################################################
######################## OPTIONS #############################
##############################################################
### CHOOSE KINEMATIC VARIABLE
variable = "eta"  # Options: inv_mass, pT, E, eta

### CHOOSE HISTOGRAM SCALING
scaling = 1  # 0 - no scaling, 1 - scaling

### GIVE SIGNAL FOLDER LOCATION
signal = "ff_z_ee"

### GIVE BKG FOLDER LOCATION
backgrounds = ["ff_zz_4e", "qg_qz", "qq_gz", "ff_za", "fa_fz"]  # List can be modified

##############################################################
##############################################################
##############################################################
nFiles = len(backgrounds) + 1  # Signal + backgrounds

if variable == "inv_mass":
    tree = "tree_m_ee"
    tree_var = "m_ee"
    function_name = "stack_inv_mass"
    filename = "stack_inv_mass.C"
    if scaling == 0:
        image_name = "stacked_inv_mass.png"
    if scaling == 1:
        image_name = "stacked_inv_mass_scaled.png"
    x_min = "66"
    x_max = "116"

if variable == "pT":
    tree = "tree_pT_ee"
    tree_var = "pT_ee"
    filename = "stack_pT.C"
    function_name = "stack_pT"
    if scaling == 0:
        image_name = "stacked_pT.png"
    if scaling == 1:
        image_name = "stacked_pT_scaled.png"
    x_min = "66"
    x_max = "116"

if variable == "E":
    tree = "tree_E_ee"
    tree_var = "E_ee"
    filename = "stack_E.C"
    function_name = "stack_E"
    if scaling == 0:
        image_name = "stacked_E.png"
    if scaling == 1:
        image_name = "stacked_E_scaled.png"
    x_min = "66"
    x_max = "300"

if variable == "eta":
    tree = "tree_eta_ee"
    tree_var = "eta_ee"
    filename = "stack_eta.C"
    function_name = "stack_eta"
    if scaling == 0:
        image_name = "stacked_eta.png"
    if scaling == 1:
        image_name = "stacked_eta_scaled.png"
    x_min = "-10"
    x_max = "10"

# DYNAMIC ROOT COMMAND
root_command = f'root -l {filename}\'("{signal}/{variable}.root"'
for bkg in backgrounds:
    root_command += f', "{bkg}/{variable}.root"'
root_command += ")'"

# DYNAMIC FUNCTION PARAMETERS
input_params = ", ".join([f"const char* inputFile{i}" for i in range(nFiles)])
input_files = ", ".join([f"inputFile{i}" for i in range(nFiles)])

content = f'''
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A FILE THAT TAKES AS INPUT ROOT FILES ({variable} {tree_var})
// THE FIRST FILE IS THE SIGNAL
// THE FOLLOWING FILES ARE BACKGOUND
// RUN LIKE THIS:
// {root_command}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <cstring>
#include <TCanvas.h>
#include <TH1F.h>
#include <TF1.h>
#include <TFile.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <vector>
#include <string>

void {function_name}({input_params}) {{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// OPEN FILES /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int nFiles = {nFiles};
    std::vector<std::string> inputFiles = {{{input_files}}};
    std::vector<TFile*> files(nFiles);
    std::vector<TTree*> trees(nFiles);

    for (int i = 0; i < nFiles; i++) {{
        files[i] = TFile::Open(inputFiles[i].c_str());
        if (!files[i]) {{
            std::cerr << "** ERROR: Could not open file " << inputFiles[i] << std::endl;
            return;
        }}

        trees[i] = (TTree*)files[i]->Get("{tree}");
        if (!trees[i]) {{
            std::cerr << "** ERROR: Could not find tree '{tree}' in file " << inputFiles[i] << std::endl;
            files[i]->Close();
            return;
        }}
    }}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// FILL HISTOGRAMS /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<TH1F*> hist(nFiles);
    std::vector<int> color = {{kGray+2, kBlue-2, kOrange+2, kGreen+2, kRed+2, kCyan+2}};

    for (int i = 0; i < nFiles; i++) {{
        hist[i] = new TH1F(Form("hist%d", i), "", 100, {x_min}, {x_max});
        hist[i]->SetFillColor(color[i]);
        hist[i]->SetLineColor(color[i]);
        hist[i]->SetLineWidth(10);
    }}

    for (int i = 0; i < nFiles; i++) {{
        trees[i]->Draw(Form("{tree_var}>>hist%d", i));
    }}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// SCALE HISTOGRAMS ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if ({scaling} == 1) {{
        std::vector<double> sigma = {{1.509e-06, 2.450e-10, 9.831e-09, 1.033e-06, 9.074e-07, 1.092e-11}};
        for (int i = 0; i < nFiles; i++) {{
            for (int bin = 1; bin <= hist[i]->GetNbinsX(); bin++) {{
                double content = hist[i]->GetBinContent(bin);
                hist[i]->SetBinContent(bin, content * sigma[i]);
            }}
        }}
    }}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// STACK HISTOGRAMS ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    THStack* stack = new THStack("stack", "");
    for (int i = 1; i < nFiles; i++) {{
        stack->Add(hist[i]);
    }}
    stack->Add(hist[0]);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// PLOT ON CANVAS /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    TCanvas* c1 = new TCanvas("c1", "", 1920, 1080);
    gStyle->SetOptStat(0);
    stack->Draw();

    std::vector<std::string> process = {{
        "f f~ > Z > e- e+ (Signal)",
        {", ".join([f'"{bkg}"' for bkg in backgrounds])}
    }};

    double y_min = 0.9;
    for (int i = nFiles - 1; i >= 0; i--) {{
        TLegend *legend = new TLegend(0.7, y_min - 0.05, 0.9, y_min);
        legend->SetTextSize(0.02);
        legend->SetFillStyle(0);
        legend->AddEntry(hist[i], process[i].c_str(), "l");
        legend->Draw();

        TPaveText *info = new TPaveText(0.7, y_min - 0.1, 0.9, y_min - 0.05, "NDC");
        info->SetTextFont(30);
        info->SetFillColor(0);
        info->SetTextSize(0.02);
        info->SetBorderSize(1);
        info->AddText(Form("Entries: %.0f     Mean: %.2f      Std Dev: %.2f", hist[i]->GetEntries(), hist[i]->GetMean(), hist[i]->GetStdDev()));
        info->Draw();
        
        y_min -= 0.1;
    }}

    gPad->RedrawAxis();
    c1->Update();
    c1->SaveAs("PLOTS/{image_name}");
}}
'''

out_file = open(filename, 'w')
out_file.write(content)
print(f" -> {filename} generated successfully!")


