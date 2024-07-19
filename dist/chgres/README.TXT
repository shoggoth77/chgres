---------------------------------------------------------------------
Screen resolution changer
Based on CHGRES.PRG for MagiC (https://github.com/th-otto/MagicMac)
These files are licensed according to GPLv3, see LICENSE file

2018, Andreas Kromke
2022, Thorsten Otto
2022, Anders Granlund
---------------------------------------------------------------------


**********************************************************************
             This is test build for Vampire V4SA.
  consider all other hardware unsupported in this particular build.
**********************************************************************


This program attempts to be a generic resolution switcher for
different Atari hardware/software combinations.

It aims to recognize the following hardware:
    Atari Shifter       (VDI, NVDI)
    Atari Videl         (VDI, NVDI)
 *  ET4000              (NVDI-VGA)
    Vamire SAGA         (VDI, NVDI, fVDI)

It will recognize the following AES:
    TOS2+
    EmuTOS
    MagiC
    MiNT + NAES
    MiNT + XaAES
    MiNT + MyAES

It may work somewhat on other AES but its unlikely that it would
be able to save the settings.

---------------------------------------------------------------------
Requirements:
---------------------------------------------------------------------
This program requires you to have WDIALOG in your auto folder unless
your AES already has that functionality built in.



---------------------------------------------------------------------
Setting resolutions
---------------------------------------------------------------------
On-the-fly resolution switch is performed if your combination
of AES/hardware/driver supports it. In those cases it usually
only need to restart AES itself.
In other cases it will warm-reboot the machine so that it can
enter the selected resolution.

---------------------------------------------------------------------
Saving resolution
---------------------------------------------------------------------
The selected resolution is always written to the config file that
is relevant for your AES and/or VDI.
I highly recommend keeping a backup of the config file(s)
in case something goes wrong!

NAES        <naes folder>/n_desk.inf
XaAES       <xaaes folder>/xaaes.cnf
MyAES       <myaes folder>/myaes.cnf
MagiC       c:\magx.inf
EmuTos      c:\emudesk.inf
Tos2+       c:\newdesk.inf
Tos1        c:\desktop.inf

NVDI-VGA    c:\nvdivga.inf
fVDI        c:\fvdi.sys



---------------------------------------------------------------------
Note about fVDI:
---------------------------------------------------------------------
fVDI is only supported for the Vampire at the moment.
You are much better off using NVDI but if you insist then CHGRES is
able to change change fVDI resolutions with some caveats:

Your FVDI.SYS can only contain a single "01r" mode line.
It will not work if you have configured multiple modes and some kind of user-select script.
You also need to add a couple of special lines to tells chgres.prg which drivers are available
for each supported bitdepth.

This example config will allow reschg.prg to change fVDI resolution:

#[driver16] saga16.sys
#[driver32] saga32.sys
01r saga16.sys mode 1280x720x16@60
