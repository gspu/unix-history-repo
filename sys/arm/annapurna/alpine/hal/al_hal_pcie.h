/*-
********************************************************************************
Copyright (C) 2015 Annapurna Labs Ltd.

This file may be licensed under the terms of the Annapurna Labs Commercial
License Agreement.

Alternatively, this file can be distributed under the terms of the GNU General
Public License V2 as published by the Free Software Foundation and can be
found at http://www.gnu.org/licenses/gpl-2.0.html

Alternatively, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

    *     Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

    *     Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/**
 * @defgroup grouppcie PCI Express Controller
 *  @{
 * @section overview Overview
 * This header file provide API for the HAL driver of the pcie port, the driver
 * provides the following functionalities:
 * - Port initialization
 * - Link operation
 * - Interrupts transactions generation (Endpoint mode).
 * - Configuration Access management functions
 * - Internal Translation Unit programming
 *
 * This API does not provide the following:
 * - PCIe transactions generation and reception (except interrupts as mentioned
 *   above) as this functionality is done by the port without need for sw
 *   intervention.
 * - Configuration Access: those transactions are generated automatically by
 *   the port (ECAM or ATU mode) when the CPU issues memory transaction
 *   through the fabric toward the PCIe port. This API provides management
 *   function for controlling the Configuration Access type and bus destination
 * - Interrupt Handling.
 * - Message Generation: common used messages are automatically generated, also,
 *   the ATU generic mechanism for generating various kind of messages.
 * - PCIe Port Management: both link and port power management features can be
 *   managed using the PCI/PCIe standard power management and PCIe capabilities
 *   registers.
 * - PCIe link and protocol error handling: the feature can be managed using
 *   the Advanced Error Handling PCIe capability registers.
 *
 * @section flows Software Flows
 * @subsection init Initialization
 *   - allocation and set zeros al_pcie_port and al_pcie_pf structures handles
 *   - call al_pcie_port_handle_init() with pointer to the allocated
 *     al_pcie_port handle, address of the port internal registers space, and
 *     port id.
 *   - call al_pcie_pf_handle_init() with pointer to the al_pcie_port handle
 *     and pf_number.
 *   - set the port mode, End-Point or Root-Compex (default).
 *   - set number of lanes connected to the controller.
 *   - enable the controller using the al_pcie_port_enable(). note that this
 *     function expect the virtual address of the PBS Functional Registers.
 *   - wait for 2000 South-bridge cycles.
 *   - prepare al_pcie_port_config_params and al_pcie_pf_config_params
 *     structures depending on chip, board and system configuration.
 *     for example, when using the port as root complex, the operating_mode
 *     field should be set to AL_PCIE_OPERATING_MODE_RC. In this example we
 *     prepare the following configuration:
 *     For port configuration
 *     - Root Complex mode
 *     - Set the Max Link Speed to Gen2
 *     - Set the max lanes width to 2 (x2)
 *     - Disable reversal mode
 *     - Enable Snoops to support I/O Hardware cache coherency
 *     - Enable pcie core RAM parity
 *     - Enable pcie core AXI parity
 *     - Keep transaction layer default credits
 *     For pf configuration
 *     - No EP parameters
 *     - No SR-IOV parameters
 *     so the structures we prepare:
 *     @code
 *     - struct al_pcie_link_params link_params = {
 *		AL_PCIE_LINK_SPEED_GEN2,
 *		AL_FALSE,	// disable reversal mode
 *		AL_PCIE_MPS_DEFAULT};
 *
 *     - struct al_pcie_port_config_params config_params = {
 *		&link_params,
 *		AL_TRUE, // enable Snoop for inbound memory transactions
 *		AL_TRUE, // enable pcie port RAM parity
 *		AL_TRUE, // enable pcie port AXI parity
 *		NULL, // use default latency/replay timers
 *		NULL, // use default gen2 pipe params
 *		NULL, // gen3_params not needed when max speed set to Gen2
 *		NULL, // don't change TL credits
 *		NULL, // end point params not needed
 *		AL_FALSE, //no fast link
 *		AL_FALSE};	//return 0xFFFFFFFF for read transactions with
 *				//pci target error
 *	@endcode
 *	- now call al_pcie_port_config() with pcie_port and port_config_params
 * @subsection link-init Link Initialization
 *  - once the port configured, we can start PCIe link:
 *  - call al_pcie_link_start()
 *  - call al_pcie_link_up_wait()
 *  - allocate al_pcie_link_status struct and call al_pcie_link_status() and
 *    check the link is established.
 *
 *  @subsection  cap Configuration Access Preparation
 *  - Once the link is established, we can prepare the port for pci
 *  configuration access, this stage requires system knowledge about the PCI
 *  buses enumeration. For example, if 5 buses were discovered on previously
 *  scanned root complex port, then we should start enumeration from bus 5 (PCI
 *  secondary bus), the sub-ordinary bus will be temporarily set to maximum
 *  value (255) until the scan process under this bus is finished, then it will
 *  updated to the maximum bus value found. So we use the following sequence:
 *  - call al_pcie_secondary_bus_set() with sec-bus = 5
 *  - call al_pcie_subordinary_bus_set() with sub-bus = 255
 *
 *  @subsection cfg Configuration (Cfg) Access Generation
 *  - we assume using ECAM method, in this method, the software issues pcie Cfg
 *  access by accessing the ECAM memory space of the pcie port. For example, to
 *  issue 4 byte Cfg Read from bus B, Device D, Function F and register R, the
 *  software issues 4 byte read access to the following physical address
 *  ECAM base address of the port + (B << 20) + (D << 15) + (F << 12) + R.
 *  But, as the default size of the ECAM address space is less than
 *  needed full range (256MB), we modify the target_bus value prior to Cfg
 *  access in order make the port generate Cfg access with bus value set to the
 *  value of the target_bus rather than bits 27:20 of the physical address.
 *  - call al_pcie_target_bus_set() with target_bus set to the required bus of
 *   the next Cfg access to be issued, mask_target_bus will be set to 0xff.
 *   no need to call that function if the next Cfg access bus equals to the last
 *   value set to target_bus.
 *
 *      @file  al_hal_pcie.h
 *      @brief HAL Driver Header for the Annapurna Labs PCI Express port.
 */

