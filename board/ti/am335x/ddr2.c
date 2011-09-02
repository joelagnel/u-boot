//Table 1-2: Operating Performance Points

//OPP          VDD    	 ARM (A8)        DDR2  	  L3/L4
 
//SRTurbo   1.26V+/-5% 	 720 MHz	 200 MHz      200/100 MHz
 
//OPP120    1.2V+/-5% 	 650 MHz	 200 MHz      200/100 MHz
 
//OPP100    1.1V+/-5%  	 550 MHz	 200 MHz      200/100 MHz
 
//OPP50     0.95V+/-5%   275 MHz     125 MHz      100/50 MHz
 
//****************************************************
//PRCM register definitions
//****************************************************

#include <common.h>

#define PRCM_BASE_ADDR 	         (0x44E00000)
#define CM_PER_EMIF_CLKCTRL      (PRCM_BASE_ADDR + 0x028)
#define CM_PER_EMIF_FW_CLKCTRL   (PRCM_BASE_ADDR + 0x0D0)

#define CM_AUTOIDLE_DPLL_MPU  (PRCM_BASE_ADDR + 0x41C)
#define CM_IDLEST_DPLL_MPU    (PRCM_BASE_ADDR + 0x420)
#define CM_CLKSEL_DPLL_MPU    (PRCM_BASE_ADDR + 0x42C)
#define CM_AUTOIDLE_DPLL_DDR  (PRCM_BASE_ADDR + 0x430)
#define CM_IDLEST_DPLL_DDR    (PRCM_BASE_ADDR + 0x434)
#define CM_CLKSEL_DPLL_DDR    (PRCM_BASE_ADDR + 0x440)
#define CM_AUTOIDLE_DPLL_DISP (PRCM_BASE_ADDR + 0x444)
#define CM_IDLEST_DPLL_DISP   (PRCM_BASE_ADDR + 0x448)
#define CM_CLKSEL_DPLL_DISP   (PRCM_BASE_ADDR + 0x454)
#define CM_AUTOIDLE_DPLL_CORE (PRCM_BASE_ADDR + 0x458)
#define CM_IDLEST_DPLL_CORE   (PRCM_BASE_ADDR + 0x45C)
#define CM_CLKSEL_DPLL_CORE   (PRCM_BASE_ADDR + 0x468)
#define CM_AUTOIDLE_DPLL_PER  (PRCM_BASE_ADDR + 0x46C)
#define CM_IDLEST_DPLL_PER    (PRCM_BASE_ADDR + 0x470)
#define CM_CLKSEL_DPLL_PER	  (PRCM_BASE_ADDR + 0x49C)

#define CM_DIV_M4_DPLL_CORE   (PRCM_BASE_ADDR + 0x480)
#define CM_DIV_M5_DPLL_CORE	  (PRCM_BASE_ADDR + 0x484)
#define CM_CLKMODE_DPLL_MPU	  (PRCM_BASE_ADDR + 0x488)
#define CM_CLKMODE_DPLL_PER	  (PRCM_BASE_ADDR + 0x48C)
#define CM_CLKMODE_DPLL_CORE  (PRCM_BASE_ADDR + 0x490)
#define CM_CLKMODE_DPLL_DDR	  (PRCM_BASE_ADDR + 0x494)
#define CM_CLKMODE_DPLL_DISP  (PRCM_BASE_ADDR + 0x498)

#define CM_DIV_M2_DPLL_DDR	  (PRCM_BASE_ADDR + 0x4A0)
#define CM_DIV_M2_DPLL_DISP	  (PRCM_BASE_ADDR + 0x4A4)
#define CM_DIV_M2_DPLL_MPU	  (PRCM_BASE_ADDR + 0x4A8)
#define CM_DIV_M2_DPLL_PER	  (PRCM_BASE_ADDR + 0x4AC)
#define CM_DIV_M6_DPLL_CORE	  (PRCM_BASE_ADDR + 0x4D8)

#define CM_CLKOUT_CTRL        (PRCM_BASE_ADDR + 0x700)

#define CONTROL_BASE_ADDR     (0x44E10000)

#define CONTROL_STATUS        (CONTROL_BASE_ADDR + 0x40)
#define CONF_XDMA_EVENT_INTR1 (CONTROL_BASE_ADDR + 0x9b4)


//*****************************************************************************
//Read write prototype
//*****************************************************************************
#define WR_MEM_32(addr, data) *(volatile unsigned int*)(addr) = (unsigned int)(data)
#define RD_MEM_32(addr) 	 *(volatile unsigned int*)(addr)
#define UWORD32 		 volatile unsigned int

//*****************************************************************************
//Global variables
//*****************************************************************************
UWORD32 OPP;
UWORD32 CLKIN;

#define    	SRTurbo  0
#define     OPP120   1
#define     OPP100   2
#define     OPP50    3

//*******************************************************************************/

static void DDR_PLL_Config(UWORD32 CLKIN,UWORD32 N,UWORD32 M,UWORD32 M2);
static void cmd_DDR2_EMIF_Config(UWORD32 REG_PHY_CTRL_SLAVE_RATIO,UWORD32 CMD_REG_PHY_CTRL_SLAVE_FORCE,UWORD32 CMD_REG_PHY_CTRL_SLAVE_DELAY,UWORD32 CMD_PHY_DLL_LOCK_DIFF,UWORD32 CMD_PHY_INVERT_CLKOUT,UWORD32 DATA_PHY_RD_DQS_SLAVE_RATIO,UWORD32 DATA_PHY_WR_DQS_SLAVE_RATIO,UWORD32 DATA_REG_PHY_WRLVL_INIT_RATIO,UWORD32 DATA_REG_PHY_GATELVL_INIT_RATIO,UWORD32 DATA_REG_PHY_FIFO_WE_SLAVE_RATIO,UWORD32 DATA_REG_PHY_WR_DATA_SLAVE_RATIO,UWORD32 RD_LATENCY,UWORD32 TIMING1,UWORD32 TIMING2,UWORD32 TIMING3,UWORD32 SDRAM_CONFIG,UWORD32 REF_CTRL);
static void EMIF_PRCM_CLK_ENABLE(void);
static void VTP_Enable(void);
static void Cmd_Macro_Config(UWORD32 REG_PHY_CTRL_SLAVE_RATIO_value,UWORD32 CMD_REG_PHY_CTRL_SLAVE_FORCE_value,UWORD32 CMD_REG_PHY_CTRL_SLAVE_DELAY_value, UWORD32 PHY_DLL_LOCK_DIFF_value,UWORD32 CMD_PHY_INVERT_CLKOUT_value);
static void Data_Macro_Config(UWORD32 dataMacroNum,UWORD32 PHY_RD_DQS_SLAVE_RATIO_value,UWORD32 PHY_WR_DQS_SLAVE_RATIO_value,UWORD32 REG_PHY_WRLVL_INIT_RATIO_value, UWORD32 REG_PHY_GATELVL_INIT_RATIO_value,UWORD32 REG_PHY_FIFO_WE_SLAVE_RATIO_value,UWORD32 REG_PHY_WR_DATA_SLAVE_RATIO_value);
static void EMIF_MMR_Config(UWORD32 Read_Latency,UWORD32 Timing1,UWORD32 Timing2,UWORD32 Timing3,UWORD32 Sdram_Config,UWORD32 Ref_Ctrl);

