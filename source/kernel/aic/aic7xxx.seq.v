/*
 ****************************************************************
 *								*
 *			aic7xxx.seq.v				*
 *								*
 *	C�digo do sequenciador para Adaptec 274x/284x/294x	*
 *								*
 *	Vers�o	4.0.0, de 15.03.01				*
 *		4.0.0, de 23.07.01				*
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

#include "aic/aic7xxx.reg.h"

/*
 *	Defini��o de algumas mensagens
 */
#define MSG_SIMPLE_Q_TAG	0x20 /* O/O */
#define MSG_IGN_WIDE_RESIDUE	0x23 /* O/O */
#define MSG_IDENTIFY_DISCFLAG	0x40 
#define MSG_IDENTIFYFLAG	0x80 
#define MSG_SAVEDATAPOINTER	0x02 /* O/O */
#define MSG_DISCONNECT		0x04 /* O/O */
#define MSG_NOOP		0x08 /* M/M */
#define MSG_RESTOREPOINTERS	0x03 /* O/O */
#define MSG_MESSAGE_REJECT	0x07 /* M/M */
#define MSG_IDENTIFY_LUNMASK	0x3F 

/*
 * A few words on the waiting SCB list:
 * After starting the selection hardware, we check for reconnecting targets
 * as well as for our selection to complete just in case the reselection wins
 * bus arbitration.  The problem with this is that we must keep track of the
 * SCB that we've already pulled from the QINFIFO and started the selection
 * on just in case the reselection wins so that we can retry the selection at
 * a later time.  This problem cannot be resolved by holding a single entry
 * in scratch ram since a reconnecting target can request sense and this will
 * create yet another SCB waiting for selection.  The solution used here is to 
 * use byte 27 of the SCB as a psuedo-next pointer and to thread a list
 * of SCBs that are awaiting selection.  Since 0-0xfe are valid SCB indexes, 
 * SCB_LIST_NULL is 0xff which is out of range.  An entry is also added to
 * this list everytime a request sense occurs or after completing a non-tagged
 * command for which a second SCB has been queued.  The sequencer will
 * automatically consume the entries.
 */

poll_for_work:
	call	clear_target_state;
	and	SXFRCTL0, ~SPIOEN;
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		clr	SCSIBUSL;
	}
poll_for_work_loop:
	test	SSTAT0, SELDO|SELDI	jnz selection;
	test	SCSISEQ, ENSELO	jnz poll_for_work_loop;
	if ((ahc->features & AHC_TWIN) != 0)
	{
		/*
		 * Twin channel devices cannot handle things like SELTO
		 * interrupts on the "background" channel.  So, if we
		 * are selecting, keep polling the current channel util
		 * either a selection or reselection occurs.
		 */
		xor	SBLKCTL,SELBUSB;	/* Toggle to the other bus */
		test	SSTAT0, SELDO|SELDI	jnz selection;
		xor	SBLKCTL,SELBUSB;	/* Toggle back */
	}
	cmp	WAITING_SCBH,SCB_LIST_NULL jne start_waiting;
test_queue:
	/* Has the driver posted any work for us? */
BEGIN_CRITICAL
	if ((ahc->features & AHC_QUEUE_REGS) != 0)
	{
		test	QOFF_CTLSTA, SCB_AVAIL jz poll_for_work_loop;
		mov	NONE, SNSCB_QOFF;
	}
	else
	{
		mov	A, QINPOS;
		cmp	KERNEL_QINPOS, A je poll_for_work_loop;
		inc	QINPOS;
	}
	mov	ARG_1, NEXT_QUEUED_SCB;
END_CRITICAL

	/*
	 * We have at least one queued SCB now and we don't have any 
	 * SCBs in the list of SCBs awaiting selection.  Allocate a
	 * card SCB for the host's SCB and get to work on it.
	 */
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		mov	ALLZEROS	call	get_free_or_disc_scb;
	}
	else
	{
		/* In the non-paging case, the SCBID == hardware SCB index */
		mov	SCBPTR, ARG_1;
	}
dma_queued_scb:
	/*
	 * DMA the SCB from host ram into the current SCB location.
	 */
	mvi	DMAPARAMS, HDMAEN|DIRECTION|FIFORESET;
	mov	ARG_1	call dma_scb;
	/*
	 * Check one last time to see if this SCB was canceled
	 * before we completed the DMA operation.  If it was,
	 * the QINFIFO next pointer will not match our saved
	 * value.
	 */
	mov	A, ARG_1;
BEGIN_CRITICAL
	cmp	NEXT_QUEUED_SCB, A jne abort_qinscb;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		if ((ahc->flags & AHC_PAGESCBS) == 0)
		{
			cmp	SCBPTR, A je . + 2;
			mvi	INTSTAT, SCBPTR_MISMATCH;
		}
		cmp	SCB_TAG, A je . + 2;
		mvi	INTSTAT, SCB_MISMATCH;
	}
	mov	NEXT_QUEUED_SCB, SCB_NEXT;
	mov	SCB_NEXT,WAITING_SCBH;
	mov	WAITING_SCBH, SCBPTR;
END_CRITICAL
start_waiting:
	/*
	 * Start the first entry on the waiting SCB list.
	 */
	mov	SCBPTR, WAITING_SCBH;
	call	start_selection;
	jmp	poll_for_work_loop;

abort_qinscb:
	mvi	INTSTAT, ABORT_QINSCB;
	call	add_scb_to_free_list;
	jmp	poll_for_work_loop;

start_selection:
	if ((ahc->features & AHC_TWIN) != 0)
	{
		and	SINDEX,~SELBUSB,SBLKCTL;/* Clear channel select bit */
		test	SCB_SCSIID, TWIN_CHNLB jz . + 2;
		or	SINDEX, SELBUSB;
		mov	SBLKCTL,SINDEX;		/* select channel */
	}
initialize_scsiid:
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		mov	SCSIID_ULTRA2, SCB_SCSIID;
	}
	else if ((ahc->features & AHC_TWIN) != 0)
	{
		and	SCSIID, TWIN_TID|OID, SCB_SCSIID;
	}
	else
	{
		mov	SCSIID, SCB_SCSIID;
	}

	if ((ahc->flags & AHC_TARGETROLE) != 0)
	{
		mov	SINDEX, SCSISEQ_TEMPLATE;
		test	SCB_CONTROL, TARGET_SCB jz . + 2;
		or	SINDEX, TEMODE;
		mov	SCSISEQ, SINDEX ret;
	}
	else
	{
		mov	SCSISEQ, SCSISEQ_TEMPLATE ret;
	}

/*
 * Initialize transfer settings and clear the SCSI channel.
 * SINDEX should contain any additional bit's the client wants
 * set in SXFRCTL0.  We also assume that the current SCB is
 * a valid SCB for the target we wish to talk to.
 */
initialize_channel:
	or	SXFRCTL0, SPIOEN|CLRSTCNT|CLRCHN;
set_transfer_settings:
	if ((ahc->features & AHC_ULTRA) != 0)
	{
		test	SCB_CONTROL, ULTRAENB jz . + 2;
		or	SXFRCTL0, FAST20;
	} 

	/*
	 * Initialize SCSIRATE with the appropriate value for this target.
	 */
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		bmov	SCSIRATE, SCB_SCSIRATE, 2 ret;
	}
	else
	{
		mov	SCSIRATE, SCB_SCSIRATE ret;
	}

selection:
	/*
	 * We aren't expecting a bus free, so interrupt
	 * the kernel driver if it happens.
	 */
	mvi	CLRSINT1,CLRBUSFREE;
	or	SIMODE1, ENBUSFREE;

	test	SSTAT0,SELDO	jnz select_out;
	mvi	CLRSINT0, CLRSELDI;
select_in:
	if ((ahc->flags & AHC_TARGETROLE) != 0)
	{
		if ((ahc->flags & AHC_INITIATORROLE) != 0)
		{
			test	SSTAT0, TARGET	jz initiator_reselect;
		}

		/*
		 * We've just been selected.  Assert BSY and
		 * setup the phase for receiving messages
		 * from the target.
		 */
		mvi	SCSISIGO, P_MESGOUT|BSYO;

		/*
		 * Setup the DMA for sending the identify and
		 * command information.
		 */
		or	SEQ_FLAGS, CMDPHASE_PENDING;

		mov     A, TQINPOS;

		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mvi	DINDEX, CCHADDR;
			mvi	SHARED_DATA_ADDR call set_32byte_addr;
			mvi	CCSCBCTL, CCSCBRESET;
		}
		else
		{
			mvi	DINDEX, HADDR;
			mvi	SHARED_DATA_ADDR call set_32byte_addr;
			mvi	DFCNTRL, FIFORESET;
		}

		/* Initiator that selected us */
		and	SAVED_SCSIID, SELID_MASK, SELID;
		/* The Target ID we were selected at */
		if ((ahc->features & AHC_MULTI_TID) != 0)
		{
			and	A, OID, TARGIDIN;
		}
		else if ((ahc->features & AHC_ULTRA2) != 0)
		{
			and	A, OID, SCSIID_ULTRA2;
		}
		else
		{
			and	A, OID, SCSIID;
		}

		or	SAVED_SCSIID, A;

		if ((ahc->features & AHC_TWIN) != 0)
		{
			test 	SBLKCTL, SELBUSB jz . + 2;
			or	SAVED_SCSIID, TWIN_CHNLB;
		}
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, SAVED_SCSIID;
		}
		else
		{
			mov	DFDAT, SAVED_SCSIID;
		}

		/*
		 * If ATN isn't asserted, the target isn't interested
		 * in talking to us.  Go directly to bus free.
		 * XXX SCSI-1 may require us to assume lun 0 if
		 * ATN is false.
		 */
		test	SCSISIGI, ATNI	jz	target_busfree;

		/*
		 * Watch ATN closely now as we pull in messages from the
		 * initiator.  We follow the guidlines from section 6.5
		 * of the SCSI-2 spec for what messages are allowed when.
		 */
		call	target_inb;

		/*
		 * Our first message must be one of IDENTIFY, ABORT, or
		 * BUS_DEVICE_RESET.
		 */
		test	DINDEX, MSG_IDENTIFYFLAG jz host_target_message_loop;

		/* Store for host */

		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, DINDEX;
		}
		else
		{
			mov	DFDAT, DINDEX;
		}

		/* Remember for disconnection decision */
		test	DINDEX, MSG_IDENTIFY_DISCFLAG jnz . + 2;
		/* XXX Honor per target settings too */
		or	SEQ_FLAGS, NO_DISCONNECT;

		test	SCSISIGI, ATNI	jz	ident_messages_done;
		call	target_inb;
		/*
		 * If this is a tagged request, the tagged message must
		 * immediately follow the identify.  We test for a valid
		 * tag message by seeing if it is >= MSG_SIMPLE_Q_TAG and
		 * < MSG_IGN_WIDE_RESIDUE.
		 */
		add	A, -MSG_SIMPLE_Q_TAG, DINDEX;
		jnc	ident_messages_done;
		add	A, -MSG_IGN_WIDE_RESIDUE, DINDEX;
		jc	ident_messages_done;
		/* Store for host */
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, DINDEX;
		}
		else
		{
			mov	DFDAT, DINDEX;
		}
		
		/*
		 * If the initiator doesn't feel like providing a tag number,
		 * we've got a failed selection and must transition to bus
		 * free.
		 */
		test	SCSISIGI, ATNI	jz	target_busfree;

		/*
		 * Store the tag for the host.
		 */
		call	target_inb;
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, DINDEX;
		}
		else
		{
			mov	DFDAT, DINDEX;
		}
		mov	INITIATOR_TAG, DINDEX;
		or	SEQ_FLAGS, TARGET_CMD_IS_TAGGED;
		test	SCSISIGI, ATNI	jz . + 2;
		/* Initiator still wants to give us messages */
		call	target_inb;
		jmp	ident_messages_done;

		/*
		 * Pushed message loop to allow the kernel to
		 * run it's own target mode message state engine.
		 */
