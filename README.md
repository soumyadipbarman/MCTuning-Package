### MC Tuning

* This package extracts chi-sqaured values from the plots saved in pdf file
* Normalize the values in scale 1 -- 10
* Discards histograms which have more than one minima
* Find sensive zone for tuning

Software --
- Rivet
- Root
- Professor
- C++/Python code 

## To extract chi-sqaured values from the pdf file/plots

```
usage: python extract_chi2.py --input INPUT --output OUTPUT

optional arguments:
  -h, --help       show this help message and exit
  --input INPUT    Input PDF file, directory, or wildcard pattern
  --output OUTPUT  Output directory name (optional). If not given, auto-generated.

It will create to `txt' files named --
- chi2_values.txt -> consists of chi2 values from the pdf file
- chi2_histo_values.txt -> consists of chi2 values and the name of the histogram from the pdf file
```

## To normalize the chi-sqaured values (scale 1 -- 10) from the pdf file

```
Compile : g++ Normalize.C $(root-config --cflags --libs) -o normalize
Execute : ./normalize --input test/chi2_values.txt --output output.txt
Example : ./normalize --input chi2_values.txt --output normalized_output.txt

Usage:
  ./normalize --input input.txt --output output.txt
```

## To plot the normalized valus versus parameter for each distribution

```
Compile : g++ Plotter.C $(root-config --cflags --libs) -o plotter
Execute : ./plotter normalized.txt --parameter aLund Fragmentation function --mode combined/individual/refined --names chi2_histo_values.txt --output dir
Usage:
./plotter normalized.txt --parameter <name> --mode [combined|individual|refined] [--names file] [--output dir] [--range xmin xmax step] [--xvalues file]
- normalized.txt - normalized values
--parameter - name of the parameter in the x-axis
--mode - combined/individual/refined 
	- combined -> plots all the histograms in one canvas
	- individual -> plots the histograms individually on per canvas basis
	- refined -> discard plots which have more than one minima and plot the refined histograms in one canvas
-- names - name of the histograms with the chi-sqaured values (chi2_histo_values.txt)
--output - name of the output directory 
--range - x-axis_min_value x-axis_max_value step
--xvalues - `txt' file consists of bin-ranges
```
## Compare two refined directories

- Compare two directories of refined histograms -- e.g. refined-A and refined-B 
- specifically two parameters scan files
- Plot histograms from two refined directories in one canvas
- Use to decide the sensitivity zone and find range for tuning a parameter

```
Compile : g++ Compare.C $(root-config --cflags --libs) -o compare
Execute : ./compare --input1 refined-A --input2 refined-B --mode combined --output comp_out --xvalues scan_points.txt
Execute : ./compare --input1 refined-A --input2 refined-B --mode individual --output comp_out --range 0.0 1.0 0.1
Usage:
  ./compare --input1 dirA --input2 dirB --mode [combined|individual] --output outdir

Optional x-axis options:
  --xvaluesA file   OR   --rangeA xmin xmax step
  --xvaluesB file   OR   --rangeB xmin xmax step
```
## Filter histograms from the ipol.dat file which are sensitive that will be tuned

```
Compile : g++ Filter.C $(root-config --cflags --libs) -o filter
Usage:
./filter lists.dat refined-A/refined_plots.txt refined-B/refined_plots.txt output.dat
```
