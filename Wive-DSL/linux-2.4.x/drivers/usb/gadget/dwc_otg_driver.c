/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_driver.c $
 * $Revision: 1.3 $
 * $Date: 2009/02/02 05:27:56 $
 * $Change: 791271 $
 *
 * Synopsys HS OTG Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 * 
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 * 
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================== */

/** @file
 * The dwc_otg_driver module provides the initialization and cleanup entry
 * points for the DWC_otg driver. This module will be dynamically installed
 * after Linux is booted using the insmod command. When the module is
 * installed, the dwc_otg_driver_init function is called. When the module is
 * removed (using rmmod), the dwc_otg_driver_cleanup function is called.
 * 
 * This module also defines a data structure for the dwc_otg_driver, which is
 * used in conjunction with the standard ARM lm_device structure. These
 * structures allow the OTG driver to comply with the standard Linux driver
 * model in which devices and drivers are registered with a bus driver. This
 * has the benefit that Linux can expose attributes of the driver and device
 * in its special sysfs file system. Users can then read or write files in
 * this file system to perform diagnostics on the driver components or the
 * device.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include "moduleparam.h"
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/stat.h>	 	/* permission constants */
#include <asm/io.h>
#include <linux/sched.h>	//cathy for request_irq
#include "dwc_otg_plat.h"
#include "dwc_otg_driver.h"
#include "dwc_otg_pcd.h"
#include "dwc_otg_hcd.h"
#include <asm/mach-realtek/rtl8672/platform.h>

#define DWC_DRIVER_VERSION	"2.60a 22-NOV-2006"
#define DWC_DRIVER_DESC		"HS OTG USB Controller driver"
#define USB2_PHY_DELAY __delay(200)

static struct lm_device *glmdev=NULL;
static const char dwc_driver_name[] = "dwc_otg";


/*-------------------------------------------------------------------------*/
/* Encapsulate the module parameter settings */

static dwc_otg_core_params_t dwc_otg_module_params = {
	.opt = dwc_param_opt_default,
	.otg_cap = DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE,	//no HNP, SRP
	.dma_enable = dwc_param_dma_enable_default,	//dma enable(RTL8672 uses internal DMA mode)
	.dma_burst_size = -1,	//(used for external DMA mode)
	.speed = DWC_SPEED_PARAM_HIGH,	//high speed
	.host_support_fs_ls_low_power = dwc_param_host_support_fs_ls_low_power_default,	//not support low power
	.host_ls_low_power_phy_clk = dwc_param_host_ls_low_power_phy_clk_default,	//??
	.enable_dynamic_fifo = dwc_param_enable_dynamic_fifo_default,	//dynamic fifo sizing
	.data_fifo_size = 1024,
	.dev_rx_fifo_size = 512,
	.dev_nperio_tx_fifo_size = 256,
	.dev_perio_tx_fifo_size = 
	{	/* dev_perio_tx_fifo_size_1 */
			256,
	/*		-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1*/
	},	/* 15 */
	.host_rx_fifo_size = 512,
	.host_nperio_tx_fifo_size = 256,
	.host_perio_tx_fifo_size = 256,
	.max_transfer_size = -1,	//??
	.max_packet_count = -1,	//??
	.host_channels = 5, //4//4  is work
	.dev_endpoints = 3,	// 1:bulk in 2:bulk out 3: interrupt
	.phy_type = DWC_PHY_TYPE_PARAM_UTMI,	//UTMI PHY
	.phy_utmi_width = 16,
	.phy_ulpi_ddr = -1,
	.phy_ulpi_ext_vbus = -1,
	.i2c_enable = -1,
	.ulpi_fs_ls = -1,
	.ts_dline = -1,
	.en_multiple_tx_fifo = 0,	//shared FIFO
	.dev_tx_fifo_size = 
	{	/* dev_tx_fifo_size */
			512,
	/*		-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1*/
	},	/* 15 */
	.thr_ctl = -1,
	.tx_thr_length = -1,
	.rx_thr_length = -1,
};

/**
 * Global Debug Level Mask.
 */
uint32_t g_dbg_lvl = 0; /* OFF */

/**
 * This function is called during module intialization to verify that
 * the module parameters are in a valid state.
 */