host_target_message_loop:
		mvi	INTSTAT, HOST_MSG_LOOP;
		nop;
		cmp	RETURN_1, EXIT_MSG_LOOP	je target_ITloop;
		test	SSTAT0, SPIORDY jz .;
		jmp	host_target_message_loop;

ident_messages_done:
		/* If ring buffer is full, return busy or queue full */
		if ((ahc->features & AHC_HS_MAILBOX) != 0)
		{
			and	A, HOST_TQINPOS, HS_MAILBOX;
		}
		else
		{
			mov	A, KERNEL_TQINPOS;
		}

		cmp	TQINPOS, A jne tqinfifo_has_space;
		mvi	P_STATUS|BSYO call change_phase;
		test	SEQ_FLAGS, TARGET_CMD_IS_TAGGED jz . + 3;
		mvi	STATUS_QUEUE_FULL call target_outb;
		jmp	target_busfree_wait;
		mvi	STATUS_BUSY call target_outb;
		jmp	target_busfree_wait;

tqinfifo_has_space:	
		/* Terminate the ident list */
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mvi	CCSCBRAM, SCB_LIST_NULL;
		}
		else
		{
			mvi	DFDAT, SCB_LIST_NULL;
		}
		or	SEQ_FLAGS, TARG_CMD_PENDING|IDENTIFY_SEEN;
		test	SCSISIGI, ATNI	jnz target_mesgout_pending;
		jmp	target_ITloop;
		
/*
 * We carefully toggle SPIOEN to allow us to return the 
 * message byte we receive so it can be checked prior to
 * driving REQ on the bus for the next byte.
 */
target_inb:
		/*
		 * Drive REQ on the bus by enabling SCSI PIO.
		 */
		or	SXFRCTL0, SPIOEN;
		/* Wait for the byte */
		test	SSTAT0, SPIORDY jz .;
		/* Prevent our read from triggering another REQ */
		and	SXFRCTL0, ~SPIOEN;
		/* Save latched contents */
		mov	DINDEX, SCSIDATL ret;
	}

if ((ahc->flags & AHC_INITIATORROLE) != 0)
{
/*
 * Reselection has been initiated by a target. Make a note that we've been
 * reselected, but haven't seen an IDENTIFY message from the target yet.
 */
initiator_reselect:
	/* XXX test for and handle ONE BIT condition */
	and	SAVED_SCSIID, SELID_MASK, SELID;
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		and	A, OID, SCSIID_ULTRA2;
	}
	else
	{
		and	A, OID, SCSIID;
	}
	or	SAVED_SCSIID, A;
	if ((ahc->features & AHC_TWIN) != 0)
	{
		test	SBLKCTL, SELBUSB	jz . + 2;
		or	SAVED_SCSIID, TWIN_CHNLB;
	}
	or	SXFRCTL0, SPIOEN|CLRSTCNT|CLRCHN;
	jmp	ITloop;
}

/*
 * After the selection, remove this SCB from the "waiting SCB"
 * list.  This is achieved by simply moving our "next" pointer into
 * WAITING_SCBH.  Our next pointer will be set to null the next time this
 * SCB is used, so don't bother with it now.
 */
select_out:
	/* Turn off the selection hardware */
	and	SCSISEQ, TEMODE|ENSELI|ENRSELI|ENAUTOATNP, SCSISEQ;
	mvi	CLRSINT0, CLRSELDO;
	mov	SCBPTR, WAITING_SCBH;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		cmp	SCB_TAG, SCB_LIST_NULL jne . + 2;
		mvi	INTSTAT, BOGUS_TAG;
	}
	mov	WAITING_SCBH,SCB_NEXT;
	mov	SAVED_SCSIID, SCB_SCSIID;
	mov	SAVED_LUN, SCB_LUN;
	call	initialize_channel;
	if ((ahc->flags & AHC_TARGETROLE) != 0)
	{
		test	SSTAT0, TARGET	jz initiator_select;

		/*
		 * We've just re-selected an initiator.
		 * Assert BSY and setup the phase for
		 * sending our identify messages.
		 */
		mvi	P_MESGIN|BSYO call change_phase;

		/*
		 * Start out with a simple identify message.
		 */
		or	SCB_LUN, MSG_IDENTIFYFLAG call target_outb;

		/*
		 * If we are the result of a tagged command, send
		 * a simple Q tag and the tag id.
		 */
		test	SCB_CONTROL, TAG_ENB	jz . + 3;
		mvi	MSG_SIMPLE_Q_TAG call target_outb;
		mov	SCB_TARGET_INFO[SCB_INITIATOR_TAG] call target_outb;
target_synccmd:
		/*
		 * Now determine what phases the host wants us
		 * to go through.
		 */
		mov	SEQ_FLAGS, SCB_TARGET_INFO[SCB_TARGET_PHASES];
		
target_ITloop:
		/*
		 * Start honoring ATN signals now that
		 * we properly identified ourselves.
		 */
		test	SCSISIGI, ATNI			jnz target_mesgout;
		test	SEQ_FLAGS, CMDPHASE_PENDING	jnz target_cmdphase;
		test	SEQ_FLAGS, DPHASE_PENDING	jnz target_dphase;
		test	SEQ_FLAGS, SPHASE_PENDING	jnz target_sphase;

		/*
		 * No more work to do.  Either disconnect or not depending
		 * on the state of NO_DISCONNECT.
		 */
		test	SEQ_FLAGS, NO_DISCONNECT jz target_disconnect; 
		if ((ahc->flags & AHC_PAGESCBS) != 0)
		{
			mov	ALLZEROS	call	get_free_or_disc_scb;
		}
		mov	RETURN_1, ALLZEROS;
		call	complete_target_cmd;
		cmp	RETURN_1, CONT_MSG_LOOP jne .;
		mvi	DMAPARAMS, HDMAEN|DIRECTION|FIFORESET;
		mov	SCB_TAG	 call dma_scb;
		jmp	target_synccmd;

target_mesgout:
		mvi	SCSISIGO, P_MESGOUT|BSYO;
target_mesgout_continue:
		call	target_inb;
target_mesgout_pending:
		/* Local Processing goes here... */
		jmp	host_target_message_loop;
		
target_disconnect:
		mvi	P_MESGIN|BSYO call change_phase;
		test	SEQ_FLAGS, DPHASE	jz . + 2;
		mvi	MSG_SAVEDATAPOINTER call target_outb;
		mvi	MSG_DISCONNECT call target_outb;

target_busfree_wait:
		/* Wait for preceeding I/O session to complete. */
		test	SCSISIGI, ACKI jnz .;
target_busfree:
		and	SIMODE1, ~ENBUSFREE;
		if ((ahc->features & AHC_ULTRA2) != 0)
		{
			clr	SCSIBUSL;
		}
		clr	SCSISIGO;
		mvi	LASTPHASE, P_BUSFREE;
		call	complete_target_cmd;
		jmp	poll_for_work;

target_cmdphase:
		mvi	P_COMMAND|BSYO call change_phase;
		call	target_inb;
		mov	A, DINDEX;
		/* Store for host */
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, A;
		}
		else
		{
			mov	DFDAT, A;
		}

		/*
		 * Determine the number of bytes to read
		 * based on the command group code via table lookup.
		 * We reuse the first 8 bytes of the TARG_SCSIRATE
		 * BIOS array for this table. Count is one less than
		 * the total for the command since we've already fetched
		 * the first byte.
		 */
		shr	A, CMD_GROUP_CODE_SHIFT;
		add	SINDEX, CMDSIZE_TABLE, A;
		mov	A, SINDIR;

		test	A, 0xFF jz command_phase_done;
command_loop:
		or	SXFRCTL0, SPIOEN;
		test	SSTAT0, SPIORDY jz .;
		cmp	A, 1 jne . + 2;
		and	SXFRCTL0, ~SPIOEN;	/* Last Byte */
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			mov	CCSCBRAM, SCSIDATL;
		}
		else
		{
			mov	DFDAT, SCSIDATL;
		}
		dec	A;
		test	A, 0xFF jnz command_loop;

command_phase_done:
		and	SEQ_FLAGS, ~CMDPHASE_PENDING;
		jmp	target_ITloop;

target_dphase:
		/*
		 * Data phases on the bus are from the
		 * perspective of the initiator.  The dma
		 * code looks at LASTPHASE to determine the
		 * data direction of the DMA.  Toggle it for
		 * target transfers.
		 */
		xor	LASTPHASE, IOI, SCB_TARGET_INFO[SCB_TARGET_DATA_DIR];
		or	SCB_TARGET_INFO[SCB_TARGET_DATA_DIR], BSYO
			call change_phase;
		jmp	p_data;

