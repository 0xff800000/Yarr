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
		po_o : out std_logic_vector(7 downto 0);
		valid_o : out std_logic
		);
	end component;
	
	component mux_8b10b
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		sample_i : in std_logic;
		write_o : out std_logic;
		data_i : in std_logic_vector(9 downto 0);
		data_o : out std_logic_vector(79 downto 0)
		);
	end component;
	
	component par_in_ser_out
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		start_i : in std_logic;
		we_i : in std_logic;
		data_i : in std_logic_vector(79 downto 0);
		data_o : out std_logic;
		done_o : out std_logic
	);
	end component;
	
	signal clk_s : std_logic;
	signal rst_s : std_logic;
	signal di_s : std_logic;
	signal valid_s : std_logic;
	signal so_s : std_logic;
	signal wo_s : std_logic;
	signal par_out : std_logic_vector(7 downto 0);
	signal enc_out : std_logic_vector(9 downto 0);
	signal par_in : std_logic_vector(79 downto 0);

	signal in_reg  : std_logic_vector(63 downto 0);
	signal bit_cnt : integer := 64;
	signal pause : integer := 0;

begin
	-- Generate clock
	process
	begin
		clk_s <= '0';
		wait for 10 ns;
		clk_s <= '1';
		wait for 10 ns;
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
		wait for 30 ns;
		rst_s <= '0';
		wait;
	end process;

	sipo : ser_in_par_out port map(
		rst_i => rst_s,
		clk_i => clk_s,
		si_i => di_s,
		po_o => par_out,
		valid_o => valid_s
		);

	encoder : enc_8b10b port map(
		reset => rst_s,
		sbyteclk => clk_s,
		ki => '0',
		ai => par_out(0),
		bi => par_out(1),
		ci => par_out(2),
		di => par_out(3),
		ei => par_out(4),
		fi => par_out(5),
		gi => par_out(6),
		hi => par_out(7),

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
		
	mux : mux_8b10b port map(
		rst_i => rst_s,
		clk_i => clk_s,
		sample_i => valid_s,
		write_o => wo_s,
		data_i => enc_out,
		data_o => par_in
		);
	
	piso : par_in_ser_out port map(
		rst_i => rst_s,
		clk_i => clk_s,
		start_i => wo_s,
		we_i => wo_s,
		data_i => par_in,
		data_o => so_s,
		done_o => open
	);
	

end Behavioral;