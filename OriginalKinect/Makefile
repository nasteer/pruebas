CXXFLAGS = -O2 -g -Wall -fmessage-length=0 `pkg-config opencv --cflags ` -L /lib64  -I /usr/include/libusb-1.0
OBJS = freenectopencvmat.o 
LIBS = `pkg-config opencv --libs` -lfreenect -lusb-1.0 -l pthread
TARGET = kinectopencv
$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
all:$(TARGET)
clean:
	rm -f $(OBJS) $(TARGET)
