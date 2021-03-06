.equ INTEGRATOR_SC_BASE , 0x90000000
.equ SC_CRG_CTRL0  ,  (INTEGRATOR_SC_BASE+0x00)
.equ SC_CRG_CTRL3  ,  (INTEGRATOR_SC_BASE+0x0C)
.equ SC_CRG_CTRL6  ,  (INTEGRATOR_SC_BASE+0x18)
.equ SC_CRG_CTRL9  ,  (INTEGRATOR_SC_BASE+0x24)
.equ SC_CRG_CTRL12 ,  (INTEGRATOR_SC_BASE+0x30)
.equ SC_CRG_CTRL13 ,  (INTEGRATOR_SC_BASE+0x34)
.equ SC_CRG_CTRL14 ,  (INTEGRATOR_SC_BASE+0x38)
.equ SC_CRG_CTRL15 ,  (INTEGRATOR_SC_BASE+0x3C)
.equ SC_CRG_CTRL18 ,  (INTEGRATOR_SC_BASE+0x48)
.equ SC_CRG_CTRL20 , (INTEGRATOR_SC_BASE+0x50)
.equ SC_CRG_CTRL21 , (INTEGRATOR_SC_BASE+0x54)
.equ SC_CRG_CTRL22 , (INTEGRATOR_SC_BASE+0x58)
.equ SC_CRG_CTRL25 , (INTEGRATOR_SC_BASE+0x64)
.equ SC_CRG_CTRL28 , (INTEGRATOR_SC_BASE+0x70)
.equ SC_CRG_CTRL31 , (INTEGRATOR_SC_BASE+0x7C)
.equ SC_CRG_CTRL34 , (INTEGRATOR_SC_BASE+0x88)
.equ SC_CRG_CTRL37 , (INTEGRATOR_SC_BASE+0x94)
.equ PWR_CTRL_BASE ,  0x90000C00
.equ PWR_CTRL0     ,  (PWR_CTRL_BASE + 0x00)
.equ PWR_CTRL13    ,  (PWR_CTRL_BASE + 0x34)

.equ SC_CRG_CLKDIV2 , (INTEGRATOR_SC_BASE+0x104)
.equ SC_CRG_CLKDIV3 , (INTEGRATOR_SC_BASE+0x108)
.equ SC_CRG_PLLLOCK_CTRL , (INTEGRATOR_SC_BASE+0x118)
.equ SC_CRG_DFS1_CTRL1 , (INTEGRATOR_SC_BASE+0x200)
.equ SC_CRG_DFS1_CTRL2 , (INTEGRATOR_SC_BASE+0x204)
.equ SC_CRG_DFS1_CTRL3 , (INTEGRATOR_SC_BASE+0x208)
.equ SC_CRG_DFS3_CTRL3 , (INTEGRATOR_SC_BASE+0x220)
.equ SC_CRG_DFS5_CTRL3 , (INTEGRATOR_SC_BASE+0x238)
.equ SC_CRG_CLKDIV6 , (INTEGRATOR_SC_BASE+0x264)
.equ SC_REMAP_CTRL ,   (INTEGRATOR_SC_BASE+0x400)  /* REMAP控制 */
.equ SC_PMU_HOLD_CTRL , (INTEGRATOR_SC_BASE+0x404)  /* PMU HOLD控制 */
.equ SC_IOS_CTRL66 ,   (INTEGRATOR_SC_BASE+0x908)

.equ  UART0_BASE, (INTEGRATOR_SC_BASE+0x7000)
.equ  UART_RBR  , 0x0
.equ  UART_THR  , 0x0
.equ  UART_DLL  , 0x0
.equ  UART_IER  , 0x4
.equ  UART_DLH  , 0x4
.equ  UART_IIR  , 0x8
.equ  UART_FCR  , 0x8
.equ  UART_LCR  , 0xC
.equ  UART_MCR  , 0x10
.equ  UART_LSR  , 0x14

.equ  M3_NVIC_VTOR, 0xE000ED08
.equ  M3_MPU_BASE,  0xE000ED90
.equ  NANDC_MEM_BASE, 0xA0000000

/*IO复用控制*/
.equ  BIT4  ,      (1<<4)
.equ  BIT6  ,      (1<<6)
.equ  BIT10  ,      (1<<10)
.equ  BIT2 ,      (1<<2)

.equ  IOS_SHARE_BASE , 0x90010000
.equ  IOS_MF_CTRL_BASE , 0x00
.equ  IOS_AF_CTRL_BASE , 0x70
.equ  IOS_IOM_CTRL_BASE , 0x100
.equ  IOS_MF_CTRL0 , (IOS_SHARE_BASE + IOS_MF_CTRL_BASE + 0*4)
.equ  IOS_AF_CTRL0 , (IOS_SHARE_BASE + IOS_AF_CTRL_BASE + 0*4)
.equ  IOS_IOM_CTRL51,(IOS_SHARE_BASE + IOS_IOM_CTRL_BASE + 51*4)
.equ  IOS_IOM_CTRL52,(IOS_SHARE_BASE + IOS_IOM_CTRL_BASE + 52*4)

.equ SRAM_ADDR ,                     (0x4FE00000)
.equ SRAM_SIZE ,                     (128*1024)
.equ STACK_TOP ,                     (SRAM_ADDR + SRAM_SIZE - 2*1024 - 4)  /* 2K的共享数据 */

/*平台宏定义*/
.equ PLATFORM_ASIC    ,  1
.equ PLATFORM_PORTING ,  2
.equ PLATFORM_EDA     ,  3
