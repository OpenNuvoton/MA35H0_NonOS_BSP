/**************************************************************************//**
 * @file    ehci.c
 * @brief   USB Host library EHCI (USB 2.0) host controller driver.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/// @cond HIDDEN_SYMBOLS

static QH_T  *_H_qh;                  /* head of reclamation list (non-cache ptr)   */
static qTD_T *_ghost_qtd;             /* used as a terminator qTD                   */
static QH_T  *qh_remove_list;         /* list of QHs to be removed (non-cache ptr)  */

static uint32_t _PA_PFList[FL_SIZE] __attribute__((aligned(4096)));  /* Periodic frame list */
static uint32_t *_PFList;

static QH_T  * _Iqh[NUM_IQH];         /* periodic QHs (non-cache ptr)  */

static ISO_EP_T  *iso_ep_list;        /* list of activated isochronous pipes (non-cache ptr)  */

static int  ehci_quit_iso_xfer(UTR_T *utr, EP_INFO_T *ep);
static void scan_isochronous_list(void);

#ifdef ENABLE_ERROR_MSG
static void dump_ehci_regs(void)
{
	USB_debug("Dump HSUSBH(0x%x) registers:\n", ptr_to_u32(&_ehci->EHCVNR));
	USB_debug("    EHCVNR   = 0x%x\n", _ehci->EHCVNR);
	USB_debug("    EHCSPR   = 0x%x\n", _ehci->EHCSPR);
	USB_debug("    EHCCPR   = 0x%x\n", _ehci->EHCCPR);
	USB_debug("    UCMDR    = 0x%x\n", _ehci->UCMDR);
	USB_debug("    USTSR    = 0x%x\n", _ehci->USTSR);
	USB_debug("    UIENR    = 0x%x\n", _ehci->UIENR);
	USB_debug("    UFINDR   = 0x%x\n", _ehci->UFINDR);
	USB_debug("    UPFLBAR  = 0x%x\n", _ehci->UPFLBAR);
	USB_debug("    UCALAR   = 0x%x\n", _ehci->UCALAR);
	USB_debug("    UCFGR    = 0x%x\n", _ehci->UCFGR);
	USB_debug("    UPSCR0   = 0x%x\n", _ehci->UPSCR[0]);
//    USB_debug("    PHYCTL0  = 0x%x\n", _ehci->USBPCR0);
//    USB_debug("    PHYCTL1  = 0x%x\n", _ehci->USBPCR1);
}

static void dump_ehci_ports()
{
	USB_debug("_ehci port0=0x%x, port1=0x%x\n", _ehci->UPSCR[0], _ehci->UPSCR[1]);
}

static void dump_ehci_qtd(qTD_T *qtd)
{
	USB_debug("    [qTD] - 0x%08x\n", (int64_t)qtd);
	USB_debug("        0x%08x (Next qtd Pointer)\n", qtd->Next_qTD);
	USB_debug("        0x%08x (Alternate Next qtd Pointer)\n", qtd->Alt_Next_qTD);
	USB_debug("        0x%08x (qtd Token) PID: %s, Bytes: %d, IOC: %d\n", qtd->Token, (((qtd->Token>>8)&0x3)==0) ? "OUT" : ((((qtd->Token>>8)&0x3)==1) ? "IN" : "SETUP"), (qtd->Token>>16)&0x7FFF, (qtd->Token>>15)&0x1);
	USB_debug("        0x%08x (Buffer Pointer (page 0))\n", qtd->Bptr[0]);
	//USB_debug("        0x%08x (Buffer Pointer (page 1))\n", qtd->Bptr[1]);
	//USB_debug("        0x%08x (Buffer Pointer (page 2))\n", qtd->Bptr[2]);
	//USB_debug("        0x%08x (Buffer Pointer (page 3))\n", qtd->Bptr[3]);
	//USB_debug("        0x%08x (Buffer Pointer (page 4))\n", qtd->Bptr[4]);
	USB_debug("\n");
}

static void dump_ehci_asynclist(void)
{
	QH_T     *qh = _H_qh;
	qTD_T    *qtd;

	USB_debug(">>> Dump EHCI Asynchronous List <<<\n");
	do {
		USB_debug("[QH] - 0x%08x\n", (int64_t)qh);
		USB_debug("    0x%08x (Queue Head Horizontal Link Pointer, Queue Head DWord 0)\n", qh->HLink);
		USB_debug("    0x%08x (Endpoint Characteristics) DevAddr: %d, EP: 0x%x, PktSz: %d, Speed: %s\n", qh->Chrst, qh->Chrst&0x7F, (qh->Chrst>>8)&0xF, (qh->Chrst>>16)&0x7FF, ((qh->Chrst>>12)&0x3 == 0) ? "Full" : (((qh->Chrst>>12)&0x3 == 1) ? "Low" : "High"));
		USB_debug("    0x%08x (Endpoint Capabilities: Queue Head DWord 2)\n", qh->Cap);
		USB_debug("    0x%08x (Current qtd Pointer)\n", qh->Curr_qTD);
		USB_debug("    --- Overlay Area ---\n");
		USB_debug("    0x%08x (Next qtd Pointer)\n", qh->OL_Next_qTD);
		USB_debug("    0x%08x (Alternate Next qtd Pointer)\n", qh->OL_Alt_Next_qTD);
		USB_debug("    0x%08x (qtd Token)\n", qh->OL_Token);
		USB_debug("    0x%08x (Buffer Pointer (page 0))\n", qh->OL_Bptr[0]);
		USB_debug("\n");

		qtd = QTD_PTR(qh->Curr_qTD);
		while (!IS_NULL_PTR(qtd)) {
			dump_ehci_qtd(qtd);
			qtd = QTD_PTR(qtd->Next_qTD);
		}
		qh = QH_PTR(qh->HLink);
	}
	while (qh != _H_qh);
}

static void dump_ehci_asynclist_simple(void)
{
	QH_T     *qh = _H_qh;

	USB_debug(">>> EHCI Asynchronous List <<<\n");
	USB_debug("[QH] => ");
	do {
		USB_debug("0x%08x ", (int64_t)qh);
		qh = QH_PTR(qh->HLink);
	}
	while (qh != _H_qh);
	USB_debug("\n");
}