#ifndef _AL_HAL_PCIE_H_
#define _AL_HAL_PCIE_H_

#include "al_hal_common.h"
#include "al_hal_pcie_regs.h"

/******************************************************************************/
/********************************* Constants **********************************/
/******************************************************************************/

/** Inbound header credits sum - rev 0/1/2 */
#define AL_PCIE_REV_1_2_IB_HCRD_SUM			97
/** Inbound header credits sum - rev 3 */
#define AL_PCIE_REV3_IB_HCRD_SUM			259

/** Number of extended registers */
#define AL_PCIE_EX_REGS_NUM				40

/*******************************************************************************
 * PCIe AER uncorrectable error bits
 * To be used with the following functions:
 * - al_pcie_aer_config
 * - al_pcie_aer_uncorr_get_and_clear
 ******************************************************************************/
/** Data Link Protocol Error */
#define AL_PCIE_AER_UNCORR_DLP_ERR			AL_BIT(4)
/** Poisoned TLP */
#define AL_PCIE_AER_UNCORR_POISIONED_TLP		AL_BIT(12)
/** Flow Control Protocol Error */
#define AL_PCIE_AER_UNCORR_FLOW_CTRL_ERR		AL_BIT(13)
/** Completion Timeout */
#define AL_PCIE_AER_UNCORR_COMPL_TO			AL_BIT(14)
/** Completer Abort */
#define AL_PCIE_AER_UNCORR_COMPL_ABT			AL_BIT(15)
/** Unexpected Completion */
#define AL_PCIE_AER_UNCORR_UNEXPCTED_COMPL		AL_BIT(16)
/** Receiver Overflow */
#define AL_PCIE_AER_UNCORR_RCV_OVRFLW			AL_BIT(17)
/** Malformed TLP */
#define AL_PCIE_AER_UNCORR_MLFRM_TLP			AL_BIT(18)
/** ECRC Error */
#define AL_PCIE_AER_UNCORR_ECRC_ERR			AL_BIT(19)
/** Unsupported Request Error */
#define AL_PCIE_AER_UNCORR_UNSUPRT_REQ_ERR		AL_BIT(20)
/** Uncorrectable Internal Error */
#define AL_PCIE_AER_UNCORR_INT_ERR			AL_BIT(22)
/** AtomicOp Egress Blocked */
#define AL_PCIE_AER_UNCORR_ATOMIC_EGRESS_BLK		AL_BIT(24)

/*******************************************************************************
 * PCIe AER correctable error bits
 * To be used with the following functions:
 * - al_pcie_aer_config
 * - al_pcie_aer_corr_get_and_clear
 ******************************************************************************/
/** Receiver Error */
#define AL_PCIE_AER_CORR_RCV_ERR			AL_BIT(0)
/** Bad TLP */
#define AL_PCIE_AER_CORR_BAD_TLP			AL_BIT(6)
/** Bad DLLP */
#define AL_PCIE_AER_CORR_BAD_DLLP			AL_BIT(7)
/** REPLAY_NUM Rollover */
#define AL_PCIE_AER_CORR_RPLY_NUM_ROLL_OVR		AL_BIT(8)
/** Replay Timer Timeout */
#define AL_PCIE_AER_CORR_RPLY_TMR_TO			AL_BIT(12)
/** Advisory Non-Fatal Error */
#define AL_PCIE_AER_CORR_ADVISORY_NON_FTL_ERR		AL_BIT(13)
/** Corrected Internal Error */
#define AL_PCIE_AER_CORR_INT_ERR			AL_BIT(14)

/** The AER erroneous TLP header length [num DWORDs] */
#define AL_PCIE_AER_ERR_TLP_HDR_NUM_DWORDS		4

/******************************************************************************/
/************************* Data Structures and Types **************************/
/******************************************************************************/

/**
 * al_pcie_ib_hcrd_config: data structure internally used in order to config
 * inbound posted/non-posted parameters.
 * Note: it's required to have this structure in pcie_port handle since it has
 *	 a state (required/not-required) which is determined by outbound
 *	 outstanding configuration
 */
struct al_pcie_ib_hcrd_config {
	/* Internally used - see 'al_pcie_ib_hcrd_os_ob_reads_config' */
	unsigned int	nof_np_hdr;

