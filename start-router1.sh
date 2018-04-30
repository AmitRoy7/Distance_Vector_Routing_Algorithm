#!/bin/bash


xterm -title A -hold -e ./dv_routing A 2000 configA.txt & sleep 1
xterm -title B -hold -e ./dv_routing B 2001 configB.txt & sleep 1
xterm -title C -hold -e ./dv_routing C 2002 configC.txt & sleep 1
xterm -title D -hold -e ./dv_routing D 2003 configD.txt & sleep 1
xterm -title E -hold -e ./dv_routing E 2004 configE.txt & sleep 1
xterm -title F -hold -e ./dv_routing F 2005 configF.txt & sleep 1
