##############################################################
######### MACRO PYTHON FILE TO GENERATE .cmnd and .C #########
####### This is only a script that writes other files ########
##############################################################
import os

##############################################################
######################## OPTIONS #############################
##############################################################
### NAME 
name = "qq_gz"
### PROCESS 
process = "q q > z z > 4e"
### NUMBER OF EVENTS
Nevents = "10000"
### BEAM SETTINGS
idA = "2212"
idB = "2212"
eCM = "13600"
### PROCESS in PYTHIA8 FORMAT
py8Process = "WeakBosonAndParton:qqbar2gmZg = on"
### OPTIONS in PYTHIA8 FORMAT
opt1 = "WeakZ0:gmZmode = 2 ! include only Z decays"
opt2 = "! Force Z decays to e- e+"
opt3 = "23:onMode = off"
opt4 = "23:onIfAny = 11 -11"
##############################################################

# CREATE FOLDER
folder_path = name
os.makedirs(folder_path, exist_ok=True)

# WRITE THE CMND FILE
file_path = os.path.join(folder_path, name + ".cmnd")
with open(file_path, "w") as file:
    file.write("!!! COMMAND FILE FOR " + process + "\n")
    file.write("\n")
    file.write("!!! RUN LIKE THIS:\n")
    file.write("!!! ./DelphesPythia8 cards/delphes_card_ATLAS.tcl simulations/pp_z_ee/1e4_events/" + name + "/" + name + ".cmnd simulations/pp_z_ee/1e4_events/" + name + "/" + name + ".root > simulations/pp_z_ee/1e4_events/" + name + "/" + name + ".log\n")
    file.write("\n\n")
    
    file.write("! Number of events to generate\n")
    file.write("Main:numberOfEvents = " + Nevents + "\n")
    file.write("\n")
    
    file.write("! BEAM SETTINGS\n")
    file.write("Beams:idA = " + idA + "\n")
    file.write("Beams:idB = " + idB + "\n")
    file.write("Beams:eCM = " + eCM + "\n")
    file.write("\n")
    
    file.write("! PROCESS\n")
    file.write(py8Process + "\n")
    file.write(opt1 + "\n")
    file.write(opt2 + "\n")
    file.write(opt3 + "\n")
    file.write(opt4 + "\n")

print(f"** DELPHES PYTHIA8 COMMAND ({name}.cmnd) HAS BEEN GENERATED.")

##############################################################################
############# THIS IS THE MODIFIED ROOT SCRIPT FOR GENERATING .C #############
######################## used GPT for faster writting ########################
##############################################################################

root_script = f'''
//////////////////////////////////////////////////////////////////////
////////////////// INVARIANT MASS OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l {name}.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY("/home/paul/delphes/libDelphes.so")
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void {name}(const char *inputFile) {{
    // Load Delphes library
    gSystem->Load("/home/paul/delphes/libDelphes.so");

    // TChain
    TChain chain("Delphes");
    chain.Add(inputFile);

    // ExRootTreeReader
    ExRootTreeReader *treeReader = new ExRootTreeReader(&chain);
    Long64_t Nevents = treeReader->GetEntries();

    // Point to the Particle branch
    TClonesArray *branchParticle = treeReader->UseBranch("Particle");

    // Book histogram for e- e+ invariant mass
    TTree *tree_m_ee = new TTree("tree_m_ee", "");
    double m_ee;
    tree_m_ee->Branch("m_ee", &m_ee, "m_ee/D");

    // Loop over events
    for (int event = 0; event < Nevents; event++) {{
        treeReader->ReadEntry(event);

        // Store electrons and positrons
        std::vector<TLorentzVector> electrons, positrons;

        // Loop over particles
        for (int i = 0; i < branchParticle->GetEntries(); i++) {{
            // Get particle i
            GenParticle *particle = (GenParticle *)branchParticle->At(i);
            int pid = particle->PID;

            // Store electrons and positrons
            if (pid == 11) {{ // electron
                TLorentzVector p4 = particle->P4();
                if (p4.Pt() > 27 && abs(p4.Eta()) < 2.47) {{
                    electrons.push_back(p4);
                }}
            }}

            if (pid == -11) {{ // positron
                TLorentzVector p4 = particle->P4();
                if (p4.Pt() > 27 && abs(p4.Eta()) < 2.47) {{
                    positrons.push_back(p4);
                }}
            }}
        }}

        // Calculate Invariant Mass m_ee
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
                m_ee = (e + e_bar).M();
                tree_m_ee->Fill();
            }}
        }}
    }}

    // SAVE histogram to file
    TFile outFile("{name}.root", "RECREATE");
    tree_m_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, f"{name}.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_script)

print(f"** ROOT SCRIPT ({name}.C) HAS BEEN GENERATED.")