static void dump_ehci_period_frame_list_simple(void)
{
	QH_T     *qh = _Iqh[NUM_IQH-1];

	USB_debug(">>> EHCI period frame list simple <<<\n");
	USB_debug("[FList] => ");
	do {
		USB_debug("0x%08x ", (int64_t)qh);
		qh = QH_PTR(qh->HLink);
	} while (!IS_NULL_PTR(qh));
	USB_debug("\n");
}

static void dump_ehci_period_frame_list()
{
	int    i;
	QH_T   *qh;

	for (i = 0; i < FL_SIZE; i++) {
		USB_debug("!%02d: ", i);
		qh = QH_PTR(_PFList[i]);;
		while (!IS_NULL_PTR(qh)) {
			// USB_debug("0x%x (0x%x) => ", (int64_t)qh, qh->HLink);
			USB_debug("0x%x => ", (int64_t)qh);
			qh = QH_PTR(qh->HLink);
		}
		USB_debug("0\n");
	}
}

#endif  /* ENABLE_ERROR_MSG */

static void init_periodic_frame_list()
{
	QH_T   *qh_p;
	int    i, idx, interval;

	_PFList = nc_ptr(_PA_PFList);
	memset(_PFList, 0, sizeof(_PA_PFList));

	iso_ep_list = NULL;

	for (i = NUM_IQH-1; i >= 0; i--) {      /* interval = i^2                             */
		_Iqh[i] = alloc_ehci_QH();
		_Iqh[i]->HLink           = QH_HLNK_END;
		_Iqh[i]->Curr_qTD        = ptr_to_u32(_ghost_qtd);
		_Iqh[i]->OL_Next_qTD     = QTD_LIST_END;
		_Iqh[i]->OL_Alt_Next_qTD = ptr_to_u32(_ghost_qtd);
		_Iqh[i]->OL_Token        = QTD_STS_HALT;

		interval = 0x1 << i;

		for (idx = interval - 1; idx < FL_SIZE; idx += interval) {
			if (_PFList[idx] == 0) {         /* is empty list, insert directly             */
				_PFList[idx] = QH_HLNK_QH(_Iqh[i]);
			} else {
				qh_p = QH_PTR(_PFList[idx]);
				while (1) {
					if (qh_p == _Iqh[i])
						break;                   /* already chained by previous visit     */

					if (qh_p->HLink == QH_HLNK_END) {       /* reach end of list?          */
						qh_p->HLink = QH_HLNK_QH(_Iqh[i]);
						break;
					}
					qh_p = QH_PTR(qh_p->HLink);
				}
			}
		}
	}
}

static QH_T * get_int_tree_head_node(int interval)
{
	int    i;

	interval /= 8;                          /* each frame list entry for 8 micro-frame    */

	for (i = 0; i < NUM_IQH - 1; i++) {
		interval >>= 1;
		if (interval == 0)
			return _Iqh[i];
	}
	return _Iqh[NUM_IQH - 1];
}

static int  make_int_s_mask(int bInterval)
{
	int   order, interval;

	interval = 1;
	while (bInterval > 1) {
		interval *= 2;
		bInterval--;
	}

	if (interval < 2)
		return 0xFF;                        /* interval 1                                 */
	if (interval < 4)
		return 0x55;                        /* interval 2                                 */
	if (interval < 8)
		return 0x22;                        /* interval 4                                 */
	for (order = 0; (interval > 1); order++)
		interval >>= 1;
	return (0x1 << (order % 8));
}

static int  ehci_init(void)
{
	int      timeout = 250*1000;            /* EHCI reset time-out 250 ms                */

	/*------------------------------------------------------------------------------------*/
	/*  Reset EHCI host controller                                                        */
	/*------------------------------------------------------------------------------------*/
	_ehci->UCMDR = HSUSBH_UCMDR_HCRST_Msk;
	while ((_ehci->UCMDR & HSUSBH_UCMDR_HCRST_Msk) && (timeout > 0)) {
		delay_us(1000);
		timeout -= 1000;
	}
	if (_ehci->UCMDR & HSUSBH_UCMDR_HCRST_Msk)
		return USBH_ERR_EHCI_INIT;

	_ehci->UCMDR = UCMDR_INT_THR_CTRL | HSUSBH_UCMDR_RUN_Msk;

	_ghost_qtd = alloc_ehci_qTD(NULL);
	_ghost_qtd->Token = 0x11197B7F;    //QTD_STS_HALT;  visit_qtd() will not remove a qTD with this mark. It represents a qhost qTD.

	/*------------------------------------------------------------------------------------*/
	/*  Initialize asynchronous list                                                      */
	/*------------------------------------------------------------------------------------*/
	qh_remove_list = NULL;

	/* Create the QH list head with H-bit 1 */
	_H_qh = (QH_T *)alloc_ehci_QH();
	_H_qh->HLink   = QH_HLNK_QH(_H_qh);     /* circular link to itself, the only one QH   */
	_H_qh->Chrst   = QH_RCLM_LIST_HEAD;     /* it's the head of reclamation list          */
	_H_qh->Curr_qTD        = ptr_to_u32(_ghost_qtd);
	_H_qh->OL_Next_qTD     = QTD_LIST_END;
	_H_qh->OL_Alt_Next_qTD = ptr_to_u32(_ghost_qtd);
	_H_qh->OL_Token        = QTD_STS_HALT;
	_ehci->UCALAR          = ptr_to_u32(_H_qh);

	//dump_ehci_asynclist();

	/*------------------------------------------------------------------------------------*/
	/*  Initialize periodic list                                                          */
	/*------------------------------------------------------------------------------------*/
	if (FL_SIZE == 256)
		_ehci->UCMDR |= (0x2<<HSUSBH_UCMDR_FLSZ_Pos);
	else if (FL_SIZE == 512)
		_ehci->UCMDR |= (0x1<<HSUSBH_UCMDR_FLSZ_Pos);
	else if (FL_SIZE == 1024)
		_ehci->UCMDR |= (0x0<<HSUSBH_UCMDR_FLSZ_Pos);
	else
		return USBH_ERR_EHCI_INIT;               /* Invalid FL_SIZE setting!              */

	_ehci->UPFLBAR = ptr_to_u32(_PA_PFList);

	/*------------------------------------------------------------------------------------*/
	/*  start run                                                                         */
	/*------------------------------------------------------------------------------------*/

	_ehci->UCFGR = 0x1;                          /* enable port routing to EHCI           */
	_ehci->UIENR = HSUSBH_UIENR_USBIEN_Msk | HSUSBH_UIENR_UERRIEN_Msk | HSUSBH_UIENR_HSERREN_Msk | HSUSBH_UIENR_IAAEN_Msk;

	delay_us(1000);                              /* delay 1 ms                            */

	_ehci->UPSCR[0] = HSUSBH_UPSCR_PP_Msk;      /* enable port 1 port power               */
	//_ehci->UPSCR[0] = HSUSBH_UPSCR_PP_Msk | HSUSBH_UPSCR_PO_Msk;     /* set port owner to OHCI              */

	init_periodic_frame_list();

	delay_us(1000);                              /* delay 1 ms                            */

	// _ehci->UPSCR[0] |= HSUSBH_UPSCR_PO_Msk;
	// dump_ehci_regs();

	return 0;
}

