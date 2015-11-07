echo "Building Sensor"
make -f sensor_makefile.mk
echo "Building Gateway"
make -f gateway_makefile.mk
echo "Building Device"
make -f device_makefile.mk
