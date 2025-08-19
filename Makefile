include Make.config
DESTDIR = ~/HostFileBdos/c/
DESTDIR1 = /var/www/html/
SUM = sum
CP = cp

all: scsitest

scsi_test.o: scsi_test.c scsi.h
	zcc $(CFLAGS) $(STORE) -c scsi_test.c

scsitest: scsi_test.o
	zcc  $(LINKOP) -oscsitest -Wunused scsi_test.o
	$(SUM) SCSITEST.COM

check:
	$(SUM) *.COM

documents:
	doxygen 