static int check_parameters(dwc_otg_core_if_t *core_if)
{
	int i;
	int retval = 0;

/* Checks if the parameter is outside of its valid range of values */
#define DWC_OTG_PARAM_TEST(_param_,_low_,_high_) \
		((dwc_otg_module_params._param_ < (_low_)) || \
		(dwc_otg_module_params._param_ > (_high_)))

/* If the parameter has been set by the user, check that the parameter value is
 * within the value range of values.  If not, report a module error. */
#define DWC_OTG_PARAM_ERR(_param_,_low_,_high_,_string_) \
		do { \
			if (dwc_otg_module_params._param_ != -1) { \
				if (DWC_OTG_PARAM_TEST(_param_,(_low_),(_high_))) { \
					DWC_ERROR("`%d' invalid for parameter `%s'\n", \
						dwc_otg_module_params._param_, _string_); \
					dwc_otg_module_params._param_ = dwc_param_##_param_##_default; \
					retval ++; \
				} \
			} \
		} while (0)
//cathy, disable host parameter check
	DWC_OTG_PARAM_ERR(opt,0,1,"opt");
	DWC_OTG_PARAM_ERR(otg_cap,0,2,"otg_cap");
	DWC_OTG_PARAM_ERR(dma_enable,0,1,"dma_enable");
	DWC_OTG_PARAM_ERR(speed,0,1,"speed");
	DWC_OTG_PARAM_ERR(enable_dynamic_fifo,0,1,"enable_dynamic_fifo");
	DWC_OTG_PARAM_ERR(data_fifo_size,32,32768,"data_fifo_size");
	DWC_OTG_PARAM_ERR(dev_rx_fifo_size,16,32768,"dev_rx_fifo_size");
	DWC_OTG_PARAM_ERR(dev_nperio_tx_fifo_size,16,32768,"dev_nperio_tx_fifo_size");
	DWC_OTG_PARAM_ERR(max_transfer_size,2047,524288,"max_transfer_size");
	DWC_OTG_PARAM_ERR(max_packet_count,15,511,"max_packet_count");
	DWC_OTG_PARAM_ERR(dev_endpoints,1,15,"dev_endpoints");
	DWC_OTG_PARAM_ERR(phy_type,0,2,"phy_type");
	DWC_OTG_PARAM_ERR(phy_ulpi_ddr,0,1,"phy_ulpi_ddr");
	DWC_OTG_PARAM_ERR(phy_ulpi_ext_vbus,0,1,"phy_ulpi_ext_vbus");
	DWC_OTG_PARAM_ERR(i2c_enable,0,1,"i2c_enable");
	DWC_OTG_PARAM_ERR(ulpi_fs_ls,0,1,"ulpi_fs_ls");
	DWC_OTG_PARAM_ERR(ts_dline,0,1,"ts_dline");

	if (dwc_otg_module_params.dma_burst_size != -1) 
	{
		if (DWC_OTG_PARAM_TEST(dma_burst_size,1,1) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,4,4) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,8,8) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,16,16) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,32,32) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,64,64) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,128,128) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,256,256))
		{
			DWC_ERROR("`%d' invalid for parameter `dma_burst_size'\n", 
				  dwc_otg_module_params.dma_burst_size);
			dwc_otg_module_params.dma_burst_size = 32;
			retval ++;
		}
	}

	if (dwc_otg_module_params.phy_utmi_width != -1) 
	{
		if (DWC_OTG_PARAM_TEST(phy_utmi_width,8,8) &&
			DWC_OTG_PARAM_TEST(phy_utmi_width,16,16)) 
		{
			DWC_ERROR("`%d' invalid for parameter `phy_utmi_width'\n", 
				  dwc_otg_module_params.phy_utmi_width);
			dwc_otg_module_params.phy_utmi_width = 16;
			retval ++;
		}
	}
	DWC_OTG_PARAM_ERR(en_multiple_tx_fifo,0,1,"en_multiple_tx_fifo");
	DWC_OTG_PARAM_ERR(thr_ctl, 0, 7, "thr_ctl");
	DWC_OTG_PARAM_ERR(tx_thr_length, 8, 128, "tx_thr_length");
	DWC_OTG_PARAM_ERR(rx_thr_length, 8, 128, "rx_thr_length");
	
	
	/* At this point, all module parameters that have been set by the user
	 * are valid, and those that have not are left unset.  Now set their
	 * default values and/or check the parameters against the hardware
	 * configurations of the OTG core. */



/* This sets the parameter to the default value if it has not been set by the
 * user */
#define DWC_OTG_PARAM_SET_DEFAULT(_param_) \
	({ \
		int changed = 1; \
		if (dwc_otg_module_params._param_ == -1) { \
			changed = 0; \
			dwc_otg_module_params._param_ = dwc_param_##_param_##_default; \
		} \
		changed; \
	})

/* This checks the macro agains the hardware configuration to see if it is
 * valid.  It is possible that the default value could be invalid.	In this
 * case, it will report a module error if the user touched the parameter.
 * Otherwise it will adjust the value without any error. */
#define DWC_OTG_PARAM_CHECK_VALID(_param_,_str_,_is_valid_,_set_valid_) \
	({ \
			int changed = DWC_OTG_PARAM_SET_DEFAULT(_param_); \
		int error = 0; \
		if (!(_is_valid_)) { \
			if (changed) { \
				DWC_ERROR("`%d' invalid for parameter `%s'.	 Check HW configuration.\n", dwc_otg_module_params._param_,_str_); \
				error = 1; \
			} \
			dwc_otg_module_params._param_ = (_set_valid_); \
		} \
		error; \
	})

	/* OTG Cap */
	retval += DWC_OTG_PARAM_CHECK_VALID(otg_cap,"otg_cap",
				  ({
					  int valid;
					  valid = 1;
					  switch (dwc_otg_module_params.otg_cap) {
					  case DWC_OTG_CAP_PARAM_HNP_SRP_CAPABLE:
						  if (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) valid = 0;
						  break;
					  case DWC_OTG_CAP_PARAM_SRP_ONLY_CAPABLE:
						  if ((core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST))
						  {
							  valid = 0;
						  }
						  break;
					  case DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE:
						  /* always valid */
						  break;
					  } 
					  valid;
			  }),
					(((core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST)) ?
					DWC_OTG_CAP_PARAM_SRP_ONLY_CAPABLE :
					DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE));

	retval += DWC_OTG_PARAM_CHECK_VALID(dma_enable,"dma_enable",
				((dwc_otg_module_params.dma_enable == 1) && (core_if->hwcfg2.b.architecture == 0)) ? 0 : 1, 
				0);

	retval += DWC_OTG_PARAM_CHECK_VALID(opt,"opt",
				1,
				0);

	DWC_OTG_PARAM_SET_DEFAULT(dma_burst_size);

	retval += DWC_OTG_PARAM_CHECK_VALID(host_support_fs_ls_low_power,
				"host_support_fs_ls_low_power",
				1, 0);

	retval += DWC_OTG_PARAM_CHECK_VALID(enable_dynamic_fifo,
					"enable_dynamic_fifo",
					((dwc_otg_module_params.enable_dynamic_fifo == 0) ||
					(core_if->hwcfg2.b.dynamic_fifo == 1)), 0);
	

	retval += DWC_OTG_PARAM_CHECK_VALID(data_fifo_size,
					"data_fifo_size",
					(dwc_otg_module_params.data_fifo_size <= core_if->hwcfg3.b.dfifo_depth),
					core_if->hwcfg3.b.dfifo_depth);

	retval += DWC_OTG_PARAM_CHECK_VALID(dev_rx_fifo_size,
					"dev_rx_fifo_size",
					(dwc_otg_module_params.dev_rx_fifo_size <= dwc_read_reg32(&core_if->core_global_regs->grxfsiz)),
					dwc_read_reg32(&core_if->core_global_regs->grxfsiz));

	retval += DWC_OTG_PARAM_CHECK_VALID(dev_nperio_tx_fifo_size,
					"dev_nperio_tx_fifo_size",
					(dwc_otg_module_params.dev_nperio_tx_fifo_size <= (dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16)),
					(dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16));

	retval += DWC_OTG_PARAM_CHECK_VALID(host_rx_fifo_size,
					"host_rx_fifo_size",
					(dwc_otg_module_params.host_rx_fifo_size <= dwc_read_reg32(&core_if->core_global_regs->grxfsiz)),
					dwc_read_reg32(&core_if->core_global_regs->grxfsiz));


	retval += DWC_OTG_PARAM_CHECK_VALID(host_nperio_tx_fifo_size,
					"host_nperio_tx_fifo_size",
					(dwc_otg_module_params.host_nperio_tx_fifo_size <= (dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16)),
					(dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16));

	retval += DWC_OTG_PARAM_CHECK_VALID(host_perio_tx_fifo_size,
					"host_perio_tx_fifo_size",
					(dwc_otg_module_params.host_perio_tx_fifo_size <= ((dwc_read_reg32(&core_if->core_global_regs->hptxfsiz) >> 16))),
					((dwc_read_reg32(&core_if->core_global_regs->hptxfsiz) >> 16)));

	retval += DWC_OTG_PARAM_CHECK_VALID(max_transfer_size,
					"max_transfer_size",
					(dwc_otg_module_params.max_transfer_size < (1 << (core_if->hwcfg3.b.xfer_size_cntr_width + 11))),
					((1 << (core_if->hwcfg3.b.xfer_size_cntr_width + 11)) - 1));

	retval += DWC_OTG_PARAM_CHECK_VALID(max_packet_count,
					"max_packet_count",
					(dwc_otg_module_params.max_packet_count < (1 << (core_if->hwcfg3.b.packet_size_cntr_width + 4))),
					((1 << (core_if->hwcfg3.b.packet_size_cntr_width + 4)) - 1));
