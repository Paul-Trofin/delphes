//////////////////////////////////////////////////////////////////////
// THIS FILE IS USED TO CALCULATE THE INVARIANT MASS OF e- e+ PAIRS //
///////////// READS INPUT FROM DELPHES-ROOT FILE /////////////////////
//////////////////////////////////////////////////////////////////////
// RUN LIKE THIS:
// root -l inv_m_ee.C'("pp_z_ee.root")'
//////////////////////////////////////////////////////////////////////

#ifdef __CLING__
R__LOAD_LIBRARY(/home/paul/delphes/libDelphes.so)
gInterpreter->AddIncludePath("/home/paul/delhes/external/ExRootAnalysis/");
gInterpreter->AddIncludePath("/home/paul/delphes/classes/");
#endif

#include "ExRootTreeReader.h"
#include "DelphesClasses.h"

void inv_m_ee(const char *inputFile) {
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
    TH1 *hist_m_ee = new TH1D("hist_m_ee", "e- e+ Invariant Mass", 200, 66, 116);

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
                if (p4.Pt() > 27 && p4.Eta() < 2.47) {
                    electrons.push_back(p4);
                }
                    
            }

            if (pid == -11) { // positron
                TLorentzVector p4 = particle->P4();
                if (p4.Pt() > 27 && p4.Eta() < 2.47) {
                    positrons.push_back(p4);
                }
            }
        }

        // Invariant mass calculation between electrons and positrons
        for (auto &e : electrons) {
            for (auto &e_bar : positrons) {
                double m_ee = (e + e_bar).M();
                hist_m_ee->Fill(m_ee);
            }
        }
    }

    // Save histogram to file
    TFile outFile("invariant_mass.root", "RECREATE");
    hist_m_ee->Write();
    outFile.Close();

    // Plot on CANVAS

    TCanvas *c1 = new TCanvas("c1","e- e+ Invariant Mass");
    gStyle->SetOptStat(0);
    hist_m_ee->GetXaxis()->SetTitle("m_ee (GeV)");
    hist_m_ee->GetYaxis()->SetTitle("(Counts / GeV)");
    hist_m_ee->Draw();
    c1->SaveAs("m_ee_plot.png");

    // Clean up
    delete treeReader;
}