static void GetInputClockFrequency(void)
{
	UWORD32 temp;
	
	temp = RD_MEM_32(CONTROL_STATUS) >> 22;
	temp = temp & 0x3;
	if (temp == 0)
	{	
		CLKIN = 19;  //19.2MHz
	}	
	if (temp == 1)
	{
		CLKIN = 24;  //24MHz
	}	
	if (temp == 2)
	{
		CLKIN = 25;  //25MHz
	}	
	if (temp == 3)
	{ 
		CLKIN = 26;  //26MHz
	}	
}

void DDR2_266MHz_Config(void)
{
GetInputClockFrequency();
if(CLKIN==24)
{
DDR_PLL_Config(  CLKIN, 23, 266, 1);
}
}

static void DDR_PLL_Config(UWORD32 CLKIN,UWORD32 N,UWORD32 M,UWORD32 M2)
{
UWORD32 ref_clk,clk_out = 0;
UWORD32 clkmode,clksel,div_m2;

ref_clk     = CLKIN/(N+1);
clk_out     = (ref_clk*M)/M2;

clkmode=RD_MEM_32(CM_CLKMODE_DPLL_DDR);
clksel= RD_MEM_32(CM_CLKSEL_DPLL_DDR);
div_m2= RD_MEM_32(CM_DIV_M2_DPLL_DDR);

clkmode =(clkmode&0xfffffff8)|0x00000004;
WR_MEM_32(CM_CLKMODE_DPLL_DDR,clkmode);
while((RD_MEM_32(CM_IDLEST_DPLL_DDR) & 0x00000100 )!=0x00000100);


clksel = clksel & (~0x7FFFF);
clksel = clksel | ((M <<0x8) | N);
WR_MEM_32(CM_CLKSEL_DPLL_DDR,clksel);
div_m2 = RD_MEM_32(CM_DIV_M2_DPLL_DDR);
div_m2 = (div_m2&0xFFFFFFE0) | M2;
WR_MEM_32(CM_DIV_M2_DPLL_DDR,div_m2);
clkmode =(clkmode&0xfffffff8)|0x00000007;
WR_MEM_32(CM_CLKMODE_DPLL_DDR,clkmode);


while((RD_MEM_32(CM_IDLEST_DPLL_DDR) & 0x00000001 )!=0x00000001);

}

//##############################################################################
//##############################################################################
//                       DDR Configuration Section
//##############################################################################
//##############################################################################

/******************************************************************************
SDRAM Types supported
******************************************************************************/
#define    mDDR     0
#define    DDR2     1
#define    DDR3     2
//#define    SDRAM    mDDR

