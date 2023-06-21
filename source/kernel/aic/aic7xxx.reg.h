/*
 ****************************************************************
 *								*
 *			aic7xxx.reg.h				*
 *								*
 *	Registros do aic7xxx para o montador			*
 *								*
 *	Vers�o	4.0.0, de 13.03.01				*
 *		4.0.0, de 13.03.01				*
 *								*
 *	M�dulo: N�cleo						*
 *		N�CLEO do TROPIX para PC			*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright � 2000 NCE/UFRJ - tecle "man licen�a"	*
 *		Baseado no FreeBSD 4.2				*
 *								*
 ****************************************************************
 */

/*
 * This file is processed by the aic7xxx_asm utility for use in assembling
 * firmware for the aic7xxx family of SCSI host adapters as well as to generate
 * a C header file for use in the kernel portion of the Aic7xxx driver.
 *
 * All page numbers refer to the Adaptec AIC-7770 Data Book available from
 * Adaptec's Technical Documents Department 1-800-934-2766
 */

/*
 * SCSI Sequence Control (p. 3-11).
 * Each bit, when set starts a specific SCSI sequence on the bus
 */
register SCSISEQ
{
	address			0x000
	access_mode RW
	bit	TEMODE		0x80
	bit	ENSELO		0x40
	bit	ENSELI		0x20
	bit	ENRSELI		0x10
	bit	ENAUTOATNO	0x08
	bit	ENAUTOATNI	0x04
	bit	ENAUTOATNP	0x02
	bit	SCSIRSTO	0x01
}

/*
 * SCSI Transfer Control 0 Register (pp. 3-13).
 * Controls the SCSI module data path.
 */
register SXFRCTL0
{
	address			0x001
	access_mode RW
	bit	DFON		0x80
	bit	DFPEXP		0x40
	bit	FAST20		0x20
	bit	CLRSTCNT	0x10
	bit	SPIOEN		0x08
	bit	SCAMEN		0x04
	bit	CLRCHN		0x02
}

/*
 * SCSI Transfer Control 1 Register (pp. 3-14,15).
 * Controls the SCSI module data path.
 */
register SXFRCTL1
{
	address			0x002
	access_mode RW
	bit	BITBUCKET	0x80
	bit	SWRAPEN		0x40
	bit	ENSPCHK		0x20
	mask	STIMESEL	0x18
	bit	ENSTIMER	0x04
	bit	ACTNEGEN	0x02
	bit	STPWEN		0x01	/* Powered Termination */
}

/*
 * SCSI Control Signal Read Register (p. 3-15).
 * Reads the actual state of the SCSI bus pins
 */
register SCSISIGI
{
	address			0x003
	access_mode RO
	bit	CDI		0x80
	bit	IOI		0x40
	bit	MSGI		0x20
	bit	ATNI		0x10
	bit	SELI		0x08
	bit	BSYI		0x04
	bit	REQI		0x02
	bit	ACKI		0x01
/*
 * Possible phases in SCSISIGI
 */
	mask	PHASE_MASK	CDI|IOI|MSGI
	mask	P_DATAOUT	0x00
	mask	P_DATAIN	IOI
	mask	P_DATAOUT_DT	P_DATAOUT|MSGI
	mask	P_DATAIN_DT	P_DATAIN|MSGI
	mask	P_COMMAND	CDI
	mask	P_MESGOUT	CDI|MSGI
	mask	P_STATUS	CDI|IOI
	mask	P_MESGIN	CDI|IOI|MSGI
}

/*
 * SCSI Control Signal Write Register (p. 3-16).
 * Writing to this register modifies the control signals on the bus.  Only
 * those signals that are allowed in the current mode (Initiator/Target) are
 * asserted.
 */
register SCSISIGO
{
	address			0x003
	access_mode WO
	bit	CDO		0x80
	bit	IOO		0x40
	bit	MSGO		0x20
	bit	ATNO		0x10
	bit	SELO		0x08
	bit	BSYO		0x04
	bit	REQO		0x02
	bit	ACKO		0x01
/*
 * Possible phases to write into SCSISIG0
 */
	mask	PHASE_MASK	CDI|IOI|MSGI
	mask	P_DATAOUT	0x00
	mask	P_DATAIN	IOI
	mask	P_COMMAND	CDI
	mask	P_MESGOUT	CDI|MSGI
	mask	P_STATUS	CDI|IOI
	mask	P_MESGIN	CDI|IOI|MSGI
}

/* 
 * SCSI Rate Control (p. 3-17).
 * Contents of this register determine the Synchronous SCSI data transfer
 * rate and the maximum synchronous Req/Ack offset.  An offset of 0 in the
 * SOFS (3:0) bits disables synchronous data transfers.  Any offset value
 * greater than 0 enables synchronous transfers.
 */
register SCSIRATE
{
	address			0x004
	access_mode RW
	bit	WIDEXFER	0x80		/* Wide transfer control */
	bit	ENABLE_CRC	0x40		/* CRC for D-Phases */
	bit	SINGLE_EDGE	0x10		/* Disable DT Transfers */
	mask	SXFR		0x70		/* Sync transfer rate */
	mask	SXFR_ULTRA2	0x0f		/* Sync transfer rate */
	mask	SOFS		0x0f		/* Sync offset */
}

/*
 * SCSI ID (p. 3-18).
 * Contains the ID of the board and the current target on the
 * selected channel.
 */
