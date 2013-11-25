#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"

#include "vm/omap35xx/intc.h"

void CPSRWriteByInstr(GCONTXT* context, CPSRreg val, u8int bytemask, bool is_exc_ret)
{
  u32int cpsr = 0;
  if ((bytemask & 8) != 0)
  {
    cpsr |= val.value & 0xF8000000; // N,Z,C,V,Q flags
    if (is_exc_ret)
    {
      cpsr |= val.value & 0x07000000; // IT<1:0>,J execution state bits
    }
  }

  if ((bytemask & 4) != 0)
  {
    // bits <23:20> are reserved SBZP bits
    cpsr |= val.value & 0x000F0000; // GE<3:0> flags
  }

  if ((bytemask & 2) != 0)
  {
    if (is_exc_ret)
    {
      cpsr |= val.value & 0x0000FC00; // IT<7:2> execution state bits
    }
    cpsr |= val.value & 0x00000200; // E bit is user-writable

    // if privileged && (IsSecure() || SCR.AW == '1' || HaveVirtExt()) then
    // privileged is TRUE, ans SCR.AW is TRUE.
    cpsr |= val.value & 0x00000100; // 'A' interrupt mask
  }


  if ((bytemask & 1) != 0)
  {
    cpsr |= val.value & 0x00000080; // I interrupt mask
#ifndef CONFIG_HW_PASSTHROUGH
    if ((val.bits.I == 0) && (context->CPSR.bits.I == 1)) // enabling interrupts
    {
      // check interrupt controller if there is an interrupt pending
      if (isIrqPending(context->vm.irqController))
      {
        context->guestIrqPending = TRUE;
      }
    }
#endif

    // if privileged && (!nmfi || value<6> == '0') &&
    //   (IsSecure() || SCR.FW == '1' || HaveVirtExt()) then
    // privileged TRUE, SCR.FW true. only need to check nmfi
    if (((getCregVal(context, CP15_SCTRL) & SYS_CTRL_NON_MASK_FIQ) == 0) || (val.bits.F == 0))
    {
      cpsr |= val.value & 0x00000040; // F interrupt mask
#ifndef CONFIG_HW_PASSTHROUGH
      if ((val.bits.F == 0) && (context->CPSR.bits.F == 1)) // enabling fiq's
      {
        DIE_NOW(context, "guest enabling fast interrupts\n");
      }
#endif
    }
    if (is_exc_ret)
    {
      cpsr |= val.value & 0x00000020; // T execution state bit
    }

    // if privileged - but always privileged!
    if (BadMode(val.bits.mode))
    {
      UNPREDICTABLE();
    }
    else
    {
      // Check for attempts to enter modes only permitted in Secure state from
      // Non-secure state. These are Monitor mode ('10110'), and FIQ mode ('10001')
      // if the Security Extensions have reserved it. The definition of UNPREDICTABLE
      // does not permit the resulting behavior to be a security hole.
      // if !IsSecure() && value<4:0> == '10110' then UNPREDICTABLE;
      // if !IsSecure() && value<4:0> == '10001' && NSACR.RFR == '1' then UNPREDICTABLE;
      // There is no Hyp mode ('11010') in Secure state, so that is UNPREDICTABLE
      // if SCR.NS == '0' && value<4:0> == '11010' then UNPREDICTABLE;
      // Cannot move into Hyp mode directly from a Non-secure PL1 mode
      // if !IsSecure() && CPSR.M != '11010' && value<4:0> == '11010' then UNPREDICTABLE;
      // Cannot move out of Hyp mode with this function except on an exception return
      // if CPSR.M == '11010' && value<4:0> != '11010' && !is_excpt_return then UNPREDICTABLE;
      cpsr |= val.value & 0x1f; // CPSR<4:0>, mode bits
    }
  }
  
  context->CPSR.value = cpsr;
  return;
}