static void ehci_suspend(void)
{
	if (_ehci->UPSCR[0] & 0x1)
		_ehci->UPSCR[0] |= HSUSBH_UPSCR_SUSPEND_Msk;
}

static void ehci_resume(void)
{
	if (_ehci->UPSCR[0] & 0x1)
		_ehci->UPSCR[0] = (_ehci->UPSCR[0] & ~HSUSBH_UPSCR_SUSPEND_Msk) | HSUSBH_UPSCR_FPR_Msk;
}

static void ehci_shutdown(void)
{
	ehci_suspend();
}

static void move_qh_to_remove_list(QH_T *qh)
{
	QH_T       *q;

	// USB_debug("move_qh_to_remove_list - 0x%x (0x%x)\n", (int)qh, qh->Chrst);

	/* check if this ED found in ed_remove_list */
	q = qh_remove_list;
	while (q) {
		if (q == qh)                        /* This QH found in qh_remove_list.           */
			return;                         /* Do nothing, return...                      */
		q = q->next;
	}

	DISABLE_EHCI_IRQ();

	/*------------------------------------------------------------------------------------*/
	/*  Search asynchronous frame list and remove qh if found in list.                    */
	/*------------------------------------------------------------------------------------*/
	q = _H_qh;                              /* find and remove it from asynchronous list  */
	while (QH_PTR(q->HLink) != _H_qh) {
		if (QH_PTR(q->HLink) == qh) {
			/* q's next QH is qh, found...           */
			q->HLink = qh->HLink;                /* remove qh from list                   */

			qh->next = qh_remove_list;           /* add qh to qh_remove_list              */
			qh_remove_list = qh;
			_ehci->UCMDR |= HSUSBH_UCMDR_IAAD_Msk;   /* trigger IAA interrupt             */
			ENABLE_EHCI_IRQ();
			return;                              /* done                                  */
		}
		q = QH_PTR(q->HLink);               /* advance to next QH in asynchronous list    */
	}

	/*------------------------------------------------------------------------------------*/
	/*  Search periodic frame list and remove qh if found in list.                        */
	/*------------------------------------------------------------------------------------*/
	q =  _Iqh[NUM_IQH-1];
	while (q->HLink != QH_HLNK_END) {
		if (QH_PTR(q->HLink) == qh) {
			/* q's next QH is qh, found...           */
			q->HLink = qh->HLink;                /* remove qh from list                   */

			qh->next = qh_remove_list;           /* add qh to qh_remove_list              */
			qh_remove_list = qh;
			_ehci->UCMDR |= HSUSBH_UCMDR_IAAD_Msk;   /* trigger IAA interrupt             */
			ENABLE_EHCI_IRQ();
			return;                              /* done                                  */
		}
		q = QH_PTR(q->HLink);               /* advance to next QH in asynchronous list    */
	}
	ENABLE_EHCI_IRQ();
}

static void append_to_qtd_list_of_QH(QH_T *qh, qTD_T *qtd)
{
	qTD_T  *q;

	if (IS_NULL_PTR(qh->qtd_list)) {
		qh->qtd_list = qtd;
	} else {
		q = qh->qtd_list;
		while (!IS_NULL_PTR(q->next))
			q = q->next;
		q->next = qtd;
	}
}

/*
 *  If ep==NULL, it's a control endpoint QH.
 */
static void  write_qh(UDEV_T *udev, EP_INFO_T *ep, QH_T *qh)
{
	uint32_t   chrst, cap;

	/*------------------------------------------------------------------------------------*/
	/*  Write QH DWord 1 - Endpoint Characteristics                                       */
	/*------------------------------------------------------------------------------------*/
	if (IS_NULL_PTR(ep))                        /* is control endpoint?                   */
	{
		if (udev->descriptor.bMaxPacketSize0 == 0) {
			if (udev->speed == SPEED_LOW)       /* give a default maximum packet size     */
				udev->descriptor.bMaxPacketSize0 = 8;
			else
				udev->descriptor.bMaxPacketSize0 = 64;
		}
		chrst = QH_DTC | QH_NAK_RL | (udev->descriptor.bMaxPacketSize0 << 16);
		if (udev->speed != SPEED_HIGH)
			chrst |= QH_CTRL_EP_FLAG;           /* non-high-speed control endpoint        */
	}
	else {                                      /* not a control endpoint                 */
		chrst = QH_NAK_RL | (ep->wMaxPacketSize << 16);
		chrst |= ((ep->bEndpointAddress & 0xf) << 8);      /* Endpoint Address             */
	}

	if (udev->speed == SPEED_LOW)
		chrst |= QH_EPS_LOW;
	else if (udev->speed == SPEED_FULL)
		chrst |= QH_EPS_FULL;
	else
		chrst |= QH_EPS_HIGH;

	chrst |= udev->dev_num;
	qh->Chrst = chrst;

	/*------------------------------------------------------------------------------------*/
	/*  Write QH DWord 2 - Endpoint Capabilities                                         */
	/*------------------------------------------------------------------------------------*/
	if (udev->speed == SPEED_HIGH) {
		cap = 0x40000000;
	} else {
		/*
		 *  Backtrace device tree until the USB 2.0 hub found
		 */
		HUB_DEV_T   *hub;
		int         port_num;

		port_num = udev->port_num;
		hub = udev->parent;

		while (!IS_NULL_PTR(hub) && (hub->iface->udev->speed != SPEED_HIGH)) {
			port_num = hub->iface->udev->port_num;
			hub = hub->iface->udev->parent;
		}
		cap = (port_num << QH_HUB_PORT_Pos) |
			  (hub->iface->udev->dev_num << QH_HUB_ADDR_Pos);
	}
	qh->Cap = cap;
}

