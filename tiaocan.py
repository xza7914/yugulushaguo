
import grade
import os
import random

res = open("result.txt", 'w')

max_score = 0
for i in range(200):
    f = open("SDK/cpp_project/macro.h", 'w')

    f.write("#ifndef __MACRO_H__\n")
    f.write("#define __MACRO_H__\n")
    f.write("\n")

    init_priority = random.uniform(0, 3000)
    f.write("double INIT_PRIORITY = " + str(init_priority) + ";\n")
    product_id = 100
    f.write("double PRODUCT_ID = " + str(product_id) + ";\n")
    level = random.uniform(0, 2000)
    f.write("double LEVEL = " + str(level) + ";\n")
    urgency = random.uniform(0, 3000)
    f.write("double URGENCY = " + str(urgency) + ";\n")
    nine_workshop = random.uniform(0, 2000)
    f.write("double NINE_WORKSHOP = " + str(nine_workshop) + ";\n")
    collide = random.uniform(0, 5000)
    f.write("double COLLIDE = " + str(collide) + ";\n")

    f.write("\n")
    f.write("#endif\n")
    f.flush()

    t = grade.get_total_score()

    res.write(str(i) + " ==============================\n")
    res.write("init_priority: " + str(init_priority) + "\n")
    res.write("product_id: " + str(product_id) + "\n")
    res.write("level: " + str(level) + "\n")
    res.write("urgency: " + str(urgency) + "\n")
    res.write("nine_workshop: " + str(nine_workshop) + ";\n")
    res.write("collide: " + str(collide) + ";\n")
    res.write("result: " + str(t) + "\n")
    res.flush()

    max_score = max(max_score, t[4])

print(max_score)