	/* Internally used - see 'al_pcie_ib_hcrd_os_ob_reads_config' */
	unsigned int	nof_p_hdr;
};

/* The Max Payload Size. Measured in bytes.
 *   DEFAULT: do not change the current MPS
 */
enum al_pcie_max_payload_size {
	AL_PCIE_MPS_DEFAULT,
	AL_PCIE_MPS_128		= 0,
	AL_PCIE_MPS_256		= 1,
	AL_PCIE_MPS_512		= 2,
	AL_PCIE_MPS_1024	= 3,
	AL_PCIE_MPS_2048	= 4,
	AL_PCIE_MPS_4096	= 5,
};

/**
 * al_pcie_port: data structure used by the HAL to handle a specific pcie port.
 * this structure is allocated and set to zeros by the upper layer, then it is
 * initialized by the al_pcie_port_handle_init() that should be called before any
 * other function of this API. later, this handle passed to the API functions.
 */
struct al_pcie_port {
	void __iomem		*pcie_reg_base;
	struct al_pcie_regs 	regs_ptrs;
	struct al_pcie_regs	*regs;
	uint32_t		*ex_regs_ptrs[AL_PCIE_EX_REGS_NUM];
	void			*ex_regs;
	void __iomem		*pbs_regs;

	/* Revision ID */
	uint8_t		rev_id;
	unsigned int	port_id;
	uint8_t		max_lanes;
	uint8_t		max_num_of_pfs;

	/* Internally used */
	struct al_pcie_ib_hcrd_config ib_hcrd_config;
};

/**
 * al_pcie_pf: the pf handle, a data structure used to handle PF specific
 * functionality. Initialized using "al_pcie_pf_handle_init()"
 */
struct al_pcie_pf {
	unsigned int		pf_num;
	struct al_pcie_port	*pcie_port;
};

/** Operating mode (endpoint, root complex) */
enum al_pcie_operating_mode {
	AL_PCIE_OPERATING_MODE_EP,
	AL_PCIE_OPERATING_MODE_RC,
	AL_PCIE_OPERATING_MODE_UNKNOWN
};

/* The maximum link speed, measured GT/s (Giga transfer / second)
 *   DEFAULT: do not change the current speed
 *   GEN1: 2.5 GT/s
 *   GEN2: 5 GT/s
 *   GEN3: 8GT/s
 *
 *   Note: The values of this enumerator are important for proper behavior
 */
enum al_pcie_link_speed {
	AL_PCIE_LINK_SPEED_DEFAULT,
	AL_PCIE_LINK_SPEED_GEN1 = 1,
	AL_PCIE_LINK_SPEED_GEN2 = 2,
	AL_PCIE_LINK_SPEED_GEN3 = 3
};

/** PCIe capabilities that supported by a specific port */
struct al_pcie_max_capability {
	al_bool		end_point_mode_supported;
	al_bool		root_complex_mode_supported;
	enum al_pcie_link_speed	max_speed;
	uint8_t		max_lanes;
	al_bool		reversal_supported;
	uint8_t		atu_regions_num;
	uint32_t	atu_min_size;
};

/** PCIe link related parameters */
struct al_pcie_link_params {
	enum al_pcie_link_speed	max_speed;
	al_bool			enable_reversal;
	enum al_pcie_max_payload_size	max_payload_size;

};

/** PCIe gen2 link parameters */
struct al_pcie_gen2_params {
	al_bool	tx_swing_low; /* set tx swing low when true, and tx swing full when false */
	al_bool	tx_compliance_receive_enable;
	al_bool	set_deemphasis;
};

/** PCIe gen 3 standard per lane equalization parameters */
struct al_pcie_gen3_lane_eq_params {
	uint8_t		downstream_port_transmitter_preset;
	uint8_t		downstream_port_receiver_preset_hint;
	uint8_t		upstream_port_transmitter_preset;
	uint8_t		upstream_port_receiver_preset_hint;
};

/** PCIe gen 3 equalization parameters */
struct al_pcie_gen3_params {
	al_bool	perform_eq;
	al_bool	interrupt_enable_on_link_eq_request;
	struct al_pcie_gen3_lane_eq_params *eq_params; /* array of lanes params */
	int	eq_params_elements; /* number of elements in the eq_params array */

	al_bool	eq_disable; /* disables the equalization feature */
	al_bool eq_phase2_3_disable; /* Equalization Phase 2 and Phase 3 */
				     /* Disable (RC mode only) */
	uint8_t local_lf; /* Full Swing (FS) Value for Gen3 Transmit Equalization */
			  /* Value Range: 12 through 63 (decimal).*/

	uint8_t	local_fs; /* Low Frequency (LF) Value for Gen3 Transmit Equalization */
};

/** Transport Layer credits parameters */
struct al_pcie_tl_credits_params {
};

/** Various configuration features */
struct al_pcie_features {
	/**
	 * Enable MSI fix from the SATA to the PCIe EP
	 * Only valid for port 0, when enabled as EP
	 */
	al_bool sata_ep_msi_fix;
};

