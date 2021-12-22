cmd_/home/pi/lux_lcd/Module.symvers := sed 's/ko$$/o/' /home/pi/lux_lcd/modules.order | scripts/mod/modpost -m -a   -o /home/pi/lux_lcd/Module.symvers -e -i Module.symvers   -T -