//********************************************************************
//EMIF4DC registers
//********************************************************************
#define EMIF_BASE_ADDR                    0x4C000000
#define EMIF_STATUS_REG                   (EMIF_BASE_ADDR + 0x004)
#define EMIF_SDRAM_CONFIG_REG             (EMIF_BASE_ADDR + 0x008)
#define EMIF_SDRAM_CONFIG_2_REG           (EMIF_BASE_ADDR + 0x00C)
#define EMIF_SDRAM_REF_CTRL_REG           (EMIF_BASE_ADDR + 0x010)
#define EMIF_SDRAM_REF_CTRL_SHDW_REG      (EMIF_BASE_ADDR + 0x014)
#define EMIF_SDRAM_TIM_1_REG              (EMIF_BASE_ADDR + 0x018)
#define EMIF_SDRAM_TIM_1_SHDW_REG         (EMIF_BASE_ADDR + 0x01C)
#define EMIF_SDRAM_TIM_2_REG              (EMIF_BASE_ADDR + 0x020)
#define EMIF_SDRAM_TIM_2_SHDW_REG         (EMIF_BASE_ADDR + 0x024)
#define EMIF_SDRAM_TIM_3_REG              (EMIF_BASE_ADDR + 0x028)
#define EMIF_SDRAM_TIM_3_SHDW_REG         (EMIF_BASE_ADDR + 0x02C)
#define EMIF_LPDDR2_NVM_TIM_REG           (EMIF_BASE_ADDR + 0x030)
#define EMIF_LPDDR2_NVM_TIM_SHDW_REG      (EMIF_BASE_ADDR + 0x034)
#define EMIF_PWR_MGMT_CTRL_REG            (EMIF_BASE_ADDR + 0x038)
#define EMIF_PWR_MGMT_CTRL_SHDW_REG       (EMIF_BASE_ADDR + 0x03C)
#define EMIF_LPDDR2_MODE_REG_DATA_REG     (EMIF_BASE_ADDR + 0x040)
#define EMIF_LPDDR2_MODE_REG_CFG_REG      (EMIF_BASE_ADDR + 0x050)
#define EMIF_OCP_CONFIG_REG               (EMIF_BASE_ADDR + 0x054)
#define EMIF_OCP_CFG_VAL_1_REG            (EMIF_BASE_ADDR + 0x058)
#define EMIF_OCP_CFG_VAL_2_REG            (EMIF_BASE_ADDR + 0x05C)
#define EMIF_IODFT_TLGC_REG               (EMIF_BASE_ADDR + 0x060)
#define EMIF_IODFT_CTRL_MISR_RSLT_REG     (EMIF_BASE_ADDR + 0x064)
#define EMIF_IODFT_ADDR_MISR_RSLT_REG     (EMIF_BASE_ADDR + 0x068)
#define EMIF_IODFT_DATA_MISR_RSLT_1_REG   (EMIF_BASE_ADDR + 0x06C)
#define EMIF_IODFT_DATA_MISR_RSLT_2_REG   (EMIF_BASE_ADDR + 0x070)
#define EMIF_IODFT_DATA_MISR_RSLT_3_REG   (EMIF_BASE_ADDR + 0x074)
#define EMIF_PERF_CNT_1_REG               (EMIF_BASE_ADDR + 0x080)
#define EMIF_PERF_CNT_2_REG               (EMIF_BASE_ADDR + 0x084)
#define EMIF_PERF_CNT_CFG_REG             (EMIF_BASE_ADDR + 0x088)
#define EMIF_PERF_CNT_SEL_REG             (EMIF_BASE_ADDR + 0x08C)
#define EMIF_PERF_CNT_TIM_REG             (EMIF_BASE_ADDR + 0x090)
#define EMIF_READ_IDLE_CTRL_REG           (EMIF_BASE_ADDR + 0x098)
#define EMIF_READ_IDLE_CTRL_SHDW_REG      (EMIF_BASE_ADDR + 0x09C)
#define EMIF_IRQ_EOI_REG                  (EMIF_BASE_ADDR + 0x0A0)
#define EMIF_IRQSTATUS_RAW_SYS_REG        (EMIF_BASE_ADDR + 0x0A4)
#define EMIF_IRQSTATUS_SYS_REG            (EMIF_BASE_ADDR + 0x0AC)
#define EMIF_IRQENABLE_SET_SYS_REG        (EMIF_BASE_ADDR + 0x0B4)
#define EMIF_IRQENABLE_CLR_SYS_REG        (EMIF_BASE_ADDR + 0x0BC)
#define EMIF_ZQ_CONFIG_REG                (EMIF_BASE_ADDR + 0x0C8)
#define EMIF_TEMP_ALERT_CONFIG_REG        (EMIF_BASE_ADDR + 0x0CC)
#define EMIF_OCP_ERR_LOG_REG              (EMIF_BASE_ADDR + 0x0D0)
#define EMIF_RDWR_LVL_RMP_WIN_REG         (EMIF_BASE_ADDR + 0x0D4)
#define EMIF_RDWR_LVL_RMP_CTRL_REG        (EMIF_BASE_ADDR + 0x0D8)
#define EMIF_RDWR_LVL_CTRL_REG            (EMIF_BASE_ADDR + 0x0DC)
#define EMIF_DDR_PHY_CTRL_1_REG           (EMIF_BASE_ADDR + 0x0E4)
#define EMIF_DDR_PHY_CTRL_1_SHDW_REG      (EMIF_BASE_ADDR + 0x0E8)
#define EMIF_DDR_PHY_CTRL_2_REG           (EMIF_BASE_ADDR + 0x0EC)
#define EMIF_PRI_COS_MAP_REG              (EMIF_BASE_ADDR + 0x100)
#define EMIF_CONNID_COS_1_MAP_REG         (EMIF_BASE_ADDR + 0x104)
#define EMIF_CONNID_COS_2_MAP_REG         (EMIF_BASE_ADDR + 0x108)
#define EMIF_RD_WR_EXEC_THRSH_REG         (EMIF_BASE_ADDR + 0x120)


//*******************************************************************
//DDR PHY registers
//*******************************************************************
#define    DDR_PHY_BASE_ADDR                                   (0x44E12000)  
//CMD0
#define    CMD0_REG_PHY_CTRL_SLAVE_RATIO_0		     	       (0x01C + DDR_PHY_BASE_ADDR)
#define    CMD0_REG_PHY_CTRL_SLAVE_FORCE_0                     (0x020 + DDR_PHY_BASE_ADDR)
#define    CMD0_REG_PHY_CTRL_SLAVE_DELAY_0                     (0x024 + DDR_PHY_BASE_ADDR)
#define    CMD0_REG_PHY_DLL_LOCK_DIFF_0   	             	   (0x028 + DDR_PHY_BASE_ADDR)
#define    CMD0_REG_PHY_INVERT_CLKOUT_0  	             	   (0x02C + DDR_PHY_BASE_ADDR)
#define    CMD0_PHY_REG_STATUS_PHY_CTRL_DLL_LOCK_0             (0x030 + DDR_PHY_BASE_ADDR)
#define    CMD0_PHY_REG_STATUS_PHY_CTRL_OF_IN_LOCK_STATE_0     (0x034 + DDR_PHY_BASE_ADDR)
#define    CMD0_PHY_REG_STATUS_PHY_CTRL_OF_IN_DELAY_VALUE_0    (0x038 + DDR_PHY_BASE_ADDR)
#define    CMD0_PHY_REG_STATUS_PHY_CTRL_OF_OUT_DELAY_VALUE_0   (0x03C + DDR_PHY_BASE_ADDR)

//CMD1
#define    CMD1_REG_PHY_CTRL_SLAVE_RATIO_0		     	       (0x050 + DDR_PHY_BASE_ADDR)
#define    CMD1_REG_PHY_CTRL_SLAVE_FORCE_0                     (0x054 + DDR_PHY_BASE_ADDR)
#define    CMD1_REG_PHY_CTRL_SLAVE_DELAY_0                     (0x058 + DDR_PHY_BASE_ADDR)
#define    CMD1_REG_PHY_DLL_LOCK_DIFF_0   	             	   (0x05C + DDR_PHY_BASE_ADDR)
#define    CMD1_REG_PHY_INVERT_CLKOUT_0  	             	   (0x060 + DDR_PHY_BASE_ADDR)
#define    CMD1_PHY_REG_STATUS_PHY_CTRL_DLL_LOCK_0             (0x064 + DDR_PHY_BASE_ADDR)
#define    CMD1_PHY_REG_STATUS_PHY_CTRL_OF_IN_LOCK_STATE_0     (0x068 + DDR_PHY_BASE_ADDR)
#define    CMD1_PHY_REG_STATUS_PHY_CTRL_OF_IN_DELAY_VALUE_0    (0x06C + DDR_PHY_BASE_ADDR)
#define    CMD1_PHY_REG_STATUS_PHY_CTRL_OF_OUT_DELAY_VALUE_0   (0x070 + DDR_PHY_BASE_ADDR)

