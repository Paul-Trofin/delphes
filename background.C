//////////////////////////////////////////////////////////////////////
///////////// BACKGROUND INVARIANT MASS OF e- e+ PAIRS ///////////////
/////////////// READS INPUT FROM DELPHES-ROOT FILE ///////////////////
//////////////////////////////////////////////////////////////////////
// RUN LIKE THIS:
// root -l background.C'("background.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delhes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void background(const char *inputFile) {
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
    TH1 *hist_m_ee = new TH1D("hist_m_ee", "", 100, 66, 116);

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

        // Calculate Invariant Mass m_ee
        for (auto &e : electrons) {
            for (auto &e_bar : positrons) {
                double m_ee = (e + e_bar).M();
                hist_m_ee->Fill(m_ee);
            }
        }
    }

    // SAVE histogram to file
    TFile outFile("background_m_ee.root", "RECREATE");
    hist_m_ee->Write();
    outFile.Close();

    // PLOT on CANVAS
    TCanvas *c1 = new TCanvas("c1","", 1920, 1080);
    gStyle->SetOptStat(0);
    hist_m_ee->GetXaxis()->SetTitle("m_ee (GeV)");
    hist_m_ee->GetYaxis()->SetTitle("(Counts / GeV)");
    hist_m_ee->Draw();

    // SAVE CANVAS
    c1->SaveAs("background_plot.png");

    // Clean up
    delete treeReader;
}