//cathy
	retval += DWC_OTG_PARAM_CHECK_VALID(dev_endpoints,
					"dev_endpoints",
					(dwc_otg_module_params.dev_endpoints <= (core_if->hwcfg2.b.num_dev_ep)),
					core_if->hwcfg2.b.num_dev_ep);

/*
 * Define the following to disable the FS PHY Hardware checking.  This is for
 * internal testing only.
 *
 * #define NO_FS_PHY_HW_CHECKS 
 */

#ifdef NO_FS_PHY_HW_CHECKS
	retval += DWC_OTG_PARAM_CHECK_VALID(phy_type,
				"phy_type", 1, 0);
#else
	retval += DWC_OTG_PARAM_CHECK_VALID(phy_type,
				"phy_type",
				({
					int valid = 0;
					if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_UTMI) &&
					((core_if->hwcfg2.b.hs_phy_type == 1) || 
					 (core_if->hwcfg2.b.hs_phy_type == 3)))
					{
						valid = 1;
					}
					else if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_ULPI) &&
						 ((core_if->hwcfg2.b.hs_phy_type == 2) || 
						  (core_if->hwcfg2.b.hs_phy_type == 3)))
					{
						valid = 1;
					}
					else if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) &&
						 (core_if->hwcfg2.b.fs_phy_type == 1))
					{
						valid = 1;
					}
					valid;
				}),
				({
					int set = DWC_PHY_TYPE_PARAM_FS;
					if (core_if->hwcfg2.b.hs_phy_type) { 
						if ((core_if->hwcfg2.b.hs_phy_type == 3) || 
						(core_if->hwcfg2.b.hs_phy_type == 1)) {
							set = DWC_PHY_TYPE_PARAM_UTMI;
						}
						else {
							set = DWC_PHY_TYPE_PARAM_ULPI;
						}
					}
					set;
				}));
#endif

	retval += DWC_OTG_PARAM_CHECK_VALID(speed,"speed",
				(dwc_otg_module_params.speed == 0) && (dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? 0 : 1,
				dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS ? 1 : 0);

	retval += DWC_OTG_PARAM_CHECK_VALID(host_ls_low_power_phy_clk,
				"host_ls_low_power_phy_clk",
				((dwc_otg_module_params.host_ls_low_power_phy_clk == DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_48MHZ) && (dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? 0 : 1),
				((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_6MHZ : DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_48MHZ));

	DWC_OTG_PARAM_SET_DEFAULT(phy_ulpi_ddr);
	DWC_OTG_PARAM_SET_DEFAULT(phy_ulpi_ext_vbus);
	DWC_OTG_PARAM_SET_DEFAULT(phy_utmi_width);
	DWC_OTG_PARAM_SET_DEFAULT(ulpi_fs_ls);
	DWC_OTG_PARAM_SET_DEFAULT(ts_dline);

#ifdef NO_FS_PHY_HW_CHECKS
	retval += DWC_OTG_PARAM_CHECK_VALID(i2c_enable,
				"i2c_enable", 1, 0);
#else
	retval += DWC_OTG_PARAM_CHECK_VALID(i2c_enable,
				"i2c_enable",
				(dwc_otg_module_params.i2c_enable == 1) && (core_if->hwcfg3.b.i2c == 0) ? 0 : 1,
				0);
#endif

	for (i=0; i<15; i++) 
	{
		int changed = 1;
		int error = 0;

		if (dwc_otg_module_params.dev_perio_tx_fifo_size[i] == -1) 
		{
			changed = 0;
			dwc_otg_module_params.dev_perio_tx_fifo_size[i] = dwc_param_dev_perio_tx_fifo_size_default;
		}
		if (!(dwc_otg_module_params.dev_perio_tx_fifo_size[i] <= (dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i])))) 
		{
			if (changed) 
			{
				DWC_ERROR("`%d' invalid for parameter `dev_perio_fifo_size_%d'.	 Check HW configuration.\n", dwc_otg_module_params.dev_perio_tx_fifo_size[i],i);
				error = 1;
			}
			dwc_otg_module_params.dev_perio_tx_fifo_size[i] = dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]);
		}
		retval += error;
	}


	retval += DWC_OTG_PARAM_CHECK_VALID(en_multiple_tx_fifo,"en_multiple_tx_fifo",
						((dwc_otg_module_params.en_multiple_tx_fifo == 1) && (core_if->hwcfg4.b.ded_fifo_en == 0)) ? 0 : 1, 
						0);

	
	for (i=0; i<15; i++) 
	{

		int changed = 1;
		int error = 0;

		if (dwc_otg_module_params.dev_tx_fifo_size[i] == -1) 
		{
			changed = 0;
			dwc_otg_module_params.dev_tx_fifo_size[i] = dwc_param_dev_tx_fifo_size_default;
		}
		if (!(dwc_otg_module_params.dev_tx_fifo_size[i] <= (dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i])))) 
		{
			if (changed) 
			{
				DWC_ERROR("%d' invalid for parameter `dev_perio_fifo_size_%d'.	Check HW configuration.\n", dwc_otg_module_params.dev_tx_fifo_size[i],i);
				error = 1;
			}
			dwc_otg_module_params.dev_tx_fifo_size[i] = dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]);
		}
		retval += error;
		
		
	}
	
	DWC_OTG_PARAM_SET_DEFAULT(thr_ctl);
	DWC_OTG_PARAM_SET_DEFAULT(tx_thr_length);
	DWC_OTG_PARAM_SET_DEFAULT(rx_thr_length);
	
	return retval;
}