register SCSIID
{
	address			0x005
	access_mode RW
	mask	TID		0xf0		/* Target ID mask */
	mask	TWIN_TID	0x70
	bit	TWIN_CHNLB	0x80
	mask	OID		0x0f		/* Our ID mask */
	/*
	 * SCSI Maximum Offset (p. 4-61 aic7890/91 Data Book)
	 * The aic7890/91 allow an offset of up to 127 transfers in both wide
	 * and narrow mode.
	 */
	alias	SCSIOFFSET
	mask	SOFS_ULTRA2	0x7f		/* Sync offset U2 chips */
}

/*
 * SCSI Latched Data (p. 3-19).
 * Read/Write latches used to transfer data on the SCSI bus during
 * Automatic or Manual PIO mode.  SCSIDATH can be used for the
 * upper byte of a 16bit wide asynchronouse data phase transfer.
 */
register SCSIDATL
{
	address			0x006
	access_mode RW
}

register SCSIDATH
{
	address			0x007
	access_mode RW
}

/*
 * SCSI Transfer Count (pp. 3-19,20)
 * These registers count down the number of bytes transferred
 * across the SCSI bus.  The counter is decremented only once
 * the data has been safely transferred.  SDONE in SSTAT0 is
 * set when STCNT goes to 0
 */ 
register STCNT
{
	address			0x008
	size	3
	access_mode RW
}

/* ALT_MODE register on Ultra160 chips */
register OPTIONMODE
{
	address			0x008
	access_mode RW
	bit	AUTORATEEN		0x80
	bit	AUTOACKEN		0x40
	bit	ATNMGMNTEN		0x20
	bit	BUSFREEREV		0x10
	bit	EXPPHASEDIS		0x08
	bit	SCSIDATL_IMGEN		0x04
	bit	AUTO_MSGOUT_DE		0x02
	bit	DIS_MSGIN_DUALEDGE	0x01
	mask	OPTIONMODE_DEFAULTS	AUTO_MSGOUT_DE|DIS_MSGIN_DUALEDGE
}

/* ALT_MODE register on Ultra160 chips */
register TARGCRCCNT
{
	address			0x00a
	size	2
	access_mode RW
}

/*
 * Clear SCSI Interrupt 0 (p. 3-20)
 * Writing a 1 to a bit clears the associated SCSI Interrupt in SSTAT0.
 */
register CLRSINT0
{
	address			0x00b
	access_mode WO
	bit	CLRSELDO	0x40
	bit	CLRSELDI	0x20
	bit	CLRSELINGO	0x10
	bit	CLRSWRAP	0x08
	bit	CLRSPIORDY	0x02
}

/*
 * SCSI Status 0 (p. 3-21)
 * Contains one set of SCSI Interrupt codes
 * These are most likely of interest to the sequencer
 */
register SSTAT0
{
	address			0x00b
	access_mode RO
	bit	TARGET		0x80	/* Board acting as target */
	bit	SELDO		0x40	/* Selection Done */
	bit	SELDI		0x20	/* Board has been selected */
	bit	SELINGO		0x10	/* Selection In Progress */
	bit	SWRAP		0x08	/* 24bit counter wrap */
	bit	IOERR		0x08	/* LVD Tranceiver mode changed */
	bit	SDONE		0x04	/* STCNT = 0x000000 */
	bit	SPIORDY		0x02	/* SCSI PIO Ready */
	bit	DMADONE		0x01	/* DMA transfer completed */
}

/*
 * Clear SCSI Interrupt 1 (p. 3-23)
 * Writing a 1 to a bit clears the associated SCSI Interrupt in SSTAT1.
 */
register CLRSINT1
{
	address			0x00c
	access_mode WO
	bit	CLRSELTIMEO	0x80
	bit	CLRATNO		0x40
	bit	CLRSCSIRSTI	0x20
	bit	CLRBUSFREE	0x08
	bit	CLRSCSIPERR	0x04
	bit	CLRPHASECHG	0x02
	bit	CLRREQINIT	0x01
}

/*
 * SCSI Status 1 (p. 3-24)
 */
register SSTAT1
{
	address			0x00c
	access_mode RO
	bit	SELTO		0x80
	bit	ATNTARG 	0x40
	bit	SCSIRSTI	0x20
	bit	PHASEMIS	0x10
	bit	BUSFREE		0x08
	bit	SCSIPERR	0x04
	bit	PHASECHG	0x02
	bit	REQINIT		0x01
}

/*
 * SCSI Status 2 (pp. 3-25,26)
 */
register SSTAT2
{
	address			0x00d
	access_mode RO
	bit	OVERRUN		0x80
	bit	SHVALID		0x40	/* Shaddow Layer non-zero */
	bit	EXP_ACTIVE	0x10	/* SCSI Expander Active */
	bit	CRCVALERR	0x08	/* CRC doesn't match (U3 only) */
	bit	CRCENDERR	0x04	/* No terminal CRC packet (U3 only) */
	bit	CRCREQERR	0x02	/* Illegal CRC packet req (U3 only) */
	bit	DUAL_EDGE_ERR	0x01	/* Incorrect data phase (U3 only) */
	mask	SFCNT		0x1f
}

/*
 * SCSI Status 3 (p. 3-26)
 */
register SSTAT3
{
	address			0x00e
	access_mode RO
	mask	SCSICNT		0xf0
	mask	OFFCNT		0x0f
}

/*
 * SCSI ID for the aic7890/91 chips
 */
register SCSIID_ULTRA2
{
	address			0x00f
	access_mode RW
	mask	TID		0xf0		/* Target ID mask */
	mask	OID		0x0f		/* Our ID mask */
}

/*
 * SCSI Interrupt Mode 1 (p. 3-28)
 * Setting any bit will enable the corresponding function
 * in SIMODE0 to interrupt via the IRQ pin.
 */
