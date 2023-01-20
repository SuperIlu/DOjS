/*
 * DZcomm : a serial port API.
 * file : comm.c
 *
 * The core code for dzcomm, links to system specific low level
 * code as necessary
 *
 * v1.0 By Neil Townsend and others, see AUTHORS file
 *
 * See readme.txt for copyright information.
 */

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

/*
 * Arrays for interpretting the codes - available to the user
 */

int baud_from_baud_bits[] = {50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
int num_stop_bits[] = {1, 2};
int num_data_bits[] = {5, 6, 7, 8};

char *flow_control_desc[] = {"no", "xon/xoff", "rts/cts"};
char *parity_desc[] = {"no", "odd", "even", "mark", "space"};

/*
 * 1. Generic internal variables - not for user use
 */

char dz_empty_string[] = EMPTY_STRING;
char szDZCommErr[128];
int comm_port_inited_list_size = 0;
comm_port **comm_port_inited_list = NULL;
comm_port_func_table *comm_port_funcs = NULL;

/*
 * 2. User access functions
 */

/*------------------------- COMM PORT INIT --------------------------*/

#define COMM_PORT_LIST_ALLOC_BLOCK_SIZE 5

comm_port *comm_port_init(comm com) {
    int i;
    int n = 0;
    comm_port *port = (comm_port *)malloc(sizeof(comm_port));
    comm_port **cpil_temp;

    szDZCommErr[0] = 0;

    if (port == NULL) {
        dz_make_comm_err("Out of memory !");
        return (NULL);
    }

    /*
     * The port will need to be added to the inited list - check we have the space.
     */
    if (comm_port_inited_list_size > 0) {
        while (comm_port_inited_list[n] != NULL) n++;
    }

    if (n >= (comm_port_inited_list_size - 1)) { /* Need a bigger list */
        if (comm_port_inited_list == NULL) {
            comm_port_inited_list = calloc(sizeof(comm_port *), COMM_PORT_LIST_ALLOC_BLOCK_SIZE);
            if (comm_port_inited_list == NULL) {
                dz_make_comm_err("Insufficient memory!");
                return NULL;
            } else {
                comm_port_inited_list_size = COMM_PORT_LIST_ALLOC_BLOCK_SIZE;
                for (i = 0; i < COMM_PORT_LIST_ALLOC_BLOCK_SIZE; i++) comm_port_inited_list[i] = NULL;
            }
        } else {
            cpil_temp = realloc(comm_port_inited_list, (comm_port_inited_list_size + COMM_PORT_LIST_ALLOC_BLOCK_SIZE) * sizeof(comm_port *));
            if (cpil_temp == NULL) {
                dz_make_comm_err("Insufficient memory for another port!");
                return NULL;
            } else {
                comm_port_inited_list = cpil_temp;
                comm_port_inited_list_size += COMM_PORT_LIST_ALLOC_BLOCK_SIZE;
                for (i = n; i < COMM_PORT_LIST_ALLOC_BLOCK_SIZE; i++) comm_port_inited_list[i] = NULL;
            }
        }
    }

    /*
     * Basic memory requirements of the port
     */
    if ((port->InBuf = queue_new_(4096, 1)) == NULL) {
        dz_make_comm_err("Out of memory !");
        free(port);
        return (NULL);
    }
    if ((port->OutBuf = queue_new_(4096, 1)) == NULL) {
        dz_make_comm_err("Out of memory !");
        queue_delete(port->InBuf);
        free(port);
        return (NULL);
    }

    /*
     * Set some default values based on standard and OS in use. The user
     * will modify these before actually using the port.
     */

    if (comm_port_funcs->init) {
        if (comm_port_funcs->init(port, com) != port) {
            dz_make_comm_err("Unable to initialise the port");
            if (port->OutBuf) queue_delete(port->OutBuf);
            if (port->InBuf) queue_delete(port->InBuf);
            free(port);
            return NULL;
        }
    }

    /* Add this port to the inited list */
    comm_port_inited_list[n] = port;
    comm_port_inited_list[n + 1] = NULL;
    return (port);
}

/*-------------------- COMM PORT LOAD SETTINGS ---------------------*/

int get_parsed_line(FILE *fp, char *line, int max_line) {
    int count = 0;
    int c;
    int start_of_line = 1;
    int keep_going = 1;

    do {
        c = fgetc(fp);

        if ((c == EOF) || (c == '\n') || (c == ';')) {
            line[count] = 0;
            keep_going = 0;
            if (c == EOF) count = -(count + 1);
        } else if (start_of_line && ((c == ' ') || (c == '\t'))) {
            /* ignore whitespace at start of line */
        } else if (c == '#') {
            line[count] = 0;
            do {
                c = fgetc(fp);
            } while ((c != EOF) && (c != '\n'));
            if (c == EOF) count = -(count + 1);
        } else {
            line[count++] = (char)c;
        }

    } while ((count < max_line) && keep_going);

    return count;
}

void get_value_from_line(char *value, char *line) {
    int i = 0;

    if (not_commented(line) && (line = strchr(line, '=')) != NULL) {
        line++;

        while ((*line != ';') && (*line != '\n') && (*line != 0) && (i < 9)) {
            if (!isspace(*line)) value[i++] = *line++;
        }
        value[i] = 0;
    }
}

int comm_port_load_settings(comm_port *port, char *ini_name) {
    FILE *ini;
    int l, i;
    baud_bits n;
    char line_from_file[80];
    char value[80];

    if ((ini = fopen(ini_name, "rt")) == NULL) {
        dz_make_comm_err("Could not open serial port description file.");
        return (0);
    }

    do {
        l = get_parsed_line(ini, line_from_file, 80);

        if (l > 0) {
            if (!strncasecmp(line_from_file, "baud", 4)) {
                get_value_from_line(value, line_from_file + 4);
                if ((i = atoi(value)) != 0) {
                    for (n = DZ_MIN_BAUD; n <= DZ_MAX_BAUD; n++) {
                        if (baud_from_baud_bits[n] == i) {
                            port->nBaud = n;
                            n = DZ_MAX_BAUD + 5;
                        }
                    }
                    if (n != (DZ_MAX_BAUD + 6)) {
                        dz_make_comm_err("Serial port description file has invalid baud rate.");
                        return 0;
                    }
                }
            } else if (!strncasecmp(line_from_file, "irq", 3)) {
                get_value_from_line(value, line_from_file + 3);
                if ((i = atoi(value)) != 0) port->nIRQ = i;
            } else if (!strncasecmp(line_from_file, "address", 7)) {
                get_value_from_line(value, line_from_file + 7);
                if ((i = strtol(value, NULL, 0)) != 0) port->nPort = i;
            } else if (!strncasecmp(line_from_file, "data", 4)) {
                get_value_from_line(value, line_from_file + 4);
                if ((i = atoi(value)) != 0) {
                    switch (i) {
                        case 8:
                            port->nData = BITS_8;
                            break;
                        case 7:
                            port->nData = BITS_7;
                            break;
                        case 6:
                            port->nData = BITS_6;
                            break;
                        case 5:
                            port->nData = BITS_5;
                            break;
                    }
                }
            } else if (!strncasecmp(line_from_file, "parity", 6)) {
                get_value_from_line(value, line_from_file + 6);

                if (!strcasecmp(value, "even"))
                    port->nParity = EVEN_PARITY;
                else if (!strcasecmp(value, "odd"))
                    port->nParity = ODD_PARITY;
                else if (!strcasecmp(value, "mark"))
                    port->nParity = MARK_PARITY;
                else if (!strcasecmp(value, "space"))
                    port->nParity = SPACE_PARITY;
                else if (!strncasecmp(value, "no", 2))
                    port->nParity = NO_PARITY;
            } else if (!strncasecmp(line_from_file, "control", 7)) {
                get_value_from_line(value, line_from_file + 7);

                if ((!strcasecmp(value, "xon_xoff")) || (!strcasecmp(value, "xon/xoff")))
                    port->control_type = XON_XOFF;
                else if ((!strcasecmp(value, "no")) || (!strcasecmp(value, "none")))
                    port->control_type = NO_CONTROL;
                else if ((!strcasecmp(value, "rts_cts")) || (!strcasecmp(value, "rts/cts")))
                    port->control_type = RTS_CTS;
            } else if (!strncasecmp(line_from_file, "stop", 4)) {
                get_value_from_line(value, line_from_file + 4);

                if ((i = atoi(value)) != 0) {
                    switch (i) {
                        case 1:
                            port->nStop = STOP_1;
                            break;
                        case 2:
                            port->nStop = STOP_2;
                            break;
                    }
                }
            } else if (!strncasecmp(line_from_file, "name", 4)) {
                get_value_from_line(value, line_from_file + 4);
                strncpy(port->szName, value, DZCOMM_MAX_PORT_NAME_LENGTH);
            }
        }

    } while (l >= 0);

    return (1);
}

/*----------------------COMM PORT ACCESS FUNCTIONS -------------------*/

void comm_port_set_name(comm_port *port, char *name) {
    if (name) strncpy(port->szName, name, DZCOMM_MAX_PORT_NAME_LENGTH);
}

void comm_port_set_port_address(comm_port *port, unsigned short int port_address) { port->nPort = port_address; }

void comm_port_set_irq_num(comm_port *port, unsigned char irq_num) { port->nIRQ = irq_num; }

void comm_port_set_baud_rate(comm_port *port, baud_bits baud_rate) { port->nBaud = baud_rate; }

void comm_port_set_data_bits(comm_port *port, data_bits num_data_bits) { port->nData = num_data_bits; }

void comm_port_set_stop_bits(comm_port *port, stop_bits num_stop_bits) { port->nStop = num_stop_bits; }

void comm_port_set_parity(comm_port *port, parity_bits parity_option) { port->nParity = parity_option; }

void comm_port_set_flow_control(comm_port *port, flow_control_type flow_option) { port->control_type = flow_option; }

void comm_port_set_lsr_handler(comm_port *port, dz_ll_handler lsr_handler) { port->lsr_handler = lsr_handler; }

void comm_port_set_msr_handler(comm_port *port, dz_ll_handler msr_handler) { port->msr_handler = msr_handler; }

char *comm_port_get_name(comm_port *port, char *name, int max_chars) {
    strncpy(name, port->szName, max_chars);
    return name;
}

unsigned short int comm_port_get_port_address(comm_port *port) { return port->nPort; }

unsigned char comm_port_get_irq_num(comm_port *port) { return port->nIRQ; }

baud_bits comm_port_get_baud_rate(comm_port *port) { return port->nBaud; }

data_bits comm_port_get_data_bits(comm_port *port) { return port->nData; }

stop_bits comm_port_get_stop_bits(comm_port *port) { return port->nStop; }

parity_bits comm_port_get_parity(comm_port *port) { return port->nParity; }

flow_control_type comm_port_get_flow_control(comm_port *port) { return port->control_type; }

dz_ll_handler comm_port_get_lsr_handler(comm_port *port) { return port->lsr_handler; }

dz_ll_handler comm_port_get_msr_handler(comm_port *port) { return port->msr_handler; }

/*----------------------- COMM PORT INSTALL HANDLER ------------------*/

int comm_port_install_handler(comm_port *port) {
    /* Do some checks */
    if (port == NULL) {
        dz_make_comm_err("NULL port passed to comm_port_install_handler.");
        return (0);
    }
    if (port->installed == PORT_INSTALLED) {
        dz_make_comm_err("Comm already installed !");
        return (0);
    }
    if (port->nIRQ > 15) { /* nIRQ is unsigned, so it can't be < 0 */
        dz_make_comm_err("IRQ number out of range ! Must be 0 ... 15 .");
        return (0);
    }
    if ((port->nBaud < DZ_MIN_BAUD) || (port->nBaud > DZ_MAX_BAUD)) {
        dz_make_comm_err("Invalid baud rate.");
        return (0);
    }
    if ((port->nStop < DZ_MIN_STOP) || (port->nStop > DZ_MAX_STOP)) {
        dz_make_comm_err("Invalid number of stop bits.");
        return (0);
    }
    if ((port->nData < DZ_MIN_DATA) || (port->nData > DZ_MAX_DATA)) {
        dz_make_comm_err("Invalid number of data bits.");
        return (0);
    }
    if ((port->control_type < DZ_MIN_CONTROL) || (port->control_type > DZ_MAX_CONTROL)) {
        dz_make_comm_err("Invalid control method.");
        return (0);
    }
    if ((port->nParity < DZ_MIN_PARITY) || (port->nParity > DZ_MAX_PARITY)) {
        dz_make_comm_err("Invalid parity.");
        return (0);
    }

    switch (port->control_type) {
        case XON_XOFF:
            port->xon = 0;
            port->xoff = 0;
            port->xonxoff_rcvd = XON_RCVD;
            port->xonxoff_send = XON_SENT;
            break;
        case RTS_CTS:
            port->cts = CTS_ON;
            port->rts = 1;
            break;
        default:
            break;
    }

    /* And call the Machine/OS appropriate installer */
    if (comm_port_funcs->install_handler) {
        if (comm_port_funcs->install_handler(port) == 0) return 0;
    }

    switch (port->nData) {
        case BITS_5:
            port->valid_bit_mask = 0x1f;
            break;
        case BITS_6:
            port->valid_bit_mask = 0x3f;
            break;
        case BITS_7:
            port->valid_bit_mask = 0x7f;
            break;
        case BITS_8:
            port->valid_bit_mask = 0xff;
            break;
    }

    port->installed = PORT_INSTALLED;
    return (1);
}

/* ----------------------- COMM PORT UNINSTALL --------------------- */

void comm_port_uninstall(comm_port *port) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return;

    /* And do the machine/OS specific uninstall */
    if (comm_port_funcs->uninstall_handler) comm_port_funcs->uninstall_handler(port);

    port->installed = PORT_NOT_INSTALLED;
}

