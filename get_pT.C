
//////////////////////////////////////////////////////////////////////
////////////////// INVARIANT MASS OF e- e+ PAIRS /////////////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
//// RUN LIKE THIS:
//// root -l get_pT.C'("ff_z_ee.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delphes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void get_pT(const char *inputFile) {
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
    TTree *tree_pT_ee = new TTree("tree_pT_ee", "");
    double pT_ee;
    tree_pT_ee->Branch("pT_ee", &pT_ee, "pT_ee/D");

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

        // Calculate Total pT
        for (auto &e : electrons) {
            for (auto &e_bar : positrons) {
                pT_ee = abs((e - e_bar).Pt());
                tree_pT_ee->Fill();
            }
        }
    }

    // SAVE histogram to file
    TFile outFile("ff_z_ee_pT_ee.root", "RECREATE");
    tree_pT_ee->Write();
    outFile.Close();

    // Clean up
    delete treeReader;
}