/**
 * Inbound posted/non-posted header credits and outstanding outbound reads
 * completion header configuration
 *
 * Constraints:
 * - nof_cpl_hdr + nof_np_hdr + nof_p_hdr ==
 *			AL_PCIE_REV_1_2_IB_HCRD_SUM/AL_PCIE_REV3_IB_HCRD_SUM
 * - nof_cpl_hdr > 0
 * - nof_p_hdr > 0
 * - nof_np_hdr > 0
 */
struct al_pcie_ib_hcrd_os_ob_reads_config {
	/** Max number of outstanding outbound reads */
	uint8_t nof_outstanding_ob_reads;

	/**
	 * This value set the possible outstanding headers CMPLs , the core
	 * can get (the core always advertise infinite credits for CMPLs).
	 */
	unsigned int nof_cpl_hdr;

	/**
	 * This value set the possible outstanding headers reads (non-posted
	 * transactions), the core can get  (it set the value in the init FC
	 * process).
	 */
	unsigned int nof_np_hdr;

	/**
	 * This value set the possible outstanding headers writes (posted
	 * transactions), the core can get  (it set the value in the init FC
	 * process).
	 */
	unsigned int nof_p_hdr;
};

/** PCIe Ack/Nak Latency and Replay timers */
struct al_pcie_latency_replay_timers {
	uint16_t	round_trip_lat_limit;
	uint16_t	replay_timer_limit;
};

/* SRIS KP counter values */
struct al_pcie_sris_params {
	/** set to AL_TRUE to use defaults and ignore the other parameters */
	al_bool		use_defaults;
	uint16_t	kp_counter_gen3;	/* only for Gen3 */
	uint16_t	kp_counter_gen21;
};

/** Relaxed ordering params */
struct al_pcie_relaxed_ordering_params {
	al_bool		enable_tx_relaxed_ordering;
	al_bool		enable_rx_relaxed_ordering;
};

/** PCIe port configuration parameters
 * This structure includes the parameters that the HAL should apply to the port
 * (by al_pcie_port_config()).
 * The fields that are pointers (e.g. link_params) can be set to NULL, in that
 * case, the al_pcie_port_config() will keep the current HW settings.
 */
struct al_pcie_port_config_params {
	struct al_pcie_link_params		*link_params;
	al_bool					enable_axi_snoop;
	al_bool					enable_ram_parity_int;
	al_bool					enable_axi_parity_int;
	struct al_pcie_latency_replay_timers	*lat_rply_timers;
	struct al_pcie_gen2_params		*gen2_params;
	struct al_pcie_gen3_params		*gen3_params;
	struct al_pcie_tl_credits_params	*tl_credits;
	struct al_pcie_features			*features;
	/* Sets all internal timers to Fast Mode for speeding up simulation.*/
	al_bool					fast_link_mode;
	/*
	 * when true, the PCI unit will return Slave Error/Decoding Error to the master unit in case
	 * of error. when false, the value 0xFFFFFFFF will be returned without error indication.
	 */
	al_bool					enable_axi_slave_err_resp;
	struct al_pcie_sris_params		*sris_params;
	struct al_pcie_relaxed_ordering_params	*relaxed_ordering_params;
};

/** BAR register configuration parameters (Endpoint Mode only) */
struct al_pcie_ep_bar_params {
	al_bool		enable;
	al_bool		memory_space; /**< memory or io */
	al_bool		memory_64_bit; /**< is memory space is 64 bit */
	al_bool		memory_is_prefetchable;
	uint64_t	size; /* the bar size in bytes */
};

/** PF config params (EP mode only) */
struct al_pcie_pf_config_params {
	al_bool				cap_d1_d3hot_dis;
	al_bool				cap_flr_dis;
	al_bool				cap_aspm_dis;
	al_bool				bar_params_valid;
	struct al_pcie_ep_bar_params	bar_params[6];
	struct al_pcie_ep_bar_params	exp_bar_params;/* expansion ROM BAR*/
};

/** PCIe link status */
struct al_pcie_link_status {
	al_bool			link_up;
	enum al_pcie_link_speed	speed;
	uint8_t			lanes;
	uint8_t			ltssm_state;
};

/** PCIe lane status */
struct al_pcie_lane_status {
	al_bool			is_reset;
	enum al_pcie_link_speed	requested_speed;
};

/** PCIe MSIX capability configuration parameters */
struct al_pcie_msix_params {
	uint16_t	table_size;
	uint16_t	table_offset;
	uint8_t		table_bar;
	uint16_t	pba_offset;
	uint16_t	pba_bar;
};

/** PCIE AER capability parameters */
struct al_pcie_aer_params {
	/** ECRC Generation Enable */
	al_bool		ecrc_gen_en;
	/** ECRC Check Enable */
	al_bool		ecrc_chk_en;

	/**
	 * Enabled reporting of correctable errors (bit mask)
	 * See 'AL_PCIE_AER_CORR_*' for details
	 * 0 - no reporting at all
	 */
	unsigned int	enabled_corr_err;
	/**
	 * Enabled reporting of non-fatal uncorrectable errors (bit mask)
	 * See 'AL_PCIE_AER_UNCORR_*' for details
	 * 0 - no reporting at all
	 */
	unsigned int	enabled_uncorr_non_fatal_err;
	/**
	 * Enabled reporting of fatal uncorrectable errors (bit mask)
	 * See 'AL_PCIE_AER_UNCORR_*' for details
	 * 0 - no reporting at all
	 */
	unsigned int	enabled_uncorr_fatal_err;
};