static void  write_qtd_bptr(qTD_T *qtd, void *buff_addr, int xfer_len)
{
	int     i, u32Addr = ptr_to_u32(buff_addr);

	qtd->xfer_len = xfer_len;
	qtd->Bptr[0] = u32Addr;

	u32Addr = (u32Addr + 0x1000) & ~0xFFF;
	for (i = 1; i < 5; i++) {
		qtd->Bptr[i] = u32Addr;
		u32Addr += 0x1000;
	}
}

static int ehci_ctrl_xfer(UTR_T *utr)
{
	UDEV_T     *udev;
	QH_T       *qh;
	qTD_T      *qtd_setup, *qtd_data, *qtd_status;
	uint32_t   token;
	int        is_new_qh = 0;

	udev = utr->udev;

	if (utr->data_len > 0) {
		if ((ptr_to_u32(utr->buff) + utr->data_len) > ((ptr_to_u32(utr->buff) & ~0xFFF) + 0x5000))
			return USBH_ERR_BUFF_OVERRUN;
	}

	/*------------------------------------------------------------------------------------*/
	/*  Allocate and link QH                                                              */
	/*------------------------------------------------------------------------------------*/
	if (!IS_NULL_PTR(udev->ep0.hw_pipe)) {
		qh = udev->ep0.hw_pipe;
		if (!IS_NULL_PTR(qh->qtd_list))
			return USBH_ERR_EHCI_QH_BUSY;
	} else {
		qh = alloc_ehci_QH();
		if (IS_NULL_PTR(qh))
			return USBH_ERR_MEMORY_OUT;

		udev->ep0.hw_pipe = (void *)qh;     /* driver can find QH from EP                 */
		is_new_qh = 1;
	}
	write_qh(udev, NULL, qh);
	utr->ep = nc_ptr(&udev->ep0);           /* driver can find EP from UTR                */

	/*------------------------------------------------------------------------------------*/
	/*  Allocate qTDs                                                                     */
	/*------------------------------------------------------------------------------------*/
	qtd_setup = alloc_ehci_qTD(utr);        /* allocate qTD for SETUP                     */

	if (utr->data_len > 0)
		qtd_data = alloc_ehci_qTD(utr);     /* allocate qTD for DATA                      */
	else
		qtd_data = NULL;

	qtd_status = alloc_ehci_qTD(utr);       /* allocate qTD for USTSR                     */
	if (IS_NULL_PTR(qtd_status))            /* out of memory?                             */
	{
		if (qtd_setup)
			free_ehci_qTD(qtd_setup);       /* free memory                                */
		if (qtd_data)
			free_ehci_qTD(qtd_data);        /* free memory                                */
		return USBH_ERR_MEMORY_OUT;         /* out of memory                              */
	}

	// USB_debug("qh=0x%x, qtd_setup=0x%x, qtd_data=0x%x, qtd_status=0x%x\n", (int)qh, (int)qtd_setup, (int)qtd_data, (int)qtd_status);

	/*------------------------------------------------------------------------------------*/
	/* prepare SETUP stage qTD                                                            */
	/*------------------------------------------------------------------------------------*/
	qtd_setup->qh = qh;
	//qtd_setup->utr = utr;
	write_qtd_bptr(qtd_setup, &utr->setup, 8);
	append_to_qtd_list_of_QH(qh, qtd_setup);
	qtd_setup->Token = (8 << 16) | QTD_ERR_COUNTER | QTD_PID_SETUP | QTD_STS_ACTIVE;

	/*------------------------------------------------------------------------------------*/
	/* prepare DATA stage qTD                                                             */
	/*------------------------------------------------------------------------------------*/
	if (utr->data_len > 0) {
		qtd_setup->Next_qTD = ptr_to_u32(qtd_data);
		qtd_data->Next_qTD = ptr_to_u32(qtd_status);

		if ((utr->setup.bmRequestType & 0x80) == REQ_TYPE_OUT)
			token = QTD_ERR_COUNTER | QTD_PID_OUT | QTD_STS_ACTIVE;
		else
			token = QTD_ERR_COUNTER | QTD_PID_IN | QTD_STS_ACTIVE;

		qtd_data->qh = qh;
		//qtd_data->utr = utr;
		write_qtd_bptr(qtd_data, utr->buff, utr->data_len);
		append_to_qtd_list_of_QH(qh, qtd_data);
		qtd_data->Token = QTD_DT | (utr->data_len << 16) | token;
	} else {
		qtd_setup->Next_qTD = ptr_to_u32(qtd_status);
	}

	/*------------------------------------------------------------------------------------*/
	/* prepare USTSR stage qTD                                                            */
	/*------------------------------------------------------------------------------------*/
	qtd_status->Next_qTD = ptr_to_u32(_ghost_qtd);
	qtd_status->Alt_Next_qTD = QTD_LIST_END;

	if ((utr->setup.bmRequestType & 0x80) == REQ_TYPE_OUT)
		token = QTD_ERR_COUNTER | QTD_PID_IN | QTD_STS_ACTIVE;
	else
		token = QTD_ERR_COUNTER | QTD_PID_OUT | QTD_STS_ACTIVE;

	qtd_status->qh = qh;
	//qtd_status->utr = utr;
	append_to_qtd_list_of_QH(qh, qtd_status);
	qtd_status->Token = QTD_DT | QTD_IOC | token;

	/*------------------------------------------------------------------------------------*/
	/* Update QH overlay                                                                  */
	/*------------------------------------------------------------------------------------*/
	qh->Curr_qTD = 0;
	qh->OL_Next_qTD = ptr_to_u32(qtd_setup);
	qh->OL_Alt_Next_qTD = QTD_LIST_END;
	qh->OL_Token = 0;

	/*------------------------------------------------------------------------------------*/
	/* Link QH and start asynchronous transfer                                            */
	/*------------------------------------------------------------------------------------*/
	if (is_new_qh) {
		qh->HLink = _H_qh->HLink;
		_H_qh->HLink = QH_HLNK_QH(qh);
	}

	/*  Start transfer */
	_ehci->UCMDR |= HSUSBH_UCMDR_ASEN_Msk;      /* start asynchronous transfer            */
	return 0;
}