/** 
 * This function is the top level interrupt handler for the Common
 * (Device and host modes) interrupts.
 */
static irqreturn_t dwc_otg_common_irq(int _irq, void *_dev, struct pt_regs *_r)
{
	dwc_otg_device_t *otg_dev = _dev;
	int32_t retval = 0;
	//printk("Yes! %s intr handler got called here\n", __func__);//shlee
	retval = dwc_otg_handle_common_intr( otg_dev->core_if );
	return IRQ_RETVAL(retval);
}

/**
 * This function is called when a lm_device is unregistered with the
 * dwc_otg_driver. This happens, for example, when the rmmod command is
 * executed. The device may or may not be electrically present. If it is
 * present, the driver stops device processing. Any resources used on behalf
 * of this device are freed.
 *
 * @param[in] _lmdev
 */
static void dwc_otg_driver_remove(struct lm_device *_lmdev)
{
	dwc_otg_device_t *otg_dev = _lmdev->dev;
	DWC_DEBUGPL(DBG_ANY, "%s(%p)\n", __func__, _lmdev);
	
	if (otg_dev == NULL) 
	{
		/* Memory allocation for the dwc_otg_device failed. */
		return;
	}
	/*
	 * Free the IRQ 
	 */
	if (otg_dev->common_irq_installed) 
	{
		free_irq( _lmdev->irq, otg_dev );
	}
#ifndef DWC_DEVICE_ONLY
	if (otg_dev->hcd != NULL) 
	{
		dwc_otg_hcd_remove( _lmdev );
	}
#endif
#ifndef DWC_HOST_ONLY
	if (otg_dev->pcd != NULL) 
	{
		dwc_otg_pcd_remove( _lmdev );
	}
#endif
	if (otg_dev->core_if != NULL) 
	{
		dwc_otg_cil_remove( otg_dev->core_if );
	}
	/*
	 * Return the memory.
	 */
	if (otg_dev->base != NULL) 
	{
		iounmap(otg_dev->base);
	}
	kfree(otg_dev);
		
	/*
	 * Clear the drvdata pointer.
	 */
	_lmdev->dev = 0;
}

/**
 * This function is called when an lm_device is bound to a
 * dwc_otg_driver. It creates the driver components required to
 * control the device (CIL, HCD, and PCD) and it initializes the
 * device. The driver components are stored in a dwc_otg_device
 * structure. A reference to the dwc_otg_device is saved in the
 * lm_device. This allows the driver to access the dwc_otg_device
 * structure on subsequent calls to driver methods for this device.
 *
 * @param[in] _lmdev  lm_device definition
 */
