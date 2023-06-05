# ESP32StepperController
## Project Video
[Watch the video](https://youtu.be/t80UnqdxAfQ)



# Autostart the python program on the Jetson:

https://forums.developer.nvidia.com/t/jetson-nano-auto-run-python-code-when-power-up/108999/4

## Generate export PYTHONPATH line
```
import sys

# Get the current sys.path array
paths = sys.path

# Join the paths using colons as separators
pythonpath = ":".join(paths)

# Export the PYTHONPATH environment variable
export_command = f'export PYTHONPATH="{pythonpath}"'
print(export_command)
```

## /usr/local/bin/mything.sh

```
#!/bin/bash
export PYTHONPATH="<python path from sys.path here>"
python3 /home/nic/Desktop/ESP32StepperController/i2ctest.py
```

## /etc/systemd/system/mything.service

```
[Unit]
Description=mything: run the steppers
After=multi-user.target

[Service]
ExecStart=/usr/local/bin/mything.sh
Restart=always
StartLimitInterval=10
RestartSec=10

[Install]
WantedBy=multi-user.target
```
