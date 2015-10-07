CXXFLAGS = -std=gnu++11  -I/usr/local/include/YSE
OBJS = prueba.o 
LIBS =  -lYSE -lsndfile -lportaudio -lboost_atomic
TARGET = pruebasonido
$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
all:$(TARGET)
clean:
	rm -f $(OBJS) $(TARGET)
