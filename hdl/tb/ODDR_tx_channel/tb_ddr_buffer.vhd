----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:51:09 06/14/2017 
-- Design Name: 
-- Module Name:    ddr_buffer - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
library UNISIM;
use UNISIM.VComponents.all;

entity ddr_buffer is
    Port ( 
           clk_o : out  STD_LOGIC);
end ddr_buffer;

architecture Behavioral of ddr_buffer is

	constant clk_period : time := 10ns;
	
	signal data_s : std_logic;
	
	signal sys_clk_i : std_logic;
	signal rst_i : std_logic;
	
	signal sys_clk_fb : std_logic;
	signal sys_clk_pll_locked : std_logic;
	signal sys_clk_40_buf_90_deg : std_logic;
	signal not_sys_clk_40_buf_90_deg : std_logic;
	signal ddr_clk_buf : std_logic;
	signal sys_clk_200_buf : std_logic;
	signal sys_clk_40_buf : std_logic;
	
	signal received_l0 : std_logic_vector(31 downto 0);
	signal received_cmd : std_logic_vector(31 downto 0);
	
	-- Wishbone slave interface
	signal wb_adr : std_logic_vector(31 downto 0);
	signal wb_dat_o : std_logic_vector(31 downto 0);
	signal wb_dat_i : std_logic_vector(31 downto 0);
	signal wb_cyc : std_logic;
	signal wb_stb : std_logic;
	signal wb_we : std_logic;
	signal wb_ack : std_logic_vector(1 downto 0);
	signal wb_stall : std_logic_vector(1 downto 0);
	signal fe_cmd_o : std_logic_vector(3 downto 0);
	signal trig_pulse : std_logic;
	signal int_trig_t : std_logic;
	
	component wb_tx_core
		generic (
			g_NUM_TX : integer range 1 to 32 := 4
		);
		port (
			-- Sys connect
			wb_clk_i	: in  std_logic;
			rst_n_i		: in  std_logic;

			-- Wishbone slave interface
			wb_adr_i	: in  std_logic_vector(31 downto 0);
			wb_dat_i	: in  std_logic_vector(31 downto 0);
			wb_dat_o	: out std_logic_vector(31 downto 0);
			wb_cyc_i	: in  std_logic;
			wb_stb_i	: in  std_logic;
			wb_we_i : in  std_logic;
			wb_ack_o	: out std_logic;
			wb_stall_o	: out std_logic;

			-- TX
			tx_clk_i	: in  std_logic;
			tx_data_o : out std_logic_vector(g_NUM_TX-1 downto 0);
			trig_pulse_o : out std_logic;
			-- Async
			ext_trig_i : in std_logic
		);
	end component;

