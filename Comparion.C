#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>

#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TStyle.h"

using namespace std;

// ============================================================
// Print usage
// ============================================================
void printUsage()
{
    cout << "\nUsage:\n";
    cout << "  ./compare --input1 dirA --input2 dirB --mode [combined|individual] --output outdir\n\n";
    cout << "Optional x-axis options:\n";
    cout << "  --xvaluesA file   OR   --rangeA xmin xmax step\n";
    cout << "  --xvaluesB file   OR   --rangeB xmin xmax step\n\n";
}

// ============================================================
// Clean histogram name
// ============================================================
string cleanName(const string& name)
{
    string s = name;
    size_t pos = s.find_last_of("/\\");
    if (pos != string::npos) s = s.substr(pos+1);

    if (s.size() >= 4 && s.substr(s.size()-4) == ".pdf")
        s = s.substr(0, s.size()-4);

    return s;
}

// ============================================================
// Read normalized values
// ============================================================
vector<vector<double>> readNormalized(const string& file)
{
    vector<vector<double>> data;
    ifstream in(file);

    if (!in.is_open()) {
        cerr << "ERROR: Cannot open file " << file << endl;
        return data;
    }

    string line;
    while (getline(in,line)) {
        if (line.empty()) continue;
        stringstream ss(line);

        vector<double> row;
        double v;
        while (ss >> v) row.push_back(v);

        if (!row.empty()) data.push_back(row);
    }
    return data;
}

// ============================================================
// Read plot names
// ============================================================
vector<string> readNames(const string& file)
{
    vector<string> names;
    ifstream in(file);

    if (!in.is_open()) {
        cerr << "ERROR: Cannot open file " << file << endl;
        return names;
    }

    string line;
    while (getline(in,line)) {
        if (line.empty()) continue;
        names.push_back(line);
    }
    return names;
}

