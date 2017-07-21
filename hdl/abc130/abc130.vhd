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
library UNISIM;
use UNISIM.VComponents.all;

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
			  Y_RSTB_P : in  STD_LOGIC;
			  Y_RSTB_N : in  STD_LOGIC;
			  Y_CLK_160_P : in  STD_LOGIC;
			  Y_CLK_160_N : in  STD_LOGIC;
			  -- Data to YARR
--           Y_DATA_L_P : out  STD_LOGIC;
--           Y_DATA_L_N : out  STD_LOGIC;
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
--           DATA_L_P : out  STD_LOGIC;
--           DATA_L_N : out  STD_LOGIC;
           DATA_R_P : in  STD_LOGIC;
           DATA_R_N : in  STD_LOGIC;
			  -- Flow control
			  XOFFF_L_P : in  STD_LOGIC;
			  XOFFF_L_N : in  STD_LOGIC;
--			  XOFFF_L_P : out  STD_LOGIC;
--			  XOFFF_L_N : out  STD_LOGIC;
			  XOFFF_R_P : out  STD_LOGIC;
			  XOFFF_R_N : out  STD_LOGIC;
--			  XOFFF_R_P : in  STD_LOGIC;
--			  XOFFF_R_N : in  STD_LOGIC;
           -- Static signals
			  TERM_O : out  STD_LOGIC;
           ADDR_O : out  STD_LOGIC_VECTOR(4 DOWNTO 0);
			  REG_EN_D_O : out  STD_LOGIC;
			  REG_EN_A_O : out  STD_LOGIC;
           SHUNT_CTL_O : out  STD_LOGIC;
			  ABCUP_O : out  STD_LOGIC);
end abc;

architecture Behavioral of abc is
	signal drc_s : std_logic;
	signal bco_s : std_logic;
	signal l0_cmd_s : std_logic;
	signal l0_cmd_sample : std_logic;
	signal r3_s : std_logic;
	signal data_l_s : std_logic;
	signal data_r_s : std_logic;
	signal rst_s : std_logic;
	
	component enc_8b10b
	port(
		reset : in std_logic;
		sbyteclk : in std_logic;
		ki : in std_logic;
		ai,bi,ci,di,ei,fi,gi,hi : in std_logic;
		ao,bo,co,do,eo,fo,go,ho,io,jo : out std_logic
		);
	end component;

	component ser_in_par_out
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		si_i : in std_logic;
		k_o : out std_logic;
		po_o : out std_logic_vector(7 downto 0)
		);
	end component;

	component STD_FIFO
		Generic (
			constant DATA_WIDTH  : positive := 10;
			constant FIFO_DEPTH	: positive := 160
		);
		port (
			CLK		: in std_logic;
			RST		: in std_logic;
			DataIn	: in std_logic_vector(9 downto 0);
			WriteEn	: in std_logic;
			ReadEn	: in std_logic;
			DataOut	: out std_logic_vector(9 downto 0);
			Full	: out std_logic;
			Empty	: out std_logic
		);
	end component;

	component par_in_ser_out
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		data_i : in std_logic_vector(9 downto 0);
		data_o : out std_logic;
		read_o : out std_logic
	);
	end component;

	signal clk_s : std_logic;
	signal clk160_s : std_logic;
	signal y_clk_160_s : std_logic;

	signal di_s : std_logic;
	signal so_s : std_logic;
	signal k_s : std_logic;
	signal read_s : std_logic;
	signal enc_in : std_logic_vector(7 downto 0);
	signal enc_out : std_logic_vector(9 downto 0);
	signal par_in : std_logic_vector(9 downto 0);

	signal in_reg  : std_logic_vector(63 downto 0);
	signal bit_cnt : integer := 64;
	signal pause : integer := 0;

begin
	-- No flow control
--	xoff_l_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) 
--							port map (O => open, I => XOFFF_L_P, IB => XOFFF_L_N);
--	xoff_L_obuf : OBUFDS port map (O => XOFFF_L_P, OB => XOFFF_L_n, I => '0');
--	XOFFF_L_P <= '1';
--	XOFFF_L_n <= '0';

--	xoff_R_obuf : OBUFDS port map (O => XOFFF_R_P, OB => XOFFF_R_n, I => '0');
	XOFFF_R_P <= '0';
	XOFFF_R_n <= '1';
	
	-- Set abc address to 0x00
--	addr_o <= (others => '0');
	addr_o <= "00001";
	
	-- No LVDS terminaison
	term_o <= '1';
	
	-- No shunt control
	SHUNT_CTL_O <= '0';

	-- Enable voltage regulators
	REG_EN_D_O <= '0';
	REG_EN_A_O <= '0';
	
	ABCUP_O <= '0';

	-- Connection from YARR to ABC
	drc_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => drc_s, I => Y_DRC_P, IB => Y_DRC_N);
	drc_obuf : OBUFDS port map (O => DRC_P, OB => DRC_N, I => drc_s);
