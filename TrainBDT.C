///////////////////////////////////////////////////
//// LOADS SIGNAL AND BACKGROUND TREES in TMVA ////
///////// USES BDT to DISTINGUISH EVENTS //////////
///////////////////////////////////////////////////
// RUN LIKE THIS:
// root -l TrainBDT.C
///////////////////////////////////////////////////
// AFTER GENERATING TMVA_BDT.root RUN:
// root -l TMVA_BDT.root
// TMVA::TMVAGui("TMVA_BDT.root")

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"

void TrainBDT() {
    // Initialize TMVA
    TMVA::Tools::Instance();
    TFile* outFile = TFile::Open("TMVA_BDT.root", "RECREATE");

    // Create a Factory and DataLoader
    TMVA::Factory* factory = new TMVA::Factory("TMVAClassification", outFile,
        "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:"
        "AnalysisType=Classification");
    TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");

    // Add input variables (features)
    dataloader->AddVariable("m_ee", 'F');   // Invariant mass of e+e-
    dataloader->AddVariable("pT_ee", 'F');  // Transverse momentum of e+e-
    dataloader->AddVariable("E_ee", 'F');   // Energy of e+e-

    // Load Signal and Background Trees
    TFile* signalFile = TFile::Open("signal.root");
    TTree* signalTree = (TTree*)signalFile->Get("tree");
    
    TFile* bkgFile_1 = TFile::Open("bkg_1.root");
    TTree* bkgTree_1 = (TTree*)bkgFile_1->Get("tree");
    
    TFile* bkgFile_2 = TFile::Open("bkg_2.root");
    TTree* bkgTree_2 = (TTree*)bkgFile_2->Get("tree");

    // Add trees to DataLoader with weights (1.0 initially)
    dataloader->AddSignalTree(signalTree, 1.0);
    dataloader->AddBackgroundTree(bkgTree_1, 1.0);
    dataloader->AddBackgroundTree(bkgTree_2, 1.0);

    // Define preselection cuts (optional)
    /*TCut Cuts = "66 <= m_ee && m_ee <= 116 &&"
                 "66 <= pT_ee && pT_ee <= 116 &&"
                 "66 <= E_ee && E_ee <= 116";*/
    TCut Cuts = "";

    // Prepare training and testing samples
    dataloader->PrepareTrainingAndTestTree(Cuts, "nTrain_Signal=16000:nTrain_Background=32000:SplitMode=Random:NormMode=NumEvents:!V");

    // Book the BDT method with tuned parameters
    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT",
        "!H:!V:NTrees=1000:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:"
        "AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:"
        "SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning");

    // Train, test, and evaluate all methods
    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    // Cleanup
    outFile->Close();
    delete factory;
    delete dataloader;

    std::cout << "=== Training, testing, and evaluation complete! ===" << std::endl;
}
