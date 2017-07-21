library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity ser_in_par_out is
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		si_i : in std_logic;
		po_o : out std_logic_vector(7 downto 0);
		valid_o : out std_logic
	);
end ser_in_par_out;

architecture Behavioral of ser_in_par_out is
	constant NB_BYTES : integer := 8;
	constant NB_BITS : integer := 8;
	constant N_CYCL : integer := 2;

	signal edge_count : integer range 0 to 7 := 0;
	signal q_word_counter : integer range 0 to 63 := 0;
	signal in_reg : std_logic_vector(63 downto 0);
	signal back : std_logic_vector(63 downto 0);
	signal clock_div : integer range 0 to 1 := 0;
	signal bytes : integer range 0 to 8 := 0;

begin
	process(clk_i,rst_i)
	begin
		if rst_i = '1' then
			in_reg <= (others => '0');
			valid_o <= '0';
			po_o <= (others => '0');
			edge_count <= 0;
		elsif rising_edge(clk_i) then
--			valid_o <= '0';

			-- Sample input
			in_reg <= std_logic_vector(unsigned(in_reg) sll 1);
			in_reg(0) <= si_i;

			-- Shift out the register
			if in_reg(63) = '1' then
				back <= in_reg;
				in_reg <= (others => '0');
				bytes <= 8; -- Send 8 bytes
			end if;

			-- Latch the output during N_CYCL + 1
			clock_div <= clock_div + 1;
			if clock_div = N_CYCL then
				clock_div <= 0;
				if bytes > 0 then
					po_o <= back(63 downto 55);
					back <= std_logic_vector(unsigned(back) sll 8);
					bytes <= bytes - 1;
				end if;
			end if;

		end if;
		
		-- Valid output (ugly code)
		if falling_edge(clk_i) and clock_div = 1 and bytes = 7 then
			valid_o <= '1';
		elsif falling_edge(clk_i) then
			valid_o <= '0';
		end if;

	end process;
	

end Behavioral;