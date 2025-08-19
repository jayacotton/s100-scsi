/*
	SCSI TEST for S100 SCSI interface board.

	The board is based on the LifeLine SCSI board design.

	A lot of inspiration from Joe Lang for the low level driver code.


	R - Read a block from the disk.  
		needs Q,L first
	W - Write a block to the disk.
		needs Q,L and C first
	L - Compute the LBA for the disk I/O operation.
	C - Create a block of data for the W command
	S - Read and decode the Controller and Disk status.
	P - Peek at the register set.
	O - Poke at the register set.
	I - Initialize the Controller.
	Q - Setup the disk for I/O.
		also set physical and logical disk number
	B - Boot the CPMLOADER from the disk.
	G - Go to the CPMLOADER
	H - Print help banner

 08/17/25  JCotton Started coding the first version.



Build instructions.

Using z88dk running on ubuntu.

cd [build directory]
make

When done you should find SCSITEST.COM, a CP/M program.  

Transfer to the test system with the convenient transfer program
and run it.

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "scsi.h"

#define STARTSTRING "SCSI test version\0"
#define VERSION "0.1\0"

extern void ScsiPhase ();
extern void dumpstat ();
unsigned int msg_in;
unsigned int msg_out;
unsigned int scsistat;
unsigned char buffa[128];
unsigned char buffb[128];
unsigned char bits[8] = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
unsigned char swapbits[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
CDB CmdDesc = { 8, 0, 1, 13, 0 };
SIOT ScsiIO =
  { 0, 0, &CmdDesc, &scsistat, &buffa, &buffb, &msg_in, &msg_out };
unsigned char
DecodePhase (status)
{
  unsigned char phase;

  phase = swapbits[status & 7];
  printf ("Phase: ");
  switch (phase)
    {
    case 0:
      printf ("Data Out\n");
      break;
    case 1:
    case 5:
      printf ("Unspecified\n");
      break;
    case 2:
      printf ("Command\n");
      break;
    case 3:
      printf ("Message Out\n");
      break;
    case 4:
      printf ("Data in\n");
      break;
    case 6:
      printf ("Status\n");
      break;
    case 7:
      printf ("Message in\n");
      break;
    default:
      break;
    }
  return phase;
}

void
one ()
{
  printf ("1");
}

void
zero ()
{
  printf ("0");
}

void
showbits (unsigned char status)
{
  int i;
  for (i = 0; i <= 7; i++)
    {
      if (status & bits[i])
	one ();
      else
	zero ();
    }
}

void
help ()
{
  printf ("\n\n BOOT 	- Boot the CPMLOADER from the disk.\n");
  printf (" CREATE - Create a block of data for the W command\n");
  printf (" GOTO 	- Go to the CPMLOADER\n");
  printf (" HELP 	- Print help banner.\n");
  printf (" INIT 	- Initialize the Controller.\n");
  printf (" LBA 	- Compute the LBA for the disk I/O operation.\n");
  printf (" O 	- Poke at the register set.\n");
  printf (" PEEK 	- Peek at the register set.\n");
  printf (" QUEUE 	- Setup the disk for I/O.\n");
  printf (" READ 	- Read a block from the disk.\n");
  printf (" STATUS - Read and decode the Controller and Disk status.\n");
  printf (" WRITE 	- Write a block to the disk.\n");
  printf (" X 	- Leave program\n");
}

int IOphase;


// read a block of data from the disk.  The block size for
// CP/M is 128 bytes (could be 512 latter).

// The read routine will loop reading bytes and checking the phase
// Note that this is recursive and may cause a stack over flow.
//  As long as the phase is data in, we will stay in this code
//  but the initiator can change the phase at any time so we need
//  to monitor that while reading.  I assume that a phase change
//  while reading a block is fatal and we end up at the main menu.

void
s_read (void *ptr)
{
  while (1);
  {
    do
      {
	scsistat = inp (sr_csbs);	// check bsy wait for req
	if (inp (sr_bsr) & sm_phm)
	  {
	    ScsiPhase ();	// recursive call
	    return;
	  }
      }
    while (!(scsistat & sm_bsy));
    if (inp (sr_bsr) & sm_phm)
      {
	ScsiPhase ();		// recursive call
	return;
      }
    // get a byte and store it
    *ScsiIO.p_inbuf++ = inp (sr_csbs);	// get a byte
    outp (sr_icr, 0x10);	// set ack
    do
      {
	scsistat = inp (sr_csbs);
      }
    while (!(scsistat & sm_req));
    outp (sr_icr, 0);		// drop ack
  }
}

void
s_write (void *ptr)
{
}

void
ScsiPhase ()
{
  do
    {
      outp (sr_icr, 0);		// reset initiator
      scsistat = inp (sr_csbs);	// get bus status
      if (scsistat & sm_bsy)
	return;
    }
  while (scsistat & sm_req);
  IOphase = swapbits[scsistat & 7];
  switch (IOphase)
    {
    case 0:
      s_write (ScsiIO.p_outbuf);
      break;
    case 1:
      s_read (ScsiIO.p_inbuf);
      break;
    case 2:
      s_write (ScsiIO.p_cdb);
      break;
    case 3:
      s_read (ScsiIO.p_status);
      break;
    case 4:
    case 5:
      printf ("Undefined state\n");
      dumpstat ();
      return;
    case 6:
      s_write (ScsiIO.p_msg_out);
      break;
    case 7:
      s_read (ScsiIO.p_msg_in);
      break;
    default:
      printf ("Undefined state\n");
      dumpstat ();
      return;
    }
  return;
}

void
boot ()
{
  int to;

  CmdDesc.readcmd = 8;
  CmdDesc.lun = 0;
  CmdDesc.lbn = 0x100;
  CmdDesc.nrsects = 13;

  ScsiIO.btdrv = 2;
/* do all the booty stuff here */
/* for our purpose we will boot the cpmloader to 0x8000 until
further notice */
  ScsiIO.p_inbuf = (unsigned int *) 0x8000;	// address for cpmloader.sys
  *ScsiIO.p_status = 0xffff;
  outp (sr_mr, 0);
  outp (sr_tcr, 0);
  outp (sr_tcr, 0);
  outp (sr_odr, ScsiIO.btdrv);
  outp (sr_icr, 1);
  outp (sr_icr, 5);
