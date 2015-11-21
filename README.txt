Assumptions:
##Person entering the house inferences
1) door_open -> motion_detected -> key_detected 
	or
   door_open -> key_detected -> motion_detected
	
	We are considering above series of events as user entered home. In above cases if key is not detected then the entered person is an intruder.

## Person exiting the house inferences
2) motion_detected -> door_closed
	We are considering above series of events as a person exited the house.

## Unwanted motion in the house
3) motion_detected -> key_chain_not_detected
	Here, irrespective of door being close or open, security alert has been raised

## Other inferences
4) Other inference rules are tried to be kept in accordance with above two inferences.