static int dwc_otg_driver_probe(struct lm_device *_lmdev)
{
	int retval = 0;
	dwc_otg_device_t *dwc_otg_device;
	int32_t snpsid;

	dwc_otg_device = kmalloc(sizeof(dwc_otg_device_t), GFP_KERNEL);
	
	if (dwc_otg_device == 0) 
	{
		retval = -ENOMEM;
		goto fail;
	}
	
	memset(dwc_otg_device, 0, sizeof(*dwc_otg_device));
	dwc_otg_device->reg_offset = 0xFFFFFFFF;

	/*
	 * Map the DWC_otg Core memory into virtual address space.
	 */
	//cathy, set base address
	(unsigned)dwc_otg_device->base = _lmdev->start;
	
	if (dwc_otg_device->base == NULL)
	{
		retval = -ENOMEM;
		goto fail;
	}

	//eason
//	dev_dbg(&_lmdev->dev, "base=0x%08x\n", (unsigned)dwc_otg_device->base);
	printk(KERN_INFO "base=0x%08x\n", (unsigned)dwc_otg_device->base);
	/////

	/*
	 * Attempt to ensure this device is really a DWC_otg Controller.
	 * Read and verify the SNPSID register contents. The value should be
	 * 0x45F42XXX, which corresponds to "OT2", as in "OTG version 2.XX".
	 */

	snpsid = dwc_read_reg32((uint32_t *)((uint8_t *)dwc_otg_device->base + 0x40));	

	if ((snpsid & 0xFFFFF000) != 0x4F542000) 
	{
//		dev_err(&_lmdev->dev, "Bad value for SNPSID: 0x%08x\n", snpsid);
		retval = -EINVAL;
		goto fail;
	}

	/*
	 * Initialize driver data to point to the global DWC_otg
	 * Device structure.
	 */
	_lmdev->dev = dwc_otg_device;
//	dev_dbg(&_lmdev->dev, "dwc_otg_device=0x%p\n", dwc_otg_device);
//printk("dwc_otg_device=0x%p\n", dwc_otg_device);	
	dwc_otg_device->core_if = dwc_otg_cil_init( dwc_otg_device->base, 
							&dwc_otg_module_params);
	if (dwc_otg_device->core_if == 0) 
	{
//		dev_err(&_lmdev->dev, "CIL initialization failed!\n");
		retval = -ENOMEM;
		goto fail;
	}
	
	/*
	 * Validate parameter values.
	 */

	/*
	 * Disable the global interrupt until all the interrupt
	 * handlers are installed.
	 */

	dwc_otg_disable_global_interrupts( dwc_otg_device->core_if );

	/*
	 * Install the interrupt handler for the common interrupts before
	 * enabling common interrupts in core_init below.
	 */
	DWC_DEBUGPL( DBG_CIL, "registering (common) handler for irq%d\n", _lmdev->irq);
	retval = request_irq(_lmdev->irq, dwc_otg_common_irq, SA_SHIRQ, "dwc_otg", dwc_otg_device );
	if (retval != 0) 
	{
		DWC_ERROR("request of irq%d failed\n", _lmdev->irq);
		retval = -EBUSY;
		goto fail;
	} 
	else
	{
		dwc_otg_device->common_irq_installed = 1;
	}


#ifdef CONFIG_MACH_IPMATE
	set_irq_type(_lmdev->irq, IRQT_LOW);
#endif

	/*
	 * Initialize the DWC_otg core.
	 */
	dwc_otg_core_init( dwc_otg_device->core_if );

#ifndef DWC_HOST_ONLY
	/*
	 * Initialize the PCD
	 */
	 printk("--init pcd--\n");
	retval = dwc_otg_pcd_init( _lmdev );
	if (retval != 0) 
	{
		DWC_ERROR("dwc_otg_pcd_init failed, retval = %d\n", retval);
		dwc_otg_device->pcd = NULL;
		goto fail;
	}
#endif
#ifndef DWC_DEVICE_ONLY
	/*
	 * Initialize the HCD
	 */
	printk("----init hcd-----\n");
	retval = dwc_otg_hcd_init(_lmdev);
	if (retval != 0) 
	{
		DWC_ERROR("dwc_otg_hcd_init failed\n");
		dwc_otg_device->hcd = NULL;
		goto fail;
	}
#endif
	/*
	 * Enable the global interrupt after all the interrupt
	 * handlers are installed.
	 */
	dwc_otg_enable_global_interrupts( dwc_otg_device->core_if );
	return 0;

 fail:
	dwc_otg_driver_remove(_lmdev);
	return retval;
}

void dwc_otg_phy_write(unsigned char reg, unsigned char val)
{
       int tmp = REG32(0xb8003314);

       if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
               printk("bad register address: 0x%02x\n", reg);
               return;
       }

       tmp = tmp & 0xFF00FF00;
       REG32(0xb8003314) = (val << 16) | tmp; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
       REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

       return;
}
                   
/**
 * This function is called when the dwc_otg_driver is installed with the
 * insmod command. It registers the dwc_otg_driver structure with the
 * appropriate bus driver. This will cause the dwc_otg_driver_probe function
 * to be called. In addition, the bus driver will automatically expose
 * attributes defined for the device and driver in the special sysfs file
 * system.
 *
 * @return
 */
static int __init dwc_otg_driver_init(void) 
{
	int retval = 0, snpsid;
	struct lm_device *lmdev;

	int tmp = REG32(0xb8003314);

	//write usb port0's phy reg 0xE7 to change vbusvalid threshold down to 3.3V
	if(IS_6085) {
		printk("RLE6085 USB phy 0\n");
		// For Port-0
		dwc_otg_phy_write(0xE7, 0x38);	//write usb port0's phy reg 0xE7 to change vbusvalid threshold down to 3.3V
		dwc_otg_phy_write(0xE2, 0x81);
		dwc_otg_phy_write(0xE5, 0x95);
#ifndef DWC_DEVICE_ONLY
		dwc_otg_phy_write(0xE6, 0xE8);
#endif
		dwc_otg_phy_write(0xF1, 0x8E);
	}
	else if(IS_RLE0315) {
		// For Port-0
		printk("RLE0315 USB phy 0\n");
		dwc_otg_phy_write(0xE0, 0xB8);
		dwc_otg_phy_write(0xE1, 0xA8);
		dwc_otg_phy_write(0xE2, 0x99);
		dwc_otg_phy_write(0xE3, 0x41);
		dwc_otg_phy_write(0xE5, 0x91);
		dwc_otg_phy_write(0xE6, 0x78);
		dwc_otg_phy_write(0xF4, 0xE3);
		dwc_otg_phy_write(0xF5, 0x12);	//change vbusvalid threshold down to 3.3V
		dwc_otg_phy_write(0xF6, 0x04);
	}
	else {
		printk("UNKNOWN chip USB phy 0\n");
		tmp = tmp & 0xFF00FF00;
		REG32(0xb8003314) = tmp|0x00000038; USB2_PHY_DELAY;	//write usb port0's phy reg 0xE7 to change vbusvalid threshold down to 3.3V
		REG32(0xb80210A4) = 0x00370000; USB2_PHY_DELAY;	
		REG32(0xb80210A4) = 0x00270000; USB2_PHY_DELAY;	
		REG32(0xb80210A4) = 0x00370000; USB2_PHY_DELAY;	
		REG32(0xb80210A4) = 0x003E0000; USB2_PHY_DELAY;	
		REG32(0xb80210A4) = 0x002E0000; USB2_PHY_DELAY;	
		REG32(0xb80210A4) = 0x003E0000; USB2_PHY_DELAY;	

		dwc_otg_phy_write(0xE2, 0x82);
		dwc_otg_phy_write(0xE5, 0x91);
#ifndef DWC_DEVICE_ONLY
		dwc_otg_phy_write(0xE6, 0xE8);
#endif
		dwc_otg_phy_write(0xF1, 0x8E);
	}
	
	printk(KERN_INFO "%s: version %s\n", dwc_driver_name, DWC_DRIVER_VERSION);

	//probe OTG
	snpsid = dwc_read_reg32((uint32_t *)((uint8_t *)OTG_BASE + 0x40));
	
	if ((snpsid & 0xFFFFF000) != 0x4F542000) {
		DWC_ERROR("OTG Device not found ! Bad value for SNPSID: 0x%08x\n", snpsid);
		return 0;
	}
	else
		printk(KERN_INFO "OTG Device work now. SNPSID: 0x%08x\n", snpsid);

	glmdev = kmalloc(sizeof(struct lm_device), GFP_KERNEL);
	memset(glmdev, 0, sizeof(struct lm_device));
	if (!glmdev) {
		printk("kmalloc Imdev in dwc_otg_driver_init failed!\n");
		return -1;
	}

	glmdev->start = OTG_BASE;
	glmdev->end = glmdev->start + 0x0003ffff;
	glmdev->irq = USB_D_IRQ;	//irq of usb device
	glmdev->id = 0;
	dwc_otg_driver_probe(glmdev);
	return 0;
}
module_init(dwc_otg_driver_init);

