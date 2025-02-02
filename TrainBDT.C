///////////////////////////////////////////////////
//// LOADS SIGNAL AND BACKGROUND TREES in TMVA ////
///////// USES BDT to DISTINGUISH EVENTS //////////
///////////////////////////////////////////////////
// RUN LIKE THIS:
// root TrainBDT.C
///////////////////////////////////////////////////
// AFTER GENERATING TMVA_BDT.root RUN:
// root -l TMVA_BDT.root
// TMVA::TMVAGui("TMVA_BDT.root")

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"

void TrainBDT() {
    // Initialize TMVA
    TMVA::Tools::Instance();
    TFile* outFile = TFile::Open("TMVA_BDT.root", "RECREATE");

    // Create a Factory and DataLoader
    TMVA::Factory* factory = new TMVA::Factory("TMVAClassification", outFile);
    TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");

    // ADD invariant_mass as a variable
    dataloader->AddVariable("m_ee", 'F');

    // LOAD Signal and Background Trees
    TFile* signalFile = TFile::Open("ff_z_ee/ff_z_ee_m_ee.root");
    TTree* signalTree = (TTree*)signalFile->Get("tree_m_ee");
    
    TFile* bkgFile_1 = TFile::Open("ff_zz_4e/ff_zz_4e_m_ee.root");
    TTree* bkgTree_1 = (TTree*)bkgFile_1->Get("tree_m_ee");
    
    TFile* bkgFile_2 = TFile::Open("qq_gz/qq_gz_m_ee.root");
    TTree* bkgTree_2 = (TTree*)bkgFile_2->Get("tree_m_ee");
    
    TFile* bkgFile_3 = TFile::Open("qg_qz/qg_qz_m_ee.root");
    TTree* bkgTree_3 = (TTree*)bkgFile_3->Get("tree_m_ee");
    
    // Add trees to DataLoader
    dataloader->AddSignalTree(signalTree, 1.0);       // start signal weight = 1
    dataloader->AddBackgroundTree(bkgTree_1, 1.0); // start back weight = 1
    dataloader->AddBackgroundTree(bkgTree_2, 1.0); // start back weight = 1
    dataloader->AddBackgroundTree(bkgTree_3, 1.0); // start back weight = 1

    // Prepare training and test samples
    dataloader->PrepareTrainingAndTestTree("", "");

    // Book the BDT method
    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT");

    // Train, test, and evaluate
    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    // SAVE results
    signalFile->Close();
    bkgFile_1->Close();
    bkgFile_2->Close();
    bkgFile_3->Close();
    delete factory;
    delete dataloader;
}
