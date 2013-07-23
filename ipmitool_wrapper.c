#include <stdlib.h>

#include "ipmitool_wrapper.h"
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/log.h>
#include "global.h"

#define TARGET_CHANNEL 0//7
#define TRANSIT_ADDRESS 0//0x82
#define TRANSIT_CHANNEL 0
#define IPMI_BMC_SLAVE_ADDR		0x20

extern struct ipmi_intf ipmi_lan_intf;

int verbose = 0;
int csv_output = 0;

static struct ipmi_intf * intf = NULL;

/** Function initializes connection with MCH using ipmitool. 
* It sets LAN interface, opens session and sets IPMI address of target device.
* \param hostname - IP address of MCH
* \param slot_number - number of slot in which the board for programming is placed
* \return
- 0 - when initialization was successful
- -1 - in case of some errors (e.g. error during interface opening)
*/
int ipmi_lib_init(char* hostname, uint8_t slot_number)
{
	uint8_t target_addr = 0x70+2*slot_number;
	uint8_t target_channel = TARGET_CHANNEL;
	uint8_t transit_addr = TRANSIT_ADDRESS;
	uint8_t transit_channel = TRANSIT_CHANNEL;
	uint8_t target_lun     = 0;
	uint8_t my_addr = 0;
	
	int devnum = 0;
	
	//use LAN interface
	intf = &ipmi_lan_intf;
	if(intf->setup(intf) != 0)
	{
		return -1;
	}
	
	/* setup log */
	log_init("PROG_TEST", 0, verbose);

	ipmi_intf_session_set_hostname(intf, hostname);

	/* setup destination lun if given */
	intf->target_lun = target_lun ;

	/* setup destination channel if given */
	intf->target_channel = target_channel ;

	intf->devnum = devnum;

	/* setup IPMB local and target address if given */
	intf->my_addr = my_addr ? : IPMI_BMC_SLAVE_ADDR;
	if (target_addr > 0) {
		/* need to open the interface first */
		if (intf->open != NULL)
		{
			if(intf->open(intf) == -1)
				return -1;
		}
		
		intf->target_addr = target_addr;

      if (transit_addr > 0) {
         intf->transit_addr    = transit_addr;
         intf->transit_channel = transit_channel;
      }
      else
      {
         intf->transit_addr = intf->my_addr;
      }
	}
	
	return 0;
}

/** Function sends IPMI command which selects memory/revision
 * \param board_type - type of board (uTC, SIS8300 or DAMC2)
 * \param memory_number - number of memory which will be selected
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_set_PROM_number_cmd(uint8_t board_type, uint8_t memory_number)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	
	ipmi_intf_session_set_timeout(intf, 15);
	ipmi_intf_session_set_retry(intf, 1);
	
	lun = intf->target_lun;
	netfn = 0x30;
	if(board_type == uTC || board_type == DAMC2)
	{
		cmd = 0x01;
	}
	else if(board_type == SIS8300)
	{
		cmd = 0x03;
	}
	
	memset(data, 0, sizeof(data));
	memset(&req, 0, sizeof(req));
	req.msg.netfn = netfn;
	req.msg.lun = lun;
	req.msg.cmd = cmd;
	req.msg.data = data;
	
	req.msg.data[0] = memory_number;
	req.msg.data_len++;
	
	printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
		return -1; 
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
			val2str(rsp->ccode, completion_code_vals));
		return -1;
	} 
	
	lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);

#ifdef DEBUG
	/* print the raw response buffer */
	for (i=0; i<rsp->data_len; i++) {
		if (((i%16) == 0) && (i != 0))
			printf("\n");
		printf(" %2.2x", rsp->data[i]);
	}
	printf("\n");
#endif
	
	return 0;
}