target_sphase:
		mvi	P_STATUS|BSYO call change_phase;
		mvi	LASTPHASE, P_STATUS;
		mov	SCB_TARGET_INFO[SCB_TARGET_STATUS] call target_outb;
		/* XXX Watch for ATN or parity errors??? */
		mvi	SCSISIGO, P_MESGIN|BSYO;
		/* MSG_CMDCMPLT is 0, but we can't do an immediate of 0 */
		mov	ALLZEROS call target_outb;
		jmp	target_busfree_wait;
	
complete_target_cmd:
		test	SEQ_FLAGS, TARG_CMD_PENDING	jnz . + 2;
		mov	SCB_TAG jmp complete_post;
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			/* Set the valid byte */
			mvi	CCSCBADDR, 24;
			mov	CCSCBRAM, ALLONES;
			mvi	CCHCNT, 28;
			or	CCSCBCTL, CCSCBEN|CCSCBRESET;
			test	CCSCBCTL, CCSCBDONE jz .;
			clr	CCSCBCTL;
		}
		else
		{
			/* Set the valid byte */
			or	DFCNTRL, FIFORESET;
			mvi	DFWADDR, 3; /* Third 64bit word or byte 24 */
			mov	DFDAT, ALLONES;
			mvi	28	call set_hcnt;
			or	DFCNTRL, HDMAEN|FIFOFLUSH;
			call	dma_finish;
		}
		inc	TQINPOS;
		mvi	INTSTAT,CMDCMPLT ret;
	}

if ((ahc->flags & AHC_INITIATORROLE) != 0)
{
initiator_select:
	/*
	 * As soon as we get a successful selection, the target
	 * should go into the message out phase since we have ATN
	 * asserted.
	 */
	mvi	MSG_OUT, MSG_IDENTIFYFLAG;
	or	SEQ_FLAGS, IDENTIFY_SEEN;

	/*
	 * Main loop for information transfer phases.  Wait for the
	 * target to assert REQ before checking MSG, C/D and I/O for
	 * the bus phase.
	 */
mesgin_phasemis:
ITloop:
	call	phase_lock;

	mov	A, LASTPHASE;

	test	A, ~P_DATAIN	jz p_data;
	cmp	A,P_COMMAND	je p_command;
	cmp	A,P_MESGOUT	je p_mesgout;
	cmp	A,P_STATUS	je p_status;
	cmp	A,P_MESGIN	je p_mesgin;

	mvi	INTSTAT,BAD_PHASE;
	jmp	ITloop;			/* Try reading the bus again. */

await_busfree:
	and	SIMODE1, ~ENBUSFREE;
	mov	NONE, SCSIDATL;		/* Ack the last byte */
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		clr	SCSIBUSL;	/* Prevent bit leakage durint SELTO */
	}
	and	SXFRCTL0, ~SPIOEN;
	test	SSTAT1,REQINIT|BUSFREE	jz .;
	test	SSTAT1, BUSFREE jnz poll_for_work;
	mvi	INTSTAT, BAD_PHASE;
}
	
clear_target_state:
	/*
	 * We assume that the kernel driver may reset us
	 * at any time, even in the middle of a DMA, so
	 * clear DFCNTRL too.
	 */
	clr	DFCNTRL;
	or	SXFRCTL0, CLRSTCNT|CLRCHN;

	/*
	 * We don't know the target we will connect to,
	 * so default to narrow transfers to avoid
	 * parity problems.
	 */
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		bmov	SCSIRATE, ALLZEROS, 2;
	}
	else
	{
		clr	SCSIRATE;

		if ((ahc->features & AHC_ULTRA) != 0)
		{
			and	SXFRCTL0, ~(FAST20);
		}
	}

	mvi	LASTPHASE, P_BUSFREE;
	/* clear target specific flags */
	clr	SEQ_FLAGS ret;

sg_advance:
	clr	A;			/* add sizeof(struct scatter) */
	add	SCB_RESIDUAL_SGPTR[0],SG_SIZEOF;
	adc	SCB_RESIDUAL_SGPTR[1],A;
	adc	SCB_RESIDUAL_SGPTR[2],A;
	adc	SCB_RESIDUAL_SGPTR[3],A ret;

idle_loop:
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		/* Did we just finish fetching segs? */
		cmp	CCSGCTL, CCSGEN|CCSGDONE je idle_sgfetch_complete;

		/* Are we actively fetching segments? */
		test	CCSGCTL, CCSGEN jnz return;

		/*
		 * Do we need any more segments?
		 */
		test	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG jnz return;

		/*
		 * Do we have any prefetch left???
		 */
		cmp	CCSGADDR, SG_PREFETCH_CNT jne idle_sg_avail;

		/*
		 * Need to fetch segments, but we can only do that
		 * if the command channel is completely idle.  Make
		 * sure we don't have an SCB prefetch going on.
		 */
		test	CCSCBCTL, CCSCBEN jnz return;

		/*
		 * We fetch a "cacheline aligned" and sized amount of data
		 * so we don't end up referencing a non-existant page.
		 * Cacheline aligned is in quotes because the kernel will
		 * set the prefetch amount to a reasonable level if the
		 * cacheline size is unknown.
		 */
		mvi	CCHCNT, SG_PREFETCH_CNT;
		and	CCHADDR[0], SG_PREFETCH_ALIGN_MASK, SCB_RESIDUAL_SGPTR;
		bmov	CCHADDR[1], SCB_RESIDUAL_SGPTR[1], 3;
		mvi	CCSGCTL, CCSGEN|CCSGRESET ret;
idle_sgfetch_complete:
		clr	CCSGCTL;
		test	CCSGCTL, CCSGEN jnz .;
		and	CCSGADDR, SG_PREFETCH_ADDR_MASK, SCB_RESIDUAL_SGPTR;
idle_sg_avail:
		if ((ahc->features & AHC_ULTRA2) != 0)
		{
			/* Does the hardware have space for another SG entry? */
			test	DFSTATUS, PRELOAD_AVAIL jz return;
			bmov 	HADDR, CCSGRAM, 4;
			bmov	SINDEX, CCSGRAM, 1;
			test	SINDEX, 0x1 jz . + 2;
			xor	DATA_COUNT_ODD, 0x1;
			bmov	HCNT[0], SINDEX, 1;
			bmov	HCNT[1], CCSGRAM, 2;
			bmov	SCB_RESIDUAL_DATACNT[3], CCSGRAM, 1;
			call	sg_advance;
			mov	SINDEX, SCB_RESIDUAL_SGPTR[0];
			test	DATA_COUNT_ODD, 0x1 jz . + 2;
			or	SINDEX, ODD_SEG;
			test	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG jz . + 2;
			or	SINDEX, LAST_SEG;
			mov	SG_CACHE_PRE, SINDEX;
			/* Load the segment by writing DFCNTRL again */
			mov	DFCNTRL, DMAPARAMS;
		}
		ret;
	}

if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0 && ahc->pci_cachesize != 0)
{
/*
 * Calculate the trailing portion of this S/G segment that cannot
 * be transferred using memory write and invalidate PCI transactions.  
 * XXX Can we optimize this for PCI writes only???
 */
calc_mwi_residual:
	/*
	 * If the ending address is on a cacheline boundary,
	 * there is no need for an extra segment.
	 */
	mov	A, HCNT[0];
	add	A, A, HADDR[0];
	and	A, CACHESIZE_MASK;
	test	A, 0xFF jz return;

	/*
	 * If the transfer is less than a cachline,
	 * there is no need for an extra segment.
	 */
	test	HCNT[1], 0xFF	jnz calc_mwi_residual_final;
	test	HCNT[2], 0xFF	jnz calc_mwi_residual_final;
	add	NONE, INVERTED_CACHESIZE_MASK, HCNT[0];
	jnc	return;

calc_mwi_residual_final:
	mov	MWI_RESIDUAL, A;
	not	A;
	inc	A;
	add	HCNT[0], A;
	adc	HCNT[1], -1;
	adc	HCNT[2], -1 ret;
}

/*
 * If we re-enter the data phase after going through another phase, the
 * STCNT may have been cleared, so restore it from the residual field.
 */
data_phase_reinit:
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		/*
		 * The preload circuitry requires us to
		 * reload the address too, so pull it from
		 * the shaddow address.
		 */
		bmov	HADDR, SHADDR, 4;
		bmov	HCNT, SCB_RESIDUAL_DATACNT, 3;
	}
	else if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	STCNT, SCB_RESIDUAL_DATACNT, 3;
	}
	else
	{
		mvi	DINDEX, STCNT;
		mvi	SCB_RESIDUAL_DATACNT call bcopy_3;
	}
	and	DATA_COUNT_ODD, 0x1, SCB_RESIDUAL_DATACNT[0];
	jmp	data_phase_loop;

p_data:
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		mvi	DMAPARAMS, PRELOADEN|SCSIEN|HDMAEN;
	}
	else
	{
		mvi	DMAPARAMS, WIDEODD|SCSIEN|SDMAEN|HDMAEN|FIFORESET;
	}
	test	LASTPHASE, IOI jnz . + 2;
	or	DMAPARAMS, DIRECTION;
	call	assert;			/*
					 * Ensure entering a data
					 * phase is okay - seen identify, etc.
					 */
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		/* We don't have any valid S/G elements */
		mvi	CCSGADDR, SG_PREFETCH_CNT;
	}
	test	SEQ_FLAGS, DPHASE	jnz data_phase_reinit;

	/* We have seen a data phase */
	or	SEQ_FLAGS, DPHASE;

	/*
	 * Initialize the DMA address and counter from the SCB.
	 * Also set SCB_RESIDUAL_SGPTR, including the LAST_SEG
	 * flag in the highest byte of the data count.  We cannot
	 * modify the saved values in the SCB until we see a save
	 * data pointers message.
	 */
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	HADDR, SCB_DATAPTR, 7;
		bmov	SCB_RESIDUAL_DATACNT[3], SCB_DATACNT[3], 5;
	}
	else
	{
		mvi	DINDEX, HADDR;
		mvi	SCB_DATAPTR	call bcopy_7;
		mvi	DINDEX, SCB_RESIDUAL_DATACNT + 3;
		mvi	SCB_DATACNT + 3 call bcopy_5;
	}
	if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0 && ahc->pci_cachesize != 0)
	{
		call	calc_mwi_residual;
	}
	and	SCB_RESIDUAL_SGPTR[0], ~SG_FULL_RESID;
	and	DATA_COUNT_ODD, 0x1, HCNT[0];

	if ((ahc->features & AHC_ULTRA2) == 0)
	{
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			bmov	STCNT, HCNT, 3;
		}
		else
		{
			call	set_stcnt_from_hcnt;
		}
	}

