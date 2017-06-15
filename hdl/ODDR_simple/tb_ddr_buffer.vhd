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
	
	signal L0_s : std_logic;
	signal CMD_s : std_logic;
	
	signal sys_clk_fb : std_logic;
	signal sys_clk_pll_locked : std_logic;
	signal sys_clk_40_buf_90_deg : std_logic;
	signal not_sys_clk_40_buf_90_deg : std_logic;
	signal ddr_clk_buf : std_logic;
	signal sys_clk_200_buf : std_logic;
	signal sys_clk_40_buf : std_logic;
	
	signal L0_request : std_logic_vector(15 downto 0) := x"dead";
	signal command : std_logic_vector(15 downto 0) := x"beef";
	signal ptr : natural := 0;
	
	signal received_l0 : std_logic_vector(15 downto 0);
	signal received_cmd : std_logic_vector(15 downto 0);
	signal ptr1 : natural := 0;
	signal ptr2 : natural := 0;

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
	
	-- Loading ODDR
	process(sys_clk_40_buf,rst_i) is
	begin
		L0_s <= L0_request(ptr);
		cmd_s <= command(ptr);
		if (rst_i = '1') then
			ptr <= 0;
		elsif rising_edge(sys_clk_40_buf) then
			if(ptr < 15) then
				ptr <= ptr + 1;
			else
				ptr <= 0;
			end if;
		end if;
	end process;
	
	-- Reading the signals from DDR buffer output
	process(sys_clk_40_buf,rst_i) is
	begin
		if (rst_i = '1') then
			ptr1 <= 0;
		elsif rising_edge(sys_clk_40_buf) then
			received_l0(ptr2) <= data_s;
			if(ptr1 < 15) then
				ptr1 <= ptr1 + 1;
			else
				ptr1 <= 0;
			end if;
		end if;
	end process;
	process(sys_clk_40_buf,rst_i) is
	begin
		if (rst_i = '1') then
			ptr2 <= 0;
		elsif falling_edge(sys_clk_40_buf) then
			received_cmd(ptr2) <= data_s;
			if(ptr2 < 15) then
				ptr2 <= ptr2 + 1;
			else
				ptr2 <= 0;
			end if;
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
		D0 => L0_s,
		D1 => CMD_s,
		R  => '0',
		S  => '0'
	);					  

end Behavioral;

