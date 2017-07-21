library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity par_in_ser_out is
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		start_i : in std_logic;
		we_i : in std_logic;
		data_i : in std_logic_vector(79 downto 0);
		data_o : out std_logic;
		done_o : out std_logic
	);
end par_in_ser_out;

architecture Behavioral of par_in_ser_out is
	signal in_reg : std_logic_vector(79 downto 0);
	signal temp : std_logic_vector(79 downto 0);
	signal sending_data : std_logic;
	signal new_data : std_logic;
	signal bit_shifted : integer := 0;
begin
	process(clk_i,rst_i)
	begin
		if rst_i = '1' then
			data_o <= '0';
			sending_data <= '0';
			new_data <= '0';
		elsif rising_edge(clk_i) then
			-- Trigger sending data
			if start_i = '1' then
				sending_data <= '1';
				in_reg <= data_i;
			end if;

			-- Send input buffer
			if sending_data = '1' or new_data = '1' then
				data_o <= in_reg(79);
				in_reg <= std_logic_vector(unsigned(in_reg) sll 1);
				bit_shifted <= bit_shifted + 1;
				if bit_shifted = 80 then
					bit_shifted <= 0;
					sending_data <= '0';
					done_o <= '0';
					new_data <= '0';
				end if;
			end if;

--			-- Write input buffer
--			if we_i = '1' then
--				temp <= data_i;
--			end if;
--			if sending_data = '0' then
--				in_reg <= temp;
--				new_data <= '1';
--				sending_data <= '1';
--			end if;

		end if;
	
	end process;

end Behavioral;
