read rtl vending-fixed.v
blast ntk
set solver -d
set safety -n p0 1
ver pdr p0 
usage
q -f
