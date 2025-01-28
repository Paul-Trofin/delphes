///////////////////////////////////////////////////
//// LOADS SIGNAL AND BACKGROUND TREES in TMVA ////
///////// USES BDT to DISTINGUISH EVENTS //////////
///////////////////////////////////////////////////

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
    dataloader->AddVariable("invariant_mass", 'F');

    // LOAD Signal and Background Trees
    TFile* treesFile = TFile::Open("signal_background_trees.root");
    TTree* signalTree = (TTree*)treesFile->Get("SignalTree");
    TTree* backgroundTree = (TTree*)treesFile->Get("BackgroundTree");

    // Add trees to DataLoader
    dataloader->AddSignalTree(signalTree, 1.0);       // start signal weight = 1
    dataloader->AddBackgroundTree(backgroundTree, 1.0); // start back weight = 1

    // Prepare training and test samples
    dataloader->PrepareTrainingAndTestTree("", "");

    // Book the BDT method
    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT");

    // Train, test, and evaluate
    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    // SAVE results
    outFile->Close();
    delete factory;
    delete dataloader;
}

