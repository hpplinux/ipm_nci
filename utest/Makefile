include ../make_config

all: xipm_unit xtimer xmemusage xapp_serial
	
xipm_unit: xipm_unit.c  
	$(CC) $(CFLAGS) -I../include -o xipm_unit $(HPM_CFLAGS) xipm_unit.c $(LDFLAGS) $(HPM_LDFLAGS)

xapp_serial: xapp_serial.c  
	$(CC) $(CFLAGS) -I../include -o xapp_serial $(HPM_CFLAGS) xapp_serial.c $(LDFLAGS) -L../lib -lipm $(HPM_LDFLAGS)

xstimer: xstimer.c  
	$(CC) $(CFLAGS) -I../include -o xstimer xstimer.c $(LDFLAGS) 

xtimer: xtimer.c  
	$(CC) $(CFLAGS) -I../include -o xtimer xtimer.c $(LDFLAGS) $(HPM_LDFLAGS)

xhpm: xhpm.c  
	$(CC) $(CFLAGS) $(HPM_CFLAGS) -I../include -o xhpm xhpm.c ../src/ipm_hpm.c $(LDFLAGS) $(HPM_LDFLAGS)

xmemusage: xmemusage.c  
	$(CC) $(CFLAGS) -I../include -I../src/ -o xmemusage xmemusage.c $(LDFLAGS) $(HPMLIB)

xexecinfo: xexecinfo.c  
	$(CC) $(CFLAGS) -I../include -I../src/ -o xexecinfo xexecinfo.c $(LDFLAGS) $(HPMLIB)

clean:

	rm -f log.xml *.o xipm_unit xstimer xtimer xenv xpcontrol_c xpcontrol_f xbit xlog xhashmap xpapi xmask xtaskdata xswitch xmalloc xlabel xhash xhpm xexecinfo xmemusage xapp_serial