/******************************************************************************/
/********************************** PCIe API **********************************/
/******************************************************************************/

/*************************** PCIe Initialization API **************************/

/**
 * Initializes a PCIe port handle structure.
 *
 * @param   pcie_port		an allocated, non-initialized instance.
 * @param   pcie_reg_base	the virtual base address of the port internal
 *				registers
 * @param   pbs_reg_base	the virtual base address of the pbs functional
 *				registers
 * @param   port_id		the port id (used mainly for debug messages)
 *
 * @return 0 if no error found.
 */
int al_pcie_port_handle_init(struct al_pcie_port *pcie_port,
			 void __iomem *pcie_reg_base,
			 void __iomem *pbs_reg_base,
			 unsigned int port_id);

/**
 * Initializes a PCIe pf handle structure
 * @param  pcie_pf   an allocated, non-initialized instance of pf handle
 * @param  pcie_port pcie port handle
 * @param  pf_num    physical function number
 * @return           0 if no error found
 */
int al_pcie_pf_handle_init(
	struct al_pcie_pf *pcie_pf,
	struct al_pcie_port *pcie_port,
	unsigned int pf_num);

/************************** Pre PCIe Port Enable API **************************/

/**
 * @brief set current pcie operating mode (root complex or endpoint)
 * This function can be called only before enabling the controller using
 * al_pcie_port_enable().
 *
 * @param pcie_port pcie port handle
 * @param mode pcie operating mode
 *
 * @return 0 if no error found.
 */
int al_pcie_port_operating_mode_config(struct al_pcie_port *pcie_port,
				  enum al_pcie_operating_mode mode);

/**
 * Configure number of lanes connected to this port.
 * This function can be called only before enabling the controller using al_pcie_port_enable().
 *
 * @param pcie_port pcie port handle
 * @param lanes number of lanes
 * Note: this function must be called before any al_pcie_port_config() calls
 *
 * @return 0 if no error found.
 */
int al_pcie_port_max_lanes_set(struct al_pcie_port *pcie_port, uint8_t lanes);

/**
 * Set maximum physical function numbers
 * @param pcie_port      pcie port handle
 * @param max_num_of_pfs number of physical functions
 * Note: this function must be called before any al_pcie_pf_config() calls
 */
int al_pcie_port_max_num_of_pfs_set(
	struct al_pcie_port *pcie_port,
	uint8_t max_num_of_pfs);

/**
 * @brief Inbound posted/non-posted header credits and outstanding outbound
 *        reads completion header configuration
 *
 * @param	pcie_port pcie port handle
 * @param	ib_hcrd_os_ob_reads_config
 * 		Inbound header credits and outstanding outbound reads
 * 		configuration
 */
int al_pcie_port_ib_hcrd_os_ob_reads_config(
	struct al_pcie_port *pcie_port,
	struct al_pcie_ib_hcrd_os_ob_reads_config *ib_hcrd_os_ob_reads_config);

/** return PCIe operating mode
 * @param pcie_port	pcie port handle
 * @return		operating mode
 */
enum al_pcie_operating_mode al_pcie_operating_mode_get(
	struct al_pcie_port *pcie_port);

/**************************** PCIe Port Enable API ****************************/

/** Enable PCIe unit (deassert reset)
 *
 * @param   pcie_port pcie port handle
 *
 * @return 0 if no error found.
 */
int al_pcie_port_enable(struct al_pcie_port *pcie_port);

/** Disable PCIe unit (assert reset)
 *
 * @param   pcie_port pcie port handle
 */
void al_pcie_port_disable(struct al_pcie_port *pcie_port);

/**
 * Port memory shutdown/up
 * Caution: This function can be called only when the controller is disabled
 *
 * @param pcie_port pcie port handle
 * @param enable memory shutdown enable or disable
 *
 */
int al_pcie_port_memory_shutdown_set(
	struct al_pcie_port	*pcie_port,
	al_bool			enable);

/**
 * Check if port enabled or not
 * @param  pcie_port pcie port handle
 * @return           AL_TRUE of port enabled and AL_FALSE otherwise
 */
al_bool al_pcie_port_is_enabled(struct al_pcie_port *pcie_port);

/*************************** PCIe Configuration API ***************************/

/**
 * @brief   configure pcie port (mode, link params, etc..)
 * this function must be called before initializing the link
 *
 * @param pcie_port pcie port handle
 * @param params configuration structure.
 *
 * @return  0 if no error found
 */
int al_pcie_port_config(struct al_pcie_port *pcie_port,
			const struct al_pcie_port_config_params *params);

/**
 * @brief Configure a specific PF (EP params, sriov params, ...)
 * this function must be called before any datapath transactions
 *
 * @param pcie_pf	pcie pf handle
 * @param params	configuration structure.
 *
 * @return		0 if no error found
 */
int al_pcie_pf_config(
	struct al_pcie_pf *pcie_pf,
	const struct al_pcie_pf_config_params *params);

/************************** PCIe Link Operations API **************************/

/**
 * @brief   start pcie link
 *
 * @param   pcie_port pcie port handle
 *
 * @return  0 if no error found
 */
int al_pcie_link_start(struct al_pcie_port *pcie_port);

