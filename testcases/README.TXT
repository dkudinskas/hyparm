These testcase will give incorrect output on original hypervisor but they will work on the hypervisor supporting block copy functionality.  Using the make file you can get uImages.  They can be tested by sending them manually to the beagleboard.  Using jta-hyparm you can use following commands:

fatload mmc 0:1 80300000 uImageX    # X has to be replaced by number 1 or 2
loadlzma
go 8c000000 -kernel 80300000 -initrd 81600000  
