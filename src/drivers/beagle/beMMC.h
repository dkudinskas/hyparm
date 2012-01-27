#ifndef MMC_HS_H
#define MMC_HS_H 

#include "common/types.h"

#include "io/mmc.h"

// uncomment me for debug: #define MMC_HS_DEBUG

#define MMCHS_BASE 0x4809C000


/* OMAP3 MMC host controller */

//PRCM defines for the MMCHS device
#define EN_MMC3 (1 << 30)
#define EN_MMC2 (1 << 25)
#define EN_MMC1 (1 << 24)


//HS bases for OMAP
#define MMCHS1 0x4809C000 //default for beagle
#define MMCHS2 0x480B4000
#define MMCHS3 0x480AD000

//HS offsets, all require 32bit access
#define MMCHS_SYSCONFIG  0x10   //RW
#define MMCHS_SYSSTATUS  0x14   // R
#define MMCHS_CSRE       0x24   //RW
#define MMCHS_SYSTEST    0x28   //RW
#define MMCHS_CON        0x2C   //RW
#define MMCHS_PWCNT      0x30   //RW
#define MMCHS_BLK       0x104   //RW
#define MMCHS_ARG       0x108   //RW 
#define MMCHS_CMD       0x10C   //RW
#define MMCHS_RSP10     0x110   // R
#define MMCHS_RSP32     0x114   // R
#define MMCHS_RSP54     0x118   // R
#define MMCHS_RSP76     0x11C   // R
#define MMCHS_DATA      0x120   //RW
#define MMCHS_PSTATE    0x124   // R
#define MMCHS_HCTL      0x128   //RW
#define MMCHS_SYSCTL    0x12C   //RW
#define MMCHS_STAT      0x130   //RW
#define MMCHS_IE        0x134   //RW
#define MMCHS_ISE       0x138   //RW
#define MMCHS_AC12      0x13C   // R
#define MMCHS_CAPA      0x140   //RW
#define MMCHS_CUR_CAPA  0x148   //RW
#define MMCHS_REV       0x1FC   // R

//HS bit masks
#define SYSCONFIG_ENAWAKEUP (1 << 2)
#define SYSCONFIG_SOFTRESET (1 << 1)
#define SYSSTATUS_RESETDONE (1 << 0)
#define SYSCTL_SRA          (1 << 24)
#define SYSCTL_SRC          (1 << 25)
#define SYSCTL_CEN          (1 << 2)
#define SYSCTL_ICE          (1 << 0)
#define SYSCTL_ICS          (1 << 1)
#define HCTL_IWE            (1 << 24)
#define HCTL_DTW_4          (1 << 1)
#define IE_CIRQ_ENABLE      (1 << 8)
#define CAPA_VS18           (1 << 26)
#define CAPA_VS30           (1 << 25)
#define CAPA_VS33           (1 << 24)
#define PSTATE_DATI         (1 << 1)
#define PSTATE_CMDI         (1 << 0)
#define RESERVED_MASK       (0x3 << 9) // 0x600

#define CON_OD              (1 << 0)
#define CON_CEATA           (1 << 12)
#define CON_DW8             (1 << 5)
#define CON_INIT            (1 << 1)
#define CON_MIT             (1 << 6)
#define CON_STR             (1 << 3)
#define CON_CTPL_MMCSD      (0 << 11)
#define CON_WPP_ACTIVEHIGH  (0 << 8)
#define CON_CDP_ACTIVEHIGH  (0 << 7)
#define CON_MODE_FUNC       (0 << 4)
#define HCTL_SDBP           (1 << 8)
#define HCTL_DTW_4BIT       (1 << 1)
#define HCTL_DTW_1BIT       (0 << 0)
#define HCTL_SDVS_18V       (0x5 << 9)
#define HCTL_SDVS_30V       (0x6 << 9)
#define STAT_CC             (1 << 0)
#define STAT_CTO            (1 << 16)
#define STAT_ERRI           (1 << 15)
#define STAT_BRR            (1 << 5)
#define STAT_BWR            (1 << 4)
#define STAT_TC             (1 << 1)

