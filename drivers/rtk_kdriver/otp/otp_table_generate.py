import xlrd
import sys
#usage python3.5 otp_generate.py Merlin4_OTP.xls fieldname offset len outputFileName 
outName=sys.argv[5] if len(sys.argv)>5 else "rtk_otp_table.h"
otpHeader=open(outName,'w')
otpTable=xlrd.open_workbook(sys.argv[1])
sheet = otpTable.sheet_by_index(0)

maxLen=0
Name_Pos=int(sys.argv[2]) if len(sys.argv)>2 else 7
StartOffset_Pos=int(sys.argv[3]) if len(sys.argv)>3 else 2
Len_Pos=int(sys.argv[4]) if len(sys.argv)>4 else 3

for i in range(sheet.nrows):
    name=sheet.cell(i-1,Name_Pos-1).value
    name=name.replace(' ','_')
    name=name.replace('\n','')
    if len(name) > maxLen and sheet.cell_type(i-1,Len_Pos-1)==xlrd.XL_CELL_NUMBER:
        maxLen = len(name)
#print( sheet.name,sheet.nrows,sheet.ncols )


otpHeader.write("#ifndef __RTK_OTP_TABLE_H__\n")
otpHeader.write("#define __RTK_OTP_TABLE_H__\n\n")

otpHeader.write("//Generate by "+str(sys.argv)+"\"\n\n")

for i in range(sheet.nrows):
    name=""
    name=sheet.cell(i-1,Name_Pos-1).value
    name=name.replace(' ','_').replace('(','_')
    name=name.replace('\n','').replace(')','_')
    if name != '' and name != "reserved" and sheet.cell_type(i-1,Len_Pos-1)==xlrd.XL_CELL_NUMBER :
        #print(name.upper())
        name = "#define OTP_TABLE_" + name.upper()
        name2= name + "_LEN"
        name = name.ljust(maxLen + 18 + 4 + 4,' ')
        name2= name2.ljust(maxLen + 18 + 4 + 4,' ')
        name = name + str(int(sheet.cell(i-1,StartOffset_Pos-1).value)) + "\n"
        name2= name2 + str(int(sheet.cell(i-1,Len_Pos-1).value)) + "\n\n"
        otpHeader.write(name)
        otpHeader.write(name2)

otpHeader.write("#endif\n")

otpHeader.close()

