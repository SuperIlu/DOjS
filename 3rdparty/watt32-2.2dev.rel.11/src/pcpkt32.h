/*!\file pcpkt32.h
 */
#if !defined(_w32_PCPKT32_H) && (DOSX)
#define _w32_PCPKT32_H

#define PM_DRVR_3C501    1  /* 3COM EtherLink I, a real museum piece! */
#define PM_DRVR_3C503    2
#define PM_DRVR_3C505    3  /* 3COM EtherLink II */
#define PM_DRVR_3C507    4
#define PM_DRVR_3C5x9    5  /* 3COM EtherLink III, ISA */
#define PM_DRVR_3C59x    6  /* 3COM EtherLink III, PCI */
#define PM_DRVR_NE2000   7  /* NE2000 compatible cards */
#define PM_DRVR_EEXPR    8  /* Intel EtherExpress */
#define PM_DRVR_RTL8139  9  /* RealTek RTL8139 */

/*!\struct PM_driver
 */
typedef struct PM_driver {
        int         type;
        const char *name;
      } PM_driver;

extern int (*_pkt32_drvr)(IREGS*);

extern const struct PM_driver pm_driver_list[];

extern int         pkt32_drvr_probe (const PM_driver *drivers);
extern int         pkt32_drvr_init  (int driver, mac_address *addr);
extern const char *pkt32_drvr_name  (int driver);

#endif

