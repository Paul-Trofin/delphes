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
name = "fa_fz"
### PROCESS 
process = "f a > f z"
### NUMBER OF EVENTS
Nevents = "10000"
### BEAM SETTINGS
idA = "2212"
idB = "2212"
eCM = "13600"
### PROCESS in PYTHIA8 FORMAT
options = []
options.append("WeakBosonAndParton:fgm2gmZf = on")
options.append("WeakZ0:gmZmode = 2 ! include only Z decays")
options.append("! Force Z decays to e- e+")
options.append("23:onMode = off")
options.append("23:onIfAny = 11 -11")
##############################################################

print("\n")
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
print("|                      Process: " + process + "                      |")
print("|                                                              |")
print("|                                                              |")
print("|                     AUTHOR : PAUL TROFIN                     |")
print("\______________________________________________________________/")

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
    for option in options:
    	file.write(option + "\n")
    	
print(f"** The folder {name} has been created.")
print(f"** Inside, the following have been generated:")
print(f"          ** COMMAND FILE:")
print(f"                 -> {name}.cmnd      (generate events)")
print(f"          ** ANALYSIS FILES:")

##############################################################################
############# THIS IS THE MODIFIED ROOT SCRIPT FOR GENERATING .C #############
######################## used GPT for faster writting ########################
##############################################################################

######################################################################
### inv_mass
######################################################################
root_inv_mass = f'''
//////////////////////////////////////////////////////////////////////
////////////////// INVARIANT MASS OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_inv_mass.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_inv_mass(const char *inputFile) {{
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
    TFile outFile("inv_mass.root", "RECREATE");
    tree_m_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_inv_mass.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_inv_mass)

print(f"                 -> get_inv_mass.C  (calculate invariant mass)")

######################################################################
### pT
######################################################################
root_pT = f'''
//////////////////////////////////////////////////////////////////////
//////////////////////// TRANSVERSE MOMENTUM /////////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_pT.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_pT(const char *inputFile) {{
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

    // Book histogram for e- e+ transverse momentum
    TTree *tree_pT_ee = new TTree("tree_pT_ee", "");
    double pT_ee;
    tree_pT_ee->Branch("pT_ee", &pT_ee, "pT_ee/D");

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

        // Calculate Total pT
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
                pT_ee = abs((e - e_bar).Pt());
                tree_pT_ee->Fill();
            }}
        }}
    }}

    // SAVE histogram to file
    TFile outFile("pT.root", "RECREATE");
    tree_pT_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_pT.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_pT)

print(f"                 -> get_pT.C        (calculate transverse momentum)")

######################################################################
### pT
######################################################################
root_E = f'''
//////////////////////////////////////////////////////////////////////
//////////////////// TOTAL ENERGY OF e- e+ PAIRS ////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_E.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_E(const char *inputFile) {{
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

    // Book tree for e- e+ total energy
    TTree *tree_E_ee = new TTree("tree_E_ee", "");
    double E_ee;
    tree_E_ee->Branch("E_ee", &E_ee, "E_ee/D");

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

        // Calculate Total Energy E_ee
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
                E_ee = (e + e_bar).E();
                tree_E_ee->Fill();
            }}
        }}
    }}

    // SAVE tree to file
    TFile outFile("E.root", "RECREATE");
    tree_E_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''


# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_E.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_E)

print(f"                 -> get_E.C         (calculate total energy)")

######################################################################
### eta
######################################################################
root_eta = f'''
//////////////////////////////////////////////////////////////////////
////////////////// PSEUDORAPIDITY OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_eta.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_eta(const char *inputFile) {{
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

    // Book tree for e- e+ pseudorapidity
    TTree *tree_eta_ee = new TTree("tree_eta_ee", "");
    double eta_ee;
    tree_eta_ee->Branch("eta_ee", &eta_ee, "eta_ee/D");

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

        // Calculate Pseudorapidity eta_ee
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
                eta_ee = (e + e_bar).Eta();
                tree_eta_ee->Fill();
            }}
        }}
    }}

    // SAVE tree to file
    TFile outFile("eta.root", "RECREATE");
    tree_eta_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_eta.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_eta)

print(f"                 -> get_eta.C       (calculate pseudorapidity)")

######################################################################
### phi
######################################################################
root_phi = f'''
//////////////////////////////////////////////////////////////////////
///////////////// AZIMUTHAL ANGLE OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_phi.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif(gen

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_phi(const char *inputFile) {{
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

    // Book tree for e- e+ azimuthal angle
    TTree *tree_phi_ee = new TTree("tree_phi_ee", "");
    double phi_ee;
    tree_phi_ee->Branch("phi_ee", &phi_ee, "phi_ee/D");

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

        // Calculate Azimuthal Angle phi_ee
        for (auto &e : electrons) {{
            for (auto &e_bar : positrons) {{
                phi_ee = (e + e_bar).Phi();
                tree_phi_ee->Fill();
            }}
        }}
    }}

    // SAVE tree to file
    TFile outFile("phi.root", "RECREATE");
    tree_phi_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_phi.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_phi)

print(f"                 -> get_phi.C       (calculate azimuthal angle)")

######################################################################
### all
######################################################################
root_all = f'''
//////////////////////////////////////////////////////////////////////
/////////// RUN ALL .C FILES IN A SINGLE COMMAND LINE ////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_all.C'("{name}.root")'
//////////////////////////////////////////////////////////////////////
void get_all(const char *inputFile) {{
    gROOT->ProcessLine(Form(".x get_inv_mass.C(\\"%s\\")", inputFile));
    gROOT->ProcessLine(Form(".x get_pT.C(\\"%s\\")", inputFile));
    gROOT->ProcessLine(Form(".x get_E.C(\\"%s\\")", inputFile));
    gROOT->ProcessLine(Form(".x get_eta.C(\\"%s\\")", inputFile));
    gROOT->ProcessLine(Form(".x get_phi.C(\\"%s\\")", inputFile));
}}
'''

# Create the ROOT script file and write the contents
root_file_path = os.path.join(folder_path, "get_all.C")
with open(root_file_path, "w") as root_file:
    root_file.write(root_all)

print(f"                 -> get_all.C       (for running all calculations at once)")
