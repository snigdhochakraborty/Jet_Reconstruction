////////////////////////////////////////
// Author: Steven Schramm
// Last update: March 10, 2020
////////////////////////////////////////

// Compile with (for example, update to point to your files and FastJet directory):
// g++ jetRecoGroom.cpp -o jetRecoGroom `~/FastJet/fastjet-install/bin/fastjet-config --cxxflags --libs --plugins` `root-config --cflags --libs` -lRecursiveTools -lEnergyCorrelator -lNsubjettiness


#include <iostream>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TLorentzVector.h"
#include "TVector2.h"


// Step 1: event-level information
// (no fastjet headers are needed)

// Step 2: Existing jets and the event weight
// (no fastjet headers are needed)

// Step 3: Building our own R=1.0 jets from topoclusters
// TODO: add headers here (jet reconstruction and trimming)
#include "fastjet/ClusterSequence.hh"
#include "fastjet/tools/Filter.hh"

// Step 4: Building other types of R=1.0 jets from topoclusters
// TODO: add headers here (Pruning, SoftDrop, Recursive SoftDrop, and Bottom-Up SoftDrop)

// Step 5: Calculating substructure variables for R=1.0 jets
// TODO: add headers here (Energy correlators and N subjettiness)

int main (int argc, char* argv[])
{
    // Check arguments
    if (argc < 5)
    {
        printf("USAGE: %s <output file> <step number> <tree name> <input file>\n",argv[0]);
        printf("Valid step number options:\n");
        printf("\t0 = all steps\n");
        printf("\t1 = only step 1  (event-level information)\n");
        printf("\t2 = up to step 2 (existing jets and the event weight)\n");
        printf("\t3 = up to step 3 (building our own R=1.0 jets from topoclusters)\n");
        printf("\t4 = up to step 4 (building other types of R=1.0 jets from topoclusters)\n");
        printf("\t5 = up to step 5 (calculating substructure variables for R=1.0 jets)\n");
        return 1;
    }

    // Parse the arguments
    const std::string outFileName = argv[1];
    const int stepNum             = atol(argv[2]);
    const std::string inTreeName  = argv[3];
    const std::string inFileName  = argv[4];
    if (stepNum < 0 || stepNum > 5)
    {
        printf("Invalid step number: %d\n",stepNum);
        return 1;
    }

    // Open the input file and get the tree
    TFile* inFile = TFile::Open(inFileName.c_str(),"READ");
    if (!inFile || inFile->IsZombie())
    {
        printf("Failed to open the input file: %s\n",inFileName.c_str());
        return 1;
    }
    TTree* inTree = dynamic_cast<TTree*>(inFile->Get(inTreeName.c_str()));
    if (!inTree)
    {
        printf("Failed to retrieve the input tree: %s\n",inTreeName.c_str());
        return 1;
    }

    ////////////////////////////////////////////////////////////
    // Specify the input branches that we want to read        //
    ////////////////////////////////////////////////////////////
    const bool isTruth = false;
    const std::string jetTypeString   = !isTruth ? "RecoJets" : "TruthJets";
    const std::string inputTypeString = !isTruth  ? "Clusters" : "Particles";

    // Step 1: event-level information
    float mu_average = 0;
    unsigned NPV     = 0;
    if (!stepNum || stepNum >= 1)
    {
        inTree->SetBranchStatus("*",0);
        inTree->SetBranchStatus("mu_average",1);
        inTree->SetBranchStatus("NPV",1);
        inTree->SetBranchAddress("mu_average",&mu_average);
        inTree->SetBranchAddress("NPV",&NPV);
    }

    // Step 2: Existing jets and the event weight
    float EventWeight = 0;
    std::vector<float>* jet_R10_ungroom_pt = nullptr;
    std::vector<float>* jet_R10_ungroom_m  = nullptr;
    std::vector<float>* jet_R10_trimmed_pt = nullptr;
    std::vector<float>* jet_R10_trimmed_m  = nullptr;
    if (!stepNum || stepNum >= 2)
    {
        inTree->SetBranchStatus("EventWeight",1);
        inTree->SetBranchStatus((jetTypeString+"_R10_pt").c_str(),1);
        inTree->SetBranchStatus((jetTypeString+"_R10_m").c_str(),1);
        inTree->SetBranchStatus((jetTypeString+"_R10_Trimmed_pt").c_str(),1);
        inTree->SetBranchStatus((jetTypeString+"_R10_Trimmed_m").c_str(),1);

        inTree->SetBranchAddress("EventWeight",&EventWeight);
        inTree->SetBranchAddress((jetTypeString+"_R10_pt").c_str(),&jet_R10_ungroom_pt);
        inTree->SetBranchAddress((jetTypeString+"_R10_m").c_str(), &jet_R10_ungroom_m);
        inTree->SetBranchAddress((jetTypeString+"_R10_Trimmed_pt").c_str(),&jet_R10_trimmed_pt);
        inTree->SetBranchAddress((jetTypeString+"_R10_Trimmed_m").c_str(), &jet_R10_trimmed_m);
    }

    // Step 3: Building our own R=1.0 jets from topoclusters
    std::vector<float>* cluster_pt  = nullptr;
    std::vector<float>* cluster_eta = nullptr;
    std::vector<float>* cluster_phi = nullptr;
    std::vector<float>* cluster_m   = nullptr;
    if (!stepNum || stepNum >= 3)
    {
        inTree->SetBranchStatus((inputTypeString+"_pt").c_str(),1);
        inTree->SetBranchStatus((inputTypeString+"_eta").c_str(),1);
        inTree->SetBranchStatus((inputTypeString+"_phi").c_str(),1);
        inTree->SetBranchStatus((inputTypeString+"_m").c_str(),1);

        inTree->SetBranchAddress((inputTypeString+"_pt").c_str(), &cluster_pt);
        inTree->SetBranchAddress((inputTypeString+"_eta").c_str(),&cluster_eta);
        inTree->SetBranchAddress((inputTypeString+"_phi").c_str(),&cluster_phi);
        inTree->SetBranchAddress((inputTypeString+"_m").c_str(),  &cluster_m);
    }

    // Step 4: Building other types of R=1.0 jets from topoclusters
    // (no new branches need to be added)

    // Step 5: Calculating substructure variables for R=1.0 jets
    // (no new branches need to be added)


    ////////////////////////////////////////////////////////////
    // Prepare the output file and histograms                 //
    ////////////////////////////////////////////////////////////

    // Step 1: event-level information
    TFile* outFile = TFile::Open(outFileName.c_str(),"RECREATE");
    TH1I hist_mu("Step1_mu","#mu_{average}",100,0,100);
    TH1I hist_npv("Step1_npv","NPV",50,0,50);

    // Step 2: Existing jets and the event weight
    TH1F hist_ungroom_pt_nw("Step2_UngroomPt_noweight","Leading ungroomed R=1.0 jet p_{T}, no weights",215,50.e3,2200.e3);
    TH1F hist_ungroom_pt("Step2_UngroomPt","Leading ungroomed R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_trimmed_pt("Step2_TrimmedPt","Leading trimmed R=1.0 jet p_{T}",215,50.e3,2200.e3);

    TH1F hist_ungroom_m("Step2_UngroomMass","Leading ungroomed R=1.0 jet mass",99,10.e3,1000.e3);
    TH1F hist_trimmed_m("Step2_TrimmedMass","Leading trimmed R=1.0 jet mass",99,10.e3,1000.e3);

    // Step 3: Building our own R=1.0 jets from topoclusters
    TH1F hist_myungroom_pt_nw("Step3_MyUngroomPt_noweight","My leading ungroomed R=1.0 jet p_{T}, no weights",215,50.e3,2200.e3);
    TH1F hist_myungroom_pt("Step3_MyUngroomPt","My leading ungroomed R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_mytrimmed_pt_nw("Step3_MyTrimmedPt_noweight","My leading trimmed R=1.0 jet p_{T}, no weights",215,50.e3,2200.e3);
    TH1F hist_mytrimmed_pt("Step3_MyTrimmedPt","My leading trimmed R=1.0 jet p_{T}",215,50.e3,2200.e3);

    // Step 4: Building other types of R=1.0 jets from topoclusters
    TH1F hist_mypruned_pt("Step4_MyPrunedPt","My leading pruned R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_mypruned_m("Step4_MyPrunedMass","My leading pruned R=1.0 jet mass",215,10.e3,1000.e3);

    TH1F hist_mySD_pt("Step4_MySDPt","My leading SD R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_mySD_m("Step4_MySDMass","My leading SD R=1.0 jet mass",99,10.e3,1000.e3);
    
    TH1F hist_myRSD_pt("Step4_MyRSDPt","My leading RSD R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_myRSD_m("Step4_MyRSDMass","My leading RSD R=1.0 jet mass",99,10.e3,1000.e3);

    TH1F hist_myBUSD_pt("Step4_MyBUSDPt","My leading BUSD R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_myBUSD_m("Step4_MyBUSDMass","My leading BUSD R=1.0 jet mass",99,10.e3,1000.e3);

    TH1F hist_myBUSDT_pt("Step4_MyBUSDTPt","My leading tight BUSD R=1.0 jet p_{T}",215,50.e3,2200.e3);
    TH1F hist_myBUSDT_m("Step4_MyBUSDTMass","My leading tight BUSD R=1.0 jet mass",99,10.e3,1000.e3);

    // Step 5: Calculating substructure variables for R=1.0 jets
    TH1F hist_ungroom_D2(   "Step5_Ungroomed_D2",   "Ungroomed R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_ungroom_tau32("Step5_Ungroomed_Tau32","Ungroomed R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_trimmed_D2(   "Step5_Trimmed_D2",   "Trimmed R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_trimmed_tau32("Step5_Trimmed_Tau32","Trimmed R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_pruned_D2(   "Step5_Pruned_D2",   "Pruned R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_pruned_tau32("Step5_Pruned_Tau32","Pruned R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_SD_D2(   "Step5_SD_D2",   "SD R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_SD_tau32("Step5_SD_Tau32","SD R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_RSD_D2(   "Step5_RSD_D2",   "RSD R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_RSD_tau32("Step5_RSD_Tau32","RSD R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_BUSD_D2(   "Step5_BUSD_D2",   "BUSD R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_BUSD_tau32("Step5_BUSD_Tau32","BUSD R=1.0 jet #tau_{32}^{WTA}",20,0,1);

    TH1F hist_BUSDT_D2(   "Step5_BUSDT_D2",   "Tight BUSD R=1.0 jet D_{2}^{#beta=1}",20,0,5);
    TH1F hist_BUSDT_tau32("Step5_BUSDT_Tau32","Tight BUSD R=1.0 jet #tau_{32}^{WTA}",20,0,1);


    ////////////////////////////////////////////////////////////
    // Specify the fastjet tools we need to make use of       //
    ////////////////////////////////////////////////////////////

    // Step 1: event-level information
    // (no fastjet tools are needed)

    // Step 2: Existing jets and the event weight
    // (no fastjet tools are needed)

    // Step 3: Building our own R=1.0 jets from topoclusters
    // TODO: add tools here (jet reconstruction and trimming)
    fastjet::JetDefinition akt10(fastjet::antikt_algorithm,1.0);
    fastjet::Transformer *trimmer = new fastjet::Filter(fastjet::JetDefinition(fastjet::kt_algorithm, 0.2), fastjet::SelectorPtFractionMin(0.05) );
    
    
    // Step 4: Building other types of R=1.0 jets from topoclusters
    // TODO: add tools here (Pruning, SoftDrop, Recursive SoftDrop, and Bottom-Up SoftDrop)
    
    // Step 5: Calculating substructure variables for R=1.0 jets
    // TODO: add tools here (Energy correlators and N subjettiness)
    

    ////////////////////////////////////////////////////////////
    // Run over the events in the file and reconstruct jets   //
    ////////////////////////////////////////////////////////////

    const long long int numEvents = inTree->GetEntries();
    for (long long iEvent = 0; iEvent < numEvents; ++iEvent)
    {
        // Print out the even number every 10k events and then load the event
        if (iEvent%10000 == 0)
            printf("Processing event %lld/%lld\n",iEvent,numEvents);
        inTree->GetEntry(iEvent);



        // Step 1: event-level information
        // Histograms to fill:
        //  hist_mu:  mu distribution
        //  hist_npv: npv distribution
        if (!stepNum || stepNum >= 1)
        {
		hist_mu.Fill(mu_average);
		hist_npv.Fill(NPV);
        }



        // Step 2: Existing jets and the event weight
        // Histograms to fill:
        //  hist_ungroom_pt_nw: Leading ungroomed R=1.0 jet pT, without the event weight
        //  hist_ungroom_pt:    Leading ungroomed R=1.0 jet pT, with the event weight
        //  hist_trimmed_pt:    Leading trimmed R=1.0 jet pT, with the event weight
        //  hist_ungroom_m:     Leading ungroomed R=1.0 jet mass, with the event weight
        //  hist_trimmed_m:     Leading trimmed R=1.0 jet mass, with the event weight
        if (!stepNum || stepNum >= 2)
        {
           if (jet_R10_ungroom_pt->size())
            {
                hist_ungroom_pt_nw.Fill(jet_R10_ungroom_pt->at(0));
                hist_ungroom_pt.Fill(jet_R10_ungroom_pt->at(0),EventWeight);
                hist_trimmed_pt.Fill(jet_R10_trimmed_pt->at(0),EventWeight);
                
                if (jet_R10_ungroom_pt->at(0) > 400.e3)
                    hist_ungroom_m.Fill(jet_R10_ungroom_m->at(0),EventWeight);
                if (jet_R10_trimmed_pt->at(0) > 400.e3)
                    hist_trimmed_m.Fill(jet_R10_trimmed_m->at(0),EventWeight);
            }
        }

        // Step 3: Building our own R=1.0 jets from topoclusters
        // Histograms to fill:
        //  hist_myungroom_pt_nw: Leading rebuilt ungroomed R=1.0 jet pT, without the event weight
        //  hist_myungroom_pt:    Leading rebuilt ungroomed R=1.0 jet pT, with the event weight
        //  hist_mytrimmed_pt_nw: Leading rebuilt trimmed R=1.0 jet pT, without the event weight
        //  hist_mytrimmed_pt:    Leading rebuilt trimmed R=1.0 jet pT, with the event weight
        if (!stepNum || stepNum >= 3)
        {
            // TODO convert the input objects (clusters) to pseudojets, then use them to build R=1.0 ungroomed and trimmed jets, for comparison to the pre-built jets in step 2
            // TODO convert the input objects (clusters) to pseudojets, then use them to build R=1.0 ungroomed and trimmed jets, for comparison to the pre-built jets in step 2
            // Convert the clusters into FastJet's four-vector (PseudoJet)
            std::vector<fastjet::PseudoJet> clusters;
            clusters.reserve(cluster_pt->size());
            for (size_t iClus = 0; iClus < cluster_pt->size(); ++iClus)
            {
                TLorentzVector cluster;
                cluster.SetPtEtaPhiM(cluster_pt->at(iClus),cluster_eta->at(iClus),cluster_phi->at(iClus),cluster_m->at(iClus));
                clusters.push_back(fastjet::PseudoJet(cluster.Px(),cluster.Py(),cluster.Pz(),cluster.E()));
		printf(cluster_pt->at(iClus));
            }

            // Use fastjet to build new jets
            fastjet::ClusterSequence cs_a10_clusters(clusters,akt10);
            std::vector<fastjet::PseudoJet> jets_a10_clusters = fastjet::sorted_by_pt(cs_a10_clusters.inclusive_jets());
            
            // Use these jets and compare to the original jets
            if (jets_a10_clusters.size())
            {
                // Trim the jet
                const fastjet::PseudoJet& ungroomed = jets_a10_clusters.at(0);
                fastjet::PseudoJet trimmed = (*trimmer)(ungroomed);
		
                // Jet pT distribution
                hist_myungroom_pt_nw.Fill(ungroomed.pt());
                hist_myungroom_pt.Fill(ungroomed.pt(),EventWeight);
                hist_mytrimmed_pt_nw.Fill(trimmed.pt());
                hist_mytrimmed_pt.Fill(trimmed.pt(),EventWeight);
            }


            // Step 4: Building other types of R=1.0 jets from topoclusters
            // Histograms to fill:
            //  hist_mypruned_pt: Leading Pruned R=1.0 jet pT, with the event weight
            //  hist_mypruned_m:  Leading Pruned R=1.0 jet mass, with the event weight
            //  hist_mySD_pt:     Leading SoftDrop R=1.0 jet pT, with the event weight
            //  hist_mySD_m:      Leading SoftDrop R=1.0 jet pT, with the event weight
            //  hist_myRSD_pt:    Leading Recursive SoftDrop R=1.0 jet pT, with the event weight
            //  hist_myRSD_m:     Leading Recursive SoftDrop R=1.0 jet mass, with the event weight
            //  hist_myBUSD_pt:   Leading Bottom-Up SoftDrop R=1.0 jet pT, with the event weight
            //  hist_myBUSD_m:    Leading Bottom-Up SoftDrop R=1.0 jet mass, with the event weight
            //  hist_myBUSDT_pt:  Leading Tighter-variant of Bottom-Up SoftDrop R=1.0 jet pT, with the event weight
            //  hist_myBUSDT_m:   Leading Tighter-variant of Bottom-Up SoftDrop R=1.0 jet mass, with the event weight
            if (!stepNum || stepNum >= 4)
            {
                // TODO groom the rebuilt ungroomed R=1.0 jets in a variety of ways and store the pT and mass
                // Only fill the mass histograms when jet pT > 400 GeV



                // Step 5: Calculating substructure variables for R=1.0 jets
                // Histograms to fill:
                //  hist_ungroom_D2:    Leading rebuilt ungroomed R=1.0 jet D2, with the event weight
                //  hist_ungroom_tau32: Leading rebuilt ungroomed R=1.0 jet tau32, with the event weight
                //  hist_trimmed_D2:    Leading rebuilt trimmed R=1.0 jet D2, with the event weight
                //  hist_trimmed_tau32: Leading rebuilt trimmed R=1.0 jet tau32, with the event weight
                //  hist_pruned_D2:     Leading Pruned R=1.0 jet D2, with the event weight
                //  hist_pruned_tau32:  Leading Pruned R=1.0 jet tau32, with the event weight
                //  hist_mySD_D2:       Leading SoftDrop R=1.0 jet D2, with the event weight
                //  hist_mySD_tau32:    Leading SoftDrop R=1.0 jet tau32, with the event weight
                //  hist_myRSD_D2:      Leading Recursive SoftDrop R=1.0 jet D2, with the event weight
                //  hist_myRSD_tau32:   Leading Recursive SoftDrop R=1.0 jet tau32, with the event weight
                //  hist_myBUSD_D2:     Leading Bottom-Up SoftDrop R=1.0 jet D2, with the event weight
                //  hist_myBUSD_tau32:  Leading Bottom-Up SoftDrop R=1.0 jet tau32, with the event weight
                //  hist_myBUSDT_D2:    Leading Tighter-variant of Bottom-Up SoftDrop R=1.0 jet D2, with the event weight
                //  hist_myBUSDT_tau32: Leading Tighter-variant of Bottom-Up SoftDrop R=1.0 jet tau32, with the event weight
                if (!stepNum || stepNum >= 5)
                {
                    // TODO calculate substructure variables for all of the jet types
                    // Only fill the histograms when jet pT > 400 GeV
                    // Recall that D2 = ECF3 * ECF1^3 / ECF2^3
                    // Recall that tau32 = tau3 / tau2
                }
            }
        }
    }
    
    ////////////////////////////////////////////////////////////
    // Save the results to the output file                    //
    ////////////////////////////////////////////////////////////

    outFile->cd();

    // Step 1: event-level information
    if (!stepNum || stepNum >= 1)
    {
        hist_mu.Write();
        hist_npv.Write();
    }

    // Step 2: Existing jets and the event weight
    if (!stepNum || stepNum >= 2)
    {
        hist_ungroom_pt_nw.Write();
        hist_ungroom_pt.Write();
        hist_trimmed_pt.Write();

        hist_ungroom_m.Write();
        hist_trimmed_m.Write();
    }

    // Step 3: Building our own R=1.0 jets from topoclusters
    if (!stepNum || stepNum >= 3)
    {
        hist_myungroom_pt_nw.Write();
        hist_myungroom_pt.Write();
        hist_mytrimmed_pt_nw.Write();
        hist_mytrimmed_pt.Write();
    }

    // Step 4: Building other types of R=1.0 jets from topoclusters
    if  (!stepNum || stepNum >= 4)
    {
        hist_mypruned_pt.Write();
        hist_mypruned_m.Write();

        hist_mySD_pt.Write();
        hist_mySD_m.Write();

        hist_myRSD_pt.Write();
        hist_myRSD_m.Write();

        hist_myBUSD_pt.Write();
        hist_myBUSD_m.Write();

        hist_myBUSDT_pt.Write();
        hist_myBUSDT_m.Write();
    }

    // Step 5: Calculating substructure variables for R=1.0 jets
    if (!stepNum || stepNum >= 5)
    {
        hist_ungroom_D2.Write();
        hist_ungroom_tau32.Write();

        hist_trimmed_D2.Write();
        hist_trimmed_tau32.Write();

        hist_pruned_D2.Write();
        hist_pruned_tau32.Write();

        hist_SD_D2.Write();
        hist_SD_tau32.Write();

        hist_RSD_D2.Write();
        hist_RSD_tau32.Write();

        hist_BUSD_D2.Write();
        hist_BUSD_tau32.Write();

        hist_BUSDT_D2.Write();
        hist_BUSDT_tau32.Write();
    }



    outFile->Close();

    return 0;
}