/** 
 * This function is called when the driver is removed from the kernel
 * with the rmmod command. The driver unregisters itself with its bus
 * driver.
 *
 */
static void __exit dwc_otg_driver_cleanup(void)
{
	printk(KERN_DEBUG "dwc_otg_driver_cleanup()\n");

	//cathy
	//driver_remove_file(&dwc_otg_driver.drv, &driver_attr_debuglevel);
	//driver_remove_file(&dwc_otg_driver.drv, &driver_attr_version);

	//lm_driver_unregister(&dwc_otg_driver);
	//usb_deregister(&dwc_otg_driver);
	if(glmdev != NULL){
		dwc_otg_driver_remove(glmdev);
	}
	printk(KERN_INFO "%s module removed\n", dwc_driver_name);
}
module_exit(dwc_otg_driver_cleanup);

MODULE_DESCRIPTION(DWC_DRIVER_DESC);
MODULE_AUTHOR("Synopsys Inc.");
MODULE_LICENSE("GPL");

//module_param_named(otg_cap, dwc_otg_module_params.otg_cap, int, 0444);
MODULE_PARM_DESC(otg_cap, "OTG Capabilities 0=HNP&SRP 1=SRP Only 2=None");
//module_param_named(opt, dwc_otg_module_params.opt, int, 0444);
MODULE_PARM_DESC(opt, "OPT Mode");
//module_param_named(dma_enable, dwc_otg_module_params.dma_enable, int, 0444);
MODULE_PARM_DESC(dma_enable, "DMA Mode 0=Slave 1=DMA enabled");
//module_param_named(dma_burst_size, dwc_otg_module_params.dma_burst_size, int, 0444);
MODULE_PARM_DESC(dma_burst_size, "DMA Burst Size 1, 4, 8, 16, 32, 64, 128, 256");
//module_param_named(speed, dwc_otg_module_params.speed, int, 0444);
MODULE_PARM_DESC(speed, "Speed 0=High Speed 1=Full Speed");
//module_param_named(host_support_fs_ls_low_power, dwc_otg_module_params.host_support_fs_ls_low_power, int, 0444);
MODULE_PARM_DESC(host_support_fs_ls_low_power, "Support Low Power w/FS or LS 0=Support 1=Don't Support");
//module_param_named(host_ls_low_power_phy_clk, dwc_otg_module_params.host_ls_low_power_phy_clk, int, 0444);
MODULE_PARM_DESC(host_ls_low_power_phy_clk, "Low Speed Low Power Clock 0=48Mhz 1=6Mhz");
//module_param_named(enable_dynamic_fifo, dwc_otg_module_params.enable_dynamic_fifo, int, 0444);
MODULE_PARM_DESC(enable_dynamic_fifo, "0=cC Setting 1=Allow Dynamic Sizing");
//module_param_named(data_fifo_size, dwc_otg_module_params.data_fifo_size, int, 0444);
MODULE_PARM_DESC(data_fifo_size, "Total number of words in the data FIFO memory 32-32768");
//module_param_named(dev_rx_fifo_size, dwc_otg_module_params.dev_rx_fifo_size, int, 0444);
MODULE_PARM_DESC(dev_rx_fifo_size, "Number of words in the Rx FIFO 16-32768");
//module_param_named(dev_nperio_tx_fifo_size, dwc_otg_module_params.dev_nperio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(dev_nperio_tx_fifo_size, "Number of words in the non-periodic Tx FIFO 16-32768");
//module_param_named(dev_perio_tx_fifo_size_1, dwc_otg_module_params.dev_perio_tx_fifo_size[0], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_1, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_2, dwc_otg_module_params.dev_perio_tx_fifo_size[1], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_2, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_3, dwc_otg_module_params.dev_perio_tx_fifo_size[2], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_3, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_4, dwc_otg_module_params.dev_perio_tx_fifo_size[3], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_4, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_5, dwc_otg_module_params.dev_perio_tx_fifo_size[4], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_5, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_6, dwc_otg_module_params.dev_perio_tx_fifo_size[5], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_6, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_7, dwc_otg_module_params.dev_perio_tx_fifo_size[6], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_7, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_8, dwc_otg_module_params.dev_perio_tx_fifo_size[7], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_8, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_9, dwc_otg_module_params.dev_perio_tx_fifo_size[8], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_9, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_10, dwc_otg_module_params.dev_perio_tx_fifo_size[9], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_10, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_11, dwc_otg_module_params.dev_perio_tx_fifo_size[10], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_11, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_12, dwc_otg_module_params.dev_perio_tx_fifo_size[11], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_12, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_13, dwc_otg_module_params.dev_perio_tx_fifo_size[12], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_13, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_14, dwc_otg_module_params.dev_perio_tx_fifo_size[13], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_14, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(dev_perio_tx_fifo_size_15, dwc_otg_module_params.dev_perio_tx_fifo_size[14], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_15, "Number of words in the periodic Tx FIFO 4-768");
//module_param_named(host_rx_fifo_size, dwc_otg_module_params.host_rx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_rx_fifo_size, "Number of words in the Rx FIFO 16-32768");
//module_param_named(host_nperio_tx_fifo_size, dwc_otg_module_params.host_nperio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_nperio_tx_fifo_size, "Number of words in the non-periodic Tx FIFO 16-32768");
//module_param_named(host_perio_tx_fifo_size, dwc_otg_module_params.host_perio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_perio_tx_fifo_size, "Number of words in the host periodic Tx FIFO 16-32768");
//module_param_named(max_transfer_size, dwc_otg_module_params.max_transfer_size, int, 0444);
/** @todo Set the max to 512K, modify checks */
MODULE_PARM_DESC(max_transfer_size, "The maximum transfer size supported in bytes 2047-65535");
//module_param_named(max_packet_count, dwc_otg_module_params.max_packet_count, int, 0444);
MODULE_PARM_DESC(max_packet_count, "The maximum number of packets in a transfer 15-511");
//module_param_named(host_channels, dwc_otg_module_params.host_channels, int, 0444);
MODULE_PARM_DESC(host_channels, "The number of host channel registers to use 1-16");
//module_param_named(dev_endpoints, dwc_otg_module_params.dev_endpoints, int, 0444);
MODULE_PARM_DESC(dev_endpoints, "The number of endpoints in addition to EP0 available for device mode 1-15");
//module_param_named(phy_type, dwc_otg_module_params.phy_type, int, 0444);
MODULE_PARM_DESC(phy_type, "0=Reserved 1=UTMI+ 2=ULPI");
//module_param_named(phy_utmi_width, dwc_otg_module_params.phy_utmi_width, int, 0444);
MODULE_PARM_DESC(phy_utmi_width, "Specifies the UTMI+ Data Width 8 or 16 bits");
//module_param_named(phy_ulpi_ddr, dwc_otg_module_params.phy_ulpi_ddr, int, 0444);
MODULE_PARM_DESC(phy_ulpi_ddr, "ULPI at double or single data rate 0=Single 1=Double");
//module_param_named(phy_ulpi_ext_vbus, dwc_otg_module_params.phy_ulpi_ext_vbus, int, 0444);
MODULE_PARM_DESC(phy_ulpi_ext_vbus, "ULPI PHY using internal or external vbus 0=Internal");
//module_param_named(i2c_enable, dwc_otg_module_params.i2c_enable, int, 0444);
MODULE_PARM_DESC(i2c_enable, "FS PHY Interface");
//module_param_named(ulpi_fs_ls, dwc_otg_module_params.ulpi_fs_ls, int, 0444);
MODULE_PARM_DESC(ulpi_fs_ls, "ULPI PHY FS/LS mode only");
//module_param_named(ts_dline, dwc_otg_module_params.ts_dline, int, 0444);
MODULE_PARM_DESC(ts_dline, "Term select Dline pulsing for all PHYs");
//module_param_named(debug, g_dbg_lvl, int, 0444);
MODULE_PARM_DESC(debug, "");