register SIMODE0
{
	address			0x010
	access_mode RW
	bit	ENSELDO		0x40
	bit	ENSELDI		0x20
	bit	ENSELINGO	0x10
	bit	ENSWRAP		0x08
	bit	ENIOERR		0x08	/* LVD Tranceiver mode changes */
	bit	ENSDONE		0x04
	bit	ENSPIORDY	0x02
	bit	ENDMADONE	0x01
}

/*
 * SCSI Interrupt Mode 1 (pp. 3-28,29)
 * Setting any bit will enable the corresponding function
 * in SIMODE1 to interrupt via the IRQ pin.
 */
register SIMODE1
{
	address			0x011
	access_mode RW
	bit	ENSELTIMO	0x80
	bit	ENATNTARG	0x40
	bit	ENSCSIRST	0x20
	bit	ENPHASEMIS	0x10
	bit	ENBUSFREE	0x08
	bit	ENSCSIPERR	0x04
	bit	ENPHASECHG	0x02
	bit	ENREQINIT	0x01
}

/*
 * SCSI Data Bus (High) (p. 3-29)
 * This register reads data on the SCSI Data bus directly.
 */
register SCSIBUSL
{
	address			0x012
	access_mode RW
}

register SCSIBUSH
{
	address			0x013
	access_mode RW
}

/*
 * SCSI/Host Address (p. 3-30)
 * These registers hold the host address for the byte about to be
 * transferred on the SCSI bus.  They are counted up in the same
 * manner as STCNT is counted down.  SHADDR should always be used
 * to determine the address of the last byte transferred since HADDR
 * can be skewed by write ahead.
 */
register SHADDR
{
	address			0x014
	size	4
	access_mode RO
}

/*
 * Selection Timeout Timer (p. 3-30)
 */
register SELTIMER
{
	address			0x018
	access_mode RW
	bit	STAGE6		0x20
	bit	STAGE5		0x10
	bit	STAGE4		0x08
	bit	STAGE3		0x04
	bit	STAGE2		0x02
	bit	STAGE1		0x01
	alias	TARGIDIN
}

/*
 * Selection/Reselection ID (p. 3-31)
 * Upper four bits are the device id.  The ONEBIT is set when the re/selecting
 * device did not set its own ID.
 */
register SELID
{
	address			0x019
	access_mode RW
	mask	SELID_MASK	0xf0
	bit	ONEBIT		0x08
}

register SCAMCTL
{
	address			0x01a
	access_mode RW
	bit	ENSCAMSELO	0x80
	bit	CLRSCAMSELID	0x40
	bit	ALTSTIM		0x20
	bit	DFLTTID		0x10
	mask	SCAMLVL		0x03
}

/*
 * Target Mode Selecting in ID bitmask (aic7890/91/96/97)
 */
register TARGID
{
	address			0x01b
	size			2
	access_mode RW
}

/*
 * Serial Port I/O Cabability register (p. 4-95 aic7860 Data Book)
 * Indicates if external logic has been attached to the chip to
 * perform the tasks of accessing a serial eeprom, testing termination
 * strength, and performing cable detection.  On the aic7860, most of
 * these features are handled on chip, but on the aic7855 an attached
 * aic3800 does the grunt work.
 */
register SPIOCAP
{
	address			0x01b
	access_mode RW
	bit	SOFT1		0x80
	bit	SOFT0		0x40
	bit	SOFTCMDEN	0x20	
	bit	HAS_BRDCTL	0x10	/* External Board control */
	bit	SEEPROM		0x08	/* External serial eeprom logic */
	bit	EEPROM		0x04	/* Writable external BIOS ROM */
	bit	ROM		0x02	/* Logic for accessing external ROM */
	bit	SSPIOCPS	0x01	/* Termination and cable detection */
}

register BRDCTL
{
	address			0x01d
	bit	BRDDAT7		0x80
	bit	BRDDAT6		0x40
	bit	BRDDAT5		0x20
	bit	BRDSTB		0x10
	bit	BRDCS		0x08
	bit	BRDRW		0x04
	bit	BRDCTL1		0x02
	bit	BRDCTL0		0x01
	/* 7890 Definitions */
	bit	BRDDAT4		0x10
	bit	BRDDAT3		0x08
	bit	BRDDAT2		0x04
	bit	BRDRW_ULTRA2	0x02
	bit	BRDSTB_ULTRA2	0x01
}

/*
 * Serial EEPROM Control (p. 4-92 in 7870 Databook)
 * Controls the reading and writing of an external serial 1-bit
 * EEPROM Device.  In order to access the serial EEPROM, you must
 * first set the SEEMS bit that generates a request to the memory
 * port for access to the serial EEPROM device.  When the memory
 * port is not busy servicing another request, it reconfigures
 * to allow access to the serial EEPROM.  When this happens, SEERDY
 * gets set high to verify that the memory port access has been
 * granted.  
 *
 * After successful arbitration for the memory port, the SEECS bit of 
 * the SEECTL register is connected to the chip select.  The SEECK, 
 * SEEDO, and SEEDI are connected to the clock, data out, and data in 
 * lines respectively.  The SEERDY bit of SEECTL is useful in that it 
 * gives us an 800 nsec timer.  After a write to the SEECTL register, 
 * the SEERDY goes high 800 nsec later.  The one exception to this is 
 * when we first request access to the memory port.  The SEERDY goes 
 * high to signify that access has been granted and, for this case, has 
 * no implied timing.
 *
 * See 93cx6.c for detailed information on the protocol necessary to 
 * read the serial EEPROM.
 */
