import random
n = 8  # number of nodes


def prob_value(p):
    q = int(10 * p)
    l = [1] * q + [0] * (10 - q)
    item = random.sample(l, 1)[0]
    return item


into_degree = [0] * n
out_degree = [0] * n
edges = []

for i in range(n - 1):
    for j in range(i + 1, n):
        if i == 0 and j == n - 1:
            continue
        prob = prob_value(0.4)  #possibility == 0.4
        if prob:
            if out_degree[i] < 2 and into_degree[j] < 2:
                edges.append((i, j))
                into_degree[j] += 1
                out_degree[i] += 1
for node, id in enumerate(into_degree):
    if node != 0:
        if id == 0:
            edges.append((0, node))
            out_degree[0] += 1
            into_degree[node] += 1
for node, od in enumerate(out_degree):
    if node != n - 1:
        if od == 0:
            edges.append((node, n - 1))
            out_degree[node] += 1
            into_degree[n - 1] += 1
print(edges)
print(into_degree, out_degree)