//module_param_named(en_multiple_tx_fifo, dwc_otg_module_params.en_multiple_tx_fifo, int, 0444);
MODULE_PARM_DESC(en_multiple_tx_fifo, "Dedicated Non Periodic Tx FIFOs 0=disabled 1=enabled");
//module_param_named(dev_tx_fifo_size_1, dwc_otg_module_params.dev_tx_fifo_size[0], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_1, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_2, dwc_otg_module_params.dev_tx_fifo_size[1], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_2, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_3, dwc_otg_module_params.dev_tx_fifo_size[2], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_3, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_4, dwc_otg_module_params.dev_tx_fifo_size[3], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_4, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_5, dwc_otg_module_params.dev_tx_fifo_size[4], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_5, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_6, dwc_otg_module_params.dev_tx_fifo_size[5], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_6, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_7, dwc_otg_module_params.dev_tx_fifo_size[6], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_7, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_8, dwc_otg_module_params.dev_tx_fifo_size[7], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_8, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_9, dwc_otg_module_params.dev_tx_fifo_size[8], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_9, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_10, dwc_otg_module_params.dev_tx_fifo_size[9], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_10, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_11, dwc_otg_module_params.dev_tx_fifo_size[10], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_11, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_12, dwc_otg_module_params.dev_tx_fifo_size[11], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_12, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_13, dwc_otg_module_params.dev_tx_fifo_size[12], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_13, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_14, dwc_otg_module_params.dev_tx_fifo_size[13], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_14, "Number of words in the Tx FIFO 4-768");
//module_param_named(dev_tx_fifo_size_15, dwc_otg_module_params.dev_tx_fifo_size[14], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_15, "Number of words in the Tx FIFO 4-768");