begin
	-- Clock process
	process
	begin
		wait for clk_period;
		sys_clk_i <= '1';
		wait for clk_period;
		sys_clk_i <= '0';
	end process;
	
	-- Reset
	process
	begin
		rst_i <= '1';
		wait for 5*clk_period;
		rst_i <= '0';
		wait;
	end process;
	
	-- Put data into the tx_core
	process
	begin
		-- Reset the module
		wb_cyc <= '0';
		wb_stb <= '0';
		wb_we <= '0';
		wb_adr <= (others => '0');
		wb_dat_i <= (others => '0');
		wait until rst_i = '0';
		
		-- Set enable mask
		wb_cyc <= '1';
		wb_stb <= '1';
		wb_we <= '1';
		wb_adr <= x"00000001";
		wb_dat_i <= x"00000008";
		wait until sys_clk_40_buf = '0';
		wait until sys_clk_40_buf = '1';
		
		-- Put data in the FIFO
		wb_cyc <= '1';
		wb_stb <= '1';
		wb_we <= '1';
		wb_adr <= (others => '0');
		wb_dat_i <= x"deadbeef";
		wait until sys_clk_40_buf = '0';
		wait until sys_clk_40_buf = '1';
		
		-- Set enable mask
		wb_cyc <= '1';
		wb_stb <= '1';
		wb_we <= '1';
		wb_adr <= x"00000001";
		wb_dat_i <= x"00000004";
		wait until sys_clk_40_buf = '0';
		wait until sys_clk_40_buf = '1';
		
		-- Put data in the FIFO
		wb_cyc <= '1';
		wb_stb <= '1';
		wb_we <= '1';
		wb_adr <= (others => '0');
		wb_dat_i <= x"c0decafe";
		wait until sys_clk_40_buf = '0';
		wait until sys_clk_40_buf = '1';
		
		-- Disable trigger
		wb_cyc <= '1';
		wb_stb <= '1';
		wb_we <= '1';
		wb_adr <= x"00000003";
		wb_dat_i <= x"00000000";
		wait until sys_clk_40_buf = '0';
		wait until sys_clk_40_buf = '1';

		-- End wisbone transaction
		wb_cyc <= '0';
		wb_stb <= '0';
		wb_we <= '0';
		wb_adr <= x"00000000";
		wb_dat_i <= x"00000000";
		wait;

	end process;
	
	-- Reading the signals from DDR buffer output
	process(sys_clk_40_buf,rst_i) is
	begin
		if rising_edge(sys_clk_40_buf) then
			received_l0 <= std_logic_vector(unsigned(received_l0) sll 1);
			received_l0(0) <= data_s;
		end if;
	end process;
	process(sys_clk_40_buf,rst_i) is
	begin
		if falling_edge(sys_clk_40_buf) then
			received_cmd <= std_logic_vector(unsigned(received_cmd) sll 1);
			received_cmd(0) <= data_s;
		end if;
	end process;
	
	-- Config PLL
	cmp_sys_clk_pll : PLL_BASE
	generic map (
		BANDWIDTH          => "OPTIMIZED",
		CLK_FEEDBACK       => "CLKFBOUT",
		COMPENSATION       => "INTERNAL",
		DIVCLK_DIVIDE      => 1,
		CLKFBOUT_MULT      => 50,
		CLKFBOUT_PHASE     => 0.000,
		CLKOUT0_DIVIDE     => 25,
		CLKOUT0_PHASE      => 90.000,
		CLKOUT0_DUTY_CYCLE => 0.500,
		CLKOUT1_DIVIDE     => 5,
		CLKOUT1_PHASE      => 0.000,
		CLKOUT1_DUTY_CYCLE => 0.500,
		CLKOUT2_DIVIDE     => 3,
		CLKOUT2_PHASE      => 0.000,
		CLKOUT2_DUTY_CYCLE => 0.500,
		CLKOUT3_DIVIDE     => 25,
		CLKOUT3_PHASE      => 0.000,
		CLKOUT3_DUTY_CYCLE => 0.500,
		CLKIN_PERIOD       => 50.0,
		REF_JITTER         => 0.016)
	port map (
		CLKFBOUT => sys_clk_fb,
		CLKOUT0  => sys_clk_40_buf,
		CLKOUT1  => sys_clk_200_buf,
		CLKOUT2  => ddr_clk_buf,
		CLKOUT3  => sys_clk_40_buf_90_deg,
		CLKOUT4  => open,
		CLKOUT5  => open,
		LOCKED   => sys_clk_pll_locked,
		RST      => '0',
		CLKFBIN  => sys_clk_fb,
		CLKIN    => sys_clk_i);

	not_sys_clk_40_buf_90_deg <= not sys_clk_40_buf_90_deg;
	clk_o <= sys_clk_40_buf_90_deg;
	
	-- Config DDR buffer
	rx_clk_ddr : ODDR2
	port map (
		Q  => data_s,
		C0 => sys_clk_40_buf_90_deg,
		C1 => not_sys_clk_40_buf_90_deg,
		CE => '1',
		D0 => fe_cmd_o(3),
		D1 => fe_cmd_o(2),
		R  => '0',
		S  => '0'
	);

	-- TX core
	cmp_wb_tx_core : wb_tx_core port map
	(
		-- Sys connect
		wb_clk_i => sys_clk_40_buf,
		rst_n_i => not rst_i ,
		-- Wishbone slave interface
		wb_adr_i => wb_adr,
		wb_dat_i => wb_dat_i(31 downto 0),
		wb_dat_o => wb_dat_o,
		wb_cyc_i => wb_cyc,
		wb_stb_i => wb_stb,
		wb_we_i => wb_we,
		wb_ack_o => wb_ack(1),
		wb_stall_o => wb_stall(1),
		-- TX
		tx_clk_i => sys_clk_40_buf,
		tx_data_o => fe_cmd_o,
		trig_pulse_o => trig_pulse,
		ext_trig_i => int_trig_t
	);	

end Behavioral;

