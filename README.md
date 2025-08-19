# s100-scsi
s100 bus scsi bus driver project.

This will consist of 2 parts.   A test program for debugging the scsi bus hardware and a
scsi bus driver for booting and running CP/M 3

CP/M 2.2 is covered by the Joe Lang project 
```
git clone https://github.com/joelang/z80-sbc.git
```

I will be including the prebuilt code for those that do not want to learn how to run
z88dk.  It's called SCSITEST.COM 

##TO BUILD z88dk
```
git clone --recursive https://github.com/z88dk/z88dk.git
cd z88dk
./build.sh
```
<< a long time latter >>
```
export PATH=`pwd`/bin:$PATH

CFG=`pwd`/lib/config/
case "`uname -s`" in
    CYGWIN*)    export ZCCCFG=`cygpath -m $CFG`;;
    *)          export ZCCCFG=$CFG ;;
esac
```
##TO BUILD the SCSI Project

cd to the a place to build
```
git clone https://github.com/jayacotton/s100-scsi.git
cd s100-scsi
make
```

INSTALLING TEST CODE

upload SCSITEST.COM to your test system.
