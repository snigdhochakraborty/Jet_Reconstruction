////////////////////////////////////////
// Author: Steven Schramm
// Last update: March 8, 2020
////////////////////////////////////////

// Compile with (for example, update to point to your files)
// g++ jetRecoExp.cpp -o jetRecoExp `root-config --cflags --libs`


#include <iostream>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2I.h"
#include "TH2F.h"
#include "TProfile2D.h"
#include "TLorentzVector.h"
#include "TVector2.h"

int main (int argc, char* argv[])
{
    // Check arguments
    if (argc < 5)
    {
        printf("USAGE: %s <output file> <step number> <tree name> <input file>\n",argv[0]);
        printf("Valid step number options:\n");
        printf("\t0 = all steps\n");
        printf("\t1 = only step 1  (event-level information)\n");
        printf("\t2 = up to step 2 (cluster and truth jets and the event weight)\n");
        printf("\t3 = up to step 3 (pileup dependence)\n");
        printf("\t4 = up to step 4 (tracks and track jets)\n");
        printf("\t5 = up to step 5 (jet response studies)\n");
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

    // Step 2: R=0.4 cluster and truth jets and the event weight
    float EventWeight = 0;
    std::vector<float>* RecoJet_pt   = nullptr;
    std::vector<float>* RecoJet_eta  = nullptr;
    std::vector<float>* RecoJet_phi  = nullptr;
    std::vector<float>* RecoJet_m    = nullptr;
    std::vector<float>* TruthJet_pt  = nullptr;
    std::vector<float>* TruthJet_eta = nullptr;
    std::vector<float>* TruthJet_phi = nullptr;
    std::vector<float>* TruthJet_m   = nullptr;
    if (!stepNum || stepNum >= 2)
    {
        inTree->SetBranchStatus("EventWeight",    1);
        inTree->SetBranchStatus("RecoJets_R4_pt", 1);
        inTree->SetBranchStatus("RecoJets_R4_eta",1);
        inTree->SetBranchStatus("RecoJets_R4_phi",1);
        inTree->SetBranchStatus("RecoJets_R4_m",  1);
        inTree->SetBranchStatus("TruthJets_R4_pt", 1);
        inTree->SetBranchStatus("TruthJets_R4_eta",1);
        inTree->SetBranchStatus("TruthJets_R4_phi",1);
        inTree->SetBranchStatus("TruthJets_R4_m",  1);

        inTree->SetBranchAddress("EventWeight",    &EventWeight);
        inTree->SetBranchAddress("RecoJets_R4_pt", &RecoJet_pt);
        inTree->SetBranchAddress("RecoJets_R4_eta",&RecoJet_eta);
        inTree->SetBranchAddress("RecoJets_R4_phi",&RecoJet_phi);
        inTree->SetBranchAddress("RecoJets_R4_m",  &RecoJet_m);
        inTree->SetBranchAddress("TruthJets_R4_pt", &TruthJet_pt);
        inTree->SetBranchAddress("TruthJets_R4_eta",&TruthJet_eta);
        inTree->SetBranchAddress("TruthJets_R4_phi",&TruthJet_phi);
        inTree->SetBranchAddress("TruthJets_R4_m",  &TruthJet_m);
    }

    // Step 3: Pileup dependence
    // (no new branches need to be added)

    // Step 4: Tracks and R=0.4 track jets 
    std::vector<float>* RecoJet_jvf  = nullptr;
    std::vector<float>* TrackJet_pt  = nullptr;
    std::vector<float>* TrackJet_eta = nullptr;
    std::vector<float>* TrackJet_phi = nullptr;
    std::vector<float>* TrackJet_m   = nullptr;
    if (!stepNum || stepNum >= 4)
    {
        inTree->SetBranchStatus("RecoJets_R4_jvf", 1);
        inTree->SetBranchStatus("TrackJets_R4_pt", 1);
        inTree->SetBranchStatus("TrackJets_R4_eta",1);
        inTree->SetBranchStatus("TrackJets_R4_phi",1);
        inTree->SetBranchStatus("TrackJets_R4_m",  1);

        inTree->SetBranchAddress("RecoJets_R4_jvf", &RecoJet_jvf);
        inTree->SetBranchAddress("TrackJets_R4_pt", &TrackJet_pt);
        inTree->SetBranchAddress("TrackJets_R4_eta",&TrackJet_eta);
        inTree->SetBranchAddress("TrackJets_R4_phi",&TrackJet_phi);
        inTree->SetBranchAddress("TrackJets_R4_m",  &TrackJet_m);
    }

    // Step 5: Jet response studies
    // (no new branches need to be added)

    
    ////////////////////////////////////////////////////////////
    // Prepare the output file and histograms                 //
    ////////////////////////////////////////////////////////////

    // Step 1: event-level information
    TFile* outFile = TFile::Open(outFileName.c_str(),"RECREATE");
    TH1I hist_mu("Step1_mu","#mu_{average}",90,0,90);
    TH1I hist_npv("Step1_npv","NPV",60,0,60);
    TH2I hist_mu_npv("Step1_mu_npv","Correlation between #mu_{average} and NPV",90,0,90,60,0,60);

    // Step 2: R=0.4 cluster and truth jets and the event weight
    TH1F hist_reco_pt_nw("Step2_RecoJet_pt_noweight","Leading R=0.4 cluster jet p_{T}, no weights",199,10.e3,2000.e3);
    TH1F hist_reco_pt("Step2_RecoJet_pt","Leading R=0.4 cluster jet p_{T}",199,10.e3,2000.e3);
    
    TH1F hist_truth_pt_nw("Step2_TruthJet_pt_noweight","Leading R=0.4 truth jet p_{T}",199,10.e3,2000.e3);
    TH1F hist_truth_pt("Step2_TruthJet_pt","Leading R=0.4 truth jet p_{T}",199,10.e3,2000.e3);

    // Step 3: Pileup dependence
    TH1F hist_reco_njets_lowmu("Step3_RecoJet_njets_lowmu","Number of cluster jets above 20 GeV, #mu_{average} < 30",15,0,30);
    TH1F hist_reco_njets_midmu("Step3_RecoJet_njets_midmu","Number of cluster jets above 20 GeV, 35 < #mu_{average} < 45",15,0,30);
    TH1F hist_reco_njets_highmu("Step3_RecoJet_njets_highmu","Number of cluster jets above 20 GeV, #mu_{average} > 50",15,0,30);

    TH1F hist_truth_njets_lowmu("Step3_TruthJet_njets_lowmu","Number of truth jets above 20 GeV, #mu_{average} < 30",15,0,30);
    TH1F hist_truth_njets_midmu("Step3_TruthJet_njets_midmu","Number of truth jets above 20 GeV, 35 < #mu_{average} < 45",15,0,30);
    TH1F hist_truth_njets_highmu("Step3_TruthJet_njets_highmu","Number of truth jets above 20 GeV, #mu_{average} > 50",15,0,30);
    
    TProfile2D hist_reco_njets_mu_npv("Step3_RecoJets_njets_2D","Average number of cluster jets above 20 GeV, vs #mu_{average} and NPV",90,0,90,60,0,60);
    TProfile2D hist_truth_njets_mu_npv("Step3_TruthJets_njets_2D","Average number of truth jets above 20 GeV, vs #mu_{average} and NPV",90,0,90,60,0,60);
    

    // Step 4: Tracks and R=0.4 track jets 
    TH1F hist_reco_jvf_pt20("Step4_RecoJet_jvf_pt20","Leading R=0.4 jet JVF, p_{T} > 20 GeV",44,-1.1,1.1);
    TH1F hist_reco_jvf_pt60("Step4_RecoJet_jvf_pt60","Leading R=0.4 jet JVF, p_{T} > 60 GeV", 44,-1.1,1.1);
    TH1F hist_reco_jvf_pt100("Step4_RecoJet_jvf_pt100","Leading R=0.4 jet JVF, p_{T} > 100 GeV",  44,-1.1,1.1);
    TH1F hist_reco_pt_jvf("Step4_RecoJet_pt_jvf","Leading R=0.4 cluster jet p_{T} after |JVF|>0.5",199,10.e3,2000.e3);
    
    TH1F hist_track_pt("Step4_TrackJet_pt","Leading R=0.4 track jet p_{T}",199,10.e3,2000.e3);
    TH1F hist_track_njets_lowmu("Step4_TrackJet_njets_lowmu","Number of track jets above 20 GeV, #mu_{average} < 30",15,0,30);
    TH1F hist_track_njets_midmu("Step4_TrackJet_njets_midmu","Number of track jets above 20 GeV, 35 < #mu_{average} < 45",15,0,30);
    TH1F hist_track_njets_highmu("Step4_TrackJet_njets_highmu","Number of track jets above 20 GeV, #mu_{average} > 50",15,0,30);
    TProfile2D hist_track_njets_mu_npv("Step4_TrackJets_njets_2D","Average number of track jets above 20 GeV, vs #mu_{average} and NPV",90,0,90,60,0,60);


    // Step 5: Jet response studies
    TH1F hist_DRtruth_reco("Step5_DRtruth_reco","DR between leading truth and reco jet",10,0,1);
    TH1F hist_DRtruth_reco_jvf("Step5_DRtruth_reco_jvf","DR between leading truth and reco jet, after |JVF| > 0.5",10,0,1);
    TH1F hist_DRtruth_track("Step5_DRtruth_track","DR between leading truth and track jet",10,0,1);

    TH1F hist_response_reco_pt20("Step5_response_reco_pt20","Cluster jet p_{T} response, p_T{}^{truth} > 20 GeV",100,0,2);
    TH1F hist_response_reco_pt100("Step5_response_reco_pt100","Cluster jet p_{T} response, p_{T}^{truth} > 100 GeV",100,0,2);
    TH1F hist_response_reco_pt1000("Step5_response_reco_pt1000","Cluster jet p_{T} response, p_{T}^{truth} > 1000 GeV",100,0,2);
    TH1F hist_response_track_pt20("Step5_response_track_pt20","Track jet p_{T} response, p_{T}^{truth} > 20 GeV",100,0,2);
    TH1F hist_response_track_pt100("Step5_response_track_pt100","Track jet p_{T} response, p_{T}^{truth} > 100 GeV",100,0,2);
    TH1F hist_response_track_pt1000("Step5_response_track_pt1000","Track jet p_{T} response, p_{T}^{truth} > 1000 GeV",100,0,2);



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
        //  hist_mu:     mu distribution
        //  hist_npv:    npv distribution
        //  hist_mu_npv: mu (x-axis) vs npv (y-axis) distribution 
        if (!stepNum || stepNum >= 1)
        {
             // TODO fill the mu, npv, and mu vs npv histograms
            hist_mu.Fill(mu_average);
            hist_npv.Fill(NPV);
            hist_mu_npv.Fill(mu_average,NPV);
        }



        // Step 2: R=0.4 cluster and truth jets and the event weight
        // Histograms to fill:
        //  hist_reco_pt_nw:  Leading R=0.4 calorimeter jet pT, without the event weight
        //  hist_reco_pt:     Leading R=0.4 calorimeter jet pT, with the event weight
        //  hist_truth_pt_nw: Leading R=0.4 truth jet pT, without the event weight
        //  hist_truth_pt:    Leading R=0.4 truth jet pT, with the event weight
        if (!stepNum || stepNum >= 2)
        {
            // TODO fill the calorimeter and truth jet pT histograms
            if (RecoJet_pt->size())
            {
                hist_reco_pt_nw.Fill(RecoJet_pt->at(0));
                hist_reco_pt.Fill(RecoJet_pt->at(0),EventWeight);
            }
            if (TruthJet_pt->size())
            {
                hist_truth_pt_nw.Fill(TruthJet_pt->at(0));
                hist_truth_pt.Fill(TruthJet_pt->at(0),EventWeight);
            }
        }

        // Step 3: Pileup dependence
        // Histograms to fill:
        //  hist_reco_njets_lowmu:   R=0.4 calorimeter jet multiplicity for pT > 20 GeV, mu < 30, with the event weight
        //  hist_reco_njets_midmu:   R=0.4 calorimeter jet multiplicity for pT > 20 GeV, 35 < mu < 45, with the event weight
        //  hist_reco_njets_highmu:  R=0.4 calorimeter jet multiplicity for pT > 20 GeV, mu > 50, with the event weight
        //  hist_truth_njets_lowmu:  R=0.4 truth jet multiplicity for pT > 20 GeV, mu < 30, with the event weight
        //  hist_truth_njets_midmu:  R=0.4 truth jet multiplicity for pT > 20 GeV, 35 < mu < 45, with the event weight
        //  hist_truth_njets_highmu: R=0.4 truth jet multiplicity for pT > 20 GeV, mu > 50, with the event weight
        //  hist_reco_njets_mu_npv:  R=0.4 calorimeter jet multiplicity for pT > 20 GeV (z-axis), vs mu (x-axis) and npv (y-axis), with the event weight
        //  hist_truth_njets_mu_npv: R=0.4 truth jet multiplicity for pT > 20 GeV (z-axis), vs mu (x-axis) and npv (y-axis), with the event weight
        if (!stepNum || stepNum >= 3)
        {
            // TODO fill the jet multiplicity histograms for calorimeter and truth jets for different mu selections, and also vs both mu and npv
        }

        // Step 4: Tracks and R=0.4 track jets 
        // Histograms to fill:
        //  hist_reco_jvf_pt20:      Leading R=0.4 calorimeter jet JVF, pT > 20 GeV, with the event weight
        //  hist_reco_jvf_pt60:      Leading R=0.4 calorimeter jet JVF, pT > 60 GeV, with the event weight
        //  hist_reco_jvf_pt100:     Leading R=0.4 calorimeter jet JVF, pT > 100 GeV, with the event weight
        //  hist_reco_pt_jvf:        Leading R=0.4 calorimeter jet pT, after |JVF|>0.5, with the event weight
        //  hist_track_pt:           Leading R=0.4 track jet pT, with the event weight
        //  hist_track_njets_lowmu:  R=0.4 track jet multiplicity for pT > 20 GeV, mu < 30, with the event weight
        //  hist_track_njets_midmu:  R=0.4 track jet multiplicity for pT > 20 GeV, 35 < mu < 45, with the event weight
        //  hist_track_njets_highmu: R=0.4 track jet multiplicity for pT > 20 GeV, mu > 50, with the event weight
        //  hist_track_njets_mu_npv: R=0.4 track jet multiplicity for pT > 20 GeV (z-axis), vs mu (x-axis) and npv (y-axis), with the event weight
        if (!stepNum || stepNum >= 4)
        {
            // TODO fill the calorimeter JVF histograms and apply a |JVF|>0.5 cut to the leading calorimeter jet pT spectrum to see the impact of tracking on jet multiplicity suppression, then look at track jets
        }

        // Step 5: Jet response studies
        // Histograms to fill:
        //  hist_DRtruth_reco:          Delta R between the leading R=0.4 truth and calorimeter jets, with the event weight
        //  hist_DRtruth_reco_jvf:      Delta R between the leading R=0.4 truth and calorimeter jets, after |JVF|>0.5, with the event weight
        //  hist_DRtruth_track:         Delta R between the leading R=0.4 truth and track jets, with the event weight
        //  hist_response_reco_pt20:    Response (pTreco/pTtrue) for the leading calorimeter jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 20 GeV, with the event weight
        //  hist_response_reco_pt100:   Response (pTreco/pTtrue) for the leading calorimeter jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 100 GeV, with the event weight
        //  hist_response_reco_pt1000:  Response (pTreco/pTtrue) for the leading calorimeter jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 1000 GeV, with the event weight
        //  hist_response_track_pt20:   Response (pTreco/pTtrue) for the leading track jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 20 GeV, with the event weight
        //  hist_response_track_pt100:  Response (pTreco/pTtrue) for the leading track jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 100 GeV, with the event weight
        //  hist_response_track_pt1000: Response (pTreco/pTtrue) for the leading track jet matched to the leading truth jet within DR of 0.3, with a truth jet pT cut of 1000 GeV, with the event weight
        if (!stepNum || stepNum >= 5)
        {
            // TODO Match reconstructed jets to truth jets, and then study the response of matched jets (both calorimeter and track jets matched to truth jets)
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
        hist_mu_npv.Write();
    }

    // Step 2: R=0.4 cluster and truth jets and the event weight
    if (!stepNum || stepNum >= 2)
    {
        hist_reco_pt_nw.Write();
        hist_reco_pt.Write();
        hist_truth_pt_nw.Write();
        hist_truth_pt.Write();
    }

    // Step 3: Pileup dependence
    if (!stepNum || stepNum >= 3)
    {
        hist_reco_njets_lowmu.Write();
        hist_reco_njets_midmu.Write();
        hist_reco_njets_highmu.Write();
        hist_truth_njets_lowmu.Write();
        hist_truth_njets_midmu.Write();
        hist_truth_njets_highmu.Write();

        hist_reco_njets_mu_npv.Write();
        hist_truth_njets_mu_npv.Write();
    }

    // Step 4: Tracks and R=0.4 track jets 
    if (!stepNum || stepNum >= 4)
    {
        hist_reco_jvf_pt20.Write();
        hist_reco_jvf_pt60.Write();
        hist_reco_jvf_pt100.Write();
        hist_reco_pt_jvf.Write();

        hist_track_pt.Write();
        hist_track_njets_lowmu.Write();
        hist_track_njets_midmu.Write();
        hist_track_njets_highmu.Write();
        hist_track_njets_mu_npv.Write();
    }

    // Step 5: Jet response studies
    if (!stepNum || stepNum >= 5)
    {
        hist_DRtruth_reco.Write();
        hist_DRtruth_reco_jvf.Write();
        hist_DRtruth_track.Write();

        hist_response_reco_pt20.Write();
        hist_response_reco_pt100.Write();
        hist_response_reco_pt1000.Write();
        hist_response_track_pt20.Write();
        hist_response_track_pt100.Write();
        hist_response_track_pt1000.Write();
    }



    outFile->Close();

    return 0;
}

