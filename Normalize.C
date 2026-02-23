//Compile : g++ Normalize.C $(root-config --cflags --libs) -o normalize
//Run : ./normalize extract_output/chi2_values.txt normalized_output.txt
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

// -------- print usage --------
void printUsage(const char* prog)
{
    cout << "\nUsage:\n";
    cout << "  " << prog << " --input input.txt --output output.txt\n\n";
    cout << "Description:\n";
    cout << "  Reads chi2 values and normalizes each set between 0 and 10.\n\n";
}

// ------------------------------------
int main(int argc, char* argv[])
{
    string inputFile = "";
    string outputFile = "";

    // -------- parse CLI --------
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--input") {
            if (i + 1 < argc) inputFile = argv[++i];
        }
        else if (arg == "--output") {
            if (i + 1 < argc) outputFile = argv[++i];
        }
        else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else {
            cout << "Unknown argument: " << arg << endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // -------- validate input --------
    if (inputFile.empty() || outputFile.empty()) {
        cout << "Error: Missing required arguments.\n";
        printUsage(argv[0]);
        return 1;
    }

    ifstream in(inputFile);
    ofstream out(outputFile);

    if (!in.is_open()) {
        cout << "Error: Cannot open input file " << inputFile << endl;
        return 1;
    }

    if (!out.is_open()) {
        cout << "Error: Cannot open output file " << outputFile << endl;
        return 1;
    }

    // ---------- read header ----------
    int nSets, nData;
    in >> nSets;   // number of PDF files (sets)
    in >> nData;   // number of chi2 per PDF

    cout << "Number of sets  = " << nSets << endl;
    cout << "Data per set    = " << nData << endl;

    vector<float> ChiSquare(nData);
    vector<float> normalized(nData);

    // ---------- loop over sets ----------
    for (int iSets = 0; iSets < nSets; iSets++)
    {
        // read one line of chi2 values
        for (int i = 0; i < nData; i++) {
            in >> ChiSquare[i];
        }

        float min = ChiSquare[0];
        float max = ChiSquare[0];

        for (int i = 1; i < nData; i++) {
            if (ChiSquare[i] < min) min = ChiSquare[i];
            if (ChiSquare[i] > max) max = ChiSquare[i];
        }

        cout << "Set " << iSets << " : Min = " << min << "  Max = " << max << endl;

        // ---------- normalize between 0 and 10 ----------
        for (int i = 0; i < nData; i++) {

            if (max != min)
                normalized[i] = 10.0f * (ChiSquare[i] - min) / (max - min);
            else
                normalized[i] = 0.0f;  // avoid division by zero

            out << normalized[i];

            if (i != nData - 1) out << " ";
        }

        out << endl;
    }

    in.close();
    out.close();

    cout << "\nNormalized output written to: " << outputFile << endl;

    return 0;
}
