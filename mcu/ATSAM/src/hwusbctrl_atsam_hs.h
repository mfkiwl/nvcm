/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     hwusbctrl_atsam_hs.h
 *  brief:    ATSAM USB HS Controller
 *  version:  1.00
 *  date:     2020-04-13
 *  authors:  nvitya
*/

#ifndef HWUSBCTRL_ATSAM_HS_H_
#define HWUSBCTRL_ATSAM_HS_H_

#include "platform.h"

#if defined(USBHS)

#define HWUSBCTRL_PRE_ONLY
#include "hwusbctrl.h"

#define HWUSB_MAX_ENDPOINTS      10

class THwUsbEndpoint_atsam_hs : public THwUsbEndpoint_pre
{
public:

	__IO uint8_t *       fifo_reg;
	__IO uint32_t *      cfg_reg;
	__IO uint32_t *      isr_reg;
	__IO uint32_t *      icr_reg;
	__IO uint32_t *      imr_reg;
	__IO uint32_t *      ier_reg;
	__IO uint32_t *      idr_reg;

	virtual ~THwUsbEndpoint_atsam_hs() { }

	bool ConfigureHwEp();
  int  ReadRecvData(void * buf, uint32_t buflen);
	int  StartSendData(void * buf, unsigned len);

	void SendAck();
  void Stall();
  void Nak();

  inline bool IsSetupRequest()  { return (*isr_reg & USBHS_DEVEPTISR_RXSTPI); }

  void EnableRecv();
  void DisableRecv();
  void StopSend();
  void FinishSend();
};

class THwUsbCtrl_atsam_hs : public THwUsbCtrl_pre
{
public:
	Usbhs *             regs = nullptr;
	uint32_t            irq_mask = 0;

	bool InitHw();

	void HandleIrq();

	void DisableIrq();
	void EnableIrq();
	void SetDeviceAddress(uint8_t aaddr);
	virtual void SetPullUp(bool aenable);

	void ResetEndpoints();
};

#define HWUSBENDPOINT_IMPL   THwUsbEndpoint_atsam_hs
#define HWUSBCTRL_IMPL       THwUsbCtrl_atsam_hs

#endif // if defined(USBHS)

#endif // def HWUSBCTRL_ATSAM_HS_H_