#define IE_CC       (0x01 << 0)
#define IE_TC       (0x01 << 1)
#define IE_BWR        (0x01 << 4)
#define IE_BRR        (0x01 << 5)
#define IE_CTO        (0x01 << 16)
#define IE_CCRC       (0x01 << 17)
#define IE_CEB        (0x01 << 18)
#define IE_CIE        (0x01 << 19)
#define IE_DTO        (0x01 << 20)
#define IE_DCRC       (0x01 << 21)
#define IE_DEB        (0x01 << 22)
#define IE_CERR       (0x01 << 28)
#define IE_BADA       (0x01 << 29)

#define SYSCTL_DTO_MASK     (0xF << 16) // 0xf0000
#define SYSCTL_DTO_15THDTO  (0xE << 16) // 0xe0000
/* SYSCTL.CLKD divisor values */
#define CLKD_REF_BYPASS_M   (0X1 << 6)
#define CLKD_OFFSET         6
#define CLKD_1023_M         (0x3FF << 6)

#define MMC_CLOCK_REFERENCE 96 /* MHz */

/*CMD and BLK bits*/
#define RSP_TYPE_OFFSET     (16)
#define RSP_TYPE_MASK       (0x3 << 16)
#define RSP_TYPE_NORSP      (0x0 << 16)
#define RSP_TYPE_LGHT136    (0x1 << 16)
#define RSP_TYPE_LGHT48     (0x2 << 16)
#define RSP_TYPE_LGHT48B    (0x3 << 16)
#define CCCE_NOCHECK        (0x0 << 19)
#define CCCE_CHECK          (0x1 << 19)
#define CICE_NOCHECK        (0x0 << 20)
#define CICE_CHECK          (0x1 << 20)
#define MSBS_SGLEBLK        (0x0 << 5)
#define MSBS_MULTIBLK       (0x1 << 5)
#define ACEN_DISABLE        (0x0 << 2)
#define BCE_DISABLE         (0x0 << 1)
#define BCE_ENABLE          (0x1 << 1)
#define DDIR_OFFSET         (4)
#define DDIR_MASK           (0x1 << 4)
#define DDIR_WRITE          (0x0 << 4)
#define DDIR_READ           (0x1 << 4)
#define DP_OFFSET           (21)
#define DP_MASK             (0x1 << 21)
#define DP_NO_DATA          (0x0 << 21)
#define DP_DATA             (0x1 << 21)
#define NBLK_STPCNT         (0x0 << 16)
#define CMD_TYPE_NORMAL     (0x0 << 22)
#define DE_DISABLE          (0x0 << 0)
#define INIT_NOINIT         (0x0 << 1)
#define INIT_INITSTREAM     (0x1 << 1)
/* Driver definitions */
#define MMCSD_SECTOR_SIZE   512
#define MMC_CARD      0
#define SD_CARD       1
#define BYTE_MODE     0
#define SECTOR_MODE     1
#define CLK_INITSEQ     0
#define CLK_400KHZ      1
#define CLK_MISC      2

/* COMMAND INDEXES */
#define CMD0 (0 << 24)
#define CMD1 (1 << 24)
#define CMD2 (2 << 24)
#define CMD3 (3 << 24)
#define CMD4 (4 << 24)
#define CMD5 (5 << 24)
#define CMD6 (6 << 24)
#define CMD7 (7 << 24)
#define CMD8 (8 << 24)
#define CMD9 (9 << 24)
#define CMD10 (10 << 24)
#define CMD11 (11 << 24)
#define CMD12 (12 << 24)
#define CMD13 (13 << 24)
#define CMD14 (14 << 24)
#define CMD15 (15 << 24)
#define CMD16 (16 << 24)
#define CMD17 (17 << 24)
#define CMD18 (18 << 24)
#define CMD19 (19 << 24)
#define CMD20 (20 << 24)
#define CMD21 (21 << 24)
#define CMD22 (22 << 24)
#define CMD23 (23 << 24)
#define CMD24 (24 << 24)
#define CMD25 (25 << 24)
#define CMD26 (26 << 24)
#define CMD27 (27 << 24)
#define CMD28 (28 << 24)
#define CMD29 (29 << 24)
#define CMD30 (30 << 24)
#define CMD31 (31 << 24)
#define CMD32 (32 << 24)
#define CMD33 (33 << 24)
#define CMD34 (34 << 24)
#define CMD35 (35 << 24)
#define CMD36 (36 << 24)
#define CMD37 (37 << 24)
#define CMD38 (38 << 24)
#define CMD39 (39 << 24)
#define CMD40 (40 << 24)
#define CMD41 (41 << 24)
#define CMD42 (42 << 24)
#define CMD43 (43 << 24)
#define CMD44 (44 << 24)
#define CMD45 (45 << 24)
#define CMD46 (46 << 24)
#define CMD47 (47 << 24)
#define CMD48 (48 << 24)
#define CMD49 (49 << 24)
#define CMD50 (50 << 24)
#define CMD51 (51 << 24)
#define CMD52 (52 << 24)
#define CMD53 (53 << 24)
#define CMD54 (54 << 24)
#define CMD55 (55 << 24)
#define CMD56 (56 << 24)
#define CMD57 (57 << 24)
#define CMD58 (58 << 24)
#define CMD59 (59 << 24)
#define CMD60 (60 << 24)
#define CMD61 (61 << 24)
#define CMD62 (62 << 24)
#define CMD63 (63 << 24)

