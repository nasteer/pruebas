CXXFLAGS = -std=gnu++11 -O2 -g -Wall  -fmessage-length=0 `pkg-config opencv --cflags ` -L /lib64  -I /usr/include/libusb-1.0 -I/usr/local/include/YSE
OBJS = kinectPrueba.o 
LIBS = `pkg-config opencv --libs` -lfreenect -lusb-1.0 -l pthread -lYSE -lsndfile -lportaudio -lboost_atomic
TARGET = kinectCaptura
$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
all:$(TARGET)
clean:
	rm -f $(OBJS) $(TARGET)