/*----------------------- COMM PORT REINSTALL ---------------------------*/

int comm_port_reinstall(comm_port *port) {
    /* Trap the obvious */
    if (port == NULL) return 0;

    if (port->installed == PORT_NOT_INSTALLED) return 0;

    comm_port_uninstall(port);

    return (comm_port_install_handler(port));
}

/*----------------------- COMM PORT DELETE ----------------------------*/

void comm_port_delete(comm_port *port) {
    int n = 0;

    /* Some obvious checks to do first ... */
    if ((port == NULL) || (comm_port_inited_list == NULL)) return;

    /* Which of the inited ports is it */
    while ((comm_port_inited_list[n] != port) && (comm_port_inited_list[n] != NULL)) n++;

    /* If it isn't currently inited then return - it's been deleted before
     * (or it never existed)
     */
    if (comm_port_inited_list[n] == NULL) return;

    /* If necessary, uninstall before we delete */
    if (port->installed == PORT_INSTALLED) {
        comm_port_flush_output(port); /* Don't wait for it to get sent */
        comm_port_uninstall(port);
    }

    /* Do machine specific stuff */
    if (comm_port_funcs->delete) comm_port_funcs->delete (port);

    /* Clear up memory */
    if (port->OutBuf) queue_delete(port->InBuf);
    if (port->InBuf) queue_delete(port->OutBuf);
    free(port);

    /* Take of inited list */
    do {
        comm_port_inited_list[n] = comm_port_inited_list[n + 1];
        n++;
    } while (comm_port_inited_list[n - 1]);
}

