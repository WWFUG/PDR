The submitted directory trees is as below
.
├── COPYING
├── design
├── doc
├── engine
│   ├── boolector
│   │   ├── build.sh
│   │   ├── clean.sh
│   │   ├── makefile
│   │   ├── minisat
│   │   │   ├── CMakeLists.txt
│   │   │   ├── doc
│   │   │   │   └── ReleaseNotes-2.2.0.txt
│   │   │   ├── LICENSE
│   │   │   ├── Makefile
│   │   │   ├── minisat
│   │   │   └── README
│   │   ├── README
│   │   └── v3_src
│   ├── boolector.script
│   ├── minisat
│   ├── minisat.script
│   ├── quteRTL
│   └── quteRTL.script
├── fmcad11_pdr.pdf
├── include
├── Makefile
├── README.md
├── README.txt
├── ref
│   ├── pdrv_ref
│   └── result.xlsx
├── report.pdf
├── SoCV_Final_PDR.doc
├── SoCV_Final_PDR.pdf
├── src
└── tests
    ├── basic
    ├── hwmcc
    ├── vending.dofile
    ├── vending-fixed.v
    ├── vending_nitp.dofile
    ├── vending_p0.do
    ├── vending_p0_pdr.do
    ├── vending_p1.do
    ├── vending_p1_pdr.do
    ├── vending_p2.do
    ├── vending_p2_pdr.do
    └── vending.v

The main program is under the src/PDR directory. 

To compile
 - $ make clean  
 - $ make


