[**Back to tBOX210 BSP**](../README.md)


---


# Test Guide

This guide describes the basic functional tests, stress test items, and real-time latency tests performed 
on the Axiomtek tBOX210 platform running QNX SDP 8.0.5.

* [Basic Function Test Items](#basic-function-test-items)
    - [Storage](#storage)
    - [Display](#display)
    - [Ethernet](#ethernet)
    - [USB](#usb)
    - [SMBus](#smbus)
    - [Digital I/O Connector](#digital-io-connector)
        - [Input Test (Dry Contact)](#input-test-dry-contact)
        - [Input Test (Wet Contact)](#input-test-wet-contact)
        - [Output Test](#output-test)
    - [Watchdog](#watchdog)
    - [Hardware Monitor](#hardware-monitor)
    - [Full-Size PCI Express Mini Card Slot (SMC1)](#full-size-pci-express-mini-card-slot-smc1)
    - [M.2 2230 Key E slot (SM2E1)](#m2-2230-key-e-slot-sm2e1)
    - [M.2 Key B Connector (SM2B1)](#m2-key-b-connector-sm2b1)

* [Stress Test Items](#stress-test-items)
    - [CPU Stress Test](#cpu-stress-test)
    - [Memory Stress Test](#memory-stress-test)
    - [USB Stress Test](#usb-stress-test)
    - [NVMe Stress Test](#nvme-stress-test)
    - [SATA Stress Test](#sata-stress-test)
    - [Ethernet Stress Test](#ethernet-stress-test)
    - [Thermal Stress Test](#thermal-stress-test)

* [Real-time Latency Test](#real-time-latency-test)
    - [Idle](#idle)
    - [Under NVMe Drive Stress Test](#under-nvme-drive-stress-test)
    - [Under Memory Stress Test](#under-memory-stress-test)
    - [Under Ethernet Stress Test](#under-ethernet-stress-test)
    - [Under CPU Stress Test](#under-cpu-stress-test)

> **NOTE**:

**Test platform:** tBOX210 (PSB558 VER: B10) with Intel Core i3-13100TE processor, and PCH=Q670E.


---



# Basic Function Test Items


<div align="right">

[**Back to Top**](#test-guide)

</div>




## Storage



<div align="right">

[**Back to Top**](#test-guide)

</div>



#### NVMe



<div align="right">

[**Back to Top**](#test-guide)

</div>


The system boots from the NVMe drive. Check the `/dev` directory to verify that the NVMe device is detected: 



![tbox210_boot_from_nvme_card_demo](./pics/tbox210_boot_from_nvme_card_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



#### SATA



<div align="right">

[**Back to Top**](#test-guide)

</div>


Connect a SATA drive to the system. Check the `/dev` directory to verify that the SATA drive is detected:


![One_sata_device_attached_to_tbox210](./pics/One_sata_device_attached_to_tbox210.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Display




<div align="right">

[**Back to Top**](#test-guide)

</div>


| Display Interface | CPU               | Result |
| :---------------: | :---------------: | :----: |
| HDMI              | Intel i3-13100TE  | OK     |




Start the QNX graphics system by running the following script:


```console
# /scripts/start-graphics.sh
```

![display-graphics](./pics/display-graphics.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


---



## Ethernet





<div align="right">

[**Back to Top**](#test-guide)

</div>



The tBOX210 provides five Ethernet ports: **LAN0, LAN1, LAN2, LAN3, and LAN4**.


### LAN0


![LAN0_general_demo](./pics/LAN0_general_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN0_ifconfig_demo](./pics/LAN0_ifconfig_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN0_ping_demo](./pics/LAN0_ping_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


### LAN1


![LAN1_general_demo](./pics/LAN1_general_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![M12_X_coded_demo](./pics/M12_X_coded_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN1_ifconfig_demo](./pics/LAN1_ifconfig_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN1_ping_demo](./pics/LAN1_ping_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


### LAN2


![LAN2_general_demo](./pics/LAN2_general_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![M12_X_coded_demo](./pics/M12_X_coded_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN2_ifconfig_demo](./pics/LAN2_ifconfig_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN2_ping_demo](./pics/LAN2_ping_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



### LAN3



![LAN3_general_demo](./pics/LAN3_general_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![M12_X_coded_demo](./pics/M12_X_coded_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN3_ifconfig_demo](./pics/LAN3_ifconfig_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN3_ping_demo](./pics/LAN3_ping_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


### LAN4



![LAN4_general_demo](./pics/LAN4_general_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![M12_X_coded_demo](./pics/M12_X_coded_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN4_ifconfig_demo](./pics/LAN4_ifconfig_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![LAN4_ping_demo](./pics/LAN4_ping_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## USB


<div align="right">

[**Back to Top**](#test-guide)

</div>


|USB Interface Type|
| :---: | 
|USB 3.2|




![USB3.2-demo-1](./pics/USB3.2-demo-1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


Boot the system from the NVMe drive, then insert a USB flash drive. Check the `/dev` directory to verify that the USB flash drive is detected:


![USB3.2-demo-2](./pics/USB3.2-demo-2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>




---


## SMBus





<div align="right">

[**Back to Top**](#test-guide)

</div>


Check the `/dev` directory to verify that `smb0` is detected: 

![tbox210_smbus_demo](./pics/tbox210_smbus_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Digital I/O Connector




<div align="right">

[**Back to Top**](#test-guide)

</div>



![tBOX210_dio_demo_1](./pics/tBOX210_dio_demo_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![tBOX210_dio_demo_2](./pics/tBOX210_dio_demo_2.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



A shared library has been developed to support the DI/DO configuration.


Three test applications are provided:

- `tbox210_dio_input_test`
- `tbox210_dio_output_high_test`
- `tbox210_dio_output_low_test`


![tBOX210_dio_demo_3](./pics/tBOX210_dio_demo_3.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>




### Input Test (Dry Contact) 


Dry Contact Logic Levels:

- **Logic Level 0:** Close to Ground
- **Logic Level 1:** Open



![input_dry_contact_wiring_sample](./pics/input_dry_contact_wiring_sample.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### No DI Pins Grounded

![DIO_input_test_without_pins_grounded](./pics/DIO_input_test_without_pins_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>

#### Pin 7 Grounded

Connect Pin 7 to Pin 5 (Pin 7 grounded):


![DIO_input_test_with_pin7_grounded](./pics/DIO_input_test_with_pin7_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Pin 8 Grounded

Connect Pin 8 to Pin 5 (Pin 8 grounded):


![DIO_input_test_with_pin8_grounded](./pics/DIO_input_test_with_pin8_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 9 Grounded

Connect Pin 9 to Pin 5 (Pin 9 grounded):


![DIO_input_test_with_pin9_grounded](./pics/DIO_input_test_with_pin9_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 10 Grounded

Connect Pin 10 to Pin 5 (Pin 10 grounded):


![DIO_input_test_with_pin10_grounded](./pics/DIO_input_test_with_pin10_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 11 Grounded

Connect Pin 11 to Pin 5 (Pin 11 grounded):


![DIO_input_test_with_pin11_grounded](./pics/DIO_input_test_with_pin11_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 12 Grounded

Connect Pin 12 to Pin 5 (Pin 12 grounded):


![DIO_input_test_with_pin12_grounded](./pics/DIO_input_test_with_pin12_grounded.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



### Input Test (Wet Contact) 


Wet Contact Logic Levels:

- **Logic Level 1:** +/- 3 VDC max
- **Logic Level 0:** +/- 10 VDC min. to +/- 30 VDC max. (Source to Digital Input)



![input_wet_contact_wiring_sample](./pics/input_wet_contact_wiring_sample.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 7


![wet-input-pin7-1](./pics/wet-input-pin7-1.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



![wet_input_P7_demo_low_1](./pics/wet_input_P7_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![wet-input-pin7-2](./pics/wet-input-pin7-2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![wet_input_P7_demo_low_2](./pics/wet_input_P7_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Pin 8

![wet_input_P8_demo_low_1](./pics/wet_input_P8_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![wet_input_P8_demo_low_2](./pics/wet_input_P8_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Pin 9


![wet_input_P9_demo_low_1](./pics/wet_input_P9_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![wet_input_P9_demo_low_2](./pics/wet_input_P9_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Pin 10

![wet_input_P10_demo_low_1](./pics/wet_input_P10_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![wet_input_P10_demo_low_2](./pics/wet_input_P10_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Pin 11

![wet_input_P11_demo_low_1](./pics/wet_input_P11_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![wet_input_P11_demo_low_2](./pics/wet_input_P11_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Pin 12

![wet_input_P12_demo_low_1](./pics/wet_input_P12_demo_low_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![wet_input_P12_demo_low_2](./pics/wet_input_P12_demo_low_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



### Output Test




<div align="right">

[**Back to Top**](#test-guide)

</div>



![tbox210_digital_output_wiring_general](./pics/tbox210_digital_output_wiring_general.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_digital_output_test_demo](./pics/tbox210_digital_output_test_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![DIO_output_high_test](./pics/DIO_output_high_test.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![DIO_output_low_test](./pics/DIO_output_low_test.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Watchdog



<div align="right">

[**Back to Top**](#test-guide)

</div>



The `WDTRST#` signal is provided by the `F81804` Super I/O controller.

A shared library, `libtbox210_wdt.so`, has been developed to support the watchdog function.

The following images show an example of using this library:



![tbox210_watchdog_test_demo_1](./pics/tbox210_watchdog_test_demo_1.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



![tbox210_watchdog_test_demo_2](./pics/tbox210_watchdog_test_demo_2.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Hardware Monitor



<div align="right">

[**Back to Top**](#test-guide)

</div>




A shared library, `libtbox210_hw_mon.so`, has been developed to provide hardware monitoring functionality.

The following images show an example of using this library:


![tbox210_hw_from_bios](./pics/tbox210_hw_from_bios.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![tbox210_hw_mon_demo](./pics/tbox210_hw_mon_demo.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---

## Full-Size PCI Express Mini Card Slot (SMC1)



<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_minicardslot_smc1_demo_1](./pics/tbox210_minicardslot_smc1_demo_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_minicardslot_smc1_demo_2](./pics/tbox210_minicardslot_smc1_demo_2.png)


![mini_test_card_demo](./pics/mini_test_card_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


After installing the test card module (**WPEA-251N**), use `pci-tool -v` to verify that the device is detected.

```text
# pci-tool -v

Partition: pci

Managing Server ID: 1, pid: 20487, running
B000:D00:F00 @ idx 0
        vid/did: 8086/4630
                Intel Corporation, <device id - unknown>
        class/subclass/reg: 06/00/00
                Host-to-PCI Bridge Device

B000:D02:F00 @ idx 1
        vid/did: 8086/4692
                Intel Corporation, Alder Lake-S GT1 [UHD Graphics 730]
        class/subclass/reg: 03/00/00
                PC Compatible VGA Display Controller

B000:D08:F00 @ idx 2
        vid/did: 8086/464f
                Intel Corporation, 12th Gen Core Processor Gaussian & Neural Accelerator
        class/subclass/reg: 08/80/00
                Other Base Systems Peripheral

B000:D20:F00 @ idx 3
        vid/did: 8086/7ae0
                Intel Corporation, Alder Lake-S PCH USB 3.2 Gen 2x2 XHCI Controller
        class/subclass/reg: 0c/03/30
                USB Serial Bus Controller (Intel eXtensible HCI)
.
.
.

B002:D00:F00 @ idx 27 in slot 6 of chassis 0
        vid/did: 168c/0034
                Qualcomm Atheros, AR9462 Wireless Network Adapter
        class/subclass/reg: 02/80/00
                Other Network Controller
.
.
.

B007:D00:F00 @ idx 32 in slot 16 of chassis 0
        vid/did: 15b7/5017
                Sandisk Corp, WD SN560/SN740/SN770/SN5000 NVMe SSD
        class/subclass/reg: 01/08/02
                Non-volatile Memory Subsystem (NVMe Interface IO Controller)


#
```



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## M.2 2230 Key E slot (SM2E1)




<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_KeyE_slot_SM2E1_demo_1](./pics/tbox210_KeyE_slot_SM2E1_demo_1.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



![tbox210_KeyE_slot_SM2E1_demo_2](./pics/tbox210_KeyE_slot_SM2E1_demo_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![M.2_Key_E_test_card](./pics/M.2_Key_E_test_card.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



After inserting the test card (**RTL8822CE**), use `pci-tool -v` to verify that the device is detected.

```text
# pci-tool -v

Partition: pci

Managing Server ID: 1, pid: 20487, running
B000:D00:F00 @ idx 0
        vid/did: 8086/4630
                Intel Corporation, <device id - unknown>
        class/subclass/reg: 06/00/00
                Host-to-PCI Bridge Device

B000:D02:F00 @ idx 1
        vid/did: 8086/4692
                Intel Corporation, Alder Lake-S GT1 [UHD Graphics 730]
        class/subclass/reg: 03/00/00
                PC Compatible VGA Display Controller

B000:D08:F00 @ idx 2
        vid/did: 8086/464f
                Intel Corporation, 12th Gen Core Processor Gaussian & Neural Accelerator
        class/subclass/reg: 08/80/00
                Other Base Systems Peripheral

B000:D20:F00 @ idx 3
        vid/did: 8086/7ae0
                Intel Corporation, Alder Lake-S PCH USB 3.2 Gen 2x2 XHCI Controller
        class/subclass/reg: 0c/03/30
                USB Serial Bus Controller (Intel eXtensible HCI)
.
.
.

B002:D00:F00 @ idx 27 in slot 4 of chassis 0
        vid/did: 10ec/c822
                Realtek Semiconductor Co., Ltd., RTL8822CE 802.11ac PCIe Wireless Network Adapter
        class/subclass/reg: 02/80/00
                Other Network Controller
.
.
.

B007:D00:F00 @ idx 32 in slot 16 of chassis 0
        vid/did: 15b7/5017
                Sandisk Corp, WD SN560/SN740/SN770/SN5000 NVMe SSD
        class/subclass/reg: 01/08/02
                Non-volatile Memory Subsystem (NVMe Interface IO Controller)


#
```



<div align="right">

[**Back to Top**](#test-guide)

</div>


---


## M.2 Key B Connector (SM2B1)




<div align="right">

[**Back to Top**](#test-guide)

</div>



![tbox210_KeyB_slot_SM2B1_demo_1](./pics/tbox210_KeyB_slot_SM2B1_demo_1.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_KeyB_slot_SM2B1_demo_2](./pics/tbox210_KeyB_slot_SM2B1_demo_2.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



![M.2_Key_B_test_card](./pics/M.2_Key_B_test_card.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



After inserting the test card (**WNFQ-262ACNI**), use `pci-tool -v` to verify that the device is detected. 


```text
# pci-tool -v

Partition: pci

Managing Server ID: 1, pid: 20487, running
B000:D00:F00 @ idx 0
        vid/did: 8086/4630
                Intel Corporation, <device id - unknown>
        class/subclass/reg: 06/00/00
                Host-to-PCI Bridge Device

B000:D02:F00 @ idx 1
        vid/did: 8086/4692
                Intel Corporation, Alder Lake-S GT1 [UHD Graphics 730]
        class/subclass/reg: 03/00/00
                PC Compatible VGA Display Controller

B000:D08:F00 @ idx 2
        vid/did: 8086/464f
                Intel Corporation, 12th Gen Core Processor Gaussian & Neural Accelerator
        class/subclass/reg: 08/80/00
                Other Base Systems Peripheral

B000:D20:F00 @ idx 3
        vid/did: 8086/7ae0
                Intel Corporation, Alder Lake-S PCH USB 3.2 Gen 2x2 XHCI Controller
        class/subclass/reg: 0c/03/30
                USB Serial Bus Controller (Intel eXtensible HCI)
.
.
.

B002:D00:F00 @ idx 27 in slot 7 of chassis 0
        vid/did: 168c/003e
                Qualcomm Atheros, QCA6174 802.11ac Wireless Network Adapter
        class/subclass/reg: 02/80/00
                Other Network Controller
.
.
.

B007:D00:F00 @ idx 32 in slot 16 of chassis 0
        vid/did: 15b7/5017
                Sandisk Corp, WD SN560/SN740/SN770/SN5000 NVMe SSD
        class/subclass/reg: 01/08/02
                Non-volatile Memory Subsystem (NVMe Interface IO Controller)


#
```


<div align="right">

[**Back to Top**](#test-guide)

</div>


---

# Stress Test Items


<div align="right">

[**Back to Top**](#test-guide)

</div>



## CPU Stress Test


![cpu_stress_test_pidin_info](./pics/cpu_stress_test_pidin_info.png)




<div align="right">

[**Back to Top**](#test-guide)

</div>



| Test Cases| CPU Workers | Duration |
|:---|:---:|:---:|
| Initial Test | 1 | 5 minutes |
| Partial Load Test | Half the logical CPUs | 30 minutes |
| Full Load Test | All logical CPUs | 1 hour |
| Endurance Test | All logical CPUs | 8–24 hours |



<div align="right">

[**Back to Top**](#test-guide)

</div>




#### Initial Test



![cpu_stress_test_1x5](./pics/cpu_stress_test_1x5.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Partial Load Test


![cpu_stress_test_4x30](./pics/cpu_stress_test_4x30.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Full Load Test



![cpu_stress_test_8x60](./pics/cpu_stress_test_8x60.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Endurance Test


![cpu_stress_test_8x480](./pics/cpu_stress_test_8x480.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Memory Stress Test



<div align="right">

[**Back to Top**](#test-guide)

</div>


![mem_stress_test_pidin_info](./pics/mem_stress_test_pidin_info.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Allocate 1024 MiB and test for 5 minutes


![mem_stress_test_1024x5](./pics/mem_stress_test_1024x5.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


#### Allocate 1024 MiB and test for 30 minutes

![mem_stress_test_1024x30](./pics/mem_stress_test_1024x30.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### Allocate 2048 MiB and test for 1 hour


![mem_stress_test_2048x600](./pics/mem_stress_test_2048x60.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>




#### Allocate 2048 MiB and test for 8 hours


![mem_stress_test_2048x480](./pics/mem_stress_test_2048x480.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


> **NOTE:**
>
> - **Requested memory:** Amount of RAM requested for the test buffer.
> - **Elapsed time:** Actual measured duration.
> - **Completed passes:** Number of complete write/read verification cycles.
> - **Total data processed:** Total bytes written and read, counted together.
> - **Write throughput:** Average write speed during the write phases.
> - **Read throughput:** Average read speed during the verification phases.
> - **Data mismatches:** Number of values that differed from the expected pattern.
> - **Test status:** Pass if at least one full pass completed without detected errors.



<div align="right">

[**Back to Top**](#test-guide)

</div>


---


## USB Stress Test



<div align="right">

[**Back to Top**](#test-guide)

</div>


#### USB Drive Writing Test


![usb_stress_write_zero](./pics/usb_stress_write_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![usb_stress_write_urandom](./pics/usb_stress_write_urandom.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### USB Drive Reading Test



![usb_stress_read_zero](./pics/usb_stress_read_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![usb_stress_read_urandom](./pics/usb_stress_read_urandom.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


#### USB Drive Data Integrity Check



![usb_stress_data_integrity](./pics/usb_stress_data_integrity.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## NVMe Stress Test





<div align="right">

[**Back to Top**](#test-guide)

</div>



#### NVMe Drive Writing Test


![nvme_stress_write_zero](./pics/nvme_stress_write_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![nvme_stress_write_urandom](./pics/nvme_stress_write_urandom.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### NVMe Drive Reading Test



![nvme_stress_read_zero](./pics/nvme_stress_read_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![nvme_stress_read_urandom](./pics/nvme_stress_read_urandom.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


#### NVMe Drive Data Integrity Check



![nvme_stress_data_integrity](./pics/nvme_stress_data_integrity.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>




---


## SATA Stress Test



<div align="right">

[**Back to Top**](#test-guide)

</div>



#### SATA Drive Writing Test


![sata_stress_write_zero](./pics/sata_stress_write_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![sata_stress_write_urandom](./pics/sata_stress_write_urandom.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### SATA Drive Reading Test



![sata_stress_read_zero](./pics/sata_stress_read_zero.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![sata_stress_read_urandom](./pics/sata_stress_read_urandom.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### SATA Drive Data Integrity Check



![sata_stress_data_integrity](./pics/sata_stress_data_integrity.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Ethernet Stress Test


<div align="right">

[**Back to Top**](#test-guide)

</div>


Use `iperf3` to test network throughput. In this example, the tBOX210 operates as the client and a Linux test machine operates as the server.


#### LAN0


![LAN0_throughput_test_server](./pics/LAN0_throughput_test_server.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN0_throughput_test_client](./pics/LAN0_throughput_test_client.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### LAN1


![LAN1_throughput_test_server](./pics/LAN1_throughput_test_server.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN1_throughput_test_client](./pics/LAN1_throughput_test_client.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### LAN2


![LAN2_throughput_test_server](./pics/LAN2_throughput_test_server.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN2_throughput_test_client](./pics/LAN2_throughput_test_client.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


#### LAN3


![LAN3_throughput_test_server](./pics/LAN3_throughput_test_server.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN3_throughput_test_client](./pics/LAN3_throughput_test_client.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



#### LAN4


![LAN4_throughput_test_server](./pics/LAN4_throughput_test_server.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


![LAN4_throughput_test_client](./pics/LAN4_throughput_test_client.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Thermal Stress Test



<div align="right">

[**Back to Top**](#test-guide)

</div>


![tbox210_thermal_stress_test_demo](./pics/tbox210_thermal_stress_test_demo.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



```text

# ./thermal_stress -n 8 -d 54000 -i 1 -o thermal_stress_test_15h8u.csv

```

Analyze the data in `thermal_stress_test_15h8u.csv` using Microsoft Excel: 

![tbox210_thermal_stress_8cpu_15hour_test](./pics/tbox210_thermal_stress_8cpu_15hour_test.png)



<div align="right">

[**Back to Top**](#test-guide)

</div>


---

# Real-time Latency Test



<div align="right">

[**Back to Top**](#test-guide)

</div>




## Idle



![periodic_timer_idle](./pics/periodic_timer_idle.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


---


## Under NVMe Drive Stress Test



![periodic_timer_under_nvme_stress_test](./pics/periodic_timer_under_nvme_stress_test.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---


## Under Memory Stress Test



![periodic_timer_under_memory_stress_test](./pics/periodic_timer_under_memory_stress_test.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>



---



## Under Ethernet Stress Test



![periodic_timer_under_ethernet_stress_test](./pics/periodic_timer_under_ethernet_stress_test.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


---


## Under CPU Stress Test 



![periodic_timer_under_cpu_stress_test](./pics/periodic_timer_under_cpu_stress_test.png)


<div align="right">

[**Back to Top**](#test-guide)

</div>


---

[**Back to tBOX210 BSP**](../README.md)

