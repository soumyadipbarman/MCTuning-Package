//Compile : g++ Plotter.C $(root-config --cflags --libs) -o plotter
//Run : ./plotter normalized.txt --parameter aLund Fragmentation function --mode combined/individual/refined --names chi2_histo_values.txt --output dir
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TStyle.h"

using namespace std;

// -------- count local minima --------
int countMinima(const vector<double>& v)
{
    int count = 0;
    for (int i = 1; i < (int)v.size()-1; i++)
        if (v[i] < v[i-1] && v[i] < v[i+1]) count++;
    return count;
}

// -------- read names file --------
vector<string> readNamesFile(const string& filename)
{
    vector<string> names;
    ifstream in(filename);

    if (!in.is_open()) {
        cout << "Warning: could not open names file " << filename << endl;
        return names;
    }

    string line;
    getline(in,line);
    getline(in,line);

    while (getline(in,line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string name;
        ss >> name;
        names.push_back(name);
    }
    return names;
}

// -------- clean plot name for legend --------
string cleanName(const string& name)
{
    string s = name;

    size_t pos = s.find_last_of("/\\");
    if (pos != string::npos)
        s = s.substr(pos + 1);

    if (s.size() >= 4 && s.substr(s.size()-4) == ".pdf")
        s = s.substr(0, s.size()-4);

    return s;
}

// -------- read x values from file --------
vector<double> readXValues(const string& filename)
{
    vector<double> vals;
    ifstream in(filename);

    if (!in.is_open()) {
        cout << "Error: cannot open x-values file " << filename << endl;
        return vals;
    }

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        double v;
        while (ss >> v)
            vals.push_back(v);

        if (!vals.empty()) break;
    }
    return vals;
}