data_phase_loop:
	/* Guard against overruns */
	test	SCB_RESIDUAL_SGPTR[0], SG_LIST_NULL jz data_phase_inbounds;

	/*
	 * Turn on `Bit Bucket' mode, wait until the target takes
	 * us to another phase, and then notify the host.
	 */
	and	DMAPARAMS, DIRECTION;
	mov	DFCNTRL, DMAPARAMS;
	or	SXFRCTL1,BITBUCKET;
	test	SSTAT1,PHASEMIS	jz .;
	and	SXFRCTL1, ~BITBUCKET;
	mvi	INTSTAT,DATA_OVERRUN;
	jmp	ITloop;

data_phase_inbounds:
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		mov	SINDEX, SCB_RESIDUAL_SGPTR[0];
		test	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG jz . + 2;
		or	SINDEX, LAST_SEG;
		test	DATA_COUNT_ODD, 0x1 jz . + 2;
		or	SINDEX, ODD_SEG;
		mov	SG_CACHE_PRE, SINDEX;
		mov	DFCNTRL, DMAPARAMS;
ultra2_dma_loop:
		call	idle_loop;
		/*
		 * The transfer is complete if either the last segment
		 * completes or the target changes phase.
		 */
		test	SG_CACHE_SHADOW, LAST_SEG_DONE jnz ultra2_dmafinish;
		test	SSTAT1,PHASEMIS	jz ultra2_dma_loop;

ultra2_dmafinish:
		test	DFCNTRL, DIRECTION jnz ultra2_dmafifoempty;
		and	DFCNTRL, ~SCSIEN;
		test	DFCNTRL, SCSIEN jnz .;
		if ((ahc->bugs & AHC_AUTOFLUSH_BUG) != 0)
		{
			test	DFSTATUS, FIFOEMP jnz ultra2_dmafifoempty;
		}
ultra2_dmafifoflush:
		if ((ahc->bugs & AHC_AUTOFLUSH_BUG) != 0)
		{
			/*
			 * On Rev A of the aic7890, the autoflush
			 * features doesn't function correctly.
			 * Perform an explicit manual flush.  During
			 * a manual flush, the FIFOEMP bit becomes
			 * true every time the PCI FIFO empties
			 * regardless of the state of the SCSI FIFO.
			 * It can take up to 4 clock cycles for the
			 * SCSI FIFO to get data into the PCI FIFO
			 * and for FIFOEMP to de-assert.  Here we
			 * guard against this condition by making
			 * sure the FIFOEMP bit stays on for 5 full
			 * clock cycles.
			 */
			or	DFCNTRL, FIFOFLUSH;
			test	DFSTATUS, FIFOEMP jz ultra2_dmafifoflush;
			test	DFSTATUS, FIFOEMP jz ultra2_dmafifoflush;
			test	DFSTATUS, FIFOEMP jz ultra2_dmafifoflush;
			test	DFSTATUS, FIFOEMP jz ultra2_dmafifoflush;
		}
		test	DFSTATUS, FIFOEMP jz ultra2_dmafifoflush;
ultra2_dmafifoempty:
		/* Don't clobber an inprogress host data transfer */
		test	DFSTATUS, MREQPEND	jnz ultra2_dmafifoempty;
ultra2_dmahalt:
		and     DFCNTRL, ~(SCSIEN|HDMAEN);
		test	DFCNTRL, HDMAEN jnz .;

		/*
		 * If, by chance, we stopped before being able
		 * to fetch additional segments for this transfer,
		 * yet the last S/G was completely exhausted,
		 * call our idle loop until it is able to load
		 * another segment.  This will allow us to immediately
		 * pickup on the next segment on the next data phase.
		 *
		 * If we happened to stop on the last segment, then
		 * our residual information is still correct from
		 * the idle loop and there is no need to perform
		 * any fixups.  Just jump to data_phase_finish.
		 */
ultra2_ensure_sg:
		test	SG_CACHE_SHADOW, LAST_SEG jz ultra2_shvalid;
		/* Record if we've consumed all S/G entries */
		test	SG_CACHE_SHADOW, LAST_SEG_DONE jz data_phase_finish;
		or	SCB_RESIDUAL_SGPTR[0], SG_LIST_NULL;
		jmp	data_phase_finish;

ultra2_shvalid:
                test    SSTAT2, SHVALID	jnz sgptr_fixup;
		call	idle_loop;
		jmp	ultra2_ensure_sg;

sgptr_fixup:
		/*
		 * Fixup the residual next S/G pointer.  The S/G preload
		 * feature of the chip allows us to load two elements
		 * in addition to the currently active element.  We
		 * store the bottom byte of the next S/G pointer in
		 * the SG_CACEPTR register so we can restore the
		 * correct value when the DMA completes.  If the next
		 * sg ptr value has advanced to the point where higher
		 * bytes in the address have been affected, fix them
		 * too.
		 */
		test	SG_CACHE_SHADOW, 0x80 jz sgptr_fixup_done;
		test	SCB_RESIDUAL_SGPTR[0], 0x80 jnz sgptr_fixup_done;
		add	SCB_RESIDUAL_SGPTR[1], -1;
		adc	SCB_RESIDUAL_SGPTR[2], -1; 
		adc	SCB_RESIDUAL_SGPTR[3], -1;
sgptr_fixup_done:
		and	SCB_RESIDUAL_SGPTR[0], SG_ADDR_MASK, SG_CACHE_SHADOW;
		clr	DATA_COUNT_ODD;
		test	SG_CACHE_SHADOW, ODD_SEG jz . + 2;
		or	DATA_COUNT_ODD, 0x1;
		clr	SCB_RESIDUAL_DATACNT[3]; /* We are not the last seg */
	}
	else
	{
		/* If we are the last SG block, tell the hardware. */
		if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0
		  && ahc->pci_cachesize != 0)
		{
			test	MWI_RESIDUAL, 0xFF jnz dma_mid_sg;
		}
		test	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG jz dma_mid_sg;
		if ((ahc->flags & AHC_TARGETROLE) != 0)
		{
			test	SSTAT0, TARGET jz dma_last_sg;
			if ((ahc->flags & AHC_TMODE_WIDEODD_BUG) != 0)
			{
				test	DMAPARAMS, DIRECTION jz dma_mid_sg;
			}
		}
dma_last_sg:
		and	DMAPARAMS, ~WIDEODD;
dma_mid_sg:
		/* Start DMA data transfer. */
		mov	DFCNTRL, DMAPARAMS;
dma_loop:
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			call	idle_loop;
		}
		test	SSTAT0,DMADONE	jnz dma_dmadone;
		test	SSTAT1,PHASEMIS	jz dma_loop;	/* ie. underrun */
dma_phasemis:
		/*
		 * We will be "done" DMAing when the transfer count goes to
		 * zero, or the target changes the phase (in light of this,
		 * it makes sense that the DMA circuitry doesn't ACK when
		 * PHASEMIS is active).  If we are doing a SCSI->Host transfer,
		 * the data FIFO should be flushed auto-magically on STCNT=0
		 * or a phase change, so just wait for FIFO empty status.
		 */
dma_checkfifo:
		test	DFCNTRL,DIRECTION	jnz dma_fifoempty;
dma_fifoflush:
		test	DFSTATUS,FIFOEMP	jz dma_fifoflush;
dma_fifoempty:
		/* Don't clobber an inprogress host data transfer */
		test	DFSTATUS, MREQPEND	jnz dma_fifoempty;

		/*
		 * Now shut off the DMA and make sure that the DMA
		 * hardware has actually stopped.  Touching the DMA
		 * counters, etc. while a DMA is active will result
		 * in an ILLSADDR exception.
		 */
dma_dmadone:
		and	DFCNTRL, ~(SCSIEN|SDMAEN|HDMAEN);
dma_halt:
		/*
		 * Some revisions of the aic78XX have a problem where, if the
		 * data fifo is full, but the PCI input latch is not empty, 
		 * HDMAEN cannot be cleared.  The fix used here is to drain
		 * the prefetched but unused data from the data fifo until
		 * there is space for the input latch to drain.
		 */
		if ((ahc->bugs & AHC_PCI_2_1_RETRY_BUG) != 0)
		{
			mov	NONE, DFDAT;
		}
		test	DFCNTRL, (SCSIEN|SDMAEN|HDMAEN) jnz dma_halt;

		/* See if we have completed this last segment */
		test	STCNT[0], 0xff	jnz data_phase_finish;
		test	STCNT[1], 0xff	jnz data_phase_finish;
		test	STCNT[2], 0xff	jnz data_phase_finish;

		/*
		 * Advance the scatter-gather pointers if needed 
		 */
		if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0
		  && ahc->pci_cachesize != 0)
		{
			test	MWI_RESIDUAL, 0xFF jz no_mwi_resid;
			/*
			 * Reload HADDR from SHADDR and setup the
			 * count to be the size of our residual.
			 */
			if ((ahc->features & AHC_CMD_CHAN) != 0)
			{
				bmov	HADDR, SHADDR, 4;
				mov	HCNT, MWI_RESIDUAL;
				bmov	HCNT[1], ALLZEROS, 2;
			}
			else
			{
				mvi	DINDEX, HADDR;
				mvi	SHADDR call bcopy_4;
				mov	MWI_RESIDUAL call set_hcnt;
			}
			clr	MWI_RESIDUAL;
			jmp	sg_load_done;
no_mwi_resid:
		}
		test	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG jz sg_load;
		or	SCB_RESIDUAL_SGPTR[0], SG_LIST_NULL;
		jmp	data_phase_finish;
