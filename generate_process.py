##############################################################
######### MACRO PYTHON FILE TO GENERATE .cmnd and .C #########
##### This script generates files for ./DelphesPythia8 #######
##############################################################
import os
                                                           #")
##############################################################
######################## OPTIONS #############################
##############################################################
### NAME 
name = "ff_gm_ee"
### PROCESS 
process = "f f~ > gm > e- e+"
### NUMBER OF EVENTS
Nevents = "10000"
### BEAM SETTINGS
idA = "2212"
idB = "2212"
eCM = "13600"
### PROCESS in PYTHIA8 FORMAT
options = []
options.append("WeakSingleBoson:ffbar2gmZ = on")
options.append("WeakSingleBoson:ffbar2ffbar(s:gm) = on")
options.append("WeakSingleBoson:ffbar2ffbar(s:gmZ) = on")
options.append("WeakDoubleBoson:ffbar2gmZgmZ = on")
options.append("WeakBosonAndParton:qqbar2gmZg = on")
options.append("WeakBosonAndParton:qg2Wq = on")
options.append("WeakBosonAndParton:qg2gmZq = on")
options.append("WeakBosonAndParton:ffbar2gmZgm = on")

#options.append("PartonLevel:MPI = off")
#options.append("HadronLevel:Hadronize = off")
#options.append("WeakZ0:gmZmode = 1 ! include only gamma decays")
options.append("! Force gamma decays to e- e+")
options.append("22:onMode = off")
options.append("22:onIfAny = 11 -11")
options.append("! Force Z decays to e- e+")
options.append("23:onMode = off")
options.append("23:onIfAny = 11 -11")
options.append("24:onMode = off")       # Turn off all W+ decays
options.append("24:onIfAny = 11 12")    # Allow only W+ → e+ ν_e
options.append("-24:onMode = off")      # Turn off all W- decays
options.append("-24:onIfAny = -11 -12") # Allow only W- → e- ν̅_e
##############################################################
print(" ______________________________________________________________")
print("|                                                              |")
print("|                ___________________________                   |")
print("|               |                           |                  |")
print("|               | DelphesPythia8 Simulation |                  |")
print("|               |___________________________|                  |")
print("|                                                              |")
print("|                                                              |")
print("|            Colliding: " + idA + " + " + idB + " at " + eCM + " GeV" + "               |")
print("|                                                              |")
print("|                      Process: " + process)
print("|                                                              |")
print("|                                                              |")
print("|                     AUTHOR : PAUL TROFIN                     |")
print("\______________________________________________________________/")

# CREATE FOLDER
folder_path = name
os.makedirs(folder_path, exist_ok=True)

# WRITE THE CMND FILE
file_path = os.path.join(folder_path, name + ".cmnd")
root_path = file_path.replace(".cmnd", ".root")
log_path = file_path.replace(".cmnd", ".log")

# Convert paths to absolute paths
abs_file_path = os.path.abspath(file_path)
abs_root_path = os.path.abspath(root_path)
abs_log_path = os.path.abspath(log_path)

with open(file_path, "w") as file:
    file.write("!!! COMMAND FILE FOR " + process + "\n")
    file.write("\n")
    file.write("!!! RUN LIKE THIS:\n")
    file.write(f"!!! ./DelphesPythia8 cards/delphes_card_ATLAS.tcl {abs_file_path} {abs_root_path} > {abs_log_path}\n")
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
    for option in options:
        file.write(option + "\n")

print(f"** The folder {name} has been created.")
print(f"** Inside, the following have been generated:")
print(f"          ** COMMAND FILE:")
print(f"                 -> {name}.cmnd      (generate events)")
print(f"          ** ANALYSIS FILES:")


##############################################################################
############# THIS IS THE MODIFIED ROOT SCRIPT FOR GENERATING .C #############
##############################################################################

##############################################################################
####################### KINAMETIC VARIABLES CALCULATION ######################
##############################################################################
root_script = f'''
//////////////////////////////////////////////////////////////////////
////////////////// INVARIANT MASS OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_variables.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_variables(const char *inputFile) {{
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

    // Initialize tree
    TTree *ElectronPairs = new TTree("ElectronPairs", "");
    
    double m_ee, pT_ee, eta_ee, phi_ee;
    
    ElectronPairs->Branch("m_ee", &m_ee, "m_ee/D");
    ElectronPairs->Branch("pT_ee", &pT_ee, "pT_ee/D");
    ElectronPairs->Branch("eta_ee", &eta_ee, "eta_ee/D");
    ElectronPairs->Branch("phi_ee", &phi_ee, "phi_ee/D");

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
                electrons.push_back(p4);

            }}

            if (pid == -11) {{ // positron
                TLorentzVector p4 = particle->P4();
                positrons.push_back(p4);
            }}
        }}

        // Calculate Kinematic variables
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
            	// INVARIANT MASS
                m_ee = (e + e_bar).M();             
                // PT
                pT_ee = (e + e_bar).Pt();
                // ETA
                eta_ee = (e + e_bar).Eta();
                // PHI
                phi_ee = std::abs(e.Phi() - e_bar.Phi());
                ElectronPairs->Fill();
            }}
        }}
    }}

    // SAVE histogram to file
    TFile outFile("variables.root", "RECREATE");
    ElectronPairs->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_variables.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_script)

print(f"                 -> get_variables.C  (calculate kinematic variables)")
