library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity enc_tb is
end enc_tb;

architecture Behavioral of enc_tb is
	
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
			constant FIFO_DEPTH	: positive := 66
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
	
	constant IDLE : std_logic_vector(7 downto 0) := "10111100";
	constant Tclk : time := 10 ns;
	
	signal clk_s : std_logic;
	signal clk160_s : std_logic;
	signal rst_s : std_logic;
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
	-- Generate clock
	process
	begin
		clk_s <= '0';
		wait for Tclk;
		clk_s <= '1';
		wait for Tclk;
	end process;
	
	process
	begin
		clk160_s <= '0';
		wait for Tclk/2;
		clk160_s <= '1';
		wait for Tclk/2;
	end process;

	-- Input bitstream
	process(clk_s)
	begin
		if rst_s = '1' then
			di_s <= '0';
			in_reg <= x"fedcba9876543210";
		elsif rising_edge(clk_s) then
			if pause > 0 then
				pause <= pause - 1;
			else
				di_s <= in_reg(bit_cnt-1);
				if bit_cnt = 1 then
					bit_cnt <= 64;
					pause <= 100;
				else
					bit_cnt <= bit_cnt - 1;
				end if;
			end if;
		end if;
	end process;

	-- Reset signal
	process
	begin
		rst_s <= '1';
		wait for 3*Tclk;
		rst_s <= '0';
		wait;
	end process;

	sipo : ser_in_par_out port map(
		rst_i => rst_s,
		clk_i => clk_s,
		si_i => di_s,
		k_o  => k_s,
		po_o => enc_in
		);

	--k_s <= '1' when enc_in = IDLE else '0';

	encoder : enc_8b10b port map(
		reset => rst_s,
		sbyteclk => clk_s,
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
			CLK		=> clk160_s,
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
		clk_i => clk160_s,
		data_i => par_in,
		data_o => so_s,
		read_o => read_s
	);
	

end Behavioral;