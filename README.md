# Convpot

Convpot is a converter program written in C++ which converts raw data from potentiostats into a sqlite database ready to be plotted by [plotpot.py](https://github.com/ahpohl/plotpot). Convpot is automatically called by plotpot.py, but can also be called on its own for example to merge files. 

## Supported instruments

* Arbin Instruments BT2000 (\*.res)
* Gamry Interface 1000 (\*.DTA)
* Biologic VMP3 (\*.mpt)
* Ivium CompactStat (\*.idf)
* Zahner IM6 (\*.txt)

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Compiling from source

For compiling from source see the [wiki pages](https://github.com/ahpohl/convpot/wiki).

### Installing

A step by step series of examples that tell you have to get a development env running

## Usage

### Single files

To process a single input file

´convpot arbintest.res´

The output is the corresponding SQLite database containing the raw data. You can use the --output option to give a different filename.

### Merging files

To process multiple files

´convpot arbintest.res gamrytest.dta´

Alternatively, the files to merge can be given in a text file listed one by line. Lines starting with the "!" character are ignored.

´convpot --file files_to_merge.txt´

## Deployment

Add additional notes about how to deploy this on a live system

## Contributing

Please read [CONTRIBUTING.md](https://github.com/ahpohl/convpot/blob/master/CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **Alexander Pohl** - *Initial work*

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details
