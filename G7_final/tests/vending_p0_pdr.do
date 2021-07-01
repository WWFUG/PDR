read rtl vending-fixed.v
blast ntk
set solver -d
set safety -n p0 0
ver pdr p0 
usage
q -f
