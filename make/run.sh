cd ../obj/gateway_front_tier/
gnome-terminal -e "./gateway_front_tier ../../SampleConfigurationFiles/SampleGatewayConfigurationFile.txt"
cd ../gateway_back_tier/
gnome-terminal -e "./gateway_back_tier ../../SampleConfigurationFiles/SampleGatewayConfigurationFile.txt ../../SampleConfigurationFiles/SampleStorageFile.txt"
cd ../door_sensor/
gnome-terminal -e "./door_sensor ../../SampleConfigurationFiles/SampleSensorConfigurationFile2.txt ../../SampleConfigurationFiles/SampleDoorSensorInputFile.txt"
cd ../motion_sensor/
gnome-terminal -e "./motion_sensor ../../SampleConfigurationFiles/SampleSensorConfigurationFile1.txt ../../SampleConfigurationFiles/SampleSensorInputFile.txt"
cd ../key_chain_sensor/
gnome-terminal -e "./key_chain_sensor ../../SampleConfigurationFiles/SampleSensorConfigurationFile3.txt ../../SampleConfigurationFiles/SampleSensorInputFile.txt"
cd ../device/
gnome-terminal -e "./device ../../SampleConfigurationFiles/SampleSmartDeviceConfigurationFile1.txt"
