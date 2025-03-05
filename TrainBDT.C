///////////////////////////////////////////////////
//// LOADS SIGNAL AND BACKGROUND TREES in TMVA ////
///////// USES BDT to DISTINGUISH EVENTS //////////
///////////////////////////////////////////////////
// RUN LIKE THIS:       / first = signal / rest background
// root -l TrainBDT.C'("ff_z_ee/variables.root", "ff_gm_ee/variables.root", "ff_W_ee/variables.root")'
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

void TrainBDT(const char* signalFileName, const char* bkgFile1Name, const char* bkgFile2Name) {
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
    dataloader->AddVariable("phi_ee", 'F');   // Phi of e+e-
    dataloader->AddVariable("eta_ee", 'F');   // Eta of e+e-

    // Load Signal and Background Trees using passed filenames
    TFile* signalFile = TFile::Open(signalFileName);
    if (!signalFile) {
        std::cerr << "** ERROR: Could not open file " << signalFileName << std::endl;
        return;
    }
    TTree* signalTree = (TTree*)signalFile->Get("ElectronPairs");
    if (!signalTree) {
        std::cerr << "** ERROR: Could not find tree 'ElectronPairs' in file " << signalFileName << std::endl;
        return;
    }

    TFile* bkgFile1 = TFile::Open(bkgFile1Name);
    if (!bkgFile1) {
        std::cerr << "** ERROR: Could not open file " << bkgFile1Name << std::endl;
        return;
    }
    TTree* bkgTree1 = (TTree*)bkgFile1->Get("ElectronPairs");
    if (!bkgTree1) {
        std::cerr << "** ERROR: Could not find tree 'ElectronPairs' in file " << bkgFile1Name << std::endl;
        return;
    }

    TFile* bkgFile2 = TFile::Open(bkgFile2Name);
    if (!bkgFile2) {
        std::cerr << "** ERROR: Could not open file " << bkgFile2Name << std::endl;
        return;
    }
    TTree* bkgTree2 = (TTree*)bkgFile2->Get("ElectronPairs");
    if (!bkgTree2) {
        std::cerr << "** ERROR: Could not find tree 'ElectronPairs' in file " << bkgFile2Name << std::endl;
        return;
    }

    // Add trees to DataLoader with weights (1.0 initially)
    dataloader->AddSignalTree(signalTree, 1.0);
    dataloader->AddBackgroundTree(bkgTree1, 1.0);
    dataloader->AddBackgroundTree(bkgTree2, 1.0);

    // Define preselection cuts (optional)
    TCut Cuts = "";

    // Prepare training and testing samples
    dataloader->PrepareTrainingAndTestTree(Cuts, "nTrain_Signal=1000:nTrain_Background=2000:SplitMode=Random:NormMode=NumEvents:!V");

    // Book the BDT method with tuned parameters
    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT",
        "!H:!V:NTrees=5000:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:"
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