/** Function sends IPMI command which gets the number of currently selected
 * memory (supported only by uTC board!!!)
 * \param board_type - type of board (uTC, SIS8300 or DAMC2)
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_get_PROM_number_cmd(uint8_t* memory_number)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	
	ipmi_intf_session_set_timeout(intf, 15);
	ipmi_intf_session_set_retry(intf, 1);
	
	lun = intf->target_lun;
	netfn = 0x30;
	cmd = 0x00;
	
	memset(data, 0, sizeof(data));
	memset(&req, 0, sizeof(req));
	req.msg.netfn = netfn;
	req.msg.lun = lun;
	req.msg.cmd = cmd;
	req.msg.data = data;
	
	printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
		return -1;
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
			val2str(rsp->ccode, completion_code_vals));
		return -1;
	} 
	
	lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);
	
	*memory_number = rsp->data[0];

#ifdef DEBUG
	/* print the raw response buffer */
	for (i=0; i<rsp->data_len; i++) {
		if (((i%16) == 0) && (i != 0))
			printf("\n");
		printf(" %2.2x", rsp->data[i]);
	}
	printf("\n");
#endif

	return 0;
}

/** Function sends IPMI command which writes value to the CPLD register
 * (supported only by uTC board!!!)
 * \param reg_addr - address of the internal CPLD register
 * \param value - value to be written
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_write_reg_cmd(uint8_t reg_addr, uint8_t value)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	uint8_t data_len = 0;
	
	ipmi_intf_session_set_timeout(intf, 15);
	ipmi_intf_session_set_retry(intf, 1);
	
	lun = intf->target_lun;
	netfn = 0x30;
	cmd = 0x03;
	
	memset(data, 0, sizeof(data));
	memset(&req, 0, sizeof(req));
	req.msg.netfn = netfn;
	req.msg.lun = lun;
	req.msg.cmd = cmd;
	req.msg.data = data;
	
	req.msg.data[data_len++] = reg_addr;
	req.msg.data[data_len++] = value;
	req.msg.data_len = data_len;
	
	printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
		return -1; 
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
			val2str(rsp->ccode, completion_code_vals));
		return -1;
	} 
	
	lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);

#ifdef DEBUG
	/* print the raw response buffer */
	for (i=0; i<rsp->data_len; i++) {
		if (((i%16) == 0) && (i != 0))
			printf("\n");
		printf(" %2.2x", rsp->data[i]);
	}
	printf("\n");
#endif

	return 0;
}

/** Function sends IPMI command which reads value from the CPLD register
 * (supported only by uTC board!!!)
 * \param reg_addr - address of the internal CPLD register
 * \param value - value to be written
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_read_reg_cmd(uint8_t reg_addr, uint8_t *value)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	uint8_t data_len = 0;
	
	ipmi_intf_session_set_timeout(intf, 15);
	ipmi_intf_session_set_retry(intf, 1);
	
	lun = intf->target_lun;
	netfn = 0x30;
	cmd = 0x02;
	
	memset(data, 0, sizeof(data));
	memset(&req, 0, sizeof(req));
	req.msg.netfn = netfn;
	req.msg.lun = lun;
	req.msg.cmd = cmd;
	req.msg.data = data;
	
	req.msg.data[data_len++] = reg_addr;
	req.msg.data_len = data_len;
	
	printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
		return -1; 
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
			val2str(rsp->ccode, completion_code_vals));
		return -1;
	} 
	
	lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);
	
	*value = rsp->data[0];
	
#ifdef DEBUG
	/* print the raw response buffer */
	for (i=0; i<rsp->data_len; i++) {
		if (((i%16) == 0) && (i != 0))
			printf("\n");
		printf(" %2.2x", rsp->data[i]);
	}
	printf("\n");
#endif

	return 0;
}

