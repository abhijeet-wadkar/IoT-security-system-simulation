cd ../obj/gateway_front_tier/
gnome-terminal -e "./gateway_front_tier ../../SampleInput/SampleGatewayConfigurationFile.txt"
cd ../gateway_back_tier/
gnome-terminal -e "./gateway_back_tier ../../SampleInput/SampleGatewayConfigurationFile.txt"
cd ../door_sensor/
gnome-termial -e "./door_sensor ../../SampleInput/SampleSensorConfigurationFile2.txt ../../SampleInput/SampleDoorSensorInputFile.txt"
cd ../motion_sensor/
gnome-termnal -e "./motion_sensor ../../SampleInput/SampleSensorConfigurationFile1.txt ../../SampleInput/SampleSensorInputFile.txt"
cd ../key_chain_sensor/
gnome-terminal -e "./key_chain_sensor ../../SampleInput/SampleSensorConfigurationFile3.txt ../../SampleInput/SampleSensorInputFile.txt"
cd ../device/
gnome-terminal -e "./device ../../SampleInput/SampleSmartDeviceConfigurationFile1.txt"
