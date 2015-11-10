echo "Building Door Sensor"
make -f door_sensor_makefile.mk
echo "Building Motion Sensor"
make -f motion_sensor_makefile.mk
echo "Building Key Chain Sensor"
make -f key_chain_sensor_makefile.mk
echo "Building Back Tier Gateway"
make -f gateway_back_tier_makefile.mk
echo "Building Front Tier Gateway"
make -f gateway_front_tier_makefile.mk
echo "Building Security Device"
make -f device_makefile.mk
