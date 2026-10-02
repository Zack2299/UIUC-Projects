module cursor ( input Reset, frame_clk,
					input [7:0] keycode,
               output [9:0]  CursorX, CursorY,
					output select, spacebar, delete, enter, keyWasPressed,
					output [1:0] inMenu);
					
	logic [9:0] Cursor_X_Pos, Cursor_Y_Pos;
	logic [7:0] lastKeycode;
	logic local_select, local_spacebar, local_delete, local_enter;
	logic [1:0] local_menu;
	logic localKeyWasPressed;
	
	parameter [9:0] Cursor_X_Min = 8*4;
	// -------------------------------------
	parameter [9:0] Cursor_X_Min2 = 160;
	parameter [9:0] Cursor_X_Min3 = 224;
	parameter [9:0] Cursor_X_Max2 = 288;
	// -------------------------------------
	parameter [9:0] Cursor_X_Max = 136*4;
	parameter [9:0] Cursor_Y_Min = 32*4;
	// -------------------------------------
	parameter [9:0] Cursor_Y_Min2 = 16;
	// -------------------------------------
	parameter [9:0] Cursor_Y_Max = 96*4;
	parameter [9:0] Cursor_X_Middle = 72*4;
	parameter [9:0] Cursor_Y_Middle = 64*4;
	
	always_ff @ (posedge Reset or posedge frame_clk)
	begin: Move_Cursor
		
		if (Reset) begin
		
			Cursor_X_Pos <= Cursor_X_Middle;
			Cursor_Y_Pos <= Cursor_Y_Middle;
			lastKeycode <= 8'h00;
			local_spacebar <= 1'b0;
			local_select <= 1'b0;
			local_delete <= 1'b0;
			local_enter <= 1'b0;
			localKeyWasPressed <= 1'b0;
			
		end else begin
		
			case (keycode)
				8'h04 : begin //A - MOVE LEFT
					if (Cursor_X_Pos != Cursor_X_Min && lastKeycode != 8'h04) begin
						Cursor_X_Pos <= Cursor_X_Pos - 64;
						lastKeycode <= 8'h04;
					end
					// --------------------------------------------------
					// pick plant area
					if (Cursor_X_Pos != Cursor_X_Min2 && Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h04) begin
						Cursor_X_Pos <= Cursor_X_Pos - 64;
						lastKeycode <= 8'h04;
					end
					if (Cursor_X_Pos == Cursor_X_Min2 && Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h04) begin
						Cursor_X_Pos <= Cursor_X_Min2;
					end
					// --------------------------------------------------
					if (lastKeycode != 8'h04)
						localKeyWasPressed <= 1'b1;
					else
						localKeyWasPressed <= 1'b0;
				end
				  
				8'h07 : begin //D - MOVE RIGHT
					if (Cursor_X_Pos != Cursor_X_Max && lastKeycode != 8'h07) begin
						Cursor_X_Pos <= Cursor_X_Pos + 64;
						lastKeycode <= 8'h07;
					end
					// -----------------------------------------------------------------------
					// pick plant area
					if (Cursor_X_Pos != Cursor_X_Max2 && Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h07) begin
						Cursor_X_Pos <= Cursor_X_Pos + 64;
						lastKeycode <= 8'h07;
					end
					if (Cursor_X_Pos == Cursor_X_Max2 && Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h07) begin
						Cursor_X_Pos <= Cursor_X_Max2;
					end
					// -----------------------------------------------------------------------
					if (lastKeycode != 8'h07)
						localKeyWasPressed <= 1'b1;
					else
						localKeyWasPressed <= 1'b0;
				end
				  
				8'h16 : begin //S - MOVE DOWN
					if (Cursor_Y_Pos != Cursor_Y_Max && lastKeycode != 8'h16) begin
						Cursor_Y_Pos <= Cursor_Y_Pos + 64;
						lastKeycode <= 8'h16;
					end
					// -----------------------------------------------------------------------
					// pick plant area
					if (Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h16) begin
						Cursor_Y_Pos <= Cursor_Y_Min;
						lastKeycode <= 8'h16;
					end
					// -----------------------------------------------------------------------
					if (lastKeycode != 8'h16)
						localKeyWasPressed <= 1'b1;
					else
						localKeyWasPressed <= 1'b0;
				end
				  
				8'h1A : begin //W - MOVE UP
					if (Cursor_Y_Pos != Cursor_Y_Min && lastKeycode != 8'h1A) begin
						Cursor_Y_Pos <= Cursor_Y_Pos - 64;
						lastKeycode <= 8'h1A;
					end
					// -----------------------------------------------------------------------
					// to select the plant menu
					// left (1-3)
					if (Cursor_Y_Pos != Cursor_Y_Min2 && Cursor_Y_Pos == Cursor_Y_Min && Cursor_X_Pos <= Cursor_X_Min2 && lastKeycode != 8'h1A) begin
						Cursor_Y_Pos <= Cursor_Y_Min2; // 28
						Cursor_X_Pos <= Cursor_X_Min2; // 160
						lastKeycode <= 8'h1A;
					end
					// middle (4)
					if (Cursor_Y_Pos != Cursor_Y_Min2 && Cursor_Y_Pos == Cursor_Y_Min && Cursor_X_Pos == Cursor_X_Min3 && lastKeycode != 8'h1A) begin
						Cursor_Y_Pos <= Cursor_Y_Min2; // 28
						Cursor_X_Pos <= Cursor_X_Min3; // 224
						lastKeycode <= 8'h1A;
					end
					// right (5+)
					if (Cursor_Y_Pos != Cursor_Y_Min2 && Cursor_Y_Pos == Cursor_Y_Min && Cursor_X_Pos >= Cursor_X_Max2 && lastKeycode != 8'h1A) begin
						Cursor_Y_Pos <= Cursor_Y_Min2; // 28
						Cursor_X_Pos <= Cursor_X_Max2; // 288
						lastKeycode <= 8'h1A;
					end
					if (Cursor_Y_Pos == Cursor_Y_Min2 && lastKeycode != 8'h1A) begin
						Cursor_Y_Pos <= Cursor_Y_Min2;
						lastKeycode <= 8'h1A;
					end
					// -----------------------------------------------------------------------
					if (lastKeycode != 8'h1A)
						localKeyWasPressed <= 1'b1;
					else
						localKeyWasPressed <= 1'b0;
				end
				
				8'h2c : begin // SPACE
					local_spacebar <= 1'b1;
					if (lastKeycode != 8'h2c)
						local_select <= 1'b1;
					else
						local_select <= 1'b0;
					lastKeycode <= 8'h2c;
					
					if (lastKeycode != 8'h2c)
						localKeyWasPressed <= 1'b1;
					else
						localKeyWasPressed <= 1'b0;
				end
				8'h4c : begin // DELETE
					if (lastKeycode != 8'h4c)
						local_delete <= 1'b1;
					else
						local_delete <= 1'b0;
					lastKeycode <= 8'h4c;
				end
				8'h28 : begin // enter
					local_enter <= 1'b1;
					lastKeycode <= 8'h28;
				end
				default: begin
					lastKeycode <= 8'h00; // reset lastKeycode
					local_spacebar <= 1'b0;
					local_delete <= 1'b0;
					local_enter <= 1'b0;
				end
			endcase	
		end
	end
	
	always_ff @ (posedge Reset or posedge frame_clk) begin
		if (Reset) begin
			local_menu <= 0;
		end else begin
			if (Cursor_Y_Pos == Cursor_Y_Min2) begin
				if (Cursor_X_Pos == Cursor_X_Min2)
					local_menu <= 1; // peashooter
				else if (Cursor_X_Pos == Cursor_X_Min2 + 64)
					local_menu <= 2; // sunflower
				else // Cursor_X_Pos == Cursor_X_Min2 + 128
					local_menu <= 3; // walnut (josh)
			end else begin
				local_menu <= 0;
			end
		end
	end
	
	assign inMenu = local_menu;
	assign delete = local_delete;
	assign select = local_select;
	assign spacebar = local_spacebar;
	assign enter = local_enter;
	assign CursorX = Cursor_X_Pos;
	assign CursorY = Cursor_Y_Pos;
	assign keyWasPressed = localKeyWasPressed;
	
endmodule