static int ehci_bulk_xfer(UTR_T *utr)
{
	UDEV_T     *udev;
	EP_INFO_T  *ep = nc_ptr(utr->ep);
	QH_T       *qh;
	qTD_T      *qtd, *qtd_pre;
	uint32_t   data_len, xfer_len;
	uint8_t    *buff;
	uint32_t   token;
	int        is_new_qh = 0;

	//USB_debug("Bulk XFER =>\n");
	// dump_ehci_asynclist_simple();

	udev = utr->udev;

	if (!IS_NULL_PTR(ep->hw_pipe)) {
		qh = ep->hw_pipe;
		if (!IS_NULL_PTR(qh->qtd_list))
			return USBH_ERR_EHCI_QH_BUSY;
	}  else {
		qh = alloc_ehci_QH();
		if (IS_NULL_PTR(qh))
			return USBH_ERR_MEMORY_OUT;
		is_new_qh = 1;
		write_qh(udev, ep, qh);
		ep->hw_pipe = (void *)qh;           /* associate QH with endpoint                 */
	}

	/*------------------------------------------------------------------------------------*/
	/* Prepare qTDs                                                                       */
	/*------------------------------------------------------------------------------------*/
	data_len = utr->data_len;
	buff = utr->buff;
	qtd_pre = NULL;

	while (data_len > 0) {
		qtd = alloc_ehci_qTD(utr);
		if (IS_NULL_PTR(qtd)) {                  /* failed to allocate a qTD              */
			qtd = qh->qtd_list;
			while (!IS_NULL_PTR(qtd)) {
				qtd_pre = qtd;
				qtd = qtd->next;
				free_ehci_qTD(qtd_pre);
			}
			if (is_new_qh) {
				free_ehci_QH(qh);
				ep->hw_pipe = NULL;
			}
			return USBH_ERR_MEMORY_OUT;
		}

		if ((ep->bEndpointAddress & EP_ADDR_DIR_MASK) == EP_ADDR_DIR_OUT)
			token = QTD_ERR_COUNTER | QTD_PID_OUT | QTD_STS_ACTIVE;
		else
			token = QTD_ERR_COUNTER | QTD_PID_IN | QTD_STS_ACTIVE;

		if (data_len > 0x4000)              /* force maximum x'fer length 16K per qTD     */
			xfer_len = 0x4000;
		else
			xfer_len = data_len;            /* remaining data length < 4K                 */

		qtd->qh = qh;
		qtd->Next_qTD = ptr_to_u32(_ghost_qtd);
		qtd->Alt_Next_qTD = QTD_LIST_END; //(uint32_t)_ghost_qtd;
		write_qtd_bptr(qtd, buff, xfer_len);
		append_to_qtd_list_of_QH(qh, qtd);
		qtd->Token = (xfer_len << 16) | token;

		buff += xfer_len;                   /* advanced buffer pointer                    */
		data_len -= xfer_len;

		if (data_len == 0) {                /* is this the latest qTD?                   */
			qtd->Token |= QTD_IOC;          /* ask to raise an interrupt on the last qTD  */
			qtd->Next_qTD = ptr_to_u32(_ghost_qtd);     /* qTD list end                     */
		}

		if (!IS_NULL_PTR(qtd_pre))
			qtd_pre->Next_qTD = ptr_to_u32(qtd);
		qtd_pre = qtd;
	}

	//USB_debug("utr=0x%x, qh=0x%x, qtd=0x%x\n", (int)utr, (int)qh, (int)qh->qtd_list);

	qtd = qh->qtd_list;

//    qh->Curr_qTD = 0; //(uint32_t)qtd;
	qh->OL_Next_qTD = ptr_to_u32(qtd);
//  qh->OL_Alt_Next_qTD = QTD_LIST_END;

	/*------------------------------------------------------------------------------------*/
	/* Link QH and start asynchronous transfer                                            */
	/*------------------------------------------------------------------------------------*/
	if (is_new_qh) {
		memcpy(&(qh->OL_Bptr[0]), &(qtd->Bptr[0]), 20);
		qh->Curr_qTD = ptr_to_u32(qtd);
		qh->OL_Token = 0; //qtd->Token;

		if (utr->ep->bToggle)
			qh->OL_Token |= QTD_DT;

		qh->HLink = _H_qh->HLink;
		_H_qh->HLink = QH_HLNK_QH(qh);
	}

	/*  Start transfer */
	_ehci->UCMDR |= HSUSBH_UCMDR_ASEN_Msk;      /* start asynchronous transfer            */
	return 0;
}

