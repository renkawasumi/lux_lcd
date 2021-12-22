cmd_/home/pi/lux_lcd/modules.order := {   echo /home/pi/lux_lcd/smile.ko; :; } | awk '!x[$$0]++' - > /home/pi/lux_lcd/modules.order
