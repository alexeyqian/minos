#include "hd.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "assert.h"
#include "klib.h"
#include "kio.h"
#include "ipc.h"

PRIVATE	uint8_t	hd_status;
PRIVATE	uint8_t	hdbuf[SECTOR_SIZE * 2];

PRIVATE void hd_handler(){
    hd_status = in_byte(REG_STATUS);
    inform_int(TASK_HD);
}

PRIVATE void init_hd(){
    // get the numbe of drives from the BIOS data area
    uint8_t *p_nr_drives = (uint8_t*)(0x475);
    printf("number of drives: %d. \n", *p_nr_drives);
    assert(*p_nr_drives);

    put_irq_handler(AT_WINI_IRQ, hd_handler);
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
}

// <ring 1> wait for a certain status
// return 1 if success, zero if timeout
PRIVATE int waitfor(int mask, int val, int timeout){
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / HZ) < timeout)
		if ((in_byte(REG_STATUS) & mask) == val)
			return 1;

	return 0;
}

PRIVATE void hd_cmd_out(struct hd_cmd* cmd){
    // for all commands, the host must first check if BSY=1
    // and should proceed no further unless and until BSY = 0
    if(!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
        panic("hd error.");

    /* Activate the Interrupt Enable (nIEN) bit */
	out_byte(REG_DEV_CTRL, 0);
	/* Load required parameters in the Command Block Registers */
	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NSECTOR,  cmd->count);
	out_byte(REG_LBA_LOW,  cmd->lba_low);
	out_byte(REG_LBA_MID,  cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE,   cmd->device);
	/* Write the command code to the Command Register */
	out_byte(REG_CMD,     cmd->command);
}

// <ring 1> wait until a disk interrupt occurs
PRIVATE void interrupt_wait(){
    MESSAGE msg;
    send_recv(RECEIVE, INTERRUPT, &msg);
}

// <ring 1>
PRIVATE void print_identify_info(uint16_t* hdinfo){
    printf(">>> print identify info\n");
    int i, k;
	char s[64];

	struct iden_info_ascii {
		int idx;
		int len;
		char * desc;
	} iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
		     {27, 40, "HD Model"} /* Model number in ASCII */ };

	for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
		char * p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len/2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printl("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printl("LBA supported: %s\n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printl("LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printl("HD size: %dMB\n", sectors * 512 / 1000000);
}

// <ring 0> get the disk information
PRIVATE void hd_identify(int drive){
    printf(">>> in hd_identify () \n");
    struct hd_cmd cmd;
    cmd.device = MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;
    hd_cmd_out(&cmd);
    interrupt_wait();
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    print_identify_info((uint16_t*)hdbuf);
}

PUBLIC void task_hd(){
    MESSAGE msg;
    init_hd();
    while(1){
        send_recv(RECEIVE, ANY, &msg);
        printf(">>> message receive passed.");
        int src = msg.source;
        switch(msg.type){
            case DEV_OPEN:
                printf("receive message DEV_OPEN");
                hd_identify(0);
                break;
            default:
                dump_msg("hd driver: unknown msg", &msg);
                spin("fs: main loop invalid message type.");
                break;
        }

        send_recv(SEND, src, &msg);
    }
}