static int ehci_int_xfer(UTR_T *utr)
{
	UDEV_T     *udev = utr->udev;
	EP_INFO_T  *ep = utr->ep;
	QH_T       *qh, *iqh;
	qTD_T      *qtd;
	uint32_t   token;
	int8_t     is_new_qh = 0;

	if (!IS_NULL_PTR(ep->hw_pipe)) {
		qh = ep->hw_pipe;
		if (qh->qtd_list)
			return USBH_ERR_EHCI_QH_BUSY;
	} else {
		qh = alloc_ehci_QH();
		if (IS_NULL_PTR(qh))
			return USBH_ERR_MEMORY_OUT;
		is_new_qh = 1;
		write_qh(udev, ep, qh);
		qh->Chrst &= ~0xF0000000;

		if (udev->speed == SPEED_HIGH) {
			qh->Cap = (0x1 << QH_MULT_Pos) | (qh->Cap & 0xff) | make_int_s_mask(ep->bInterval);
		} else {
			qh->Cap = (0x1 << QH_MULT_Pos) | (qh->Cap & ~(QH_C_MASK_Msk | QH_S_MASK_Msk)) | 0x7802;
		}
		ep->hw_pipe = (void *)qh;           /* associate QH with endpoint                 */
	}

	/*------------------------------------------------------------------------------------*/
	/*  Prepare qTD                                                                       */
	/*------------------------------------------------------------------------------------*/
	qtd = alloc_ehci_qTD(utr);
	if (IS_NULL_PTR(qtd)) {                 /* failed to allocate a qTD                   */
		if (is_new_qh) {
			free_ehci_QH(qh);
			ep->hw_pipe = NULL;
		}
		return USBH_ERR_MEMORY_OUT;
	}

	if ((ep->bEndpointAddress & EP_ADDR_DIR_MASK) == EP_ADDR_DIR_OUT)
		token = QTD_ERR_COUNTER | QTD_PID_OUT | QTD_STS_ACTIVE;
	else
		token = QTD_ERR_COUNTER | QTD_PID_IN | QTD_STS_ACTIVE;

	qtd->qh = qh;
	qtd->Next_qTD = QTD_LIST_END; //(uint32_t)_ghost_qtd;
	qtd->Alt_Next_qTD = QTD_LIST_END; //(uint32_t)_ghost_qtd;
	write_qtd_bptr(qtd, utr->buff, utr->data_len);
	append_to_qtd_list_of_QH(qh, qtd);
	qtd->Token = QTD_IOC | (utr->data_len << 16) | token;

	DISABLE_EHCI_IRQ();

	//qh->Curr_qTD = 0; //(uint32_t)qtd;
	qh->OL_Next_qTD = ptr_to_u32(qtd);
	//qh->OL_Alt_Next_qTD = QTD_LIST_END;

	// sysprintf("ehci_int_xfer - qh: 0x%x, 0x%x, 0x%x\n", (int)qh, (int)qh->Chrst, (int)qh->Cap);

	if (is_new_qh) {
		memcpy(&(qh->OL_Bptr[0]), &(qtd->Bptr[0]), 20);
		qh->Curr_qTD = ptr_to_u32(qtd);
		qh->OL_Token = qtd->Token;

		if (udev->speed == SPEED_HIGH)      /* get head node of this interval             */
			iqh = get_int_tree_head_node(ep->bInterval);
		else
			iqh = get_int_tree_head_node(ep->bInterval * 8);
		qh->HLink = iqh->HLink;             /* Add to list of the same interval           */
		iqh->HLink = QH_HLNK_QH(qh);
	}

	ENABLE_EHCI_IRQ();

	_ehci->UCMDR |= HSUSBH_UCMDR_PSEN_Msk;      /* periodic list enable                   */
	return 0;
}

/*
 *  Quit current trasnfer via UTR or hardware EP.
 */
static int ehci_quit_xfer(UTR_T *utr, EP_INFO_T *ep)
{
	QH_T       *qh;

	// USB_debug("ehci_quit_xfer - utr: 0x%x, ep: 0x%x\n", (int)utr, (int)ep);

	DISABLE_EHCI_IRQ();
	if (ehci_quit_iso_xfer(utr, ep) == 0) {
		ENABLE_EHCI_IRQ();
		return 0;
	}
	ENABLE_EHCI_IRQ();

	if (!IS_NULL_PTR(utr)) {
		if (IS_NULL_PTR(utr->ep))
			return USBH_ERR_NOT_FOUND;

		qh = nc_ptr(utr->ep->hw_pipe);
		if (IS_NULL_PTR(qh))
			return USBH_ERR_NOT_FOUND;

		/* add the QH to remove list, it will be removed on the next IAAD interrupt       */
		move_qh_to_remove_list(qh);
		utr->ep->hw_pipe = NULL;
	}

	if (!IS_NULL_PTR(ep) && !IS_NULL_PTR(ep->hw_pipe))
	{
		qh = nc_ptr(ep->hw_pipe);
		/* add the QH to remove list, it will be removed on the next IAAD interrupt       */
		move_qh_to_remove_list(qh);
		ep->hw_pipe = NULL;
	}
	delay_us(2000);
	return 0;
}

static int visit_qtd(qTD_T *qtd)
{
	if ((qtd->Token == 0x11197B7F) || (qtd->Token == 0x1197B7F))
		return 0;                    /* A Dummy qTD or qTD on writing, don't touch it.    */

	// USB_debug("Visit qtd 0x%x - 0x%x\n", (int)qtd, qtd->Token);

	if ((qtd->Token & QTD_STS_ACTIVE) == 0) {
		if (qtd->Token & (QTD_STS_HALT | QTD_STS_DATA_BUFF_ERR | QTD_STS_BABBLE |
							QTD_STS_XactErr | QTD_STS_MISS_MF)) {
			USB_error("qTD error token=0x%x!  0x%x\n", qtd->Token, qtd->Bptr[0]);
			if (qtd->utr->status == 0)
				qtd->utr->status = USBH_ERR_TRANSACTION;
		} else {
			if ((qtd->Token & QTD_PID_Msk) != QTD_PID_SETUP) {
				qtd->utr->xfer_len += qtd->xfer_len - QTD_TODO_LEN(qtd->Token);
				// USB_debug("0x%x  utr->xfer_len += %d\n", qtd->Token, qtd->xfer_len - QTD_TODO_LEN(qtd->Token));
			}
		}
		return 1;
	}
	return 0;
}

