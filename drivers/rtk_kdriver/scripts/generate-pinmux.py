#!/usr/bin/env python2

import sys
import re



def usage():
    print "./generate-pinmux.py <linux_base_dir> <chip_name> <pinmux_header_filename>"


if __name__ == '__main__':
    if len(sys.argv) < 4:
        usage()
        exit()

    pinmux_header_name = sys.argv[3]
    pinmux_c_name = "pinmux_internal.c"
    input_file_name=sys.argv[1] + "/drivers/rtk_kdriver/common_header/" + sys.argv[2] + "/include/rbus/" + sys.argv[3]
    fr = open(input_file_name, "r")
    output_file_name=sys.argv[1] + "/drivers/rtk_kdriver/misc/" + sys.argv[2]+ "/" + pinmux_c_name
    fw = open(output_file_name, "w")
    d = {}

    out_s = "/* Auto Created File */\n"
    out_s += "#include <rtk_kdriver/io.h>\n"
    out_s += "#include <rbus/" + pinmux_header_name + ">\n\n"

    fw.write(out_s)

    #get all pinmux from pinmux_reg.h
    for line in fr:
        if re.search(r' PINMUX_(.*)reg ', line):
            line = re.sub(' +',' ',line)
            line = line.replace('\"','')
            line = line.replace('#define ','')
            value, key = line.strip().split(' ')
            d[key.lower()] = value

    start = end = 0
    group = 0

    #create pinmux range table
    for i in sorted (d):
        if start == 0:
            group = 1
            start = end = i
        else :
            curr = int(end, 16) + 4
            curr_str = hex(curr)
            if curr_str == i:
                end = i
            else :
                out_s ="#define PINMUX_MAIN_" + str(group) + "_BEGIN " + d[start]
                fw.write(out_s + "\n")
                out_s ="#define PINMUX_MAIN_" + str(group) + "_END (" + d[end] + "+ 0x4)\n"
                fw.write(out_s + "\n")
                start = end = i
                group += 1

    if group != 0:
        out_s ="#define PINMUX_MAIN_" + str(group) + "_BEGIN " + d[start]
        fw.write(out_s + "\n")
        out_s ="#define PINMUX_MAIN_" + str(group) + "_END (" + d[end] + "+ 0x4)\n"
        fw.write(out_s + "\n")
        out_s = "unsigned int pinmux_buffer[("
        for i in range(1, group + 1):
            if i != 1:
                out_s += ' + ' + "\n\t"
            out_s += "("+"PINMUX_MAIN_" + str(i)+ "_END - PINMUX_MAIN_" + str(i) + "_BEGIN)"
        out_s += ")/4];\n\n"
        fw.write(out_s)

    #generate function
    out_s = "static int pinmux_suspend_store(unsigned int *idx, unsigned int register_begin, unsigned int register_end)\n{\n"
    out_s += "\twhile(register_begin < register_end){\n"
    out_s += "\t\tpinmux_buffer[(*idx)] = rtd_inl(register_begin);\n"
    out_s += "\t\tregister_begin += 4;\n"
    out_s += "\t\t(*idx)++;\n"
    out_s += "\t}\n"
    out_s += "\n\treturn 0;\n"
    out_s += "}\n"

    fw.write(out_s)

    out_s = "static int pinmux_resume_reload(unsigned int *idx, unsigned int register_begin, unsigned int register_end)\n{\n"
    out_s += "\twhile(register_begin < register_end){\n"
    out_s += "\t\trtd_outl(register_begin, pinmux_buffer[(*idx)]);\n"
    out_s += "\t\tregister_begin += 4;\n"
    out_s += "\t\t(*idx)++;\n"
    out_s += "\t}\n"
    out_s += "\n\treturn 0;\n"
    out_s += "}\n\n"
    fw.write(out_s)

    out_s = "\nvoid pinmux_suspend_store_range(void)\n{\n";
    out_s += "\tunsigned int idx = 0;\n"
    for i in range(1, group + 1):
        out_s += "\tpinmux_suspend_store(&idx, PINMUX_MAIN_" + str(i) + "_BEGIN, PINMUX_MAIN_" + str(i) + "_END);\n"
    out_s += "}\n"

    fw.write(out_s)

    out_s = "\nvoid pinmux_resume_reload_range(void)\n{\n";
    out_s += "\tunsigned int idx = 0;\n"
    for i in range(1, group + 1):
        out_s += "\tpinmux_resume_reload(&idx, PINMUX_MAIN_" + str(i) + "_BEGIN, PINMUX_MAIN_" + str(i) + "_END);\n"
    out_s += "}\n"

    fw.write(out_s)

    print "Done: generate file: " + output_file_name