// ------------------------------------
int main(int argc, char* argv[])
{
    if (argc < 5) {
        cout << "Usage:\n";
        cout << argv[0] << " normalized.txt --parameter <name> --mode [combined|individual|refined] "
             << "[--names file] [--output dir] [--range xmin xmax step] [--xvalues file]\n";
        return 1;
    }

    string inputFile = argv[1];
    string paramName = "";
    string modeValue = "";
    string namesFile = "";
    string outputDir = "";
    string xvaluesFile = "";

    double xmin=0, xmax=0, xstep=0;
    bool useRange = false;

    // -------- parse CLI --------
    for (int i=2; i<argc; i++) {
        string arg = argv[i];

        if (arg == "--parameter") {
            i++;
            while (i<argc && string(argv[i]).find("--") != 0) {
                if (!paramName.empty()) paramName += " ";
                paramName += argv[i];
                i++;
            }
            i--;
        }
        else if (arg == "--mode") {
            modeValue = argv[++i];
        }
        else if (arg == "--names") {
            namesFile = argv[++i];
        }
        else if (arg == "--output") {
            outputDir = argv[++i];
        }
        else if (arg == "--range") {
            xmin = atof(argv[++i]);
            xmax = atof(argv[++i]);
            xstep = atof(argv[++i]);
            useRange = true;
        }
        else if (arg == "--xvalues") {
            xvaluesFile = argv[++i];
        }
    }

    bool combinedMode   = (modeValue == "combined");
    bool individualMode = (modeValue == "individual");
    bool refinedMode    = (modeValue == "refined");

    // -------- create output directory --------
    if (outputDir != "") {
        mkdir(outputDir.c_str(), 0777);
    }

    auto makePath = [&](const string& fname) {
        if (outputDir == "") return fname;
        return outputDir + "/" + fname;
    };

    // -------- read normalized data --------
    ifstream in(inputFile);
    if (!in.is_open()) {
        cout << "Error opening " << inputFile << endl;
        return 1;
    }

    vector<vector<double>> data;
    string line;

    while (getline(in,line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        vector<double> row;
        double v;
        while (ss >> v) row.push_back(v);

        if (!row.empty()) data.push_back(row);
    }

    int nSets = data.size();
    int nData = data[0].size();

    // -------- load names --------
    vector<string> plotNames;
    if (namesFile != "")
        plotNames = readNamesFile(namesFile);

    // -------- build x axis --------
    vector<double> x(nData);
    vector<double> xFromFile;

    if (xvaluesFile != "") {
        xFromFile = readXValues(xvaluesFile);

        if ((int)xFromFile.size() != nData) {
            cout << "Error: x-values size mismatch\n";
            return 1;
        }
        x = xFromFile;
    }
    else if (useRange) {
        for (int i=0;i<nData;i++)
            x[i] = xmin + i*xstep;
    }
    else {
        for (int i=0;i<nData;i++)
            x[i] = i+1;
    }

    // -------- style --------
    gStyle->SetOptStat(0);

    vector<int> colors = {
        kRed, kBlue, kGreen+2, kMagenta, kOrange+7,
        kCyan+2, kViolet, kAzure+2, kPink+7, kTeal+3
    };
    int markers[] = {20,21,22,23,24,25,26,27,28,29};

    // ===========================
    // COMBINED MODE
    // ===========================
    if (combinedMode) {

        TCanvas *c = new TCanvas("c","Combined",900,700);
        TLegend *leg = new TLegend(0.65,0.7,0.88,0.88);

        for (int i=0;i<nSets;i++) {

            int col = colors[i % colors.size()];
            TGraph *gr = new TGraph(nData, &x[0], &data[i][0]);

            gr->SetLineColor(col);
            gr->SetMarkerColor(col);
            gr->SetMarkerStyle(markers[i%10]);
            gr->SetLineWidth(2);

            if (i==0) {
                gr->Draw("ALP");
                gr->GetXaxis()->SetTitle(paramName.c_str());
                gr->GetYaxis()->SetTitle("Normalized #chi^{2}");
                gr->GetYaxis()->SetRangeUser(0,10);

                if (!xFromFile.empty())
                    gr->GetXaxis()->SetLimits(x.front(), x.back());
                else if (useRange)
                    gr->GetXaxis()->SetLimits(xmin, xmax);
            }
            else {
                gr->Draw("LP SAME");
            }

            string pname = (i < plotNames.size()) ? plotNames[i] : Form("Set_%d",i+1);
            leg->AddEntry(gr, cleanName(pname).c_str(), "lp");
        }

        leg->Draw();
        c->SaveAs(makePath("combined_plot.pdf").c_str());
    }

    // ===========================
    // INDIVIDUAL MODE
    // ===========================
    if (individualMode) {

        for (int i=0;i<nSets;i++) {

            int col = colors[i % colors.size()];
            string pname = (i < plotNames.size()) ? plotNames[i] : Form("plot_set_%d",i+1);

            TCanvas *c = new TCanvas(Form("c_%d",i),"",800,600);
            TGraph *gr = new TGraph(nData, &x[0], &data[i][0]);

            gr->SetLineColor(col);
            gr->SetMarkerColor(col);
            gr->SetMarkerStyle(markers[i%10]);
            gr->SetLineWidth(2);

            gr->Draw("ALP");
            gr->GetXaxis()->SetTitle(paramName.c_str());
            gr->GetYaxis()->SetTitle("Normalized #chi^{2}");
            gr->GetYaxis()->SetRangeUser(0,10);

            if (!xFromFile.empty())
                gr->GetXaxis()->SetLimits(x.front(), x.back());
            else if (useRange)
                gr->GetXaxis()->SetLimits(xmin, xmax);

            TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);
            leg->AddEntry(gr, cleanName(pname).c_str(), "lp");
            leg->Draw();

            string fname = pname;
            size_t pos = fname.find_last_of("/\\");
            if (pos != string::npos) fname = fname.substr(pos + 1);
            if (fname.size() < 4 || fname.substr(fname.size()-4) != ".pdf")
                fname += ".pdf";

            c->SaveAs(makePath(fname).c_str());
        }
    }

    // ===========================
    // REFINED MODE
    // ===========================
    if (refinedMode) {

        TCanvas *c = new TCanvas("c_refined","Refined",900,700);
        TLegend *leg = new TLegend(0.65,0.7,0.88,0.88);

        ofstream outNames(makePath("refined_plots.txt"));
        ofstream outVals(makePath("refined_normalize_values.txt"));

        int counter = 0;

        for (int i=0;i<nSets;i++) {

            if (countMinima(data[i]) != 1) continue;

            int col = colors[counter % colors.size()];
            TGraph *gr = new TGraph(nData, &x[0], &data[i][0]);

            gr->SetLineColor(col);
            gr->SetMarkerColor(col);
            gr->SetMarkerStyle(markers[counter%10]);
            gr->SetLineWidth(2);

            if (counter==0) {
                gr->Draw("ALP");
                gr->GetXaxis()->SetTitle(paramName.c_str());
                gr->GetYaxis()->SetTitle("Normalized #chi^{2}");
                gr->GetYaxis()->SetRangeUser(0,10);

                if (!xFromFile.empty())
                    gr->GetXaxis()->SetLimits(x.front(), x.back());
                else if (useRange)
                    gr->GetXaxis()->SetLimits(xmin, xmax);
            }
            else {
                gr->Draw("LP SAME");
            }

            string pname = (i < plotNames.size()) ? plotNames[i] : Form("Set_%d",i+1);

            leg->AddEntry(gr, cleanName(pname).c_str(), "lp");
            outNames << pname << endl;

            for (int j=0;j<nData;j++) {
                outVals << data[i][j];
                if (j != nData-1) outVals << " ";
            }
            outVals << endl;

            counter++;
        }

        leg->Draw();
        c->SaveAs(makePath("refined_plot.pdf").c_str());

        outNames.close();
        outVals.close();

        cout << "Saved refined plot list → " << makePath("refined_plots.txt") << endl;
        cout << "Saved refined normalized values → " << makePath("refined_normalize_values.txt") << endl;
    }

    return 0;
}
