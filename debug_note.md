# For bug fix

## What components can be connected to pins 0/1 (RX/TX) without preventing uploading of code?

For other connection issues, such as COM connection issues, try these steps:

Press the little "Reset" button (or "RST") on your board.
Unplug the board's USB cable.
Unplug anything from the TX/RX pins.
Close all Arduino IDE windows including the serial monitor.
Plug the USB cable back in and wait a few seconds.
Reopen the Arduino window and choose the correct port under Tools > Port. If you see multiple unlabeled COM ports, you can use the "Get Board Info" option or simple trial and error to figure out the right port.
Open the serial monitor and make sure the baud rate matches the one found in your code.
Upload the sketch.
If the upload fails, wait a few seconds and try it a second time.