/**
 * @brief   stop pcie link
 *
 * @param   pcie_port pcie port handle
 *
 * @return  0 if no error found
 */
int al_pcie_link_stop(struct al_pcie_port *pcie_port);

/**
 * @brief   trigger link-disable
 *
 * @param   pcie_port pcie port handle
 * @param   disable   AL_TRUE to disable the link and AL_FALSE to enable it
 *
 * Note: this functionality differs from "al_pcie_link_stop" as it's a spec
 *       functionality where both sides of the PCIe agrees to disable the link
 * @return  0 if no error found
 */
int al_pcie_link_disable(struct al_pcie_port *pcie_port, al_bool disable);

/**
 * @brief   wait for link up indication
 * this function waits for link up indication, it polls LTSSM state until link is ready
 *
 * @param   pcie_port pcie port handle
 * @param   timeout_ms maximum timeout in milli-seconds to wait for link up
 *
 * @return  0 if link up indication detected
 * 	    -ETIME if not.
 */
int al_pcie_link_up_wait(struct al_pcie_port *pcie_port, uint32_t timeout_ms);

/**
 * @brief   get link status
 *
 * @param   pcie_port pcie port handle
 * @param   status structure for link status
 *
 * @return  0 if no error found
 */
int al_pcie_link_status(struct al_pcie_port *pcie_port, struct al_pcie_link_status *status);

/**
 * @brief   get lane status
 *
 * @param	pcie_port
 *		pcie port handle
 * @param	lane
 *		PCIe lane
 * @param	status
 *		Pointer to returned structure for lane status
 *
 */
void al_pcie_lane_status_get(
	struct al_pcie_port		*pcie_port,
	unsigned int			lane,
	struct al_pcie_lane_status	*status);

/**
 * @brief   trigger hot reset
 *
 * @param   pcie_port pcie port handle
 * @param   enable   AL_TRUE to enable hot-reset and AL_FALSE to disable it
 *
 * @return  0 if no error found
 */
int al_pcie_link_hot_reset(struct al_pcie_port *pcie_port, al_bool enable);

/**
 * @brief   trigger link-retain
 * this function initiates Link retraining by directing the Physical Layer LTSSM
 * to the Recovery state. If the LTSSM is already in Recovery or Configuration,
 * re-entering Recovery is permitted but not required.

 * @param   pcie_port pcie port handle
 *
 * Note: there's no need to disable initiating link-retrain
 * @return  0 if no error found
 */
int al_pcie_link_retrain(struct al_pcie_port *pcie_port);

/**
 * @brief   change port speed
 * this function changes the port speed, it doesn't wait for link re-establishment
 *
 * @param   pcie_port pcie port handle
 * @param   new_speed the new speed gen to set
 *
 * @return  0 if no error found
 */
int al_pcie_link_change_speed(struct al_pcie_port *pcie_port, enum al_pcie_link_speed new_speed);

/* TODO: check if this function needed */
int al_pcie_link_change_width(struct al_pcie_port *pcie_port, uint8_t width);

/**************************** Post Link Start API *****************************/

/************************** Snoop Configuration API ***************************/

/**
 * @brief   configure pcie port axi snoop
 *
 * @param pcie_port pcie port handle
 * @param enable_axi_snoop enable snoop.
 *
 * @return  0 if no error found
 */
/* TODO: Can this API be called after port enable? */
int al_pcie_port_snoop_config(struct al_pcie_port *pcie_port,
				al_bool enable_axi_snoop);

/************************** Configuration Space API ***************************/

/**
 * Configuration Space Access Through PCI-E_ECAM_Ext PASW (RC mode only)
 */

/**
 * @brief   get base address of pci configuration space header
 * @param   pcie_pf	pcie pf handle
 * @param   addr	pointer for returned address;
 * @return              0 if no error found
 */
int al_pcie_config_space_get(
	struct al_pcie_pf *pcie_pf,
	uint8_t __iomem **addr);

/**
 * Read data from the local configuration space
 *
 * @param	pcie_pf	pcie	pf handle
 * @param	reg_offset	Configuration space register offset
 * @return	Read data
 */
uint32_t al_pcie_local_cfg_space_read(
	struct al_pcie_pf	*pcie_pf,
	unsigned int		reg_offset);

/**
 * Write data to the local configuration space
 *
 * @param	pcie_pf		PCIe pf handle
 * @param	reg_offset	Configuration space register offset
 * @param	data		Data to write
 * @param	cs2		Should be AL_TRUE if dbi_cs2 must be asserted
 *				to enable writing to this register, according to
 *				the PCIe Core specifications
 * @param	allow_ro_wr	AL_TRUE to allow writing into read-only regs
 *
 */
void al_pcie_local_cfg_space_write(
	struct al_pcie_pf	*pcie_pf,
	unsigned int		reg_offset,
	uint32_t		data,
	al_bool			cs2,
	al_bool			allow_ro_wr);

/**
 * @brief   set target_bus and mask_target_bus
 * @param   pcie_port pcie port handle
 * @param   target_bus
 * @param   mask_target_bus
 * @return  0 if no error found
 */
int al_pcie_target_bus_set(struct al_pcie_port *pcie_port,
			   uint8_t target_bus,
			   uint8_t mask_target_bus);

