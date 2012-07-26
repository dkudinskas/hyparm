#ifdef __VM__OMAP_35XX__I2C_H__
#define __VM__OMAP_35XX__I2C_H__

#define TWL_GRP_OFFSET  0x48
#define TWL_GRP_48      0
#define TWL_GRP_49      1
#define TWL_GRP_4a      2
#define TWL_GRP_4b      3
#define TWL_PIH_ISR_P1  0x81
#define TWL_PIH_ISR0    0x1
#define TWL_GPIO0ISR1   0x1
#define TWL_GPIO_ISR1A  0xB1
#define TWL_GPIODATAIN1 0x98
#define TWL_GPIO1_HIGH  0x1

void initTwl4030(int no);
void twl4030_start(u8int address, int read);
void twl4030_stop(void);
u8int twl4030_read(void);
void twl4030_write(u8int value);

enum Twl4030State
{
  IDLE,
  SELECT,
  READ1,
  WRITE1,
  WRITE2
};

struct Twl4030
{
  u8int selected_group;
  u8int selected_reg;
  enum Twl4030State state;
  u8int registers[5][0xFF];
};

#endif /* __VM__OMAP_35XX__I2C_H__ */
