//////////////////////////////////////////////////////////////////////
/////////////////////// PSEUDORAPIDITY OF e- e+ PAIRS ///////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_eta.C'("ff_z_ee.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_eta(const char *inputFile) {
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
    for (int event = 0; event < Nevents; event++) {
        treeReader->ReadEntry(event);

        // Store electrons and positrons
        std::vector<TLorentzVector> electrons, positrons;

        // Loop over particles
        for (int i = 0; i < branchParticle->GetEntries(); i++) {
            // Get particle i
            GenParticle *particle = (GenParticle *)branchParticle->At(i);
            int pid = particle->PID;

            // Store electrons and positrons
            if (pid == 11) { // electron
                TLorentzVector p4 = particle->P4();
                if (p4.Pt() > 27 && abs(p4.Eta()) < 2.47) {
                    electrons.push_back(p4);
                }
            }

            if (pid == -11) { // positron
                TLorentzVector p4 = particle->P4();
                if (p4.Pt() > 27 && abs(p4.Eta()) < 2.47) {
                    positrons.push_back(p4);
                }
            }
        }

        // Calculate Pseudorapidity eta_ee
        for (auto &e : electrons) {
            for (auto &e_bar : positrons) {
                eta_ee = (e + e_bar).Eta();
                tree_eta_ee->Fill();
            }
        }
    }

    // SAVE tree to file
    TFile outFile("ff_z_ee_eta.root", "RECREATE");
    tree_eta_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}