register SEECTL
{
	address			0x01e
	bit	EXTARBACK	0x80
	bit	EXTARBREQ	0x40
	bit	SEEMS		0x20
	bit	SEERDY		0x10
	bit	SEECS		0x08
	bit	SEECK		0x04
	bit	SEEDO		0x02
	bit	SEEDI		0x01
}
/*
 * SCSI Block Control (p. 3-32)
 * Controls Bus type and channel selection.  In a twin channel configuration
 * addresses 0x00-0x1e are gated to the appropriate channel based on this
 * register.  SELWIDE allows for the coexistence of 8bit and 16bit devices
 * on a wide bus.
 */
register SBLKCTL
{
	address			0x01f
	access_mode RW
	bit	DIAGLEDEN	0x80	/* Aic78X0 only */
	bit	DIAGLEDON	0x40	/* Aic78X0 only */
	bit	AUTOFLUSHDIS	0x20
	bit	SELBUSB		0x08
	bit	ENAB40		0x08	/* LVD transceiver active */
	bit	ENAB20		0x04	/* SE/HVD transceiver active */
	bit	SELWIDE		0x02
	bit	XCVR		0x01	/* External transceiver active */
}

/*
 * Sequencer Control (p. 3-33)
 * Error detection mode and speed configuration
 */
register SEQCTL
{
	address			0x060
	access_mode RW
	bit	PERRORDIS	0x80
	bit	PAUSEDIS	0x40
	bit	FAILDIS		0x20
	bit	FASTMODE	0x10
	bit	BRKADRINTEN	0x08
	bit	STEP		0x04
	bit	SEQRESET	0x02
	bit	LOADRAM		0x01
}

/*
 * Sequencer RAM Data (p. 3-34)
 * Single byte window into the Scratch Ram area starting at the address
 * specified by SEQADDR0 and SEQADDR1.  To write a full word, simply write
 * four bytes in succession.  The SEQADDRs will increment after the most
 * significant byte is written
 */
register SEQRAM
{
	address			0x061
	access_mode RW
}

/*
 * Sequencer Address Registers (p. 3-35)
 * Only the first bit of SEQADDR1 holds addressing information
 */
register SEQADDR0
{
	address			0x062
	access_mode RW
}

register SEQADDR1
{
	address			0x063
	access_mode RW
	mask	SEQADDR1_MASK	0x01
}

/*
 * Accumulator
 * We cheat by passing arguments in the Accumulator up to the kernel driver
 */
register ACCUM
{
	address			0x064
	access_mode RW
	accumulator
}

register SINDEX
{
	address			0x065
	access_mode RW
	sindex
}

register DINDEX
{
	address			0x066
	access_mode RW
}

register ALLONES
{
	address			0x069
	access_mode RO
	allones
}

register ALLZEROS
{
	address			0x06a
	access_mode RO
	allzeros
}

register NONE
{
	address			0x06a
	access_mode WO
	none
}

register FLAGS
{
	address			0x06b
	access_mode RO
	bit	ZERO		0x02
	bit	CARRY		0x01
}

register SINDIR
{
	address			0x06c
	access_mode RO
}

register DINDIR	
{
	address			0x06d
	access_mode WO
}

register FUNCTION1
{
	address			0x06e
	access_mode RW
}

register STACK
{
	address			0x06f
	access_mode RO
}

/*
 * Board Control (p. 3-43)
 */
register BCTL
{
	address			0x084
	access_mode RW
	bit	ACE		0x08
	bit	ENABLE		0x01
}

/*
 * On the aic78X0 chips, Board Control is replaced by the DSCommand
 * register (p. 4-64)
 */
register DSCOMMAND0
{
	address			0x084
	access_mode RW
	bit	CACHETHEN	0x80	/* Cache Threshold enable */
	bit	DPARCKEN	0x40	/* Data Parity Check Enable */
	bit	MPARCKEN	0x20	/* Memory Parity Check Enable */
	bit	EXTREQLCK	0x10	/* External Request Lock */
	/* aic7890/91/96/97 only */
	bit	INTSCBRAMSEL	0x08	/* Internal SCB RAM Select */
	bit	RAMPS		0x04	/* External SCB RAM Present */
	bit	USCBSIZE32	0x02	/* Use 32byte SCB Page Size */
	bit	CIOPARCKEN	0x01	/* Internal bus parity error enable */
}

/*
 * Bus On/Off Time (p. 3-44)
 */
register BUSTIME
{
	address			0x085
	access_mode RW
	mask	BOFF		0xf0
	mask	BON		0x0f
}

/*
 * Bus Speed (p. 3-45) aic7770 only
 */
register BUSSPD
{
	address			0x086
	access_mode RW
	mask	DFTHRSH		0xc0
	mask	STBOFF		0x38
	mask	STBON		0x07
	mask	DFTHRSH_100	0xc0
	mask	DFTHRSH_75	0x80
}

/* aic7850/55/60/70/80/95 only */
register DSPCISTATUS
{
	address			0x086
	mask	DFTHRSH_100	0xc0
}

/* aic7890/91/96/97 only */
register HS_MAILBOX
{
	address			0x086
	mask	HOST_MAILBOX	0xF0
	mask	SEQ_MAILBOX	0x0F
	mask	HOST_TQINPOS	0x80	/* Boundary at either 0 or 128 */
}

const	HOST_MAILBOX_SHIFT	4
const	SEQ_MAILBOX_SHIFT	0

/*
 * Host Control (p. 3-47) R/W
 * Overall host control of the device.
 */
register HCNTRL
{
	address			0x087
	access_mode RW
	bit	POWRDN		0x40
	bit	SWINT		0x10
	bit	IRQMS		0x08
	bit	PAUSE		0x04
	bit	INTEN		0x02
	bit	CHIPRST		0x01
	bit	CHIPRSTACK	0x01
}

