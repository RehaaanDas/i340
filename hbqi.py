#这是汇编器为i340
#从 .hb6 汇编向 .mbdm6

import sys

with open(sys.argv[1], "r") as file:
    program = file.readlines()

def append(data):
    with open(sys.argv[2], "a") as file:
        file.write(data)

with open(sys.argv[2], "w") as file:
    file.write = ""

for num, instruction in enumerate(program):
    instruction = instruction.split(" ")
    binary = ""

    type = 0
    try:
        match instruction[0]:
            case "LDI":
                binary += "0010"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2]), "016b")
            case "LDR":
                binary += "0011"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
            case "STI":
                binary += "0100"
                binary += format(int(instruction[1]), "016b")
                binary += format(int(instruction[2][1:]), "04b")
            case "STR":
                binary += "0101"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
            case "REG":
                binary += "011"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2]), "016b")
            case "MEM":
                binary += "100"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2]), "016b")
            case "ADD":
                binary += "101000"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "SUB":
                binary += "101001"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "XOR":
                binary += "101010"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
            case "XORB":
                binary += "101011"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "NOT":
                binary += "101100"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
            case "NOTB":
                binary += "101101"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "ANDB":
                binary += "101110"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "ORB":
                binary += "101111"
                binary += format(int(instruction[1][1:]), "04b")
                binary += format(int(instruction[2][1:]), "04b")
                binary += format(int(instruction[3][1:]), "04b")
            case "JUCI":
                binary += "110000"
                binary += format(int(instruction[1]), "016b")
            case "JZI":
                binary += "110001"
                binary += format(int(instruction[1]), "016b")
            case "JNI":
                binary += "110010"
                binary += format(int(instruction[1]), "016b")
            case "JOI":
                binary += "110011"
                binary += format(int(instruction[1]), "016b")
            case "JUCR":
                binary += "110100"
                binary += format(int(instruction[1][1:]), "04b")
            case "JZR":
                binary += "110101"
                binary += format(int(instruction[1][1:]), "04b")
            case "JNR":
                binary += "110110"
                binary += format(int(instruction[1][1:]), "04b")
            case "JOR":
                binary += "110111"
                binary += format(int(instruction[1][1:]), "04b")
            case "HLT\n":
                binary += "111"
            case "NOP\n":
                binary += "000"
            case _:
                binary += format(int(instruction[0]), "016b")
                type = 1

        if type == 0:
            binary = binary.ljust(32, "0")

            upper = binary[0:16]
            lower = binary[16:32]

            append(str(int(upper, 2)) + "\n")
            append(str(int(lower, 2)) + "\n")
        if type == 1:
            append(str(int(binary, 2)) + "\n")
            append("0000000000000000\n")
    except:
        print(f"\x1B[31m{num}  {"".join(instruction)}\x1B[0m")
