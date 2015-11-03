/* low level logic tests of masking and bit shifting w/ long long ints */ 

 if((task.flags & DEBUG) && !task.mpi_rank) {
  key = 0 ;
  while(key<10) {
   if(!task.mpi_rank) {
    printf("IPM: 0 showbits key=%lld bits=",key);
    IPM_SHOWBITS(key);
    printf("\n");
   }
   key++;
  }


  test_flags = 0;
  for(rv=0;rv<64;rv++) {
   test_flags |= (0x0000000000000001ULL<<rv);
   printf("bits %2d ", rv);
 IPM_SHOWBITS(test_flags);
 printf("\n");
  }

  test_flags = 0;
  for(i=0;i<61;i++) {
   test_flags |= (0x0000000000000001ULL<<i);
   if((test_flags&(0x0000000000000001ULL<<i))==(0x0000000000000001ULL<<i)) {
    test_flags = 0;    } else {
    printf("IPM: %d bit logic test failed : %lld != %lld (0x0000000000000001ULL << %d)\n",
        0,         test_flags&(0x0000000000000001ULL<<i),
        (0x0000000000000001ULL<<i),
        i);
    test_flags = 0;
   }
  }
  test_flags = 0;

 }

