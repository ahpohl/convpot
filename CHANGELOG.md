# Convpot changelog

## v1.2.1 - 2017-11-01
* test dQ/dV and efficiency for "not a number"
* calculate average current over all data points
* use average current for half cycle step index
* windows installation possible as non-admin user

## v1.2.0 - 2017-10-22
* verbose option can be given multiple times
* do full cell calculations
* insert step index into half cycle table
* calculate average current per cycle
* add device and loading columns to global table

## v1.1.7 - 2017-09-30
* fix linux compilation
* upload PKGBUILD to arch linux user repo

## v1.1.1 - 2017-08-27
* add file name and creation date columns to global table
* fix typo in short option argument (-m)

## v1.1.0 - 2017-08-09
* cpack: NSIS windows installer
* cmake: insert git version during build
* api: merging files now with --merge option (previoulsy --file)

## v1.0.1 - 2017-08-01
* first public release
* cmake: simple zip archive