/*
 * Host Address (p. 3-48)
 * This register contains the address of the byte about
 * to be transferred across the host bus.
 */
register HADDR
{
	address			0x088
	size	4
	access_mode RW
}

register HCNT
{
	address			0x08c
	size	3
	access_mode RW
}

/*
 * SCB Pointer (p. 3-49)
 * Gate one of the SCBs into the SCBARRAY window.
 */
register SCBPTR
{
	address			0x090
	access_mode RW
}

/*
 * Interrupt Status (p. 3-50)
 * Status for system interrupts
 */
register INTSTAT
{
	address			0x091
	access_mode RW
	bit	BRKADRINT 0x08
	bit	SCSIINT	  0x04
	bit	CMDCMPLT  0x02
	bit	SEQINT    0x01
	mask	BAD_PHASE	SEQINT		/* unknown scsi bus phase */
	mask	SEND_REJECT	0x10|SEQINT	/* sending a message reject */
	mask	NO_IDENT	0x20|SEQINT	/* no IDENTIFY after reconnect*/
	mask	NO_MATCH	0x30|SEQINT	/* no cmd match for reconnect */
	mask	IGN_WIDE_RES	0x40|SEQINT	/* Complex IGN Wide Res Msg */
	mask	RESIDUAL	0x50|SEQINT	/* Residual byte count != 0 */
	mask	HOST_MSG_LOOP	0x60|SEQINT	/*
						 * The bus is ready for the
						 * host to perform another
						 * message transaction.  This
						 * mechanism is used for things
						 * like sync/wide negotiation
						 * that require a kernel based
						 * message state engine.
						 */
	mask	BAD_STATUS	0x70|SEQINT	/* Bad status from target */
	mask	PERR_DETECTED	0x80|SEQINT	/*
						 * Either the phase_lock
						 * or inb_next routine has
						 * noticed a parity error.
						 */
	mask	DATA_OVERRUN	0x90|SEQINT	/*
						 * Target attempted to write
						 * beyond the bounds of its
						 * command.
						 */
	mask	BOGUS_TAG	0xa0|SEQINT	/*
						 * Sequencer given an SCB with
						 * a Tag value of 0xFF.
						 */
	mask	SCBPTR_MISMATCH	0xb0|SEQINT	/*
						 * In the SCB paging case, our
						 * SCBPTR is not the same as
						 * we originally set prior to
						 * download of a new scb.
						 */
	mask	SCB_MISMATCH	0xc0|SEQINT	/*
						 * Downloaded SCB's tag does
						 * not match the entry we
						 * intended to download.
						 */
	mask	ABORT_QINSCB	0xd0|SEQINT	/*
						 * An SCB was aborted
						 * during download.
						 * Informational.
						 */
	mask	NO_FREE_SCB	0xe0|SEQINT	/*
						 * get_free_or_disc_scb failed.
						 */
	mask	OUT_OF_RANGE	0xf0|SEQINT

	mask	SEQINT_MASK	0xf0|SEQINT	/* SEQINT Status Codes */
	mask	INT_PEND  (BRKADRINT|SEQINT|SCSIINT|CMDCMPLT)
}

/*
 * Hard Error (p. 3-53)
 * Reporting of catastrophic errors.  You usually cannot recover from
 * these without a full board reset.
 */
register ERROR
{
	address			0x092
	access_mode RO
	bit	CIOPARERR	0x80	/* Ultra2 only */
	bit	PCIERRSTAT	0x40	/* PCI only */
	bit	MPARERR		0x20	/* PCI only */
	bit	DPARERR		0x10	/* PCI only */
	bit	SQPARERR	0x08
	bit	ILLOPCODE	0x04
	bit	ILLSADDR	0x02
	bit	ILLHADDR	0x01
}

/*
 * Clear Interrupt Status (p. 3-52)
 */
register CLRINT
{
	address			0x092
	access_mode WO
	bit	CLRPARERR	0x10	/* PCI only */
	bit	CLRBRKADRINT	0x08
	bit	CLRSCSIINT      0x04
	bit	CLRCMDINT 	0x02
	bit	CLRSEQINT 	0x01
}

register DFCNTRL
{
	address			0x093
	access_mode RW
	bit	PRELOADEN	0x80	/* aic7890 only */
	bit	WIDEODD		0x40
	bit	SCSIEN		0x20
	bit	SDMAEN		0x10
	bit	SDMAENACK	0x10
	bit	HDMAEN		0x08
	bit	HDMAENACK	0x08
	bit	DIRECTION	0x04
	bit	FIFOFLUSH	0x02
	bit	FIFORESET	0x01
}

register DFSTATUS
{
	address			0x094
	access_mode RO
	bit	PRELOAD_AVAIL	0x80
	bit	DWORDEMP	0x20
	bit	MREQPEND	0x10
	bit	HDONE		0x08
	bit	DFTHRESH	0x04
	bit	FIFOFULL	0x02
	bit	FIFOEMP		0x01
}

register DFWADDR
{
	address			0x95
	access_mode RW
}

register DFRADDR
{
	address			0x97
	access_mode RW
}

register DFDAT
{
	address			0x099
	access_mode RW
}

/*
 * SCB Auto Increment (p. 3-59)
 * Byte offset into the SCB Array and an optional bit to allow auto
 * incrementing of the address during download and upload operations
 */
register SCBCNT
{
	address			0x09a
	access_mode RW
	bit	SCBAUTO		0x80
	mask	SCBCNT_MASK	0x1f
}

/*
 * Queue In FIFO (p. 3-60)
 * Input queue for queued SCBs (commands that the seqencer has yet to start)
 */
register QINFIFO
{
	address			0x09b
	access_mode RW
}

