
my early linux kernel modules


PCI1:
Contains a driver to contorl digital IO of a parallel port that is connected to my computer via a PCIE slot.
I register my driver with the kernal, create a char dev, and have the driver register the PCI card. 
I write to the chardev via the command line to alter the pin states connected to the parallel port
I have leds connected to the parallelport to detect chang in voltage state of the parlallel port output.