/*----------------------- COMM PORT IN EMPTY ---------------------------*/

int comm_port_in_empty(comm_port *port) {
    int rv = 0;

    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    /* Do the machine/OS specific thing */
    if (comm_port_funcs->io_buffer_query) rv = comm_port_funcs->io_buffer_query(port, DZ_IO_BUFFER_IN, DZ_IO_BUFFER_EMPTY);

    if (rv == 1) rv = -1;
    return rv;
}

/*---------------------- COMM PORT OUT EMPTY --------------------------*/

int comm_port_out_empty(comm_port *port) {
    int rv = 0;

    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    /* Do the machine/OS specific thing */
    if (comm_port_funcs->io_buffer_query) rv = comm_port_funcs->io_buffer_query(port, DZ_IO_BUFFER_OUT, DZ_IO_BUFFER_EMPTY);

    if (rv == 1) rv = -1;
    return rv;
}

/*----------------------- COMM PORT IN FULL ----------------------------*/

int comm_port_in_full(comm_port *port) {
    int rv = 0;

    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    /* Do the machine/OS specific thing */
    if (comm_port_funcs->io_buffer_query) rv = comm_port_funcs->io_buffer_query(port, DZ_IO_BUFFER_IN, DZ_IO_BUFFER_FULL);

    if (rv == 1) rv = -1;
    return rv;
}

