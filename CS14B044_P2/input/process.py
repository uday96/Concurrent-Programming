finp = open('10kprimes.txt','r')
fop = open('10kprimes_clean.txt','w')

for line in finp.readlines():
	parts = line.split(' ')[1:-1]
	for part in parts:
		fop.write(part+'\n')

finp.close()
fop.close()