/*
 * Queue In Count (p. 3-60)
 * Number of queued SCBs
 */
register QINCNT
{
	address			0x09c
	access_mode RO
}

/*
 * Queue Out FIFO (p. 3-61)
 * Queue of SCBs that have completed and await the host
 */
register QOUTFIFO
{
	address			0x09d
	access_mode WO
}

register CRCCONTROL1
{
	address			0x09d
	access_mode RW
	bit	CRCONSEEN		0x80
	bit	CRCVALCHKEN		0x40
	bit	CRCENDCHKEN		0x20
	bit	CRCREQCHKEN		0x10
	bit	TARGCRCENDEN		0x08
	bit	TARGCRCCNTEN		0x04
}


/*
 * Queue Out Count (p. 3-61)
 * Number of queued SCBs in the Out FIFO
 */
register QOUTCNT
{
	address			0x09e
	access_mode RO
}

register SCSIPHASE
{
	address			0x09e
	access_mode RO
	bit	STATUS_PHASE	0x20
	bit	COMMAND_PHASE	0x10
	bit	MSG_IN_PHASE	0x08
	bit	MSG_OUT_PHASE	0x04
	bit	DATA_IN_PHASE	0x02
	bit	DATA_OUT_PHASE	0x01
}

/*
 * Special Function
 */
register SFUNCT
{
	address			0x09f
	access_mode RW
	bit	ALT_MODE	0x80
}

/*
 * SCB Definition (p. 5-4)
 */
scb
{
	address			0x0a0
	SCB_CDB_PTR
	{
		size	4
		alias	SCB_RESIDUAL_DATACNT
		alias	SCB_CDB_STORE
		alias	SCB_TARGET_INFO
	}
	SCB_RESIDUAL_SGPTR
	{
		size	4
	}
	SCB_SCSI_STATUS
	{
		size	1
	}
	SCB_CDB_STORE_PAD
	{
		size	3
	}
	SCB_DATAPTR
	{
		size	4
	}
	SCB_DATACNT
	{
		/*
		 * The last byte is really the high address bits for
		 * the data address.
		 */
		size	4
		bit	SG_LAST_SEG		0x80	/* In the fourth byte */
		mask	SG_HIGH_ADDR_BITS	0x7F	/* In the fourth byte */
	}
	SCB_SGPTR
	{
		size	4
		bit	SG_RESID_VALID	0x04	/* In the first byte */
		bit	SG_FULL_RESID	0x02	/* In the first byte */
		bit	SG_LIST_NULL	0x01	/* In the first byte */
	}
	SCB_CONTROL
	{
		size	1
		bit	TARGET_SCB			0x80
		bit	DISCENB				0x40
		bit	TAG_ENB				0x20
		bit	MK_MESSAGE			0x10
		bit	ULTRAENB			0x08
		bit	DISCONNECTED			0x04
		mask	SCB_TAG_TYPE			0x03
	}
	SCB_SCSIID
	{
		size	1
		bit	TWIN_CHNLB			0x80
		mask	TWIN_TID			0x70
		mask	TID				0xf0
		mask	OID				0x0f
	}
	SCB_LUN
	{
		mask	LID				0xff
		size	1
	}
	SCB_TAG
	{
		size	1
	}
	SCB_CDB_LEN
	{
		size	1
	}
	SCB_SCSIRATE
	{
		size	1
	}
	SCB_SCSIOFFSET
	{
		size	1
	}
	SCB_NEXT
	{
		size	1
	}
	SCB_64_SPARE
	{
		size	16
	}
	SCB_64_BTT
	{
		size	16
	}
}

const	SCB_UPLOAD_SIZE		32
const	SCB_DOWNLOAD_SIZE	32
const	SCB_DOWNLOAD_SIZE_64	48

const	SG_SIZEOF	0x08		/* sizeof(struct ahc_dma) */

/* --------------------- AHA-2840-only definitions -------------------- */

register SEECTL_2840
{
	address			0x0c0
	access_mode RW
	bit	CS_2840		0x04
	bit	CK_2840		0x02
	bit	DO_2840		0x01
}

register STATUS_2840
{
	address			0x0c1
	access_mode RW
	bit	EEPROM_TF	0x80
	mask	BIOS_SEL	0x60
	mask	ADSEL		0x1e
	bit	DI_2840		0x01
}

/* --------------------- AIC-7870-only definitions -------------------- */

register CCHADDR
{
	address			0x0E0
	size 8
}

register CCHCNT
{
	address			0x0E8
}

register CCSGRAM
{
	address			0x0E9
}

register CCSGADDR
{
	address			0x0EA
}

register CCSGCTL
{
	address			0x0EB
	bit	CCSGDONE	0x80
	bit	CCSGEN		0x08
	bit	SG_FETCH_NEEDED 0x02	/* Bit used for software state */
	bit	CCSGRESET	0x01
}

register CCSCBCNT
{
	address			0xEF
}

register CCSCBCTL
{
	address			0x0EE
	bit	CCSCBDONE	0x80
	bit	ARRDONE		0x40	/* SCB Array prefetch done */
	bit	CCARREN		0x10
	bit	CCSCBEN		0x08
	bit	CCSCBDIR	0x04
	bit	CCSCBRESET	0x01
}

register CCSCBADDR
{
	address			0x0ED
}

register CCSCBRAM
{
	address			0xEC
}

/*
 * SCB bank address (7895/7896/97 only)
 */
register SCBBADDR
{
	address			0x0F0
	access_mode RW
}

register CCSCBPTR
{
	address			0x0F1
}

register HNSCB_QOFF
{
	address			0x0F4
}

