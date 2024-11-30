# S5L8442Pwnage2

***Important note**: although this code is unlikely to permanently break anything, you still should be very careful. I'm not responsible for any possible damage this might cause*

Implementation of Pwnage 2.0 exploit for S5L8442 SoC which is used in iPod shuffle 3. It allows you to:

* Dump arbitrary memory
* Decrypt Image1-wrapped firmwares
    * ...as well as decrypt/encrypt arbitrary data with embedded GID key
* Boot custom 2nd-stage images

I only tested this on a M4 Max MacBook Pro, but given that it doesn't use any special USB haxx, it should run anywhere

For more info about Pwnage 2.0 itself, there is a good [article](https://freemyipod.org/wiki/Pwnage_2.0) on freemyipod wiki (the only thing I'd correct is that SecureROMs are not vulnerable to this bug because they are based on entirely different code)

## How to run
### Prerequisites

* iPod shuffle 3
    * Firmware [image](https://secure-appldnld.apple.com/iPod/SBML/osx/bundles/061-6315.20090526.AQS4R/iPod_132.1.1.ipsw) and ability to restore it
        * Finder in modern macOS can do it, even though it's very buggy

* Python 3
    * `pyusb` is the only external dependency
        * Available from **pip**

### Entering DFU on iPod shuffle 3

As you could notice, there is not a single button on that device, only 3-position switch

The only way to enter DFU is flashing a firmware with broken 2nd-stage bootloader:

1. Download the IPSW linked above
2. Unzip it
3. Open `D98.Bootloader.rb3` in a hex-editor and replace a random byte or 2 somewhere in a middle of the file
4. ZIP it back and rename file extension to `.ipsw`
5. Flash the resulting file into your iPod with Finder
6. At some point it will fail or get stuck
7. Unplug the iPod

As soon as Finder/iTunes detects an iPod in DFU mode, it will try to download a recovery image and upload **WTF** from it

We want to avoid it - you can either send SIGSTOP signal to `AMP*` processes on your Mac, or just temporarily disable Internet connection (so it cannot download the recovery image)

You know you did everything right if you see this on USB:

```
USB DFU Device:

  Product ID:	0x1224
  Vendor ID:	0x05ac (Apple Inc.)
  Version:	0.01
  Serial Number:	84420000000000ɧ
  Speed:	Up to 480 Mb/s
  Manufacturer:	Apple, Inc.
  Location ID:	0x00100000 / 1
  Current Available (mA):	500
  Current Required (mA):	100
  Extra Operating Current (mA):	0
```

Garbage in the end of serial number string is there by default, because they overwrite the end of it by some pointer (?!)

### If your iPod ever gets completely stuck

There is a reset combination:

1. Unplug your iPod from Mac
2. Turn the switch to the OFF position
3. Wait 10 seconds
4. Turn the switch to any other position
5. Plug it back to your Mac and restore

### Running the exploit

When your iPod is in DFU mode, just run the following:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 pwn                                                                           
DONE
```

You know it reached pwned DFU mode if you see this on USB:

```
S5L8442 pwnDFU:

  Product ID:	0x1224
  Vendor ID:	0x05ac (Apple Inc.)
  Version:	0.01
  Serial Number:	8442000000000001
  Speed:	Up to 480 Mb/s
  Manufacturer:	Apple, Inc.
  Location ID:	0x00100000 / 2
  Current Available (mA):	500
  Current Required (mA):	100
  Extra Operating Current (mA):	0
```

Device name is changed to `S5L8442 pwnDFU` and serial number string is fixed

### Sample operations

Dumping ROM:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 dump /tmp/rom.bin 0x20000000 0x10000
dumping: 100%
DONE
```

Dumping SRAM:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 dump /tmp/sram.bin 0x62000000 0x80000
dumping: 100%
DONE
```

Decrypting Image1:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 image1 D98.Bootloader.rb3 /tmp/d98_boot.bin
Image1 v1.0 (8442): type: 0x3 entry: 0x0 bodylen: 0x18824 datalen: 0x19493 certoff: 0x188b0 certlen: 0xbe3
decrypting: 100%
DONE
```

Rebooting back into normal DFU:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 reboot                                                                           
DONE
```

Booting a 2nd-stage bootloader:

```
➜  S5L8442Pwnage2 git:(master) ✗ ./S5L8442Pwnage2 boot /tmp/d98_boot.bin
DONE
```

## Running your own code

Initial payload source code as well as DFU upload callback source code are available in this repository

You can modify them and recompile (requires ARM GNU toolchain)

Running makefile should yield a new `pwn.dfu` file. By default, `S5L8442Pwnage2 pwn` will use just that, but you can override what file it sends with `-o` option

`template.dfu` is here for you to run arbitrary payloads - just put your code at offset 0x800 (16 KiBs max) and send it to your iPod

## Precautions

* iPod shuffle 3 gets pretty hot after staying in DFU mode for a while, so do not leave it connected when you don't use it

* The exploit should also work for untethered boot as well, but you shouldn't try it, because in case of any seemingly minor problem - for instance, your custom bootloader entered an infinite loop - iPod might be unrecoverable!
    * You won't be able to enter DFU mode anymore, unless you find a "force DFU" test point on the MLB (if it even exists)

## Known issues

* Apparently padding in AES operations isn't handled properly, so for inputs not aligned to 16 bytes (AES block size) it might yield some garbage in the end of an output
    * This doesn't seem to cause many problems though, as all possible inputs are raw code images which always end with a lot of zeroes anyway

## Fun facts

* The CPU core is very cursed - none of the firmwares (ROM, WTF, bootloader, disk mode, OSOS) ever access CP15 registers, and running MCR/MRC instructions via the exploit seems to cause an exception
    * This means there is no MMU at all!
    * It also makes it very complex to understand what core this even is - `MIDR` register is also in CP15

* Even though iPod shuffle 3 was released in 2009, the S5L8442 ROM looks more similar to S5L8900 & S5L8702 ones (2007), than to S5L8730 (2009) or even S5L8720 (2008)

* SRAM is pretty large (512 KiB), but it seems that there's no DRAM, so it's the only memory available to software
    * Don't quote me on that though, as I didn't check it deeply

* Logging in post-ROM stages seems to be done via *semihosting*
    * Whenever it wants to print something, in essence it just does a certain supervisor call
    * External debugger traps it and fetches the string
    * If there's no external debugger, software handler just returns
    * Is there even normal UART?

* Firmwares don't seem to use EFI for anything unlike bigger non-iOS iPods

## Credits

* iPhone Dev Team - for the original Pwnage 2.0 bug & exploit
* q3k - for sharing a lot of research on iPod bootroms and helping me
