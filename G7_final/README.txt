The submitted directory trees is as below
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
├── include
├── Makefile
├── README.txt
├── report.pdf
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