sg_load:
		/*
		 * Load the next SG element's data address and length
		 * into the DMA engine.  If we don't have hardware
		 * to perform a prefetch, we'll have to fetch the
		 * segment from host memory first.
		 */
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			/* Wait for the idle loop to complete */
			test	CCSGCTL, CCSGEN jz . + 3;
			call	idle_loop;
			test	CCSGCTL, CCSGEN jnz . - 1;
			bmov 	HADDR, CCSGRAM, 7;
			test	CCSGRAM, SG_LAST_SEG jz . + 2;
			or	SCB_RESIDUAL_DATACNT[3], SG_LAST_SEG;
		}
		else
		{
			mvi	DINDEX, HADDR;
			mvi	SCB_RESIDUAL_SGPTR	call bcopy_4;

			mvi	SG_SIZEOF	call set_hcnt;

			or	DFCNTRL, HDMAEN|DIRECTION|FIFORESET;

			call	dma_finish;

			mvi	DINDEX, HADDR;
			call	dfdat_in_7;
			mov	SCB_RESIDUAL_DATACNT[3], DFDAT;
		}

		if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0
		  && ahc->pci_cachesize != 0)
		{
			call calc_mwi_residual;
		}

		/* Point to the new next sg in memory */
		call	sg_advance;

sg_load_done:
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			bmov	STCNT, HCNT, 3;
		}
		else
		{
			call	set_stcnt_from_hcnt;
		}
		/* Track odd'ness */
		test	HCNT[0], 0x1 jz . + 2;
		xor	DATA_COUNT_ODD, 0x1;

		if ((ahc->flags & AHC_TARGETROLE) != 0)
		{
			test	SSTAT0, TARGET jnz data_phase_loop;
		}
	}

data_phase_finish:
	/*
	 * If the target has left us in data phase, loop through
	 * the dma code again.  In the case of ULTRA2 adapters,
	 * we should only loop if there is a data overrun.  For
	 * all other adapters, we'll loop after each S/G element
	 * is loaded as well as if there is an overrun.
	 */
	if ((ahc->flags & AHC_TARGETROLE) != 0)
	{
		test	SSTAT0, TARGET jnz data_phase_done;
	}
	if ((ahc->flags & AHC_INITIATORROLE) != 0)
	{
		test	SSTAT1, REQINIT jz .;
		test	SSTAT1,PHASEMIS	jz data_phase_loop;
	
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			/* Kill off any pending prefetch */
			clr	CCSGCTL;
			test	CCSGCTL, CCSGEN jnz .;
		}
	}

data_phase_done:
	/*
	 * After a DMA finishes, save the SG and STCNT residuals back into
	 * the SCB.  We use STCNT instead of HCNT, since it's a reflection
	 * of how many bytes were transferred on the SCSI (as opposed to the
	 * host) bus.
	 */
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		/* Kill off any pending prefetch */
		clr	CCSGCTL;
		test	CCSGCTL, CCSGEN jnz .;
	}

	if ((ahc->bugs & AHC_PCI_MWI_BUG) != 0
	  && ahc->pci_cachesize != 0)
	{
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			test	MWI_RESIDUAL, 0xFF jz bmov_resid;
		}
		mov	A, MWI_RESIDUAL;
		add	SCB_RESIDUAL_DATACNT[0], A, STCNT[0];
		clr	A;
		adc	SCB_RESIDUAL_DATACNT[1], A, STCNT[1];
		adc	SCB_RESIDUAL_DATACNT[2], A, STCNT[2];
		clr	MWI_RESIDUAL;
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			jmp	. + 2;
bmov_resid:
			bmov	SCB_RESIDUAL_DATACNT, STCNT, 3;
		}
	}
	else if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	SCB_RESIDUAL_DATACNT, STCNT, 3;
	}
	else
	{
		mov	SCB_RESIDUAL_DATACNT[0], STCNT[0];
		mov	SCB_RESIDUAL_DATACNT[1], STCNT[1];
		mov	SCB_RESIDUAL_DATACNT[2], STCNT[2];
	}

	/*
	 * Since we've been through a data phase, the SCB_RESID* fields
	 * are now initialized.  Clear the full residual flag.
	 */
	and	SCB_SGPTR[0], ~SG_FULL_RESID;

	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		/* Clear the channel in case we return to data phase later */
		or	SXFRCTL0, CLRSTCNT|CLRCHN;
	}

	if ((ahc->flags & AHC_TARGETROLE) != 0)
	{
		test	SEQ_FLAGS, DPHASE_PENDING jz ITloop;
		and	SEQ_FLAGS, ~DPHASE_PENDING;
		/*
		 * For data-in phases, wait for any pending acks from the
		 * initiator before changing phase.
		 */
		test	DFCNTRL, DIRECTION jz target_ITloop;
		test	SSTAT1, REQINIT	jnz .;
		jmp	target_ITloop;
	}
	else
	{
		jmp	ITloop;
	}

if ((ahc->flags & AHC_INITIATORROLE) != 0)
{
/*
 * Command phase.  Set up the DMA registers and let 'er rip.
 */
p_command:
	call	assert;

	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		bmov	HCNT[0], SCB_CDB_LEN,  1;
		bmov	HCNT[1], ALLZEROS, 2;
		mvi	SG_CACHE_PRE, LAST_SEG;
	}
	else if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	STCNT[0], SCB_CDB_LEN, 1;
		bmov	STCNT[1], ALLZEROS, 2;
	}
	else
	{
		mov	STCNT[0], SCB_CDB_LEN;
		clr	STCNT[1];
		clr	STCNT[2];
	}
	add	NONE, -13, SCB_CDB_LEN;
	mvi	SCB_CDB_STORE jnc p_command_embedded;
p_command_from_host:
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		bmov	HADDR[0], SCB_CDB_PTR, 4;
		mvi	DFCNTRL, (PRELOADEN|SCSIEN|HDMAEN|DIRECTION);
	}
	else
	{
		if ((ahc->features & AHC_CMD_CHAN) != 0)
		{
			bmov	HADDR[0], SCB_CDB_PTR, 4;
			bmov	HCNT, STCNT, 3;
		}
		else
		{
			mvi	DINDEX, HADDR;
			mvi	SCB_CDB_PTR call bcopy_4;
			mov	SCB_CDB_LEN call set_hcnt;
		}
		mvi	DFCNTRL, (SCSIEN|SDMAEN|HDMAEN|DIRECTION|FIFORESET);
	}
	jmp	p_command_loop;
p_command_embedded:
	/*
	 * The data fifo seems to require 4 byte alligned
	 * transfers from the sequencer.  Force this to
	 * be the case by clearing HADDR[0] even though
	 * we aren't going to touch host memeory.
	 */
	clr	HADDR[0];
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		mvi	DFCNTRL, (PRELOADEN|SCSIEN|DIRECTION);
		bmov	DFDAT, SCB_CDB_STORE, 12; 
	}
	else if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		if ((ahc->features & AHC_SCB_BTT) != 0)
		{
			/*
			 * On the 7895 the data FIFO will
			 * get corrupted if you try to dump
			 * data from external SCB memory into
			 * the FIFO while it is enabled.  So,
			 * fill the fifo and then enable SCSI
			 * transfers.
			 */
			mvi	DFCNTRL, (DIRECTION|FIFORESET);
		}
		else
		{
			mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFORESET);
		}
		bmov	DFDAT, SCB_CDB_STORE, 12; 
		if ((ahc->features & AHC_SCB_BTT) != 0)
		{
			mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFOFLUSH);
		}
		else
		{
			or	DFCNTRL, FIFOFLUSH;
		}
	}
	else
	{
		mvi	DFCNTRL, (SCSIEN|SDMAEN|DIRECTION|FIFORESET);
		call	copy_to_fifo_6;
		call	copy_to_fifo_6;
		or	DFCNTRL, FIFOFLUSH;
	}
p_command_loop:
	test	SSTAT0, SDONE jnz . + 2;
	test    SSTAT1, PHASEMIS jz p_command_loop;
	/*
	 * Wait for our ACK to go-away on it's own
	 * instead of being killed by SCSIEN getting cleared.
	 */
	test	SCSISIGI, ACKI jnz .;
	and	DFCNTRL, ~(SCSIEN|SDMAEN|HDMAEN);
	test	DFCNTRL, (SCSIEN|SDMAEN|HDMAEN) jnz .;
	if ((ahc->features & AHC_ULTRA2) != 0)
	{
		/* Drop any residual from the S/G Preload queue */
		or	SXFRCTL0, CLRSTCNT;
	}
	jmp	ITloop;

/*
 * Status phase.  Wait for the data byte to appear, then read it
 * and store it into the SCB.
 */
p_status:
	call	assert;

	mov	SCB_SCSI_STATUS, SCSIDATL;
	jmp	ITloop;

/*
 * Message out phase.  If MSG_OUT is MSG_IDENTIFYFLAG, build a full
 * indentify message sequence and send it to the target.  The host may
 * override this behavior by setting the MK_MESSAGE bit in the SCB
 * control byte.  This will cause us to interrupt the host and allow
 * it to handle the message phase completely on its own.  If the bit
 * associated with this target is set, we will also interrupt the host,
 * thereby allowing it to send a message on the next selection regardless
 * of the transaction being sent.
 * 
 * If MSG_OUT is == HOST_MSG, also interrupt the host and take a message.
 * This is done to allow the host to send messages outside of an identify
 * sequence while protecting the seqencer from testing the MK_MESSAGE bit
 * on an SCB that might not be for the current nexus. (For example, a
 * BDR message in responce to a bad reselection would leave us pointed to
 * an SCB that doesn't have anything to do with the current target).
 *
 * Otherwise, treat MSG_OUT as a 1 byte message to send (abort, abort tag,
 * bus device reset).
 *
 * When there are no messages to send, MSG_OUT should be set to MSG_NOOP,
 * in case the target decides to put us in this phase for some strange
 * reason.
 */
p_mesgout_retry:
	or	SCSISIGO,ATNO,LASTPHASE;/* turn on ATN for the retry */
p_mesgout:
	mov	SINDEX, MSG_OUT;
	cmp	SINDEX, MSG_IDENTIFYFLAG jne p_mesgout_from_host;
	test	SCB_CONTROL,MK_MESSAGE	jnz host_message_loop;
	mov	FUNCTION1, SCB_SCSIID;
	mov	A, FUNCTION1;
	mov	SINDEX, TARGET_MSG_REQUEST[0];
	if ((ahc->features & AHC_TWIN) != 0)
	{
		/* Second Channel uses high byte bits */
		test	SCB_SCSIID, TWIN_CHNLB jz . + 2;
		mov	SINDEX, TARGET_MSG_REQUEST[1];
	}
	else if ((ahc->features & AHC_WIDE) != 0)
	{
		test	SCB_SCSIID, 0x80	jz . + 2; /* target > 7 */
		mov	SINDEX, TARGET_MSG_REQUEST[1];
	}
	test	SINDEX, A	jnz host_message_loop;