//CMD2
#define    CMD2_REG_PHY_CTRL_SLAVE_RATIO_0	             	   (0x084 + DDR_PHY_BASE_ADDR)
#define    CMD2_REG_PHY_CTRL_SLAVE_FORCE_0                     (0x088 + DDR_PHY_BASE_ADDR)
#define    CMD2_REG_PHY_CTRL_SLAVE_DELAY_0                     (0x08C + DDR_PHY_BASE_ADDR)
#define    CMD2_REG_PHY_DLL_LOCK_DIFF_0   	             	   (0x090 + DDR_PHY_BASE_ADDR)
#define    CMD2_REG_PHY_INVERT_CLKOUT_0  	             	   (0x094 + DDR_PHY_BASE_ADDR)
#define    CMD2_PHY_REG_STATUS_PHY_CTRL_DLL_LOCK_0             (0x098 + DDR_PHY_BASE_ADDR)
#define    CMD2_PHY_REG_STATUS_PHY_CTRL_OF_IN_LOCK_STATE_0     (0x09C + DDR_PHY_BASE_ADDR)
#define    CMD2_PHY_REG_STATUS_PHY_CTRL_OF_IN_DELAY_VALUE_0    (0x0A0 + DDR_PHY_BASE_ADDR)
#define    CMD2_PHY_REG_STATUS_PHY_CTRL_OF_OUT_DELAY_VALUE_0   (0x0A4 + DDR_PHY_BASE_ADDR)

//DATA0
#define	   DATA0_REG_PHY_DATA_SLICE_IN_USE_0	               (0x0B8 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_DIS_CALIB_RST_0	                   (0x0BC + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RDC_FIFO_RST_ERR_CNT_CLR_0	           (0x0C0 + DDR_PHY_BASE_ADDR)
#define	   DATA0_PHY_RDC_FIFO_RST_ERR_CNT_0 	               (0x0C4 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0           	   (0x0C8 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_1                  (0x0CC + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RD_DQS_SLAVE_FORCE_0                  (0x0D0 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RD_DQS_SLAVE_DELAY_0           	   (0x0D4 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_RD_DQS_SLAVE_DELAY_1                  (0x0D8 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0           	   (0x0DC + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_1                  (0x0E0 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WR_DQS_SLAVE_FORCE_0                  (0x0E4 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WR_DQS_SLAVE_DELAY_0           	   (0x0E8 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WR_DQS_SLAVE_DELAY_1                  (0x0EC + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WRLVL_INIT_RATIO_0             	   (0x0F0 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WRLVL_INIT_RATIO_1             	   (0x0F4 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_WRLVL_INIT_MODE_0             	       (0x0F8 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_GATELVL_INIT_RATIO_0             	   (0x0FC + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_GATELVL_INIT_RATIO_1             	   (0x100 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_GATELVL_INIT_MODE_0           	       (0x104 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0           	   (0x108 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_1           	   (0x10C + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_FIFO_WE_IN_FORCE_0           	       (0x110 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_FIFO_WE_IN_DELAY_0           	       (0x114 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_FIFO_WE_IN_DELAY_1           	       (0x118 + DDR_PHY_BASE_ADDR)
#define	   DATA0_REG_PHY_DQ_OFFSET_0           	               (0x11C + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0                 (0x120 + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_1                 (0x124 + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_WR_DATA_SLAVE_FORCE_0                 (0x128 + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_WR_DATA_SLAVE_DELAY_0                 (0x12C + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_WR_DATA_SLAVE_DELAY_1                 (0x130 + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_USE_RANK0_DELAYS_0                    (0x134 + DDR_PHY_BASE_ADDR)
#define    DATA0_REG_PHY_DLL_LOCK_DIFF_0                       (0x138 + DDR_PHY_BASE_ADDR)
#define    DATA0_PHY_REG_STATUS_DLL_LOCK_0                     (0x13C + DDR_PHY_BASE_ADDR)
#define    DATA0_PHY_REG_STATUS_OF_IN_LOCK_STATE_0             (0x140 + DDR_PHY_BASE_ADDR)
#define    DATA0_PHY_REG_STATUS_OF_IN_DELAY_VALUE_0            (0x144 + DDR_PHY_BASE_ADDR)
#define    DATA0_PHY_REG_STATUS_OF_OUT_DELAY_VALUE_0           (0x148 + DDR_PHY_BASE_ADDR)

