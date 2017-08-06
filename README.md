# Convpot

Convpot is a converter program written in C++ which converts raw data from potentiostats into a sqlite database ready to be plotted by [plotpot.py](https://github.com/ahpohl/plotpot). Convpot is automatically called by plotpot.py, but can also be called on its own for example to merge files. 

## Supported instruments

* Arbin Instruments BT2000
* Gamry Interface 1000
* Biologic VMP3
* Ivium CompactStat
* Zahner IM6

## Getting Started

### Installing

Follow these instructions (for Windows):

1. Download the latest convpot [release](https://github.com/ahpohl/convpot/releases/latest).
1. Extract the zip archive to `C:\Program Files (x86)\convpot`.
1. Add the folder `C:\Program Files (x86)\convpot\convpot-x.y.z-win32\bin` to the PATH variable for the current user.
1. Open a CMD window by typing `cmd` into the "Search programs and files" start menu field.
1. Change to the examples folder and run a test:
```
cd C:\Program Files (x86)\convpot\convpot-x.y.z-win32\examples
convpot arbintest.res
```
If all goes well you should now have the file `arbintest.sqlite` created.

### Compiling from source

For compiling from source see the [wiki pages](https://github.com/ahpohl/convpot/wiki).

## Usage

### Single files

To process a single input file

`convpot arbintest.res`

The output is the corresponding SQLite database containing the raw data. You can use the --output option to give a different filename.

### Merging files

To process multiple files

`convpot arbintest.res gamrytest.dta`

Alternatively, the files to merge can be given in a text file listed one by line. Lines starting with the "!" character are ignored.

`convpot --file files_to_merge.txt`

## Authors

* **Alexander Pohl** - *Initial work*

See also the list of [CONTRIBUTORS](https://github.com/ahpohl/convpot/blob/master/CONTRIBUTORS.md) who participated in this project.

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details