/*---------------------- COMM PORT OUT FULL ---------------------------*/

int comm_port_out_full(comm_port *port) {
    int rv = 0;

    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    /* Do the machine/OS specific thing */
    if (comm_port_funcs->io_buffer_query) rv = comm_port_funcs->io_buffer_query(port, DZ_IO_BUFFER_OUT, DZ_IO_BUFFER_FULL);

    if (rv == 1) rv = -1;
    return rv;
}

/*------------------------- COMM PORT TEST -----------------------------*/

int comm_port_test(comm_port *port) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED) || (comm_port_funcs->in == NULL)) return -1;

    /* If nothing there don't bother trying */
    if (comm_port_in_empty(port)) return (-1);

    /* Do the machine/OS specific thing */
    return comm_port_funcs->in(port);
}

/*------------------ COMM PORT SEND BREAK -----------------------------*/

void comm_port_send_break(comm_port *port, int msec_duration) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return;

    /* Do the machine/OS specific thing */
    if (comm_port_funcs->send_break) comm_port_funcs->send_break(port, msec_duration);
}

/*------------------------- COMM PORT OUT -----------------------------*/

int comm_port_out(comm_port *port, unsigned char c) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    /* Make sure it is being sent in a machine/OS correct way */
    return comm_port_funcs->out(port, &c);
}

