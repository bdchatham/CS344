#!/usr/bin/python
from random import randint

file1 = "rustie.txt"
file2 = "porterRobinson.txt"
file3 = "odesza.txt"

fileHandle1 = open(file1, "w+")
fileHandle2 = open(file2, "w+")
fileHandle3 = open(file3, "w+")
characters = ''

print "File 1, ", file1, " contains: "
for x in xrange(0, 10):
	random = randint(97, 122)
	random = chr(random)
	characters += random 
	fileHandle1.write(random)
else:
	fileHandle1.write("\n");
print characters
characters = ''

print "File 2, ", file2, " contains: "
for x in xrange(0, 10):
	random = randint(97, 122)
	random = chr(random)
	characters += random
	fileHandle2.write(random)
else:
	fileHandle2.write("\n");
print characters
characters = ''

print "File 3, ", file3, " contains: "
for x in xrange(0, 10):
	random = randint(97, 122)
	random = chr(random)
	characters += random
	fileHandle3.write(random)
else:
	fileHandle3.write("\n");
print characters
characters = ''

fileHandle1.close()
fileHandle2.close()
fileHandle3.close()

random1 = randint(1, 42)
random2 = randint(1, 42)
product = random1 * random2 

print "The product of ", random1, " and ", random2, " is: ", product
