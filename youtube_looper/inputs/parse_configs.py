
def parse_config_map():
    d = {}
    with open("configs/yl.conf") as f:
        for line in f:
            (val, key) = line.split()
            d[key] = val
    print(d)
    return d