p_mesgout_identify:
	or	SINDEX, MSG_IDENTIFYFLAG|DISCENB, SCB_LUN;
	test	SCB_CONTROL, DISCENB jnz . + 2;
	and	SINDEX, ~DISCENB;
/*
 * Send a tag message if TAG_ENB is set in the SCB control block.
 * Use SCB_TAG (the position in the kernel's SCB array) as the tag value.
 */
p_mesgout_tag:
	test	SCB_CONTROL,TAG_ENB jz  p_mesgout_onebyte;
	mov	SCSIDATL, SINDEX;	/* Send the identify message */
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	and	SCSIDATL,TAG_ENB|SCB_TAG_TYPE,SCB_CONTROL;
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	jne p_mesgout_done;
	mov	SCB_TAG	jmp p_mesgout_onebyte;
/*
 * Interrupt the driver, and allow it to handle this message
 * phase and any required retries.
 */
p_mesgout_from_host:
	cmp	SINDEX, HOST_MSG	jne p_mesgout_onebyte;
	jmp	host_message_loop;

p_mesgout_onebyte:
	mvi	CLRSINT1, CLRATNO;
	mov	SCSIDATL, SINDEX;

/*
 * If the next bus phase after ATN drops is message out, it means
 * that the target is requesting that the last message(s) be resent.
 */
	call	phase_lock;
	cmp	LASTPHASE, P_MESGOUT	je p_mesgout_retry;

p_mesgout_done:
	mvi	CLRSINT1,CLRATNO;	/* Be sure to turn ATNO off */
	mov	LAST_MSG, MSG_OUT;
	mvi	MSG_OUT, MSG_NOOP;	/* No message left */
	jmp	ITloop;

/*
 * Message in phase.  Bytes are read using Automatic PIO mode.
 */
p_mesgin:
	mvi	ACCUM		call inb_first;	/* read the 1st message byte */

	test	A,MSG_IDENTIFYFLAG	jnz mesgin_identify;
	cmp	A,MSG_DISCONNECT	je mesgin_disconnect;
	cmp	A,MSG_SAVEDATAPOINTER	je mesgin_sdptrs;
	cmp	ALLZEROS,A		je mesgin_complete;
	cmp	A,MSG_RESTOREPOINTERS	je mesgin_rdptrs;
	cmp	A,MSG_IGN_WIDE_RESIDUE	je mesgin_ign_wide_residue;
	cmp	A,MSG_NOOP		je mesgin_done;

/*
 * Pushed message loop to allow the kernel to
 * run it's own message state engine.  To avoid an
 * extra nop instruction after signaling the kernel,
 * we perform the phase_lock before checking to see
 * if we should exit the loop and skip the phase_lock
 * in the ITloop.  Performing back to back phase_locks
 * shouldn't hurt, but why do it twice...
 */
host_message_loop:
	mvi	INTSTAT, HOST_MSG_LOOP;
	call	phase_lock;
	cmp	RETURN_1, EXIT_MSG_LOOP	je ITloop + 1;
	jmp	host_message_loop;

mesgin_ign_wide_residue:
if ((ahc->features & AHC_WIDE) != 0)
{
	test	SCSIRATE, WIDEXFER jz mesgin_reject;
	/* Pull the residue byte */
	mvi	ARG_1	call inb_next;
	cmp	ARG_1, 0x01 jne mesgin_reject;
	test	SCB_RESIDUAL_SGPTR[0], SG_LIST_NULL jz . + 2;
	test	DATA_COUNT_ODD, 0x1	jz mesgin_done;
	mvi	INTSTAT, IGN_WIDE_RES;
	jmp	mesgin_done;
}

mesgin_reject:
	mvi	MSG_MESSAGE_REJECT	call mk_mesg;
mesgin_done:
	mov	NONE,SCSIDATL;		/*dummy read from latch to ACK*/
	jmp	ITloop;

mesgin_complete:
/*
 * We received a "command complete" message.  Put the SCB_TAG into the QOUTFIFO,
 * and trigger a completion interrupt.  Before doing so, check to see if there
 * is a residual or the status byte is something other than STATUS_GOOD (0).
 * In either of these conditions, we upload the SCB back to the host so it can
 * process this information.  In the case of a non zero status byte, we 
 * additionally interrupt the kernel driver synchronously, allowing it to
 * decide if sense should be retrieved.  If the kernel driver wishes to request
 * sense, it will fill the kernel SCB with a request sense command, requeue
 * it to the QINFIFO and tell us not to post to the QOUTFIFO by setting 
 * RETURN_1 to SEND_SENSE.
 */

/*
 * First check for residuals
 */
	test	SCB_SGPTR, SG_LIST_NULL jnz check_status;/* No xfer */
	test	SCB_SGPTR, SG_FULL_RESID jnz upload_scb;/* Never xfered */
	test	SCB_RESIDUAL_SGPTR, SG_LIST_NULL jz upload_scb;
check_status:
	test	SCB_SCSI_STATUS,0xff	jz complete;	/* Good Status? */
upload_scb:
	or	SCB_SGPTR, SG_RESID_VALID;
	mvi	DMAPARAMS, FIFORESET;
	mov	SCB_TAG		call dma_scb;
	test	SCB_SCSI_STATUS, 0xff	jz complete;	/* Just a residual? */
	mvi	INTSTAT, BAD_STATUS;			/* let driver know */
	nop;
	cmp	RETURN_1, SEND_SENSE	jne complete;
	call	add_scb_to_free_list;
	jmp	await_busfree;
complete:
	mov	SCB_TAG call complete_post;
	jmp	await_busfree;
}

complete_post:
	/* Post the SCBID in SINDEX and issue an interrupt */
	call	add_scb_to_free_list;
	mov	ARG_1, SINDEX;
	if ((ahc->features & AHC_QUEUE_REGS) != 0)
	{
		mov	A, SDSCB_QOFF;
	}
	else
	{
		mov	A, QOUTPOS;
	}
	mvi	QOUTFIFO_OFFSET call post_byte_setup;
	mov	ARG_1 call post_byte;
	if ((ahc->features & AHC_QUEUE_REGS) == 0)
	{
		inc 	QOUTPOS;
	}
	mvi	INTSTAT,CMDCMPLT ret;

if ((ahc->flags & AHC_INITIATORROLE) != 0)
{
/*
 * Is it a disconnect message?  Set a flag in the SCB to remind us
 * and await the bus going free.  If this is an untagged transaction
 * store the SCB id for it in our untagged target table for lookup on
 * a reselction.
 */
mesgin_disconnect:
	or	SCB_CONTROL,DISCONNECTED;
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		call	add_scb_to_disc_list;
	}
	test	SCB_CONTROL, TAG_ENB jnz await_busfree;
	mov	ARG_1, SCB_TAG;
	mov	SAVED_LUN, SCB_LUN;
	mov	SCB_SCSIID	call set_busy_target;
	jmp	await_busfree;

/*
 * Save data pointers message:
 * Copying RAM values back to SCB, for Save Data Pointers message, but
 * only if we've actually been into a data phase to change them.  This
 * protects against bogus data in scratch ram and the residual counts
 * since they are only initialized when we go into data_in or data_out.
 */
mesgin_sdptrs:
	test	SEQ_FLAGS, DPHASE	jz mesgin_done;

	/*
	 * The SCB_SGPTR becomes the next one we'll download,
	 * and the SCB_DATAPTR becomes the current SHADDR.
	 * Use the residual number since STCNT is corrupted by
	 * any message transfer.
	 */
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	SCB_DATAPTR, SHADDR, 4;
		bmov	SCB_DATACNT, SCB_RESIDUAL_DATACNT, 8;
	}
	else
	{
		mvi	DINDEX, SCB_DATAPTR;
		mvi	SHADDR call bcopy_4;
		mvi	SCB_RESIDUAL_DATACNT call bcopy_8;
	}
	jmp	mesgin_done;

/*
 * Restore pointers message?  Data pointers are recopied from the
 * SCB anytime we enter a data phase for the first time, so all
 * we need to do is clear the DPHASE flag and let the data phase
 * code do the rest.
 */
mesgin_rdptrs:
	and	SEQ_FLAGS, ~DPHASE;		/*
						 * We'll reload them
						 * the next time through
						 * the dataphase.
						 */
	jmp	mesgin_done;

/*
 * Index into our Busy Target table.  SINDEX and DINDEX are modified
 * upon return.  SCBPTR may be modified by this action.
 */
set_busy_target:
	shr	DINDEX, 4, SINDEX;
	if ((ahc->features & AHC_SCB_BTT) != 0)
	{
		mov	SCBPTR, SAVED_LUN;
		add	DINDEX, SCB_64_BTT;
	}
	else
	{
		add	DINDEX, BUSY_TARGETS;
	}
	mov	DINDIR, ARG_1 ret;

/*
 * Identify message?  For a reconnecting target, this tells us the lun
 * that the reconnection is for - find the correct SCB and switch to it,
 * clearing the "disconnected" bit so we don't "find" it by accident later.
 */
mesgin_identify:
	and	SAVED_LUN, MSG_IDENTIFY_LUNMASK, A;
	/*
	 * Determine whether a target is using tagged or non-tagged
	 * transactions by first looking at the transaction stored in
	 * the busy target array.  If there is no untagged transaction
	 * for this target or the transaction is for a different lun, then
	 * this must be an untagged transaction.
	 */
fetch_busy_target:
	shr	A, 4, SAVED_SCSIID;
	if ((ahc->features & AHC_SCB_BTT) != 0)
	{
		add	SINDEX, SCB_64_BTT, A;
		mov	SCBPTR, SAVED_LUN;
	}
	else
	{
		add	SINDEX, BUSY_TARGETS, A;
		if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
			add	A, -BUSY_TARGETS, SINDEX;
			jc	. + 2;
			mvi	INTSTAT, OUT_OF_RANGE;
			add	A, -(BUSY_TARGETS + 16), SINDEX;
			jnc	. + 2;
			mvi	INTSTAT, OUT_OF_RANGE;
		}
	}
	mov	ARG_1, SINDIR;
	cmp	ARG_1, SCB_LIST_NULL	je snoop_tag;
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		mov	RETURN_1 call findSCB;
	}
	else
	{
		mov	SCBPTR, RETURN_1;
	}
	if ((ahc->features & AHC_SCB_BTT) != 0)
	{
		jmp setup_SCB_id_lun_okay;
	}
	else
	{
		mov	A, SCB_LUN;
		cmp	SAVED_LUN, A		je setup_SCB_id_lun_okay;
	}
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		call	add_scb_to_disc_list;
	}