/** Function configures JTAG chain for memory programming (supported by DAMC2 board!!!)
 * \param board_type - type of board (uTC, SIS8300 or DAMC2)
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_configure_JTAG_chain(uint8_t board_type)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	uint8_t data_len = 0;
	
	if(board_type == DAMC2)
	{
		ipmi_intf_session_set_timeout(intf, 15);
		ipmi_intf_session_set_retry(intf, 1);
		
		lun = intf->target_lun;
		netfn = 0x30;
		cmd = 0x03;		//self-defined command
		
		memset(data, 0, sizeof(data));
		memset(&req, 0, sizeof(req));
		
		data[data_len++] = 0x01;
		
		req.msg.netfn = netfn;
		req.msg.lun = lun;
		req.msg.cmd = cmd;
		req.msg.data = data;
		req.msg.data_len = data_len;
		
		printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
		
		rsp = intf->sendrecv(intf, &req);
		
		if (rsp == NULL) {
			lprintf(LOG_ERR, "Unable to send RAW command "
				"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
				intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
			return -1;
		}
		if (rsp->ccode > 0) {
			lprintf(LOG_ERR, "Unable to send RAW command "
				"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
				intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
				val2str(rsp->ccode, completion_code_vals));
			return -1;
		} 
		
		lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);
		
#ifdef DEBUG
		/* print the raw response buffer */
		for (i=0; i<rsp->data_len; i++) {
			if (((i%16) == 0) && (i != 0))
				printf("\n");
			printf(" %2.2x", rsp->data[i]);
		}
		printf("\n");
#endif
		
		return 0;
	}
	else
	{
		fprintf(stderr, "ipmi_configure_JTAG_chain command is supported only by DAMC2\n\n");
		return -1;
	}	
}

/** Function sends IPMI command which causes FPGA reboot
 * \param memory_number - pointer to a memory space where the memory number is to be stored
 * \return
 - 0 - if command was succesfully sent and acknowledged
 - -1 - otherwise (e.g. when command was not acknowledged)
*/
int ipmi_fru_reset(uint8_t board_type)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;
	uint8_t netfn, cmd, lun;
	int i;
	uint8_t data[32];
	uint8_t data_len = 0;
	
	ipmi_intf_session_set_timeout(intf, 15);
	ipmi_intf_session_set_retry(intf, 1);
	
	lun = intf->target_lun;
	if(board_type == uTC)	
	{
		netfn = 0x30;
		cmd = 0x04;		//self-defined command
	}
	else if(board_type == SIS8300 || board_type == DAMC2) 
	{
		netfn = 0x2C;
		cmd = 0x04;		//PICMG frucontrol
	}
	
	memset(data, 0, sizeof(data));
	memset(&req, 0, sizeof(req));
	
	if(board_type == SIS8300 || board_type == DAMC2)		//data for PICMG frucontrol command
	{
		data[data_len++] = 0x00; //PICMG identifier
		data[data_len++] = 0x00; //FRU Device ID
		data[data_len++] = 0x00; //Cold Reset
	}	
	
	req.msg.netfn = netfn;
	req.msg.lun = lun;
	req.msg.cmd = cmd;
	req.msg.data = data;
	req.msg.data_len = data_len;
	
	printbuf(req.msg.data, req.msg.data_len, "RAW REQUEST");
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x)",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd);
		return -1;
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Unable to send RAW command "
			"(channel=0x%x netfn=0x%x lun=0x%x cmd=0x%x rsp=0x%x): %s",
			intf->target_channel & 0x0f, req.msg.netfn, req.msg.lun, req.msg.cmd, rsp->ccode,
			val2str(rsp->ccode, completion_code_vals));
		return -1;
	} 
	
	lprintf(LOG_INFO, "RAW RSP (%d bytes)", rsp->data_len);

#ifdef DEBUG
	/* print the raw response buffer */
	for (i=0; i<rsp->data_len; i++) {
		if (((i%16) == 0) && (i != 0))
			printf("\n");
		printf(" %2.2x", rsp->data[i]);
	}
	printf("\n");
#endif

	return 0;
}

/** Function closes connection with MCH */
void ipmi_lib_close()
{
	/* clean repository caches */
	ipmi_cleanup(intf);

	/* call interface close function if available */
	if (intf->opened > 0 && intf->close != NULL)
		intf->close(intf);
		
	log_halt();
}
