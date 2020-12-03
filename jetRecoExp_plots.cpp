////////////////////////////////////////
// Author: Steven Schramm
// Last update: March 8, 2020
////////////////////////////////////////

// Compile with (for example, update to point to your files)
// g++ jetRecoExp_plots.cpp -o jetRecoExp_plots `root-config --cflags --libs`


#include <iostream>
#include <vector>

#include "TFile.h"
#include "TString.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"

template <class C>
C* getHist(TFile& inFile, const std::string& histName)
{
    C* hist = dynamic_cast<C*>(inFile.Get(histName.c_str()));
    if (!hist)
    {
        printf("Unable to retrieve a required histogram, please make sure that the input file was run up to the required step\n");
        printf("The missing histogram is named: %s\n",histName.c_str());
        return nullptr;
    }
    return hist;
}

TH1I* getTH1I(TFile& inFile, const std::string& histName)
{
    return getHist<TH1I>(inFile,histName);
}

TH1F* getTH1F(TFile& inFile, const std::string& histName)
{
    return getHist<TH1F>(inFile,histName);
}

void scaleMeVToGeV(TH1* hist)
{
    const double xmin = hist->GetXaxis()->GetXmin();
    const double xmax = hist->GetXaxis()->GetXmax();
    hist->GetXaxis()->SetLimits(xmin/1.e3,xmax/1.e3);
}

