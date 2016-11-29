module fullAdder(A, B, Cin, sum, carry);
	input A, B, Cin;
	output sum, carry;
	wire AXorB, CinAndAXorB, AB;
	xor #2 (AXorB, A, B);
	and #2 (AB, A, B);
	and #2 (CinAndAXorB, Cin, AXorB);
	xor #2 (sum, AXorB, Cin);
	or #2 (carry, AB, CinAndAXorB);
endmodule

module Adder1_16bit(A, B, Cin, sum, carry);

	input [15:0] A, B;
	input Cin;
	output [15:0] sum;
	output carry;

	// First Half Adder
	wire [14:0] middleCarry;

	fullAdder
		FA0 (A[0], B[0], Cin, sum[0], middleCarry[0]),
		FA1 (A[1], B[1], middleCarry[0], sum[1], middleCarry[1]),
		FA2 (A[2], B[2], middleCarry[1], sum[2], middleCarry[2]),
		FA3 (A[3], B[3], middleCarry[2], sum[3], middleCarry[3]),
		FA4 (A[4], B[4], middleCarry[3], sum[4], middleCarry[4]),
		FA5 (A[5], B[5], middleCarry[4], sum[5], middleCarry[5]),
		FA6 (A[6], B[6], middleCarry[5], sum[6], middleCarry[6]),
		FA7 (A[7], B[7], middleCarry[6], sum[7], middleCarry[7]),
		FA8 (A[8], B[8], middleCarry[7], sum[8], middleCarry[8]),
		FA9 (A[9], B[9], middleCarry[8], sum[9], middleCarry[9]),
		FA10 (A[10], B[10], middleCarry[9], sum[10], middleCarry[10]),
		FA11 (A[11], B[11], middleCarry[10], sum[11], middleCarry[11]),
		FA12 (A[12], B[12], middleCarry[11], sum[12], middleCarry[12]),
		FA13 (A[13], B[13], middleCarry[12], sum[13], middleCarry[13]),
		FA14 (A[14], B[14], middleCarry[13], sum[14], middleCarry[14]),
		FA15 (A[15], B[15], middleCarry[14], sum[15], carry);

endmodule

module BitCarryLookaheadAdder_4(A, B, Cin, sum, carry);

	input [3:0] A, B;
	input Cin;
	output [3:0] sum;
	output carry;

	wire [3:0] G;
	wire [3:0] P;
	wire [3:1] middleCarry;
	wire C1Helper;
	wire [1:0] C2Helper;
	wire [2:0] C3Helper;
	wire [3:0] C4Helper;

	and 
		#2 a1 (G[0], A[0], B[0]),
		a2 (G[1], A[1], B[1]),
		a3 (G[2], A[2], B[2]),
		a4 (G[3], A[3], B[3]);

	xor
		#2 x1 (P[0], A[0], B[0]),
		x2 (P[1], A[1], B[1]),
		x3 (P[2], A[2], B[2]),
		x4 (P[3], A[3], B[3]);

	// Carry 1
	and #2 (C1Helper, P[0], Cin);
	or #2 (middleCarry[1], C1Helper, G[0]);

	// Carry 2
	and #3 (C2Helper[0], P[1], P[0], Cin);
	and #2 (C2Helper[1], P[1], G[0]);
	or #3 (middleCarry[2], C2Helper[0], C2Helper[1], G[1]);

	// Carry 3
	and #4 (C3Helper[0], P[2], P[1], P[0], Cin);
	and #3 (C3Helper[1], P[2], P[1], G[0]);
	and #2 (C3Helper[2], P[2], G[1]);
	or #4 (middleCarry[3], C3Helper[0], C3Helper[1], C3Helper[2], G[2]);

	// Carry 4
	and #5 (C4Helper[0], P[3], P[2], P[1], P[0], Cin);
	and #4 (C4Helper[1], P[3], P[2], P[1], G[0]);
	and #3 (C4Helper[2], P[3], P[2], G[1]);
	and #2 (C4Helper[3], P[3], G[2]);
	or #5 (carry, C4Helper[0], C4Helper[1], C4Helper[2], C4Helper[3], G[3]);

	xor #2 (sum[0], P[0], Cin),
		(sum[1], P[1], middleCarry[1]),
		(sum[2], P[2], middleCarry[2]),
		(sum[3], P[3], middleCarry[3]);

endmodule

module Adder2_16bit(A, B, Cin, sum, carry);

	input [15:0] A, B;
	input Cin;
	output [15:0] sum;
	output carry;

	wire [1:3] middleCarry;

	BitCarryLookaheadAdder_4
		B1 (A[3:0], B[3:0], Cin, sum[3:0], middleCarry[1]),
		B2 (A[7:4], B[7:4], middleCarry[1], sum[7:4], middleCarry[2]),
		B3 (A[11:8], B[11:8], middleCarry[2], sum[11:8], middleCarry[3]),
		B4 (A[15:12], B[15:12], middleCarry[3], sum[15:12], carry);

endmodule

module Adder16_TB;

	reg [15:0] A, B;
	reg Cin;
	wire [15:0] rcSum, claSum;
	wire rcCarry, claCarry;

	Adder1_16bit rcAdder(A, B, Cin, rcSum, rcCarry);
	Adder2_16bit claAdder(A, B, Cin, claSum, claCarry);
	
	always @(rcCarry) 
	   if(rcCarry==1) 
	   	$display("Ripple carry ==> Overflow Happened");
	always @(claCarry) 
	   if(claCarry==1)
		  $display("Carry Lookahead ==> Overflow Happened");

	initial begin

    	#1
		  A = 16'b0000000000000000; B = 16'b0000000000000000; Cin = 1'b0;
		#99
		  $display("A = %b (%d) | B = %b (%d)", A, A, B, B,);
		  $display("Result of carry lookahead adder ==> Sum: %b (%d) | Carry: %b", claSum, claSum, claCarry);
		  $display("Result of ripple carry adder ==> Sum: %b (%d) | Carry: %b", rcSum, rcSum, rcCarry);
		#100 
		  A = 16'b0010010011010111; B = 16'b0000001111111100; Cin = 1'b0;
		#100
		  $display("A = %b (%d) | B = %b (%d)", A, A, B, B,);
		  $display("Result of carry lookahead adder ==> Sum: %b (%d) | Carry: %b", claSum, claSum, claCarry);
		  $display("Result of ripple carry adder ==> Sum: %b (%d) | Carry: %b", rcSum, rcSum, rcCarry);
		#100 
		  A = 16'b1111110111101000; B = 16'b0000001111111100; Cin = 1'b0;
		#200
		  $display("A = %b (%d) | B = %b (%d)", A, A, B, B,);
		  $display("Result of carry lookahead adder ==> Sum: %b (%d) | Carry: %b", claSum, claSum, claCarry);
		  $display("Result of ripple carry adder ==> Sum: %b (%d) | Carry: %b", rcSum, rcSum, rcCarry);

	end	

endmodule