//DATA1
#define    DATA1_REG_PHY_DATA_SLICE_IN_USE_0	               (0x15C + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_DIS_CALIB_RST_0	                   (0x160 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RDC_FIFO_RST_ERR_CNT_CLR_0	           (0x164 + DDR_PHY_BASE_ADDR)
#define    DATA1_PHY_RDC_FIFO_RST_ERR_CNT_0 	               (0x168 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0           	   (0x16C + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_1                  (0x170 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RD_DQS_SLAVE_FORCE_0                  (0x174 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RD_DQS_SLAVE_DELAY_0           	   (0x178 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_RD_DQS_SLAVE_DELAY_1                  (0x17C + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0           	   (0x180 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_1                  (0x184 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DQS_SLAVE_FORCE_0                  (0x188 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DQS_SLAVE_DELAY_0           	   (0x18C + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DQS_SLAVE_DELAY_1                  (0x190 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WRLVL_INIT_RATIO_0             	   (0x194 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WRLVL_INIT_RATIO_1             	   (0x198 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WRLVL_INIT_MODE_0             	       (0x19C + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_GATELVL_INIT_RATIO_0             	   (0x1A0 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_GATELVL_INIT_RATIO_1             	   (0x1A4 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_GATELVL_INIT_MODE_0           	       (0x1A8 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0           	   (0x1AC + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_1           	   (0x1B0 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_FIFO_WE_IN_FORCE_0           	       (0x1B4 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_FIFO_WE_IN_DELAY_0           	       (0x1B8 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_FIFO_WE_IN_DELAY_1           	       (0x1BC + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_DQ_OFFSET_0           	               (0x1C0 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0                 (0x1C4 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_1                 (0x1C8 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DATA_SLAVE_FORCE_0                 (0x1CC + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DATA_SLAVE_DELAY_0                 (0x1D0 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_WR_DATA_SLAVE_DELAY_1                 (0x1D4 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_USE_RANK0_DELAYS_0                    (0x1D8 + DDR_PHY_BASE_ADDR)
#define    DATA1_REG_PHY_DLL_LOCK_DIFF_0                       (0x1DC + DDR_PHY_BASE_ADDR)
#define    DATA1_PHY_REG_STATUS_DLL_LOCK_0                     (0x1E0 + DDR_PHY_BASE_ADDR)
#define    DATA1_PHY_REG_STATUS_OF_IN_LOCK_STATE_0             (0x1E4 + DDR_PHY_BASE_ADDR)
#define    DATA1_PHY_REG_STATUS_OF_IN_DELAY_VALUE_0            (0x1E8 + DDR_PHY_BASE_ADDR)
#define    DATA1_PHY_REG_STATUS_OF_OUT_DELAY_VALUE_0           (0x1EC + DDR_PHY_BASE_ADDR)

//FIFO
#define FIFO_WE_OUT0_IO_CONFIG_I_0                             (0x338 + DDR_PHY_BASE_ADDR)
#define FIFO_WE_OUT0_IO_CONFIG_SR_0                            (0x33C + DDR_PHY_BASE_ADDR)
#define FIFO_WE_OUT1_IO_CONFIG_I_0                             (0x340 + DDR_PHY_BASE_ADDR)
#define FIFO_WE_OUT1_IO_CONFIG_SR_0                            (0x344 + DDR_PHY_BASE_ADDR)
#define FIFO_WE_IN2_IO_CONFIG_I_0                              (0x348 + DDR_PHY_BASE_ADDR)
#define FIFO_WE_IN2_IO_CONFIG_SR_0                             (0x34C + DDR_PHY_BASE_ADDR)
#define FIFO_WE_IN3_IO_CONFIG_I_0                              (0x350 + DDR_PHY_BASE_ADDR)
#define FIFO_WE_IN3_IO_CONFIG_SR_0                             (0x354 + DDR_PHY_BASE_ADDR)

//Leveling
#define DATA0_REG_PHY_WRLVL_NUM_OF_DQ0                         (0x35C + DDR_PHY_BASE_ADDR)
#define DATA0_REG_PHY_GATELVL_NUM_OF_DQ0                       (0x360 + DDR_PHY_BASE_ADDR)
#define DATA1_REG_PHY_WRLVL_NUM_OF_DQ0                         (0x364 + DDR_PHY_BASE_ADDR)
#define DATA1_REG_PHY_GATELVL_NUM_OF_DQ0                       (0x368 + DDR_PHY_BASE_ADDR)

//*******************************************************************
//DDR IO Control Registers
//*******************************************************************
#define CONTROL_BASE_ADDR (0x44E10000)

#define	   DDR_IO_CTRL	      	 (CONTROL_BASE_ADDR + 0x0E04) //must be set to 1 for DDR interface
#define    VTP_CTRL_REG          (CONTROL_BASE_ADDR + 0x0E0C) 
#define    DDR_CKE_CTRL          (CONTROL_BASE_ADDR + 0x131C) //CKE control by EMIF/DDR PHY,
#define	   DDR_CMD0_IOCTRL       (CONTROL_BASE_ADDR + 0x1404)
#define	   DDR_CMD1_IOCTRL       (CONTROL_BASE_ADDR + 0x1408)
#define	   DDR_CMD2_IOCTRL       (CONTROL_BASE_ADDR + 0x140C)
#define	   DDR_DATA0_IOCTRL      (CONTROL_BASE_ADDR + 0x1440)
#define	   DDR_DATA1_IOCTRL      (CONTROL_BASE_ADDR + 0x1444)

#define    DDR_IOCTRL_VALUE      (0x18B)

//******************************************************************* 
//Common DDR PHY parameters
//*******************************************************************

#define    PHY_REG_USE_RANK0_DELAY                   0x01
#define    CMD_REG_PHY_CTRL_SLAVE_DELAY_DEFINE       0x00
#define    CMD_REG_PHY_CTRL_SLAVE_FORCE_DEFINE       0x00
#define    DATA_MACRO_0                               0
#define    DATA_MACRO_1                               1
#define    PHY_DLL_LOCK_DIFF_DEFINE 	             0x0     //0x0f for maximum

//******************************************************************
//DDR2 parameters
//******************************************************************
#define  DDR2_REG_PHY_CTRL_SLAVE_RATIO_DEFINE          0x80
#define  DDR2_PHY_RD_DQS_SLAVE_RATIO_DEFINE            0x12
#define  DDR2_PHY_WR_DQS_SLAVE_RATIO_DEFINE            0
#define  DDR2_REG_PHY_WRLVL_INIT_RATIO_DEFINE          0
#define  DDR2_REG_PHY_GATELVL_INIT_RATIO_DEFINE        0
#define  DDR2_REG_PHY_FIFO_WE_SLAVE_RATIO_DEFINE       0x80
#define  DDR2_REG_PHY_WR_DATA_SLAVE_RATIO_DEFINE       0x40
#define  DDR2_PHY_DLL_LOCK_DIFF_DEFINE                 0x0    
#define  DDR2_PHY_INVERT_CLKOUT_DEFINE                 0x0