/*
 * Here we "snoop" the bus looking for a SIMPLE QUEUE TAG message.
 * If we get one, we use the tag returned to find the proper
 * SCB.  With SCB paging, we must search for non-tagged
 * transactions since the SCB may exist in any slot.  If we're not
 * using SCB paging, we can use the tag as the direct index to the
 * SCB.
 */
snoop_tag:
	mov	NONE,SCSIDATL;		/* ACK Identify MSG */
	call	phase_lock;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		or	SEQ_FLAGS, 0x1;
	}
	cmp	LASTPHASE, P_MESGIN	jne not_found;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		or	SEQ_FLAGS, 0x2;
	}
	cmp	SCSIBUSL,MSG_SIMPLE_Q_TAG jne not_found;
get_tag:
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		mvi	ARG_1	call inb_next;	/* tag value */
		mov	ARG_1	call findSCB;
	}
	else
	{
		mvi	ARG_1	call inb_next;	/* tag value */
		mov	SCBPTR, ARG_1;
	}

/*
 * Ensure that the SCB the tag points to is for
 * an SCB transaction to the reconnecting target.
 */
setup_SCB:
	mov	A, SAVED_SCSIID;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		or	SEQ_FLAGS, 0x4;
	}
	cmp	SCB_SCSIID, A	jne not_found_cleanup_scb;
	mov	A, SAVED_LUN;
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		or	SEQ_FLAGS, 0x8;
	}
	cmp	SCB_LUN, A	jne not_found_cleanup_scb;
setup_SCB_id_lun_okay:
	if ((ahc->flags & AHC_SEQUENCER_DEBUG) != 0)
	{
		or	SEQ_FLAGS, 0x10;
	}
	test	SCB_CONTROL,DISCONNECTED jz not_found_cleanup_scb;
	and	SCB_CONTROL,~DISCONNECTED;
	mvi	SEQ_FLAGS,IDENTIFY_SEEN;	/* make note of IDENTIFY */
	test	SCB_CONTROL, TAG_ENB	jnz setup_SCB_tagged;
	mov	A, SCBPTR;
	mvi	ARG_1, SCB_LIST_NULL;
	mov	SAVED_SCSIID	call	set_busy_target;
	mov	SCBPTR, A;
setup_SCB_tagged:
	call	set_transfer_settings;
	/* See if the host wants to send a message upon reconnection */
	test	SCB_CONTROL, MK_MESSAGE jz mesgin_done;
	mvi	HOST_MSG	call mk_mesg;
	jmp	mesgin_done;

not_found_cleanup_scb:
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
		call	add_scb_to_free_list;
	}
not_found:
	mvi	INTSTAT, NO_MATCH;
	jmp	mesgin_done;

mk_mesg:
	or	SCSISIGO,ATNO,LASTPHASE;/* turn on ATNO */
	mov	MSG_OUT,SINDEX ret;

/*
 * Functions to read data in Automatic PIO mode.
 *
 * According to Adaptec's documentation, an ACK is not sent on input from
 * the target until SCSIDATL is read from.  So we wait until SCSIDATL is
 * latched (the usual way), then read the data byte directly off the bus
 * using SCSIBUSL.  When we have pulled the ATN line, or we just want to
 * acknowledge the byte, then we do a dummy read from SCISDATL.  The SCSI
 * spec guarantees that the target will hold the data byte on the bus until
 * we send our ACK.
 *
 * The assumption here is that these are called in a particular sequence,
 * and that REQ is already set when inb_first is called.  inb_{first,next}
 * use the same calling convention as inb.
 */
inb_next_wait_perr:
	mvi	INTSTAT, PERR_DETECTED;
	jmp	inb_next_wait;
inb_next:
	mov	NONE,SCSIDATL;		/*dummy read from latch to ACK*/
inb_next_wait:
	/*
	 * If there is a parity error, wait for the kernel to
	 * see the interrupt and prepare our message response
	 * before continuing.
	 */
	test	SSTAT1, REQINIT	jz inb_next_wait;
	test	SSTAT1, SCSIPERR jnz inb_next_wait_perr;
inb_next_check_phase:
	and	LASTPHASE, PHASE_MASK, SCSISIGI;
	cmp	LASTPHASE, P_MESGIN jne mesgin_phasemis;
inb_first:
	mov	DINDEX,SINDEX;
	mov	DINDIR,SCSIBUSL	ret;		/*read byte directly from bus*/
inb_last:
	mov	NONE,SCSIDATL ret;		/*dummy read from latch to ACK*/
}

if ((ahc->flags & AHC_TARGETROLE) != 0)
{
/*
 * Change to a new phase.  If we are changing the state of the I/O signal,
 * from out to in, wait an additional data release delay before continuing.
 */
change_phase:
	/* Wait for preceeding I/O session to complete. */
	test	SCSISIGI, ACKI jnz .;

	/* Change the phase */
	and	DINDEX, IOI, SCSISIGI;
	mov	SCSISIGO, SINDEX;
	and	A, IOI, SINDEX;

	/*
	 * If the data direction has changed, from
	 * out (initiator driving) to in (target driving),
	 * we must wait at least a data release delay plus
	 * the normal bus settle delay. [SCSI III SPI 10.11.0]
	 */
	cmp 	DINDEX, A je change_phase_wait;
	test	SINDEX, IOI jz change_phase_wait;
	call	change_phase_wait;
change_phase_wait:
	nop;
	nop;
	nop;
	nop ret;

/*
 * Send a byte to an initiator in Automatic PIO mode.
 */
target_outb:
	or	SXFRCTL0, SPIOEN;
	test	SSTAT0, SPIORDY	jz .;
	mov	SCSIDATL, SINDEX;
	test	SSTAT0, SPIORDY	jz .;
	and	SXFRCTL0, ~SPIOEN ret;
}
	

/*
 * Assert that if we've been reselected, then we've seen an IDENTIFY
 * message.
 */
assert:
	test	SEQ_FLAGS,IDENTIFY_SEEN	jnz return;	/* seen IDENTIFY? */

	mvi	INTSTAT,NO_IDENT 	ret;	/* no - tell the kernel */

/*
 * Locate a disconnected SCB by SCBID.  Upon return, SCBPTR and SINDEX will
 * be set to the position of the SCB.  If the SCB cannot be found locally,
 * it will be paged in from host memory.  RETURN_2 stores the address of the
 * preceding SCB in the disconnected list which can be used to speed up
 * removal of the found SCB from the disconnected list.
 */
if ((ahc->flags & AHC_PAGESCBS) != 0)
{
findSCB:
	mov	A, SINDEX;			/* Tag passed in SINDEX */
	cmp	DISCONNECTED_SCBH, SCB_LIST_NULL je findSCB_notFound;
	mov	SCBPTR, DISCONNECTED_SCBH;	/* Initialize SCBPTR */
	mvi	ARG_2, SCB_LIST_NULL;		/* Head of list */
	jmp	findSCB_loop;
findSCB_next:
	cmp	SCB_NEXT, SCB_LIST_NULL je findSCB_notFound;
	mov	ARG_2, SCBPTR;
	mov	SCBPTR,SCB_NEXT;
findSCB_loop:
	cmp	SCB_TAG, A	jne findSCB_next;
rem_scb_from_disc_list:
	cmp	ARG_2, SCB_LIST_NULL	je rHead;
	mov	DINDEX, SCB_NEXT;
	mov	SINDEX, SCBPTR;
	mov	SCBPTR, ARG_2;
	mov	SCB_NEXT, DINDEX;
	mov	SCBPTR, SINDEX ret;
rHead:
	mov	DISCONNECTED_SCBH,SCB_NEXT ret;
findSCB_notFound:
	/*
	 * We didn't find it.  Page in the SCB.
	 */
	mov	ARG_1, A; /* Save tag */
	mov	ALLZEROS call get_free_or_disc_scb;
	mvi	DMAPARAMS, HDMAEN|DIRECTION|FIFORESET;
	mov	ARG_1	jmp dma_scb;
}

/*
 * Prepare the hardware to post a byte to host memory given an
 * index of (A + (256 * SINDEX)) and a base address of SHARED_DATA_ADDR.
 */
post_byte_setup:
	mov	ARG_2, SINDEX;
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		mvi	DINDEX, CCHADDR;
		mvi	SHARED_DATA_ADDR call	set_1byte_addr;
		mvi	CCHCNT, 1;
		mvi	CCSCBCTL, CCSCBRESET ret;
	}
	else
	{
		mvi	DINDEX, HADDR;
		mvi	SHARED_DATA_ADDR call	set_1byte_addr;
		mvi	1	call set_hcnt;
		mvi	DFCNTRL, FIFORESET ret;
	}

post_byte:
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		bmov	CCSCBRAM, SINDEX, 1;
		or	CCSCBCTL, CCSCBEN|CCSCBRESET;
		test	CCSCBCTL, CCSCBDONE jz .;
		clr	CCSCBCTL ret;
	}
	else
	{
		mov	DFDAT, SINDEX;
		or	DFCNTRL, HDMAEN|FIFOFLUSH;
		jmp	dma_finish;
	}

phase_lock_perr:
	mvi	INTSTAT, PERR_DETECTED;
phase_lock:     
	/*
	 * If there is a parity error, wait for the kernel to
	 * see the interrupt and prepare our message response
	 * before continuing.
	 */
	test	SSTAT1, REQINIT jz phase_lock;
	test	SSTAT1, SCSIPERR jnz phase_lock_perr;
phase_lock_latch_phase:
	and	SCSISIGO, PHASE_MASK, SCSISIGI;
	and	LASTPHASE, PHASE_MASK, SCSISIGI ret;

