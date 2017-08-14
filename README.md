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

1. Download the latest [convpot installer](https://github.com/ahpohl/convpot/releases/latest) from GitHub.
1. Run the installer ``convpot-x.y.z-win32.exe`` and choose the option to add Convpot to the PATH variable for the current user or system-wide.
1. Navigate to the Convpot menu entry and click on ``Run Convpot``.
1. A Cmd window opens. You can now run one of the examples:
```
convpot arbintest.res
```
If all goes well you should now have the file ``arbintest.sqlite`` created.

### Compiling from source

For compiling from source see the [wiki pages](https://github.com/ahpohl/convpot/wiki).

## Usage

### Single file

To process a single input file

```
convpot arbintest.res
```

The output is the corresponding SQLite database containing the raw data. You can use the --output option to give a different filename.

### Merge files

To process multiple files

```
convpot arbintest.res gamrytest.DTA
```

Alternatively, the files to merge can be given in a text file listed one by line. Lines starting with the "!" character are ignored.

```
convpot --merge files_to_merge.txt
```

### Command-line Options

* **--help or -h** - show convpot help
* **--version or -V** - show the version header
* **--verbose or -v** - print verbose output, can be given multiple times to increase verbosity
* **--info or -i** - display supported instruments
* **--timer** - benchmark the program run time, useful for very large files
* **--output FILE** - give alternative output filename. The default is the name of the first input file
* **--merge FILE** - a file with filenames to merge one by line. A "!" denotes a comment.
* **--smooth LEVEL** - smooth current and voltage data. Useful for dQ/dV plots which show artefacts due to noise.

## Authors

* **Alexander Pohl** - *Initial work*

See also the list of [CONTRIBUTORS](https://github.com/ahpohl/convpot/blob/master/CONTRIBUTORS.md) who participated in this project.

## Changelog

All notable changes and releases are documented in the [CHANGELOG](https://github.com/ahpohl/convpot/blob/master/CHANGELOG.md).

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details