#define ACMD0 (0 << 24)
#define ACMD1 (1 << 24)
#define ACMD2 (2 << 24)
#define ACMD3 (3 << 24)
#define ACMD4 (4 << 24)
#define ACMD5 (5 << 24)
#define ACMD6 (6 << 24)
#define ACMD7 (7 << 24)
#define ACMD8 (8 << 24)
#define ACMD9 (9 << 24)
#define ACMD10 (10 << 24)
#define ACMD11 (11 << 24)
#define ACMD12 (12 << 24)
#define ACMD13 (13 << 24)
#define ACMD14 (14 << 24)
#define ACMD15 (15 << 24)
#define ACMD16 (16 << 24)
#define ACMD17 (17 << 24)
#define ACMD18 (18 << 24)
#define ACMD19 (19 << 24)
#define ACMD20 (20 << 24)
#define ACMD21 (21 << 24)
#define ACMD22 (22 << 24)
#define ACMD23 (23 << 24)
#define ACMD24 (24 << 24)
#define ACMD25 (25 << 24)
#define ACMD26 (26 << 24)
#define ACMD27 (27 << 24)
#define ACMD28 (28 << 24)
#define ACMD29 (29 << 24)
#define ACMD30 (30 << 24)
#define ACMD31 (31 << 24)
#define ACMD32 (32 << 24)
#define ACMD33 (33 << 24)
#define ACMD34 (34 << 24)
#define ACMD35 (35 << 24)
#define ACMD36 (36 << 24)
#define ACMD37 (37 << 24)
#define ACMD38 (38 << 24)
#define ACMD39 (39 << 24)
#define ACMD40 (40 << 24)
#define ACMD41 (41 << 24)
#define ACMD42 (42 << 24)
#define ACMD43 (43 << 24)
#define ACMD44 (44 << 24)
#define ACMD45 (45 << 24)
#define ACMD46 (46 << 24)
#define ACMD47 (47 << 24)
#define ACMD48 (48 << 24)
#define ACMD49 (49 << 24)
#define ACMD50 (50 << 24)
#define ACMD51 (51 << 24)
#define ACMD52 (52 << 24)
#define ACMD53 (53 << 24)
#define ACMD54 (54 << 24)
#define ACMD55 (55 << 24)
#define ACMD56 (56 << 24)
#define ACMD57 (57 << 24)
#define ACMD58 (58 << 24)
#define ACMD59 (59 << 24)
#define ACMD60 (60 << 24)
#define ACMD61 (61 << 24)
#define ACMD62 (62 << 24)
#define ACMD63 (63 << 24)

int mmcInit(struct mmc *mmc);

int mmcSendCommand(struct mmc *dev, struct mmcCommand *cmd, struct mmcData *data);

struct mmc* mmcInterfaceInit(void);

void mmcEnableClocks(void);

void mmcSoftReset(struct mmc *dev);

void mmcSetVoltageCapa(struct mmc *dev);

void mmcWakeupConfig(struct mmc *dev);

int mmcRead(struct mmc *dev, char *buf, u32int size);

int mmcWrite(struct mmc *dev, const char *buf, u32int size);

#endif