static void scan_asynchronous_list()
{
	QH_T    *qh, *qh_tmp;
	qTD_T   *q_pre, *qtd, *qtd_tmp;
	UTR_T   *utr;

	qh =  QH_PTR(_H_qh->HLink);
	while (qh != _H_qh)
	{
		// USB_debug("Scan qh=0x%x, 0x%x\n", (int)qh, qh->OL_Token);
		utr = NULL;
		qtd = qh->qtd_list;
		while (!IS_NULL_PTR(qtd))
		{
			if (visit_qtd(qtd))                  /* if TRUE, reclaim this qtd             */
			{
				/* qTD is completed, will remove it      */
				utr = qtd->utr;
				if (qtd == qh->qtd_list)
					qh->qtd_list = qtd->next;    /* unlink the qTD from qtd_list          */
				else
					q_pre->next = qtd->next;     /* unlink the qTD from qtd_list          */

				qtd_tmp = qtd;                   /* remember this qTD for freeing later   */
				qtd = qtd->next;                 /* advance to the next qTD               */

				qtd_tmp->next = qh->done_list;   /* push this qTD to QH's done list       */
				qh->done_list = qtd_tmp;
			} else {
				q_pre = qtd;                     /* remember this qTD as a preceder       */
				qtd = qtd->next;                 /* advance to next qTD                   */
			}
		}

		qh_tmp = qh;
		qh = QH_PTR(qh->HLink);                  /* advance to the next QH                */

		/* If all TDs are done, call-back to requester and then remove this QH.           */
		if (IS_NULL_PTR(qh_tmp->qtd_list) && !IS_NULL_PTR(utr))
		{
			// sysprintf("T %d [%d]\n", (qh_tmp->Chrst>>8)&0xf, (qh_tmp->OL_Token&QTD_DT) ? 1 : 0);
			if (qh_tmp->OL_Token & QTD_DT)
				utr->ep->bToggle = 1;
			else
				utr->ep->bToggle = 0;

			utr->bIsTransferDone = 1;
			if (!IS_NULL_PTR(utr->func))
				utr->func(utr);

			_ehci->UCMDR |= HSUSBH_UCMDR_IAAD_Msk;   /* trigger IAA to reclaim done_list  */
		}
	}
}

static void scan_periodic_frame_list()
{
	QH_T    *qh;
	qTD_T   *qtd;
	UTR_T   *utr;

	/*------------------------------------------------------------------------------------*/
	/* Scan interrupt frame list                                                          */
	/*------------------------------------------------------------------------------------*/
	qh =  _Iqh[NUM_IQH-1];
	while (!IS_NULL_PTR(qh))
	{
		qtd = qh->qtd_list;                 /* There's only one qTD in list at most.      */
		if (IS_NULL_PTR(qtd))
		{
			/* empty QH                                   */
			qh = QH_PTR(qh->HLink);         /* advance to the next QH                     */
			continue;
		}

		if (visit_qtd(qtd)) {               /* if TRUE, reclaim this qtd                  */
			qtd->next = qh->done_list;      /* push qTD into the done list                */
			qh->done_list = qtd;            /* move qTD to done list                      */
			qh->qtd_list = NULL;            /* qtd_list becomes empty                     */
		}

		qtd = qh->done_list;

		/* If all TDs are done, call-back to requester and then remove this QH.           */
		if (!IS_NULL_PTR(qtd) && IS_NULL_PTR(qh->qtd_list))
		{
			utr = qtd->utr;

			if (qh->OL_Token & QTD_DT)
				utr->ep->bToggle = 1;
			else
				utr->ep->bToggle = 0;

			utr->bIsTransferDone = 1;
			if (!IS_NULL_PTR(utr->func))
				utr->func(utr);

			_ehci->UCMDR |= HSUSBH_UCMDR_IAAD_Msk;   /* trigger IAA to reclaim done_list  */
		}

		qh = QH_PTR(qh->HLink);                  /* advance to the next QH                */
	}

	/*------------------------------------------------------------------------------------*/
	/* Scan isochronous frame list                                                          */
	/*------------------------------------------------------------------------------------*/
	scan_isochronous_list();
}

static void iaad_remove_qh()
{
	QH_T    *qh;
	qTD_T   *qtd;
	UTR_T   *utr;

	/*------------------------------------------------------------------------------------*/
	/* Remove all QHs in qh_remove_list...                                                */
	/*------------------------------------------------------------------------------------*/
	while (!IS_NULL_PTR(qh_remove_list))
	{
		qh = qh_remove_list;
		qh_remove_list = qh->next;

		// USB_debug("iaad_remove_qh - remove QH 0x%x\n", (int)qh);

		while (!IS_NULL_PTR(qh->done_list)) /* we can free the qTDs now                   */
		{
			qtd = qh->done_list;
			qh->done_list = qtd->next;
			free_ehci_qTD(qtd);
		}

		if (!IS_NULL_PTR(qh->qtd_list))      /* still have incomplete qTDs?               */
		{
			utr = qh->qtd_list->utr;
			while (!IS_NULL_PTR(qh->qtd_list))
			{
				qtd = qh->qtd_list;
				qh->qtd_list = qtd->next;
				free_ehci_qTD(qtd);
			}
			utr->status = USBH_ERR_ABORT;
			utr->bIsTransferDone = 1;
			if (!IS_NULL_PTR(utr->func))
				utr->func(utr);             /* call back                                  */
		}
		free_ehci_QH(qh);                   /* free the QH                                */
	}

	/*------------------------------------------------------------------------------------*/
	/* Free all qTD in done_list of each asynchronous QH                                  */
	/*------------------------------------------------------------------------------------*/
	qh =  QH_PTR(_H_qh->HLink);
	while (qh != _H_qh) {
		while (qh->done_list) {             /* we can free the qTDs now                   */
			qtd = qh->done_list;
			qh->done_list = qtd->next;
			free_ehci_qTD(qtd);
		}
		qh = QH_PTR(qh->HLink);                  /* advance to the next QH                */
	}

	/*------------------------------------------------------------------------------------*/
	/* Free all qTD in done_list of each QH of periodic frame list                        */
	/*------------------------------------------------------------------------------------*/
	qh =  _Iqh[NUM_IQH-1];
	while (!IS_NULL_PTR(qh)) {
		while (!IS_NULL_PTR(qh->done_list)) {    /* we can free the qTDs now                   */
			qtd = qh->done_list;
			qh->done_list = nc_ptr(qtd->next);
			free_ehci_qTD(qtd);
		}
		qh = QH_PTR(qh->HLink);                  /* advance to the next QH                */
	}
}

