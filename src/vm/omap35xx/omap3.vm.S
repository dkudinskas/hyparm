vm OMAP3 {

  /*
   * Default handlers
   */
  handlers loadFailure storeFailure

  
  /*
   * Device classes
   */

  class GPIO {
    memory - 8k
    handlers loadGpio storeGpio
  }

  class I2C {
    memory - 128
    handlers loadI2c storeI2c
  }

  class MMC {
    memory - 8k
    handlers loadMmc storeMmc
  }

  class UART {
    memory - 8k
    handlers loadUart storeUart
  }


  /*
   * Devices
   */

#ifndef CONFIG_HW_PASSTHROUGH

  device ClockManager {
    memory 0x48004000 16k
    handlers loadClockManager storeClockManager
  }

  device ControlModule {
    memory 0x4830A000 8k
    handlers loadControlModule storeControlModule
  }

  device GPIO1: GPIO {
    memory 0x48310000 -
  }

  device GPIO2: GPIO {
    memory 0x49050000 -
  }

  device GPIO3: GPIO {
    memory 0x49052000 -
  }

  device GPIO4: GPIO {
    memory 0x49054000 -
  }

  device GPIO5: GPIO {
    memory 0x49056000 -
  }

  device GPIO6: GPIO {
    memory 0x49058000 -
  }

#endif /* !CONFIG_HW_PASSTHROUGH */


  device GPMC {
    memory 0x6E000000 16M
    handlers loadGpmc storeGpmc
  }

  
#ifndef CONFIG_HW_PASSTHROUGH

  device GPTimer1 {
    memory 0x48318000 8k
    handlers loadGPTimer storeGPTimer
  }

  device InterruptController {
    memory 0x48200000 8k
    handlers loadIntc storeIntc
  }

#endif  /* !CONFIG_HW_PASSTHROUGH */


  device SRAMInternal {
    memory 0x40200000 64k
    handlers loadSramInternal storeSramInternal
  }

  device ProtectionMechanism {
    memory 0x68010000 17k
    handlers loadProtectionMechanism storeProtectionMechanism
  }


#ifndef CONFIG_HW_PASSTHROUGH

  device PowerResetManager {
    memory 0x48306000 16k
    handlers loadPrm storePrm
  }

  device SystemControlModule {
    memory 0x48002000 8k
    handlers loadSysCtrlModule storeSysCtrlModule
  }

  device SDMA {
    memory 0x48056000 8k
    handlers loadSdma storeSdma
  }

#endif /* !CONFIG_HW_PASSTHROUGH */


  device SDRAM {
    memory 0x80000000 1G
    handlers loadSdram storeSdram
  }

  device SDRC {
    memory 0x6D000000 16M
    handlers loadSdrc storeSdrc
  }

  device SMS {
    memory 0x6C000000 16M
    handlers loadSms storeSms
  }


#ifndef CONFIG_HW_PASSTHROUGH

  device Timer32k {
    memory 0x48320000 8k
    handlers loadTimer32k storeTimer32k
  }

#endif /* !CONFIG_HW_PASSTHROUGH */


  device UART1: UART {
    memory 0x4806a000 -
  }

  device UART2: UART {
    memory 0x4806c000 -
  }

  device UART3: UART {
    memory 0x49020000 -
  }


#ifndef CONFIG_HW_PASSTHROUGH

  device WatchdogTimer2 {
    memory 0x48314000 8k
    handlers loadWDTimer2 storeWDTimer2
  }

#endif /* !CONFIG_HW_PASSTHROUGH */


#if !defined(CONFIG_HW_PASSTHROUGH) && defined(CONFIG_MMC_GUEST_ACCESS)

  device I2C1: I2C {
    memory 0x48070000 -
  }

  device I2C2: I2C {
    memory 0x48072000 -
  }

  device I2C3: I2C {
    memory 0x48060000 -
  }

  device MMC1: MMC {
    memory 0x4809C000 -
  }

  device MMC2: MMC {
    memory 0x480B4000 -
  }

  device MMC3: MMC {
    memory 0x480AD000 -
  }

#endif /* !CONFIG_HW_PASSTHROUGH && CONFIG_MMC_GUEST_ACCESS */


#ifdef CONFIG_PROFILER

  device Profiler {
    memory 0x48000000 8k
    handlers loadProfilerInt storeProfilerInt
  }

#endif /* CONFIG_PROFILER */
}
