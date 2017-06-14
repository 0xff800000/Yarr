----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:37:03 06/12/2017 
-- Design Name: 
-- Module Name:    abc - Behavioral 
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
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity abc is

    Port (
           -- Commands from YARR
			  Y_DRC_P : in  STD_LOGIC;
           Y_DRC_N : in  STD_LOGIC;
           Y_BCO_P : in  STD_LOGIC;
           Y_BCO_N : in  STD_LOGIC;
           Y_L0_CMD_P : in  STD_LOGIC;
           Y_L0_CMD_N : in  STD_LOGIC;
           Y_R3_P : in  STD_LOGIC;
           Y_R3_N : in  STD_LOGIC;
			  Y_RSTB_O : in  STD_LOGIC;
			  -- Data to YARR
           Y_DATA_L_P : out  STD_LOGIC;
           Y_DATA_L_N : out  STD_LOGIC;
           Y_DATA_R_P : out  STD_LOGIC;
           Y_DATA_R_N : out  STD_LOGIC;
           -- Commands to ABC
			  DRC_P : out  STD_LOGIC;
           DRC_N : out  STD_LOGIC;
           BCO_P : out  STD_LOGIC;
           BCO_N : out  STD_LOGIC;
           L0_CMD_P : out  STD_LOGIC;
           L0_CMD_N : out  STD_LOGIC;
           R3_P : out  STD_LOGIC;
           R3_N : out  STD_LOGIC;
			  RSTB_O : out  STD_LOGIC;
			  -- Data from ABC
           DATA_L_P : in  STD_LOGIC;
           DATA_L_N : in  STD_LOGIC;
           DATA_R_P : in  STD_LOGIC;
           DATA_R_N : in  STD_LOGIC;
			  -- Flow control
			  XOFFF_L_P : out  STD_LOGIC;
			  XOFFF_L_N : out  STD_LOGIC;
			  XOFFF_R_P : out  STD_LOGIC;
			  XOFFF_R_N : out  STD_LOGIC;
           -- Static signals
			  TERM_O : out  STD_LOGIC;
           ADDR_O : out  STD_LOGIC_VECTOR(4 DOWNTO 0);
			  REG_EN_D_O : out  STD_LOGIC;
			  REG_EN_A_O : out  STD_LOGIC;
           SHUNT_CTL_O : out  STD_LOGIC);
end abc;

architecture Behavioral of abc is
	
begin
	-- No flow control
	XOFFF_L_P <= '0';
	XOFFF_L_n <= '1';
	XOFFF_R_P <= '0';
	XOFFF_R_n <= '1';
	
	-- Set abc address to 0x00
	addr_o <= (others => '0');
	
	-- No LVDS terminaison
	term_o <= '0';
	
	-- No shunt control
	SHUNT_CTL_O <= '0';

	-- Enable voltage regulators
	REG_EN_D_O <= '0';
	REG_EN_A_O <= '0';

	-- Connection from YARR to ABC
	DRC_P <= Y_DRC_P;
	DRC_N <= Y_DRC_N;
	BCO_P <= Y_BCO_P;
	BCO_N <= Y_BCO_N;
	L0_CMD_P <= Y_L0_CMD_P;
	L0_CMD_N <= Y_L0_CMD_N;
	R3_P <= Y_R3_P;
	R3_N <= Y_R3_N;
	RSTB_O <= Y_RSTB_O;
	
	-- Connection from ABC to YARR
	Y_DATA_L_P <= DATA_L_P ;
	Y_DATA_L_N <= DATA_L_N;
	Y_DATA_R_P <= DATA_R_P;
	Y_DATA_R_N <= DATA_R_N;

end Behavioral;