//static irqreturn_t ehci_irq (struct usb_hcd *hcd)
void EHCI_IRQHandler(void)
{
	uint32_t  intsts;

	dmb();
	intsts = _ehci->USTSR;

	//if (ptr_to_u32(ptr_to_u32(&_ehci->EHCVNR)) == 0x40140000)
	//  sysprintf("EHCI0 IRQ! 0x%x\n", intsts);
	//else
	//  sysprintf("EHCI1 IRQ! 0x%x\n", intsts);

	_ehci->USTSR = intsts;                  /* clear interrupt status                     */

	// USB_debug("Eirq USTSR=0x%x\n", intsts);

	if (intsts & HSUSBH_USTSR_UERRINT_Msk)
	{
		// USB_error("Transfer error!\n");
	}

	if (intsts & HSUSBH_USTSR_USBINT_Msk)
	{
		// sysprintf("USBINT!\n");
		/* some transfers completed, travel asynchronous */
		/* and periodic lists to find and reclaim them.  */
		scan_asynchronous_list();

		scan_periodic_frame_list();
	}

	if (intsts & HSUSBH_USTSR_IAA_Msk)
	{
		iaad_remove_qh();
	}
	dmb();
}

static UDEV_T * ehci_find_device_by_port(int port)
{
	UDEV_T  *udev;

	udev = g_udev_list;
	while (!IS_NULL_PTR(udev))
	{
		if (IS_NULL_PTR(udev->parent) && (udev->port_num == port) && (udev->speed == SPEED_HIGH))
			return udev;
		udev = udev->next;
	}
	return NULL;
}

static int ehci_rh_port_reset(int port)
{
	int       retry;
	int       reset_time;
	uint32_t  t0;

	reset_time = PORT_RESET_TIME_MS;

	for (retry = 0; retry < PORT_RESET_RETRY; retry++)
	{
		_ehci->UPSCR[port] = (_ehci->UPSCR[port] | HSUSBH_UPSCR_PRST_Msk) & ~HSUSBH_UPSCR_PE_Msk;
		t0 = get_ticks();
		while (get_ticks() - t0 <  reset_time + 1) ;     /* wait at least 50 ms        */

		_ehci->UPSCR[port] &= ~HSUSBH_UPSCR_PRST_Msk;

		t0 = get_ticks();
		while (get_ticks() - t0 < reset_time + 1)
		{
			if (!(_ehci->UPSCR[port] & HSUSBH_UPSCR_CCS_Msk) ||
					((_ehci->UPSCR[port] & (HSUSBH_UPSCR_CCS_Msk | HSUSBH_UPSCR_PE_Msk)) == (HSUSBH_UPSCR_CCS_Msk | HSUSBH_UPSCR_PE_Msk)))
				goto port_reset_done;
		}
		reset_time += PORT_RESET_RETRY_INC_MS;
	}

	USB_debug("EHCI port %d (0x%x) - port reset failed!\n", port + 1, _ehci->UPSCR[port]);
	return USBH_ERR_PORT_RESET;

port_reset_done:
	if ((_ehci->UPSCR[port] & HSUSBH_UPSCR_CCS_Msk) == 0)    /* check again if device disconnected */
	{
		_ehci->UPSCR[port] |= HSUSBH_UPSCR_CSC_Msk;          /* clear CSC                          */
		return USBH_ERR_DISCONNECTED;
	}
	_ehci->UPSCR[port] |= HSUSBH_UPSCR_PEC_Msk;              /* clear port enable change status    */
	return USBH_OK;                                          /* port reset success                 */
}

static int ehci_rh_polling(void)
{
	UDEV_T    *udev;
	int       i, ret;
	int       connect_status, t0;

	for (i = 0; i < 1; i++)
	{
		if (!(_ehci->UPSCR[i] & HSUSBH_UPSCR_CSC_Msk))
			return 0;

		/*------------------------------------------------------------------------------------*/
		/*  connect status change                                                             */
		/*------------------------------------------------------------------------------------*/

		// USB_debug("EHCI port1 status change: 0x%x\n", _ehci->UPSCR[i]);

		/*--------------------------------------------------------------------------------*/
		/*  Disconnect the devices attached to this port.                                 */
		/*--------------------------------------------------------------------------------*/
		while (1)
		{
			udev = ehci_find_device_by_port(i+1);
			if (IS_NULL_PTR(udev))
				break;
			disconnect_device(udev);
		}

		/*--------------------------------------------------------------------------------*/
		/*  Port de-bounce                                                                */
		/*--------------------------------------------------------------------------------*/
		t0 = get_ticks();
		connect_status = _ehci->UPSCR[i] & HSUSBH_UPSCR_CCS_Msk;
		while (get_ticks() - t0 < HUB_DEBOUNCE_TIME)
		{
			if (connect_status != (_ehci->UPSCR[i] & HSUSBH_UPSCR_CCS_Msk))
			{
				/* reset stable time counting                                             */
				t0 = get_ticks();
				connect_status = _ehci->UPSCR[i] & HSUSBH_UPSCR_CCS_Msk;
			}
		}

		_ehci->UPSCR[i] |= HSUSBH_UPSCR_CSC_Msk;     /* clear connect status change bit   */

		if (connect_status == HSUSBH_UPSCR_CCS_Msk)
		{
			/*--------------------------------------------------------------------------------*/
			/*  A new device connected.                                                       */
			/*--------------------------------------------------------------------------------*/
			if (ehci_rh_port_reset(i) != USBH_OK)
			{
				/* port reset failed, maybe an USB 1.1 device */
				_ehci->UPSCR[i] |= HSUSBH_UPSCR_PO_Msk;     /* change port owner to OHCI      */
				_ehci->UPSCR[i] |= HSUSBH_UPSCR_CSC_Msk;    /* clear all status change bits   */
				return 0;
			}
			/*
			 *  Port reset success. Start to enumerate this new device.
			 */
			udev = alloc_device();
			if (IS_NULL_PTR(udev))
				return 0;                       /* out-of-memory, do nothing...               */

			udev->parent = NULL;
			udev->port_num = i+1;
			udev->speed = SPEED_HIGH;
			udev->hc_driver = &ehci_driver;

			ret = connect_device(udev);
			if (ret < 0)
			{
				USB_error("connect_device error! [%d]\n", ret);
				free_device(udev);
			}
		}
		else
		{
			/*
			 *  Device disconnected
			 */
			while (1)
			{
				udev = ehci_find_device_by_port(i+1);
				if (IS_NULL_PTR(udev))
					break;
				disconnect_device(udev);
			}
		}
	}
	return 1;
}

/// @endcond HIDDEN_SYMBOLS