int main (int argc, char* argv[])
{
    // Check arguments
    if (argc < 4)
    {
        printf("USAGE: %s <output pdf file> <step number> <input root file>\n",argv[0]);
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
    const TString outFileName = argv[1];
    const int stepNum         = atol(argv[2]);
    const TString inFileName  = argv[3];
    
    // Check the arguments
    if (!outFileName.EndsWith(".pdf"))
    {
        printf("The output file should be a pdf file, check that the file name ends with .pdf: %s\n",outFileName.Data());
        return 1;
    }
    if (!inFileName.EndsWith(".root"))
    {
        printf("The input file should be a root file, check that the file name ends with .root: %s\n",inFileName.Data());
        return 1;
    }
    if (stepNum < 0 || stepNum > 5)
    {
        printf("Invalid step number: %d\n",stepNum);
        return 1;
    }
    
    // Open the input file
    TFile* inFile = TFile::Open(inFileName.Data(),"READ");
    if (!inFile || inFile->IsZombie())
    {
        printf("Unable to open the specified input file, please check that it exists: %s\n",inFileName.Data());
        return 1;
    }


    ////////////////////////////////////////////////////////////
    // Get the input histograms                               //
    ////////////////////////////////////////////////////////////

    // Step 1: event-level information
    TH1I* hist_mu     = nullptr;
    TH1I* hist_npv    = nullptr;
    TH2I* hist_mu_npv = nullptr;
    if (!stepNum || stepNum >= 1)
    {
        hist_mu  = getTH1I(*inFile,"Step1_mu");
        hist_npv = getTH1I(*inFile,"Step1_npv");
        hist_mu_npv = getHist<TH2I>(*inFile,"Step1_mu_npv");

        if (!hist_mu || !hist_npv || !hist_mu_npv) return 1;
    }

    // Step 2: R=0.4 cluster and truth jets and the event weight
    TH1F* hist_reco_pt_nw  = nullptr;
    TH1F* hist_reco_pt     = nullptr;
    TH1F* hist_truth_pt_nw = nullptr;
    TH1F* hist_truth_pt    = nullptr;
    if (!stepNum || stepNum >= 2)
    {
        hist_reco_pt_nw  = getTH1F(*inFile,"Step2_RecoJet_pt_noweight");
        hist_reco_pt     = getTH1F(*inFile,"Step2_RecoJet_pt");
        hist_truth_pt_nw = getTH1F(*inFile,"Step2_TruthJet_pt_noweight");
        hist_truth_pt    = getTH1F(*inFile,"Step2_TruthJet_pt");

        if (!hist_reco_pt_nw  || !hist_reco_pt
         || !hist_truth_pt_nw || !hist_truth_pt) return 1;
    }

    // Step 3: Pileup dependence
    TH1F* hist_reco_njets_lowmu         = nullptr;
    TH1F* hist_reco_njets_midmu         = nullptr;
    TH1F* hist_reco_njets_highmu        = nullptr;
    TH1F* hist_truth_njets_lowmu        = nullptr;
    TH1F* hist_truth_njets_midmu        = nullptr;
    TH1F* hist_truth_njets_highmu       = nullptr;
    TProfile2D* hist_reco_njets_mu_npv  = nullptr;
    TProfile2D* hist_truth_njets_mu_npv = nullptr;
    if (!stepNum || stepNum >= 3)
    {
        hist_reco_njets_lowmu   = getTH1F(*inFile,"Step3_RecoJet_njets_lowmu");
        hist_reco_njets_midmu   = getTH1F(*inFile,"Step3_RecoJet_njets_midmu");
        hist_reco_njets_highmu  = getTH1F(*inFile,"Step3_RecoJet_njets_highmu");
        hist_truth_njets_lowmu  = getTH1F(*inFile,"Step3_TruthJet_njets_lowmu");
        hist_truth_njets_midmu  = getTH1F(*inFile,"Step3_TruthJet_njets_midmu");
        hist_truth_njets_highmu = getTH1F(*inFile,"Step3_TruthJet_njets_highmu");
        hist_reco_njets_mu_npv  = getHist<TProfile2D>(*inFile,"Step3_RecoJets_njets_2D");
        hist_truth_njets_mu_npv = getHist<TProfile2D>(*inFile,"Step3_TruthJets_njets_2D");

        if (!hist_reco_njets_lowmu  || !hist_reco_njets_midmu  || !hist_reco_njets_highmu
         || !hist_truth_njets_lowmu || !hist_truth_njets_midmu || !hist_truth_njets_highmu
         || !hist_reco_njets_mu_npv || !hist_truth_njets_mu_npv) return 1;
    }

    // Step 4: Tracks and R=0.4 track jets
    TH1F* hist_reco_jvf_pt20            = nullptr;
    TH1F* hist_reco_jvf_pt60            = nullptr;
    TH1F* hist_reco_jvf_pt100           = nullptr;
    TH1F* hist_reco_pt_jvf              = nullptr;
    TH1F* hist_track_pt                 = nullptr;
    TH1F* hist_track_njets_lowmu        = nullptr;
    TH1F* hist_track_njets_midmu        = nullptr;
    TH1F* hist_track_njets_highmu       = nullptr;
    TProfile2D* hist_track_njets_mu_npv = nullptr;
    if (!stepNum || stepNum >= 4)
    {
        hist_reco_jvf_pt20      = getTH1F(*inFile,"Step4_RecoJet_jvf_pt20");
        hist_reco_jvf_pt60      = getTH1F(*inFile,"Step4_RecoJet_jvf_pt60");
        hist_reco_jvf_pt100     = getTH1F(*inFile,"Step4_RecoJet_jvf_pt100");
        hist_reco_pt_jvf        = getTH1F(*inFile,"Step4_RecoJet_pt_jvf");
        hist_track_pt           = getTH1F(*inFile,"Step4_TrackJet_pt");
        hist_track_njets_lowmu  = getTH1F(*inFile,"Step4_TrackJet_njets_lowmu");
        hist_track_njets_midmu  = getTH1F(*inFile,"Step4_TrackJet_njets_midmu");
        hist_track_njets_highmu = getTH1F(*inFile,"Step4_TrackJet_njets_highmu");
        hist_track_njets_mu_npv = getHist<TProfile2D>(*inFile,"Step4_TrackJets_njets_2D");

        if (!hist_reco_jvf_pt20     || !hist_reco_jvf_pt60     || !hist_reco_jvf_pt100
         || !hist_track_njets_lowmu || !hist_track_njets_midmu || !hist_track_njets_highmu
         || !hist_truth_pt          || !hist_track_njets_mu_npv) return 1;
    }

    // Step 5: Jet response studies
    TH1F* hist_DRtruth_reco           = nullptr;
    TH1F* hist_DRtruth_reco_jvf       = nullptr;
    TH1F* hist_DRtruth_track          = nullptr;
    TH1F* hist_response_reco_pt20     = nullptr;
    TH1F* hist_response_reco_pt100    = nullptr;
    TH1F* hist_response_reco_pt1000   = nullptr;
    TH1F* hist_response_track_pt20    = nullptr;
    TH1F* hist_response_track_pt100   = nullptr;
    TH1F* hist_response_track_pt1000  = nullptr;
    if (!stepNum || stepNum >= 5)
    {
        hist_DRtruth_reco           = getTH1F(*inFile,"Step5_DRtruth_reco");
        hist_DRtruth_reco_jvf       = getTH1F(*inFile,"Step5_DRtruth_reco_jvf");
        hist_DRtruth_track          = getTH1F(*inFile,"Step5_DRtruth_track");
        hist_response_reco_pt20     = getTH1F(*inFile,"Step5_response_reco_pt20");
        hist_response_reco_pt100    = getTH1F(*inFile,"Step5_response_reco_pt100");
        hist_response_reco_pt1000   = getTH1F(*inFile,"Step5_response_reco_pt1000");
        hist_response_track_pt20    = getTH1F(*inFile,"Step5_response_track_pt20");
        hist_response_track_pt100   = getTH1F(*inFile,"Step5_response_track_pt100");
        hist_response_track_pt1000  = getTH1F(*inFile,"Step5_response_track_pt1000");

        if (!hist_DRtruth_reco        || !hist_DRtruth_reco_jvf     || !hist_DRtruth_track
         || !hist_response_reco_pt20  || !hist_response_reco_pt100  || !hist_response_reco_pt1000
         || !hist_response_track_pt20 || !hist_response_track_pt100 || !hist_response_track_pt1000) return 1;
    }


    ////////////////////////////////////////////////////////////
    // Make the plots                                         //
    ////////////////////////////////////////////////////////////

    // Make the canvas and open the output pdf file
    TCanvas canvas("canvas","canvas",800,600);
    canvas.cd();
    canvas.Print(outFileName+"[");


    // Step 1: event-level information
    if (!stepNum || stepNum >= 1)
    {
        // Mu
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_mu->SetLineColor(kBlack);
        hist_mu->SetLineWidth(2);
        hist_mu->GetXaxis()->SetTitle("Average number of interactions");
        hist_mu->GetYaxis()->SetTitle("Number of events");
        hist_mu->SetStats(0);
        hist_mu->Draw();
        canvas.Print(outFileName);

        // NPV
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_npv->SetLineColor(kBlack);
        hist_npv->SetLineWidth(2);
        hist_npv->GetXaxis()->SetTitle("Number of primary vertices");
        hist_npv->GetYaxis()->SetTitle("Number of events");
        hist_npv->SetStats(0);
        hist_npv->Draw();
        canvas.Print(outFileName);

        // Mu vs NPV
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_mu_npv->GetXaxis()->SetTitle("Average number of interactions");
        hist_mu_npv->GetYaxis()->SetTitle("Number of primary vertices");
        hist_mu_npv->SetTitle("Number of events vs #mu_{average} and NPV");
        hist_mu_npv->SetStats(0);
        hist_mu_npv->Draw("colz");
        canvas.Print(outFileName);
    }
    

    // Step 2: R=0.4 cluster and truth jets and the event weight
    if (!stepNum || stepNum >= 2)
    {
        // Leading jet pT (without weights)
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_truth_pt_nw->SetLineColor(kRed);
        hist_truth_pt_nw->SetLineWidth(2);
        scaleMeVToGeV(hist_truth_pt_nw);
        hist_truth_pt_nw->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_truth_pt_nw->GetYaxis()->SetTitle("Number of events");
        hist_truth_pt_nw->SetTitle("Leading R=0.4 jet p_{T}, no weights");
        hist_truth_pt_nw->GetYaxis()->SetRangeUser(0,4000);
        hist_truth_pt_nw->SetStats(0);
        hist_truth_pt_nw->Draw();

        hist_reco_pt_nw->SetLineColor(kBlue);
        hist_reco_pt_nw->SetLineWidth(2);
        scaleMeVToGeV(hist_reco_pt_nw);
        hist_reco_pt_nw->Draw("same");

        TLegend legend_pt_nw(0.50,0.65,0.89,0.75);
        legend_pt_nw.AddEntry(hist_truth_pt_nw,"Truth jet");
        legend_pt_nw.AddEntry(hist_reco_pt_nw,"Cluster jet");
        legend_pt_nw.SetBorderSize(0);
        legend_pt_nw.Draw("same");
        canvas.Print(outFileName);



        // Leading jet pT (with weights)
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_truth_pt->SetLineColor(kRed);
        hist_truth_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_truth_pt);
        hist_truth_pt->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_truth_pt->GetYaxis()->SetTitle("Weighted number of events");
        hist_truth_pt->SetTitle("Leading R=0.4 jet p_{T}");
        hist_truth_pt->GetXaxis()->SetMoreLogLabels();
        hist_truth_pt->GetXaxis()->SetTitleOffset(1.25);
        hist_truth_pt->SetStats(0);
        hist_truth_pt->Draw();

        hist_reco_pt->SetLineColor(kBlue);
        hist_reco_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_reco_pt);
        hist_reco_pt->Draw("same");

        TLegend legend_pt(0.50,0.65,0.89,0.75);
        legend_pt.AddEntry(hist_truth_pt,"Truth jet");
        legend_pt.AddEntry(hist_reco_pt,"Cluster jet");
        legend_pt.SetBorderSize(0);
        legend_pt.Draw("same");
        canvas.Print(outFileName);
    }


    // Step 3: Pileup dependence
    if (!stepNum || stepNum >= 3)
    {
        // Cluster jet multiplicity vs pileup
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_reco_njets_highmu->Scale(1./hist_reco_njets_highmu->Integral());
        hist_reco_njets_highmu->SetLineColor(kRed);
        hist_reco_njets_highmu->SetLineWidth(2);
        hist_reco_njets_highmu->GetXaxis()->SetTitle("Number of jets");
        hist_reco_njets_highmu->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_reco_njets_highmu->SetTitle("Number of cluster jets with p_{T} > 20 GeV");
        hist_reco_njets_highmu->GetYaxis()->SetRangeUser(1.e-3,1.0);
        hist_reco_njets_highmu->SetStats(0);
        hist_reco_njets_highmu->Draw();

        hist_reco_njets_midmu->Scale(1./hist_reco_njets_midmu->Integral());
        hist_reco_njets_midmu->SetLineColor(kBlue);
        hist_reco_njets_midmu->SetLineWidth(2);
        hist_reco_njets_midmu->Draw("same");

        hist_reco_njets_lowmu->Scale(1./hist_reco_njets_lowmu->Integral());
        hist_reco_njets_lowmu->SetLineColor(kGreen+2);
        hist_reco_njets_lowmu->SetLineWidth(2);
        hist_reco_njets_lowmu->Draw("same");

        TLegend legend_cl(0.50,0.65,0.70,0.80);
        legend_cl.AddEntry(hist_reco_njets_highmu,"#mu > 50");
        legend_cl.AddEntry(hist_reco_njets_midmu,"35 < #mu < 45");
        legend_cl.AddEntry(hist_reco_njets_lowmu,"#mu < 30");
        legend_cl.SetBorderSize(0);
        legend_cl.Draw("same");
        canvas.Print(outFileName);



        // Truth jet multiplicity vs pileup
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_truth_njets_highmu->Scale(1./hist_truth_njets_highmu->Integral());
        hist_truth_njets_highmu->SetLineColor(kRed);
        hist_truth_njets_highmu->SetLineWidth(2);
        hist_truth_njets_highmu->GetXaxis()->SetTitle("Number of jets");
        hist_truth_njets_highmu->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_truth_njets_highmu->SetTitle("Number of truth jets with p_{T} > 20 GeV");
        hist_truth_njets_highmu->GetYaxis()->SetRangeUser(1.e-3,1.0);
        hist_truth_njets_highmu->SetStats(0);
        hist_truth_njets_highmu->Draw();

        hist_truth_njets_midmu->Scale(1./hist_truth_njets_midmu->Integral());
        hist_truth_njets_midmu->SetLineColor(kBlue);
        hist_truth_njets_midmu->SetLineWidth(2);
        hist_truth_njets_midmu->Draw("same");

        hist_truth_njets_lowmu->Scale(1./hist_truth_njets_lowmu->Integral());
        hist_truth_njets_lowmu->SetLineColor(kGreen+2);
        hist_truth_njets_lowmu->SetLineWidth(2);
        hist_truth_njets_lowmu->Draw("same");

        TLegend legend_tr(0.50,0.65,0.70,0.80);
        legend_tr.AddEntry(hist_truth_njets_highmu,"#mu > 50");
        legend_tr.AddEntry(hist_truth_njets_midmu,"35 < #mu < 45");
        legend_tr.AddEntry(hist_truth_njets_lowmu,"#mu < 30");
        legend_tr.SetBorderSize(0);
        legend_tr.Draw("same");
        canvas.Print(outFileName);



        // Cluster jet multiplicity profile
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_reco_njets_mu_npv->GetXaxis()->SetTitle("Average number of interactions");
        hist_reco_njets_mu_npv->GetYaxis()->SetTitle("Number of primary vertices");
        hist_reco_njets_mu_npv->GetZaxis()->SetTitle("Average number of jets");
        hist_reco_njets_mu_npv->GetZaxis()->SetTitleOffset(0.7);
        hist_reco_njets_mu_npv->GetZaxis()->SetRangeUser(0,80);
        hist_reco_njets_mu_npv->SetStats(0);
        hist_reco_njets_mu_npv->Draw("colz");
        canvas.Print(outFileName);



        // Truth jet multiplicity profile
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_truth_njets_mu_npv->GetXaxis()->SetTitle("Average number of interactions");
        hist_truth_njets_mu_npv->GetYaxis()->SetTitle("Number of primary vertices");
        hist_truth_njets_mu_npv->GetZaxis()->SetTitle("Average number of jets");
        hist_truth_njets_mu_npv->GetZaxis()->SetTitleOffset(0.7);
        hist_truth_njets_mu_npv->GetZaxis()->SetRangeUser(0,80);
        hist_truth_njets_mu_npv->SetStats(0);
        hist_truth_njets_mu_npv->Draw("colz");
        canvas.Print(outFileName);
    }


    // Step 4: Tracks and R=0.4 track jets 
    if (!stepNum || stepNum >= 4)
    {
        // JVF distributions
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_reco_jvf_pt20->Scale(1./hist_reco_jvf_pt20->Integral());
        hist_reco_jvf_pt20->SetLineColor(kRed);
        hist_reco_jvf_pt20->SetLineWidth(2);
        hist_reco_jvf_pt20->GetXaxis()->SetTitle("Number of jets");
        hist_reco_jvf_pt20->GetYaxis()->SetTitle("Weighted number of events");
        hist_reco_jvf_pt20->SetTitle("Leading jet JVF distribution");
        hist_reco_jvf_pt20->GetYaxis()->SetRangeUser(0,0.4);
        hist_reco_jvf_pt20->SetStats(0);
        hist_reco_jvf_pt20->Draw();

        hist_reco_jvf_pt60->Scale(1./hist_reco_jvf_pt60->Integral());
        hist_reco_jvf_pt60->SetLineColor(kBlue);
        hist_reco_jvf_pt60->SetLineWidth(2);
        hist_reco_jvf_pt60->Draw("same");

        hist_reco_jvf_pt100->Scale(1./hist_reco_jvf_pt100->Integral());
        hist_reco_jvf_pt100->SetLineColor(kGreen+2);
        hist_reco_jvf_pt100->SetLineWidth(2);
        hist_reco_jvf_pt100->Draw("same");

        TLegend legend_jvf(0.60,0.55,0.80,0.75);
        legend_jvf.AddEntry(hist_reco_jvf_pt20,"p_{T} > 20 GeV");
        legend_jvf.AddEntry(hist_reco_jvf_pt60,"p_{T} > 60 GeV");
        legend_jvf.AddEntry(hist_reco_jvf_pt100,"p_{T} > 100 GeV");
        legend_jvf.SetBorderSize(0);
        legend_jvf.Draw("same");
        canvas.Print(outFileName);



        // Jet pT comparison after JVT
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_truth_pt->SetStats(0);
        hist_truth_pt->Draw();

        hist_reco_pt->Draw("same");

        hist_reco_pt_jvf->SetLineColor(kGreen+2);
        hist_reco_pt_jvf->SetLineWidth(2);
        scaleMeVToGeV(hist_reco_pt_jvf);
        hist_reco_pt_jvf->Draw("same");

        TLegend legend_pt(0.50,0.60,0.89,0.75);
        legend_pt.AddEntry(hist_truth_pt,"Truth jet");
        legend_pt.AddEntry(hist_reco_pt,"Cluster jet");
        legend_pt.AddEntry(hist_reco_pt_jvf,"Cluster jet, |JVF|>0.5");
        legend_pt.SetBorderSize(0);
        legend_pt.Draw("same");
        canvas.Print(outFileName);



        // Truth jet multiplicity vs pileup
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_track_njets_highmu->Scale(1./hist_track_njets_highmu->Integral());
        hist_track_njets_highmu->SetLineColor(kRed);
        hist_track_njets_highmu->SetLineWidth(2);
        hist_track_njets_highmu->GetXaxis()->SetTitle("Number of jets");
        hist_track_njets_highmu->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_track_njets_highmu->SetTitle("Number of track jets with p_{T} > 20 GeV");
        hist_track_njets_highmu->GetYaxis()->SetRangeUser(1.e-3,1);
        hist_track_njets_highmu->SetStats(0);
        hist_track_njets_highmu->Draw();

        hist_track_njets_midmu->Scale(1./hist_track_njets_midmu->Integral());
        hist_track_njets_midmu->SetLineColor(kBlue);
        hist_track_njets_midmu->SetLineWidth(2);
        hist_track_njets_midmu->Draw("same");

        hist_track_njets_lowmu->Scale(1./hist_track_njets_lowmu->Integral());
        hist_track_njets_lowmu->SetLineColor(kGreen+2);
        hist_track_njets_lowmu->SetLineWidth(2);
        hist_track_njets_lowmu->Draw("same");

        TLegend legend_trk(0.50,0.65,0.70,0.80);
        legend_trk.AddEntry(hist_track_njets_highmu,"#mu > 50");
        legend_trk.AddEntry(hist_track_njets_midmu,"35 < #mu < 45");
        legend_trk.AddEntry(hist_track_njets_lowmu,"#mu < 30");
        legend_trk.SetBorderSize(0);
        legend_trk.Draw("same");
        canvas.Print(outFileName);
        
        
        
        // Track jet multiplicity profile
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_track_njets_mu_npv->GetXaxis()->SetTitle("Average number of interactions");
        hist_track_njets_mu_npv->GetYaxis()->SetTitle("Number of primary vertices");
        hist_track_njets_mu_npv->GetZaxis()->SetTitle("Average number of jets");
        hist_track_njets_mu_npv->GetZaxis()->SetTitleOffset(0.7);
        hist_track_njets_mu_npv->GetZaxis()->SetRangeUser(0,80);
        hist_track_njets_mu_npv->SetStats(0);
        hist_track_njets_mu_npv->Draw("colz");
        canvas.Print(outFileName);



        // Track jet pT spectrum
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_truth_pt->SetStats(0);
        hist_truth_pt->Draw();

        hist_reco_pt_jvf->Draw("same");

        hist_track_pt->SetLineColor(kViolet);
        hist_track_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_track_pt);
        hist_track_pt->Draw("same");

        TLegend legend_ptrk(0.50,0.74,0.89,0.89);
        legend_ptrk.AddEntry(hist_truth_pt,"Truth jet");
        legend_ptrk.AddEntry(hist_reco_pt_jvf,"Cluster jet, |JVF|>0.5");
        legend_ptrk.AddEntry(hist_track_pt,"Track jet");
        legend_ptrk.SetBorderSize(0);
        legend_ptrk.Draw("same");
        canvas.Print(outFileName);
    }

    // Step 5: Jet response studies
    if (!stepNum || stepNum >= 5)
    {
        // DR for matching
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_DRtruth_reco->Scale(1./hist_DRtruth_reco->Integral());
        hist_DRtruth_reco->SetLineColor(kRed);
        hist_DRtruth_reco->SetLineWidth(2);
        hist_DRtruth_reco->GetXaxis()->SetTitle("Delta R");
        hist_DRtruth_reco->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_DRtruth_reco->SetTitle("Delta R from the leading truth jet, p_{T}^{truth} > 20 GeV");
        hist_DRtruth_reco->GetYaxis()->SetRangeUser(0,1);
        hist_DRtruth_reco->SetStats(0);
        hist_DRtruth_reco->Draw();

        hist_DRtruth_reco_jvf->Scale(1./hist_DRtruth_reco_jvf->Integral());
        hist_DRtruth_reco_jvf->SetLineColor(kGreen+2);
        hist_DRtruth_reco_jvf->SetLineWidth(2);
        hist_DRtruth_reco_jvf->Draw("same");

        hist_DRtruth_track->Scale(1./hist_DRtruth_track->Integral());
        hist_DRtruth_track->SetLineColor(kViolet);
        hist_DRtruth_track->SetLineWidth(2);
        hist_DRtruth_track->Draw("same");

        TLegend legend_DR(0.40,0.65,0.75,0.85);
        legend_DR.AddEntry(hist_DRtruth_reco,"Cluster jets");
        legend_DR.AddEntry(hist_DRtruth_reco_jvf,"Cluster jets, |JVF|>0.5");
        legend_DR.AddEntry(hist_DRtruth_track,"Track jets");
        legend_DR.SetBorderSize(0);
        legend_DR.Draw("same");
        canvas.Print(outFileName);



        // Cluster jet response
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_response_reco_pt20->Scale(1./hist_response_reco_pt20->Integral());
        hist_response_reco_pt20->SetLineColor(kRed);
        hist_response_reco_pt20->SetLineWidth(2);
        hist_response_reco_pt20->GetXaxis()->SetTitle("Jet response");
        hist_response_reco_pt20->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_response_reco_pt20->SetTitle("Cluster jet p_{T} response, p_{T}^{cluster}/p_{T}^{truth}");
        hist_response_reco_pt20->GetYaxis()->SetRangeUser(0,0.25);
        TF1 gauss20c("gauss20c","gaus",0,2);
        gauss20c.SetLineColor(kRed);
        hist_response_reco_pt20->Fit(&gauss20c,"E","",0.7,1.6);

        hist_response_reco_pt100->Scale(1./hist_response_reco_pt100->Integral());
        hist_response_reco_pt100->SetLineColor(kBlue);
        hist_response_reco_pt100->SetLineWidth(2);
        TF1 gauss100c("gauss100c","gaus",0,2);
        gauss100c.SetLineColor(kBlue);
        hist_response_reco_pt100->Fit(&gauss100c,"E","",0.8,1.2);

        hist_response_reco_pt1000->Scale(1./hist_response_reco_pt1000->Integral());
        hist_response_reco_pt1000->SetLineColor(kGreen+2);
        hist_response_reco_pt1000->SetLineWidth(2);
        TF1 gauss1000c("gauss1000c","gaus",0,2);
        gauss1000c.SetLineColor(kGreen+2);
        hist_response_reco_pt1000->Fit(&gauss1000c,"E","",0.8,1.2);
        
        hist_response_reco_pt20->SetStats(0);
        hist_response_reco_pt20->Draw();
        hist_response_reco_pt100->Draw("same");
        hist_response_reco_pt1000->Draw("same");

        TLegend legend_rc(0.12,0.65,0.45,0.85);
        legend_rc.AddEntry(hist_response_reco_pt20,Form("p_{T}^{truth} > 20 GeV, #sigma/#mu = %.2f",gauss20c.GetParameter(2)/gauss20c.GetParameter(1)));
        legend_rc.AddEntry(hist_response_reco_pt100,Form("p_{T}^{truth} > 100 GeV, #sigma/#mu = %.2f",gauss100c.GetParameter(2)/gauss100c.GetParameter(1)));
        legend_rc.AddEntry(hist_response_reco_pt1000,Form("p_{T}^{truth} > 1000 GeV, #sigma/#mu = %.2f",gauss1000c.GetParameter(2)/gauss1000c.GetParameter(1)));
        legend_rc.SetBorderSize(0);
        legend_rc.Draw("same");
        canvas.Print(outFileName);



        // Track jet response
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_response_track_pt20->Scale(1./hist_response_track_pt20->Integral());
        hist_response_track_pt20->SetLineColor(kRed);
        hist_response_track_pt20->SetLineWidth(2);
        hist_response_track_pt20->GetXaxis()->SetTitle("Jet response");
        hist_response_track_pt20->GetYaxis()->SetTitle("Weighted fraction of events");
        hist_response_track_pt20->SetTitle("Track jet p_{T} response, p_{T}^{track}/p_{T}^{truth}");
        hist_response_track_pt20->GetYaxis()->SetRangeUser(0,0.25);
        TF1 gauss20t("gauss20t","gaus",0,2);
        gauss20t.SetLineColor(kRed);
        hist_response_track_pt20->Fit(&gauss20t,"E","",0.3,1.1);

        hist_response_track_pt100->Scale(1./hist_response_track_pt100->Integral());
        hist_response_track_pt100->SetLineColor(kBlue);
        hist_response_track_pt100->SetLineWidth(2);
        TF1 gauss100t("gauss100t","gaus",0,2);
        gauss100t.SetLineColor(kBlue);
        hist_response_track_pt100->Fit(&gauss100t,"E","",0.3,1.0);

        hist_response_track_pt1000->Scale(1./hist_response_track_pt1000->Integral());
        hist_response_track_pt1000->SetLineColor(kGreen+2);
        hist_response_track_pt1000->SetLineWidth(2);
        TF1 gauss1000t("gauss1000t","gaus",0,2);
        gauss1000t.SetLineColor(kGreen+2);
        hist_response_track_pt1000->Fit(&gauss1000t,"E","",0.2,1.0);
        
        hist_response_track_pt20->SetStats(0);
        hist_response_track_pt20->Draw();
        hist_response_track_pt100->Draw("same");
        hist_response_track_pt1000->Draw("same");

        TLegend legend_rt(0.12,0.65,0.45,0.85);
        legend_rt.AddEntry(hist_response_track_pt20,Form("p_{T}^{truth} > 20 GeV, #sigma/#mu = %.2f",gauss20t.GetParameter(2)/gauss20t.GetParameter(1)));
        legend_rt.AddEntry(hist_response_track_pt100,Form("p_{T}^{truth} > 100 GeV, #sigma/#mu = %.2f",gauss100t.GetParameter(2)/gauss100t.GetParameter(1)));
        legend_rt.AddEntry(hist_response_track_pt1000,Form("p_{T}^{truth} > 1000 GeV, #sigma/#mu = %.2f",gauss1000t.GetParameter(2)/gauss1000t.GetParameter(1)));
        legend_rt.SetBorderSize(0);
        legend_rt.Draw("same");
        canvas.Print(outFileName);
    }

    
    // Close the output pdf file
    canvas.Print(outFileName+"]");

    return 0;
}
