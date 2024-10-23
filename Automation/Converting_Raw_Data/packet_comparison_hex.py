# Program to check bit errors between 2 packets, hex

def hex_data_to_list(hex_data):
    # Remove spaces, split into values
    return hex_data.strip().split()

def compare_and_count_differences(data1, data2):
    # Convert to hex
    list1 = hex_data_to_list(data1)
    list2 = hex_data_to_list(data2)
    
    # Check if lengths are the same
    if len(list1) != len(list2):
        print(f"Data lengths are different: {len(list1)} vs {len(list2)}")
        return None

    # Counter for differences
    differences = 0
    
    # Loop to count differences
    for i in range(len(list1)):
        if list1[i] != list2[i]:
            differences += 1
            print(f"Difference at index {i}: {list1[i]} != {list2[i]}")
    
    return differences

#Original from Tx node
data1 = """
DF D0 DB 51 3E 36 7C B3 8F F6 FB 2B 3F 76 F6 DD 
DD F6 D0 0F FA 2D 3D FC 91 EF F7 76 1F ED FB BC 
C3 87 78 F0 9B 3C F0 1D D9 C1 DF DA F1 C5 9B 3E 
D2 B7 1A BC E3 E0 E1 23 6F D8 DA 78 97 0F F4 9E 
47 7F A7 E1 C7 FD E6 DE EA 01 1F F8 DB 3B F8 21 
DF EA 01 6F 78 E4 EF EE D0 C1 D1 3B 3E CE 6F EC 
8E C7 7E E0 6F ED 61 BF AB 0F FC DB EB 1C FC 88 
37 EC 8C BF D9 C3 BE CB 87 F9 5E 6F F6 D8 B7 1D 
7A CB E8 E0 DB F5 1E 70 E3 FD 1E 7A F0 FB 7A D0 
07 2C BE 55 F9 01 47 EF B8 EF 43 FC EE CA 87 16 
77 1E F0 9D DE E9 03 1C 3C FC DB F8 10 1F F1 BB 
7B D0 C1 C7 FC EE 38 76 FC DB FB 48 8F FF 2E 1E 
F8 51 DF 6F FC CD 1E 79 E7 9D 6E 3B B8 FC 88 1B 
77 1C BD F5 66 0F CC 5D 47 7F 67 8F FE A8 B7 DC 
73 C7 87 BC F3 7D 7C 9C 0F FA 3E C3 5B 59 58 DE 
1A FE F6 1E F9 21 7B D9 7D 6F 97 8D 3E E2 9B 75 
0E 1D BC EF D8 E2 CE D1 DF DD 63 DF F4 88 7B 0F 
FA 2E 1B 47 DE 76 FC 2D 07 73 C3 C1 C5 72 F9 FD 
7A E1 A3 BE 4F EF E8 F1 3B 06 1F F0 86 BD DB 86 
1F F7 BE 77 3A 70 EB D8 47 BC ED BD 1E F2 0D F8 
40 F7 DE E2 F0 F0 01 BF B5 E2 C3 0F 7F 77 D9 C3 
73 47 67 74 F4 B6 DE F0 DE DE 63 DF 54 DC 28 17 
6F B9 E7 81 6F BB E5 E8 DE 87 F8 7D BD DD F1 8F 
F9 F0 DF D6 78 36 BA 77 CB C6 C1 8F F8 3D 3D E6 
C7 FB 16 83 6F 75 DB 23 3F E2 7D 6F 34 7A F0 C6 
0D 0F FF 98 DF CA 63 3E 6E F6 06 87 DE F4 80 E3 
DF D8 83 F7 B2 F7 7B C4 5E F1 C8 9B 1E FD 9B 3A 
76 EF E8 F0 3B 7B EC 
"""
#rx
data2 = """
DF D0 DB 51 3E 36 7C B3 8F F6 FB 2B 3F 
76 F6 DD DD F6 D0 0F FA 2D 3D FC 91 EF F7 76 1F 
ED FB BC C3 87 78 F0 9B 3C F0 1D D9 C1 DF DA F1 
C5 9B 3E D2 B7 1A BC E3 E0 E1 23 6F D8 DA 78 97 
0F F4 9E 47 7F A7 E1 C7 FD E6 DE EA 01 1F F8 DB 
3B F8 21 DF EA 01 6F 78 E4 EF EE D0 C1 D1 3B 3E 
CE 6F EC 8E C7 7E E0 6F ED 61 BF AB 0F FC DB EB 
1C FC 88 37 EC 8C BF D9 C3 BE CB 87 F9 5E 6F F6 
D8 B7 1D 7A CB E8 E0 DB F5 1E 70 E3 FD 1E 7A F0 
FB 7A D0 07 2C BE 55 F9 01 47 EF B8 EF 43 FC EE 
CA 87 16 77 1E F0 9D DE E9 03 1C 3C FC DB F8 10 
1F F1 BB 7B D0 C1 C7 FC EE 38 76 FC DB FB 48 8F 
FF 2E 1E F8 51 DF 6F FC CD 1E 79 E7 9D 6E 3B B8 
FC 88 1B 77 1C BD F5 66 0F CC 5D 47 7F 67 8F FE 
A8 B7 DC 73 C7 87 BC F3 7D 7C 9C 0F FA 3E C3 5B 
59 58 DE 1A FE F6 1E F9 21 7B D9 7D 6F 97 8D 
3E E2 9B 75 0E 1D BC EF D8 E2 CE D1 DF 
DD 63 DF F4 88 7B 0F FA 2E 1B 47 DE 76 FC 2D 07 
73 C3 C1 C5 72 F9 FD 7A E1 A3 BE 4F EF E8 F1 3B 
06 1F F0 86 BD DB 86 1F F7 BE 77 3A 70 EB D8 47 
BC ED BD 1E F2 0D F8 40 F7 DE E2 F0 F0 01 BF B5 
E2 C3 0F 7F 77 D9 C3 73 47 67 74 F4 B6 DE F0 DE 
DE 63 DF 54 DC 28 17 6F B9 E7 81 6F BB E5 E8 DE 
87 F8 7D BD DD F1 8F F9 F0 DF D6 78 36 BA 77 CB 
C6 C1 8F F8 3D 3D E6 C7 FB 16 83 6F 75 DB 23 3F 
E2 7D 6F 34 7A F0 C6 0D 0F FF 98 DF CA 63 3E 6E 
F6 06 87 DE F4 80 E3 DF D8 83 F7 B2 F7 7B C4 5E 
F1 C8 9B 1E FD 9B 3A 76 EF E8 F0 3B 7B EC 



 

"""

differences = compare_and_count_differences(data1, data2)

if differences is not None:
    print(f"Total differences: {differences}")
