library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity mux_8b10b is
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		sample_i : in std_logic;
		write_o : out std_logic;
		data_i : in std_logic_vector(9 downto 0);
		data_o : out std_logic_vector(79 downto 0)
	);
end mux_8b10b;

architecture Behavioral of mux_8b10b is
	constant N_CYCL : integer := 2;
	signal in_reg : std_logic_vector(79 downto 0);
	signal packets : integer := 0;
	signal clock_div : integer := 0;
begin
	process(clk_i,rst_i)
	begin
		if rst_i = '1' then
			data_o <= (others => '0');
			write_o <= '0';
			in_reg <= (others => '0');
			packets <= 0;
			write_o <= '0';
		elsif rising_edge(clk_i) then
			
			-- Triggering the sampling
			if sample_i = '1' then
				packets <= 7;
				in_reg(9 downto 0) <= data_i;
			end if;
			
			-- Sample 8 packets of the 8b10b output
			if packets > 0 then
				clock_div <= clock_div + 1;
				if clock_div = N_CYCL then
					clock_div <= 0;				
					in_reg <= std_logic_vector(unsigned(in_reg) sll 10);
					in_reg(9 downto 0) <= data_i;
					packets <= packets - 1;
				end if;
			end if;
		end if;
		
		-- Write output + send write signal
		if falling_edge(clk_i) then
			if packets = 0 then
				data_o <= in_reg;
				write_o <= '1';
				packets <= -1; -- ugly workaround
			else
				write_o <= '0';
			end if;
		end if;

	end process;

end Behavioral;