//******************************************************************
//DDR2=200MHz
//SRTurbo
//*****************************************************************
//DDR2
#define SRTurbo_DDR2_READ_LATENCY    0x09       //CL=3 Max=(CL+7)-1 ie.10-1=9  Min=(CL+1)-1 ie.4-1=3
#define SRTurbo_DDR2_SDRAM_TIMING1   0x04447289   
#define SRTurbo_DDR2_SDRAM_TIMING2   0x241B31CA   
#define SRTurbo_DDR2_SDRAM_TIMING3   0x0000019F
#define SRTurbo_DDR2_SDRAM_CONFIG    0x40805632
#define SRTurbo_DDR2_REF_CTRL        0x00000618

//******************************************************************
//DDR2=200MHz
//OPP120
//******************************************************************

//DDR2
#define OPP120_DDR2_READ_LATENCY    0x09       //CL=3 Max=(CL+7)-1 ie.10-1=9  Min=(CL+1)-1 ie.4-1=3
#define OPP120_DDR2_SDRAM_TIMING1   0x04447289   
#define OPP120_DDR2_SDRAM_TIMING2   0x241B31CA   
#define OPP120_DDR2_SDRAM_TIMING3   0x0000019F
#define OPP120_DDR2_SDRAM_CONFIG    0x40805632
#define OPP120_DDR2_REF_CTRL        0x00000618

//******************************************************************
//DDR2=200MHz
//OPP100
//******************************************************************

#define OPP100_DDR2_READ_LATENCY                0x04    
#define OPP100_DDR2_SDRAM_TIMING1               0x0666B3D6
#define OPP100_DDR2_SDRAM_TIMING2               0x143731DA 
#define OPP100_DDR2_SDRAM_TIMING3               0x00000347 
//#define OPP100_DDR2_SDRAM_CONFIG                0x40805032
#define OPP100_DDR2_SDRAM_CONFIG                0x40805332
//#define OPP100_DDR2_REF_CTRL                    0x0000030C
#define OPP100_DDR2_REF_CTRL                    0x0000081a

//******************************************************************
//DDR2=125MHz
//OPP50
//******************************************************************
//DDR2
#define OPP50_DDR2_READ_LATENCY    0x09    //CL=3 Max=(CL+7)-1 ie.10-1=9  Min=(CL+1)-1 ie.4-1=3
#define OPP50_DDR2_SDRAM_TIMING1   0x02224188   
#define OPP50_DDR2_SDRAM_TIMING2   0x240F31C2   
#define OPP50_DDR2_SDRAM_TIMING3   0x000000FF
#define OPP50_DDR2_SDRAM_CONFIG    0x40805632
#define OPP50_DDR2_REF_CTRL        0x000003CF

void DDR2_EMIF_Config(void)
{
		
	cmd_DDR2_EMIF_Config(DDR2_REG_PHY_CTRL_SLAVE_RATIO_DEFINE,CMD_REG_PHY_CTRL_SLAVE_FORCE_DEFINE,CMD_REG_PHY_CTRL_SLAVE_DELAY_DEFINE,DDR2_PHY_DLL_LOCK_DIFF_DEFINE,DDR2_PHY_INVERT_CLKOUT_DEFINE,DDR2_PHY_RD_DQS_SLAVE_RATIO_DEFINE,DDR2_PHY_WR_DQS_SLAVE_RATIO_DEFINE,DDR2_REG_PHY_WRLVL_INIT_RATIO_DEFINE,DDR2_REG_PHY_GATELVL_INIT_RATIO_DEFINE,DDR2_REG_PHY_FIFO_WE_SLAVE_RATIO_DEFINE,DDR2_REG_PHY_WR_DATA_SLAVE_RATIO_DEFINE,OPP100_DDR2_READ_LATENCY,OPP100_DDR2_SDRAM_TIMING1,OPP100_DDR2_SDRAM_TIMING2,
	OPP100_DDR2_SDRAM_TIMING3,OPP100_DDR2_SDRAM_CONFIG,OPP100_DDR2_REF_CTRL);
}

static void cmd_DDR2_EMIF_Config(UWORD32 REG_PHY_CTRL_SLAVE_RATIO,UWORD32 CMD_REG_PHY_CTRL_SLAVE_FORCE,UWORD32 CMD_REG_PHY_CTRL_SLAVE_DELAY,UWORD32 CMD_PHY_DLL_LOCK_DIFF,UWORD32 CMD_PHY_INVERT_CLKOUT,UWORD32 DATA_PHY_RD_DQS_SLAVE_RATIO,UWORD32 DATA_PHY_WR_DQS_SLAVE_RATIO,UWORD32 DATA_REG_PHY_WRLVL_INIT_RATIO,UWORD32 DATA_REG_PHY_GATELVL_INIT_RATIO,UWORD32 DATA_REG_PHY_FIFO_WE_SLAVE_RATIO,UWORD32 DATA_REG_PHY_WR_DATA_SLAVE_RATIO,UWORD32 RD_LATENCY,UWORD32 TIMING1,UWORD32 TIMING2,UWORD32 TIMING3,UWORD32 SDRAM_CONFIG,UWORD32 REF_CTRL)
{
	EMIF_PRCM_CLK_ENABLE();


	//Enable VTP
	VTP_Enable();
 
	Cmd_Macro_Config(REG_PHY_CTRL_SLAVE_RATIO,CMD_REG_PHY_CTRL_SLAVE_FORCE,CMD_REG_PHY_CTRL_SLAVE_DELAY,CMD_PHY_DLL_LOCK_DIFF,CMD_PHY_INVERT_CLKOUT);	
	Data_Macro_Config(DATA_MACRO_0,DATA_PHY_RD_DQS_SLAVE_RATIO,DATA_PHY_WR_DQS_SLAVE_RATIO,DATA_REG_PHY_WRLVL_INIT_RATIO,
			  DATA_REG_PHY_GATELVL_INIT_RATIO,DATA_REG_PHY_FIFO_WE_SLAVE_RATIO,DATA_REG_PHY_WR_DATA_SLAVE_RATIO);
	Data_Macro_Config(DATA_MACRO_1,DATA_PHY_RD_DQS_SLAVE_RATIO,DATA_PHY_WR_DQS_SLAVE_RATIO,DATA_REG_PHY_WRLVL_INIT_RATIO,
			  DATA_REG_PHY_GATELVL_INIT_RATIO,DATA_REG_PHY_FIFO_WE_SLAVE_RATIO,DATA_REG_PHY_WR_DATA_SLAVE_RATIO);
	
	WR_MEM_32(DATA0_REG_PHY_USE_RANK0_DELAYS_0,PHY_REG_USE_RANK0_DELAY);
	WR_MEM_32(DATA1_REG_PHY_USE_RANK0_DELAYS_0,PHY_REG_USE_RANK0_DELAY);

	//set IO control registers
	WR_MEM_32(DDR_CMD0_IOCTRL,DDR_IOCTRL_VALUE);
	WR_MEM_32(DDR_CMD1_IOCTRL,DDR_IOCTRL_VALUE);
	WR_MEM_32(DDR_CMD2_IOCTRL,DDR_IOCTRL_VALUE);
	WR_MEM_32(DDR_DATA0_IOCTRL,DDR_IOCTRL_VALUE);
	WR_MEM_32(DDR_DATA1_IOCTRL,DDR_IOCTRL_VALUE);
	
        //IO to work for DDR2
        WR_MEM_32(DDR_IO_CTRL, RD_MEM_32(DDR_IO_CTRL) & ~0x10000000 );

	 //CKE controlled by EMIF/DDR_PHY
        WR_MEM_32(DDR_CKE_CTRL, RD_MEM_32(DDR_CKE_CTRL) | 0x00000001);
	
	EMIF_MMR_Config(RD_LATENCY,TIMING1,TIMING2,TIMING3,SDRAM_CONFIG,REF_CTRL);
	
	if((RD_MEM_32(EMIF_STATUS_REG) & 0x4) == 0x4)
	{
	}

	
}


