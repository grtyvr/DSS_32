# H1 Configuring the Bbox

Command: 	| Z (ASCII 90)
Format:	| Z	±xxxxx ±yyyyy
Response:	*
Where: 	a space (ASCII 032) delimits each item
		±xxxxx is the integer value for the x-axis’ resolution
		±yyyyy is the integer value for the y-axis’ resolution

A properly formatted Z command is acknowledged by a single asterisk (*).  This setting can be checked by sending an H (ASCII 072), to which the Bbox will respond by sending the current configuration in the same format.

Checking the Status
Command:	P	(ASCII 080)
Format:	P
Response:	xyb
Where		x is a single-digit number of errors on the x-axis
		y is a single-digit number of errors on the y-axis
		b is the power (battery) status; 1=ok, 0=low.

Obtaining Encoder Positions
Command:	Q	(ASCII 081)
Format:	Q
Response:	±xxxxx ±yyyyy

Where:		a tab (ASCII009) delimits each value
		±xxxxx is the integer value of the x-axis’ position
		±yyyyy is the integer value of the y-axis’ position