/**
 * @brief   get target_bus and mask_target_bus
 * @param   pcie_port pcie port handle
 * @param   target_bus
 * @param   mask_target_bus
 * @return  0 if no error found
 */
int al_pcie_target_bus_get(struct al_pcie_port *pcie_port,
			   uint8_t *target_bus,
			   uint8_t *mask_target_bus);

/**
 * Set secondary bus number
 *
 * @param pcie_port pcie port handle
 * @param secbus pci secondary bus number
 *
 * @return 0 if no error found.
 */
int al_pcie_secondary_bus_set(struct al_pcie_port *pcie_port, uint8_t secbus);

/**
 * Set subordinary bus number
 *
 * @param   pcie_port pcie port handle
 * @param   subbus the highest bus number of all of the buses that can be reached
 *		downstream of the PCIE instance.
 *
 * @return 0 if no error found.
 */
int al_pcie_subordinary_bus_set(struct al_pcie_port *pcie_port,uint8_t subbus);

/**
 * @brief Enable/disable deferring incoming configuration requests until
 * initialization is complete. When enabled, the core completes incoming
 * configuration requests with a Configuration Request Retry Status.
 * Other incoming Requests complete with Unsupported Request status.
 *
 * @param pcie_port pcie port handle
 * @param en enable/disable
 */
void al_pcie_app_req_retry_set(struct al_pcie_port *pcie_port, al_bool en);

/*************** Internal Address Translation Unit (ATU) API ******************/

enum al_pcie_atu_dir {
	AL_PCIE_ATU_DIR_OUTBOUND = 0,
	AL_PCIE_ATU_DIR_INBOUND = 1,
};

enum al_pcie_atu_tlp {
	AL_PCIE_TLP_TYPE_MEM = 0,
	AL_PCIE_TLP_TYPE_IO = 2,
	AL_PCIE_TLP_TYPE_CFG0 = 4,
	AL_PCIE_TLP_TYPE_CFG1 = 5,
	AL_PCIE_TLP_TYPE_MSG = 0x10,
	AL_PCIE_TLP_TYPE_RESERVED = 0x1f
};

enum al_pcie_atu_response {
	AL_PCIE_RESPONSE_NORMAL = 0,
	AL_PCIE_RESPONSE_UR = 1,
	AL_PCIE_RESPONSE_CA = 2
};

struct al_pcie_atu_region {
	al_bool			enable;
	/* outbound or inbound */
	enum al_pcie_atu_dir	direction;
	/* region index */
	uint8_t			index;
	uint64_t		base_addr;
	/** limit marks the region's end address. only bits [39:0] are valid
	 * given the Alpine PoC maximum physical address space
	 */
	uint64_t		limit;
	/** the address that matches will be translated to this address + offset
	 */
	uint64_t		target_addr;
	al_bool			invert_matching;
	/* pcie tlp type*/
	enum al_pcie_atu_tlp	tlp_type;
	/* pcie frame header attr field*/
	uint8_t			attr;
	/**
	 * outbound specific params
	 */
	/* pcie message code */
	uint8_t			msg_code;
	al_bool			cfg_shift_mode;
	/**
	 * inbound specific params
	 */
	uint8_t			bar_number;
	/* BAR match mode, used in EP for MEM and IO tlps*/
	uint8_t			match_mode;
	/**
	 * For outbound: enables taking the function number of the translated
	 * TLP from the PCIe core. For inbound: enables ATU function match mode
	 * Note: this boolean is ignored in RC mode
	 */
	al_bool			function_match_bypass_mode;
	/**
	 * The function number to match/bypass (see previous parameter)
	 * Note: this parameter is ignored when previous param is FALSE
	 */
	uint8_t			function_match_bypass_mode_number;
	/* response code */
	enum al_pcie_atu_response response;
	al_bool			enable_attr_match_mode;
	al_bool			enable_msg_match_mode;
	/**
	 * USE WITH CAUTION: setting this boolean to AL_TRUE allows setting the
	 * outbound ATU even after link is already started. DO NOT SET this
	 * boolean to AL_TRUE unless there have been NO traffic before calling
	 * al_pcie_atu_region_set function
	 */
	al_bool			enforce_ob_atu_region_set;
};

/**
 * @brief   program internal ATU region entry
 * @param   pcie_port	pcie port handle
 * @param   atu_region	data structure that contains the region index and the
 *          translation parameters
 * @return  0 if no error
 */
int al_pcie_atu_region_set(
	struct al_pcie_port *pcie_port,
	struct al_pcie_atu_region *atu_region);

/**
 * @brief  get internal ATU is enabled and base/target addresses
 * @param  pcie_port   pcie port handle
 * @param  direction   input: iATU direction (IB/OB)
 * @param  index       input: iATU index
 * @param  enable      output: AL_TRUE if the iATU is enabled
 * @param  base_addr   output: the iATU base address
 * @param  target_addr output: the iATU target address
 */
void al_pcie_atu_region_get_fields(
	struct al_pcie_port *pcie_port,
	enum al_pcie_atu_dir direction, uint8_t index,
	al_bool *enable, uint64_t *base_addr, uint64_t *target_addr);

/**
 * @brief   Configure axi io bar.
 *          every hit to this bar will override size to 4 bytes.
 * @param   pcie_port pcie port handle
 * @param   start the first address of the memory
 * @param   end the last address of the memory
 * @return
 */