register SNSCB_QOFF
{
	address			0x0F6
}

register SDSCB_QOFF
{
	address			0x0F8
}

register QOFF_CTLSTA
{
	address			0x0FA
	bit	SCB_AVAIL	0x40
	bit	SNSCB_ROLLOVER	0x20
	bit	SDSCB_ROLLOVER	0x10
	mask	SCB_QSIZE	0x07
	mask	SCB_QSIZE_256	0x06
}

register DFF_THRSH
{
	address			0x0FB
	mask	WR_DFTHRSH	0x70
	mask	RD_DFTHRSH	0x07
	mask	RD_DFTHRSH_MIN	0x00
	mask	RD_DFTHRSH_25	0x01
	mask	RD_DFTHRSH_50	0x02
	mask	RD_DFTHRSH_63	0x03
	mask	RD_DFTHRSH_75	0x04
	mask	RD_DFTHRSH_85	0x05
	mask	RD_DFTHRSH_90	0x06
	mask	RD_DFTHRSH_MAX	0x07
	mask	WR_DFTHRSH_MIN	0x00
	mask	WR_DFTHRSH_25	0x10
	mask	WR_DFTHRSH_50	0x20
	mask	WR_DFTHRSH_63	0x30
	mask	WR_DFTHRSH_75	0x40
	mask	WR_DFTHRSH_85	0x50
	mask	WR_DFTHRSH_90	0x60
	mask	WR_DFTHRSH_MAX	0x70
}

register SG_CACHE_PRE
{
	access_mode WO
	address			0x0fc
	mask	SG_ADDR_MASK	0xf8
	bit	ODD_SEG		0x04
	bit	LAST_SEG	0x02
	bit	LAST_SEG_DONE	0x01
}

register SG_CACHE_SHADOW
{
	access_mode RO
	address			0x0fc
	mask	SG_ADDR_MASK	0xf8
	bit	ODD_SEG		0x04
	bit	LAST_SEG	0x02
	bit	LAST_SEG_DONE	0x01
}
/* ---------------------- Scratch RAM Offsets ------------------------- */
/* These offsets are either to values that are initialized by the board's
 * BIOS or are specified by the sequencer code.
 *
 * The host adapter card (at least the BIOS) uses 20-2f for SCSI
 * device information, 32-33 and 5a-5f as well. As it turns out, the
 * BIOS trashes 20-2f, writing the synchronous negotiation results
 * on top of the BIOS values, so we re-use those for our per-target
 * scratchspace (actually a value that can be copied directly into
 * SCSIRATE).  The kernel driver will enable synchronous negotiation
 * for all targets that have a value other than 0 in the lower four
 * bits of the target scratch space.  This should work regardless of
 * whether the bios has been installed.
 */