// ============================================================
// Read x values
// ============================================================
vector<double> readXValues(const string& file)
{
    vector<double> vals;
    ifstream in(file);

    if (!in.is_open()) {
        cerr << "ERROR: Cannot open x-values file " << file << endl;
        return vals;
    }

    string line;
    while (getline(in,line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        double v;
        while (ss >> v) vals.push_back(v);

        if (!vals.empty()) break;
    }
    return vals;
}

// ============================================================
// MAIN
// ============================================================
int main(int argc, char* argv[])
{
    // -------- no arguments --------
    if (argc == 1) {
        printUsage();
        return 0;
    }

    string dir1="", dir2="", outdir="", mode="";
    string xfileA="", xfileB="";
    double xminA=0,xmaxA=0,stepA=0;
    double xminB=0,xmaxB=0,stepB=0;
    bool useRangeA=false, useRangeB=false;

    // ======================================================
    // Parse CLI safely
    // ======================================================
    for (int i=1;i<argc;i++) {

        string arg = argv[i];

        if (arg == "--help") {
            printUsage();
            return 0;
        }

        else if (arg == "--input1" && i+1<argc) dir1 = argv[++i];
        else if (arg == "--input2" && i+1<argc) dir2 = argv[++i];
        else if (arg == "--output" && i+1<argc) outdir = argv[++i];
        else if (arg == "--mode" && i+1<argc) mode = argv[++i];

        else if (arg == "--xvaluesA" && i+1<argc) xfileA = argv[++i];
        else if (arg == "--xvaluesB" && i+1<argc) xfileB = argv[++i];

        else if (arg == "--rangeA" && i+3<argc) {
            xminA = atof(argv[++i]);
            xmaxA = atof(argv[++i]);
            stepA = atof(argv[++i]);
            useRangeA = true;
        }
        else if (arg == "--rangeB" && i+3<argc) {
            xminB = atof(argv[++i]);
            xmaxB = atof(argv[++i]);
            stepB = atof(argv[++i]);
            useRangeB = true;
        }
        else {
            cerr << "Unknown or incomplete argument: " << arg << endl;
            printUsage();
            return 1;
        }
    }

    // -------- required checks --------
    if (dir1.empty() || dir2.empty() || outdir.empty() || mode.empty()) {
        cerr << "ERROR: Missing required arguments\n";
        printUsage();
        return 1;
    }

    if (mode != "combined" && mode != "individual") {
        cerr << "ERROR: mode must be 'combined' or 'individual'\n";
        return 1;
    }

    mkdir(outdir.c_str(),0777);

    // ======================================================
    // Read input data
    // ======================================================
    vector<vector<double>> dataA = readNormalized(dir1+"/refined_normalize_values.txt");
    vector<vector<double>> dataB = readNormalized(dir2+"/refined_normalize_values.txt");

    vector<string> namesA = readNames(dir1+"/refined_plots.txt");
    vector<string> namesB = readNames(dir2+"/refined_plots.txt");

    if (dataA.empty() || dataB.empty()) {
        cerr << "ERROR: One of the normalized files is empty\n";
        return 1;
    }

    // ======================================================
    // Build x axes
    // ======================================================
    vector<double> xA(dataA[0].size());
    vector<double> xB(dataB[0].size());

    if (!xfileA.empty()) xA = readXValues(xfileA);
    else if (useRangeA)
        for (int i=0;i<(int)xA.size();i++) xA[i] = xminA + i*stepA;
    else
        for (int i=0;i<(int)xA.size();i++) xA[i] = i+1;

    if (!xfileB.empty()) xB = readXValues(xfileB);
    else if (useRangeB)
        for (int i=0;i<(int)xB.size();i++) xB[i] = xminB + i*stepB;
    else
        for (int i=0;i<(int)xB.size();i++) xB[i] = i+1;

    gStyle->SetOptStat(0);

    // ======================================================
    // COMBINED MODE
    // ======================================================
    if (mode=="combined") {

        TCanvas *c = new TCanvas("c","Combined",1000,700);
        TLegend *leg = new TLegend(0.60,0.60,0.88,0.88);

        vector<int> colors = {kRed, kBlue, kGreen+2, kMagenta, kOrange+7,
                              kCyan+2, kViolet, kAzure+2, kPink+7};

        bool first=true;

        // A curves
        for (int i=0;i<(int)dataA.size();i++) {
            int col = colors[i % colors.size()];

            TGraph *g = new TGraph(xA.size(), &xA[0], &dataA[i][0]);
            g->SetLineColor(col);
            g->SetLineWidth(2);

            if (first) {
                g->Draw("AL");
                g->GetXaxis()->SetTitle("Scan parameter");
                g->GetYaxis()->SetTitle("Normalized #chi^{2}");
                g->GetYaxis()->SetRangeUser(0,10);
                first=false;
            } else g->Draw("L SAME");

            leg->AddEntry(g, (cleanName(namesA[i])+" (A)").c_str(),"l");
        }

        // B curves
        for (int i=0;i<(int)dataB.size();i++) {
            int col = colors[i % colors.size()];

            TGraph *g = new TGraph(xB.size(), &xB[0], &dataB[i][0]);
            g->SetLineColor(col);
            g->SetLineStyle(2);
            g->SetLineWidth(2);

            g->Draw("L SAME");
            leg->AddEntry(g, (cleanName(namesB[i])+" (B)").c_str(),"l");
        }

        leg->Draw();
        c->SaveAs((outdir+"/comparison_combined.pdf").c_str());
    }

    // ======================================================
    // INDIVIDUAL MATCHED MODE
    // ======================================================
    if (mode=="individual") {

        map<string,int> mapB;
        for (int j=0;j<(int)namesB.size();j++)
            mapB[namesB[j]] = j;

        for (int i=0;i<(int)namesA.size();i++) {

            string name = namesA[i];

            if (mapB.find(name)==mapB.end()) continue;

            int j = mapB[name];

            TCanvas *c = new TCanvas("c","Compare",900,700);
            TLegend *leg = new TLegend(0.65,0.7,0.88,0.88);

            TGraph *gA = new TGraph(xA.size(), &xA[0], &dataA[i][0]);
            gA->SetLineColor(kRed);
            gA->SetLineWidth(2);

            TGraph *gB = new TGraph(xB.size(), &xB[0], &dataB[j][0]);
            gB->SetLineColor(kBlue);
            gB->SetLineStyle(2);
            gB->SetLineWidth(2);

            gA->Draw("AL");
            gA->GetXaxis()->SetTitle("Scan parameter");
            gA->GetYaxis()->SetTitle("Normalized #chi^{2}");
            gA->GetYaxis()->SetRangeUser(0,10);

            gB->Draw("L SAME");

            leg->AddEntry(gA,"A","l");
            leg->AddEntry(gB,"B","l");
            leg->Draw();

            string outname = outdir + "/" + cleanName(name) + ".pdf";
            c->SaveAs(outname.c_str());
        }
    }

    return 0;
}
