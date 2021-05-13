# Firmware

This document details how to setup the CameraThing's firmware and perform development on it.



## Debugging

The image from the camera can be outputted over serial by using `frameBufferToSerial` within the `takePicture` method. This is useful to make sure everything's wired up correctly.