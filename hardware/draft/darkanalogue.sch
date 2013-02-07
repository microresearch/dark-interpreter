EESchema Schematic File Version 2  date Thu 07 Feb 2013 17:54:54 GMT
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:stm32
LIBS:SparkFunEagle-6-6-07
LIBS:maxim-filter
LIBS:ad620
LIBS:codecshield_-cache
LIBS:dark-cache
EELAYER 24  0
EELAYER END
$Descr A4 11700 8267
Sheet 2 2
Title ""
Date "7 feb 2013"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L LM358 U?
U 1 1 5113EA67
P 1700 5500
F 0 "U?" H 1650 5700 60  0000 L CNN
F 1 "LM358" H 1650 5250 60  0000 L CNN
	1    1700 5500
	1    0    0    -1  
$EndComp
Text Notes 9000 5600 0    118  ~ 0
pads: \n\n- ad620 3x\n- signal in/out\n- amp pads\n- clock pad\n- 40106 pads\n- pwm out pad\n- 40106 distort pads\n
Text Notes 700  6750 0    118  ~ 0
- headphone or line out from codec\n- multiple 40106 configs\n- test filter/mixer\n- switchable amps on input\n- switch on/off clock for maxim
$Comp
L MAX7409CPA U?
U 1 1 5113E94F
P 2100 1650
F 0 "U?" H 1700 2300 50  0000 L BNN
F 1 "MAX7400" H 1700 950 50  0000 L BNN
F 2 "maxim-filter-DIL08" H 2100 1800 50  0001 C CNN
	1    2100 1650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5113E94E
P 2700 2600
F 0 "#PWR?" H 2700 2600 30  0001 C CNN
F 1 "GND" H 2700 2530 30  0001 C CNN
	1    2700 2600
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 5113E94D
P 2700 2350
F 0 "C?" H 2750 2450 50  0000 L CNN
F 1 "100N" H 2750 2250 50  0000 L CNN
	1    2700 2350
	1    0    0    -1  
$EndComp
Text Label 1150 1950 0    60   ~ 0
CLOCK
Text Label 1000 1350 0    60   ~ 0
FILTERIN
Text Label 3050 1450 0    60   ~ 0
OUT
$Comp
L +5V #PWR?
U 1 1 5113E94C
P 2100 600
F 0 "#PWR?" H 2100 690 20  0001 C CNN
F 1 "+5V" H 2100 690 30  0000 C CNN
	1    2100 600 
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 5113E94B
P 1500 900
F 0 "C?" H 1550 1000 50  0000 L CNN
F 1 "100N" H 1550 800 50  0000 L CNN
	1    1500 900 
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5113E94A
P 1500 1200
F 0 "#PWR?" H 1500 1200 30  0001 C CNN
F 1 "GND" H 1500 1130 30  0001 C CNN
	1    1500 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 1950 1500 1950
Connection ~ 2100 700 
Wire Wire Line
	1500 700  2100 700 
Wire Wire Line
	2700 850  2100 850 
Wire Wire Line
	2700 1250 2700 850 
Wire Wire Line
	2100 2550 2700 2550
Wire Wire Line
	2100 2450 2100 2550
Connection ~ 2700 2050
Wire Wire Line
	2700 2150 2700 1850
Wire Wire Line
	2700 2600 2700 2550
Wire Wire Line
	2700 1450 3300 1450
Wire Wire Line
	2100 600  2100 850 
Wire Wire Line
	1500 1100 1500 1200
Wire Wire Line
	1000 1350 1500 1350
$Comp
L 40106 U?
U 1 1 5113E8E2
P 8350 1100
F 0 "U?" H 8450 1250 50  0000 L CNN
F 1 "40106" H 8400 950 50  0000 L CNN
	1    8350 1100
	1    0    0    -1  
$EndComp
$Comp
L 4066 U?
U 1 1 5113E8DC
P 10400 1000
F 0 "U?" H 10600 851 40  0000 C CNN
F 1 "4066" H 10600 1150 40  0000 C CNN
	1    10400 1000
	1    0    0    -1  
$EndComp
Text GLabel 1500 3100 0    60   Input ~ 0
AVDD
Wire Wire Line
	1500 3250 1500 3100
$Comp
L AD620 IC1
U 1 1 51138567
P 1800 4050
F 0 "IC1" H 1500 4250 50  0000 L BNN
F 1 "AD620" H 1500 4150 50  0000 L BNN
F 2 "ad620-DIL8" H 1800 4200 50  0001 C CNN
	1    1800 4050
	1    0    0    -1  
$EndComp
$EndSCHEMATC