/*----------------------- COMM PORT STRING SEND -------------------------*/

int comm_port_string_send(comm_port *port, const char *s) {
    int i, c;

    /* Trap the obvious */
    if ((s == NULL) || (port == NULL) || (port->installed == PORT_NOT_INSTALLED) || (comm_port_funcs->out == NULL)) return 0;

    /* Make sure it is being sent in a machine/OS correct way */
    c = comm_port_funcs->out(port, (unsigned char *)s);
    for (i = 1; ((s[i] != 0) && (c == i)); i++) c += comm_port_funcs->out(port, (unsigned char *)&s[i]);

    return c;
}

/*---------------------- COMM PORT COMMAND SEND -------------------------*/

int comm_port_command_send(comm_port *port, char *s) {
    int i, c;
    unsigned char r = '\r';

    /* Trap the obvious */
    if ((s == NULL) || (port == NULL) || (port->installed == PORT_NOT_INSTALLED) || (comm_port_funcs->out == NULL)) return 0;

    /* Make sure it is being sent in a machine/OS correct way */
    c = comm_port_funcs->out(port, (unsigned char *)s);
    for (i = 1; ((s[i] != 0) && (c == i)); i++) c += comm_port_funcs->out(port, (unsigned char *)&s[i]);
    if (c == i) c += comm_port_funcs->out(port, &r);

    return c;
}

/*---------------------- COMM PORT FLUSH INPUT -------------------------*/
void comm_port_flush_input(comm_port *port) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return;

    if (port->InBuf) queue_reset(port->InBuf);

    if (comm_port_funcs->flush_input) comm_port_funcs->flush_input(port);
}

/*---------------------- COMM PORT FLUSH OUTPUT ------------------------*/
void comm_port_flush_output(comm_port *port) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return;

    if (port->OutBuf) queue_reset(port->OutBuf);

    if (comm_port_funcs->flush_output) comm_port_funcs->flush_output(port);
}

/*------------------- MODEM HANG UP -----------------*/

int modem_hangup(comm_port *port) {
    /* Trap the obvious */
    if ((port == NULL) || (port->installed == PORT_NOT_INSTALLED)) return 0;

    if (comm_port_out(port, 2) != 1) return 0;
    sleep(3);
    if (comm_port_string_send(port, "+++") != 3) return 0;
    sleep(3);
    if (comm_port_string_send(port, "ATH0\r") != 5) return 0;

    return 1;
}

/*------------------------- COMM PORT CLOSE DOWN -----------------------*/

void dzcomm_closedown(void) {
    /* Ensure that all ports are properly closed */
    if (comm_port_inited_list != NULL) {
        while (comm_port_inited_list[0]) {
            comm_port_delete(comm_port_inited_list[0]);
        }
        free(comm_port_inited_list);
        comm_port_inited_list = NULL;
    }

    comm_port_inited_list_size = 0;

    /* Do any machine/OS specific closedown stuff */
    if (comm_port_funcs != NULL) {
        if (comm_port_funcs->closedown != NULL) comm_port_funcs->closedown();
    }

    /* Indicate that we are no longer inited so that we can be reinited if desired */
    comm_port_funcs = NULL;
}

/*------------------------- COMM PORT INIT -----------------------------*/

int dzcomm_init(void) {
    int i;

    /* Check that we aren't already inited */
    if (comm_port_funcs != NULL) return 0;

    /* Do the generic stuff */

    /* And do the machine/OS specific stuff, trying several if necessary */
    for (i = 0; _comm_port_driver_list[i].driver; i++) {
        comm_port_funcs = _comm_port_driver_list[i].driver;
        if (comm_port_funcs->method_init() == 1) {
            atexit(dzcomm_closedown);
            return 1;
        }
    }

    return 0;
}

/*-------------------- COMM GIVE LINE STATUS ------------------------*/

int comm_port_give_line_status(comm_port *port, dzcomm_line line) {
    /* This function will return the status of the CTS pin */
    int v;

    v = comm_port_funcs->give_line_status(port, line);

    return v;
}

/*-------------------- COMM SET LINE STATUS ------------------------*/

int comm_port_set_line_status(comm_port *port, dzcomm_line line, int value) {
    /* This function will return the status of the DSR pin */
    int v;

    v = comm_port_funcs->set_line_status(port, line, value);

    return v;
}
