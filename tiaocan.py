
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

    init_priority = random.uniform(0, 5000)
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
    map4_4 = random.uniform(0, 5000)
    f.write("double MAP4_4 = " + str(map4_4) + ";\n")
    map2_6 = random.uniform(0, 1000)
    f.write("double MAP2_6 = " + str(map2_6) + ";\n")

    f.write("\n")
    f.write("#endif\n")
    f.flush()

    t = grade.get_total_score()

    res.write(str(i) + " ==============================\n")
    res.write("INIT_PRIORITY = " + str(init_priority) + ";\n")
    res.write("PRODUCT_ID = " + str(product_id) + ";\n")
    res.write("LEVEL = " + str(level) + ";\n")
    res.write("URGENCY = " + str(urgency) + ";\n")
    res.write("NINE_WORKSHOP = " + str(nine_workshop) + ";\n")
    res.write("COLLIDE = " + str(collide) + ";\n")
    res.write("MAP4_4 = " + str(map4_4) + ";\n")
    res.write("MAP2_6 = " + str(map2_6) + ";\n")
    res.write("result: " + str(t) + "\n")
    res.flush()

    # max_score = max(max_score, t[4])

# print(max_score)
