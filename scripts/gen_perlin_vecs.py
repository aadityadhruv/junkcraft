c = 0
for i in range(-1,2):
    for j in range(-1,2):
        for k in range(-1,2):
            # for l in range(-1,2):
                v = [ i, j, k ]
                v = [str(x) for x in v]
                print("{", ','.join(v), "}", end=',')
                c += 1
                if c == 4:
                    print("")
                    c = 0
