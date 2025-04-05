// RUN LIKE THIS:
// root -l Scale_Low_Entries.C'("variables_low.root")'

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TObjArray.h>
#include <TBranch.h>
#include <iostream>

void Scale_Low_Entries(const char* input_file_name) {
    // Target number of entries
    int N_target = 70000;

    // Open the input ROOT file
    TFile* file = TFile::Open(input_file_name, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open the file " << input_file_name << std::endl;
        return;
    }

    // Get the TTree from the file
    TTree* tree = (TTree*)file->Get("Dielectrons");
    if (!tree) {
        std::cerr << "Error: Tree 'Dielectrons' not found in the file!" << std::endl;
        file->Close();
        return;
    }

    // Get the number of entries in the current Tree
    int N_current = tree->GetEntries();
    std::cout << "Current number of entries: " << N_current << std::endl;

    // Calculate the scaling factor (for informational purposes)
    double scale_factor = (double)N_target / N_current;
    std::cout << "Scaling factor: " << scale_factor << std::endl;

    // Create a new ROOT file for the output
    TFile* output_file = new TFile("variables.root", "RECREATE");

    // Clone the tree structure (same branches), but don't copy the entries yet
    TTree* new_tree = tree->CloneTree(0); // 0 means no entries are copied initially

    // Loop through the original tree entries and fill the new tree
    int N_added = 0;
    while (N_added < N_target) {
        for (int i = 0; i < N_current; ++i) {
            tree->GetEntry(i);  // Get the entry

            // Now fill the new tree with the current entry's data
            new_tree->Fill();

            ++N_added;
            if (N_added >= N_target) break;  // Stop once we've reached N_target
        }
    }

    // Write the new tree to the output file
    new_tree->Write();

    // Close the files
    output_file->Close();
    file->Close();

    std::cout << "Tree has been scaled and written to 'variables.root'" << std::endl;
}
