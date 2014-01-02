#ifndef __VM__OMAP_35XX__MMC_H__
#define __VM__OMAP_35XX__MMC_H__

void initVirtMmc(virtualMachine *vm, u32int mmcNumber);
u32int loadMmc(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeMmc(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);
void mmcDoDmaXfer(GCONTXT *context, int mmcId, int dmaChannelId);

#define MMCHS_SYSCONFIG 0x010
#define MMCHS_SYSSTATUS 0x014
#define MMCHS_CSRE      0x024
#define MMCHS_SYSTEST   0x028
#define MMCHS_CON       0x02C
#define MMCHS_PWCNT     0x030
#define MMCHS_BLK       0x104
#define MMCHS_ARG       0x108
#define MMCHS_CMD       0x10C
#define MMCHS_RSP10     0x110
#define MMCHS_RSP32     0x114
#define MMCHS_RSP54     0x118
#define MMCHS_RSP76     0x11C
#define MMCHS_DATA      0x120
#define MMCHS_PSTATE    0x124
#define MMCHS_HCTL      0x128
#define MMCHS_SYSCTL    0x12C
#define MMCHS_STAT      0x130
#define MMCHS_IE        0x134
#define MMCHS_ISE       0x138
#define MMCHS_AC12      0x13C
#define MMCHS_CAPA      0x140
#define MMCHS_CUR_CAPA  0x148
#define MMCHS_REV       0x1FC

#define MMC_SOFTRESET   0x2
#define MMC_RESETDONE   0x1

// MMCHS_SYSCTL
#define MMC_ICE         0x1
#define MMC_ICS         0x2
#define MMC_SRC         (1 << 25)
#define MMC_SRD         (1 << 26)

// MMCHS_CON
#define MMC_INIT        0x2

// MMCHS_STAT
#define MMC_CC          1
#define MMC_TC          (1 << 1)
#define MMC_BRR         (1 << 5)
#define MMC_ERRI        (1 << 15)
#define MMC_CTO         (1 << 16)

// MMC R1
#define MMC_APP_CMD     (1 << 5)

// MMC_CMD
#define MMC_DE          1

// helpers
#define MMC_CMD_INDEX(cmd) ((cmd >> 24) & 0x3f)

#define MMC_READ        TRUE
#define MMC_WRITE       FALSE
struct Mmc 
{
  u32int mmcSysconfig;
  u32int mmcSysstatus;
  u32int mmcCsre;
  u32int mmcSystest;
  u32int mmcCon;
  u32int mmcPwcnt;
  u32int mmcBlk;
  u32int mmcArg;
  u32int mmcCmd;
  u32int mmcRsp10;
  u32int mmcRsp32;
  u32int mmcRsp54;
  u32int mmcRsp76;
  u32int mmcData;
  u32int mmcPstate;
  u32int mmcHctl;
  u32int mmcSysctl;
  u32int mmcStat;
  u32int mmcIe;
  u32int mmcIse;
  u32int mmcAc12;
  u32int mmcCapa;
  u32int mmcCurCapa;
  u32int mmcRev;
  
  bool   cardPresent;
};

extern struct Mmc *mmc[3];

#endif /* __VM__OMAP_35XX__MMC_H__ */