static void EMIF_PRCM_CLK_ENABLE()
{
   	   /* Enable EMIF4DC Firewall clocks*/
   	   WR_MEM_32(CM_PER_EMIF_FW_CLKCTRL,0x02);
   	   /* Enable EMIF4DC clocks*/
           WR_MEM_32(CM_PER_EMIF_CLKCTRL,0x02);
   	   /* Poll for module is functional */
   	   while(RD_MEM_32(CM_PER_EMIF_CLKCTRL)!= 0x02);
}

/*************************************************************************
* Cmd_Macro_Config() routine                                             *
*************************************************************************/
static void Cmd_Macro_Config(UWORD32 REG_PHY_CTRL_SLAVE_RATIO_value,UWORD32 CMD_REG_PHY_CTRL_SLAVE_FORCE_value,UWORD32 CMD_REG_PHY_CTRL_SLAVE_DELAY_value, UWORD32 PHY_DLL_LOCK_DIFF_value,UWORD32 CMD_PHY_INVERT_CLKOUT_value)
{

	   WR_MEM_32(CMD0_REG_PHY_CTRL_SLAVE_RATIO_0,REG_PHY_CTRL_SLAVE_RATIO_value);		     	
       WR_MEM_32(CMD0_REG_PHY_CTRL_SLAVE_FORCE_0,CMD_REG_PHY_CTRL_SLAVE_FORCE_value);                   	
       WR_MEM_32(CMD0_REG_PHY_CTRL_SLAVE_DELAY_0,CMD_REG_PHY_CTRL_SLAVE_DELAY_value);                   	
	   WR_MEM_32(CMD0_REG_PHY_DLL_LOCK_DIFF_0,PHY_DLL_LOCK_DIFF_value);   	             	
	   WR_MEM_32(CMD0_REG_PHY_INVERT_CLKOUT_0,CMD_PHY_INVERT_CLKOUT_value);


	   WR_MEM_32(CMD1_REG_PHY_CTRL_SLAVE_RATIO_0,REG_PHY_CTRL_SLAVE_RATIO_value);		     	
       WR_MEM_32(CMD1_REG_PHY_CTRL_SLAVE_FORCE_0,CMD_REG_PHY_CTRL_SLAVE_FORCE_value);                   	
       WR_MEM_32(CMD1_REG_PHY_CTRL_SLAVE_DELAY_0,CMD_REG_PHY_CTRL_SLAVE_DELAY_value);                   	
	   WR_MEM_32(CMD1_REG_PHY_DLL_LOCK_DIFF_0,PHY_DLL_LOCK_DIFF_value);   	             	
	   WR_MEM_32(CMD1_REG_PHY_INVERT_CLKOUT_0,CMD_PHY_INVERT_CLKOUT_value);


	   WR_MEM_32(CMD2_REG_PHY_CTRL_SLAVE_RATIO_0,REG_PHY_CTRL_SLAVE_RATIO_value);		     	
       WR_MEM_32(CMD2_REG_PHY_CTRL_SLAVE_FORCE_0,CMD_REG_PHY_CTRL_SLAVE_FORCE_value);                   	
       WR_MEM_32(CMD2_REG_PHY_CTRL_SLAVE_DELAY_0,CMD_REG_PHY_CTRL_SLAVE_DELAY_value);                   	
	   WR_MEM_32(CMD2_REG_PHY_DLL_LOCK_DIFF_0,PHY_DLL_LOCK_DIFF_value);   	             	
	   WR_MEM_32(CMD2_REG_PHY_INVERT_CLKOUT_0,CMD_PHY_INVERT_CLKOUT_value); 	             	
}

