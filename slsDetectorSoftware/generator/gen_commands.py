import json

# with open('commands.json') as f:
#     commands = json.load(f)


message = ["// WARNING this file is auto generated\n", 
           "// changes might be overwritten at any time\n\n"]

#Generate the header file
with open("Caller.in.h") as input:
    with open('../src/Caller.h', 'w') as output:
        output.writelines(message)
        for line in input:
            output.write(line)

#Generate the .cpp file
with open("Caller.in.cpp") as input:
    with open('../src/Caller.cpp', 'w') as output:
        output.writelines(message)
        for line in input:
            output.write(line)