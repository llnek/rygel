1000 PRINT "### DATA READ INTO ARRAY ###"
1010 DIM A(5)
1015 PRINT "READ DATA"
1020 FOR I=1 TO 5
1025 PRINT "READ INDEX ";I
1030 READ A(I)
1035 PRINT "DONE"
1040 NEXT
1050 PRINT "PRINT DATA"
1060 FOR I=1 TO 5
1070 PRINT A(I)
1080 NEXT
1090 PRINT "DONE"
1150 END
2000 DATA 10,20,30,40,50
