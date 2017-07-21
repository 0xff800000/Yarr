library IEEE;

use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity ser_in_par_out is
	port(
		rst_i : in std_logic;
		clk_i : in std_logic;
		si_i : in std_logic;
		k_o : out std_logic;
		po_o : out std_logic_vector(7 downto 0)
	);
end ser_in_par_out;

architecture Behavioral of ser_in_par_out is
	constant IDLE : std_logic_vector(7 downto 0) := "10111100";
	signal in_reg : std_logic_vector(63 downto 0);
	signal fifo : std_logic_vector(71 downto 0);

begin
	-- Input register
	process(clk_i,rst_i)
	begin
		if rst_i = '1' then
			in_reg <= (others => '0');
			po_o <= (others => '0');
			fifo <= (others => '0');
		elsif rising_edge(clk_i) then
			-- Sample input
			in_reg <= std_logic_vector(unsigned(in_reg) sll 1);
			in_reg(0) <= si_i;

			-- Shift out the register
			if in_reg(63) = '1' then
				--fifo <= in_reg; -- replace fifo with data
				fifo <= (others => '0');
				fifo(7 downto 0) <= in_reg(7 downto 0);
				fifo(16 downto 9) <= in_reg(15 downto 8);
				fifo(25 downto 18) <= in_reg(23 downto 16);
				fifo(34 downto 27) <= in_reg(31 downto 24);
				fifo(43 downto 36) <= in_reg(39 downto 32);
				fifo(52 downto 45) <= in_reg(47 downto 40);
				fifo(61 downto 54) <= in_reg(55 downto 48);
				fifo(70 downto 63) <= in_reg(63 downto 56);

				in_reg <= (others => '0');
			else
				fifo(7 downto 0) <= IDLE;
				fifo(8) <= '1';	-- K special
			end if;

		end if;
		
		if falling_edge(clk_i) then
			if unsigned(fifo) > 0 then
				-- Shift the register << 8
				--fifo <= std_logic_vector(unsigned(fifo) sll 8);
				fifo <= std_logic_vector(unsigned(fifo) sll 9);
				--po_o <= fifo(63 downto 56);
				po_o <= fifo(70 downto 63);
				k_o <= fifo(71);
			end if;
		end if;
	end process;
	
	
	

end Behavioral;