#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>

using namespace std;

// ----------------------------------
// Extract only histogram ID
// Remove path, remove .pdf/.pdf.pdf
// Example:
//   "/ATLAS/.../d01-x01-y01.pdf" -> "d01-x01-y01"
// ----------------------------------
string extractHistID(string name)
{
    // remove leading/trailing spaces
    while (!name.empty() && isspace(name.front())) name.erase(0,1);
    while (!name.empty() && isspace(name.back()))  name.pop_back();

    // remove path (keep only last token)
    size_t pos = name.find_last_of("/\\");
    if (pos != string::npos)
        name = name.substr(pos + 1);

    // remove ".pdf" extension (once or twice)
    for (int i=0;i<2;i++) {
        if (name.size() >= 4 && name.substr(name.size()-4) == ".pdf")
            name = name.substr(0, name.size()-4);
    }

    return name;
}

// ----------------------------------
// Read refined plots list and store IDs
// ----------------------------------
set<string> readRefinedList(const string& filename)
{
    set<string> names;

    ifstream in(filename);
    if (!in.is_open()) {
        cout << "Error opening " << filename << endl;
        return names;
    }

    string line;

    while (getline(in,line)) {
        if (line.empty()) continue;

        string id = extractHistID(line);
        names.insert(id);
    }

    return names;
}

// ----------------------------------
// Extract histogram ID from lists.dat line
// ----------------------------------
string extractIDfromDatLine(const string& line)
{
    stringstream ss(line);
    string fullName;
    ss >> fullName;   // first column
    return extractHistID(fullName);
}

// ----------------------------------
int main(int argc, char* argv[])
{
    if (argc < 5) {
        cout << "\nUsage:\n";
        cout << argv[0] << " lists.dat refined-A/refined_plots.txt refined-B/refined_plots.txt output.dat\n\n";
        return 1;
    }

    string inputDat  = argv[1];
    string refinedA  = argv[2];
    string refinedB  = argv[3];
    string outputDat = argv[4];

    // ---------- read refined sets ----------
    set<string> setA = readRefinedList(refinedA);
    set<string> setB = readRefinedList(refinedB);

    // ---------- union ----------
    set<string> keepIDs = setA;
    keepIDs.insert(setB.begin(), setB.end());

    cout << "Refined-A IDs: " << setA.size() << endl;
    cout << "Refined-B IDs: " << setB.size() << endl;
    cout << "Union IDs    : " << keepIDs.size() << endl;

    // ---------- open input/output ----------
    ifstream in(inputDat);
    ofstream out(outputDat);

    if (!in.is_open()) {
        cout << "Error opening input dat file\n";
        return 1;
    }

    string line;
    int total = 0;
    int kept  = 0;

    // ---------- filter ----------
    while (getline(in,line)) {

        if (line.empty()) continue;

        string id = extractIDfromDatLine(line);

        if (keepIDs.find(id) != keepIDs.end()) {
            out << line << endl;
            kept++;
        }

        total++;
    }

    // ---------- summary ----------
    cout << "Total lines read : " << total << endl;
    cout << "Total lines kept : " << kept << endl;
    cout << "Saved to         : " << outputDat << endl;

    return 0;
}