//module_param_named(thr_ctl, dwc_otg_module_params.thr_ctl, int, 0444);
MODULE_PARM_DESC(thr_ctl, "Thresholding enable flag bit 0 - non ISO Tx thr., 1 - ISO Tx thr., 2 - Rx thr.- bit 0=disabled 1=enabled");
//module_param_named(tx_thr_length, dwc_otg_module_params.tx_thr_length, int, 0444);
MODULE_PARM_DESC(tx_thr_length, "Tx Threshold length in 32 bit DWORDs");
//module_param_named(rx_thr_length, dwc_otg_module_params.rx_thr_length, int, 0444);
MODULE_PARM_DESC(rx_thr_length, "Rx Threshold length in 32 bit DWORDs");
/** @page "Module Parameters"
 *
 * The following parameters may be specified when starting the module.
 * These parameters define how the DWC_otg controller should be
 * configured.	Parameter values are passed to the CIL initialization
 * function dwc_otg_cil_init
 *
 * Example: <code>modprobe dwc_otg speed=1 otg_cap=1</code>
 *
 
 <table>
 <tr><td>Parameter Name</td><td>Meaning</td></tr> 
 
 <tr>
 <td>otg_cap</td>
 <td>Specifies the OTG capabilities. The driver will automatically detect the
 value for this parameter if none is specified.
 - 0: HNP and SRP capable (default, if available)
 - 1: SRP Only capable
 - 2: No HNP/SRP capable
 </td></tr>
 
 <tr>
 <td>dma_enable</td>
 <td>Specifies whether to use slave or DMA mode for accessing the data FIFOs.
 The driver will automatically detect the value for this parameter if none is
 specified.
 - 0: Slave
 - 1: DMA (default, if available)
 </td></tr>
 
 <tr>
 <td>dma_burst_size</td>
 <td>The DMA Burst size (applicable only for External DMA Mode).
 - Values: 1, 4, 8 16, 32, 64, 128, 256 (default 32)
 </td></tr>
 
 <tr>
 <td>speed</td>
 <td>Specifies the maximum speed of operation in host and device mode. The
 actual speed depends on the speed of the attached device and the value of
 phy_type.
 - 0: High Speed (default)
 - 1: Full Speed
 </td></tr>
 
 <tr>
 <td>host_support_fs_ls_low_power</td>
 <td>Specifies whether low power mode is supported when attached to a Full
 Speed or Low Speed device in host mode.
 - 0: Don't support low power mode (default)
 - 1: Support low power mode
 </td></tr>
 
 <tr>
 <td>host_ls_low_power_phy_clk</td>
 <td>Specifies the PHY clock rate in low power mode when connected to a Low
 Speed device in host mode. This parameter is applicable only if
 HOST_SUPPORT_FS_LS_LOW_POWER is enabled.
 - 0: 48 MHz (default)
 - 1: 6 MHz
 </td></tr>
 
 <tr>
 <td>enable_dynamic_fifo</td>
 <td> Specifies whether FIFOs may be resized by the driver software.
 - 0: Use cC FIFO size parameters
 - 1: Allow dynamic FIFO sizing (default)
 </td></tr>
 
 <tr>
 <td>data_fifo_size</td>
 <td>Total number of 4-byte words in the data FIFO memory. This memory
 includes the Rx FIFO, non-periodic Tx FIFO, and periodic Tx FIFOs.
 - Values: 32 to 32768 (default 8192)

 Note: The total FIFO memory depth in the FPGA configuration is 8192.
 </td></tr>
 
 <tr>
 <td>dev_rx_fifo_size</td>
 <td>Number of 4-byte words in the Rx FIFO in device mode when dynamic
 FIFO sizing is enabled.
 - Values: 16 to 32768 (default 1064)
 </td></tr>
 
 <tr>
 <td>dev_nperio_tx_fifo_size</td>
 <td>Number of 4-byte words in the non-periodic Tx FIFO in device mode when
 dynamic FIFO sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>
 
 <tr>
 <td>dev_perio_tx_fifo_size_n (n = 1 to 15)</td>
 <td>Number of 4-byte words in each of the periodic Tx FIFOs in device mode
 when dynamic FIFO sizing is enabled.
 - Values: 4 to 768 (default 256)
 </td></tr>
 
 <tr>
 <td>host_rx_fifo_size</td>
 <td>Number of 4-byte words in the Rx FIFO in host mode when dynamic FIFO
 sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>
 
 <tr>
 <td>host_nperio_tx_fifo_size</td>
 <td>Number of 4-byte words in the non-periodic Tx FIFO in host mode when
 dynamic FIFO sizing is enabled in the core.
 - Values: 16 to 32768 (default 1024)
 </td></tr>
 
 <tr>
 <td>host_perio_tx_fifo_size</td>
 <td>Number of 4-byte words in the host periodic Tx FIFO when dynamic FIFO
 sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>
 
 <tr>
 <td>max_transfer_size</td>
 <td>The maximum transfer size supported in bytes.
 - Values: 2047 to 65,535 (default 65,535)
 </td></tr>
 
 <tr>
 <td>max_packet_count</td>
 <td>The maximum number of packets in a transfer.
 - Values: 15 to 511 (default 511)
 </td></tr>
 
 <tr>
 <td>host_channels</td>
 <td>The number of host channel registers to use.
 - Values: 1 to 16 (default 12)

 Note: The FPGA configuration supports a maximum of 12 host channels.
 </td></tr>
 
 <tr>
 <td>dev_endpoints</td>
 <td>The number of endpoints in addition to EP0 available for device mode
 operations.
 - Values: 1 to 15 (default 6 IN and OUT)

 Note: The FPGA configuration supports a maximum of 6 IN and OUT endpoints in
 addition to EP0.
 </td></tr>
 
 <tr>
 <td>phy_type</td>
 <td>Specifies the type of PHY interface to use. By default, the driver will
 automatically detect the phy_type.
 - 0: Full Speed
 - 1: UTMI+ (default, if available)
 - 2: ULPI
 </td></tr>
 
 <tr>
 <td>phy_utmi_width</td>
 <td>Specifies the UTMI+ Data Width. This parameter is applicable for a
 phy_type of UTMI+. Also, this parameter is applicable only if the
 OTG_HSPHY_WIDTH cC parameter was set to "8 and 16 bits", meaning that the
 core has been configured to work at either data path width.
 - Values: 8 or 16 bits (default 16)
 </td></tr>
 
 <tr>
 <td>phy_ulpi_ddr</td>
 <td>Specifies whether the ULPI operates at double or single data rate. This
 parameter is only applicable if phy_type is ULPI.
 - 0: single data rate ULPI interface with 8 bit wide data bus (default)
 - 1: double data rate ULPI interface with 4 bit wide data bus
 </td></tr>

 <tr>
 <td>i2c_enable</td>
 <td>Specifies whether to use the I2C interface for full speed PHY. This
 parameter is only applicable if PHY_TYPE is FS.
 - 0: Disabled (default)
 - 1: Enabled
 </td></tr>

 <tr>
 <td>otg_en_multiple_tx_fifo</td>
 <td>Specifies whether dedicatedto tx fifos are enabled for non periodic IN EPs.
 The driver will automatically detect the value for this parameter if none is
 specified.
 - 0: Disabled
 - 1: Enabled (default, if available)
 </td></tr>

 <tr>
 <td>dev_tx_fifo_size_n (n = 1 to 15)</td>
 <td>Number of 4-byte words in each of the Tx FIFOs in device mode
 when dynamic FIFO sizing is enabled.
 - Values: 4 to 768 (default 256)
 </td></tr>

*/
