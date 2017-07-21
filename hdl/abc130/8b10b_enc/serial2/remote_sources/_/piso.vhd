library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity par_in_ser_out is
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		data_i : in std_logic_vector(9 downto 0);
		data_o : out std_logic;
		read_o : out std_logic
	);
end par_in_ser_out;

architecture Behavioral of par_in_ser_out is
	signal in_reg : std_logic_vector(9 downto 0);
	signal bit_shifted : integer := 0;
begin
	-- Serialize data
	process(clk_i,rst_i)
	begin
		if rst_i = '1' then
			data_o <= '0';
			in_reg <= (others => '0');
		elsif rising_edge(clk_i) then
			read_o <= '0';
			data_o <= in_reg(9);
			in_reg <= std_logic_vector(unsigned(in_reg) sll 1);
			bit_shifted <= bit_shifted + 1;
			if bit_shifted = 10 then
				read_o <= '1';
				in_reg <= data_i;
				bit_shifted <= 0;
			end if;
				
		end if;
	end process;
end Behavioral;
