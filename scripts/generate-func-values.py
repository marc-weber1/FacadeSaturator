X = [a/240 for a in list(range(0,240))]
Y = [2*a**3-3*a**2+1 for a in X]

print("{ ", end="")
for i in Y:
    print("{:.6f}f, ".format(i), end="")
print("}", end="")