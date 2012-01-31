#ifndef __VM__OMAP_35XX__GPTIMER_H__
#define __VM__OMAP_35XX__GPTIMER_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


// base addresses
#define GPTIMER1        0x48318000
#define GPTIMER2        0x49032000
#define GPTIMER3        0x49034000
#define GPTIMER4        0x49036000
#define GPTIMER5        0x49038000
#define GPTIMER6        0x4903A000
#define GPTIMER7        0x4903C000
#define GPTIMER8        0x4903E000
#define GPTIMER9        0x49040000
#define GPTIMER10       0x48086000
#define GPTIMER11       0x48088000
#define GPTIMER12       0x48304000


// register offsets and bit values
#define GPT_REG_TIOCP_CFG      0x010 // CONFIG register
#define GPT_TIOCP_CFG_RESERVED      0xFFFFFCC0
#define GPT_TIOCP_CFG_CLOCKACTIVITY 0x00000300
#define GPT_TIOCP_CFG_EMUFREE       0x00000020
#define GPT_TIOCP_CFG_IDLEMODE      0x00000018
#define GPT_TIOCP_CFG_ENABLEWAKEUP  0x00000004
#define GPT_TIOCP_CFG_SOFTRESET     0x00000002
#define GPT_TIOCP_CFG_AUTOIDLE      0x00000001

#define GPT_REG_TISTAT         0x014 // STATUS register
#define GPT_TISTAT_RESERVED         0xFFFFFFFE
#define GPT_TISTAT_RESETDONE        0x00000001

#define GPT_REG_TISR           0x018 // INTERRUPT status
#define GPT_TISR_RESERVED           0xFFFFFFF8
#define GPT_TISR_CAPTURE            0x00000004
#define GPT_TISR_OVERFLOW           0x00000002
#define GPT_TISR_MATCH              0x00000001

#define GPT_REG_TIER           0x01C // INTERRUPT enable
#define GPT_TIER_RESERVED           0xFFFFFFF8
#define GPT_TIER_CAPTURE            0x00000004
#define GPT_TIER_OVERFLOW           0x00000002
#define GPT_TIER_MATCH              0x00000001

#define GPT_REG_TWER           0x020 // WAKE UP enable
#define GPT_TWER_RESERVED           0xFFFFFFF8
#define GPT_TWER_CAPTURE            0x00000004
#define GPT_TWER_OVERFLOW           0x00000002
#define GPT_TWER_MATCH              0x00000001

#define GPT_REG_TCLR           0x024 // CONTROL
#define GPT_TCLR_RESERVED           0xFFFF8000
#define GPT_TCLR_GPO_CFG            0x00004000
#define GPT_TCLR_CAPT_MODE          0x00002000
#define GPT_TCLR_PULSE_TOGGLE       0x00001000
#define GPT_TCLR_TRIGGER_MODE       0x00000C00
#define GPT_TCLR_TRANS_CAPT         0x00000300
#define GPT_TCLR_PWM                0x00000080
#define GPT_TCLR_COMPARE_ENABLE     0x00000040
#define GPT_TCLR_PRESCALER_ENABLE   0x00000020
#define GPT_TCLR_TRIGGER_OUTPUT     0x0000001C
#define GPT_TCLR_AUTORELOAD         0x00000002 // ONE SHOT mode - stopped after OVF
#define GPT_TCLR_START_STOP         0x00000001

#define GPT_REG_TCRR           0x028 // INTERNAL clock register value
#define GPT_TCRR_COUNTER_VALUE      0xFFFFFFFF

#define GPT_REG_TLDR           0x02C // LOAD register value
#define GPT_TLDR_LOAD_VALUE         0xFFFFFFFF

#define GPT_REG_TTGR           0x030 // TRIGGER value
#define GPT_TTGR_TRIGGER_VALUE      0xFFFFFFFF

#define GPT_REG_TWPS           0x034 // WRITE-Posted pending
#define GPT_TWPS_RESERVED           0xFFFFFC00
#define GPT_TWPS_W_PEND_TOWR        0x00000200
#define GPT_TWPS_W_PEND_TOCR        0x00000100
#define GPT_TWPS_W_PEND_TCVR        0x00000080
#define GPT_TWPS_W_PEND_TNIR        0x00000040
#define GPT_TWPS_W_PEND_TPIR        0x00000020
#define GPT_TWPS_W_PEND_TMAR        0x00000010
#define GPT_TWPS_W_PEND_TTGR        0x00000008
#define GPT_TWPS_W_PEND_TLDR        0x00000004
#define GPT_TWPS_W_PEND_TCRR        0x00000002
#define GPT_TWPS_W_PEND_TCLR        0x00000001

#define GPT_REG_TMAR           0x038 // MATCH register value
#define GPT_TMAR_MATCH_VALUE        0xFFFFFFFF

#define GPT_REG_TCAR1          0x03C // FIRST captured counter
#define GPT_TCAR1_CAPTURE_VALUE     0xFFFFFFFF

#define GPT_REG_TSICR          0x040 // INTERFACE control
#define GPT_TSICR_RESERVED          0xFFFFFFF9
#define GPT_TSICR_POSTED            0x00000004
#define GPT_TSICR_SFTRESET          0x00000002

#define GPT_REG_TCAR2          0x044 // SECOND captured counter
#define GPT_TCAR2_CAPTURE_VALUE     0xFFFFFFFF

#define GPT_REG_TPIR           0x048 // POSITIVE Increment value (TCVR+TPIR)
#define GPT_TPIR_POS_INC            0xFFFFFFFF

#define GPT_REG_TNIR           0x04C // NEGATIVE Increment value (TCVR-TPIC)
#define GPT_TNIR_NEG_INC            0xFFFFFFFF

#define GPT_REG_TCVR           0x050 // CLOCK value register
#define GPT_TCVR_CLOCK              0xFFFFFFFF

#define GPT_REG_TOCR           0x054 // MASK tick irq for a selected nr of ticks
#define GPT_TOCR_RESERVED           0xFF000000
#define GPT_TOCR_OVF_COUNTER        0x00FFFFFF

#define GPT_REG_TOWR           0x058 // NUMBER Of masked overflow interrupt events
#define GPT_TOWR_RESERVED           0xFF000000
#define GPT_TOWR_OVF_WRAPPING       0x00FFFFFF


/* values for 1 milisecond clock generation */
#define TPIR1MS   232000
#define TNIR1MS  -768000
#define TLDR1MS  0xFFFFFFE0


void initGPTimer(void);

void resetGPTimer(void);

u32int loadGPTimer(device * dev, ACCESS_SIZE size, u32int address);

void storeGPTimer(device * dev, ACCESS_SIZE size, u32int address, u32int value);


struct GeneralPurposeTimer
{
  // registers
  u32int gptTiocpCfg;
  u32int gptTistat;
  u32int gptTisr;
  u32int gptTier;
  u32int gptTwer;
  u32int gptTclr;
  u32int gptTcrr;
  u32int gptTldr;
  u32int gptTtgr;
  u32int gptTwps;
  u32int gptTmar;
  u32int gptTcar1;
  u32int gptTsicr;
  u32int gptTcar2;
  u32int gptTpir;
  u32int gptTnir;
  u32int gptTcvr;
  u32int gptTocr;
  u32int gptTowr;
  // internal variables...
  u32int shadowValue;
};

#endif