scratch_ram
{
	address			0x020

	/*
	 * 1 byte per target starting at this address for configuration values
	 */
	CMDSIZE_TABLE
	{
		alias		TARG_SCSIRATE
		size		8
	}
	BUSY_TARGETS
	{
		size		8
	}
	/*
	 * Bit vector of targets that have ULTRA enabled as set by
	 * the BIOS.  The Sequencer relies on a per-SCB field to
	 * control whether to enable Ultra transfers or not.  During
	 * initialization, we read this field and reuse it for 2
	 * entries in the busy target table.
	 */
	ULTRA_ENB
	{
		size		2
	}
	/*
	 * Bit vector of targets that have disconnection disabled as set by
	 * the BIOS.  The Sequencer relies in a per-SCB field to control the
	 * disconnect priveldge.  During initialization, we read this field
	 * and reuse it for 2 entries in the busy target table.
	 */
	DISC_DSB
	{
		size		2
	}
	BUSY_TARGETS_TAIL
	{
		size		4
	}
	/*
	 * Partial transfer past cacheline end to be
	 * transferred using an extra S/G.
	 */
	MWI_RESIDUAL
	{
		size		1
	}
	/*
	 * SCBID of the next SCB to be started by the controller.
	 */
	NEXT_QUEUED_SCB
	{
		size		1
	}
	/*
	 * Single byte buffer used to designate the type or message
	 * to send to a target.
	 */
	MSG_OUT
	{
		size		1
	}
	/* Parameters for DMA Logic */
	DMAPARAMS
	{
		size		1
		bit	PRELOADEN	0x80
		bit	WIDEODD		0x40
		bit	SCSIEN		0x20
		bit	SDMAEN		0x10
		bit	SDMAENACK	0x10
		bit	HDMAEN		0x08
		bit	HDMAENACK	0x08
		bit	DIRECTION	0x04
		bit	FIFOFLUSH	0x02
		bit	FIFORESET	0x01
	}
	SEQ_FLAGS
	{
		size		1
		bit	IDENTIFY_SEEN		0x80
		bit	TARGET_CMD_IS_TAGGED	0x40
		bit	DPHASE			0x20
		/* Target flags */
		bit	TARG_CMD_PENDING	0x10
		bit	CMDPHASE_PENDING	0x08
		bit	DPHASE_PENDING		0x04
		bit	SPHASE_PENDING		0x02
		bit	NO_DISCONNECT		0x01
	}
	/*
	 * Temporary storage for the
	 * target/channel/lun of a
	 * reconnecting target
	 */
	SAVED_SCSIID
	{
		size		1
	}
	SAVED_LUN
	{
		size		1
	}
	/*
	 * The last bus phase as seen by the sequencer. 
	 */
	LASTPHASE
	{
		size		1
		bit	CDI		0x80
		bit	IOI		0x40
		bit	MSGI		0x20
		mask	PHASE_MASK	CDI|IOI|MSGI
		mask	P_DATAOUT	0x00
		mask	P_DATAIN	IOI
		mask	P_COMMAND	CDI
		mask	P_MESGOUT	CDI|MSGI
		mask	P_STATUS	CDI|IOI
		mask	P_MESGIN	CDI|IOI|MSGI
		mask	P_BUSFREE	0x01
	}
	/*
	 * head of list of SCBs awaiting
	 * selection
	 */
	WAITING_SCBH
	{
		size		1
	}
	/*
	 * head of list of SCBs that are
	 * disconnected.  Used for SCB
	 * paging.
	 */
	DISCONNECTED_SCBH
	{
		size		1
	}
	/*
	 * head of list of SCBs that are
	 * not in use.  Used for SCB paging.
	 */
	FREE_SCBH
	{
		size		1
	}
	/*
	 * Address of the hardware scb array in the host.
	 */
	HSCB_ADDR
	{
		size		4
	}
	/*
	 * Base address of our shared data with the kernel driver in host
	 * memory.  This includes the qoutfifo and target mode
	 * incoming command queue.
	 */
	SHARED_DATA_ADDR
	{
		size		4
	}
	KERNEL_QINPOS
	{
		size		1
	}
	QINPOS
	{
		size		1
	}
	QOUTPOS
	{
		size		1
	}
	/*
	 * Kernel and sequencer offsets into the queue of
	 * incoming target mode command descriptors.  The
	 * queue is full when the KERNEL_TQINPOS == TQINPOS.
	 */
	KERNEL_TQINPOS
	{
		size		1
	}
	TQINPOS {                
		size		1
	}
	ARG_1
	{
		size		1
		mask	SEND_MSG		0x80
		mask	SEND_SENSE		0x40
		mask	SEND_REJ		0x20
		mask	MSGOUT_PHASEMIS		0x10
		mask	EXIT_MSG_LOOP		0x08
		mask	CONT_MSG_LOOP		0x04
		mask	CONT_TARG_SESSION	0x02
		alias	RETURN_1
	}
	ARG_2
	{
		size		1
		alias	RETURN_2
	}

	/*
	 * Snapshot of MSG_OUT taken after each message is sent.
	 */
	LAST_MSG
	{
		size		1
	}

	/*
	 * Interrupt kernel for a message to this target on
	 * the next transaction.  This is usually used for
	 * negotiation requests.
	 */
	TARGET_MSG_REQUEST
	{
		size		2
	}

	/*
	 * Sequences the kernel driver has okayed for us.  This allows
	 * the driver to do things like prevent initiator or target
	 * operations.
	 */
	SCSISEQ_TEMPLATE
	{
		size		1
		bit	ENSELO		0x40
		bit	ENSELI		0x20
		bit	ENRSELI		0x10
		bit	ENAUTOATNO	0x08
		bit	ENAUTOATNI	0x04
		bit	ENAUTOATNP	0x02
	}

	/*
	 * Track whether the transfer byte count for
	 * the current data phase is odd.
	 */
	DATA_COUNT_ODD
	{
		size		1
	}

	/*
	 * The initiator specified tag for this target mode transaction.
	 */
	INITIATOR_TAG
	{
		size		1
	}

	/*
	 * These are reserved registers in the card's scratch ram.  Some of
	 * the values are specified in the AHA2742 technical reference manual
	 * and are initialized by the BIOS at boot time.
	 */
	SCSICONF
	{
		address		0x05a
		size		1
		bit	TERM_ENB	0x80
		bit	RESET_SCSI	0x40
		bit	ENSPCHK		0x20
		mask	HSCSIID		0x07	/* our SCSI ID */
		mask	HWSCSIID	0x0f	/* our SCSI ID if Wide Bus */
	}
	INTDEF
	{
		address		0x05c
		size		1
		bit	EDGE_TRIG	0x80
		mask	VECTOR		0x0f
	}
	HOSTCONF
	{
		address		0x05d
		size		1
	}
	HA_274_BIOSCTRL
	{
		address		0x05f
		size		1
		mask	BIOSMODE		0x30
		mask	BIOSDISABLED		0x30	
		bit	CHANNEL_B_PRIMARY	0x08
	}
	/*
	 * Per target SCSI offset values for Ultra2 controllers.
	 */
	TARG_OFFSET
	{
		address		0x070
		size		16
	}
}

const TID_SHIFT		4
const SCB_LIST_NULL	0xff
const TARGET_CMD_CMPLT	0xfe

const CCSGADDR_MAX	0x80
const CCSGRAM_MAXSEGS	16

/* WDTR Message values */
const BUS_8_BIT			0x00
const BUS_16_BIT		0x01
const BUS_32_BIT		0x02

/* Offset maximums */
const MAX_OFFSET_8BIT		0x0f
const MAX_OFFSET_16BIT		0x08
const MAX_OFFSET_ULTRA2		0x7f
const HOST_MSG			0xff

/* Target mode command processing constants */
const CMD_GROUP_CODE_SHIFT	0x05

const STATUS_BUSY		0x08
const STATUS_QUEUE_FULL	0x28
const SCB_TARGET_PHASES		0
const SCB_TARGET_DATA_DIR	1
const SCB_TARGET_STATUS		2
const SCB_INITIATOR_TAG		3
const TARGET_DATA_IN		1

/*
 * Downloaded (kernel inserted) constants
 */
/* Offsets into the SCBID array where different data is stored */
const QOUTFIFO_OFFSET download
const QINFIFO_OFFSET download
const CACHESIZE_MASK download
const INVERTED_CACHESIZE_MASK download
const SG_PREFETCH_CNT download
const SG_PREFETCH_ALIGN_MASK download
const SG_PREFETCH_ADDR_MASK download
