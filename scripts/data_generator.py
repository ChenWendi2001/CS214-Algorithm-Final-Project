import json
import random


def prob_value(p):
    q = int(10 * p)
    l = [1] * q + [0] * (10 - q)
    item = random.sample(l, 1)[0]
    return item


def generate_DAG(job_id):
    # # of tasks
    # n = random.randint(6, 20)
    short_or_long = random.randint(1, 10)
    n = 0
    if short_or_long <= 8:
        n = max(2, int(random.gauss(4, 5)))
    else:
        n = max(10, int(random.gauss(20, 10)))
    num_of_resource = n + random.randint(0, 3)  # predefined

    constraint = []
    resource = []
    for i in range(num_of_resource):
        resource.append(job_id + "_resource" + str(i))

    into_degree = [0] * n
    out_degree = [0] * n
    edges = []
    for i in range(n - 1):
        for j in range(i + 1, n):
            if i == 0 and j == n - 1:
                continue
            prob = prob_value(0.4)  # possibility == 0.4
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

    task_list = []
    for i in range(n):
        if i == 0 or i == n - 1:
            continue
        task_list.append({
            "name": job_id + "_task" + str(i),
            "resource": [],
            "time": max(1, random.gauss(3, 2))
        })
        local_resource = []
        num_of_local_resource = random.randint(2, min(num_of_resource, i + 1))
        for j in range(num_of_local_resource):
            r = random.randint(0, num_of_local_resource - 1)
            if resource[r] not in local_resource:
                local_resource.append(resource[r])
                task_list[i - 1]["resource"].append({
                    "name":
                    resource[r],
                    "size":
                    max(random.gauss(400, 400), 50)
                })

    for e in edges:
        if e[0] == 0 or e[1] == n - 1:
            continue
        constraint.append(
            [task_list[e[0] - 1]["name"], task_list[e[1] - 1]["name"]])
        task_list[e[1] - 1]["resource"].append({
            "name":
            task_list[e[0] - 1]["name"],
            "size":
            random.randint(50, 300)
        })

    # task_json = json.dumps(task_list, sort_keys=True, indent=4, separators=(',', ': '))
    # print(task_json)
    return task_list, constraint, resource


def generate_DC(data):
    DC = {}
    DC_slot_size = [2, 1, 3, 2, 1, 5, 2, 2, 2, 1, 2, 4,
                    4]  # same as the toy data
    DC["DC"] = []
    for i in range(13):
        DC["DC"].append({
            "name": "DC" + str(i + 1),
            "size": DC_slot_size[i],
            "data": []
        })
    dc = random.randint(0, 12)
    for i, d in enumerate(data):
        if i % 4 == 3:
            dc = random.randint(0, 12)
        DC["DC"][dc]["data"].append(d)
    DC_json = json.dumps(DC, sort_keys=True, indent=4, separators=(',', ': '))
    with open('DC.json', 'w') as f:
        f.write(DC_json)


def generate_job_list(n):
    job = {}
    job["job"] = []
    constraint = {}
    constraint["constraint"] = []
    data = []

    for i in range(n):
        job_id = "job" + str(i)
        task_list, const, resource = generate_DAG(job_id)
        job["job"].append({"name": job_id, "task": task_list})

        for c in const:
            constraint["constraint"].append({"start": c[0], "end": c[1]})
        data = data + (resource)

    job_json = json.dumps(job,
                          sort_keys=True,
                          indent=4,
                          separators=(',', ': '))
    constraint_json = json.dumps(constraint,
                                 sort_keys=True,
                                 indent=4,
                                 separators=(',', ': '))
    with open('job_list.json', 'w') as j:
        j.write(job_json)
    with open('constraint.json', 'w') as c:
        c.write(constraint_json)

    return data


def main():
    data = generate_job_list(50)
    generate_DC(data)


if __name__ == '__main__':
    main()
