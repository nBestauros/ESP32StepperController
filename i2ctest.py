import time
from smbus2 import SMBus
import Jetson.GPIO as GPIO
import random
import atexit

FOLLOWER_ADDRESS = 0x23

FORWARD = 0x00
BACKWARD = 0x01
TURN_FORWARD = 0x02
TURN_BACKWARD = 0x03

def micros():
    return int(time.time() * 1000000)

last_execution_time = micros()

GPIO.setmode(GPIO.BOARD)
interrupt_pin = 29

GPIO.setup(interrupt_pin, GPIO.IN)

@atexit.register
def cleanup_gpio():
    GPIO.cleanup()

def interrupts():
    GPIO.add_event_detect(interrupt_pin, GPIO.RISING, callback=pin_change_callback) # safety manuever interrupt whenever interrupt_pin goes high

def no_interrupts():
    GPIO.remove_event_detect(interrupt_pin)

def convert_to_hex_bytes(number):
    if number < 0 or number > 0xFFFFFFFF:
        raise ValueError("Number is out of range: [0, 0xFFFFFFFF]")
    hex_string = hex(number)[2:].zfill(8)
    hex_bytes = [int(hex_string[i:i+2], 16) for i in range (6, -1, -2)] # store in little endian.

    return hex_bytes

def convert_to_signed_hex_byte(number):
    if number <-90 or number > 90:
        raise ValueError("Number is out of range: [-90, 90]")

    signed_byte = number & 0xFF

    return signed_byte

emergency_data_1 = [0x03]
emergency_data_1 += convert_to_hex_bytes(4000)
emergency_data_1.append(convert_to_signed_hex_byte(-45))

emergency_data_2 = [0x03]
emergency_data_2 += convert_to_hex_bytes(4000)
emergency_data_2.append(convert_to_signed_hex_byte(-45))

def pin_change_callback(channel):
    try:
        global last_execution_time
        last_execution_time = micros()
        no_interrupts()
        bus=SMBus(1)
        num = random.randint(1,2)
        if num == 1:
            emergency_data = emergency_data_1
        elif num == 2:
            emergency_data = emergency_data_2
        for i in range(len(emergency_data)):
            bus.write_byte(FOLLOWER_ADDRESS, emergency_data[i])
            print("Sent: ", emergency_data[i])
        bus.close()
        interrupts()
    except:
        try:
            interrupts()
            bus.close()
        except:
            pass
        return

interrupts()

# byte 0: 
# data = [0x48, 0x69, 0x21, 0x00]




# numbers should be little endian i.e number 0x01234567 should be [0x67, 0x45, 0x23, 0x01]

# format: [type, dist, dist, dist, dist, angle] (angle ignored in forward and backward cases, but it should still be sent)
instructions = []

instr1 = [0x00]
instr1 += convert_to_hex_bytes(2500)
instr1.append(convert_to_signed_hex_byte(0))

instr2 = [0x03]
instr2 += convert_to_hex_bytes(3500)
instr2.append(convert_to_signed_hex_byte(67))

def instr_gen():
    instr = [random.choice([FORWARD, BACKWARD, TURN_FORWARD, TURN_BACKWARD])]
    instr += convert_to_hex_bytes(random.randint(50, 6000))
    instr.append(convert_to_signed_hex_byte(random.randint(-89, 89)))
    return instr

instructions.append(instr1)
instructions.append(instr2)
print(instructions)

# while True:                                              
#     bus = SMBus(1)
#     for i in range(len(data)):
#         bus.write_byte(FOLLOWER_ADDRESS, data[i])
#         print("Sent: ",data[i])
#         time.sleep(0.1)
#     bus.close()



while True:
    current_time = micros()
    if current_time-last_execution_time < 5000:
        print(current_time-last_execution_time)
        continue
    try:
        data = instr_gen() #generate new movement
        last_execution_time = current_time
        no_interrupts()
        bus=SMBus(1)
        for i in range(len(data)):
            bus.write_byte(FOLLOWER_ADDRESS, data[i])
            print("Sent: ", data[i])
        bus.close()
        interrupts()
    except:
        try:
            print("excepted")
            interrupts()
            bus.close()
        except:
            continue



