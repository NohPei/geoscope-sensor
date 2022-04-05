* [x] Minimal Changes (Order Swaps, etc.)
* [x] Removing unnecessary functions
* [ ] What is happening every 43s to lose a packet?
    * It appears to be related to timestamp roundoff. Possibly passing the heat test was a red herring
    * [x] Send mactime with packets for better timestamping
* [x] MQTT Library change
* [ ] Use Library WiFi Manager
* [ ] Switch wire format to MessagePack
* [x] Control sample timer from mactime difference
