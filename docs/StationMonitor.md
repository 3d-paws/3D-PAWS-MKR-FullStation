# Station Monitor
[←Top](../README.md)<BR>

With Serial Console jumper in place at boot the Station Monitor run upon setup completion.  The station monitor will display various sensor values. Updating every second. A good way to check rain gauge tips and wind direction.

The station monitor will run for 10 minutes. After which, the main work loop will be started. This is to safeguard against the jump wire being left connected on a deployment in the field.

Caveat: If you reapply the jumper after removing, the station monitor will run for the remaining time left of the 10 minutes allotted.
