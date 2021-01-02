////////////////////////////////////////
// Author: Steven Schramm
// Last update: March 8, 2020
////////////////////////////////////////

// Compile with (for example, update to point to your files)
// g++ jetRecoGroom_plots.cpp -o jetRecoGroom_plots `root-config --cflags --libs`


#include <iostream>
#include <vector>

#include "TFile.h"
#include "TString.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"


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
        printf("\t2 = up to step 2 (existing jets and the event weight)\n");
        printf("\t3 = up to step 3 (building our own R=1.0 jets from topoclusters)\n");
        printf("\t4 = up to step 4 (building other types of R=1.0 jets from topoclusters)\n");
        printf("\t5 = up to step 5 (calculating substructure variables for R=1.0 jets)\n");
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
    TH1I* hist_mu  = nullptr;
    TH1I* hist_npv = nullptr;
    if (!stepNum || stepNum >= 1)
    {
        hist_mu = getTH1I(*inFile,"Step1_mu");
        hist_npv = getTH1I(*inFile,"Step1_npv");

        if (!hist_mu || !hist_npv) return 1;
    }

    // Step 2: Existing jets and the event weight
    TH1F* hist_ungroom_pt_nw = nullptr;
    TH1F* hist_ungroom_pt    = nullptr;
    TH1F* hist_trimmed_pt    = nullptr;
    TH1F* hist_ungroom_m     = nullptr;
    TH1F* hist_trimmed_m     = nullptr;
    if (!stepNum || stepNum >= 2)
    {
        hist_ungroom_pt_nw = getTH1F(*inFile,"Step2_UngroomPt_noweight");
        hist_ungroom_pt    = getTH1F(*inFile,"Step2_UngroomPt");
        hist_trimmed_pt    = getTH1F(*inFile,"Step2_TrimmedPt");
        hist_ungroom_m     = getTH1F(*inFile,"Step2_UngroomMass");
        hist_trimmed_m     = getTH1F(*inFile,"Step2_TrimmedMass");

        if (!hist_ungroom_pt_nw || !hist_ungroom_pt || !hist_trimmed_pt
         || !hist_ungroom_m     || !hist_trimmed_m) return 1;
    }

    // Step 3: Building our own R=1.0 jets from topoclusters
    TH1F* hist_myungroom_pt_nw   = nullptr;
    TH1F* hist_myungroom_pt      = nullptr;
    TH1F* hist_mytrimmed_pt_nw   = nullptr;
    TH1F* hist_mytrimmed_pt      = nullptr;
    if (!stepNum || stepNum >= 3)
    {
        hist_myungroom_pt_nw   = getTH1F(*inFile,"Step3_MyUngroomPt_noweight");
        hist_myungroom_pt      = getTH1F(*inFile,"Step3_MyUngroomPt");
        hist_mytrimmed_pt_nw   = getTH1F(*inFile,"Step3_MyTrimmedPt_noweight");
        hist_mytrimmed_pt      = getTH1F(*inFile,"Step3_MyTrimmedPt");

        if (!hist_myungroom_pt_nw   || !hist_myungroom_pt
         || !hist_mytrimmed_pt_nw   || !hist_mytrimmed_pt) return 1;
    }
    
    // Step 4: Building other types of R=1.0 jets from topoclusters
    TH1F* hist_mypruned_pt = nullptr;
    TH1F* hist_mypruned_m  = nullptr;
    TH1F* hist_mySD_pt     = nullptr;
    TH1F* hist_mySD_m      = nullptr;
    TH1F* hist_myRSD_pt    = nullptr;
    TH1F* hist_myRSD_m     = nullptr;
    TH1F* hist_myBUSD_pt   = nullptr;
    TH1F* hist_myBUSD_m    = nullptr;
    TH1F* hist_myBUSDT_pt   = nullptr;
    TH1F* hist_myBUSDT_m    = nullptr;
    if (!stepNum || stepNum >= 4)
    {
        hist_mypruned_pt = getTH1F(*inFile,"Step4_MyPrunedPt");
        hist_mypruned_m  = getTH1F(*inFile,"Step4_MyPrunedMass");
        hist_mySD_pt     = getTH1F(*inFile,"Step4_MySDPt");
        hist_mySD_m      = getTH1F(*inFile,"Step4_MySDMass");
        hist_myRSD_pt    = getTH1F(*inFile,"Step4_MyRSDPt");
        hist_myRSD_m     = getTH1F(*inFile,"Step4_MyRSDMass");
        hist_myBUSD_pt   = getTH1F(*inFile,"Step4_MyBUSDPt");
        hist_myBUSD_m    = getTH1F(*inFile,"Step4_MyBUSDMass");
        hist_myBUSDT_pt  = getTH1F(*inFile,"Step4_MyBUSDTPt");
        hist_myBUSDT_m   = getTH1F(*inFile,"Step4_MyBUSDTMass");

        if (!hist_mypruned_pt || !hist_mypruned_m
         || !hist_mySD_pt     || !hist_mySD_m
         || !hist_myRSD_pt    || !hist_myRSD_m
         || !hist_myBUSD_pt   || !hist_myBUSD_m
         || !hist_myBUSDT_pt  || !hist_myBUSDT_m) return 1;
    }

    // Step 5: Calculating substructure variables for R=1.0 jets
    TH1F* hist_ungroom_D2    = nullptr;
    TH1F* hist_ungroom_tau32 = nullptr;
    TH1F* hist_trimmed_D2      = nullptr;
    TH1F* hist_trimmed_tau32   = nullptr;
    TH1F* hist_pruned_D2       = nullptr;
    TH1F* hist_pruned_tau32    = nullptr;
    TH1F* hist_SD_D2           = nullptr;
    TH1F* hist_SD_tau32        = nullptr;
    TH1F* hist_RSD_D2          = nullptr;
    TH1F* hist_RSD_tau32       = nullptr;
    TH1F* hist_BUSD_D2         = nullptr;
    TH1F* hist_BUSD_tau32      = nullptr;
    TH1F* hist_BUSDT_D2        = nullptr;
    TH1F* hist_BUSDT_tau32     = nullptr;
    if (!stepNum || stepNum >= 5)
    {
        hist_ungroom_D2      = getTH1F(*inFile,"Step5_Ungroomed_D2");
        hist_ungroom_tau32   = getTH1F(*inFile,"Step5_Ungroomed_Tau32");
        hist_trimmed_D2      = getTH1F(*inFile,"Step5_Trimmed_D2");
        hist_trimmed_tau32   = getTH1F(*inFile,"Step5_Trimmed_Tau32");
        hist_pruned_D2       = getTH1F(*inFile,"Step5_Pruned_D2");
        hist_pruned_tau32    = getTH1F(*inFile,"Step5_Pruned_Tau32");
        hist_SD_D2           = getTH1F(*inFile,"Step5_SD_D2");
        hist_SD_tau32        = getTH1F(*inFile,"Step5_SD_Tau32");
        hist_RSD_D2          = getTH1F(*inFile,"Step5_RSD_D2");
        hist_RSD_tau32       = getTH1F(*inFile,"Step5_RSD_Tau32");
        hist_BUSD_D2         = getTH1F(*inFile,"Step5_BUSD_D2");
        hist_BUSD_tau32      = getTH1F(*inFile,"Step5_BUSD_Tau32");
        hist_BUSDT_D2        = getTH1F(*inFile,"Step5_BUSDT_D2");
        hist_BUSDT_tau32     = getTH1F(*inFile,"Step5_BUSDT_Tau32");

        if (!hist_ungroom_D2 || !hist_ungroom_tau32
         || !hist_trimmed_D2 || !hist_trimmed_tau32
         || !hist_pruned_D2  || !hist_pruned_tau32
         || !hist_SD_D2      || !hist_SD_tau32
         || !hist_RSD_D2     || !hist_RSD_tau32
         || !hist_BUSD_D2    || !hist_BUSD_tau32
         || !hist_BUSDT_D2   || !hist_BUSDT_tau32) return 1;
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
    }
    

    // Step 2: Existing jets and the event weight
    if (!stepNum || stepNum >= 2)
    {
        // Leading ungroomed jet pT (without weights)
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_ungroom_pt_nw->SetLineColor(kRed);
        hist_ungroom_pt_nw->SetLineWidth(2);
        scaleMeVToGeV(hist_ungroom_pt_nw);
        hist_ungroom_pt_nw->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_ungroom_pt_nw->GetYaxis()->SetTitle("Number of events");
        hist_ungroom_pt_nw->SetStats(0);
        hist_ungroom_pt_nw->Draw();
        canvas.Print(outFileName);



        // Leading ungroomed and trimmed jet pT (with weights)
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_ungroom_pt->SetLineColor(kRed);
        hist_ungroom_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_ungroom_pt);
        hist_ungroom_pt->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_ungroom_pt->GetYaxis()->SetTitle("Weighted number of events");
        hist_ungroom_pt->SetTitle("Leading R=1.0 jet p_{T}");
        hist_ungroom_pt->GetXaxis()->SetMoreLogLabels();
        hist_ungroom_pt->GetXaxis()->SetTitleOffset(1.25);
        hist_ungroom_pt->SetStats(0);
        hist_ungroom_pt->Draw();

        hist_trimmed_pt->SetLineColor(kBlue);
        hist_trimmed_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_trimmed_pt);
        hist_trimmed_pt->Draw("same");
        
        TLegend legend_pt(0.50,0.65,0.89,0.75);
        legend_pt.AddEntry(hist_ungroom_pt,"Leading ungroomed R=1.0 jet p_{T}");
        legend_pt.AddEntry(hist_trimmed_pt);
        legend_pt.SetBorderSize(0);
        legend_pt.Draw("same");
        canvas.Print(outFileName);



        // Leading ungroomed and trimmed jet mass
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_ungroom_m->SetLineColor(kRed);
        hist_ungroom_m->SetLineWidth(2);
        scaleMeVToGeV(hist_ungroom_m);
        hist_ungroom_m->GetXaxis()->SetTitle("Jet mass [GeV]");
        hist_ungroom_m->GetYaxis()->SetTitle("Weighted number of events");
        hist_ungroom_m->SetTitle("Leading R=1.0 jet mass, p_{T} > 400 GeV");
        hist_ungroom_m->GetXaxis()->SetMoreLogLabels();
        hist_ungroom_m->GetXaxis()->SetTitleOffset(1.25);
        hist_ungroom_m->SetStats(0);
        hist_ungroom_m->Draw();

        hist_trimmed_m->SetLineColor(kBlue);
        hist_trimmed_m->SetLineWidth(2);
        scaleMeVToGeV(hist_trimmed_m);
        hist_trimmed_m->Draw("same");
        
        TLegend legend_m(0.15,0.15,0.55,0.25);
        legend_m.AddEntry(hist_ungroom_m,"Leading ungroomed R=1.0 jet mass");
        legend_m.AddEntry(hist_trimmed_m);
        legend_m.SetBorderSize(0);
        legend_m.Draw("same");
        canvas.Print(outFileName);
    }


    // Step 3: Building our own R=1.0 jets from topoclusters
    if (!stepNum || stepNum >= 3)
    {
        // Leading ungroomed jet pT (without weights), original vs rebuilt
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_myungroom_pt_nw->SetLineColor(kBlue);
        hist_myungroom_pt_nw->SetLineWidth(2);
        scaleMeVToGeV(hist_myungroom_pt_nw);
        hist_myungroom_pt_nw->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_myungroom_pt_nw->GetYaxis()->SetTitle("Number of events");
        hist_myungroom_pt_nw->SetTitle("Leading ungroomed R=1.0 jet p_{T}, no weights");
        hist_myungroom_pt_nw->SetStats(0);
        hist_myungroom_pt_nw->Draw();

        hist_ungroom_pt_nw->Draw("same");

        TLegend legend_ungroom_nw(0.50,0.65,0.89,0.75);
        legend_ungroom_nw.AddEntry(hist_ungroom_pt_nw,"Original jets");
        legend_ungroom_nw.AddEntry(hist_myungroom_pt_nw,"Rebuilt jets");
        legend_ungroom_nw.SetBorderSize(0);
        legend_ungroom_nw.Draw("same");
        canvas.Print(outFileName);


        
        // Leading ungroomed jet pT (with weights), original vs rebuilt
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_myungroom_pt->SetLineColor(kBlue);
        hist_myungroom_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_myungroom_pt);
        hist_myungroom_pt->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_myungroom_pt->GetYaxis()->SetTitle("Weighted number of events");
        hist_myungroom_pt->SetTitle("Leading ungroomed R=1.0 jet p_{T}");
        hist_myungroom_pt->GetXaxis()->SetMoreLogLabels();
        hist_myungroom_pt->GetXaxis()->SetTitleOffset(1.25);
        hist_myungroom_pt->SetStats(0);
        hist_myungroom_pt->Draw();

        hist_ungroom_pt->Draw("same");

        TLegend legend_ungroom(0.50,0.65,0.89,0.75);
        legend_ungroom.AddEntry(hist_ungroom_pt,"Original jets");
        legend_ungroom.AddEntry(hist_myungroom_pt,"Rebuilt jets");
        legend_ungroom.SetBorderSize(0);
        legend_ungroom.Draw("same");
        canvas.Print(outFileName);



        // Leading trimmed jet pT (with weights), original vs rebuilt
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_mytrimmed_pt->SetLineColor(kBlue);
        hist_mytrimmed_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_mytrimmed_pt);
        hist_mytrimmed_pt->GetXaxis()->SetTitle("Jet p_{T} [GeV]");
        hist_mytrimmed_pt->GetYaxis()->SetTitle("Weighted number of events");
        hist_mytrimmed_pt->SetTitle("Leading trimmed R=1.0 jet p_{T}");
        hist_mytrimmed_pt->GetXaxis()->SetMoreLogLabels();
        hist_mytrimmed_pt->GetXaxis()->SetTitleOffset(1.25);
        hist_mytrimmed_pt->SetStats(0);
        hist_mytrimmed_pt->Draw();

        hist_trimmed_pt->SetLineColor(kRed);
        hist_trimmed_pt->Draw("same");

        TLegend legend_trimmed(0.50,0.65,0.89,0.75);
        legend_trimmed.AddEntry(hist_trimmed_pt,"Original jets");
        legend_trimmed.AddEntry(hist_mytrimmed_pt,"Rebuilt jets");
        legend_trimmed.SetBorderSize(0);
        legend_trimmed.Draw("same");
        canvas.Print(outFileName);
    }


    // Step 4: Building other types of R=1.0 jets from topoclusters
    if (!stepNum || stepNum >= 4)
    {
        // Leading jet pT plot
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_myungroom_pt->SetLineColor(kRed);
        hist_myungroom_pt->SetTitle("Leading R=1.0 jet p_{T}");
        hist_myungroom_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_myungroom_pt->GetXaxis()->SetMoreLogLabels();
        hist_myungroom_pt->GetXaxis()->SetTitleOffset(1.25);
        hist_myungroom_pt->SetStats(0);
        hist_myungroom_pt->Draw();

        hist_mytrimmed_pt->SetLineColor(kBlue);
        hist_mytrimmed_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_mytrimmed_pt->Draw("same");

        hist_mypruned_pt->SetLineColor(kGreen+2);
        hist_mypruned_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_mypruned_pt);
        hist_mypruned_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_mypruned_pt->Draw("same");

        hist_mySD_pt->SetLineColor(kViolet);
        hist_mySD_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_mySD_pt);
        hist_mySD_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_mySD_pt->Draw("same");

        hist_myRSD_pt->SetLineColor(kCyan);
        hist_myRSD_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_myRSD_pt);
        hist_myRSD_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_myRSD_pt->Draw("same");

        hist_myBUSD_pt->SetLineColor(kOrange+1);
        hist_myBUSD_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_myBUSD_pt);
        hist_myBUSD_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_myBUSD_pt->Draw("same");

        hist_myBUSDT_pt->SetLineColor(kBlack);
        hist_myBUSDT_pt->SetLineWidth(2);
        scaleMeVToGeV(hist_myBUSDT_pt);
        hist_myBUSDT_pt->GetXaxis()->SetRangeUser(150,1000);
        hist_myBUSDT_pt->Draw("same");

        TLegend legend_pt(0.50,0.5,0.89,0.75);
        legend_pt.AddEntry(hist_myungroom_pt,"Ungroomed jets");
        legend_pt.AddEntry(hist_mytrimmed_pt,"Trimmed jets");
        legend_pt.AddEntry(hist_mypruned_pt,"Pruned jets");
        legend_pt.AddEntry(hist_mySD_pt,"Soft Drop jets");
        legend_pt.AddEntry(hist_myRSD_pt,"Recursive Soft Drop jets");
        legend_pt.AddEntry(hist_myBUSD_pt,"Bottom-Up Soft Drop jets");
        legend_pt.AddEntry(hist_myBUSDT_pt,"Tighter Bottom-Up Soft Drop jets");
        legend_pt.SetBorderSize(0);
        legend_pt.Draw("same");
        canvas.Print(outFileName);



        // Leading jet mass plot
        canvas.SetLogx(true);
        canvas.SetLogy(true);
        hist_ungroom_m->Scale(1./hist_ungroom_m->Integral());
        hist_ungroom_m->SetLineColor(kRed);
        hist_ungroom_m->SetTitle("Leading R=1.0 jet mass, p_{T} > 400 GeV");
        hist_ungroom_m->GetYaxis()->SetTitle("Fraction of weighted events");
        hist_ungroom_m->GetXaxis()->SetRangeUser(10,500);
        hist_ungroom_m->GetXaxis()->SetMoreLogLabels();
        hist_ungroom_m->GetXaxis()->SetTitleOffset(1.25);
        hist_ungroom_m->GetYaxis()->SetRangeUser(1.e-4,2.5e-1);
        hist_ungroom_m->SetStats(0);
        hist_ungroom_m->Draw();

        hist_trimmed_m->Scale(1./hist_trimmed_m->Integral());
        hist_trimmed_m->SetLineColor(kBlue);
        hist_trimmed_m->GetXaxis()->SetRangeUser(10,500);
        hist_trimmed_m->Draw("same");

        hist_mypruned_m->Scale(1./hist_mypruned_m->Integral());
        hist_mypruned_m->SetLineColor(kGreen+2);
        hist_mypruned_m->SetLineWidth(2);
        scaleMeVToGeV(hist_mypruned_m);
        hist_mypruned_m->GetXaxis()->SetRangeUser(10,500);
        hist_mypruned_m->Draw("same");

        hist_mySD_m->Scale(1./hist_mySD_m->Integral());
        hist_mySD_m->SetLineColor(kViolet);
        hist_mySD_m->SetLineWidth(2);
        scaleMeVToGeV(hist_mySD_m);
        hist_mySD_m->GetXaxis()->SetRangeUser(10,500);
        hist_mySD_m->Draw("same");

        hist_myRSD_m->Scale(1./hist_myRSD_m->Integral());
        hist_myRSD_m->SetLineColor(kCyan);
        hist_myRSD_m->SetLineWidth(2);
        scaleMeVToGeV(hist_myRSD_m);
        hist_myRSD_m->GetXaxis()->SetRangeUser(10,500);
        hist_myRSD_m->Draw("same");

        hist_myBUSD_m->Scale(1./hist_myBUSD_m->Integral());
        hist_myBUSD_m->SetLineColor(kOrange+1);
        hist_myBUSD_m->SetLineWidth(2);
        scaleMeVToGeV(hist_myBUSD_m);
        hist_myBUSD_m->GetXaxis()->SetRangeUser(10,500);
        hist_myBUSD_m->Draw("same");

        hist_myBUSDT_m->Scale(1./hist_myBUSDT_m->Integral());
        hist_myBUSDT_m->SetLineColor(kBlack);
        hist_myBUSDT_m->SetLineWidth(2);
        scaleMeVToGeV(hist_myBUSDT_m);
        hist_myBUSDT_m->GetXaxis()->SetRangeUser(10,500);
        hist_myBUSDT_m->Draw("same");

        TLegend legend_m(0.12,0.12,0.52,0.37);
        legend_m.AddEntry(hist_ungroom_m,"Ungroomed jets");
        legend_m.AddEntry(hist_trimmed_m,"Trimmed jets");
        legend_m.AddEntry(hist_mypruned_m,"Pruned jets");
        legend_m.AddEntry(hist_mySD_m,"Soft Drop jets");
        legend_m.AddEntry(hist_myRSD_m,"Recursive Soft Drop jets");
        legend_m.AddEntry(hist_myBUSD_m,"Bottom-Up Soft Drop jets");
        legend_m.AddEntry(hist_myBUSDT_m,"Tighter Bottom-Up Soft Drop jets");
        legend_m.SetBorderSize(0);
        legend_m.Draw("same");
        canvas.Print(outFileName);
    }


    // Step 5: Calculating substructure variables for R=1.0 jets
    if (!stepNum || stepNum >= 5)
    {
        // Leading jet D2 plot
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_ungroom_D2->Scale(1./hist_ungroom_D2->Integral());
        hist_ungroom_D2->SetLineColor(kRed);
        hist_ungroom_D2->SetLineWidth(2);
        hist_ungroom_D2->GetXaxis()->SetTitle("Jet D_{2}^{#beta=1}");
        hist_ungroom_D2->GetYaxis()->SetTitle("Fraction of weighted events");
        hist_ungroom_D2->SetTitle("Leading R=1.0 jet D_{2}^{#beta=1}, p_{T} > 400 GeV");
        hist_ungroom_D2->GetYaxis()->SetRangeUser(0,0.2);
        hist_ungroom_D2->SetStats(0);
        hist_ungroom_D2->Draw();

        hist_trimmed_D2->Scale(1./hist_trimmed_D2->Integral());
        hist_trimmed_D2->SetLineColor(kBlue);
        hist_trimmed_D2->SetLineWidth(2);
        hist_trimmed_D2->Draw("same");

        hist_pruned_D2->Scale(1./hist_pruned_D2->Integral());
        hist_pruned_D2->SetLineColor(kGreen+2);
        hist_pruned_D2->SetLineWidth(2);
        hist_pruned_D2->Draw("same");

        hist_SD_D2->Scale(1./hist_SD_D2->Integral());
        hist_SD_D2->SetLineColor(kViolet);
        hist_SD_D2->SetLineWidth(2);
        hist_SD_D2->Draw("same");

        hist_RSD_D2->Scale(1./hist_RSD_D2->Integral());
        hist_RSD_D2->SetLineColor(kCyan);
        hist_RSD_D2->SetLineWidth(2);
        hist_RSD_D2->Draw("same");

        hist_BUSD_D2->Scale(1./hist_BUSD_D2->Integral());
        hist_BUSD_D2->SetLineColor(kOrange+1);
        hist_BUSD_D2->SetLineWidth(2);
        hist_BUSD_D2->Draw("same");

        hist_BUSDT_D2->Scale(1./hist_BUSDT_D2->Integral());
        hist_BUSDT_D2->SetLineColor(kBlack);
        hist_BUSDT_D2->SetLineWidth(2);
        hist_BUSDT_D2->Draw("same");

        TLegend legend_D2(0.50,0.64,0.89,0.89);
        legend_D2.AddEntry(hist_ungroom_D2,"Ungroomed jets");
        legend_D2.AddEntry(hist_trimmed_D2,"Trimmed jets");
        legend_D2.AddEntry(hist_pruned_D2,"Pruned jets");
        legend_D2.AddEntry(hist_SD_D2,"Soft Drop jets");
        legend_D2.AddEntry(hist_RSD_D2,"Recursive Soft Drop jets");
        legend_D2.AddEntry(hist_BUSD_D2,"Bottom-Up Soft Drop jets");
        legend_D2.AddEntry(hist_BUSDT_D2,"Tighter Bottom-Up Soft Drop jets");
        legend_D2.SetBorderSize(0);
        legend_D2.Draw("same");
        canvas.Print(outFileName);



        // Leading jet tau32 plot
        canvas.SetLogx(false);
        canvas.SetLogy(false);
        hist_ungroom_tau32->Scale(1./hist_ungroom_tau32->Integral());
        hist_ungroom_tau32->SetLineColor(kRed);
        hist_ungroom_tau32->SetLineWidth(2);
        hist_ungroom_tau32->GetXaxis()->SetTitle("Jet #tau_{32}^{WTA}");
        hist_ungroom_tau32->GetYaxis()->SetTitle("Fraction of weighted events");
        hist_ungroom_tau32->SetTitle("Leading R=1.0 jet #tau_{32}^{WTA}, p_{T} > 400 GeV");
        hist_ungroom_tau32->GetYaxis()->SetRangeUser(0,0.5);
        hist_ungroom_tau32->SetStats(0);
        hist_ungroom_tau32->Draw();

        hist_trimmed_tau32->Scale(1./hist_trimmed_tau32->Integral());
        hist_trimmed_tau32->SetLineColor(kBlue);
        hist_trimmed_tau32->SetLineWidth(2);
        hist_trimmed_tau32->Draw("same");

        hist_pruned_tau32->Scale(1./hist_pruned_tau32->Integral());
        hist_pruned_tau32->SetLineColor(kGreen+2);
        hist_pruned_tau32->SetLineWidth(2);
        hist_pruned_tau32->Draw("same");

        hist_SD_tau32->Scale(1./hist_SD_tau32->Integral());
        hist_SD_tau32->SetLineColor(kViolet);
        hist_SD_tau32->SetLineWidth(2);
        hist_SD_tau32->Draw("same");

        hist_RSD_tau32->Scale(1./hist_RSD_tau32->Integral());
        hist_RSD_tau32->SetLineColor(kCyan);
        hist_RSD_tau32->SetLineWidth(2);
        hist_RSD_tau32->Draw("same");

        hist_BUSD_tau32->Scale(1./hist_BUSD_tau32->Integral());
        hist_BUSD_tau32->SetLineColor(kOrange+1);
        hist_BUSD_tau32->SetLineWidth(2);
        hist_BUSD_tau32->Draw("same");

        hist_BUSDT_tau32->Scale(1./hist_BUSDT_tau32->Integral());
        hist_BUSDT_tau32->SetLineColor(kBlack);
        hist_BUSDT_tau32->SetLineWidth(2);
        hist_BUSDT_tau32->Draw("same");

        TLegend legend_tau32(0.15,0.64,0.54,0.89);
        legend_tau32.AddEntry(hist_ungroom_tau32,"Ungroomed jets");
        legend_tau32.AddEntry(hist_trimmed_tau32,"Trimmed jets");
        legend_tau32.AddEntry(hist_pruned_tau32,"Pruned jets");
        legend_tau32.AddEntry(hist_SD_tau32,"Soft Drop jets");
        legend_tau32.AddEntry(hist_RSD_tau32,"Recursive Soft Drop jets");
        legend_tau32.AddEntry(hist_BUSD_tau32,"Bottom-Up Soft Drop jets");
        legend_tau32.AddEntry(hist_BUSDT_tau32,"Tighter Bottom-Up Soft Drop jets");
        legend_tau32.SetBorderSize(0);
        legend_tau32.Draw("same");
        canvas.Print(outFileName);
    }


    // Close the output pdf file
    canvas.Print(outFileName+"]");

    return 0;
}

