import traceback
import re
import matplotlib.pyplot as plt

try:
    file = open("output.txt", "r")
    result = [0, 0, 0]
    graph = []
    for line in file:
        if line.startswith("Minimum timevalue"):
            s = re.findall('\d+', line)
            result[1] = float(s[0])
        elif line.startswith("Maximum timevalue"):
            s = re.findall('\d+', line)
            result[2] = float(s[0])
        elif line.startswith("Final number"):
            # print(line)
            s = re.findall('\d+', line)
            # print(s)
            result[0] = float(s[0])
        elif line.startswith("Timeslice"):
            t = re.findall('\d+', line)
            graph.append(t)

    print(result)
    for element in graph:
        print element
        print("Timeslice {} has {} spacelike faces.".format(element[0], element[1]))

except:
    traceback.print_exc()

finally:
    file.close

print graph
timevalues = []
volume = []
for element in graph:
    timevalues.append(int(element[0]))
    volume.append(int(element[1]))
print(timevalues)
print(volume)    
plt.plot(timevalues, volume)
plt.xlabel('Timeslice')
plt.ylabel('Volume (spacelike faces)')
plt.title('Volume Profile')
plt.grid(True)
plt.show()
