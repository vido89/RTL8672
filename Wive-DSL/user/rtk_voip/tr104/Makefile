#AR = mips-linux-ar
include ../.linux_config

# parameter structure (VoIP)
VOIP_TR104_LIB=voip_tr104.a
OBJS = prmt_voiceservice.o prmt_capabilities.o prmt_voice_profile.o prmt_voice_profile_line.o
OBJS += porting_tr104.o

#for ipc
OBJS +=cwmpevt.o

# other assistant 
OBJS += str_utility.o str_mib.o mib_tr104_8672.o 

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X), y)
	MIB_INCLUDE=-I../../../include -I../../mib
endif
 
CFLAGS += -I../ -I../includes/  $(MIB_INCLUDE) -Wall -O 
CFLAGS += -I../../cwmp-tr069/cwmpClient/ -I../../cwmp-tr069/include/ -I../../boa/src-project/LINUX -I../flash/

all: $(VOIP_TR104_LIB)

$(VOIP_TR104_LIB): $(OBJS) 
	$(AR) rcs $(VOIP_TR104_LIB) $(OBJS) 

%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

dep:
	$(CC) $(CFLAGS) -MM *.c > .depend

clean:
	-rm -f $(EXEC) *.o
	-rm -f $(EXEC) *.a
	-rm -f .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
