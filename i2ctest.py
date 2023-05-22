import time
from smbus2 import SMBus

FOLLOWER_ADDRESS = 0x23

FORWARD = 0x00
BACKWARD = 0x01
TURN_FORWARD = 0x02
TURN_BACKWARD = 0x03



# byte 0: 
# data = [0x48, 0x69, 0x21, 0x00]

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


# numbers should be little endian i.e number 0x01234567 should be [0x67, 0x45, 0x23, 0x01]

# format: [type, dist, dist, dist, dist, angle] (angle ignored in forward and backward cases, but it should still be sent)
instructions = []

instr1 = [0x00]
instr1 += convert_to_hex_bytes(2500)
instr1.append(convert_to_signed_hex_byte(0))

instr2 = [0x03]
instr2 += convert_to_hex_bytes(3500)
instr2.append(convert_to_signed_hex_byte(67))

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
    for data in instructions:
        bus=SMBus(1)
        for i in range(len(data)):
            bus.write_byte(FOLLOWER_ADDRESS, data[i])
            print("Sent: ", data[i])
        bus.close()
        time.sleep(5)



