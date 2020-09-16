
The cdr-code is a 4-bit field of a 32-bit cons cell.
It is treated as an on-page indicator bit and a 3-bit scaled offset.

Certain combinations are treated specially.


on-page| offset	| interpretation
-------|--------|----------------------------------------------------------------------------
   1   |    0	| CDR is NIL
   1   |  1 - 7	| CDR is at 2*offset (counted in 32-bit cells) on same page
   0   |    0	| CDR is indirect, CDR(car)
   0   |  1 - 7	| CDR is not a cons cell but is in the car of cell at 2*offset on same page
-------|--------|----------------------------------------------------------------------------