if ((ahc->features & AHC_CMD_CHAN) == 0)
{
set_hcnt:
	mov	HCNT[0], SINDEX;
clear_hcnt:
	clr	HCNT[1];
	clr	HCNT[2] ret;

set_stcnt_from_hcnt:
	mov	STCNT[0], HCNT[0];
	mov	STCNT[1], HCNT[1];
	mov	STCNT[2], HCNT[2] ret;

bcopy_8:
	mov	DINDIR, SINDIR;
bcopy_7:
	mov	DINDIR, SINDIR;
	mov	DINDIR, SINDIR;
bcopy_5:
	mov	DINDIR, SINDIR;
bcopy_4:
	mov	DINDIR, SINDIR;
bcopy_3:
	mov	DINDIR, SINDIR;
	mov	DINDIR, SINDIR;
	mov	DINDIR, SINDIR ret;
}

if ((ahc->flags & AHC_TARGETROLE) != 0)
{
/*
 * Setup addr assuming that A is an index into
 * an array of 32byte objects, SINDEX contains
 * the base address of that array, and DINDEX
 * contains the base address of the location
 * to store the indexed address.
 */
set_32byte_addr:
	shr	ARG_2, 3, A;
	shl	A, 5;
	jmp	set_1byte_addr;
}

/*
 * Setup addr assuming that A is an index into
 * an array of 64byte objects, SINDEX contains
 * the base address of that array, and DINDEX
 * contains the base address of the location
 * to store the indexed address.
 */
set_64byte_addr:
	shr	ARG_2, 2, A;
	shl	A, 6;

/*
 * Setup addr assuming that A + (ARG_2 * 256) is an
 * index into an array of 1byte objects, SINDEX contains
 * the base address of that array, and DINDEX contains
 * the base address of the location to store the computed
 * address.
 */
set_1byte_addr:
	add     DINDIR, A, SINDIR;
	mov     A, ARG_2;
	adc	DINDIR, A, SINDIR;
	clr	A;
	adc	DINDIR, A, SINDIR;
	adc	DINDIR, A, SINDIR ret;

/*
 * Either post or fetch and SCB from host memory based on the
 * DIRECTION bit in DMAPARAMS. The host SCB index is in SINDEX.
 */
dma_scb:
	mov	A, SINDEX;
	if ((ahc->features & AHC_CMD_CHAN) != 0)
	{
		mvi	DINDEX, CCHADDR;
		mvi	HSCB_ADDR call set_64byte_addr;
		mov	CCSCBPTR, SCBPTR;
		test	DMAPARAMS, DIRECTION jz dma_scb_tohost;
		if ((ahc->features & AHC_SCB_BTT) != 0)
		{
			mvi	CCHCNT, SCB_DOWNLOAD_SIZE_64;
		}
		else
		{
			mvi	CCHCNT, SCB_DOWNLOAD_SIZE;
		}
		mvi	CCSCBCTL, CCARREN|CCSCBEN|CCSCBDIR|CCSCBRESET;
		cmp	CCSCBCTL, CCSCBDONE|ARRDONE|CCARREN|CCSCBEN|CCSCBDIR jne .;
		jmp	dma_scb_finish;
dma_scb_tohost:
		mvi	CCHCNT, SCB_UPLOAD_SIZE;
		if ((ahc->features & AHC_ULTRA2) == 0)
		{
			mvi	CCSCBCTL, CCSCBRESET;
			bmov	CCSCBRAM, SCB_BASE, SCB_UPLOAD_SIZE;
			or	CCSCBCTL, CCSCBEN|CCSCBRESET;
			test	CCSCBCTL, CCSCBDONE jz .;
		}
		else if ((ahc->bugs & AHC_SCBCHAN_UPLOAD_BUG) != 0)
		{
			mvi	CCSCBCTL, CCARREN|CCSCBRESET;
			cmp	CCSCBCTL, ARRDONE|CCARREN jne .;
			mvi	CCHCNT, SCB_UPLOAD_SIZE;
			mvi	CCSCBCTL, CCSCBEN|CCSCBRESET;
			cmp	CCSCBCTL, CCSCBDONE|CCSCBEN jne .;
		}
		else
		{
			mvi	CCSCBCTL, CCARREN|CCSCBEN|CCSCBRESET;
			cmp	CCSCBCTL, CCSCBDONE|ARRDONE|CCARREN|CCSCBEN jne .;
		}
dma_scb_finish:
		clr	CCSCBCTL;
		test	CCSCBCTL, CCARREN|CCSCBEN jnz .;
		ret;
	}
	else
	{
		mvi	DINDEX, HADDR;
		mvi	HSCB_ADDR call set_64byte_addr;
		mvi	SCB_DOWNLOAD_SIZE call set_hcnt;
		mov	DFCNTRL, DMAPARAMS;
		test	DMAPARAMS, DIRECTION	jnz dma_scb_fromhost;
		/* Fill it with the SCB data */
copy_scb_tofifo:
		mvi	SINDEX, SCB_BASE;
		add	A, SCB_DOWNLOAD_SIZE, SINDEX;
copy_scb_tofifo_loop:
		call	copy_to_fifo_8;
		cmp	SINDEX, A jne copy_scb_tofifo_loop;
		or	DFCNTRL, HDMAEN|FIFOFLUSH;
		jmp	dma_finish;
dma_scb_fromhost:
		mvi	DINDEX, SCB_BASE;
		if ((ahc->bugs & AHC_PCI_2_1_RETRY_BUG) != 0)
		{
			/*
			 * The PCI module will only issue a PCI
			 * retry if the data FIFO is empty.  If the
			 * host disconnects in the middle of a
			 * transfer, we must empty the fifo of all
			 * available data to force the chip to
			 * continue the transfer.  This does not
			 * happen for SCSI transfers as the SCSI module
			 * will drain the FIFO as data is made available.
			 * When the hang occurs, we know that at least
			 * 8 bytes are in the FIFO because the PCI
			 * module has an 8 byte input latch that only
			 * dumps to the FIFO when HCNT == 0 or the
			 * latch is full.
			 */
			mvi	A, -24;
			/* Wait for some data to arrive. */
dma_scb_hang_fifo:
			test	DFSTATUS, FIFOEMP jnz dma_scb_hang_fifo;
dma_scb_hang_wait:
			test	DFSTATUS, MREQPEND jnz dma_scb_hang_wait;
			test	DFSTATUS, HDONE	jnz dma_scb_hang_dma_done;
			test	DFSTATUS, HDONE	jnz dma_scb_hang_dma_done;
			test	DFSTATUS, HDONE	jnz dma_scb_hang_dma_done;
			/*
			 * The PCI no longer intends to perform a PCI
			 * transaction and HDONE has not come true.
			 * We are hung.  Drain the fifo.
			 */
dma_scb_hang_empty_fifo:
			call	dfdat_in_8;
			add	A, 8;
			add	SINDEX, A, HCNT; 
			/*
			 * The result will be <= 0 (carry set) if at
			 * least 8 bytes of data have been placed
			 * into the fifo.
			 */
			jc	dma_scb_hang_empty_fifo;
			jmp	dma_scb_hang_fifo;
dma_scb_hang_dma_done:
			and	DFCNTRL, ~HDMAEN;
			test	DFCNTRL, HDMAEN jnz .;
			call	dfdat_in_8;
			add	A, 8;
			cmp	A, 8 jne . - 2;
		}
		else
		{
			call	dma_finish;
			/* If we were putting the SCB, we are done */
			call	dfdat_in_8;
			call	dfdat_in_8;
			call	dfdat_in_8;
		}
dfdat_in_8:
		mov	DINDIR,DFDAT;
dfdat_in_7:
		mov	DINDIR,DFDAT;
		mov	DINDIR,DFDAT;
		mov	DINDIR,DFDAT;
		mov	DINDIR,DFDAT;
		mov	DINDIR,DFDAT;
dfdat_in_2:
		mov	DINDIR,DFDAT;
		mov	DINDIR,DFDAT ret;
	}

copy_to_fifo_8:
	mov	DFDAT,SINDIR;
	mov	DFDAT,SINDIR;
copy_to_fifo_6:
	mov	DFDAT,SINDIR;
copy_to_fifo_5:
	mov	DFDAT,SINDIR;
copy_to_fifo_4:
	mov	DFDAT,SINDIR;
	mov	DFDAT,SINDIR;
	mov	DFDAT,SINDIR;
	mov	DFDAT,SINDIR ret;

/*
 * Wait for DMA from host memory to data FIFO to complete, then disable
 * DMA and wait for it to acknowledge that it's off.
 */
dma_finish:
	test	DFSTATUS,HDONE	jz dma_finish;
	/* Turn off DMA */
	and	DFCNTRL, ~HDMAEN;
	test	DFCNTRL, HDMAEN jnz .;
	ret;

add_scb_to_free_list:
	if ((ahc->flags & AHC_PAGESCBS) != 0)
	{
BEGIN_CRITICAL
		mov	SCB_NEXT, FREE_SCBH;
		mvi	SCB_TAG, SCB_LIST_NULL;
		mov	FREE_SCBH, SCBPTR ret;
END_CRITICAL
	}
	else
	{
		mvi	SCB_TAG, SCB_LIST_NULL ret;
	}

if ((ahc->flags & AHC_PAGESCBS) != 0)
{
get_free_or_disc_scb:
	cmp	FREE_SCBH, SCB_LIST_NULL jne dequeue_free_scb;
	cmp	DISCONNECTED_SCBH, SCB_LIST_NULL jne dequeue_disc_scb;
return_error:
	mvi	INTSTAT, NO_FREE_SCB;
	mvi	SINDEX, SCB_LIST_NULL	ret;
dequeue_disc_scb:
	mov	SCBPTR, DISCONNECTED_SCBH;
dma_up_scb:
	mvi	DMAPARAMS, FIFORESET;
	mov	SCB_TAG	call dma_scb;
unlink_disc_scb:
	mov	DISCONNECTED_SCBH, SCB_NEXT ret;
dequeue_free_scb:
	mov	SCBPTR, FREE_SCBH;
	mov	FREE_SCBH, SCB_NEXT ret;

add_scb_to_disc_list:
/*
 * Link this SCB into the DISCONNECTED list.  This list holds the
 * candidates for paging out an SCB if one is needed for a new command.
 * Modifying the disconnected list is a critical(pause dissabled) section.
 */
BEGIN_CRITICAL
	mov	SCB_NEXT, DISCONNECTED_SCBH;
	mov	DISCONNECTED_SCBH, SCBPTR ret;
END_CRITICAL
}
return:
	ret;
