#define bootdrv 2       //boot drive target ID
#define bootlun 0       //boot drive LUN
;
#define nsects  13      //number of blocks
#define ccp     0xdc00   //cp/m ccp location
#define Xbdos    0xe406   //dos location
#define bios    	0xf200   //ios location

//io port addresses
#define uart    0x80
#define scsi    0x88
//ncr 5380 i/o registers
#define sr_csd  scsi
#define sr_icr  scsi+1
#define sr_mr   scsi+2
#define sr_tcr  scsi+3
#define sr_csbs scsi+4
#define sr_bsr  scsi+5
#define sr_idr  scsi+6
#define sr_rpi  scsi+7
//ncr 5380 output only registers
#define sr_odr  scsi
#define sr_ser  scsi+4
#define sr_sds  scsi+5
#define sr_sdtr scsi+6
#define sr_sdir scsi+7
//current bus status bit masks
#define sm_rst  0x80     //reset
#define sm_bsy  0x40     //busy
#define sm_req  0x20     
#define sm_msg  0x10     //message
#define sm_cd   0x08     //cmd/data
#define sm_io   0x04     //in/out
#define sm_sel  0x02     //select
#define sm_dbp  0x01     
#define sm_phm  0x08     //phase match bit

/* scsi i/o table and command descriptor */

typedef struct s_cdb {
	unsigned char readcmd;	
	unsigned char lun;
	unsigned int lbn; 
	unsigned char nrsects;
	unsigned char flags;
}CDB;

typedef struct s_iot {
	unsigned char btdrv;
	unsigned char spacer;
	struct s_cdb *p_cdb;
	unsigned int *p_status;
	unsigned int *p_outbuf;
	unsigned int *p_inbuf;
	unsigned int *p_msg_in;
	unsigned int *p_msg_out;
}SIOT;

 
