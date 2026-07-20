srcs := preload.so toy_launcher
all: $(srcs)

%.so: %.cpp
	hipcc $^ -o $@ --shared -fPIC

toy_launcher: toy_launcher.cpp
	hipcc -g $^ -o $@ -L/opt/rocm/lib -lhsa-runtime64
	#hipcc $^ -o $@ -L/opt/rocm-7.0.2/lib -lhsa-runtime64

clean:
	rm -f *.so toy_launcher


