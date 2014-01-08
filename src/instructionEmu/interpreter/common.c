#include "instructionEmu/interpreter/common.h"
#include "instructionEmu/interpreter/internals.h"

#include "vm/omap35xx/intc.h"

u32int armExpandImm(u32int imm12, bool carryIn)
{
  return shiftCarry(imm12 & 0xFF, SHIFT_TYPE_ROR, (imm12 & 0xF00) >> 7, carryIn).val;
}


u32int_c asrCarry(u32int value, u32int amount)
{
  DIE_NOW(0, "asrCarry unimplemented");
}


void CPSRWriteByInstr(GCONTXT* context, u32int val, u8int bytemask, bool is_exc_ret)
{
  CPSRreg newValue = {.value = val};
  u32int cpsr = 0;
  if ((bytemask & 8) != 0)
  {
    cpsr |= newValue.value & 0xF8000000; // N,Z,C,V,Q flags
    if (is_exc_ret)
    {
      cpsr |= newValue.value & 0x07000000; // IT<1:0>,J execution state bits
    }
  }

  if ((bytemask & 4) != 0)
  {
    // bits <23:20> are reserved SBZP bits
    cpsr |= newValue.value & 0x000F0000; // GE<3:0> flags
  }

  if ((bytemask & 2) != 0)
  {
    if (is_exc_ret)
    {
      cpsr |= newValue.value & 0x0000FC00; // IT<7:2> execution state bits
    }
    cpsr |= newValue.value & 0x00000200; // E bit is user-writable

    // if privileged && (IsSecure() || SCR.AW == '1' || HaveVirtExt()) then
    // privileged is TRUE, ans SCR.AW is TRUE.
    cpsr |= newValue.value & 0x00000100; // 'A' interrupt mask
  }


  if ((bytemask & 1) != 0)
  {
    cpsr |= newValue.value & 0x00000080; // I interrupt mask
#ifndef CONFIG_HW_PASSTHROUGH
    if ((newValue.bits.I == 0) && (context->CPSR.bits.I == 1)) // enabling interrupts
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
    if (((getCregVal(context, CP15_SCTRL) & SYS_CTRL_NON_MASK_FIQ) == 0) || (newValue.bits.F == 0))
    {
      cpsr |= newValue.value & 0x00000040; // F interrupt mask
#ifndef CONFIG_HW_PASSTHROUGH
      if ((newValue.bits.F == 0) && (context->CPSR.bits.F == 1)) // enabling fiq's
      {
        DIE_NOW(context, "guest enabling fast interrupts\n");
      }
#endif
    }
    if (is_exc_ret)
    {
      cpsr |= newValue.value & 0x00000020; // T execution state bit
    }

    // if privileged - but always privileged!
    if (BadMode(newValue.bits.mode))
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
      cpsr |= newValue.value & 0x1f; // CPSR<4:0>, mode bits
      if (newValue.bits.mode != context->CPSR.bits.mode)
      {
        guestChangeMode(context, newValue.bits.mode);
      }
    }
  }
  
  context->CPSR.value = cpsr;
  return;
}


u32int_c lslCarry(u32int value, u32int amount)
{
  DIE_NOW(0, "lslCarry unimplemented");
}


u32int_c lsrCarry(u32int value, u32int amount)
{
  DIE_NOW(0, "lsrCarry unimplemented");
}


u32int_c rorCarry(u32int value, u32int amount)
{
  u32int_c result;
  result.val = (value >> amount) | (value << (32-amount));
  result.carry = ((result.val & 0x80000000) != 0);
  return result;
}


u32int_c rrxCarry(u32int value, bool carryIn)
{
  DIE_NOW(0, "rrxCarry unimplemented");
}


u32int_c shiftCarry(u32int value, ShiftType type, u32int amount, bool carryIn)
{
  if ((type == SHIFT_TYPE_RRX) && (amount != 1))
    UNPREDICTABLE();

  u32int_c result;
  if (amount == 0)
  {
    result.val = value;
    result.carry = carryIn;
  }
  else
  {
    switch (type)
    {
      case SHIFT_TYPE_LSL: result = lslCarry(value, amount); break;
      case SHIFT_TYPE_LSR: result = lsrCarry(value, amount); break;
      case SHIFT_TYPE_ASR: result = asrCarry(value, amount); break;
      case SHIFT_TYPE_ROR: result = rorCarry(value, amount); break;
      case SHIFT_TYPE_RRX: result = rrxCarry(value, carryIn); break;
    }
  }
  return result;
}


void SPSRWriteByInstr(GCONTXT* context, u32int val, u8int bytemask)
{
  if (CurrentModeIsUserOrSystem(context))
  {
    UNPREDICTABLE();
  }
  u32int spsr = 0;

  if ((bytemask & 8) != 0)
  {
    spsr |= val & 0xff000000; // N,Z,C,V,Q flags, IT<1:0>,J execution state bits
  }
  
  if ((bytemask & 4) != 0)
  {
    // bits <23:20> are reserved SBZP bits
    spsr |= val & 0x000f0000; // GE<3:0> flags
  }

  if ((bytemask & 2) != 0)
  {
    spsr |= val & 0x0000ff00; // IT<7:2> execution state bits, E bit, A interrupt mask
  }

  if ((bytemask & 1) != 0)
  {
    spsr |= val & 0x000000e0; // I,F interrupt masks, T execution state bit
    if (BadMode(val & 0x0000001f))
    {
      UNPREDICTABLE();
    }
    else
    {
      spsr |= val & 0x0000001f; // Mode bits
    }
  }

  SPSRwrite(context, spsr);
  return;
}