void al_pcie_axi_io_config(
	struct al_pcie_port *pcie_port,
	al_phys_addr_t start,
	al_phys_addr_t end);

/************** Interrupt generation (Endpoint mode Only) API *****************/

enum al_pcie_legacy_int_type{
	AL_PCIE_LEGACY_INTA = 0,
	AL_PCIE_LEGACY_INTB,
	AL_PCIE_LEGACY_INTC,
	AL_PCIE_LEGACY_INTD
};

/**
 * @brief		generate INTx Assert/DeAssert Message
 * @param  pcie_pf	pcie pf handle
 * @param  assert	when true, Assert Message is sent
 * @param  type		type of message (INTA, INTB, etc)
 * @return		0 if no error found
 */
int al_pcie_legacy_int_gen(
	struct al_pcie_pf		*pcie_pf,
	al_bool				assert,
	enum al_pcie_legacy_int_type	type);

/**
 * @brief		generate MSI interrupt
 * @param  pcie_pf	pcie pf handle
 * @param  vector	the vector index to send interrupt for.
 * @return		0 if no error found
 */
int al_pcie_msi_int_gen(struct al_pcie_pf *pcie_pf, uint8_t vector);

/**
 * @brief   configure MSIX capability
 * @param   pcie_pf	pcie pf handle
 * @param   msix_params	MSIX capability configuration parameters
 * @return  0 if no error found
 */
int al_pcie_msix_config(
		struct al_pcie_pf		*pcie_pf,
		struct al_pcie_msix_params	*msix_params);

/**
 * @brief   check whether MSIX capability is enabled
 * @param   pcie_pf	pcie pf handle
 * @return  AL_TRUE if MSIX capability is enabled, AL_FALSE otherwise
 */
al_bool al_pcie_msix_enabled(struct al_pcie_pf	*pcie_pf);

/**
 * @brief   check whether MSIX capability is masked
 * @param   pcie_pf	pcie pf handle
 * @return  AL_TRUE if MSIX capability is masked, AL_FALSE otherwise
 */
al_bool al_pcie_msix_masked(struct al_pcie_pf *pcie_pf);

/******************** Advanced Error Reporting (AER) API **********************/

/**
 * @brief   configure AER capability
 * @param   pcie_pf	pcie pf handle
 * @param   params	AER capability configuration parameters
 * @return  0 if no error found
 */
int al_pcie_aer_config(
	struct al_pcie_pf		*pcie_pf,
	struct al_pcie_aer_params	*params);

/**
 * @brief   AER uncorretable errors get and clear
 * @param   pcie_pf	pcie pf handle
 * @return  bit mask of uncorrectable errors - see 'AL_PCIE_AER_UNCORR_*' for
 *          details
 */
unsigned int al_pcie_aer_uncorr_get_and_clear(struct al_pcie_pf	*pcie_pf);

/**
 * @brief   AER corretable errors get and clear
 * @param   pcie_pf	pcie pf handle
 * @return  bit mask of correctable errors - see 'AL_PCIE_AER_CORR_*' for
 *          details
 */
unsigned int al_pcie_aer_corr_get_and_clear(struct al_pcie_pf	*pcie_pf);

/**
 * @brief   AER get the header for the TLP corresponding to a detected error
 * @param   pcie_pf	pcie pf handle
 * @param   hdr		pointer to an array for getting the header
 */
void al_pcie_aer_err_tlp_hdr_get(
	struct al_pcie_pf	*pcie_pf,
	uint32_t		hdr[AL_PCIE_AER_ERR_TLP_HDR_NUM_DWORDS]);

/******************** Loop-Back mode (RC and Endpoint modes) ******************/

/**
 * @brief   enter local pipe loop-back mode
 *  This mode will connect the pipe RX signals to TX.
 *  no need to start link when using this mode.
 *  Gen3 equalization must be disabled before enabling this mode
 *  The caller must make sure the port is ready to accept the TLPs it sends to
 *  itself. for example, BARs should be initialized before sending memory TLPs.
 *
 * @param   pcie_port pcie port handle
 * @return  0 if no error found
 */
int al_pcie_local_pipe_loopback_enter(struct al_pcie_port *pcie_port);

/**
 * @brief   exit local pipe loopback mode
 *
 * @param   pcie_port pcie port handle
 * @return  0 if no error found
 */
int al_pcie_local_pipe_loopback_exit(struct al_pcie_port *pcie_port);

/**
 * @brief   enter master remote loopback mode
 *  No need to configure the link partner to enter slave remote loopback mode
 *  as this should be done as response to special training sequence directives
 *  when master works in remote loopback mode.
 *  The caller must make sure the port is ready to accept the TLPs it sends to
 *  itself. for example, BARs should be initialized before sending memory TLPs.
 *
 * @param   pcie_port pcie port handle
 * @return  0 if no error found
 */
int al_pcie_remote_loopback_enter(struct al_pcie_port *pcie_port);

/**
 * @brief   exit remote loopback mode
 *
 * @param   pcie_port pcie port handle
 * @return  0 if no error found
 */
int al_pcie_remote_loopback_exit(struct al_pcie_port *pcie_port);

#endif
/** @} end of grouppcie group */