/*************************************************************************
* Data_Macro_Config() routine                                            *
*************************************************************************/
static void Data_Macro_Config(UWORD32 dataMacroNum,UWORD32 PHY_RD_DQS_SLAVE_RATIO_value,UWORD32 PHY_WR_DQS_SLAVE_RATIO_value,UWORD32 REG_PHY_WRLVL_INIT_RATIO_value, UWORD32 REG_PHY_GATELVL_INIT_RATIO_value,UWORD32 REG_PHY_FIFO_WE_SLAVE_RATIO_value,UWORD32 REG_PHY_WR_DATA_SLAVE_RATIO_value)
{
        UWORD32 BaseAddrOffset = 0;
	if(dataMacroNum == DATA_MACRO_0)
	{
	  BaseAddrOffset = 0x00;
	}
	else if(dataMacroNum == DATA_MACRO_1)
	{
	  BaseAddrOffset = 0xA4;
	}	  
	
	  WR_MEM_32((DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0 + BaseAddrOffset),((PHY_RD_DQS_SLAVE_RATIO_value<<30)|(PHY_RD_DQS_SLAVE_RATIO_value<<20)|(PHY_RD_DQS_SLAVE_RATIO_value<<10)|(PHY_RD_DQS_SLAVE_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_1 + BaseAddrOffset),PHY_RD_DQS_SLAVE_RATIO_value>>2);

      WR_MEM_32((DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0 + BaseAddrOffset),((PHY_WR_DQS_SLAVE_RATIO_value<<30)|(PHY_WR_DQS_SLAVE_RATIO_value<<20)|(PHY_WR_DQS_SLAVE_RATIO_value<<10)|(PHY_WR_DQS_SLAVE_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_1 + BaseAddrOffset),PHY_WR_DQS_SLAVE_RATIO_value>>2);

	  WR_MEM_32((DATA0_REG_PHY_WRLVL_INIT_RATIO_0 + BaseAddrOffset),((REG_PHY_WRLVL_INIT_RATIO_value<<30)|(REG_PHY_WRLVL_INIT_RATIO_value<<20)|(REG_PHY_WRLVL_INIT_RATIO_value<<10)|(REG_PHY_WRLVL_INIT_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_WRLVL_INIT_RATIO_1 + BaseAddrOffset),REG_PHY_WRLVL_INIT_RATIO_value>>2);

      WR_MEM_32((DATA0_REG_PHY_GATELVL_INIT_RATIO_0 + BaseAddrOffset),((REG_PHY_GATELVL_INIT_RATIO_value<<30)|(REG_PHY_GATELVL_INIT_RATIO_value<<20)|(REG_PHY_GATELVL_INIT_RATIO_value<<10)|(REG_PHY_GATELVL_INIT_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_GATELVL_INIT_RATIO_1 + BaseAddrOffset),REG_PHY_GATELVL_INIT_RATIO_value>>2);

	  WR_MEM_32((DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0 + BaseAddrOffset),((REG_PHY_FIFO_WE_SLAVE_RATIO_value<<30)|(REG_PHY_FIFO_WE_SLAVE_RATIO_value<<20)|(REG_PHY_FIFO_WE_SLAVE_RATIO_value<<10)|(REG_PHY_FIFO_WE_SLAVE_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_1 + BaseAddrOffset),REG_PHY_FIFO_WE_SLAVE_RATIO_value>>2);

	  WR_MEM_32((DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0 + BaseAddrOffset),((REG_PHY_WR_DATA_SLAVE_RATIO_value<<30)|(REG_PHY_WR_DATA_SLAVE_RATIO_value<<20)|(REG_PHY_WR_DATA_SLAVE_RATIO_value<<10)|(REG_PHY_WR_DATA_SLAVE_RATIO_value<<0)));
	  WR_MEM_32((DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_1 + BaseAddrOffset),REG_PHY_WR_DATA_SLAVE_RATIO_value>>2);	  

	  WR_MEM_32((DATA0_REG_PHY_DLL_LOCK_DIFF_0 + BaseAddrOffset),PHY_DLL_LOCK_DIFF_DEFINE);	  
	 
}
/*************************************************************************
* EMIF_MMR_Config() routine                                              *
*************************************************************************/

static void EMIF_MMR_Config(UWORD32 Read_Latency,UWORD32 Timing1,UWORD32 Timing2,UWORD32 Timing3,UWORD32 Sdram_Config,UWORD32 Ref_Ctrl)
{
	unsigned int i=0;
	 
          WR_MEM_32(EMIF_DDR_PHY_CTRL_1_REG, Read_Latency);
	  WR_MEM_32(EMIF_DDR_PHY_CTRL_1_SHDW_REG, Read_Latency);
	  WR_MEM_32(EMIF_DDR_PHY_CTRL_2_REG, Read_Latency);

	  WR_MEM_32(EMIF_SDRAM_TIM_1_REG,Timing1);
	  WR_MEM_32(EMIF_SDRAM_TIM_1_SHDW_REG,Timing1);

	  WR_MEM_32(EMIF_SDRAM_TIM_2_REG,Timing2);
	  WR_MEM_32(EMIF_SDRAM_TIM_2_SHDW_REG,Timing2);

	  WR_MEM_32(EMIF_SDRAM_TIM_3_REG,Timing3);
	  WR_MEM_32(EMIF_SDRAM_TIM_3_SHDW_REG,Timing3);

	  WR_MEM_32(EMIF_SDRAM_CONFIG_REG, Sdram_Config);
	  WR_MEM_32(EMIF_SDRAM_CONFIG_2_REG, Sdram_Config);
	  WR_MEM_32(EMIF_SDRAM_REF_CTRL_REG,0x00004650);
	  WR_MEM_32(EMIF_SDRAM_REF_CTRL_SHDW_REG,0x00004650); 

	  for(i=0;i<5000;i++)
      {
      }

	  WR_MEM_32(EMIF_SDRAM_REF_CTRL_REG,Ref_Ctrl);
	  WR_MEM_32(EMIF_SDRAM_REF_CTRL_SHDW_REG,Ref_Ctrl); 

	  WR_MEM_32(EMIF_SDRAM_CONFIG_REG, Sdram_Config);
	  WR_MEM_32(EMIF_SDRAM_CONFIG_2_REG, Sdram_Config);


}
//************************************************************************
//VTP_Enable() routine                                                   *
//************************************************************************

static void VTP_Enable(void)
{
	  //Write 1 to enable VTP
      WR_MEM_32(VTP_CTRL_REG ,(RD_MEM_32(VTP_CTRL_REG) | 0x00000040));
	  //Write 0 to CLRZ bit
	  WR_MEM_32(VTP_CTRL_REG ,(RD_MEM_32(VTP_CTRL_REG) & 0xFFFFFFFE));
	  //Write 1 to CLRZ bit
      WR_MEM_32(VTP_CTRL_REG ,(RD_MEM_32(VTP_CTRL_REG) | 0x00000001));
	 //Check for VTP ready bit
	  while((RD_MEM_32(VTP_CTRL_REG) & 0x00000020) != 0x00000020);	  
}