--	DRC_P <= Y_DRC_P;
--	DRC_N <= Y_DRC_N;

	bco_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => bco_s, I => Y_BCO_P, IB => Y_BCO_N);
	bco_obuf : OBUFDS port map (O => BCO_P, OB => BCO_N, I => bco_s);
--	BCO_P <= Y_BCO_P;
--	BCO_N <= Y_BCO_N;

	l0_cmd_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => l0_cmd_sample, I => Y_L0_CMD_P, IB => Y_L0_CMD_N);
	--l0_cmd_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => l0_cmd_s, I => Y_L0_CMD_P, IB => Y_L0_CMD_N);
	l0_cmd_obuf : OBUFDS port map (O => L0_CMD_P, OB => L0_CMD_N, I => l0_cmd_s);
	process(drc_s)
	begin
		if rising_edge(drc_s) then
			l0_cmd_s <= l0_cmd_sample;
		end if;
	end process;
--	L0_CMD_P <= Y_L0_CMD_P;
--	L0_CMD_N <= Y_L0_CMD_N;

	r3_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => r3_s, I => Y_R3_P, IB => Y_R3_N);
	r3_obuf : OBUFDS port map (O => R3_P, OB => R3_N, I => '0');
--	R3_P <= Y_R3_P;
--	R3_N <= Y_R3_N;


	rst_buf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => rst_s, I => Y_RSTB_P, IB => Y_RSTB_N);
	RSTB_O <= rst_s;
--	RSTB_O <= '1';
	
	-- Connection from ABC to YARR
--	data_l_s <= '0';
--	data_l_n <= '1';
--	data_l_p <= '0';
	
--	data_l_ibuf : IBUFDS generic map(DIFF_TERM => false, IBUF_LOW_PWR => FALSE) port map (O => data_l_s, I => DATA_L_P, IB => DATA_L_N);
--	data_l_s <= data_l_p;
--	data_l_obuf : OBUFDS port map (O => DATA_L_N, OB => DATA_L_P, I => '1');
--	Y_DATA_L_P <= DATA_L_P;
--	Y_DATA_L_N <= DATA_L_N;

	data_r_ibuf : IBUFDS generic map(DIFF_TERM => true, IBUF_LOW_PWR => FALSE) port map (O => data_r_s, I => DATA_R_N, IB => DATA_R_P);
--	data_r_s <= data_r_p;
	data_r_obuf : OBUFDS port map (O => Y_DATA_R_P, OB => Y_DATA_R_N, I => so_s);
--	data_r_obuf : OBUFDS port map (O => Y_DATA_R_P, OB => Y_DATA_R_N, I => '0');
--	Y_DATA_R_P <= DATA_R_P;
--	Y_DATA_R_N <= DATA_R_N;

	clk_160 : IBUFDS generic map(DIFF_TERM => true, IBUF_LOW_PWR => FALSE) port map (O => y_clk_160_s, I => Y_CLK_160_N, IB => Y_CLK_160_P);

	sipo : ser_in_par_out port map(
		rst_i => rst_s,
		clk_i => bco_s,
		si_i => data_r_s,
		k_o  => k_s,
		po_o => enc_in
		);

	--k_s <= '1' when enc_in = IDLE else '0';

	encoder : enc_8b10b port map(
		reset => rst_s,
		sbyteclk => bco_s,
		ki => k_s,
		ai => enc_in(0),
		bi => enc_in(1),
		ci => enc_in(2),
		di => enc_in(3),
		ei => enc_in(4),
		fi => enc_in(5),
		gi => enc_in(6),
		hi => enc_in(7),

		ao => enc_out(0),
		bo => enc_out(1),
		co => enc_out(2),
		do => enc_out(3),
		eo => enc_out(4),
		fo => enc_out(6),
		go => enc_out(7),
		ho => enc_out(8),
		io => enc_out(5),
		jo => enc_out(9)
		);

	fifo : STD_FIFO
		PORT MAP (
			CLK		=> y_clk_160_s,
			RST		=> rst_s,
			DataIn	=> enc_out,
			WriteEn	=> clk_s,
			ReadEn	=> read_s,
			DataOut	=> par_in,
			Full	=> open,
			Empty	=> open
		);

	piso : par_in_ser_out port map(
		rst_i => rst_s,
		clk_i => y_clk_160_s,
		data_i => par_in,
		data_o => so_s,
		read_o => read_s
	);


end Behavioral;