// wait for busy to clear
  to = 256;
  while (to--)
    {
      if (!(inp (sr_csbs) & sm_bsy))
	{
	  ScsiPhase ();
	  return;
	}
    }
  outp (sr_icr, 0);
  printf ("Read busy timeout\n");
  dumpstat ();
  return;
}

void
mkblock ()
{
  int i;
  for (i = 0; i < 128; i++)
    {
      buffa[i] = i;
    }
}

void
jumpto ()
{
#asm
  jp 0x8000
#endasm
}

void
scsiinit ()
{
  printf ("INIT\n");
}

void
complba ()
{
  printf ("LBA\n");
}

void
pokereg ()
{
  printf ("POKE\n");
}

void
peekreg ()
{
  printf ("PEEK\n");
}

void
initdisk ()
{
  printf ("INIT DISK\n");
}

void
readblk ()
{
  printf ("READ\n");
}

void
dumpstat ()
{
  unsigned char status;
  status = inp (sr_bsr);
  printf ("Bus and Status Register ");
  showbits (status);
  printf ("\n");
/* decode bits */
  if (status & 0x80)
    printf ("End of DMA\n");
  if (status & 0x40)
    printf ("DMA Request\n");
  if (status & 0x20)
    printf ("Parity Error\n");
  if (status & 0x10)
    printf ("Interrupt Request\n");
  if (status & 0x8)
    printf ("Phase match\n");
  if (status & 0x4)
    printf ("Busy error\n");
  if (status & 0x2)
    printf ("ATN inactive\n");
  else
    printf ("ATN active\n");
  if (status & 0x1)
    printf ("ACK inactive\n");
  else
    printf ("ACK active\n");
  status = inp (sr_csbs);
  printf ("Current SCSI Bus Status Register ");
  showbits (status);
  printf ("\n");
  DecodePhase (status);
  if (!(status & 0x80))
    printf ("RST active\n");
  if (!(status & 0x40))
    printf ("BSY active\n");
  if (!(status & 0x20))
    printf ("REQ active\n");
  if (!(status & 0x10))
    printf ("MSG active\n");
  if (!(status & 0x8))
    printf ("C/D active\n");
  if (!(status & 0x4))
    printf ("I/O active\n");
  if (!(status & 0x2))
    printf ("SEL active\n");
  if (!(status & 0x1))
    printf ("DBP active\n");
}

void
writeblk ()
{
  printf ("WRITE\n");
}

void
main ()
{
  printf ("%s %s\n", STARTSTRING, VERSION);
  help ();

/* read console bytes and switch on first character for a command */
  while (1)
    {
      switch (toupper (getchar ()))
	{
	case 'B':		/* Boot the CPMLOADER from the disk. */
	  {
	    boot ();
	    break;
	  }
	case 'C':		/* Create a block of data for the W command */
	  {
	    mkblock ();
	    break;
	  }
	case 'G':		/* Go to the CPMLOADER */
	  {
	    jumpto ();
	    break;
	  }
	case 'H':		/* Print help banner. */
	  {
	    help ();
	    break;
	  }
	case 'I':		/* Initialize the Controller. */
	  {
	    scsiinit ();
	    break;
	  }
	case 'L':		/* Compute the LBA for the disk I/O operation. */
	  {
	    complba ();
	    break;
	  }
	case 'O':		/* Poke at the register set. */
	  {
	    pokereg ();
	    break;
	  }
	case 'P':		/* Peek at the register set. */
	  {
	    peekreg ();
	    break;
	  }
	case 'Q':		/* Setup the disk for I/O. */
	  {
	    initdisk ();
	    break;
	  }
	case 'R':		/* Read a block from the disk. */
	  {
	    readblk ();
	    break;
	  }
	case 'S':		/* Read and decode the Controller and Disk status. */
	  {
	    dumpstat ();
	    break;
	  }
	case 'W':		/* Write a block to the disk. */
	  {
	    writeblk ();
	    break;
	  }
	case 'X':		/* Leave program */
	default:
	  {
		exit(0);
	  }
	}
    }
}
