# Arbitrated vehicle safety state. safety_manager is the only node allowed
# to publish this; all actuation nodes must gate their outputs on it.

builtin_interfaces/Time stamp

uint8 NORMAL         = 0
uint8 AVOID          = 1
uint8 BRAKE          = 2
uint8 REVERSE         = 3
uint8 RECOVER         = 4
uint8 EMERGENCY_STOP  = 5

uint8 state
string reason
